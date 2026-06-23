// gus/platform/render2d/alpha_bbox.hpp
//
// MEDICAO do bounding-box do CONTEUDO nao-transparente (alpha > limiar) de uma
// imagem RGBA8, POCO puro: SO aritmetica sobre um buffer de bytes - SEM SDL, SEM
// GPU, SEM I/O. Por que aqui (platform/render2d) e nao em core/: o conceito
// "pixel/alpha de uma textura" e de RENDER (o ContentBbox vive no contrato do
// IRenderer); mas a VARREDURA em si nao toca SDL, entao da pra travar com Catch2
// sem display (platform/tests/alpha_bbox_test.cpp).
//
// PARA QUE SERVE: o Render2dSdl decodifica o PNG via stb_image (RGBA8) no
// load_texture; chama scan_alpha_content_bbox sobre esses pixels e cacheia o
// ContentBbox por TextureId. O anchor do sprite (app/screens/sprite_anchor.hpp)
// le ContentBbox::bottom_margin pra COLAR o pe na base da AABB (M1-BUG.SUL), sem
// numero magico - cada sprite/direcao mede a propria sobra transparente.

#ifndef GUS_PLATFORM_RENDER2D_ALPHA_BBOX_HPP
#define GUS_PLATFORM_RENDER2D_ALPHA_BBOX_HPP

#include <cstddef>
#include <cstdint>

#include "gus/platform/render2d/i_renderer.hpp"  // ContentBbox

namespace gus::platform::render2d {

// Limiar de alpha (0..255) acima do qual um pixel conta como CONTEUDO. 0 ignora
// apenas o transparente puro (o caso do PixelLab: fundo alpha 0). Pixels semi-
// transparentes (sombra leve) acima do limiar contam - desejado pra achar a base.
inline constexpr std::uint8_t kAlphaContentThreshold = 0;

// Varre o buffer RGBA8 (4 bytes/pixel, linha-maior, origem topo-esquerda) e
// devolve o ContentBbox do conteudo com alpha > kAlphaContentThreshold. Entradas
// degeneradas (rgba == nullptr, w<=0, h<=0) ou imagem TODA transparente devolvem um
// ContentBbox com valid()==false (width/height = 0) - o chamador degrada pro anchor
// legado (margem 0). canvas_w/h sao preenchidos com w/h dados quando w,h > 0.
[[nodiscard]] inline ContentBbox scan_alpha_content_bbox(const std::uint8_t* rgba,
                                                         int w, int h) noexcept {
    ContentBbox box;
    if (rgba == nullptr || w <= 0 || h <= 0) {
        return box;  // valid() == false (canvas 0)
    }
    box.canvas_w = w;
    box.canvas_h = h;

    int min_x = w, min_y = h, max_x = -1, max_y = -1;
    for (int y = 0; y < h; ++y) {
        const std::uint8_t* row =
            rgba + static_cast<std::size_t>(y) * static_cast<std::size_t>(w) * 4;
        for (int x = 0; x < w; ++x) {
            const std::uint8_t a = row[static_cast<std::size_t>(x) * 4 + 3];  // alpha
            if (a > kAlphaContentThreshold) {
                if (x < min_x) min_x = x;
                if (x > max_x) max_x = x;
                if (y < min_y) min_y = y;
                if (y > max_y) max_y = y;
            }
        }
    }

    if (max_x < 0) {
        // Nenhum pixel opaco: tudo transparente. width/height ficam 0 => invalido.
        return box;
    }
    box.left = min_x;
    box.top = min_y;
    box.width = max_x - min_x + 1;
    box.height = max_y - min_y + 1;
    return box;
}

}  // namespace gus::platform::render2d

#endif  // GUS_PLATFORM_RENDER2D_ALPHA_BBOX_HPP
