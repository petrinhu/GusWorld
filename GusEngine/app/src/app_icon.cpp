// SPDX-License-Identifier: GPL-3.0-or-later
// gus/app/src/app_icon.cpp
//
// Implementacao de set_window_icon_if_available. Ver header.
//
// PNG -> pixels RGBA via glintfx::decode_image_file (STB-IMAGE-APP, 2026-07-29); pixels ->
// SDL_Surface via SDL_CreateSurfaceFrom; SDL_Surface -> icone da janela via
// SDL_SetWindowIcon (que COPIA os pixels pro proprio uso interno - a surface/buffer podem
// ser liberados logo em seguida, sem lifetime compartilhado com a janela).
//
// STB-IMAGE-APP (2026-07-29): antes este arquivo incluia "stb_image.h" e chamava
// stbi_load() direto - violacao do contrato de camadas (app/ e a UNICA camada, com
// platform/, que pode tocar biblioteca externa DE FRONTEIRA; stb_image e detalhe de
// decode, nao de fronteira SDL/GL). A v0.25.0 do glintfx (bump STB-IMAGE-APP) trouxe
// glintfx::decode_image_file (glintfx/include/glintfx/image.hpp), que decodifica o PNG em
// pixels sem exigir contexto GL - substitui stbi_load 1-pra-1 aqui.
//
// ALPHA: decode_image_file devolve alpha STRAIGHT (nao-premultiplicado), exatamente o que
// stbi_load ja devolvia e o que SDL_CreateSurfaceFrom/SDL_SetWindowIcon esperam (o SDL
// compoe o icone sozinho, fora do pipeline de blend do jogo - NAO passa por
// Draw2d::create_texture, que premultiplica Rgba8 na ingestao pro proprio blend GL). Como
// este arquivo nunca toca create_texture, nao ha conversao de premultiply a fazer aqui;
// bastasse reusar o buffer straight-alpha tal como decodificado.

#include "gus/app/app_icon.hpp"

#include <string>

#include <glintfx/image.hpp>

#include "gus/core/asset_paths.hpp"
#include "gus/platform/assets/asset_source.hpp"  // ASSETS-VFS-F1 (ADR-013): porteiro

namespace gus::app {

void set_window_icon_if_available(SDL_Window* window) {
    const std::string id = std::string(gus::core::assets::kAppIconDir) + "/" +
                            std::string(gus::core::assets::kAppIconFile256);
    const std::string path =
        gus::platform::assets::FilesystemAssetSource().resolve_path(id);

    const glintfx::DecodedImagePixels decoded = glintfx::decode_image_file(path.c_str());
    if (!decoded.ok) {
        SDL_Log("gus::app: icone do app ausente/ilegivel (%s) - seguindo sem icone",
                path.c_str());
        return;  // degradacao segura: janela sem icone, nao crasha
    }

    // const_cast: SDL_CreateSurfaceFrom exige void* (a API dela nao distingue leitura de
    // escrita), mas ela so LE os pixels pra montar a surface - decoded.pixels continua dono
    // do buffer e o libera normalmente ao sair de escopo (RAII, sem free manual).
    SDL_Surface* surface = SDL_CreateSurfaceFrom(
        decoded.width, decoded.height, SDL_PIXELFORMAT_RGBA32,
        const_cast<unsigned char*>(decoded.pixels.data()), decoded.width * 4);
    if (surface == nullptr) {
        SDL_Log("gus::app: SDL_CreateSurfaceFrom (icone) falhou: %s", SDL_GetError());
        return;  // degradacao segura
    }

    if (!SDL_SetWindowIcon(window, surface)) {
        SDL_Log("gus::app: SDL_SetWindowIcon falhou: %s", SDL_GetError());
        // segue sem icone - nao crasha
    }

    SDL_DestroySurface(surface);
}

}  // namespace gus::app
