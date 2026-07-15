// combat_status_effects_test.cpp
//
// Spec executavel (Catch2 v3) dos status de combate (F2-E.5b): Silence, Disrupt, Break,
// Decrypt, Haste/Slow, Knockback. Portado de
// engine/tests/turn_combat/CombatStatusEffectsTests.cs. POCO puro, ZERO Qt.
//
// RNG cravado (FixedRandom 0.5) zera a variancia => dano deterministico.
//
// Cross-ref: engine/tests/turn_combat/CombatStatusEffectsTests.cs;
//            docs/design/mecanicas/combat.md secao 8/9/10/15.

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
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

CombatActor hero(const std::string& id = "gus", int hp = 100, int spd = 20, int atk = 8,
                 int def = 4, CardFamily family = CardFamily::Eletrico) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/true);
}

CombatActor foe(const std::string& id = "enemy", int hp = 500, int spd = 10, int atk = 6,
                int def = 4, CardFamily family = CardFamily::Cinetico) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/false);
}

Card make_card(const std::string& id, CardFamily family, int power, int mana_cost = 1,
               std::optional<StatusEffect> status_applied = std::nullopt) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = family;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = mana_cost;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = TargetShape::Single;
    c.status_applied = std::move(status_applied);
    return c;
}

std::unordered_map<std::string, Card> registry(const Card& c) {
    std::unordered_map<std::string, Card> d;
    d.emplace(c.id, c);
    return d;
}

StatusEffect effect(StatusId id, int mag, int dur, CardFamily origin) {
    return StatusEffect{id, mag, dur, StackRule::Replace, origin};
}

CombatActionProvider play_once(CombatAction action) {
    auto done = std::make_shared<bool>(false);
    return [action, done](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side() || *done) return CombatAction::pass();
        *done = true;
        return action;
    };
}

const StatusEffect* find_status(const CombatActor& a, StatusId id) {
    for (const auto& s : a.status_effects())
        if (s.id == id) return &s;
    return nullptr;
}

std::vector<std::string> order_ids(const CombatStateMachine& sm) {
    std::vector<std::string> ids;
    for (const CombatActor* a : sm.queue().order()) ids.push_back(a->id());
    return ids;
}

bool log_has(const CombatStateMachine& sm, CombatActionType action, const std::string& needle) {
    for (const auto& e : sm.log())
        if (e.action == action && e.message.find(needle) != std::string::npos) return true;
    return false;
}

}  // namespace

// ===== SILENCE =====

TEST_CASE("status: silence bloqueia usecard com erro de compilacao",
          "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, 20, 8, 4, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 200, 10, 6, /*def=*/0);
    h.add_status(effect(StatusId::Silence, 0, 2, CardFamily::Sonico));
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    auto reg = registry(card);
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    REQUIRE_THROWS_AS(sm.run_active_turn_to_end(), std::logic_error);
    REQUIRE(e.hp() == 200);
}

TEST_CASE("status: silence permite ataque basico", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, 20, /*atk=*/10, 4, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 200, 10, 6, /*def=*/2);
    h.add_status(effect(StatusId::Silence, 0, 2, CardFamily::Sonico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::attack(e.id())), nullptr, nullptr,
                          &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.max_hp() - e.hp() == 8);
}

TEST_CASE("status: silence permite defender", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, 20, 8, /*def=*/7, CardFamily::Eletrico);
    CombatActor e = foe();
    h.add_status(effect(StatusId::Silence, 0, 2, CardFamily::Sonico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::defend()), nullptr, nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(find_status(h, StatusId::Shield) != nullptr);
}

TEST_CASE("status: silence permite flee", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, /*spd=*/100, 8, 4, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 500, /*spd=*/1);
    h.add_status(effect(StatusId::Silence, 0, 2, CardFamily::Sonico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::flee()), nullptr, nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(sm.outcome() == CombatOutcome::Fled);
}

TEST_CASE("status: silence expira e libera usecard", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, 20, /*atk=*/0, 4, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 200, 10, 6, /*def=*/0, CardFamily::Eletrico);
    h.add_status(effect(StatusId::Silence, 0, 1, CardFamily::Sonico));
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    auto reg = registry(card);
    FixedRandom rng;

    auto card_played = std::make_shared<bool>(false);
    CombatStateMachine sm({&h, &e}, [card_played, &card, &e](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side()) return CombatAction::pass();
        for (const auto& s : a.status_effects())
            if (s.id == StatusId::Silence) return CombatAction::pass();
        if (*card_played) return CombatAction::pass();
        *card_played = true;
        return CombatAction::use_card(card.id, e.id());
    }, &reg, nullptr, &rng);

    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // hero silenciado
    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // foe
    REQUIRE(find_status(h, StatusId::Silence) == nullptr);
    sm.begin_turn(); sm.run_active_turn_to_end();  // hero joga carta
    REQUIRE(e.max_hp() - e.hp() == 10);
}

// ===== DISRUPT =====

TEST_CASE("status: disrupt 30 reduz dano da proxima carta em 30pct",
          "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, 20, /*atk=*/0, 4, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 200, 10, 6, /*def=*/0, CardFamily::Eletrico);
    h.add_status(effect(StatusId::Disrupt, 30, 3, CardFamily::Sonico));
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    auto reg = registry(card);
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.max_hp() - e.hp() == 7);  // 10 * 0.70
}

TEST_CASE("status: disrupt consome na primeira carta e some", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, 20, /*atk=*/0, 4, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 200, 10, 6, /*def=*/0, CardFamily::Eletrico);
    h.add_status(effect(StatusId::Disrupt, 50, 3, CardFamily::Sonico));
    Card card = make_card("pulso.eletrico", CardFamily::Eletrico, 10);
    auto reg = registry(card);
    FixedRandom rng;
    auto plays = std::make_shared<int>(0);
    CombatStateMachine sm({&h, &e}, [plays, &card, &e](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side()) return CombatAction::pass();
        ++(*plays);
        if (*plays <= 2) return CombatAction::use_card(card.id, e.id());
        return CombatAction::pass();
    }, &reg, nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.max_hp() - e.hp() == 15);  // 5 + 10
    REQUIRE(find_status(h, StatusId::Disrupt) == nullptr);
}

TEST_CASE("status: disrupt nao afeta ataque basico", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, 20, /*atk=*/10, 4, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 200, 10, 6, /*def=*/2);
    h.add_status(effect(StatusId::Disrupt, 50, 3, CardFamily::Sonico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::attack(e.id())), nullptr, nullptr,
                          &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.max_hp() - e.hp() == 8);  // 10 - 2
}

// ===== BREAK =====

TEST_CASE("status: break reduz def ao aplicar", "[domain][combat][status]") {
    CombatActor f = foe("enemy", 500, 10, 6, /*def=*/10);
    CombatActor h = hero("gus", 100, /*spd=*/5);
    f.add_status(effect(StatusId::Break, 6, 2, CardFamily::Cinetico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &f}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    sm.begin_turn();  // foe age primeiro (spd 10 > hero 5)
    REQUIRE(f.def() == 4);
}

TEST_CASE("status: break restaura def ao expirar", "[domain][combat][status]") {
    CombatActor f = foe("enemy", 500, /*spd=*/10, 6, /*def=*/10);
    CombatActor h = hero("gus", 100, /*spd=*/5);
    f.add_status(effect(StatusId::Break, 6, 1, CardFamily::Cinetico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &f}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    sm.begin_turn();
    REQUIRE(f.def() == 4);
    sm.run_active_turn_to_end();
    REQUIRE(f.def() == 10);
    REQUIRE(find_status(f, StatusId::Break) == nullptr);
}

TEST_CASE("status: break def reduzida persiste durante a duracao",
          "[domain][combat][status]") {
    CombatActor f = foe("enemy", 500, /*spd=*/10, 6, /*def=*/10);
    CombatActor h = hero("gus", 100, /*spd=*/5);
    f.add_status(effect(StatusId::Break, 6, 3, CardFamily::Cinetico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &f}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // foe
    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // hero
    REQUIRE(f.def() == 4);
    REQUIRE(find_status(f, StatusId::Break) != nullptr);
}

// ===== HASTE / SLOW =====

TEST_CASE("status: haste aumenta spd e adianta na fila", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, /*spd=*/10);
    CombatActor e = foe("enemy", 500, /*spd=*/20);
    h.add_status(effect(StatusId::Haste, 15, 3, CardFamily::Eletrico));
    FixedRandom rng;
    CombatStateMachine sm({&e, &h}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    REQUIRE(order_ids(sm)[0] == "enemy");
    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // foe
    sm.begin_turn();  // hero tick aplica Haste
    REQUIRE(h.spd() == 25);
    REQUIRE(order_ids(sm)[0] == "gus");
}

TEST_CASE("status: slow reduz spd e atrasa na fila", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, /*spd=*/30);
    CombatActor e = foe("enemy", 500, /*spd=*/20);
    h.add_status(effect(StatusId::Slow, 15, 3, CardFamily::Eletrico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    REQUIRE(order_ids(sm)[0] == "gus");
    sm.begin_turn();  // hero tick aplica Slow
    REQUIRE(h.spd() == 15);
    REQUIRE(order_ids(sm)[0] == "enemy");
}

TEST_CASE("status: slow clamp spd em zero", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, /*spd=*/10);
    CombatActor e = foe("enemy", 500, /*spd=*/5);
    h.add_status(effect(StatusId::Slow, 50, 3, CardFamily::Eletrico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    sm.begin_turn();
    REQUIRE(h.spd() == 0);
}

TEST_CASE("status: haste restaura spd ao expirar", "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, /*spd=*/10);
    CombatActor e = foe("enemy", 500, /*spd=*/5);
    h.add_status(effect(StatusId::Haste, 15, 1, CardFamily::Eletrico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    sm.begin_turn();
    REQUIRE(h.spd() == 25);
    sm.run_active_turn_to_end();
    REQUIRE(h.spd() == 10);
    REQUIRE(find_status(h, StatusId::Haste) == nullptr);
}

// ===== KNOCKBACK (K-B: "adia o turno", decisao do lider 2026-07-15, =====
// =====            COMBATE-FILA-CURSOR-FIX/A2) =====

// Semantica revisada (substitui o comportamento antigo, que so deslocava o ator na fila
// SEM adiar o turno - bug QA: o vizinho pendente era empurrado pra TRAS do cursor e pulava
// a rodada inteira). Agora Knockback ADIA o turno do alvo: o vizinho que jogaria em seguida
// age PRIMEIRO, e o ator com Knockback (one-shot, consumido no disparo) age logo depois.
// Todos agem EXATAMENTE 1x na rodada.

TEST_CASE("status: knockback (K-B) adia o turno - o vizinho pendente joga primeiro, o "
          "empurrado joga em seguida (one-shot, consumido)",
          "[domain][combat][status]") {
    CombatActor a = hero("a", 100, /*spd=*/30);
    CombatActor b = foe("b", 500, /*spd=*/20);
    CombatActor c = foe("c", 500, /*spd=*/10);
    b.add_status(effect(StatusId::Knockback, 0, 2, CardFamily::Cinetico));
    FixedRandom rng;
    CombatStateMachine sm({&a, &b, &c}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    REQUIRE(order_ids(sm) == std::vector<std::string>{"a", "b", "c"});

    sm.begin_turn(); sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // a age.

    sm.begin_turn();  // slot do cursor era b (Knockback) -> dispara ANTES do current() fixar.
    REQUIRE(sm.active_actor()->id() == "c");  // c (vizinho pendente) joga primeiro.
    REQUIRE(order_ids(sm) == std::vector<std::string>{"a", "c", "b"});
    REQUIRE(find_status(b, StatusId::Knockback) == nullptr);  // consumido no disparo.
    REQUIRE(log_has(sm, CombatActionType::StatusTick, "empurrado (Knockback)"));

    sm.run_active_turn_to_end(); sm.advance_to_next_actor();  // c age.
    REQUIRE(sm.active_actor()->id() == "b");  // b (empurrado) fecha a rodada, joga em seguida.
}

TEST_CASE("status: knockback (K-B) - cada ator age exatamente 1x na rodada (sem "
          "turno-duplo nem turno-pulado)",
          "[domain][combat][status]") {
    CombatActor a = hero("a", 100, /*spd=*/30, /*atk=*/0);
    CombatActor b = foe("b", 500, /*spd=*/20, /*atk=*/10);
    CombatActor c = foe("c", 200, /*spd=*/10, /*atk=*/0, /*def=*/0);
    b.add_status(effect(StatusId::Knockback, 0, 2, CardFamily::Cinetico));
    FixedRandom rng;
    auto b_acted = std::make_shared<bool>(false);
    CombatStateMachine sm({&a, &b, &c}, [b_acted](CombatActor& act, const CombatState&) -> CombatAction {
        if (act.id() == "b" && !*b_acted) { *b_acted = true; return CombatAction::attack("c"); }
        return CombatAction::pass();
    }, nullptr, nullptr, &rng);

    std::vector<std::string> act_order;
    for (int i = 0; i < 3; ++i) {
        sm.begin_turn();
        act_order.push_back(sm.active_actor()->id());
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
    }

    // a age normal (1o); b tem Knockback -> adia, c (vizinho) joga em 2o; b fecha em 3o e
    // ainda ataca c (a acao NAO se perde, so adia). Cada id aparece EXATAMENTE 1x.
    REQUIRE(act_order == std::vector<std::string>{"a", "c", "b"});
    REQUIRE(order_ids(sm) == std::vector<std::string>{"a", "c", "b"});
    REQUIRE(*b_acted);
    REQUIRE(c.max_hp() - c.hp() == 10);
}

// ===== QA-2: repro do bug Knockback-pula-vizinho (achado QA 2026-07-15, ============
// =====        COMBATE-FILA-CURSOR-FIX/A2) - reorder_actor(+1) cru empurrava o alvo =====
// =====        pra TRAS do cursor, fazendo o vizinho pendente perder a rodada inteira.

TEST_CASE("status QA-2: [a, b(Knockback), c, d] - c (vizinho) age na rodada 0, todos "
          "os 4 agem exatamente 1x",
          "[domain][combat][status][qa]") {
    CombatActor a = hero("a", 100, /*spd=*/40);
    CombatActor b = foe("b", 500, /*spd=*/30);
    CombatActor c = foe("c", 500, /*spd=*/20);
    CombatActor d = foe("d", 500, /*spd=*/10);
    b.add_status(effect(StatusId::Knockback, 0, 1, CardFamily::Cinetico));
    FixedRandom rng;
    CombatStateMachine sm({&a, &b, &c, &d},
                          [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    REQUIRE(order_ids(sm) == std::vector<std::string>{"a", "b", "c", "d"});

    std::vector<std::string> act_order;
    for (int i = 0; i < 4; ++i) {
        sm.begin_turn();
        act_order.push_back(sm.active_actor()->id());
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
    }

    // a normal (1o); b(Knockback) adia -> c (vizinho) joga em 2o SEM perder a rodada (o bug
    // antigo pulava c inteiro); b fecha em 3o; d normal em 4o. Cada id EXATAMENTE 1x.
    REQUIRE(act_order == std::vector<std::string>{"a", "c", "b", "d"});
    REQUIRE(order_ids(sm) == std::vector<std::string>{"a", "c", "b", "d"});
}

TEST_CASE("status QA-2: Knockback encadeado (2 vizinhos adjacentes, ambos com o status) - "
          "o loop bounded resolve os 2 links no mesmo begin_turn, sem crash nem perda de "
          "turno",
          "[domain][combat][status][qa]") {
    CombatActor a = hero("a", 100, /*spd=*/40);
    CombatActor b = foe("b", 500, /*spd=*/30);
    CombatActor c = foe("c", 500, /*spd=*/20);
    CombatActor d = foe("d", 500, /*spd=*/10);
    b.add_status(effect(StatusId::Knockback, 0, 1, CardFamily::Cinetico));
    c.add_status(effect(StatusId::Knockback, 0, 1, CardFamily::Cinetico));
    FixedRandom rng;
    CombatStateMachine sm({&a, &b, &c, &d},
                          [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);

    std::vector<std::string> act_order;
    for (int i = 0; i < 4; ++i) {
        sm.begin_turn();
        act_order.push_back(sm.active_actor()->id());
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
    }

    // b e c sao vizinhos adjacentes com Knockback: b adia atras de c, e c (agora no cursor)
    // TAMBEM tem Knockback, entao adia de volta atras de b - o par "cancela" a reordenacao
    // liquida (2 links resolvidos no MESMO begin_turn), mas cada um consome o proprio status
    // e todos os 4 agem EXATAMENTE 1x (sem crash, sem loop infinito - guard bounded).
    REQUIRE(act_order == std::vector<std::string>{"a", "b", "c", "d"});
    REQUIRE(find_status(b, StatusId::Knockback) == nullptr);
    REQUIRE(find_status(c, StatusId::Knockback) == nullptr);
    int tick_logs = 0;
    for (const auto& e : sm.log())
        if (e.action == CombatActionType::StatusTick) ++tick_logs;
    REQUIRE(tick_logs == 2);  // 1 log por link da cadeia.
}

TEST_CASE("status QA-2: Knockback (begin_turn) e DelayAction/Einstein (reorder_pending "
          "via UseCard) convivem na mesma rodada sem pular nem duplicar nenhum ator",
          "[domain][combat][status][qa]") {
    CombatActor h = hero("h", 100, /*spd=*/50, /*atk=*/0);
    CombatActor e0 = foe("e0", 300, /*spd=*/40);
    e0.add_status(effect(StatusId::Knockback, 0, 1, CardFamily::Cinetico));
    CombatActor e1 = foe("e1", 300, /*spd=*/30);
    CombatActor e2 = foe("e2", 300, /*spd=*/20);

    Card einstein;
    einstein.id = "qa_einstein";
    einstein.display_name = "qa_einstein";
    einstein.family = CardFamily::Cinetico;
    einstein.base_type = CardBaseType::Glifo;
    einstein.mana_cost = 0;
    einstein.ap_cost = 1;
    einstein.power = 0;
    einstein.target_shape = TargetShape::Single;
    einstein.tier = CardTier::Especial;
    einstein.category = CardCategory::Ativa;
    einstein.effects = {EffectSpec{
        .trigger = TriggerHook::OnCast, .kind = EffectKind::DelayAction, .magnitude = 0}};
    auto reg = registry(einstein);

    auto h_acted = std::make_shared<bool>(false);
    CombatStateMachine sm(
        {&h, &e0, &e1, &e2},
        [h_acted](CombatActor& act, const CombatState&) -> CombatAction {
            if (act.id() == "h" && !*h_acted) {
                *h_acted = true;
                return CombatAction::use_card("qa_einstein", "e1");
            }
            return CombatAction::pass();
        },
        &reg);

    std::vector<std::string> act_order;
    for (int i = 0; i < 4; ++i) {
        sm.begin_turn();
        act_order.push_back(sm.active_actor()->id());
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
    }

    // h abre e conjura Einstein em e1 (empurra e1 pro fim da fila via reorder_pending); e0
    // tem Knockback e adia o proprio turno em begin_turn (via delay_current) - os 2
    // mecanismos de reordenacao intra-rodada convivem no MESMO round. NENHUM ator falta,
    // NENHUM repete.
    std::vector<std::string> sorted_order = act_order;
    std::sort(sorted_order.begin(), sorted_order.end());
    REQUIRE(sorted_order == std::vector<std::string>{"e0", "e1", "e2", "h"});
    REQUIRE(*h_acted);
}

TEST_CASE("status: haste no proprio tick nao faz o ator perder o turno",
          "[domain][combat][status]") {
    CombatActor h = hero("gus", 100, /*spd=*/30, /*atk=*/10);
    CombatActor e = foe("enemy", 200, /*spd=*/20, 6, /*def=*/0);
    h.add_status(effect(StatusId::Haste, 15, 3, CardFamily::Eletrico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::attack(e.id())), nullptr, nullptr,
                          &rng);
    sm.begin_turn();  // tick Haste recomputa; cursor segue hero
    REQUIRE(sm.active_actor()->id() == "gus");
    sm.run_active_turn_to_end();
    REQUIRE(e.max_hp() - e.hp() == 10);
}

// ===== DECRYPT =====

TEST_CASE("status: decrypt remove buffs existentes no tick", "[domain][combat][status]") {
    CombatActor f = foe("enemy", 500, /*spd=*/10);
    CombatActor h = hero("gus", 100, /*spd=*/5);
    f.add_status(effect(StatusId::Shield, 20, 5, CardFamily::Eletrico));
    f.add_status(effect(StatusId::Regen, 3, 5, CardFamily::Bioquimico));
    f.add_status(effect(StatusId::Decrypt, 0, 2, CardFamily::Criptografico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &f}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    sm.begin_turn();  // foe tick
    REQUIRE(find_status(f, StatusId::Shield) == nullptr);
    REQUIRE(find_status(f, StatusId::Regen) == nullptr);
    REQUIRE(find_status(f, StatusId::Decrypt) != nullptr);
}

TEST_CASE("status: decrypt remove TODOS os buffs do alvo", "[domain][combat][status]") {
    CombatActor f = foe("enemy", 500, /*spd=*/10);
    CombatActor h = hero("gus", 100, /*spd=*/5);
    f.add_status(effect(StatusId::Shield, 20, 5, CardFamily::Eletrico));
    f.add_status(effect(StatusId::Regen, 3, 5, CardFamily::Bioquimico));
    f.add_status(effect(StatusId::Haste, 7, 5, CardFamily::Eletrico));
    f.add_status(effect(StatusId::Decrypt, 0, 2, CardFamily::Criptografico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &f}, [](CombatActor&, const CombatState&) { return CombatAction::pass(); },
                          nullptr, nullptr, &rng);
    sm.begin_turn();
    for (const auto& s : f.status_effects())
        REQUIRE_FALSE(CombatActor::is_buff(s.id));
    REQUIRE(find_status(f, StatusId::Shield) == nullptr);
    REQUIRE(find_status(f, StatusId::Regen) == nullptr);
    REQUIRE(find_status(f, StatusId::Haste) == nullptr);
    REQUIRE(find_status(f, StatusId::Decrypt) != nullptr);
    REQUIRE(f.spd() == 10);  // Haste dispelado: SPD restaurado
}

TEST_CASE("status: decrypt nao bloqueia reaplicacao de buff", "[domain][combat][status]") {
    CombatActor f = foe("enemy", 500, /*spd=*/10);
    CombatActor h = hero("gus", 100, /*spd=*/5);
    f.add_status(effect(StatusId::Shield, 20, 5, CardFamily::Eletrico));
    f.add_status(effect(StatusId::Decrypt, 0, 3, CardFamily::Criptografico));
    FixedRandom rng;
    CombatStateMachine sm({&h, &f}, [](CombatActor& a, const CombatState&) {
        return !a.is_player_side() ? CombatAction::defend() : CombatAction::pass();
    }, nullptr, nullptr, &rng);
    sm.begin_turn();
    REQUIRE(find_status(f, StatusId::Shield) == nullptr);
    sm.run_active_turn_to_end();  // foe defende -> Shield reaplicado
    REQUIRE(find_status(f, StatusId::Shield) != nullptr);
    REQUIRE(find_status(f, StatusId::Decrypt) != nullptr);
}

TEST_CASE("status: decrypt nao bloqueia debuffs", "[domain][combat][status]") {
    CombatActor f = foe("enemy", 500, /*spd=*/10, 6, 4, CardFamily::Eletrico);
    CombatActor h = hero("gus", 100, /*spd=*/20, /*atk=*/0, 4, CardFamily::Bioquimico);
    f.add_status(effect(StatusId::Decrypt, 0, 3, CardFamily::Criptografico));
    StatusEffect poison = effect(StatusId::Poison, 4, 3, CardFamily::Bioquimico);
    Card card = make_card("raiz.toxica", CardFamily::Bioquimico, 5, 1, poison);
    auto reg = registry(card);
    FixedRandom rng;
    CombatStateMachine sm({&h, &f}, play_once(CombatAction::use_card(card.id, f.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();  // hero age primeiro (spd 20)
    sm.run_active_turn_to_end();
    REQUIRE(find_status(f, StatusId::Poison) != nullptr);
}
