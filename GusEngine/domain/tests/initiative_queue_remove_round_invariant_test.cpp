// SPDX-License-Identifier: Apache-2.0
// initiative_queue_remove_round_invariant_test.cpp
//
// FILA-REMOVE-ROUND-WRAP (achado de auditoria independente 2026-08-06). TERCEIRO portao da
// MESMA raiz ja fechada duas vezes: "cada ator age EXATAMENTE UMA VEZ por rodada da fila"
// (1o portao reorder_actor, fechado 2026-07-15; 2o portao recompute_by_speed, fechado
// 2026-08-01, ver initiative_queue_recompute_round_invariant_test.cpp).
//
// ---------------------------------------------------------------------------------
// O BUG
// ---------------------------------------------------------------------------------
// InitiativeQueue::remove() terminava com:
//
//     if (cursor_ >= static_cast<int>(order_.size()))
//         cursor_ = 0;
//
// ou seja, DAVA A VOLTA NA FILA sem incrementar round_index_ - ao contrario de advance(),
// que faz a mesma volta e conta a rodada (o unico ++round_index_ do arquivo). Depois do
// wrap mudo, o advance() seguinte (advance_to_next_actor) levava o cursor de 0 pra 1 e a
// rodada CONTINUAVA aberta: todo ator do indice 1 em diante jogava DE NOVO na mesma rodada,
// com AP e mana RECARREGADOS por refresh_resources_for_turn - e o ator do indice 0 era
// PULADO na rodada seguinte (a mesma classe de bug, nos dois sentidos, do 2o portao).
//
// CONDICAO EXATA: o ator removido e o do cursor E esta no ULTIMO slot
// (idx == cursor_ == size-1). Se o ultimo morre por ataque de OUTRO ator, nao reproduz
// (idx > cursor_, sem wrap) - ver o caso "controle negativo" abaixo.
//
// ALCANCE EM PRODUCAO (nao era teorico): prune_dead() chama queue_.remove()
// (combat_state_machine.cpp:2572) e e chamado de check_end() (:869) e de
// advance_to_next_actor() (:910). O gatilho comum e veneno/corrosao matando o ator no
// PROPRIO TurnStart (apply_status_tick), mecanica embarcada.
//
// ---------------------------------------------------------------------------------
// A SEMANTICA IMPLEMENTADA (o conserto)
// ---------------------------------------------------------------------------------
// remove() nao conta rodada NENHUMA: advance() segue sendo o UNICO dono do ++round_index_.
// Quando o current no ultimo slot sai da fila, o cursor recua pro NOVO ultimo slot
// (cursor_ = size-1) em vez de saltar pro topo. Efeito: a volta fica PENDENTE e acontece no
// advance() seguinte - que a conta, dispara os hooks de fim de rodada (process_round_end_
// hooks/regroup_round_by_side/advance_period_clock, que so rodam quando
// advance_to_next_actor ve round_index subir) e abre a rodada nova no topo da fila.
// Preserva tambem a particao "todo indice <= cursor ja agiu / todo indice > cursor esta
// pendente" (o slot pro qual o cursor recua e de um ator que JA agiu), a mesma particao que
// o 2o portao existe pra proteger.
//
// Subsistema: domain/combat (InitiativeQueue + CombatStateMachine). POCO puro, headless.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/initiative_queue.hpp"

using namespace gus::domain::combat;

namespace {

CombatActor unit(const std::string& id, int spd, bool player = true, int max_hp = 500,
                 int atk = 5) {
    return CombatActor(id, id, max_hp, atk, /*def=*/0, spd,
                       player ? CardFamily::Eletrico : CardFamily::Cinetico, player);
}

std::vector<std::string> order_ids(const InitiativeQueue& q) {
    std::vector<std::string> ids;
    for (const CombatActor* a : q.order())
        ids.push_back(a->id());
    return ids;
}

// Um turno observado: quem agiu, em que rodada, com quantos recursos NO INICIO do turno.
struct TurnStart {
    int round = 0;
    std::string actor_id;
    int ap = 0;
    int mana = 0;
};

// Provider que GRAVA o inicio de cada turno e ataca o primeiro inimigo vivo (pra o combate
// terminar de verdade em Victory, sem stalemate). O provider e chamado ate `ap` acabar; so
// a PRIMEIRA chamada do turno (ap == max_ap, logo apos refresh_resources_for_turn) vira um
// TurnStart - as demais sao continuacao do MESMO turno.
struct TurnRecorder {
    std::vector<TurnStart>* out;
    std::vector<CombatActor*> enemies;
    bool enemies_pass = true;

    CombatAction operator()(CombatActor& a, const CombatState& state) {
        if (a.ap() == a.max_ap())
            out->push_back(TurnStart{state.round_index(), a.id(), a.ap(), a.mana()});

        if (!a.is_player_side() && enemies_pass)
            return CombatAction::pass();

        for (CombatActor* e : enemies)
            if (e->is_alive() && e->is_player_side() != a.is_player_side())
                return CombatAction::attack(e->id());
        return CombatAction::pass();
    }
};

// Quantas vezes `id` abriu turno na rodada `round`.
int turns_of(const std::vector<TurnStart>& turns, int round, const std::string& id) {
    return static_cast<int>(std::count_if(
        turns.begin(), turns.end(),
        [&](const TurnStart& t) { return t.round == round && t.actor_id == id; }));
}

std::string render(const std::vector<TurnStart>& turns) {
    std::string s;
    for (const TurnStart& t : turns)
        s += "r" + std::to_string(t.round) + ":" + t.actor_id + "(ap=" +
             std::to_string(t.ap) + ",mana=" + std::to_string(t.mana) + ") ";
    return s;
}

StatusEffect poison(int magnitude, int duration) {
    return StatusEffect{StatusId::Poison, magnitude, duration, StackRule::Replace,
                        CardFamily::Bioquimico};
}

}  // namespace

// ============================================================================
// NIVEL FILA (unitario): as 4 formas de remove() e o que cada uma faz com a rodada
// ============================================================================

TEST_CASE(
    "regressao (FILA-REMOVE-ROUND-WRAP): remover o current do ULTIMO slot deixa a volta "
    "PENDENTE - o advance seguinte conta a rodada UMA vez e abre no topo",
    "[domain][combat][queue][regressao]") {
    CombatActor a = unit("a", 30), b = unit("b", 20), c = unit("c", 10);
    InitiativeQueue q({&a, &b, &c});

    q.advance();  // b
    q.advance();  // c: current NO ULTIMO SLOT (a condicao exata do bug)
    REQUIRE(q.current() == &c);
    REQUIRE(q.cursor() == 2);
    REQUIRE(q.round_index() == 0);

    q.remove(&c);  // morreu no proprio TurnStart (veneno/corrosao)

    // remove NAO conta rodada - advance() e o unico dono do contador (e o unico ponto onde
    // a FSM detecta a fronteira pra disparar os hooks de fim de rodada).
    REQUIRE(q.round_index() == 0);
    REQUIRE(order_ids(q) == std::vector<std::string>{"a", "b"});

    // ANTES DO FIX: remove saltava o cursor pro topo (0) sem contar rodada; este advance
    // levava a 1 (=b) e dava a `b` um SEGUNDO turno na MESMA rodada (com AP/mana
    // recarregados), pulando `a` na rodada seguinte.
    q.advance();
    CHECK(q.round_index() == 1);
    CHECK(q.current() == &a);
    CHECK(q.cursor() == 0);
}

TEST_CASE(
    "controle negativo (FILA-REMOVE-ROUND-WRAP): o ULTIMO da fila morto por ataque de OUTRO "
    "ator nao mexe na rodada nem no cursor",
    "[domain][combat][queue][regressao]") {
    CombatActor a = unit("a", 30), b = unit("b", 20), c = unit("c", 10);
    InitiativeQueue q({&a, &b, &c});

    q.advance();  // b e o current; c (ultimo) leva o golpe e cai
    REQUIRE(q.current() == &b);

    q.remove(&c);

    CHECK(q.round_index() == 0);
    CHECK(q.current() == &b);   // identidade do current preservada
    CHECK(q.cursor() == 1);
    CHECK(order_ids(q) == std::vector<std::string>{"a", "b"});

    // A volta so acontece no fim da fila encurtada, e conta UMA rodada.
    q.advance();
    CHECK(q.round_index() == 1);
    CHECK(q.current() == &a);
}

TEST_CASE(
    "FILA-REMOVE-ROUND-WRAP: remover o current no MEIO da fila passa o turno ao seguinte, "
    "sem contar rodada",
    "[domain][combat][queue][regressao]") {
    CombatActor a = unit("a", 30), b = unit("b", 20), c = unit("c", 10);
    InitiativeQueue q({&a, &b, &c});

    q.advance();      // b e o current (slot 1, NAO e o ultimo)
    q.remove(&b);

    CHECK(q.round_index() == 0);
    CHECK(q.current() == &c);   // o seguinte assume o turno
    CHECK(q.cursor() == 1);

    q.advance();
    CHECK(q.round_index() == 1);
    CHECK(q.current() == &a);
}

TEST_CASE(
    "FILA-REMOVE-ROUND-WRAP: remover um JA-AGIDO (antes do cursor) desliza o cursor e "
    "preserva o current, sem contar rodada",
    "[domain][combat][queue][regressao]") {
    CombatActor a = unit("a", 30), b = unit("b", 20), c = unit("c", 10);
    InitiativeQueue q({&a, &b, &c});

    q.advance();
    q.advance();      // c e o current (ultimo slot)
    q.remove(&a);     // `a` ja agiu nesta rodada e sai da fila

    CHECK(q.round_index() == 0);
    CHECK(q.current() == &c);   // continua sendo a vez do c
    CHECK(q.cursor() == 1);
    CHECK(order_ids(q) == std::vector<std::string>{"b", "c"});

    q.advance();
    CHECK(q.round_index() == 1);
    CHECK(q.current() == &b);
}

TEST_CASE(
    "FILA-REMOVE-ROUND-WRAP: remover um JA-AGIDO com o cursor NO MEIO da fila preserva o "
    "current por IDENTIDADE (o caso dificil: sem a guarda do wrap pra corrigir por acidente)",
    "[domain][combat][queue][regressao]") {
    // Este caso existe porque o de 3 atores acima e o caso FACIL: la o cursor esta no ULTIMO
    // slot, entao mesmo SEM o slide `--cursor_` a guarda do wrap traria o cursor de volta pro
    // valor certo por acidente (um mutante que apaga o slide sobrevive). Com 4 atores e o
    // cursor no MEIO, o slide e a UNICA coisa que segura o current no lugar: sem ele o cursor
    // fica onde estava e o ator seguinte rouba o turno de quem esta agindo.
    CombatActor a = unit("a", 40), b = unit("b", 30), c = unit("c", 20), d = unit("d", 10);
    InitiativeQueue q({&a, &b, &c, &d});

    q.advance();
    q.advance();  // c e o current, no slot 2 (NAO e o ultimo: `d` ainda esta pendente)
    REQUIRE(q.current() == &c);
    REQUIRE(q.cursor() == 2);

    q.remove(&a);  // `a` ja agiu nesta rodada e sai da fila

    CHECK(q.round_index() == 0);
    CHECK(q.current() == &c);   // continua sendo a vez do c - `d` NAO pode roubar o turno
    CHECK(q.cursor() == 1);
    CHECK(order_ids(q) == std::vector<std::string>{"b", "c", "d"});

    // E `d` continua pendente: age em seguida, ainda nesta rodada.
    q.advance();
    CHECK(q.round_index() == 0);
    CHECK(q.current() == &d);
    q.advance();
    CHECK(q.round_index() == 1);
    CHECK(q.current() == &b);
}

TEST_CASE(
    "FILA-REMOVE-ROUND-WRAP: DOIS mortos no mesmo prune (um ja-agido + o current no ultimo "
    "slot) contam UMA unica rodada no advance seguinte",
    "[domain][combat][queue][regressao]") {
    CombatActor a = unit("a", 40), b = unit("b", 30), c = unit("c", 20), d = unit("d", 10);
    InitiativeQueue q({&a, &b, &c, &d});

    q.advance();
    q.advance();
    q.advance();  // d e o current, no ultimo slot
    REQUIRE(q.current() == &d);

    // Mesma ordem de prune_dead (varre order() e remove um a um).
    q.remove(&b);  // ja-agido
    q.remove(&d);  // o current, agora no ultimo slot da fila encurtada

    CHECK(q.round_index() == 0);
    CHECK(order_ids(q) == std::vector<std::string>{"a", "c"});

    q.advance();
    CHECK(q.round_index() == 1);   // UMA rodada, nao duas, nao zero
    CHECK(q.current() == &a);
}

TEST_CASE(
    "FILA-REMOVE-ROUND-WRAP: fila de 2 - remover o current no ultimo slot deixa a volta "
    "pendente pro unico sobrevivente",
    "[domain][combat][queue][regressao]") {
    CombatActor a = unit("a", 20), b = unit("b", 10);
    InitiativeQueue q({&a, &b});

    q.advance();  // b, ultimo slot
    q.remove(&b);

    CHECK(q.round_index() == 0);
    CHECK(q.current() == &a);
    CHECK(q.count() == 1);

    q.advance();
    CHECK(q.round_index() == 1);
    CHECK(q.current() == &a);
}

// ============================================================================
// NIVEL FSM (repro do auditor): veneno mata o ULTIMO da fila no proprio TurnStart
// ============================================================================

TEST_CASE(
    "regressao (FILA-REMOVE-ROUND-WRAP): veneno matando o ULTIMO ator da fila no proprio "
    "TurnStart NAO reabre a rodada - ninguem age duas vezes com AP/mana recarregados",
    "[domain][combat][queue][regressao]") {
    // Repro do auditor: party gus(SPD 12) + volt(SPD 10) contra e1(SPD 11) e e2(SPD 4,
    // HP 4, veneno 3). A party abre a rodada (SPD max 12 >= 11), entao o e2 - o mais lento
    // de todos - e sempre o ULTIMO slot. Na rodada 1 o veneno o mata no proprio TurnStart.
    CombatActor gus = unit("gus", /*spd=*/12);
    CombatActor volt = unit("volt", /*spd=*/10);
    CombatActor e1 = unit("e1", /*spd=*/11, /*player=*/false, /*max_hp=*/200);
    CombatActor e2 = unit("e2", /*spd=*/4, /*player=*/false, /*max_hp=*/4);
    e2.add_status(poison(/*magnitude=*/3, /*duration=*/9));

    std::vector<TurnStart> turns;
    CombatStateMachine sm({&gus, &volt, &e1, &e2},
                          TurnRecorder{&turns, {&e1, &e2}, /*enemies_pass=*/true});

    REQUIRE(order_ids(sm.queue()).back() == "e2");  // pre-condicao: e2 no ULTIMO slot

    const CombatResult result = sm.run_until_end();
    INFO("turnos observados: " << render(turns));
    REQUIRE(result.outcome == CombatOutcome::Victory);

    // e2 morre no TurnStart da rodada 1 (HP 4, veneno 3: sobra 1 na rodada 0, cai na 1).
    REQUIRE(turns_of(turns, 1, "e2") == 0);
    REQUIRE(!e2.is_alive());

    // ANTES DO FIX: o wrap mudo de remove() reabria a rodada 1 e gus/volt/e1 (todos os
    // indices >= 1 apos o wrap) ganhavam um SEGUNDO turno nela.
    CHECK(turns_of(turns, 1, "gus") == 1);
    CHECK(turns_of(turns, 1, "volt") == 1);
    CHECK(turns_of(turns, 1, "e1") == 1);

    // E ninguem e PULADO na rodada seguinte (o espelho do mesmo bug).
    CHECK(turns_of(turns, 2, "gus") == 1);
    CHECK(turns_of(turns, 2, "volt") == 1);
    CHECK(turns_of(turns, 2, "e1") == 1);
}

TEST_CASE(
    "controle negativo (FILA-REMOVE-ROUND-WRAP): o ULTIMO da fila morto por ATAQUE de outro "
    "ator (nao pelo proprio tick) nao muda nada",
    "[domain][combat][queue][regressao]") {
    // Mesma armacao, mas o e2 tem HP baixo e SEM veneno: quem o mata e o ataque da party,
    // durante o turno de OUTRO ator (idx > cursor_, sem wrap). Nenhum turno duplo, nem
    // antes nem depois do fix - este caso existe pra travar um conserto largo demais (do
    // tipo "conta rodada em todo remove"), que quebraria AQUI.
    CombatActor gus = unit("gus", /*spd=*/12);
    CombatActor volt = unit("volt", /*spd=*/10);
    CombatActor e1 = unit("e1", /*spd=*/11, /*player=*/false, /*max_hp=*/200);
    CombatActor e2 = unit("e2", /*spd=*/4, /*player=*/false, /*max_hp=*/1);

    std::vector<TurnStart> turns;
    CombatStateMachine sm({&gus, &volt, &e1, &e2},
                          TurnRecorder{&turns, {&e2, &e1}, /*enemies_pass=*/true});

    REQUIRE(order_ids(sm.queue()).back() == "e2");

    const CombatResult result = sm.run_until_end();
    INFO("turnos observados: " << render(turns));
    REQUIRE(result.outcome == CombatOutcome::Victory);
    REQUIRE(!e2.is_alive());
    REQUIRE(turns_of(turns, 0, "e2") == 0);  // morreu no 1o ataque do gus, antes da vez dele

    for (int r = 0; r <= 2; ++r) {
        INFO("rodada " << r);
        CHECK(turns_of(turns, r, "gus") == 1);
        CHECK(turns_of(turns, r, "volt") == 1);
        CHECK(turns_of(turns, r, "e1") == 1);
    }
}

// ============================================================================
// INVARIANTE GERAL: para QUALQUER sequencia de mortes, cada ator vivo abre no maximo
// UM turno por valor de round_index. Nao e o repro - e a propriedade que faltou nas
// duas fatias anteriores (elas travaram o SITIO, nao o invariante).
// ============================================================================

TEST_CASE(
    "invariante (FILA-REMOVE-ROUND-WRAP): sob mortes em posicoes variadas da fila, nenhum "
    "ator abre dois turnos no mesmo round_index",
    "[domain][combat][queue][regressao][invariante]") {
    struct Scenario {
        const char* name;
        int spd_p1, spd_p2;
        int spd_e1, spd_e2, spd_e3;
        int hp_e1, hp_e2, hp_e3;      // HP baixo => morre cedo, por ataque
        int poison_on;                 // 0=nenhum, 1..3 = e1..e3 recebe veneno
        int poison_hp;                 // HP do envenenado (decide EM QUE rodada ele cai)
    };

    // Varia (a) quem e o ultimo slot, (b) quem morre pelo PROPRIO tick (veneno) e em que
    // rodada, (c) quem morre por ataque (durante o turno de outro), (d) quem abre a rodada
    // (party abre se SPD max >= a do inimigo; senao o inimigo abre, e o ULTIMO slot passa a
    // ser da party). Cada linha e um combate inteiro, conduzido pela FSM real.
    const std::vector<Scenario> scenarios = {
        {"veneno mata o ultimo na rodada 1",      12, 10, 11, 8, 4, 200, 200,  4, 3,   4},
        {"veneno mata o ultimo na rodada 2",      12, 10, 11, 8, 4, 200, 200,  7, 3,   7},
        {"veneno mata o do meio na rodada 1",     12, 10, 11, 8, 4, 200,   4, 200, 2,   4},
        {"veneno mata o primeiro inimigo",        12, 10, 11, 8, 4,   4, 200, 200, 1,   4},
        {"inimigo abre; veneno mata o ultimo",     9,  7, 20, 8, 4, 200, 200,  4, 3,   4},
        {"inimigo abre; veneno na rodada 3",       9,  7, 20, 8, 4, 200, 200, 10, 3,  10},
        {"sem veneno: mortes so por ataque",      12, 10, 11, 8, 4,  30,  20,  10, 0,   0},
        {"empate de SPD entre lados",             11, 10, 11, 8, 4, 200, 200,  4, 3,   4},
    };

    for (const Scenario& s : scenarios) {
        INFO("cenario: " << s.name);

        CombatActor p1 = unit("p1", s.spd_p1, /*player=*/true, /*max_hp=*/9999);
        CombatActor p2 = unit("p2", s.spd_p2, /*player=*/true, /*max_hp=*/9999);
        CombatActor e1 = unit("e1", s.spd_e1, /*player=*/false, s.hp_e1);
        CombatActor e2 = unit("e2", s.spd_e2, /*player=*/false, s.hp_e2);
        CombatActor e3 = unit("e3", s.spd_e3, /*player=*/false, s.hp_e3);

        std::vector<CombatActor*> foes = {&e1, &e2, &e3};
        if (s.poison_on > 0) {
            CombatActor* victim = foes[static_cast<std::size_t>(s.poison_on - 1)];
            victim->take_damage(std::max(0, victim->max_hp() - s.poison_hp));
            victim->add_status(poison(/*magnitude=*/3, /*duration=*/20));
        }

        std::vector<TurnStart> turns;
        CombatStateMachine sm({&p1, &p2, &e1, &e2, &e3},
                              TurnRecorder{&turns, foes, /*enemies_pass=*/true});
        const CombatResult result = sm.run_until_end();

        INFO("turnos observados: " << render(turns));
        REQUIRE(result.outcome == CombatOutcome::Victory);
        REQUIRE(turns.size() >= 6);  // o cenario precisa ter combate de verdade pra medir

        // O INVARIANTE: (rodada, ator) nunca se repete.
        std::map<std::pair<int, std::string>, int> seen;
        for (const TurnStart& t : turns)
            ++seen[{t.round, t.actor_id}];
        for (const auto& [key, n] : seen) {
            INFO("ator '" << key.second << "' na rodada " << key.first);
            CHECK(n == 1);
        }

        // E o corolario de RECURSO: todo turno aberto comeca com AP cheio e mana da rampa
        // da PROPRIA rodada (kBaseMana + round_index). Um segundo turno na mesma rodada
        // apareceria aqui como uma segunda recarga - a evidencia que o jogador sente.
        for (const TurnStart& t : turns) {
            INFO("turno de '" << t.actor_id << "' na rodada " << t.round);
            CHECK(t.ap == combat_constants::kBaseApPerTurn);
            CHECK(t.mana == std::min(combat_constants::kManaCap,
                                     combat_constants::kBaseMana + t.round));
        }

        // As rodadas observadas formam uma sequencia SEM buraco a partir de 0 (o contador
        // nao pula rodada por causa de um wrap extra em remove()).
        int max_round = 0;
        for (const TurnStart& t : turns) max_round = std::max(max_round, t.round);
        for (int r = 0; r <= max_round; ++r) {
            INFO("rodada " << r << " precisa ter ao menos um turno");
            CHECK(std::any_of(turns.begin(), turns.end(),
                              [r](const TurnStart& t) { return t.round == r; }));
        }
    }
}
