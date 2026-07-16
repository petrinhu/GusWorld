// master_cards_test.cpp
//
// Spec executavel (Catch2 v3) do catalogo data-driven das cartas ESPECIAIS dos mestres
// suportadas pelo executor techMagic (ADR-016, item TECHMAGIC-EXECUTOR / MVP steps 4-8 +
// manifesto itens 5-6): volta, newton, pythagoras, mandelbrot, ada, godel, faraday, euler,
// turing, menger, tesla, einstein, planck, dee. POCO puro, ZERO Qt.
//
// Cobre: (1) as 14 cartas existem, ids unicos, nenhuma usa EffectKind::CloneAlly (guarda
// que von Neumann/Bruno NAO entraram nesta leva); (2) campos canonicos por-carta (tier/
// category/mana/power/ignores_weakness_wheel); (3) as 10 executaveis (volta/newton/
// pythagoras/mandelbrot/ada/tesla/einstein/faraday/godel/dee) resolvem via techMagic::execute
// SEM logic_error num contexto minimo; (4) as 3 fora-de-combate (euler/turing/menger) tem
// effects vazio; (5) paridade i18n das 14 chaves CARD_EXEC_<FIGURA>_NAME (2 locales).
//
// John Dee (Black-Mirror/Scrying, ADR-016 step 8, manifesto item 6, decisoes D1-D4 do lider
// 2026-07-15, AMB-07): Hibrida/Universal/TargetShape::Self, OnCast -> RevealIntent. Os
// testes EXAUSTIVOS (buff Scrying, dump/re-dump, D2 caotico->ruido, brain ausente
// fail-soft, no-op 0-inimigos, Scan aprimorado, stub posse-only, TESTE-REI de
// determinismo) vivem em techmagic_reveal_test.cpp - este arquivo so cobre o CATALOGO.
//
// Faraday (EM-Shield, ADR-016 Balde B, decisao do lider 2026-07-15): passou de
// ForaDeCombate (posse-only) pra Hibrida (ganhou face de combate castavel). Os testes
// EXAUSTIVOS do portao de imunidade (BlockedByImmunity, F-1/F-2/F-4, side_filter, dominó
// dos sitios de status ofensivo) vivem em techmagic_faraday_test.cpp - este arquivo so
// cobre o CATALOGO (campos + smoke test de execucao), mesmo padrao das outras 11 cartas.
//
// Godel (Null-Proof, ADR-016 Balde B PR3, decisao do lider 2026-07-15): passou de Passiva
// (so a flag ignores_weakness_wheel) pra Ativa castavel (OnCast -> ApplyStatus NullProof,
// AllyOnly), mantendo a flag original. Os testes EXAUSTIVOS do wiring do pierce (G-2/G-3:
// consome so quando ha algo a furar, fura Imune E Resistente, paridade preview, determinismo
// de RNG, dominó Tesla) vivem em techmagic_godel_test.cpp - este arquivo so cobre o CATALOGO.
//
// Planck (Quantum-Lock, manifesto item 5, decisoes do lider 2026-07-15): Passiva/Universal/
// mana 0, NAO executa via techMagic::execute (handler no-op deliberado, ver
// techmagic.cpp::handle_damage_quantize) - a quantizacao pluga direto no resolvedor/preview.
// Os testes EXAUSTIVOS (degraus corretos, paridade preview<->real, fronteiras do sorteio,
// determinismo de RNG, escopo por-ator, dominó crit/fumble) vivem em
// techmagic_quantize_test.cpp - este arquivo so cobre o CATALOGO.
//
// Cross-ref: gus/domain/combat/master_cards.hpp; techmagic.hpp; placeholder_cards_test.cpp
//            (mesmo padrao de spec de registry); docs/design/roster-analogos/
//            _EFEITOS-ESCOLHIDOS.md; ADR-016.

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <string>
#include <vector>

#include "fixed_random.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/initiative_queue.hpp"
#include "gus/domain/combat/master_cards.hpp"
#include "gus/domain/combat/techmagic.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::FixedRandom;

namespace {

CombatActor make_actor(const std::string& id, bool player_side, int hp = 100, int atk = 8,
                       int def = 0, int spd = 20) {
    return CombatActor(id, id, hp, atk, def, spd, CardFamily::Eletrico, player_side);
}

}  // namespace

// ===== 1. As 8 cartas existem, ids unicos, guarda anti-CloneAlly =====

TEST_CASE("master_cards: build_registry tem exatamente as 14 cartas suportadas",
          "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    REQUIRE(reg.size() == 14);
    for (const char* id : {"volta", "newton", "pythagoras", "mandelbrot", "ada", "godel",
                           "faraday", "euler", "turing", "menger", "tesla", "einstein",
                           "planck", "dee"})
        REQUIRE(reg.count(id) == 1);
}

TEST_CASE("master_cards: nenhuma carta usa EffectKind::CloneAlly (von Neumann/Bruno "
         "ficam de fora desta leva)",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    for (const auto& [id, card] : reg) {
        INFO("carta: " << id);
        for (const EffectSpec& spec : card.effects)
            REQUIRE(spec.kind != EffectKind::CloneAlly);
    }
}

// ===== 2. Campos canonicos por-carta =====

TEST_CASE("master_cards: todas as 8 sao tier Especial", "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    for (const auto& [id, card] : reg) {
        INFO("carta: " << id);
        REQUIRE(card.tier == CardTier::Especial);
    }
}

TEST_CASE("master_cards: volta = Ativa/Eletrico/mana 6, effects [OnDamageDealt Leech]",
          "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("volta");
    REQUIRE(c.category == CardCategory::Ativa);
    REQUIRE(c.family == CardFamily::Eletrico);
    REQUIRE(c.mana_cost == 6);
    REQUIRE(c.ignores_weakness_wheel == false);
    REQUIRE(c.effects.size() == 1);
    REQUIRE(c.effects[0].trigger == TriggerHook::OnDamageDealt);
    REQUIRE(c.effects[0].kind == EffectKind::Leech);
    REQUIRE(c.effects[0].percent == 50);
}

TEST_CASE("master_cards: newton = Hibrida/Universal/mana 6/Grupo, effects [OnCast "
         "ApplyStatus Stun EnemyOnly] + [OnCast ApplyStatus Reflect AllyOnly] + "
         "[OnDamageReceived Reflect]",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("newton");
    REQUIRE(c.category == CardCategory::Hibrida);
    REQUIRE(c.family == CardFamily::Universal);
    REQUIRE(c.mana_cost == 6);
    REQUIRE(c.target_shape == TargetShape::Grupo);  // N-1: Poco Gravitacional e AoE.
    REQUIRE(c.ignores_weakness_wheel == false);
    REQUIRE(c.effects.size() == 3);

    const auto& stun = c.effects[0];
    REQUIRE(stun.trigger == TriggerHook::OnCast);
    REQUIRE(stun.kind == EffectKind::ApplyStatus);
    REQUIRE(stun.status == StatusId::Stun);
    REQUIRE(stun.duration == 1);
    REQUIRE(stun.side_filter == SideFilter::EnemyOnly);

    const auto& reflect_status = c.effects[1];
    REQUIRE(reflect_status.trigger == TriggerHook::OnCast);
    REQUIRE(reflect_status.kind == EffectKind::ApplyStatus);
    REQUIRE(reflect_status.status == StatusId::Reflect);
    REQUIRE(reflect_status.magnitude == 30);
    REQUIRE(reflect_status.duration == 3);
    REQUIRE(reflect_status.stack_rule == StackRule::Refresh);
    REQUIRE(reflect_status.side_filter == SideFilter::AllyOnly);

    const auto& reflect_passive = c.effects[2];
    REQUIRE(reflect_passive.trigger == TriggerHook::OnDamageReceived);
    REQUIRE(reflect_passive.kind == EffectKind::Reflect);
    REQUIRE(reflect_passive.percent == 30);
}

TEST_CASE("master_cards: pythagoras = Passiva/Universal/mana 0, effects [OnRoundEnd "
         "HypotenuseCombo]",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("pythagoras");
    REQUIRE(c.category == CardCategory::Passiva);
    REQUIRE(c.family == CardFamily::Universal);
    REQUIRE(c.mana_cost == 0);
    REQUIRE(c.ignores_weakness_wheel == false);
    REQUIRE(c.effects.size() == 1);
    REQUIRE(c.effects[0].trigger == TriggerHook::OnRoundEnd);
    REQUIRE(c.effects[0].kind == EffectKind::HypotenuseCombo);
}

TEST_CASE("master_cards: mandelbrot = Ativa/Universal/mana 6, effects [OnCast "
         "RepeatLastAction magnitude 0 percent 50]",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("mandelbrot");
    REQUIRE(c.category == CardCategory::Ativa);
    REQUIRE(c.family == CardFamily::Universal);
    REQUIRE(c.mana_cost == 6);
    REQUIRE(c.ignores_weakness_wheel == false);
    REQUIRE(c.effects.size() == 1);
    REQUIRE(c.effects[0].trigger == TriggerHook::OnCast);
    REQUIRE(c.effects[0].kind == EffectKind::RepeatLastAction);
    REQUIRE(c.effects[0].magnitude == 0);  // sempre dispara, 0 consumo de RNG.
    REQUIRE(c.effects[0].percent == 50);
}

TEST_CASE("master_cards: ada = Passiva/Universal/mana 0, effects [OnAllyTurnEnd "
         "RepeatLastAction magnitude 34 percent 100]",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("ada");
    REQUIRE(c.category == CardCategory::Passiva);
    REQUIRE(c.family == CardFamily::Universal);
    REQUIRE(c.mana_cost == 0);
    REQUIRE(c.ignores_weakness_wheel == false);
    REQUIRE(c.effects.size() == 1);
    REQUIRE(c.effects[0].trigger == TriggerHook::OnAllyTurnEnd);
    REQUIRE(c.effects[0].kind == EffectKind::RepeatLastAction);
    REQUIRE(c.effects[0].magnitude == 34);  // chance% do Re-Run.
    REQUIRE(c.effects[0].percent == 100);   // Q3: ecoa a 100%, o freio e a chance.
}

TEST_CASE("master_cards: godel = Ativa/Universal/mana 0, effects [OnCast ApplyStatus "
         "NullProof AllyOnly], trunfo ignores_weakness_wheel=true",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("godel");
    REQUIRE(c.category == CardCategory::Ativa);
    REQUIRE(c.family == CardFamily::Universal);
    REQUIRE(c.mana_cost == 0);
    REQUIRE(c.ignores_weakness_wheel == true);
    REQUIRE(c.effects.size() == 1);

    const auto& null_proof = c.effects[0];
    REQUIRE(null_proof.trigger == TriggerHook::OnCast);
    REQUIRE(null_proof.kind == EffectKind::ApplyStatus);
    REQUIRE(null_proof.status == StatusId::NullProof);
    REQUIRE(null_proof.duration == 99);  // sentinela: sai por consumo, nao por tick.
    REQUIRE(null_proof.stack_rule == StackRule::Refresh);
    REQUIRE(null_proof.side_filter == SideFilter::AllyOnly);
}

TEST_CASE("master_cards: godel (ApplyStatus NullProof em OnCast) executa via "
         "techMagic::execute sem lancar, alvo ALIADO recebe o status",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("godel");

    CombatActor caster = make_actor("h", /*player_side=*/true);
    CombatActor ally = make_actor("h2", /*player_side=*/true);

    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &ally;  // side_filter AllyOnly: alvo do mesmo lado, nao dissipa.

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, c, ctx));
    bool has_null_proof = false;
    for (const auto& s : ally.status_effects())
        if (s.id == StatusId::NullProof) has_null_proof = true;
    REQUIRE(has_null_proof);
}

TEST_CASE("master_cards: as 3 fora-de-combate (euler/turing/menger) sao "
         "ForaDeCombate/mana 0/effects vazio; SOMENTE godel tem "
         "ignores_weakness_wheel=true",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();

    const Card& euler = reg.at("euler");
    REQUIRE(euler.category == CardCategory::ForaDeCombate);
    REQUIRE(euler.family == CardFamily::Eletrico);
    REQUIRE(euler.mana_cost == 0);
    REQUIRE(euler.effects.empty());
    REQUIRE(euler.ignores_weakness_wheel == false);

    const Card& turing = reg.at("turing");
    REQUIRE(turing.category == CardCategory::ForaDeCombate);
    REQUIRE(turing.family == CardFamily::Universal);
    REQUIRE(turing.mana_cost == 0);
    REQUIRE(turing.effects.empty());
    REQUIRE(turing.ignores_weakness_wheel == false);

    const Card& menger = reg.at("menger");
    REQUIRE(menger.category == CardCategory::ForaDeCombate);
    REQUIRE(menger.family == CardFamily::Universal);
    REQUIRE(menger.mana_cost == 0);
    REQUIRE(menger.effects.empty());
    REQUIRE(menger.ignores_weakness_wheel == false);

    // SO godel tem o trunfo fora-da-roda; nenhuma das 10 alem dela.
    int flagged = 0;
    for (const auto& [id, card] : reg)
        if (card.ignores_weakness_wheel) ++flagged;
    REQUIRE(flagged == 1);
}

// ===== 3. As 5 executaveis resolvem SEM logic_error num contexto minimo =====

TEST_CASE("master_cards: volta (Leech) executa via techMagic::execute sem lancar, "
         "caster ganha HP e mana",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("volta");

    CombatActor caster = make_actor("h", /*player_side=*/true, /*hp=*/50);
    // Estabelece capacidade de mana do turno (max_mana so existe apos o refresh da FSM),
    // depois gasta tudo: assim o restore_mana do Leech tem para onde subir (senao clampa
    // em max_mana_=0). heal idem: abrimos espaco de HP com dano previo.
    caster.refresh_resources_for_turn(/*round_index=*/0);  // mana_ = max_mana_ = 2.
    caster.spend_mana(caster.mana());                      // mana 2 -> 0.
    caster.take_damage(30);                                // hp 50 -> 20.
    const int mana_before = caster.mana();                 // 0.
    const int hp_before = caster.hp();                     // 20.
    CombatActor target = make_actor("e", /*player_side=*/false, /*hp=*/300);

    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &target;
    ctx.damage = 20;  // evento de dano ja resolvido (OnDamageDealt); Leech le daqui.

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnDamageDealt, c, ctx));
    REQUIRE(caster.hp() > hp_before);      // Leech curou.
    REQUIRE(caster.mana() > mana_before);  // Leech tambem restaura mana (clamp em max_mana).
}

TEST_CASE("master_cards: newton (ApplyStatus Stun em OnCast + Reflect em "
         "OnDamageReceived) executa sem lancar",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("newton");

    CombatActor caster = make_actor("h", /*player_side=*/true);
    CombatActor foe = make_actor("e", /*player_side=*/false);

    // OnCast: aplica Stun no counterpart (alvo da carta).
    techMagic::TechMagicContext cast_ctx;
    cast_ctx.caster = &caster;
    cast_ctx.counterpart = &foe;
    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, c, cast_ctx));
    bool stunned = false;
    for (const auto& s : foe.status_effects())
        if (s.id == StatusId::Stun) stunned = true;
    REQUIRE(stunned);

    // OnDamageReceived: caster sofreu dano de counterpart (o atacante original); Reflect
    // devolve fracao pro atacante.
    CombatActor attacker = make_actor("atk", /*player_side=*/false, /*hp=*/100);
    techMagic::TechMagicContext hit_ctx;
    hit_ctx.caster = &caster;       // dono da passiva (quem SOFREU o dano).
    hit_ctx.counterpart = &attacker;  // quem CAUSOU.
    hit_ctx.damage = 10;
    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnDamageReceived, c, hit_ctx));
    REQUIRE(attacker.hp() < 100);  // Reflect acertou o atacante.
}

TEST_CASE("master_cards: pythagoras (HypotenuseCombo em OnRoundEnd) executa sem "
         "lancar com um ledger minimo",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("pythagoras");

    CombatActor h1 = make_actor("h1", /*player_side=*/true);
    CombatActor h2 = make_actor("h2", /*player_side=*/true);
    CombatActor e = make_actor("e", /*player_side=*/false, /*hp=*/500);

    const std::vector<techMagic::RoundHitEntry> hits = {
        techMagic::RoundHitEntry{&h1, &e, 8}, techMagic::RoundHitEntry{&h2, &e, 5}};

    techMagic::TechMagicContext ctx;
    ctx.caster = &h1;  // dono da passiva precisa estar entre os atacantes (Q4).
    ctx.round_hits = &hits;

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnRoundEnd, c, ctx));
    // Nao valida o numero exato do bonus (ja coberto em techmagic_roundend_test.cpp) -
    // so que o handler existe e roda ate o fim sem lancar.
}

TEST_CASE("master_cards: mandelbrot (RepeatLastAction em OnCast, magnitude 0) ecoa "
         "50% da ultima acao sem tocar ctx.rng",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("mandelbrot");

    CombatActor caster = make_actor("h", /*player_side=*/true);
    CombatActor ally = make_actor("h2", /*player_side=*/true);
    CombatActor target = make_actor("e", /*player_side=*/false, /*hp=*/300);

    const techMagic::LastActionRecord last{
        &ally, CombatActionType::Attack, /*card_id=*/"", {{&target, 7}}};

    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.last_action = &last;  // ctx.rng deliberadamente nulo: magnitude 0 nao deve tocar.

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, c, ctx));
    // lround(7 * 50/100.0) = lround(3.5) = 4.
    REQUIRE(target.hp() == 296);
}

TEST_CASE("master_cards: ada (RepeatLastAction em OnAllyTurnEnd, magnitude 34) ecoa "
         "100% so quando a chance crava dentro; falha nao ecoa",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("ada");

    CombatActor caster = make_actor("h", /*player_side=*/true);
    CombatActor ally = make_actor("h2", /*player_side=*/true);
    CombatActor target = make_actor("e", /*player_side=*/false, /*hp=*/300);

    const techMagic::LastActionRecord last{
        &ally, CombatActionType::Attack, /*card_id=*/"", {{&target, 9}}};

    // Ramo DISPARA: next(100) devolve 0 < 34.
    {
        FixedRandom rng(/*next_double=*/0.5, /*next_int=*/0);
        techMagic::TechMagicContext ctx;
        ctx.caster = &caster;
        ctx.last_action = &last;
        ctx.rng = &rng;
        REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnAllyTurnEnd, c, ctx));
        REQUIRE(target.hp() == 291);  // 100% de 9.
    }

    // Ramo NAO DISPARA: next(100) devolve 99 >= 34; alvo intacto.
    {
        CombatActor target2 = make_actor("e2", /*player_side=*/false, /*hp=*/300);
        const techMagic::LastActionRecord last2{
            &ally, CombatActionType::Attack, /*card_id=*/"", {{&target2, 9}}};
        FixedRandom rng(/*next_double=*/0.5, /*next_int=*/99);
        techMagic::TechMagicContext ctx;
        ctx.caster = &caster;
        ctx.last_action = &last2;
        ctx.rng = &rng;
        REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnAllyTurnEnd, c, ctx));
        REQUIRE(target2.hp() == 300);
    }
}

TEST_CASE("master_cards: tesla = Ativa/Eletrico/mana 6/power 8, effects [OnCast "
         "ChainDamage magnitude 2 percent 62]",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("tesla");
    REQUIRE(c.category == CardCategory::Ativa);
    REQUIRE(c.family == CardFamily::Eletrico);
    REQUIRE(c.mana_cost == 6);
    REQUIRE(c.power == 8);  // EXCECAO: a cadeia escala do dano-base do primario.
    REQUIRE(c.ignores_weakness_wheel == false);
    REQUIRE(c.effects.size() == 1);
    REQUIRE(c.effects[0].trigger == TriggerHook::OnCast);
    REQUIRE(c.effects[0].kind == EffectKind::ChainDamage);
    REQUIRE(c.effects[0].magnitude == 2);  // 2 saltos = 3 alvos.
    REQUIRE(c.effects[0].percent == 62);   // retencao por salto.
}

TEST_CASE("master_cards: tesla (ChainDamage em OnCast) executa via techMagic::execute sem "
         "lancar e a cadeia salta nos 2 inimigos seguintes (62%/salto)",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("tesla");

    CombatActor caster = make_actor("h", /*player_side=*/true);
    CombatActor primary = make_actor("e0", /*player_side=*/false, /*hp=*/300);
    CombatActor e1 = make_actor("e1", /*player_side=*/false, /*hp=*/300);
    CombatActor e2 = make_actor("e2", /*player_side=*/false, /*hp=*/300);
    const std::vector<CombatActor*> roster = {&caster, &primary, &e1, &e2};

    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &primary;   // ja levou o dano-base fora do handler.
    ctx.combatants = &roster;
    ctx.damage = 10;              // dano-base efetivo no primario (D).

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, c, ctx));
    // Salto 1: lround(10 * 0.62^1) = lround(6.2) = 6. Salto 2: lround(10 * 0.62^2) =
    // lround(3.844) = 4. O primario nao recebe salto extra (so o dano-base, fora daqui).
    REQUIRE(e1.hp() == 300 - 6);
    REQUIRE(e2.hp() == 300 - 4);
    REQUIRE(primary.hp() == 300);  // sem salto no primario.
}

TEST_CASE("master_cards: einstein = Ativa/Cinetico/mana 6, effects [OnCast DelayAction "
         "magnitude 0]",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("einstein");
    REQUIRE(c.category == CardCategory::Ativa);
    REQUIRE(c.family == CardFamily::Cinetico);
    REQUIRE(c.mana_cost == 6);
    REQUIRE(c.ignores_weakness_wheel == false);
    REQUIRE(c.effects.size() == 1);
    REQUIRE(c.effects[0].trigger == TriggerHook::OnCast);
    REQUIRE(c.effects[0].kind == EffectKind::DelayAction);
    REQUIRE(c.effects[0].magnitude == 0);  // fim da fila (decisao do criador 2026-07-15).
}

TEST_CASE("master_cards: einstein (DelayAction em OnCast) executa via techMagic::execute "
         "sem lancar e empurra o alvo pro fim da fila",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("einstein");

    CombatActor caster = make_actor("h", /*player_side=*/true, /*hp=*/100, /*atk=*/8,
                                    /*def=*/0, /*spd=*/50);
    CombatActor target = make_actor("e0", /*player_side=*/false, /*hp=*/300, /*atk=*/0,
                                    /*def=*/0, /*spd=*/40);
    CombatActor e1 = make_actor("e1", /*player_side=*/false, /*hp=*/300, /*atk=*/0, /*def=*/0,
                                /*spd=*/30);
    InitiativeQueue queue({&caster, &target, &e1});  // ordem por SPD: caster, target, e1.

    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &target;
    ctx.queue = &queue;

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, c, ctx));
    REQUIRE(queue.order().back() == &target);  // fim da fila.
    REQUIRE(target.hp() == 300);               // sem dano.
}

TEST_CASE("master_cards: planck = Passiva/Universal/mana 0, effects [OnCast DamageQuantize "
         "magnitude 50 percent 25]",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("planck");
    REQUIRE(c.category == CardCategory::Passiva);
    REQUIRE(c.family == CardFamily::Universal);
    REQUIRE(c.mana_cost == 0);
    REQUIRE(c.ignores_weakness_wheel == false);
    REQUIRE(c.effects.size() == 1);

    const auto& quantize = c.effects[0];
    REQUIRE(quantize.trigger == TriggerHook::OnCast);
    REQUIRE(quantize.kind == EffectKind::DamageQuantize);
    REQUIRE(quantize.magnitude == 50);  // chance% do degrau CENTRAL.
    REQUIRE(quantize.percent == 25);    // chance% de CADA extremo (piso E teto).
}

TEST_CASE("master_cards: planck (DamageQuantize em OnCast) executa via techMagic::execute "
         "sem lancar - handler no-op deliberado (a quantizacao vive fora do dispatcher)",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("planck");

    CombatActor caster = make_actor("h", /*player_side=*/true);

    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, c, ctx));
}

TEST_CASE("master_cards: faraday = Hibrida/Eletrico/mana kActiveManaCost, effects [OnCast "
         "ApplyStatus BlindagemEM duration 3 Refresh AllyOnly]",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("faraday");
    REQUIRE(c.category == CardCategory::Hibrida);
    REQUIRE(c.family == CardFamily::Eletrico);
    REQUIRE(c.mana_cost == 6);  // kActiveManaCost, mesmo //PLAYTEST de volta/newton/mandelbrot.
    REQUIRE(c.ignores_weakness_wheel == false);
    REQUIRE(c.effects.size() == 1);

    const auto& shield = c.effects[0];
    REQUIRE(shield.trigger == TriggerHook::OnCast);
    REQUIRE(shield.kind == EffectKind::ApplyStatus);
    REQUIRE(shield.status == StatusId::BlindagemEM);
    REQUIRE(shield.duration == 3);
    REQUIRE(shield.stack_rule == StackRule::Refresh);
    REQUIRE(shield.side_filter == SideFilter::AllyOnly);
}

TEST_CASE("master_cards: faraday (ApplyStatus BlindagemEM em OnCast) executa via "
         "techMagic::execute sem lancar, alvo ALIADO recebe a blindagem",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("faraday");

    CombatActor caster = make_actor("h", /*player_side=*/true);
    CombatActor ally = make_actor("h2", /*player_side=*/true);

    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &ally;  // side_filter AllyOnly: alvo do mesmo lado, nao dissipa.

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, c, ctx));
    bool shielded = false;
    for (const auto& s : ally.status_effects())
        if (s.id == StatusId::BlindagemEM) shielded = true;
    REQUIRE(shielded);
}

TEST_CASE("master_cards: dee = Hibrida/Universal/mana kActiveManaCost/Self, effects [OnCast "
         "RevealIntent duration 3 status Scrying Refresh]",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("dee");
    REQUIRE(c.category == CardCategory::Hibrida);
    REQUIRE(c.family == CardFamily::Universal);
    REQUIRE(c.mana_cost == 6);  // kActiveManaCost, mesmo //PLAYTEST de volta/newton/faraday.
    REQUIRE(c.target_shape == TargetShape::Self);
    REQUIRE(c.ignores_weakness_wheel == false);
    REQUIRE(c.effects.size() == 1);

    const auto& reveal = c.effects[0];
    REQUIRE(reveal.trigger == TriggerHook::OnCast);
    REQUIRE(reveal.kind == EffectKind::RevealIntent);
    REQUIRE(reveal.duration == 3);
    REQUIRE(reveal.status == StatusId::Scrying);
    REQUIRE(reveal.stack_rule == StackRule::Refresh);
}

TEST_CASE("master_cards: dee (RevealIntent em OnCast) executa via techMagic::execute sem "
         "lancar, caster ganha Scrying",
         "[domain][combat][cards][techmagic][mastercards]") {
    const auto reg = MasterCards::build_registry();
    const Card& c = reg.at("dee");

    CombatActor caster = make_actor("h", /*player_side=*/true);
    const InitiativeQueue queue({&caster});
    const std::vector<CombatActor*> combatants = {&caster};

    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;
    ctx.counterpart = &caster;  // TargetShape::Self.
    ctx.combatants = &combatants;
    ctx.queue = const_cast<InitiativeQueue*>(&queue);

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, c, ctx));
    bool scrying = false;
    for (const auto& s : caster.status_effects())
        if (s.id == StatusId::Scrying) scrying = true;
    REQUIRE(scrying);
}

// ===== 4. Paridade i18n das 14 chaves (2 locales) =====

namespace {

// Le um .md de traducao e devolve o conjunto de chaves "## CHAVE" encontradas. Parser
// minimo local (nao reusa tools/i18n_parity.py, que e Python fora do escopo C++ deste
// teste); so verifica presenca, mesmo contrato de MdTranslationLoader (secao 5).
std::vector<std::string> keys_in(const std::string& path) {
    std::ifstream in(path);
    std::vector<std::string> out;
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("## ", 0) == 0) out.push_back(line.substr(3));
    }
    return out;
}

bool has_key(const std::vector<std::string>& keys, const std::string& key) {
    for (const auto& k : keys)
        if (k == key) return true;
    return false;
}

}  // namespace

// GUSWORLD_TRANSLATIONS_DIR e injetado por tests/CMakeLists.txt
// (${CMAKE_SOURCE_DIR}/../game/translations) - caminho ABSOLUTO, robusto ao CWD do
// ctest (catch_discover_tests roda o binario a partir do build dir).
#ifndef GUSWORLD_TRANSLATIONS_DIR
#define GUSWORLD_TRANSLATIONS_DIR "../../../../game/translations"
#endif

TEST_CASE("master_cards: as 14 chaves CARD_EXEC_<FIGURA>_NAME existem em pt_br.md e "
         "en_intl.md",
         "[domain][combat][cards][techmagic][mastercards][i18n]") {
    const std::vector<std::string> pt =
        keys_in(std::string(GUSWORLD_TRANSLATIONS_DIR) + "/pt_br.md");
    const std::vector<std::string> en =
        keys_in(std::string(GUSWORLD_TRANSLATIONS_DIR) + "/en_intl.md");
    REQUIRE_FALSE(pt.empty());
    REQUIRE_FALSE(en.empty());

    for (const char* figura : {"VOLTA", "NEWTON", "PYTHAGORAS", "MANDELBROT", "ADA",
                               "GODEL", "FARADAY", "EULER", "TURING", "MENGER", "TESLA",
                               "EINSTEIN", "PLANCK", "DEE"}) {
        const std::string key = std::string("CARD_EXEC_") + figura + "_NAME";
        INFO("chave: " << key);
        REQUIRE(has_key(pt, key));
        REQUIRE(has_key(en, key));
    }
}
