// gus/domain/src/combat/master_cards.cpp
//
// Implementacao do catalogo das 12 cartas ESPECIAIS suportadas pelo executor techMagic
// (ADR-016, MVP steps 4-7). Ver header para o contrato. Mesmo padrao de
// placeholder_cards.cpp: cartas construidas uma vez, emplace fail-fast em id duplicado.
// POCO puro, ZERO Qt.
//
// Regra de familia (cartas-technomagik.md secao 2.3): dominio eletromagnetico -> Eletrico;
// resto -> Universal. Volta/Faraday/Euler/Tesla = Eletrico; Newton/Pythagoras/Godel/Turing/
// Menger/Mandelbrot/Ada = Universal. Einstein = Cinetico (decisao do criador, brief
// TIME-DILATE 2026-07-15 - a dilatacao temporal empurra a fila, mesma familia das cartas
// COMUNS de reordenar/knockback).
//
// Faraday (EM-Shield, ADR-016 Balde B, decisao do lider 2026-07-15): passa de ForaDeCombate
// (posse-only) pra Hibrida - ganha uma face de combate castavel (OnCast -> ApplyStatus
// BlindagemEM) alem da face fora-de-combate (anti-PEM, posse-only, ainda FEAT FUTURA, sem
// programa). Mana kActiveManaCost (nao mais 0) e sujeita ao gate 1x/batalha das
// Ativa/Hibrida (resolve_use_card).
//
// NUMEROS PROVISORIOS (//PLAYTEST): mana das ativas/hibridas = 6; percent do Leech (Volta)
// = 50; percent do Reflect (Newton) = 30; percent do eco Fractal-Echo (Mandelbrot) = 50;
// percent do eco Re-Run (Ada) = 100/magnitude(chance) = 34; magnitude do DelayAction
// (Einstein) = 0 (fim da fila). Balanceamento real (VOLTA-LEECH-%, statline por carta) e
// trabalho futuro.
//
// base_type: as ativas/hibridas usam Glifo (carta-programa nao-elemental; //PLAYTEST, nao
// ha convencao fechada pra especiais). As posse-only/passivas idem Glifo por coerencia (o
// base_type diegetico nao afeta o executor).
//
// Cross-ref: gus/domain/combat/master_cards.hpp; docs/design/roster-analogos/
//            _EFEITOS-ESCOLHIDOS.md; docs/design/mecanicas/cartas-technomagik.md; ADR-016.

#include "gus/domain/combat/master_cards.hpp"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "gus/domain/combat/combat_enums.hpp"

namespace gus::domain::combat::MasterCards {

namespace {

// Numeros provisorios centralizados (todos //PLAYTEST).
constexpr int kActiveManaCost = 6;    //PLAYTEST mana das ativas/hibridas (Volta, Newton).
constexpr int kVoltaLeechPercent = 50;  //PLAYTEST VOLTA-LEECH-% (fracao do Leech do Volta).
constexpr int kNewtonReflectPercent = 30;  //PLAYTEST fracao do Reflect (Newton "acao e reacao").
// Reflect-por-STATUS concedido no modo-aliado (N-3, ADR-016 Balde B PR2, decisao do lider
// 2026-07-15): mesma fracao da passiva-propria (N-4: as duas fontes somam, ver
// apply_damage_with_hooks). Duracao independente da do Poco Gravitacional (Stun 1t).
constexpr int kNewtonReflectDuration = 3;  //PLAYTEST duracao do Reflect-status (3 turnos).
// RepeatLastAction (ADR-016 secao 20 item 5, decisoes Q1-Q4 do lider 2026-07-14).
constexpr int kMandelbrotEchoPercent = 50;  //PLAYTEST escala do eco Fractal-Echo (Mandelbrot).
constexpr int kAdaEchoPercent = 100;  // Q3: Ada ecoa a 100% - o freio dela e a CHANCE, nao a escala.
constexpr int kAdaEchoChance = 34;    //PLAYTEST chance% do Re-Run (Ada); easter-egg velado.
// ChainDamage (Tesla, ADR-016 step 6): cadeia de dano decrescente que salta pros proximos
// inimigos vivos.
constexpr int kTeslaChainRetentionPercent = 62;  //PLAYTEST retencao por salto da cadeia.
constexpr int kTeslaPower = 8;  //PLAYTEST dano-base do primario (Tesla e excecao: cadeia escala dele).
// DelayAction (Einstein/Time-Dilate, ADR-016 step 7): empurra a acao do alvo pro fim da
// fila da rodada (magnitude 0). //PLAYTEST reversivel se o playtest pedir N-posicoes fixas.
constexpr int kEinsteinDelayMagnitude = 0;  // 0 = fim da fila (decisao do criador 2026-07-15).
// BlindagemEM (Faraday/EM-Shield, ADR-016 Balde B, decisao do lider 2026-07-15): duracao
// da imunidade a debuff eletrico.
constexpr int kFaradayShieldDuration = 3;  //PLAYTEST duracao da BlindagemEM (3 turnos).
// NullProof (Godel/Null-Proof, ADR-016 Balde B PR3, decisao do lider 2026-07-15): duracao-
// sentinela ALTA (nao expira por turno na pratica do slice) - o status e removido por
// CONSUMO (resolve_use_card, no hit que fura), nao por tick de duracao. //PLAYTEST.
constexpr int kNullProofDuration = 99;  //PLAYTEST sentinela (consumido, nao tickado).

// Monta uma carta ESPECIAL base. base_type = Glifo (carta-programa nao-elemental,
// //PLAYTEST). ap_cost default 1 (herda o mesmo default do placeholder). Sem
// status_applied, sem modifiers, sem crit/mastery (defaults do struct).
Card make_special(std::string id,
                  std::string display_name,
                  CardFamily family,
                  CardCategory category,
                  int mana_cost,
                  std::vector<EffectSpec> effects,
                  bool ignores_weakness_wheel = false,
                  int power = 0,
                  TargetShape target_shape = TargetShape::Single) {
    Card c;
    c.id = std::move(id);
    c.display_name = std::move(display_name);
    c.family = family;
    c.base_type = CardBaseType::Glifo;  //PLAYTEST (sem convencao fechada pra especiais).
    c.mana_cost = mana_cost;
    c.ap_cost = 1;
    // Default 0: o dano/efeito das outras especiais vem do programa (EffectSpec), nao de
    // power base. Excecao = Tesla (ChainDamage escala do dano-base do primario) -> power > 0.
    c.power = power;
    // Default Single; Newton override pra Grupo (Poco Gravitacional, N-1: AoE nos inimigos
    // ou, no ramo assimetrico do modo-aliado, single no aliado alvo - ver resolve_targets).
    c.target_shape = target_shape;
    c.tier = CardTier::Especial;
    c.category = category;
    c.effects = std::move(effects);
    c.ignores_weakness_wheel = ignores_weakness_wheel;
    return c;
}

std::unordered_map<std::string, Card> assemble() {
    const Card cards[] = {
        // --- Volta (Volt-Leech): Ativa, Eletrico. OnDamageDealt -> Leech (dreno
        // termodinamico; % de conversao ainda provisorio, VOLTA-LEECH-%). ---
        make_special(
            "volta", "CARD_EXEC_VOLTA_NAME", CardFamily::Eletrico, CardCategory::Ativa,
            kActiveManaCost,
            {EffectSpec{.trigger = TriggerHook::OnDamageDealt,
                       .kind = EffectKind::Leech,
                       .percent = kVoltaLeechPercent}}),

        // --- Newton (Force-Law): Hibrida, Universal, Grupo (N-1, ADR-016 Balde B PR2,
        // decisao do lider 2026-07-15). Duas faces, roteadas por side_filter (ambas
        // dissipam-no-lado-errado, no-op+log, nunca lancam):
        //   OnCast -> ApplyStatus Stun 1t, side_filter EnemyOnly (Poco Gravitacional):
        //     imobiliza TODOS os inimigos vivos do grupo; o dano-de-ATK que sai da formula
        //     base ja acerta o mesmo grupo (power segue 0, so o ATK do conjurador conta).
        //     Em aliado, dissipa (a face ofensiva nao recai no proprio time).
        //   OnCast -> ApplyStatus Reflect (status, magnitude=percent), side_filter AllyOnly
        //     (N-3, modo-aliado): concede Reflect 30%/3t ao ALIADO alvo (ramo assimetrico
        //     de resolve_targets - Grupo mirando um aliado vira single nesse 1 aliado). Em
        //     inimigo, dissipa.
        // OnDamageReceived -> Reflect (passiva-propria do dono, INTOCADA): se o proprio
        // dono TAMBEM tiver o Reflect-status (ex. auto-cast), as duas fontes SOMAM (N-4,
        // apply_damage_with_hooks honra as duas separadamente - de graca, sem wiring
        // extra aqui). ---
        make_special(
            "newton", "CARD_EXEC_NEWTON_NAME", CardFamily::Universal, CardCategory::Hibrida,
            kActiveManaCost,
            {EffectSpec{.trigger = TriggerHook::OnCast,
                       .kind = EffectKind::ApplyStatus,
                       .duration = 1,  //PLAYTEST duracao do Stun (1 turno).
                       .status = StatusId::Stun,
                       .stack_rule = StackRule::Replace,
                       .side_filter = SideFilter::EnemyOnly},
             EffectSpec{.trigger = TriggerHook::OnCast,
                       .kind = EffectKind::ApplyStatus,
                       .magnitude = kNewtonReflectPercent,  // status.magnitude = fracao %.
                       .duration = kNewtonReflectDuration,  //PLAYTEST 3 turnos.
                       .status = StatusId::Reflect,
                       .stack_rule = StackRule::Refresh,
                       .side_filter = SideFilter::AllyOnly},
             EffectSpec{.trigger = TriggerHook::OnDamageReceived,
                       .kind = EffectKind::Reflect,
                       .percent = kNewtonReflectPercent}},
            /*ignores_weakness_wheel=*/false, /*power=*/0, /*target_shape=*/TargetShape::Grupo),

        // --- Pythagoras (Hypotenuse): Passiva, Universal, mana 0. OnRoundEnd ->
        // HypotenuseCombo (golpe-bonus quando >=2 aliados batem no mesmo alvo). ---
        make_special(
            "pythagoras", "CARD_EXEC_PYTHAGORAS_NAME", CardFamily::Universal,
            CardCategory::Passiva, 0,
            {EffectSpec{.trigger = TriggerHook::OnRoundEnd,
                       .kind = EffectKind::HypotenuseCombo}}),

        // --- Mandelbrot (Fractal-Echo): Ativa, Universal. OnCast -> RepeatLastAction
        // (magnitude 0 = sempre dispara, 0 consumo de RNG): eco de 50% do dano da ULTIMA
        // acao de dano de um aliado nesta rodada (Q1-Q4, ADR-016 secao 20 item 5). ---
        make_special(
            "mandelbrot", "CARD_EXEC_MANDELBROT_NAME", CardFamily::Universal,
            CardCategory::Ativa, kActiveManaCost,
            {EffectSpec{.trigger = TriggerHook::OnCast,
                       .kind = EffectKind::RepeatLastAction,
                       .magnitude = 0,  // sempre dispara (nenhuma chance, Mandelbrot).
                       .percent = kMandelbrotEchoPercent}}),

        // --- Ada Lovelace (Re-Run): Passiva, Universal, mana 0. OnAllyTurnEnd ->
        // RepeatLastAction (percent 100, magnitude = chance% do Re-Run): no fim do turno
        // de OUTRO aliado, chance de ecoar a ultima acao de dano dele por completo
        // (Q1-Q4, ADR-016 secao 20 item 5). ---
        make_special(
            "ada", "CARD_EXEC_ADA_NAME", CardFamily::Universal, CardCategory::Passiva, 0,
            {EffectSpec{.trigger = TriggerHook::OnAllyTurnEnd,
                       .kind = EffectKind::RepeatLastAction,
                       .magnitude = kAdaEchoChance,  // chance% (0..99 roll < magnitude).
                       .percent = kAdaEchoPercent}}),

        // --- Godel (Null-Proof): Ativa, Universal, mana 0 (ADR-016 Balde B PR3, decisao do
        // lider 2026-07-15, G-1). Duas vias fura-defesa INDEPENDENTES, wired no resolvedor
        // (item 11 GENERICO + G-2/G-3, ver combat_state_machine.cpp::resolve_use_card):
        //   (a) ignores_weakness_wheel=true: o TRUNFO original de Godel - qualquer carta com
        //       esta flag SEMPRE resolve mult 1.0 contra a roda de fraqueza, sem tocar em
        //       status de ninguem (generico, vale pra qualquer carta futura que a declare).
        //   (b) OnCast -> ApplyStatus NullProof, side_filter AllyOnly (inclui self): concede
        //       o status ao PORTADOR (proprio Godel ou um aliado). No proximo ataque do
        //       portador contra alvo Resistente/Imune (mult < 1.0), o resolvedor forca mult
        //       1.0 e CONSOME o status (G-2); contra Neutro/Fraco fica intacto (G-3: fura os
        //       DOIS tiers, Imune 0.0 e Resistente 0.66). Duracao-sentinela alta (99,
        //       kNullProofDuration) porque a saida real e por consumo, nao por tick.
        // 1x/batalha via specials_cast_ (Ativa/Hibrida, secao 2.1) - mesmo gate de Volta/
        // Newton/Faraday. ---
        make_special(
            "godel", "CARD_EXEC_GODEL_NAME", CardFamily::Universal, CardCategory::Ativa, 0,
            {EffectSpec{.trigger = TriggerHook::OnCast,
                       .kind = EffectKind::ApplyStatus,
                       .duration = kNullProofDuration,
                       .status = StatusId::NullProof,
                       .stack_rule = StackRule::Refresh,
                       .side_filter = SideFilter::AllyOnly}},
            /*ignores_weakness_wheel=*/true),

        // --- Faraday (EM-Shield): Hibrida, Eletrico (ADR-016 Balde B, decisao do lider
        // 2026-07-15). OnCast -> ApplyStatus BlindagemEM (dur 3, Refresh, side_filter
        // AllyOnly incluindo self): previne + limpa debuffs eletricos no alvo (F-1/F-2,
        // portao em CombatActor::try_add_status). Cast em INIMIGO dissipa (side_filter,
        // F-4). A face fora-de-combate (anti-PEM, posse-only) fica FEAT FUTURA - nao
        // implementada aqui (so a face de combate castavel). ---
        make_special(
            "faraday", "CARD_EXEC_FARADAY_NAME", CardFamily::Eletrico, CardCategory::Hibrida,
            kActiveManaCost,
            {EffectSpec{.trigger = TriggerHook::OnCast,
                       .kind = EffectKind::ApplyStatus,
                       .duration = kFaradayShieldDuration,
                       .status = StatusId::BlindagemEM,
                       .stack_rule = StackRule::Refresh,
                       .side_filter = SideFilter::AllyOnly}}),

        // --- Euler (Bridge-Walk): ForaDeCombate, Eletrico, mana 0. Posse-only (atalho +
        // revelar grafo da dungeon); sem programa de combate. ---
        make_special("euler", "CARD_EXEC_EULER_NAME", CardFamily::Eletrico,
                     CardCategory::ForaDeCombate, 0, /*effects=*/{}),

        // --- Turing (Decrypt-All): ForaDeCombate, Universal, mana 0. Posse-only
        // (cripto-chave universal / revela cifras); sem programa de combate. ---
        make_special("turing", "CARD_EXEC_TURING_NAME", CardFamily::Universal,
                     CardCategory::ForaDeCombate, 0, /*effects=*/{}),

        // --- Menger (Barter): ForaDeCombate, Universal, mana 0. Posse-only (valor
        // marginal + escambo loot->Credito); economia, sem programa de combate. ---
        make_special("menger", "CARD_EXEC_MENGER_NAME", CardFamily::Universal,
                     CardCategory::ForaDeCombate, 0, /*effects=*/{}),

        // --- Tesla (Chain-Arc): Ativa, Eletrico. OnCast -> ChainDamage: o dano-base
        // (power) atinge o alvo primario e a descarga SALTA em 2 (magnitude) inimigos vivos
        // seguintes, retendo 62% (percent) a cada salto (decaimento multiplicativo) = 3
        // alvos no total. EXCECAO das especiais: precisa de power > 0 (a cadeia escala do
        // dano-base do primario, nao de um EffectSpec). O dano-base e turbinavel por item
        // futuro (capacitor serie/paralelo), feat separada. ---
        make_special(
            "tesla", "CARD_EXEC_TESLA_NAME", CardFamily::Eletrico, CardCategory::Ativa,
            kActiveManaCost,
            {EffectSpec{.trigger = TriggerHook::OnCast,
                       .kind = EffectKind::ChainDamage,
                       .magnitude = 2,  //PLAYTEST 2 saltos = 3 alvos.
                       .percent = kTeslaChainRetentionPercent}},
            /*ignores_weakness_wheel=*/false, /*power=*/kTeslaPower),

        // --- Einstein (Time-Dilate): Ativa, Cinetico. OnCast -> DelayAction: empurra a
        // acao do alvo pro FIM da fila da rodada corrente (magnitude 0), via
        // InitiativeQueue::reorder_actor (mesma primitiva do Gambito-Reordenar). Um alvo
        // que ja agiu nesta rodada dissipa a carta (no-op + log), nao banca pra proxima
        // rodada. Sem dano. ---
        make_special(
            "einstein", "CARD_EXEC_EINSTEIN_NAME", CardFamily::Cinetico, CardCategory::Ativa,
            kActiveManaCost,
            {EffectSpec{.trigger = TriggerHook::OnCast,
                       .kind = EffectKind::DelayAction,
                       .magnitude = kEinsteinDelayMagnitude}}),
    };

    std::unordered_map<std::string, Card> registry;
    registry.reserve(12);
    for (const auto& card : cards) {
        // emplace (nao indexer) falha-cedo se algum id duplicar (mesmo padrao do placeholder).
        const auto [it, inserted] = registry.emplace(card.id, card);
        (void)it;
        if (!inserted)
            throw std::logic_error("MasterCards: id de carta duplicado: " + card.id);
    }
    return registry;
}

}  // namespace

std::unordered_map<std::string, Card> build_registry() { return assemble(); }

}  // namespace gus::domain::combat::MasterCards
