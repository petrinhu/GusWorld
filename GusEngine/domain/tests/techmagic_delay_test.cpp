// techmagic_delay_test.cpp
//
// Spec executavel (Catch2 v3) do executor techMagic (ADR-016, MVP step 7 + addendum
// "modo-aliado assimetrico"): EffectKind::DelayAction (Einstein/Time-Dilate). Regras
// (decisao do lider 2026-07-15, brief TIME-DILATE + addendum ALIADO 2026-07-15):
//   - Alvo INIMIGO (lado oposto ao caster): empurra a acao pro FIM da fila da rodada
//     corrente (spec.magnitude == 0) ou N posicoes fixas atrasando (spec.magnitude > 0),
//     via InitiativeQueue::reorder_actor (MESMA primitiva do Gambito-Reordenar).
//   - Alvo ALIADO (mesmo lado do caster): ESPELHO BENEFICO - a acao ADIANTA pro primeiro
//     slot ainda-nao-agido logo apos o ator atual (spec.magnitude == 0, "ao extremo") ou N
//     posicoes fixas adiantando (spec.magnitude > 0), clampado pra nunca ultrapassar
//     cursor()+1 (invariante anti turno-duplo: o alvo NUNCA pode ir pra indice <= cursor(),
//     o que desincronizaria current() do ator realmente em resolucao).
//   - Alvo que JA AGIU nesta rodada (indice em order() < cursor()) dissipa a carta: no-op
//     + log, NAO banca pra proxima rodada - vale nos dois lados (atrasar ou adiantar).
//   - Alvo que E o current() (em acao agora), morto, ou fora da fila: tambem no-op + log
//     (estados NORMAIS, nao erro) - guard compartilhado, independente de lado.
//   - 0 consumo de RNG (DelayAction nunca sorteia), nos dois lados.
//   - Sem dano: NAO toca take_damage/round_hits/last_action.
//   - A reordenacao e PERSISTENTE ate a proxima recomputacao natural por SPD
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

// ===== 15. Alvo do MESMO lado do caster (aliado, magnitude 0): ESPELHO BENEFICO - avanca ===
// =====     pro primeiro slot ainda-nao-agido logo apos o ator atual (decisao do lider ======
// =====     2026-07-15, addendum modo-aliado assimetrico). Ninguem que ja agiu se mexe e ====
// =====     current()/cursor ficam inalterados (o caster continua "em resolucao"). ==========

TEST_CASE("techmagic delay: alvo ALIADO (magnitude 0) avanca pro primeiro slot "
         "ainda-nao-agido apos o ator atual - current()/cursor inalterados",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("p1", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor p2 = make_actor("p2", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor p3 = make_actor("p3", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/30);
    CombatActor e1 = make_actor("e1", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/20);
    InitiativeQueue queue({&caster, &p2, &p3, &e1});  // ordem por SPD: p1, p2, p3, e1.

    Card einstein = delay_card("techmagic.delay.ally_advance", /*magnitude=*/0);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &p3;  // aliado mais atras na fila.
    ctx.queue = &queue;

    techMagic::execute(TriggerHook::OnCast, einstein, ctx);

    // p3 sai do indice 2 e vai pro indice 1 (cursor()+1 = 0+1): [p1, p3, p2, e1]. p2 (que
    // nao se mexeu de proposito) desliza uma casa, mas ninguem "ja agiu" foi tocado.
    REQUIRE(queue.order() == std::vector<CombatActor*>{&caster, &p3, &p2, &e1});
    REQUIRE(queue.current() == &caster);  // cursor nao mexeu.
    REQUIRE(queue.cursor() == 0);
}

// ===== 16. Aliado JA E o proximo (indice == cursor()+1): no-op, delta 0, sem crash =====

TEST_CASE("techmagic delay: alvo ALIADO ja e o primeiro slot ainda-nao-agido - no-op, "
         "ordem inalterada",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("p1", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor p2 = make_actor("p2", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor p3 = make_actor("p3", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/30);
    InitiativeQueue queue({&caster, &p2, &p3});
    const std::vector<CombatActor*> before = queue.order();

    Card einstein = delay_card("techmagic.delay.ally_already_next", /*magnitude=*/0);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &p2;  // ja no indice cursor()+1 = 1.
    ctx.queue = &queue;

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, einstein, ctx));
    REQUIRE(queue.order() == before);
}

// ===== 17. Aliado que JA AGIU nesta rodada (indice < cursor): dissipa igual ao lado ========
// =====     inimigo - ordem byte-identica + log de dissipacao ================================

TEST_CASE("techmagic delay: alvo ALIADO que ja agiu nesta rodada dissipa a carta (ordem "
         "byte-identica + log de dissipacao)",
         "[domain][combat][techmagic][delay]") {
    CombatActor p2 = make_actor("p2", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor caster = make_actor("p1", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor e1 = make_actor("e1", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/30);
    InitiativeQueue queue({&p2, &caster, &e1});  // ordem: p2, p1(caster), e1.
    queue.advance();                             // cursor 0 -> 1 (p2 "ja agiu").
    REQUIRE(queue.current() == &caster);
    const std::vector<CombatActor*> before = queue.order();

    Card einstein = delay_card("techmagic.delay.ally_already_acted", /*magnitude=*/0);
    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &p2;  // p2 esta no indice 0, cursor e 1 -> ja agiu.
    ctx.queue = &queue;
    ctx.log = &log;

    techMagic::execute(TriggerHook::OnCast, einstein, ctx);

    REQUIRE(queue.order() == before);
    REQUIRE(log.size() == 1);
    REQUIRE(log.back().message.find("dissipa") != std::string::npos);
}

// ===== 18. Aliado que E o current() dissipa - guard compartilhado com o lado inimigo, =====
// =====     independente de lado (o alvo esta em resolucao agora, nao ha "acao futura") =====

TEST_CASE("techmagic delay: alvo ALIADO que e o current() dissipa a carta (guard "
         "compartilhado, nao especifico de lado)",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("p1", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/30);
    CombatActor p2 = make_actor("p2", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);  // current().
    CombatActor e1 = make_actor("e1", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/40);
    InitiativeQueue queue({&p2, &caster, &e1});  // ordem por SPD desc: p2, e1, p1(caster).
    REQUIRE(queue.current() == &p2);
    const std::vector<CombatActor*> before = queue.order();

    Card einstein = delay_card("techmagic.delay.ally_current", /*magnitude=*/0);
    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;  // caster nao e quem esta em resolucao neste ctx sintetico.
    ctx.counterpart = &p2;  // aliado (mesmo lado) E o current().
    ctx.queue = &queue;
    ctx.log = &log;

    techMagic::execute(TriggerHook::OnCast, einstein, ctx);

    REQUIRE(queue.order() == before);
    REQUIRE(queue.current() == &p2);
    REQUIRE(log.size() == 1);
    REQUIRE(log.back().message.find("dilatacao nao se aplica") != std::string::npos);
}

// ===== 19. Aliado com magnitude > 0: adianta exatamente N posicoes fixas =====

TEST_CASE("techmagic delay: alvo ALIADO com magnitude > 0 adianta exatamente N posicoes",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("p1", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor p2 = make_actor("p2", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/45);
    CombatActor p3 = make_actor("p3", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor p4 = make_actor("p4", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/35);
    CombatActor e1 = make_actor("e1", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/30);
    InitiativeQueue queue({&caster, &p2, &p3, &p4, &e1});  // p4 no indice 3.

    Card einstein = delay_card("techmagic.delay.ally_fixed", /*magnitude=*/1);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &p4;
    ctx.queue = &queue;

    techMagic::execute(TriggerHook::OnCast, einstein, ctx);

    // p4 sai do indice 3 e vai pro indice 2 (3-1): [p1, p2, p4, p3, e1].
    REQUIRE(queue.order() == std::vector<CombatActor*>{&caster, &p2, &p4, &p3, &e1});
}

// ===== 20. Aliado com magnitude > 0 muito maior que a fila: clampa em cursor()+1, sem =====
// =====     ultrapassar pro slot do ator em resolucao (invariante anti turno-duplo) =========

TEST_CASE("techmagic delay: alvo ALIADO com magnitude > 0 muito maior que a fila clampa "
         "em cursor()+1, sem ultrapassar o ator em resolucao",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("p1", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor p2 = make_actor("p2", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/45);
    CombatActor p3 = make_actor("p3", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor p4 = make_actor("p4", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/35);
    InitiativeQueue queue({&caster, &p2, &p3, &p4});  // p4 no indice 3.

    Card einstein = delay_card("techmagic.delay.ally_overflow", /*magnitude=*/99);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &p4;
    ctx.queue = &queue;

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, einstein, ctx));

    // magnitude=99 pediria indice 3-99 (negativo); clampado em cursor()+1 = 1, NUNCA em 0
    // (indice 0 e o caster em resolucao - ultrapassar corromperia current()).
    REQUIRE(queue.order() == std::vector<CombatActor*>{&caster, &p4, &p2, &p3});
    REQUIRE(queue.current() == &caster);
    REQUIRE(queue.cursor() == 0);
}

// ===== 21. ZERO consumo de RNG no ramo ALIADO (mesma garantia do ramo inimigo, mas o =====
// =====     ramo aliado e codigo NOVO - cobre mutante que injeta sorteio acidental) =========

TEST_CASE("techmagic delay: ZERO consumo de RNG no ramo ALIADO (codigo novo do "
         "modo-aliado assimetrico)",
         "[domain][combat][techmagic][delay]") {
    CombatActor caster = make_actor("p1", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor p2 = make_actor("p2", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor p3 = make_actor("p3", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/30);
    InitiativeQueue queue({&caster, &p2, &p3});

    Card einstein = delay_card("techmagic.delay.ally_rng", /*magnitude=*/0);
    CountingRandom counting;
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &p3;
    ctx.queue = &queue;
    ctx.rng = &counting;  // presente, mas DelayAction nunca deve toca-lo.

    techMagic::execute(TriggerHook::OnCast, einstein, ctx);

    // A dilatacao rodou de fato (prova que 0-consumo nao e no-op mascarado)...
    REQUIRE(queue.order()[1] == &p3);
    // ...e mesmo assim nao houve sorteio.
    REQUIRE(counting.next_calls == 0);
    REQUIRE(counting.next_double_calls == 0);
}

// ===== 22. E2E FSM (anti turno-duplo, ramo ALIADO): castear Einstein num aliado atras na =====
// =====     fila - cada ator age EXATAMENTE 1x na rodada, e o aliado avancado age MAIS =======
// =====     CEDO do que agiria sem o Einstein (nunca 2x, nunca pulado) =======================

TEST_CASE("techmagic delay (via FSM): rodada completa com Einstein em ALIADO - cada ator "
         "age exatamente uma vez e o aliado avancado age mais cedo",
         "[domain][combat][techmagic][delay]") {
    // Ordem natural por SPD (sem Einstein): h(50), e0(40), p2(30) - h primeiro, e0 segundo,
    // p2 terceiro. h conjura Einstein em p2 (aliado) no seu proprio turno: p2 deve pular
    // pra agir logo em seguida (2o), empurrando e0 pro 3o.
    CombatActor caster = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/40);
    CombatActor p2 = make_actor("p2", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/30);

    auto reg = registry({delay_card("einstein_test", /*magnitude=*/0)});
    auto provider = provider_for(
        {{"h", {{0, {CombatAction::use_card("einstein_test", p2.id())}}}}});
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);
    CombatStateMachine sm({&caster, &e0, &p2}, provider, &reg, nullptr, &rng);

    std::vector<std::string> act_order;
    for (int i = 0; i < 3; ++i) {
        act_order.push_back(sm.queue().current()->id());
        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
    }

    // h age primeiro (adianta p2); p2 (aliado avancado) age em 2o - mais cedo do que o 3o
    // lugar que teria por SPD natural; e0 fecha a rodada. Cada id aparece exatamente 1x.
    REQUIRE(act_order == std::vector<std::string>{"h", "p2", "e0"});
}
