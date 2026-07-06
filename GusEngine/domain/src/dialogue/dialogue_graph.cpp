// gus/domain/src/dialogue/dialogue_graph.cpp
//
// Implementacao de DialogueGraph::validate(). Ver o header para o contrato de
// invariantes. POCO puro, ZERO Qt/SDL/I-O.

#include "gus/domain/dialogue/dialogue_graph.hpp"

#include <stdexcept>
#include <unordered_set>
#include <vector>

namespace gus::domain::dialogue {

void DialogueGraph::validate() const {
    if (entry_node_id.empty())
        throw std::invalid_argument("DialogueGraph: entry_node_id vazio.");
    if (nodes.find(entry_node_id) == nodes.end())
        throw std::invalid_argument("DialogueGraph: entry_node_id '" + entry_node_id +
                                    "' nao existe em nodes.");

    for (const auto& [id, node] : nodes) {
        if (id != node.id)
            throw std::invalid_argument("DialogueGraph: chave '" + id +
                                        "' diverge de node.id '" + node.id + "'.");

        if (node.options.size() == 1)
            throw std::invalid_argument("DialogueGraph: no '" + id +
                                        "' tem exatamente 1 opcao (no de escolha exige"
                                        " >=2; no linear usa next_node_id, sem options).");

        if (node.options.empty()) {
            // No LINEAR: precisa de destino.
            if (node.next_node_id.empty())
                throw std::invalid_argument("DialogueGraph: no linear '" + id +
                                            "' sem next_node_id (beco sem saida).");
            if (!is_exit_node_id(node.next_node_id) &&
                nodes.find(node.next_node_id) == nodes.end())
                throw std::invalid_argument("DialogueGraph: no '" + id +
                                            "' aponta para next_node_id '" +
                                            node.next_node_id + "' inexistente.");
        } else {
            // No de ESCOLHA: cada opcao precisa de destino.
            for (const auto& opt : node.options) {
                if (opt.next_node_id.empty())
                    throw std::invalid_argument("DialogueGraph: opcao '" + opt.label_key +
                                                "' do no '" + id +
                                                "' sem next_node_id.");
                if (!is_exit_node_id(opt.next_node_id) &&
                    nodes.find(opt.next_node_id) == nodes.end())
                    throw std::invalid_argument("DialogueGraph: opcao '" + opt.label_key +
                                                "' do no '" + id +
                                                "' aponta para next_node_id '" +
                                                opt.next_node_id + "' inexistente.");
            }
        }
    }

    // Sem no orfao: todo no e alcancavel a partir do entry_node_id (BFS/DFS
    // iterativo; kExitNodeId nao entra na pilha, e um destino terminal).
    std::unordered_set<std::string> reachable;
    std::vector<std::string> stack{entry_node_id};
    reachable.insert(entry_node_id);
    while (!stack.empty()) {
        const std::string cur = std::move(stack.back());
        stack.pop_back();
        const auto it = nodes.find(cur);
        if (it == nodes.end()) continue;  // ja validado acima; defesa em profundidade
        const DialogueNode& node = it->second;

        const auto visit = [&](const std::string& next) {
            if (is_exit_node_id(next)) return;
            if (reachable.insert(next).second) stack.push_back(next);
        };
        if (node.options.empty()) {
            visit(node.next_node_id);
        } else {
            for (const auto& opt : node.options) visit(opt.next_node_id);
        }
    }
    for (const auto& [id, node] : nodes) {
        (void)node;
        if (reachable.find(id) == reachable.end())
            throw std::invalid_argument("DialogueGraph: no orfao '" + id +
                                        "' inalcancavel a partir do entry_node_id.");
    }
}

}  // namespace gus::domain::dialogue
