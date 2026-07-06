// gus/app/screens/npc_dialogue_overlay.hpp
//
// Logica de INTERACAO do overlay funcional simples de dialogo do NPC (M7-DIALOGO,
// NPC-MVP). A apresentacao visual FINA (RCSS terminal vs caixa quente) e o item
// paralelo DIALOGO-TERMINAL, que so entra quando a UI de verdade existir; aqui e so
// o suficiente pra PROVAR o ciclo (falar, escolher, flag muda, sair) com texto
// simples desenhado pelo MESMO Render2dSdl que a cidade ja usa (draw_filled_rect +
// draw_text via IRenderer, sem GL/glintfx - ver SdlWindow::render_dialogue_overlay_
// frame e npc_dialogue_loop.hpp).
//
// TUDO POCO/testavel headless (SEM SDL) - a UNICA parte que toca SDL de fato
// (poll de evento + o desenho em si) fica em npc_dialogue_loop.hpp/.cpp e
// sdl_window.cpp (irredutivel, mesmo racional de system_menu_loop.cpp).

#ifndef GUS_APP_SCREENS_NPC_DIALOGUE_OVERLAY_HPP
#define GUS_APP_SCREENS_NPC_DIALOGUE_OVERLAY_HPP

#include <string>
#include <vector>

#include "gus/app/i18n/translator.hpp"
#include "gus/domain/dialogue/dialogue_graph.hpp"
#include "gus/domain/dialogue/dialogue_runtime.hpp"

namespace gus::app::screens {

// Move a selecao por `delta` (+1 = baixo, -1 = cima) dentre `option_count` opcoes,
// com WRAP (desce do ultimo item volta pro 0; sobe do 0 vai pro ultimo).
// option_count<=0 e defensivo (devolve 0, nunca UB/divisao por zero).
[[nodiscard]] constexpr int npc_dialogue_move_selection(int selected, int delta,
                                                         int option_count) noexcept {
    if (option_count <= 0) return 0;
    int idx = (selected + delta) % option_count;
    if (idx < 0) idx += option_count;
    return idx;
}

// Monta as linhas de texto do frame ATUAL: "<Speaker>:" (resolvido via
// "ACTOR_<ID EM MAIUSCULO>" com fallback pro proprio id em minusculo se a chave
// faltar - MESMA convencao de ACTOR_GUS/ACTOR_CAUA em pt_br.md §8), a fala
// (tr(node.text_key)), e - se for no de ESCOLHA - 1 linha por opcao com "> " a
// prefixar a SELECIONADA ("  " as demais). No LINEAR (sem opcoes) fecha com
// tr("DIALOGUE_CONTINUE") (chave ja existente no catalogo, reusada - nao inventa
// uma nova so pro "continuar").
[[nodiscard]] std::vector<std::string> npc_dialogue_overlay_lines(
    const gus::domain::dialogue::DialogueNode& node,
    const gus::app::i18n::Translator& translator, int selected_option);

// Acao de input do jogador durante o dialogo (traducao de tecla -> intencao,
// FEITA por quem chama - o loop SDL real ou um teste headless).
enum class NpcDialogueInputAction {
    None,
    MoveUp,
    MoveDown,
    Confirm,
};

// Aplica UMA acao de input ao par (runtime, selected_option): MoveUp/MoveDown so
// mudam a selecao (no-op se o no atual for LINEAR, sem opcoes); Confirm AVANCA (no
// LINEAR, via DialogueRuntime::advance()) OU ESCOLHE a opcao `selected_option` (no
// de ESCOLHA, via DialogueRuntime::choose()) - nunca chama advance()/choose() no
// tipo de no errado (evita o std::logic_error do runtime). Devolve o NOVO
// selected_option (SEMPRE 0 apos uma transicao de no - a selecao nao "vaza" de um
// no de escolha pro seguinte). Nao-op se runtime.finished() (a conversa ja acabou;
// o CHAMADOR e quem decide fechar o loop olhando finished()).
int apply_npc_dialogue_input(gus::domain::dialogue::DialogueRuntime& runtime,
                              NpcDialogueInputAction action, int selected_option);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_NPC_DIALOGUE_OVERLAY_HPP
