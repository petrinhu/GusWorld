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
// Cross-ref: gus/app/screens/npc_dialogue_rml.hpp (RML/RCSS gerado);
//            gus/app/screens/npc_dialogue_overlay.hpp (logica pura de input/
//            selecao, ja testada - REUSADA aqui, so a apresentacao muda);
//            gus/app/screens/system_menu_loop.hpp (MESMA receita de contexto GL
//            owning + stage dir + reload-on-change);
//            gus/app/maestro.cpp (to_npc_dialogue, o chamador).

#ifndef GUS_APP_SCREENS_NPC_DIALOGUE_LOOP_GL_HPP
#define GUS_APP_SCREENS_NPC_DIALOGUE_LOOP_GL_HPP

#include <SDL3/SDL.h>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/sdl_window.hpp"
#include "gus/domain/dialogue/dialogue_runtime.hpp"

namespace gus::app::screens {

// Roda o dialogo (runtime JA POSICIONADO via runtime.enter(), feito pelo
// CHAMADOR) ate runtime.finished()==true OU o jogador fechar a JANELA. Cria seu
// PROPRIO contexto GL em `window` (destruido ao sair, mesmos atributos de
// run_system_menu_loop_owning_gl). O CHAMADOR e responsavel por
// city.release_renderer()/city.reacquire_renderer() POR FORA desta chamada (MESMO
// contrato de open_pause_from_city/run_battle_preview_embedded - "trocar
// escondido atras do preto" nao se aplica aqui, mas o principio de troca de
// backend na MESMA janela e identico). Devolve true SO se o jogador fechou a
// JANELA durante a conversa (MESMO contrato de run_npc_dialogue_loop/to_battle/
// open_pause_from_city - o chamador encerra o programa); false = conversa
// encerrada normalmente (@exit) - a cidade retoma de onde estava.
[[nodiscard]] bool run_npc_dialogue_loop_gl(
    SDL_Window* window, gus::app::SdlWindow& city,
    gus::domain::dialogue::DialogueRuntime& runtime,
    const gus::app::i18n::Translator& translator);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_NPC_DIALOGUE_LOOP_GL_HPP
