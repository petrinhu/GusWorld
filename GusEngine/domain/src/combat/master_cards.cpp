// gus/domain/src/combat/master_cards.cpp
//
// Implementacao do catalogo das 10 cartas ESPECIAIS suportadas pelo executor techMagic
// (ADR-016, MVP steps 4-5). Ver header para o contrato. Mesmo padrao de
// placeholder_cards.cpp: cartas construidas uma vez, emplace fail-fast em id duplicado.
// POCO puro, ZERO Qt.
//
// Regra de familia (cartas-technomagik.md secao 2.3): dominio eletromagnetico -> Eletrico;
// resto -> Universal. Volta/Faraday/Euler = Eletrico; Newton/Pythagoras/Godel/Turing/
// Menger/Mandelbrot/Ada = Universal.
//
// NUMEROS PROVISORIOS (//PLAYTEST): mana das ativas/hibridas = 6; percent do Leech (Volta)
// = 50; percent do Reflect (Newton) = 30; percent do eco Fractal-Echo (Mandelbrot) = 50;
// percent do eco Re-Run (Ada) = 100/magnitude(chance) = 34. Balanceamento real
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
// RepeatLastAction (ADR-016 secao 20 item 5, decisoes Q1-Q4 do lider 2026-07-14).
constexpr int kMandelbrotEchoPercent = 50;  //PLAYTEST escala do eco Fractal-Echo (Mandelbrot).
constexpr int kAdaEchoPercent = 100;  // Q3: Ada ecoa a 100% - o freio dela e a CHANCE, nao a escala.
constexpr int kAdaEchoChance = 34;    //PLAYTEST chance% do Re-Run (Ada); easter-egg velado.

// Monta uma carta ESPECIAL base. base_type = Glifo (carta-programa nao-elemental,
// //PLAYTEST). ap_cost default 1 (herda o mesmo default do placeholder). Sem
// status_applied, sem modifiers, sem crit/mastery (defaults do struct).
Card make_special(std::string id,
                  std::string display_name,
                  CardFamily family,
                  CardCategory category,
                  int mana_cost,
                  std::vector<EffectSpec> effects,
                  bool ignores_weakness_wheel = false) {
    Card c;
    c.id = std::move(id);
    c.display_name = std::move(display_name);
    c.family = family;
    c.base_type = CardBaseType::Glifo;  //PLAYTEST (sem convencao fechada pra especiais).
    c.mana_cost = mana_cost;
    c.ap_cost = 1;
    c.power = 0;  // o dano/efeito vem do programa (EffectSpec), nao de power base.
    c.target_shape = TargetShape::Single;
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

        // --- Newton (Force-Law): Hibrida, Universal. OnCast -> Stun 1t no alvo (Poco
        // Gravitacional, imobiliza) + OnDamageReceived -> Reflect 30% (Acao e Reacao). ---
        make_special(
            "newton", "CARD_EXEC_NEWTON_NAME", CardFamily::Universal, CardCategory::Hibrida,
            kActiveManaCost,
            {EffectSpec{.trigger = TriggerHook::OnCast,
                       .kind = EffectKind::ApplyStatus,
                       .duration = 1,  //PLAYTEST duracao do Stun (1 turno).
                       .status = StatusId::Stun,
                       .stack_rule = StackRule::Replace},
             EffectSpec{.trigger = TriggerHook::OnDamageReceived,
                       .kind = EffectKind::Reflect,
                       .percent = kNewtonReflectPercent}}),

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

        // --- Godel (Null-Proof): Passiva, Universal, mana 0. Sem programa: o trunfo E a
        // flag ignores_weakness_wheel; wiring no resolvedor e step futuro. ---
        make_special(
            "godel", "CARD_EXEC_GODEL_NAME", CardFamily::Universal, CardCategory::Passiva, 0,
            /*effects=*/{}, /*ignores_weakness_wheel=*/true),

        // --- Faraday (EM-Shield): ForaDeCombate, Eletrico, mana 0. Posse-only (anti-PEM
        // + imunidade a debuff eletrico); query por id = feat futura, sem programa hoje. ---
        make_special("faraday", "CARD_EXEC_FARADAY_NAME", CardFamily::Eletrico,
                     CardCategory::ForaDeCombate, 0, /*effects=*/{}),

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
    };

    std::unordered_map<std::string, Card> registry;
    registry.reserve(10);
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
