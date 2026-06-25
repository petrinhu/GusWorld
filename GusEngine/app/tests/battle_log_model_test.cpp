// GusEngine/app/tests/battle_log_model_test.cpp
//
// Catch2 (headless) do MODELO PURO do log (M5, incremento 3, D7). Prova, SEM SDL: a
// classificacao de CombatLogEntry/StatusEffectChange em categoria de cor, o filtro de
// NOTAVEIS (D7: nao todo hit), e o corte pro tamanho da caixa.

#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "gus/app/screens/battle_log_model.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"

using gus::app::screens::build_log_lines;
using gus::app::screens::classify;
using gus::app::screens::is_notable;
using gus::app::screens::LogLineKind;
using gus::domain::combat::CombatActionType;
using gus::domain::combat::CombatLogEntry;
using gus::domain::combat::StatusChangeKind;
using gus::domain::combat::StatusEffectChange;
using gus::domain::combat::StatusId;

namespace {

CombatLogEntry attack_entry(int value, std::string msg) {
    CombatLogEntry e;
    e.actor_id = "atacante";
    e.action = CombatActionType::Attack;
    e.target_id = "alvo";
    e.value = value;
    e.message = std::move(msg);
    return e;
}

}  // namespace

TEST_CASE("log: hit comum pequeno NAO e notavel (D7)", "[battle_log]") {
    // Dano baixo, sem marcador, sem sistema: nao entra na caixa (numero flutuante cobre).
    REQUIRE_FALSE(is_notable(attack_entry(5, "atacante ataca alvo por 5.")));
}

TEST_CASE("log: dano grande e CRITICO sao notaveis", "[battle_log]") {
    REQUIRE(is_notable(attack_entry(25, "atacante ataca alvo por 25.")));  // >= piso
    REQUIRE(is_notable(attack_entry(7, "compila X em alvo por 7. [CRITICO]")));
    REQUIRE(is_notable(attack_entry(0, "compila X em alvo por 0. FALHA DE COMPILACAO")));
}

TEST_CASE("log: mensagens de sistema sao notaveis e classificam como System",
          "[battle_log]") {
    CombatLogEntry combo;
    combo.action = CombatActionType::UseCard;
    combo.value = 0;
    combo.message = "COMPILADO: Descarga Tripla";
    REQUIRE(is_notable(combo));
    REQUIRE(classify(combo).kind == LogLineKind::System);

    // Scan/Gambito/Flee sempre ecoam (sistema/informacao).
    CombatLogEntry scan;
    scan.action = CombatActionType::Scan;
    scan.message = "atacante scaneia alvo: HP 30";
    REQUIRE(is_notable(scan));
    REQUIRE(classify(scan).kind == LogLineKind::System);
}

TEST_CASE("log: dano comum (sem sistema) classifica como Damage", "[battle_log]") {
    REQUIRE(classify(attack_entry(25, "atacante ataca alvo por 25.")).kind ==
            LogLineKind::Damage);
}

TEST_CASE("log: Defender classifica como Status (Shield)", "[battle_log]") {
    CombatLogEntry def;
    def.action = CombatActionType::Defend;
    def.message = "ator defende (Shield 5).";
    REQUIRE(is_notable(def));
    REQUIRE(classify(def).kind == LogLineKind::Status);
}

TEST_CASE("log: StatusEffectChange Applied classifica como Status", "[battle_log]") {
    StatusEffectChange c;
    c.actor_id = "alvo";
    c.id = StatusId::Poison;
    c.kind = StatusChangeKind::Applied;
    REQUIRE(classify(c).kind == LogLineKind::Status);
}

TEST_CASE("build_log_lines: filtra notaveis e corta pro tamanho da caixa",
          "[battle_log]") {
    std::vector<CombatLogEntry> log;
    log.push_back(attack_entry(3, "hit comum 3"));    // NAO notavel
    log.push_back(attack_entry(30, "hit grande 30")); // notavel (Damage)
    CombatLogEntry combo;
    combo.action = CombatActionType::UseCard;
    combo.message = "COMPILADO: X";
    log.push_back(combo);                              // notavel (System)

    std::vector<StatusEffectChange> changes;
    StatusEffectChange applied;
    applied.actor_id = "alvo";
    applied.id = StatusId::Stun;
    applied.kind = StatusChangeKind::Applied;
    changes.push_back(applied);                        // notavel (Status)
    StatusEffectChange expired;
    expired.kind = StatusChangeKind::Expired;
    changes.push_back(expired);                        // ruido (nao entra)

    // Sem corte: 3 linhas (hit grande + combo + status applied; hit comum e expired fora).
    const auto all = build_log_lines(log, changes, /*max_lines=*/0);
    REQUIRE(all.size() == 3);

    // Corte em 2: mantem as 2 ULTIMAS (a caixa rola).
    const auto last2 = build_log_lines(log, changes, /*max_lines=*/2);
    REQUIRE(last2.size() == 2);
    // A ultima e o status applied (entra depois dos logs de acao).
    REQUIRE(last2.back().kind == LogLineKind::Status);
}
