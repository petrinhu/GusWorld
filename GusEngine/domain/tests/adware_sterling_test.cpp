// adware_sterling_test.cpp
//
// Spec executavel (Catch2 v3) da fatia CARDS-HW-3C (Adware Sterling, SO logica de dominio):
// docs/design/mecanicas/cartas-spec-logica.md secao 1/4.1/9. Cobre:
//   - Card::has_adware (flag de catalogo, aditivo, default false - carta comum intocada).
//   - AdwareExposureTracker (gus/domain/combat/adware_sterling.hpp): decisao PURA, ZERO
//     acoplamento com CombatActor/mana (roll_exposure nem recebe um) - exposicoes 1-3
//     SEMPRE ShowFull (0 RNG); da 4a exposicao em diante, distribuicao 70/30 (AMB-11
//     resolvida pelo lider 2026-07-20/21) via 1 consumo de IRandomSource::next(100).
//   - Integracao via CombatStateMachine::resolve_use_card: gate roda ANTES do debito de
//     mana (secao 1 do doc-fonte), loga em TODO desfecho (regra "todo efeito loga"), e o
//     cast SEMPRE PROSSEGUE apos a decisao (debito + efeito nominal) - NAO ha cancelamento
//     nesta fatia (ver NOTA/FLAG AMB-12 em combat_state_machine.cpp::dispatch_adware_gate).
//   - Regressao: carta SEM has_adware = pipeline IDENTICO (nenhum log/tracker tocado).
//
// Cross-ref: gus/domain/combat/adware_sterling.hpp; gus/domain/cards/card_records.hpp
//            (Card::has_adware); combat_state_machine.cpp (dispatch_adware_gate,
//            resolve_use_card).

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "fixed_random.hpp"
#include "gus/domain/combat/adware_sterling.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/random_source.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::FixedRandom;

namespace {

// Duplo de IRandomSource que LANCA se chamado - prova arquitetural de "zero consumo de RNG"
// (exposicoes 1..kAdwareAlwaysShowThreshold nunca deveriam tocar a porta).
class ThrowingRandom final : public IRandomSource {
public:
    double next_double() override { throw std::logic_error("next_double nao deveria ser chamado."); }
    int next(int) override { throw std::logic_error("next nao deveria ser chamado."); }
};

// Duplo que CONTA quantas vezes next(int) foi chamado, devolvendo sempre `next_int` (mesmo
// padrao de CountingRandom em card_virus_combat_test.cpp/cartas_comuns_engine_test.cpp).
class CountingRandom final : public IRandomSource {
public:
    explicit CountingRandom(int next_int) : next_int_(next_int) {}
    double next_double() override { return 0.5; }
    int next(int max_value) override {
        ++next_calls;
        return max_value <= 0 ? 0 : std::min(next_int_, max_value - 1);
    }
    int next_calls = 0;

private:
    int next_int_;
};

CombatActor hero(const std::string& id = "gus", int hp = 50, int spd = 20, int atk = 8,
                 int def = 2, CardFamily family = CardFamily::Eletrico) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/true);
}

CombatActor foe(const std::string& id = "enemy", int hp = 500, int spd = 10, int atk = 0,
                int def = 0, CardFamily family = CardFamily::Eletrico) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/false);
}

Card adware_card(const std::string& id, bool has_adware, int mana_cost = 1, int power = 20) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Cinetico;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = mana_cost;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = TargetShape::Single;
    c.tier = CardTier::Comum;
    c.has_adware = has_adware;
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

// Joga `action` UMA VEZ POR RODADA (mesmo padrao de cartas_comuns_engine_test.cpp) - usado
// pra atravessar N turnos completos do hero (spd maior => sempre abre a rodada).
CombatActionProvider play_once_per_round(CombatAction action) {
    auto last_round = std::make_shared<int>(-1);
    return [action, last_round](CombatActor& a, const CombatState& state) -> CombatAction {
        if (!a.is_player_side()) return CombatAction::pass();
        if (*last_round == state.round_index()) return CombatAction::pass();
        *last_round = state.round_index();
        return action;
    };
}

bool log_has(const CombatStateMachine& sm, const std::string& needle) {
    for (const auto& entry : sm.log())
        if (entry.message.find(needle) != std::string::npos) return true;
    return false;
}

int count_log(const CombatStateMachine& sm, const std::string& needle) {
    int n = 0;
    for (const auto& entry : sm.log())
        if (entry.message.find(needle) != std::string::npos) ++n;
    return n;
}

}  // namespace

// ===== Card::has_adware (flag de catalogo) ===========================================

TEST_CASE("Card::has_adware: default false, aditivo, nao muda igualdade de carta existente",
          "[domain][combat][adware][card]") {
    Card comum;
    comum.id = "pulso_eletrico";
    REQUIRE_FALSE(comum.has_adware);

    Card copia = comum;
    REQUIRE(copia == comum);  // operator== ainda funciona (campo novo participa por valor).

    copia.has_adware = true;
    REQUIRE_FALSE(copia == comum);
}

// ===== AdwareExposureTracker: decisao PURA, desacoplada de recurso ===================

TEST_CASE("AdwareExposureTracker: exposicoes 1, 2 e 3 SEMPRE ShowFull, ZERO consumo de RNG",
          "[domain][combat][adware][tracker]") {
    AdwareExposureTracker tracker;
    ThrowingRandom rng;  // se qualquer metodo for chamado, o teste falha por excecao.

    const AdwareGateResult r1 = tracker.roll_exposure(rng);
    REQUIRE(r1.outcome == AdwareOutcome::ShowFull);
    REQUIRE(r1.exposure_index == 1);

    const AdwareGateResult r2 = tracker.roll_exposure(rng);
    REQUIRE(r2.outcome == AdwareOutcome::ShowFull);
    REQUIRE(r2.exposure_index == 2);

    const AdwareGateResult r3 = tracker.roll_exposure(rng);
    REQUIRE(r3.outcome == AdwareOutcome::ShowFull);
    REQUIRE(r3.exposure_index == 3);

    REQUIRE(tracker.exposure_count() == 3);
}

TEST_CASE("AdwareExposureTracker: 4a exposicao em diante consome EXATAMENTE 1 next(100) e "
          "respeita o limiar de 70%",
          "[domain][combat][adware][tracker]") {
    AdwareExposureTracker tracker;
    ThrowingRandom throwing;
    (void)tracker.roll_exposure(throwing);
    (void)tracker.roll_exposure(throwing);
    (void)tracker.roll_exposure(throwing);
    REQUIRE(tracker.exposure_count() == 3);

    // roll=69 (<70) -> ShowFull; roll=70 (>=70) -> Skip. Fronteira exata do limiar.
    {
        CountingRandom rng(69);
        const AdwareGateResult r = tracker.roll_exposure(rng);
        REQUIRE(r.outcome == AdwareOutcome::ShowFull);
        REQUIRE(r.exposure_index == 4);
        REQUIRE(rng.next_calls == 1);
    }
    {
        AdwareExposureTracker tracker2;
        (void)tracker2.roll_exposure(throwing);
        (void)tracker2.roll_exposure(throwing);
        (void)tracker2.roll_exposure(throwing);
        CountingRandom rng(70);
        const AdwareGateResult r = tracker2.roll_exposure(rng);
        REQUIRE(r.outcome == AdwareOutcome::Skip);
        REQUIRE(r.exposure_index == 4);
        REQUIRE(rng.next_calls == 1);
    }
}

TEST_CASE("AdwareExposureTracker: distribuicao 70/30 seedada - enumeracao exaustiva de "
          "next_int 0..99 na 4a exposicao",
          "[domain][combat][adware][tracker]") {
    int show_count = 0;
    int skip_count = 0;
    for (int roll = 0; roll < 100; ++roll) {
        AdwareExposureTracker tracker;
        ThrowingRandom throwing;
        (void)tracker.roll_exposure(throwing);
        (void)tracker.roll_exposure(throwing);
        (void)tracker.roll_exposure(throwing);
        CountingRandom rng(roll);
        const AdwareGateResult r = tracker.roll_exposure(rng);
        if (r.outcome == AdwareOutcome::ShowFull)
            ++show_count;
        else
            ++skip_count;
    }
    REQUIRE(show_count == kAdwareShowChanceAfter3);  // 70
    REQUIRE(skip_count == 100 - kAdwareShowChanceAfter3);  // 30
}

TEST_CASE("AdwareExposureTracker: 5a+ exposicao continua no regime 70/30 (nao volta a "
          "ShowFull garantido)",
          "[domain][combat][adware][tracker]") {
    AdwareExposureTracker tracker;
    ThrowingRandom throwing;
    for (int i = 0; i < 3; ++i) (void)tracker.roll_exposure(throwing);  // 1-3: gratis.
    CountingRandom rng4(10);  // 4a exposicao: <70 -> ShowFull.
    (void)tracker.roll_exposure(rng4);
    REQUIRE(tracker.exposure_count() == 4);

    CountingRandom rng(99);  // >=70 -> Skip, na 5a exposicao.
    const AdwareGateResult r = tracker.roll_exposure(rng);
    REQUIRE(r.outcome == AdwareOutcome::Skip);
    REQUIRE(r.exposure_index == 5);
}

TEST_CASE("adware constants: kAdwareMinWatchSeconds e DADO de UI (5s), nao logica deste "
          "header - Skip nunca modela espera (mesmo struct de ShowFull, sem campo extra)",
          "[domain][combat][adware][tracker]") {
    REQUIRE(kAdwareMinWatchSeconds == 5);
    REQUIRE(kAdwareAlwaysShowThreshold == 3);
    REQUIRE(kAdwareShowChanceAfter3 == 70);
}

// ===== Integracao via CombatStateMachine::resolve_use_card ============================

TEST_CASE("adware integracao: carta SEM has_adware = pipeline identico (nenhum log/tracker "
          "tocado, debito normal)",
          "[domain][combat][adware][integration]") {
    CombatActor h = hero();
    CombatActor e = foe();
    Card card = adware_card("pulso_sem_adware", /*has_adware=*/false, /*mana_cost=*/1);
    auto reg = registry({card});
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE_FALSE(log_has(sm, "Sterling"));
    REQUIRE(sm.adware_exposure_count() == 0);
    REQUIRE(h.mana() == combat_constants::kBaseMana - 1);  // debito normal, sem interferencia.
    REQUIRE(e.hp() < e.max_hp());  // efeito nominal resolveu normalmente.
}

TEST_CASE("adware integracao: 1a exposicao mostra o anuncio completo, loga o texto exato da "
          "spec, e o cast PROSSEGUE (debito de mana + efeito nominal)",
          "[domain][combat][adware][integration]") {
    CombatActor h = hero();
    CombatActor e = foe();
    Card card = adware_card("exec_sterling_ads", /*has_adware=*/true, /*mana_cost=*/1);
    auto reg = registry({card});
    FixedRandom rng;  // canal COMUM default (secao 11), irrelevante pro roll de adware aqui.
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);

    sm.begin_turn();  // refresh de TurnStart: mana = kBaseMana, ANTES de qualquer debito.
    REQUIRE(h.mana() == combat_constants::kBaseMana);
    sm.run_active_turn_to_end();

    REQUIRE(sm.adware_exposure_count() == 1);
    REQUIRE(log_has(sm, "propaganda Sterling dispensada. Prosseguindo com a compila"));
    // O DEBITO (recurso) so acontece DEPOIS do gate ja ter decidido/logado (secao 1: gate
    // ANTES do debito) - aqui confirmamos que o debito de fato aconteceu (prossegue), nao
    // que foi bloqueado/cancelado (NAO ha cancelamento nesta fatia, ver AMB-12).
    REQUIRE(h.mana() == combat_constants::kBaseMana - 1);
    REQUIRE(e.hp() < e.max_hp());  // efeito nominal tambem resolveu (prossegue de verdade).
}

TEST_CASE("adware integracao: exposicoes 2 e 3 (rodadas seguintes) tambem sempre mostram o "
          "anuncio completo",
          "[domain][combat][adware][integration]") {
    CombatActor h = hero("gus", 50, /*spd=*/20);
    CombatActor e = foe("enemy", 500, /*spd=*/1);  // hero sempre abre a rodada.
    Card card = adware_card("exec_sterling_ads2", /*has_adware=*/true, /*mana_cost=*/0,
                            /*power=*/0);
    auto reg = registry({card});
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once_per_round(CombatAction::use_card(card.id, e.id())),
                          &reg, nullptr, &rng);

    for (int round = 0; round < 3; ++round) {
        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();  // hero -> enemy
        sm.begin_turn();
        sm.run_active_turn_to_end();  // enemy passa (nao esta no provider)
        sm.advance_to_next_actor();  // enemy -> hero (proxima rodada)
    }

    REQUIRE(sm.adware_exposure_count() == 3);
    REQUIRE(count_log(sm, "propaganda Sterling dispensada") == 3);
}

TEST_CASE("adware integracao: gate roda ANTES do debito de mana - o log do anuncio ja existe "
          "mesmo quando o debito subsequente falha por mana insuficiente (prova de ordem, "
          "vetor QA de mutacao)",
          "[domain][combat][adware][integration]") {
    CombatActor h = hero();  // mana inicial = kBaseMana (begin_turn abaixo confirma).
    CombatActor e = foe();
    Card card = adware_card("exec_sterling_mana_insuf", /*has_adware=*/true,
                            /*mana_cost=*/combat_constants::kBaseMana + 100);
    auto reg = registry({card});
    FixedRandom rng;
    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(card.id, e.id())), &reg,
                          nullptr, &rng);

    sm.begin_turn();
    REQUIRE(h.mana() == combat_constants::kBaseMana);

    // spend_mana lanca (mana insuficiente) - so alcancavel SE o gate ja rodou antes (secao 1:
    // "gate ANTES do debito"). Se a ordem fosse invertida (debito primeiro), a excecao
    // interromperia o resolve ANTES do tracker/log do adware serem tocados - este teste
    // pegaria a inversao (mutation vetor 2).
    REQUIRE_THROWS_AS(sm.run_active_turn_to_end(), std::logic_error);

    REQUIRE(sm.adware_exposure_count() == 1);
    REQUIRE(log_has(sm, "propaganda Sterling dispensada"));
}

TEST_CASE("adware integracao: 4a exposicao com RNG forcando SKIP - loga texto proprio (nao "
          "'dispensada'), e o cast AINDA PROSSEGUE (sem espera)",
          "[domain][combat][adware][integration]") {
    CombatActor h = hero("gus", 50, /*spd=*/20);
    CombatActor e = foe("enemy", 500, /*spd=*/1);
    Card card = adware_card("exec_sterling_ads3", /*has_adware=*/true, /*mana_cost=*/0,
                            /*power=*/0);
    auto reg = registry({card});
    CountingRandom rng(85);  // >=70 -> Skip na 4a exposicao; 1-3 nao tocam o RNG do adware.
    CombatStateMachine sm({&h, &e}, play_once_per_round(CombatAction::use_card(card.id, e.id())),
                          &reg, nullptr, &rng);

    for (int round = 0; round < 4; ++round) {
        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
    }

    REQUIRE(sm.adware_exposure_count() == 4);
    REQUIRE(count_log(sm, "propaganda Sterling dispensada") == 3);  // so as 3 primeiras.
    REQUIRE(log_has(sm, "propaganda Sterling pulada"));             // a 4a, skip proprio.
    REQUIRE(count_log(sm, "compila " + card.id) == 4);  // as 4 cartas resolveram (prossegue).
}
