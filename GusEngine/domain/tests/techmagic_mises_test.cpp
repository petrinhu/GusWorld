// techmagic_mises_test.cpp
//
// Spec executavel (Catch2 v3) do Calc-Edge (Mises), CARD-ENGINE-MANIFESTO item 9 do
// executor techMagic (ADR-016, decisoes do lider 2026-07-16):
//   Passiva DUPLA mana-0, equip-only, Universal. MARCADOR fora do dispatcher (mesmo padrao
//   DamageQuantize/Planck e DiversityBonus/Hayek).
//   Face 1 ("party aloca melhor"): +1 AP (magnitude) NESTE turno pro DONO da passiva
//   equipada, em CombatStateMachine::begin_turn (DEPOIS de refresh_resources_for_turn) -
//   NAO muta max_ap_, nao persiste pro proximo turno.
//   Face 2 ("comando central"): atores com CombatActor::central_command()==true DO LADO
//   OPOSTO a quem porta a Mises equipada sao (a) empurrados pro FIM do bloco do proprio
//   lado na fronteira de rodada (CombatStateMachine::regroup_round_by_side, 2a
//   stable_partition sobre InitiativeQueue::regroup_stable) e (b) sofrem desconto FIXO
//   percent% (kMisesAimError) no dano ofensivo que causam (carta E ataque basico, ULTIMO
//   fator da cadeia divisiva/raw - gemeo preview<->real). 0 RNG em qualquer face.
//
// Carta de teste montada LOCALMENTE (id "techmagic.mises.*"), NUNCA do registry de
// producao (MasterCards) - mesmo padrao de isolamento de techmagic_quantize_test.cpp/
// techmagic_hayek_test.cpp em relacao a master_cards_test.cpp (que so cobre o CATALOGO,
// campos + smoke test de execucao). O motor e AGNOSTICO por-ator (zero hardcode de
// id/nome de personagem no domain) - so 1 EnemyTemplate/CombatActor de TESTE ganha a tag
// aqui; a curadoria de quais inimigos canonicos do bestiario levam "comando central" e
// decisao de design/lore do criador, fora de escopo desta leva (ver AMB-10,
// _EFEITOS-ESCOLHIDOS.md).
//
// Cross-ref: gus/domain/combat/combat_enums.hpp (EffectKind::ApEfficiency);
//            gus/domain/combat/combat_actor.hpp (central_command()/grant_bonus_ap);
//            combat_state_machine.cpp (apefficiency_spec_of/apefficiency_spec_on_side/
//            mises_aim_error_spec_for/mises_aim_error_mult/mises_aim_log_suffix - wiring
//            real; regroup_round_by_side - a 2a stable_partition; estimate_card_damage/
//            preview_basic_attack_damage - gemeo PURO);
//            docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md (AMB-10);
//            docs/design/mecanicas/cartas-technomagik.md; ADR-016.

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <unordered_map>
#include <vector>

#include "counting_random.hpp"
#include "fixed_random.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/techmagic.hpp"

using namespace gus::domain::combat;
using gus::domain::tests::CountingRandom;
using gus::domain::tests::FixedRandom;

namespace {

CombatActor make_actor(const std::string& id, bool player_side, int hp = 100, int atk = 8,
                       int def = 0, int spd = 20,
                       CardFamily family = CardFamily::Eletrico, int kills = 0) {
    return CombatActor(id, id, hp, atk, def, spd, family, player_side, /*is_boss=*/false,
                       kills);
}

// Carta de dano PLANA (tier Comum - isenta do gate 1x/batalha). Mesmo padrao de
// damage_card() em techmagic_hayek_test.cpp/techmagic_quantize_test.cpp.
Card damage_card(const std::string& id, CardFamily family, int power, int crit_chance = 0) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = family;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = power;
    c.crit_chance = crit_chance;
    c.target_shape = TargetShape::Single;
    return c;
}

// Duplo LOCAL da Mises de producao: Passiva/Universal/mana 0, equip-only, 1 EffectSpec
// OnCast->ApEfficiency. magnitude=+AP da face 1; percent=desconto% da face 2.
Card mises_card(const std::string& id, int bonus_ap = 1, int aim_error_pct = 13) {
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
    c.category = CardCategory::Passiva;
    c.effects = {EffectSpec{.trigger = TriggerHook::OnCast,
                            .kind = EffectKind::ApEfficiency,
                            .magnitude = bonus_ap,
                            .percent = aim_error_pct}};
    return c;
}

std::unordered_map<std::string, Card> registry(std::initializer_list<Card> cards) {
    std::unordered_map<std::string, Card> d;
    for (const auto& c : cards) d.emplace(c.id, c);
    return d;
}

// Mesmo padrao de play_by_actor de techmagic_hayek_test.cpp: mapeia UMA acao por id de
// ator (1x cada); fora disso passa (0 AP).
CombatActionProvider play_by_actor(std::unordered_map<std::string, CombatAction> actions) {
    auto acts = std::make_shared<std::unordered_map<std::string, CombatAction>>(std::move(actions));
    return [acts](CombatActor& a, const CombatState&) -> CombatAction {
        auto it = acts->find(a.id());
        if (it == acts->end()) return CombatAction::pass();
        CombatAction action = it->second;
        acts->erase(it);
        return action;
    };
}

bool log_has(const CombatStateMachine& sm, const std::string& needle) {
    for (const auto& entry : sm.log())
        if (entry.message.find(needle) != std::string::npos) return true;
    return false;
}

}  // namespace

// ===== 1. Face 1: +1 AP so pro DONO equipado, nao persiste ao proximo turno ================

TEST_CASE("techmagic mises: +1 AP concedido no begin_turn do dono equipado, sem mutar "
         "max_ap (nao persiste)",
         "[domain][combat][techmagic][mises]") {
    Card mises = mises_card("techmagic.mises.ap.card");
    auto reg = registry({mises});

    CombatActor m = make_actor("m", true, 100, 8, 0, 90);
    m.set_equipped_special_ids({mises.id});
    CombatActor e = make_actor("e", false, 100, 8, 0, 10);

    auto provider = play_by_actor({});
    CombatStateMachine sm({&m, &e}, provider, &reg, nullptr, nullptr);

    sm.begin_turn();
    REQUIRE(m.ap() == combat_constants::kBaseApPerTurn + 1);
    REQUIRE(m.max_ap() == combat_constants::kBaseApPerTurn);  // max_ap_ intocado
    REQUIRE(log_has(sm, "Calc-Edge"));
    REQUIRE(log_has(sm, "+1 AP"));

    sm.run_active_turn_to_end();  // m passa (0 acoes).
    sm.advance_to_next_actor();
    sm.begin_turn();  // turno de e (sem Mises).
    REQUIRE(e.ap() == combat_constants::kBaseApPerTurn);  // sem bonus, ator sem a carta.
}

TEST_CASE("techmagic mises: sem a carta equipada, AP normal (sem bonus, sem log Calc-Edge)",
         "[domain][combat][techmagic][mises]") {
    CombatActor m = make_actor("m", true, 100, 8, 0, 90);
    CombatActor e = make_actor("e", false, 100, 8, 0, 10);

    auto provider = play_by_actor({});
    CombatStateMachine sm({&m, &e}, provider, nullptr, nullptr, nullptr);

    sm.begin_turn();
    REQUIRE(m.ap() == combat_constants::kBaseApPerTurn);
    REQUIRE_FALSE(log_has(sm, "Calc-Edge"));
}

// ===== 2. Face 2 (atraso): taggeado vai pro FIM do bloco inimigo, SO com Mises ativo =======

TEST_CASE("techmagic mises: party abre - taggeado com Mises ativo vai pro FIM do bloco "
         "inimigo (2a stable_partition)",
         "[domain][combat][techmagic][mises]") {
    Card mises = mises_card("techmagic.mises.delay.party.card");
    auto reg = registry({mises});

    CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);
    CombatActor s2 = make_actor("s2", true, 100, 8, 0, 80);  // porta a Mises.
    CombatActor e1 = make_actor("e1", false, 100, 8, 0, 70);  // taggeado.
    CombatActor e2 = make_actor("e2", false, 100, 8, 0, 60);  // NAO taggeado.
    s2.set_equipped_special_ids({mises.id});
    e1.set_central_command(true);

    auto provider = play_by_actor({});
    CombatStateMachine sm({&s1, &s2, &e1, &e2}, provider, &reg, nullptr, nullptr);

    // Party abre (SPD 90/80 > 70/60): ordem esperada apos regroup do construtor =
    // [s1, s2, e2(nao-taggeado), e1(taggeado, empurrado pro fim do bloco inimigo)].
    const auto& order = sm.queue().order();
    REQUIRE(order.size() == 4);
    REQUIRE(order[0]->id() == "s1");
    REQUIRE(order[1]->id() == "s2");
    REQUIRE(order[2]->id() == "e2");
    REQUIRE(order[3]->id() == "e1");
    REQUIRE(log_has(sm, "comando central"));
    REQUIRE(log_has(sm, "Mises ativo"));
}

TEST_CASE("techmagic mises: inimigo abre - taggeado com Mises ativo vai pro FIM do "
         "bloco inimigo, ANTES do bloco da party (SPD do taggeado > do nao-taggeado, "
         "pra o atraso INVERTER a ordem natural de SPD - mata M4 sem coincidencia)",
         "[domain][combat][techmagic][mises]") {
    Card mises = mises_card("techmagic.mises.delay.enemy.card");
    auto reg = registry({mises});

    CombatActor s1 = make_actor("s1", true, 100, 8, 0, 10);  // porta a Mises.
    // CRITICO (reforco QA): o TAGGEADO tem SPD MAIOR que o nao-taggeado. Sem a logica de
    // atraso, a ordem natural de SPD seria [e2(90), e1(80), s1(10)]; COM o atraso, e2
    // (taggeado) e EMPURRADO pro fim do bloco inimigo -> [e1, e2, s1]. Assim o teste
    // distingue mecanismo-ATIVO de AUSENTE (se e2 tivesse SPD menor, ele ja viria atras
    // por SPD e o teste passaria por coincidencia, sem exercitar o atraso).
    CombatActor e1 = make_actor("e1", false, 100, 8, 0, 80);  // NAO taggeado, SPD MENOR.
    CombatActor e2 = make_actor("e2", false, 100, 8, 0, 90);  // taggeado, SPD MAIOR.
    s1.set_equipped_special_ids({mises.id});
    e2.set_central_command(true);

    auto provider = play_by_actor({});
    CombatStateMachine sm({&s1, &e1, &e2}, provider, &reg, nullptr, nullptr);

    // Inimigo abre (SPD 90/80 > 10). Ordem NATURAL de SPD seria [e2(90), e1(80), s1(10)];
    // o atraso INVERTE o bloco inimigo -> [e1(nao-taggeado), e2(taggeado, empurrado),
    // s1(party, so age depois do bloco inimigo INTEIRO)].
    const auto& order = sm.queue().order();
    REQUIRE(order.size() == 3);
    REQUIRE(order[0]->id() == "e1");
    REQUIRE(order[1]->id() == "e2");
    REQUIRE(order[2]->id() == "s1");
}

TEST_CASE("techmagic mises: taggeado SEM Mises ativo do lado oposto - sem atraso "
         "(ordem SPD normal), log declara 'sem Mises ativo'",
         "[domain][combat][techmagic][mises]") {
    CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);
    CombatActor e1 = make_actor("e1", false, 100, 8, 0, 70);  // taggeado, mas ninguem tem Mises.
    CombatActor e2 = make_actor("e2", false, 100, 8, 0, 60);
    e1.set_central_command(true);

    auto provider = play_by_actor({});
    CombatStateMachine sm({&s1, &e1, &e2}, provider, nullptr, nullptr, nullptr);

    // Sem Mises ativo: ordem SPD normal (party abre, inimigos na ordem de SPD de sempre -
    // e1 ANTES de e2, sem empurrao).
    const auto& order = sm.queue().order();
    REQUIRE(order[0]->id() == "s1");
    REQUIRE(order[1]->id() == "e1");
    REQUIRE(order[2]->id() == "e2");
    REQUIRE(log_has(sm, "comando central"));
    REQUIRE(log_has(sm, "sem Mises ativo"));
}

TEST_CASE("techmagic mises: sem NENHUM ator taggeado, regroup normal e SEM log de "
         "'comando central' (mesmo com Mises equipada)",
         "[domain][combat][techmagic][mises]") {
    Card mises = mises_card("techmagic.mises.notag.card");
    auto reg = registry({mises});

    CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);
    s1.set_equipped_special_ids({mises.id});
    CombatActor e1 = make_actor("e1", false, 100, 8, 0, 70);

    auto provider = play_by_actor({});
    CombatStateMachine sm({&s1, &e1}, provider, &reg, nullptr, nullptr);

    REQUIRE_FALSE(log_has(sm, "comando central"));
}

// ===== 3. Face 2 (erro de mira): -13% no dano do taggeado, carta E ataque basico ===========

TEST_CASE("techmagic mises: erro de mira -13% no ATAQUE BASICO do taggeado, SO com "
         "Mises ativa do lado oposto; paridade preview<->real",
         "[domain][combat][techmagic][mises]") {
    Card mises = mises_card("techmagic.mises.basic.card", /*bonus_ap=*/1, /*aim_error_pct=*/13);
    auto reg = registry({mises});

    CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);
    s1.set_equipped_special_ids({mises.id});
    // atk alto pra evitar coincidencia de arredondamento no lround do desconto.
    CombatActor e1 = make_actor("e1", false, 100000, 20, 0, 10);
    e1.set_central_command(true);

    auto provider = play_by_actor({
        {"s1", CombatAction::attack(e1.id())},
        {"e1", CombatAction::attack(s1.id())},
    });
    CombatStateMachine sm({&s1, &e1}, provider, &reg, nullptr, nullptr);

    sm.begin_turn();
    sm.run_active_turn_to_end();  // s1 ataca e1 (sem desconto - s1 nao e taggeado).
    sm.advance_to_next_actor();

    // Preview ANTES de e1 agir de verdade.
    const int preview = sm.preview_basic_attack_damage(e1, s1);
    // raw = max(kMinDamage, 20-0) = 20; com -13%: lround(20*0.87) = lround(17.4) = 17.
    REQUIRE(preview == 17);

    const int hp_before = s1.hp();
    sm.begin_turn();
    sm.run_active_turn_to_end();  // e1 ataca s1 de verdade.
    REQUIRE(hp_before - s1.hp() == preview);
    REQUIRE(log_has(sm, "comando central"));
    REQUIRE(log_has(sm, "erro de calculo"));
    REQUIRE(log_has(sm, "-13%"));
}

TEST_CASE("techmagic mises: erro de mira -13% na CARTA do taggeado; paridade "
         "preview<->real (estimate_card_damage)",
         "[domain][combat][techmagic][mises]") {
    Card mises = mises_card("techmagic.mises.card.card", /*bonus_ap=*/1, /*aim_error_pct=*/13);
    Card bolt = damage_card("techmagic.mises.card.bolt", CardFamily::Eletrico, 20);
    auto reg = registry({mises, bolt});

    CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);
    s1.set_equipped_special_ids({mises.id});
    CombatActor e1 = make_actor("e1", false, 100000, 20, 0, 10, CardFamily::Sonico,
                                /*kills=*/20);  // v no piso (0.05), numeros limpos.
    e1.set_central_command(true);

    // r=0.0 forca o canal COMUM pro degrau PISO (variancia zero na pratica com v=0.05,
    // mas cravamos r=0.0 pra determinismo total); roll=99 fica seguro em COMUM.
    FixedRandom rng(/*next_double=*/0.0, /*next_int=*/99);
    auto provider = play_by_actor({
        {"s1", CombatAction::attack(e1.id())},
        {"e1", CombatAction::use_card(bolt.id, s1.id())},
    });
    CombatStateMachine sm({&s1, &e1}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    sm.run_active_turn_to_end();  // s1 ataca (nao taggeado, sem desconto).
    sm.advance_to_next_actor();

    const CardDamageEstimate preview = sm.estimate_card_damage(e1, s1, bolt);
    // base0 = (power=20 + atk=20) * mult_fraqueza(1.0, Sonico x Eletrico neutro-ish nao
    // importa aqui pois so comparamos preview<->real) * ... * mult_mises(0.87). Nao
    // recalculamos a formula inteira aqui - o que importa e a IGUALDADE preview==real.
    const int hp_before = s1.hp();
    sm.begin_turn();
    sm.run_active_turn_to_end();  // e1 joga a carta de verdade (r=0.0 -> canal PISO).
    REQUIRE(hp_before - s1.hp() == preview.min_damage);
    REQUIRE(log_has(sm, "erro de calculo"));
}

TEST_CASE("techmagic mises: SEM Mises ativo do lado oposto, taggeado NAO sofre desconto "
         "(mult 1.0), mas o log ainda cita 'sem Mises ativo, mira intacta'",
         "[domain][combat][techmagic][mises]") {
    CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);  // SEM Mises equipada.
    CombatActor e1 = make_actor("e1", false, 100000, 20, 0, 10);
    e1.set_central_command(true);

    auto provider = play_by_actor({
        {"s1", CombatAction::attack(e1.id())},
        {"e1", CombatAction::attack(s1.id())},
    });
    CombatStateMachine sm({&s1, &e1}, provider, nullptr, nullptr, nullptr);

    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();

    const int hp_before = s1.hp();
    sm.begin_turn();
    sm.run_active_turn_to_end();  // e1 ataca s1: raw = max(kMinDamage, 20-0) = 20, SEM desconto.
    REQUIRE(hp_before - s1.hp() == 20);
    REQUIRE(log_has(sm, "sem Mises ativo, mira intacta"));
}

TEST_CASE("techmagic mises: Mises ativa mas o ATACANTE NAO e taggeado - sem desconto, "
         "sem sufixo 'comando central' no log dele",
         "[domain][combat][techmagic][mises]") {
    Card mises = mises_card("techmagic.mises.untagged.card");
    auto reg = registry({mises});

    CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);
    s1.set_equipped_special_ids({mises.id});
    CombatActor e1 = make_actor("e1", false, 100000, 20, 0, 10);  // SEM a tag.

    auto provider = play_by_actor({
        {"s1", CombatAction::attack(e1.id())},
        {"e1", CombatAction::attack(s1.id())},
    });
    CombatStateMachine sm({&s1, &e1}, provider, &reg, nullptr, nullptr);

    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();

    const int hp_before = s1.hp();
    sm.begin_turn();
    sm.run_active_turn_to_end();  // e1 (sem tag) ataca s1: SEM desconto.
    REQUIRE(hp_before - s1.hp() == 20);

    bool e1_attack_has_comando_central = false;
    for (const auto& entry : sm.log())
        if (entry.actor_id == "e1" && entry.action == CombatActionType::Attack &&
            entry.message.find("comando central") != std::string::npos)
            e1_attack_has_comando_central = true;
    REQUIRE_FALSE(e1_attack_has_comando_central);
}

// ===== 4. Determinismo: RNG identico com/sem Mises (nenhuma face consome RNG) ==============

TEST_CASE("techmagic mises: consumo de RNG identico com/sem Mises equipada+taggeado "
         "(nenhuma das 2 faces sorteia nada)",
         "[domain][combat][techmagic][mises]") {
    Card mises = mises_card("techmagic.mises.rng.card");
    Card bolt = damage_card("techmagic.mises.rng.bolt", CardFamily::Eletrico, 12);
    auto reg = registry({mises, bolt});

    const auto run = [&](bool with_mises) {
        CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);
        if (with_mises) s1.set_equipped_special_ids({mises.id});
        CombatActor e1 = make_actor("e1", false, 300, 8, 0, 10);
        if (with_mises) e1.set_central_command(true);

        CountingRandom rng;
        auto provider = play_by_actor({
            {"s1", CombatAction::use_card(bolt.id, e1.id())},
            {"e1", CombatAction::attack(s1.id())},
        });
        CombatStateMachine sm({&s1, &e1}, provider, &reg, nullptr, &rng);

        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
        sm.begin_turn();
        sm.run_active_turn_to_end();

        return std::pair<int, int>{rng.next_calls, rng.next_double_calls};
    };

    const auto without_mises = run(false);
    const auto with_mises = run(true);

    // s1 (UseCard) = 1 next + 1 next_double (canal COMUM); e1 (Attack) = 0 RNG.
    REQUIRE(without_mises.first == 1);
    REQUIRE(without_mises.second == 1);
    REQUIRE(with_mises.first == without_mises.first);
    REQUIRE(with_mises.second == without_mises.second);
}

// ===== 5. Handler no-op no dispatcher =======================================================

TEST_CASE("techmagic mises: ApEfficiency tem handler no dispatcher (marker no-op "
         "deliberado, mesmo padrao de DamageQuantize/DiversityBonus) - nao lanca "
         "logic_error",
         "[domain][combat][techmagic][mises]") {
    Card mises = mises_card("techmagic.mises.dispatcher.card");

    CombatActor caster = make_actor("m", /*player_side=*/true);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, mises, ctx));
}
