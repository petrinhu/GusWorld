// gus/core/anim/fade_transition.hpp
//
// FadeTransition: matematica PURA (POCO, ZERO SDL/GL/I-O) do overlay preto usado nas
// transicoes de tela (M7-COSTURA Inc 2, ADR-012 decisao 5: "fade preto curto com
// crossfade de musica"). Nao desenha nada e nao mede tempo real - so converte um
// "elapsed_seconds desde o inicio da fase" numa fracao de opacidade em [0,1]. Quem
// mede o tempo real (SDL_GetTicksNS, mesmo padrao do FixedTimestep) e quem DESENHA o
// retangulo preto (IRenderer::draw_filled_rect com DrawColor{0,0,0,alpha}) fica na
// camada app/ (gus/app/sdl_window.hpp::step_with_fade, gus/app/screens/battle_preview.
// cpp) - aqui e so a curva determinista, testavel sem janela.
//
// DUAS FASES da transicao (ver ADR-012 secao 5 + o relatorio do Inc 2):
//   kOut: a tela ESCURECE (0 -> 1) sobre a cena ATUAL, ainda visivel por baixo. Usado
//         ao SAIR de uma tela (cidade indo pra batalha, ou vice-versa).
//   kIn:  a tela CLAREIA (1 -> 0) sobre a cena NOVA, ja pronta por baixo. Usado ao
//         ENTRAR na tela seguinte, depois da troca tecnica (release/reacquire de
//         renderer, ou o contexto GL da batalha) ja ter acontecido no "escurinho".
//
// O CROSSFADE DE MUSICA (stop_music/play_music) e disparado pelo CHAMADOR no instante
// em que a curva kOut atinge 1.0 (tela 100% preta) - nao e responsabilidade deste
// modulo (audio fica em platform/audio; aqui so a curva visual).

#ifndef GUS_CORE_ANIM_FADE_TRANSITION_HPP
#define GUS_CORE_ANIM_FADE_TRANSITION_HPP

namespace gus::core::anim {

enum class FadeDirection {
    kOut,  // escurecendo: alpha 0 -> 1
    kIn,   // clareando: alpha 1 -> 0
};

// Alpha do overlay preto na fase `direction`, em elapsed_seconds desde o INICIO da
// fase, com duracao total duration_seconds. Clampa elapsed em [0, duration_seconds]
// antes de converter (tempos negativos ou alem do fim nao extrapolam). duration_
// seconds <= 0 devolve o estado FINAL da fase direto (1.0 pra kOut, 0.0 pra kIn) - uma
// transicao "instantanea" ainda tem um resultado bem definido, sem dividir por zero.
[[nodiscard]] float fade_overlay_alpha(FadeDirection direction, float elapsed_seconds,
                                        float duration_seconds) noexcept;

}  // namespace gus::core::anim

#endif  // GUS_CORE_ANIM_FADE_TRANSITION_HPP
