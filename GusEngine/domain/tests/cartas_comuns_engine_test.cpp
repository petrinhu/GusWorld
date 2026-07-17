// cartas_comuns_engine_test.cpp
//
// Spec executavel (Catch2 v3) do CARTAS-COMUNS-ENGINE (TODO.md, decisoes do lider
// 2026-07-16): 2 pecas de engine pequenas pras cartas COMUNS (record-base secao 7 + formula
// divisiva secao 11, NAO o executor techMagic/ADR-016 - zero EffectKind/StatusId novo).
//
//   PECA 1 (SynergyStatus, Finalizador Opcao A): campo Card::synergy_statuses (lista de
//   StatusId) + Card::synergy_percent generaliza o mult_expose (secao 9/11, que so cobre
//   Expose) pra QUALQUER StatusId presente no alvo. Fator FIXO (+40% na carta canonica) se
//   >=1 status da lista esta presente; NAO stacka por-status. Inserido na cadeia divisiva
//   ANTES do sorteio de canal, MESMO padrao ordinal do mult_expose - preview
//   (estimate_card_damage) e o gemeo PURO obrigatorio.
//
//   PECA 2 (Recarga de recurso, Eletrico-utilidade "Tavus-Overclock"): campos
//   Card::restore_ap/restore_mana concedem grant_bonus_ap/restore_mana ao CONJURADOR, DEPOIS
//   do custo de mana pago, sujeitos a trava 1x/turno (CombatActor::overclock_used_,
//   resetada no refresh de TurnStart - mesmo lugar que zera AP/mana, secao 5).
//
// Cross-ref: docs/design/mecanicas/combat.md secao 7/9/11;
//            docs/design/mecanicas/cartas-comuns-statlines.md;
//            gus/domain/combat/combat_records.hpp (Card::synergy_statuses/synergy_percent/
//            restore_ap/restore_mana); gus/domain/combat/combat_actor.hpp
//            (CombatActor::overclock_used()); combat_state_machine.cpp
//            (resolve_use_card/estimate_card_damage).

#include <catch2/catch_test_macros.hpp>

#include <memory>
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

CombatActor hero(const std::string& id = "gus", int hp = 50, int spd = 20, int atk = 8,
                 int def = 2, CardFamily family = CardFamily::Eletrico) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/true);
}

CombatActor foe(const std::string& id = "enemy", int hp = 500, int spd = 10, int atk = 6,
                int def = 0, CardFamily family = CardFamily::Eletrico, int kills = 0) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/false, /*is_boss=*/false,
                       kills);
}

Card make_card(const std::string& id, CardFamily family, int power, int mana_cost = 1) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = family;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = mana_cost;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = TargetShape::Single;
    return c;
}

std::unordered_map<std::string, Card> registry(std::initializer_list<Card> cards) {
    std::unordered_map<std::string, Card> d;
    for (const auto& c : cards) d.emplace(c.id, c);
    return d;
}

CombatActionProvider play_once(CombatAction action) {
    auto done = std::make_shared<bool>(false);
    return [action, done](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side() || *done) return CombatAction::pass();
        *done = true;
        return action;
    };
}

// Joga `action` UMA VEZ POR RODADA (gated por round_index) - usado pra atravessar 2 turnos
// completos do mesmo ator (hero) e provar reset de trava/bonus no TurnStart seguinte.
CombatActionProvider play_once_per_round(CombatAction action) {
    auto last_round = std::make_shared<int>(-1);
    return [action, last_round](CombatActor& a, const CombatState& state) -> CombatAction {
        if (!a.is_player_side()) return CombatAction::pass();
        if (*last_round == state.round_index()) return CombatAction::pass();
        *last_round = state.round_index();
        return action;
    };
}

// RNG fixo QUE CONTA consumos (mesmo padrao de combat_state_machine_test.cpp::
// CountingRandomSm / techmagic_quantize_test.cpp::CountingRandom).
class CountingRandom final : public IRandomSource {
public:
    explicit CountingRandom(double next_double = 0.5, int next_int = 99)
        : next_double_(next_double), next_int_(next_int) {}
    double next_double() override { ++double_calls; return next_double_; }
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

bool log_has_substring(const CombatStateMachine& sm, const std::string& needle) {
    for (const auto& entry : sm.log())
        if (entry.message.find(needle) != std::string::npos) return true;
    return false;
}

int count_log_substring(const CombatStateMachine& sm, const std::string& needle) {
    int count = 0;
    for (const auto& entry : sm.log())
        if (entry.message.find(needle) != std::string::npos) ++count;
    return count;
}

}  // namespace

// ================================================================================
// PECA 1: SynergyStatus (Finalizador +40% fixo)
// ================================================================================

TEST_CASE("synergy: status presente na lista amplia dano por synergy_percent (+40%)",
          "[domain][combat][cartas_comuns_engine][synergy]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/0, /*def=*/2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 500, 10, /*atk=*/6, /*def=*/0, CardFamily::Eletrico);
    e.add_status({StatusId::Stun, 0, 1, StackRule::Replace, CardFamily::Eletrico});

    Card card = make_card("tavusa_fulminante", CardFamily::Eletrico, /*power=*/10);
    card.synergy_statuses = {StatusId::Stun};
    card.synergy_percent = 40;  // //PLAYTEST
    auto reg = registry({card});

    FixedRandom rng;  // roll=99 (COMUM), next_double=0.5 (variancia ZERO)
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(e.max_hp() - e.hp() == 14);  // round(10 * 1.4)
}

TEST_CASE("synergy: status AUSENTE no alvo -> fator neutro x1.0 (mesmo com lista nao-vazia)",
          "[domain][combat][cartas_comuns_engine][synergy]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/0, /*def=*/2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 500, 10, /*atk=*/6, /*def=*/0, CardFamily::Eletrico);
    // e SEM Stun.

    Card card = make_card("tavusa_fulminante", CardFamily::Eletrico, /*power=*/10);
    card.synergy_statuses = {StatusId::Stun};
    card.synergy_percent = 40;
    auto reg = registry({card});

    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(e.max_hp() - e.hp() == 10);  // sem bonus
    REQUIRE_FALSE(log_has_substring(sm, "SINERGIA"));
}

TEST_CASE("synergy: 2+ status da lista presentes no alvo NAO stackam (continua x1.4, nao x1.8)",
          "[domain][combat][cartas_comuns_engine][synergy]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/0, /*def=*/2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 500, 10, /*atk=*/6, /*def=*/0, CardFamily::Eletrico);
    e.add_status({StatusId::Poison, 1, 3, StackRule::Replace, CardFamily::Bioquimico});
    e.add_status({StatusId::Corrode, 1, 3, StackRule::StackMagnitude, CardFamily::Bioquimico});

    Card card = make_card("erynin_epidemia", CardFamily::Eletrico, /*power=*/10);
    card.synergy_statuses = {StatusId::Poison, StatusId::Corrode};
    card.synergy_percent = 40;
    auto reg = registry({card});

    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    // Poison/Corrode causam tick de dano proprio no TurnStart do ALVO (nao do atacante) -
    // aqui so o hero age (begin_turn e do hero), entao nenhum tick roda sobre `e` ainda.
    REQUIRE(e.max_hp() - e.hp() == 14);  // round(10 * 1.4), NAO 18 (1.8 seria stack)
}

TEST_CASE("synergy: preview (estimate_card_damage) == real nas fronteiras do canal COMUM "
          "(gemeo obrigatorio)",
          "[domain][combat][cartas_comuns_engine][synergy]") {
    Card card = make_card("tavusa_fulminante", CardFamily::Eletrico, /*power=*/10, /*mana=*/0);
    card.synergy_statuses = {StatusId::Stun};
    card.synergy_percent = 40;
    auto reg = registry({card});

    CombatActor h1 = hero("gus", 50, 20, /*atk=*/10, /*def=*/2, CardFamily::Eletrico);
    CombatActor e1 = foe("enemy", 500, 10, /*atk=*/6, /*def=*/0, CardFamily::Eletrico);
    e1.add_status({StatusId::Stun, 0, 1, StackRule::Replace, CardFamily::Eletrico});
    CombatStateMachine sm_est({&h1, &e1}, play_once(CombatAction::pass()), &reg, nullptr,
                              nullptr);
    const CardDamageEstimate est = sm_est.estimate_card_damage(h1, e1, card);

    CombatActor h2 = hero("gus", 50, 20, /*atk=*/10, /*def=*/2, CardFamily::Eletrico);
    CombatActor e2 = foe("enemy", 500, 10, /*atk=*/6, /*def=*/0, CardFamily::Eletrico);
    e2.add_status({StatusId::Stun, 0, 1, StackRule::Replace, CardFamily::Eletrico});
    CountingRandom rng_floor(/*next_double=*/0.0, /*next_int=*/99);
    CombatStateMachine sm_floor({&h2, &e2}, play_once(CombatAction::use_card(card.id, e2.id())),
                                &reg, nullptr, &rng_floor);
    sm_floor.begin_turn();
    sm_floor.run_active_turn_to_end();
    REQUIRE(e2.max_hp() - e2.hp() == est.min_damage);

    CombatActor h3 = hero("gus", 50, 20, /*atk=*/10, /*def=*/2, CardFamily::Eletrico);
    CombatActor e3 = foe("enemy", 500, 10, /*atk=*/6, /*def=*/0, CardFamily::Eletrico);
    e3.add_status({StatusId::Stun, 0, 1, StackRule::Replace, CardFamily::Eletrico});
    CountingRandom rng_ceil(/*next_double=*/1.0, /*next_int=*/99);
    CombatStateMachine sm_ceil({&h3, &e3}, play_once(CombatAction::use_card(card.id, e3.id())),
                               &reg, nullptr, &rng_ceil);
    sm_ceil.begin_turn();
    sm_ceil.run_active_turn_to_end();
    REQUIRE(e3.max_hp() - e3.hp() == est.max_damage);
}

TEST_CASE("synergy: preview == real no canal CRIT (gemeo)",
          "[domain][combat][cartas_comuns_engine][synergy]") {
    Card card = make_card("tavusa_fulminante", CardFamily::Eletrico, /*power=*/10, /*mana=*/0);
    card.synergy_statuses = {StatusId::Stun};
    card.synergy_percent = 40;
    auto reg = registry({card});

    CombatActor h1 = hero("gus", 50, 20, /*atk=*/10, /*def=*/2, CardFamily::Eletrico);
    CombatActor e1 = foe("enemy", 500, 10, /*atk=*/6, /*def=*/0, CardFamily::Eletrico);
    e1.add_status({StatusId::Stun, 0, 1, StackRule::Replace, CardFamily::Eletrico});
    CombatStateMachine sm_est({&h1, &e1}, play_once(CombatAction::pass()), &reg, nullptr,
                              nullptr);
    const CardDamageEstimate est = sm_est.estimate_card_damage(h1, e1, card);

    CombatActor h2 = hero("gus", 50, 20, /*atk=*/10, /*def=*/2, CardFamily::Eletrico);
    CombatActor e2 = foe("enemy", 500, 10, /*atk=*/6, /*def=*/0, CardFamily::Eletrico);
    e2.add_status({StatusId::Stun, 0, 1, StackRule::Replace, CardFamily::Eletrico});
    // kills=0 => fumble=5, crit=5 => CRIT em roll in [5,10).
    CountingRandom rng_crit(/*next_double=*/0.5, /*next_int=*/5);
    CombatStateMachine sm_crit({&h2, &e2}, play_once(CombatAction::use_card(card.id, e2.id())),
                               &reg, nullptr, &rng_crit);
    sm_crit.begin_turn();
    sm_crit.run_active_turn_to_end();
    REQUIRE(e2.max_hp() - e2.hp() == est.crit_damage);
}

TEST_CASE("synergy: contagem de RNG INALTERADA - mesmo numero de consumos com e sem sinergia",
          "[domain][combat][cartas_comuns_engine][synergy]") {
    Card card = make_card("tavusa_fulminante", CardFamily::Eletrico, /*power=*/10, /*mana=*/0);
    card.synergy_statuses = {StatusId::Stun};
    card.synergy_percent = 40;
    auto reg = registry({card});

    // Com sinergia (status presente): canal COMUM, roll=99.
    CombatActor h1 = hero("gus", 50, 20, /*atk=*/10, /*def=*/2, CardFamily::Eletrico);
    CombatActor e1 = foe("enemy", 500, 10, /*atk=*/6, /*def=*/0, CardFamily::Eletrico);
    e1.add_status({StatusId::Stun, 0, 1, StackRule::Replace, CardFamily::Eletrico});
    CountingRandom rng1;
    CombatStateMachine sm1({&h1, &e1}, play_once(CombatAction::use_card(card.id, e1.id())), &reg,
                           nullptr, &rng1);
    sm1.begin_turn();
    sm1.run_active_turn_to_end();

    // Sem sinergia (status ausente): mesma configuracao de RNG.
    CombatActor h2 = hero("gus", 50, 20, /*atk=*/10, /*def=*/2, CardFamily::Eletrico);
    CombatActor e2 = foe("enemy", 500, 10, /*atk=*/6, /*def=*/0, CardFamily::Eletrico);
    CountingRandom rng2;
    CombatStateMachine sm2({&h2, &e2}, play_once(CombatAction::use_card(card.id, e2.id())), &reg,
                           nullptr, &rng2);
    sm2.begin_turn();
    sm2.run_active_turn_to_end();

    REQUIRE(rng1.int_calls == rng2.int_calls);
    REQUIRE(rng1.double_calls == rng2.double_calls);
    REQUIRE(rng1.int_calls == 1);
    REQUIRE(rng1.double_calls == 1);
}

// ---- Interacao com Planck/Quantum-Lock: degraus rodam sobre a base JA amplificada pela ----
// ---- sinergia; a media ponderada (25/50/25) continua preservando a base (mesmo helper -----
// ---- comum_channel_damage do canal COMUM de sempre, so o `r` muda de continuo pra fixo). --
// Duplo LOCAL da Planck de producao (mesmo padrao de techmagic_quantize_test.cpp::planck_card).

namespace {
Card planck_card(const std::string& id) {
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
                            .magnitude = 50,
                            .percent = 25}};
    return c;
}
}  // namespace

TEST_CASE("synergy + Planck: os 3 degraus quantizados rodam sobre a base JA amplificada pela "
          "sinergia, e batem EXATAMENTE com estimate_card_damage (gemeo, media preservada por "
          "construcao - mesmo helper comum_channel_damage do canal COMUM sem Planck)",
          "[domain][combat][cartas_comuns_engine][synergy][planck]") {
    Card card = make_card("tavusa_fulminante", CardFamily::Eletrico, /*power=*/12, /*mana=*/0);
    card.synergy_statuses = {StatusId::Stun};
    card.synergy_percent = 40;
    Card planck = planck_card("planck.synergy.passive");
    auto reg = registry({card, planck});

    CombatActor h_est = hero("gus", 50, 20, /*atk=*/8, /*def=*/2, CardFamily::Eletrico);
    h_est.set_equipped_special_ids({planck.id});
    CombatActor e_est = foe("enemy", 500, 10, /*atk=*/0, /*def=*/0, CardFamily::Sonico,
                            /*kills=*/5);
    e_est.add_status({StatusId::Stun, 0, 1, StackRule::Replace, CardFamily::Eletrico});
    CombatStateMachine sm_est({&h_est, &e_est}, play_once(CombatAction::pass()), &reg, nullptr,
                              nullptr);
    const CardDamageEstimate est = sm_est.estimate_card_damage(h_est, e_est, card);
    REQUIRE(est.quantized);

    auto resolved_at = [&](int next_int_degree) {
        CombatActor h = hero("gus", 50, 20, /*atk=*/8, /*def=*/2, CardFamily::Eletrico);
        h.set_equipped_special_ids({planck.id});
        CombatActor e = foe("enemy", 500, 10, /*atk=*/0, /*def=*/0, CardFamily::Sonico,
                            /*kills=*/5);
        e.add_status({StatusId::Stun, 0, 1, StackRule::Replace, CardFamily::Eletrico});
        FixedRandom rng(/*next_double=*/0.5, /*next_int=*/next_int_degree);
        CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                              nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        return e.max_hp() - e.hp();
    };

    // next_int=10: channel roll=10 (kills=5 -> fumble=0, crit=5 -> COMUM), degree roll=10
    // (<25) -> PISO.
    REQUIRE(resolved_at(10) == est.min_damage);
    // next_int=50: degree roll=50 (25<=50<75) -> CENTRO.
    REQUIRE(resolved_at(50) == est.mid_damage);
    // next_int=80: degree roll=80 (>=75) -> TETO.
    REQUIRE(resolved_at(80) == est.max_damage);
}

TEST_CASE("synergy: log diegetico presente com sufixo SINERGIA quando dispara",
          "[domain][combat][cartas_comuns_engine][synergy]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/0, /*def=*/2, CardFamily::Eletrico);
    CombatActor e = foe("enemy", 500, 10, /*atk=*/6, /*def=*/0, CardFamily::Eletrico);
    e.add_status({StatusId::Stun, 0, 1, StackRule::Replace, CardFamily::Eletrico});

    Card card = make_card("tavusa_fulminante", CardFamily::Eletrico, /*power=*/10);
    card.synergy_statuses = {StatusId::Stun};
    card.synergy_percent = 40;
    auto reg = registry({card});

    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(log_has_substring(sm, "SINERGIA"));
    REQUIRE(log_has_substring(sm, "+40%"));
}

// NOTA DE COBERTURA (curto-circuito de imunidade x sinergia): o tier Imune (mult_fraqueza ==
// 0.0) e "flag de inimigo/lore, incremento futuro" e NAO e exercitavel pela roda de fraqueza
// publica hoje (mesma limitacao documentada em weakness_wheel_property_test.cpp INV-8, linha
// 14-18) - nao ha combinacao de familias que produza Imune nos testes deste motor. mult_synergy
// e computado ANTES do guard `if (mult_fraqueza == 0.0f)` (mesma posicao ordinal de
// mult_expose, que tem a MESMA propriedade e tambem nao e coberta neste ponto especifico) -
// quando o guard dispara, o `continue` descarta base_damage inteiro (que incluiria
// mult_synergy) e forca dano 0 SEM consumir RNG, entao a prioridade e estrutural por
// construcao (mesmo helper/ordem que ja protege mult_expose), nao uma peca nova de logica a
// testar isoladamente. Ver combat_state_machine.cpp::resolve_use_card, comentario "Curto-
// circuito de imunidade".

// ================================================================================
// PECA 2: Recarga de AP/mana (Tavus-Overclock), trava 1x/turno
// ================================================================================

// ---- Unit tests puros de CombatActor (sem FSM) ----

TEST_CASE("overclock actor: grant_bonus_ap NAO persiste - some no proximo refresh de TurnStart",
          "[domain][combat][cartas_comuns_engine][overclock]") {
    CombatActor a = hero();
    a.refresh_resources_for_turn(0);
    a.spend_ap(1);
    REQUIRE(a.ap() == combat_constants::kBaseApPerTurn - 1);
    a.grant_bonus_ap(1);
    REQUIRE(a.ap() == combat_constants::kBaseApPerTurn);  // 2 + 1 bonus = 3 = max_ap
    a.refresh_resources_for_turn(1);  // TurnStart seguinte
    REQUIRE(a.ap() == a.max_ap());    // bonus NAO persistiu, so o reset normal
}

TEST_CASE("overclock actor: restore_mana clampa no teto max_mana",
          "[domain][combat][cartas_comuns_engine][overclock]") {
    CombatActor a = hero();
    a.refresh_resources_for_turn(0);  // max_mana = 2 + 0 = 2
    a.spend_mana(1);
    REQUIRE(a.mana() == 1);
    a.restore_mana(5);  // clamp
    REQUIRE(a.mana() == 2);
    REQUIRE(a.mana() == a.max_mana());
}

TEST_CASE("overclock actor: trava overclock_used_ e resetada no refresh de TurnStart",
          "[domain][combat][cartas_comuns_engine][overclock]") {
    CombatActor a = hero();
    REQUIRE_FALSE(a.overclock_used());
    a.set_overclock_used(true);
    REQUIRE(a.overclock_used());
    a.refresh_resources_for_turn(0);
    REQUIRE_FALSE(a.overclock_used());
}

// ---- Integracao via CombatStateMachine (resolve_use_card real) ----

namespace {
Card overclock_card(const std::string& id, int mana_cost, int restore_ap, int restore_mana) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Eletrico;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = mana_cost;
    c.ap_cost = 1;
    c.power = 0;
    c.target_shape = TargetShape::Self;
    c.restore_ap = restore_ap;
    c.restore_mana = restore_mana;
    return c;
}
}  // namespace

TEST_CASE("overclock: 1a jogada no turno recarrega AP/mana e loga sucesso",
          "[domain][combat][cartas_comuns_engine][overclock]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/8, /*def=*/2);
    CombatActor e = foe();
    Card card = overclock_card("tavusa_overclock", /*mana_cost=*/1, /*restore_ap=*/1,
                               /*restore_mana=*/2);
    auto reg = registry({card});
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, h.id())), &reg,
                          nullptr, &rng);

    sm.begin_turn();  // max_ap=3, max_mana=2
    REQUIRE(h.ap() == combat_constants::kBaseApPerTurn);
    REQUIRE(h.mana() == combat_constants::kBaseMana);

    sm.run_active_turn_to_end();

    REQUIRE(h.overclock_used());
    REQUIRE(h.ap() == combat_constants::kBaseApPerTurn);  // -1 (custo) +1 (bonus) = 3
    REQUIRE(h.mana() == combat_constants::kBaseMana);     // -1 (custo) +2 (clamp em 2) = 2
    REQUIRE(log_has_substring(sm, "tavus-overclock"));
    REQUIRE(log_has_substring(sm, "+1 AP"));
    REQUIRE(log_has_substring(sm, "+2 mana"));
}

TEST_CASE("overclock: 2a jogada no MESMO turno NAO recarrega (trava 1x/turno) e loga bloqueio; "
          "custo de mana continua sendo pago normalmente",
          "[domain][combat][cartas_comuns_engine][overclock]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/8, /*def=*/2);
    CombatActor e = foe();
    Card card = overclock_card("tavusa_overclock", /*mana_cost=*/1, /*restore_ap=*/1,
                               /*restore_mana=*/2);
    auto reg = registry({card});
    FixedRandom rng;

    auto plays = std::make_shared<int>(0);
    CombatActionProvider provider = [card, h, plays](CombatActor& a,
                                                      const CombatState&) -> CombatAction {
        if (!a.is_player_side() || *plays >= 2) return CombatAction::pass();
        ++*plays;
        return CombatAction::use_card(card.id, a.id());
    };
    CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    // 1a jogada: ap 3->2 (custo) ->3 (bonus); mana 2->1 (custo) ->2 (restore, clamp).
    // 2a jogada: ap 3->2 (custo, SEM bonus, trava); mana 2->1 (custo, SEM restore, trava).
    REQUIRE(h.ap() == combat_constants::kBaseApPerTurn - 1);  // 2
    REQUIRE(h.mana() == combat_constants::kBaseMana - 1);     // 1
    REQUIRE(h.overclock_used());
    REQUIRE(count_log_substring(sm, "tavus-overclock") == 2);
    REQUIRE(count_log_substring(sm, "+1 AP") == 1);            // so a 1a
    REQUIRE(log_has_substring(sm, "ja recarregado"));          // bloqueio da 2a
    REQUIRE(log_has_substring(sm, "bloqueada"));
}

TEST_CASE("overclock: trava reseta no turno seguinte - a carta volta a recarregar",
          "[domain][combat][cartas_comuns_engine][overclock]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/8, /*def=*/2);
    CombatActor e = foe("enemy", 500, 1, /*atk=*/0, /*def=*/0);  // SPD baixo: hero sempre 1o
    Card card = overclock_card("tavusa_overclock", /*mana_cost=*/1, /*restore_ap=*/1,
                               /*restore_mana=*/2);
    auto reg = registry({card});
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once_per_round(CombatAction::use_card(card.id, h.id())),
                          &reg, nullptr, &rng);

    // Rodada 0: hero joga, foe passa.
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(h.overclock_used());
    sm.advance_to_next_actor();
    sm.begin_turn();
    sm.run_active_turn_to_end();  // foe passa (nao esta no provider, cai no default pass)
    sm.advance_to_next_actor();

    // Rodada 1 (wrap de volta pro hero): begin_turn chama refresh_resources_for_turn - a
    // trava reseta, o Tavus-Overclock volta a funcionar.
    sm.begin_turn();
    REQUIRE_FALSE(h.overclock_used());  // resetado no refresh, ANTES da 2a jogada
    sm.run_active_turn_to_end();

    REQUIRE(h.overclock_used());  // recarregou de novo nesta rodada
    REQUIRE(count_log_substring(sm, "tavus-overclock") == 2);  // 1 por rodada, ambas sucesso
    REQUIRE_FALSE(log_has_substring(sm, "ja recarregado"));    // nunca bloqueou (1x por turno)
}

TEST_CASE("overclock: custo de mana e SEMPRE pago ANTES - mana insuficiente na 2a carta lanca "
          "mesmo sendo carta de recarga (sem recarga silenciosa que 'paga a si mesma')",
          "[domain][combat][cartas_comuns_engine][overclock]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/8, /*def=*/2);
    CombatActor e = foe();
    // mana_cost == max_mana (2): 1a jogada zera a mana; restore_mana=1 (NAO cobre o proximo
    // custo de 2) - a trava bloqueia o restore da 2a jogada de qualquer forma, entao a 2a
    // SEMPRE falta mana (1 < 2), provando que o custo nao "se auto-paga" via a propria carta.
    Card card = overclock_card("tavusa_overclock_caro", /*mana_cost=*/2, /*restore_ap=*/0,
                               /*restore_mana=*/1);
    auto reg = registry({card});
    FixedRandom rng;

    CombatActionProvider provider = [card](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side()) return CombatAction::pass();
        return CombatAction::use_card(card.id, a.id());  // tenta jogar ate falhar
    };
    CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    REQUIRE_THROWS_AS(sm.run_active_turn_to_end(), std::logic_error);
}

TEST_CASE("overclock: Silence bloqueia o cast ANTES de qualquer coisa - sem recarga, sem trava "
          "marcada",
          "[domain][combat][cartas_comuns_engine][overclock]") {
    CombatActor h = hero("gus", 50, 20, /*atk=*/8, /*def=*/2);
    CombatActor e = foe();
    h.add_status({StatusId::Silence, 0, 2, StackRule::Replace, CardFamily::Sonico});
    Card card = overclock_card("tavusa_overclock", /*mana_cost=*/1, /*restore_ap=*/1,
                               /*restore_mana=*/2);
    auto reg = registry({card});
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, h.id())), &reg,
                          nullptr, &rng);

    sm.begin_turn();
    REQUIRE_THROWS_AS(sm.run_active_turn_to_end(), std::logic_error);

    REQUIRE_FALSE(h.overclock_used());
    REQUIRE(h.mana() == combat_constants::kBaseMana);  // custo nunca chegou a ser pago
    REQUIRE_FALSE(log_has_substring(sm, "tavus-overclock"));
}
