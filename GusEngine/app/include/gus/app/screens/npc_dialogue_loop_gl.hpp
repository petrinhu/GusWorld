// gus/app/screens/npc_dialogue_loop_gl.hpp
//
// LOOP INTERATIVO REAL do dialogo do NPC via glintfx::UiLayer (a caixa "quente" -
// moldura de latao + retrato + nome + fala + escolhas, ver npc_dialogue_rml.hpp),
// substituindo o overlay funcional simples de texto (npc_dialogue_loop.hpp,
// aposentado pela Maestro - deixado no lugar como historico/defesa-em-profundidade,
// nao mais chamado no caminho real).
//
// TECNICA (MESMA de Maestro::open_pause_from_city, ja CONFIRMADA SEGURA ao vivo
// pelo lider - o menu de pausa na cidade funciona sem crash): contexto GL PROPRIO/
// DEDICADO, criado SO ao entrar no dialogo e destruido ao sair - igual a
// run_system_menu_loop_owning_gl (system_menu_loop.cpp), MESMOS atributos GL
// (profile core 3.3, double-buffer, stencil 8). Isto e SEGURO aqui porque a CIDADE
// roda SDL_Renderer puro (release_renderer() feito pelo CHAMADOR ANTES de entrar -
// ver Maestro::to_npc_dialogue) - nenhum glintfx::UiLayer fica vivo simultaneamente.
// Diferente da BATALHA (que JA tem um UiLayer vivo pro cockpit - abrir um 2o la
// causou o crash real que foi revertido, ver historico git) - este loop NUNCA e
// chamado de dentro da batalha.
//
// FUNDO: deliberadamente ABSTRATO/ESTATICO (MESMA decisao ja tomada pro menu de
// pausa, ver system_menu_loop.hpp - reusa Render2dGl3::begin_frame, clear + vinheta
// radial). O mock aprovado (docs/design/mockups/05-...) mostra um "city-backdrop"
// borrado so como ILUSTRACAO da composicao final; capturar/borrar o frame real da
// cidade exigiria uma tecnica NOVA (render-to-texture) fora do escopo desta
// correcao - a caixa quente em si (o pedido central do lider) fica identica ao
// mock; o fundo por-tras segue o MESMO padrao ja aprovado do menu de pausa.
//
// FIX BUG (playtest ao vivo do lider, M7-DIALOGO NPC-MVP: "Gus anda sozinho apos
// fechar o dialogo"): chama SdlWindow::clear_input() ao ENTRAR e ao SAIR (ver o
// comentario completo la) - este loop faz o proprio SDL_PollEvent (independente de
// SdlInput::pump_events), entao uma tecla de movimento cujo KEY_UP acontecesse
// DURANTE a conversa nunca chegaria no estado da cidade sem isto.
//
// BOTAO "Continuar" + HOVER/CLIQUE/SOM (pedido do lider, pos-DIALOGO-TERMINAL):
// no de ESCOLHA nao muda (navegacao so por teclado UP/DOWN/Enter, MESMO
// comportamento de sempre - fora do escopo deste pedido); o no LINEAR (sem
// opcoes) ganha um BOTAO "Continuar" DE VERDADE (ver npc_dialogue_rml.hpp:
// id="npcdlg-continue-btn", classe "warm-continue-btn"). MESMA receita
// JA PROVADA do menu de sistema (system_menu_loop.cpp):
//   - HOVER: `:hover` NATIVO do glintfx (RCSS) alimentado por
//     ui.process_event(MouseMove) em TODO SDL_EVENT_MOUSE_MOTION; o SOM de
//     hover usa a fatia GENERICA gus/app/screens/ui_hover.hpp (UiHoverBox +
//     ui_hover_index + ui_hover_entered_new_item - MESMA dupla que o menu/
//     cockpit ja usam, so passando 1 UNICA caixa em vez de N) - edge-detect
//     PURO, so dispara audio.play_sfx(hover) quando o hover ENTRA no botao.
//   - CLIQUE: SDL_EVENT_MOUSE_BUTTON_DOWN com hit-test em get_element_box("
//     npcdlg-continue-btn") (MESMO padrao de hit_test em system_menu_loop.cpp)
//     E Enter/Espaco de TECLADO (ja existente) convergem no MESMO choke-point
//     (flash_pressed() no .cpp): toca kMenuClickSfxFile + renderiza alguns
//     frames com ".pressed" ANTES de avancar o dialogo de fato - 1 unico lugar
//     pros dois canais de entrada, sem duplicar logica (MESMA razao de ser de
//     flash_pressed() em system_menu_loop.cpp).
// `audio` (AudioEngine, NAO-dono - a Maestro e dona, ver maestro.hpp): reusa
// kMenuHoverSfxFile/kMenuClickSfxFile (asset_paths.hpp) - MESMOS 2 arquivos de
// SFX que o menu de sistema/cockpit ja usam, zero asset novo.
//
// Cross-ref: gus/app/screens/npc_dialogue_rml.hpp (RML/RCSS gerado, incl. o
//            botao "Continuar");
//            gus/app/screens/npc_dialogue_overlay.hpp (logica pura de input/
//            selecao, ja testada - REUSADA aqui, so a apresentacao muda);
//            gus/app/screens/ui_hover.hpp (fatia PURA/generica de edge-detect
//            de hover, REUSADA aqui - MESMA que o menu de sistema/cockpit);
//            gus/app/screens/system_menu_loop.hpp (MESMA receita de contexto GL
//            owning + stage dir + reload-on-change + hover/clique/flash_pressed);
//            gus/app/maestro.cpp (to_npc_dialogue, o chamador).

#ifndef GUS_APP_SCREENS_NPC_DIALOGUE_LOOP_GL_HPP
#define GUS_APP_SCREENS_NPC_DIALOGUE_LOOP_GL_HPP

#include <SDL3/SDL.h>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/sdl_window.hpp"
#include "gus/domain/dialogue/dialogue_runtime.hpp"
#include "gus/platform/audio/audio_engine.hpp"

namespace gus::app::screens {

// Roda o dialogo (runtime JA POSICIONADO via runtime.enter(), feito pelo
// CHAMADOR) ate runtime.finished()==true OU o jogador fechar a JANELA. Cria seu
// PROPRIO contexto GL em `window` (destruido ao sair, mesmos atributos de
// run_system_menu_loop_owning_gl). O CHAMADOR e responsavel por
// city.release_renderer()/city.reacquire_renderer() POR FORA desta chamada (MESMO
// contrato de open_pause_from_city/run_battle_preview_embedded - "trocar
// escondido atras do preto" nao se aplica aqui, mas o principio de troca de
// backend na MESMA janela e identico). `audio` (ver header acima) toca o SOM de
// hover/clique do botao "Continuar" - NAO-dono, a Maestro mantem viva. Devolve
// true SO se o jogador fechou a JANELA durante a conversa (MESMO contrato de
// run_npc_dialogue_loop/to_battle/open_pause_from_city - o chamador encerra o
// programa); false = conversa encerrada normalmente (@exit) - a cidade retoma
// de onde estava.
[[nodiscard]] bool run_npc_dialogue_loop_gl(
    SDL_Window* window, gus::app::SdlWindow& city,
    gus::domain::dialogue::DialogueRuntime& runtime,
    const gus::app::i18n::Translator& translator,
    gus::platform::audio::AudioEngine& audio);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_NPC_DIALOGUE_LOOP_GL_HPP
