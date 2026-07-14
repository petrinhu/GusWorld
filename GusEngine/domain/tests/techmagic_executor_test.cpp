// techmagic_executor_test.cpp
//
// Spec executavel (Catch2 v3) do executor techMagic (ADR-016, MVP step 2): dispatcher
// OnCast/OnDamageDealt/OnDamageReceived/Always sobre EffectSpec de Card (ApplyStatus/
// Leech/Reflect) via a CombatStateMachine ja existente. POCO puro, ZERO Qt, headless.
//
// Cartas usadas aqui sao DE TESTE (montadas localmente, id "techmagic.test.*"), NUNCA do
// registry de producao (PlaceholderCards). RNG cravado (FixedRandom) zera a variancia =>
// dano deterministico, mesma tecnica de combat_status_effects_test.cpp.
//
// Cross-ref: gus/domain/combat/techmagic.hpp; combat_state_machine.cpp (pontos de hook);
//            docs/design/mecanicas/cartas-technomagik.md; ADR-016.

#include <catch2/catch_test_macros.hpp>

#include <initializer_list>
#include <optional>
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
#include "gus/domain/combat/random_source.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::FixedRandom;

namespace {

CombatActor hero(const std::string& id = "gus", int hp = 100, int spd = 20, int atk = 8,
                 int def = 4, CardFamily family = CardFamily::Eletrico) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/true);
}

CombatActor foe(const std::string& id = "enemy", int hp = 300, int spd = 10, int atk = 6,
                int def = 0, CardFamily family = CardFamily::Eletrico) {
    return CombatActor(id, id, hp, atk, def, spd, family, /*player=*/false);
}

// Carta de teste generica. Defaults => Comum/Ativa/sem effects (comportamento pre-ADR-016
// intacto); testes de techMagic passam tier/category/effects explicitamente.
Card make_card(const std::string& id, CardFamily family, int power, int mana_cost = 1,
              CardTier tier = CardTier::Comum, CardCategory category = CardCategory::Ativa,
              std::vector<EffectSpec> effects = {}) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = family;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = mana_cost;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = TargetShape::Single;
    c.tier = tier;
    c.category = category;
    c.effects = std::move(effects);
    return c;
}

std::unordered_map<std::string, Card> registry(std::initializer_list<Card> cards) {
    std::unordered_map<std::string, Card> d;
    for (const auto& c : cards) d.emplace(c.id, c);
    return d;
}

CombatAction always_pass(CombatActor&, const CombatState&) { return CombatAction::pass(); }

// Executa `action` na 1a chamada do lado da party; passa (0 AP) dali em diante. Mesmo
// padrao de combat_status_effects_test.cpp.
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

// Duplo de IRandomSource que CONTA as chamadas (prova que techMagic nao consome RNG
// extra); os valores devolvidos espelham FixedRandom(0.5, 99): canal COMUM, variancia zero.
class CountingRandom final : public IRandomSource {
public:
    double next_double() override {
        ++next_double_calls;
        return 0.5;
    }
    int next(int max_value) override {
        ++next_calls;
        return max_value <= 0 ? 0 : std::min(99, max_value - 1);
    }
    int next_calls = 0;
    int next_double_calls = 0;
};

}  // namespace

// ===== 1. ATIVA OnCast =====

TEST_CASE("techmagic: especial ativa oncast aplica status apos a base; 2o uso na "
         "mesma batalha lanca",
         "[domain][combat][techmagic]") {
    CombatActor h = hero();
    CombatActor e = foe();

    Card special = make_card(
        "techmagic.test.ativa.oncast", CardFamily::Eletrico, /*power=*/5, /*mana_cost=*/1,
        CardTier::Especial, CardCategory::Ativa,
        {EffectSpec{.trigger = TriggerHook::OnCast,
                   .kind = EffectKind::ApplyStatus,
                   .magnitude = 7,
                   .percent = 0,
                   .duration = 3,
                   .status = StatusId::Poison,
                   .stack_rule = StackRule::Replace}});
    auto reg = registry({special});
    FixedRandom rng;

    int calls = 0;
    CombatActionProvider provider = [&](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side()) return CombatAction::pass();
        ++calls;
        if (calls <= 2) return CombatAction::use_card(special.id, e.id());
        return CombatAction::pass();
    };

    CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
    sm.begin_turn();
    REQUIRE_THROWS_AS(sm.run_active_turn_to_end(), std::logic_error);

    // O 1o cast resolveu antes do 2o lancar: o status aplicado pelo OnCast persiste.
    const StatusEffect* poison = find_status(e, StatusId::Poison);
    REQUIRE(poison != nullptr);
    REQUIRE(poison->magnitude == 7);
    REQUIRE(poison->duration == 3);
    REQUIRE(calls == 2);
}

TEST_CASE("techmagic: carta comum pode ser jogada mais de 1x na mesma batalha "
         "(isenta da regra 1x)",
         "[domain][combat][techmagic]") {
    CombatActor h = hero();
    CombatActor e = foe();

    // tier=Comum, category=Ativa (defaults) e effects vazio: carta comum de verdade.
    Card plain = make_card("techmagic.test.comum2x", CardFamily::Eletrico, /*power=*/3);
    auto reg = registry({plain});
    FixedRandom rng;

    int calls = 0;
    CombatActionProvider provider = [&](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side()) return CombatAction::pass();
        ++calls;
        if (calls <= 2) return CombatAction::use_card(plain.id, e.id());
        return CombatAction::pass();
    };

    CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
    sm.begin_turn();
    REQUIRE_NOTHROW(sm.run_active_turn_to_end());
    // 2 casts (1 AP cada) + 1 consulta final ao provider com 1 AP sobrando (devolve pass,
    // encerra o turno sem custo) = 3 chamadas.
    REQUIRE(calls == 3);
}

// ===== 2. PASSIVA Always =====

TEST_CASE("techmagic: passiva equipada aplica status no dono a cada TurnStart; "
         "re-aplicado apos remocao manual",
         "[domain][combat][techmagic]") {
    CombatActor h = hero();
    CombatActor e = foe();

    Card passive = make_card(
        "techmagic.test.passiva.always", CardFamily::Eletrico, /*power=*/0, /*mana_cost=*/0,
        CardTier::Especial, CardCategory::Passiva,
        {EffectSpec{.trigger = TriggerHook::Always,
                   .kind = EffectKind::ApplyStatus,
                   .magnitude = 3,
                   .percent = 0,
                   .duration = 2,
                   .status = StatusId::Haste,
                   .stack_rule = StackRule::Refresh}});
    auto reg = registry({passive});
    h.set_equipped_special_ids({passive.id});

    CombatStateMachine sm({&h, &e}, always_pass, &reg);

    sm.begin_turn();  // h TurnStart, rodada 0: Always injeta Haste.
    const StatusEffect* haste = find_status(h, StatusId::Haste);
    REQUIRE(haste != nullptr);
    REQUIRE(haste->magnitude == 3);
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();  // -> e

    sm.begin_turn();  // e TurnStart (sem equipadas: no-op).
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();  // wrap -> h, rodada 1

    h.remove_status(StatusId::Haste);
    REQUIRE(find_status(h, StatusId::Haste) == nullptr);

    sm.begin_turn();  // h TurnStart, rodada 1: Always REAPLICA Haste.
    REQUIRE(find_status(h, StatusId::Haste) != nullptr);
}

// ===== 3. HIBRIDA =====

TEST_CASE("techmagic: hibrida e passiva desde o setup e ativa no cast; 1x/batalha "
         "so na parte ativa",
         "[domain][combat][techmagic]") {
    CombatActor h = hero();
    CombatActor e = foe();

    Card hybrid = make_card(
        "techmagic.test.hibrida", CardFamily::Eletrico, /*power=*/5, /*mana_cost=*/1,
        CardTier::Especial, CardCategory::Hibrida,
        {EffectSpec{.trigger = TriggerHook::Always,
                   .kind = EffectKind::ApplyStatus,
                   .magnitude = 4,
                   .percent = 0,
                   .duration = 2,
                   .status = StatusId::Regen,
                   .stack_rule = StackRule::Refresh},
         EffectSpec{.trigger = TriggerHook::OnCast,
                   .kind = EffectKind::ApplyStatus,
                   .magnitude = 9,
                   .percent = 0,
                   .duration = 2,
                   .status = StatusId::Break,
                   .stack_rule = StackRule::Replace}});
    auto reg = registry({hybrid});
    h.set_equipped_special_ids({hybrid.id});
    FixedRandom rng;

    int calls = 0;
    CombatActionProvider provider = [&](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side()) return CombatAction::pass();
        ++calls;
        return CombatAction::use_card(hybrid.id, e.id());
    };

    CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    // Passivo desde o setup (TurnStart): Regen ja presente no dono ANTES de qualquer cast.
    REQUIRE(find_status(h, StatusId::Regen) != nullptr);

    // Ativa no cast: 1a jogada aplica Break no alvo; 2a jogada (mesma batalha) lanca -
    // a regra 1x/batalha vale so pra parte ATIVA (Hibrida conta como Ativa/Hibrida).
    REQUIRE_THROWS_AS(sm.run_active_turn_to_end(), std::logic_error);
    REQUIRE(find_status(e, StatusId::Break) != nullptr);
    REQUIRE(calls == 2);
}

// ===== 4. Leech =====

TEST_CASE("techmagic: leech (OnDamageDealt) cura e restaura mana do caster, "
         "clamp no teto",
         "[domain][combat][techmagic]") {
    CombatActor h = hero("gus", /*hp=*/100, /*spd=*/20, /*atk=*/0, /*def=*/4);
    CombatActor e = foe("enemy", /*hp=*/500, /*spd=*/10, /*atk=*/6, /*def=*/0);
    h.take_damage(30);  // hp=70: headroom pra observar a cura sem clamp em max_hp.

    Card leech_card = make_card(
        "techmagic.test.leech", CardFamily::Eletrico, /*power=*/10, /*mana_cost=*/1,
        CardTier::Especial, CardCategory::Ativa,
        {EffectSpec{.trigger = TriggerHook::OnDamageDealt,
                   .kind = EffectKind::Leech,
                   .magnitude = 0,
                   .percent = 50}});
    auto reg = registry({leech_card});
    FixedRandom rng;  // (0.5, 99): canal COMUM, variancia zero.

    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(leech_card.id, e.id())),
                          &reg, nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    // base = (power+atk) * (100/(100+def)) * mult_fraqueza(1, mesma familia=Neutro) = 10*1=10.
    // canal COMUM com r=0.5 (variancia 0) => damage == round(base) == 10.
    REQUIRE(e.max_hp() - e.hp() == 10);
    // Leech 50% de 10 = 5: HP 70+5=75; mana gasta 1 (custo da carta, max_mana round0=2) ->
    // mana=1, Leech +5 clampado no teto (2) -> mana=2.
    REQUIRE(h.hp() == 75);
    REQUIRE(h.mana() == 2);
}

TEST_CASE("techmagic: leech nao dispara quando o dano do evento e zero (canal FALHA)",
         "[domain][combat][techmagic]") {
    CombatActor h = hero("gus", /*hp=*/100, /*spd=*/20, /*atk=*/0, /*def=*/4);
    CombatActor e = foe("enemy", /*hp=*/500, /*spd=*/10, /*atk=*/6, /*def=*/0);
    h.take_damage(30);  // hp=70

    Card leech_card = make_card(
        "techmagic.test.leech.falha", CardFamily::Eletrico, /*power=*/10, /*mana_cost=*/1,
        CardTier::Especial, CardCategory::Ativa,
        {EffectSpec{.trigger = TriggerHook::OnDamageDealt,
                   .kind = EffectKind::Leech,
                   .magnitude = 0,
                   .percent = 50}});
    auto reg = registry({leech_card});
    // fumble_chance com kills=0 é 5%; roll=0 < 5 => canal FALHA (dano 0).
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/0);

    CombatStateMachine sm({&h, &e},
                          play_once(CombatAction::use_card(leech_card.id, e.id())), &reg,
                          nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(e.hp() == e.max_hp());  // FALHA: 0 de dano.
    REQUIRE(h.hp() == 70);          // Leech nao disparou (guard ctx.damage<=0).
}

// ===== 5. Reflect (dois sitios) + anti-recursao =====

TEST_CASE("techmagic: reflect (OnDamageReceived) devolve dano via ataque basico E "
         "via UseCard, sem re-refletir",
         "[domain][combat][techmagic]") {
    CombatActor h = hero("gus", /*hp=*/100, /*spd=*/20, /*atk=*/10, /*def=*/0);
    CombatActor e = foe("enemy", /*hp=*/300, /*spd=*/10, /*atk=*/0, /*def=*/0);

    Card reflect_h = make_card(
        "techmagic.test.reflect.h", CardFamily::Eletrico, /*power=*/0, /*mana_cost=*/0,
        CardTier::Especial, CardCategory::Passiva,
        {EffectSpec{.trigger = TriggerHook::OnDamageReceived,
                   .kind = EffectKind::Reflect,
                   .magnitude = 0,
                   .percent = 20}});
    Card reflect_e = make_card(
        "techmagic.test.reflect.e", CardFamily::Eletrico, /*power=*/0, /*mana_cost=*/0,
        CardTier::Especial, CardCategory::Passiva,
        {EffectSpec{.trigger = TriggerHook::OnDamageReceived,
                   .kind = EffectKind::Reflect,
                   .magnitude = 0,
                   .percent = 30}});
    Card plain_hit = make_card("techmagic.test.reflect.plain", CardFamily::Eletrico,
                               /*power=*/8, /*mana_cost=*/1);
    auto reg = registry({reflect_h, reflect_e, plain_hit});

    h.set_equipped_special_ids({reflect_h.id});
    e.set_equipped_special_ids({reflect_e.id});
    FixedRandom rng;  // canal COMUM, variancia zero.

    int calls = 0;
    CombatActionProvider provider = [&](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side()) return CombatAction::pass();
        ++calls;
        if (calls == 1) return CombatAction::attack(e.id());
        if (calls == 2) return CombatAction::use_card(plain_hit.id, e.id());
        return CombatAction::pass();
    };

    CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    // 1) Ataque basico: dano = max(kMinDamage, atk-def) = 10. e sofre 10 -> reflect_e
    //    devolve round(10*0.3)=3 pra h, via take_damage PURO (sem redisparar hooks).
    // 2) UseCard(plain_hit): base=(8+10)*(100/100)*1=18; canal COMUM r=0.5 => dano 18.
    //    e sofre +18 -> reflect_e devolve round(18*0.3)=5 pra h.
    // Total: e = 300-10-18 = 272; h = 100-3-5 = 92. Se o refletido tivesse re-disparado o
    // Reflect de h (anti-recursao quebrada), e sofreria dano extra de volta - nao sofre.
    REQUIRE(e.hp() == 272);
    REQUIRE(h.hp() == 92);
    // Attack(1 AP) + UseCard(1 AP) + 1 consulta final com 1 AP sobrando (devolve pass).
    REQUIRE(calls == 3);
}

// ===== 6. Regressao de determinismo =====

TEST_CASE("techmagic: carta sem effects nao muda dano nem consumo de rng "
         "(regressao)",
         "[domain][combat][techmagic]") {
    CombatActor h = hero("gus", /*hp=*/100, /*spd=*/20, /*atk=*/8, /*def=*/4);
    CombatActor e = foe("enemy", /*hp=*/300, /*spd=*/10, /*atk=*/6, /*def=*/0);

    // tier=Comum, category=Ativa (defaults), effects vazio: nenhum hook techMagic dispara.
    Card plain = make_card("techmagic.test.plain.regressao", CardFamily::Eletrico,
                           /*power=*/12);
    auto reg = registry({plain});
    CountingRandom counting;

    CombatStateMachine sm({&h, &e}, play_once(CombatAction::use_card(plain.id, e.id())), &reg,
                          nullptr, &counting);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    // base = (12+8) * (100/100) * 1 = 20; canal COMUM r=0.5 (variancia 0) => dano 20.
    REQUIRE(e.max_hp() - e.hp() == 20);
    // techMagic nao introduz NENHUM consumo extra de RNG quando a carta nao tem effects e
    // nenhum ator tem especiais equipadas: exatamente 1 sorteio de canal + 1 variancia
    // COMUM, identico ao motor pre-ADR-016.
    REQUIRE(counting.next_calls == 1);
    REQUIRE(counting.next_double_calls == 1);
}
