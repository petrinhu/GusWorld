// gus/app/src/glitch_overlay.cpp
//
// Implementacao de draw_glitch_overlay. Ver header. Travado por
// GusEngine/app/tests/glitch_overlay_test.cpp (TEST-FIRST).

#include "gus/app/glitch_overlay.hpp"

#include "gus/core/anim/glitch_dissolve.hpp"

namespace gus::app {

namespace {

using gus::core::spatial::Rect;
using gus::platform::render2d::DrawColor;

// Largura da FRANJA (wavefront) em unidades do envelope [0,1]: celulas com
// "folga" (alpha - limiar) menor que isto ficam coloridas (cyan/magenta) em vez
// de preto solido - o "flash" de energia da compilacao. ~6% do envelope inteiro:
// numa transicao de kTransitionFadeSeconds (~0.4s, ver maestro.cpp) a 60fps, cada
// celula fica na franja por 1-2 frames - punchy, nao um fade suave.
constexpr float kWavefrontBand = 0.06f;

// Deslocamento horizontal MAXIMO (fracao da largura da celula) aplicado as
// celulas na franja - o "bloco deslocado" (datamosh-lite) pedido. 35%: visivel
// sem descaracterizar a grade (a celula ainda cobre a maior parte do seu lugar).
constexpr float kShiftMagnitude = 0.35f;

// As 2 cores de "flash" da franja - cyan e o accent "hit"/critico, ambas JA
// canonicas na paleta do cockpit (gus/app/screens/battle_preview.cpp, RCSS do
// cockpit: box-shadow cyan do halo #22D3EE; #log .hit usa E11D74). Reusadas aqui
// em vez de inventar uma cor nova.
constexpr DrawColor kFlashCyan{0x22 / 255.0f, 0xD3 / 255.0f, 0xEE / 255.0f, 1.0f};
constexpr DrawColor kFlashMagenta{0xE1 / 255.0f, 0x1D / 255.0f, 0x74 / 255.0f, 1.0f};

// Cor da celula "assentada" (fora da franja, ja compilada) - preto solido, igual
// ao retangulo liso original.
constexpr DrawColor kSettledBlack{0.0f, 0.0f, 0.0f, 1.0f};

// Escolha DETERMINISTICA (sem nova funcao no POCO - so combinatoria de col/row)
// de qual accent a franja da celula (col,row) usa: ~1 em 5 celulas fica magenta,
// o resto cyan (o cyan e a cor "de sistema" default; o magenta pontua como
// "erro/critico" ocasional, sem dominar a paleta).
[[nodiscard]] bool cell_uses_magenta_accent(int col, int row) noexcept {
    return (col * 7 + row * 13) % 5 == 0;
}

// N linhas finas escurecidas (scanlines - textura de CRT/estatica), so na banda
// MEIO da transicao (nunca nos extremos - ver o guard em draw_glitch_overlay).
constexpr int kScanlineCount = 30;
constexpr float kScanlineBaseAlpha = 0.16f;

// Quantas vezes a barra de "leitura" (scan beam) cyan varre a grade de cima a
// baixo ao longo de UMA transicao inteira (alpha 0->1 ou 1->0) - puramente
// deterministico a partir do envelope `alpha` (sem tempo real/estado): reforca a
// leitura de "sistema processando/lendo o frame".
constexpr float kSweepCycles = 2.0f;
constexpr float kScanBeamAlpha = 0.45f;

}  // namespace

void draw_glitch_overlay(gus::platform::render2d::IRenderer& renderer,
                          const Rect& screen_rect, float alpha) {
    namespace anim = gus::core::anim;

    if (alpha <= 0.0f) {
        return;  // tela limpa - nada a desenhar (mesmo invariante do fade liso).
    }

    const float cell_w = screen_rect.w / static_cast<float>(anim::kGlitchGridCols);
    const float cell_h = screen_rect.h / static_cast<float>(anim::kGlitchGridRows);

    // ---- 1) DISSOLVE EM BLOCOS (a base do efeito - substitui o retangulo liso) ----
    for (int row = 0; row < anim::kGlitchGridRows; ++row) {
        for (int col = 0; col < anim::kGlitchGridCols; ++col) {
            const float cell_alpha = anim::glitch_block_alpha(col, row, alpha);
            if (cell_alpha <= 0.0f) {
                continue;  // celula ainda nao "compilou" - a cena aparece por baixo.
            }

            Rect cell{screen_rect.x + static_cast<float>(col) * cell_w,
                      screen_rect.y + static_cast<float>(row) * cell_h, cell_w,
                      cell_h};

            DrawColor color = kSettledBlack;
            if (anim::glitch_cell_is_wavefront(col, row, alpha, kWavefrontBand)) {
                // Franja: colore de accent (cyan/magenta) E desloca horizontalmente
                // (a celula inteira, sem ler pixel nenhum da cena por baixo) - o
                // "bloco deslocado"/datamosh-lite pedido.
                color = cell_uses_magenta_accent(col, row) ? kFlashMagenta : kFlashCyan;
                const float shift = anim::glitch_cell_shift_fraction(col, row);
                cell.x += shift * cell_w * kShiftMagnitude;
            }
            renderer.draw_filled_rect(cell, color);
        }
    }

    // ---- 2) SCANLINES + SCAN BEAM (so no MEIO da transicao - nunca nos extremos,
    // preserva o invariante de seguranca: no pico so a grade solida acima existe) ----
    if (alpha >= 1.0f) {
        return;
    }

    const float scan_h = screen_rect.h / static_cast<float>(kScanlineCount);
    const DrawColor scanline_color{0.0f, 0.0f, 0.0f, kScanlineBaseAlpha * alpha};
    for (int i = 0; i < kScanlineCount; i += 2) {  // alternadas (a cada 2a linha)
        const Rect line{screen_rect.x, screen_rect.y + static_cast<float>(i) * scan_h,
                        screen_rect.w, scan_h * 0.4f};
        renderer.draw_filled_rect(line, scanline_color);
    }

    // Barra de "leitura" cyan: varre kSweepCycles vezes ao longo de alpha [0,1],
    // 100% determinista (mesmo alpha -> mesma linha sempre, sem estado/tempo real).
    const float sweep_pos = alpha * static_cast<float>(anim::kGlitchGridRows) * kSweepCycles;
    const int beam_row =
        static_cast<int>(sweep_pos) % anim::kGlitchGridRows;
    const DrawColor beam_color{kFlashCyan.r, kFlashCyan.g, kFlashCyan.b,
                                kScanBeamAlpha * alpha};
    const Rect beam{screen_rect.x, screen_rect.y + static_cast<float>(beam_row) * cell_h,
                    screen_rect.w, cell_h * 0.12f};
    renderer.draw_filled_rect(beam, beam_color);
}

}  // namespace gus::app
