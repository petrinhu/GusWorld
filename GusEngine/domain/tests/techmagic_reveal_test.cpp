// techmagic_reveal_test.cpp
//
// Spec executavel (Catch2 v3) do executor techMagic (ADR-016 step 8, manifesto item 6):
// EffectKind::RevealIntent (John Dee/Black-Mirror, "Scrying"). Decisoes do lider
// 2026-07-15 (D1-D4, AMB-07):
//   D1 (i)  MUNDO (baus/passagens ocultas): STUB posse-only, ZERO codigo de combate aqui -
//           a query e "equipped_special_ids() contem uma carta com EffectKind::
//           RevealIntent?" (mesmo mecanismo do D1-ii abaixo). Nao ha sistema de ocultos no
//           overworld ainda; o teste #8 abaixo so verifica a AUSENCIA do bonus quando a
//           carta nao esta equipada (documenta o gate que o stub futuro reusara).
//   D1 (ii) SCAN DE COMBATE melhorado: com a carta equipada, resolve_scan revela TAMBEM
//           status ativos + posicao na fila + intent previsto do alvo escaneado. So dados
//           que JA existem no modelo (nenhum atributo oculto novo).
//   D2      Intent CAOTICO (Patch-Zero) retorna RUIDO, nunca revela os campos previstos -
//           preserva a one-way door do boss.
//   D3      Duracao 3, renovada via RE-DUMP na fronteira de rodada enquanto o buff Scrying
//           estiver ativo (CombatStateMachine::process_scrying_hooks); expira normal por
//           tick de duracao (mesmo motor de status de sempre).
//   D4      Mana kActiveManaCost (~6) - nao exercitado aqui (cartas de teste locais usam
//           mana_cost=0, mesmo padrao de techmagic_faraday_test.cpp/techmagic_godel_test.cpp;
//           o mana real da producao e coberto por master_cards_test.cpp).
//
// Fail-SOFT no brain ausente (assimetria DELIBERADA vs o Gambito manual/
// resolve_gambit_predict, que lanca std::out_of_range): ver doc-comment de
// techmagic.hpp::log_intent_for.
//
// TESTE-REI (mutation-alvo, obrigatorio): dois combates com a MESMA seed - um com Scrying
// ativo no caster, outro sem - produzem HP/dano/ordem-de-fila/contagem-de-RNG BYTE-
// IDENTICOS; so o log difere. Prova (a) 0 consumo de RNG e (b) que preview_intent extra
// nao muta nenhum brain (telegraph honesto).
//
// Cartas de teste montadas LOCALMENTE (id "techmagic.reveal.*"), NUNCA do registry de
// producao (MasterCards) - o teste do CATALOGO (dee em MasterCards::build_registry()) vive
// em master_cards_test.cpp.
//
// Cross-ref: gus/domain/combat/techmagic.hpp (log_intent_for/dump_reveal_intent);
//            combat_state_machine.cpp (process_scrying_hooks, resolve_scan aprimorado,
//            has_reveal_intent_equipped); master_cards.cpp (dee); docs/design/
//            roster-analogos/_EFEITOS-ESCOLHIDOS.md (AMB-07); ADR-016.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/enemy_brain.hpp"
#include "gus/domain/combat/initiative_queue.hpp"
#include "gus/domain/combat/random_source.hpp"
#include "gus/domain/combat/techmagic.hpp"

using namespace gus::domain::combat;

namespace {

CombatActor make_actor(const std::string& id, bool player_side, int hp = 100, int atk = 8,
                       int def = 0, int spd = 20, CardFamily family = CardFamily::Eletrico) {
    return CombatActor(id, id, hp, atk, def, spd, family, player_side);
}

// Carta "dee" de teste: Hibrida, Universal, TargetShape::Self, OnCast -> RevealIntent.
// mana_cost=0 (mesmo padrao de techmagic_faraday_test.cpp/techmagic_godel_test.cpp - o
// mana real da producao, kActiveManaCost, e coberto por master_cards_test.cpp).
Card reveal_card(const std::string& id, int duration = 3) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Universal;
    c.base_type = CardBaseType::Glifo;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = 0;
    c.target_shape = TargetShape::Self;
    c.tier = CardTier::Especial;
    c.category = CardCategory::Hibrida;
    c.effects = {EffectSpec{.trigger = TriggerHook::OnCast,
                            .kind = EffectKind::RevealIntent,
                            .duration = duration,
                            .status = StatusId::Scrying,
                            .stack_rule = StackRule::Refresh}};
    return c;
}

// Carta de dano PLANA (tier Comum default - isenta do gate 1x/batalha), pro TESTE-REI
// (precisa de uma acao que CONSOME rng_ pra comparar contagem).
Card damage_card(const std::string& id, int power) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Eletrico;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = 0;
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

// Brain stub configuravel: devolve o IntentPreview que o teste mandar (mesmo padrao de
// combat_inc3_test.cpp::FakeBrain).
class FakeBrain final : public IEnemyBrain {
public:
    explicit FakeBrain(IntentPreview preview) : preview_(std::move(preview)) {}
    IntentPreview preview_intent(const CombatState&, const CombatActor&) override {
        return preview_;
    }
    CombatAction decide_action(const CombatState&, const CombatActor&) override {
        return CombatAction::pass();
    }

private:
    IntentPreview preview_;
};

// Duplo de IRandomSource que CONTA as chamadas (mesmo padrao de techmagic_godel_test.cpp/
// techmagic_chain_test.cpp), pro TESTE-REI de determinismo.
class CountingRandom final : public IRandomSource {
public:
    explicit CountingRandom(double next_double = 0.5, int next_int = 50)
        : next_double_(next_double), next_int_(next_int) {}
    double next_double() override {
        ++double_calls;
        return next_double_;
    }
    int next(int max_value) override {
        ++int_calls;
        return max_value <= 0 ? 0 : std::min(next_int_, max_value - 1);
    }
    int int_calls = 0;
    int double_calls = 0;

private:
    double next_double_;
    int next_int_;
};

// ActionPlan por (id de ator, indice de rodada) -> sequencia de acoes; fora do plano ou
// esgotado, devolve Pass (mesmo padrao de techmagic_chain_test.cpp::provider_for).
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

int count_occurrences(const CombatStateMachine& sm, const std::string& needle) {
    int n = 0;
    for (const auto& e : sm.log())
        if (e.message.find(needle) != std::string::npos) ++n;
    return n;
}

bool log_has(const CombatStateMachine& sm, const std::string& needle) {
    return count_occurrences(sm, needle) > 0;
}

const StatusEffect* find_status(const CombatActor& a, StatusId id) {
    for (const auto& s : a.status_effects())
        if (s.id == id) return &s;
    return nullptr;
}

}  // namespace

// ===== 1. Ativa: cast aplica Scrying + dump inicial (1 linha por inimigo vivo) ============

TEST_CASE("techmagic reveal: cast aplica Scrying (dur 3) no caster e dumpa o intent do "
         "inimigo vivo",
         "[domain][combat][techmagic][reveal]") {
    CombatActor h = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    FakeBrain brain(IntentPreview{"e0", "attack", 7, TargetShape::Single, "h", false});
    std::unordered_map<std::string, IEnemyBrain*> brains{{e0.id(), &brain}};
    auto reg = registry({reveal_card("techmagic.reveal.dee")});

    auto provider = provider_for(
        {{"h", {{0, {CombatAction::use_card("techmagic.reveal.dee", h.id())}}}}});
    CombatStateMachine sm({&h, &e0}, provider, &reg, &brains, nullptr);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    const StatusEffect* scrying = find_status(h, StatusId::Scrying);
    REQUIRE(scrying != nullptr);
    REQUIRE(scrying->duration == 3);
    REQUIRE(scrying->stack_rule == StackRule::Refresh);

    REQUIRE(log_has(sm, "RevealIntent"));
    REQUIRE(log_has(sm, "e0"));
    REQUIRE(log_has(sm, "preve attack -> h (dano 7)"));
    REQUIRE(log_has(sm, "[LEGIVEL]"));

    // Item 3 (regra todo-efeito-loga): a LINHA de resumo do cast e uma mensagem COMPLETA e
    // especifica - silenciar SO essa linha (mutante) quebra este assert (as substrings
    // acima aparecem tambem nas linhas de dump por-inimigo; esta e exclusiva do cast).
    REQUIRE(log_has(sm, "h tavus-executa techmagic.reveal.dee: Scrying ativo (dur 3) - "
                       "varredura de intents inimigos."));
}

// ===== 1b. TESTE-REI (cast REAL via FSM): castar "dee" NAO consome NENHUM saque de RNG =====

TEST_CASE("techmagic reveal: TESTE-REI (cast real) - castar a carta via CombatAction "
         "use_card NAO consome nenhum saque de RNG (int_calls/double_calls intactos)",
         "[domain][combat][techmagic][reveal]") {
    // Complemento do TESTE-REI #9 (que cobre o RE-DUMP de fronteira de rodada, status
    // injetado direto): aqui o caminho PRINCIPAL - o CAST da carta via FSM - e exercitado
    // com um RNG que CONTA consumo. Um mutante que fizesse o RevealIntent sortear algo
    // (preview_intent, dump, buff) morreria: o RevealIntent e 100% read-only/deterministico.
    CombatActor h = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    FakeBrain brain(IntentPreview{"e0", "attack", 7, TargetShape::Single, "h", false});
    std::unordered_map<std::string, IEnemyBrain*> brains{{e0.id(), &brain}};
    auto reg = registry({reveal_card("techmagic.reveal.dee_rng")});

    auto provider = provider_for(
        {{"h", {{0, {CombatAction::use_card("techmagic.reveal.dee_rng", h.id())}}}}});
    CountingRandom rng;
    CombatStateMachine sm({&h, &e0}, provider, &reg, &brains, &rng);

    sm.begin_turn();
    const int int_before = rng.int_calls;        // snapshot IMEDIATAMENTE antes do cast.
    const int double_before = rng.double_calls;

    sm.run_active_turn_to_end();                  // o turno do cast (h joga "dee").

    // O cast do RevealIntent (buff Scrying + dump do intent do inimigo) e read-only: NENHUM
    // saque de RNG a mais, byte-identico ao estado pre-cast.
    REQUIRE(rng.int_calls == int_before);
    REQUIRE(rng.double_calls == double_before);

    // Prova que o cast REALMENTE rodou (nao e um verde vazio): o buff foi aplicado e o dump
    // saiu no log.
    REQUIRE(find_status(h, StatusId::Scrying) != nullptr);
    REQUIRE(log_has(sm, "preve attack -> h (dano 7)"));
}

// ===== 2. No-op: zero inimigos vivos loga dissipacao, nao lanca ===========================

TEST_CASE("techmagic reveal: zero inimigos vivos loga dissipacao (no-op), nao lanca",
         "[domain][combat][techmagic][reveal]") {
    CombatActor h = make_actor("h", true);
    const std::vector<CombatActor*> combatants = {&h};  // sem inimigo nenhum na lista.

    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &h;
    ctx.combatants = &combatants;
    ctx.log = &log;

    REQUIRE_NOTHROW(techMagic::dump_reveal_intent(ctx));
    REQUIRE(log.size() == 1);
    REQUIRE(log.front().message.find("nenhum sinal") != std::string::npos);
}

// ===== 3. Brain ausente: FAIL-SOFT, loga "sem sinal" e segue (NAO lanca) ==================

TEST_CASE("techmagic reveal: inimigo sem brain no registry -> FAIL-SOFT (sem sinal), "
         "assimetria deliberada vs o Gambito manual (que lanca)",
         "[domain][combat][techmagic][reveal]") {
    CombatActor h = make_actor("h", true);
    CombatActor e0 = make_actor("e0", false);
    const std::vector<CombatActor*> combatants = {&h, &e0};

    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &h;
    ctx.combatants = &combatants;
    ctx.log = &log;
    ctx.brain_registry = nullptr;  // sem registry algum.

    REQUIRE_NOTHROW(techMagic::dump_reveal_intent(ctx));
    REQUIRE(log.size() == 1);
    REQUIRE(log.front().message.find("sem sinal") != std::string::npos);
    REQUIRE(log.front().message.find("e0") != std::string::npos);
}

TEST_CASE("techmagic reveal: registry presente mas id do inimigo AUSENTE dele -> tambem "
         "FAIL-SOFT",
         "[domain][combat][techmagic][reveal]") {
    CombatActor h = make_actor("h", true);
    CombatActor e0 = make_actor("e0", false);
    const std::vector<CombatActor*> combatants = {&h, &e0};
    const std::unordered_map<std::string, IEnemyBrain*> empty_brains;

    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &h;
    ctx.combatants = &combatants;
    ctx.log = &log;
    ctx.brain_registry = &empty_brains;

    REQUIRE_NOTHROW(techMagic::dump_reveal_intent(ctx));
    REQUIRE(log.size() == 1);
    REQUIRE(log.front().message.find("sem sinal") != std::string::npos);
}

// ===== 4. D2 - intent CAOTICO retorna RUIDO, nunca revela os campos previstos =============

TEST_CASE("techmagic reveal: D2 - intent CAOTICO (Patch-Zero) loga ruido, NUNCA revela "
         "acao/alvo/dano previstos",
         "[domain][combat][techmagic][reveal]") {
    CombatActor h = make_actor("h", true);
    CombatActor boss = make_actor("boss", false);
    const std::vector<CombatActor*> combatants = {&h, &boss};

    FakeBrain brain(IntentPreview{"boss", "definitely_hidden_action", 999, TargetShape::Single,
                                  "definitely_hidden_target", /*is_chaotic=*/true});
    std::unordered_map<std::string, IEnemyBrain*> brains{{boss.id(), &brain}};

    InitiativeQueue queue({&h, &boss});
    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &h;
    ctx.combatants = &combatants;
    ctx.log = &log;
    ctx.brain_registry = &brains;
    ctx.queue = &queue;

    REQUIRE_NOTHROW(techMagic::dump_reveal_intent(ctx));
    REQUIRE(log.size() == 1);
    const std::string& msg = log.front().message;
    REQUIRE(msg.find("CAOTICO") != std::string::npos);
    // Preserva a one-way door do boss: NENHUM campo previsto vaza no log.
    REQUIRE(msg.find("definitely_hidden_action") == std::string::npos);
    REQUIRE(msg.find("definitely_hidden_target") == std::string::npos);
    REQUIRE(msg.find("999") == std::string::npos);
}

// ===== 5. Guards fail-fast: caster/combatants/queue nulos lancam logic_error ===============

TEST_CASE("techmagic reveal: guards fail-fast (caster/combatants nulos lancam logic_error; "
         "queue nulo SO lanca quando ha brain de fato pra consultar)",
         "[domain][combat][techmagic][reveal]") {
    CombatActor h = make_actor("h", true);
    CombatActor e0 = make_actor("e0", false);
    const std::vector<CombatActor*> combatants = {&h, &e0};
    InitiativeQueue queue({&h, &e0});
    FakeBrain brain(IntentPreview{"e0", "attack", 1, TargetShape::Single, "h", false});
    std::unordered_map<std::string, IEnemyBrain*> brains{{e0.id(), &brain}};

    {
        techMagic::TechMagicContext ctx;
        ctx.combatants = &combatants;
        REQUIRE_THROWS_AS(techMagic::dump_reveal_intent(ctx), std::logic_error);
    }
    {
        techMagic::TechMagicContext ctx;
        ctx.caster = &h;
        REQUIRE_THROWS_AS(techMagic::dump_reveal_intent(ctx), std::logic_error);
    }
    {
        // caster nulo: lanca mesmo sem brain nenhum (guard mais externo).
        techMagic::TechMagicContext ctx;
        REQUIRE_THROWS_AS(techMagic::log_intent_for(e0, ctx), std::logic_error);
    }
    {
        // caster presente, brain ENCONTRADO, mas queue nulo: precisa montar CombatState
        // pro brain.preview_intent - lanca (bug de call site).
        techMagic::TechMagicContext ctx;
        ctx.caster = &h;
        ctx.brain_registry = &brains;
        REQUIRE_THROWS_AS(techMagic::log_intent_for(e0, ctx), std::logic_error);
    }
    {
        techMagic::TechMagicContext ctx;
        ctx.queue = &queue;
        REQUIRE_THROWS_AS(techMagic::log_intent_for(e0, ctx), std::logic_error);
    }
}

// ===== 6. D3 - re-dump na fronteira de rodada enquanto Scrying estiver ativo; expira ======
// =====    normal por tick de duracao (3 redumps de rodada + 1 dump inicial = 4 no total) ==

TEST_CASE("techmagic reveal: D3 - re-dump na fronteira de rodada enquanto Scrying ativo; "
         "expira apos 3 tick do proprio turno do caster (para de re-dumpar)",
         "[domain][combat][techmagic][reveal]") {
    CombatActor h = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    FakeBrain brain(IntentPreview{"e0", "attack", 7, TargetShape::Single, "h", false});
    std::unordered_map<std::string, IEnemyBrain*> brains{{e0.id(), &brain}};
    auto reg = registry({reveal_card("techmagic.reveal.dee2")});

    auto provider = provider_for(
        {{"h", {{0, {CombatAction::use_card("techmagic.reveal.dee2", h.id())}}}}});
    CombatStateMachine sm({&h, &e0}, provider, &reg, &brains, nullptr);

    // 8 turnos = 4 rodadas completas (h, e0 cada uma): cast na rodada 0 + 3 fronteiras de
    // rodada com Scrying ainda ativo (dur 3->2->1->0), a 4a fronteira ja sem o buff.
    for (int i = 0; i < 8; ++i) {
        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
    }

    // Scrying expirou (dur chegou a 0 e foi removido no TurnEnd do proprio turno do caster).
    REQUIRE(find_status(h, StatusId::Scrying) == nullptr);

    // 1 dump inicial (OnCast) + 3 re-dumps de fronteira de rodada = 4 linhas totais.
    REQUIRE(count_occurrences(sm, "preve attack -> h (dano 7)") == 4);
}

TEST_CASE("techmagic reveal: sem Scrying ativo, a fronteira de rodada NAO redumpa nada",
         "[domain][combat][techmagic][reveal]") {
    CombatActor h = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    FakeBrain brain(IntentPreview{"e0", "attack", 7, TargetShape::Single, "h", false});
    std::unordered_map<std::string, IEnemyBrain*> brains{{e0.id(), &brain}};

    auto provider = provider_for({});  // ninguem faz nada alem de Pass.
    CombatStateMachine sm({&h, &e0}, provider, nullptr, &brains, nullptr);

    for (int i = 0; i < 4; ++i) {
        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
    }

    REQUIRE(count_occurrences(sm, "RevealIntent") == 0);
}

// ===== 7. Scan aprimorado (D1-ii): status ativos + posicao na fila + intent previsto ======

TEST_CASE("techmagic reveal: Scan aprimorado - scanner com a carta EQUIPADA revela status "
         "ativos + posicao na fila + intent previsto do alvo",
         "[domain][combat][techmagic][reveal]") {
    CombatActor h = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 100, /*atk=*/0, /*def=*/0, /*spd=*/10);
    e0.add_status(StatusEffect{StatusId::Stun, 0, 2, StackRule::Replace, CardFamily::Eletrico});

    FakeBrain brain(IntentPreview{"e0", "attack", 5, TargetShape::Single, "h", false});
    std::unordered_map<std::string, IEnemyBrain*> brains{{e0.id(), &brain}};
    auto reg = registry({reveal_card("techmagic.reveal.dee3")});
    h.set_equipped_special_ids({"techmagic.reveal.dee3"});

    auto provider = provider_for({{"h", {{0, {CombatAction::scan(e0.id())}}}}});
    CombatStateMachine sm({&h, &e0}, provider, &reg, &brains, nullptr);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(log_has(sm, "HP " + std::to_string(e0.hp())));  // Scan base preservado.
    REQUIRE(log_has(sm, "status ativos"));
    REQUIRE(log_has(sm, "Stun"));
    REQUIRE(log_has(sm, "na fila de iniciativa"));
    REQUIRE(log_has(sm, "preve attack -> h (dano 5)"));

    // Item 2: assert NUMERICO da posicao (nao so a substring "posicao"). h (spd 50, party)
    // abre a rodada no slot 0; e0 (spd 10, inimigo) fica no slot 1. Sem o valor exato, um
    // mutante `index=999` sobreviveria. Casa com queue().index_of(e0).
    const int expected_index = sm.queue().index_of(&e0);
    REQUIRE(expected_index == 1);
    REQUIRE(log_has(sm, "posicao " + std::to_string(expected_index)));
}

// ===== 8. Stub posse-only (D1-i): SEM a carta equipada, o Scan NAO ganha bonus nenhum ======

TEST_CASE("techmagic reveal: sem a carta equipada, Scan fica no comportamento BASE (sem "
         "bonus) - documenta o gate que o stub de mundo (D1-i) reusara",
         "[domain][combat][techmagic][reveal]") {
    CombatActor h = make_actor("h", true, 100, /*atk=*/0, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 100, /*atk=*/0, /*def=*/0, /*spd=*/10);

    FakeBrain brain(IntentPreview{"e0", "attack", 5, TargetShape::Single, "h", false});
    std::unordered_map<std::string, IEnemyBrain*> brains{{e0.id(), &brain}};
    // Registry TEM a carta, mas o scanner NAO a equipou (equipped_special_ids vazio).
    auto reg = registry({reveal_card("techmagic.reveal.dee4")});

    auto provider = provider_for({{"h", {{0, {CombatAction::scan(e0.id())}}}}});
    CombatStateMachine sm({&h, &e0}, provider, &reg, &brains, nullptr);

    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(log_has(sm, "HP " + std::to_string(e0.hp())));  // Scan base ainda roda.
    REQUIRE_FALSE(log_has(sm, "RevealIntent"));
    REQUIRE_FALSE(log_has(sm, "status ativos"));
    REQUIRE_FALSE(log_has(sm, "posicao"));
}

// ===== 9. TESTE-REI: com e sem Scrying, HP/ordem/contagem de RNG BYTE-IDENTICOS ============

TEST_CASE("techmagic reveal: TESTE-REI - com e sem Scrying ativo, HP/ordem-de-fila/"
         "contagem-de-RNG ficam BYTE-IDENTICOS ao longo de 4 rodadas; so o log difere",
         "[domain][combat][techmagic][reveal]") {
    struct Result {
        int h_hp = 0;
        int e_hp = 0;
        std::vector<std::string> order_ids;
        int int_calls = 0;
        int double_calls = 0;
        int reveal_lines = 0;
    };

    auto run = [](bool with_scrying) {
        CombatActor h = make_actor("h", true, 500, /*atk=*/8, /*def=*/0, /*spd=*/50);
        CombatActor e = make_actor("e", false, 500, /*atk=*/6, /*def=*/0, /*spd=*/10,
                                   CardFamily::Bioquimico);
        FakeBrain brain(IntentPreview{"e", "attack", 5, TargetShape::Single, "h", false});
        std::unordered_map<std::string, IEnemyBrain*> brains{{e.id(), &brain}};

        if (with_scrying)
            h.add_status(StatusEffect{StatusId::Scrying, 0, 3, StackRule::Refresh,
                                      CardFamily::Universal});

        auto reg = registry({damage_card("techmagic.reveal.dmg_h", 10),
                             damage_card("techmagic.reveal.dmg_e", 6)});
        auto provider = provider_for({
            {"h",
             {{0, {CombatAction::use_card("techmagic.reveal.dmg_h", e.id())}},
              {1, {CombatAction::use_card("techmagic.reveal.dmg_h", e.id())}},
              {2, {CombatAction::use_card("techmagic.reveal.dmg_h", e.id())}},
              {3, {CombatAction::use_card("techmagic.reveal.dmg_h", e.id())}}}},
            {"e",
             {{0, {CombatAction::use_card("techmagic.reveal.dmg_e", h.id())}},
              {1, {CombatAction::use_card("techmagic.reveal.dmg_e", h.id())}},
              {2, {CombatAction::use_card("techmagic.reveal.dmg_e", h.id())}},
              {3, {CombatAction::use_card("techmagic.reveal.dmg_e", h.id())}}}},
        });

        CountingRandom rng;
        CombatStateMachine sm({&h, &e}, provider, &reg, &brains, &rng);

        for (int i = 0; i < 8; ++i) {  // 4 rodadas completas.
            sm.begin_turn();
            sm.run_active_turn_to_end();
            sm.advance_to_next_actor();
        }

        Result r;
        r.h_hp = h.hp();
        r.e_hp = e.hp();
        for (const CombatActor* a : sm.queue().order()) r.order_ids.push_back(a->id());
        r.int_calls = rng.int_calls;
        r.double_calls = rng.double_calls;
        r.reveal_lines = count_occurrences(sm, "RevealIntent");
        return r;
    };

    const Result without_scrying = run(/*with_scrying=*/false);
    const Result with_scrying = run(/*with_scrying=*/true);

    // O estado de combate (HP/dano/ordem-de-fila/contagem-de-RNG) e IDENTICO nos dois casos:
    // RevealIntent e 100% read-only, 0 consumo de RNG, nao muta a fila.
    REQUIRE(without_scrying.h_hp == with_scrying.h_hp);
    REQUIRE(without_scrying.e_hp == with_scrying.e_hp);
    REQUIRE(without_scrying.order_ids == with_scrying.order_ids);
    REQUIRE(without_scrying.int_calls == with_scrying.int_calls);
    REQUIRE(without_scrying.double_calls == with_scrying.double_calls);

    // Prova que a comparacao nao e trivial (0==0): RNG foi de fato consumido nas cartas.
    REQUIRE(without_scrying.int_calls > 0);
    REQUIRE(without_scrying.double_calls > 0);

    // So o LOG difere: sem Scrying, zero linhas de RevealIntent; com, pelo menos 1.
    REQUIRE(without_scrying.reveal_lines == 0);
    REQUIRE(with_scrying.reveal_lines > 0);
}
