// SPDX-License-Identifier: Apache-2.0
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

#include <SDL3/SDL.h>

namespace gus::app::screens {

// Inicializa SDL (video), abre a janela do viewer, roda o loop e fecha tudo.
// Devolve 0 em saida limpa; != 0 se SDL/janela falharem ou nao houver animacao
// alguma (pasta de sprites ausente). Chamado pelo main quando --anim-preview.
int run_anim_preview();

// F4-1b.6 (onda F4 "casca SDL -> App mode do glintfx", fatia 1b.6 - ULTIMA
// tela com pump proprio da onda, fora as ja convertidas + npc_dialogue_loop
// aposentado na fatia 7): estado MINIMO que anim_preview_step precisa pra
// decidir o roteamento de UM SDL_Event - SO o indice da anim corrente + quantas
// ha no catalogo. NUNCA os recursos de GPU/IO (frames carregados via
// IRenderer::load_texture, o AnimClock inteiro) - a troca REAL de anim
// (recarregar frames + AnimClock::set_frame_count) e efeito colateral e fica
// com o CHAMADOR (AnimPreviewScreen::handle_event, anim_preview.cpp).
struct AnimPreviewState {
    int anim_idx = 0;
    int catalog_size = 1;
};

// Resultado de UM anim_preview_step(): o que o CHAMADOR deve fazer a seguir -
// TODOS os side effects reais (SDL_SetWindowTitle, IRenderer::load_texture,
// AnimClock::set_frame_count/nudge_fps) ficam FORA desta struct/funcao, que so
// decide.
struct AnimPreviewStepResult {
    bool window_closed = false;  // SDL_EVENT_QUIT
    bool finished = false;       // Esc
    bool switch_anim = false;    // Tab/seta-direita/seta-esquerda
    int new_anim_idx = 0;        // valido SO se switch_anim==true (JA com o
                                  // wrap aplicado nos 2 sentidos - o CHAMADOR
                                  // so recarrega os frames dessa anim)
    float fps_delta = 0.0f;      // seta-cima=+1.0f / seta-baixo=-1.0f; 0.0f se
                                  // nenhuma foi pressionada (o CHAMADOR aplica
                                  // via AnimClock::nudge_fps)
};

// O NUCLEO PURO do roteamento de evento do viewer - extraido behavior-preserving
// do while(true) antigo (ver anim_preview.cpp) pra ficar 100% testavel headless
// (SEM SDL_Init/janela/IRenderer/AnimClock real - ver anim_preview_step_test.cpp).
// NAO consulta IRenderer/AnimClock - so calcula a DECISAO a partir de
// (anim_idx, catalog_size) + o SDL_Event JA pumpado. Eventos que este viewer nao
// roteia (ex.: MOUSE_MOTION) devolvem o resultado default (tudo false/0) -
// no-op TOTAL.
[[nodiscard]] AnimPreviewStepResult anim_preview_step(const AnimPreviewState& state,
                                                       const SDL_Event& ev) noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_ANIM_PREVIEW_HPP
