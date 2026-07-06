// gus/domain/dialogue/dialogue_runtime.hpp
//
// Runtime PURO de dialogo (ADR-014). Percorre um DialogueGraph JA VALIDADO sobre
// uma REFERENCIA EXTERNA a um mapa de flags (o SaveData::flags do jogo real, ou um
// std::map de teste) -- por design, domain/dialogue NAO depende de domain/save
// (subdominios independentes; o app/maestro passa save_data.flags por referencia
// na integracao real). POCO puro, ZERO Qt/SDL/UI: o app/ le current()/
// current_register() e desenha a caixa-quente OU o terminal sem que o motor saiba
// qual (a spec visual fina e o item paralelo DIALOGO-TERMINAL).
//
// CONTRATO:
//   enter()    posiciona no entry_node_id do grafo; aplica o on_enter do no de
//              entrada (se houver).
//   current()  no atual. Lanca std::logic_error se chamado antes de enter() ou
//              depois que finished()==true.
//   current_register()  register_override.value_or(graph.default_register) do no
//              atual.
//   choose(i)  SO valido em no de ESCOLHA (options.size()>=2): aplica o FlagEffect
//              da opcao i (se houver) e segue para options[i].next_node_id.
//              std::out_of_range se i estiver fora de alcance; std::logic_error se
//              o no atual for linear (use advance()).
//   advance()  SO valido em no LINEAR (options vazio): segue next_node_id.
//              std::logic_error se o no atual for de escolha (use choose()).
//   finished() true quando a ultima transicao (via choose()/advance()) alcancou
//              kExitNodeId ("@exit"). current()/choose()/advance() lancam depois.
//
// on_enter dispara toda vez que o runtime ENTRA num no -- via enter() (no de
// entrada) OU via choose()/advance() (no seguinte) -- nao so na 1a visita do grafo
// inteiro. E assim que npc_intro.met (on_enter de n0_greet) e setado ao entrar na
// conversa, e um FlagEffect de opcao (ex. npc_intro.choice_curioso) e aplicado ao
// escolher, mesmo que 2+ opcoes reconvirjam para o MESMO no seguinte (cada uma
// carrega seu proprio efeito, aplicado antes da transicao -- cenario Bertoldo,
// dialogue-tree-npc-intro.md §5).
//
// Cross-ref: ADR-014, dialogue_graph.hpp.

#ifndef GUS_DOMAIN_DIALOGUE_DIALOGUE_RUNTIME_HPP
#define GUS_DOMAIN_DIALOGUE_DIALOGUE_RUNTIME_HPP

#include <cstddef>
#include <map>
#include <string>

#include "gus/domain/dialogue/dialogue_graph.hpp"

namespace gus::domain::dialogue {

class DialogueRuntime {
public:
    // graph: referencia externa, deve sobreviver ao runtime (dono e o chamador,
    // tipicamente um catalogo de dialogos carregado 1x). flags: referencia externa
    // ao mapa de flags mutavel (SaveData::flags real ou std::map de teste).
    DialogueRuntime(const DialogueGraph& graph, std::map<std::string, bool>& flags);

    // Posiciona no entry_node_id do grafo e aplica o on_enter do no de entrada.
    // Idempotente-reentrante: pode ser chamado de novo para reiniciar a conversa.
    void enter();

    [[nodiscard]] const DialogueNode& current() const;
    [[nodiscard]] DialogueRegister current_register() const;

    void choose(std::size_t option_index);
    void advance();

    [[nodiscard]] bool finished() const noexcept;

private:
    void apply_effect(const std::optional<FlagEffect>& effect);
    void goto_next(const std::string& next_node_id);

    const DialogueGraph& graph_;
    std::map<std::string, bool>& flags_;
    std::string current_node_id_;
    bool entered_ = false;
    bool finished_ = false;
};

}  // namespace gus::domain::dialogue

#endif  // GUS_DOMAIN_DIALOGUE_DIALOGUE_RUNTIME_HPP
