// PROBE EFEMERO (NAO COMMITADO - app/tools/ fica de fora do git, MESMO padrao de
// npcdlg_screenshot_probe.cpp/frozen_bg_probe.cpp neste mesmo diretorio) - captura
// headless (Xvfb :99, NUNCA :0) de 1 frame REAL da tela Controles (M2), com
// EXATAMENTE gus::domain::input::default_controls() (o mesmo que o boot carrega
// quando nao ha controls.json em disco), pra INSPECIONAR VISUALMENTE (pixel real,
// nao so o texto do RML) o que a coluna "Teclado" mostra. Descartar apos a prova.

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <SDL3/SDL.h>
#include <glintfx/ui_layer.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/system_menu.hpp"
#include "gus/app/screens/system_menu_rml.hpp"
#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect
#include "gus/domain/input/controls_restore.hpp"  // default_controls()
#include "gus/platform/render2d/render2d_gl3.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"

namespace fs = std::filesystem;

namespace {
std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}
}  // namespace

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }

    int kW = 960;
    int kH = 540;
    if (const char* w = std::getenv("GUSWORLD_PROBE_WIN_W")) kW = std::atoi(w);
    if (const char* h = std::getenv("GUSWORLD_PROBE_WIN_H")) kH = std::atoi(h);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window =
        SDL_CreateWindow("sysmenu_controls_screenshot_probe", kW, kH, SDL_WINDOW_OPENGL);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow falhou: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_GLContext gl = SDL_GL_CreateContext(window);
    if (gl == nullptr) {
        std::cerr << "SDL_GL_CreateContext falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_GL_MakeCurrent(window, gl);
    SDL_GL_SetSwapInterval(0);

    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        std::cerr << "gl3_load_functions falhou (glad)\n";
        return 1;
    }

    gus::app::i18n::Translator translator;
    const std::string tr_path = gus::app::i18n::resolve_translations_path();
    if (!translator.load_from_file(tr_path)) {
        std::cerr << "Translator::load_from_file falhou: " << tr_path << "\n";
        return 1;
    }

    // Estado EXATO: tela Controles com controls_config = default_controls() (o
    // MESMO que gus::platform::fs::load_controls devolve quando NAO ha
    // controls.json em disco - controls_file_store.cpp::load_controls, ramo
    // "!exists(path)"). Foco na 1a action (move_forward) - MESMA convencao de
    // entrada real (system_menu_key_down: ConfigCategoryItem::Controls seta
    // controls_selected=0).
    gus::app::screens::SystemMenuState state;
    state.screen = gus::app::screens::SystemMenuScreen::Controls;
    state.controls_config = gus::domain::input::default_controls();
    state.controls_selected = 0;

    const int nav_down = std::getenv("GUSWORLD_PROBE_NAV_DOWN")
                              ? std::atoi(std::getenv("GUSWORLD_PROBE_NAV_DOWN"))
                              : 0;
    for (int i = 0; i < nav_down; ++i) {
        state.controls_selected = (state.controls_selected + 1) %
                                    gus::app::screens::kControlsItemCount;
    }

    // GUSWORLD_PROBE_CAPTURING=1 (M2, investigacao do contorno ciano cortado): forca
    // o estado de MODO DE CAPTURA (.ctrl-row.capturing no RCSS) na linha selecionada
    // acima - MESMO estado "Pressione uma tecla..." reportado pelo lider ao vivo.
    if (const char* cap = std::getenv("GUSWORLD_PROBE_CAPTURING")) {
        if (cap[0] == '1') state.controls_capturing = true;
    }

    std::string rml = gus::app::screens::build_system_menu_rml(state, translator, -1);

    const fs::path stage = fs::temp_directory_path() / "gusworld_sysmenu_screenshot_probe";
    std::error_code ec;
    fs::create_directories(stage, ec);

    std::string fonts_dir = GUSWORLD_FONTS_DIR;
    if (const char* envf = std::getenv("GUSWORLD_FONTS")) {
        if (envf[0] != '\0') fonts_dir = envf;
    }
    if (!fonts_dir.empty()) {
        fs::copy_file(join(fonts_dir, "PixelOperatorMono.ttf"), stage / "PixelOperatorMono.ttf",
                      fs::copy_options::overwrite_existing, ec);
        fs::copy_file(join(fonts_dir, "PixelOperatorMono-Bold.ttf"),
                      stage / "PixelOperatorMono-Bold.ttf",
                      fs::copy_options::overwrite_existing, ec);
    }

    const std::string needle = "<style>\n";
    const std::size_t pos = rml.find(needle);
    if (pos != std::string::npos) {
        rml.insert(pos + needle.size(),
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "src: \"PixelOperatorMono.ttf\"; }\n"
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "font-weight: bold; src: \"PixelOperatorMono-Bold.ttf\"; }\n");
    }
    const fs::path rml_path = stage / "system_menu.rml";
    {
        std::ofstream f(rml_path);
        f << rml;
    }

    // dp_ratio MESMA formula de run_system_menu_loop_gl_current (system_menu_loop.cpp):
    // pw/960.0f - producao real roda a 1280x720 (Maestro::kWindowW/kWindowH), entao
    // dp_ratio=1.333... la, NAO 1.0 (default deste probe so serve pro canvas logico
    // 960x540 puro).
    const float dp_ratio = static_cast<float>(kW) / 960.0f;

    glintfx::UiLayer ui(glintfx::UiLayer::Config{/*logical_width=*/960,
                                                  /*logical_height=*/540,
                                                  /*load_gl=*/true,
                                                  /*dp_ratio=*/dp_ratio});
    if (!ui.ok()) {
        std::cerr << "glintfx::UiLayer::ok()=false (attach falhou)\n";
        return 1;
    }
    ui.set_asset_base_url(stage.string().c_str());
    ui.load(rml_path.string().c_str());
    ui.set_viewport(kW, kH);
    ui.set_dp_ratio(dp_ratio);

    // REPLICA reload() de system_menu_loop.cpp (M2, investigacao do contorno ciano
    // cortado): scroll_element_into_view(id) roda com align_with_top=TRUE (default
    // da API, ver ui_layer.hpp) apos QUALQUER reload na tela Controles -
    // ScrollAlignment::Start SEMPRE realinha a linha ao TOPO do recorte (nao e
    // "scroll so se precisar" - RmlUi::GetScrollOffsetDelta caso Start devolve
    // begin_offset incondicional, ver Element.cpp), mesmo se a linha ja estivesse
    // visivel. GUSWORLD_PROBE_CAPTURING/GUSWORLD_PROBE_NAV_DOWN acima so montam o
    // ESTADO (classe .capturing/.sel); sem este passo o probe pulava o efeito
    // colateral REAL de reload() e o contorno saia intacto (nao reproduzia o bug
    // reportado ao vivo).
    if (const char* cap = std::getenv("GUSWORLD_PROBE_SCROLL_INTO_VIEW")) {
        if (cap[0] == '1') {
            ui.update();
            const std::string scroll_id =
                "controls-item-" + std::to_string(state.controls_selected);
            ui.scroll_element_into_view(scroll_id.c_str());
        }
    }

    gus::platform::render2d::Render2dGl3 backdrop(/*gl_active=*/true);
    const gus::core::spatial::Rect cam{0.0f, 0.0f, static_cast<float>(kW),
                                        static_cast<float>(kH)};

    constexpr int kSettleFrames = 20;
    for (int frame = 0; frame < kSettleFrames; ++frame) {
        backdrop.begin_frame(cam, kW, kH);
        backdrop.end_frame();
        ui.update();
        ui.render();
        SDL_GL_SwapWindow(window);
    }

    for (int i = 0; i < gus::app::screens::kControlsItemCount; ++i) {
        const std::string id = "controls-item-" + std::to_string(i);
        const glintfx::ElementBox box = ui.get_element_box(id.c_str());
        if (!box.found) continue;
        std::cout << id << ": found=1 x=" << box.x << " y=" << box.y << " w=" << box.w
                  << " h=" << box.h << "\n";
    }
    const glintfx::ElementBox head = ui.get_element_box("ctrl-cols-head");
    std::cout << "ctrl-cols-head: found=" << head.found << " x=" << head.x
              << " y=" << head.y << " w=" << head.w << " h=" << head.h << "\n";
    const glintfx::ElementBox list = ui.get_element_box("ctrl-list");
    std::cout << "ctrl-list: found=" << list.found << " x=" << list.x << " y=" << list.y
              << " w=" << list.w << " h=" << list.h << "\n";

    std::vector<unsigned char> pixels(static_cast<std::size_t>(kW) * kH * 4);
    const char* out_path_env = std::getenv("GUSWORLD_SCREENSHOT_OUT");
    const std::string out_path = (out_path_env != nullptr && out_path_env[0] != '\0')
                                      ? out_path_env
                                      : "/tmp/sysmenu_controls_probe.png";
    int ok = 0;
    if (gus::platform::rmlui::gl3_read_backbuffer_rgba(kW, kH, pixels.data())) {
        ok = stbi_write_png(out_path.c_str(), kW, kH, 4, pixels.data(), kW * 4);
    } else {
        std::cerr << "gl3_read_backbuffer_rgba falhou\n";
    }
    std::cout << "stbi_write_png(" << out_path << ") = " << ok << "\n";

    SDL_GL_DestroyContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return ok ? 0 : 1;
}
