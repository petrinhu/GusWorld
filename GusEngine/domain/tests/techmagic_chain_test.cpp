// techmagic_chain_test.cpp
//
// Spec executavel (Catch2 v3) do executor techMagic (ADR-016, MVP step 6):
// EffectKind::ChainDamage (Tesla). Regras (decisoes do lider, fechadas no brief):
//   - 3 alvos no total = primario (dano-base, fora do handler) + magnitude saltos.
//   - Decaimento MULTIPLICATIVO: salto k (1-indexado) = lround(ctx.damage * (percent/100)^k).
//   - Alvos-salto = proximos inimigos VIVOS na ORDEM da fila, lado OPOSTO ao caster,
//     EXCLUINDO o primario. Ate magnitude; se ha menos, salta em menos.
//   - Para quando o salto arredonda pra <=0.
//   - Primario imune (ctx.damage 0) => no-op (a cadeia escala do dano do primario).
//   - take_damage PURO: nao redispara hooks, nao entra no ledger (round_hits_).
//   - 0 consumo de RNG (ChainDamage nunca sorteia).
//
// Cartas de teste montadas localmente (id "techmagic.chain.*"), NUNCA do registry de
// producao (MasterCards) - o teste do catalogo vive em master_cards_test.cpp.
//
// Cross-ref: gus/domain/combat/techmagic.hpp (TechMagicContext.combatants);
//            combat_state_machine.cpp (wiring OnCast: combatants + damage);
//            docs/design/mecanicas/cartas-technomagik.md; ADR-016.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <initializer_list>
#include <memory>
#include <map>
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

CombatActor make_actor(const std::string& id, bool player_side, int hp, int atk = 0,
                       int def = 0, int spd = 20) {
    return CombatActor(id, id, hp, atk, def, spd, CardFamily::Eletrico, player_side);
}

// Carta Tesla de teste: Ativa, OnCast -> ChainDamage. magnitude = numero de saltos,
// percent = retencao por salto. power configuravel (dano-base do primario); 0 nos casos
// que injetam ctx.damage direto (chamada isolada do executor).
Card chain_card(const std::string& id, int magnitude, int percent, int power = 0) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Eletrico;
    c.base_type = CardBaseType::Glifo;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = power;
    c.target_shape = TargetShape::Single;
    c.tier = CardTier::Especial;
    c.category = CardCategory::Ativa;
    c.effects = {EffectSpec{.trigger = TriggerHook::OnCast,
                            .kind = EffectKind::ChainDamage,
                            .magnitude = magnitude,
                            .percent = percent}};
    return c;
}

// Newton de teste (Reflect 30% OnDamageReceived): so pra provar que o salto PURO NAO
// redispara a passiva de dano-recebido do alvo secundario.
Card reflect_card(const std::string& id, int percent) {
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

// ===== 1. Cadeia completa: 3 inimigos vivos, decaimento multiplicativo 62%/salto =====

TEST_CASE("techmagic chain: cadeia completa D=10 -> saltos 6 e 4 (lround do decaimento "
         "multiplicativo 62%)",
         "[domain][combat][techmagic][chain]") {
    CombatActor caster = make_actor("h", true, 100);
    CombatActor primary = make_actor("e0", false, 300);
    CombatActor e1 = make_actor("e1", false, 300);
    CombatActor e2 = make_actor("e2", false, 300);
    const std::vector<CombatActor*> roster = {&caster, &primary, &e1, &e2};

    Card tesla = chain_card("techmagic.chain.full", /*magnitude=*/2, /*percent=*/62);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &primary;
    ctx.combatants = &roster;
    ctx.damage = 10;  // D = dano efetivo no primario.

    techMagic::execute(TriggerHook::OnCast, tesla, ctx);

    // Salto 1: lround(10 * 0.62) = lround(6.2) = 6. Salto 2: lround(10 * 0.62^2) =
    // lround(3.844) = 4.
    REQUIRE(e1.hp() == 300 - 6);
    REQUIRE(e2.hp() == 300 - 4);
    REQUIRE(primary.hp() == 300);  // primario nunca recebe salto extra.
}

TEST_CASE("techmagic chain: cadeia completa D=8 -> saltos 5 e 3",
         "[domain][combat][techmagic][chain]") {
    CombatActor caster = make_actor("h", true, 100);
    CombatActor primary = make_actor("e0", false, 300);
    CombatActor e1 = make_actor("e1", false, 300);
    CombatActor e2 = make_actor("e2", false, 300);
    const std::vector<CombatActor*> roster = {&caster, &primary, &e1, &e2};

    Card tesla = chain_card("techmagic.chain.full8", /*magnitude=*/2, /*percent=*/62);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &primary;
    ctx.combatants = &roster;
    ctx.damage = 8;

    techMagic::execute(TriggerHook::OnCast, tesla, ctx);

    // Salto 1: lround(8 * 0.62) = lround(4.96) = 5. Salto 2: lround(8 * 0.62^2) =
    // lround(3.0752) = 3.
    REQUIRE(e1.hp() == 300 - 5);
    REQUIRE(e2.hp() == 300 - 3);
}

// ===== 2. Primario imune (ctx.damage 0): nenhum salto, secundarios intactos =====

TEST_CASE("techmagic chain: primario imune (ctx.damage 0) nao propaga a cadeia",
         "[domain][combat][techmagic][chain]") {
    CombatActor caster = make_actor("h", true, 100);
    CombatActor primary = make_actor("e0", false, 300);
    CombatActor e1 = make_actor("e1", false, 300);
    CombatActor e2 = make_actor("e2", false, 300);
    const std::vector<CombatActor*> roster = {&caster, &primary, &e1, &e2};

    Card tesla = chain_card("techmagic.chain.immune", /*magnitude=*/2, /*percent=*/62);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &primary;
    ctx.combatants = &roster;
    ctx.damage = 0;  // primario imune / dano-base 0.

    techMagic::execute(TriggerHook::OnCast, tesla, ctx);

    REQUIRE(e1.hp() == 300);
    REQUIRE(e2.hp() == 300);
}

// ===== 3. Menos inimigos que magnitude: so 1 salto, sem crash =====

TEST_CASE("techmagic chain: com so 1 secundario vivo, salta 1 vez (magnitude 2) sem crash",
         "[domain][combat][techmagic][chain]") {
    CombatActor caster = make_actor("h", true, 100);
    CombatActor primary = make_actor("e0", false, 300);
    CombatActor e1 = make_actor("e1", false, 300);
    const std::vector<CombatActor*> roster = {&caster, &primary, &e1};

    Card tesla = chain_card("techmagic.chain.short", /*magnitude=*/2, /*percent=*/62);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &primary;
    ctx.combatants = &roster;
    ctx.damage = 10;

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, tesla, ctx));
    REQUIRE(e1.hp() == 300 - 6);  // so o salto 1 (lround(6.2)=6); nao ha salto 2.
}

// ===== 4. Inimigos mortos sao pulados na coleta; o proximo vivo recebe =====

TEST_CASE("techmagic chain: inimigo morto e pulado; os proximos vivos recebem os saltos "
         "1 e 2 (a coleta nao 'gasta' salto num cadaver)",
         "[domain][combat][techmagic][chain]") {
    CombatActor caster = make_actor("h", true, 100);
    CombatActor primary = make_actor("e0", false, 300);
    CombatActor e_dead = make_actor("e_dead", false, 300);
    CombatActor e1 = make_actor("e1", false, 300);
    CombatActor e2 = make_actor("e2", false, 300);
    e_dead.take_damage(300);  // hp 0 -> nao esta vivo.
    REQUIRE_FALSE(e_dead.is_alive());
    // Ordem da fila: e_dead ANTES dos vivos, pra provar que e' pulado e e1 pega o salto 1.
    const std::vector<CombatActor*> roster = {&caster, &primary, &e_dead, &e1, &e2};

    Card tesla = chain_card("techmagic.chain.dead", /*magnitude=*/2, /*percent=*/62);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &primary;
    ctx.combatants = &roster;
    ctx.damage = 10;

    techMagic::execute(TriggerHook::OnCast, tesla, ctx);

    REQUIRE(e_dead.hp() == 0);       // cadaver nao recebe salto.
    REQUIRE(e1.hp() == 300 - 6);     // salto 1 (nao "queimado" no cadaver).
    REQUIRE(e2.hp() == 300 - 4);     // salto 2.
}

// ===== 5. O primario (counterpart) NAO recebe salto extra, mesmo estando no roster =====

TEST_CASE("techmagic chain: o alvo primario (counterpart) e excluido dos saltos mesmo "
         "estando no roster",
         "[domain][combat][techmagic][chain]") {
    CombatActor caster = make_actor("h", true, 100);
    CombatActor primary = make_actor("e0", false, 300);
    CombatActor e1 = make_actor("e1", false, 300);
    // So o primario + 1 secundario: se o primario NAO fosse excluido, ele levaria um salto.
    const std::vector<CombatActor*> roster = {&caster, &primary, &e1};

    Card tesla = chain_card("techmagic.chain.excl", /*magnitude=*/2, /*percent=*/62);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &primary;
    ctx.combatants = &roster;
    ctx.damage = 10;

    techMagic::execute(TriggerHook::OnCast, tesla, ctx);

    REQUIRE(primary.hp() == 300);  // handler nao toca no primario.
    REQUIRE(e1.hp() == 300 - 6);   // e1 pega o salto 1, nao o salto 2.
}

// ===== 6. take_damage PURO via a FSM: o salto NAO entra no ledger nem redispara o =====
// =====    Reflect equipado no alvo secundario ======================================

TEST_CASE("techmagic chain (via FSM): o salto e PURO - nao entra no round_hits e nao "
         "redispara a passiva Reflect do alvo secundario",
         "[domain][combat][techmagic][chain]") {
    CombatActor caster = make_actor("h", true, 100, /*atk=*/12, /*def=*/0, /*spd=*/50);
    CombatActor e0 = make_actor("e0", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/20);
    CombatActor e1 = make_actor("e1", false, 300, /*atk=*/0, /*def=*/0, /*spd=*/10);
    e1.set_equipped_special_ids({"newton_e1"});  // Reflect OnDamageReceived.

    auto reg = registry({chain_card("tesla_test", /*magnitude=*/2, /*percent=*/62, /*power=*/8),
                         reflect_card("newton_e1", /*percent=*/30)});

    auto provider = provider_for(
        {{"h", {{0, {CombatAction::use_card("tesla_test", e0.id())}}}}});
    // COMUM sem variancia (roll 99 != fumble/crit; next_double 0.5 = variancia 0): dano-base
    // deterministico > 0, o suficiente pra cadeia propagar.
    FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);
    CombatStateMachine sm({&caster, &e0, &e1}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    sm.run_active_turn_to_end();  // turno de h (spd mais alta): base em e0 + cadeia salta em e1.

    // A cadeia chegou em e1 (salto landed via FSM).
    REQUIRE(e1.hp() < 300);
    // O salto foi PURO: NAO redisparou o Reflect de e1 -> o caster nao levou dano de volta.
    REQUIRE(caster.hp() == caster.max_hp());
    // O salto NAO entrou no ledger: so o hit-base em e0 (target Single) esta la.
    REQUIRE(sm.round_hits().size() == 1);
    REQUIRE(sm.round_hits().front().target == &e0);
}

// ===== 7. Determinismo: ChainDamage nao consome RNG (0 next / 0 next_double) =====

TEST_CASE("techmagic chain: ZERO consumo de RNG (o handler nunca sorteia)",
         "[domain][combat][techmagic][chain]") {
    CombatActor caster = make_actor("h", true, 100);
    CombatActor primary = make_actor("e0", false, 300);
    CombatActor e1 = make_actor("e1", false, 300);
    CombatActor e2 = make_actor("e2", false, 300);
    const std::vector<CombatActor*> roster = {&caster, &primary, &e1, &e2};

    Card tesla = chain_card("techmagic.chain.rng", /*magnitude=*/2, /*percent=*/62);
    CountingRandom counting;
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &primary;
    ctx.combatants = &roster;
    ctx.damage = 10;
    ctx.rng = &counting;  // presente, mas ChainDamage nunca deve toca-lo.

    techMagic::execute(TriggerHook::OnCast, tesla, ctx);

    // A cadeia rodou de fato (prova que 0-consumo nao e no-op mascarado)...
    REQUIRE(e1.hp() == 300 - 6);
    REQUIRE(e2.hp() == 300 - 4);
    // ...e mesmo assim nao houve sorteio.
    REQUIRE(counting.next_calls == 0);
    REQUIRE(counting.next_double_calls == 0);
}

// ===== 8. Guards fail-fast: combatants nulo e counterpart nulo lancam logic_error =====

TEST_CASE("techmagic chain: guards fail-fast (combatants nulo e counterpart nulo lancam "
         "logic_error)",
         "[domain][combat][techmagic][chain]") {
    CombatActor caster = make_actor("h", true, 100);
    CombatActor primary = make_actor("e0", false, 300);
    const std::vector<CombatActor*> roster = {&caster, &primary};

    Card tesla = chain_card("techmagic.chain.guard", /*magnitude=*/2, /*percent=*/62);

    // combatants == nullptr -> bug de call site (a FSM sempre injeta &queue_.order()).
    {
        techMagic::TechMagicContext ctx;
        ctx.caster = &caster;
        ctx.counterpart = &primary;
        ctx.combatants = nullptr;
        ctx.damage = 10;
        REQUIRE_THROWS_AS(techMagic::execute(TriggerHook::OnCast, tesla, ctx),
                          std::logic_error);
    }

    // counterpart == nullptr -> OnCast sempre tem alvo primario.
    {
        techMagic::TechMagicContext ctx;
        ctx.caster = &caster;
        ctx.counterpart = nullptr;
        ctx.combatants = &roster;
        ctx.damage = 10;
        REQUIRE_THROWS_AS(techMagic::execute(TriggerHook::OnCast, tesla, ctx),
                          std::logic_error);
    }
}

// ===== 9. Mutation testing (QA adversarial 2026-07-15): saltos que arredondam pra 0 no =====
// =====    MEIO da cadeia (magnitude 3) param de vez - nao "queimam" jump_targets vazios =====
// =====    num cadaver metaforico de dano 0. Mata o mutante `jump < 0` (deveria ser =====
// =====    `jump <= 0`): com esse mutante, e2/e3 continuam recebendo take_damage(0) (HP =====
// =====    nao muda - indistinguivel so por HP) e GANHAM entrada de log espuria; so o =====
// =====    log.size() denuncia a diferenca. =====

TEST_CASE("techmagic chain: salto que arredonda pra 0 no meio da cadeia (magnitude 3) para "
         "de vez - nao gera log espurio nos alvos seguintes",
         "[domain][combat][techmagic][chain][mutation]") {
    CombatActor caster = make_actor("h", true, 100);
    CombatActor primary = make_actor("e0", false, 300);
    CombatActor e1 = make_actor("e1", false, 300);
    CombatActor e2 = make_actor("e2", false, 300);
    CombatActor e3 = make_actor("e3", false, 300);
    const std::vector<CombatActor*> roster = {&caster, &primary, &e1, &e2, &e3};

    // D=3, percent=25%: salto1 = lround(3*0.25) = lround(0.75) = 1 (>0, propaga). salto2 =
    // lround(3*0.0625) = lround(0.1875) = 0 -> a cadeia PARA aqui (o mutante `jump < 0` nao
    // pararia, e continuaria "gastando" e2 e e3 com take_damage(0) espurio).
    Card tesla = chain_card("techmagic.chain.zero_mid", /*magnitude=*/3, /*percent=*/25);
    std::vector<CombatLogEntry> log;
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &primary;
    ctx.combatants = &roster;
    ctx.damage = 3;
    ctx.log = &log;

    techMagic::execute(TriggerHook::OnCast, tesla, ctx);

    REQUIRE(e1.hp() == 300 - 1);  // salto 1: unico que propaga.
    REQUIRE(e2.hp() == 300);      // salto 2 arredondou pra 0: cadeia para, sem take_damage(0).
    REQUIRE(e3.hp() == 300);      // nunca alcancado - a cadeia ja parou no salto 2.
    // O oraculo real: exatamente 1 entrada de log (so o salto 1). Com o mutante `jump < 0`,
    // e2 e e3 tambem gerariam log (take_damage(0) e chamado, HP fica igual mas o log denuncia).
    REQUIRE(log.size() == 1);
}

// ===== 10. Mutation testing: com magnitude=2 mas 3 inimigos vivos na fila, so os 2 =====
// =====     primeiros (na ordem) recebem salto - o terceiro fica intocado. Mata o =====
// =====     mutante `jump_targets.size() > max_jumps` (deveria ser `>=`), que coletaria =====
// =====     magnitude+1 alvos-salto. =====

TEST_CASE("techmagic chain: com magnitude 2 mas 3 inimigos vivos na fila, o terceiro fica "
         "de fora (a cadeia nao excede magnitude)",
         "[domain][combat][techmagic][chain][mutation]") {
    CombatActor caster = make_actor("h", true, 100);
    CombatActor primary = make_actor("e0", false, 300);
    CombatActor e1 = make_actor("e1", false, 300);
    CombatActor e2 = make_actor("e2", false, 300);
    CombatActor e3 = make_actor("e3", false, 300);
    const std::vector<CombatActor*> roster = {&caster, &primary, &e1, &e2, &e3};

    Card tesla = chain_card("techmagic.chain.off_by_one", /*magnitude=*/2, /*percent=*/62);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &primary;
    ctx.combatants = &roster;
    ctx.damage = 10;

    techMagic::execute(TriggerHook::OnCast, tesla, ctx);

    REQUIRE(e1.hp() == 300 - 6);  // salto 1.
    REQUIRE(e2.hp() == 300 - 4);  // salto 2 - ultimo (magnitude=2).
    REQUIRE(e3.hp() == 300);      // NUNCA deveria ser tocado (so 2 saltos declarados).
}

// ===== 11. Mutation testing: um ALIADO (nao-caster) no roster nunca recebe salto, mesmo =====
// =====     posicionado ANTES dos inimigos de verdade na fila. Mata o mutante que so =====
// =====     exclui `a == ctx.caster` em vez do LADO inteiro (um aliado que nao e o caster =====
// =====     escaparia do filtro e "roubaria" o salto de um inimigo real). =====

TEST_CASE("techmagic chain: um aliado (nao-caster) no roster jamais recebe salto, mesmo "
         "posicionado antes dos inimigos na fila",
         "[domain][combat][techmagic][chain][mutation]") {
    CombatActor caster = make_actor("h", true, 100);
    CombatActor primary = make_actor("e0", false, 300);
    CombatActor ally = make_actor("h2", true, 100);  // aliado do caster, NAO e o caster.
    CombatActor e1 = make_actor("e1", false, 300);
    CombatActor e2 = make_actor("e2", false, 300);
    // Ordem deliberada: o aliado vem LOGO APOS o primario, ANTES dos inimigos de verdade -
    // se o filtro de lado falhasse (so excluisse o caster por identidade), o aliado
    // "roubaria" o salto 1 de e1.
    const std::vector<CombatActor*> roster = {&caster, &primary, &ally, &e1, &e2};

    Card tesla = chain_card("techmagic.chain.ally_guard", /*magnitude=*/2, /*percent=*/62);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &primary;
    ctx.combatants = &roster;
    ctx.damage = 10;

    techMagic::execute(TriggerHook::OnCast, tesla, ctx);

    REQUIRE(ally.hp() == 100);    // aliado jamais e alvo da cadeia (mesmo lado do caster).
    REQUIRE(e1.hp() == 300 - 6);  // salto 1 vai pro inimigo real, nao pro aliado.
    REQUIRE(e2.hp() == 300 - 4);  // salto 2.
}
