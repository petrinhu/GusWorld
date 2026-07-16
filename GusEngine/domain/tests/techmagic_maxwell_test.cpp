// techmagic_maxwell_test.cpp
//
// Spec executavel (Catch2 v3) do Maxwell (Spectra-Wave/"Onda Unificada"), manifesto item 12
// (ADR-016, decisao do lider 2026-07-16, AMB-08 em
// docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md).
//
// Maxwell e a carta ESPECIAL mais simples do executor: Hibrida/Eletrico/TargetShape::Grupo,
// effects VAZIO (nenhum EffectSpec, nenhum EffectKind novo). "Onda Unificada" e dano-base
// PURO - reusa o MESMO caminho Grupo/AoE ja pronto do Newton (resolve_targets/
// resolve_use_card): a cadeia divisiva de sempre (secao 11), com card.power somado ao ATK do
// conjurador, atinge TODOS os inimigos vivos do lado oposto ao caster. Como nao ha OnCast pra
// despachar (card.effects.empty()), o bloco `if (!card.effects.empty())` de resolve_use_card
// nunca roda pra esta carta - ZERO wiring novo, so o dado (Card) muda.
//
// Cobre:
//   1. Cast contra inimigos: TODOS os inimigos VIVOS tomam dano do power+ATK do conjurador;
//      inimigo ja morto fica de fora (mesmo padrao do teste 1 de techmagic_newton_test.cpp).
//   2. Paridade preview<->real: estimate_card_damage (PURO, sem consumir RNG) bate
//      EXATAMENTE com o dano resolvido pelo motor real nas fronteiras do canal COMUM (r=0.0
//      piso / r=1.0 teto, FixedRandom), o MESMO helper comum_channel_damage dos dois lados.
//   3. Cast mirando ALIADO: dissipa - dano 0 + log de dissipacao ("fogo amigo desligado").
//      Maxwell NAO tem EffectSpec (ao contrario do Newton, que ainda concede Reflect-status
//      no modo-aliado) - o ramo assimetrico de resolve_targets devolve o aliado como alvo
//      unico, mas sem OnCast pra rodar o resultado e um no-op PURO (nada alem do log de
//      dissipacao do guard de fogo amigo).
//
// Cartas de teste montadas LOCALMENTE (id "techmagic.maxwell.*"), NUNCA do registry de
// producao (MasterCards) - o teste do CATALOGO (Maxwell em MasterCards::build_registry())
// vive em master_cards_test.cpp. Mesma convencao de techmagic_newton_test.cpp/
// techmagic_faraday_test.cpp/techmagic_chain_test.cpp.
//
// Cross-ref: gus/domain/combat/master_cards.hpp (maxwell); combat_state_machine.cpp
//            (resolve_targets Grupo, regra geral "fogo amigo desligado", estimate_card_damage);
//            docs/design/mecanicas/cartas-technomagik.md; docs/design/roster-analogos/
//            _EFEITOS-ESCOLHIDOS.md (AMB-08); ADR-016.

#include <catch2/catch_test_macros.hpp>

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "counting_random.hpp"
#include "fixed_random.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::CountingRandom;
using gus::domain::tests::FixedRandom;

namespace {

CombatActor make_actor(const std::string& id, bool player_side, int hp = 100, int atk = 0,
                       int def = 0, int spd = 20) {
    return CombatActor(id, id, hp, atk, def, spd, CardFamily::Eletrico, player_side);
}

int count_log_matches(const CombatStateMachine& sm, const std::string& needle) {
    int n = 0;
    for (const auto& e : sm.log())
        if (e.message.find(needle) != std::string::npos) ++n;
    return n;
}

// Duplo LOCAL do Maxwell de producao (master_cards.cpp): Hibrida/Eletrico/Grupo, effects
// VAZIO (dano-base puro, ZERO EffectSpec).
Card maxwell_card(const std::string& id, int power = 5) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Eletrico;
    c.base_type = CardBaseType::Glifo;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = TargetShape::Grupo;
    c.tier = CardTier::Especial;
    c.category = CardCategory::Hibrida;
    // effects deliberadamente vazio (dano-base puro).
    return c;
}

std::unordered_map<std::string, Card> registry(std::initializer_list<Card> cards) {
    std::unordered_map<std::string, Card> d;
    for (const auto& c : cards) d.emplace(c.id, c);
    return d;
}

using ActionPlan =
    std::unordered_map<std::string, std::unordered_map<int, std::vector<CombatAction>>>;

CombatActionProvider provider_for(ActionPlan plan) {
    auto plan_ptr = std::make_shared<ActionPlan>(std::move(plan));
    auto idx_ptr = std::make_shared<std::map<std::pair<std::string, int>, std::size_t>>();
    return [plan_ptr, idx_ptr](CombatActor& a, const CombatState& state) -> CombatAction {
        const auto ait = plan_ptr->find(a.id());
        if (ait == plan_ptr->end()) return CombatAction::pass();
        const auto rit = ait->second.find(state.round_index());
        if (rit == ait->second.end()) return CombatAction::pass();
        std::size_t& i = (*idx_ptr)[{a.id(), state.round_index()}];
        if (i < rit->second.size()) return rit->second[i++];
        return CombatAction::pass();
    };
}

}  // namespace

// ===== 1. Cast contra inimigo: TODOS os inimigos vivos tomam dano; morto fica de fora =====

TEST_CASE("techmagic maxwell: cast (Onda Unificada) contra inimigo - TODOS os inimigos "
         "VIVOS tomam dano do power+ATK do conjurador; inimigo ja morto fica de fora",
         "[domain][combat][techmagic][maxwell]") {
    CombatActor h = make_actor("h", true, 100, /*atk=*/10, 0, /*spd=*/50);
    CombatActor e1 = make_actor("e1", false, 100, 0, 0, /*spd=*/40);
    CombatActor e2 = make_actor("e2", false, 100, 0, 0, /*spd=*/30);
    CombatActor e3 = make_actor("e3", false, 100, 0, 0, /*spd=*/20);
    e3.take_damage(9999);  // ja morto ANTES do cast.
    REQUIRE_FALSE(e3.is_alive());

    const Card maxwell = maxwell_card("techmagic.maxwell.aoe");
    auto reg = registry({maxwell});
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);  // canal COMUM, sem crit/fumble.
    auto provider =
        provider_for({{"h", {{0, {CombatAction::use_card(maxwell.id, e1.id())}}}}});
    CombatStateMachine sm({&h, &e1, &e2, &e3}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(e1.hp() < e1.max_hp());
    REQUIRE(e2.hp() < e2.max_hp());
    REQUIRE(e3.hp() == 0);  // ja morto: nunca foi alvo (fora de queue_.order() vivo).

    // Mesmo dano-base pros dois (mesmo atk do conjurador, mesmo def/family dos alvos, canal
    // COMUM deterministico via FixedRandom) - prova que o AoE trata os dois simetricamente.
    REQUIRE(e1.hp() == e2.hp());

    // Regra "todo efeito loga": 2 linhas de UseCard (e1, e2), nenhuma de dissipacao (nao ha
    // alvo aliado neste cenario).
    REQUIRE(count_log_matches(sm, "compila techmagic.maxwell.aoe") == 2);
}

// ===== 2. Paridade preview<->real: estimate_card_damage bate com o motor real nas =====
// =====    fronteiras do canal COMUM (r=0.0 piso / r=1.0 teto) =================================

TEST_CASE("techmagic maxwell: paridade preview - estimate_card_damage bate EXATAMENTE com "
         "o dano resolvido pelo motor real (piso r=0.0 e teto r=1.0, canal COMUM)",
         "[domain][combat][techmagic][maxwell]") {
    const Card maxwell = maxwell_card("techmagic.maxwell.parity");

    // Piso (r=0.0): variancia MINIMA.
    {
        CombatActor h = make_actor("h", true, 100, /*atk=*/10, 0, /*spd=*/50);
        CombatActor e1 = make_actor("e1", false, 500, 0, /*def=*/2, /*spd=*/40);
        auto reg = registry({maxwell});
        FixedRandom preview_rng;  // preview e PURO, nao consome - RNG so pra construir a FSM.
        CombatStateMachine preview_sm({&h, &e1}, provider_for({}), &reg, nullptr, &preview_rng);
        const CardDamageEstimate est = preview_sm.estimate_card_damage(h, e1, maxwell);
        REQUIRE_FALSE(est.immune);

        FixedRandom rng(/*next_double=*/0.0, /*next_int=*/99);  // COMUM, r=0.0 (piso).
        auto provider =
            provider_for({{"h", {{0, {CombatAction::use_card(maxwell.id, e1.id())}}}}});
        CombatStateMachine sm({&h, &e1}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();

        REQUIRE(e1.max_hp() - e1.hp() == est.min_damage);
    }

    // Teto (r=1.0): variancia MAXIMA.
    {
        CombatActor h = make_actor("h", true, 100, /*atk=*/10, 0, /*spd=*/50);
        CombatActor e2 = make_actor("e2", false, 500, 0, /*def=*/2, /*spd=*/40);
        auto reg = registry({maxwell});
        FixedRandom preview_rng;
        CombatStateMachine preview_sm({&h, &e2}, provider_for({}), &reg, nullptr, &preview_rng);
        const CardDamageEstimate est = preview_sm.estimate_card_damage(h, e2, maxwell);

        FixedRandom rng(/*next_double=*/1.0, /*next_int=*/99);  // COMUM, r=1.0 (teto).
        auto provider =
            provider_for({{"h", {{0, {CombatAction::use_card(maxwell.id, e2.id())}}}}});
        CombatStateMachine sm({&h, &e2}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();

        REQUIRE(e2.max_hp() - e2.hp() == est.max_damage);
    }
}

// ===== 3. Cast mirando ALIADO: dissipa - dano 0, no-op puro (SEM EffectSpec pra rodar) =====

TEST_CASE("techmagic maxwell: cast mirando ALIADO - dissipa (dano 0 + log de dissipacao), "
         "no-op puro (carta sem EffectSpec, nao ha nada alem do guard de fogo amigo)",
         "[domain][combat][techmagic][maxwell]") {
    CombatActor h = make_actor("h", true, 100, /*atk=*/10, 0, /*spd=*/50);
    CombatActor buddy = make_actor("buddy", true, 100, 0, 0, /*spd=*/40);
    CombatActor e1 = make_actor("e1", false, 100, 0, 0, /*spd=*/30);

    const Card maxwell = maxwell_card("techmagic.maxwell.allymode");
    auto reg = registry({maxwell});
    FixedRandom rng(0.5, 99);
    auto provider =
        provider_for({{"h", {{0, {CombatAction::use_card(maxwell.id, buddy.id())}}}}});
    CombatStateMachine sm({&h, &buddy, &e1}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(buddy.hp() == buddy.max_hp());          // fogo amigo desligado: dano 0.
    REQUIRE(buddy.status_effects().empty());        // sem EffectSpec: NENHUM status concedido
                                                     // (ao contrario do Newton, N-3).
    REQUIRE(e1.hp() == e1.max_hp());                // nenhum inimigo alcancado (ramo assimetrico
                                                     // devolve SO o aliado declarado).
    REQUIRE(count_log_matches(sm, "aliado, fogo amigo desligado") == 1);
    REQUIRE(count_log_matches(sm, "compila techmagic.maxwell.allymode") == 1);  // so a linha
                                                                                 // de dissipacao.
}

// ===== 4. Gate 1x/batalha: castar Maxwell 2x na MESMA batalha lanca logic_error =====
// =====    (mesmo gate specials_cast_ das outras Ativa/Hibrida - Faraday/Godel/Dee) ===========

TEST_CASE("techmagic maxwell: castar 2x na mesma batalha lanca logic_error (gate "
         "1x/batalha das especiais Ativa/Hibrida via specials_cast_)",
         "[domain][combat][techmagic][maxwell]") {
    // Reusa a resolucao DIRETA (resolve_action) pra provar o gate sem depender do provider/
    // ramp de mana - mesmo padrao de techmagic_faraday_test/godel_test. Duas cartas cast
    // seguidas: a 1a insere "maxwell" em specials_cast_; a 2a bate no gate e lanca.
    CombatActor h = make_actor("h", true, 100, /*atk=*/10, 0, /*spd=*/50);
    CombatActor e1 = make_actor("e1", false, 500, 0, 0, /*spd=*/40);

    const Card maxwell = maxwell_card("techmagic.maxwell.gate");
    auto reg = registry({maxwell});
    FixedRandom rng(0.5, 99);
    // 2 casts de Maxwell na MESMA rodada (mesma batalha): o 2o deve lancar no gate.
    auto provider = provider_for(
        {{"h",
          {{0, {CombatAction::use_card(maxwell.id, e1.id()),
                CombatAction::use_card(maxwell.id, e1.id())}}}}});
    CombatStateMachine sm({&h, &e1}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    // O 2o cast (dentro do mesmo turno de h) bate no gate specials_cast_ e propaga o
    // logic_error da FSM (Maxwell e Hibrida, sujeita ao 1x/batalha).
    REQUIRE_THROWS_AS(sm.run_active_turn_to_end(), std::logic_error);
}

// ===== 5. Determinismo de RNG do caminho Grupo (invariante sagrado, gemeo do Newton #905): =
// =====    N alvos vivos consomem EXATAMENTE N*next() + N*next_double(), nem mais nem menos ==

TEST_CASE("techmagic maxwell: determinismo - o AoE consome EXATAMENTE 1 next() + 1 "
         "next_double() por inimigo VIVO (3 vivos = 3+3; morto fora nao sorteia)",
         "[domain][combat][techmagic][maxwell]") {
    CombatActor h = make_actor("h", true, 100, /*atk=*/10, 0, /*spd=*/50);
    CombatActor e1 = make_actor("e1", false, 500, 0, 0, /*spd=*/40);
    CombatActor e2 = make_actor("e2", false, 500, 0, 0, /*spd=*/30);
    CombatActor e3 = make_actor("e3", false, 500, 0, 0, /*spd=*/25);
    CombatActor e4 = make_actor("e4", false, 500, 0, 0, /*spd=*/20);
    e4.take_damage(9999);  // ja morto ANTES do cast: NAO deve sortear nada.
    REQUIRE_FALSE(e4.is_alive());

    const Card maxwell = maxwell_card("techmagic.maxwell.rng");
    auto reg = registry({maxwell});
    CountingRandom rng;  // roll fixo 99 (COMUM, sem fumble/crit) + next_double fixo 0.5.
    auto provider =
        provider_for({{"h", {{0, {CombatAction::use_card(maxwell.id, e1.id())}}}}});
    CombatStateMachine sm({&h, &e1, &e2, &e3, &e4}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    // 3 alvos vivos (e1,e2,e3), cada um COMUM (roll 99 fixo): 1 next() + 1 next_double() por
    // alvo. Exatamente 3+3 prova (a) cada alvo sorteia INDEPENDENTE (nao 1 sorteio pro grupo),
    // (b) o inimigo MORTO (e4) nao consome, (c) ZERO consumo espurio na Maxwell (mata o
    // mutante "RNG extra"). Gemeo do teste de determinismo do Newton (techmagic_newton_test.cpp).
    REQUIRE(rng.next_calls == 3);
    REQUIRE(rng.next_double_calls == 3);
}
