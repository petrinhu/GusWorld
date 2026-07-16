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

// ===== 6. Ada: NAO ecoa quando quem FECHA o turno e INIMIGO, mesmo com last_action_ =====
// =====    ainda do lado do JOGADOR (filtro de LADO no despacho de
// =====    process_ally_turn_end_hooks, combat_state_machine.cpp:504 - guard DISTINTO do
// =====    check "echoable" interno de handle_repeat_last_action, que so compara o lado
// =====    de last_action_.actor contra ctx.caster, NAO quem fechou o turno) =============

TEST_CASE("techmagic repeat: ada nao ecoa quando quem fecha o turno e INIMIGO, mesmo com "
         "last_action_ ainda do lado do jogador (filtro de lado no DESPACHO, nao no "
         "echoable interno do handler)",
         "[domain][combat][techmagic][repeat]") {
    CombatActor h1 = make_actor("h1", true, 100, 6, 0, 50);  // ataca e; grava last_action_.
    CombatActor h2 = make_actor("h2", true, 100, 0, 0, 45);  // dono do Ada; so passa.
    CombatActor e = make_actor("e", false, 100, 0, 0, 10);   // NUNCA ataca; so defende.
    h2.set_equipped_special_ids({"ada_test"});
    auto reg = registry({ada_card("ada_test", /*chance=*/34)});

    auto provider = provider_for({{"h1", {{0, {CombatAction::attack(e.id())}}}},
                                  {"e", {{0, {CombatAction::defend()}}}}});
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/0);  // Ada sempre dispara quando roda.
    CombatStateMachine sm({&h1, &h2, &e}, provider, &reg, nullptr, &rng);

    act(sm);  // h1 ataca e (6): last_action_={h1,{e:6}}. Turno de h1 fecha (caminho NORMAL,
              // MESMO lado do dono do Ada) -> Ada (h2) ecoa 100% de 6 -> e leva mais 6.
    REQUIRE(e.hp() == 100 - 6 - 6);

    act(sm);  // h2 passa (0 dano; last_action_ intacto). Turno de h2 fecha -> despacha pros
              // OUTROS aliados do MESMO lado (so h1, sem Ada equipada) -> no-op.
    REQUIRE(e.hp() == 100 - 6 - 6);

    act(sm);  // e DEFENDE (0 dano; last_action_ CONTINUA {h1,{e:6}}, tecnicamente
              // "echoable" pelo check interno do Ada - mesmo lado de h1). Turno de 'e'
              // fecha -> process_ally_turn_end_hooks(e) filtra por LADO: so considera
              // OUTROS aliados do MESMO lado de 'e' (inimigo) - h2 (jogador) e EXCLUIDO no
              // DESPACHO, mesmo que o echoable interno do Ada aprovasse se rodasse.
    REQUIRE(e.hp() == 100 - 6 - 6);  // SEM 3o eco: a Ada de h2 nao rodou nesta chamada.
}

// ===== 7. Mandelbrot (magnitude==0): ZERO consumo de RNG por construcao, mesmo com uma =====
// =====    acao echoable ja gravada (chamada isolada do executor, tecmagic.cpp:210 - o
// =====    branch `if (spec.magnitude > 0)` nunca roda pra 0) ===========================

TEST_CASE("techmagic repeat: mandelbrot (magnitude==0) nunca consome RNG, mesmo com uma "
         "acao echoable ja gravada",
         "[domain][combat][techmagic][repeat]") {
    CombatActor h1 = make_actor("h1", true, 100, 6, 0, 40);  // "autor" da acao echoable.
    CombatActor h2 = make_actor("h2", true, 100, 0, 0, 35);  // conjura o Mandelbrot.
    CombatActor e = make_actor("e", false, 300, 0, 0, 10);   // alvo do eco.

    const techMagic::LastActionRecord recorded{&h1, CombatActionType::Attack,
                                                /*card_id=*/std::string{}, {{&e, 6}}};

    Card mandelbrot = mandelbrot_card("mandelbrot_zero_rng_test", /*percent=*/50);

    CountingRandom counting;
    techMagic::TechMagicContext ctx;
    ctx.caster = &h2;
    ctx.last_action = &recorded;
    ctx.rng = &counting;

    techMagic::execute(TriggerHook::OnCast, mandelbrot, ctx);

    // O eco de fato rodou (prova que o 0-consumo NAO e um no-op mascarado):
    // lround(6*50/100) = 3.
    REQUIRE(e.hp() == 300 - 3);
    REQUIRE(counting.next_calls == 0);
    REQUIRE(counting.next_double_calls == 0);
}

// ===== 8. Ada: TAMBEM ecoa quando quem fecha o turno e um aliado STUNNED (2o caminho =====
// =====    de fim-de-turno, combat_state_machine.cpp:559 expire_on_stunned_turn_end - =====
// =====    gemeo de run_active_turn_to_end, so acessivel via run_until_end() porque =====
// =====    expire_on_stunned_turn_end e privado) ==========================================

TEST_CASE("techmagic repeat: ada tambem ecoa quando quem fecha o turno e um aliado "
         "STUNNED (expire_on_stunned_turn_end, o 2o caminho de fim-de-turno)",
         "[domain][combat][techmagic][repeat]") {
    CombatActor h1 = make_actor("h1", true, 100, 6, 0, 50);        // ataca e; spd mais alta.
    CombatActor h_stun = make_actor("h_stun", true, 100, 0, 0, 45); // perde o turno (Stun).
    CombatActor h_ada = make_actor("h_ada", true, 100, 0, 0, 40);   // dono do Ada.
    CombatActor e = make_actor("e", false, 18, 0, 0, 10);  // 18 = 6 base + 2 ecos de 6.
    h_ada.set_equipped_special_ids({"ada_test"});
    h_stun.add_status(StatusEffect{.id = StatusId::Stun, .duration = 1});

    auto reg = registry({ada_card("ada_test", /*chance=*/34)});

    // h1 ataca e na rodada 0 E na rodada 1 (rede de seguranca: se o caminho stunned NAO
    // disparasse o hook, o combate so fecharia na rodada 1, nao na 0 - ver comentario da
    // asercao de rounds_elapsed abaixo).
    auto provider = provider_for(
        {{"h1", {{0, {CombatAction::attack(e.id())}}, {1, {CombatAction::attack(e.id())}}}}});
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/0);  // Ada sempre dispara (roll 0 < 34).
    CombatStateMachine sm({&h1, &h_stun, &h_ada, &e}, provider, &reg, nullptr, &rng);

    const CombatResult result = sm.run_until_end();

    REQUIRE(result.outcome == CombatOutcome::Victory);
    // e: 18 - 6 (h1 ataca) - 6 (Ada eco #1, turno de h1 fechando - caminho NORMAL, ja
    // coberto noutro teste) - 6 (Ada eco #2, turno STUNNED de h_stun fechando - o caminho
    // NOVO sob teste aqui, via expire_on_stunned_turn_end) = 0, tudo ainda na rodada 0.
    REQUIRE(e.hp() == 0);
    REQUIRE_FALSE(e.is_alive());
    // Se expire_on_stunned_turn_end NAO disparasse process_ally_turn_end_hooks (o guard sob
    // teste), o 2o eco nao aconteceria: e sobreviveria a rodada 0 com hp=6, e o combate so
    // fecharia na RODADA 1 (quando h1 ataca de novo) - rounds_elapsed seria 1, nao 0.
    REQUIRE(result.rounds_elapsed == 0);
}

// ===== 5. Guarda de dominó (achado, CARD-ENGINE-MANIFESTO item 8): CloneAlly GANHOU =====
// =====    handler (deixou de ser o "sem handler" desta suite) - o switch de execute() =====
// =====    despacha pra handle_clone_ally, NAO mais pro default fail-fast. O guard de =====
// =====    validacao de argumentos migrou pro PROPRIO handler (ctx.counterpart nulo). =====
// =====    Testes EXAUSTIVOS de CloneAlly/Eco vivem em techmagic_clone_test.cpp. ===========

TEST_CASE("techmagic repeat: RepeatLastAction e CloneAlly TEM handler ambos - nenhum "
         "cai no default fail-fast do switch (mas CloneAlly ainda valida ctx.counterpart)",
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

    // CloneAlly TEM handler hoje (handle_clone_ally) - o logic_error aqui NAO vem mais do
    // default "sem handler" do switch, e sim da validacao de argumento do PROPRIO handler
    // (ctx.counterpart nulo - OnCast sempre precisa de alvo). Prova que RepeatLastAction
    // ganhar handler nao "engoliu" o caso de CloneAlly no switch (cada EffectKind cai no
    // seu PROPRIO case).
    techMagic::TechMagicContext ctx;
    ctx.caster = &h;

    REQUIRE_THROWS_AS(techMagic::execute(TriggerHook::OnCast, clone_card, ctx),
                      std::logic_error);

    // RepeatLastAction roda ate o fim sem lancar (mesmo sem last_action echoable - vira
    // no-op+log, nao excecao).
    Card repeat_card = mandelbrot_card("techmagic.repeat.clone.failfast.mandelbrot",
                                       /*percent=*/50);
    techMagic::TechMagicContext repeat_ctx;
    repeat_ctx.caster = &h;
    const techMagic::LastActionRecord empty_record;
    repeat_ctx.last_action = &empty_record;
    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, repeat_card, repeat_ctx));
}
