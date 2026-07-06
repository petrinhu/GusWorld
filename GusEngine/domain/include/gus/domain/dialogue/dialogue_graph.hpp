// gus/domain/dialogue/dialogue_graph.hpp
//
// Grafo de dialogo POCO (ADR-014). Namespace gus::domain::dialogue. Camada domain/
// PURA: ZERO Qt, ZERO SDL, ZERO I/O (mesma disciplina de domain/map, domain/save).
// Este e o DADO materializado pelo parser (dialogue_text.hpp); o runtime
// (dialogue_runtime.hpp) o percorre.
//
// MODELO:
//   - DialogueNode e LINEAR (options vazio, usa next_node_id) OU de ESCOLHA
//     (options.size() >= 2). Convergencia = 2+ options mirando o MESMO
//     next_node_id -- sem infra dedicada, e so um dado repetido (ver
//     dialogue-tree-npc-intro.md §5, o caso do Bertoldo).
//   - kExitNodeId ("@exit") e o sentinela de saida: NAO existe como chave em
//     DialogueGraph::nodes; e so um valor VALIDO para next_node_id/option.
//   - DialogueRegister (Terminal x Warm, combat-flavor.md §5): default por grafo
//     (DialogueGraph::default_register) + override opcional por no
//     (DialogueNode::register_override). O runtime repassa so a ETIQUETA efetiva
//     (register_override.value_or(default_register)); a pintura fica no app/
//     (item DIALOGO-TERMINAL, paralelo a este).
//   - FlagCondition existe na estrutura para leitura futura (gates de revisita,
//     ADR-014 dec.3, n7_revisit_hub); NAO consumida pelo DialogueRuntime nesta
//     onda -- so testada por unidade (ver dialogue_graph_test.cpp).
//
// VALIDACAO (validate(), fail-fast, mesma disciplina de TileMap::validate() /
// SaveData::validate()):
//   - entry_node_id nao-vazio e existe em nodes;
//   - toda chave do map == node.id correspondente;
//   - no de escolha (options nao-vazio) tem >=2 options (nunca exatamente 1);
//   - no linear (options vazio) tem next_node_id nao-vazio (sem beco sem saida);
//   - todo next_node_id/option.next_node_id aponta para um no existente OU
//     kExitNodeId;
//   - sem no orfao: todo no e alcancavel a partir do entry_node_id (BFS).
//
// Cross-ref: ADR-014 (docs/tech/adr/ADR-014-dialogue-runtime-poco.md),
//            gus/domain/dialogue/dialogue_text.hpp (parser do formato-texto),
//            gus/domain/dialogue/dialogue_runtime.hpp (motor),
//            docs/design/mecanicas/combat-flavor.md §5 (registro terminal x quente),
//            docs/design/narrativa/dialogue-tree-npc-intro.md (blueprint Bertoldo).

#ifndef GUS_DOMAIN_DIALOGUE_DIALOGUE_GRAPH_HPP
#define GUS_DOMAIN_DIALOGUE_DIALOGUE_GRAPH_HPP

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace gus::domain::dialogue {

// Sentinela de saida do grafo: valor VALIDO de next_node_id/option.next_node_id
// que nunca e uma chave real em DialogueGraph::nodes.
inline constexpr std::string_view kExitNodeId = "@exit";

// true sse id (next_node_id de um no ou opcao) representa a saida do grafo.
[[nodiscard]] inline bool is_exit_node_id(std::string_view id) noexcept {
    return id == kExitNodeId;
}

// Registro de apresentacao da fala (combat-flavor.md §5): Terminal = software/
// maquina fala (tela de terminal); Warm = o coracao fala (caixa quente + retrato).
enum class DialogueRegister { Terminal, Warm };

// Equalidade sobre uma flag (bool). Sem expressoes/operadores: so ==, suficiente
// para o gate first-visit/revisit. Pronta na estrutura; exercitada so por teste de
// unidade no M7 (o NPC-MVP nao ramifica por leitura).
struct FlagCondition {
    std::string flag_key;
    bool expected_value = true;

    [[nodiscard]] bool operator==(const FlagCondition&) const = default;
};

// Efeito colateral de um no/opcao sobre o mapa de flags externo (SaveData::flags
// ou um std::map de teste -- ver dialogue_runtime.hpp).
struct FlagEffect {
    std::string flag_key;
    bool value = true;

    [[nodiscard]] bool operator==(const FlagEffect&) const = default;
};

// Opcao de escolha do jogador. label_key = chave i18n do texto do botao.
struct DialogueOption {
    std::string label_key;
    std::string next_node_id;
    std::optional<FlagEffect> effect;  // ex: npc_intro.choice_curioso = true

    [[nodiscard]] bool operator==(const DialogueOption&) const = default;
};

// No de dialogo. Linear (options vazio -> segue next_node_id) OU no de escolha
// (options.size() >= 2).
struct DialogueNode {
    std::string id;
    std::string speaker_id;                              // "bertoldo" -> retrato/prompt
    std::string text_key;                                // chave i18n da fala
    std::optional<DialogueRegister> register_override;   // vazio = herda default do grafo
    std::optional<FlagEffect> on_enter;                  // ex: npc_intro.met ao entrar
    std::string next_node_id;                            // usado quando options vazio
    std::vector<DialogueOption> options;                 // >=2 = no de escolha

    [[nodiscard]] bool operator==(const DialogueNode&) const = default;
};

// Grafo completo de um dialogo (1 NPC/1 conversa). O parser do formato-texto
// materializa isto; validate() e fail-fast.
struct DialogueGraph {
    std::string dialogue_id;
    DialogueRegister default_register = DialogueRegister::Warm;
    std::string entry_node_id;
    std::map<std::string, DialogueNode> nodes;  // std::map = ordem deterministica

    [[nodiscard]] bool operator==(const DialogueGraph&) const = default;

    // Valida invariantes (fail-fast). Lanca std::invalid_argument na 1a violacao
    // encontrada (mesma disciplina de TileMap/SaveData).
    void validate() const;
};

}  // namespace gus::domain::dialogue

#endif  // GUS_DOMAIN_DIALOGUE_DIALOGUE_GRAPH_HPP
