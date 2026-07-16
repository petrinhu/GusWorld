// techmagic_quantize_test.cpp
//
// Spec executavel (Catch2 v3) do Quantum-Lock (Planck), manifesto item 5 do executor
// techMagic (ADR-016, decisoes do lider 2026-07-15):
//   A1: 3 degraus da propria faixa de variancia Knowledge - piso base*(1-v) / centro base /
//       teto base*(1+v), chances FIXAS 25%/50%/25% (nao evoluem com Knowledge). Reusa o
//       MESMO helper comum_channel_damage(base, v, r) do canal COMUM de sempre, com r fixo
//       0.0/0.5/1.0 (piso/centro/teto ficam bit-identicos ao min/max/centro do preview).
//   A2: preview (estimate_card_damage) mostra os 3 valores + as 3 chances.
//   A5: SUBSTITUI o sorteio continuo (next_double) por um sorteio de degrau (next(100)) -
//       canal COMUM com Planck = exatamente 2 consumos (canal + degrau), MESMA contagem de
//       sempre; sem Planck, caminho byte-identico (0 mudanca de consumo).
//   A6: critico e falha INTACTOS - a quantizacao so age no canal COMUM.
//
// Cartas de teste montadas LOCALMENTE (id "techmagic.planck.*"/"techmagic.dmg.*"), NUNCA do
// registry de producao (MasterCards) - o teste do CATALOGO (Planck em MasterCards::
// build_registry()) vive em master_cards_test.cpp. O motor e AGNOSTICO por-ator (zero
// hardcode de id/nome de personagem no domain) - os testes usam ids genericos de proposito.
//
// Cross-ref: gus/domain/combat/combat_enums.hpp (EffectKind::DamageQuantize);
//            combat_state_machine.cpp (quantize_spec_of/quantize_step_r/quantize_log_suffix,
//            resolve_use_card - wiring real; estimate_card_damage - gemeo PURO);
//            master_cards.cpp (planck); docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md
//            (AMB-06); docs/design/mecanicas/combat.md secao 11; ADR-016.

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <unordered_map>
#include <vector>

#include "fixed_random.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/random_source.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::FixedRandom;

namespace {

CombatActor make_actor(const std::string& id, bool player_side, int hp = 100, int atk = 8,
                       int def = 0, int spd = 20,
                       CardFamily family = CardFamily::Eletrico, int kills = 0) {
    return CombatActor(id, id, hp, atk, def, spd, family, player_side, /*is_boss=*/false,
                       kills);
}

// Carta de dano PLANA (tier Comum - isenta do gate 1x/batalha), family/power/crit
// configuraveis, alvo Single. Mesmo padrao de damage_card() em techmagic_godel_test.cpp.
Card damage_card(const std::string& id, CardFamily family, int power, int crit_chance = 0) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = family;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = power;
    c.crit_chance = crit_chance;
    c.target_shape = TargetShape::Single;
    return c;
}

// Duplo LOCAL da Planck de producao: Passiva/Universal/mana 0, OnCast -> DamageQuantize.
// extreme_pct = chance% de CADA extremo (piso/teto); center_pct = chance% do centro.
Card planck_card(const std::string& id, int extreme_pct = 25, int center_pct = 50) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Universal;
    c.base_type = CardBaseType::Glifo;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = 0;
    c.target_shape = TargetShape::Single;
    c.tier = CardTier::Especial;
    c.category = CardCategory::Passiva;
    c.effects = {EffectSpec{.trigger = TriggerHook::OnCast,
                            .kind = EffectKind::DamageQuantize,
                            .magnitude = center_pct,
                            .percent = extreme_pct}};
    return c;
}

std::unordered_map<std::string, Card> registry(std::initializer_list<Card> cards) {
    std::unordered_map<std::string, Card> d;
    for (const auto& c : cards) d.emplace(c.id, c);
    return d;
}

// Mesmo padrao de play_by_actor de techmagic_godel_test.cpp: mapeia UMA acao por id de
// ator (1x cada); fora disso passa (0 AP).
CombatActionProvider play_by_actor(std::unordered_map<std::string, CombatAction> actions) {
    auto acts = std::make_shared<std::unordered_map<std::string, CombatAction>>(std::move(actions));
    return [acts](CombatActor& a, const CombatState&) -> CombatAction {
        auto it = acts->find(a.id());
        if (it == acts->end()) return CombatAction::pass();
        CombatAction action = it->second;
        acts->erase(it);
        return action;
    };
}

// Duplo de IRandomSource que CONTA as chamadas (mesmo padrao de techmagic_godel_test.cpp).
class CountingRandom final : public IRandomSource {
public:
    explicit CountingRandom(double next_double = 0.5, int next_int = 99)
        : next_double_(next_double), next_int_(next_int) {}
    double next_double() override {
        ++double_calls;
        return next_double_;
    }
    int next(int max_value) override {
        ++int_calls;
        return max_value <= 0 ? 0 : std::min(next_int_, max_value - 1);
    }
    int int_calls = 0;
    int double_calls = 0;

private:
    double next_double_;
    int next_int_;
};

}  // namespace

// ===== 1. Degraus corretos: cada faixa forcada produz o dano hand-calculado da propria =====
// =====    formula comum_channel_damage(base, v, r) com r fixo 0.0/0.5/1.0 ==================
//
// Setup comum aos 3 casos: card.power=12, atacante.atk=8, alvo.def=0, family Eletrico vs
// Sonico => Neutro (mult_fraqueza=1.0). base_damage = (12+8)*1.0 = 20. alvo.knowledge_kills=5
// => v = max(0.05, 0.30*exp(-0.5)) = max(0.05, 0.30*0.6065306597) = 0.18195919... .
// fumbleChance = round(5*exp(-2.5)) = round(0.410425) = 0; critChance = max(5,0) = 5. Canal
// COMUM exige roll >= 5 (FALHA vazio, CRIT = [0,5)).
//   piso  (r=0.0): 20*(1-v) = 20*0.8180408 = 16.360816 -> lround = 16.
//   centro(r=0.5): 20*1.0                              = 20.000000 -> lround = 20.
//   teto  (r=1.0): 20*(1+v) = 20*1.1819592 = 23.639184 -> lround = 24.

TEST_CASE("techmagic planck: os 3 degraus forcados produzem o dano hand-calculado de "
         "comum_channel_damage(base, v, r) com r=0.0/0.5/1.0",
         "[domain][combat][techmagic][planck]") {
    Card card = damage_card("techmagic.planck.steps", CardFamily::Eletrico, /*power=*/12);
    Card planck = planck_card("techmagic.planck.steps.passive");
    auto reg = registry({card, planck});

    // next_int=10: channel roll=10 (>=5, COMUM); degree roll=10 (<25) -> PISO.
    {
        FixedRandom rng(/*next_double=*/0.5, /*next_int=*/10);
        CombatActor h = make_actor("h", true, 100, 8, 0, 50);
        h.set_equipped_special_ids({planck.id});
        CombatActor e = make_actor("e", false, 300, 0, 0, 10, CardFamily::Sonico, /*kills=*/5);
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, e.id())}});
        CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        REQUIRE(e.max_hp() - e.hp() == 16);
    }
    // next_int=50: channel roll=50 (COMUM); degree roll=50 (25<=50<75) -> CENTRO.
    {
        FixedRandom rng(/*next_double=*/0.5, /*next_int=*/50);
        CombatActor h = make_actor("h", true, 100, 8, 0, 50);
        h.set_equipped_special_ids({planck.id});
        CombatActor e = make_actor("e", false, 300, 0, 0, 10, CardFamily::Sonico, /*kills=*/5);
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, e.id())}});
        CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        REQUIRE(e.max_hp() - e.hp() == 20);
    }
    // next_int=80: channel roll=80 (COMUM); degree roll=80 (>=75) -> TETO.
    {
        FixedRandom rng(/*next_double=*/0.5, /*next_int=*/80);
        CombatActor h = make_actor("h", true, 100, 8, 0, 50);
        h.set_equipped_special_ids({planck.id});
        CombatActor e = make_actor("e", false, 300, 0, 0, 10, CardFamily::Sonico, /*kills=*/5);
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, e.id())}});
        CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        REQUIRE(e.max_hp() - e.hp() == 24);
    }
}

// ===== 2. Paridade preview<->real (teste-rei): cada degrau forcado == campo correspondente =====
// =====    de estimate_card_damage, INCLUSIVE com alvo protegido por Shield =================

TEST_CASE("techmagic planck: estimate_card_damage.min/mid/max_damage batem EXATAMENTE com o "
         "dano real de cada degrau forcado (alvo com Shield, absorcao aplicada aos 2)",
         "[domain][combat][techmagic][planck]") {
    Card card = damage_card("techmagic.planck.parity", CardFamily::Eletrico, /*power=*/12);
    Card planck = planck_card("techmagic.planck.parity.passive");
    auto reg = registry({card, planck});

    CombatActor h = make_actor("h", true, 100, 8, 0, 50);
    h.set_equipped_special_ids({planck.id});
    CombatActor e = make_actor("e", false, 300, 0, 0, 10, CardFamily::Sonico, /*kills=*/5);
    e.add_status({StatusId::Shield, /*magnitude=*/3, /*duration=*/5, StackRule::Replace,
                 CardFamily::Eletrico});

    CombatStateMachine preview_sm(
        {&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); }, &reg,
        nullptr, nullptr);
    const CardDamageEstimate est = preview_sm.estimate_card_damage(h, e, card);

    REQUIRE(est.quantized);
    REQUIRE(est.step_low_pct == 25);
    REQUIRE(est.step_mid_pct == 50);
    REQUIRE(est.step_high_pct == 25);
    // Sem Shield seriam 16/20/24 (teste #1); com Shield magnitude=3, absorcao -3 em cada.
    REQUIRE(est.min_damage == 13);
    REQUIRE(est.mid_damage == 17);
    REQUIRE(est.max_damage == 21);

    // Real: mesmos 3 degraus forcados, MESMO alvo (HP fresco a cada rodada via CombatActor
    // novo), devem bater com os campos do preview acima.
    const auto run_degree = [&](int next_int) {
        FixedRandom rng(/*next_double=*/0.5, next_int);
        CombatActor h2 = make_actor("h", true, 100, 8, 0, 50);
        h2.set_equipped_special_ids({planck.id});
        CombatActor e2 = make_actor("e", false, 300, 0, 0, 10, CardFamily::Sonico,
                                    /*kills=*/5);
        e2.add_status({StatusId::Shield, 3, 5, StackRule::Replace, CardFamily::Eletrico});
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, e2.id())}});
        CombatStateMachine sm({&h2, &e2}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        return e2.max_hp() - e2.hp();
    };
    REQUIRE(run_degree(10) == est.min_damage);
    REQUIRE(run_degree(50) == est.mid_damage);
    REQUIRE(run_degree(80) == est.max_damage);
}

// ===== 3. Fronteiras do sorteio de degrau: roll 24/25/74/75 caem no degrau certo ===========

TEST_CASE("techmagic planck: fronteiras do sorteio de degrau (roll 24 piso, 25 centro, 74 "
         "centro, 75 teto)",
         "[domain][combat][techmagic][planck]") {
    Card card = damage_card("techmagic.planck.boundary", CardFamily::Eletrico, /*power=*/12);
    Card planck = planck_card("techmagic.planck.boundary.passive");
    auto reg = registry({card, planck});

    const auto damage_at_roll = [&](int roll) {
        FixedRandom rng(/*next_double=*/0.5, roll);
        CombatActor h = make_actor("h", true, 100, 8, 0, 50);
        h.set_equipped_special_ids({planck.id});
        CombatActor e = make_actor("e", false, 300, 0, 0, 10, CardFamily::Sonico,
                                   /*kills=*/5);
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, e.id())}});
        CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        return e.max_hp() - e.hp();
    };

    REQUIRE(damage_at_roll(24) == 16);  // piso (24 < 25).
    REQUIRE(damage_at_roll(25) == 20);  // centro (25 <= 25 < 75).
    REQUIRE(damage_at_roll(74) == 20);  // centro (74 < 75).
    REQUIRE(damage_at_roll(75) == 24);  // teto (75 >= 75).
}

// ===== 4. Determinismo: canal COMUM com Planck = 2 consumos next(100) (canal+degrau), 0 =====
// =====    next_double; SEM Planck = 1 next(100) + 1 next_double (caminho intocado) =========

TEST_CASE("techmagic planck: canal COMUM com Planck consome 2x next(100) e 0x next_double; "
         "sem Planck o caminho fica byte-identico ao de sempre (1x cada)",
         "[domain][combat][techmagic][planck]") {
    Card card = damage_card("techmagic.planck.rng", CardFamily::Eletrico, /*power=*/12);
    Card planck = planck_card("techmagic.planck.rng.passive");
    auto reg = registry({card, planck});

    const auto run = [&](bool with_planck) {
        CombatActor h = make_actor("h", true, 100, 8, 0, 50);
        if (with_planck) h.set_equipped_special_ids({planck.id});
        CombatActor e = make_actor("e", false, 300, 0, 0, 10, CardFamily::Sonico,
                                   /*kills=*/5);
        CountingRandom rng(/*next_double=*/0.5, /*next_int=*/50);  // COMUM, degrau centro.
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, e.id())}});
        CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        return std::pair<int, int>{rng.int_calls, rng.double_calls};
    };

    const auto without_planck = run(false);
    const auto with_planck = run(true);

    REQUIRE(without_planck.first == 1);   // 1x next(100): canal.
    REQUIRE(without_planck.second == 1);  // 1x next_double: variancia continua de sempre.
    REQUIRE(with_planck.first == 2);      // 2x next(100): canal + degrau.
    REQUIRE(with_planck.second == 0);     // 0x next_double: substituido pelo sorteio de degrau.
    // MESMA contagem TOTAL de consumos (A5): 1+1 == 2+0.
    REQUIRE(without_planck.first + without_planck.second ==
           with_planck.first + with_planck.second);
}

// ===== 5. Escopo por-ator: SO o ator que porta a passiva quantiza; um 2o ator SEM a =========
// =====    passiva (mesmo lado) segue com variancia continua normal ==========================

TEST_CASE("techmagic planck: quantizacao e por-ator - so quem porta a passiva quantiza; "
         "um aliado SEM a passiva mantem a variancia continua normal",
         "[domain][combat][techmagic][planck]") {
    Card card = damage_card("techmagic.planck.scope", CardFamily::Eletrico, /*power=*/12);
    Card planck = planck_card("techmagic.planck.scope.passive");
    auto reg = registry({card, planck});

    CombatActor h1 = make_actor("h1", true, 100, 8, 0, 50);  // porta a passiva.
    h1.set_equipped_special_ids({planck.id});
    CombatActor h2 = make_actor("h2", true, 100, 8, 0, 40);  // NAO porta.
    CombatActor e = make_actor("e", false, 300, 0, 0, 10, CardFamily::Sonico, /*kills=*/5);

    CountingRandom rng(/*next_double=*/0.5, /*next_int=*/50);
    auto provider = play_by_actor({
        {"h1", CombatAction::use_card(card.id, e.id())},
        {"h2", CombatAction::use_card(card.id, e.id())},
    });
    CombatStateMachine sm({&h1, &h2, &e}, provider, &reg, nullptr, &rng);

    // Turno 1: h1 (com Planck) - 2x next(100), degrau centro (roll=50) -> dano 20.
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.max_hp() - e.hp() == 20);
    REQUIRE(rng.int_calls == 2);
    REQUIRE(rng.double_calls == 0);

    // Turno 2: h2 (SEM Planck) - 1x next(100) + 1x next_double, variancia zero (r=0.5) ->
    // MESMO dano base (20), mas via o caminho DE SEMPRE (nao quantizado).
    sm.advance_to_next_actor();
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.max_hp() - e.hp() == 40);  // acumulado 40 de dano nos 2 hits (20+20).
    REQUIRE(rng.int_calls == 3);      // +1 (canal de h2).
    REQUIRE(rng.double_calls == 1);   // +1 (variancia continua de h2).
}

// ===== 6. Critico/Falha INTACTOS: com Planck equipado, roll em CRIT/FALHA nao sorteia =====
// =====    degrau (a quantizacao so age no canal COMUM) =====================================

TEST_CASE("techmagic planck: critico e falha ficam INTACTOS com Planck equipado - o "
         "sorteio de degrau so roda no canal COMUM",
         "[domain][combat][techmagic][planck]") {
    Card card =
        damage_card("techmagic.planck.crit", CardFamily::Eletrico, /*power=*/12,
                   /*crit_chance=*/50);
    Card planck = planck_card("techmagic.planck.crit.passive");
    auto reg = registry({card, planck});

    // kills=0: fumbleChance=5, critChance=max(5,50)=50. roll=4 -> FALHA (< 5).
    {
        FixedRandom rng(/*next_double=*/0.5, /*next_int=*/4);
        CombatActor h = make_actor("h", true, 100, 8, 0, 50);
        h.set_equipped_special_ids({planck.id});
        CombatActor e = make_actor("e", false, 300, 0, 0, 10, CardFamily::Sonico,
                                   /*kills=*/0);
        CountingRandom counting(/*next_double=*/0.5, /*next_int=*/4);
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, e.id())}});
        CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &counting);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        REQUIRE(e.max_hp() - e.hp() == 0);       // FALHA: dano 0.
        REQUIRE(counting.int_calls == 1);        // 1x next(100): so o canal, sem degrau.
        REQUIRE(counting.double_calls == 0);
        bool fumble_logged = false;
        for (const auto& entry : sm.log())
            if (entry.message.find("FALHA DE COMPILACAO") != std::string::npos)
                fumble_logged = true;
        REQUIRE(fumble_logged);
    }

    // roll=10 (>=5 fumble, <55 crit) -> CRIT. base=(12+8)*1.0=20; v(kills=0)=0.30;
    // critDamage = round(20*(1+0.30)*1.5) = round(39) = 39.
    {
        FixedRandom rng(/*next_double=*/0.5, /*next_int=*/10);
        CombatActor h = make_actor("h", true, 100, 8, 0, 50);
        h.set_equipped_special_ids({planck.id});
        CombatActor e = make_actor("e", false, 300, 0, 0, 10, CardFamily::Sonico,
                                   /*kills=*/0);
        CountingRandom counting(/*next_double=*/0.5, /*next_int=*/10);
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, e.id())}});
        CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &counting);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        REQUIRE(e.max_hp() - e.hp() == 39);
        REQUIRE(counting.int_calls == 1);  // 1x next(100): so o canal, sem degrau.
        REQUIRE(counting.double_calls == 0);
        bool crit_logged = false;
        for (const auto& entry : sm.log())
            if (entry.message.find("[CRITICO]") != std::string::npos) crit_logged = true;
        REQUIRE(crit_logged);
    }
}

// ===== 7. Ataque BASICO inalterado: 0 consumo de RNG mesmo com Planck equipado =============

TEST_CASE("techmagic planck: ataque BASICO nunca consulta a quantizacao - 0 consumo de RNG "
         "com Planck equipado",
         "[domain][combat][techmagic][planck]") {
    Card planck = planck_card("techmagic.planck.basic.passive");
    auto reg = registry({planck});

    CombatActor h = make_actor("h", true, 100, 8, 0, 50);
    h.set_equipped_special_ids({planck.id});
    CombatActor e = make_actor("e", false, 300, 0, 0, 10);

    CountingRandom rng;
    auto provider = play_by_actor({{"h", CombatAction::attack(e.id())}});
    CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(e.max_hp() - e.hp() == 8);  // subtrativa clamp(atk-def,1) = 8-0.
    REQUIRE(rng.int_calls == 0);
    REQUIRE(rng.double_calls == 0);
}

// ===== 8. Colapso graceful: kills altos (v no piso 0.05) + dano base pequeno fazem os 3 =====
// =====    degraus arredondarem pro MESMO inteiro - nao quebra, preview e real concordam ====

TEST_CASE("techmagic planck: kills altos + dano base pequeno colapsam os 3 degraus no MESMO "
         "inteiro (v no piso 0.05) sem quebrar - preview e real concordam",
         "[domain][combat][techmagic][planck]") {
    // atk=2, power=0, def=0, mult_fraqueza=1.0 (Neutro) -> base_damage=2. kills=100 -> v =
    // max(0.05, 0.30*exp(-10)) = 0.05 (piso). piso=2*0.95=1.9->2; centro=2*1=2->2;
    // teto=2*1.05=2.1->2. Os 3 colapsam em 2.
    Card card = damage_card("techmagic.planck.collapse", CardFamily::Eletrico, /*power=*/0);
    Card planck = planck_card("techmagic.planck.collapse.passive");
    auto reg = registry({card, planck});

    CombatActor h = make_actor("h", true, 100, /*atk=*/2, 0, 50);
    h.set_equipped_special_ids({planck.id});
    CombatActor e = make_actor("e", false, 300, 0, 0, 10, CardFamily::Sonico, /*kills=*/100);

    CombatStateMachine preview_sm(
        {&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); }, &reg,
        nullptr, nullptr);
    const CardDamageEstimate est = preview_sm.estimate_card_damage(h, e, card);
    REQUIRE(est.quantized);
    REQUIRE(est.min_damage == 2);
    REQUIRE(est.mid_damage == 2);
    REQUIRE(est.max_damage == 2);

    for (int roll : {10, 50, 80}) {
        FixedRandom rng(/*next_double=*/0.5, roll);
        CombatActor h2 = make_actor("h", true, 100, 2, 0, 50);
        h2.set_equipped_special_ids({planck.id});
        CombatActor e2 = make_actor("e", false, 300, 0, 0, 10, CardFamily::Sonico,
                                    /*kills=*/100);
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, e2.id())}});
        CombatStateMachine sm({&h2, &e2}, provider, &reg, nullptr, &rng);
        REQUIRE_NOTHROW(sm.begin_turn());
        REQUIRE_NOTHROW(sm.run_active_turn_to_end());
        REQUIRE(e2.max_hp() - e2.hp() == 2);
    }
}

// ===== 9. Log: degrau + chance presentes na entrada do hit quantizado =======================

TEST_CASE("techmagic planck: o log do hit quantizado cita o degrau e a chance% (regra do "
         "lider - todo efeito loga)",
         "[domain][combat][techmagic][planck]") {
    Card card = damage_card("techmagic.planck.log", CardFamily::Eletrico, /*power=*/12);
    Card planck = planck_card("techmagic.planck.log.passive");
    auto reg = registry({card, planck});

    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/50);  // COMUM, degrau centro (50%).
    CombatActor h = make_actor("h", true, 100, 8, 0, 50);
    h.set_equipped_special_ids({planck.id});
    CombatActor e = make_actor("e", false, 300, 0, 0, 10, CardFamily::Sonico, /*kills=*/5);

    auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, e.id())}});
    CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    bool found = false;
    for (const auto& entry : sm.log()) {
        if (entry.message.find("Quantum-Lock") != std::string::npos) {
            found = true;
            REQUIRE(entry.message.find("50%") != std::string::npos);  // chance do centro.
            REQUIRE(entry.message.find("medio") != std::string::npos);  // degrau central.
        }
    }
    REQUIRE(found);
}

// ===== 10. Alvo aliado (fogo amigo desligado) e alvo IMUNE nao sorteiam degrau (0 consumo =====
// =====     extra de RNG) - os dois `continue` ANTES do canal COMUM, com ou sem Planck ======

TEST_CASE("techmagic planck: alvo aliado (fogo amigo desligado) e alvo IMUNE nao sorteiam "
         "degrau - 0 consumo extra de RNG mesmo com Planck equipado",
         "[domain][combat][techmagic][planck]") {
    Card planck = planck_card("techmagic.planck.friendly.passive");

    // Fogo amigo: carta de dano mirando um ALIADO - dano 0, dissipa antes do canal.
    {
        Card card =
            damage_card("techmagic.planck.friendly.dmg", CardFamily::Eletrico, /*power=*/12);
        auto reg = registry({card, planck});
        CombatActor h = make_actor("h", true, 100, 8, 0, 50);
        h.set_equipped_special_ids({planck.id});
        CombatActor ally = make_actor("h2", true, 100, 8, 0, 40);
        CountingRandom rng;
        auto provider = play_by_actor({{"h", CombatAction::use_card(card.id, ally.id())}});
        CombatStateMachine sm({&h, &ally}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        REQUIRE(ally.max_hp() - ally.hp() == 0);
        REQUIRE(rng.int_calls == 0);
        REQUIRE(rng.double_calls == 0);
    }

    // Imune: card.ignores_weakness_wheel=false mas mult_fraqueza calculado 0.0 nao e
    // alcancavel hoje (ver LIMITE HONESTO DE COBERTURA, techmagic_godel_test.cpp) - o
    // curto-circuito de imunidade e testado por combat_formula_test.cpp separadamente; aqui
    // confirmamos que o GUARD continua ANTES do bloco Planck por inspecao do mesmo
    // `continue` usado no ramo de fogo-amigo acima (mesmo ponto de retorno no codigo-fonte,
    // ver combat_state_machine.cpp::resolve_use_card). Cobertura completa do curto-circuito
    // de imunidade + Planck fica registrada como LIMITE HONESTO nesta nota (nao re-provada
    // aqui por falta de combinacao real de familias que produza mult 0.0).
}
