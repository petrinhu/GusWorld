// urandom_test.cpp
//
// Spec executavel (Catch2 v3) da fatia B da onda CARDS-HW-2 (carta `urandom`, a carta-caos
// do Gus): docs/design/mecanicas/cartas-spec-logica.md secao 7,
// docs/design/mecanicas/cartas-numeros-proposta.md secao 4.
//
// Duas camadas de cobertura:
//   1. gus/domain/combat/urandom_algorithm.hpp: as 2 funcoes PURAS (weighted_pick_urandom_
//      faixa/classify_urandom_faixa) - fronteiras exatas das tabelas de peso + distribuicao
//      estatistica sob RNG seedado (PropertyRandom) + classificacao fraco/medio/forte/jackpot.
//   2. CombatStateMachine::resolve_use_card (branch por card_id "urandom",
//      resolve_urandom/resolve_redirected_card_effect): a integracao completa - backfire
//      SEMPRE no caster, pool vazio dissipa, anti-recursao (urandom nunca se sorteia), 3
//      draws deterministicos (faixa/carta/lado), e o redirecionamento produz efeito
//      IDENTICO ao cast direto da carta sorteada (mesmo resolvedor, so entra por outra
//      porta).
//
// Cartas de teste montadas LOCALMENTE (nunca do registry de producao), mesma convencao de
// card_virus_combat_test.cpp/techmagic_faraday_test.cpp.
//
// Cross-ref: gus/domain/combat/urandom_algorithm.hpp; gus/domain/combat/
//            card_collection_snapshot.hpp; gus/domain/combat/combat_state_machine.cpp
//            (resolve_urandom/resolve_redirected_card_effect).

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "fixed_random.hpp"
#include "property_gen.hpp"
#include "gus/domain/combat/card_collection_snapshot.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/random_source.hpp"
#include "gus/domain/combat/urandom_algorithm.hpp"
#include "gus/domain/hardware/card_provenance.hpp"

using namespace gus::domain::combat;
using gus::domain::hardware::CardOrigin;
using gus::domain::tests::FixedRandom;
using gus::domain::tests::PropertyRandom;

namespace {

CombatActor make_actor(const std::string& id, bool player_side, int hp = 100, int atk = 10,
                       int def = 0, int spd = 20, CardFamily family = CardFamily::Universal) {
    return CombatActor(id, id, hp, atk, def, spd, family, player_side);
}

const StatusEffect* find_status(const CombatActor& a, StatusId id) {
    for (const auto& s : a.status_effects())
        if (s.id == id) return &s;
    return nullptr;
}

std::unordered_map<std::string, Card> registry(std::initializer_list<Card> cards) {
    std::unordered_map<std::string, Card> d;
    for (const auto& c : cards) d.emplace(c.id, c);
    return d;
}

// Carta urandom de teste: mana 0 (foco no algoritmo, nao no ramp de mana), TargetShape::Self
// (o alvo REAL e resolvido dentro de resolve_urandom, nao pelo pipeline normal), effects
// vazio (o efeito inteiro roda fora do dispatcher techMagic, mesmo padrao de Planck/Hayek).
Card urandom_card(int mana_cost = 0) {
    Card c;
    c.id = "urandom";
    c.display_name = "urandom";
    c.family = CardFamily::Universal;
    c.base_type = CardBaseType::Glifo;
    c.mana_cost = mana_cost;
    c.ap_cost = 1;
    c.power = 0;
    c.target_shape = TargetShape::Self;
    c.tier = CardTier::Especial;
    c.category = CardCategory::Ativa;
    return c;
}

// Carta COMUM de teste classificavel por faixa (mana_cost 1/2/3 = fraco/medio/forte).
Card comum_card(const std::string& id, int mana_cost, int power,
                std::optional<StatusEffect> status = std::nullopt) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Cinetico;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = mana_cost;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = TargetShape::Single;
    c.status_applied = status;
    c.tier = CardTier::Comum;
    return c;
}

// Carta ESPECIAL de teste (jackpot): programa techMagic minimo (ApplyStatus).
Card especial_card(const std::string& id, StatusId status_id, int magnitude, int duration) {
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
    c.category = CardCategory::Ativa;
    c.effects = {EffectSpec{.trigger = TriggerHook::OnCast,
                            .kind = EffectKind::ApplyStatus,
                            .magnitude = magnitude,
                            .duration = duration,
                            .status = status_id,
                            .stack_rule = StackRule::Replace}};
    return c;
}

CombatActionProvider play_sequence(std::vector<CombatAction> actions) {
    auto acts = std::make_shared<std::vector<CombatAction>>(std::move(actions));
    auto idx = std::make_shared<std::size_t>(0);
    return [acts, idx](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side() || *idx >= acts->size()) return CombatAction::pass();
        return (*acts)[(*idx)++];
    };
}

bool log_has(const CombatStateMachine& sm, const std::string& needle) {
    for (const auto& entry : sm.log())
        if (entry.message.find(needle) != std::string::npos) return true;
    return false;
}

// Duplo de IRandomSource que devolve uma SEQUENCIA pre-cravada de next() (consumida em
// ordem; esgotada => 0) e um next_double() fixo. Conta consumos dos dois metodos (mesmo
// padrao de CountingRandom, mas com controle fino por-draw - necessario pra cravar
// faixa/pool-idx/lado do urandom EM SEQUENCIA, sem depender de um unico valor fixo).
class SequenceRandom final : public IRandomSource {
public:
    explicit SequenceRandom(std::vector<int> next_values, double next_double_value = 0.5)
        : values_(std::move(next_values)), next_double_value_(next_double_value) {}

    double next_double() override {
        ++next_double_calls;
        return next_double_value_;
    }

    int next(int max_value) override {
        ++next_calls;
        if (max_value <= 0) return 0;
        const int v = idx_ < values_.size() ? values_[idx_++] : 0;
        return std::min(v, max_value - 1);
    }

    int next_calls = 0;
    int next_double_calls = 0;

private:
    std::vector<int> values_;
    std::size_t idx_ = 0;
    double next_double_value_;
};

}  // namespace

// ============================================================================================
// 1. urandom_algorithm.hpp: funcoes PURAS (weighted_pick_urandom_faixa/classify_urandom_faixa)
// ============================================================================================

TEST_CASE("urandom_algorithm: weighted_pick fronteiras EXATAS da tabela ORIGINAL "
         "(fraco 21/medio 34/forte 21/jackpot 8, total 84)",
         "[domain][combat][urandom][algorithm]") {
    REQUIRE(kUrandomOriginalWeightsCount == 4);

    // Fraco: [0, 21).
    {
        FixedRandom rng(0.5, 0);
        REQUIRE(weighted_pick_urandom_faixa(kUrandomOriginalWeights, kUrandomOriginalWeightsCount,
                                            rng) == UrandomFaixa::Fraco);
    }
    {
        FixedRandom rng(0.5, 20);
        REQUIRE(weighted_pick_urandom_faixa(kUrandomOriginalWeights, kUrandomOriginalWeightsCount,
                                            rng) == UrandomFaixa::Fraco);
    }
    // Medio: [21, 55).
    {
        FixedRandom rng(0.5, 21);
        REQUIRE(weighted_pick_urandom_faixa(kUrandomOriginalWeights, kUrandomOriginalWeightsCount,
                                            rng) == UrandomFaixa::Medio);
    }
    {
        FixedRandom rng(0.5, 54);
        REQUIRE(weighted_pick_urandom_faixa(kUrandomOriginalWeights, kUrandomOriginalWeightsCount,
                                            rng) == UrandomFaixa::Medio);
    }
    // Forte: [55, 76).
    {
        FixedRandom rng(0.5, 55);
        REQUIRE(weighted_pick_urandom_faixa(kUrandomOriginalWeights, kUrandomOriginalWeightsCount,
                                            rng) == UrandomFaixa::Forte);
    }
    {
        FixedRandom rng(0.5, 75);
        REQUIRE(weighted_pick_urandom_faixa(kUrandomOriginalWeights, kUrandomOriginalWeightsCount,
                                            rng) == UrandomFaixa::Forte);
    }
    // Jackpot: [76, 84).
    {
        FixedRandom rng(0.5, 76);
        REQUIRE(weighted_pick_urandom_faixa(kUrandomOriginalWeights, kUrandomOriginalWeightsCount,
                                            rng) == UrandomFaixa::Jackpot);
    }
    {
        FixedRandom rng(0.5, 83);
        REQUIRE(weighted_pick_urandom_faixa(kUrandomOriginalWeights, kUrandomOriginalWeightsCount,
                                            rng) == UrandomFaixa::Jackpot);
    }
}

TEST_CASE("urandom_algorithm: weighted_pick fronteiras EXATAS da tabela PIRATA (fraco "
         "7/medio 2/forte 1/jackpot 0/backfire 5, total 15, backfire = 1/3 exato)",
         "[domain][combat][urandom][algorithm]") {
    REQUIRE(kUrandomPirataWeightsCount == 5);

    // Fraco: [0, 7).
    {
        FixedRandom rng(0.5, 0);
        REQUIRE(weighted_pick_urandom_faixa(kUrandomPirataWeights, kUrandomPirataWeightsCount,
                                            rng) == UrandomFaixa::Fraco);
    }
    {
        FixedRandom rng(0.5, 6);
        REQUIRE(weighted_pick_urandom_faixa(kUrandomPirataWeights, kUrandomPirataWeightsCount,
                                            rng) == UrandomFaixa::Fraco);
    }
    // Medio: [7, 9).
    {
        FixedRandom rng(0.5, 7);
        REQUIRE(weighted_pick_urandom_faixa(kUrandomPirataWeights, kUrandomPirataWeightsCount,
                                            rng) == UrandomFaixa::Medio);
    }
    {
        FixedRandom rng(0.5, 8);
        REQUIRE(weighted_pick_urandom_faixa(kUrandomPirataWeights, kUrandomPirataWeightsCount,
                                            rng) == UrandomFaixa::Medio);
    }
    // Forte: [9, 10).
    {
        FixedRandom rng(0.5, 9);
        REQUIRE(weighted_pick_urandom_faixa(kUrandomPirataWeights, kUrandomPirataWeightsCount,
                                            rng) == UrandomFaixa::Forte);
    }
    // Jackpot: peso 0 - banda [10, 10), INALCANCAVEL por construcao (nunca puxa uma
    // especial de verdade na pirata). Nao ha valor de roll que resolva Jackpot aqui.
    // Backfire: [10, 15) = 1/3 exato (5/15).
    {
        FixedRandom rng(0.5, 10);
        REQUIRE(weighted_pick_urandom_faixa(kUrandomPirataWeights, kUrandomPirataWeightsCount,
                                            rng) == UrandomFaixa::Backfire);
    }
    {
        FixedRandom rng(0.5, 14);
        REQUIRE(weighted_pick_urandom_faixa(kUrandomPirataWeights, kUrandomPirataWeightsCount,
                                            rng) == UrandomFaixa::Backfire);
    }
}

TEST_CASE("urandom_algorithm: distribuicao estatistica (RNG seedado, N=40000) casa com a "
         "tabela ORIGINAL dentro de tolerancia, ZERO backfire",
         "[domain][combat][urandom][algorithm][stats]") {
    constexpr int kTrials = 40000;
    int fraco = 0, medio = 0, forte = 0, jackpot = 0, backfire = 0;
    PropertyRandom rng(12345u);
    for (int i = 0; i < kTrials; ++i) {
        switch (weighted_pick_urandom_faixa(kUrandomOriginalWeights, kUrandomOriginalWeightsCount,
                                            rng)) {
            case UrandomFaixa::Fraco: ++fraco; break;
            case UrandomFaixa::Medio: ++medio; break;
            case UrandomFaixa::Forte: ++forte; break;
            case UrandomFaixa::Jackpot: ++jackpot; break;
            case UrandomFaixa::Backfire: ++backfire; break;
        }
    }
    REQUIRE(backfire == 0);  // original NUNCA tem backfire (cartas-numeros secao 4).
    const double n = static_cast<double>(kTrials);
    // Tolerancia +-2pp em torno de 25.0/40.5/25.0/9.5%.
    REQUIRE(std::abs(fraco / n - 0.250) < 0.02);
    REQUIRE(std::abs(medio / n - 0.405) < 0.02);
    REQUIRE(std::abs(forte / n - 0.250) < 0.02);
    REQUIRE(std::abs(jackpot / n - 0.095) < 0.02);
}

TEST_CASE("urandom_algorithm: distribuicao estatistica (RNG seedado, N=40000) casa com a "
         "tabela PIRATA dentro de tolerancia, JAMAIS jackpot, backfire ~1/3",
         "[domain][combat][urandom][algorithm][stats]") {
    constexpr int kTrials = 40000;
    int fraco = 0, medio = 0, forte = 0, jackpot = 0, backfire = 0;
    PropertyRandom rng(67890u);
    for (int i = 0; i < kTrials; ++i) {
        switch (weighted_pick_urandom_faixa(kUrandomPirataWeights, kUrandomPirataWeightsCount,
                                            rng)) {
            case UrandomFaixa::Fraco: ++fraco; break;
            case UrandomFaixa::Medio: ++medio; break;
            case UrandomFaixa::Forte: ++forte; break;
            case UrandomFaixa::Jackpot: ++jackpot; break;
            case UrandomFaixa::Backfire: ++backfire; break;
        }
    }
    REQUIRE(jackpot == 0);  // pirata NUNCA acerta o jackpot de especial de verdade.
    const double n = static_cast<double>(kTrials);
    REQUIRE(std::abs(fraco / n - 0.467) < 0.02);
    REQUIRE(std::abs(medio / n - 0.133) < 0.02);
    REQUIRE(std::abs(forte / n - 0.067) < 0.02);
    REQUIRE(std::abs(backfire / n - 0.333) < 0.02);  // 1/3 exato.
}

TEST_CASE("urandom_algorithm: classify_urandom_faixa - COMUM por ManaCost 1/2/3 = "
         "fraco/medio/forte; tier != Comum = jackpot; sem classe = nullopt",
         "[domain][combat][urandom][algorithm]") {
    REQUIRE(classify_urandom_faixa(comum_card("jab", /*mana=*/1, /*power=*/5)) ==
           UrandomFaixa::Fraco);
    REQUIRE(classify_urandom_faixa(comum_card("golpe", /*mana=*/2, /*power=*/8)) ==
           UrandomFaixa::Medio);
    REQUIRE(classify_urandom_faixa(comum_card("assinatura", /*mana=*/3, /*power=*/12)) ==
           UrandomFaixa::Forte);
    REQUIRE(classify_urandom_faixa(especial_card("qualquer", StatusId::Stun, 0, 1)) ==
           UrandomFaixa::Jackpot);
    // Comum com ManaCost fora de {1,2,3} nao classifica em nenhuma faixa.
    REQUIRE_FALSE(classify_urandom_faixa(comum_card("gratis", /*mana=*/0, /*power=*/1))
                     .has_value());
    REQUIRE_FALSE(classify_urandom_faixa(comum_card("caro", /*mana=*/4, /*power=*/1))
                     .has_value());
}

// ============================================================================================
// 2. Integracao FSM: CombatStateMachine::resolve_use_card branch "urandom"
// ============================================================================================

// ----- Backfire (SO pirata): status ruim SEMPRE no caster, NUNCA no inimigo -----

TEST_CASE("urandom FSM: pirata sorteia Backfire -> status negativo leve SEMPRE no CASTER, "
         "alvo intacto, 2 draws (faixa + pool do backfire)",
         "[domain][combat][urandom][fsm][backfire]") {
    CombatActor caster = make_actor("h", true);
    CombatActor target = make_actor("e", false, /*hp=*/300);

    Card urandom = urandom_card();
    auto reg = registry({urandom});

    std::vector<CardCollectionEntry> snapshot = {
        CardCollectionEntry{/*instance_id=*/0, urandom.id, /*owner_actor_id=*/1,
                            CardOrigin::PirateClone}};

    CombatAction cast = CombatAction::use_card(urandom.id, caster.id());
    cast.card_instance_id = 0;

    // draw1 = 10 (banda Backfire [10,15) da tabela pirata); draw2 = 0 (indice do pool de
    // status ruim -> kUrandomBackfirePool[0] = Stun).
    SequenceRandom rng({10, 0});
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng, nullptr, &snapshot);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(target.hp() == target.max_hp());          // inimigo INTOCADO.
    REQUIRE(find_status(caster, StatusId::Stun) != nullptr);  // backfire aplicado no caster.
    REQUIRE(log_has(sm, "PIRATA FALHA"));
    REQUIRE(rng.next_calls == 2);
    REQUIRE(rng.next_double_calls == 0);
}

TEST_CASE("urandom FSM: original (nao-pirata) NUNCA sorteia Backfire - banda inexistente na "
         "tabela ORIGINAL, mesmo forcando o maior roll possivel",
         "[domain][combat][urandom][fsm][backfire]") {
    CombatActor caster = make_actor("h", true);
    CombatActor target = make_actor("e", false, /*hp=*/300);

    Card urandom = urandom_card();
    auto reg = registry({urandom});

    std::vector<CardCollectionEntry> snapshot = {
        CardCollectionEntry{/*instance_id=*/0, urandom.id, /*owner_actor_id=*/1,
                            CardOrigin::OriginalRom}};

    CombatAction cast = CombatAction::use_card(urandom.id, caster.id());
    cast.card_instance_id = 0;

    // draw1 = 83 (topo da banda Jackpot [76,84) da tabela ORIGINAL - nunca Backfire, que nem
    // existe nessa tabela). Pool vazio (nenhuma especial possuida) -> dissipa.
    SequenceRandom rng({83});
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng, nullptr, &snapshot);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE_FALSE(log_has(sm, "FALHA"));
    REQUIRE(find_status(caster, StatusId::Stun) == nullptr);
    REQUIRE(find_status(caster, StatusId::Poison) == nullptr);
}

// ----- Pool vazio (AMB-09): dissipa, 1 draw -----

TEST_CASE("urandom FSM: pool vazio (faixa sorteada sem carta correspondente na colecao) -> "
         "dissipa (log, nada acontece), 1 unico draw",
         "[domain][combat][urandom][fsm][dissipate]") {
    CombatActor caster = make_actor("h", true);
    CombatActor target = make_actor("e", false, /*hp=*/300);

    Card urandom = urandom_card();
    Card fraco_card = comum_card("jab", /*mana=*/1, /*power=*/9);
    auto reg = registry({urandom, fraco_card});

    // Colecao do caster so tem uma carta FRACO; forcar faixa=Forte -> pool vazio.
    std::vector<CardCollectionEntry> snapshot = {
        CardCollectionEntry{/*instance_id=*/0, urandom.id, /*owner_actor_id=*/1,
                            CardOrigin::OriginalRom},
        CardCollectionEntry{/*instance_id=*/1, fraco_card.id, /*owner_actor_id=*/1,
                            CardOrigin::OriginalRom}};

    CombatAction cast = CombatAction::use_card(urandom.id, caster.id());
    cast.card_instance_id = 0;

    SequenceRandom rng({55});  // banda Forte [55,76) da tabela ORIGINAL.
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng, nullptr, &snapshot);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(target.hp() == target.max_hp());
    REQUIRE(log_has(sm, "dissipa"));
    REQUIRE(rng.next_calls == 1);
}

// ----- Anti-recursao: a propria urandom nunca entra no pool -----

TEST_CASE("urandom FSM: anti-recursao - a propria carta urandom (mesmo em 2 copias) NUNCA "
         "entra no pool, mesmo classificando Jackpot (tier Especial)",
         "[domain][combat][urandom][fsm][antirecursion]") {
    CombatActor caster = make_actor("h", true);
    CombatActor target = make_actor("e", false, /*hp=*/300);

    Card urandom = urandom_card();
    auto reg = registry({urandom});

    // Colecao do caster tem 2 instancias de urandom (a jogada + outra copia) e MAIS NADA -
    // se nao houvesse anti-recursao, Jackpot acharia a 2a copia (tier Especial).
    std::vector<CardCollectionEntry> snapshot = {
        CardCollectionEntry{/*instance_id=*/0, urandom.id, /*owner_actor_id=*/1,
                            CardOrigin::OriginalRom},
        CardCollectionEntry{/*instance_id=*/1, urandom.id, /*owner_actor_id=*/1,
                            CardOrigin::OriginalRom}};

    CombatAction cast = CombatAction::use_card(urandom.id, caster.id());
    cast.card_instance_id = 0;

    SequenceRandom rng({76});  // banda Jackpot [76,84) da tabela ORIGINAL.
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng, nullptr, &snapshot);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(target.hp() == target.max_hp());
    REQUIRE(log_has(sm, "dissipa"));  // pool ficou vazio - nunca a propria urandom.
}

// ----- Fail-safe: sem snapshot / sem card_instance_id / instancia ausente -> dissipa -----

TEST_CASE("urandom FSM: sem snapshot injetado (nullptr, default) -> dissipa (fail-safe), "
         "ZERO consumo de RNG",
         "[domain][combat][urandom][fsm][failsafe]") {
    CombatActor caster = make_actor("h", true);
    CombatActor target = make_actor("e", false, /*hp=*/300);

    Card urandom = urandom_card();
    auto reg = registry({urandom});

    CombatAction cast = CombatAction::use_card(urandom.id, caster.id());
    cast.card_instance_id = 0;

    SequenceRandom rng({0});
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng);  // sem snapshot.

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(target.hp() == target.max_hp());
    REQUIRE(log_has(sm, "dissipa"));
    REQUIRE(rng.next_calls == 0);
}

TEST_CASE("urandom FSM: card_instance_id ausente na acao -> dissipa (fail-safe)",
         "[domain][combat][urandom][fsm][failsafe]") {
    CombatActor caster = make_actor("h", true);
    CombatActor target = make_actor("e", false, /*hp=*/300);

    Card urandom = urandom_card();
    auto reg = registry({urandom});
    std::vector<CardCollectionEntry> snapshot = {
        CardCollectionEntry{0, urandom.id, 1, CardOrigin::OriginalRom}};

    CombatAction cast = CombatAction::use_card(urandom.id, caster.id());
    // cast.card_instance_id NAO setado (nullopt).

    SequenceRandom rng({0});
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng, nullptr, &snapshot);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(target.hp() == target.max_hp());
    REQUIRE(log_has(sm, "dissipa"));
    REQUIRE(rng.next_calls == 0);
}

// ----- Contagem deterministica de draws (secao 11: determinismo canonico) -----

TEST_CASE("urandom FSM: redirecionamento bem-sucedido consome EXATAMENTE 3 draws (faixa + "
         "carta + lado) quando o lado sorteado e SELF (0 draws downstream)",
         "[domain][combat][urandom][fsm][determinism]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100);
    CombatActor target = make_actor("e", false, /*hp=*/300);

    Card urandom = urandom_card();
    Card jab = comum_card("jab", /*mana=*/1, /*power=*/9);
    auto reg = registry({urandom, jab});

    std::vector<CardCollectionEntry> snapshot = {
        CardCollectionEntry{0, urandom.id, 1, CardOrigin::OriginalRom},
        CardCollectionEntry{1, jab.id, 1, CardOrigin::OriginalRom}};

    CombatAction cast = CombatAction::use_card(urandom.id, caster.id());
    cast.card_instance_id = 0;

    // draw1=0 (Fraco), draw2=0 (unico candidato do pool), draw3=0 (lado=SELF - fogo amigo
    // desligado, nenhum draw a mais no redirect).
    SequenceRandom rng({0, 0, 0});
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng, nullptr, &snapshot);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(rng.next_calls == 3);
    REQUIRE(rng.next_double_calls == 0);
    REQUIRE(log_has(sm, "sorteou jab"));
}

// ----- Redirecionamento produz efeito IDENTICO ao cast direto (dano + status) -----

TEST_CASE("urandom FSM: redirecionamento pra uma COMUM produz o MESMO dano + status que o "
         "cast DIRETO da mesma carta (mesmo resolvedor, mesmas draws downstream)",
         "[domain][combat][urandom][fsm][parity]") {
    // ---- Referencia: cast DIRETO da carta "sting" (mana 1, power 4, Stun 1t). ----
    Card sting = comum_card("sting", /*mana=*/1, /*power=*/4,
                            StatusEffect{StatusId::Stun, /*mag=*/0, /*dur=*/1,
                                        StackRule::Replace, CardFamily::Cinetico});
    int direct_target_hp = 0;
    {
        CombatActor caster = make_actor("h1", true, /*hp=*/100, /*atk=*/10, /*def=*/0);
        CombatActor target = make_actor("e1", false, /*hp=*/300, /*atk=*/0, /*def=*/0);
        auto reg = registry({sting});
        CombatAction cast = CombatAction::use_card(sting.id, target.id());
        FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);  // canal COMUM, sem crit/fumble.
        auto provider = play_sequence({cast});
        CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        direct_target_hp = target.hp();
        REQUIRE(find_status(target, StatusId::Stun) != nullptr);
        REQUIRE(direct_target_hp < 300);
    }

    // ---- Redirecionado: urandom sorteia "sting" e aplica no inimigo (lado=enemy). ----
    int redirect_target_hp = 0;
    {
        CombatActor caster = make_actor("h2", true, /*hp=*/100, /*atk=*/10, /*def=*/0);
        CombatActor target = make_actor("e2", false, /*hp=*/300, /*atk=*/0, /*def=*/0);
        Card urandom = urandom_card();
        auto reg = registry({urandom, sting});
        std::vector<CardCollectionEntry> snapshot = {
            CardCollectionEntry{0, urandom.id, 1, CardOrigin::OriginalRom},
            CardCollectionEntry{1, sting.id, 1, CardOrigin::OriginalRom}};
        CombatAction cast = CombatAction::use_card(urandom.id, caster.id());
        cast.card_instance_id = 0;
        // draw1=0 (Fraco), draw2=0 (unico candidato, "sting"), draw3=1 (lado=inimigo),
        // depois downstream do redirect: draw4=99 (canal COMUM sting), next_double=0.5.
        SequenceRandom rng({0, 0, 1, 99}, /*next_double_value=*/0.5);
        auto provider = play_sequence({cast});
        CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng, nullptr,
                              &snapshot);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        redirect_target_hp = target.hp();
        REQUIRE(find_status(target, StatusId::Stun) != nullptr);
        REQUIRE(log_has(sm, "sorteou sting"));
    }

    REQUIRE(redirect_target_hp == direct_target_hp);
}

// ----- URANDOM-AOE-REDIRECT-LOG (TODO.md): aviso diegetico do downgrade area->single ---

TEST_CASE("urandom FSM: carta sorteada com TargetShape de area (Grupo/Area3x3/Linha) loga "
         "o sufixo de downgrade (redirect e SEMPRE single-target, comportamento intacto)",
         "[domain][combat][urandom][fsm][log]") {
    // "blast": mesma receita de comum_card, so com TargetShape::Grupo no lugar de Single -
    // na resolucao NORMAL seria um efeito de area; via urandom vira single no alvo ja
    // sorteado (ver doc-comment de resolve_redirected_card_effect no header).
    Card blast = comum_card("blast", /*mana=*/1, /*power=*/4);
    blast.target_shape = TargetShape::Grupo;

    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/10, /*def=*/0);
    CombatActor target = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0);
    Card urandom = urandom_card();
    auto reg = registry({urandom, blast});
    std::vector<CardCollectionEntry> snapshot = {
        CardCollectionEntry{0, urandom.id, 1, CardOrigin::OriginalRom},
        CardCollectionEntry{1, blast.id, 1, CardOrigin::OriginalRom}};
    CombatAction cast = CombatAction::use_card(urandom.id, caster.id());
    cast.card_instance_id = 0;
    // draw1=0 (Fraco), draw2=0 (unico candidato, "blast"), draw3=1 (lado=inimigo), depois
    // downstream: draw4=99 (canal COMUM), next_double=0.5.
    SequenceRandom rng({0, 0, 1, 99}, /*next_double_value=*/0.5);
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng, nullptr,
                          &snapshot);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(log_has(sm, "sorteou blast"));
    REQUIRE(log_has(sm, "(efeito de area reduzido a alvo unico)"));
    // Comportamento intacto: continua single-target (so o "e" leva o dano, nao ha
    // segundo alvo no combate pra checar - a ausencia de crash/segundo hit ja prova).
    REQUIRE(target.hp() < 300);
}

TEST_CASE("urandom FSM: carta sorteada com TargetShape::Single NAO loga o sufixo de "
         "downgrade (nao houve reducao nenhuma pra avisar)",
         "[domain][combat][urandom][fsm][log]") {
    Card sting = comum_card("sting", /*mana=*/1, /*power=*/4);  // TargetShape::Single (default)

    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/10, /*def=*/0);
    CombatActor target = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0);
    Card urandom = urandom_card();
    auto reg = registry({urandom, sting});
    std::vector<CardCollectionEntry> snapshot = {
        CardCollectionEntry{0, urandom.id, 1, CardOrigin::OriginalRom},
        CardCollectionEntry{1, sting.id, 1, CardOrigin::OriginalRom}};
    CombatAction cast = CombatAction::use_card(urandom.id, caster.id());
    cast.card_instance_id = 0;
    SequenceRandom rng({0, 0, 1, 99}, /*next_double_value=*/0.5);
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng, nullptr,
                          &snapshot);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(log_has(sm, "sorteou sting"));
    REQUIRE_FALSE(log_has(sm, "(efeito de area reduzido a alvo unico)"));
}

TEST_CASE("urandom FSM: redirecionamento pra uma ESPECIAL (jackpot) executa via "
         "techMagic::execute e produz o MESMO status que o cast direto",
         "[domain][combat][urandom][fsm][parity][jackpot]") {
    Card special = especial_card("poison-touch", StatusId::Poison, /*magnitude=*/5,
                                 /*duration=*/2);

    // ---- Referencia: execucao DIRETA via techMagic::execute. ----
    {
        CombatActor caster = make_actor("h1", true);
        CombatActor target = make_actor("e1", false);
        techMagic::TechMagicContext ctx{&caster, &target, /*damage=*/0, nullptr};
        techMagic::execute(TriggerHook::OnCast, special, ctx);
        REQUIRE(find_status(target, StatusId::Poison) != nullptr);
    }

    // ---- Redirecionado: urandom sorteia a especial (jackpot) e aplica no inimigo. ----
    {
        CombatActor caster = make_actor("h2", true);
        CombatActor target = make_actor("e2", false);
        Card urandom = urandom_card();
        auto reg = registry({urandom, special});
        std::vector<CardCollectionEntry> snapshot = {
            CardCollectionEntry{0, urandom.id, 1, CardOrigin::OriginalRom},
            CardCollectionEntry{1, special.id, 1, CardOrigin::OriginalRom}};
        CombatAction cast = CombatAction::use_card(urandom.id, caster.id());
        cast.card_instance_id = 0;
        // draw1=76 (Jackpot), draw2=0 (unico candidato jackpot), draw3=1 (lado=inimigo).
        SequenceRandom rng({76, 0, 1});
        auto provider = play_sequence({cast});
        CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng, nullptr,
                              &snapshot);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        REQUIRE(find_status(target, StatusId::Poison) != nullptr);
        REQUIRE(log_has(sm, "sorteou poison-touch"));
    }
}

TEST_CASE("urandom FSM: lado sorteado SELF (caotico, AMB-10) redireciona pro proprio caster "
         "- fogo amigo desligado, dano 0, mas status/effects OnCast ainda podem se aplicar",
         "[domain][combat][urandom][fsm][chaos]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100);
    CombatActor target = make_actor("e", false, /*hp=*/300);

    Card urandom = urandom_card();
    Card jab = comum_card("jab", /*mana=*/1, /*power=*/9);
    auto reg = registry({urandom, jab});
    std::vector<CardCollectionEntry> snapshot = {
        CardCollectionEntry{0, urandom.id, 1, CardOrigin::OriginalRom},
        CardCollectionEntry{1, jab.id, 1, CardOrigin::OriginalRom}};

    CombatAction cast = CombatAction::use_card(urandom.id, caster.id());
    cast.card_instance_id = 0;
    SequenceRandom rng({0, 0, 0});  // draw3=0 -> lado=SELF.
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng, nullptr, &snapshot);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(caster.hp() == 100);  // fogo amigo desligado: dano 0 no proprio caster.
    REQUIRE(target.hp() == 300);  // inimigo nem foi tocado.
    REQUIRE(log_has(sm, "o proprio caster"));
}

// ----- Regressao: ledger de vírus/snapshot novos nao quebram calls sites pre-existentes ---

TEST_CASE("urandom: regressao - collection_snapshot ausente (default, ctor antigo) preserva "
         "TODO combate sem urandom identico ao motor pre-existente",
         "[domain][combat][urandom][regression]") {
    CombatActor caster = make_actor("h", true);
    CombatActor target = make_actor("e", false, /*hp=*/300);

    Card jab = comum_card("jab", /*mana=*/1, /*power=*/9);
    auto reg = registry({jab});
    CombatAction cast = CombatAction::use_card(jab.id, target.id());
    FixedRandom rng(0.5, 99);
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng);  // 6 args, como sempre.

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(target.hp() < 300);
    REQUIRE_FALSE(log_has(sm, "urandom"));
}
