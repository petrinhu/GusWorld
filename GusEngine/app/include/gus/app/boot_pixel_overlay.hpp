// gus/app/boot_pixel_overlay.hpp
//
// BootPixelOverlay: carregamento + DESENHO da SEQUENCIA DE FRAMES pre-renderizada
// (boot de sistema pixelizado, 20 PNGs 320x180 em resources/vfx/boot_pixel/) que
// SUBSTITUI o glitch procedural (M7-COSTURA Inc 2c - o lider VETOU o glitch ao vivo:
// "pareceu bug"; a sequencia de frames FOI aprovada: um monitor CRT bootando, Pillar
// "magia = software" - o sistema runico do Gus ligando pro combate). Camada app/
// (RENDER, nao POCO): a matematica determinista de INDICE de frame (progresso ->
// frame) mora em gus/core/anim/boot_pixel_sequence.hpp; aqui e so o carregamento +
// o laco de desenho.
//
// PONTOS UNICOS de chamada (os MESMOS 3 lugares do glitch_dissolve/glitch_overlay
// aposentados por este modulo): gus/app/sdl_window.cpp::step_with_fade (lado CIDADE,
// backend Render2dSdl) e gus/app/screens/battle_preview.cpp (os 2 loops de fade,
// entrada e saida, lado BATALHA, backend Render2dGl3). CADA lado precisa da SUA
// PROPRIA instancia + load() (TextureId sao locais ao renderer VIVO - mesmo racional
// de enemy_marker_tex_/load_player_sprites em sdl_window.cpp: handles de uma tabela
// de texturas que NAO sobrevive a troca/destruicao do renderer). Recarregar em
// reacquire_renderer() (apos release_renderer) segue o MESMO padrao ja usado pros
// sprites do Gus.
//
// FACTIBILIDADE NOS 2 BACKENDS: usa SOMENTE IRenderer::load_texture +
// draw_textured_rect + draw_filled_rect (a MESMA interface que o resto do app ja usa
// - funciona IDENTICO no SDL_Renderer classico da cidade e no GL3 da arena, sem
// shader dedicado). draw_textured_rect JA e NEAREST sampling por contrato do header
// i_renderer.hpp (pixel-art crisp) - desenhar o frame 320x180 esticado pro
// screen_rect logico (960x540 em AMBOS os lados) da o upscale nearest-neighbor
// pedido de graca, sem nenhuma logica de escala manual aqui.
//
// CAMADA DE SEGURANCA (herdada do fade preto liso ORIGINAL, de antes do glitch
// existir): os frames sao OPACOS por design (cobrem a tela inteira), mas draw()
// SEMPRE desenha PRIMEIRO um retangulo solido (gunmetal escuro, MESMO tom de fundo do
// cockpit #0c1322 - reusado, nao inventado) escalado por gus::core::anim::
// boot_pixel_safety_alpha(leg,t) - a MESMA forma que fade_overlay_alpha calculava
// (Darkening cresce 0->1, Revealing decresce 1->0) - se um frame tiver borda
// transparente (aliasing do PNG) OU os 20 frames falharem ao carregar (headless/asset
// ausente), o solido garante que a troca de SDL_Renderer<->contexto GL da Maestro
// (release_renderer/reacquire_renderer) continua 100% escondida. Mesmo invariante do
// fade/glitch antigos: alpha<=0 nao desenha nada (tela limpa) - alcancado nos
// extremos exatos t=0 (Darkening) e t=1 (Revealing).

#ifndef GUS_APP_BOOT_PIXEL_OVERLAY_HPP
#define GUS_APP_BOOT_PIXEL_OVERLAY_HPP

#include <string>
#include <vector>

#include "gus/core/anim/boot_pixel_sequence.hpp"  // BootPixelLeg
#include "gus/core/spatial/camera_clamp.hpp"      // gus::core::spatial::Rect
#include "gus/platform/render2d/i_renderer.hpp"

namespace gus::app {

class BootPixelOverlay {
public:
    // Carrega os gus::core::anim::kBootPixelFrameCount frames (frame_00.png..
    // frame_NN.png) de `frames_dir` no `renderer` DADO. Chamar UMA VEZ por instancia
    // de renderer vivo (init/init_attached da SdlWindow; 1x por entrada na batalha do
    // battle_preview) - e de novo apos reacquire_renderer() (os TextureId antigos nao
    // sobrevivem a troca de renderer). Devolve ready() (true SO se OS 20 carregaram -
    // ver o comentario de ready()).
    bool load(gus::platform::render2d::IRenderer& renderer,
              const std::string& frames_dir);

    // true SO se TODOS os 20 frames carregaram (TextureId valido) na ultima load().
    // false degrada draw() pra SO a camada solida de seguranca (NUNCA mostra uma
    // sequencia PARCIAL/quebrada - isso pareceria um bug de novo, o que o lider
    // vetou). Tambem false antes de qualquer load() ter rodado (estado inicial).
    [[nodiscard]] bool ready() const noexcept;

    // Desenha o overlay por cima do frame ja renderizado (mesmo ponto onde o
    // retangulo preto/glitch entrava - DEPOIS da cena, ANTES do present/swap).
    // `screen_rect` e o retangulo de MUNDO que cobre a tela logica inteira (cam.rect
    // na cidade; battle_screen_rect() na batalha - ambos 960x540). `leg` identifica
    // QUAL das 4 pernas da transicao este ponto de desenho representa (ver o
    // comentario extenso em gus/core/anim/boot_pixel_sequence.hpp - a fonte de
    // verdade de qual leg cada um dos 4 call-sites usa). `t` e elapsed/duration DESTA
    // metade fisica ([0,1], sempre ascendente no tempo real, independente do leg) -
    // dirige TANTO a camada solida de seguranca (gus::core::anim::
    // boot_pixel_safety_alpha) QUANTO o INDICE do frame corrente (gus::core::anim::
    // boot_pixel_frame_index), garantindo que os 20 frames formem UM UNICO arco
    // continuo ao longo da transicao inteira (nao um reset no meio, ver o header do
    // POCO).
    void draw(gus::platform::render2d::IRenderer& renderer,
              const gus::core::spatial::Rect& screen_rect,
              gus::core::anim::BootPixelLeg leg, float t) const;

private:
    std::vector<gus::platform::render2d::TextureId> frames_;
    bool ready_ = false;
};

}  // namespace gus::app

#endif  // GUS_APP_BOOT_PIXEL_OVERLAY_HPP
