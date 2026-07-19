// gus/domain/combat/combat_enums.hpp
//
// Enums POCO do sistema de combate turn-based. FONTE CANONICA dos enums de RUNTIME
// de combate (portado de engine/foundation/turn_combat/CombatEnums.cs). POCO puro,
// ZERO Qt (invariante de domain/, engine-design.md secao 2). Header-only.
//
// ATOM-2 (decomposicao atomica ao nivel de modulo, generalizando ADR-019): o
// VOCABULARIO DE CARTA (CardFamily, CardBaseType, TargetShape, CardModifier,
// StackRule, StatusId, CardTier, CardCategory, TriggerHook, EffectKind, SideFilter)
// foi EXTRAIDO para gus/domain/cards/card_enums.hpp (LAR CANONICO daqueles enums).
// Este header virou FACHADA para esse pedaco: inclui card_enums.hpp e RE-EXPORTA
// os nomes em gus::domain::combat via using-declaration (MESMA identidade de tipo,
// nao um typedef/copia) - os ~90 consumidores existentes de combat_enums.hpp
// continuam compilando e enxergando `gus::domain::combat::CardFamily` etc.
// intocados. cards/ NAO inclui combat/ (inversao de dependencia; gate de camadas).
//
// Os enums de RUNTIME de combate PROPRIAMENTE DITO (WeaknessTier, CombatActionType,
// CombatPhase, CombatOutcome, PipelineSlotKind, StatusChangeKind) continuam aqui -
// nao sao vocabulario de carta, sao estado da FSM/resolucao.
//
// Sobre CardFamily: gus/domain/cards/card_enums.hpp e a FONTE CANONICA do enum
// (espelha CombatEnums.cs 1:1, ordinais 0..4). Existe hoje uma copia local em
// gus/domain/templates/card_family.hpp (portada no M3 antes do motor de combate
// existir). A RELIGACAO de templates/ para essa fonte (e a remocao da copia) e o
// item A1, fechado no chunk 4 - NAO neste sub-porte. Aqui apenas garantimos que os
// ordinais BATEM (validado por combat_enums_test.cpp) para a religacao futura ser
// trivial.
//
// Todos os enums sao std::uint32_t com ordinais EXPLICITOS: o ordinal e o contrato
// binario do serializer futuro (mesmo racional dos enums de templates/). Reordenar
// quebra saves; os valores explicitos travam isso. No C# os ordinais sao implicitos
// (0..N na ordem de declaracao); aqui os tornamos explicitos sem mudar a ordem.
//
// Cross-ref: gus/domain/cards/card_enums.hpp (LAR do vocabulario de carta);
//            engine/foundation/turn_combat/CombatEnums.cs;
//            docs/design/mecanicas/combat.md secao 3/6/7/8/9/16; ADR-006; ADR-019.

#ifndef GUS_DOMAIN_COMBAT_COMBAT_ENUMS_HPP
#define GUS_DOMAIN_COMBAT_COMBAT_ENUMS_HPP

#include <cstdint>

#include "gus/domain/cards/card_enums.hpp"

namespace gus::domain::combat {

// ---- Re-exporta o vocabulario de carta (LAR CANONICO: gus/domain/cards/card_enums.hpp,
// ATOM-2). Using-declaration preserva a IDENTIDADE de tipo (gus::domain::combat::CardFamily
// e gus::domain::cards::CardFamily sao o MESMO tipo, so acessivel por dois nomes
// qualificados) - nenhum consumidor existente precisa mudar.
using cards::CardBaseType;
using cards::CardCategory;
using cards::CardFamily;
using cards::CardModifier;
using cards::CardTier;
using cards::EffectKind;
using cards::SideFilter;
using cards::StackRule;
using cards::StatusId;
using cards::TargetShape;
using cards::TriggerHook;

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
    // Efeito de SISTEMA disparado num tick de status no TurnStart (ex.: Knockback adiando o
    // turno do current(), secao 9), NAO amarrado a uma CombatAction de jogador. Append-only
    // (decisao do lider 2026-07-15, COMBATE-FILA-CURSOR-FIX): a regra canonica "todo efeito
    // loga uma mensagem diegetica" exige um CombatLogEntry mesmo quando nao ha acao de
    // jogador correspondente - reusar Attack/UseCard/etc classificaria errado no log/UI.
    StatusTick = 9,
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
