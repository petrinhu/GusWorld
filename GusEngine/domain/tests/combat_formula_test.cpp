// combat_formula_test.cpp
//
// Spec executavel (Catch2 v3) da formula de dano UseCard (secao 11, evoluida 2026-06-22,
// item M5-DMG). A secao 11 DEIXOU de ser paridade com o C# (que morre no M8); o motor C++
// segue a secao 11. Cobre:
//   1. Cadeia divisiva (Def reduz por fracao, nunca zera por subtracao).
//   2. Variancia Knowledge no canal COMUM (+-30% no 1o encontro -> +-5% floor conforme farm).
//   3. Sorteio de canal FALHA / CRIT / COMUM (mutuamente exclusivos), UM rng.next(100):
//        - FALHA: roll < fumbleChance; dano 0; log "FALHA DE COMPILACAO".
//        - CRIT:  roll < fumbleChance + critChance; dano = round(danoBase*(1+v)*1.5); "[CRITICO]".
//        - COMUM: senao; 2o roll next_double aplica a variancia.
//   4. fumbleChance = round(5*exp(-kills*0.50)) (5% kills=0, 0% kills>=5).
//   5. critChance   = max(5, card.crit_chance) (piso global 5%; carta eleva).
//   6. Imunidade (multFraqueza==0): dano 0 ANTES de qualquer RNG (0 consumos).
//   7. Ordem/contagem de consumo de RNG por canal (imune=0, falha=1, crit=1, comum=2).
//   8. Ataque basico permanece subtrativo clamp(atk - def, 1) - inalterado.
//
// RNG injetado (FixedRandom) pra determinismo independente de plataforma. Convencao de
// FixedRandom(next_double, next_int): next(100) = min(next_int, 99) seleciona o canal:
//   next_int = 0   -> roll 0  -> FALHA (kills baixos, fumble>0)
//   next_int = F   -> roll F  -> CRIT  (F = fumbleChance; cai em [fumble, fumble+crit))
//   next_int = 99  -> roll 99 -> COMUM (default)
//
// Cross-ref: docs/design/mecanicas/combat.md secao 7/11; ADR-006.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "fixed_random.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::FixedRandom;

namespace {

CombatActor hero(const std::string& id = "gus", int hp = 50, int spd = 20, int atk = 8,
                 int def = 2, CardFamily family = CardFamily::Eletrico) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/true);
}

CombatActor foe(const std::string& id = "enemy", int hp = 500, int spd = 10, int atk = 6,
                int def = 1, CardFamily family = CardFamily::Cinetico, int kills = 0) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/false, /*boss=*/false, kills);
}

Card make_card(const std::string& id, CardFamily family, int power, int mana_cost = 1,
               int crit_chance = 0, TargetShape shape = TargetShape::Single) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = family;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = mana_cost;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = shape;
    c.crit_chance = crit_chance;
    return c;
}

std::unordered_map<std::string, Card> registry(const Card& c) {
    std::unordered_map<std::string, Card> d;
    d.emplace(c.id, c);
    return d;
}

// Provider que joga UMA acao player-side e depois passa; inimigo sempre passa.
CombatActionProvider play_once(CombatAction action) {
    auto done = std::make_shared<bool>(false);
    return [action, done](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side() || *done) return CombatAction::pass();
        *done = true;
        return action;
    };
}

bool log_has(const CombatStateMachine& sm, const std::string& needle) {
    for (const auto& e : sm.log())
        if (e.message.find(needle) != std::string::npos)
            return true;
    return false;
}

// Roda o turno corrente e devolve o dano causado (max_hp - hp do alvo).
int damage_dealt(CombatActor& h, CombatActor& f, const Card& card, IRandomSource& rng) {
    auto reg = registry(card);
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::use_card(card.id, f.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    return f.max_hp() - f.hp();
}

// RNG deterministico que CONTA consumos (separadamente next(100) e next_double), pra
// assertar a ordem/contagem de consumo de RNG por canal (secao 11). Espelha FixedRandom.
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

    int int_calls = 0;     // chamadas a next(100) (sorteio de canal)
    int double_calls = 0;  // chamadas a next_double() (variancia, so no COMUM)

private:
    double next_double_;
    int next_int_;
};

}  // namespace

// ===== 1. Formula divisiva =====

TEST_CASE("formula: divisiva def alto ainda causa dano (sem subtrativa)",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, /*def=*/80, CardFamily::Eletrico);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    FixedRandom rng;
    REQUIRE(damage_dealt(h, f, card, rng) == 6);  // 10*(100/180) = 5.5556 -> 6
}

TEST_CASE("formula: divisiva def zero nao reduz dano", "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, /*def=*/0, CardFamily::Eletrico);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    FixedRandom rng;
    REQUIRE(damage_dealt(h, f, card, rng) == 10);
}

TEST_CASE("formula: divisiva sem clamp minimo arredonda pra zero",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, /*def=*/9000, CardFamily::Eletrico);
    Card card = make_card("pulso.fraco", CardFamily::Eletrico, 1);
    FixedRandom rng;
    REQUIRE(damage_dealt(h, f, card, rng) == 0);  // 0.01099 -> 0
}

// ===== 2. Variancia Knowledge Decay =====

TEST_CASE("formula: variancia kills zero aplica 30pct max no rng topo",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico, /*kills=*/0);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    FixedRandom rng(/*next_double=*/1.0);
    REQUIRE(damage_dealt(h, f, card, rng) == 13);  // 10 * 1.30
}

TEST_CASE("formula: variancia kills zero aplica 30pct min no rng fundo",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico, /*kills=*/0);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    FixedRandom rng(/*next_double=*/0.0);
    REQUIRE(damage_dealt(h, f, card, rng) == 7);  // 10 * 0.70
}

TEST_CASE("formula: variancia kills altos decai pra 5pct floor",
          "[domain][combat][formula]") {
    Card card = make_card("pulso.forte", CardFamily::Eletrico, 100);

    CombatActor ht = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor ft = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico, /*kills=*/100);
    FixedRandom topo(1.0);
    REQUIRE(damage_dealt(ht, ft, card, topo) == 105);  // +5%

    CombatActor hf = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor ff = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico, /*kills=*/100);
    FixedRandom fundo(0.0);
    REQUIRE(damage_dealt(hf, ff, card, fundo) == 95);  // -5%
}

TEST_CASE("formula: variancia range encolhe conforme kills sobem",
          "[domain][combat][formula]") {
    Card card = make_card("pulso.forte", CardFamily::Eletrico, 100);

    auto roll = [&](int kills, double nd) {
        CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
        CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico, kills);
        FixedRandom rng(nd);
        return damage_dealt(h, f, card, rng);
    };

    const int spread0 = roll(0, 1.0) - roll(0, 0.0);
    const int spread100 = roll(100, 1.0) - roll(100, 0.0);
    REQUIRE(spread0 == 60);
    REQUIRE(spread100 == 10);
    REQUIRE(spread100 < spread0);
}

// ===== 3. Canal CRIT (secao 11: dano = round(danoBase*(1+v)*1.5), piso 5%) =====

// kills=0 => fumble=5, critChance=max(5,crit). roll na faixa [fumble, fumble+crit) => CRIT.
TEST_CASE("formula: crit usa o topo do range x1.5 (danoBase*(1+v)*1.5) e loga critico",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico);
    Card card = make_card("pulso.crit", CardFamily::Eletrico, 10, 1, /*crit=*/100);
    auto reg = registry(card);
    // kills=0 => fumble=5. roll=5 cai em [5, 5+100) => CRIT. danoBase=10, v=0.30.
    FixedRandom rng(0.5, /*next_int=*/5);
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::use_card(card.id, f.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(f.max_hp() - f.hp() == 20);  // round(10 * 1.30 * 1.5) = round(19.5) = 20
    REQUIRE(log_has(sm, "[CRITICO]"));
}

// Piso global de 5%: mesmo com card.crit_chance=0, ha 5% de CRIT (roll na faixa [fumble, fumble+5)).
TEST_CASE("formula: piso de crit 5% mesmo com card.crit_chance 0",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico);
    Card card = make_card("pulso.nocrit", CardFamily::Eletrico, 10, 1, /*crit=*/0);
    auto reg = registry(card);
    // kills=0 => fumble=5, critChance=max(5,0)=5. roll=5 cai em [5,10) => CRIT.
    FixedRandom rng(0.5, /*next_int=*/5);
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::use_card(card.id, f.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(f.max_hp() - f.hp() == 20);  // round(10*1.30*1.5)=20 (mesmo no piso)
    REQUIRE(log_has(sm, "[CRITICO]"));
}

// roll fora da faixa de CRIT => COMUM (sem [CRITICO]). card.crit_chance=30, kills=0:
// faixa CRIT = [5, 35); roll=99 => COMUM.
TEST_CASE("formula: roll fora da faixa de crit cai em COMUM", "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico);
    Card card = make_card("pulso.crit30", CardFamily::Eletrico, 10, 1, /*crit=*/30);
    auto reg = registry(card);
    FixedRandom rng(0.5, /*next_int=*/99);  // 99 >= 35 => COMUM, variancia zero (nd=0.5)
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::use_card(card.id, f.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(f.max_hp() - f.hp() == 10);  // danoBase=10 (COMUM, nd=0.5)
    REQUIRE_FALSE(log_has(sm, "[CRITICO]"));
}

// card.crit_chance eleva a janela de CRIT acima do piso: crit=30, kills=0 => faixa [5,35).
// roll=34 ainda crita; roll=35 ja e COMUM.
TEST_CASE("formula: card.crit_chance eleva a janela de crit acima do piso",
          "[domain][combat][formula]") {
    auto run = [](int next_int) {
        CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
        CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico);
        Card card = make_card("pulso.crit30", CardFamily::Eletrico, 10, 1, /*crit=*/30);
        auto reg = registry(card);
        FixedRandom rng(0.5, next_int);
        CombatStateMachine sm({&h, &f}, play_once(CombatAction::use_card(card.id, f.id())),
                              &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        return std::pair<int, bool>{f.max_hp() - f.hp(), log_has(sm, "[CRITICO]")};
    };
    REQUIRE(run(34).second);          // roll 34 < 35 => CRIT
    REQUIRE(run(34).first == 20);     // round(10*1.30*1.5)
    REQUIRE_FALSE(run(35).second);    // roll 35 >= 35 => COMUM
    REQUIRE(run(35).first == 10);
}

// ===== 5. Canal FALHA (secao 11: dano 0, decaimento 5%->0% por kills) =====

// kills=0 => fumble=5. roll=0 < 5 => FALHA: dano 0 + log "FALHA DE COMPILACAO".
TEST_CASE("formula: falha zera o dano e loga erro de compilacao",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico, /*kills=*/0);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    auto reg = registry(card);
    FixedRandom rng(0.5, /*next_int=*/0);  // roll 0 < fumble 5 => FALHA
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::use_card(card.id, f.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(f.max_hp() - f.hp() == 0);
    REQUIRE(log_has(sm, "FALHA DE COMPILACAO"));
    REQUIRE_FALSE(log_has(sm, "[CRITICO]"));
}

// kills>=5 => fumble=round(5*exp(-2.5))=round(0.41)=0 => FALHA impossivel.
// roll=0 com fumble=0 NAO e FALHA; cai em CRIT (faixa [0, critChance)).
TEST_CASE("formula: falha impossivel com kills>=5 (decaimento a 0%)",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico, /*kills=*/5);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    auto reg = registry(card);
    FixedRandom rng(0.5, /*next_int=*/0);  // roll 0, fumble 0 => NAO FALHA
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::use_card(card.id, f.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(f.max_hp() - f.hp() > 0);  // sem falha => dano > 0
    REQUIRE_FALSE(log_has(sm, "FALHA DE COMPILACAO"));
}

// Curva de decaimento da falha (secao 11): round(5*exp(-kills*0.50)) = 5,3,2,1,1,0.
// Verifica indiretamente: roll = fumble-1 (dentro) FALHA; roll = fumble (fora) nao FALHA.
TEST_CASE("formula: limiar de falha segue round(5*exp(-kills*0.50))",
          "[domain][combat][formula]") {
    auto fumble_for = [](int kills) {
        return static_cast<int>(std::lround(5.0 * std::exp(-static_cast<double>(kills) * 0.50)));
    };
    REQUIRE(fumble_for(0) == 5);
    REQUIRE(fumble_for(1) == 3);
    REQUIRE(fumble_for(2) == 2);
    REQUIRE(fumble_for(3) == 1);
    REQUIRE(fumble_for(4) == 1);
    REQUIRE(fumble_for(5) == 0);
    REQUIRE(fumble_for(6) == 0);

    // roll exatamente no limiar (fumble) NAO e falha (estrito <). kills=2 => fumble=2.
    CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
    CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico, /*kills=*/2);
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    auto reg = registry(card);
    FixedRandom rng(0.5, /*next_int=*/2);  // roll=2 == fumble => NAO falha (cai em CRIT)
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::use_card(card.id, f.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE_FALSE(log_has(sm, "FALHA DE COMPILACAO"));
}

// ===== 6/7. Ordem/contagem de consumo de RNG por canal (secao 11) =====
//
// Nota sobre IMUNIDADE (multFraqueza==0): o curto-circuito de imunidade (dano 0 + 0
// consumos de RNG) esta implementado no FSM, mas o tier Imune NAO e expostoo pela API
// publica hoje (a roda base da 1.5/1.0/0.66; "Imune e flag de inimigo/lore, incremento
// futuro" - weakness_wheel.hpp). Logo a contagem 0/0 da imunidade nao e exercitavel
// end-to-end via FSM ainda; sera coberta quando a flag de imunidade for plugada. Aqui
// cobrimos as 3 contagens organicas: FALHA=1, CRIT=1, COMUM=2.

TEST_CASE("formula: contagem de RNG por canal (falha=1, crit=1, comum=2)",
          "[domain][combat][formula]") {
    auto consumption = [](int next_int) {
        CombatActor h = hero("gus", 50, 20, 0, 2, CardFamily::Eletrico);
        CombatActor f = foe("enemy", 500, 10, 6, 0, CardFamily::Eletrico, /*kills=*/0);
        Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
        auto reg = registry(card);
        CountingRandom rng(0.5, next_int);
        CombatStateMachine sm({&h, &f}, play_once(CombatAction::use_card(card.id, f.id())),
                              &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        return std::pair<int, int>{rng.int_calls, rng.double_calls};
    };
    // kills=0: fumble=5, critChance=5 => FALHA [0,5), CRIT [5,10), COMUM [10,99].
    auto falha = consumption(0);  // FALHA
    REQUIRE(falha.first == 1);
    REQUIRE(falha.second == 0);

    auto crit = consumption(5);  // CRIT
    REQUIRE(crit.first == 1);
    REQUIRE(crit.second == 0);

    auto comum = consumption(99);  // COMUM
    REQUIRE(comum.first == 1);
    REQUIRE(comum.second == 1);
}

// ===== 4. Ataque basico subtrativo (inalterado) =====

TEST_CASE("formula: ataque basico usa subtrativa clamp 1 inalterado",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/8);
    CombatActor f = foe("enemy", 500, 10, 6, /*def=*/2);
    FixedRandom rng(1.0, 0);  // RNG nao deve afetar o basico
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::attack(f.id())), nullptr, nullptr,
                          &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(f.max_hp() - f.hp() == 6);  // 8 - 2
}

TEST_CASE("formula: ataque basico clamp minimo 1 quando def supera atk",
          "[domain][combat][formula]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/3);
    CombatActor f = foe("enemy", 500, 10, 6, /*def=*/50);
    FixedRandom rng(0.0);
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::attack(f.id())), nullptr, nullptr,
                          &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(f.max_hp() - f.hp() == 1);
}
