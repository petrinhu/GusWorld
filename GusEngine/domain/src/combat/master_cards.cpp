// gus/domain/src/combat/master_cards.cpp
//
// Implementacao do catalogo das 15 cartas ESPECIAIS suportadas pelo executor techMagic
// (ADR-016, MVP steps 4-8 + manifesto itens 5-6, 12). Ver header para o contrato. Mesmo padrao de
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
// Maxwell (Spectra-Wave, manifesto item 12, decisao do lider 2026-07-16, AMB-08): Hibrida,
// Eletrico, TargetShape::Grupo. NENHUM EffectSpec (effects vazio) - "Onda Unificada" reusa
// PURO o caminho Grupo/AoE ja pronto do Newton (resolve_targets/resolve_use_card): dano em
// TODOS os inimigos vivos do lado oposto, via a cadeia divisiva de sempre (power + ATK do
// conjurador), zero EffectKind novo. Modo-aliado dissipa de graca (a regra geral "fogo
// amigo desligado" ja zera o dano nesse ramo; sem EffectSpec, nao ha nada pra rodar ali -
// no-op puro). Hibrida pela FACE irma fora-de-combate ("iluminar areas escuras" na
// exploracao) - STUB posse-only, ZERO codigo aqui (feat futura separada,
// MAXWELL-AREAS-ESCURAS, e decisao de design do criador, nao deste executor). Gate
// 1x/batalha via specials_cast_ (mesmo mecanismo generico das outras Ativa/Hibrida).
//
// NUMEROS PROVISORIOS (//PLAYTEST): mana das ativas/hibridas = 6; percent do Leech (Volta)
// = 50; percent do Reflect (Newton) = 30; percent do eco Fractal-Echo (Mandelbrot) = 50;
// percent do eco Re-Run (Ada) = 100/magnitude(chance) = 34; magnitude do DelayAction
// (Einstein) = 0 (fim da fila); power do Spectra-Wave (Maxwell) = 5. Balanceamento real
// (VOLTA-LEECH-%, statline por carta) e trabalho futuro.
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
// Quantum-Lock (Planck, manifesto item 5, decisoes do lider 2026-07-15): chances FIXAS dos
// 3 degraus do canal COMUM. kPlanckStepCenterPercent = chance% do degrau CENTRAL;
// kPlanckStepExtremePercent = chance% de CADA extremo (piso E teto, simetrico). Soma =
// 100% (25+50+25). Numeros fechados pelo lider (nao //PLAYTEST-livre como os outros).
constexpr int kPlanckStepCenterPercent = 50;
constexpr int kPlanckStepExtremePercent = 25;
// Scrying (John Dee/Black-Mirror, manifesto item 6, decisao D3 do lider 2026-07-15):
// duracao do buff Scrying (numero FECHADO pelo lider, nao //PLAYTEST-livre como a maioria
// dos outros - mesma categoria de kPlanckStepCenterPercent/kPlanckStepExtremePercent acima).
constexpr int kDeeScryingDuration = 3;
// Onda Unificada (Maxwell/Spectra-Wave, manifesto item 12, decisao AUTONOMA do
// orquestrador 2026-07-16, AMB-08): dano-base do AoE puro (SEM EffectSpec, reusa a cadeia
// divisiva de sempre via card.power). AoE cheio a 8 (mesmo power-base do primario da
// Coil-Arc/Tesla) eclipsaria o nicho dela; 5 preserva a diferenca (Tesla = burst
// concentrado decaindo por salto; Maxwell = dano uniforme mais fraco espalhado em TODOS os
// inimigos). //PLAYTEST - decisao a confirmar no playtest real, mesma categoria das outras
// constantes acima (NAO e um numero fechado pelo lider, ao contrario de
// kPlanckStepCenterPercent/kDeeScryingDuration).
constexpr int kMaxwellPower = 5;  //PLAYTEST

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
        // InitiativeQueue::reorder_pending (mesma primitiva do Gambito-Reordenar; migrada
        // de reorder_actor em COMBATE-FILA-CURSOR-FIX 2026-07-15). Um alvo que ja agiu
        // nesta rodada dissipa a carta (no-op + log), nao banca pra proxima rodada. Sem
        // dano. ---
        make_special(
            "einstein", "CARD_EXEC_EINSTEIN_NAME", CardFamily::Cinetico, CardCategory::Ativa,
            kActiveManaCost,
            {EffectSpec{.trigger = TriggerHook::OnCast,
                       .kind = EffectKind::DelayAction,
                       .magnitude = kEinsteinDelayMagnitude}}),

        // --- Planck (Quantum-Lock): Passiva, Universal, mana 0 (manifesto item 5, decisoes
        // do lider 2026-07-15). NAO executa via techMagic::execute (marcador no-op, ver
        // techmagic.cpp::handle_damage_quantize) - a quantizacao pluga DIRETO no canal
        // COMUM do resolvedor/preview (combat_state_machine.cpp::quantize_spec_of): 3
        // degraus fixos da propria faixa da variancia Knowledge (piso/centro/teto = r
        // 0.0/0.5/1.0 em comum_channel_damage), chances FIXAS 25%/50%/25%
        // (kPlanckStepExtremePercent/kPlanckStepCenterPercent). Media = danoBase (zero
        // mudanca de balance); Knowledge continua mandando na LARGURA dos degraus, as
        // chances NAO evoluem com kills. Carta HISTORICA (so o Gus equipa, decisao
        // narrativa fora do motor - o motor e agnostico por-ator, mesmo padrao do
        // Reflect/Newton: nenhum hardcode de id/nome de personagem aqui). ---
        make_special(
            "planck", "CARD_EXEC_PLANCK_NAME", CardFamily::Universal, CardCategory::Passiva,
            0,
            {EffectSpec{.trigger = TriggerHook::OnCast,
                       .kind = EffectKind::DamageQuantize,
                       .magnitude = kPlanckStepCenterPercent,  // chance% do degrau CENTRAL.
                       .percent = kPlanckStepExtremePercent}}),  // chance% de CADA extremo.

        // --- John Dee (Black-Mirror/Scrying): Hibrida, Universal, TargetShape::Self
        // (manifesto item 6, decisoes D1-D4 do lider 2026-07-15, AMB-07). Duas faces:
        //   OnCast -> RevealIntent (ATIVA/"Scrying"): aplica o buff Scrying (dur
        //     kDeeScryingDuration, Refresh) no PROPRIO caster + dump read-only do
        //     IntentPreview de cada inimigo vivo. Enquanto QUALQUER aliado vivo portar
        //     Scrying, a FSM RE-DUMPA na fronteira de rodada
        //     (CombatStateMachine::process_scrying_hooks) - NAO via este EffectSpec de
        //     novo (Scrying e status, nao carta equipada). D2: intent CAOTICO
        //     (Patch-Zero) retorna ruido, preserva a one-way door do boss.
        //   Scan aprimorado ("Black-Mirror"/PASSIVA, D1-ii): quando esta carta esta
        //     EQUIPADA (has_reveal_intent_equipped, combat_state_machine.cpp), a acao
        //     Scan passa a revelar TAMBEM status ativos + posicao na fila + intent
        //     previsto do alvo escaneado. So dados que JA existem no modelo (nenhum
        //     atributo oculto novo). Wiring FORA do dispatcher techMagic::execute (mesmo
        //     padrao de Planck/DamageQuantize) - resolve_scan chama techMagic::
        //     log_intent_for direto.
        //   MUNDO (D1-i, "Espelho Negro" original do roster - baus/passagens ocultas na
        //     exploracao): NAO existe sistema de ocultos no overworld ainda -> STUB
        //     posse-only (mesmo padrao de Euler/Turing/Menger acima), ZERO codigo de
        //     combate aqui. Ponto de extensao: quando o sistema de ocultos existir, a
        //     query e "a party tem 'dee' em equipped_special_ids()?" (mesmo mecanismo
        //     ja usado por has_reveal_intent_equipped) - NENHUM campo/EffectKind novo
        //     necessario.
        // 1x/batalha via specials_cast_ (Ativa/Hibrida, secao 2.1) - mesmo gate das
        // outras Ativa/Hibrida. ---
        make_special(
            "dee", "CARD_EXEC_DEE_NAME", CardFamily::Universal, CardCategory::Hibrida,
            kActiveManaCost,
            {EffectSpec{.trigger = TriggerHook::OnCast,
                       .kind = EffectKind::RevealIntent,
                       .duration = kDeeScryingDuration,
                       .status = StatusId::Scrying,
                       .stack_rule = StackRule::Refresh}},
            /*ignores_weakness_wheel=*/false, /*power=*/0, /*target_shape=*/TargetShape::Self),

        // --- Maxwell (Spectra-Wave): Hibrida, Eletrico, Grupo (manifesto item 12, decisao
        // do lider 2026-07-16, AMB-08). NENHUM EffectSpec: "Onda Unificada" e dano-base PURO
        // (power=kMaxwellPower + ATK do conjurador, cadeia divisiva de sempre) em TODOS os
        // inimigos vivos do lado oposto, via o MESMO caminho Grupo/AoE do Newton
        // (resolve_targets/resolve_use_card) - zero EffectKind novo, zero wiring novo no
        // resolvedor. Modo-aliado (mirar um aliado) dissipa: a regra geral "fogo amigo
        // desligado" ja zera o dano nesse ramo e, sem EffectSpec, nao ha OnCast pra rodar -
        // no-op puro (dano 0 + log de dissipacao, nada mais). Hibrida pela FACE irma
        // fora-de-combate "iluminar areas escuras" (exploracao) - STUB posse-only, FEAT
        // FUTURA separada (MAXWELL-AREAS-ESCURAS), ZERO codigo aqui. ---
        make_special(
            "maxwell", "CARD_EXEC_MAXWELL_NAME", CardFamily::Eletrico, CardCategory::Hibrida,
            kActiveManaCost, /*effects=*/{}, /*ignores_weakness_wheel=*/false,
            /*power=*/kMaxwellPower, /*target_shape=*/TargetShape::Grupo),
    };

    std::unordered_map<std::string, Card> registry;
    registry.reserve(15);
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
