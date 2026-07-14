// PROBE EFEMERO (NAO COMMITADO - app/tools/ fica de fora do git ate hoje, MESMO
// padrao de npcdlg_hover_probe.cpp/repro_bertoldo.cpp neste mesmo diretorio) -
// captura headless (Xvfb :99, NUNCA :0) de 1 frame da caixa quente do dialogo com
// o botao "Continuar" real (texto + fonte de producao), pra "observar antes de
// liberar" (pedido do lider). Escreve um PNG via stb_image_write (ja vendorizado
// em third_party/stb/). Descartar apos a prova (nao faz parte do jogo).
//
// Nao passa por run_npc_dialogue_loop_gl (esse loop nao expoe hook de screenshot)
// - reconstroi o MESMO setup de contexto GL + glintfx::UiLayer que ele usa
// internamente (MESMOS atributos GL, MESMA tecnica de stage dir com fonte+retrato
// copiados), so que lendo o CATALOGO REAL de traducao (resolve_translations_path())
// em vez de um Translator sintetico, pra provar o texto de PRODUCAO ("Continuar").

#include <cstdlib>
#include <cstring>
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
#include "gus/core/asset_paths.hpp"
#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect
#include "gus/domain/dialogue/dialogue_graph.hpp"
#include "gus/platform/render2d/render2d_gl3.hpp"  // Render2dGl3 (clear real, MESMA tecnica de
                                                     // npc_dialogue_loop_gl.cpp - evita GL cru)
#include "gus/platform/rmlui/gl3_loader.hpp"  // glad load + gl3_read_backbuffer_rgba (captura)

namespace fs = std::filesystem;

namespace {

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

std::string resolve_assets_subdir(std::string_view rel) {
    const std::string sub(rel);
    if (const char* env = std::getenv("GUSWORLD_ASSETS")) {
        if (env[0] != '\0') return join(env, sub);
    }
    const std::string compiled = GUSWORLD_ASSETS_DIR;
    if (!compiled.empty()) return join(compiled, sub);
    return join("resources", sub);
}

}  // namespace

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }

    constexpr int kW = 960;
    constexpr int kH = 540;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window =
        SDL_CreateWindow("npcdlg_screenshot_probe", kW, kH, SDL_WINDOW_OPENGL);
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

    // Traducao REAL (game/translations/pt_br.md) - prova o texto de PRODUCAO, nao
    // um valor sintetico digitado no probe.
    gus::app::i18n::Translator translator;
    const std::string tr_path = gus::app::i18n::resolve_translations_path();
    if (!translator.load_from_file(tr_path)) {
        std::cerr << "Translator::load_from_file falhou: " << tr_path << "\n";
        return 1;
    }
    std::cout << "DIALOGUE_CONTINUE resolvido = \"" << translator.tr("DIALOGUE_CONTINUE")
              << "\" (catalogo: " << tr_path << ", " << translator.size() << " chaves)\n";

    // No sintetico LINEAR (options vazio -> mostra o botao "Continuar"), MESMO
    // speaker_id=bertoldo do NPC real (retrato existente em disco).
    gus::domain::dialogue::DialogueNode node;
    node.id = "n0";
    node.speaker_id = "bertoldo";
    node.text_key = "DIALOGUE_NPC_INTRO_N0_GREET";

    const std::string portrait_file =
        gus::app::screens::npc_dialogue_portrait_file(node.speaker_id);
    const std::string rml =
        gus::app::screens::build_npc_dialogue_rml(node, translator, /*selected_option=*/0,
                                                    portrait_file, /*continue_pressed=*/false);

    // Stage dir (MESMA receita de npc_dialogue_loop_gl.cpp): fonte + retrato copiados
    // flat, doc referencia por nome flat.
    const fs::path stage = fs::temp_directory_path() / "gusworld_npcdlg_screenshot_probe";
    std::error_code ec;
    fs::create_directories(stage, ec);

    const char* fonts_dir_env = std::getenv("GUSWORLD_FONTS");
    const std::string fonts_dir =
        (fonts_dir_env != nullptr && fonts_dir_env[0] != '\0')
            ? std::string(fonts_dir_env)
            : std::string(GUSWORLD_FONTS_DIR);
    if (!fonts_dir.empty()) {
        fs::copy_file(join(fonts_dir, "PixelOperatorMono.ttf"),
                      stage / "PixelOperatorMono.ttf",
                      fs::copy_options::overwrite_existing, ec);
        fs::copy_file(join(fonts_dir, "PixelOperatorMono-Bold.ttf"),
                      stage / "PixelOperatorMono-Bold.ttf",
                      fs::copy_options::overwrite_existing, ec);
    }

    const std::string retratos_dir =
        resolve_assets_subdir(gus::core::assets::kRetratosDir);
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
        std::cerr << "glintfx::UiLayer::ok()=false (attach falhou)\n";
        return 1;
    }
    ui.set_asset_base_url(stage.string().c_str());
    ui.load(rml_path.string().c_str());
    ui.set_viewport(kW, kH);
    ui.set_dp_ratio(1.0f);

    // Render2dGl3 faz o clear real (MESMA tecnica de present_frame() em
    // npc_dialogue_loop_gl.cpp) - evita GL cru neste probe (gl3_loader.hpp e
    // "header limpo", nao expoe glClear/glReadPixels diretamente).
    gus::platform::render2d::Render2dGl3 backdrop(/*gl_active=*/true);
    const gus::core::spatial::Rect cam{0.0f, 0.0f, static_cast<float>(kW),
                                        static_cast<float>(kH)};

    // ~20 frames pra assentar o bake do atlas de fonte (MESMO capture_at_frame=20
    // do smoke visual --battle em battle_preview.cpp) antes do shot real.
    constexpr int kSettleFrames = 20;
    for (int frame = 0; frame < kSettleFrames; ++frame) {
        backdrop.begin_frame(cam, kW, kH);
        backdrop.end_frame();
        ui.update();
        ui.render();
        SDL_GL_SwapWindow(window);
    }

    // Hit-box do botao "Continuar" (prova de que existe/renderizou, complementa a
    // conferencia visual do PNG).
    const glintfx::ElementBox box = ui.get_element_box("npcdlg-continue-btn");
    std::cout << "npcdlg-continue-btn box: found=" << box.found << " x=" << box.x
              << " y=" << box.y << " w=" << box.w << " h=" << box.h << "\n";

    std::vector<unsigned char> pixels(static_cast<std::size_t>(kW) * kH * 4);
    const char* out_path_env = std::getenv("GUSWORLD_SCREENSHOT_OUT");
    const std::string out_path =
        (out_path_env != nullptr && out_path_env[0] != '\0')
            ? out_path_env
            : "/tmp/npcdlg_continue_probe.png";
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
