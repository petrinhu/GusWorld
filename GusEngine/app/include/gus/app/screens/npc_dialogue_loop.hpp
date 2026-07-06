// gus/app/screens/npc_dialogue_loop.hpp
//
// Loop INTERATIVO do dialogo do NPC (M7-DIALOGO, NPC-MVP). Ao contrario da
// batalha/menu de pausa (que trocam pra um contexto GL/glintfx PROPRIO), o dialogo
// fica no MESMO SDL_Renderer da cidade (Render2dSdl) - "overlay funcional simples"
// (ver ADR-014 + TODO.md M7-DIALOGO): nenhuma troca de backend e necessaria pra
// provar o ciclo. Poll de SDL_Event PROPRIO (MESMO padrao independente de SdlInput
// que system_menu_loop.cpp ja usa pro menu de pausa - navegacao de dialogo/UI nao e
// "movimento do jogador", nao deve mexer no InputMapper/mapper_ da cidade):
//   Up/Down (setas ou W/S)  -> move a selecao de opcao (com wrap)
//   Enter/Espaco            -> avanca (no linear) ou escolhe (no de escolha)
// Desenha via SdlWindow::render_dialogue_overlay_frame (texto simples sobre a
// cidade PARADA); a apresentacao visual fina (RCSS terminal/caixa-quente) e o item
// paralelo DIALOGO-TERMINAL.
//
// Contrato de retorno (MESMO de Maestro::to_battle()/open_pause_from_city()): true
// = o jogador fechou a JANELA durante o dialogo (o chamador encerra o programa);
// false = a conversa terminou normalmente (@exit alcancado), a cidade retoma de
// onde estava (nunca foi destruida/pausada de verdade - so nao recebeu step_fixed).

#ifndef GUS_APP_SCREENS_NPC_DIALOGUE_LOOP_HPP
#define GUS_APP_SCREENS_NPC_DIALOGUE_LOOP_HPP

#include "gus/app/i18n/translator.hpp"
#include "gus/app/sdl_window.hpp"
#include "gus/domain/dialogue/dialogue_runtime.hpp"

namespace gus::app::screens {

[[nodiscard]] bool run_npc_dialogue_loop(
    gus::app::SdlWindow& city, gus::domain::dialogue::DialogueRuntime& runtime,
    const gus::app::i18n::Translator& translator);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_NPC_DIALOGUE_LOOP_HPP
