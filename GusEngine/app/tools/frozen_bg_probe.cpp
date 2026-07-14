// PROBE EFEMERO (NAO COMMITADO - app/tools/ fica de fora do git, MESMO padrao de
// npcdlg_screenshot_probe.cpp/repro_bertoldo.cpp neste mesmo diretorio) - prova
// headless (Xvfb :99, NUNCA :0) do fundo real congelado (M7-DIALOGO/MENU-PAUSA-
// CONFIG-SOM): captura 1 frame REAL da cidade (SdlWindow::capture_frame_to_png,
// MESMA tecnica de Maestro::to_npc_dialogue/open_pause_from_city), depois desenha
// a caixa de dialogo E o menu de pausa por cima dessa cena real (Render2dGl3::
// draw_textured_rect, MESMA tecnica dos loops de producao) - 2 PNGs de saida.
// Descartar apos a prova (nao faz parte do jogo).

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
#include "gus/app/screens/npc_dialogue_rml.hpp"
#include "gus/app/screens/system_menu.hpp"
#include "gus/app/screens/system_menu_rml.hpp"
#include "gus/app/sdl_window.hpp"
#include "gus/core/asset_paths.hpp"
#include "gus/core/spatial/camera_clamp.hpp"
#include "gus/domain/dialogue/dialogue_graph.hpp"
#include "gus/platform/render2d/render2d_gl3.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"

namespace fs = std::filesystem;

namespace {

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

bool make_gl_context(SDL_Window* window, SDL_GLContext* out_gl) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    *out_gl = SDL_GL_CreateContext(window);
    if (*out_gl == nullptr) {
        std::cerr << "SDL_GL_CreateContext falhou: " << SDL_GetError() << "\n";
        return false;
    }
    SDL_GL_MakeCurrent(window, *out_gl);
    SDL_GL_SetSwapInterval(0);
    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        std::cerr << "gl3_load_functions falhou (glad)\n";
        return false;
    }
    return true;
}

bool capture_screenshot(SDL_Window* window, int w, int h, const std::string& out_path) {
    std::vector<unsigned char> pixels(static_cast<std::size_t>(w) * h * 4);
    if (!gus::platform::rmlui::gl3_read_backbuffer_rgba(w, h, pixels.data())) {
        std::cerr << "gl3_read_backbuffer_rgba falhou\n";
        return false;
    }
    const int ok = stbi_write_png(out_path.c_str(), w, h, 4, pixels.data(), w * 4);
    std::cout << "stbi_write_png(" << out_path << ") = " << ok << "\n";
    (void)window;
    return ok != 0;
}

}  // namespace

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }

    constexpr int kW = 960;
    constexpr int kH = 540;

    // JANELA UNICA com flag OPENGL desde o inicio - MESMA receita de Maestro::init()
    // (SdlWindow::init_attached cria o SDL_Renderer NUMA janela ja OPENGL-flagged).
    SDL_Window* window =
        SDL_CreateWindow("frozen_bg_probe", kW, kH, SDL_WINDOW_OPENGL);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow falhou: " << SDL_GetError() << "\n";
        return 1;
    }

    // 1) CAPTURA REAL da cidade (MESMA tecnica de Maestro::to_npc_dialogue/
    // open_pause_from_city -> SdlWindow::capture_frame_to_png) - ANTES de soltar o
    // renderer.
    gus::app::SdlWindow city;
    if (!city.init_attached(window)) {
        std::cerr << "city.init_attached falhou\n";
        return 1;
    }
    // Alguns ticks de update (parado no spawn e suficiente pra provar a tecnica -
    // nao precisa andar ate o Bertoldo/inimigo pra esta prova).
    for (int i = 0; i < 5; ++i) {
        city.step();
    }
    const std::string frozen_path =
        (fs::temp_directory_path() / "gusworld_frozen_bg_probe_city.png").string();
    const bool captured = city.capture_frame_to_png(frozen_path);
    std::cout << "capture_frame_to_png(" << frozen_path << ") = " << captured << "\n";
    if (!captured) {
        return 1;
    }
    city.release_renderer();

    // 2) CONTEXTO GL PROPRIO (MESMA janela - troca de backend, MESMA tecnica de
    // run_npc_dialogue_loop_gl/run_system_menu_loop_owning_gl).
    SDL_GLContext gl = nullptr;
    if (!make_gl_context(window, &gl)) {
        return 1;
    }

    gus::app::i18n::Translator translator;
    const std::string tr_path = gus::app::i18n::resolve_translations_path();
    if (!translator.load_from_file(tr_path)) {
        std::cerr << "Translator::load_from_file falhou: " << tr_path << "\n";
        return 1;
    }

    const gus::core::spatial::Rect cam{0.0f, 0.0f, static_cast<float>(kW),
                                        static_cast<float>(kH)};
    int rc = 0;

    // ---------------------------------------------------------------- DIALOGO
    {
        gus::domain::dialogue::DialogueNode node;
        node.id = "n0";
        node.speaker_id = "bertoldo";
        node.text_key = "DIALOGUE_NPC_INTRO_N0_GREET";
        const std::string portrait_file =
            gus::app::screens::npc_dialogue_portrait_file(node.speaker_id);
        const std::string rml = gus::app::screens::build_npc_dialogue_rml(
            node, translator, /*selected_option=*/0, portrait_file,
            /*continue_pressed=*/false);

        const fs::path stage =
            fs::temp_directory_path() / "gusworld_frozen_bg_probe_dialogue";
        std::error_code ec;
        fs::create_directories(stage, ec);
        const char* fonts_dir_env = std::getenv("GUSWORLD_FONTS");
        const std::string fonts_dir =
            (fonts_dir_env != nullptr) ? std::string(fonts_dir_env) : std::string();
        if (!fonts_dir.empty()) {
            fs::copy_file(join(fonts_dir, "PixelOperatorMono.ttf"),
                          stage / "PixelOperatorMono.ttf",
                          fs::copy_options::overwrite_existing, ec);
            fs::copy_file(join(fonts_dir, "PixelOperatorMono-Bold.ttf"),
                          stage / "PixelOperatorMono-Bold.ttf",
                          fs::copy_options::overwrite_existing, ec);
        }
        const char* retratos_env = std::getenv("GUSWORLD_ASSETS");
        const std::string retratos_dir =
            join(retratos_env != nullptr ? std::string(retratos_env) : std::string("resources"),
                 std::string(gus::core::assets::kRetratosDir));
        fs::copy_file(join(retratos_dir, portrait_file), stage / portrait_file,
                      fs::copy_options::overwrite_existing, ec);

        std::string rml_with_fonts = rml;
        const std::string needle = "<style>\n";
        const std::size_t pos = rml_with_fonts.find(needle);
        if (pos != std::string::npos) {
            rml_with_fonts.insert(
                pos + needle.size(),
                "@font-face { font-family: \"Pixel Operator Mono\"; "
                "src: \"PixelOperatorMono.ttf\"; }\n"
                "@font-face { font-family: \"Pixel Operator Mono\"; "
                "font-weight: bold; src: \"PixelOperatorMono-Bold.ttf\"; }\n");
        }
        const fs::path rml_path = stage / "npc_dialogue.rml";
        {
            std::ofstream f(rml_path);
            f << rml_with_fonts;
        }

        glintfx::UiLayer ui(glintfx::UiLayer::Config{/*logical_width=*/kW,
                                                      /*logical_height=*/kH,
                                                      /*load_gl=*/true,
                                                      /*dp_ratio=*/1.0f});
        if (!ui.ok()) {
            std::cerr << "glintfx::UiLayer::ok()=false (dialogo)\n";
            rc = 1;
        } else {
            ui.set_asset_base_url(stage.string().c_str());
            ui.load(rml_path.string().c_str());
            ui.set_viewport(kW, kH);
            ui.set_dp_ratio(1.0f);

            gus::platform::render2d::Render2dGl3 backdrop(/*gl_active=*/true);
            const gus::platform::render2d::TextureId frozen_tex =
                backdrop.load_texture(frozen_path.c_str());
            std::cout << "[dialogo] frozen_tex valido="
                      << (frozen_tex != gus::platform::render2d::kInvalidTexture) << "\n";

            constexpr int kSettleFrames = 20;
            for (int frame = 0; frame < kSettleFrames; ++frame) {
                backdrop.begin_frame(cam, kW, kH);
                if (frozen_tex != gus::platform::render2d::kInvalidTexture) {
                    backdrop.draw_textured_rect(
                        cam, frozen_tex,
                        gus::platform::render2d::UvRect{0.0f, 0.0f, 1.0f, 1.0f},
                        gus::platform::render2d::DrawColor{1.0f, 1.0f, 1.0f, 1.0f});
                }
                backdrop.end_frame();
                ui.update();
                ui.render();
                SDL_GL_SwapWindow(window);
            }

            const std::string out_path =
                (fs::temp_directory_path() / "gusworld_frozen_bg_probe_dialogue.png")
                    .string();
            if (!capture_screenshot(window, kW, kH, out_path)) rc = 1;
        }
    }

    // ---------------------------------------------------------------- PAUSA
    {
        gus::app::screens::SystemMenuState state;
        state.music_volume = 0.8f;
        state.sfx_volume = 0.6f;
        gus::app::screens::system_menu_open(state);
        const std::string rml = gus::app::screens::build_system_menu_rml(state, translator);

        const fs::path stage =
            fs::temp_directory_path() / "gusworld_frozen_bg_probe_sysmenu";
        std::error_code ec;
        fs::create_directories(stage, ec);
        const char* fonts_dir_env = std::getenv("GUSWORLD_FONTS");
        const std::string fonts_dir =
            (fonts_dir_env != nullptr) ? std::string(fonts_dir_env) : std::string();
        if (!fonts_dir.empty()) {
            fs::copy_file(join(fonts_dir, "PixelOperatorMono.ttf"),
                          stage / "PixelOperatorMono.ttf",
                          fs::copy_options::overwrite_existing, ec);
            fs::copy_file(join(fonts_dir, "PixelOperatorMono-Bold.ttf"),
                          stage / "PixelOperatorMono-Bold.ttf",
                          fs::copy_options::overwrite_existing, ec);
        }

        std::string rml_with_fonts = rml;
        const std::string needle = "<style>\n";
        const std::size_t pos = rml_with_fonts.find(needle);
        if (pos != std::string::npos) {
            rml_with_fonts.insert(
                pos + needle.size(),
                "@font-face { font-family: \"Pixel Operator Mono\"; "
                "src: \"PixelOperatorMono.ttf\"; }\n"
                "@font-face { font-family: \"Pixel Operator Mono\"; "
                "font-weight: bold; src: \"PixelOperatorMono-Bold.ttf\"; }\n");
        }
        const fs::path rml_path = stage / "system_menu.rml";
        {
            std::ofstream f(rml_path);
            f << rml_with_fonts;
        }

        glintfx::UiLayer ui(glintfx::UiLayer::Config{/*logical_width=*/kW,
                                                      /*logical_height=*/kH,
                                                      /*load_gl=*/true,
                                                      /*dp_ratio=*/1.0f});
        if (!ui.ok()) {
            std::cerr << "glintfx::UiLayer::ok()=false (pausa)\n";
            rc = 1;
        } else {
            ui.set_asset_base_url(stage.string().c_str());
            ui.load(rml_path.string().c_str());
            ui.set_viewport(kW, kH);
            ui.set_dp_ratio(1.0f);

            gus::platform::render2d::Render2dGl3 backdrop(/*gl_active=*/true);
            const gus::platform::render2d::TextureId frozen_tex =
                backdrop.load_texture(frozen_path.c_str());
            std::cout << "[pausa] frozen_tex valido="
                      << (frozen_tex != gus::platform::render2d::kInvalidTexture) << "\n";

            constexpr int kSettleFrames = 20;
            for (int frame = 0; frame < kSettleFrames; ++frame) {
                backdrop.begin_frame(cam, kW, kH);
                if (frozen_tex != gus::platform::render2d::kInvalidTexture) {
                    backdrop.draw_textured_rect(
                        cam, frozen_tex,
                        gus::platform::render2d::UvRect{0.0f, 0.0f, 1.0f, 1.0f},
                        gus::platform::render2d::DrawColor{1.0f, 1.0f, 1.0f, 1.0f});
                }
                backdrop.end_frame();
                ui.update();
                ui.render();
                SDL_GL_SwapWindow(window);
            }

            const std::string out_path =
                (fs::temp_directory_path() / "gusworld_frozen_bg_probe_sysmenu.png")
                    .string();
            if (!capture_screenshot(window, kW, kH, out_path)) rc = 1;
        }
    }

    SDL_GL_DestroyContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return rc;
}
