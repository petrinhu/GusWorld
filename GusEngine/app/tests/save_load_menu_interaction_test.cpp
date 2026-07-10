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
#include <optional>
#include <string>

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
        "## SAVE_DELETE_BUTTON_LABEL\nApagar\n\n"
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

TEST_CASE("save_load_menu (harness headless): clique de mouse REAL (SDL_PushEvent) "
          "no slot vazio dispara o save de fato em disco (bug 3/4 - 'clico e "
          "nada acontece')",
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

    // Pre-mede a posicao REAL do slot 1 (vazio, selecionavel em modo Save) - o
    // MESMO documento/viewport/dp_ratio (960x540, dp_ratio=1.0 numa janela de
    // 960px) que run_save_load_menu_loop_gl_current monta INTERNAMENTE - layout
    // deterministico, a posicao medida aqui bate com a que o loop de producao
    // usa de fato.
    SaveLoadMenuState probe_state;
    std::array<SaveSlotPreview, kSlotCount> empty_slots{};
    for (int i = 0; i < kSlotCount; ++i) {
        empty_slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    }
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

    // Injeta os eventos NA FILA REAL do SDL (SDL_PushEvent) - a MESMA fila que
    // SDL_PollEvent dentro do loop de producao consome:
    //   1) MOUSE_MOTION ate o centro do slot 1 (assenta o hover nativo/RCSS);
    //   2) MOUSE_BUTTON_DOWN no MESMO ponto - o clique que deveria salvar;
    //   3) KEY_DOWN Escape - fecha a tela de volta (senao o while(true) do loop
    //      ficaria esperando o PROXIMO evento pra sempre; o teste precisa que a
    //      chamada RETORNE).
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
