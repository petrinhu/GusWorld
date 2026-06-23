// gus/app/screens/sprite_anchor.hpp
//
// ANCORAGEM do sprite pelos PES REAIS (POCO puro, mesma disciplina do
// sprite_animation: SEM Qt, SEM SDL, SEM I/O, SEM GPU). So aritmetica.
//
// PROBLEMA (M1-BUG.SUL): os PNGs do PixelLab tem MARGEM INFERIOR TRANSPARENTE
// (o conteudo/pes terminam alguns px ACIMA da base do canvas; ex.: canvas 68px,
// pes em y=57 => 11px transparentes embaixo). Se a base do CANVAS cai sobre a
// base da AABB de colisao, os PES VISUAIS ficam ~11px ACIMA da base da AABB. No
// SUL, onde o pe e o ponto de contato com a parede de baixo, vira um vao visivel.
//
// FIX: descontar a margem inferior transparente convertida para unidades de
// MUNDO, subindo a base do CANVAS o tanto necessario pra o PE REAL coincidir com
// a base da AABB. A margem e MEDIDA do alpha de cada sprite (alpha-bbox) no load
// - NAO hardcode: varia por personagem e por direcao.
//
// FONTE DA MEDICAO: o renderer (render2d_sdl) ja decodifica o PNG via stb_image
// no load_texture; ele expoe o content-bbox (IRenderer::texture_content_bbox). O
// loader le o bottom-margin do sprite IDLE de cada direcao e preenche o
// FootInset (em fracao do canvas). Aqui so convertemos fracao -> mundo e
// posicionamos. Ancorar pelo IDLE (nao por frame de walk) mantem o anchor
// ESTAVEL: o tronco/cabeca nao "pula" entre quadros (o pe ja "anda" via os
// proprios frames).
//
// Coberto por app/tests/sprite_anchor_test.cpp (TEST-FIRST).

#ifndef GUS_APP_SCREENS_SPRITE_ANCHOR_HPP
#define GUS_APP_SCREENS_SPRITE_ANCHOR_HPP

#include "gus/app/screens/sprite_animation.hpp"  // kDirectionCount

namespace gus::app::screens {

// Fracao [0,1] do canvas que e MARGEM INFERIOR TRANSPARENTE, por direcao (mesma
// ordem do enum Direction: Sul, Norte, Leste, Oeste). 0 = pe encosta na base do
// canvas (sem margem); 0.16 ~ 11/68 (caso medido do Caua south). Default 0 =
// comportamento legado (base do canvas == base da AABB). Preenchido pelo loader a
// partir do alpha-bbox medido; pode ficar zerado no headless (sem decode).
struct FootInset {
    float bottom_fraction[kDirectionCount] = {0.0f, 0.0f, 0.0f, 0.0f};

    [[nodiscard]] float for_direction(Direction d) const noexcept {
        const int i = static_cast<int>(d);
        if (i < 0 || i >= kDirectionCount) {
            return 0.0f;
        }
        return bottom_fraction[i];
    }
};

// Converte uma MARGEM INFERIOR em pixels (bottom_margin_px = canvas_h - bbox_b)
// para FRACAO do canvas [0,1]. Entradas degeneradas (canvas <= 0, margem < 0)
// devolvem 0 (anchor legado, seguro). Margem >= canvas (sprite todo
// transparente) clampa em 1.
[[nodiscard]] constexpr float bottom_margin_fraction(int bottom_margin_px,
                                                     int canvas_h_px) noexcept {
    if (canvas_h_px <= 0 || bottom_margin_px <= 0) {
        return 0.0f;
    }
    if (bottom_margin_px >= canvas_h_px) {
        return 1.0f;
    }
    return static_cast<float>(bottom_margin_px) /
           static_cast<float>(canvas_h_px);
}

// Calcula a coordenada Y (mundo, topo do quad do sprite) pra que o PE REAL do
// desenho caia sobre aabb_bottom_world.
//
//   sy = aabb_bottom_world - sprite_h_world + foot_inset_world + manual_offset_world
//
// onde foot_inset_world = bottom_fraction * sprite_h_world (sobe a base do canvas
// o tanto da margem transparente, em mundo) e manual_offset_world e o ajuste fino
// do lider (sprite_foot_offset, somado por cima; >0 desce a base).
//
// Sem margem (bottom_fraction = 0) e sem ajuste manual, recai EXATAMENTE no
// comportamento legado (base do canvas na base da AABB) - preserva os testes
// antigos e nao mexe na colisao, so no desenho.
[[nodiscard]] constexpr float sprite_top_y(float aabb_bottom_world,
                                           float sprite_h_world,
                                           float bottom_fraction,
                                           float manual_offset_world) noexcept {
    const float foot_inset_world = bottom_fraction * sprite_h_world;
    return aabb_bottom_world - sprite_h_world + foot_inset_world +
           manual_offset_world;
}

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_SPRITE_ANCHOR_HPP
