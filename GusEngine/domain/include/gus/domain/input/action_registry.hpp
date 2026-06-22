// gus/domain/input/action_registry.hpp
//
// Registry canonico de actions GusWorld, portado de
// engine/foundation/input_remap/ActionRegistry.cs (pos-ADR-002 + F2-E.7).
// Lista immutable de 37 actions canonicas, cada uma com categoria e label i18n.
//
// POCO puro, ZERO Qt (invariante de domain/, engine-design.md secao 2). E o lado
// canonico/estatico do mapa logico de input (o "mapa acao->tecla e puro";
// engine-design.md secao 3); o backend de evento Qt (platform/input/) NAO faz
// parte deste porte (M2, metade de logica pura).
//
// MAPEAMENTO C# -> C++:
//   enum ActionCategory                 -> enum class ActionCategory
//   sealed record ActionDefinition      -> struct + operator== = default
//   static class ActionRegistry         -> classe so com membros static (sem estado)
//   IReadOnlyList<ActionDefinition>     -> const std::vector<ActionDefinition>& (singleton)
//   ActionDefinition? GetByName(...)    -> const ActionDefinition* (nullptr = nao achou)
//   IEnumerable<...> GetByCategory(...) -> std::vector<const ActionDefinition*> (ponteiros
//                                          para os elementos do registry, sem copia)
//
// Cross-ref: engine/foundation/input_remap/ActionRegistry.cs;
//            docs/tech/engine-modules.md secao 2.4; engine-design.md secao 2/3.

#ifndef GUS_DOMAIN_INPUT_ACTION_REGISTRY_HPP
#define GUS_DOMAIN_INPUT_ACTION_REGISTRY_HPP

#include <array>
#include <string>
#include <string_view>
#include <vector>

namespace gus::domain::input {

// Categoria de action (pra UI agrupamento). Espelha ActionCategory do C# na
// MESMA ordem de declaracao (valores subjacentes 0..7).
enum class ActionCategory {
    Movement,
    Camera,
    Interact,
    Menu,
    Combat,
    Dialogue,
    Inventory,
    Diary,
};

// Todas as categorias, na ordem de declaracao. Espelha Enum.GetValues<ActionCategory>()
// do C# (usado pela spec para somar as actions por categoria).
[[nodiscard]] inline constexpr std::array<ActionCategory, 8> all_action_categories() {
    return {
        ActionCategory::Movement,  ActionCategory::Camera,    ActionCategory::Interact,
        ActionCategory::Menu,      ActionCategory::Combat,    ActionCategory::Dialogue,
        ActionCategory::Inventory, ActionCategory::Diary,
    };
}

// Definicao canonica de uma action (nome + categoria + label i18n key). Espelha
// o record ActionDefinition do C# (igualdade por valor).
struct ActionDefinition {
    std::string action_name;
    ActionCategory category = ActionCategory::Movement;
    std::string label_i18n_key;

    [[nodiscard]] bool operator==(const ActionDefinition&) const = default;
};

// Registry canonico de todas as actions GusWorld. Sem estado: so membros static
// sobre a lista imutavel. Espelha a static class ActionRegistry do C#.
//
// A lista canonica NAO se modifica em runtime. Cada nova action MUST ser
// adicionada na fonte (action_registry.cpp) + ter default binding no backend.
class ActionRegistry {
public:
    ActionRegistry() = delete;  // so static; nao instanciavel (espelha static class C#).

    // Lista canonica completa, na ordem de declaracao. Referencia para um
    // singleton imutavel (sem copia). Espelha ActionRegistry.Actions.
    [[nodiscard]] static const std::vector<ActionDefinition>& actions();

    // Total de actions canonicas (debug helper). Espelha ActionRegistry.Count.
    [[nodiscard]] static int count();

    // Lookup de action por nome. Devolve nullptr se nao existe (espelha o
    // ActionDefinition? do C#). O ponteiro aponta para o elemento do registry
    // imutavel (valido enquanto o processo viver).
    [[nodiscard]] static const ActionDefinition* get_by_name(std::string_view action_name);

    // Actions filtradas por categoria, na ordem do registry. Devolve ponteiros
    // para os elementos do registry (sem copia). Espelha GetByCategory.
    [[nodiscard]] static std::vector<const ActionDefinition*> get_by_category(
        ActionCategory category);
};

}  // namespace gus::domain::input

#endif  // GUS_DOMAIN_INPUT_ACTION_REGISTRY_HPP
