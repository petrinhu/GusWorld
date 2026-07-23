// gus/core/anim/boot_pixel_sequence.hpp
//
// boot_pixel_frame_index / boot_pixel_safety_alpha: matematica PURA (POCO, ZERO SDL/
// GL/I-O) da sequencia de BOOT PIXELIZADO (M7-COSTURA Inc 2c: 20 PNGs pre-
// renderizados, resources/vfx/boot_pixel/frame_00.png..frame_19.png, que SUBSTITUEM
// o glitch procedural vetado pelo lider - "pareceu bug"). Nao desenha nada, nao
// carrega textura, nao mede tempo real - testavel sem janela.
//
// POR QUE UMA UNICA SEQUENCIA DE 20 FRAMES PRECISA DE 4 "PERNAS" (legs): a transicao
// cidade<->batalha tecnicamente acontece em DUAS METADES fisicas (a Maestro faz
// release_renderer/reacquire_renderer no ESCURINHO entre elas - ver maestro.cpp):
//   1) o lado que ESCURECE (a tela ATUAL, ainda visivel por baixo, sumindo)
//   2) o lado que REVELA (a tela NOVA, ja pronta por baixo, aparecendo)
// Cada METADE dura ~kTransitionFadeSeconds (~0.4s) - juntas, ~0.7-0.8s (o "~20 frames
// em ~0,7s" do pedido). Pra o boot LER como UM UNICO arco continuo (nao dois arcos de
// 20 frames cada, que reiniciaria em frame_00 bem no meio da transicao - um "soluco"
// visual), os 20 frames sao DIVIDIDOS ao meio: a 1a metade (indices [0, N/2-1]) toca
// durante a metade que ESCURECE; a 2a metade (indices [N/2, N-1]) toca durante a
// metade que REVELA. O INDO pra batalha usa as 2 metades em ORDEM NORMAL (a Costura
// termina exatamente em "SYSTEM READY." quando a arena fica 100% visivel - "o sistema
// BOOTANDO conforme a arena aparece"); o VOLTANDO pra cidade usa as MESMAS 2 metades
// em ORDEM REVERSA (termina no frame_00 escuro/cursor quando a cidade reaparece - "o
// sistema desligando").
//
// OS 4 LEGS (nomeados pela METADE fisica + o SENTIDO da viagem, NAO por FadeDirection
// - um modulo decidido a proposito para NAO depender de core::anim::FadeDirection,
// zero acoplamento extra):
//   kToBattleDarkening   - cidade escurecendo, indo pra batalha.   Frames [0, N/2-1) ASCENDENTE.
//   kToBattleRevealing   - arena revelando,   indo pra batalha.    Frames [N/2, N-1] ASCENDENTE (termina no ULTIMO).
//   kFromBattleDarkening - arena escurecendo, voltando pra cidade. Frames [N-1, N/2] DESCENDENTE (espelha kToBattleRevealing).
//   kFromBattleRevealing - cidade revelando,  voltando pra cidade. Frames (N/2-1, 0] DESCENDENTE (espelha kToBattleDarkening, termina no PRIMEIRO).
//
// Cada leg recebe `t` = elapsed_seconds/duration_seconds da SUA PROPRIA metade fisica,
// clampado [0,1] (a MESMA fracao de progresso que fade_overlay_alpha ja calcula
// internamente antes de aplicar kOut/kIn - aqui SEMPRE ascendente no tempo real,
// independente do leg) - o CHAMADOR (app/) sabe qual leg cada um dos 4 pontos de
// desenho e (2 na cidade via FadeDirection recebido em SdlWindow::step_with_fade, 2 na
// batalha - cada loop de fade e textualmente SO um dos dois).
//
// QUEM CARREGA/DESENHA (draw_textured_rect por frame, sobre um retangulo solido de
// seguranca por baixo) fica em app/ (gus/app/boot_pixel_overlay.hpp) - aqui e so a
// matematica determinista.
//
// boot_pixel_idle_frame_index (M7-FB3, MENU-INICIAL-FUNDO): o MESMO asset de 20
// frames GANHA um 2o uso, DESACOPLADO da transicao de 4 pernas acima - fundo VIVO da
// TELA DE TITULO (gus/app/screens/title_menu_loop.cpp), que fica aberta por tempo
// INDEFINIDO (nao um `t`=[0,1] de UMA transicao com duracao fixa). Playtest do Gus
// Dragon: "menu inicial de jogo tem arte/animacao PROPRIA por tras, nao a tela de onde
// o jogador estava" - decisao do lider: o fundo do menu inicial passa a ser o MESMO
// monitor CRT do boot, so que agora precisa ficar "vivo" indefinidamente sem: (a)
// congelar no ultimo frame (leria como travado) nem (b) repetir os 20 frames inteiros
// em loop (pareceria o boot REINICIANDO - o MESMO "pareceu bug" que o glitch procedural
// original levou o veto). A solucao mais barata que ainda le como "vivo": cicla PRA
// FRENTE, devagar, so pelos ULTIMOS kBootPixelIdleWindowFrames indices (17,18,19 de 20)
// - exatamente o dump hexadecimal + "SYSTEM READY." + cursor do frame final (ver
// contact_sheet.png) - o CONTEUDO desses 3 frames ja e ruido/telemetria mudando
// (valores de registrador, nao linhas de log sendo apagadas), entao o ciclo NUNCA
// parece "voltar atras" ou "perder conteudo" - so parece o terminal seguindo vivo.

#ifndef GUS_CORE_ANIM_BOOT_PIXEL_SEQUENCE_HPP
#define GUS_CORE_ANIM_BOOT_PIXEL_SEQUENCE_HPP

namespace gus::core::anim {

// Quantidade de frames do asset pre-renderizado (resources/vfx/boot_pixel/frame_00.png
// .. frame_19.png). Fonte unica: qualquer mudanca na contagem de frames do asset edita
// SO esta constante (o loader em app/ itera 0..kBootPixelFrameCount-1).
inline constexpr int kBootPixelFrameCount = 20;

// As 4 "pernas" da transicao inteira - ver o comentario do topo do arquivo pro
// desenho completo (qual metade fisica, qual sentido, qual sub-faixa de indices).
enum class BootPixelLeg {
    kToBattleDarkening,
    kToBattleRevealing,
    kFromBattleDarkening,
    kFromBattleRevealing,
};

// Indice [0, frame_count-1] do frame a mostrar nesta perna `leg`, dado o progresso
// `t` (elapsed/duration DESTA metade fisica, [0,1] esperado - SEMPRE ascendente no
// tempo real) e `frame_count` (numero de frames disponiveis - normalmente
// kBootPixelFrameCount, parametrizado pra ser testavel sem depender da constante).
// Clampa t em [0,1] antes de mapear (defensivo) e arredonda pro inteiro mais proximo.
// GARANTIA DE COSTURA: boot_pixel_frame_index(kToBattleDarkening, 1.0, N) e
// boot_pixel_frame_index(kToBattleRevealing, 0.0, N) diferem em NO MAXIMO 1 (a
// transicao NUNCA reseta pra frame_00 no meio do arco) - mesma garantia espelhada pro
// par kFromBattleDarkening/kFromBattleRevealing. frame_count<=0 devolve 0 (sem UB nem
// divisao por zero - um "asset vazio" ainda tem uma resposta bem definida).
[[nodiscard]] int boot_pixel_frame_index(BootPixelLeg leg, float t,
                                          int frame_count) noexcept;

// Alpha [0,1] da camada SOLIDA de seguranca (ver gus/app/boot_pixel_overlay.hpp) nesta
// perna `leg`, dado o MESMO progresso `t` acima. Espelha exatamente o que
// core::anim::fade_overlay_alpha ja fazia pro fade liso original: pernas "Darkening"
// crescem 0->1 com t (a tela fica cada vez mais coberta); pernas "Revealing" decrescem
// 1->0 com t (a tela vai revelando). Clampa t em [0,1] antes de calcular.
[[nodiscard]] float boot_pixel_safety_alpha(BootPixelLeg leg, float t) noexcept;

// Quantos dos ULTIMOS frames do asset formam a "janela de repouso" (ver o comentario
// grande acima) - com kBootPixelFrameCount=20, a janela e [17,18,19]. Clampada por
// boot_pixel_idle_frame_index quando frame_count < este valor (asset hipotetico menor).
inline constexpr int kBootPixelIdleWindowFrames = 3;

// Segundos que CADA frame da janela de repouso fica visivel antes de avancar pro
// proximo - devagar o bastante pra nao virar slideshow ilegivel, rapido o bastante pra
// o olho perceber que mudou (mesmo espirito "log de terminal rolando devagar").
inline constexpr float kBootPixelIdleFrameSeconds = 0.6f;

// Indice [0, frame_count-1] do frame a mostrar no fundo VIVO da tela de titulo, dado
// `elapsed_seconds` (tempo corrido DESDE que a tela abriu - SEMPRE crescente, sem
// duracao fixa, ao contrario do `t`=[0,1] de boot_pixel_frame_index) e `frame_count`
// (normalmente kBootPixelFrameCount, parametrizado pra testabilidade). Cicla PRA
// FRENTE (round-robin, nunca recua) so pelos ULTIMOS min(kBootPixelIdleWindowFrames,
// frame_count) indices - NUNCA volta ao frame_00 (isso pareceria o boot reiniciando).
// elapsed_seconds negativo e tratado como 0 (defensivo). frame_count<=0 devolve 0 (sem
// UB nem divisao por zero - mesma garantia de boot_pixel_frame_index).
[[nodiscard]] int boot_pixel_idle_frame_index(float elapsed_seconds,
                                               int frame_count) noexcept;

}  // namespace gus::core::anim

#endif  // GUS_CORE_ANIM_BOOT_PIXEL_SEQUENCE_HPP
