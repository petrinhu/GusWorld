// PROBE EFEMERO (NAO COMMITADO - app/tools/ fica de fora do git, MESMO padrao de
// sysmenu_controls_screenshot_probe.cpp/npcdlg_screenshot_probe.cpp neste mesmo
// diretorio) - captura headless (Xvfb :99, NUNCA :0) de 1 frame REAL da tela
// de save/load (SAVE-LOAD-UI), pra PROVA VISUAL do que o mock aprovado
// (07-save-load.html) exige: slot mostrando XP + Capitulo + timestamp/playtime,
// autosave so-leitura, slot vazio, e o mini-dialogo de sobrescrita. Descartar
// apos a prova.
//
// Uso: GUSWORLD_PROBE_MODE=save|load (default save)
//      GUSWORLD_PROBE_CONFIRM=1 (mostra o mini-dialogo de sobrescrita)
//      GUSWORLD_SCREENSHOT_OUT=/caminho/saida.png

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
#include "gus/app/screens/save_load_menu.hpp"
#include "gus/app/screens/save_load_menu_rml.hpp"
#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect
#include "gus/platform/render2d/render2d_gl3.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"

namespace fs = std::filesystem;

namespace {
std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

gus::domain::save::SaveData make_save_data(int xp, const std::string& scene,
                                            std::int64_t timestamp_ms,
                                            double playtime_seconds) {
    gus::domain::save::SaveData data;
    data.current_scene_path = scene;
    data.timestamp_ms = timestamp_ms;
    data.playtime_seconds = playtime_seconds;
    data.character_states["gus"].xp = xp;
    return data;
}
}  // namespace

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }

    int kW = 960;
    int kH = 540;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window =
        SDL_CreateWindow("save_load_screenshot_probe", kW, kH, SDL_WINDOW_OPENGL);
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

    using gus::app::screens::SaveLoadMenuState;
    using gus::app::screens::SaveLoadMode;
    using gus::app::screens::SaveSlotPreview;
    using gus::domain::save::kAutosaveSlot;
    using gus::domain::save::kSlotCount;

    std::array<SaveSlotPreview, kSlotCount> slots{};
    slots[kAutosaveSlot] = gus::app::screens::build_slot_preview(
        make_save_data(550, "distritos_inferiores", 1783627200000LL, 3300.0),
        kAutosaveSlot);
    slots[1] = gus::app::screens::build_slot_preview(
        make_save_data(340, "distritos_inferiores", 1783455240000LL, 2532.0), 1);
    slots[2] = gus::app::screens::build_slot_preview(
        make_save_data(810, "distritos_inferiores", 1783368660000LL, 4680.0), 2);
    for (int i = 3; i < kSlotCount; ++i)
        slots[static_cast<std::size_t>(i)] = gus::app::screens::empty_slot_preview(i);

    const std::string mode_env = std::getenv("GUSWORLD_PROBE_MODE")
                                      ? std::getenv("GUSWORLD_PROBE_MODE")
                                      : "save";
    const SaveLoadMode mode = (mode_env == "load") ? SaveLoadMode::Load : SaveLoadMode::Save;

    SaveLoadMenuState state;
    gus::app::screens::save_load_menu_open(state, mode, slots);

    if (const char* confirm = std::getenv("GUSWORLD_PROBE_CONFIRM")) {
        if (confirm[0] == '1') {
            gus::app::screens::save_load_menu_key_down(state, SDLK_RETURN);
        }
    }

    std::string rml = gus::app::screens::build_save_load_menu_rml(state, translator);

    const fs::path stage = fs::temp_directory_path() / "gusworld_save_load_screenshot_probe";
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
    const fs::path rml_path = stage / "save_load_menu.rml";
    {
        std::ofstream f(rml_path);
        f << rml;
    }

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

    for (const char* id : {"slmenu-scrim", "slmenu-panel", "slmenu-slot-1", "slmenu-back"}) {
        const glintfx::ElementBox box = ui.get_element_box(id);
        std::cout << id << ": found=" << box.found << " x=" << box.x << " y=" << box.y
                  << " w=" << box.w << " h=" << box.h << "\n";
    }

    std::vector<unsigned char> pixels(static_cast<std::size_t>(kW) * kH * 4);
    const char* out_path_env = std::getenv("GUSWORLD_SCREENSHOT_OUT");
    const std::string out_path = (out_path_env != nullptr && out_path_env[0] != '\0')
                                      ? out_path_env
                                      : "/tmp/save_load_probe.png";
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
