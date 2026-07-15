// techmagic_newton_test.cpp
//
// Spec executavel (Catch2 v3) do Newton (Poco Gravitacional + Reflect-em-aliado), PR2 do
// "Balde B" (ADR-016, decisoes do lider 2026-07-15, N-1..N-4):
//   N-1: OnCast -> ApplyStatus Stun dur1, side_filter EnemyOnly - AoE, imobiliza TODOS os
//        inimigos vivos do grupo; o dano-de-ATK que sai da formula base (power segue 0) ja
//        acerta o mesmo grupo (fogo amigo desligado, Parte A, zera o dano em aliado).
//   N-3: OnCast -> ApplyStatus Reflect (status), side_filter AllyOnly - modo-aliado: concede
//        Reflect 30%/3t ao ALIADO alvo (ramo assimetrico de resolve_targets: Grupo mirando
//        um aliado vira single nesse 1 aliado).
//   N-4: se o dono da passiva-propria (OnDamageReceived Reflect, equipada) TAMBEM tiver o
//        Reflect-status, as duas fontes SOMAM (apply_damage_with_hooks honra as duas
//        separadamente, de graca).
//
// Tambem cobre o achado-dominó do PR2: resolve_targets(Grupo) hardcodava "!is_player_side()"
// (sempre os inimigos) - se um INIMIGO castasse uma carta Grupo, o alvo virava o proprio
// time dele. Consertado: alvos = lado OPOSTO ao `actor` (mesmo guard do ChainDamage).
//
// Cartas de teste montadas LOCALMENTE (id "techmagic.newton.*"), NUNCA do registry de
// producao (MasterCards) - o teste do CATALOGO (Newton em MasterCards::build_registry())
// vive em master_cards_test.cpp. Mesma convencao de techmagic_faraday_test.cpp/
// techmagic_delay_test.cpp/techmagic_chain_test.cpp.
//
// Cross-ref: gus/domain/combat/techmagic.hpp (side_filter, TechMagicContext);
//            combat_state_machine.cpp (Parte A: fogo amigo desligado; resolve_targets Grupo;
//            apply_damage_with_hooks: Reflect-por-STATUS); master_cards.cpp (newton);
//            docs/design/mecanicas/cartas-technomagik.md; ADR-016.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <map>
#include <memory>
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

CombatActor make_actor(const std::string& id, bool player_side, int hp = 100, int atk = 0,
                       int def = 0, int spd = 20) {
    return CombatActor(id, id, hp, atk, def, spd, CardFamily::Eletrico, player_side);
}

const StatusEffect* find_status(const CombatActor& a, StatusId id) {
    for (const auto& s : a.status_effects())
        if (s.id == id) return &s;
    return nullptr;
}

int count_log_matches(const CombatStateMachine& sm, const std::string& needle) {
    int n = 0;
    for (const auto& e : sm.log())
        if (e.message.find(needle) != std::string::npos) ++n;
    return n;
}

// Duplo LOCAL do Newton de producao (master_cards.cpp): Hibrida/Universal/Grupo, mana 0
// (foco no fix, nao no ramp de mana da FSM). 3 EffectSpec: Stun EnemyOnly (Poco
// Gravitacional) + Reflect-status AllyOnly (N-3) + Reflect-passiva OnDamageReceived
// (INTOCADA, N-4).
Card newton_card(const std::string& id, int reflect_percent = 30) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Universal;
    c.base_type = CardBaseType::Glifo;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = 0;
    c.target_shape = TargetShape::Grupo;
    c.tier = CardTier::Especial;
    c.category = CardCategory::Hibrida;
    c.effects = {EffectSpec{.trigger = TriggerHook::OnCast,
                            .kind = EffectKind::ApplyStatus,
                            .duration = 1,
                            .status = StatusId::Stun,
                            .stack_rule = StackRule::Replace,
                            .side_filter = SideFilter::EnemyOnly},
                EffectSpec{.trigger = TriggerHook::OnCast,
                            .kind = EffectKind::ApplyStatus,
                            .magnitude = reflect_percent,
                            .duration = 3,
                            .status = StatusId::Reflect,
                            .stack_rule = StackRule::Refresh,
                            .side_filter = SideFilter::AllyOnly},
                EffectSpec{.trigger = TriggerHook::OnDamageReceived,
                            .kind = EffectKind::Reflect,
                            .percent = reflect_percent}};
    return c;
}

// Carta ofensiva COMUM (sem programa techMagic) so pra exercitar a Parte A (fogo amigo
// desligado) num alvo que ja carrega o Reflect-status (teste 3b).
Card plain_attack_card(const std::string& id) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Eletrico;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = 0;
    c.target_shape = TargetShape::Single;
    return c;
}

// Passiva-propria de Reflect (OnDamageReceived), equipavel - gemeo de reflect_card()
// (techmagic_chain_test.cpp), pra provar N-4 (soma com o Reflect-status).
Card reflect_passive_card(const std::string& id, int percent) {
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
    c.effects = {EffectSpec{.trigger = TriggerHook::OnDamageReceived,
                            .kind = EffectKind::Reflect,
                            .percent = percent}};
    return c;
}

std::unordered_map<std::string, Card> registry(std::initializer_list<Card> cards) {
    std::unordered_map<std::string, Card> d;
    for (const auto& c : cards) d.emplace(c.id, c);
    return d;
}

// Plano de acoes por (ator, rodada) - mesmo padrao de techmagic_chain_test.cpp/
// techmagic_delay_test.cpp: suporta QUALQUER lado agindo (dominó/teste 8 precisa de um
// INIMIGO castando).
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

// RNG que CONTA consumos (mesmo padrao de CountingRandom* dos outros arquivos deste dir).
class CountingRandom final : public IRandomSource {
public:
    double next_double() override { ++next_double_calls; return 0.5; }
    int next(int max_value) override {
        ++next_calls;
        return max_value <= 0 ? 0 : std::min(99, max_value - 1);
    }
    int next_calls = 0;
    int next_double_calls = 0;
};

}  // namespace

// ===== 1. Cast contra inimigo: TODOS os inimigos vivos ganham Stun dur1 + tomam dano; =====
// =====    inimigo MORTO nao e alcancado (N-1, AoE) ==========================================

TEST_CASE("techmagic newton: cast contra inimigo - TODOS os inimigos VIVOS ganham Stun "
         "dur1 e tomam dano do atk do conjurador; inimigo ja morto fica de fora",
         "[domain][combat][techmagic][newton]") {
    CombatActor h = make_actor("h", true, 100, /*atk=*/10, 0, /*spd=*/50);
    CombatActor e1 = make_actor("e1", false, 100, 0, 0, /*spd=*/40);
    CombatActor e2 = make_actor("e2", false, 100, 0, 0, /*spd=*/30);
    CombatActor e3 = make_actor("e3", false, 100, 0, 0, /*spd=*/20);
    e3.take_damage(9999);  // ja morto ANTES do cast.
    REQUIRE_FALSE(e3.is_alive());

    const Card newton = newton_card("techmagic.newton.aoe");
    auto reg = registry({newton});
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);  // canal COMUM, sem crit/fumble.
    auto provider =
        provider_for({{"h", {{0, {CombatAction::use_card(newton.id, e1.id())}}}}});
    CombatStateMachine sm({&h, &e1, &e2, &e3}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    const StatusEffect* s1 = find_status(e1, StatusId::Stun);
    const StatusEffect* s2 = find_status(e2, StatusId::Stun);
    REQUIRE(s1 != nullptr);
    REQUIRE(s1->duration == 1);
    REQUIRE(s2 != nullptr);
    REQUIRE(s2->duration == 1);
    REQUIRE(e1.hp() < e1.max_hp());
    REQUIRE(e2.hp() < e2.max_hp());
    REQUIRE(e3.hp() == 0);
    REQUIRE(find_status(e3, StatusId::Stun) == nullptr);  // nunca foi alvo (ja morto).
}

// ===== 2. Cast mirando ALIADO: SO o aliado ganha Reflect; NENHUM inimigo atingido/ =====
// =====    stunado; aliado NAO toma dano (Parte A) (N-3, ramo assimetrico) ===================

TEST_CASE("techmagic newton: cast mirando ALIADO - so ele ganha Reflect-status, nenhum "
         "inimigo e afetado, aliado NAO toma dano",
         "[domain][combat][techmagic][newton]") {
    CombatActor h = make_actor("h", true, 100, /*atk=*/10, 0, /*spd=*/50);
    CombatActor buddy = make_actor("buddy", true, 100, 0, 0, /*spd=*/40);
    CombatActor e1 = make_actor("e1", false, 100, 0, 0, /*spd=*/30);

    const Card newton = newton_card("techmagic.newton.allymode");
    auto reg = registry({newton});
    FixedRandom rng(0.5, 99);
    auto provider =
        provider_for({{"h", {{0, {CombatAction::use_card(newton.id, buddy.id())}}}}});
    CombatStateMachine sm({&h, &buddy, &e1}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(buddy.hp() == buddy.max_hp());  // Parte A: fogo amigo desligado.
    REQUIRE(find_status(buddy, StatusId::Stun) == nullptr);  // EnemyOnly dissipa em aliado.
    const StatusEffect* reflect = find_status(buddy, StatusId::Reflect);
    REQUIRE(reflect != nullptr);
    REQUIRE(reflect->magnitude == 30);
    REQUIRE(reflect->duration == 3);

    REQUIRE(e1.hp() == e1.max_hp());  // nenhum inimigo alcancado.
    REQUIRE(find_status(e1, StatusId::Stun) == nullptr);
    REQUIRE(find_status(e1, StatusId::Reflect) == nullptr);  // AllyOnly dissipa em inimigo.
}

// ===== 3. Reflect-por-STATUS: hit normal reflete % arredondado; dano 0 (fogo amigo, =====
// =====    Parte A) NAO reflete (guard damage<=0 vem ANTES do check de status) ===============

TEST_CASE("techmagic newton: aliado com Reflect-status e atingido - atacante sofre % "
         "arredondado; dano 0 (fogo amigo) NAO reflete",
         "[domain][combat][techmagic][newton]") {
    // (a) hit normal via ataque basico: damage=17, reflect=round(17*0.30)=round(5.1)=5.
    {
        CombatActor attacker = make_actor("atk", false, 100, /*atk=*/17, 0, /*spd=*/50);
        CombatActor target = make_actor("tgt", true, 100, 0, 0, /*spd=*/40);
        REQUIRE(target.try_add_status(StatusEffect{StatusId::Reflect, 30, 3,
                                                   StackRule::Refresh,
                                                   CardFamily::Universal}) ==
               StatusApplyResult::Applied);
        (void)target.drain_status_changes();

        auto provider =
            provider_for({{"atk", {{0, {CombatAction::attack(target.id())}}}}});
        CombatStateMachine sm({&attacker, &target}, provider);

        sm.begin_turn();
        sm.run_active_turn_to_end();

        REQUIRE(target.hp() == target.max_hp() - 17);
        REQUIRE(attacker.hp() == attacker.max_hp() - 5);
    }

    // (b) dano 0 via fogo amigo (Parte A): aliado com Reflect-status NAO reflete nada -
    // prova que o guard damage<=0 roda ANTES da checagem de StatusId::Reflect.
    {
        CombatActor h = make_actor("h", true, 100, /*atk=*/10, 0, /*spd=*/50);
        CombatActor buddy = make_actor("buddy", true, 100, 0, 0, /*spd=*/40);
        REQUIRE(buddy.try_add_status(StatusEffect{StatusId::Reflect, 30, 3,
                                                  StackRule::Refresh,
                                                  CardFamily::Universal}) ==
               StatusApplyResult::Applied);
        (void)buddy.drain_status_changes();

        const Card plain = plain_attack_card("techmagic.newton.zero_reflect");
        auto reg = registry({plain});
        FixedRandom rng(0.5, 99);
        auto provider =
            provider_for({{"h", {{0, {CombatAction::use_card(plain.id, buddy.id())}}}}});
        CombatStateMachine sm({&h, &buddy}, provider, &reg, nullptr, &rng);

        sm.begin_turn();
        sm.run_active_turn_to_end();

        REQUIRE(buddy.hp() == buddy.max_hp());  // fogo amigo: dano 0.
        REQUIRE(h.hp() == h.max_hp());          // 0 reflexo (guard damage<=0).
    }

    // (c) arredondamento PRA CIMA importa (mata mutante "sem lround"/truncamento): damage=1,
    // magnitude=55 -> 1*0.55=0.55 -> lround=1 (reflete), int trunc=0 (NAO reflete, o guard
    // reflected<=0 quebraria o loop e nada aconteceria). Se o codigo usar truncamento em vez
    // de arredondamento, este teste falha (attacker fica ILESO e sem log de "reflete").
    {
        CombatActor attacker = make_actor("atk", false, 100, /*atk=*/5, /*def=*/0, /*spd=*/50);
        CombatActor target = make_actor("tgt", true, 100, 0, /*def=*/5, /*spd=*/40);
        REQUIRE(target.try_add_status(StatusEffect{StatusId::Reflect, /*magnitude=*/55, 3,
                                                   StackRule::Refresh,
                                                   CardFamily::Universal}) ==
               StatusApplyResult::Applied);
        (void)target.drain_status_changes();

        auto provider =
            provider_for({{"atk", {{0, {CombatAction::attack(target.id())}}}}});
        CombatStateMachine sm({&attacker, &target}, provider);

        sm.begin_turn();
        sm.run_active_turn_to_end();

        REQUIRE(target.hp() == target.max_hp() - 1);  // dano-base = kMinDamage = 1 (atk<=def).
        REQUIRE(attacker.hp() == attacker.max_hp() - 1);  // lround(1*0.55)=1, NAO 0.
        REQUIRE(count_log_matches(sm, "reflete") == 1);
    }
}

// ===== 4. Anti-recursao: A(Reflect) ataca B(Reflect) -> EXATAMENTE 1 reflexo cada, =====
// =====    sem loop (EXECUTE de verdade via ataque basico real) ==============================

TEST_CASE("techmagic newton: anti-recursao - A(Reflect) ataca B(Reflect), exatamente 1 "
         "reflexo, sem loop infinito",
         "[domain][combat][techmagic][newton]") {
    CombatActor a = make_actor("a", false, 100, /*atk=*/20, 0, /*spd=*/50);
    CombatActor b = make_actor("b", true, 100, 0, 0, /*spd=*/40);
    REQUIRE(a.try_add_status(StatusEffect{StatusId::Reflect, 30, 3, StackRule::Refresh,
                                         CardFamily::Universal}) == StatusApplyResult::Applied);
    REQUIRE(b.try_add_status(StatusEffect{StatusId::Reflect, 30, 3, StackRule::Refresh,
                                         CardFamily::Universal}) == StatusApplyResult::Applied);
    (void)a.drain_status_changes();
    (void)b.drain_status_changes();

    auto provider = provider_for({{"a", {{0, {CombatAction::attack(b.id())}}}}});
    CombatStateMachine sm({&a, &b}, provider);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    // damage a->b = max(1, 20-0) = 20; reflect de B (Reflect-status) = round(20*0.30) = 6.
    // O reflexo em A e take_damage PURO - NUNCA reentra no helper, entao o Reflect-status de
    // A jamais e consultado pra ESTE dano (sem loop A->B->A->B...).
    REQUIRE(b.hp() == b.max_hp() - 20);
    REQUIRE(a.hp() == a.max_hp() - 6);
    REQUIRE(count_log_matches(sm, "reflete") == 1);  // 1 SO reflexo, nao 2 (nem mais).
}

// ===== 5. Reflexo NAO entra em round_hits_/last_action_ (sem eco Mandelbrot/Hipotenuse) =====

TEST_CASE("techmagic newton: reflexo-por-status NAO entra no ledger (round_hits_) nem no "
         "last_action_ (sem eco em HypotenuseCombo/RepeatLastAction)",
         "[domain][combat][techmagic][newton]") {
    CombatActor a = make_actor("a", false, 100, /*atk=*/20, 0, /*spd=*/50);
    CombatActor b = make_actor("b", true, 100, 0, 0, /*spd=*/40);
    REQUIRE(b.try_add_status(StatusEffect{StatusId::Reflect, 30, 3, StackRule::Refresh,
                                         CardFamily::Universal}) == StatusApplyResult::Applied);
    (void)b.drain_status_changes();

    auto provider = provider_for({{"a", {{0, {CombatAction::attack(b.id())}}}}});
    CombatStateMachine sm({&a, &b}, provider);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(a.hp() == a.max_hp() - 6);  // reflexo aconteceu (round(20*0.30)=6).

    // round_hits_: SO o hit ORIGINAL (a->b, 20) - o reflexo (b->a, 6) nao entra.
    REQUIRE(sm.round_hits().size() == 1);
    REQUIRE(sm.round_hits()[0].attacker == &a);
    REQUIRE(sm.round_hits()[0].target == &b);
    REQUIRE(sm.round_hits()[0].damage == 20);

    // last_action_: idem, SO o hit original de `a` contra `b`.
    REQUIRE(sm.last_action().actor == &a);
    REQUIRE(sm.last_action().hits.size() == 1);
    REQUIRE(sm.last_action().hits[0].first == &b);
    REQUIRE(sm.last_action().hits[0].second == 20);
}

// ===== 6. N-4: passiva Reflect EQUIPADA + Reflect-status no MESMO ator -> as duas =====
// =====    fontes SOMAM (60% de 100 = 60, arredondado) =======================================

TEST_CASE("techmagic newton: N-4 - passiva Reflect equipada + Reflect-status somam "
         "(30%+30% de 100 = 60)",
         "[domain][combat][techmagic][newton]") {
    CombatActor attacker = make_actor("atk", false, 100, /*atk=*/100, 0, /*spd=*/50);
    CombatActor target = make_actor("tgt", true, 100, 0, 0, /*spd=*/40);
    target.set_equipped_special_ids({"reflect_passive"});
    REQUIRE(target.try_add_status(StatusEffect{StatusId::Reflect, 30, 3, StackRule::Refresh,
                                              CardFamily::Universal}) ==
           StatusApplyResult::Applied);
    (void)target.drain_status_changes();

    auto reg = registry({reflect_passive_card("reflect_passive", /*percent=*/30)});
    auto provider = provider_for({{"atk", {{0, {CombatAction::attack(target.id())}}}}});
    CombatStateMachine sm({&attacker, &target}, provider, &reg);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(target.hp() == target.max_hp() - 100);       // dano base cheio.
    REQUIRE(attacker.hp() == attacker.max_hp() - 60);    // 30 (passiva) + 30 (status) = 60.
}

// ===== 7. Determinismo: consumo de RNG do cast Grupo = SOMA dos sorteios por alvo =====

TEST_CASE("techmagic newton: determinismo - consumo de RNG do cast Grupo e a SOMA dos "
         "sorteios por alvo (3 alvos = 3 next + 3 next_double, seed fixa)",
         "[domain][combat][techmagic][newton]") {
    CombatActor h = make_actor("h", true, 100, /*atk=*/10, 0, /*spd=*/50);
    CombatActor e1 = make_actor("e1", false, 100, 0, 0, /*spd=*/40);
    CombatActor e2 = make_actor("e2", false, 100, 0, 0, /*spd=*/30);
    CombatActor e3 = make_actor("e3", false, 100, 0, 0, /*spd=*/20);

    const Card newton = newton_card("techmagic.newton.rng");
    auto reg = registry({newton});
    CountingRandom rng;  // roll fixo 99 (COMUM, sem fumble/crit) + next_double fixo 0.5.
    auto provider =
        provider_for({{"h", {{0, {CombatAction::use_card(newton.id, e1.id())}}}}});
    CombatStateMachine sm({&h, &e1, &e2, &e3}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    // 3 alvos vivos (e1,e2,e3), cada um COMUM (roll 99 fixo): 1 next() + 1 next_double() por
    // alvo - a soma prova que cada alvo sorteia INDEPENDENTEMENTE (nao 1 sorteio unico pro
    // grupo inteiro).
    REQUIRE(rng.next_calls == 3);
    REQUIRE(rng.next_double_calls == 3);
}

// ===== 8. DOMINO: INIMIGO casta carta Grupo -> atinge a PARTY (nao o proprio time) =====

TEST_CASE("techmagic newton (dominó): INIMIGO casta carta Grupo - atinge a PARTY inteira, "
         "NUNCA o proprio time do inimigo (resolve_targets consertado)",
         "[domain][combat][techmagic][newton]") {
    CombatActor e_caster = make_actor("e_caster", false, 100, /*atk=*/10, 0, /*spd=*/50);
    CombatActor e_ally = make_actor("e_ally", false, 100, 0, 0, /*spd=*/20);  // MESMO time.
    CombatActor h1 = make_actor("h1", true, 100, 0, 0, /*spd=*/40);
    CombatActor h2 = make_actor("h2", true, 100, 0, 0, /*spd=*/30);

    const Card newton = newton_card("techmagic.newton.domino");
    auto reg = registry({newton});
    FixedRandom rng(0.5, 99);
    auto provider = provider_for(
        {{"e_caster", {{0, {CombatAction::use_card(newton.id, h1.id())}}}}});
    CombatStateMachine sm({&e_caster, &e_ally, &h1, &h2}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    // A PARTY (lado OPOSTO ao e_caster) foi atingida - stunada e danificada.
    REQUIRE(find_status(h1, StatusId::Stun) != nullptr);
    REQUIRE(find_status(h2, StatusId::Stun) != nullptr);
    REQUIRE(h1.hp() < h1.max_hp());
    REQUIRE(h2.hp() < h2.max_hp());

    // O proprio time do e_caster (e_ally) NUNCA foi alvo (o hardcode antigo "!is_player_
    // side()" teria acertado e_ally, ja que ele tambem e enemy-side).
    REQUIRE(e_ally.hp() == e_ally.max_hp());
    REQUIRE(find_status(e_ally, StatusId::Stun) == nullptr);
}

// ===== 9. Anti turno-duplo/estado: Stun AoE em 2 inimigos simultaneos + fim de rodada =====
// =====    fecha a fila sem travar cursor (via run_until_end publico, caminho privado ========
// =====    expire_on_stunned_turn_end/process_ally_turn_end_hooks realmente exercitado) ======

TEST_CASE("techmagic newton: Stun AoE em 2 inimigos simultaneos - fim de rodada fecha a "
         "fila sem travar cursor; combate conclui normalmente (sem stalemate/excecao)",
         "[domain][combat][techmagic][newton]") {
    CombatActor h = make_actor("h", true, 100, /*atk=*/10, 0, /*spd=*/50);
    CombatActor e1 = make_actor("e1", false, 20, 0, 0, /*spd=*/40);
    CombatActor e2 = make_actor("e2", false, 20, 0, 0, /*spd=*/30);

    const Card newton = newton_card("techmagic.newton.antidouble");
    const Card slash = plain_attack_card("techmagic.newton.finisher");
    auto reg = registry({newton, slash});
    FixedRandom rng(0.5, 99);
    // Rodada 0: h so casta Newton (AoE: 10 dano + Stun dur1 em e1/e2, hp 20->10 cada).
    // Rodada 1: h finaliza os dois (ataque basico, 10 dano cada -> hp 0), fechando o combate
    // com AMBOS ja tendo passado por 1 turno PROPRIO stunado na rodada 0 (skip real via
    // expire_on_stunned_turn_end, exercitado pelo run_until_end publico).
    auto provider = provider_for(
        {{"h",
         {{0, {CombatAction::use_card(newton.id, e1.id())}},
          {1, {CombatAction::attack(e1.id()), CombatAction::attack(e2.id())}}}}});
    CombatStateMachine sm({&h, &e1, &e2}, provider, &reg, nullptr, &rng);

    const CombatResult result = sm.run_until_end();  // nao deve lancar (sem stalemate).

    REQUIRE(result.outcome == CombatOutcome::Victory);
    // Os 2 inimigos passaram pelo Stun (dur1) na rodada 0 e o combate seguiu normalmente ate
    // o fim - prova que 2 skips simultaneos nao travam o cursor/fila.
    REQUIRE(e1.hp() == 0);
    REQUIRE(e2.hp() == 0);
}
