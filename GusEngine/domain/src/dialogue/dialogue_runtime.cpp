// gus/domain/src/dialogue/dialogue_runtime.cpp
//
// Implementacao do DialogueRuntime. Ver o header para o contrato. POCO puro,
// ZERO Qt/SDL/I-O.

#include "gus/domain/dialogue/dialogue_runtime.hpp"

#include <stdexcept>

namespace gus::domain::dialogue {

DialogueRuntime::DialogueRuntime(const DialogueGraph& graph,
                                 std::map<std::string, bool>& flags)
    : graph_(graph), flags_(flags) {}

void DialogueRuntime::enter() {
    current_node_id_ = graph_.entry_node_id;
    entered_ = true;
    finished_ = false;
    apply_effect(current().on_enter);
}

const DialogueNode& DialogueRuntime::current() const {
    if (!entered_)
        throw std::logic_error("DialogueRuntime::current(): chamado antes de enter().");
    if (finished_)
        throw std::logic_error(
            "DialogueRuntime::current(): dialogo ja terminou (finished()==true).");
    const auto it = graph_.nodes.find(current_node_id_);
    if (it == graph_.nodes.end())
        throw std::logic_error("DialogueRuntime::current(): no '" + current_node_id_ +
                               "' nao existe no grafo (grafo invalido?).");
    return it->second;
}

DialogueRegister DialogueRuntime::current_register() const {
    const DialogueNode& node = current();
    return node.register_override.value_or(graph_.default_register);
}

void DialogueRuntime::choose(std::size_t option_index) {
    const DialogueNode& node = current();
    if (node.options.empty())
        throw std::logic_error("DialogueRuntime::choose(): no '" + node.id +
                               "' e linear (sem opcoes); use advance().");
    if (option_index >= node.options.size())
        throw std::out_of_range(
            "DialogueRuntime::choose(): indice " + std::to_string(option_index) +
            " fora de alcance (no '" + node.id + "' tem " +
            std::to_string(node.options.size()) + " opcoes).");
    const DialogueOption& opt = node.options[option_index];
    apply_effect(opt.effect);
    goto_next(opt.next_node_id);
}

void DialogueRuntime::advance() {
    const DialogueNode& node = current();
    if (!node.options.empty())
        throw std::logic_error("DialogueRuntime::advance(): no '" + node.id +
                               "' e de escolha; use choose().");
    goto_next(node.next_node_id);
}

bool DialogueRuntime::finished() const noexcept { return finished_; }

void DialogueRuntime::apply_effect(const std::optional<FlagEffect>& effect) {
    if (effect.has_value()) flags_[effect->flag_key] = effect->value;
}

void DialogueRuntime::goto_next(const std::string& next_node_id) {
    if (is_exit_node_id(next_node_id)) {
        finished_ = true;
        current_node_id_.clear();
        return;
    }
    current_node_id_ = next_node_id;
    apply_effect(current().on_enter);  // on_enter dispara toda entrada, nao so a 1a
}

}  // namespace gus::domain::dialogue
