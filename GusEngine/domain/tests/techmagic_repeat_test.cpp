// techmagic_repeat_test.cpp
//
// Spec executavel (Catch2 v3) do executor techMagic (ADR-016, MVP step 5):
// EffectKind::RepeatLastAction (Mandelbrot/Fractal-Echo, OnCast + Ada/Re-Run,
// OnAllyTurnEnd), decisoes Q1-Q4 do lider (2026-07-14):
//   Q1 - eco do RESULTADO: reaplica o DANO que a acao causou em cada alvo, na % da
//        carta, direto via take_damage PURO (sem novo sorteio/critico/mana/status).
//   Q2 - so ataque basico ou carta ofensiva que causou dano>0 grava last_action_.
//   Q3 - Ada ecoa a 100% (percent=100); o freio dela e a CHANCE (magnitude=34).
//   Q4 - janela = ultima acao de dano de QUALQUER aliado NESTA RODADA; zera no wrap.
//
// Cartas de teste montadas localmente (id "techmagic.repeat.*" / "*_test"), NUNCA do
// registry de producao (MasterCards). Ataques BASICOS pra dano deterministico SEM RNG
// onde possivel; FixedRandom crava a chance de Ada nos casos que dependem dela.
//
// Cross-ref: gus/domain/combat/techmagic.hpp (LastActionRecord/TechMagicContext);
//            combat_state_machine.cpp (gravacao em resolve_basic_attack/resolve_use_card,
//            despacho em process_ally_turn_end_hooks/process_round_end_hooks);
//            docs/design/mecanicas/cartas-technomagik.md; ADR-016.

#include <catch2/catch_test_macros.hpp>

#include <initializer_list>
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

CombatActor make_actor(const std::string& id, bool player_side, int hp, int atk, int def,
                       int spd) {
    return CombatActor(id, id, hp, atk, def, spd, CardFamily::Eletrico, player_side);
}

// Mandelbrot de teste: Ativa, OnCast -> RepeatLastAction magnitude 0 (sempre dispara, 0
// RNG), percent configuravel. power=0/atk=0 no caster garante que o PROPRIO cast nao
// cause dano (isola o eco).
Card mandelbrot_card(const std::string& id, int percent) {
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
                            .kind = EffectKind::RepeatLastAction,
                            .magnitude = 0,
                            .percent = percent}};
    return c;
}

// Ada de teste: Passiva mana 0, OnAllyTurnEnd -> RepeatLastAction percent 100 (Q3),
// magnitude = chance configuravel.
Card ada_card(const std::string& id, int chance) {
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
    c.effects = {EffectSpec{.trigger = TriggerHook::OnAllyTurnEnd,
                            .kind = EffectKind::RepeatLastAction,
                            .magnitude = chance,
                            .percent = 100}};
    return c;
}

// Carta OnDamageDealt->Leech, so pra provar que o eco NAO redispara hooks (o Leech do
// atacante original so pode disparar 1x, da propria acao, nunca do eco).
Card leech_card(const std::string& id, int percent) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Eletrico;
    c.base_type = CardBaseType::Glifo;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = 0;
    c.target_shape = TargetShape::Single;
    c.tier = CardTier::Especial;
    c.category = CardCategory::Ativa;
    c.effects = {EffectSpec{.trigger = TriggerHook::OnDamageDealt,
                            .kind = EffectKind::Leech,
                            .percent = percent}};
    return c;
}

std::unordered_map<std::string, Card> registry(std::initializer_list<Card> cards) {
    std::unordered_map<std::string, Card> d;
    for (const auto& c : cards) d.emplace(c.id, c);
    return d;
}

// Plano de acoes POR RODADA (mesmo padrao de techmagic_roundend_test.cpp).
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

// Conduz UM turno completo (begin_turn -> run_active_turn_to_end -> advance_to_next_actor).
void act(CombatStateMachine& sm) {
    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();
}

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

// Cenario compartilhado dos testes de Ada: h1 ataca 'e' (base echoable); h2 e o dono do
// Ada (nao ataca); 'e' revida em h1. `fixed_roll` crava o next(100) que decide a chance
// do Re-Run. Devolve os 3 HPs finais por saida.
void run_ada_scenario(int fixed_roll, int* out_e_hp, int* out_h1_hp, int* out_h2_hp) {
    CombatActor h1 = make_actor("h1", true, 100, 6, 0, 40);  // ataca e.
    CombatActor h2 = make_actor("h2", true, 100, 0, 0, 35);  // dono do Ada; nao ataca.
    CombatActor e = make_actor("e", false, 100, 5, 0, 10);   // ataca h1 de volta.
    h2.set_equipped_special_ids({"ada_test"});
    auto reg = registry({ada_card("ada_test", /*chance=*/34)});

    auto provider = provider_for({{"h1", {{0, {CombatAction::attack(e.id())}}}},
                                  {"e", {{0, {CombatAction::attack(h1.id())}}}}});
    FixedRandom rng(/*next_double=*/0.5, fixed_roll);
    CombatStateMachine sm({&h1, &h2, &e}, provider, &reg, nullptr, &rng);

    act(sm);  // h1 ataca e (6): last_action = h1. Turno de h1 fecha -> Ada (dono h2,
              // aliado de h1) reage: echoable (mesmo lado) -> 1 consumo de RNG.
    act(sm);  // h2 passa. Turno de h2 fecha -> exclui o PROPRIO h2: Ada nao roda sobre si.
    act(sm);  // e ataca h1 (5): last_action = e (lado inimigo). Turno de e fecha ->
              // dispatch filtrado por LADO (so outros inimigos); h2 nao e considerado.

    *out_e_hp = e.hp();
    *out_h1_hp = h1.hp();
    *out_h2_hp = h2.hp();
}

}  // namespace

// ===== 1. Mandelbrot: eco 50% com lround em dano IMPAR; nao entra no ledger; nao =====
// =====    redispara hooks (Leech do ataque original so dispara 1x) ==================

TEST_CASE("techmagic repeat: mandelbrot ecoa 50% (lround em dano impar), eco fora do "
         "ledger e sem redisparar hooks",
         "[domain][combat][techmagic][repeat]") {
    CombatActor h1 = make_actor("h1", true, 100, 7, 0, 40);   // ataca por 7 (impar).
    CombatActor h2 = make_actor("h2", true, 100, 0, 0, 35);   // conjura o Mandelbrot.
    CombatActor e = make_actor("e", false, 300, 0, 0, 10);
    h1.take_damage(50);  // hp=50: headroom pra observar o Leech sem clamp em max_hp.
    h1.set_equipped_special_ids({"leech_h1"});

    auto reg = registry({mandelbrot_card("mandelbrot_test", /*percent=*/50),
                         leech_card("leech_h1", /*percent=*/50)});

    auto provider = provider_for(
        {{"h1", {{0, {CombatAction::attack(e.id())}}}},
         {"h2", {{0, {CombatAction::use_card("mandelbrot_test", e.id())}}}}});
    CombatStateMachine sm({&h1, &h2, &e}, provider, &reg);

    act(sm);  // h1 ataca e por 7; Leech de h1 cura/restaura lround(7*50/100)=lround(3.5)=4.
    REQUIRE(e.hp() == 300 - 7);
    REQUIRE(h1.hp() == 54);  // 50 + 4 (Leech, 1x, da propria acao).
    REQUIRE(sm.round_hits().size() == 1);

    act(sm);  // h2 conjura Mandelbrot (power 0 + atk 0 = 0 dano proprio) -> OnCast ecoa
              // 50% do hit de h1 (7): lround(3.5) = 4, via take_damage PURO em 'e'.
    REQUIRE(e.hp() == 300 - 7 - 4);
    // O eco NAO passa por apply_damage_with_hooks: nao entra no ledger (so o hit de h1
    // continua la) e NAO redispara o Leech de h1 (hp continua 54, nao 58).
    REQUIRE(sm.round_hits().size() == 1);
    REQUIRE(h1.hp() == 54);
}

// ===== 2. Ada: dispara/nao dispara (chance cravada nos 2 ramos); so no turno de =====
// =====    ALIADO; exclusao do proprio dono; SEM custo de AP; determinismo ===========

TEST_CASE("techmagic repeat: ada - ramo DISPARA (roll < chance) ecoa 100% do hit de h1 "
         "em cima de 'e'; nao dispara sobre si mesma nem no turno inimigo",
         "[domain][combat][techmagic][repeat]") {
    int e_hp = 0, h1_hp = 0, h2_hp = 0;
    run_ada_scenario(/*fixed_roll=*/0, &e_hp, &h1_hp, &h2_hp);

    // e: 100 - 6 (h1 ataca) - 6 (eco de Ada, 100% de 6) = 88.
    REQUIRE(e_hp == 88);
    // h1: 100 - 5 (e ataca de volta) = 95. Sem eco extra (o hit de 'e' e do lado
    // inimigo; o turno-fim de 'e' nao alcanca o Ada de h2 - filtrado por lado no despacho).
    REQUIRE(h1_hp == 95);
    REQUIRE(h2_hp == 100);  // Ada nunca ecoa sobre si mesma nem sofre dano.
}

TEST_CASE("techmagic repeat: ada - ramo NAO DISPARA (roll >= chance) nao ecoa",
         "[domain][combat][techmagic][repeat]") {
    int e_hp = 0, h1_hp = 0, h2_hp = 0;
    run_ada_scenario(/*fixed_roll=*/99, &e_hp, &h1_hp, &h2_hp);

    REQUIRE(e_hp == 94);   // so o hit de h1 (6): 100-6.
    REQUIRE(h1_hp == 95);  // so o hit de 'e' (5): 100-5.
    REQUIRE(h2_hp == 100);
}

TEST_CASE("techmagic repeat: ada - determinismo (2 execucoes identicas produzem o "
         "mesmo HP; a chance e cravada nos DOIS ramos)",
         "[domain][combat][techmagic][repeat]") {
    int e_hp1 = 0, h1_hp1 = 0, h2_hp1 = 0;
    int e_hp2 = 0, h1_hp2 = 0, h2_hp2 = 0;
    run_ada_scenario(0, &e_hp1, &h1_hp1, &h2_hp1);
    run_ada_scenario(0, &e_hp2, &h1_hp2, &h2_hp2);

    REQUIRE(e_hp1 == e_hp2);
    REQUIRE(h1_hp1 == h1_hp2);
    REQUIRE(h2_hp1 == h2_hp2);
}

TEST_CASE("techmagic repeat: ada - ZERO consumo de RNG quando nao ha acao echoable "
         "ainda (dono termina turno primeiro, ninguem causou dano)",
         "[domain][combat][techmagic][repeat]") {
    CombatActor h_first = make_actor("h_first", true, 100, 0, 0, 40);  // termina turno 1o.
    CombatActor h_ada = make_actor("h_ada", true, 100, 0, 0, 35);      // dono do Ada.
    CombatActor e = make_actor("e", false, 100, 0, 0, 10);
    h_ada.set_equipped_special_ids({"ada_test"});
    auto reg = registry({ada_card("ada_test", /*chance=*/34)});

    CountingRandom counting;
    CombatActionProvider always_pass = [](CombatActor&, const CombatState&) {
        return CombatAction::pass();
    };
    CombatStateMachine sm({&h_first, &h_ada, &e}, always_pass, &reg, nullptr, &counting);

    act(sm);  // h_first passa (nenhum dano na batalha ainda): turno-fim -> Ada (h_ada)
              // roda, mas last_action_.actor==nullptr -> nao echoable, RETORNA ANTES de
              // tocar ctx.rng (0 consumo).
    REQUIRE(counting.next_calls == 0);
    REQUIRE(e.hp() == e.max_hp());
}

// ===== 3. Anti-loop: Ada nao repete o eco da Mandelbrot (repeticao-de-repeticao =====
// =====    impossivel, por construcao - o eco nunca atualiza last_action_) ===========

TEST_CASE("techmagic repeat: ada reagindo ao turno de quem conjurou Mandelbrot ecoa a "
         "acao ORIGINAL de dano (h1), nunca o eco da propria Mandelbrot",
         "[domain][combat][techmagic][repeat]") {
    CombatActor h1 = make_actor("h1", true, 100, 6, 0, 45);  // ataca e (6, base); dono do Ada.
    CombatActor h2 = make_actor("h2", true, 100, 0, 0, 40);  // conjura Mandelbrot.
    CombatActor e = make_actor("e", false, 300, 0, 0, 10);
    h1.set_equipped_special_ids({"ada_test"});

    auto reg = registry({mandelbrot_card("mandelbrot_test", /*percent=*/50),
                         ada_card("ada_test", /*chance=*/34)});

    auto provider = provider_for(
        {{"h1", {{0, {CombatAction::attack(e.id())}}}},
         {"h2", {{0, {CombatAction::use_card("mandelbrot_test", e.id())}}}}});
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/0);  // Ada sempre dispara (roll 0).
    CombatStateMachine sm({&h1, &h2, &e}, provider, &reg, nullptr, &rng);

    act(sm);  // h1 ataca e (6). Turno de h1 fecha: NINGUEM MAIS tem Ada equipada (h1 e
              // excluido de si mesmo) - no-op aqui.
    REQUIRE(e.hp() == 300 - 6);

    act(sm);  // h2 conjura Mandelbrot: eco 50% de 6 = 3 em 'e' (last_action_ NAO muda -
              // o cast do proprio h2 teve 0 hits proprios). Turno de h2 fecha -> Ada
              // (h1, aliado de h2) reage: ecoa last_action_, que ainda e {h1, hits
              // {e:6}} (a acao ORIGINAL), NAO o eco de 3 da Mandelbrot (que nunca virou
              // last_action_). Ada ecoa 100% de 6 = 6.
    // Se o anti-loop estivesse quebrado (Ada ecoando o eco de 3 em vez do hit original de
    // 6), o total seria 300-6-3-3=288, nao 285.
    REQUIRE(e.hp() == 300 - 6 - 3 - 6);
}

// ===== 4. Registro de last_action: so grava com dano>0; preserva o anterior numa =====
// =====    acao sem dano; zera na fronteira de rodada =================================

TEST_CASE("techmagic repeat: last_action grava apos dano>0, preserva o registro numa "
         "acao sem dano, e zera na fronteira de rodada",
         "[domain][combat][techmagic][repeat]") {
    CombatActor h1 = make_actor("h1", true, 100, 8, 0, 40);
    CombatActor h2 = make_actor("h2", true, 100, 0, 0, 35);
    CombatActor e = make_actor("e", false, 300, 0, 0, 10);

    // Antes de qualquer acao: registro vazio (estado normal, nao bug).
    auto provider = provider_for({{"h2", {{0, {CombatAction::defend()}}}},
                                  {"e", {{0, {CombatAction::defend()}}}}});
    CombatStateMachine sm({&h1, &h2, &e}, provider, nullptr);
    REQUIRE(sm.last_action().actor == nullptr);
    REQUIRE(sm.last_action().hits.empty());

    act(sm);  // h1 (sem plano -> pass do provider default? NAO: h1 nao tem entrada no
              // plano, provider_for devolve pass() -> 0 dano, registro CONTINUA vazio.
    REQUIRE(sm.last_action().actor == nullptr);

    // Substitui o plano: agora h1 ataca de fato. Reconstroi a FSM com h1 atacando.
    auto provider2 = provider_for(
        {{"h1", {{0, {CombatAction::attack(e.id())}}}},
         {"h2", {{0, {CombatAction::defend()}}}},
         {"e", {{0, {CombatAction::defend()}}}}});
    CombatStateMachine sm2({&h1, &h2, &e}, provider2, nullptr);

    act(sm2);  // h1 ataca e por 8.
    REQUIRE(sm2.last_action().actor == &h1);
    REQUIRE(sm2.last_action().type == CombatActionType::Attack);
    REQUIRE(sm2.last_action().card_id.empty());
    REQUIRE(sm2.last_action().hits.size() == 1);
    REQUIRE(sm2.last_action().hits.front().first == &e);
    REQUIRE(sm2.last_action().hits.front().second == 8);

    act(sm2);  // h2 defende (0 dano): o registro de h1 e PRESERVADO, nao apagado.
    REQUIRE(sm2.last_action().actor == &h1);
    REQUIRE(sm2.last_action().hits.size() == 1);

    act(sm2);  // e defende -> fecha a rodada (wrap): process_round_end_hooks zera
              // last_action_ junto do round_hits_ (Q4 - a janela nao atravessa rodada).
    REQUIRE(sm2.last_action().actor == nullptr);
    REQUIRE(sm2.last_action().hits.empty());
    REQUIRE(sm2.round_hits().empty());
}

// ===== 5. Fail-fast intacto: CloneAlly continua sem handler (RepeatLastAction nao =====
// =====    "engoliu" o default do switch) ============================================

TEST_CASE("techmagic repeat: RepeatLastAction ganhou handler, mas CloneAlly continua "
         "sem handler (lanca logic_error - fail-fast intacto)",
         "[domain][combat][techmagic][repeat]") {
    CombatActor h = make_actor("h", true, 100, 8, 0, 20);

    Card clone_card;
    clone_card.id = "techmagic.repeat.clone.failfast";
    clone_card.display_name = clone_card.id;
    clone_card.family = CardFamily::Universal;
    clone_card.tier = CardTier::Especial;
    clone_card.category = CardCategory::Ativa;
    clone_card.effects = {
        EffectSpec{.trigger = TriggerHook::OnCast, .kind = EffectKind::CloneAlly}};

    techMagic::TechMagicContext ctx;
    ctx.caster = &h;

    REQUIRE_THROWS_AS(techMagic::execute(TriggerHook::OnCast, clone_card, ctx),
                      std::logic_error);

    // RepeatLastAction, em contraste, roda ate o fim sem lancar (mesmo sem last_action
    // echoable - vira no-op+log, nao excecao).
    Card repeat_card = mandelbrot_card("techmagic.repeat.clone.failfast.mandelbrot",
                                       /*percent=*/50);
    techMagic::TechMagicContext repeat_ctx;
    repeat_ctx.caster = &h;
    const techMagic::LastActionRecord empty_record;
    repeat_ctx.last_action = &empty_record;
    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, repeat_card, repeat_ctx));
}
