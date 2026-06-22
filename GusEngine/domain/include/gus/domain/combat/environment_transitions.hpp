// gus/domain/combat/environment_transitions.hpp
//
// Tabela FECHADA e DETERMINISTICA de mutabilidade de ambiente (secao 18.6), portada de
// engine/foundation/turn_combat/EnvironmentTransitions.cs. Sem RNG. As transicoes so
// avancam/transformam de modo legivel e NUNCA pulam estados intermediarios. POCO puro,
// ZERO Qt (invariante de domain/, engine-design.md secao 2).
//
// No slice, as cartas/inimigos que disparam transicoes sao fase posterior (secao 18.10);
// esta tabela ja sao os DADOS preparados. O gatilho e modelado como (familia forte OU
// evento de ambiente).
//
// O C# modela como `static class` + `enum EnvironmentTrigger`. Aqui o enum fica no
// namespace do combate e a tabela/Resolve num namespace EnvironmentTransitions de funcoes
// livres. EnvironmentId? (nullable) -> std::optional<EnvironmentId>.
//
// Cross-ref: engine/foundation/turn_combat/EnvironmentTransitions.cs;
//            engine/tests/turn_combat/environments/EnvironmentTransitionsTests.cs;
//            docs/design/mecanicas/combat.md secao 18.6/18.4 (T4 lapida em T1); ADR-006.

#ifndef GUS_DOMAIN_COMBAT_ENVIRONMENT_TRANSITIONS_HPP
#define GUS_DOMAIN_COMBAT_ENVIRONMENT_TRANSITIONS_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "gus/domain/combat/environment_enums.hpp"

namespace gus::domain::combat {

// Gatilho de uma transicao de ambiente (secao 18.6). Ou uma FAMILIA forte incidindo no
// terreno/clima, ou OUTRO ambiente coexistindo, ou um evento de arena. Ordinais explicitos
// na ordem de declaracao do C# (contrato estavel).
enum class EnvironmentTrigger : std::uint32_t {
    // Familia Eletrica forte OU Calor (calor e o sabor climatico equivalente).
    EletricoOuCalor = 0,
    // Familia Sonica forte.
    SonicoForte = 1,
    // Clima Vento ativo.
    Vento = 2,
    // Clima Chuva coexistindo.
    Chuva = 3,
    // Clima/ambiente Eletrico coexistindo.
    EletricoAmbiente = 4,
    // Clima Gelo coexistindo.
    Gelo = 5,
    // Clima Agua coexistindo.
    Agua = 6,
    // Acaceiro saudavel proximo (estado de arena) - vetor de purificacao canon.
    AcaceiroSaudavel = 7,
    // Arena de T4 Ashlar Bruto vencida (entre encontros) - progressao maconica.
    AshlarVencidoEntreEncontros = 8,
};

// Transicao deterministica (secao 18.6): (gatilho, ambiente-alvo) -> ambiente-resultado,
// com nota. result = EnvironmentId::None quando o resultado e "ar limpo" (a camada de
// clima e REMOVIDA, nao substituida por outra).
struct EnvironmentTransition {
    EnvironmentTrigger trigger = EnvironmentTrigger::EletricoOuCalor;
    EnvironmentId target = EnvironmentId::None;
    EnvironmentId result = EnvironmentId::None;
    std::string note;

    [[nodiscard]] bool operator==(const EnvironmentTransition&) const = default;
};

// Tabela fechada de mutabilidade (secao 18.6). Funcoes livres; deterministica. A resolucao
// e (gatilho, alvo) -> resultado: nunca ambigua (no maximo 1 linha casa).
namespace EnvironmentTransitions {

// Todas as transicoes canonicas (secao 18.6), na ordem da tabela do doc.
[[nodiscard]] const std::vector<EnvironmentTransition>& all();

// Resolve a transicao deterministica (secao 18.6): dado um gatilho e o ambiente-alvo,
// devolve o ambiente resultante. Retorna nullopt se nenhuma linha casa (sem efeito; a
// curadoria nao inventa transicoes). Nunca ambiguo: no maximo 1 linha casa cada
// (gatilho, alvo).
[[nodiscard]] std::optional<EnvironmentId> resolve(EnvironmentTrigger trigger,
                                                   EnvironmentId target);

}  // namespace EnvironmentTransitions

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_ENVIRONMENT_TRANSITIONS_HPP
