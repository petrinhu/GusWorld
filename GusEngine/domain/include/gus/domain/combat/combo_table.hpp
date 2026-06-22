// gus/domain/combat/combo_table.hpp
//
// Tabela de receitas de combo (formato canonico secao 10). Resolvedor deterministico,
// portado de engine/foundation/turn_combat/ComboTable.cs. POCO puro, ZERO Qt.
//
// A pipeline de ate 3 slots do Tavus-Drive casa uma assinatura ORDENADA de cartas/
// modificadores contra esta tabela e dispara um combo deterministico nomeado. A tabela
// completa (~200 combos) e trabalho futuro; aqui ha 2 receitas mockup pro vertical slice.
//
// Casamento por assinatura EXATA: cada slot compara Kind + Ref, na mesma ordem e mesmo
// tamanho. Sem RNG (Pillar 2: combos sao receitas, nao sorteio).
//
// O C# modela como `static class`. Aqui adotamos um namespace ComboTable com funcoes
// livres (equivalente C++ idiomatico): ComboTable::recipes() / ComboTable::match(...)
// espelham ComboTable.Recipes / ComboTable.Match(...) do C#.
//
// ComboRecipe? Match(...) (nullable) -> std::optional<ComboRecipe>.
//
// Cross-ref: engine/foundation/turn_combat/ComboTable.cs; docs/design/mecanicas/combat.md secao 10.

#ifndef GUS_DOMAIN_COMBAT_COMBO_TABLE_HPP
#define GUS_DOMAIN_COMBAT_COMBO_TABLE_HPP

#include <optional>
#include <vector>

#include "gus/domain/combat/combat_records.hpp"

namespace gus::domain::combat::ComboTable {

// Receitas curadas. 2 mockups no vertical slice; tabela completa (~200) e jogo posterior.
[[nodiscard]] const std::vector<ComboRecipe>& recipes();

// Casa uma pipeline de ate 3 slots contra a tabela por assinatura exata (Kind + Ref de
// cada slot, mesma ordem e tamanho). Retorna a receita ou nullopt.
[[nodiscard]] std::optional<ComboRecipe> match(const std::vector<PipelineSlot>& pipeline);

}  // namespace gus::domain::combat::ComboTable

#endif  // GUS_DOMAIN_COMBAT_COMBO_TABLE_HPP
