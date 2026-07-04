// gus/app/glitch_overlay.hpp
//
// draw_glitch_overlay: DESENHO do efeito de GLITCH DIGITAL/"PROCESSANDO" que
// substitui o fade preto liso da transicao cidade<->batalha (M7-COSTURA Inc 2b,
// pedido ao vivo do Gus Dragon + veredito do lider: "o sistema runico do Gus
// REBOOTANDO pro modo de combate" - Pillar "magia = software"). Camada app/
// (RENDER, nao POCO) - a matematica determinista (limiar por celula/franja/
// deslocamento) mora em gus/core/anim/glitch_dissolve.hpp; aqui e so o LACO que
// converte essa matematica em chamadas de draw_filled_rect.
//
// FACTIBILIDADE NOS 2 BACKENDS (decisao desta onda, ver relatorio): usa SOMENTE
// IRenderer::draw_filled_rect (a MESMA primitiva que o retangulo preto liso ja
// usava) - funciona IDENTICO no SDL_Renderer classico da cidade (Render2dSdl) e
// no GL3 da arena (Render2dGl3), sem precisar de shader nem de ler o framebuffer
// (que nenhum dos dois backends expoe via IRenderer). Um "datamosh" literal
// (deslocar PIXELS JA RENDERIZADOS) exigiria acesso ao conteudo do frame anterior
// - fora do escopo desta interface; o efeito abaixo APROXIMA a sensacao de
// "bloco deslocado" desenhando a propria celula (cor solida) num x levemente
// deslocado, sem tocar pixel nenhum da cena por baixo.
//
// PONTOS UNICOS de chamada (substituindo o antigo draw_filled_rect(rect, preto)):
// gus/app/sdl_window.cpp::step_with_fade (lado CIDADE) e gus/app/screens/
// battle_preview.cpp (os 2 loops de fade, entrada e saida, lado BATALHA).
//
// INVARIANTE DE SEGURANCA preservado (ver o header do POCO): alpha<=0 nao desenha
// nada (tela limpa) e alpha>=1 cobre a grade INTEIRA com preto solido opaco, sem
// franja colorida nem celula deslocada - o instante em que a Maestro troca o
// SDL_Renderer pelo contexto GL (release_renderer/reacquire_renderer) continua
// 100% escondido atras de um retangulo opaco equivalente ao fade antigo.

#ifndef GUS_APP_GLITCH_OVERLAY_HPP
#define GUS_APP_GLITCH_OVERLAY_HPP

#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect
#include "gus/platform/render2d/i_renderer.hpp"

namespace gus::app {

// Desenha o overlay de glitch por cima do frame ja renderizado (chamar DEPOIS da
// cena, ANTES do present/swap - mesmo ponto onde o retangulo preto liso entrava).
// `screen_rect` e o retangulo de MUNDO que cobre a tela inteira (cam.rect na
// cidade; gus::app::screens::battle_screen_rect() na batalha). `alpha` e o
// envelope da transicao (gus::core::anim::fade_overlay_alpha), [0,1] = quao
// "coberta"/"compilada" a tela deveria estar AGORA - o MESMO valor que antes ia
// direto pro DrawColor::a do retangulo preto liso.
void draw_glitch_overlay(gus::platform::render2d::IRenderer& renderer,
                          const gus::core::spatial::Rect& screen_rect,
                          float alpha);

}  // namespace gus::app

#endif  // GUS_APP_GLITCH_OVERLAY_HPP
