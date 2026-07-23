// GusEngine/app/tests/screen_state_sync_input_integration_test.cpp
//
// F4-1a QA-FOLLOWUP (auditoria adversarial): teste de INTEGRACAO que amarra a
// CORRENTE REAL - gus::app::run_screen_state + gus::app::SdlWindow::
// sync_input_event + gus::platform::input::SdlInput (por dentro de SdlWindow) -
// reproduzindo a SEQUENCIA EXATA do bug relatado ao vivo pelo lider ("Gus anda
// sozinho apos fechar o dialogo com o Bertoldo"): tecla de movimento pressionada
// ANTES do modal abrir, solta DURANTE o modal, cidade correta DEPOIS. Nenhum
// componente e FALSO aqui (ao contrario de screen_state_test.cpp, que usa um
// ScreenState fake pra testar SO o contrato do runner) - SdlWindow e o objeto de
// PRODUCAO real (headless: ctor nao pede SDL_Init/janela/GL, MESMO estado que
// sdl_window_marker_defer_test.cpp ja prova seguro), e o sync_hook passado a
// run_screen_state e EXATAMENTE a lambda que Maestro::to_npc_dialogue() usa
// (`[&city](const SDL_Event& ev) { city.sync_input_event(ev); }`).
//
// O 2o caso (SEM sync_hook) e o CONTROLE: prova que e o MECANISMO que fecha o
// bug, nao coincidencia - sem ele, a mesma sequencia de eventos reproduz o bug
// histórico (a tecla fica "presa").

#include <catch2/catch_test_macros.hpp>

#include "gus/app/sdl_window.hpp"
#include "gus/app/screen_state.hpp"

using gus::app::SdlWindow;

namespace {

constexpr int kKeyD = 'd';  // SDLK_D

// ScreenState MINIMO representando "o dialogo esta aberto": nao reage ao
// KEY_UP de forma nenhuma (MESMO comportamento real de NpcDialogueScreen - o
// switch de handle_event() la nao tem case pra KEY_UP, so KEY_DOWN/MOUSE_*) -
// so existe pra dar ao runner um ScreenState de verdade pra rodar. Termina
// apos processar 1 evento (simula "o dialogo consumiu o frame onde a tecla foi
// solta e, em seguida, o jogador fechou a conversa").
class DialogueLikeScreen : public gus::app::ScreenState {
public:
    int enter_calls = 0;
    int handled = 0;

    void enter() override { ++enter_calls; }
    void handle_event(const SDL_Event&) override { ++handled; }
    void tick(float) override {}
    [[nodiscard]] bool finished() const override { return handled >= 1; }
    void exit() override {}
    [[nodiscard]] bool window_closed() const override { return false; }
};

gus::app::EventPollFn make_single_key_up_batch(SDL_Keycode key) {
    auto delivered = std::make_shared<bool>(false);
    return [delivered, key](SDL_Event& ev) -> bool {
        if (*delivered) return false;
        *delivered = true;
        ev = SDL_Event{};
        ev.type = SDL_EVENT_KEY_UP;
        ev.key.key = key;
        return true;
    };
}

}  // namespace

TEST_CASE("INTEGRACAO F4-1a: tecla pressionada ANTES do modal + solta DURANTE "
          "(run_screen_state + sync_input_event real) -> cidade correta DEPOIS, "
          "SEM clear_input() de saida (a corrente completa que fecha o bug)",
          "[screen_state][sdlwindow][f4-1a][integration]") {
    SdlWindow city;

    // 1) Tecla pressionada ANTES do modal (MESMO estado que city_->step() real
    // teria deixado nos frames normais anteriores, antes do jogador esbarrar
    // no NPC).
    city.process_key_for_test(kKeyD, /*pressed=*/true);
    REQUIRE(city.input_dx() == 1);

    // 2) O modal abre e roda - MESMA chamada que Maestro::to_npc_dialogue faz
    // (run_screen_state com o sync_hook real apontando pra sync_input_event) -
    // e o jogador SOLTA D durante a conversa (o SDL_EVENT_KEY_UP chega aqui).
    DialogueLikeScreen screen;
    gus::app::EventPollFn poll_fn = make_single_key_up_batch(SDLK_D);
    gus::app::EventSyncHook sync_hook = [&city](const SDL_Event& ev) {
        city.sync_input_event(ev);
    };

    gus::app::run_screen_state(screen, sync_hook, poll_fn, [] { return 0ULL; });

    REQUIRE(screen.enter_calls == 1);
    REQUIRE(screen.handled == 1);

    // 3) O modal fechou (SEM nenhum clear_input() de saida - Maestro::
    // to_npc_dialogue REMOVEU essa chamada nesta fatia, ver o comentario la) -
    // a cidade retoma e D JA esta corretamente solta.
    REQUIRE(city.input_dx() == 0);
}

TEST_CASE("INTEGRACAO F4-1a (CONTROLE - prova que e o MECANISMO, nao "
          "coincidencia): a MESMA sequencia SEM o sync_hook reproduz o bug "
          "historico - a tecla solta DURANTE o modal fica presa",
          "[screen_state][sdlwindow][f4-1a][integration][regressao-dialogo]") {
    SdlWindow city;
    city.process_key_for_test(kKeyD, /*pressed=*/true);
    REQUIRE(city.input_dx() == 1);

    DialogueLikeScreen screen;
    gus::app::EventPollFn poll_fn = make_single_key_up_batch(SDLK_D);

    // SEM sync_hook (nullptr) - MESMA situacao de ANTES desta fatia (o loop
    // modal fazia o proprio SDL_PollEvent, isolado do SdlInput da cidade): o
    // KEY_UP e visto pela tela (screen.handled==1) mas NUNCA chega em city.
    gus::app::run_screen_state(screen, /*sync_hook=*/nullptr, poll_fn,
                               [] { return 0ULL; });

    REQUIRE(screen.handled == 1);
    // BUG REPRODUZIDO: sem o sync_hook, D continua "presa" - a cidade
    // retomaria andando sozinha (o mecanismo que este teste isola e o MESMO
    // que fecha o bug no teste acima, nao coincidencia).
    REQUIRE(city.input_dx() == 1);
}
