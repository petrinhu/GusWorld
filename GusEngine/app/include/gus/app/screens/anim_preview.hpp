// gus/app/screens/anim_preview.hpp
//
// AnimPreview: VIEWER de animacao do Gus, acionado por --anim-preview. Reusa a
// MESMA casca de plataforma do jogo (janela + SDL_Renderer + Render2dSdl atras de
// IRenderer); NAO cria outro backend grafico. E uma casca app/ (toca SDL: janela,
// poll de evento, titulo), igual a SdlWindow - so que mostra um sprite por vez em
// loop, em vez de simular o overworld.
//
// O QUE FAZ:
//   - resolve_gus_sprites_dir() + build_gus_anim_catalog(): lista as animacoes
//     (anims/, walk/<dir>, turntable das rotations/) em runtime;
//   - carrega os frames da anim corrente via IRenderer.load_texture (cache do
//     Render2dSdl evita recarga);
//   - AnimClock (core, POCO) dirige o avanco de quadro por TEMPO (~10 fps default);
//   - desenha o frame CENTRALIZADO e escalado pra caber, fundo neutro;
//   - HUD no titulo da janela (SDL_SetWindowTitle): "<anim> | frame i/N | fps".
//
// TECLAS: Tab / seta-direita = proxima anim; seta-esquerda = anim anterior;
//         seta-cima / seta-baixo = +/- fps; Esc = sair.
//
// Inclui <SDL3/SDL.h> (camada app/, SDL permitido). O run() abre/fecha a janela e
// roda o loop ate Esc/fechar. run_anim_preview() e o ponto de entrada chamado pelo
// main (cuida do SDL_Init/SDL_Quit e devolve o codigo de saida).

#ifndef GUS_APP_SCREENS_ANIM_PREVIEW_HPP
#define GUS_APP_SCREENS_ANIM_PREVIEW_HPP

namespace gus::app::screens {

// Inicializa SDL (video), abre a janela do viewer, roda o loop e fecha tudo.
// Devolve 0 em saida limpa; != 0 se SDL/janela falharem ou nao houver animacao
// alguma (pasta de sprites ausente). Chamado pelo main quando --anim-preview.
int run_anim_preview();

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_ANIM_PREVIEW_HPP
