// gus/platform/render2d/font_atlas.hpp
//
// FONT ATLAS (M5, incremento 3.5): carrega um .ttf via stb_truetype e RASTERIZA o ASCII
// printable (32..126) num BITMAP de glifos (grayscale, 1 byte/px) + metricas/UV por
// caractere. Vive em platform/ (a fronteira: stb_truetype faz I/O de arquivo, proibido
// em core/domain por ADR-008). O bake e na CPU - testavel HEADLESS (sem SDL/janela): o
// Render2dSdl depois sobe o bitmap pra um SDL_Texture e desenha o texto glifo a glifo.
//
// MONOSPACE (Pixel Operator Mono, CC0): celula quadrada cell_px x cell_px por glifo,
// dispostas em grade. UV de cada caractere = sua celula normalizada [0,1] no atlas. As
// metricas de LARGURA pro layout vem do text_metrics (puro); aqui guardamos o raster.
//
// DEGRADACAO: arquivo ausente / stb_truetype falhar => FontAtlas com valid()==false e
// pixels vazio. O caller (Render2dSdl/app) degrada pro comportamento sem-fonte
// (barras/marcas), mantendo o headless/CI verde.
//
// Cross-ref: third_party/stb/stb_truetype.h; gus/platform/render2d/text_metrics.hpp;
//            gus/platform/render2d/render2d_sdl.hpp (sobe o atlas pra textura).

#ifndef GUS_PLATFORM_RENDER2D_FONT_ATLAS_HPP
#define GUS_PLATFORM_RENDER2D_FONT_ATLAS_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "gus/platform/render2d/i_renderer.hpp"  // UvRect

namespace gus::platform::render2d {

// Primeiro/ultimo codepoint ASCII rasterizado (printable). Espaco a til.
inline constexpr int kFontFirstChar = 32;   // ' '
inline constexpr int kFontLastChar = 126;   // '~'
inline constexpr int kFontGlyphCount = kFontLastChar - kFontFirstChar + 1;  // 95

// Atlas de glifos bakeado: bitmap grayscale (alpha do glifo) + grade de celulas. POCO
// de DADOS (sem SDL): o Render2dSdl converte pixels -> SDL_Texture. Movel, nao copiavel-
// caro (vetor de bytes); construido por bake_font_atlas.
struct FontAtlas {
    int cell_px = 0;       // lado da celula quadrada de cada glifo (px nativos do bake)
    int cols = 0;          // colunas da grade de glifos
    int rows = 0;          // linhas da grade
    int atlas_w = 0;       // largura do bitmap (cols * cell_px)
    int atlas_h = 0;       // altura do bitmap (rows * cell_px)
    std::vector<std::uint8_t> pixels;  // atlas_w*atlas_h bytes; 0=transparente, 255=tinta

    // true se o atlas foi bakeado (fonte carregada e rasterizada).
    [[nodiscard]] bool valid() const noexcept {
        return cell_px > 0 && atlas_w > 0 && atlas_h > 0 &&
               static_cast<int>(pixels.size()) == atlas_w * atlas_h;
    }

    // true se 'c' esta no range printable rasterizado (e o atlas e valido).
    [[nodiscard]] bool has_glyph(char c) const noexcept;

    // UV normalizada [0,1] da celula do caractere 'c'. w==0 (UV vazio) se 'c' esta fora
    // do range ou o atlas e invalido.
    [[nodiscard]] UvRect glyph_uv(char c) const noexcept;

    // true se a celula do glifo tem ALGUM pixel aceso (tinta). Util pro teste provar que
    // a letra foi rasterizada e o espaco nao. false se sem glifo/invalido.
    [[nodiscard]] bool glyph_has_ink(char c) const noexcept;
};

// Resolve o caminho de um .ttf da pasta de fontes da engine (assets/fonts/). Ordem,
// igual aos outros resolvers: env GUSWORLD_ASSETS (<env>/../assets/fonts? nao - ver .cpp)
// > macro GUSWORLD_FONTS_DIR embutido em compilacao > relativo ao CWD. So monta a
// STRING; nao abre arquivo.
[[nodiscard]] std::string resolve_font_path(const std::string& ttf_file);

// Bakeia o atlas de um .ttf no tamanho cell_px (altura da celula). Le o arquivo (I/O,
// fronteira platform/) e rasteriza 32..126 via stb_truetype. Devolve um FontAtlas
// invalido (valid()==false) se o arquivo faltar, stb_truetype recusar, ou cell_px<=0.
[[nodiscard]] FontAtlas bake_font_atlas(const std::string& ttf_path, int cell_px);

}  // namespace gus::platform::render2d

#endif  // GUS_PLATFORM_RENDER2D_FONT_ATLAS_HPP
