// gus/domain/combat/combat_enums.hpp
//
// Enums POCO do sistema de combate turn-based. FONTE CANONICA dos enums do motor de
// combate (portado de engine/foundation/turn_combat/CombatEnums.cs). POCO puro, ZERO
// Qt (invariante de domain/, engine-design.md secao 2). Header-only.
//
// Sobre CardFamily: este header e a FONTE CANONICA do enum (espelha CombatEnums.cs
// 1:1, ordinais 0..4). Existe hoje uma copia local em
// gus/domain/templates/card_family.hpp (portada no M3 antes do motor de combate
// existir). A RELIGACAO de templates/ para esta fonte (e a remocao da copia) e o
// item A1, fechado no chunk 4 - NAO neste sub-porte. Aqui apenas garantimos que os
// ordinais BATEM (validado por combat_enums_test.cpp) para a religacao futura ser
// trivial.
//
// Todos os enums sao std::uint32_t com ordinais EXPLICITOS: o ordinal e o contrato
// binario do serializer futuro (mesmo racional dos enums de templates/). Reordenar
// quebra saves; os valores explicitos travam isso. No C# os ordinais sao implicitos
// (0..N na ordem de declaracao); aqui os tornamos explicitos sem mudar a ordem.
//
// Cross-ref: engine/foundation/turn_combat/CombatEnums.cs;
//            docs/design/mecanicas/combat.md secao 3/6/7/8/9/16; ADR-006.

#ifndef GUS_DOMAIN_COMBAT_COMBAT_ENUMS_HPP
#define GUS_DOMAIN_COMBAT_COMBAT_ENUMS_HPP

#include <cstdint>

namespace gus::domain::combat {

// As 5 familias de carta da roda de fraqueza (identidade mecanica nao-sobreposta),
// MAIS Universal (append-only, ordinal 5). secao 6.
// Gus = todas as familias (compilador universal); companions = especialistas.
// FONTE CANONICA. Ordem/ordinais espelham CombatEnums.cs e templates/card_family.hpp.
//
// Universal (decisao do criador 2026-07-14, achado PS-R1; docs/design/mecanicas/combat.md
// secao 20): familia das CARTAS que NAO competem na roda de fraqueza — multFraqueza
// SEMPRE 1.0, SEM Fraco/Resistente/Imune (ver weakness_wheel.hpp). Cobre as ~13 cartas
// especiais nao-elementais dos mestres + o "utilitario" ja usado em secao 9
// (Shield/Regen/Haste/Slow). SO-CARTAS: por decisao do criador, personagem/inimigo
// (character_template.hpp / enemy_template.hpp) continuam restritos a roda de 5
// (templates::kWheelFamilyCount); Universal NAO e um valor valido pra family de
// template hoje. Ordinal 5 e ADITIVO: NAO reordena os 0..4 (contrato binario do
// serializer .gdt).
enum class CardFamily : std::uint32_t {
    Eletrico = 0,
    Bioquimico = 1,
    Sonico = 2,
    Cinetico = 3,
    Criptografico = 4,
    Universal = 5,
};

// Tipo-base diegetico da carta (gramatica "tipo.familia"). secao 7.
enum class CardBaseType : std::uint32_t {
    Pulso = 0,
    Raiz = 1,
    Eco = 2,
    Fenda = 3,
    Glifo = 4,
};

// Forma de alvo da acao/carta. secao 7. (Single = nome diegetico canonico, secao 7.)
enum class TargetShape : std::uint32_t {
    Self = 0,
    Single = 1,
    Linha = 2,
    Area3x3 = 3,
    Grupo = 4,
};

// Modificadores anexaveis em runtime (modelo B). secao 8. Cada um soma mana.
// (Object/Null/Stream = nomes diegeticos canonicos da spec secao 8.)
enum class CardModifier : std::uint32_t {
    // Cria entidade persistente no campo (+1 mana).
    Object = 0,
    // Converte single em area OU multi-hit (+2 mana).
    Stream = 1,
    // Inverte/cancela efeito (+1 mana). Requer Scan previo no alvo.
    Null = 2,
};

// Regra de empilhamento de status. secao 9.
enum class StackRule : std::uint32_t {
    Replace = 0,
    Refresh = 1,
    StackMagnitude = 2,
    StackDuration = 3,
};

// Identidade de status aplicavel a qualquer ator. secao 9.
enum class StatusId : std::uint32_t {
    Stun = 0,
    Poison = 1,
    Corrode = 2,
    Disrupt = 3,
    Silence = 4,
    Knockback = 5,
    Break = 6,
    Expose = 7,
    Decrypt = 8,
    Shield = 9,
    Regen = 10,
    Haste = 11,
    Slow = 12,
};

// Tier de fraqueza da roda deterministica. secao 6.
// Fraco = 1.5, Neutro = 1.0, Resistente = 0.66, Imune = 0.0.
enum class WeaknessTier : std::uint32_t {
    Fraco = 0,
    Neutro = 1,
    Resistente = 2,
    Imune = 3,
};

// Tipo de acao que um ator pode tomar em ActionSelect. secao 3/5.
enum class CombatActionType : std::uint32_t {
    Attack = 0,
    Defend = 1,
    UseCard = 2,
    Flee = 3,
    Scan = 4,
    ScanEnvironment = 5,
    GambitPredict = 6,
    GambitReorder = 7,
    Pass = 8,
};

// Fases da FSM de combate. secao 3.
enum class CombatPhase : std::uint32_t {
    SetupPhase = 0,
    TurnStart = 1,
    ActionSelect = 2,
    ActionResolve = 3,
    TurnEnd = 4,
    CheckEnd = 5,
    CombatEnd = 6,
};

// Resultado de uma rodada de combate. secao 3 (CheckEnd / CombatEnd).
enum class CombatOutcome : std::uint32_t {
    Ongoing = 0,
    Victory = 1,
    Defeat = 2,
    Fled = 3,
};

// Tipo de slot da pipeline de compilacao (3 slots). secao 10.
enum class PipelineSlotKind : std::uint32_t {
    Card = 0,
    Modifier = 1,
};

// Natureza de uma mudanca de status registrada pos-turno (secao 16).
//   Applied:  status adicionado a um ator (carta StatusApplied, combo, Defend/Shield).
//   Expired:  status removido por Duration <= 0 (TurnEnd) ou por depleção de Shield.
//   Absorbed: Shield absorveu dano (pool decrementado), sem expirar ainda.
enum class StatusChangeKind : std::uint32_t {
    Applied = 0,
    Expired = 1,
    Absorbed = 2,
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_COMBAT_ENUMS_HPP
