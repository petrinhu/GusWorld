// techmagic_delay_test.cpp
//
// Spec executavel (Catch2 v3) do executor techMagic (ADR-016, MVP step 7):
// EffectKind::DelayAction (Einstein/Time-Dilate). Regras (decisao do lider 2026-07-15,
// brief TIME-DILATE):
//   - Empurra a acao do alvo pro FIM da fila da rodada corrente (spec.magnitude == 0) ou
//     N posicoes fixas (spec.magnitude > 0), via InitiativeQueue::reorder_actor (MESMA
//     primitiva do Gambito-Reordenar).
//   - Alvo que JA AGIU nesta rodada (indice em order() < cursor()) dissipa a carta: no-op
//     + log, NAO banca pra proxima rodada.
//   - Alvo que E o current() (em acao agora), morto, ou fora da fila: tambem no-op + log
//     (estados NORMAIS, nao erro).
//   - 0 consumo de RNG (DelayAction nunca sorteia).
//   - Sem dano: NAO toca take_damage/round_hits/last_action.
//   - O empurrao e uma reordenacao PERSISTENTE ate a proxima recomputacao natural por SPD
//     (InitiativeQueue::recompute_by_speed desfaz, mesma regra do Gambito - contrato D3).
//
// Cartas de teste montadas localmente (id "techmagic.delay.*"), NUNCA do registry de
// producao (MasterCards) - o teste do catalogo vive em master_cards_test.cpp.
//
// Cross-ref: gus/domain/combat/techmagic.hpp (TechMagicContext.queue);
//            gus/domain/combat/initiative_queue.hpp (reorder_actor/cursor/current);
//            combat_state_machine.cpp (wiring OnCast: cast_ctx.queue = &queue_);
//            docs/design/mecanicas/cartas-technomagik.md; ADR-016;
//            docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md (AMB-02).

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
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
#include "gus/domain/combat/initiative_queue.hpp"
#include "gus/domain/combat/random_source.hpp"
#include "gus/domain/combat/techmagic.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::FixedRandom;

namespace {

CombatActor make_actor(const std::string& id, bool player_side, int hp, int atk = 0,
                       int def = 0, int spd = 20) {
    return CombatActor(id, id, hp, atk, def, spd, CardFamily::Eletrico, player_side);
}

// Carta Einstein de teste: Ativa, OnCast -> DelayAction. magnitude = 0 (fim da fila) ou N
// (posicoes fixas). power=0 (Einstein nao causa dano; a base loop de resolve_use_card
// ainda roda, mas com power+atk=0 o dano fica 0 e nao grava last_action, ver teste 7).
Card delay_card(const std::string& id, int magnitude) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Cinetico;
    c.base_type = CardBaseType::Glifo;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = 0;
    c.target_shape = TargetShape::Single;
    c.tier = CardTier::Especial;
    c.category = CardCategory::Ativa;
    c.effects = {EffectSpec{.trigger = TriggerHook::OnCast,
                            .kind = EffectKind::DelayAction,
                            .magnitude = magnitude}};
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

// ===== 1. Fim da fila (magnitude 0): alvo no meio da ordem termina EXATAMENTE no ultimo =====
// =====    slot (mata mutante que underestima o delta, ex. (count-2)-index) ==================

TEST_CASE("techmagic delay: fim da fila (magnitude 0) - alvo no meio termina exatamente no "
         "ultimo slot; current()/cursor inalterados",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor e1 = make_actor("e1", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/30);
    CombatActor e2 = make_actor("e2", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/20);
    InitiativeQueue queue({&caster, &e0, &e1, &e2});  // ordem por SPD: h, e0, e1, e2.

    Card einstein = delay_card("techmagic.delay.end", /*magnitude=*/0);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &e0;
    ctx.queue = &queue;

    techMagic::execute(TriggerHook::OnCast, einstein, ctx);

    REQUIRE(queue.order() == std::vector<CombatActor*>{&caster, &e1, &e2, &e0});
    REQUIRE(queue.current() == &caster);  // cursor nao mexeu.
    REQUIRE(queue.cursor() == 0);
}

// ===== 2. magnitude > 0: empurra exatamente N posicoes (mata mutante que ignora spec) =====

TEST_CASE("techmagic delay: magnitude > 0 empurra exatamente N posicoes fixas",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor e1 = make_actor("e1", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/30);
    CombatActor e2 = make_actor("e2", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/20);
    CombatActor e3 = make_actor("e3", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/10);
    InitiativeQueue queue({&caster, &e0, &e1, &e2, &e3});  // e0 no indice 1.

    Card einstein = delay_card("techmagic.delay.fixed", /*magnitude=*/2);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &e0;
    ctx.queue = &queue;

    techMagic::execute(TriggerHook::OnCast, einstein, ctx);

    // e0 sai do indice 1 e vai pro indice 3 (1+2): [h, e1, e2, e0, e3].
    REQUIRE(queue.order() == std::vector<CombatActor*>{&caster, &e1, &e2, &e0, &e3});
}

// ===== 3. Clamp: alvo ja e o ultimo -> ordem inalterada, sem estouro =====

TEST_CASE("techmagic delay: alvo ja e o ultimo da fila - ordem inalterada, sem estouro",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e1 = make_actor("e1", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/30);
    CombatActor e0 = make_actor("e0", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/20);  // ja ultimo.
    InitiativeQueue queue({&caster, &e1, &e0});
    const std::vector<CombatActor*> before = queue.order();

    Card einstein = delay_card("techmagic.delay.clamp", /*magnitude=*/0);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &e0;
    ctx.queue = &queue;

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, einstein, ctx));
    REQUIRE(queue.order() == before);
}

// ===== 4. Ja agiu nesta rodada (indice < cursor): dissipa - ordem BYTE-IDENTICA + log =====

TEST_CASE("techmagic delay: alvo que ja agiu nesta rodada dissipa a carta (ordem "
         "byte-identica + log de dissipacao)",
         "[domain][combat][techmagic][delay]") {
    CombatActor e0 = make_actor("e0", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor caster = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor e1 = make_actor("e1", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/30);
    InitiativeQueue queue({&e0, &caster, &e1});  // ordem: e0, h, e1.
    queue.advance();                             // cursor 0 -> 1 (e0 "ja agiu").
    REQUIRE(queue.current() == &caster);
    const std::vector<CombatActor*> before = queue.order();

    Card einstein = delay_card("techmagic.delay.already_acted", /*magnitude=*/0);
    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &e0;  // e0 esta no indice 0, cursor e 1 -> ja agiu.
    ctx.queue = &queue;
    ctx.log = &log;

    techMagic::execute(TriggerHook::OnCast, einstein, ctx);

    REQUIRE(queue.order() == before);  // byte-identica (mata mutante `<` -> `<=`).
    REQUIRE(log.size() == 1);
    REQUIRE(log.back().message.find("dissipa") != std::string::npos);
}

// ===== 5. Guards fail-fast: queue nulo e counterpart nulo lancam logic_error =====

TEST_CASE("techmagic delay: guards fail-fast (queue nulo e counterpart nulo lancam "
         "logic_error)",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("h", true, 100);
    CombatActor e0 = make_actor("e0", false, 300);
    InitiativeQueue queue({&caster, &e0});

    Card einstein = delay_card("techmagic.delay.guard", /*magnitude=*/0);

    // queue == nullptr -> bug de call site (a FSM sempre injeta &queue_).
    {
        techMagic::TechMagicContext ctx;
        ctx.caster = &caster;
        ctx.counterpart = &e0;
        ctx.queue = nullptr;
        REQUIRE_THROWS_AS(techMagic::execute(TriggerHook::OnCast, einstein, ctx),
                          std::logic_error);
    }

    // counterpart == nullptr -> OnCast sempre tem alvo.
    {
        techMagic::TechMagicContext ctx;
        ctx.caster = &caster;
        ctx.counterpart = nullptr;
        ctx.queue = &queue;
        REQUIRE_THROWS_AS(techMagic::execute(TriggerHook::OnCast, einstein, ctx),
                          std::logic_error);
    }
}

// ===== 6. Determinismo: DelayAction nao consome RNG (0 next / 0 next_double) =====

TEST_CASE("techmagic delay: ZERO consumo de RNG (o handler nunca sorteia)",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor e1 = make_actor("e1", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/30);
    InitiativeQueue queue({&caster, &e0, &e1});

    Card einstein = delay_card("techmagic.delay.rng", /*magnitude=*/0);
    CountingRandom counting;
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &e0;
    ctx.queue = &queue;
    ctx.rng = &counting;  // presente, mas DelayAction nunca deve toca-lo.

    techMagic::execute(TriggerHook::OnCast, einstein, ctx);

    // A dilatacao rodou de fato (prova que 0-consumo nao e no-op mascarado)...
    REQUIRE(queue.order().back() == &e0);
    // ...e mesmo assim nao houve sorteio.
    REQUIRE(counting.next_calls == 0);
    REQUIRE(counting.next_double_calls == 0);
}

// ===== 7. Nao grava last_action (via FSM): castear Einstein NAO sobrescreve o registro =====
// =====    da ULTIMA acao de dano (adversarial - interacao com RepeatLastAction) ============

TEST_CASE("techmagic delay (via FSM): castear Einstein (sem dano) preserva o "
         "LastActionRecord da acao anterior, nao o sobrescreve",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/40);

    auto reg = registry({delay_card("einstein_test", /*magnitude=*/0)});

    // h ataca e0 (grava last_action_ com dano>0, kMinDamage=1 mesmo com atk=0), DEPOIS
    // conjura Einstein no mesmo alvo (dano 0 - power=0/atk=0 -> nao deveria sobrescrever).
    auto provider = provider_for({{"h",
                                   {{0,
                                     {CombatAction::attack(e0.id()),
                                      CombatAction::use_card("einstein_test", e0.id())}}}}});
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);  // sem crit/fumble.
    CombatStateMachine sm({&caster, &e0}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    sm.run_active_turn_to_end();  // Attack (grava last_action) + UseCard Einstein (0 dano).

    REQUIRE(sm.last_action().type == CombatActionType::Attack);  // NAO virou UseCard.
    REQUIRE(sm.last_action().actor == &caster);
    REQUIRE(sm.last_action().hits.size() == 1);
    REQUIRE(sm.last_action().hits.front().first == &e0);
}

// ===== 8. End-to-end FSM (anti turno-duplo): cada ator age exatamente 1x na rodada E o =====
// =====    alvo (empurrado) age por ultimo ==================================================

TEST_CASE("techmagic delay (via FSM): rodada completa com Einstein - cada ator age "
         "exatamente uma vez e o alvo empurrado age por ultimo",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor e1 = make_actor("e1", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/30);

    auto reg = registry({delay_card("einstein_test", /*magnitude=*/0)});
    auto provider = provider_for(
        {{"h", {{0, {CombatAction::use_card("einstein_test", e0.id())}}}}});
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);
    CombatStateMachine sm({&caster, &e0, &e1}, provider, &reg, nullptr, &rng);

    std::vector<std::string> act_order;
    for (int i = 0; i < 3; ++i) {
        act_order.push_back(sm.queue().current()->id());
        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
    }

    // h age primeiro (empurra e0 pro fim); e1 e e0 seguem na ordem resultante - e0
    // (alvo empurrado) age por ultimo. Cada id aparece exatamente 1x (sem turno duplo).
    REQUIRE(act_order == std::vector<std::string>{"h", "e1", "e0"});
    REQUIRE(act_order.back() == "e0");
}

// ===== 9. Recompute desfaz o empurrao (contrato D3, mesma regra do Gambito) =====

TEST_CASE("techmagic delay: recompute_by_speed (gatilho de mudanca de SPD, ex. expiracao "
         "de Haste/Slow) re-sorta a fila e desfaz o empurrao do Einstein",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor e1 = make_actor("e1", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/30);
    InitiativeQueue queue({&caster, &e0, &e1});

    Card einstein = delay_card("techmagic.delay.recompute", /*magnitude=*/0);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &e0;
    ctx.queue = &queue;
    techMagic::execute(TriggerHook::OnCast, einstein, ctx);

    REQUIRE(queue.order() == std::vector<CombatActor*>{&caster, &e1, &e0});  // empurrado.

    // Um evento que muda SPD (ex. Haste/Slow expira) chama recompute_by_speed - o mesmo
    // primitivo que desfaz reordenacoes manuais do Gambito. Reordena por SPD "crua"
    // (inalterada aqui), o que ja e suficiente pra provar que o empurrao NAO sobrevive.
    queue.recompute_by_speed();

    REQUIRE(queue.order() == std::vector<CombatActor*>{&caster, &e0, &e1});  // empurrao some.
}

// ===== 10. Alvo morto ou fora da fila: no-op + log (defensivo) =====

TEST_CASE("techmagic delay: alvo morto ou fora da fila - no-op + log, sem lancar",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/40);
    InitiativeQueue queue({&caster, &e0});

    Card einstein = delay_card("techmagic.delay.dead_or_absent", /*magnitude=*/0);

    // Alvo morto (ainda na fila).
    {
        e0.take_damage(300);
        REQUIRE_FALSE(e0.is_alive());
        const std::vector<CombatActor*> before = queue.order();
        std::vector<CombatLogEntry> log;
        techMagic::TechMagicContext ctx;
        ctx.caster = &caster;
        ctx.counterpart = &e0;
        ctx.queue = &queue;
        ctx.log = &log;
        REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, einstein, ctx));
        REQUIRE(queue.order() == before);
        REQUIRE(log.size() == 1);
    }

    // Alvo fora da fila (nunca esteve nela).
    {
        CombatActor outsider = make_actor("ghost", false, 100);
        const std::vector<CombatActor*> before = queue.order();
        std::vector<CombatLogEntry> log;
        techMagic::TechMagicContext ctx;
        ctx.caster = &caster;
        ctx.counterpart = &outsider;
        ctx.queue = &queue;
        ctx.log = &log;
        REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, einstein, ctx));
        REQUIRE(queue.order() == before);
        REQUIRE(log.size() == 1);
    }
}

// ===== 11. Alvo E o current() (auto-alvo/quem esta em resolucao agora) - no-op + log =====
// =====     (mutation testing adversarial 2026-07-15, mutante SOBREVIVENTE: remover este ===
// =====     guard NAO quebra nenhum dos testes 1-10 porque index_of(current())==cursor() ===
// =====     e o guard de "ja agiu" (< cursor) exige STRITAMENTE menor - sem este teste, o ==
// =====     reorder_actor desloca o PROPRIO ator cuja resolucao esta em andamento sem =======
// =====     mover cursor_, dessincronizando queue.current() do ator que a FSM pensa estar ==
// =====     resolvendo (evidencia empirica: order vira [e0,e1,caster], cursor fica 0, =====
// =====     current() passa a ser e0 mesmo com o caster ainda "em turno") ===================

TEST_CASE("techmagic delay: alvo E o current() (auto-alvo em pleno turno) - no-op + log, "
         "NAO desloca quem esta em resolucao (guard anti-desync cursor/ator)",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor e1 = make_actor("e1", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/30);
    InitiativeQueue queue({&caster, &e0, &e1});
    REQUIRE(queue.current() == &caster);
    const std::vector<CombatActor*> before = queue.order();

    Card einstein = delay_card("techmagic.delay.self_current", /*magnitude=*/0);
    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &caster;  // alvo == current(): o proprio dono, em plena resolucao.
    ctx.queue = &queue;
    ctx.log = &log;

    techMagic::execute(TriggerHook::OnCast, einstein, ctx);

    REQUIRE(queue.order() == before);      // byte-identica: quem esta resolvendo nao pode
                                            // ser deslocado por dentro da propria resolucao.
    REQUIRE(queue.current() == &caster);   // cursor/ator continuam sincronizados.
    REQUIRE(queue.cursor() == 0);
    REQUIRE(log.size() == 1);
    REQUIRE(log.back().message.find("dilatacao nao se aplica") != std::string::npos);
}

// ===== 12. E2E FSM: Einstein mirando quem JA AGIU nesta rodada dissipa - o round inteiro =====
// =====     continua "cada ator age exatamente 1x", sem pular nem duplicar o alvo ja-agiu ====
// =====     (fecha a pergunta do brief "o caso end-to-end anti-duplo pega o already-acted?": ==
// =====     o teste 8 existente so cobre alvo-ainda-nao-agiu; este cobre o caminho oposto) ====

TEST_CASE("techmagic delay (via FSM): Einstein mirando quem ja agiu nesta rodada dissipa - "
         "rodada completa mantem cada ator agindo exatamente 1x (sem pular nem duplicar)",
         "[domain][combat][techmagic][delay]") {
    // regroup_round_by_side (secao 4.1) agrupa o LADO INTEIRO com maior SPD primeiro: aqui
    // enemy_max(60, e0) > party_max(50, h), entao o lado inimigo abre a rodada INTEIRO
    // (e0 depois e1) antes do lado da party (h) agir. h e o ULTIMO a agir e mira e0, que ja
    // agiu no bloco anterior - exatamente o cenario "already acted" via FSM real (nao so a
    // fila crua do teste 4/11).
    CombatActor e0 = make_actor("e0", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/60);  // 1o a agir.
    CombatActor caster = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);  // ultimo.
    CombatActor e1 = make_actor("e1", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/30);  // 2o a agir.

    auto reg = registry({delay_card("einstein_test", /*magnitude=*/0)});
    auto provider = provider_for(
        {{"h", {{0, {CombatAction::use_card("einstein_test", e0.id())}}}}});
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);
    CombatStateMachine sm({&e0, &caster, &e1}, provider, &reg, nullptr, &rng);

    std::vector<std::string> act_order;
    for (int i = 0; i < 3; ++i) {
        act_order.push_back(sm.queue().current()->id());
        sm.begin_turn();
        sm.run_active_turn_to_end();  // h (3o/ultimo a agir) mira e0 (ja agiu 1o) - dissipa.
        sm.advance_to_next_actor();
    }

    // e0 e e1 agem primeiro (bloco inimigo, SPD desc); h fecha a rodada mirando e0, mas a
    // carta dissipa (e0 ja tinha agido). Cada id aparece EXATAMENTE 1x - sem turno-duplo de
    // e0, sem turno pulado.
    REQUIRE(act_order == std::vector<std::string>{"e0", "e1", "h"});
    REQUIRE(sm.queue().order() == std::vector<CombatActor*>{&e0, &e1, &caster});  // ordem intacta.
}

// ===== 13. Clamp com magnitude > 0 estourando o limite da fila - clampa sem lancar =====
// =====     (mutation testing adversarial: nenhum teste 1-10 exercitava o clamp() real de ===
// =====     InitiativeQueue::reorder_actor pelo ramo magnitude>0 - so o ramo magnitude==0 ===
// =====     testava fim-de-fila, e nele o delta calculado NUNCA estoura por construcao; =====
// =====     removendo o clamp_int inteiro de reorder_actor, os 10 testes originais deste ====
// =====     arquivo continuavam 100% verdes - SOBREVIVENTE fechado aqui) ====================

TEST_CASE("techmagic delay: magnitude > 0 muito maior que a fila clampa no ultimo slot, "
         "sem lancar nem estourar limites",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor e1 = make_actor("e1", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/30);
    InitiativeQueue queue({&caster, &e0, &e1});  // e0 no indice 1, so 1 slot livre a frente.

    Card einstein = delay_card("techmagic.delay.overflow", /*magnitude=*/99);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &e0;
    ctx.queue = &queue;

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, einstein, ctx));

    // magnitude=99 pediria indice 1+99=100; clampado no ultimo slot valido (indice 2).
    REQUIRE(queue.order() == std::vector<CombatActor*>{&caster, &e1, &e0});
}

// ===== 14. Fim da fila (magnitude 0): o VALOR logado (delta) e o EXATO numero de posicoes =====
// =====     percorridas, nao so a ordem final - mata mutante count-index (sem o -1) que ====
// =====     e EQUIVALENTE pra ordem final (o clamp de reorder_actor absorve o overshoot de =
// =====     +1) mas continua reportando um delta ERRADO no log (visivel ao jogador/telemetria)

TEST_CASE("techmagic delay: fim da fila (magnitude 0) - o delta logado e exatamente "
         "(count-1)-index, nao um valor a mais absorvido soh pelo clamp",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor e1 = make_actor("e1", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/30);
    CombatActor e2 = make_actor("e2", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/20);
    InitiativeQueue queue({&caster, &e0, &e1, &e2});  // e0 no indice 1, count=4.

    Card einstein = delay_card("techmagic.delay.log_value", /*magnitude=*/0);
    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &e0;
    ctx.queue = &queue;
    ctx.log = &log;

    techMagic::execute(TriggerHook::OnCast, einstein, ctx);

    // (count-1)-index = (4-1)-1 = 2 - a distancia REAL percorrida (indice 1 -> indice 3).
    REQUIRE(log.size() == 1);
    REQUIRE(log.back().value == 2);
}

// ===== 15. Alvo do MESMO lado do caster (aliado): o handler NAO filtra por lado - reordena =
// =====     igual, sem efeito colateral estranho. PIN de comportamento ATUAL, NAO endosso ===
// =====     de design: a flavor-text canonica ("desacelera 1 INIMIGO", AMB-02) sugere alvo ==
// =====     inimigo-only, mas nem o handler nem resolve_targets(Single) da FSM restringem ===
// =====     lado pra cartas single-target - decisao de design pendente do lider (reportado) =

TEST_CASE("techmagic delay: alvo do MESMO lado do caster (aliado) - reordena igual, sem "
         "filtro de lado no handler (pin de comportamento, decisao de design pendente)",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor ally = make_actor("h2", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor e0 = make_actor("e0", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/30);
    InitiativeQueue queue({&caster, &ally, &e0});

    Card einstein = delay_card("techmagic.delay.ally_target", /*magnitude=*/0);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &ally;  // aliado do MESMO lado do caster.
    ctx.queue = &queue;

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, einstein, ctx));
    REQUIRE(queue.order() == std::vector<CombatActor*>{&caster, &e0, &ally});
}
