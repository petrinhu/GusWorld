// gus/app/src/app_icon.cpp
//
// Implementacao de set_window_icon_if_available. Ver header.
//
// PNG -> pixels RGBA via stb_image; pixels -> SDL_Surface via SDL_CreateSurfaceFrom;
// SDL_Surface -> icone da janela via SDL_SetWindowIcon (que COPIA os pixels pro proprio
// uso interno - a surface/buffer podem ser liberados logo em seguida, sem lifetime
// compartilhado com a janela).

#include "gus/app/app_icon.hpp"

#include <string>

#include "gus/core/asset_paths.hpp"
#include "gus/platform/assets/asset_source.hpp"  // ASSETS-VFS-F1 (ADR-013): porteiro

// stb_image: SO o header aqui, SEM STB_IMAGE_IMPLEMENTATION (a implementacao ja existe
// numa UNICA TU, gus/platform/src/render2d/render2d_sdl.cpp, dentro de gusengine_platform
// - target que gusengine_app ja linka PUBLIC, ver CMakeLists.txt) - reusa os simbolos ja
// compilados de stbi_load/stbi_image_free, sem duplicar.
#include "stb_image.h"

namespace gus::app {

void set_window_icon_if_available(SDL_Window* window) {
    const std::string id = std::string(gus::core::assets::kAppIconDir) + "/" +
                            std::string(gus::core::assets::kAppIconFile256);
    const std::string path =
        gus::platform::assets::FilesystemAssetSource().resolve_path(id);

    int w = 0, h = 0, channels = 0;
    unsigned char* pixels =
        stbi_load(path.c_str(), &w, &h, &channels, /*desired_channels=*/4);
    if (pixels == nullptr) {
        SDL_Log("gus::app: icone do app ausente/ilegivel (%s) - seguindo sem icone",
                path.c_str());
        return;  // degradacao segura: janela sem icone, nao crasha
    }

    SDL_Surface* surface =
        SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, pixels, w * 4);
    if (surface == nullptr) {
        SDL_Log("gus::app: SDL_CreateSurfaceFrom (icone) falhou: %s", SDL_GetError());
        stbi_image_free(pixels);
        return;  // degradacao segura
    }

    if (!SDL_SetWindowIcon(window, surface)) {
        SDL_Log("gus::app: SDL_SetWindowIcon falhou: %s", SDL_GetError());
        // segue sem icone - nao crasha
    }

    SDL_DestroySurface(surface);
    stbi_image_free(pixels);
}

}  // namespace gus::app
