// SPDX-License-Identifier: Apache-2.0
// initiative_queue_recompute_round_invariant_test.cpp
//
// FALHA CONHECIDA, DOCUMENTADA DE PROPOSITO (Catch2 [!shouldfail]). Estes dois casos
// FALHAM hoje - e e exatamente por isso que existem. O [!shouldfail] faz o ctest
// registra-los como PASSED enquanto a falha persistir, entao a suite continua verde; no
// dia em que o bug for consertado, eles passam a PASSAR e o [!shouldfail] os reprova,
// gritando "o bug morreu, tire a marcacao". Sao o CRITERIO DE ACEITE do conserto.
//
// ---------------------------------------------------------------------------------
// O BUG
// ---------------------------------------------------------------------------------
// Invariante violado: "cada ator age EXATAMENTE UMA VEZ por rodada da fila" (o mesmo
// invariante que motivou o COMBATE-FILA-CURSOR-FIX, TODO.md).
//
// RAIZ: InitiativeQueue::recompute_by_speed() (domain/src/combat/initiative_queue.cpp)
// re-ordena a fila INTEIRA por SPD - inclusive a regiao [0, cursor_], a dos atores que
// JA AGIRAM nesta rodada - e so preserva a IDENTIDADE do current(), reapontando o cursor
// com `cursor_ = index_of(current_actor)`. E a MESMA raiz que o COMBATE-FILA-CURSOR-FIX
// fechou em reorder_actor/reorder_pending/delay_current ("a regiao [0, cursor_] NUNCA e
// reescrita"), sobrevivendo por um portao que aquele fix nao tocou.
//
// Consequencia, nos dois sentidos:
//   - um ator PENDENTE pode acabar ATRAS do cursor  => perde o turno (caso 1);
//   - um ator JA-AGIDO pode acabar NA FRENTE do cursor => age de novo, com AP e mana
//     recarregados por refresh_resources_for_turn (caso 2, o mais grave).
//
// ALCANCE REAL (nao e teorico): a FSM chama recompute_by_speed NO MEIO DA RODADA em tres
// sitios de producao, sempre que Haste/Slow entra ou expira -
//   domain/src/combat/combat_state_machine.cpp:842   (fim de turno ativo, expire)
//   domain/src/combat/combat_state_machine.cpp:1039  (fim de turno perdido por Stun/morte)
//   domain/src/combat/combat_state_machine.cpp:2611  (tick de status que muda SPD)
// Estes testes dirigem a CombatStateMachine DE VERDADE e usam SO mecanica embarcada
// (o status Haste, que environment_catalog.cpp:169/318 concede como facilitated status).
// Nenhum deles mexe na fila por tras.
//
// ---------------------------------------------------------------------------------
// DECISAO DO LIDER (2026-07-28) - a semantica a implementar
// ---------------------------------------------------------------------------------
// Quando Haste/Slow muda a SPD no meio da rodada, recompute_by_speed passa a PRESERVAR A
// PARTICAO: ordena por SPD DENTRO de [0, cursor_] e DENTRO de (cursor_, fim] de forma
// independente, sem nunca mover ator de um lado pro outro. Quem ja agiu continua tendo
// agido; quem falta continua faltando. O efeito de velocidade aparece na hora (a ordem
// dos pendentes muda), mas ninguem age duas vezes nem e pulado.
// O lider preferiu isto a diferir o recompute para a virada da rodada.
//
// QUANDO O CONSERTO ENTRAR:
//   1. estes dois casos vao PASSAR e, por causa do [!shouldfail], serao reportados como
//      FAILED pelo ctest - isso e o sinal, nao uma regressao;
//   2. TIRE o "[!shouldfail]" das duas tags e deixe os casos como testes normais (eles
//      viram a rede de regressao permanente da semantica nova);
//   3. so entao mexa no ramo de fuzz morto do F2-QA.8 (initiative_queue_property_test.cpp
//      case 2 declara "mudar SPD + recompute" e nunca muda SPD) e acrescente ali a
//      propriedade "cada ator age 1x por rodada". Os dois andam JUNTOS, de proposito.
//
// Subsistema: domain/combat (InitiativeQueue + CombatStateMachine). POCO puro, headless.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"

using namespace gus::domain::combat;

namespace {

CombatActor party_member(const std::string& id, int spd) {
    // HP alto e ATK 1: ninguem morre, o combate so anda e as rodadas ficam observaveis.
    return CombatActor(id, id, /*max_hp=*/9999, /*atk=*/1, /*def=*/2, spd,
                       CardFamily::Eletrico, /*is_player_side=*/true);
}

CombatActor enemy(const std::string& id, int spd) {
    return CombatActor(id, id, /*max_hp=*/9999, /*atk=*/1, /*def=*/2, spd,
                       CardFamily::Cinetico, /*is_player_side=*/false);
}

// Snapshot do INICIO de um turno (tirado logo apos begin_turn, ou seja, DEPOIS de
// refresh_resources_for_turn): quem age, em que rodada, e com quantos recursos.
struct TurnStart {
    int round = 0;
    std::string actor_id;
    int ap = 0;
    int mana = 0;
};

int turns_of(const std::vector<TurnStart>& turns, int round, const std::string& id) {
    return static_cast<int>(std::count_if(
        turns.begin(), turns.end(),
        [&](const TurnStart& t) { return t.round == round && t.actor_id == id; }));
}

// Roda o combate por `max_turns` turnos ou ate fechar `stop_at_round` rodadas, gravando um
// TurnStart por turno. Conducao normal da FSM (begin_turn -> run_active_turn_to_end ->
// check_end -> advance), identica a de qualquer outro teste deste diretorio.
std::vector<TurnStart> drive(CombatStateMachine& sm, int stop_at_round, int max_turns = 24) {
    std::vector<TurnStart> turns;
    for (int i = 0; i < max_turns; ++i) {
        const bool stunned = sm.begin_turn();
        const CombatActor* cur = sm.queue().current();
        turns.push_back(TurnStart{sm.queue().round_index(), cur->id(), cur->ap(), cur->mana()});
        if (!stunned)
            sm.run_active_turn_to_end();
        sm.check_end();
        sm.queue().advance();
        if (sm.queue().round_index() >= stop_at_round)
            break;
    }
    return turns;
}

// Carta minima so pra DRENAR mana no primeiro turno (o caso 2 precisa provar que a segunda
// recarga e recarga de verdade, nao um no-op sobre recurso ja cheio).
Card mana_sink_card() {
    Card c;
    c.id = "carta_dreno";
    c.display_name = c.id;
    c.family = CardFamily::Eletrico;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = 1;
    c.ap_cost = 1;
    c.power = 1;
    c.target_shape = TargetShape::Single;
    return c;
}

}  // namespace

// ============================================================================
// Caso 1 - ATOR PULADO
// ============================================================================

TEST_CASE(
    "FALHA CONHECIDA (recompute_by_speed reordena a regiao [0, cursor_]): Haste expirando "
    "no meio da rodada empurra o portador para TRAS do cursor e o ator seguinte PERDE o "
    "turno - viola 'cada ator age exatamente 1x por rodada' (combat_state_machine.cpp:842)",
    "[domain][combat][queue][regressao][!shouldfail]") {
    // volt abre a rodada (SPD 8). gus tem SPD base 5, mas ganha Haste +10 no proprio
    // TurnStart (SPD 15) e o buff expira no FIM desse mesmo turno, derrubando pra 5.
    CombatActor volt = party_member("volt", /*spd=*/8);
    CombatActor gus = party_member("gus", /*spd=*/5);
    CombatActor e1 = enemy("e1", /*spd=*/7);

    gus.add_status(
        StatusEffect{StatusId::Haste, /*magnitude=*/10, /*duration=*/1, StackRule::Replace,
                     CardFamily::Eletrico});

    CombatStateMachine sm({&volt, &gus, &e1},
                          [](CombatActor&, const CombatState&) { return CombatAction::pass(); });

    const std::vector<TurnStart> turns = drive(sm, /*stop_at_round=*/1);

    // O que acontece HOJE: ao expirar o Haste no fim do turno do gus, o recompute joga o
    // gus pro fim da fila e o cursor o segue (cursor = index_of(gus) = 2). O advance
    // seguinte da a volta e abre a rodada 1 - o e1, que estava pendente, nunca foi chamado.
    INFO("turnos observados na rodada 0: " << [&] {
        std::string s;
        for (const TurnStart& t : turns)
            if (t.round == 0) s += t.actor_id + " ";
        return s;
    }());

    CHECK(turns_of(turns, 0, "volt") == 1);
    CHECK(turns_of(turns, 0, "gus") == 1);
    // ESTA e a assercao que falha hoje (e1 = 0 turnos: foi pulado).
    CHECK(turns_of(turns, 0, "e1") == 1);
}

// ============================================================================
// Caso 2 - TURNO DUPLO, com AP e mana recarregados 2x na mesma rodada
// ============================================================================

TEST_CASE(
    "FALHA CONHECIDA (recompute_by_speed reordena a regiao [0, cursor_]): Haste aplicado no "
    "meio da rodada joga um ator JA-AGIDO para a frente do cursor e ele age DUAS VEZES na "
    "mesma rodada, com AP e mana recarregados nas duas - viola 'cada ator age exatamente 1x "
    "por rodada' (combat_state_machine.cpp:2611)",
    "[domain][combat][queue][regressao][!shouldfail]") {
    // Mesma armacao do caso 1, com o Haste do gus DURANDO (nao expira dentro da rodada):
    // o recompute do TurnStart do gus o joga pro slot 0 e empurra o volt - que JA AGIU -
    // pro slot 1, na frente do cursor. O advance seguinte devolve o turno ao volt.
    CombatActor volt = party_member("volt", /*spd=*/8);
    CombatActor gus = party_member("gus", /*spd=*/5);
    CombatActor e1 = enemy("e1", /*spd=*/7);

    gus.add_status(
        StatusEffect{StatusId::Haste, /*magnitude=*/10, /*duration=*/5, StackRule::Replace,
                     CardFamily::Eletrico});

    const Card dreno = mana_sink_card();
    std::unordered_map<std::string, Card> registry;
    registry.emplace(dreno.id, dreno);

    // O volt gasta AP e mana UMA unica vez (no primeiro turno dele); todo mundo passa no
    // resto. Assim, se ele receber um segundo turno, os recursos do snapshot provam que
    // houve uma segunda recarga de verdade - e nao leitura de um recurso que nunca baixou.
    bool volt_ja_gastou = false;
    CombatStateMachine sm(
        {&volt, &gus, &e1},
        [&volt_ja_gastou, &dreno](CombatActor& a, const CombatState&) -> CombatAction {
            if (a.id() == "volt" && !volt_ja_gastou) {
                volt_ja_gastou = true;
                return CombatAction::use_card(dreno.id, "e1");
            }
            return CombatAction::pass();
        },
        &registry);

    // Recursos "cheios" na rodada 0. AP e fixo (kBaseApPerTurn). Mana NAO pode vir de
    // volt.max_mana() aqui: max_mana_ so e populado por refresh_resources_for_turn, ou
    // seja, vale 0 antes do combate comecar - a rampa e kBaseMana + round_index (secao 5).
    const int ap_cheio = volt.max_ap();
    const int mana_cheia = combat_constants::kBaseMana + 0;

    const std::vector<TurnStart> turns = drive(sm, /*stop_at_round=*/1);

    INFO("turnos observados na rodada 0: " << [&] {
        std::string s;
        for (const TurnStart& t : turns)
            if (t.round == 0)
                s += t.actor_id + "(ap=" + std::to_string(t.ap) + ",mana=" +
                     std::to_string(t.mana) + ") ";
        return s;
    }());

    // Pre-condicoes do cenario (se QUALQUER uma destas quebrar, o teste nao esta medindo o
    // que promete - conferir antes de confiar na assercao final).
    REQUIRE(volt_ja_gastou);
    REQUIRE(ap_cheio >= 1);
    REQUIRE(mana_cheia >= 1);

    // Evidencia dos recursos: TODO turno do volt na rodada 0 comeca com AP e mana CHEIOS.
    // Como ele gastou 1 de cada no primeiro, um segundo turno cheio so pode vir de um
    // segundo refresh_resources_for_turn dentro da MESMA rodada.
    for (const TurnStart& t : turns) {
        if (t.round == 0 && t.actor_id == "volt") {
            CHECK(t.ap == ap_cheio);
            CHECK(t.mana == mana_cheia);
        }
    }

    CHECK(turns_of(turns, 0, "gus") == 1);
    CHECK(turns_of(turns, 0, "e1") == 1);
    // ESTA e a assercao que falha hoje (volt = 2 turnos, ambos com recursos recarregados).
    CHECK(turns_of(turns, 0, "volt") == 1);
}
