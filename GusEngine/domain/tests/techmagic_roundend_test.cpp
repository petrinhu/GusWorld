// techmagic_roundend_test.cpp
//
// Spec executavel (Catch2 v3) do ledger cross-ator + hook OnRoundEnd + handler
// HypotenuseCombo (ADR-016, MVP step 3; decisoes Q1-Q4 do lider, 2026-07-14):
//   Q1 - golpe-BONUS extra (soma-se aos hits normais, nao substitui).
//   Q2 - formula estende a N atacantes: round(sqrt(a^2+b^2+c^2+...)).
//   Q3 - mesmo aliado que bate 2x conta como 1 componente = soma dos danos dele.
//   Q4 - o DONO da passiva precisa ser um dos atacantes distintos do alvo.
//
// Cartas de teste montadas localmente (id "techmagic.roundend.*"), NUNCA do registry de
// producao. Cenarios usam ataque BASICO (CombatAction::attack) pra dano deterministico
// SEM RNG (dano = max(kMinDamage, atk-def), nenhum sorteio de canal) - isola o
// comportamento do ledger/OnRoundEnd da formula de UseCard (ja coberta em
// techmagic_executor_test.cpp / combat_formula_test.cpp).
//
// Cross-ref: gus/domain/combat/techmagic.hpp; combat_state_machine.cpp (registro do
//            ledger em apply_damage_with_hooks; despacho em process_round_end_hooks);
//            docs/design/mecanicas/cartas-technomagik.md; ADR-016.

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <map>
#include <memory>
#include <stdexcept>
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
#include "gus/domain/combat/random_source.hpp"
#include "gus/domain/combat/techmagic.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::FixedRandom;

namespace {

// ---- Factories ----

CombatActor make_actor(const std::string& id, bool player_side, int hp, int atk, int def,
                       int spd) {
    return CombatActor(id, id, hp, atk, def, spd, CardFamily::Eletrico, player_side);
}

// Carta-passiva de teste com o programa OnRoundEnd->HypotenuseCombo (magnitude/percent
// nao usados por este handler, ver techmagic.cpp).
Card hypotenuse_card(const std::string& id) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Eletrico;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = 0;
    c.target_shape = TargetShape::Single;
    c.tier = CardTier::Especial;
    c.category = CardCategory::Passiva;
    c.effects = {EffectSpec{.trigger = TriggerHook::OnRoundEnd,
                            .kind = EffectKind::HypotenuseCombo}};
    return c;
}

// Carta plana (sem techMagic effects), pro teste do guard de ledger (dano 0/FALHA).
Card plain_card(const std::string& id, int power, int mana_cost = 1) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Eletrico;
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

// Plano de acoes POR RODADA: plan[actor_id][round_index] = fila de acoes consumida (1
// item por CHAMADA do provider) SO durante aquela rodada especifica. Fora do round_index
// listado, ou apos a fila daquela rodada se esgotar, o ator sempre devolve pass(). Isola
// os 2 usos que os testes precisam:
//   - varias acoes na MESMA rodada (mesmo turno): ex. um ator ataca 2x antes de passar.
//   - a MESMA chave de ator roteirizada em rodadas DIFERENTES (ex. ataca so na rodada 1).
// state.round_index() (CombatState, secao 3) e a fonte de verdade do round corrente -
// resolve a ambiguidade que um cursor "flat" (consumido por CHAMADA, sem round) nao
// resolveria (2 acoes na fila de round0 seriam AMBAS consumidas no mesmo turno de round0
// se o ator tiver AP sobrando, mesmo que a intencao fosse 1 por rodada).
using ActionPlan = std::unordered_map<std::string, std::unordered_map<int, std::vector<CombatAction>>>;

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

// Conduz UM turno completo (begin_turn -> run_active_turn_to_end -> advance_to_next_actor)
// do ator CORRENTE. Quando o ultimo ator da fila termina, advance_to_next_actor detecta o
// wrap e despacha process_round_end_hooks internamente (ver combat_state_machine.cpp).
void act(CombatStateMachine& sm) {
    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();
}

std::vector<const CombatLogEntry*> combo_entries(const std::vector<CombatLogEntry>& log) {
    std::vector<const CombatLogEntry*> out;
    for (const auto& e : log)
        if (e.message.find("Hipotenuse combo") != std::string::npos) out.push_back(&e);
    return out;
}

// Duplo de IRandomSource que CONTA as chamadas (prova ausencia de consumo extra de RNG na
// fronteira de rodada). Mesmo padrao de techmagic_executor_test.cpp::CountingRandom.
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

// ===== 1. Bonus basico (N=2, dono e um dos atacantes) =====

TEST_CASE("techmagic roundend: 2 aliados distintos (dono incluso) no mesmo alvo -> "
         "golpe-bonus round(sqrt(a^2+b^2)) somado aos hits normais",
         "[domain][combat][techmagic][roundend]") {
    CombatActor h1 = make_actor("h1", true, 100, 8, 0, 30);   // dono, dmg 8
    CombatActor h2 = make_actor("h2", true, 100, 5, 0, 25);   // ally, dmg 5
    CombatActor e = make_actor("e", false, 300, 0, 0, 10);
    h1.set_equipped_special_ids({"hypo"});
    auto reg = registry({hypotenuse_card("hypo")});

    auto provider = provider_for({{"h1", {{0, {CombatAction::attack(e.id())}}}},
                                  {"h2", {{0, {CombatAction::attack(e.id())}}}}});
    CombatStateMachine sm({&h1, &h2, &e}, provider, &reg);

    act(sm);  // h1
    act(sm);  // h2
    act(sm);  // e -> wrap, OnRoundEnd

    // dmg h1=8, dmg h2=5; bonus = round(sqrt(64+25)) = round(sqrt(89)) = 9.
    REQUIRE(e.hp() == 300 - 8 - 5 - 9);
    const auto entries = combo_entries(sm.log());
    REQUIRE(entries.size() == 1);
    REQUIRE(entries.front()->value == 9);
    REQUIRE(entries.front()->target_id == e.id());
    REQUIRE(sm.round_hits().empty());
}

// ===== 2. N=3 =====

TEST_CASE("techmagic roundend: formula estende a N=3 atacantes distintos",
         "[domain][combat][techmagic][roundend]") {
    CombatActor h1 = make_actor("h1", true, 100, 6, 0, 40);  // dono, dmg 6
    CombatActor h2 = make_actor("h2", true, 100, 9, 0, 35);  // dmg 9
    CombatActor h3 = make_actor("h3", true, 100, 2, 0, 30);  // dmg 2
    CombatActor e = make_actor("e", false, 1000, 0, 0, 10);
    h1.set_equipped_special_ids({"hypo"});
    auto reg = registry({hypotenuse_card("hypo")});

    auto provider = provider_for({{"h1", {{0, {CombatAction::attack(e.id())}}}},
                                  {"h2", {{0, {CombatAction::attack(e.id())}}}},
                                  {"h3", {{0, {CombatAction::attack(e.id())}}}}});
    CombatStateMachine sm({&h1, &h2, &h3, &e}, provider, &reg);

    act(sm);  // h1
    act(sm);  // h2
    act(sm);  // h3
    act(sm);  // e -> wrap

    // 6^2+9^2+2^2 = 36+81+4 = 121 = 11^2 (exato, sem ambiguidade de arredondamento).
    REQUIRE(e.hp() == 1000 - 6 - 9 - 2 - 11);
    const auto entries = combo_entries(sm.log());
    REQUIRE(entries.size() == 1);
    REQUIRE(entries.front()->value == 11);
}

// ===== 3. Sozinho (mesmo 2 hits) = sem bonus =====

TEST_CASE("techmagic roundend: 1 unico atacante distinto (mesmo com 2 hits no turno) "
         "nao ativa o combo",
         "[domain][combat][techmagic][roundend]") {
    CombatActor h1 = make_actor("h1", true, 100, 8, 0, 30);
    CombatActor e = make_actor("e", false, 300, 0, 0, 10);
    h1.set_equipped_special_ids({"hypo"});
    auto reg = registry({hypotenuse_card("hypo")});

    auto provider = provider_for(
        {{"h1", {{0, {CombatAction::attack(e.id()), CombatAction::attack(e.id())}}}}});
    CombatStateMachine sm({&h1, &e}, provider, &reg);

    act(sm);  // h1 ataca 2x no proprio turno
    act(sm);  // e -> wrap

    REQUIRE(e.hp() == 300 - 8 - 8);  // so os 2 hits base, zero bonus.
    REQUIRE(combo_entries(sm.log()).empty());
    REQUIRE(sm.round_hits().empty());
}

// ===== 4. Alvos diferentes nao combinam =====

TEST_CASE("techmagic roundend: 2 aliados distintos batendo em ALVOS DIFERENTES nao "
         "combinam",
         "[domain][combat][techmagic][roundend]") {
    CombatActor h1 = make_actor("h1", true, 100, 8, 0, 30);
    CombatActor h2 = make_actor("h2", true, 100, 5, 0, 25);
    CombatActor e1 = make_actor("e1", false, 300, 0, 0, 15);
    CombatActor e2 = make_actor("e2", false, 300, 0, 0, 10);
    h1.set_equipped_special_ids({"hypo"});
    auto reg = registry({hypotenuse_card("hypo")});

    auto provider = provider_for({{"h1", {{0, {CombatAction::attack(e1.id())}}}},
                                  {"h2", {{0, {CombatAction::attack(e2.id())}}}}});
    CombatStateMachine sm({&h1, &h2, &e1, &e2}, provider, &reg);

    act(sm);  // h1 -> e1
    act(sm);  // h2 -> e2
    act(sm);  // e1
    act(sm);  // e2 -> wrap

    REQUIRE(e1.hp() == 300 - 8);
    REQUIRE(e2.hp() == 300 - 5);
    REQUIRE(combo_entries(sm.log()).empty());
}

// ===== 5. Q4: dono nao bateu no alvo =====

TEST_CASE("techmagic roundend: Q4 - dono NAO esta entre os atacantes do alvo -> sem "
         "bonus mesmo com 2 aliados distintos batendo",
         "[domain][combat][techmagic][roundend]") {
    CombatActor h1 = make_actor("h1", true, 100, 0, 0, 30);  // dono, NAO ataca.
    CombatActor h2 = make_actor("h2", true, 100, 8, 0, 25);
    CombatActor h3 = make_actor("h3", true, 100, 5, 0, 20);
    CombatActor e = make_actor("e", false, 300, 0, 0, 10);
    h1.set_equipped_special_ids({"hypo"});
    auto reg = registry({hypotenuse_card("hypo")});

    auto provider = provider_for({{"h2", {{0, {CombatAction::attack(e.id())}}}},
                                  {"h3", {{0, {CombatAction::attack(e.id())}}}}});
    CombatStateMachine sm({&h1, &h2, &h3, &e}, provider, &reg);

    act(sm);  // h1 passa
    act(sm);  // h2
    act(sm);  // h3
    act(sm);  // e -> wrap

    REQUIRE(e.hp() == 300 - 8 - 5);  // sem bonus: dono fora do combo.
    REQUIRE(combo_entries(sm.log()).empty());
}

// ===== 6. Q3: mesmo aliado 2x = 1 componente (soma) =====

TEST_CASE("techmagic roundend: Q3 - mesmo aliado bate 2x conta como 1 componente = "
         "soma dos danos dele",
         "[domain][combat][techmagic][roundend]") {
    CombatActor h1 = make_actor("h1", true, 100, 3, 0, 30);  // dono, 2 hits de 3 = 6.
    CombatActor h2 = make_actor("h2", true, 100, 2, 0, 25);  // 1 hit de 2.
    CombatActor e = make_actor("e", false, 500, 0, 0, 10);
    h1.set_equipped_special_ids({"hypo"});
    auto reg = registry({hypotenuse_card("hypo")});

    auto provider = provider_for(
        {{"h1", {{0, {CombatAction::attack(e.id()), CombatAction::attack(e.id())}}}},
         {"h2", {{0, {CombatAction::attack(e.id())}}}}});
    CombatStateMachine sm({&h1, &h2, &e}, provider, &reg);

    act(sm);  // h1 ataca 2x
    act(sm);  // h2
    act(sm);  // e -> wrap

    // componentes: h1=3+3=6, h2=2; bonus = round(sqrt(36+4)) = round(sqrt(40)) = 6.
    REQUIRE(e.hp() == 500 - 3 - 3 - 2 - 6);
    const auto entries = combo_entries(sm.log());
    REQUIRE(entries.size() == 1);
    REQUIRE(entries.front()->value == 6);
}

// ===== 7. Sem carta equipada: ledger registra, bonus zero =====

TEST_CASE("techmagic roundend: ledger registra hits mesmo sem NINGUEM equipado; sem "
         "especial equipada, OnRoundEnd nao aplica bonus",
         "[domain][combat][techmagic][roundend]") {
    CombatActor h1 = make_actor("h1", true, 100, 8, 0, 30);
    CombatActor h2 = make_actor("h2", true, 100, 5, 0, 25);
    CombatActor e = make_actor("e", false, 300, 0, 0, 10);
    // Nenhum equipped_special_ids setado; card_registry nao e nem necessario.

    auto provider = provider_for({{"h1", {{0, {CombatAction::attack(e.id())}}}},
                                  {"h2", {{0, {CombatAction::attack(e.id())}}}}});
    CombatStateMachine sm({&h1, &h2, &e}, provider, nullptr);

    act(sm);  // h1: ledger ja tem 1 entrada aqui (getter observavel mid-round).
    REQUIRE(sm.round_hits().size() == 1);
    REQUIRE(sm.round_hits().front().damage == 8);

    act(sm);  // h2
    act(sm);  // e -> wrap (sem ninguem equipado, no-op de hooks; ledger limpo no fim)

    REQUIRE(e.hp() == 300 - 8 - 5);  // so os hits base, zero bonus.
    REQUIRE(combo_entries(sm.log()).empty());
    REQUIRE(sm.round_hits().empty());
}

// ===== 8. Dano 0 nao entra no ledger =====

TEST_CASE("techmagic roundend: dano 0 (canal FALHA/imune) nunca entra no ledger",
         "[domain][combat][techmagic][roundend]") {
    CombatActor h = make_actor("h", true, 100, 0, 4, 20);
    CombatActor e = make_actor("e", false, 500, 6, 0, 10);
    Card p = plain_card("techmagic.roundend.falha", /*power=*/10);
    auto reg = registry({p});
    // fumble_chance com kills=0 e 5%; roll=0 < 5 => canal FALHA (dano 0), mesmo padrao de
    // techmagic_executor_test.cpp "leech nao dispara quando o dano do evento e zero".
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/0);

    auto provider = provider_for({{"h", {{0, {CombatAction::use_card(p.id, e.id())}}}}});
    CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);

    act(sm);  // h joga a carta: canal FALHA, dano 0.
    REQUIRE(sm.round_hits().empty());  // guard: dano<=0 nunca chega no ledger.

    act(sm);  // e -> wrap

    REQUIRE(e.hp() == e.max_hp());
    REQUIRE(sm.round_hits().empty());
}

// ===== 9. Fronteira de rodada: hits de rodadas diferentes NAO combinam (clear) =====

TEST_CASE("techmagic roundend: ledger limpa na fronteira de rodada - hit da rodada "
         "anterior nao vaza pro combo da rodada seguinte",
         "[domain][combat][techmagic][roundend]") {
    CombatActor h1 = make_actor("h1", true, 100, 8, 0, 30);  // dono
    CombatActor h2 = make_actor("h2", true, 100, 5, 0, 25);
    CombatActor e = make_actor("e", false, 2000, 0, 0, 10);
    h1.set_equipped_special_ids({"hypo"});
    auto reg = registry({hypotenuse_card("hypo")});

    // h1 ataca nas 2 rodadas (0 e 1); h2 so ataca na rodada 1 (rodada 0: default pass).
    auto provider =
        provider_for({{"h1",
                       {{0, {CombatAction::attack(e.id())}},
                        {1, {CombatAction::attack(e.id())}}}},
                      {"h2", {{1, {CombatAction::attack(e.id())}}}}});
    CombatStateMachine sm({&h1, &h2, &e}, provider, &reg);

    // Rodada 0: so h1 ataca (1 distinto) -> sem bonus, ledger limpo no wrap.
    act(sm);  // h1
    act(sm);  // h2 (sem acao pra rodada 0 -> pass)
    act(sm);  // e -> wrap rodada 0
    REQUIRE(combo_entries(sm.log()).empty());
    REQUIRE(e.hp() == 2000 - 8);

    // Rodada 1: h1 ataca de novo + h2 ataca -> 2 distintos, dono presente -> bonus.
    // SE o ledger da rodada 0 tivesse vazado, o componente de h1 seria 8+8=16 (bug); com
    // o clear correto, o componente de h1 e SO o hit desta rodada (8).
    act(sm);  // h1
    act(sm);  // h2
    act(sm);  // e -> wrap rodada 1

    // componentes rodada 1: h1=8, h2=5; bonus = round(sqrt(64+25)) = 9 (NAO 17).
    REQUIRE(e.hp() == 2000 - 8 - 8 - 5 - 9);
    const auto entries = combo_entries(sm.log());
    REQUIRE(entries.size() == 1);  // so 1 combo em todo o teste (rodada 1).
    REQUIRE(entries.front()->value == 9);
}

// ===== 10. Lados: hit de lado oposto nao combina; espelho do lado inimigo funciona =====

TEST_CASE("techmagic roundend: hits de lados diferentes no mesmo alvo nao combinam; "
         "espelhado no lado inimigo (2 inimigos + dono inimigo) o combo funciona",
         "[domain][combat][techmagic][roundend]") {
    auto reg = registry({hypotenuse_card("hypo")});

    // --- Parte A: dono ALIADO + "fogo amigo" inimigo no MESMO alvo inimigo. So o hit do
    // mesmo lado do dono conta -> 1 distinto -> sem bonus.
    {
        CombatActor h1 = make_actor("h1", true, 100, 8, 0, 40);   // dono (player).
        CombatActor e2 = make_actor("e2", false, 100, 5, 0, 30);  // inimigo, ataca o proprio lado.
        CombatActor e = make_actor("e", false, 300, 0, 0, 10);    // alvo comum.
        h1.set_equipped_special_ids({"hypo"});

        auto provider = provider_for({{"h1", {{0, {CombatAction::attack(e.id())}}}},
                                      {"e2", {{0, {CombatAction::attack(e.id())}}}}});
        CombatStateMachine sm({&h1, &e2, &e}, provider, &reg);

        act(sm);  // h1 -> e
        act(sm);  // e2 -> e (lado oposto ao dono)
        act(sm);  // e -> wrap

        REQUIRE(e.hp() == 300 - 8 - 5);  // sem bonus (so h1 conta pro combo do dono).
        REQUIRE(combo_entries(sm.log()).empty());
    }

    // --- Parte B: espelho - dono INIMIGO, 2 inimigos batem no MESMO alvo (party). Ambos
    // do mesmo lado do dono -> combo ativa normalmente (simetria de is_player_side).
    {
        CombatActor e1 = make_actor("e1", false, 100, 8, 0, 40);  // dono (enemy).
        CombatActor e3 = make_actor("e3", false, 100, 5, 0, 30);
        CombatActor h = make_actor("h", true, 300, 0, 0, 10);
        e1.set_equipped_special_ids({"hypo"});

        auto provider = provider_for({{"e1", {{0, {CombatAction::attack(h.id())}}}},
                                      {"e3", {{0, {CombatAction::attack(h.id())}}}}});
        CombatStateMachine sm({&e1, &e3, &h}, provider, &reg);

        act(sm);  // e1 -> h
        act(sm);  // e3 -> h
        act(sm);  // h -> wrap

        REQUIRE(h.hp() == 300 - 8 - 5 - 9);  // round(sqrt(64+25)) = 9, igual ao teste 1.
        const auto entries = combo_entries(sm.log());
        REQUIRE(entries.size() == 1);
        REQUIRE(entries.front()->value == 9);
        REQUIRE(entries.front()->target_id == h.id());
    }
}

// ===== 11. Cadaver: alvo morto ANTES do fecho -> sem bonus postumo =====

TEST_CASE("techmagic roundend: alvo morto ANTES do fecho da rodada nao recebe bonus "
         "postumo (sem crash)",
         "[domain][combat][techmagic][roundend]") {
    CombatActor h1 = make_actor("h1", true, 100, 8, 0, 30);  // dono, dmg 8 (nao letal).
    CombatActor h2 = make_actor("h2", true, 100, 5, 0, 25);  // dmg 5 (letal: remata).
    CombatActor e = make_actor("e", false, 10, 0, 0, 10);    // 10 hp: sobrevive a 8, morre a 5.
    h1.set_equipped_special_ids({"hypo"});
    auto reg = registry({hypotenuse_card("hypo")});

    auto provider = provider_for({{"h1", {{0, {CombatAction::attack(e.id())}}}},
                                  {"h2", {{0, {CombatAction::attack(e.id())}}}}});
    CombatStateMachine sm({&h1, &h2, &e}, provider, &reg);

    act(sm);  // h1: e.hp 10 -> 2.
    REQUIRE(e.hp() == 2);
    REQUIRE(e.is_alive());

    act(sm);  // h2: e.hp 2 -> 0 (morre); prune remove 'e' da fila -> fila vira [h1,h2] e
              // o proprio advance_to_next_actor deste turno ja fecha a rodada (wrap).

    REQUIRE_FALSE(e.is_alive());
    REQUIRE(e.hp() == 0);           // clamp em 0, nao vai negativo.
    REQUIRE(combo_entries(sm.log()).empty());  // sem bonus postumo.
    REQUIRE(sm.round_hits().empty());
}

// ===== 12. Letal: bonus mata o alvo -> sem crash, vitoria no proximo check_end =====

TEST_CASE("techmagic roundend: bonus que mata o alvo nao gera crash; check_end "
         "detecta vitoria em seguida",
         "[domain][combat][techmagic][roundend]") {
    CombatActor h1 = make_actor("h1", true, 100, 3, 0, 30);  // dono, dmg 3.
    CombatActor h2 = make_actor("h2", true, 100, 2, 0, 25);  // dmg 2.
    CombatActor e = make_actor("e", false, 6, 0, 0, 10);     // 6 hp: 3+2=5 sobra 1; bonus mata.
    // DUPLA FUNCAO deste caso (pergunta do lider sobre floats/arredondamento): dmg 3 e 2 dao
    // sum_of_squares = 9+4 = 13; sqrt(13) = 3.6055... Como a fracao e > 0.5, lround = 4 mas
    // static_cast<int> (truncar) daria 3. Logo este teste TAMBEM e o regression de arredondamento:
    // uma mutacao lround->truncar no handler quebra aqui (bonus 4 -> 3). Empate exato em x.5 e
    // impossivel (sum_of_squares e sempre inteiro), entao fracao >0.5 vs <0.5 e a unica distincao.
    h1.set_equipped_special_ids({"hypo"});
    auto reg = registry({hypotenuse_card("hypo")});

    auto provider = provider_for({{"h1", {{0, {CombatAction::attack(e.id())}}}},
                                  {"h2", {{0, {CombatAction::attack(e.id())}}}}});
    CombatStateMachine sm({&h1, &h2, &e}, provider, &reg);

    act(sm);  // h1
    act(sm);  // h2: e.hp = 6-3-2 = 1, ainda vivo -> age (passa) na sequencia abaixo.
    REQUIRE(e.hp() == 1);
    REQUIRE(e.is_alive());

    act(sm);  // e passa -> wrap: bonus = round(sqrt(9+4)) = round(sqrt(13)) = 4; mata 'e'.

    REQUIRE_FALSE(e.is_alive());
    REQUIRE(e.hp() == 0);  // clamp em 0 (take_damage puro), sem crash.
    const auto entries = combo_entries(sm.log());
    REQUIRE(entries.size() == 1);
    REQUIRE(entries.front()->value == 4);

    REQUIRE_NOTHROW(sm.check_end());
    REQUIRE(sm.outcome() == CombatOutcome::Victory);
}

// ===== 13. Fail-fast: OnRoundEnd sem ledger lanca; CloneAlly (que GANHOU handler no =====
// =====     CARD-ENGINE-MANIFESTO item 8) tambem lanca aqui, mas pela validacao de =====
// =====     ARGUMENTO do PROPRIO handle_clone_ally (ctx.counterpart nulo), NAO mais pelo =====
// =====     default fail-fast do switch de execute() (mesma correcao ja feita no arquivo =====
// =====     irmao techmagic_repeat_test.cpp) ===============================================

TEST_CASE("techmagic roundend: execute(OnRoundEnd, HypotenuseCombo) sem ctx.round_hits "
         "lanca logic_error; CloneAlly TEM handler mas tambem lanca aqui (ctx.counterpart "
         "nulo, validacao interna do handle_clone_ally - nao mais o default do switch)",
         "[domain][combat][techmagic][roundend]") {
    CombatActor h = make_actor("h", true, 100, 8, 0, 20);

    Card hypo = hypotenuse_card("techmagic.roundend.failfast.hypo");
    Card clone = plain_card("techmagic.roundend.failfast.clone", /*power=*/0, /*mana_cost=*/0);
    clone.tier = CardTier::Especial;
    clone.category = CardCategory::Passiva;
    clone.effects = {
        EffectSpec{.trigger = TriggerHook::OnRoundEnd, .kind = EffectKind::CloneAlly}};

    techMagic::TechMagicContext ctx;  // round_hits == nullptr, counterpart == nullptr (default).
    ctx.caster = &h;

    REQUIRE_THROWS_AS(techMagic::execute(TriggerHook::OnRoundEnd, hypo, ctx),
                      std::logic_error);
    // CloneAlly ganhou handler (handle_clone_ally) no manifesto item 8 - o switch de
    // execute() NAO cai mais no default fail-fast pra ele. O logic_error aqui vem da
    // validacao de argumento DENTRO do handler (OnCast/OnRoundEnd sempre precisa de alvo,
    // ctx.counterpart nulo). Prova que o dispatch acha o case certo E que o handler mantem
    // fail-fast em argumento invalido.
    REQUIRE_THROWS_AS(techMagic::execute(TriggerHook::OnRoundEnd, clone, ctx),
                      std::logic_error);
}

// ===== 14. Determinismo: 2 execucoes identicas -> mesmo log/HP; RNG sem consumo extra =====

TEST_CASE("techmagic roundend: 2 execucoes identicas produzem o mesmo log/HP; a "
         "fronteira de rodada nao consome RNG novo",
         "[domain][combat][techmagic][roundend]") {
    auto run = [](std::vector<CombatLogEntry>* out_log, int* out_hp, int* out_next_calls,
                 int* out_next_double_calls) {
        CombatActor h1 = make_actor("h1", true, 100, 8, 0, 30);
        CombatActor h2 = make_actor("h2", true, 100, 5, 0, 25);
        CombatActor e = make_actor("e", false, 300, 0, 0, 10);
        h1.set_equipped_special_ids({"hypo"});
        auto reg = registry({hypotenuse_card("hypo")});
        CountingRandom counting;

        auto provider = provider_for({{"h1", {{0, {CombatAction::attack(e.id())}}}},
                                      {"h2", {{0, {CombatAction::attack(e.id())}}}}});
        CombatStateMachine sm({&h1, &h2, &e}, provider, &reg, nullptr, &counting);

        act(sm);
        act(sm);
        act(sm);  // wrap -> OnRoundEnd

        *out_log = sm.log();
        *out_hp = e.hp();
        *out_next_calls = counting.next_calls;
        *out_next_double_calls = counting.next_double_calls;
    };

    std::vector<CombatLogEntry> log1;
    std::vector<CombatLogEntry> log2;
    int hp1 = 0;
    int hp2 = 0;
    int next1 = -1;
    int next2 = -1;
    int nextd1 = -1;
    int nextd2 = -1;

    run(&log1, &hp1, &next1, &nextd1);
    run(&log2, &hp2, &next2, &nextd2);

    REQUIRE(hp1 == hp2);
    REQUIRE(log1 == log2);
    // Ataque basico nao consome RNG (nenhum sorteio de canal); a fronteira de rodada
    // (OnRoundEnd/HypotenuseCombo) tambem nao consome - contador fica em zero nas 2 runs.
    REQUIRE(next1 == 0);
    REQUIRE(next2 == 0);
    REQUIRE(nextd1 == 0);
    REQUIRE(nextd2 == 0);
}
