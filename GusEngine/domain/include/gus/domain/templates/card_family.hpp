// gus/domain/templates/card_family.hpp
//
// FRONTEIRA FECHADA (A1, auditoria M3): templates::CardFamily NAO e mais uma copia local
// do enum. Agora e um ALIAS da FONTE CANONICA do combate
// (gus::domain::combat::CardFamily, em combat/combat_enums.hpp). Uma definicao so, sem
// duplicacao. Os ordinais (0..4) sao identicos por construcao (e o mesmo enum), entao o
// contrato binario do serializer .gdt NAO muda: family continua gravado como o mesmo u32.
//
// HISTORICO: no M3 (antes do motor de combate ser portado) declaravamos uma copia minima
// aqui pra nao arrastar todo o turn_combat por um enum. O chunk 1 do M5 criou a fonte
// canonica em combat/ com paridade de ordinal travada por teste; o chunk 4 (A1) faz a
// religacao: templates/ passa a CONSUMIR essa fonte. O nome gus::domain::templates::
// CardFamily continua valido (alias) para o codigo existente compilar sem mudanca.
//
// POCO puro, ZERO Qt (invariante de domain/, engine-design.md secao 2).
//
// Cross-ref: gus/domain/combat/combat_enums.hpp (fonte canonica);
//            engine/foundation/turn_combat/CombatEnums.cs (origem),
//            docs/design/mecanicas/combat.md secao 6/7, ADR-006.

#ifndef GUS_DOMAIN_TEMPLATES_CARD_FAMILY_HPP
#define GUS_DOMAIN_TEMPLATES_CARD_FAMILY_HPP

#include <cstdint>

#include "gus/domain/combat/combat_enums.hpp"

namespace gus::domain::templates {

// Alias da fonte canonica (combat/combat_enums.hpp). Ordinais 0..4 preservados (e o MESMO
// enum): o contrato binario do .gdt e inalterado. Religacao do A1.
using CardFamily = gus::domain::combat::CardFamily;

// CARD-FAMILY-UNIVERSAL (decisao do criador 2026-07-14, PS-R1): CardFamily ganhou um 6o
// valor (Universal = 5, append-only), mas Universal vale SO PARA CARTAS. Templates de
// personagem/inimigo continuam restritos a roda de 5 — por isso dois contadores:
//
//   kWheelFamilyCount = 5  -> dominio da RODA (0..4). Usado pela validacao de ordinal
//                             (A1) em CharacterTemplate::validate()/EnemyTemplate::validate():
//                             rejeita family fora de {0..4}, INCLUSIVE Universal (5).
//                             Comportamento INALTERADO em relacao ao A1 (so o nome/motivo
//                             mudou: antes "todo o dominio canonico", agora "a roda").
//   kCardFamilyCount  = 6  -> dominio TOTAL do enum (0..5, roda + Universal). Existe pra
//                             consumidores futuros que precisem enumerar TODAS as
//                             familias de CARTA (ex.: catalogo/UI de cartas). NAO usado
//                             na validacao de template (essa usa kWheelFamilyCount).
inline constexpr std::uint32_t kWheelFamilyCount = 5;
inline constexpr std::uint32_t kCardFamilyCount = 6;

}  // namespace gus::domain::templates

#endif  // GUS_DOMAIN_TEMPLATES_CARD_FAMILY_HPP
