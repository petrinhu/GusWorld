// gus/platform/src/render2d/render2d_sdl.cpp
//
// Implementacao do Render2dSdl (backend de IRenderer sobre SDL_Renderer). Ver
// header. Travado por platform/tests/render2d_sdl_test.cpp (TEST-FIRST, caminho
// headless) + smoke do app (caminho com renderer real).
//
// PNG -> SDL_Texture via stb_image (vendorizado em third_party/stb). A definicao
// da implementacao do stb fica AQUI (esta e a unica TU que a usa no render2d).

#include "gus/platform/render2d/render2d_sdl.hpp"

#include <cstdint>

#include "gus/platform/render2d/viewport_transform.hpp"

// stb_image: so esta TU define a implementacao (evita simbolos duplicados).
// Usamos stbi_load(path,...) que abre o arquivo (precisa de stdio - default ON).
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG  // o pipeline de arte do GusWorld exporta PNG
#include "stb_image.h"

namespace gus::platform::render2d {

namespace {

// Converte DrawColor [0,1] para o byte [0,255] do SDL (clampa).
std::uint8_t to_byte(float c) noexcept {
    if (c <= 0.0f) return 0;
    if (c >= 1.0f) return 255;
    return static_cast<std::uint8_t>(c * 255.0f + 0.5f);
}

}  // namespace

Render2dSdl::Render2dSdl(SDL_Renderer* renderer) noexcept : renderer_(renderer) {
    // Slot 0 reservado: kInvalidTexture == 0 nunca indexa uma textura real.
    textures_.push_back(nullptr);
}

Render2dSdl::~Render2dSdl() {
    // Libera as SDL_Texture owned (slot 0 e nullptr reservado).
    for (std::size_t i = 1; i < textures_.size(); ++i) {
        if (textures_[i] != nullptr) {
            SDL_DestroyTexture(textures_[i]);
        }
    }
}

void Render2dSdl::begin_frame(const gus::core::spatial::Rect& camera_world,
                              int pixel_w, int pixel_h) {
    camera_ = camera_world;
    pixel_w_ = pixel_w;
    pixel_h_ = pixel_h;
    draw_count_ = 0;

    if (renderer_ == nullptr) {
        return;  // headless: nada a limpar
    }
    // Limpa a tela com um cinza-fundo neutro (placeholder; sem chao desenhado).
    SDL_SetRenderDrawColor(renderer_, 24, 26, 34, 255);
    SDL_RenderClear(renderer_);
}

void Render2dSdl::draw_filled_rect(const gus::core::spatial::Rect& world_rect,
                                   const DrawColor& color) {
    ++draw_count_;
    if (renderer_ == nullptr) {
        return;
    }
    const QuadScreen q =
        build_quad_screen(world_rect, camera_, pixel_w_, pixel_h_);
    SDL_SetRenderDrawColor(renderer_, to_byte(color.r), to_byte(color.g),
                           to_byte(color.b), to_byte(color.a));
    SDL_FRect dst{q.x, q.y, q.w, q.h};
    SDL_RenderFillRect(renderer_, &dst);
}

void Render2dSdl::draw_rect_outline(const gus::core::spatial::Rect& world_rect,
                                    const DrawColor& color,
                                    float /*thickness_world*/) {
    ++draw_count_;
    if (renderer_ == nullptr) {
        return;
    }
    const QuadScreen q =
        build_quad_screen(world_rect, camera_, pixel_w_, pixel_h_);
    SDL_SetRenderDrawColor(renderer_, to_byte(color.r), to_byte(color.g),
                           to_byte(color.b), to_byte(color.a));
    // Contorno de 1px (a espessura em mundo e aproximada por SDL_RenderRect).
    SDL_FRect dst{q.x, q.y, q.w, q.h};
    SDL_RenderRect(renderer_, &dst);
}

TextureId Render2dSdl::load_texture(const char* path) {
    if (path == nullptr || path[0] == '\0') {
        return kInvalidTexture;
    }
    // Cache por caminho: o mesmo arquivo devolve o mesmo handle.
    const std::string key(path);
    auto it = texture_by_path_.find(key);
    if (it != texture_by_path_.end()) {
        return it->second;
    }

    // Headless (sem renderer): nao ha como criar SDL_Texture. Degrada.
    if (renderer_ == nullptr) {
        return kInvalidTexture;
    }

    // Carrega o PNG como RGBA8 (4 canais forcados) via stb_image.
    int w = 0, h = 0, channels = 0;
    stbi_uc* pixels = stbi_load(path, &w, &h, &channels, 4);
    if (pixels == nullptr || w <= 0 || h <= 0) {
        if (pixels != nullptr) {
            stbi_image_free(pixels);
        }
        return kInvalidTexture;
    }

    // Cria a SDL_Texture estatica RGBA32 e sobe os pixels.
    SDL_Texture* tex = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32,
                                         SDL_TEXTUREACCESS_STATIC, w, h);
    if (tex == nullptr) {
        stbi_image_free(pixels);
        return kInvalidTexture;
    }
    // pitch = w * 4 bytes (RGBA8).
    SDL_UpdateTexture(tex, nullptr, pixels, w * 4);
    stbi_image_free(pixels);

    // NEAREST (pixel-art crisp) + blend alpha (sprite com transparencia).
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    const TextureId id = static_cast<TextureId>(textures_.size());
    textures_.push_back(tex);
    texture_by_path_[key] = id;
    return id;
}

void Render2dSdl::draw_textured_rect(const gus::core::spatial::Rect& world_rect,
                                     TextureId texture, const UvRect& uv,
                                     const DrawColor& tint) {
    // Textura invalida e no-op (cabe ao chamador o fallback) - nao conta como draw.
    if (texture == kInvalidTexture || texture >= textures_.size()) {
        return;
    }
    ++draw_count_;
    if (renderer_ == nullptr) {
        return;
    }
    SDL_Texture* tex = textures_[texture];
    if (tex == nullptr) {
        return;
    }

    // Destino em pixels (projecao mundo->tela).
    const QuadScreen q =
        build_quad_screen(world_rect, camera_, pixel_w_, pixel_h_);
    SDL_FRect dst{q.x, q.y, q.w, q.h};

    // Origem (UV [0,1] -> pixels da textura). uv {0,0,1,1} = textura inteira.
    float tw = 0.0f, th = 0.0f;
    SDL_GetTextureSize(tex, &tw, &th);
    SDL_FRect src{uv.u * tw, uv.v * th, uv.w * tw, uv.h * th};

    // Tint multiplicativo (cor + alpha). Branco {1,1,1,1} = sem tinta.
    SDL_SetTextureColorMod(tex, to_byte(tint.r), to_byte(tint.g), to_byte(tint.b));
    SDL_SetTextureAlphaMod(tex, to_byte(tint.a));

    SDL_RenderTexture(renderer_, tex, &src, &dst);
}

void Render2dSdl::end_frame() {
    last_draw_count_ = draw_count_;
    if (renderer_ == nullptr) {
        return;  // headless: nada a apresentar
    }
    SDL_RenderPresent(renderer_);
}

}  // namespace gus::platform::render2d
