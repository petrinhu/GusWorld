// gus/domain/templates/card_family.hpp
//
// Enum LOCAL MINIMO da familia da roda de fraqueza (combat.md secao 6/7). FRONTEIRA
// documentada: no C# CardFamily vive em turn_combat/CombatEnums.cs, junto do motor
// de combate (CombatActor, IEnemyBrain), que ainda NAO foi portado neste marco. Para
// nao arrastar todo o turn_combat so por um enum, declaramos a copia minima aqui,
// mesmo padrao ja usado no progression (CombatOutcome/DefeatedActor locais). Quando
// turn_combat for portado, a fonte canonica do enum se move para la e templates/
// passa a inclui-la (a ordem dos valores DEVE ser preservada: e o contrato binario
// serializado, ordinais 0..4).
//
// POCO puro, ZERO Qt (invariante de domain/, engine-design.md secao 2).
//
// Cross-ref: engine/foundation/turn_combat/CombatEnums.cs (origem canonica),
//            docs/design/mecanicas/combat.md secao 6/7, ADR-006.

#ifndef GUS_DOMAIN_TEMPLATES_CARD_FAMILY_HPP
#define GUS_DOMAIN_TEMPLATES_CARD_FAMILY_HPP

#include <cstdint>

namespace gus::domain::templates {

// Familia-base da carta / roda de fraqueza (combat.md secao 6). A ORDEM e o
// contrato binario do serializer (ordinal 0..4); espelha CombatEnums.cs 1:1.
enum class CardFamily : std::uint32_t {
    Eletrico = 0,
    Bioquimico = 1,
    Sonico = 2,
    Cinetico = 3,
    Criptografico = 4,
};

}  // namespace gus::domain::templates

#endif  // GUS_DOMAIN_TEMPLATES_CARD_FAMILY_HPP
