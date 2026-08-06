// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/battle_preview_interaction_test.cpp
//
// Catch2 HEADLESS (GL REAL - Xvfb :99, MESMA receita de difficulty_menu_loop_
// interaction_test.cpp/save_load_menu_interaction_test.cpp) do HOST REAL da batalha
// (F4-1b.5, gus::app::screens::run_battle_preview_embedded_gl_current) - o FIO
// INTEIRO ate a funcao de producao que a Maestro chama, provando o wiring de saida
// (SDL_EVENT_QUIT real fecha a janela, o MESMO sinal que battle_screen_step_test.cpp
// prova puro em battle_screen_should_close_on_event).
//
// SETUP PESADO (avisado no brief da fatia): o enter() da BattleScreen monta a cena de
// demo inteira (party+inimigos), carrega retratos/sprites/icones/traducao e liga o
// glintfx::UiLayer do cockpit - MUITO mais custoso que o das telas de menu (Difficulty/
// SaveLoad/Title). Este teste prova o MINIMO que garante o wiring de saida ponta-a-
// ponta (QUIT real -> *out_quit_requested==true) SEM depender de nenhum asset externo
// (fade_in/fade_out=0.0f, ver os defaults do header - o teste nao exercita as fases de
// fade, ja cobertas puras em battle_screen_step_test.cpp) - degrada seguro (0
// assercoes) se GL/display estiver indisponivel, MESMO espirito dos harnesses irmaos.
//
// Cross-ref: gus/app/screens/battle_preview.hpp (a funcao sob teste);
//            battle_screen_step_test.cpp (a FSM de fase PURA, sem GL/janela);
//            difficulty_menu_loop_interaction_test.cpp (o harness GlTestEnv/try_boot_gl
//            do qual este e uma variante, copiado AUTO-CONTIDO por convencao da suite).

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>

#include <SDL3/SDL.h>

#include "gus/app/screens/battle_preview.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/platform/audio/audio_engine.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"

namespace {

// MESMA receita de bootstrap GL de difficulty_menu_loop_interaction_test.cpp (GlTestEnv/
// try_boot_gl) - copiada aqui (arquivo AUTO-CONTIDO, MESMO nao-acoplamento entre
// harnesses ja estabelecido nas telas de producao). run_battle_preview_embedded_gl_
// current EXIGE que o glad ja esteja carregado (gl3_load_functions) - MESMA pre-
// condicao que run_battle_preview_embedded (o wrapper que cria o contexto de verdade)
// satisfaz antes de chamar o nucleo.
struct GlTestEnv {
    SDL_Window* window = nullptr;
    SDL_GLContext gl = nullptr;
    bool ok = false;

    ~GlTestEnv() {
        if (gl != nullptr) SDL_GL_DestroyContext(gl);
        if (window != nullptr) SDL_DestroyWindow(window);
    }
};

GlTestEnv try_boot_gl() {
    GlTestEnv env;
    if (!SDL_WasInit(SDL_INIT_VIDEO) && !SDL_InitSubSystem(SDL_INIT_VIDEO)) return env;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    env.window = SDL_CreateWindow("battle_preview_interaction_test", 960, 540,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (env.window == nullptr) return env;

    env.gl = SDL_GL_CreateContext(env.window);
    if (env.gl == nullptr) return env;
    SDL_GL_MakeCurrent(env.window, env.gl);

    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        return env;
    }
    env.ok = true;
    return env;
}

// SFX-COCKPIT: env var COM RAII. A BattleScreen le os env vars de diagnostico dentro do
// proprio enter(), e a suite Catch2 roda TODOS os casos no MESMO processo - deixar uma
// var setada contaminaria os testes seguintes (o de QUIT acima, por exemplo, passaria a
// abrir a batalha ja no modo-mira). Nao ha "unsetenv depois" confiavel sem RAII: um
// REQUIRE que falha no meio do teste sai da funcao por excecao.
class ScopedEnv {
public:
    ScopedEnv(const char* name, const char* value) : name_(name) {
        const char* previous = std::getenv(name);
        had_previous_ = previous != nullptr;
        if (had_previous_) previous_ = previous;
        setenv(name, value, /*overwrite=*/1);
    }
    ~ScopedEnv() {
        if (had_previous_) {
            setenv(name_, previous_.c_str(), /*overwrite=*/1);
        } else {
            unsetenv(name_);
        }
    }
    ScopedEnv(const ScopedEnv&) = delete;
    ScopedEnv& operator=(const ScopedEnv&) = delete;

private:
    const char* name_;
    bool had_previous_ = false;
    std::string previous_;
};

// SFX-COCKPIT: os SoundId sao 1-BASED na ORDEM de load_sfx (contrato explicito de
// gus/platform/audio/audio_engine.hpp, que documenta este uso: "um consumidor de teste
// que sabe a ordem de load_sfx do caminho de producao pode comparar last_sfx_id() contra
// o id esperado"). BattleScreen::enter() carrega, nesta ordem exata:
//   1 = SFX de HIT (resolve_hit_sfx_path)
//   2 = blip de HOVER de UI (kMenuHoverSfxFile)
//   3 = blip de CLIQUE de UI (kMenuClickSfxFile)
//   4 = blip de BLOQUEADO de UI (kMenuBlockedSfxFile) - SFX-COCKPIT-AJUSTES, 2026-08-06
//   5 = CONFIRMACAO da abertura (kUiConfirmSfxFile) - SFX-ABERTURA-TIMBRE, 2026-08-06
// O engine e' construido PELO TESTE e passado como external_audio, entao nada mais e'
// carregado nele antes. Se alguem inserir um load_sfx novo antes desses, estes testes
// falham ALTO (e o conserto e' 1 linha aqui) - preferivel a um teste que so conta plays
// e nao sabe QUAL blip tocou, que e' o que deixaria "tocou o som errado" passar.
//
// O 4o e o 5o entraram no FIM do enter() de proposito, justamente pra NAO deslocar os ids
// acima: ZERO destes casos precisou de ajuste por causa da POSICAO deles (o unico caso que
// mudou foi o da ABERTURA, e por DECISAO DE TIMBRE do lider, nao por deslocamento).
constexpr gus::platform::audio::SoundId kHitSfxId = 1;
constexpr gus::platform::audio::SoundId kUiHoverSfxId = 2;
constexpr gus::platform::audio::SoundId kUiClickSfxId = 3;
constexpr gus::platform::audio::SoundId kUiBlockedSfxId = 4;
constexpr gus::platform::audio::SoundId kUiConfirmSfxId = 5;

void push_key_down(SDL_Keycode key) {
    SDL_Event ev{};
    ev.type = SDL_EVENT_KEY_DOWN;
    ev.key.key = key;
    ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&ev));
}

void push_quit() {
    SDL_Event ev{};
    ev.type = SDL_EVENT_QUIT;
    REQUIRE(SDL_PushEvent(&ev));
}

// Roda o HOST REAL da batalha com o AudioEngine do teste e devolve quantos play_sfx
// aconteceram DEPOIS que o setup terminou - ou seja, os que o INPUT causou.
//
// POR QUE DELTA E NAO ABSOLUTO (medido, nao suposto): o bloco GUSWORLD_BATTLE_AIM de
// enter() assenta a cena bombeando `skip() + update()` ate a vez do jogador, e nesse
// caminho os INIMIGOS AGEM - cada golpe dispara o SFX de HIT no MESMO engine. Medi 2 e 3
// hits em rodadas diferentes do mesmo teste: o absoluto NAO e' deterministico. Comparar
// contra um numero fixo daria um teste que falha sozinho (ou, pior, que passa por
// coincidencia). O `sync_hook` de run_screen_state recebe TODO evento pumpado ANTES de a
// tela trata-lo, entao a 1a chamada dele e' exatamente a fronteira "setup acabou, o
// primeiro input ainda nao foi tratado" - e a costura que faltava pra medir so o input.
unsigned int run_and_measure(gus::platform::audio::AudioEngine& audio, SDL_Window* window,
                             bool expect_quit) {
    unsigned int baseline = 0;
    bool captured = false;
    const gus::app::EventSyncHook hook = [&](const SDL_Event&) {
        if (!captured) {
            baseline = audio.sfx_play_count();
            captured = true;
        }
    };

    gus::domain::combat::CombatOutcome outcome = gus::domain::combat::CombatOutcome::Ongoing;
    bool quit_requested = false;
    gus::app::screens::run_battle_preview_embedded_gl_current(
        window, &outcome, &quit_requested, &audio, /*fade_in_seconds=*/0.0f,
        /*fade_out_seconds=*/0.0f, hook);

    REQUIRE(quit_requested == expect_quit);
    REQUIRE(captured);  // o hook viu ao menos 1 evento (senao o delta seria fake)
    return audio.sfx_play_count() - baseline;
}

unsigned int input_sfx_plays(gus::platform::audio::AudioEngine& audio, SDL_Window* window) {
    return run_and_measure(audio, window, /*expect_quit=*/true);
}

}  // namespace

// ---------------------------------------------------------------- QUIT real (F4-1b.5)

TEST_CASE("battle_preview (harness headless): SDL_EVENT_QUIT real (SDL_PushEvent) "
          "fecha a janela DURANTE a batalha - run_battle_preview_embedded_gl_current "
          "grava *out_quit_requested=true (FIX BUG-3, o MESMO sinal que a Maestro usa "
          "pra NAO voltar a cidade)",
          "[battle_preview_interaction][gl]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel neste ambiente (sem Xvfb) - harness pulado "
             "(degradacao segura, 0 assercoes). Rode com Xvfb :99 (export DISPLAY=:99) "
             "pra exercitar de fato.");
        return;
    }

    // QUIT ja na fila ANTES do 1o pump: o enter() da BattleScreen roda um setup pesado
    // (cena de demo + assets), mas nenhum self-test/fade esta ativo aqui
    // (fade_in_seconds/fade_out_seconds default 0.0f) - o 1o handle_event ja deve
    // consumir o QUIT e encerrar sem desenhar frame nenhum (MESMO guard `if (!running)
    // break;`/window_closed() do host).
    SDL_Event quit_ev{};
    quit_ev.type = SDL_EVENT_QUIT;
    REQUIRE(SDL_PushEvent(&quit_ev));

    gus::domain::combat::CombatOutcome outcome = gus::domain::combat::CombatOutcome::Ongoing;
    bool quit_requested = false;
    gus::app::screens::run_battle_preview_embedded_gl_current(
        env.window, &outcome, &quit_requested, /*external_audio=*/nullptr,
        /*fade_in_seconds=*/0.0f, /*fade_out_seconds=*/0.0f);

    // ANTES desta fatia (se a propagacao SDL_EVENT_QUIT -> BattleScreen::window_closed_
    // -> *out_quit_requested tivesse quebrado no meio da conversao pra ScreenState): o
    // teste TRAVARIA esperando o PROXIMO evento pra sempre (fila vazia apos o QUIT) em
    // vez de falhar rapido - MESMO padrao de deteccao ja documentado em
    // difficulty_menu_loop_interaction_test.cpp/title_menu_loop_interaction_test.cpp.
    REQUIRE(quit_requested);
    // Fechar a janela NAO e nenhum CombatOutcome (nao e vitoria/derrota/fuga) - a FSM
    // de combate nunca chegou a CombatEnd (a Maestro trata isto como "abortou", ver o
    // comentario grande em battle_preview.hpp).
    REQUIRE(outcome == gus::domain::combat::CombatOutcome::Ongoing);
}

// ============================================================== SFX-COCKPIT (2026-08-06)
//
// A FIACAO do som no HOST REAL. A DECISAO (que blip sai, quantas vezes, em qual
// transicao) e' travada headless em app/tests/battle_ui_sfx_test.cpp; o que ESTES casos
// provam e' o que so a funcao de producao pode provar: que BattleScreen::
// handle_event_main_ de fato CHAMA aquela camada, no evento SDL certo.
//
// Sem eles, o defeito que a auditoria achou continuaria de pe do lado do host: era
// possivel apagar o disparo de som e ver a suite inteira VERDE.
//
// RECEITA (por que ela e' deterministica, e nao um teste de sorte):
//   - GUSWORLD_BATTLE_AIM=1 assenta a cena NO MODO-MIRA ainda DENTRO do enter(), de forma
//     SINCRONA (o bloco stop_in_aim de battle_preview.cpp) - nao dependemos de N frames de
//     pacing nem de geometria de RmlUi ja calculada.
//   - todos os eventos sao enfileirados ANTES da chamada; run_screen_state drena a fila
//     inteira no 1o pump e o QUIT final encerra ANTES do 1o tick() - logo NENHUM frame e'
//     desenhado e NENHUM update() roda, o que elimina qualquer SFX de combate (hit) como
//     ruido na contagem.
//   - a prova e' por sfx_play_count() E por last_sfx_id(): a contagem sozinha nao
//     distinguiria "tocou o blip certo" de "tocou o som de hit".

TEST_CASE("battle_preview (harness headless, SFX-COCKPIT): CONTROLE - abrir a batalha no "
          "modo-mira e sair sem NENHUM input nao toca blip de UI algum (prova que a "
          "contagem dos casos abaixo mede o INPUT, nao ruido do setup)",
          "[battle_preview_interaction][gl][sfx-cockpit]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }
    const ScopedEnv aim("GUSWORLD_BATTLE_AIM", "1");

    push_quit();

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);
    REQUIRE(input_sfx_plays(audio, env.window) == 0u);
}

TEST_CASE("battle_preview (harness headless, SFX-COCKPIT): superficie MIRA / canal "
          "TECLADO - a seta que troca de inimigo toca o blip de HOVER no HOST REAL "
          "(ANTES desta fatia: silencio; grep play_sfx battle_input.cpp = 0, e o hover so "
          "saia no ramo SDL_EVENT_MOUSE_MOTION)",
          "[battle_preview_interaction][gl][sfx-cockpit]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }
    const ScopedEnv aim("GUSWORLD_BATTLE_AIM", "1");

    push_key_down(SDLK_DOWN);  // aim_move(+1): o encontro de demo tem 2 inimigos vivos
    push_quit();

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);
    REQUIRE(input_sfx_plays(audio, env.window) == 1u);
    REQUIRE(audio.last_sfx_id() == kUiHoverSfxId);  // hover, NAO clique, NAO hit
}

TEST_CASE("battle_preview (harness headless, SFX-COCKPIT): superficie MIRA / canal "
          "TECLADO - Enter que CONFIRMA o alvo toca o blip de CLIQUE no HOST REAL, e SO "
          "ele (confirmar nao soma um hover junto)",
          "[battle_preview_interaction][gl][sfx-cockpit]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }
    const ScopedEnv aim("GUSWORLD_BATTLE_AIM", "1");

    push_key_down(SDLK_RETURN);  // aim_confirm
    push_quit();

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);
    REQUIRE(input_sfx_plays(audio, env.window) == 1u);
    REQUIRE(audio.last_sfx_id() == kUiClickSfxId);
}

TEST_CASE("battle_preview (harness headless, SFX-COCKPIT): superficie MIRA / canal "
          "TECLADO - Esc CANCELA em silencio (cancelar nao e acionar); o blip so volta "
          "quando o jogador navega de novo, ja no menu de verbos",
          "[battle_preview_interaction][gl][sfx-cockpit]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }
    const ScopedEnv aim("GUSWORLD_BATTLE_AIM", "1");

    push_key_down(SDLK_ESCAPE);  // aim_cancel -> volta ao menu de verbos, MUDO
    push_key_down(SDLK_DOWN);    // ja no menu: navega um verbo -> 1 HOVER
    push_quit();

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);
    REQUIRE(input_sfx_plays(audio, env.window) == 1u);
    REQUIRE(audio.last_sfx_id() == kUiHoverSfxId);
    // (Se o Esc tivesse soado, seriam 2. Se a seta no MENU DE VERBOS nao soasse - o
    // buraco de paridade teclado x mouse - seria 0.)
}

TEST_CASE("battle_preview (harness headless, SFX-COCKPIT): a tecla que NAO navega nem "
          "aciona (Q, auto-resolve placeholder) e' muda no HOST REAL",
          "[battle_preview_interaction][gl][sfx-cockpit]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }
    const ScopedEnv aim("GUSWORLD_BATTLE_AIM", "1");

    push_key_down(SDLK_Q);
    push_quit();

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);
    REQUIRE(input_sfx_plays(audio, env.window) == 0u);
    // O unico som da sessao foi o HIT que o proprio setup (skip+update ate a vez do
    // jogador) disparou - nenhum blip de UI entrou depois dele.
    REQUIRE(audio.last_sfx_id() == kHitSfxId);
}

TEST_CASE("battle_preview (harness headless, SFX-COCKPIT-AJUSTES): a ABERTURA soa no HOST "
          "REAL - o Enter que 'Encara' toca 1 blip (decisao do lider 2026-08-06; a fatia "
          "anterior deixava a intro MUDA de proposito)",
          "[battle_preview_interaction][gl][sfx-cockpit]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }
    // SEM GUSWORLD_BATTLE_AIM de proposito: e' o UNICO caso desta suite que precisa da
    // cena PARADA na abertura ("BATALHA!"), que e' justamente o estado que aquele env
    // atravessa. Consequencia util: sem o assentamento, o setup nao bombeia turno de
    // inimigo nenhum, entao nao ha SFX de hit competindo pela contagem.
    push_key_down(SDLK_RETURN);  // Encarar
    push_quit();

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);
    REQUIRE(input_sfx_plays(audio, env.window) == 1u);
    // O timbre de HOJE = kBattleOpeningSfxSlot == BattleUiSfxSlot::Confirm
    // (battle_ui_sfx.hpp), decisao do lider em SFX-ABERTURA-TIMBRE (2026-08-06). Se ele
    // escolher outro som pra abertura, ESTA linha muda junto com aquela constante - de
    // proposito, pra a troca de timbre nao passar despercebida.
    //
    // Prova pelo ID no HOST REAL: a contagem `== 1u` acima sozinha nao distingue "tocou o
    // som da abertura" de "tocou qualquer outro blip carregado no enter()". Este e' o
    // unico caso da suite que atravessa a cadeia inteira - load_sfx de producao, ordem
    // real dos SoundId, roteamento de teclado do host - ate o alto-falante.
    REQUIRE(audio.last_sfx_id() == kUiConfirmSfxId);
    // E NAO e' nenhum dos outros 4 que o mesmo enter() carregou (o clique era o timbre
    // PROVISORIO que esta fatia substituiu; enumerados um a um, nao "!= o de antes").
    REQUIRE(audio.last_sfx_id() != kUiClickSfxId);
    REQUIRE(audio.last_sfx_id() != kUiBlockedSfxId);
    REQUIRE(audio.last_sfx_id() != kUiHoverSfxId);
    REQUIRE(audio.last_sfx_id() != kHitSfxId);
}

TEST_CASE("battle_preview (harness headless, SFX-COCKPIT): superficie PILLS DE VERBO / "
          "canal MOUSE - a varredura sintetica dos 6 pills toca 6 hovers (1 por pill), 0 "
          "de repique, +1 ao sair-e-voltar e +1 no clique, atravessando a QUERY DE CAIXA "
          "REAL do glintfx - o unico ponto que nenhum outro teste alcanca",
          "[battle_preview_interaction][gl][sfx-cockpit]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }
    // ESTE CASO EXISTE POR UM MUTANTE QUE SOBREVIVEU. Com os 5 casos acima + os 32
    // headless no lugar, trocar `b.found` por `false` na montagem das UiHoverBox de
    // BattleScreen::handle_cockpit_hover_ (isto e': o cockpit deixa de achar QUALQUER
    // pill) mantinha a suite INTEIRA verde, 2997/2997. Aquele laco e' o unico pedaco do
    // som do cockpit que so existe COM RmlUi vivo, e os outros 5 casos GL nao passam por
    // ele: eles enfileiram todos os eventos ANTES da chamada, e run_screen_state drena a
    // fila inteira no 1o pump - ANTES do 1o ui_->update(), quando a geometria do
    // documento ainda nao assentou e get_element_box devolve found=false pra tudo.
    //
    // A SAIDA nao foi inventar um jeito de injetar evento no frame N: o proprio host ja
    // tem esse caminho pronto - GUSWORLD_BATTLE_UI_SFX_SELFTEST=1 assenta a cena ate a
    // vez do jogador, espera 12 frames pela geometria e ENTAO varre MouseMoves sinteticos
    // pelos 6 pills chamando o MESMO handle_cockpit_hover_ de producao. Ele existia desde
    // 2026-07-04 e SO IMPRIMIA no terminal: nao estava no ctest, nem no CI, nem no
    // check.sh - "prova" que ninguem rodava (achado (1) da auditoria). Aqui ele deixa de
    // ser um printf e vira assercao.
    const ScopedEnv selftest("GUSWORLD_BATTLE_UI_SFX_SELFTEST", "1");

    // 1 evento inofensivo SO pra o sync_hook capturar a baseline (ele so dispara com
    // evento). Canto superior esquerdo: fora da coluna do cockpit - e, no frame 0, a
    // geometria nem existe ainda. Nao ha QUIT: o proprio self-test encerra a tela
    // (running_=false) quando termina a prova, entao expect_quit=false.
    SDL_Event motion{};
    motion.type = SDL_EVENT_MOUSE_MOTION;
    motion.motion.x = 5.0f;
    motion.motion.y = 5.0f;
    REQUIRE(SDL_PushEvent(&motion));

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);
    const unsigned int plays = run_and_measure(audio, env.window, /*expect_quit=*/false);

    // 6 (varredura) + 0 (repique) + 1 (sair-e-voltar) + 1 (clique ATACAR) = 8.
    REQUIRE(plays == 8u);
    REQUIRE(audio.last_sfx_id() == kUiClickSfxId);  // o ultimo gesto foi o clique
}
