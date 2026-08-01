// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/save_load_menu_interaction_test.cpp
//
// Catch2 HEADLESS (GL REAL - Xvfb :99, nao so a logica pura) da tela de Salvar/
// Carregar (SAVE-LOAD-UI): prova POSICIONAMENTO (slot nao invade a scrollbar da
// lista, bug 2/8) + INTERACAO (Voltar/slots/icone-de-apagar sao hit-testaveis, e
// um CLIQUE DE MOUSE REAL - SDL_PushEvent, injetado na MESMA fila que o loop de
// producao consome - de fato dispara a acao esperada, bugs 1/3/6/9 relatados ao
// vivo: "clico e nada acontece"/"Voltar nao responde ao mouse"). Este e o
// entregavel-chave pedido pelo lider: os testes anteriores desta base so
// verificavam PNGs ESTATICOS (nao pegam interacao quebrada nem colisao fina de
// layout) - este harness EXERCITA de fato get_element_box + o roteamento de
// mouse do loop real.
//
// DEGRADACAO SEGURA: se NAO houver GL/display disponivel (sandbox/CI sem Xvfb,
// driver dummy sem GL de verdade), cada TEST_CASE registra um INFO e RETORNA sem
// nenhuma assercao (Catch2 conta "passou", 0 assertions) - NUNCA falha o SUITE
// por falta de display. Rode com Xvfb :99 (DISPLAY=:99, opcionalmente
// SDL_VIDEODRIVER=x11) pra exercitar de fato - MESMO espirito de degradacao
// segura ja usado no resto desta base (ex.: glintfx::UiLayer::ok()==false).
//
// GENERICO: os helpers de assercao (ui_box_assertions.hpp) nao dependem de nada
// especifico DESTA tela - a intencao e reusar em telas glintfx futuras (o
// bootstrap GL local (GlTestEnv/try_boot_gl) tambem e copiavel/factorable se um
// 2o harness precisar da mesma receita).
//
// Cross-ref: gus/app/screens/save_load_menu_loop.hpp/.cpp (o codigo sob teste);
//            ui_box_assertions.hpp (helpers genericos de box); app/tools/
//            save_load_screenshot_probe.cpp (mesma receita de UiLayer standalone,
//            efemero/nao commitado - este arquivo E o analogo COMMITADO/testavel).

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <SDL3/SDL.h>
#include <glintfx/ui_layer.hpp>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/save_load_menu.hpp"
#include "gus/app/screens/save_load_menu_loop.hpp"
#include "gus/app/screens/save_load_menu_rml.hpp"
#include "gus/domain/save/save_data.hpp"
#include "gus/platform/audio/audio_engine.hpp"
#include "gus/platform/fs/save_file_store.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"
#include "ui_box_assertions.hpp"

// stb_image_write: item 2 (retoque ao vivo 2026-07-10/11), prova visual PNG do
// dialogo "Deseja salvar no Espaco [N] (vazio)?" - SO a DECLARACAO aqui (a
// IMPLEMENTACAO ja vive UMA vez em battle_preview.cpp/gusengine_app, linkado por
// este alvo de teste - nao redefinir STB_IMAGE_WRITE_IMPLEMENTATION 2x, MESMO
// padrao de difficulty_menu_loop.cpp/save_load_menu_loop.cpp).
#include "stb_image_write.h"

using namespace gus::app::screens;
using gus::app::testing::box_hittable;
using gus::app::testing::horizontal_gap;
using gus::domain::save::kAutosaveSlot;
using gus::domain::save::kSlotCount;
using gus::domain::save::SaveData;

#ifndef GUSWORLD_FONTS_DIR
#define GUSWORLD_FONTS_DIR ""
#endif

namespace {

constexpr int kWinW = 960;
constexpr int kWinH = 540;

// Ambiente GL do teste (RAII): janela ESCONDIDA + contexto GL 3.3 core,
// destruidos automaticamente. `ok=false` sinaliza degradacao segura (ver
// try_boot_gl) - o CHAMADOR (TEST_CASE) nunca faz REQUIRE contra isto direto.
struct GlTestEnv {
    SDL_Window* window = nullptr;
    SDL_GLContext gl = nullptr;
    bool ok = false;

    ~GlTestEnv() {
        if (gl != nullptr) SDL_GL_DestroyContext(gl);
        if (window != nullptr) SDL_DestroyWindow(window);
    }
};

// Tenta montar um contexto GL 3.3 core REAL numa janela ESCONDIDA - MESMA
// receita de bootstrap de run_system_menu_loop_owning_gl/os probes efemeros de
// app/tools/. Qualquer passo que falhar (sem display/Xvfb, driver dummy sem GL)
// devolve env.ok=false - NUNCA lanca/aborta (degradacao segura, o TEST_CASE
// decide pular).
GlTestEnv try_boot_gl() {
    GlTestEnv env;
    if (!SDL_WasInit(SDL_INIT_VIDEO) && !SDL_InitSubSystem(SDL_INIT_VIDEO)) return env;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    env.window = SDL_CreateWindow("save_load_menu_interaction_test", kWinW, kWinH,
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

// Escreve build_save_load_menu_rml num arquivo temporario e carrega numa
// glintfx::UiLayer PROPRIA - INJETANDO A MESMA FONTE REAL que
// write_save_load_rml_file (save_load_menu_loop.cpp) injeta em producao.
// ACHADO EMPIRICO (nao obvio, registrado pra nao re-diagnosticar): sem a fonte
// real, o fallback do RmlUi tem metricas DIFERENTES (line-height) - a altura da
// linha 0 muda, deslocando a posicao Y de TODAS as linhas seguintes - um
// harness que mede posicao SEM a mesma fonte da producao mede um alvo ERRADO
// (a suite so pegou isto rodando o clique de fato e vendo o slot 1 nao bater).
// 2x update() por seguranca (layout assentado apos load() trocar de documento -
// mesma cautela de system_menu_loop.cpp).
std::optional<glintfx::UiLayer> load_ui(const SaveLoadMenuState& state,
                                         const gus::app::i18n::Translator& tr) {
    glintfx::UiLayer ui(glintfx::UiLayer::Config{/*logical_width=*/960,
                                                  /*logical_height=*/540,
                                                  /*load_gl=*/true,
                                                  /*dp_ratio=*/1.0f});
    if (!ui.ok()) return std::nullopt;

    const std::filesystem::path stage =
        std::filesystem::temp_directory_path() / "gusworld_save_load_interaction_test";
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

    std::string rml = build_save_load_menu_rml(state, tr);
    const std::string needle = "<style>\n";
    const std::size_t pos = rml.find(needle);
    if (pos != std::string::npos) {
        rml.insert(pos + needle.size(),
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "src: \"PixelOperatorMono.ttf\"; }\n"
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "font-weight: bold; src: \"PixelOperatorMono-Bold.ttf\"; }\n");
    }
    const std::filesystem::path rml_path = stage / "save_load_menu.rml";
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
        "## SAVE_SCREEN_TITLE_SAVE\nSalvar\n\n"
        "## SAVE_SCREEN_TITLE_LOAD\nCarregar\n\n"
        "## SAVE_SCREEN_SUBTITLE_SAVE\n{0}\n\n"
        "## SAVE_SCREEN_SUBTITLE_LOAD\n{0}\n\n"
        "## SAVE_SCREEN_FOOTER_SAVE\nx\n\n"
        "## SAVE_SCREEN_FOOTER_LOAD\nx\n\n"
        "## SAVE_SLOT_EMPTY\nVazio {0}\n\n"
        "## SAVE_SLOT_LABEL\nEspaco {0}\n\n"
        "## SAVE_SLOT_AUTO_NAME\nAuto\n\n"
        "## SAVE_SLOT_READONLY_TAG\n(so-leitura)\n\n"
        "## SAVE_XP_LABEL\nXP {0}\n\n"
        "## SAVE_CHAPTER_LABEL\nCap. {0}\n\n"
        "## SAVE_CONFIRM_OVERWRITE\nSobrescrever este slot?\n\n"
        "## SAVE_OVERWRITE_CONFIRM_YES\nSim, sobrescrever\n\n"
        "## SAVE_OVERWRITE_CONFIRM_NO\nCancelar\n\n"
        "## SAVE_CONFIRM_EMPTY\nDeseja salvar no Espaco {0} (vazio)?\n\n"
        "## SAVE_EMPTY_CONFIRM_YES\nSalvar\n\n"
        "## SAVE_EMPTY_CONFIRM_NO\nCancelar\n\n"
        "## SAVE_DELETE_BUTTON_LABEL\nApagar\n\n"
        "## SAVE_LOAD_WARN_DAMAGED\nEste save esta danificado.\n\n"
        "## SAVE_LOAD_WARN_VERSION\nEste save e de uma versao mais nova.\n\n"
        "## SAVE_LOAD_RECOVER_TRY\nTentar recuperar\n\n"
        "## SAVE_LOAD_RECOVER_FAILED\nNao foi possivel recuperar.\n\n"
        "## SAVE_LOAD_SLOT_DAMAGED_LABEL\n! Danificado\n\n"
        "## SAVE_LOAD_SLOT_VERSION_LABEL\n! Versao incompativel\n\n"
        "## SAVE_LOAD_WARN_CANCEL\nCancelar\n\n"
        "## SETTINGS_BACK\nVoltar\n\n"
        "## LOCATION_PRACA_COMPILACAO\nx\n\n"
        "## LOCATION_UNKNOWN\nx\n\n");
    return tr;
}

SaveData make_save_data(int xp, std::int64_t timestamp_ms = 1783455240000LL,
                         double playtime_seconds = 2532.0) {
    SaveData data;
    data.current_scene_path = "distritos_inferiores";
    data.timestamp_ms = timestamp_ms;
    data.playtime_seconds = playtime_seconds;
    data.character_states["gus"].xp = xp;
    return data;
}

// Mix REALISTA (autosave ocupado + 1 manual ocupado + resto vazio) - MESMO
// padrao de app/tools/save_load_screenshot_probe.cpp.
std::array<SaveSlotPreview, kSlotCount> make_mixed_slots() {
    std::array<SaveSlotPreview, kSlotCount> slots{};
    slots[kAutosaveSlot] = build_slot_preview(make_save_data(550), kAutosaveSlot);
    slots[1] = build_slot_preview(make_save_data(340), 1);
    for (int i = 2; i < kSlotCount; ++i) {
        slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    }
    return slots;
}

}  // namespace

// ---------------------------------------------------------------- (a)(b)(c) posicionamento

TEST_CASE("save_load_menu (harness headless): slots nao invadem a scrollbar da "
          "lista (bug 2/8) + Voltar/slots/icone-de-apagar sao hit-testaveis (bug "
          "1/6/9)",
          "[save_load_menu_interaction][gl]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel neste ambiente (sem Xvfb) - harness "
             "pulado (degradacao segura, 0 assercoes). Rode com Xvfb :99 "
             "(export DISPLAY=:99) pra exercitar de fato.");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Load, make_mixed_slots());

    auto ui = load_ui(state, translator);
    REQUIRE(ui.has_value());

    // (a) NENHUM slot invade a faixa da scrollbar (8dp de largura - RCSS
    // "#slmenu-list scrollbarvertical", ver save_load_menu_rml.cpp), com folga
    // minima >= 4px. ANTES do fix (.slot width:400dp + .slot-list padding-
    // right:6dp), o gap era 0 (slot.right == scrollbar.left EXATO) - este
    // REQUIRE teria FALHADO no codigo de ontem.
    constexpr float kScrollbarWidthDp = 8.0f;
    constexpr float kMinGap = 4.0f;
    const glintfx::ElementBox list_box = ui->get_element_box("slmenu-list");
    REQUIRE(list_box.found);
    const glintfx::ElementBox scrollbar_left_edge{
        /*found=*/true, list_box.x + list_box.w - kScrollbarWidthDp, list_box.y,
        kScrollbarWidthDp, list_box.h};
    for (int i = 0; i < kSlotCount; ++i) {
        const glintfx::ElementBox slot =
            ui->get_element_box(("slmenu-slot-" + std::to_string(i)).c_str());
        REQUIRE(slot.found);
        INFO("slot " << i << ": gap ate a scrollbar = "
                      << horizontal_gap(slot, scrollbar_left_edge) << "px");
        REQUIRE(horizontal_gap(slot, scrollbar_left_edge) >= kMinGap);
    }

    // (b) Voltar e hit-testavel (pegaria "Voltar morto pro mouse", bug 1/6/9 -
    // ANTES desta onda, save_load_menu_loop.cpp NAO tratava NENHUM
    // SDL_EVENT_MOUSE_BUTTON_DOWN, entao o clique caia no vazio mesmo com a
    // caixa presente/valida).
    REQUIRE(box_hittable(ui->get_element_box("slmenu-back"), kWinW, kWinH));

    // (c) icone de apagar dos slots OCUPADOS (0=autosave, 1=manual) e
    // hit-testavel (feature "Apagar").
    REQUIRE(box_hittable(ui->get_element_box("slmenu-delete-0"), kWinW, kWinH));
    REQUIRE(box_hittable(ui->get_element_box("slmenu-delete-1"), kWinW, kWinH));

    // (c) cada slot SELECIONAVEL (0 e 1, ambos ocupados, modo Load) e
    // hit-testavel e esta dentro do recorte visivel da lista.
    for (const int slot : {0, 1}) {
        const glintfx::ElementBox box =
            ui->get_element_box(("slmenu-slot-" + std::to_string(slot)).c_str());
        REQUIRE(box_hittable(box, kWinW, kWinH));
    }
}

// ---------------------------------------------------------------- (d) clique real dispara a acao

// ATUALIZADO (item 2, retoque ao vivo 2026-07-10/11): desde o AJUSTE polish
// playtest 2026-07-10, um clique num slot vazio NAO salva mais DIRETO - abre o
// mini-dialogo de confirmacao (MESMO gate que ja existia pra slot ocupado, ver
// o comentario extenso em save_load_menu.hpp). Este teste (regressao original
// "bug 3/4 - clico e nada acontece") foi ATUALIZADO pra confirmar o dialogo
// (clique em "Salvar") em vez de assumir save direto - a REGRESSAO ORIGINAL
// (clique nao fazia NADA) continua coberta pelo mesmo REQUIRE(build_called)/
// REQUIRE(has_save) no fim, so que agora passando pelo gate de confirmacao. O
// caso "sem confirmar" (Esc fecha SO o dialogo, NAO salva) tem teste PROPRIO
// dedicado, ver "(f) item 2" abaixo. ACHADO AO VIVO nesta atualizacao: a
// versao ANTERIOR deste teste (so 1 Esc apos o clique) ficou HANG ate o
// timeout do ctest (282s) apos o item 2 mudar o comportamento - o Esc unico
// fechava so o dialogo (OverwriteCancelled, loop CONTINUA), nunca chegava a
// SaveLoadLoopExit nenhum - REGISTRADO como o exemplo vivo de por que TODO
// harness baseado em fila de eventos fixa precisa ser revisitado quando a
// maquina de estado que ele exercita ganha um passo a mais.
TEST_CASE("save_load_menu (harness headless): clique de mouse REAL (SDL_PushEvent) "
          "no slot vazio, confirmado no mini-dialogo, dispara o save de fato em "
          "disco (bug 3/4 - 'clico e nada acontece')",
          "[save_load_menu_interaction][gl]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    const std::filesystem::path saves_dir =
        std::filesystem::temp_directory_path() / "gusworld_save_load_interaction_saves";
    std::filesystem::remove_all(saves_dir);  // hermetico (nunca o $HOME real do host)

    std::array<SaveSlotPreview, kSlotCount> empty_slots{};
    for (int i = 0; i < kSlotCount; ++i) {
        empty_slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    }

    // Pre-mede a posicao REAL do slot 1 (vazio, selecionavel em modo Save) - o
    // MESMO documento/viewport/dp_ratio (960x540, dp_ratio=1.0 numa janela de
    // 960px) que run_save_load_menu_loop_gl_current monta INTERNAMENTE - layout
    // deterministico, a posicao medida aqui bate com a que o loop de producao
    // usa de fato.
    SaveLoadMenuState probe_state;
    save_load_menu_open(probe_state, SaveLoadMode::Save, empty_slots);
    float slot1_cx = 0.0f, slot1_cy = 0.0f;
    {
        auto probe_ui = load_ui(probe_state, translator);
        REQUIRE(probe_ui.has_value());
        const glintfx::ElementBox slot1_box = probe_ui->get_element_box("slmenu-slot-1");
        REQUIRE(box_hittable(slot1_box, kWinW, kWinH));
        slot1_cx = slot1_box.x + slot1_box.w * 0.5f;
        slot1_cy = slot1_box.y + slot1_box.h * 0.5f;
        // FECHA a UiLayer de sondagem ANTES do loop real abrir a SUA PROPRIA
        // (RmlUi NAO suporta 2 UiLayer simultaneos no processo - crash real ja
        // documentado, ver o comentario extenso em system_menu_loop.cpp).
    }

    // Pre-mede a posicao REAL da pill "Salvar" (slmenu-confirm-yes) no
    // mini-dialogo JA ABERTO pra um slot vazio (item 2) - MESMA receita de
    // 2-probes ja usada pelo teste (e)/SAVE-LOAD-AVISOS abaixo.
    SaveLoadMenuState probe_confirm;
    save_load_menu_open(probe_confirm, SaveLoadMode::Save, empty_slots);
    (void)save_load_menu_click_slot(probe_confirm, 1);
    REQUIRE(probe_confirm.confirming_overwrite);
    float yes_cx = 0.0f, yes_cy = 0.0f;
    {
        auto probe_ui = load_ui(probe_confirm, translator);
        REQUIRE(probe_ui.has_value());
        const glintfx::ElementBox box = probe_ui->get_element_box("slmenu-confirm-yes");
        REQUIRE(box_hittable(box, kWinW, kWinH));
        yes_cx = box.x + box.w * 0.5f;
        yes_cy = box.y + box.h * 0.5f;
    }

    // Injeta os eventos NA FILA REAL do SDL (SDL_PushEvent) - a MESMA fila que
    // SDL_PollEvent dentro do loop de producao consome:
    //   1) MOUSE_MOTION ate o centro do slot 1 (assenta o hover nativo/RCSS);
    //   2) MOUSE_BUTTON_DOWN no MESMO ponto - abre o mini-dialogo (item 2);
    //   3-4) motion+click na pill "Salvar" - confirma, salva de fato;
    //   5) KEY_DOWN Escape - fecha a tela de volta (senao o while(true) do loop
    //      ficaria esperando o PROXIMO evento pra sempre; o teste precisa que a
    //      chamada RETORNE - OverwriteConfirmed faz do_save+reload SEM fechar a
    //      tela, ver handle_action em save_load_menu_loop.cpp).
    SDL_Event motion_ev{};
    motion_ev.type = SDL_EVENT_MOUSE_MOTION;
    motion_ev.motion.x = slot1_cx;
    motion_ev.motion.y = slot1_cy;
    REQUIRE(SDL_PushEvent(&motion_ev));

    SDL_Event click_ev{};
    click_ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    click_ev.button.button = SDL_BUTTON_LEFT;
    click_ev.button.x = slot1_cx;
    click_ev.button.y = slot1_cy;
    REQUIRE(SDL_PushEvent(&click_ev));

    SDL_Event motion2_ev{};
    motion2_ev.type = SDL_EVENT_MOUSE_MOTION;
    motion2_ev.motion.x = yes_cx;
    motion2_ev.motion.y = yes_cy;
    REQUIRE(SDL_PushEvent(&motion2_ev));

    SDL_Event click2_ev{};
    click2_ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    click2_ev.button.button = SDL_BUTTON_LEFT;
    click2_ev.button.x = yes_cx;
    click2_ev.button.y = yes_cy;
    REQUIRE(SDL_PushEvent(&click2_ev));

    SDL_Event esc_ev{};
    esc_ev.type = SDL_EVENT_KEY_DOWN;
    esc_ev.key.key = SDLK_ESCAPE;
    esc_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&esc_ev));

    bool build_called = false;
    const std::function<gus::domain::save::SaveData()> build_data = [&]() {
        build_called = true;
        gus::domain::save::SaveData d;
        d.current_scene_path = "city_intro";
        d.party_roster = {"gus"};
        d.party_active = {"gus"};
        return d;
    };

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);  // sem hardware no CI
    const SaveLoadLoopExit exit = run_save_load_menu_loop_gl_current(
        env.window, audio, translator, SaveLoadMode::Save, saves_dir.string(),
        build_data, /*apply_loaded_save_data=*/{});

    // ANTES desta onda (save_load_menu_loop.cpp sem NENHUM handling de
    // SDL_EVENT_MOUSE_BUTTON_DOWN): o clique era ignorado por completo -
    // build_called ficaria false e has_save(1,...) ficaria false, so o Esc
    // fecharia a tela (exit==BackToPause) - este teste teria FALHADO nos 2
    // REQUIRE seguintes.
    REQUIRE(exit == SaveLoadLoopExit::BackToPause);
    REQUIRE(build_called);
    REQUIRE(gus::platform::fs::has_save(1, saves_dir.string()));

    std::filesystem::remove_all(saves_dir);
}

// ---------------------------------------------------------------- (d.1) B1 revisao 2
// (last-input-wins, decisao do lider 2026-08-01): mouse MOVE a selecao E toca o SFX
// de hover ao entrar num item DIFERENTE, e NAO redispara num 2o motion sobre o MESMO
// item. Substitui o teste antigo do callback NATIVO (glintfx::UiLayer::
// set_hover_callback, REMOVIDO nesta revisao - o mecanismo agora e route_mouse_hover,
// disparado SO por SDL_EVENT_MOUSE_MOTION fisico real).

TEST_CASE("save_load_menu (harness headless, B1 revisao 2): mouse move a selecao (e "
          "toca hover_sfx) ao entrar num slot DIFERENTE, e NAO redispara num 2o motion "
          "sobre o MESMO slot (last-input-wins)",
          "[save_load_menu_interaction][gl][b1-last-input-wins]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    const std::filesystem::path saves_dir =
        std::filesystem::temp_directory_path() / "gusworld_save_load_interaction_hover_saves";
    std::filesystem::remove_all(saves_dir);  // hermetico (nunca o $HOME real do host)

    // Modo Load LE OS PREVIEWS DE VERDADE DO DISCO - grava autosave (0) + slot 1
    // (MESMA receita do teste b4-hover acima). save_load_menu_open abre focado no
    // slot 1 ("sel" do mock) - mover o mouse pro AUTOSAVE (slot 0, DIFERENTE do
    // focado) e uma mudanca real de selecao.
    REQUIRE(gus::platform::fs::save_game(make_save_data(550), kAutosaveSlot, saves_dir.string()));
    REQUIRE(gus::platform::fs::save_game(make_save_data(340), 1, saves_dir.string()));

    // Pre-mede a posicao REAL do slot 0 (autosave) - MESMO documento/viewport/
    // dp_ratio que run_save_load_menu_loop_gl_current monta INTERNAMENTE.
    SaveLoadMenuState probe_state;
    std::array<SaveSlotPreview, kSlotCount> probe_slots{};
    probe_slots[kAutosaveSlot] = build_slot_preview(make_save_data(550), kAutosaveSlot);
    probe_slots[1] = build_slot_preview(make_save_data(340), 1);
    for (int i = 2; i < kSlotCount; ++i) probe_slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    save_load_menu_open(probe_state, SaveLoadMode::Load, probe_slots);
    float slot0_cx = 0.0f, slot0_cy = 0.0f;
    {
        auto probe_ui = load_ui(probe_state, translator);
        REQUIRE(probe_ui.has_value());
        const glintfx::ElementBox slot0_box = probe_ui->get_element_box("slmenu-slot-0");
        REQUIRE(box_hittable(slot0_box, kWinW, kWinH));
        slot0_cx = slot0_box.x + slot0_box.w * 0.5f;
        slot0_cy = slot0_box.y + slot0_box.h * 0.5f;
        // FECHA a UiLayer de sondagem ANTES do loop real abrir a SUA PROPRIA
        // (RmlUi NAO suporta 2 UiLayer simultaneos no processo, ver o comentario
        // extenso no teste (d) acima).
    }

    // 2 MOUSE_MOTION IDENTICOS sobre o slot 0 (MESMA posicao): o 1o e a ENTRADA
    // (muda state.selected de 1 -> 0, deve tocar hover_sfx 1x, via route_mouse_hover);
    // o 2o e um "motion parado" sobre o MESMO item - NAO deve redisparar (0 ja e
    // state.selected, ui_hover_entered_new_item(0,0)==false). Sem clique nenhum
    // aqui (so hover) - Esc fecha a tela pra a chamada RETORNAR.
    SDL_Event motion1{};
    motion1.type = SDL_EVENT_MOUSE_MOTION;
    motion1.motion.x = slot0_cx;
    motion1.motion.y = slot0_cy;
    REQUIRE(SDL_PushEvent(&motion1));

    SDL_Event motion2 = motion1;  // MESMA posicao - "parado sobre o mesmo item"
    REQUIRE(SDL_PushEvent(&motion2));

    SDL_Event esc_ev{};
    esc_ev.type = SDL_EVENT_KEY_DOWN;
    esc_ev.key.key = SDLK_ESCAPE;
    esc_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&esc_ev));

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);  // sem hardware no CI
    const SaveLoadLoopExit exit = run_save_load_menu_loop_gl_current(
        env.window, audio, translator, SaveLoadMode::Load, saves_dir.string(),
        /*build_current_save_data=*/{}, /*apply_loaded_save_data=*/{});

    // Prova headless e objetiva (AudioEngine::sfx_play_count(), NAO julgamento
    // visual): exatamente 1 play pros 2 motions sobre o MESMO item (dedup por
    // indice, ui_hover_entered_new_item).
    REQUIRE(exit == SaveLoadLoopExit::BackToPause);
    REQUIRE(audio.sfx_play_count() == 1);

    std::filesystem::remove_all(saves_dir);
}

// ---------------------------------------------------------------- (d.1b) B1 revisao 2:
// checagem EXPLICITA de "som duplo" (o team-lead pediu pra verificar - a combinacao de
// B1 escrevendo em state.selected + B4 comparando o indice de foco antes/depois PODERIA,
// em tese, somar 2 caminhos independentes que antes eram separados). N transicoes de
// mouse por N slots DIFERENTES tem que tocar EXATAMENTE N plays - se houvesse um 2o
// caminho de som (ex.: o callback nativo antigo REINTRODUZIDO por engano, ou um duplo
// disparo dentro de route_mouse_hover/save_load_screen_step), o numero seria 2N ou mais.
//
// MISTERIO DO 3o SLOT (investigado e FECHADO, 2026-08-01 - o team-lead pediu pra
// nao aceitar a hipotese confortavel sem medir): a 1a versao deste teste usava 3
// slots e mediu 2 plays em vez de 3. Instrumentado com prints de coordenada +
// indice hit-testado a CADA motion (removidos depois de fechar o caso) - NAO e
// bug de producao (nem perda de evento, nem som duplo): e desvio de GEOMETRIA.
// O motion 1 (pre-medido, ANTES de qualquer reload) bateu no slot 0 com a caixa
// EXATA (y=142==142). O motion 2 (tambem pre-medido) bateu no slot 1, mas a
// caixa JA tinha se deslocado -6px (pre-medido y=204, real y=198 - so bateu por
// margem, o alvo ainda caiu dentro da faixa de 56px de altura). O motion 3 (pre-
// medido pro slot 2) NAO bateu em NADA - o desvio acumulado empurrou o alvo pra
// fora da caixa real. A causa mais provavel: reload_() (save_load_menu_loop.cpp)
// chama scroll_element_into_view (B7) a CADA mudanca de selecao, e isso pode
// deslocar a lista por poucos pixels mesmo quando o item "ja esta visivel" -
// cada reload subsequente desvia mais a geometria da PRE-medicao estatica feita
// 1x no inicio do teste (a sonda separada load_ui() so reflete o estado ANTES do
// 1o reload rodar). Por isso ESTE teste fica com 2 slots (motion 1 bate exato,
// motion 2 ainda bate por estar no MESMO reload de origem) - o teste de "mouse
// PARADO durante navegacao de teclado" (b1-last-input-wins, acima) ja prova o
// caso de 3 slots com SEGURANCA porque so precisa de 1 medicao (o mouse fica
// PARADO o tempo todo, nunca precisa de uma 2a posicao pre-medida apos um
// reload ja ter rodado) - a limitacao e do HARNESS de pre-medicao estatica
// deste teste especifico, nao da tela de producao.

TEST_CASE("save_load_menu (harness headless, B1 revisao 2): mover o mouse por N slots "
          "DIFERENTES em sequencia toca EXATAMENTE N plays de hover_sfx (checagem "
          "explicita de 'som duplo' apos a fusao B1+B4)",
          "[save_load_menu_interaction][gl][b1-last-input-wins]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    const std::filesystem::path saves_dir = std::filesystem::temp_directory_path() /
                                             "gusworld_save_load_interaction_no_double_sfx_saves";
    std::filesystem::remove_all(saves_dir);

    // 2 slots ocupados (autosave=0, 1) - o mouse vai visitar os 2, em
    // sequencia, cada um DIFERENTE do anterior (0 -> 1): 2 mudancas reais de
    // selecao a partir do foco inicial (1) - ver a conta abaixo.
    REQUIRE(gus::platform::fs::save_game(make_save_data(550), kAutosaveSlot, saves_dir.string()));
    REQUIRE(gus::platform::fs::save_game(make_save_data(340), 1, saves_dir.string()));

    SaveLoadMenuState probe_state;
    std::array<SaveSlotPreview, kSlotCount> probe_slots{};
    probe_slots[kAutosaveSlot] = build_slot_preview(make_save_data(550), kAutosaveSlot);
    probe_slots[1] = build_slot_preview(make_save_data(340), 1);
    for (int i = 2; i < kSlotCount; ++i) probe_slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    save_load_menu_open(probe_state, SaveLoadMode::Load, probe_slots);
    REQUIRE(probe_state.selected == 1);  // foco inicial (mock: slot 1 "sel")

    float slot_cx[2] = {0.0f, 0.0f};
    float slot_cy[2] = {0.0f, 0.0f};
    {
        auto probe_ui = load_ui(probe_state, translator);
        REQUIRE(probe_ui.has_value());
        for (int i = 0; i < 2; ++i) {
            const glintfx::ElementBox box =
                probe_ui->get_element_box(("slmenu-slot-" + std::to_string(i)).c_str());
            REQUIRE(box_hittable(box, kWinW, kWinH));
            slot_cx[i] = box.x + box.w * 0.5f;
            slot_cy[i] = box.y + box.h * 0.5f;
        }
    }

    // Sequencia: foco inicial=1 -> mouse entra em 0 (MUDA, play 1) -> mouse
    // entra em 1 (MUDA de volta, play 2) - 2 mudancas reais = 2 plays
    // esperados, NUNCA 4 (o que indicaria som duplo por transicao).
    for (const int slot : {0, 1}) {
        SDL_Event motion_ev{};
        motion_ev.type = SDL_EVENT_MOUSE_MOTION;
        motion_ev.motion.x = slot_cx[slot];
        motion_ev.motion.y = slot_cy[slot];
        REQUIRE(SDL_PushEvent(&motion_ev));
    }

    SDL_Event esc_ev{};
    esc_ev.type = SDL_EVENT_KEY_DOWN;
    esc_ev.key.key = SDLK_ESCAPE;
    esc_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&esc_ev));

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);
    const SaveLoadLoopExit exit = run_save_load_menu_loop_gl_current(
        env.window, audio, translator, SaveLoadMode::Load, saves_dir.string(),
        /*build_current_save_data=*/{}, /*apply_loaded_save_data=*/{});

    REQUIRE(exit == SaveLoadLoopExit::BackToPause);
    REQUIRE(audio.sfx_play_count() == 2);

    std::filesystem::remove_all(saves_dir);
}

// ---------------------------------------------------------------- (d.2) B1 revisao 2:
// o CASO CRITICO que motivou a revisao - mouse PARADO sobre um slot enquanto o
// TECLADO navega. A selecao tem que SEGUIR O TECLADO, nao voltar pro slot sob o
// ponteiro parado (last-input-wins: so um MOUSE_MOTION FISICO real reivindica a
// selecao pro mouse - reload()/update() programaticos NUNCA disparam
// route_mouse_hover).

TEST_CASE("save_load_menu (harness headless, B1 revisao 2): mouse PARADO sobre um "
          "slot enquanto o TECLADO navega - a selecao segue o teclado, nao volta pro "
          "slot sob o ponteiro (last-input-wins)",
          "[save_load_menu_interaction][gl][b1-last-input-wins]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    const std::filesystem::path saves_dir = std::filesystem::temp_directory_path() /
                                             "gusworld_save_load_interaction_mouse_parado_saves";
    std::filesystem::remove_all(saves_dir);

    // 3 slots ocupados (autosave=0, 1, 2) - abre focado no 1. O mouse vai
    // parar sobre o slot 2 (reivindica a selecao 1x, MESMO racional do teste
    // acima) e DEPOIS o teclado navega (Baixo: 1->2 nao muda nada pq ja e o
    // slot 2... entao usamos Cima: 1->0) - a selecao final tem que ser a do
    // TECLADO (0), nao voltar pro slot 2 so porque o mouse ainda esta la.
    REQUIRE(gus::platform::fs::save_game(make_save_data(550), kAutosaveSlot, saves_dir.string()));
    REQUIRE(gus::platform::fs::save_game(make_save_data(340), 1, saves_dir.string()));
    REQUIRE(gus::platform::fs::save_game(make_save_data(810), 2, saves_dir.string()));

    SaveLoadMenuState probe_state;
    std::array<SaveSlotPreview, kSlotCount> probe_slots{};
    probe_slots[kAutosaveSlot] = build_slot_preview(make_save_data(550), kAutosaveSlot);
    probe_slots[1] = build_slot_preview(make_save_data(340), 1);
    probe_slots[2] = build_slot_preview(make_save_data(810), 2);
    for (int i = 3; i < kSlotCount; ++i) probe_slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    save_load_menu_open(probe_state, SaveLoadMode::Load, probe_slots);
    float slot2_cx = 0.0f, slot2_cy = 0.0f;
    {
        auto probe_ui = load_ui(probe_state, translator);
        REQUIRE(probe_ui.has_value());
        const glintfx::ElementBox slot2_box = probe_ui->get_element_box("slmenu-slot-2");
        REQUIRE(box_hittable(slot2_box, kWinW, kWinH));
        slot2_cx = slot2_box.x + slot2_box.w * 0.5f;
        slot2_cy = slot2_box.y + slot2_box.h * 0.5f;
    }

    // 1) mouse entra no slot 2 (1->2, reivindica a selecao pro mouse).
    SDL_Event motion_ev{};
    motion_ev.type = SDL_EVENT_MOUSE_MOTION;
    motion_ev.motion.x = slot2_cx;
    motion_ev.motion.y = slot2_cy;
    REQUIRE(SDL_PushEvent(&motion_ev));

    // 2) teclado aperta Cima 2x (2->1->0) - o mouse NAO se move de novo, entao
    // route_mouse_hover NUNCA roda de novo (nenhum SDL_EVENT_MOUSE_MOTION na
    // fila) - a selecao final tem que ser 0 (teclado), mesmo com o cursor
    // fisicamente ainda sobre o slot 2.
    SDL_Event up1{};
    up1.type = SDL_EVENT_KEY_DOWN;
    up1.key.key = SDLK_UP;
    up1.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&up1));
    SDL_Event up2 = up1;
    REQUIRE(SDL_PushEvent(&up2));

    // 3) Enter confirma o slot FOCADO - se a selecao tivesse voltado pro
    // slot 2 (sob o mouse parado), isto carregaria o slot ERRADO.
    SDL_Event enter_ev{};
    enter_ev.type = SDL_EVENT_KEY_DOWN;
    enter_ev.key.key = SDLK_RETURN;
    enter_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&enter_ev));

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);
    gus::domain::save::SaveData loaded;
    bool apply_called = false;
    const std::function<void(const gus::domain::save::SaveData&)> apply_data =
        [&](const gus::domain::save::SaveData& d) {
            apply_called = true;
            loaded = d;
        };
    const SaveLoadLoopExit exit = run_save_load_menu_loop_gl_current(
        env.window, audio, translator, SaveLoadMode::Load, saves_dir.string(),
        /*build_current_save_data=*/{}, apply_data);

    REQUIRE(exit == SaveLoadLoopExit::ClosedAfterLoad);
    REQUIRE(apply_called);
    // O slot 0 (autosave) tem xp=550 - se a selecao tivesse ficado presa no
    // slot 2 (xp=810, sob o mouse parado), este REQUIRE falharia.
    REQUIRE(gus::app::screens::save_xp_for_display(loaded) == 550);

    std::filesystem::remove_all(saves_dir);
}

// ---------------------------------------------------------------- (d.3) B1 revisao 2:
// passar o mouse sobre um slot NAO SELECIONAVEL (autosave readonly em modo Save)
// nunca move a selecao pra ele.

TEST_CASE("save_load_menu (harness headless, B1 revisao 2): passar o mouse sobre o "
          "autosave READONLY em modo Save NAO move a selecao pra ele",
          "[save_load_menu_interaction][gl][b1-last-input-wins]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    const std::filesystem::path saves_dir = std::filesystem::temp_directory_path() /
                                             "gusworld_save_load_interaction_readonly_saves";
    std::filesystem::remove_all(saves_dir);

    // Modo Save: autosave (0) OCUPADO mas SO-LEITURA (nao selecionavel); slot 1
    // vazio (selecionavel, foco inicial). Grava o autosave de fato (modo Save
    // TAMBEM le previews reais do disco, ver build_previews_and_cache).
    REQUIRE(gus::platform::fs::save_game(make_save_data(550), kAutosaveSlot, saves_dir.string()));

    SaveLoadMenuState probe_state;
    std::array<SaveSlotPreview, kSlotCount> probe_slots{};
    probe_slots[kAutosaveSlot] = build_slot_preview(make_save_data(550), kAutosaveSlot);
    for (int i = 1; i < kSlotCount; ++i) probe_slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    save_load_menu_open(probe_state, SaveLoadMode::Save, probe_slots);
    REQUIRE(probe_state.selected == 1);  // pula o autosave (nao selecionavel em Save)
    float slot0_cx = 0.0f, slot0_cy = 0.0f;
    {
        auto probe_ui = load_ui(probe_state, translator);
        REQUIRE(probe_ui.has_value());
        const glintfx::ElementBox slot0_box = probe_ui->get_element_box("slmenu-slot-0");
        REQUIRE(box_hittable(slot0_box, kWinW, kWinH));
        slot0_cx = slot0_box.x + slot0_box.w * 0.5f;
        slot0_cy = slot0_box.y + slot0_box.h * 0.5f;
    }

    // Mouse entra no autosave (readonly, NAO selecionavel) - route_mouse_hover
    // deve ser NO-OP (state.selected continua 1). Enter confirma o slot 1
    // (vazio) - se a selecao tivesse ido pro autosave, o fluxo seria outro
    // (autosave nao abre confirming_overwrite, e o teste falharia adiante).
    SDL_Event motion_ev{};
    motion_ev.type = SDL_EVENT_MOUSE_MOTION;
    motion_ev.motion.x = slot0_cx;
    motion_ev.motion.y = slot0_cy;
    REQUIRE(SDL_PushEvent(&motion_ev));

    SDL_Event enter_ev{};
    enter_ev.type = SDL_EVENT_KEY_DOWN;
    enter_ev.key.key = SDLK_RETURN;
    enter_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&enter_ev));

    // Enter no slot 1 (vazio, modo Save) abre confirming_overwrite (AJUSTE
    // polish playtest 2026-07-10, copy "Deseja salvar...") - 1o Esc cancela o
    // DIALOGO (volta a lista, tela AINDA ABERTA); 2o Esc fecha a tela de fato
    // (BackToPause) - SEM o 2o Esc a chamada NUNCA RETORNA (fila vazia,
    // achado ao vivo ja documentado no teste (f) deste arquivo).
    SDL_Event esc1_ev{};
    esc1_ev.type = SDL_EVENT_KEY_DOWN;
    esc1_ev.key.key = SDLK_ESCAPE;
    esc1_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&esc1_ev));

    SDL_Event esc2_ev = esc1_ev;
    REQUIRE(SDL_PushEvent(&esc2_ev));

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);
    const SaveLoadLoopExit exit = run_save_load_menu_loop_gl_current(
        env.window, audio, translator, SaveLoadMode::Save, saves_dir.string(),
        /*build_current_save_data=*/{}, /*apply_loaded_save_data=*/{});

    // Se a selecao tivesse ido pro autosave (readonly, NAO selecionavel), o
    // Enter teria sido NO-OP (autosave nunca abre dialogo em modo Save) - a
    // sequencia so fecha limpo (BackToPause) porque o Enter realmente abriu
    // o dialogo do slot 1 e o 1o Esc o cancelou, MESMO fluxo esperado com a
    // selecao intocada pelo mouse.
    REQUIRE(exit == SaveLoadLoopExit::BackToPause);

    std::filesystem::remove_all(saves_dir);
}

// ---------------------------------------------------------------- (B4) paridade
// teclado x mouse: navegar com o TECLADO (seta) tambem toca o SFX de hover -
// decisao do lider 2026-08-01, retoque ao vivo pos-bugs 1-9. ANTES desta onda,
// nenhuma tecla tocava som nesta tela (omissao DOCUMENTADA, ver save_load_menu_
// loop.hpp) - este teste E o fio REAL (SDL_PushEvent ate
// run_save_load_menu_loop_gl_current) que teria falhado no codigo de ontem.

TEST_CASE("save_load_menu (harness headless): navegar com SETA (teclado) entre "
          "slots selecionaveis toca o SFX de hover de fato (B4 - paridade "
          "teclado x mouse)",
          "[save_load_menu_interaction][gl][b4-hover]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    const std::filesystem::path saves_dir =
        std::filesystem::temp_directory_path() / "gusworld_save_load_interaction_kbhover_saves";
    std::filesystem::remove_all(saves_dir);  // hermetico (nunca o $HOME real do host)

    // Modo Load LE OS PREVIEWS DE VERDADE DO DISCO (build_previews_and_cache,
    // save_load_menu_loop.cpp) - PRECISA gravar saves REAIS via
    // gus::platform::fs::save_game ANTES do loop abrir, senao TODOS os slots
    // ficam vazios/nao-selecionaveis e save_load_menu_open degrada pra
    // selected=kAutosaveSlot sem NENHUM slot pra navegar (MESMO padrao ja usado
    // pelos testes (e)/(f) deste arquivo pra popular o disco de fato). Autosave
    // (0) + slot 1 OCUPADOS - ambos selecionaveis em modo Load
    // (save_load_menu_open abre focado no slot 1, "sel" do mock) - 1 seta CIMA
    // move o foco 1 -> 0 (indice NOVO, deve soar).
    REQUIRE(gus::platform::fs::save_game(make_save_data(550), kAutosaveSlot, saves_dir.string()));
    REQUIRE(gus::platform::fs::save_game(make_save_data(340), 1, saves_dir.string()));

    SDL_Event up_ev{};
    up_ev.type = SDL_EVENT_KEY_DOWN;
    up_ev.key.key = SDLK_UP;
    up_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&up_ev));

    SDL_Event esc_ev{};
    esc_ev.type = SDL_EVENT_KEY_DOWN;
    esc_ev.key.key = SDLK_ESCAPE;
    esc_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&esc_ev));

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);  // sem hardware no CI
    const SaveLoadLoopExit exit = run_save_load_menu_loop_gl_current(
        env.window, audio, translator, SaveLoadMode::Load, saves_dir.string(),
        /*build_current_save_data=*/{}, /*apply_loaded_save_data=*/{});

    // ANTES desta onda: 0 plays (teclado nunca tocava SFX nesta tela) - este
    // REQUIRE prova, headless e objetivo (AudioEngine::sfx_play_count(), NAO
    // julgamento visual), que a seta de fato soou 1x (a mudanca de foco
    // 1->0), e que Esc (Back, sem I/O) NAO soa alem disso.
    REQUIRE(exit == SaveLoadLoopExit::BackToPause);
    REQUIRE(audio.sfx_play_count() == 1);

    std::filesystem::remove_all(saves_dir);
}

// ---------------------------------------------------------------- (e) SAVE-LOAD-AVISOS:
// clique real no slot Danificado -> aviso abre -> clique real em "Tentar recuperar" ->
// load_game_from_backup de fato recupera (fluxo COMPLETO ao vivo, nao so a logica pura).

TEST_CASE("save_load_menu (harness headless): clique de mouse REAL num slot "
          "Danificado abre o aviso, e clicar 'Tentar recuperar' recupera via "
          "backup de fato (SAVE-LOAD-AVISOS)",
          "[save_load_menu_interaction][gl][save-load-avisos]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    const std::filesystem::path saves_dir = std::filesystem::temp_directory_path() /
                                             "gusworld_save_load_interaction_recover_saves";
    std::filesystem::remove_all(saves_dir);  // hermetico

    // Grava 2x no slot 1 (a 1a gravacao vira backup1) e corrompe o PRIMARIO - so
    // o backup fica Ok (MESMO repro de CRIT-1/save_file_store_test.cpp).
    gus::domain::save::SaveData first;
    first.current_scene_path = "city_intro";
    first.party_roster = {"gus"};
    first.party_active = {"gus"};
    first.slot_id = 1;
    first.playtime_seconds = 1.0;
    REQUIRE(gus::platform::fs::save_game(first, 1, saves_dir.string()));
    gus::domain::save::SaveData second = first;
    second.playtime_seconds = 2.0;
    REQUIRE(gus::platform::fs::save_game(second, 1, saves_dir.string()));  // first -> backup1
    {
        std::fstream f(saves_dir / "save_1.sav", std::ios::in | std::ios::out | std::ios::binary);
        REQUIRE(f.is_open());
        char byte = 0;
        f.seekg(10);
        f.read(&byte, 1);
        byte = static_cast<char>(byte ^ 0xFF);
        f.seekp(10);
        f.write(&byte, 1);
    }
    const auto primary_outcome = gus::platform::fs::load_game(1, saves_dir.string());
    REQUIRE(primary_outcome.has_value());
    REQUIRE(primary_outcome->result != gus::domain::save::LoadResult::Ok);

    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = unreadable_slot_preview(1, primary_outcome->result);

    // Probe #1: posicao REAL da linha do slot 1 em modo Load (MESMO documento/
    // viewport/dp_ratio que o loop de producao monta internamente).
    SaveLoadMenuState probe1;
    save_load_menu_open(probe1, SaveLoadMode::Load, slots);
    float slot1_cx = 0.0f, slot1_cy = 0.0f;
    {
        auto probe_ui = load_ui(probe1, translator);
        REQUIRE(probe_ui.has_value());
        const glintfx::ElementBox box = probe_ui->get_element_box("slmenu-slot-1");
        REQUIRE(box_hittable(box, kWinW, kWinH));
        slot1_cx = box.x + box.w * 0.5f;
        slot1_cy = box.y + box.h * 0.5f;
        // FECHA a sondagem ANTES da proxima (RmlUi nao suporta 2 UiLayer
        // simultaneos no processo, ver o comentario extenso no teste (d) acima).
    }

    // Probe #2: posicao REAL do botao "Tentar recuperar" no aviso Damaged JA
    // ABERTO (o MESMO estado que o clique real no slot 1 produz - confirmado via
    // save_load_menu_click_slot, a logica PURA ja provada em save_load_menu_test.cpp).
    SaveLoadMenuState probe2;
    save_load_menu_open(probe2, SaveLoadMode::Load, slots);
    (void)save_load_menu_click_slot(probe2, 1);
    REQUIRE(probe2.warning_kind == SaveLoadMenuState::WarningKind::Damaged);
    float recover_cx = 0.0f, recover_cy = 0.0f;
    {
        auto probe_ui = load_ui(probe2, translator);
        REQUIRE(probe_ui.has_value());
        const glintfx::ElementBox box = probe_ui->get_element_box("slmenu-warn-recover");
        REQUIRE(box_hittable(box, kWinW, kWinH));
        recover_cx = box.x + box.w * 0.5f;
        recover_cy = box.y + box.h * 0.5f;
    }

    // Injeta a sequencia REAL na fila do SDL (MESMA fila que o loop de producao
    // consome): motion+click no slot 1 (abre o aviso), motion+click em "Tentar
    // recuperar" (confirma) - tudo ANTES de chamar o loop, MESMA receita do
    // teste (d) acima. Sem Esc no fim: RecoverRequested bem-sucedido fecha a
    // tela sozinho (ClosedAfterLoad), o loop retorna por conta propria.
    SDL_Event motion1_ev{};
    motion1_ev.type = SDL_EVENT_MOUSE_MOTION;
    motion1_ev.motion.x = slot1_cx;
    motion1_ev.motion.y = slot1_cy;
    REQUIRE(SDL_PushEvent(&motion1_ev));

    SDL_Event click1_ev{};
    click1_ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    click1_ev.button.button = SDL_BUTTON_LEFT;
    click1_ev.button.x = slot1_cx;
    click1_ev.button.y = slot1_cy;
    REQUIRE(SDL_PushEvent(&click1_ev));

    SDL_Event motion2_ev{};
    motion2_ev.type = SDL_EVENT_MOUSE_MOTION;
    motion2_ev.motion.x = recover_cx;
    motion2_ev.motion.y = recover_cy;
    REQUIRE(SDL_PushEvent(&motion2_ev));

    SDL_Event click2_ev{};
    click2_ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    click2_ev.button.button = SDL_BUTTON_LEFT;
    click2_ev.button.x = recover_cx;
    click2_ev.button.y = recover_cy;
    REQUIRE(SDL_PushEvent(&click2_ev));

    std::optional<gus::domain::save::SaveData> applied;
    const std::function<void(const gus::domain::save::SaveData&)> apply_data =
        [&](const gus::domain::save::SaveData& data) { applied = data; };

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);  // sem hardware no CI
    const SaveLoadLoopExit exit = run_save_load_menu_loop_gl_current(
        env.window, audio, translator, SaveLoadMode::Load, saves_dir.string(),
        /*build_current_save_data=*/{}, apply_data);

    // ANTES desta onda (sem RecoverRequested/do_recover): o clique no botao
    // "Tentar recuperar" nao existia (nem o botao, nem o handler) - este teste
    // teria FALHADO nos 3 REQUIRE seguintes.
    REQUIRE(exit == SaveLoadLoopExit::ClosedAfterLoad);
    REQUIRE(applied.has_value());
    // playtime_seconds==1.0 = "first" (a geracao do BACKUP1), NAO 2.0 ("second",
    // a geracao corrompida do primario) - prova que a recuperacao usou o backup
    // de fato, nao fingiu um load.
    REQUIRE(applied->playtime_seconds == 1.0);

    std::filesystem::remove_all(saves_dir);
}

// ---------------------------------------------------------------- (f) item 2 (retoque
// ao vivo 2026-07-10/11): slot GENUINAMENTE vazio agora EXIGE confirmacao antes de
// gravar - clique real no slot vazio, sem confirmar, NAO grava; confirmando, grava.

TEST_CASE("save_load_menu (harness headless): clique de mouse REAL num slot "
          "GENUINAMENTE VAZIO abre o mini-dialogo de confirmacao (AJUSTE polish "
          "playtest 2026-07-10) e NAO grava enquanto nao confirmado (Esc fecha o "
          "dialogo sem I/O nenhum)",
          "[save_load_menu_interaction][gl][item2-empty-confirm]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    const std::filesystem::path saves_dir = std::filesystem::temp_directory_path() /
                                             "gusworld_save_load_interaction_empty_confirm_saves";
    std::filesystem::remove_all(saves_dir);  // hermetico

    std::array<SaveSlotPreview, kSlotCount> empty_slots{};
    for (int i = 0; i < kSlotCount; ++i) empty_slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);

    // Probe #1: posicao REAL do slot 1 (vazio, selecionavel em modo Save).
    SaveLoadMenuState probe1;
    save_load_menu_open(probe1, SaveLoadMode::Save, empty_slots);
    float slot1_cx = 0.0f, slot1_cy = 0.0f;
    {
        auto probe_ui = load_ui(probe1, translator);
        REQUIRE(probe_ui.has_value());
        const glintfx::ElementBox box = probe_ui->get_element_box("slmenu-slot-1");
        REQUIRE(box_hittable(box, kWinW, kWinH));
        slot1_cx = box.x + box.w * 0.5f;
        slot1_cy = box.y + box.h * 0.5f;
        // FECHA a sondagem ANTES da proxima (RmlUi 1-instancia, ver o comentario
        // extenso no teste (d) acima).
    }

    SDL_Event motion_ev{};
    motion_ev.type = SDL_EVENT_MOUSE_MOTION;
    motion_ev.motion.x = slot1_cx;
    motion_ev.motion.y = slot1_cy;
    REQUIRE(SDL_PushEvent(&motion_ev));

    SDL_Event click_ev{};
    click_ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    click_ev.button.button = SDL_BUTTON_LEFT;
    click_ev.button.x = slot1_cx;
    click_ev.button.y = slot1_cy;
    REQUIRE(SDL_PushEvent(&click_ev));

    // 2 Esc: o 1o fecha SO o mini-dialogo (OverwriteCancelled, MESMA seguranca de
    // confirming_delete/controls_confirming_discard - ESC == "Nao"), o loop
    // CONTINUA na lista (reload, nao retorna); o 2o sai de fato da tela
    // (SaveLoadMenuAction::Back). ANTES desta onda (slot vazio salvava DIRETO no
    // Enter/clique, sem abrir dialogo), o 1o Esc aqui ja teria fechado a tela
    // inteira (o clique ja teria gravado) - o REQUIRE de has_save abaixo teria
    // FALHADO (o save ja teria acontecido).
    SDL_Event esc1_ev{};
    esc1_ev.type = SDL_EVENT_KEY_DOWN;
    esc1_ev.key.key = SDLK_ESCAPE;
    esc1_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&esc1_ev));

    SDL_Event esc2_ev = esc1_ev;
    REQUIRE(SDL_PushEvent(&esc2_ev));

    bool build_called = false;
    const std::function<gus::domain::save::SaveData()> build_data = [&]() {
        build_called = true;
        gus::domain::save::SaveData d;
        d.current_scene_path = "city_intro";
        d.party_roster = {"gus"};
        d.party_active = {"gus"};
        return d;
    };

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);  // sem hardware no CI
    const SaveLoadLoopExit exit = run_save_load_menu_loop_gl_current(
        env.window, audio, translator, SaveLoadMode::Save, saves_dir.string(), build_data,
        /*apply_loaded_save_data=*/{});

    REQUIRE(exit == SaveLoadLoopExit::BackToPause);
    // O GATE (item 2): clicar num slot vazio sem CONFIRMAR nao chama build_data
    // nem grava nada em disco.
    REQUIRE_FALSE(build_called);
    REQUIRE_FALSE(gus::platform::fs::has_save(1, saves_dir.string()));

    std::filesystem::remove_all(saves_dir);
}

TEST_CASE("save_load_menu (harness headless): confirmar ('Salvar') o mini-dialogo "
          "de um slot GENUINAMENTE VAZIO grava de fato em disco (item 2, retoque "
          "ao vivo 2026-07-10/11)",
          "[save_load_menu_interaction][gl][item2-empty-confirm]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    const std::filesystem::path saves_dir =
        std::filesystem::temp_directory_path() /
        "gusworld_save_load_interaction_empty_confirm_yes_saves";
    std::filesystem::remove_all(saves_dir);  // hermetico

    std::array<SaveSlotPreview, kSlotCount> empty_slots{};
    for (int i = 0; i < kSlotCount; ++i) empty_slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);

    // Probe #1: posicao REAL do slot 1 (vazio).
    SaveLoadMenuState probe1;
    save_load_menu_open(probe1, SaveLoadMode::Save, empty_slots);
    float slot1_cx = 0.0f, slot1_cy = 0.0f;
    {
        auto probe_ui = load_ui(probe1, translator);
        REQUIRE(probe_ui.has_value());
        const glintfx::ElementBox box = probe_ui->get_element_box("slmenu-slot-1");
        REQUIRE(box_hittable(box, kWinW, kWinH));
        slot1_cx = box.x + box.w * 0.5f;
        slot1_cy = box.y + box.h * 0.5f;
    }

    // Probe #2: posicao REAL da pill "Salvar" (slmenu-confirm-yes) no mini-dialogo
    // JA ABERTO pra um slot vazio (MESMO estado que o clique real no slot 1
    // produz - confirmado via save_load_menu_click_slot, a logica PURA ja
    // provada em save_load_menu_test.cpp/save_load_menu_rml_test.cpp).
    SaveLoadMenuState probe2;
    save_load_menu_open(probe2, SaveLoadMode::Save, empty_slots);
    (void)save_load_menu_click_slot(probe2, 1);
    REQUIRE(probe2.confirming_overwrite);
    float yes_cx = 0.0f, yes_cy = 0.0f;
    {
        auto probe_ui = load_ui(probe2, translator);
        REQUIRE(probe_ui.has_value());
        const glintfx::ElementBox box = probe_ui->get_element_box("slmenu-confirm-yes");
        REQUIRE(box_hittable(box, kWinW, kWinH));
        yes_cx = box.x + box.w * 0.5f;
        yes_cy = box.y + box.h * 0.5f;
    }

    SDL_Event motion1_ev{};
    motion1_ev.type = SDL_EVENT_MOUSE_MOTION;
    motion1_ev.motion.x = slot1_cx;
    motion1_ev.motion.y = slot1_cy;
    REQUIRE(SDL_PushEvent(&motion1_ev));

    SDL_Event click1_ev{};
    click1_ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    click1_ev.button.button = SDL_BUTTON_LEFT;
    click1_ev.button.x = slot1_cx;
    click1_ev.button.y = slot1_cy;
    REQUIRE(SDL_PushEvent(&click1_ev));

    SDL_Event motion2_ev{};
    motion2_ev.type = SDL_EVENT_MOUSE_MOTION;
    motion2_ev.motion.x = yes_cx;
    motion2_ev.motion.y = yes_cy;
    REQUIRE(SDL_PushEvent(&motion2_ev));

    SDL_Event click2_ev{};
    click2_ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    click2_ev.button.button = SDL_BUTTON_LEFT;
    click2_ev.button.x = yes_cx;
    click2_ev.button.y = yes_cy;
    REQUIRE(SDL_PushEvent(&click2_ev));

    // OverwriteConfirmed grava e faz reload() SEM fechar a tela (do_save + reload,
    // ver handle_action em save_load_menu_loop.cpp) - precisa de 1 Esc pra sair de
    // fato (lista, ja sem o dialogo).
    SDL_Event esc_ev{};
    esc_ev.type = SDL_EVENT_KEY_DOWN;
    esc_ev.key.key = SDLK_ESCAPE;
    esc_ev.key.repeat = 0;
    REQUIRE(SDL_PushEvent(&esc_ev));

    bool build_called = false;
    const std::function<gus::domain::save::SaveData()> build_data = [&]() {
        build_called = true;
        gus::domain::save::SaveData d;
        d.current_scene_path = "city_intro";
        d.party_roster = {"gus"};
        d.party_active = {"gus"};
        return d;
    };

    gus::platform::audio::AudioEngine audio(/*device_active=*/false);  // sem hardware no CI
    const SaveLoadLoopExit exit = run_save_load_menu_loop_gl_current(
        env.window, audio, translator, SaveLoadMode::Save, saves_dir.string(), build_data,
        /*apply_loaded_save_data=*/{});

    REQUIRE(exit == SaveLoadLoopExit::BackToPause);
    REQUIRE(build_called);
    REQUIRE(gus::platform::fs::has_save(1, saves_dir.string()));

    std::filesystem::remove_all(saves_dir);
}

// ---------------------------------------------------------------- (g) item 2: PROVA
// VISUAL headless (PNG) do dialogo "Deseja salvar no Espaco [N] (vazio)?" - prova que
// a COPY renderizada e a do VAZIO (nao a de sobrescrita), nao so a logica de gate
// acima. GUSWORLD_TMPDIR (opcional): onde salvar o PNG (default: temp_directory_path()).

TEST_CASE("save_load_menu (harness headless): PROVA VISUAL - PNG do mini-dialogo "
          "'Deseja salvar no Espaco [N] (vazio)?' aberto por um clique real num "
          "slot vazio (item 2, retoque ao vivo 2026-07-10/11)",
          "[save_load_menu_interaction][gl][item2-empty-confirm]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99, "
             "DISPLAY=:99 EXPLICITO - NUNCA :0, a tela real do lider).");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    std::array<SaveSlotPreview, kSlotCount> empty_slots{};
    for (int i = 0; i < kSlotCount; ++i) empty_slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);

    // Estado JA no mini-dialogo de confirmacao pra um slot vazio (MESMO estado
    // que save_load_menu_click_slot produz, PROVADO no teste (f) acima -
    // aqui construimos direto, sem precisar de outro round de eventos SDL, so
    // pra CAPTURAR o frame).
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Save, empty_slots);
    (void)save_load_menu_click_slot(state, 1);
    REQUIRE(state.confirming_overwrite);

    glintfx::UiLayer ui(glintfx::UiLayer::Config{/*logical_width=*/kWinW,
                                                  /*logical_height=*/kWinH,
                                                  /*load_gl=*/true,
                                                  /*dp_ratio=*/1.0f});
    REQUIRE(ui.ok());

    const std::filesystem::path stage =
        std::filesystem::temp_directory_path() / "gusworld_save_load_empty_confirm_png";
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

    std::string rml = build_save_load_menu_rml(state, translator);
    // Prova ESTRUTURAL (o mesmo REQUIRE de save_load_menu_rml_test.cpp, repetido
    // aqui pra amarrar a prova visual ao texto REAL que vai pra tela - nao so
    // "abriu algum dialogo", e "abriu O CERTO"): a copy do VAZIO, NAO a de
    // sobrescrita.
    REQUIRE(rml.find("Deseja salvar no Espaco 1 (vazio)?") != std::string::npos);
    REQUIRE(rml.find("Sobrescrever este slot?") == std::string::npos);

    const std::string needle = "<style>\n";
    const std::size_t pos = rml.find(needle);
    if (pos != std::string::npos) {
        rml.insert(pos + needle.size(),
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "src: \"PixelOperatorMono.ttf\"; }\n"
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "font-weight: bold; src: \"PixelOperatorMono-Bold.ttf\"; }\n");
    }
    const std::filesystem::path rml_path = stage / "save_load_empty_confirm.rml";
    {
        std::ofstream f(rml_path);
        f << rml;
    }
    ui.set_asset_base_url(stage.string().c_str());
    ui.load(rml_path.string().c_str());
    ui.set_viewport(kWinW, kWinH);
    ui.set_dp_ratio(1.0f);

    for (int i = 0; i < 6; ++i) {
        ui.update();
        ui.render();
        SDL_GL_SwapWindow(env.window);
    }

    std::vector<unsigned char> buf(static_cast<std::size_t>(kWinW) *
                                    static_cast<std::size_t>(kWinH) * 4);
    REQUIRE(gus::platform::rmlui::gl3_read_backbuffer_rgba(kWinW, kWinH, buf.data()));

    std::string out_dir = std::filesystem::temp_directory_path().string();
    if (const char* envd = std::getenv("GUSWORLD_TMPDIR")) {
        if (envd[0] != '\0') out_dir = envd;
    }
    std::error_code ec_dir;
    std::filesystem::create_directories(out_dir, ec_dir);
    const std::filesystem::path png_path =
        std::filesystem::path(out_dir) / "save_load_empty_confirm.png";
    const int written = stbi_write_png(png_path.string().c_str(), kWinW, kWinH, 4, buf.data(),
                                        kWinW * 4);
    REQUIRE(written != 0);
    INFO("PNG salvo em: " << png_path.string());
    std::cout << "save_load_menu_interaction_test: [PROVA VISUAL item 2] "
              << png_path.string() << " (" << kWinW << "x" << kWinH << ")\n";
    REQUIRE(std::filesystem::exists(png_path));
    REQUIRE(std::filesystem::file_size(png_path) > 0);
}

// ---------------------------------------------------------------- (h) B5: scrollbar
// NATIVA arrastavel - decisao do lider 2026-08-01. ATE esta fatia,
// SaveLoadScreen::handle_event NUNCA encaminhava o botao do mouse pro RmlUi (so
// hit-test MANUAL, route_mouse_click) - a `sliderbar` nativa (CSS "#slmenu-list
// scrollbarvertical", save_load_menu_rml.cpp) so arrasta se o proprio RmlUi
// souber que o botao esta PRESSIONADO. Este teste prova a TECNICA em si
// (MouseButton(pressed=true) -> MouseMove* -> MouseButton(pressed=false), via
// glintfx::UiLayer::process_event DIRETO) - o encaminhamento no FIO de
// producao (save_load_menu_loop.cpp) ja tem cobertura de NAO-REGRESSAO pelos
// testes de clique em slot/botao/pill deste MESMO arquivo, que continuam
// verdes com o forwarding ligado (nenhuma colisao: esta tela nunca registra
// glintfx::UiLayer::set_click_callback, grep confirmado).

TEST_CASE("save_load_menu (harness headless, B5): arrastar a barra de rolagem "
          "NATIVA com o mouse (MouseButton+MouseMove) move o scroll de fato",
          "[save_load_menu_interaction][gl][b5-scrollbar-drag]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel - harness pulado (rode com Xvfb :99).");
        return;
    }

    const gus::app::i18n::Translator translator = make_translator();

    // TODOS os 7 slots ocupados - forca overflow real de `.slot-list` (300dp
    // de altura, ver save_load_menu_rml.cpp) - MESMA populacao de dados do
    // probe efemero de B7 (app/tools/save_load_screenshot_probe.cpp).
    std::array<SaveSlotPreview, kSlotCount> slots{};
    slots[kAutosaveSlot] = build_slot_preview(make_save_data(550), kAutosaveSlot);
    for (int i = 1; i < kSlotCount; ++i) {
        slots[static_cast<std::size_t>(i)] = build_slot_preview(make_save_data(100 * i), i);
    }
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Load, slots);

    auto ui = load_ui(state, translator);
    REQUIRE(ui.has_value());

    const glintfx::ElementBox list_box = ui->get_element_box("slmenu-list");
    REQUIRE(list_box.found);

    float scroll_top_before = -1.0f;
    REQUIRE(ui->get_element_scroll_top("slmenu-list", scroll_top_before));
    REQUIRE(scroll_top_before == 0.0f);  // recem-aberta, topo (7 slots > recorte de 300dp)

    // Faixa da scrollbar: 8dp de largura, encostada na borda direita de
    // `.slot-list` - MESMA constante kScrollbarWidthDp do teste de
    // posicionamento (a)(b)(c) acima.
    constexpr float kScrollbarWidthDp = 8.0f;
    const float scrollbar_cx = list_box.x + list_box.w - kScrollbarWidthDp * 0.5f;
    const float drag_start_y = list_box.y + 10.0f;              // perto do topo da faixa
    const float drag_end_y = list_box.y + list_box.h - 10.0f;   // quase no fim da faixa

    // MESMA sequencia que SaveLoadScreen::handle_event agora encaminha de
    // verdade (MouseMove assenta o hover sobre a sliderbar ANTES do
    // MouseButtonDown - mesma exigencia "hover, nao foco" ja documentada pro
    // MouseWheel em glintfx/ui_event.hpp).
    glintfx::UiEvent mv1{};
    mv1.type = glintfx::UiEvent::Type::MouseMove;
    mv1.x = scrollbar_cx;
    mv1.y = drag_start_y;
    ui->process_event(mv1);

    glintfx::UiEvent down_ev{};
    down_ev.type = glintfx::UiEvent::Type::MouseButton;
    down_ev.button = 0;
    down_ev.pressed = true;
    ui->process_event(down_ev);

    // Varios passos intermediarios (MESMO racional de um arraste real - N
    // MOUSE_MOTION entre down e up, nao 1 salto so).
    constexpr int kDragSteps = 6;
    for (int step = 1; step <= kDragSteps; ++step) {
        const float t = static_cast<float>(step) / static_cast<float>(kDragSteps);
        glintfx::UiEvent mv{};
        mv.type = glintfx::UiEvent::Type::MouseMove;
        mv.x = scrollbar_cx;
        mv.y = drag_start_y + t * (drag_end_y - drag_start_y);
        ui->process_event(mv);
    }

    glintfx::UiEvent up_ev{};
    up_ev.type = glintfx::UiEvent::Type::MouseButton;
    up_ev.button = 0;
    up_ev.pressed = false;
    ui->process_event(up_ev);

    float scroll_top_after = -1.0f;
    REQUIRE(ui->get_element_scroll_top("slmenu-list", scroll_top_after));
    INFO("scroll_top_before=" << scroll_top_before << " scroll_top_after=" << scroll_top_after);
    REQUIRE(scroll_top_after > scroll_top_before);
}
