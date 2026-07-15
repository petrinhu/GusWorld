// techmagic_faraday_test.cpp
//
// Spec executavel (Catch2 v3) do EM-Shield (Faraday), PR1 do "Balde B" (ADR-016, decisoes
// do lider 2026-07-15, F-1..F-4):
//   F-1: BlindagemEM bloqueia SO debuffs de origem eletrica (!is_buff && family_origin ==
//        Eletrico). Buffs eletricos e debuffs de outras familias passam direto.
//   F-2: "previne + limpa" - aplicar BlindagemEM tambem remove os debuffs eletricos JA
//        ativos no alvo (nao so previne os futuros).
//   F-3: Faraday vira Hibrida - OnCast aplica BlindagemEM (mana kActiveManaCost, side_filter
//        AllyOnly incluindo self), duracao 3, Refresh. A face fora-de-combate (anti-PEM,
//        posse-only) fica FEAT FUTURA, nao implementada aqui.
//   F-4: cast em INIMIGO dissipa (side_filter AllyOnly rejeita lado oposto), nunca lanca.
//
// Duas primitivas COMPARTILHADAS entregues aqui (reusadas pelo PR2/Newton):
//   - EffectSpec.side_filter (SideFilter::Any/EnemyOnly/AllyOnly) - filtro de lado
//     data-driven, checado em handle_apply_status.
//   - CombatActor::try_add_status - portao de imunidade UNICO (choke point): todo sitio de
//     status OFENSIVO do motor (handle_apply_status, e os 4 sitios de status_applied/
//     result_status em combat_state_machine.cpp::resolve_use_card) passa por aqui.
//
// Cartas de teste montadas LOCALMENTE (id "techmagic.faraday.*"), NUNCA do registry de
// producao (MasterCards) - mesmo padrao de techmagic_delay_test.cpp/techmagic_chain_test.cpp.
// O teste do CATALOGO (Faraday em MasterCards::build_registry()) vive em master_cards_test.cpp.
//
// NOTA DE COBERTURA (transparencia, ver teste 6b abaixo): a producao ComboTable
// (combo_table.cpp) so tem 1 recipe com result_status ("raiz_null", Regen/Bioquimico, um
// BUFF) - nenhum recipe hoje produz um DEBUFF de familia Eletrico. O BLOQUEIO especifico do
// sitio combo->result_status portanto NAO e alcancavel via dado de producao real hoje.
// LIMITE HONESTO (refino do qa adversarial 2026-07-15): como o unico result_status de
// producao e um BUFF (nunca bloqueado por add_status NEM por try_add_status), os dois
// caminhos sao OBSERVACIONALMENTE IDENTICOS pra esse dado - o teste 6b NAO prova o
// roteamento pelo choke point (um mutante que trocasse apply_offensive_status por
// add_status direto AQUI passaria despercebido). A garantia real e por IDENTIDADE DE
// CODIGO (mesma chamada nos 4 sitios, revisada), NAO por teste observavel; so uma recipe
// de combo com debuff Eletrico (decisao de conteudo do lider) tornaria o bloqueio testavel.
//
// Cross-ref: gus/domain/combat/combat_actor.hpp (StatusApplyResult/try_add_status);
//            gus/domain/combat/techmagic.cpp (handle_apply_status); combat_state_machine.cpp
//            (apply_offensive_status + os 4 sitios do dominó); master_cards.cpp (faraday);
//            docs/design/mecanicas/cartas-technomagik.md secao 5;
//            docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md (AMB-03); ADR-016.

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
#include "gus/domain/combat/random_source.hpp"
#include "gus/domain/combat/techmagic.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::FixedRandom;

namespace {

CombatActor make_actor(const std::string& id, bool player_side, int hp = 100, int atk = 0,
                       int def = 0, int spd = 20, CardFamily family = CardFamily::Eletrico) {
    return CombatActor(id, id, hp, atk, def, spd, family, player_side);
}

const StatusEffect* find_status(const CombatActor& a, StatusId id) {
    for (const auto& s : a.status_effects())
        if (s.id == id) return &s;
    return nullptr;
}

// Carta de teste pro EXECUTOR (techMagic::execute direto): 1 EffectSpec OnCast/ApplyStatus.
// A familia da carta vira family_origin do StatusEffect (mesmo racional de handle_apply_status).
Card apply_status_card(const std::string& id, CardFamily family, StatusId status,
                       int magnitude = 2, int duration = 1,
                       SideFilter side_filter = SideFilter::Any) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = family;
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
                            .status = status,
                            .stack_rule = StackRule::Replace,
                            .side_filter = side_filter}};
    return c;
}

// Duplo LOCAL da Faraday de producao: mesmo shape (Hibrida/Eletrico, OnCast -> ApplyStatus
// BlindagemEM dur 3 Refresh AllyOnly), mana 0 pra focar no gate/portao (nao no ramp de mana
// da FSM) - mesma convencao "cartas de teste nunca do registry de producao".
Card faraday_card(const std::string& id) {
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
    c.category = CardCategory::Hibrida;
    c.effects = {EffectSpec{.trigger = TriggerHook::OnCast,
                            .kind = EffectKind::ApplyStatus,
                            .duration = 3,
                            .status = StatusId::BlindagemEM,
                            .stack_rule = StackRule::Refresh,
                            .side_filter = SideFilter::AllyOnly}};
    return c;
}

// Carta COMUM (tier default, categoria default) eletrica com status_applied - caminho BASE
// de resolve_use_card (NAO o executor techMagic), pro teste do dominó (#6a): sem isso, o
// portao de imunidade bloquearia so a carta ESPECIAL (Faraday), mas um Stun de carta COMUM
// eletrica passaria batido por CombatActor::add_status legado.
Card comum_electric_stun_card(const std::string& id) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Eletrico;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = 6;
    c.target_shape = TargetShape::Single;
    c.status_applied =
        StatusEffect{StatusId::Stun, 2, 1, StackRule::Replace, CardFamily::Eletrico};
    return c;
}

std::unordered_map<std::string, Card> registry(std::initializer_list<Card> cards) {
    std::unordered_map<std::string, Card> d;
    for (const auto& c : cards) d.emplace(c.id, c);
    return d;
}

// Toca as acoes de `actions`, em ordem, so quando e a vez do lado da party; dali em diante
// passa (0 AP). Mesmo padrao de play_once (techmagic_executor_test.cpp), estendido pra N
// acoes em sequencia (pro teste do gate 1x/batalha, #8).
CombatActionProvider play_sequence(std::vector<CombatAction> actions) {
    auto acts = std::make_shared<std::vector<CombatAction>>(std::move(actions));
    auto idx = std::make_shared<std::size_t>(0);
    return [acts, idx](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side() || *idx >= acts->size()) return CombatAction::pass();
        return (*acts)[(*idx)++];
    };
}

// Duplo de IRandomSource que CONTA as chamadas (mesmo padrao de techmagic_delay_test.cpp/
// techmagic_executor_test.cpp).
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

// ===== 1. Blindado recebe debuff ELETRICO: bloqueado (status ausente, log de bloqueio, =====
// =====    SEM StatusEffectChange::Applied) ==================================================

TEST_CASE("techmagic faraday: BlindagemEM ativa bloqueia debuff ELETRICO (status ausente, "
         "log de bloqueio, SEM StatusEffectChange::Applied)",
         "[domain][combat][techmagic][faraday]") {
    CombatActor caster = make_actor("h", true);
    CombatActor target = make_actor("e", false);

    REQUIRE(target.try_add_status(StatusEffect{StatusId::BlindagemEM, 0, 3, StackRule::Refresh,
                                               CardFamily::Eletrico}) ==
           StatusApplyResult::Applied);
    (void)target.drain_status_changes();  // limpa o Applied da preparacao (nao contamina a asserção).

    Card stun_eletrico = apply_status_card("techmagic.faraday.stun_eletrico",
                                           CardFamily::Eletrico, StatusId::Stun);
    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &target;
    ctx.log = &log;

    techMagic::execute(TriggerHook::OnCast, stun_eletrico, ctx);

    REQUIRE(target.index_of_status(StatusId::Stun) < 0);  // nao aplicou.
    REQUIRE(target.status_effects().size() == 1);          // so a BlindagemEM restante.
    REQUIRE(target.drain_status_changes().empty());        // SEM Applied do bloqueio.
    REQUIRE(log.size() == 1);
    REQUIRE(log.back().message.find("bloqueado") != std::string::npos);
}

// ===== 2. Blindado recebe debuff CINETICO: aplica (mata mutante family== sempre-true) =====

TEST_CASE("techmagic faraday: BlindagemEM ativa NAO bloqueia debuff CINETICO (mutante que "
         "trocaria o == de familia por sempre-true mataria a blindagem)",
         "[domain][combat][techmagic][faraday]") {
    CombatActor caster = make_actor("h", true);
    CombatActor target = make_actor("e", false);
    target.try_add_status(StatusEffect{StatusId::BlindagemEM, 0, 3, StackRule::Refresh,
                                       CardFamily::Eletrico});
    (void)target.drain_status_changes();

    Card stun_cinetico = apply_status_card("techmagic.faraday.stun_cinetico",
                                           CardFamily::Cinetico, StatusId::Stun);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &target;

    techMagic::execute(TriggerHook::OnCast, stun_cinetico, ctx);

    const StatusEffect* stun = find_status(target, StatusId::Stun);
    REQUIRE(stun != nullptr);
    REQUIRE(stun->magnitude == 2);
}

// ===== 3. Blindado recebe BUFF eletrico: aplica (mata mutante que esquece !is_buff) =====

TEST_CASE("techmagic faraday: BlindagemEM ativa NAO bloqueia BUFF eletrico (mutante que "
         "esquece o !is_buff no portao mataria buffs legitimos)",
         "[domain][combat][techmagic][faraday]") {
    CombatActor caster = make_actor("h", true);
    CombatActor target = make_actor("e", false);
    target.try_add_status(StatusEffect{StatusId::BlindagemEM, 0, 3, StackRule::Refresh,
                                       CardFamily::Eletrico});
    (void)target.drain_status_changes();

    Card shield_eletrico = apply_status_card("techmagic.faraday.shield_eletrico",
                                             CardFamily::Eletrico, StatusId::Shield,
                                             /*magnitude=*/5, /*duration=*/2);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &target;

    techMagic::execute(TriggerHook::OnCast, shield_eletrico, ctx);

    const StatusEffect* shield = find_status(target, StatusId::Shield);
    REQUIRE(shield != nullptr);
    REQUIRE(shield->magnitude == 5);
}

// ===== 4. Aplicar BlindagemEM em alvo COM SobrecargaTermica ativa: limpa (F-2); status =====
// =====    NAO-eletricos sobrevivem =========================================================

TEST_CASE("techmagic faraday: aplicar BlindagemEM LIMPA debuffs eletricos ja ativos (F-2); "
         "status NAO-eletricos sobrevivem",
         "[domain][combat][techmagic][faraday]") {
    CombatActor caster = make_actor("h", true);
    CombatActor target = make_actor("e", true);  // aliado do caster (side_filter AllyOnly).

    target.try_add_status(StatusEffect{StatusId::SobrecargaTermica, 8, 3, StackRule::Refresh,
                                       CardFamily::Eletrico});
    target.try_add_status(
        StatusEffect{StatusId::Poison, 3, 2, StackRule::Replace, CardFamily::Bioquimico});
    (void)target.drain_status_changes();

    Card faraday = faraday_card("techmagic.faraday.cleanse");
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &target;

    techMagic::execute(TriggerHook::OnCast, faraday, ctx);

    REQUIRE(find_status(target, StatusId::SobrecargaTermica) == nullptr);  // limpo (F-2).
    REQUIRE(find_status(target, StatusId::Poison) != nullptr);             // sobrevive.
    REQUIRE(find_status(target, StatusId::BlindagemEM) != nullptr);        // aplicada.
}

// ===== 4b. Aplicar BlindagemEM em alvo com BUFF de familia ELETRICA ja ativo (Shield =====
// =====     de Defend, family_origin==Eletrico por convencao de resolve_defend): o =====
// =====     cleanse F-2 so mata DEBUFF, o buff sobrevive (mata mutante que remove o =====
// =====     !is_buff de clear_electric_debuffs - StatusEffect.family_origin default e =====
// =====     Eletrico, entao QUALQUER Shield criado sem family_origin explicito, e o Shield =====
// =====     do proprio Defend, cairia nessa armadilha se o cleanse nao checasse is_buff) ====

TEST_CASE("techmagic faraday: aplicar BlindagemEM NAO limpa BUFF de familia ELETRICA ja ativo "
         "(Shield estilo Defend, family_origin==Eletrico) - so debuff eletrico e alvo do F-2",
         "[domain][combat][techmagic][faraday]") {
    CombatActor caster = make_actor("h", true);
    CombatActor target = make_actor("e", true);  // aliado do caster (side_filter AllyOnly).

    // Mesma convencao de CombatStateMachine::resolve_defend (combat_state_machine.cpp): Shield
    // criado com family_origin==Eletrico explicito, MESMO sem ligacao tematica com a familia
    // Eletrico - e so o default/convencao do Defend. Se clear_electric_debuffs perder o guard
    // is_buff, este Shield (buff legitimo) seria apagado junto com o SobrecargaTermica.
    target.try_add_status(
        StatusEffect{StatusId::Shield, 10, 1, StackRule::Replace, CardFamily::Eletrico});
    target.try_add_status(StatusEffect{StatusId::SobrecargaTermica, 8, 3, StackRule::Refresh,
                                       CardFamily::Eletrico});
    (void)target.drain_status_changes();

    Card faraday = faraday_card("techmagic.faraday.cleanse_preserves_buff");
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &target;

    techMagic::execute(TriggerHook::OnCast, faraday, ctx);

    REQUIRE(find_status(target, StatusId::SobrecargaTermica) == nullptr);  // debuff: limpo.
    const StatusEffect* shield = find_status(target, StatusId::Shield);
    REQUIRE(shield != nullptr);         // buff eletrico: sobrevive (F-2 so mata debuff).
    REQUIRE(shield->magnitude == 10);   // intacto, nao so "presente".
    REQUIRE(find_status(target, StatusId::BlindagemEM) != nullptr);
}

// ===== 5. Cast em INIMIGO: dissipa (side_filter AllyOnly, F-4), estado do inimigo intacto =====

TEST_CASE("techmagic faraday: cast em alvo INIMIGO dissipa (side_filter AllyOnly, F-4), "
         "estado do inimigo intacto, nunca lanca",
         "[domain][combat][techmagic][faraday]") {
    CombatActor caster = make_actor("h", true);
    CombatActor foe_actor = make_actor("e", false);

    Card faraday = faraday_card("techmagic.faraday.enemy_dissipate");
    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &foe_actor;
    ctx.log = &log;

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, faraday, ctx));

    REQUIRE(foe_actor.status_effects().empty());
    REQUIRE(log.size() == 1);
    REQUIRE(log.back().message.find("hostis") != std::string::npos);
}

// ===== 5b. Espelho do F-4: SideFilter::EnemyOnly (primitiva compartilhada, reusada pelo =====
// =====     PR2/Newton - ZERO carta de producao usa EnemyOnly hoje, so AllyOnly via =====
// =====     Faraday; sem este teste um mutante que trocasse o `ally` do ramo EnemyOnly por =====
// =====     sempre-false sobreviveria) - dissipa em ALIADO, aplica em INIMIGO =============

TEST_CASE("techmagic faraday: SideFilter::EnemyOnly (espelho de F-4, primitiva "
         "compartilhada) - dissipa em ALIADO, aplica normalmente em INIMIGO",
         "[domain][combat][techmagic][faraday]") {
    // Ramo dissipa: alvo ALIADO com filtro EnemyOnly.
    {
        CombatActor caster = make_actor("h", true);
        CombatActor ally = make_actor("h2", true);
        Card enemy_only = apply_status_card("techmagic.faraday.enemyonly_dissipate",
                                            CardFamily::Cinetico, StatusId::Stun,
                                            /*magnitude=*/2, /*duration=*/1,
                                            SideFilter::EnemyOnly);
        std::vector<CombatLogEntry> log;
        techMagic::TechMagicContext ctx;
        ctx.caster = &caster;
        ctx.counterpart = &ally;
        ctx.log = &log;

        techMagic::execute(TriggerHook::OnCast, enemy_only, ctx);

        REQUIRE(ally.status_effects().empty());
        REQUIRE(log.size() == 1);
        REQUIRE(log.back().message.find("aliados") != std::string::npos);
    }

    // Ramo aplica: alvo INIMIGO com filtro EnemyOnly.
    {
        CombatActor caster = make_actor("h", true);
        CombatActor foe = make_actor("e", false);
        Card enemy_only = apply_status_card("techmagic.faraday.enemyonly_apply",
                                            CardFamily::Cinetico, StatusId::Stun,
                                            /*magnitude=*/2, /*duration=*/1,
                                            SideFilter::EnemyOnly);

        techMagic::TechMagicContext ctx;
        ctx.caster = &caster;
        ctx.counterpart = &foe;
        techMagic::execute(TriggerHook::OnCast, enemy_only, ctx);

        const StatusEffect* stun = find_status(foe, StatusId::Stun);
        REQUIRE(stun != nullptr);
        REQUIRE(stun->magnitude == 2);
    }
}

// ===== 6a. Dominó: status_applied de carta COMUM eletrica tambem e bloqueado (via FSM) =====

TEST_CASE("techmagic faraday (via FSM, dominó): status_applied de carta COMUM ELETRICA e "
         "bloqueado quando o alvo esta blindado (sem isso, uma carta comum passaria batida "
         "mesmo com o Faraday bloqueando a carta especial)",
         "[domain][combat][techmagic][faraday]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor target = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10);
    target.try_add_status(StatusEffect{StatusId::BlindagemEM, 0, 3, StackRule::Refresh,
                                       CardFamily::Eletrico});
    (void)target.drain_status_changes();

    Card common = comum_electric_stun_card("techmagic.faraday.common_stun");
    auto reg = registry({common});
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);  // canal COMUM, sem crit/fumble.
    auto provider = play_sequence({CombatAction::use_card(common.id, target.id())});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(find_status(target, StatusId::Stun) == nullptr);
    bool blocked_logged = false;
    for (const auto& entry : sm.log())
        if (entry.message.find("bloqueado") != std::string::npos) blocked_logged = true;
    REQUIRE(blocked_logged);
}

// ===== 6b. Dominó: combo->result_status roteia pelo MESMO portao (nota de cobertura no =====
// =====     topo do arquivo - producao nao tem recipe com debuff eletrico pra testar o ======
// =====     bloqueio em si; este teste prova que o sitio NAO regrediu) ======================

TEST_CASE("techmagic faraday (via FSM, dominó): combo->result_status roteia pelo mesmo "
         "helper de try_add_status (raiz_null/Regen, buff Bioquimico - nao bloqueado, "
         "prova ausencia de regressao no sitio)",
         "[domain][combat][techmagic][faraday]") {
    CombatActor caster = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor target = make_actor("e", false, /*hp=*/300, /*atk=*/0, /*def=*/0, /*spd=*/10);
    target.set_scanned(true);  // precondicao do modificador Null (resolve_use_card).
    target.try_add_status(StatusEffect{StatusId::BlindagemEM, 0, 3, StackRule::Refresh,
                                       CardFamily::Eletrico});
    (void)target.drain_status_changes();

    Card raiz;
    raiz.id = "raiz.bioquimico";  // casa a assinatura da recipe "raiz_null" (combo_table.cpp).
    raiz.display_name = raiz.id;
    raiz.family = CardFamily::Bioquimico;
    raiz.base_type = CardBaseType::Raiz;
    raiz.mana_cost = 0;
    raiz.ap_cost = 1;
    raiz.power = 0;
    raiz.target_shape = TargetShape::Single;
    auto reg = registry({raiz});

    CombatAction cast = CombatAction::use_card(raiz.id, target.id());
    cast.modifier = CardModifier::Null;

    FixedRandom rng(0.5, 99);
    auto provider = play_sequence({cast});
    CombatStateMachine sm({&caster, &target}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    const StatusEffect* regen = find_status(target, StatusId::Regen);
    REQUIRE(regen != nullptr);       // buff aplicou normalmente (nao bloqueado, F-1: so debuff).
    REQUIRE(regen->magnitude == 5);  // raiz_null: Regen mag 5 dur 2 (combo_table.cpp).
    REQUIRE(find_status(target, StatusId::BlindagemEM) != nullptr);  // blindagem intacta.
}

// ===== 7. Expiracao: apos 3 turnos DO ALVO a blindagem some; proximo debuff eletrico aplica =====

TEST_CASE("techmagic faraday (via FSM): BlindagemEM expira apos 3 turnos do alvo - dali em "
         "diante um debuff eletrico volta a aplicar normalmente",
         "[domain][combat][techmagic][faraday]") {
    CombatActor target = make_actor("e", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor filler = make_actor("filler", false, /*hp=*/100, /*atk=*/0, /*def=*/0,
                                    /*spd=*/10);
    REQUIRE(target.try_add_status(StatusEffect{StatusId::BlindagemEM, 0, 3, StackRule::Refresh,
                                               CardFamily::Eletrico}) ==
           StatusApplyResult::Applied);
    (void)target.drain_status_changes();

    CombatActionProvider provider = [](CombatActor&, const CombatState&) {
        return CombatAction::pass();
    };
    CombatStateMachine sm({&target, &filler}, provider, nullptr, nullptr, nullptr);

    // 6 turnos (target,filler) x3 = 3 rodadas = exatamente 3 ticks do TARGET (duration
    // 3->2->1->0, expira no TurnEnd do 3o tick do proprio alvo).
    for (int i = 0; i < 6; ++i) {
        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
    }

    REQUIRE(target.index_of_status(StatusId::BlindagemEM) < 0);  // expirou.

    // Proximo debuff eletrico aplica normalmente (o portao ja nao trava).
    REQUIRE(target.try_add_status(StatusEffect{StatusId::Stun, 2, 1, StackRule::Replace,
                                               CardFamily::Eletrico}) ==
           StatusApplyResult::Applied);
}

// ===== 8. 1x/batalha: 2o cast de Faraday lanca ERRO DE COMPILACAO (mesmo gate das outras =====
// =====    Ativa/Hibrida, resolve_use_card) ==================================================

TEST_CASE("techmagic faraday (via FSM): 1x/batalha - 2o cast lanca ERRO DE COMPILACAO",
         "[domain][combat][techmagic][faraday]") {
    CombatActor h = make_actor("h", true, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e = make_actor("e", false, /*hp=*/100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    Card faraday = faraday_card("techmagic.faraday.gate");  // mana 0: foco no gate, nao no ramp.
    auto reg = registry({faraday});
    FixedRandom rng(0.5, 99);

    int calls = 0;
    CombatActionProvider provider = [&](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side()) return CombatAction::pass();
        ++calls;
        return CombatAction::use_card(faraday.id, h.id());  // self-cast (AllyOnly aceita).
    };

    CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, &rng);
    sm.begin_turn();

    bool threw = false;
    try {
        sm.run_active_turn_to_end();
    } catch (const std::logic_error& ex) {
        threw = true;
        REQUIRE(std::string(ex.what()).find("ERRO DE COMPILACAO") != std::string::npos);
    }
    REQUIRE(threw);
    REQUIRE(calls == 2);
    REQUIRE(find_status(h, StatusId::BlindagemEM) != nullptr);  // 1o cast aplicou normalmente.
}

// ===== 9. Determinismo: 0 consumo de RNG no handler ApplyStatus, nos dois ramos =====
// =====    (aplicado e bloqueado) ============================================================

TEST_CASE("techmagic faraday: ZERO consumo de RNG no handler ApplyStatus/BlindagemEM, tanto "
         "no ramo APLICADO quanto no ramo BLOQUEADO",
         "[domain][combat][techmagic][faraday]") {
    // Ramo aplicado.
    {
        CombatActor caster = make_actor("h", true);
        CombatActor ally = make_actor("h2", true);
        Card faraday = faraday_card("techmagic.faraday.rng_applied");
        CountingRandom counting;
        techMagic::TechMagicContext ctx;
        ctx.caster = &caster;
        ctx.counterpart = &ally;
        ctx.rng = &counting;

        techMagic::execute(TriggerHook::OnCast, faraday, ctx);

        REQUIRE(find_status(ally, StatusId::BlindagemEM) != nullptr);
        REQUIRE(counting.next_calls == 0);
        REQUIRE(counting.next_double_calls == 0);
    }

    // Ramo bloqueado.
    {
        CombatActor caster = make_actor("h", true);
        CombatActor ally = make_actor("h2", true);
        ally.try_add_status(StatusEffect{StatusId::BlindagemEM, 0, 3, StackRule::Refresh,
                                         CardFamily::Eletrico});
        (void)ally.drain_status_changes();

        Card stun_eletrico = apply_status_card("techmagic.faraday.rng_blocked",
                                               CardFamily::Eletrico, StatusId::Stun);
        CountingRandom counting;
        techMagic::TechMagicContext ctx;
        ctx.caster = &caster;
        ctx.counterpart = &ally;
        ctx.rng = &counting;

        techMagic::execute(TriggerHook::OnCast, stun_eletrico, ctx);

        REQUIRE(ally.index_of_status(StatusId::Stun) < 0);
        REQUIRE(counting.next_calls == 0);
        REQUIRE(counting.next_double_calls == 0);
    }
}
