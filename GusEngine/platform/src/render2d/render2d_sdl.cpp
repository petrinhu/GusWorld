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
#include <vector>

#include "gus/core/asset_paths.hpp"  // nomes dos arquivos de fonte centralizados
#include "gus/platform/render2d/alpha_bbox.hpp"  // scan_alpha_content_bbox (POCO)
#include "gus/platform/render2d/text_metrics.hpp"  // glyph_advance (monospace)
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
    // Slot 0 reservado: kInvalidTexture == 0 nunca indexa uma textura real. Os dois
    // caches (textura + alpha-bbox) sao indexados PARALELO pelo mesmo TextureId.
    textures_.push_back(nullptr);
    bboxes_.push_back(ContentBbox{});  // slot 0: invalido (valid() == false)
}

Render2dSdl::~Render2dSdl() {
    // Libera as SDL_Texture owned (slot 0 e nullptr reservado).
    for (std::size_t i = 1; i < textures_.size(); ++i) {
        if (textures_[i] != nullptr) {
            SDL_DestroyTexture(textures_[i]);
        }
    }
    // Texturas do atlas de fonte (regular/bold).
    if (font_regular_.texture != nullptr) {
        SDL_DestroyTexture(font_regular_.texture);
    }
    if (font_bold_.texture != nullptr) {
        SDL_DestroyTexture(font_bold_.texture);
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

    // ANCORAGEM (M1-BUG.SUL): mede o alpha-bbox do conteudo AGORA, com os pixels
    // ainda em memoria (antes de liberar). E o que da pra COLAR o pe na base da
    // AABB sem numero magico - cada sprite mede a propria margem inferior vazia.
    const ContentBbox bbox =
        scan_alpha_content_bbox(static_cast<const std::uint8_t*>(pixels), w, h);
    stbi_image_free(pixels);

    // NEAREST (pixel-art crisp) + blend alpha (sprite com transparencia).
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    const TextureId id = static_cast<TextureId>(textures_.size());
    textures_.push_back(tex);
    bboxes_.push_back(bbox);  // PARALELO a textures_ (mesmo id indexa os dois)
    texture_by_path_[key] = id;
    return id;
}

ContentBbox Render2dSdl::texture_content_bbox(TextureId texture) const {
    // Fora de faixa / invalido / slot 0: ContentBbox vazio (valid() == false). O
    // chamador (anchor do sprite) degrada pro comportamento legado (margem 0). No
    // headless (renderer nulo) nenhuma textura e criada, entao sempre cai aqui.
    if (texture == kInvalidTexture || texture >= bboxes_.size()) {
        return ContentBbox{};
    }
    return bboxes_[texture];
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

Render2dSdl::FontFace* Render2dSdl::ensure_font(bool bold) {
    FontFace& face = bold ? font_bold_ : font_regular_;
    if (face.tried) {
        return face.atlas.valid() && face.texture != nullptr ? &face : nullptr;
    }
    face.tried = true;

    // Headless (sem renderer): nao da pra criar textura. Fica indisponivel (no-op).
    if (renderer_ == nullptr) {
        return nullptr;
    }

    // Bake na CPU (16px nativo = multiplo de 8 da Pixel Operator, crisp ao escalar).
    // Nomes dos .ttf vem do header central de caminhos de asset.
    const std::string file(bold ? gus::core::assets::kFontMonoBoldFile
                                 : gus::core::assets::kFontMonoRegularFile);
    face.atlas = bake_font_atlas(resolve_font_path(file), /*cell_px=*/16);
    if (!face.atlas.valid()) {
        return nullptr;  // fonte ausente: degrada (sem texto, so fallback do caller)
    }

    // Converte o atlas grayscale (alpha) -> RGBA32 (branco + alpha do glifo). O
    // color-mod no draw tinge depois. NEAREST = crisp ao escalar pro px_size logico.
    const int w = face.atlas.atlas_w;
    const int h = face.atlas.atlas_h;
    std::vector<std::uint8_t> rgba(static_cast<std::size_t>(w) * h * 4);
    for (std::size_t i = 0; i < static_cast<std::size_t>(w) * h; ++i) {
        const std::uint8_t a = face.atlas.pixels[i];
        rgba[i * 4 + 0] = 255;  // R
        rgba[i * 4 + 1] = 255;  // G
        rgba[i * 4 + 2] = 255;  // B
        rgba[i * 4 + 3] = a;    // A = tinta do glifo
    }
    SDL_Texture* tex = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32,
                                         SDL_TEXTUREACCESS_STATIC, w, h);
    if (tex == nullptr) {
        face.atlas = FontAtlas{};  // invalida pra cair no no-op
        return nullptr;
    }
    SDL_UpdateTexture(tex, nullptr, rgba.data(), w * 4);
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    face.texture = tex;
    return &face;
}

void Render2dSdl::draw_text(const char* text, float x, float y, float px_size,
                            const DrawColor& color, bool bold) {
    if (text == nullptr || px_size <= 0.0f) {
        return;
    }
    FontFace* face = ensure_font(bold);
    if (face == nullptr) {
        return;  // sem fonte (headless/ausente): no-op, o caller ja desenhou o fallback
    }

    SDL_Texture* tex = face->texture;
    float tw = 0.0f, th = 0.0f;
    SDL_GetTextureSize(tex, &tw, &th);
    SDL_SetTextureColorMod(tex, to_byte(color.r), to_byte(color.g), to_byte(color.b));
    SDL_SetTextureAlphaMod(tex, to_byte(color.a));

    const float advance = glyph_advance(px_size);  // largura monospace por glifo (mundo)
    float pen_x = x;
    // BUG A: DECODIFICA UTF-8 -> code points (acentos do pt-br sao multibyte). Itera por
    // CODE POINT, nao por byte: cada code point ocupa 1 celula monospace e mapeia 1 glifo.
    for (const std::uint32_t cp : decode_utf8(text)) {
        const UvRect uv = face->atlas.glyph_uv(static_cast<int>(cp));
        if (uv.w > 0.0f) {
            // Quad de MUNDO da celula (px_size x px_size) na caneta atual; projeta.
            const gus::core::spatial::Rect cell_world{pen_x, y, px_size, px_size};
            const QuadScreen q =
                build_quad_screen(cell_world, camera_, pixel_w_, pixel_h_);
            SDL_FRect dst{q.x, q.y, q.w, q.h};
            SDL_FRect src{uv.u * tw, uv.v * th, uv.w * tw, uv.h * th};
            SDL_RenderTexture(renderer_, tex, &src, &dst);
            ++draw_count_;  // 1 por glifo desenhavel (codepoint dentro das faixas baked)
        }
        pen_x += advance;  // monospace: 1 avanco por code point, mesmo sem glifo
    }
}

void Render2dSdl::end_frame() {
    last_draw_count_ = draw_count_;
    if (renderer_ == nullptr) {
        return;  // headless: nada a apresentar
    }
    // ADR-009: quando o HUD do RmlUi vai compor por cima, o present e ADIADO (o dono do
    // frame chama present() depois do compose). Sem defer, mantem o comportamento antigo.
    if (defer_present_) {
        return;
    }
    SDL_RenderPresent(renderer_);
}

void Render2dSdl::present() {
    if (renderer_ == nullptr) {
        return;  // headless: nada a apresentar
    }
    SDL_RenderPresent(renderer_);
}

}  // namespace gus::platform::render2d
