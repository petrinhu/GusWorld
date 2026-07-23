// GusEngine/app/tests/screen_state_test.cpp
//
// Catch2 (headless, SEM SDL_Init/janela/GL) de gus::app::run_screen_state (F4-1a,
// onda F4 "casca SDL -> App mode do glintfx", fatia 1: loops modais bloqueantes ->
// maquina de estados com 1 unico pump de eventos por frame - ver gus/app/
// screen_state.hpp). O runner e PURO o suficiente pra ser exercitado com um
// ScreenState FALSO e poll_fn/now_fn SINTETICOS (injetados, sem tocar
// SDL_PollEvent/SDL_GetTicksNS reais) - prova o CONTRATO (ordem enter/handle_event/
// tick/exit, o fan-out do EventSyncHook, os 2 guards de "nao roda 1 frame extra
// apos terminar/fechar a janela") sem depender de display/GL nenhum. O CONSUMIDOR
// real (NpcDialogueScreen, GL-heavy) continua sem teste direto - ver o topo de
// gus/app/src/screens/npc_dialogue_loop_gl.cpp.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "gus/app/screen_state.hpp"

namespace {

// ScreenState FALSO: grava toda chamada (enter/handle_event/tick/exit) em
// contadores/vetores simples - permite AFIRMAR ordem e contagem exata sem
// nenhuma dependencia de GL/glintfx/SDL real.
class FakeScreen : public gus::app::ScreenState {
public:
    int enter_calls = 0;
    int exit_calls = 0;
    std::vector<Uint32> handled_event_types;
    std::vector<float> tick_dts;
    bool finished_flag = false;
    bool window_closed_flag = false;
    // -1 = nao termina por contagem de eventos/ticks (so via finished_flag
    // setado direto pelo teste, se for o caso).
    int finish_after_events = -1;
    int finish_after_ticks = -1;

    void enter() override { ++enter_calls; }

    void handle_event(const SDL_Event& ev) override {
        handled_event_types.push_back(ev.type);
        if (finish_after_events >= 0 &&
            static_cast<int>(handled_event_types.size()) >= finish_after_events) {
            finished_flag = true;
        }
    }

    void tick(float dt) override {
        tick_dts.push_back(dt);
        if (finish_after_ticks >= 0 &&
            static_cast<int>(tick_dts.size()) >= finish_after_ticks) {
            finished_flag = true;
        }
    }

    [[nodiscard]] bool finished() const override { return finished_flag; }
    void exit() override { ++exit_calls; }
    [[nodiscard]] bool window_closed() const override { return window_closed_flag; }
};

// Fonte de eventos SINTETICA organizada em "rajadas" (cada rajada = os eventos
// que 1 SDL_PollEvent real entregaria numa iteracao do loop antes de esvaziar a
// fila) - MESMO contrato de retorno de SDL_PollEvent (true enquanto ha evento,
// false quando a rajada atual esgota).
gus::app::EventPollFn make_batched_poll_fn(std::vector<std::vector<Uint32>> batches,
                                            std::size_t* out_batch_idx = nullptr) {
    auto batches_ptr = std::make_shared<std::vector<std::vector<Uint32>>>(std::move(batches));
    auto batch_idx = std::make_shared<std::size_t>(0);
    auto event_idx = std::make_shared<std::size_t>(0);
    if (out_batch_idx != nullptr) *out_batch_idx = 0;
    return [batches_ptr, batch_idx, event_idx](SDL_Event& ev) -> bool {
        if (*batch_idx >= batches_ptr->size()) return false;
        const auto& current = (*batches_ptr)[*batch_idx];
        if (*event_idx >= current.size()) {
            ++(*batch_idx);
            *event_idx = 0;
            return false;  // rajada atual esgotada - o runner tenta tick()
        }
        ev = SDL_Event{};
        ev.type = current[*event_idx];
        ++(*event_idx);
        return true;
    };
}

gus::app::ClockNowNsFn make_zero_clock() {
    return [] { return 0ULL; };
}

}  // namespace

TEST_CASE("run_screen_state: enter() 1x, despacha eventos por handle_event() na "
          "ordem, tick() apos drenar CADA rajada, exit() 1x ao terminar",
          "[screen_state][f4-1a]") {
    FakeScreen screen;
    screen.finish_after_ticks = 3;  // termina exatamente apos a 3a rajada

    gus::app::EventPollFn poll_fn = make_batched_poll_fn({
        {SDL_EVENT_KEY_DOWN, SDL_EVENT_MOUSE_MOTION},
        {},  // rajada vazia (nenhum evento neste frame) - ainda assim tick() roda
        {SDL_EVENT_KEY_UP},
    });

    unsigned long long clock_ns = 0;
    gus::app::ClockNowNsFn now_fn = [&clock_ns] {
        clock_ns += 1'000'000;  // 1ms por chamada
        return clock_ns;
    };

    gus::app::run_screen_state(screen, /*sync_hook=*/nullptr, poll_fn, now_fn);

    REQUIRE(screen.enter_calls == 1);
    REQUIRE(screen.exit_calls == 1);
    REQUIRE(screen.handled_event_types.size() == 3);
    REQUIRE(screen.handled_event_types[0] == SDL_EVENT_KEY_DOWN);
    REQUIRE(screen.handled_event_types[1] == SDL_EVENT_MOUSE_MOTION);
    REQUIRE(screen.handled_event_types[2] == SDL_EVENT_KEY_UP);
    REQUIRE(screen.tick_dts.size() == 3);
    REQUIRE(screen.tick_dts[0] == 0.0f);  // 1o tick: sem "last" ainda - dt=0
    REQUIRE(screen.tick_dts[1] > 0.0f);   // ticks seguintes: dt real (now_fn injetado)
    REQUIRE(screen.tick_dts[2] > 0.0f);
}

TEST_CASE("run_screen_state: TODO evento pumpado vai pro EventSyncHook, na MESMA "
          "ordem que screen.handle_event() recebe (F4-1a - a peca que fecha "
          "'Gus anda sozinho apos fechar o dialogo' pela raiz)",
          "[screen_state][f4-1a]") {
    FakeScreen screen;
    screen.finish_after_events = 2;  // termina assim que o 2o evento for tratado

    std::vector<Uint32> synced;
    gus::app::EventSyncHook sync_hook = [&synced](const SDL_Event& ev) {
        synced.push_back(ev.type);
    };

    gus::app::EventPollFn poll_fn =
        make_batched_poll_fn({{SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP}});

    gus::app::run_screen_state(screen, sync_hook, poll_fn, make_zero_clock());

    REQUIRE(synced.size() == 2);
    REQUIRE(synced[0] == SDL_EVENT_KEY_DOWN);
    REQUIRE(synced[1] == SDL_EVENT_KEY_UP);
    REQUIRE(screen.handled_event_types == synced);  // MESMOS eventos, MESMA ordem
}

TEST_CASE("run_screen_state: se a tela terminar NO MEIO do drenar de uma rajada, "
          "para de despachar eventos na hora e NAO chama tick() (sem desenhar 1 "
          "frame extra apos o evento que encerrou)",
          "[screen_state][f4-1a]") {
    FakeScreen screen;
    screen.finish_after_events = 1;  // termina no 1o evento

    gus::app::EventPollFn poll_fn = make_batched_poll_fn(
        {{SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP, SDL_EVENT_MOUSE_MOTION}});

    gus::app::run_screen_state(screen, /*sync_hook=*/nullptr, poll_fn, make_zero_clock());

    REQUIRE(screen.handled_event_types.size() == 1);  // parou de drenar na hora
    REQUIRE(screen.tick_dts.empty());                 // nenhum tick() chamado
    REQUIRE(screen.enter_calls == 1);
    REQUIRE(screen.exit_calls == 1);
}

TEST_CASE("run_screen_state: window_closed()==true encerra o loop mesmo com "
          "finished() ainda false (SDL_EVENT_QUIT) - tick() nunca roda depois",
          "[screen_state][f4-1a]") {
    class QuitScreen : public gus::app::ScreenState {
    public:
        int enter_calls = 0;
        int exit_calls = 0;
        bool closed = false;
        void enter() override { ++enter_calls; }
        void handle_event(const SDL_Event& ev) override {
            if (ev.type == SDL_EVENT_QUIT) closed = true;
        }
        void tick(float) override { FAIL("tick() nao deveria rodar apos QUIT"); }
        [[nodiscard]] bool finished() const override { return false; }
        void exit() override { ++exit_calls; }
        [[nodiscard]] bool window_closed() const override { return closed; }
    } screen;

    gus::app::EventPollFn poll_fn = make_batched_poll_fn({{SDL_EVENT_QUIT}});

    gus::app::run_screen_state(screen, /*sync_hook=*/nullptr, poll_fn, make_zero_clock());

    REQUIRE(screen.enter_calls == 1);
    REQUIRE(screen.exit_calls == 1);
    REQUIRE(screen.closed == true);
}

// F4-1a QA-FOLLOWUP (auditoria adversarial, achado 2): a auditoria escreveu um
// ScreenState cujo tick() chama run_screen_state() de novo - ANTES desta
// guarda, isso compilava e RODAVA (2418/2418 verde) sem protecao nenhuma
// contra reentrada, quebrando a EXCLUSIVIDADE DO UILAYER (gus/app/
// screen_state.hpp): 2 telas com enter() chamado sem o exit() da anterior ja
// ter rodado. Prova que a guarda FALHA ALTO (excecao, nao silencio) e que a
// flag e liberada de novo depois (RAII) - uma chamada NORMAL seguinte
// funciona sem problema.
TEST_CASE("run_screen_state: reentrada (ScreenState chamando run_screen_state "
          "de dentro do proprio tick()) lanca std::logic_error na hora - a "
          "exclusividade do UiLayer e invariante em codigo, nao convencao",
          "[screen_state][f4-1a][mutation]") {
    class InnerFakeScreen : public gus::app::ScreenState {
    public:
        void enter() override {}
        void handle_event(const SDL_Event&) override {}
        void tick(float) override {}
        [[nodiscard]] bool finished() const override { return true; }
        void exit() override {}
        [[nodiscard]] bool window_closed() const override { return false; }
    };

    class ReentrantOuterScreen : public gus::app::ScreenState {
    public:
        int tick_calls = 0;
        void enter() override {}
        void handle_event(const SDL_Event&) override {}
        void tick(float) override {
            ++tick_calls;
            // Tenta rodar um 2o run_screen_state() de DENTRO do tick() do 1o -
            // exatamente o cenario que a auditoria reproduziu.
            InnerFakeScreen inner;
            gus::app::EventPollFn inner_poll = [](SDL_Event&) { return false; };
            gus::app::run_screen_state(inner, /*sync_hook=*/nullptr, inner_poll,
                                       [] { return 0ULL; });
        }
        [[nodiscard]] bool finished() const override { return tick_calls >= 1; }
        void exit() override {}
        [[nodiscard]] bool window_closed() const override { return false; }
    };

    ReentrantOuterScreen outer;
    gus::app::EventPollFn outer_poll = [](SDL_Event&) { return false; };  // 1
        // rajada vazia -> 1 tick (onde a reentrada acontece)

    REQUIRE_THROWS_AS(
        gus::app::run_screen_state(outer, /*sync_hook=*/nullptr, outer_poll,
                                   [] { return 0ULL; }),
        std::logic_error);

    // A guarda foi LIBERADA mesmo apos a excecao (RAII) - uma chamada normal/
    // nao-reentrante SEGUINTE continua funcionando (nao ficou "travada").
    InnerFakeScreen again;
    gus::app::EventPollFn again_poll = [](SDL_Event&) { return false; };
    REQUIRE_NOTHROW(gus::app::run_screen_state(again, /*sync_hook=*/nullptr,
                                               again_poll, [] { return 0ULL; }));
}

TEST_CASE("run_screen_state: pula o loop inteiro se a tela ja estiver finished() "
          "logo apos enter() (ex.: glintfx::UiLayer::ok()==false) - exit() ainda "
          "roda, poll_fn NUNCA e chamado",
          "[screen_state][f4-1a]") {
    class BailScreen : public gus::app::ScreenState {
    public:
        int enter_calls = 0;
        int exit_calls = 0;
        void enter() override { ++enter_calls; }
        void handle_event(const SDL_Event&) override {
            FAIL("handle_event() nao deveria ser chamado");
        }
        void tick(float) override { FAIL("tick() nao deveria ser chamado"); }
        [[nodiscard]] bool finished() const override { return true; }
        void exit() override { ++exit_calls; }
        [[nodiscard]] bool window_closed() const override { return false; }
    } screen;

    gus::app::EventPollFn poll_fn = [](SDL_Event&) -> bool {
        FAIL("poll_fn nao deveria ser chamado - a tela ja terminou logo apos enter()");
        return false;
    };

    gus::app::run_screen_state(screen, /*sync_hook=*/nullptr, poll_fn, make_zero_clock());

    REQUIRE(screen.enter_calls == 1);
    REQUIRE(screen.exit_calls == 1);
}
