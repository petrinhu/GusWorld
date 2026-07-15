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

TEST_CASE("log: StatusTick (Knockback adiando o turno, COMBATE-FILA-CURSOR-FIX) sempre "
          "ecoa e classifica como Status",
          "[battle_log]") {
    CombatLogEntry tick;
    tick.action = CombatActionType::StatusTick;
    tick.message = "b e empurrado (Knockback): recua na fila, c age primeiro.";
    REQUIRE(is_notable(tick));
    REQUIRE(classify(tick).kind == LogLineKind::Status);
}

TEST_CASE("log: StatusEffectChange Applied classifica como Status", "[battle_log]") {
    StatusEffectChange c;
    c.actor_id = "alvo";
    c.id = StatusId::Poison;
    c.kind = StatusChangeKind::Applied;
    REQUIRE(classify(c).kind == LogLineKind::Status);
}

TEST_CASE("build_log_lines: NARRA toda ACAO; status NAO polui o rolling (bug-fix)",
          "[battle_log]") {
    // BUG-FIX (criador testou no display): o log nao mostrava os ataques porque os
    // status_changes (Applied) do INICIO da batalha enchiam as ultimas N linhas. Agora o
    // log e a NARRACAO DAS ACOES; o status fica FORA do rolling (aparece como icone sob o
    // ator). O log narra TODA acao/dano (nao so notavel).
    std::vector<CombatLogEntry> log;
    log.push_back(attack_entry(3, "hit comum 3"));    // entra (Damage)
    log.push_back(attack_entry(30, "hit grande 30")); // entra (Damage)
    CombatLogEntry combo;
    combo.action = CombatActionType::UseCard;
    combo.message = "COMPILADO: X";
    log.push_back(combo);                              // entra (System)

    std::vector<StatusEffectChange> changes;
    StatusEffectChange applied;  // dump inicial de status NAO deve esconder o combate
    applied.actor_id = "alvo";
    applied.id = StatusId::Stun;
    applied.kind = StatusChangeKind::Applied;
    changes.push_back(applied);

    // Sem corte: 3 linhas = as 3 ACOES (status fora do rolling de narracao).
    const auto all = build_log_lines(log, changes, /*max_lines=*/0);
    REQUIRE(all.size() == 3);
    REQUIRE(all[0].kind == LogLineKind::Damage);  // o hit comum (3) aparece
    REQUIRE(all[2].kind == LogLineKind::System);  // o COMPILADO aparece
    // Nenhuma linha de Status no rolling (o status spam nao esconde mais o combate).
    for (const auto& l : all) {
        REQUIRE(l.kind != LogLineKind::Status);
    }

    // Corte em 2: mantem as 2 ULTIMAS ACOES (a caixa rola, mostra o combate recente).
    const auto last2 = build_log_lines(log, changes, /*max_lines=*/2);
    REQUIRE(last2.size() == 2);
    REQUIRE(last2.back().kind == LogLineKind::System);  // COMPILADO (a acao mais recente)
}

// ---- D12: log narra a CONSEQUENCIA (dano + status aplicado) -------------------------

TEST_CASE("status_name_key mapeia StatusId pra a chave i18n STATUS_<id>_NAME",
          "[battle_log]") {
    using gus::app::screens::status_name_key;
    REQUIRE(status_name_key(StatusId::Stun) == std::string_view("STATUS_STUN_NAME"));
    REQUIRE(status_name_key(StatusId::Poison) == std::string_view("STATUS_POISON_NAME"));
    REQUIRE(status_name_key(StatusId::Shield) == std::string_view("STATUS_SHIELD_NAME"));
}

// ADR-016 Balde B (decisao do lider 2026-07-15): os 5 StatusId mais recentes
// (SobrecargaTermica/Resfriamento/Reflect/BlindagemEM/NullProof) tinham CASE FALTANDO no
// switch de status_name_key (-Wswitch, higiene do COMBATE-FILA-CURSOR-FIX: "todo efeito
// loga uma mensagem diegetica" tambem cobre o NOME do status no consequence_suffix, senao
// cai no guard STATUS_STUN_NAME e o player le "Atordoamento" pro status errado). Mata
// mutante "case removido volta a cair no default/guard".
TEST_CASE("status_name_key cobre os 5 StatusId novos (ADR-016 Balde B) sem cair no guard",
          "[battle_log]") {
    using gus::app::screens::status_name_key;
    REQUIRE(status_name_key(StatusId::SobrecargaTermica) ==
            std::string_view("STATUS_SOBRECARGATERMICA_NAME"));
    REQUIRE(status_name_key(StatusId::Resfriamento) ==
            std::string_view("STATUS_RESFRIAMENTO_NAME"));
    REQUIRE(status_name_key(StatusId::Reflect) == std::string_view("STATUS_REFLECT_NAME"));
    REQUIRE(status_name_key(StatusId::BlindagemEM) ==
            std::string_view("STATUS_BLINDAGEMEM_NAME"));
    REQUIRE(status_name_key(StatusId::NullProof) ==
            std::string_view("STATUS_NULLPROOF_NAME"));
}

TEST_CASE("consequence_suffix monta '; <alvo> ficou com <status>' so quando ha status",
          "[battle_log]") {
    using gus::app::screens::consequence_suffix;
    // Sem status aplicado neste evento: sufixo vazio (a linha fica so a acao+dano).
    std::vector<StatusEffectChange> none;
    REQUIRE(consequence_suffix("alvo", none).empty());

    // Status aplicado no ALVO neste evento: sufixo com o nome (KEY, a UI resolve via tr).
    std::vector<StatusEffectChange> ch;
    StatusEffectChange poison;
    poison.actor_id = "alvo";
    poison.id = StatusId::Poison;
    poison.kind = StatusChangeKind::Applied;
    ch.push_back(poison);
    const std::string sfx = consequence_suffix("alvo", ch);
    REQUIRE_FALSE(sfx.empty());
    REQUIRE(sfx.find("STATUS_POISON_NAME") != std::string::npos);

    // Status aplicado em OUTRO ator (nao o alvo do golpe): nao entra no sufixo do alvo.
    std::vector<StatusEffectChange> other;
    StatusEffectChange stun;
    stun.actor_id = "outro";
    stun.id = StatusId::Stun;
    stun.kind = StatusChangeKind::Applied;
    other.push_back(stun);
    REQUIRE(consequence_suffix("alvo", other).empty());
}
