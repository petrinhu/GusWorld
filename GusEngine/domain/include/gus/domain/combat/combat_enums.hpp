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
// APPEND-ONLY a partir daqui: 13+ sao do executor techMagic (ADR-016); NAO reordenar
// 0..12 (contrato binario do serializer futuro).
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
    SobrecargaTermica = 13,  // cartas-technomagik.md secao 5.1: DoT 3t 8/5/2 + Slow -1
                             // SPD; Refresh; familia Eletrico.
    Resfriamento = 14,       // secao 5.2: +1 SPD + (-1 mana na 1a carta, clamp custo min
                             // 1); Dur 3; Refresh; buff.
    Reflect = 15,            // marcador de reflexao (Newton); comportamento = executor
                             // techMagic (step 2+).
    BlindagemEM = 16,        // EM-Shield (Faraday, ADR-016 Balde B): imunidade a debuff
                             // eletrico (bloqueia + limpa Sobrecarga Termica/Stun/etc de
                             // familia Eletrico no alvo). Ver CombatActor::try_add_status.
    NullProof = 17,          // Null-Proof (Godel, ADR-016 Balde B PR3): trunfo fura-imunidade
                             // guardado no PORTADOR (buff, self ou aliado via side_filter
                             // AllyOnly). No proximo hit do portador contra um alvo Resistente
                             // OU Imune (mult_fraqueza < 1.0), o resolvedor forca mult 1.0 e
                             // CONSOME (remove) o status; contra Neutro/Fraco fica intacto,
                             // guardado. Ver combat_state_machine.cpp::resolve_use_card/
                             // estimate_card_damage.
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

// ---- Executor techMagic (ADR-016, MVP step 1) -------------------------------------
// Enums novos do motor techMagic: a carta especial/super e um programa (lista ordenada
// de EffectSpec, ver combat_records.hpp) que o Tavus-Drive executa. Record-only neste
// step (nenhuma logica de resolucao ainda). APPEND-ONLY dali em diante (contrato
// binario do serializer futuro).

// Raridade/trilha de producao da carta. secao 6/7. Comum = as 5 do placeholder_cards
// (intocadas); Especial/Super = trilha do executor techMagic.
enum class CardTier : std::uint32_t {
    Comum = 0,
    Especial = 1,
    Super = 2,
};

// Categoria de uso da carta; semantica so aplica quando tier != Comum.
enum class CardCategory : std::uint32_t {
    Ativa = 0,
    Passiva = 1,
    ForaDeCombate = 2,
    Hibrida = 3,
};

// Gatilho de execucao de um EffectSpec dentro do programa da carta.
enum class TriggerHook : std::uint32_t {
    OnCast = 0,
    OnDamageDealt = 1,
    OnDamageReceived = 2,
    OnAllyTurnEnd = 3,
    OnRoundEnd = 4,
    Always = 5,
};

// Tipo de efeito executavel por um EffectSpec. Conjunto inicial do MVP; cresce por
// demanda (ate ~20-25) conforme as 20 especiais forem definidas. APPEND-ONLY.
enum class EffectKind : std::uint32_t {
    ApplyStatus = 0,
    Leech = 1,
    Reflect = 2,
    HypotenuseCombo = 3,
    CloneAlly = 4,
    // Re-Run/Fractal-Echo (MVP step 5, Mandelbrot+Ada; decisoes Q1-Q4 do lider,
    // 2026-07-14): reaplica o DANO>0 da ULTIMA ACAO de dano de QUALQUER aliado NESTA
    // RODADA (LastActionRecord), escalado por EffectSpec.percent, direto via
    // CombatActor::take_damage PURO (eco do resultado, sem novo sorteio/status/mana).
    // EffectSpec.magnitude = chance% do Re-Run (0 = sempre, Mandelbrot; >0 consome 1
    // rng->next(100), Ada). Ver techmagic.cpp::handle_repeat_last_action.
    RepeatLastAction = 5,
    // Cadeia de dano decrescente (Tesla): apos o dano-base atingir o alvo primario, a
    // descarga SALTA pros proximos inimigos VIVOS na ordem da fila (ate EffectSpec.magnitude
    // saltos), retendo EffectSpec.percent% do dano a cada salto (decaimento multiplicativo).
    // Dano PURO (take_damage), 0 consumo de RNG. Ver techmagic.cpp::handle_chain_damage.
    ChainDamage = 6,
    // Dilatacao temporal (Einstein/Time-Dilate): empurra a acao de 1 inimigo pro FIM da
    // fila da rodada corrente (age por ultimo; toda a party restante age antes). Primitiva
    // = InitiativeQueue::reorder_actor (mesma da Gambito-Reordenar). EffectSpec.magnitude
    // == 0 = empurra pro fim da fila (Einstein); >0 = N posicoes fixas. Alvo que JA agiu
    // nesta rodada (indice < cursor da fila) e um no-op + log de dissipacao, NAO reaplica
    // na proxima rodada. 0 consumo de RNG. Ver techmagic.cpp::handle_delay_action.
    DelayAction = 7,
};

// Filtro de lado do alvo de um EffectSpec (ADR-016 Balde B, Faraday/EM-Shield). Data-driven:
// codifica a filosofia "efeito-de-inimigo no aliado = beneficio" sem precisar de um
// EffectKind novo por carta. Any (default) preserva o comportamento de TODAS as cartas/
// EffectSpec ja existentes (nenhum filtro). EnemyOnly/AllyOnly dissipam (no-op + log) quando
// o alvo resolvido esta do lado errado - ver techmagic.cpp::handle_apply_status. APPEND-ONLY.
enum class SideFilter : std::uint32_t {
    Any = 0,
    EnemyOnly = 1,
    AllyOnly = 2,
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_COMBAT_ENUMS_HPP
