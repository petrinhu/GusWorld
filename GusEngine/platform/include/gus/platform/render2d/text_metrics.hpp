// gus/platform/render2d/text_metrics.hpp
//
// METRICAS DE TEXTO puras (POCO testavel SEM SDL, SEM I/O): largura/altura de uma
// string numa fonte MONOSPACE, dado o tamanho em px logico. Vive em platform/render2d/
// (junto da fonte) mas NAO toca SDL nem stb_truetype: e so aritmetica de layout, pra a
// logica que depende da LARGURA do texto (centrar verbo na caixa, caber o log) rodar
// headless. O atlas/raster (stb_truetype + textura) fica no font_atlas (parte SDL).
//
// MONOSPACE (Pixel Operator Mono): cada glifo avanca a MESMA largura. A razao
// advance/altura e uma propriedade da fonte (kMonoAdvanceRatio): largura_glifo =
// ratio * px_size. A altura de uma linha = px_size (o caller soma entrelinha se quiser).
// Conta CARACTERES como bytes ASCII (o texto de UI do slice e ASCII/Latin-1 simples;
// acentos UTF-8 multibyte sao raros nos rotulos curtos e tratados como 1 avanco cada
// byte - aceitavel pro layout do slice, refinavel depois sem mudar a interface).
//
// Cross-ref: gus/platform/render2d/font_atlas.hpp (raster, parte SDL);
//            gus/platform/render2d/i_renderer.hpp (draw_text).

#ifndef GUS_PLATFORM_RENDER2D_TEXT_METRICS_HPP
#define GUS_PLATFORM_RENDER2D_TEXT_METRICS_HPP

#include <cstdint>
#include <string_view>
#include <vector>

namespace gus::platform::render2d {

// Razao largura-de-avanco / altura-de-celula da Pixel Operator Mono (monospace). Medida
// da fonte: a celula mono tem ~0.5 da altura em largura de avanco. Constante de layout;
// se trocar a fonte, re-mede aqui (a parte SDL do atlas confirma com stbtt_GetFontVMetrics).
inline constexpr float kMonoAdvanceRatio = 0.5f;

// Decodifica uma string UTF-8 numa lista de CODE POINTS (BUG A, incremento 6): o jogo e
// pt-br, entao acento (c-cedilha, a-til, e-agudo...) e essencial. Um code point latino e
// 2 bytes em UTF-8 (ex.: 'c' = 0xC3 0xA7); iterar por BYTE quebraria o texto. Sequencias
// invalidas/truncadas viram U+FFFD (replacement) ou sao ignoradas com seguranca (nunca
// lanca). Continuation bytes soltos sao pulados. POCO puro (sem SDL).
[[nodiscard]] std::vector<std::uint32_t> decode_utf8(std::string_view text);

// Numero de CODE POINTS (avancos monospace) de uma string UTF-8 (= decode_utf8().size()).
[[nodiscard]] int text_char_count(std::string_view text) noexcept;

// Largura em px logico de uma string monospace ao tamanho px_size (altura da celula).
// = char_count * kMonoAdvanceRatio * px_size. px_size <= 0 => 0. Pura/deterministica.
[[nodiscard]] float text_width(std::string_view text, float px_size) noexcept;

// Altura em px logico de UMA linha ao tamanho px_size (= px_size, clamp >= 0).
[[nodiscard]] float text_height(float px_size) noexcept;

// Avanco (largura) de UM glifo ao tamanho px_size (= kMonoAdvanceRatio * px_size).
[[nodiscard]] float glyph_advance(float px_size) noexcept;

}  // namespace gus::platform::render2d

#endif  // GUS_PLATFORM_RENDER2D_TEXT_METRICS_HPP
