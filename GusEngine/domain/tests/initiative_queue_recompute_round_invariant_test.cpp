// SPDX-License-Identifier: Apache-2.0
// initiative_queue_recompute_round_invariant_test.cpp
//
// CONSERTADO (COMBATE-FILA-CURSOR-FIX, 2026-08-01). Ate esta data os dois casos abaixo
// FALHAVAM DE PROPOSITO (Catch2 [!shouldfail]): o ctest os registrava como PASSED enquanto
// a falha persistisse, mantendo a suite verde; no dia do conserto eles passariam a PASSAR
// de verdade e o [!shouldfail] os reprovaria, gritando "o bug morreu, tire a marcacao".
// Esse dia chegou - a marcacao foi REMOVIDA e os dois casos agora sao a rede de regressao
// PERMANENTE da semantica nova. Historia preservada abaixo por registro.
//
// ---------------------------------------------------------------------------------
// O BUG (historico - ja corrigido)
// ---------------------------------------------------------------------------------
// Invariante violado: "cada ator age EXATAMENTE UMA VEZ por rodada da fila" (o mesmo
// invariante que motivou o COMBATE-FILA-CURSOR-FIX, TODO.md).
//
// RAIZ: InitiativeQueue::recompute_by_speed() (domain/src/combat/initiative_queue.cpp)
// re-ordenava a fila INTEIRA por SPD - inclusive a regiao [0, cursor_], a dos atores que
// JA AGIRAM nesta rodada - e so preservava a IDENTIDADE do current(), reapontando o cursor
// com `cursor_ = index_of(current_actor)`. Era a MESMA raiz que o COMBATE-FILA-CURSOR-FIX
// ja tinha fechado em reorder_actor/reorder_pending/delay_current ("a regiao [0, cursor_]
// NUNCA e reescrita"), sobrevivendo por um portao que aquele fix ainda nao tinha tocado.
//
// Consequencia, nos dois sentidos:
//   - um ator PENDENTE podia acabar ATRAS do cursor  => perdia o turno (caso 1);
//   - um ator JA-AGIDO podia acabar NA FRENTE do cursor => agia de novo, com AP e mana
//     recarregados por refresh_resources_for_turn (caso 2, o mais grave).
//
// ALCANCE REAL (nao era teorico): a FSM chama recompute_by_speed NO MEIO DA RODADA em tres
// sitios de producao, sempre que Haste/Slow entra ou expira -
//   domain/src/combat/combat_state_machine.cpp:842   (fim de turno ativo, expire)
//   domain/src/combat/combat_state_machine.cpp:1039  (fim de turno perdido por Stun/morte)
//   domain/src/combat/combat_state_machine.cpp:2611  (tick de status que muda SPD)
// Estes testes dirigem a CombatStateMachine DE VERDADE e usam SO mecanica embarcada
// (o status Haste, que environment_catalog.cpp:169/318 concede como facilitated status).
// Nenhum deles mexe na fila por tras.
//
// ---------------------------------------------------------------------------------
// DECISAO DO LIDER (2026-07-27/28) - a semantica implementada
// ---------------------------------------------------------------------------------
// Quando Haste/Slow muda a SPD no meio da rodada, recompute_by_speed PRESERVA A PARTICAO:
// ordena por SPD DENTRO de [0, cursor_) e DENTRO de (cursor_, fim] de forma independente,
// sem nunca mover ator de um lado pro outro. O current() (slot cursor_) fica FIXO no
// proprio lugar - nao entra em nenhum dos dois sorts. Quem ja agiu continua tendo agido;
// quem falta continua faltando. O efeito de velocidade aparece na hora pros PENDENTES
// (a ordem entre eles muda), mas ninguem age duas vezes nem e pulado.
// O lider preferiu isto a diferir o recompute para a virada da rodada.
//
// Nota de implementacao (initiative_queue.cpp): o current NAO participa do sort do bloco
// "ja-agido" (nao e um sort inclusivo de [0, cursor_] com o cursor recalculado por
// index_of depois). Se participasse e saisse mais rapido que algum ja-agido, o cursor
// teria que RECUAR pra acompanhar sua nova posicao - reabrindo como "pendentes" indices
// que ja tinham atores agidos, dando a eles um 2o turno na mesma rodada (a MESMA classe de
// bug, so espelhada). Fixar o current no proprio slot e a unica construcao que satisfaz
// "current() identico" E "ninguem age 2x nem e pulado" simultaneamente, pra qualquer
// posicao do cursor - inclusive o caso-limite cursor_ no ultimo slot (onde a regiao
// "ja-agida" e a fila INTEIRA), coberto por teste em initiative_queue_test.cpp.
//
// F2-QA.8 (initiative_queue_property_test.cpp): o fuzz morto (case 2 se declarava "mudar
// SPD + recompute" e nunca mudava SPD) foi corrigido no mesmo passo, com a propriedade
// "cada ator age 1x por rodada" (preservacao da particao) acrescentada ali.
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
    "regressao (COMBATE-FILA-CURSOR-FIX): Haste expirando no meio da rodada NAO empurra o "
    "portador para TRAS do cursor - o ator seguinte nao perde o turno - 'cada ator age "
    "exatamente 1x por rodada' (combat_state_machine.cpp:842)",
    "[domain][combat][queue][regressao]") {
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

    // ANTES DO FIX: ao expirar o Haste no fim do turno do gus, o recompute jogava o gus pro
    // fim da fila e o cursor o seguia (cursor = index_of(gus) = 2). O advance seguinte dava
    // a volta e abria a rodada 1 - o e1, que estava pendente, nunca era chamado. DEPOIS DO
    // FIX: gus fica FIXO no proprio slot (nao entra no sort do bloco ja-agido), entao o
    // cursor nao recua e e1 recebe o turno normalmente.
    INFO("turnos observados na rodada 0: " << [&] {
        std::string s;
        for (const TurnStart& t : turns)
            if (t.round == 0) s += t.actor_id + " ";
        return s;
    }());

    CHECK(turns_of(turns, 0, "volt") == 1);
    CHECK(turns_of(turns, 0, "gus") == 1);
    CHECK(turns_of(turns, 0, "e1") == 1);
}

// ============================================================================
// Caso 2 - TURNO DUPLO, com AP e mana recarregados 2x na mesma rodada
// ============================================================================

TEST_CASE(
    "regressao (COMBATE-FILA-CURSOR-FIX): Haste aplicado no meio da rodada NAO joga um ator "
    "JA-AGIDO para a frente do cursor - ninguem age DUAS VEZES na mesma rodada - 'cada ator "
    "age exatamente 1x por rodada' (combat_state_machine.cpp:2611)",
    "[domain][combat][queue][regressao]") {
    // Mesma armacao do caso 1, com o Haste do gus DURANDO (nao expira dentro da rodada).
    // ANTES DO FIX: o recompute do TurnStart do gus o jogava pro slot 0 e empurrava o volt -
    // que JA AGIU - pro slot 1, na frente do cursor; o advance seguinte devolvia o turno ao
    // volt. DEPOIS DO FIX: gus fica FIXO no proprio slot (ultimo indice da rodada), entao o
    // bloco ja-agido {volt} nao e reaberto como pendente.
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
