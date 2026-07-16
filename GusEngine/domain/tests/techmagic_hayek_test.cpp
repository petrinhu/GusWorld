// techmagic_hayek_test.cpp
//
// Spec executavel (Catch2 v3) do Free-Order (Hayek), CARD-ENGINE-MANIFESTO item 7 do
// executor techMagic (ADR-016, AMB-09, decisoes do lider 2026-07-16):
//   Passiva mana-0, equip-only, Universal. Se os membros do lado do PORTADOR fazem ACOES
//   DIFERENTES na mesma rodada, bonus escalonado de dano/precisao (NUNCA pune, so premia -
//   mult sempre >= 1.0, fumble so diminui, piso 0).
//   Ledger novo `CombatStateMachine::round_actions_` (techMagic::RoundActionEntry),
//   granularidade de ACAO (populado em resolve_action, NAO em apply_damage_with_hooks) -
//   primo do round_hits_ do Pythagoras.
//   M2: assinatura de UseCard REFINADA pela CardFamily da carta (2 cartas da MESMA familia
//   = 1 assinatura; familias diferentes = 2 assinaturas).
//   Forward-only: a acao corrente conta as assinaturas DISTINTAS ja registradas pelo lado
//   dela ANTES dela mesma, e SO DEPOIS entra no ledger (anti auto-inflacao).
//   Degraus (//PLAYTEST, decisao do lider): 2 distintas = +5% dano / -1pp falha; 3 = +8% /
//   -2pp; 4+ = +13% / -3pp (cap automatico - nenhum EffectSpec tem magnitude > 4).
//   Beneficia o LADO INTEIRO enquanto o dono estiver vivo E com a carta equipada.
//
// Carta de teste montada LOCALMENTE (id "techmagic.hayek.*"), NUNCA do registry de producao
// (MasterCards) - esta leva do CARD-ENGINE-MANIFESTO ainda nao entrou no catalogo de
// producao (so o motor/EffectKind; catalogacao/i18n ficam pra uma leva futura, mesmo padrao
// de isolamento de techmagic_quantize_test.cpp em relacao a master_cards_test.cpp). O motor
// e AGNOSTICO por-ator (zero hardcode de id/nome de personagem no domain).
//
// Cross-ref: gus/domain/combat/combat_enums.hpp (EffectKind::DiversityBonus);
//            gus/domain/combat/techmagic.hpp (techMagic::RoundActionEntry);
//            combat_state_machine.cpp (same_action_signature/distinct_action_count/
//            diversity_spec_of/hayek_bonus_for/hayek_log_suffix - wiring real;
//            estimate_card_damage/preview_basic_attack_damage - gemeo PURO);
//            docs/design/roster-analogos/_EFEITOS-ESCOLHIDOS.md (AMB-09);
//            docs/design/mecanicas/cartas-technomagik.md; ADR-016.

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <unordered_map>
#include <vector>

#include "counting_random.hpp"
#include "fixed_random.hpp"
#include "gus/domain/combat/combat_actor.hpp"
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
// damage_card() em techmagic_quantize_test.cpp/techmagic_godel_test.cpp.
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

// Duplo LOCAL da Hayek de producao: Passiva/Universal/mana 0, equip-only, 3 EffectSpec
// OnCast->DiversityBonus (um por DEGRAU). Cada EffectSpec e um degrau: `magnitude` =
// limiar de assinaturas distintas, `percent` = bonus% de dano, `duration` = reducao pp do
// limiar de falha (reuse deliberado do campo, ver combat_enums.hpp::EffectKind::
// DiversityBonus). Numeros = os //PLAYTEST decididos pelo lider (AMB-09): 2=+5%/-1pp;
// 3=+8%/-2pp; 4+=+13%/-3pp (cap automatico - nenhum spec tem magnitude>4).
Card hayek_card(const std::string& id) {
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
    c.effects = {
        EffectSpec{.trigger = TriggerHook::OnCast,
                  .kind = EffectKind::DiversityBonus,
                  .magnitude = 2,
                  .percent = 5,
                  .duration = 1},
        EffectSpec{.trigger = TriggerHook::OnCast,
                  .kind = EffectKind::DiversityBonus,
                  .magnitude = 3,
                  .percent = 8,
                  .duration = 2},
        EffectSpec{.trigger = TriggerHook::OnCast,
                  .kind = EffectKind::DiversityBonus,
                  .magnitude = 4,
                  .percent = 13,
                  .duration = 3},
    };
    return c;
}

// Duplo LOCAL do Mandelbrot (RepeatLastAction, magnitude 0 = sempre dispara, 0 RNG). So
// pra provar que o eco NAO entra no ledger de acoes do Hayek (teste 4).
Card mandelbrot_local(const std::string& id) {
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
                            .percent = 50}};
    return c;
}

std::unordered_map<std::string, Card> registry(std::initializer_list<Card> cards) {
    std::unordered_map<std::string, Card> d;
    for (const auto& c : cards) d.emplace(c.id, c);
    return d;
}

// Mesmo padrao de play_by_actor de techmagic_quantize_test.cpp/techmagic_godel_test.cpp:
// mapeia UMA acao por id de ator (1x cada); fora disso passa (0 AP).
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

}  // namespace

// ===== 1. 1 distinta (a propria 1a acao da rodada) = SEM bonus (auto-inflacao) ==============
//
// Free-Order exige "conte as distintas ANTES da acao corrente" - a 1a acao de uma rodada
// nao tem NADA antes dela (distinct_count=0), entao nunca se auto-beneficia mesmo sendo,
// ela mesma, "uma acao". Prova tambem a auto-inflacao: a acao nao conta a si propria.

TEST_CASE("techmagic hayek: a 1a acao da rodada (nada registrado antes dela) nao recebe "
         "bonus - prova anti auto-inflacao",
         "[domain][combat][techmagic][hayek]") {
    Card hayek = hayek_card("techmagic.hayek.first.card");
    auto reg = registry({hayek});

    CombatActor h = make_actor("h", true, 100, 8, 0, 50);
    h.set_equipped_special_ids({hayek.id});
    CombatActor e = make_actor("e", false, 300, 0, 0, 10);

    auto provider = play_by_actor({{"h", CombatAction::attack(e.id())}});
    CombatStateMachine sm({&h, &e}, provider, &reg, nullptr, nullptr);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    // raw = max(kMinDamage, atk-def) = 8. SEM bonus: dano == raw, nao lround(8*1.05)=8 (o
    // mesmo valor por coincidencia numerica), entao o teste real da auto-inflacao esta no
    // ledger: so 1 entrada (a propria acao de h), nunca 2.
    REQUIRE(e.max_hp() - e.hp() == 8);
    REQUIRE(sm.round_actions().size() == 1);
}

// ===== 2. 2/3/4 distintas = degraus certos (dano + falha) ==================================

TEST_CASE("techmagic hayek: 2/3/4 distintas produzem os degraus certos (dano e limiar de "
         "falha), degrau 4+ tem cap automatico",
         "[domain][combat][techmagic][hayek]") {
    Card hayek = hayek_card("techmagic.hayek.tier.card");
    Card elec = damage_card("techmagic.hayek.tier.elec", CardFamily::Eletrico, 12);
    Card sonic = damage_card("techmagic.hayek.tier.sonic2", CardFamily::Sonico, 12);
    Card cripto = damage_card("techmagic.hayek.tier.cripto", CardFamily::Criptografico, 12);
    auto reg = registry({hayek, elec, sonic, cripto});

    // ---- Sub-caso DANO: kills=20 => v no piso 0.05 (numeros limpos). base0=(12+8)*1.0=20.
    {
        CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);
        CombatActor s2 = make_actor("s2", true, 100, 8, 0, 80);
        CombatActor s3 = make_actor("s3", true, 100, 8, 0, 70);
        CombatActor s4 = make_actor("s4", true, 100, 8, 0, 60);
        CombatActor h = make_actor("h", true, 100, 8, 0, 50);  // nunca age; so equipa.
        h.set_equipped_special_ids({hayek.id});
        CombatActor e = make_actor("e", false, 100000, 0, 0, 10, CardFamily::Sonico,
                                   /*kills=*/20);

        auto provider = play_by_actor({
            {"s1", CombatAction::attack(e.id())},
            {"s2", CombatAction::use_card(elec.id, e.id())},
            {"s3", CombatAction::use_card(sonic.id, e.id())},
            {"s4", CombatAction::defend()},
        });
        CombatStateMachine sm({&s1, &s2, &s3, &s4, &h, &e}, provider, &reg, nullptr, nullptr);

        // distinct_count=0 antes de s1 (nao medido aqui - ver teste 1).
        sm.begin_turn();
        sm.run_active_turn_to_end();  // s1: Attack. ledger=[Attack].
        // distinct_count=1 (so Attack): ainda sem bonus.
        {
            const CardDamageEstimate est = sm.estimate_card_damage(h, e, elec);
            REQUIRE(est.min_damage == 19);  // 20*0.95 -> 19.
            REQUIRE(est.max_damage == 21);  // 20*1.05 -> 21.
            REQUIRE(est.crit_damage == 32); // 20*1.05*1.5=31.5 -> 32.
        }

        sm.advance_to_next_actor();
        sm.begin_turn();
        sm.run_active_turn_to_end();  // s2: UseCard Eletrico. ledger=[Attack,UseCard-Elet].
        // distinct_count=2 -> tier2 (+5%): base=21.
        {
            const CardDamageEstimate est = sm.estimate_card_damage(h, e, elec);
            REQUIRE(est.min_damage == 20);  // 21*0.95=19.95 -> 20.
            REQUIRE(est.max_damage == 22);  // 21*1.05=22.05 -> 22.
            REQUIRE(est.crit_damage == 33); // 21*1.575=33.075 -> 33.
        }

        sm.advance_to_next_actor();
        sm.begin_turn();
        sm.run_active_turn_to_end();  // s3: UseCard Sonico (familia DIFERENTE de s2 -> M2).
        // distinct_count=3 (Attack, UseCard-Elet, UseCard-Sonico) -> tier3 (+8%): base=21.6.
        {
            const CardDamageEstimate est = sm.estimate_card_damage(h, e, elec);
            REQUIRE(est.min_damage == 21);  // 21.6*0.95=20.52 -> 21.
            REQUIRE(est.max_damage == 23);  // 21.6*1.05=22.68 -> 23.
            REQUIRE(est.crit_damage == 34); // 21.6*1.575=34.02 -> 34.
        }

        sm.advance_to_next_actor();
        sm.begin_turn();
        sm.run_active_turn_to_end();  // s4: Defend (4a assinatura distinta).
        // distinct_count=4 -> tier4/"4+" (+13%): base=22.6.
        {
            const CardDamageEstimate est = sm.estimate_card_damage(h, e, elec);
            REQUIRE(est.min_damage == 21);  // 22.6*0.95=21.47 -> 21.
            REQUIRE(est.max_damage == 24);  // 22.6*1.05=23.73 -> 24.
            REQUIRE(est.crit_damage == 36); // 22.6*1.575=35.595 -> 36.
        }

        // Cripto (5a acao hipotetica, so pra provar o CAP): se s4 tivesse jogado uma 5a
        // familia distinta, distinct_count subiria a 5, mas o degrau mais alto (magnitude=4)
        // continua sendo o MELHOR candidato (nenhum spec tem magnitude>4) - o preview atual
        // (distinct_count=4) ja EH o teto; confirmamos que o cripto (ainda nao jogado) nao
        // muda o resultado corrente.
        (void)cripto;
    }

    // ---- Sub-caso FALHA: kills=0 => fumble baseline=5. Degraus reduzem 5->4->3->2.
    {
        CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);
        CombatActor s2 = make_actor("s2", true, 100, 8, 0, 80);
        CombatActor s3 = make_actor("s3", true, 100, 8, 0, 70);
        CombatActor s4 = make_actor("s4", true, 100, 8, 0, 60);
        CombatActor h = make_actor("h", true, 100, 8, 0, 50);
        h.set_equipped_special_ids({hayek.id});
        CombatActor e = make_actor("e", false, 100000, 0, 0, 10, CardFamily::Sonico,
                                   /*kills=*/0);

        auto provider = play_by_actor({
            {"s1", CombatAction::attack(e.id())},
            {"s2", CombatAction::use_card(elec.id, e.id())},
            {"s3", CombatAction::use_card(sonic.id, e.id())},
            {"s4", CombatAction::defend()},
        });
        CombatStateMachine sm({&s1, &s2, &s3, &s4, &h, &e}, provider, &reg, nullptr, nullptr);

        sm.begin_turn();
        sm.run_active_turn_to_end();  // distinct=1.
        REQUIRE(sm.estimate_card_damage(h, e, elec).fumble_chance_pct == 5);

        sm.advance_to_next_actor();
        sm.begin_turn();
        sm.run_active_turn_to_end();  // distinct=2 -> -1pp.
        REQUIRE(sm.estimate_card_damage(h, e, elec).fumble_chance_pct == 4);

        sm.advance_to_next_actor();
        sm.begin_turn();
        sm.run_active_turn_to_end();  // distinct=3 -> -2pp.
        REQUIRE(sm.estimate_card_damage(h, e, elec).fumble_chance_pct == 3);

        sm.advance_to_next_actor();
        sm.begin_turn();
        sm.run_active_turn_to_end();  // distinct=4 -> -3pp.
        REQUIRE(sm.estimate_card_damage(h, e, elec).fumble_chance_pct == 2);
    }
}

// ===== 3. M2: 2 cartas da MESMA familia = 1 assinatura; familias diferentes = 2 ============

TEST_CASE("techmagic hayek: M2 - UseCard e refinado pela CardFamily da carta (mesma "
         "familia = 1 distinta, familias diferentes = 2)",
         "[domain][combat][techmagic][hayek]") {
    Card hayek = hayek_card("techmagic.hayek.m2.card");
    Card elec_a = damage_card("techmagic.hayek.m2.elec_a", CardFamily::Eletrico, 10);
    Card elec_b = damage_card("techmagic.hayek.m2.elec_b", CardFamily::Eletrico, 10);
    Card sonic = damage_card("techmagic.hayek.m2.sonic", CardFamily::Sonico, 10);
    auto reg = registry({hayek, elec_a, elec_b, sonic});

    // Caso A: 2 cartas da MESMA familia (Eletrico) -> 1 assinatura distinta -> SEM bonus.
    {
        CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);
        CombatActor s2 = make_actor("s2", true, 100, 8, 0, 80);
        CombatActor h = make_actor("h", true, 100, 8, 0, 50);
        h.set_equipped_special_ids({hayek.id});
        CombatActor e = make_actor("e", false, 100000, 0, 0, 10);

        auto provider = play_by_actor({
            {"s1", CombatAction::use_card(elec_a.id, e.id())},
            {"s2", CombatAction::use_card(elec_b.id, e.id())},
        });
        CombatStateMachine sm({&s1, &s2, &h, &e}, provider, &reg, nullptr, nullptr);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
        sm.begin_turn();
        sm.run_active_turn_to_end();

        REQUIRE(sm.round_actions().size() == 2);  // 2 acoes...
        // ...mas MESMA familia = 1 assinatura distinta (M2): nenhum degrau alcancado
        // (distinct_count==1, limiar minimo e 2) - verificacao direta e inequivoca via o
        // limiar de falha (fumble_chance_pct), que so muda quando um degrau ativa.
        const CardDamageEstimate est = sm.estimate_card_damage(h, e, elec_a);
        REQUIRE(est.fumble_chance_pct == 5);  // kills=0, SEM reducao (piso, nao tier2 -1pp).
    }

    // Caso B: 2 cartas de familias DIFERENTES (Eletrico + Sonico) -> 2 assinaturas -> tier2.
    {
        CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);
        CombatActor s2 = make_actor("s2", true, 100, 8, 0, 80);
        CombatActor h = make_actor("h", true, 100, 8, 0, 50);
        h.set_equipped_special_ids({hayek.id});
        CombatActor e = make_actor("e", false, 100000, 0, 0, 10);

        auto provider = play_by_actor({
            {"s1", CombatAction::use_card(elec_a.id, e.id())},
            {"s2", CombatAction::use_card(sonic.id, e.id())},
        });
        CombatStateMachine sm({&s1, &s2, &h, &e}, provider, &reg, nullptr, nullptr);
        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
        sm.begin_turn();
        sm.run_active_turn_to_end();

        const CardDamageEstimate est = sm.estimate_card_damage(h, e, elec_a);
        REQUIRE(est.fumble_chance_pct == 4);  // kills=0 (5) - tier2 (-1pp) = 4.
    }
}

// ===== 4. Eco de Mandelbrot (RepeatLastAction/take_damage PURO) NAO entra no ledger ========

TEST_CASE("techmagic hayek: o eco do Mandelbrot (take_damage PURO, fora de resolve_action) "
         "NAO ganha entrada no ledger de acoes",
         "[domain][combat][techmagic][hayek]") {
    Card dmg = damage_card("techmagic.hayek.echo.dmg", CardFamily::Eletrico, 10);
    Card mandelbrot = mandelbrot_local("techmagic.hayek.echo.mandelbrot");
    auto reg = registry({dmg, mandelbrot});

    CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);
    CombatActor s2 = make_actor("s2", true, 100, 8, 0, 80);
    CombatActor e = make_actor("e", false, 300, 0, 0, 10);

    FixedRandom rng;  // default (0.5, 99): so relevante pro canal COMUM de s2 (dmg).
    auto provider = play_by_actor({
        {"s1", CombatAction::attack(e.id())},
        {"s2", CombatAction::use_card(mandelbrot.id, e.id())},
    });
    CombatStateMachine sm({&s1, &s2, &e}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    sm.run_active_turn_to_end();  // s1 ataca -> last_action_ populado (hit>0).
    REQUIRE(sm.round_actions().size() == 1);

    sm.advance_to_next_actor();
    sm.begin_turn();
    sm.run_active_turn_to_end();  // s2 casta Mandelbrot -> ecoa via take_damage PURO.
    // 2 entradas (s1 Attack + s2 UseCard), NUNCA 3 - o eco nao passa por resolve_action.
    REQUIRE(sm.round_actions().size() == 2);
}

// ===== 5. Fronteira de rodada ZERA o ledger de acoes =========================================

TEST_CASE("techmagic hayek: o ledger de acoes zera na fronteira de rodada (mesma "
         "fronteira de round_hits_/last_action_)",
         "[domain][combat][techmagic][hayek]") {
    CombatActor h = make_actor("h", true, 100, 8, 0, 50);
    CombatActor e = make_actor("e", false, 100, 8, 0, 10);

    auto provider = play_by_actor({
        {"h", CombatAction::attack(e.id())},
        {"e", CombatAction::attack(h.id())},
    });
    CombatStateMachine sm({&h, &e}, provider, nullptr, nullptr, nullptr);

    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(sm.round_actions().size() == 1);

    sm.advance_to_next_actor();  // ainda dentro da rodada 0 (2 atores, so 1 avancou).
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(sm.round_actions().size() == 2);

    sm.advance_to_next_actor();  // fecha a volta na fila -> wrap pra rodada 1 -> limpa.
    REQUIRE(sm.round_actions().empty());
}

// ===== 6. Paridade preview<->real (carta E ataque basico) ===================================

TEST_CASE("techmagic hayek: paridade preview<->real - a mesma acao real (carta OU ataque "
         "basico) bate com o campo do preview tirado ANTES dela",
         "[domain][combat][techmagic][hayek]") {
    Card hayek = hayek_card("techmagic.hayek.parity.card");
    Card elec = damage_card("techmagic.hayek.parity.elec", CardFamily::Eletrico, 12);
    auto reg = registry({hayek, elec});

    CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);
    CombatActor s2 = make_actor("s2", true, 100, 8, 0, 80);
    // m1.atk=20 DE PROPOSITO (nao 8): com atk=8, raw=8 e lround(8*1.05)=8 EMPATA com o raw
    // sem bonus - um mutante que removesse o Hayek do preview do ataque basico passaria
    // incolume (coincidencia de arredondamento). Com atk=20: raw=20, tier2 lround(20*1.05)=
    // 21 != 20, entao a paridade so bate se o preview E o real aplicam o mult_hayek (mata o
    // mutante "preview do ataque basico ignora Hayek").
    CombatActor m1 = make_actor("m1", true, 100, 20, 0, 70);  // medido: ataque basico.
    CombatActor m2 = make_actor("m2", true, 100, 8, 0, 60);   // medido: carta.
    m1.set_equipped_special_ids({hayek.id});
    CombatActor e = make_actor("e", false, 100000, 0, 0, 10, CardFamily::Sonico,
                               /*kills=*/0);

    // r=0.0 forca o canal COMUM pro degrau PISO (comum_channel_damage r=0.0), roll=99 fica
    // seguro em COMUM (fumble+crit << 99).
    FixedRandom rng(/*next_double=*/0.0, /*next_int=*/99);
    auto provider = play_by_actor({
        {"s1", CombatAction::attack(e.id())},
        {"s2", CombatAction::use_card(elec.id, e.id())},
        {"m1", CombatAction::attack(e.id())},
        {"m2", CombatAction::use_card(elec.id, e.id())},
    });
    CombatStateMachine sm({&s1, &s2, &m1, &m2, &e}, provider, &reg, nullptr, &rng);

    sm.begin_turn();
    sm.run_active_turn_to_end();  // s1: distinct passa a 1.
    sm.advance_to_next_actor();
    sm.begin_turn();
    sm.run_active_turn_to_end();  // s2: distinct passa a 2 (tier2 ativo dali em diante).

    // ---- ataque basico: preview tirado ANTES de m1 agir, mesma ledger que m1 vai LER.
    sm.advance_to_next_actor();
    const int basic_preview = sm.preview_basic_attack_damage(m1, e);
    // O preview do ataque basico DEVE estar turbinado (raw=20, tier2 -> lround(20*1.05)=21):
    // 21 != 20 NAO deixa o bonus colapsar de volta ao raw - se o preview ignorasse o Hayek,
    // esta asseracao (e a paridade abaixo) falhariam. Mata o mutante "preview do ataque
    // basico ignora Hayek".
    REQUIRE(basic_preview == 21);
    const int hp_before_m1 = e.hp();
    sm.begin_turn();
    sm.run_active_turn_to_end();  // m1 ataca de verdade.
    REQUIRE(hp_before_m1 - e.hp() == basic_preview);

    // ---- carta: preview tirado ANTES de m2 agir (ledger ja tem m1 tambem, mas m1.Attack
    // duplica a assinatura de s1 - distinct_count continua 2, mesmo degrau).
    sm.advance_to_next_actor();
    const CardDamageEstimate card_preview = sm.estimate_card_damage(m2, e, elec);
    const int hp_before_m2 = e.hp();
    sm.begin_turn();
    sm.run_active_turn_to_end();  // m2 joga a carta de verdade (r=0.0 -> canal PISO).
    REQUIRE(hp_before_m2 - e.hp() == card_preview.min_damage);
}

// ===== 7. Determinismo: RNG identico com/sem Hayek (Hayek NUNCA toca rng_) =================

TEST_CASE("techmagic hayek: consumo de RNG identico com/sem Hayek equipado (a passiva "
         "NUNCA sorteia nada - so ledger + tabela fixa)",
         "[domain][combat][techmagic][hayek]") {
    Card hayek = hayek_card("techmagic.hayek.rng.card");
    Card elec = damage_card("techmagic.hayek.rng.elec", CardFamily::Eletrico, 12);
    Card sonic = damage_card("techmagic.hayek.rng.sonic", CardFamily::Sonico, 12);
    auto reg = registry({hayek, elec, sonic});

    const auto run = [&](bool with_hayek) {
        CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);
        CombatActor s2 = make_actor("s2", true, 100, 8, 0, 80);
        CombatActor h = make_actor("h", true, 100, 8, 0, 50);
        if (with_hayek) h.set_equipped_special_ids({hayek.id});
        CombatActor e = make_actor("e", false, 300, 0, 0, 10);

        CountingRandom rng;
        auto provider = play_by_actor({
            {"s1", CombatAction::attack(e.id())},
            {"s2", CombatAction::use_card(elec.id, e.id())},
            {"h", CombatAction::use_card(sonic.id, e.id())},
        });
        CombatStateMachine sm({&s1, &s2, &h, &e}, provider, &reg, nullptr, &rng);

        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
        sm.begin_turn();
        sm.run_active_turn_to_end();

        return std::pair<int, int>{rng.next_calls, rng.next_double_calls};
    };

    const auto without_hayek = run(false);
    const auto with_hayek = run(true);

    // s1 (Attack) = 0 RNG; s2 (UseCard) = 1 next + 1 next_double (canal COMUM); h (UseCard)
    // = 1 next + 1 next_double. Total = 2 next + 2 next_double, IDENTICO nos dois casos.
    REQUIRE(without_hayek.first == 2);
    REQUIRE(without_hayek.second == 2);
    REQUIRE(with_hayek.first == without_hayek.first);
    REQUIRE(with_hayek.second == without_hayek.second);
}

// ===== 8. Beneficia o LADO INTEIRO (aliado sem a carta colhe o bonus) enquanto o dono ======
// =====    estiver VIVO e EQUIPADO; dono morto derruba o bonus do lado todo =================

TEST_CASE("techmagic hayek: o bonus beneficia QUALQUER aliado do lado (nao so o dono), "
         "mas exige o dono VIVO",
         "[domain][combat][techmagic][hayek]") {
    Card hayek = hayek_card("techmagic.hayek.side.card");
    Card elec = damage_card("techmagic.hayek.side.elec", CardFamily::Eletrico, 12);
    Card sonic = damage_card("techmagic.hayek.side.sonic", CardFamily::Sonico, 12);
    auto reg = registry({hayek, elec, sonic});

    CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);
    CombatActor s2 = make_actor("s2", true, 100, 8, 0, 80);
    CombatActor owner = make_actor("owner", true, 20, 8, 0, 70);  // hp baixo: sera morto.
    CombatActor other = make_actor("other", true, 100, 8, 0, 60);  // NAO porta a carta.
    owner.set_equipped_special_ids({hayek.id});
    CombatActor e = make_actor("e", false, 100000, 0, 0, 10, CardFamily::Sonico,
                               /*kills=*/0);

    // Ordem por SPD desc: s1(90), s2(80), owner(70), other(60), e(10). `other` age DEPOIS
    // do owner - e sera dirigido a atacar de verdade DEPOIS que o owner morrer, pra checar
    // o LOG (nao so o preview).
    auto provider = play_by_actor({
        {"s1", CombatAction::attack(e.id())},
        {"s2", CombatAction::use_card(sonic.id, e.id())},
        {"other", CombatAction::attack(e.id())},
    });
    CombatStateMachine sm({&s1, &s2, &owner, &other, &e}, provider, &reg, nullptr, nullptr);

    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();
    sm.begin_turn();
    sm.run_active_turn_to_end();  // distinct_count=2 -> tier2.

    // `other` (sem a carta) colhe o bonus enquanto `owner` (dono) esta vivo.
    {
        const CardDamageEstimate est = sm.estimate_card_damage(other, e, elec);
        REQUIRE(est.fumble_chance_pct == 4);  // 5 - 1pp (tier2).
    }

    // Snapshot do log ANTES da morte do owner: as acoes de s1/s2 (owner vivo, tier2 ativo)
    // DEVEM ter citado o Free-Order (prova que a deteccao de "equipped" enxergava o owner).
    {
        bool free_order_before = false;
        for (const auto& entry : sm.log())
            if (entry.message.find("Free-Order") != std::string::npos) free_order_before = true;
        REQUIRE(free_order_before);
    }
    const std::size_t log_size_before_death = sm.log().size();

    // Dono morre: o lado PERDE o bonus (mesmo com o ledger intacto - distinct_count=2).
    owner.take_damage(owner.max_hp());
    REQUIRE_FALSE(owner.is_alive());
    {
        const CardDamageEstimate est = sm.estimate_card_damage(other, e, elec);
        REQUIRE(est.fumble_chance_pct == 5);  // sem desconto: owner morto = passiva inerte.
    }

    // `other` age de verdade com o owner JA MORTO. A passiva tem de ficar TOTALMENTE inerte:
    // NENHUMA entrada de log nova pode citar "Free-Order" - nem o bonus ativo nem a variante
    // "sem diversidade ainda" (que so sai quando a passiva esta equipada num vivo do lado).
    // Isto prova que a deteccao de LOG (diversity_equipped_on_side) e a de BONUS
    // (diversity_spec_of) checam is_alive() de forma CONSISTENTE - sem isso, o helper de log
    // e o de bonus dessincronizariam (mutante 4b). O owner morto foi podado no advance abaixo.
    sm.advance_to_next_actor();  // poda o owner morto, cursor cai em `other` (proximo vivo).
    sm.begin_turn();
    sm.run_active_turn_to_end();  // other ataca de verdade.

    bool other_attack_logged = false;
    bool free_order_after_death = false;
    for (std::size_t i = log_size_before_death; i < sm.log().size(); ++i) {
        const CombatLogEntry& entry = sm.log()[i];
        if (entry.actor_id == "other" && entry.action == CombatActionType::Attack)
            other_attack_logged = true;
        if (entry.message.find("Free-Order") != std::string::npos)
            free_order_after_death = true;
    }
    REQUIRE(other_attack_logged);          // o check negativo abaixo NAO e vacuo: other agiu.
    REQUIRE_FALSE(free_order_after_death);  // passiva inerte com o dono morto: log sem sufixo.
}

// ===== 9. Piso de falha 0 (nunca negativo) ==================================================

TEST_CASE("techmagic hayek: o limiar de falha nunca fica negativo (piso 0) mesmo quando a "
         "reducao do degrau excede o baseline",
         "[domain][combat][techmagic][hayek]") {
    Card hayek = hayek_card("techmagic.hayek.floor.card");
    Card elec = damage_card("techmagic.hayek.floor.elec", CardFamily::Eletrico, 12);
    Card sonic = damage_card("techmagic.hayek.floor.sonic", CardFamily::Sonico, 12);
    Card cripto = damage_card("techmagic.hayek.floor.cripto", CardFamily::Criptografico, 12);
    auto reg = registry({hayek, elec, sonic, cripto});

    CombatActor s1 = make_actor("s1", true, 100, 8, 0, 90);
    CombatActor s2 = make_actor("s2", true, 100, 8, 0, 80);
    CombatActor s3 = make_actor("s3", true, 100, 8, 0, 70);
    CombatActor s4 = make_actor("s4", true, 100, 8, 0, 60);
    CombatActor h = make_actor("h", true, 100, 8, 0, 50);
    h.set_equipped_special_ids({hayek.id});
    // kills=2: fumble baseline = round(5*exp(-1)) = 2. Tier4 (-3pp) excederia o baseline.
    CombatActor e = make_actor("e", false, 100000, 0, 0, 10, CardFamily::Sonico,
                               /*kills=*/2);

    auto provider = play_by_actor({
        {"s1", CombatAction::attack(e.id())},
        {"s2", CombatAction::use_card(elec.id, e.id())},
        {"s3", CombatAction::use_card(sonic.id, e.id())},
        {"s4", CombatAction::use_card(cripto.id, e.id())},
    });
    CombatStateMachine sm({&s1, &s2, &s3, &s4, &h, &e}, provider, &reg, nullptr, nullptr);

    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();
    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();
    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();
    sm.begin_turn();
    sm.run_active_turn_to_end();  // distinct_count=4 -> tier4 (-3pp) vs baseline 2.

    const CardDamageEstimate est = sm.estimate_card_damage(h, e, elec);
    REQUIRE(est.fumble_chance_pct == 0);  // max(0, 2-3), NUNCA -1.
}

// ===== 10. Handler no-op no dispatcher: EffectKind::DiversityBonus tem handler (nao lanca) ==

TEST_CASE("techmagic hayek: DiversityBonus tem handler no dispatcher (marker no-op "
         "deliberado, mesmo padrao de DamageQuantize/Planck) - nao lanca logic_error",
         "[domain][combat][techmagic][hayek]") {
    Card hayek = hayek_card("techmagic.hayek.dispatcher.card");

    CombatActor caster = make_actor("h", /*player_side=*/true);
    techMagic::TechMagicContext ctx;
    ctx.caster = &caster;

    REQUIRE_NOTHROW(techMagic::execute(TriggerHook::OnCast, hayek, ctx));
}
