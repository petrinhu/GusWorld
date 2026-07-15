// combat_actor_test.cpp
//
// Spec executavel (Catch2 v3) do CombatActor (modelo MUTAVEL de combatente), portado de
// engine/foundation/turn_combat/CombatActor.cs (origem canonica) e dos casos de
// engine/tests/turn_combat/CombatActorTests.cs (xUnit = SPEC). POCO puro, ZERO Qt,
// headless. Marco M5 (chunk 2: atores/filas do combate). Paridade 1:1 com o C#.
//
// CombatActor e sealed class de REFERENCIA no C# (entidade com identidade + ciclo de
// vida mutavel). No C++ portamos como classe mutavel; a fila e a FSM guardam ponteiros
// estaveis (CombatActor*) pra mutar in-place, mesma semantica de referencia do C#.
//
// Invariantes garantidos no construtor (fail-fast): Id nao vazio; MaxHp > 0;
// Atk/Def/Spd >= 0; Hp inicia cheio (= MaxHp). C# lanca ArgumentException /
// ArgumentOutOfRangeException -> aqui std::invalid_argument / std::out_of_range.
//
// Cross-ref: engine/foundation/turn_combat/CombatActor.cs;
//            engine/tests/turn_combat/CombatActorTests.cs;
//            docs/design/mecanicas/combat.md secao 5/6/9/16/17.

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"

using namespace gus::domain::combat;

namespace {

// Espelha o helper MakeActor de CombatActorTests.cs.
CombatActor make_actor(int hp = 30, int spd = 10, bool player_side = true) {
    return CombatActor(
        /*id=*/"actor_1",
        /*display_name=*/"Teste",
        /*max_hp=*/hp,
        /*atk=*/8,
        /*def=*/3,
        /*spd=*/spd,
        /*family=*/CardFamily::Eletrico,
        /*is_player_side=*/player_side);
}

}  // namespace

// ---- Construtor (Construtor_inicializa_hp_cheio) ----------------------------------

TEST_CASE("combat_actor: construtor inicializa hp cheio",
          "[domain][combat][actor]") {
    const auto a = make_actor(/*hp=*/30);
    REQUIRE(a.hp() == 30);
    REQUIRE(a.max_hp() == 30);
    REQUIRE(a.is_alive());
}

TEST_CASE("combat_actor: construtor rejeita max_hp nao positivo",
          "[domain][combat][actor]") {
    REQUIRE_THROWS_AS(
        CombatActor("x", "x", /*max_hp=*/0, 1, 0, 1, CardFamily::Eletrico, true),
        std::out_of_range);
}

TEST_CASE("combat_actor: construtor rejeita id vazio",
          "[domain][combat][actor]") {
    REQUIRE_THROWS_AS(
        CombatActor("", "x", /*max_hp=*/10, 1, 0, 1, CardFamily::Eletrico, true),
        std::invalid_argument);
    // id so de espacos tambem e vazio (IsNullOrWhiteSpace no C#).
    REQUIRE_THROWS_AS(
        CombatActor("   ", "x", /*max_hp=*/10, 1, 0, 1, CardFamily::Eletrico, true),
        std::invalid_argument);
}

TEST_CASE("combat_actor: construtor rejeita atributos negativos",
          "[domain][combat][actor]") {
    REQUIRE_THROWS_AS(
        CombatActor("x", "x", 10, /*atk=*/-1, 0, 1, CardFamily::Eletrico, true),
        std::out_of_range);
    REQUIRE_THROWS_AS(
        CombatActor("x", "x", 10, 0, /*def=*/-1, 1, CardFamily::Eletrico, true),
        std::out_of_range);
    REQUIRE_THROWS_AS(
        CombatActor("x", "x", 10, 0, 0, /*spd=*/-1, CardFamily::Eletrico, true),
        std::out_of_range);
}

TEST_CASE("combat_actor: construtor rejeita knowledge_kills negativo",
          "[domain][combat][actor]") {
    REQUIRE_THROWS_AS(
        CombatActor("x", "x", 10, 0, 0, 1, CardFamily::Eletrico, true,
                    /*is_boss=*/false, /*knowledge_kills=*/-1),
        std::out_of_range);
}

// ---- TakeDamage (TakeDamage_reduz_hp_e_nao_passa_de_zero) --------------------------

TEST_CASE("combat_actor: take_damage reduz hp e nao passa de zero",
          "[domain][combat][actor]") {
    auto a = make_actor(/*hp=*/10);
    a.take_damage(4);
    REQUIRE(a.hp() == 6);
    a.take_damage(100);
    REQUIRE(a.hp() == 0);
    REQUIRE_FALSE(a.is_alive());
}

TEST_CASE("combat_actor: take_damage negativo e rejeitado",
          "[domain][combat][actor]") {
    auto a = make_actor();
    REQUIRE_THROWS_AS(a.take_damage(-1), std::out_of_range);
}

// ---- Heal (Heal_aumenta_hp_com_cap_no_maxHp) --------------------------------------

TEST_CASE("combat_actor: heal aumenta hp com cap no max_hp",
          "[domain][combat][actor]") {
    auto a = make_actor(/*hp=*/20);
    a.take_damage(15);  // hp = 5
    a.heal(3);
    REQUIRE(a.hp() == 8);
    a.heal(1000);
    REQUIRE(a.hp() == 20);
}

TEST_CASE("combat_actor: heal negativo e rejeitado",
          "[domain][combat][actor]") {
    auto a = make_actor();
    REQUIRE_THROWS_AS(a.heal(-1), std::out_of_range);
}

// ---- AP/Mana (Ap_inicia_em_zero_ate_TurnStart) ------------------------------------

TEST_CASE("combat_actor: ap inicia em zero ate turn start",
          "[domain][combat][actor]") {
    // AP e preparado no TurnStart pela FSM, nao na construcao.
    const auto a = make_actor();
    REQUIRE(a.ap() == 0);
}

// ---- Defaults canonicos do construtor ---------------------------------------------

TEST_CASE("combat_actor: defaults canonicos (max_ap, flags, knowledge)",
          "[domain][combat][actor]") {
    const auto a = make_actor();
    REQUIRE(a.max_ap() == combat_constants::kBaseApPerTurn);  // 3
    REQUIRE(a.is_player_side());
    REQUIRE_FALSE(a.is_boss());
    REQUIRE_FALSE(a.is_scanned());
    REQUIRE_FALSE(a.is_universal_compiler());
    REQUIRE(a.knowledge_kills() == 0);
    REQUIRE(a.status_effects().empty());
}

// ---- RefreshResourcesForTurn (ramp de mana, secao 5) ------------------------------
// Comportamento exercitado pela FSM no C#; aqui validamos a regra de recurso pura.

TEST_CASE("combat_actor: refresh_resources_for_turn aplica ramp de mana",
          "[domain][combat][actor]") {
    auto a = make_actor();
    a.refresh_resources_for_turn(/*round_index=*/0);
    REQUIRE(a.ap() == combat_constants::kBaseApPerTurn);   // reset = MaxAp = 3
    REQUIRE(a.max_mana() == combat_constants::kBaseMana);  // 2 + 0 = 2
    REQUIRE(a.mana() == 2);

    a.refresh_resources_for_turn(/*round_index=*/3);
    REQUIRE(a.max_mana() == 5);  // 2 + 3
    REQUIRE(a.mana() == 5);

    a.refresh_resources_for_turn(/*round_index=*/100);
    REQUIRE(a.max_mana() == combat_constants::kManaCap);  // cap 8
    REQUIRE(a.mana() == 8);
}

// ---- SpendAp / SpendMana ----------------------------------------------------------

TEST_CASE("combat_actor: spend_ap consome e falha se insuficiente",
          "[domain][combat][actor]") {
    auto a = make_actor();
    a.refresh_resources_for_turn(0);  // Ap = 3
    a.spend_ap(2);
    REQUIRE(a.ap() == 1);
    REQUIRE_THROWS_AS(a.spend_ap(2), std::logic_error);
    REQUIRE_THROWS_AS(a.spend_ap(-1), std::out_of_range);
}

TEST_CASE("combat_actor: spend_mana consome e falha se insuficiente",
          "[domain][combat][actor]") {
    auto a = make_actor();
    a.refresh_resources_for_turn(0);  // Mana = 2
    a.spend_mana(1);
    REQUIRE(a.mana() == 1);
    REQUIRE_THROWS_AS(a.spend_mana(5), std::logic_error);
    REQUIRE_THROWS_AS(a.spend_mana(-1), std::out_of_range);
}

// ---- ReduceDef (Corrode, secao 9) -------------------------------------------------

TEST_CASE("combat_actor: reduce_def reduz com clamp em zero",
          "[domain][combat][actor]") {
    auto a = make_actor();  // def = 3
    a.reduce_def(1);
    REQUIRE(a.def() == 2);
    a.reduce_def(100);
    REQUIRE(a.def() == 0);
    REQUIRE_THROWS_AS(a.reduce_def(-1), std::out_of_range);
}

// ---- Status: AddStatus + StackRule (secao 9) --------------------------------------

TEST_CASE("combat_actor: add_status sem existente apenas adiciona",
          "[domain][combat][actor]") {
    auto a = make_actor();
    a.add_status(StatusEffect{StatusId::Poison, /*magnitude=*/3, /*duration=*/2,
                              StackRule::Replace, CardFamily::Bioquimico});
    REQUIRE(a.status_effects().size() == 1);
    REQUIRE(a.status_effects()[0].id == StatusId::Poison);
    REQUIRE(a.status_effects()[0].magnitude == 3);
}

TEST_CASE("combat_actor: add_status Replace troca o existente",
          "[domain][combat][actor]") {
    auto a = make_actor();
    a.add_status(StatusEffect{StatusId::Poison, 3, 2, StackRule::Replace});
    a.add_status(StatusEffect{StatusId::Poison, 7, 5, StackRule::Replace});
    REQUIRE(a.status_effects().size() == 1);
    REQUIRE(a.status_effects()[0].magnitude == 7);
    REQUIRE(a.status_effects()[0].duration == 5);
}

TEST_CASE("combat_actor: add_status Refresh eleva a maior duration",
          "[domain][combat][actor]") {
    auto a = make_actor();
    a.add_status(StatusEffect{StatusId::Poison, 3, 5, StackRule::Refresh});
    a.add_status(StatusEffect{StatusId::Poison, 9, 2, StackRule::Refresh});
    REQUIRE(a.status_effects().size() == 1);
    // mantem o existente, eleva duration ao maior (max(5,2)=5), magnitude do existente.
    REQUIRE(a.status_effects()[0].magnitude == 3);
    REQUIRE(a.status_effects()[0].duration == 5);
}

TEST_CASE("combat_actor: add_status StackMagnitude soma magnitude",
          "[domain][combat][actor]") {
    auto a = make_actor();
    a.add_status(StatusEffect{StatusId::Poison, 3, 2, StackRule::StackMagnitude});
    a.add_status(StatusEffect{StatusId::Poison, 4, 9, StackRule::StackMagnitude});
    REQUIRE(a.status_effects().size() == 1);
    REQUIRE(a.status_effects()[0].magnitude == 7);  // 3 + 4
}

TEST_CASE("combat_actor: add_status StackDuration soma duration",
          "[domain][combat][actor]") {
    auto a = make_actor();
    a.add_status(StatusEffect{StatusId::Poison, 3, 2, StackRule::StackDuration});
    a.add_status(StatusEffect{StatusId::Poison, 9, 4, StackRule::StackDuration});
    REQUIRE(a.status_effects().size() == 1);
    REQUIRE(a.status_effects()[0].duration == 6);  // 2 + 4
}

TEST_CASE("combat_actor: remove_status remove por id",
          "[domain][combat][actor]") {
    auto a = make_actor();
    a.add_status(StatusEffect{StatusId::Poison, 3, 2, StackRule::Replace});
    REQUIRE(a.remove_status(StatusId::Poison));
    REQUIRE(a.status_effects().empty());
    REQUIRE_FALSE(a.remove_status(StatusId::Poison));  // ja removido
}

// ---- Shield: TakeDamage absorve antes do HP (secao 9, BUG-3) -----------------------

TEST_CASE("combat_actor: shield absorve dano antes do hp e depleta",
          "[domain][combat][actor]") {
    auto a = make_actor(/*hp=*/20);
    a.add_status(StatusEffect{StatusId::Shield, /*magnitude=*/5, /*duration=*/3, StackRule::Replace});
    // dano 3 < pool 5: absorve tudo, hp intacto, pool = 2.
    a.take_damage(3);
    REQUIRE(a.hp() == 20);
    const auto idx = a.index_of_status(StatusId::Shield);
    REQUIRE(idx >= 0);
    REQUIRE(a.status_effects()[idx].magnitude == 2);
    // dano 5 > pool 2: absorve 2 (pool zera/remove Shield), 3 vazam pro hp.
    a.take_damage(5);
    REQUIRE(a.hp() == 17);
    REQUIRE(a.index_of_status(StatusId::Shield) < 0);  // shield removido ao depletar
}

// ---- DrainStatusChanges (secao 16) ------------------------------------------------

TEST_CASE("combat_actor: drain_status_changes consome uma unica vez",
          "[domain][combat][actor]") {
    auto a = make_actor();
    a.add_status(StatusEffect{StatusId::Poison, 3, 2, StackRule::Replace});  // gera Applied
    const auto first = a.drain_status_changes();
    REQUIRE(first.size() == 1);
    REQUIRE(first[0].kind == StatusChangeKind::Applied);
    REQUIRE(first[0].id == StatusId::Poison);
    // segunda drenagem: buffer vazio.
    REQUIRE(a.drain_status_changes().empty());
}

// ---- IsBuff (classificacao por exclusao, secao 9) ---------------------------------

TEST_CASE("combat_actor: is_buff classifica por exclusao dos nao-buffs",
          "[domain][combat][actor]") {
    // Buffs (beneficos): Shield, Regen, Haste.
    REQUIRE(CombatActor::is_buff(StatusId::Shield));
    REQUIRE(CombatActor::is_buff(StatusId::Regen));
    REQUIRE(CombatActor::is_buff(StatusId::Haste));
    // Nao-buffs (debuffs + neutro Decrypt).
    REQUIRE_FALSE(CombatActor::is_buff(StatusId::Poison));
    REQUIRE_FALSE(CombatActor::is_buff(StatusId::Stun));
    REQUIRE_FALSE(CombatActor::is_buff(StatusId::Slow));
    REQUIRE_FALSE(CombatActor::is_buff(StatusId::Decrypt));
    // SobrecargaTermica: debuff eletrico (DoT+Slow, cartas-technomagik.md secao 5.1).
    // Regressao (achado ADR-016 Balde B/Faraday 2026-07-15): faltava na lista de
    // nao-buffs, o que classificava como BUFF por exclusao e furava o portao de
    // imunidade EM-Shield/F-1 (que so trava DEBUFF de familia Eletrico).
    REQUIRE_FALSE(CombatActor::is_buff(StatusId::SobrecargaTermica));
    // Resfriamento e BlindagemEM SAO buffs (utilitario/imunidade, secao 5.2 + ADR-016
    // Balde B) - continuam corretos por exclusao, sem entrar na lista de nao-buffs.
    REQUIRE(CombatActor::is_buff(StatusId::Resfriamento));
    REQUIRE(CombatActor::is_buff(StatusId::BlindagemEM));
}

// ---- ApplyStatDelta / RevertStatDelta (Break/Haste/Slow, secao 9) ------------------

TEST_CASE("combat_actor: apply/revert stat delta de Break restaura Def",
          "[domain][combat][actor]") {
    auto a = make_actor();  // def = 3
    // Break: defDelta -2.
    REQUIRE(a.apply_stat_delta(StatusId::Break, /*def_delta=*/-2, /*spd_delta=*/0));
    REQUIRE(a.def() == 1);
    // idempotente: 2o apply nao reaplica.
    REQUIRE_FALSE(a.apply_stat_delta(StatusId::Break, -2, 0));
    REQUIRE(a.def() == 1);
    // revert restaura Def (retorna false: Break nao mexe em SPD).
    REQUIRE_FALSE(a.revert_stat_delta(StatusId::Break));
    REQUIRE(a.def() == 3);
}

TEST_CASE("combat_actor: apply/revert stat delta de Haste mexe em SPD",
          "[domain][combat][actor]") {
    auto a = make_actor(/*hp=*/30, /*spd=*/10);
    REQUIRE(a.apply_stat_delta(StatusId::Haste, /*def_delta=*/0, /*spd_delta=*/+5));
    REQUIRE(a.spd() == 15);
    // revert de SPD retorna true (caller recomputa a fila).
    REQUIRE(a.revert_stat_delta(StatusId::Haste));
    REQUIRE(a.spd() == 10);
}

// ---- ExpireElapsedStatuses (TurnEnd, secao 9) -------------------------------------

TEST_CASE("combat_actor: expire_elapsed_statuses remove duration <= 0",
          "[domain][combat][actor]") {
    auto a = make_actor();
    a.add_status(StatusEffect{StatusId::Poison, 3, /*duration=*/0, StackRule::Replace});
    a.add_status(StatusEffect{StatusId::Regen, 2, /*duration=*/2, StackRule::Replace});
    a.drain_status_changes();  // limpa Applied
    const bool spd_changed = a.expire_elapsed_statuses();
    REQUIRE_FALSE(spd_changed);  // nenhum status de SPD expirou
    REQUIRE(a.status_effects().size() == 1);
    REQUIRE(a.status_effects()[0].id == StatusId::Regen);
    // gerou um Expired pro Poison.
    const auto changes = a.drain_status_changes();
    REQUIRE(changes.size() == 1);
    REQUIRE(changes[0].kind == StatusChangeKind::Expired);
    REQUIRE(changes[0].id == StatusId::Poison);
}
