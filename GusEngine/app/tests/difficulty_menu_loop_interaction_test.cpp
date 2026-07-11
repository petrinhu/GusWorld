// GusEngine/app/tests/difficulty_menu_loop_interaction_test.cpp
//
// Catch2 HEADLESS (GL REAL - Xvfb :99, MESMA receita de save_load_menu_
// interaction_test.cpp) da tela de selecao de dificuldade (MODOS-MORTE Fase 0):
// item 1 do retoque ao vivo 2026-07-10/11 ("card Hardcore bloqueado toca SFX
// grave/abafado"). Prova, por CONTAGEM/SoundId (gus::platform::audio::
// AudioEngine::sfx_play_count()/last_sfx_id() - NAO julgamento visual/print, que
// nao pega som), que hover/clique REAL de mouse (SDL_PushEvent, MESMA fila que o
// loop de producao consome) no card Hardcore BLOQUEADO toca o SFX PROPRIO
// (kMenuBlockedSfxFile), NAO o hover/click normal - e que um item SELECIONAVEL
// (Facil) continua roteado pro hover/click normal (regressao de controle: a
// mudanca NAO "vazou" pros itens normais).
//
// DEGRADACAO SEGURA: sem GL/display (sem Xvfb), cada TEST_CASE registra um INFO e
// retorna sem asserções (0 assertions, Catch2 conta "passou") - MESMO espirito de
// save_load_menu_interaction_test.cpp.
//
// Cross-ref: gus/app/screens/difficulty_menu_loop.cpp (o codigo sob teste,
//            sfx_for_item/parse_difficulty_list_item_index); gus/platform/audio/
//            audio_engine.hpp (last_sfx_id(), hook de teste ADITIVO desta onda -
//            sfx_play_count() sozinho so prova QUANTOS plays, nao QUAL SoundId,
//            insuficiente pra provar ROTEAMENTO); save_load_menu_interaction_
//            test.cpp (harness GL/SDL_PushEvent do qual este e uma variante).

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

#include <SDL3/SDL.h>
#include <glintfx/ui_layer.hpp>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/difficulty_menu.hpp"
#include "gus/app/screens/difficulty_menu_loop.hpp"
#include "gus/app/screens/difficulty_menu_rml.hpp"
#include "gus/core/asset_paths.hpp"
#include "gus/domain/save/save_data.hpp"
#include "gus/platform/assets/asset_source.hpp"
#include "gus/platform/audio/audio_engine.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"
#include "ui_box_assertions.hpp"

using namespace gus::app::screens;
using gus::app::testing::box_hittable;
using gus::platform::audio::AudioEngine;
using gus::platform::audio::kInvalidSound;
using gus::platform::audio::SoundId;

#ifndef GUSWORLD_FONTS_DIR
#define GUSWORLD_FONTS_DIR ""
#endif

namespace {

constexpr int kWinW = 960;
constexpr int kWinH = 540;

// MESMA receita de bootstrap GL de save_load_menu_interaction_test.cpp (GlTestEnv/
// try_boot_gl) - copiada aqui (arquivo AUTO-CONTIDO, MESMO nao-acoplamento entre
// harnesses ja estabelecido nas telas de producao).
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

    env.window = SDL_CreateWindow("difficulty_menu_loop_interaction_test", kWinW, kWinH,
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

std::string join_path(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

// Sonda de POSICAO (UiLayer PROPRIA, standalone) - MESMA receita de load_ui() em
// save_load_menu_interaction_test.cpp, adaptada pra build_difficulty_menu_rml.
// FECHADA (RAII, sai de escopo) ANTES do loop real abrir a SUA (RmlUi so aceita 1
// instancia viva por processo, MESMO gotcha documentado la).
std::optional<glintfx::UiLayer> load_probe_ui(const DifficultyMenuState& state,
                                               const gus::app::i18n::Translator& tr) {
    glintfx::UiLayer ui(glintfx::UiLayer::Config{/*logical_width=*/960,
                                                  /*logical_height=*/540,
                                                  /*load_gl=*/true,
                                                  /*dp_ratio=*/1.0f});
    if (!ui.ok()) return std::nullopt;

    const std::filesystem::path stage =
        std::filesystem::temp_directory_path() / "gusworld_difficulty_interaction_test";
    std::error_code ec;
    std::filesystem::create_directories(stage, ec);

    std::string fonts_dir = GUSWORLD_FONTS_DIR;
    if (const char* envf = std::getenv("GUSWORLD_FONTS")) {
        if (envf[0] != '\0') fonts_dir = envf;
    }
    if (!fonts_dir.empty()) {
        std::filesystem::copy_file(join_path(fonts_dir, "PixelOperatorMono.ttf"),
                                    stage / "PixelOperatorMono.ttf",
                                    std::filesystem::copy_options::overwrite_existing, ec);
        std::filesystem::copy_file(join_path(fonts_dir, "PixelOperatorMono-Bold.ttf"),
                                    stage / "PixelOperatorMono-Bold.ttf",
                                    std::filesystem::copy_options::overwrite_existing, ec);
    }

    std::string rml = build_difficulty_menu_rml(state, tr);
    const std::string needle = "<style>\n";
    const std::size_t pos = rml.find(needle);
    if (pos != std::string::npos) {
        rml.insert(pos + needle.size(),
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "src: \"PixelOperatorMono.ttf\"; }\n"
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "font-weight: bold; src: \"PixelOperatorMono-Bold.ttf\"; }\n");
    }
    const std::filesystem::path rml_path = stage / "difficulty_menu_probe.rml";
    {
        std::ofstream f(rml_path);
        f << rml;
    }
    ui.set_asset_base_url(stage.string().c_str());
    ui.load(rml_path.string().c_str());
    ui.set_viewport(kWinW, kWinH);
    ui.set_dp_ratio(1.0f);
    ui.update();
    ui.update();
    return ui;
}

gus::app::i18n::Translator make_translator() {
    gus::app::i18n::Translator tr;
    tr.load_from_content(
        "## SAVE_DIFFICULTY_TITLE\nEscolha a dificuldade\n\n"
        "## SAVE_DIFFICULTY_HINT\nEssa escolha e definitiva\n\n"
        "## SAVE_DIFFICULTY_FACIL_LABEL\nFacil\n\n"
        "## SAVE_DIFFICULTY_FACIL_DESC\nVolta pro ultimo save\n\n"
        "## SAVE_DIFFICULTY_MEDIO_LABEL\nMedio\n\n"
        "## SAVE_DIFFICULTY_MEDIO_BADGE\nRecomendado\n\n"
        "## SAVE_DIFFICULTY_MEDIO_DESC\nAcorda no Hospital\n\n"
        "## SAVE_DIFFICULTY_DIFICIL_LABEL\nDificil\n\n"
        "## SAVE_DIFFICULTY_DIFICIL_DESC\nAcorda longe e fraco\n\n"
        "## SAVE_DIFFICULTY_HARDCORE_LABEL\nHardcore\n\n"
        "## SAVE_DIFFICULTY_HARDCORE_DESC_LOCKED\nAlgo sombrio aguarda\n\n"
        "## SAVE_DIFFICULTY_HARDCORE_DESC_UNLOCKED\nSo para os valorosos\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_TITLE_FACIL\nJogar no Facil?\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_TITLE_MEDIO\nJogar no Medio?\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_TITLE_DIFICIL\nJogar no Dificil?\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_BODY\nNao da pra trocar depois\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_YES_FACIL\nSim, jogar no Facil\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_YES_MEDIO\nSim, jogar no Medio\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_YES_DIFICIL\nSim, jogar no Dificil\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_NO\nCancelar\n\n"
        "## SAVE_DIFFICULTY_FOOTER_HINT\nCima/Baixo navega - Enter seleciona\n\n");
    return tr;
}

// Resolve o caminho REAL de um SFX de menu (MESMO resolvedor - FilesystemAssetSource
// + kSfxDir - que resolve_menu_sfx_path em difficulty_menu_loop.cpp usa; a funcao
// de producao e `static` no .cpp, entao replicamos aqui a MESMA receita de 1
// linha, nao a logica de negocio).
std::string resolve_sfx_path(std::string_view file) {
    const std::string id = join_path(std::string(gus::core::assets::kSfxDir), std::string(file));
    return gus::platform::assets::FilesystemAssetSource().resolve_path(id);
}

// IDs esperados (hover=1, click=2, blocked=3, 1-based na ordem de load_sfx - ver o
// comentario de last_sfx_id() em audio_engine.hpp) via uma AudioEngine de SONDA
// PROPRIA, descartada antes da AudioEngine real do teste - como AMBAS partem
// vazias e carregam os MESMOS 3 arquivos NA MESMA ORDEM que
// run_difficulty_menu_loop_gl_current carrega internamente, os SoundId batem
// (determinismo do contador 1-based, nao coincidencia).
struct ExpectedSfxIds {
    SoundId hover = kInvalidSound;
    SoundId click = kInvalidSound;
    SoundId blocked = kInvalidSound;
};

ExpectedSfxIds probe_expected_sfx_ids() {
    AudioEngine probe(/*device_active=*/false);
    ExpectedSfxIds ids;
    ids.hover = probe.load_sfx(resolve_sfx_path(gus::core::assets::kMenuHoverSfxFile).c_str());
    ids.click = probe.load_sfx(resolve_sfx_path(gus::core::assets::kMenuClickSfxFile).c_str());
    ids.blocked =
        probe.load_sfx(resolve_sfx_path(gus::core::assets::kMenuBlockedSfxFile).c_str());
    return ids;
}

}  // namespace

// ---------------------------------------------------------------- carrega o .wav

TEST_CASE("difficulty_menu_loop (harness headless): kMenuBlockedSfxFile carrega "
          "(SoundId valido, arquivo .wav real em assets/sfx/) e e DISTINTO do "
          "hover/click normal",
          "[difficulty_menu_loop_interaction]") {
    // Sem GL: load_sfx depende so do device de audio (null-device aceito, ver
    // AudioEngine), NAO de janela/GL - roda sempre, sem degradacao.
    const ExpectedSfxIds ids = probe_expected_sfx_ids();
    REQUIRE(ids.hover != kInvalidSound);
    REQUIRE(ids.click != kInvalidSound);
    REQUIRE(ids.blocked != kInvalidSound);
    REQUIRE(ids.blocked != ids.hover);
    REQUIRE(ids.blocked != ids.click);
}

// ---------------------------------------------------------------- hover REAL no bloqueado

TEST_CASE("difficulty_menu_loop (harness headless): hover de mouse REAL "
          "(SDL_PushEvent) no card Hardcore BLOQUEADO toca o SFX bloqueado, NAO "
          "o hover normal (item 1, retoque 2026-07-10/11)",
          "[difficulty_menu_loop_interaction][gl]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel neste ambiente (sem Xvfb) - harness pulado "
             "(degradacao segura, 0 assercoes). Rode com Xvfb :99 (export "
             "DISPLAY=:99) pra exercitar de fato.");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();
    const ExpectedSfxIds ids = probe_expected_sfx_ids();

    // Posicao REAL do card Hardcore (indice 3, SEMPRE visivel/bloqueado nesta
    // Fase 0 - ver difficulty_menu_open/hardcore_unlocked=false default) - MESMO
    // documento/viewport/dp_ratio (960x540, dp_ratio=1.0) que
    // run_difficulty_menu_loop_gl_current monta internamente.
    DifficultyMenuState probe_state;
    difficulty_menu_open(probe_state);
    float item3_cx = 0.0f, item3_cy = 0.0f;
    {
        auto probe_ui = load_probe_ui(probe_state, translator);
        REQUIRE(probe_ui.has_value());
        const glintfx::ElementBox box = probe_ui->get_element_box("difficulty-item-3");
        REQUIRE(box_hittable(box, kWinW, kWinH));
        item3_cx = box.x + box.w * 0.5f;
        item3_cy = box.y + box.h * 0.5f;
        // FECHA a sondagem ANTES do loop real abrir a SUA (RmlUi 1-instancia).
    }

    SDL_Event motion_ev{};
    motion_ev.type = SDL_EVENT_MOUSE_MOTION;
    motion_ev.motion.x = item3_cx;
    motion_ev.motion.y = item3_cy;
    REQUIRE(SDL_PushEvent(&motion_ev));

    SDL_Event esc_ev{};
    esc_ev.type = SDL_EVENT_KEY_DOWN;
    esc_ev.key.key = SDLK_ESCAPE;
    esc_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&esc_ev));

    AudioEngine audio(/*device_active=*/false);  // sem hardware no CI - fresh, MESMA
                                                  // ordem de load de probe_expected_sfx_ids()
    gus::domain::save::DifficultyLevel out_difficulty{};
    const DifficultyLoopExit exit =
        run_difficulty_menu_loop_gl_current(env.window, audio, translator, &out_difficulty);

    // ANTES desta onda (hover sem roteamento condicional): o hover no Hardcore
    // bloqueado tocaria hover_sfx_id (o normal) - este REQUIRE teria FALHADO.
    REQUIRE(exit == DifficultyLoopExit::Cancelled);
    REQUIRE(audio.sfx_play_count() == 1);
    REQUIRE(audio.last_sfx_id() == ids.blocked);
    REQUIRE(audio.last_sfx_id() != ids.hover);
}

// ---------------------------------------------------------------- clique REAL no bloqueado

TEST_CASE("difficulty_menu_loop (harness headless): clique de mouse REAL "
          "(SDL_PushEvent) no card Hardcore BLOQUEADO toca o SFX bloqueado, NAO "
          "o click normal, e permanece no-op DE ESTADO (nao abre o splash)",
          "[difficulty_menu_loop_interaction][gl]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();
    const ExpectedSfxIds ids = probe_expected_sfx_ids();

    DifficultyMenuState probe_state;
    difficulty_menu_open(probe_state);
    float item3_cx = 0.0f, item3_cy = 0.0f;
    {
        auto probe_ui = load_probe_ui(probe_state, translator);
        REQUIRE(probe_ui.has_value());
        const glintfx::ElementBox box = probe_ui->get_element_box("difficulty-item-3");
        REQUIRE(box_hittable(box, kWinW, kWinH));
        item3_cx = box.x + box.w * 0.5f;
        item3_cy = box.y + box.h * 0.5f;
    }

    SDL_Event motion_ev{};
    motion_ev.type = SDL_EVENT_MOUSE_MOTION;
    motion_ev.motion.x = item3_cx;
    motion_ev.motion.y = item3_cy;
    REQUIRE(SDL_PushEvent(&motion_ev));

    SDL_Event click_ev{};
    click_ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    click_ev.button.button = SDL_BUTTON_LEFT;
    click_ev.button.x = item3_cx;
    click_ev.button.y = item3_cy;
    REQUIRE(SDL_PushEvent(&click_ev));

    // 1 SO Esc: se o clique no bloqueado tivesse (incorretamente) aberto o
    // splash de confirmacao, este Esc so fecharia o splash (None) e o loop
    // ficaria esperando o PROXIMO evento pra sempre (fila vazia) - o teste
    // travaria em vez de falhar rapido, expondo a regressao de forma bem mais
    // visivel que um REQUIRE comum (contrato de no-op TOTAL de
    // difficulty_menu_click_option, ja provado a parte em difficulty_menu_test.cpp).
    SDL_Event esc_ev{};
    esc_ev.type = SDL_EVENT_KEY_DOWN;
    esc_ev.key.key = SDLK_ESCAPE;
    esc_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&esc_ev));

    AudioEngine audio(/*device_active=*/false);
    gus::domain::save::DifficultyLevel out_difficulty{};
    const DifficultyLoopExit exit =
        run_difficulty_menu_loop_gl_current(env.window, audio, translator, &out_difficulty);

    // ANTES desta onda (click sem roteamento condicional): o clique no Hardcore
    // bloqueado tocaria click_sfx_id (o normal) - este REQUIRE teria FALHADO.
    REQUIRE(exit == DifficultyLoopExit::Cancelled);
    REQUIRE(audio.sfx_play_count() == 2);  // hover (motion) + click, ambos bloqueados
    REQUIRE(audio.last_sfx_id() == ids.blocked);
    REQUIRE(audio.last_sfx_id() != ids.click);
}

// ---------------------------------------------------------------- regressao: item NORMAL

TEST_CASE("difficulty_menu_loop (harness headless): hover+clique REAL num item "
          "SELECIONAVEL (Facil) continua roteado pro hover/click NORMAL, nunca "
          "pro SFX bloqueado (controle de regressao do item 1)",
          "[difficulty_menu_loop_interaction][gl]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();
    const ExpectedSfxIds ids = probe_expected_sfx_ids();

    // Facil = indice 0, SEMPRE selecionavel (ver difficulty_item_selectable).
    DifficultyMenuState probe_state;
    difficulty_menu_open(probe_state);
    float item0_cx = 0.0f, item0_cy = 0.0f;
    {
        auto probe_ui = load_probe_ui(probe_state, translator);
        REQUIRE(probe_ui.has_value());
        const glintfx::ElementBox box = probe_ui->get_element_box("difficulty-item-0");
        REQUIRE(box_hittable(box, kWinW, kWinH));
        item0_cx = box.x + box.w * 0.5f;
        item0_cy = box.y + box.h * 0.5f;
    }

    SDL_Event motion_ev{};
    motion_ev.type = SDL_EVENT_MOUSE_MOTION;
    motion_ev.motion.x = item0_cx;
    motion_ev.motion.y = item0_cy;
    REQUIRE(SDL_PushEvent(&motion_ev));

    SDL_Event click_ev{};
    click_ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    click_ev.button.button = SDL_BUTTON_LEFT;
    click_ev.button.x = item0_cx;
    click_ev.button.y = item0_cy;
    REQUIRE(SDL_PushEvent(&click_ev));

    // Facil E selecionavel -> o clique ABRE o splash de confirmacao de fato
    // (diferente do teste do bloqueado acima) - precisa de 2 Esc pra sair
    // (fecha o splash, depois sai da lista), MESMO racional documentado no
    // comentario do teste anterior.
    SDL_Event esc1_ev{};
    esc1_ev.type = SDL_EVENT_KEY_DOWN;
    esc1_ev.key.key = SDLK_ESCAPE;
    esc1_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&esc1_ev));

    SDL_Event esc2_ev = esc1_ev;
    REQUIRE(SDL_PushEvent(&esc2_ev));

    AudioEngine audio(/*device_active=*/false);
    gus::domain::save::DifficultyLevel out_difficulty{};
    const DifficultyLoopExit exit =
        run_difficulty_menu_loop_gl_current(env.window, audio, translator, &out_difficulty);

    REQUIRE(exit == DifficultyLoopExit::Cancelled);
    REQUIRE(audio.sfx_play_count() == 2);  // hover (motion) + click, ambos NORMAIS
    REQUIRE(audio.last_sfx_id() == ids.click);
    REQUIRE(audio.last_sfx_id() != ids.blocked);
}
