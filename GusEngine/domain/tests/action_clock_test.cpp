// action_clock_test.cpp
//
// Spec executavel (Catch2 v3) do ActionClock ISOLADO (ADR-017, pedido do Gus). Prova a
// mecanica (escalonador por ticks + desempate D3 + 3 estilos + decaimento do Agil +
// starvation-guard D1 + PendingResolution) contra os 6 casos de exemplo do pedido
// (docs/design/mecanicas/pedido-arceus-battle-engine.md), TDD antes da implementacao.
// POCO puro, ZERO Qt/SDL. NAO toca CombatStateMachine/InitiativeQueue de producao - modulo
// novo e autonomo (o cutover e onda pos-M7).
//
// Convencao de teste: SPD=10 pra ambos os lados nos Casos 1-6 (nao especificado pelo
// pedido/ADR - e uma escolha do teste, nao um numero de balance), cardSpeedMult =
// Basico (1.0, ataque generico sem carta - os casos do pedido nao falam de linguagem de
// carta). BASE_CLOCK/styleMult/decaimento SEM alteracao dos valores baseline do ADR-017.
//
// Cross-ref: docs/tech/adr/ADR-017-action-clock-combat-unificado.md;
//            docs/design/mecanicas/pedido-arceus-battle-engine.md.

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <vector>

#include "gus/domain/combat/action_clock.hpp"

using namespace gus::domain::combat;

namespace {

constexpr ActorId kP1 = 1;
constexpr ActorId kP2 = 2;
constexpr int kSpd = 10;  // ambos os lados, Casos 1-6 (convencao do teste).

// Inicializa um clock de 2 atores com SPD igual, cadencia inicial natural
// round(BASE_CLOCK/SPD) = 100, P1 com preferencia (menor preference_order).
ActionClock make_two_actor_clock() {
    ActionClock clock;
    clock.add_actor(kP1, /*initial_next_action_at=*/100, kSpd, /*preference_order=*/0);
    clock.add_actor(kP2, /*initial_next_action_at=*/100, kSpd, /*preference_order=*/1);
    return clock;
}

}  // namespace

// ---------------------------------------------------------------------------------
// Caso 1 (ambos Normal): P1 -> P2 -> P1 -> P2 -> P1 -> P2.
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock Caso 1: ambos Normal alterna estritamente", "[action_clock][caso1]") {
    ActionClock clock = make_two_actor_clock();
    std::vector<ActorId> order;
    for (int i = 0; i < 6; ++i) {
        ActorId who = clock.advance();
        order.push_back(who);
        clock.reset_after_action(who, kSpd, CardSpeedClass::Basico, CastStyle::Normal);
    }
    std::vector<ActorId> expected{kP1, kP2, kP1, kP2, kP1, kP2};
    REQUIRE(order == expected);
}

// ---------------------------------------------------------------------------------
// Caso 2 (P1 Forte, P2 Normal): P1-Forte -> P2 -> P2 -> P1-Forte -> P2 -> P2.
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock Caso 2: P1 Forte constante da 2 turnos de P2 por turno de P1",
          "[action_clock][caso2]") {
    ActionClock clock = make_two_actor_clock();
    std::vector<ActorId> order;
    for (int i = 0; i < 6; ++i) {
        ActorId who = clock.advance();
        order.push_back(who);
        CastStyle style = (who == kP1) ? CastStyle::Forte : CastStyle::Normal;
        clock.reset_after_action(who, kSpd, CardSpeedClass::Basico, style);
    }
    std::vector<ActorId> expected{kP1, kP2, kP2, kP1, kP2, kP2};
    REQUIRE(order == expected);
}

// ---------------------------------------------------------------------------------
// Caso 3 (P1 Agil continuo): double-turn CONTROLADO (surge apos o warm-up, nao
// infinito - o decaimento suave trava o mult do Agil em 0.90 a partir da 3a
// consecutiva). Ver relatorio: o pedido ilustra um "double-turn a cada ciclo" a mao
// (sem formula real); a formula real com decaimento produz double-turn OCASIONAL, que
// e exatamente o que o ADR pede ("controlado, com limite" - nao "todo ciclo").
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock Caso 3: Agil continuo eventualmente da double-turn controlado",
          "[action_clock][caso3]") {
    ActionClock clock = make_two_actor_clock();
    std::vector<ActorId> order;
    for (int i = 0; i < 13; ++i) {
        ActorId who = clock.advance();
        order.push_back(who);
        CastStyle style = (who == kP1) ? CastStyle::Agil : CastStyle::Normal;
        clock.reset_after_action(who, kSpd, CardSpeedClass::Basico, style);
    }

    // As 10 primeiras acoes alternam estritamente (o Agil ainda nao acumulou
    // vantagem suficiente pra ultrapassar P2 duas vezes seguidas).
    for (int i = 0; i < 10; ++i) {
        REQUIRE(order[static_cast<std::size_t>(i)] == ((i % 2 == 0) ? kP1 : kP2));
    }

    // O double-turn emerge nas posicoes 11-12 (indices 10,11): P1 age duas vezes
    // seguidas - prova que o Agil consegue double-turn de fato.
    REQUIRE(order[10] == kP1);
    REQUIRE(order[11] == kP1);

    // "Com limite": logo em seguida P2 volta a agir (nao vira P1 monopolizando).
    REQUIRE(order[12] == kP2);
}

TEST_CASE("ActionClock Caso 3: decaimento do Agil consecutivo trava em 0.90 (nao infinito)",
          "[action_clock][caso3][decay]") {
    ActionClock clock;
    clock.add_actor(kP1, 100, kSpd, 0);

    // 1a Agil: streak vira 1, mult 0.55 -> delta = round(1000/10 * 1.0 * 0.55) = 55.
    ActorId who = clock.advance();
    REQUIRE(who == kP1);
    int tick_before = clock.global_tick();
    clock.reset_after_action(who, kSpd, CardSpeedClass::Basico, CastStyle::Agil);
    REQUIRE(clock.entry_of(kP1).next_action_at - tick_before == 55);
    REQUIRE(clock.entry_of(kP1).agile_streak == 1);

    // 2a Agil consecutiva: streak 2, mult 0.75 -> delta = 75.
    who = clock.advance();
    tick_before = clock.global_tick();
    clock.reset_after_action(who, kSpd, CardSpeedClass::Basico, CastStyle::Agil);
    REQUIRE(clock.entry_of(kP1).next_action_at - tick_before == 75);
    REQUIRE(clock.entry_of(kP1).agile_streak == 2);

    // 3a Agil consecutiva: streak 3, mult 0.90 -> delta = 90.
    who = clock.advance();
    tick_before = clock.global_tick();
    clock.reset_after_action(who, kSpd, CardSpeedClass::Basico, CastStyle::Agil);
    REQUIRE(clock.entry_of(kP1).next_action_at - tick_before == 90);
    REQUIRE(clock.entry_of(kP1).agile_streak == 3);

    // 4a Agil consecutiva: "3a+" continua 0.90 (nao decai mais / nao acelera mais -
    // trava o double-turn infinito).
    who = clock.advance();
    tick_before = clock.global_tick();
    clock.reset_after_action(who, kSpd, CardSpeedClass::Basico, CastStyle::Agil);
    REQUIRE(clock.entry_of(kP1).next_action_at - tick_before == 90);
    REQUIRE(clock.entry_of(kP1).agile_streak == 4);

    // Usar Normal reseta o contador (D3: "o contador por-ator reseta ao usar
    // Normal/Forte").
    who = clock.advance();
    clock.reset_after_action(who, kSpd, CardSpeedClass::Basico, CastStyle::Normal);
    REQUIRE(clock.entry_of(kP1).agile_streak == 0);
}

// ---------------------------------------------------------------------------------
// Caso 4 (ambos Forte na 1a acao, depois Normal): P1-Forte -> P2-Forte -> P1 -> P2 ->
// P1 -> P2 ("mais justo").
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock Caso 4: ambos Forte uma vez, depois normal, mantem alternancia",
          "[action_clock][caso4]") {
    ActionClock clock = make_two_actor_clock();
    std::vector<ActorId> order;
    int forte_used = 0;
    for (int i = 0; i < 6; ++i) {
        ActorId who = clock.advance();
        order.push_back(who);
        CastStyle style = (forte_used < 2) ? CastStyle::Forte : CastStyle::Normal;
        ++forte_used;
        clock.reset_after_action(who, kSpd, CardSpeedClass::Basico, style);
    }
    std::vector<ActorId> expected{kP1, kP2, kP1, kP2, kP1, kP2};
    REQUIRE(order == expected);
}

// ---------------------------------------------------------------------------------
// Caso 5 (P1 Forte SEMPRE, mais longo que o Caso 2): o pedido ilustra (a mao, sem
// formula real) um gap de P2 que CRESCE (P2 chegando a agir 3x seguidas). ACHADO
// reportado (ver relatorio final): com a formula real do ADR-017 - Forte fixo 1.60x
// que NAO escala com uso repetido (ao contrario do Agil, que TEM decaimento) e SPD
// igual - o ratio fica LIMITADO em no maximo 2 turnos de P2 por turno de P1, para
// SEMPRE (verificado ate 40 acoes; nunca surge uma sequencia de 3 P2 seguidos). Isto
// e ARGUAVELMENTE o comportamento mais SEGURO (nenhuma das duas alavancas produz
// vantagem runaway), mas diverge da ilustracao literal do pedido. NAO alterei nenhuma
// constante do ADR pra forcar o "3 seguidos" - documentado aqui como achado pra
// confirmar com o lider/lead-game-designer no N=3 (talvez Forte precise de um
// decaimento proprio, espelhando o do Agil, se o "preso vendo o oponente" tiver que
// ficar mais severo).
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock Caso 5: Forte espamado da vantagem estavel (LIMITADA) a P2, nao "
          "crescente",
          "[action_clock][caso5]") {
    ActionClock clock = make_two_actor_clock();
    std::vector<ActorId> order;
    for (int i = 0; i < 13; ++i) {
        ActorId who = clock.advance();
        order.push_back(who);
        CastStyle style = (who == kP1) ? CastStyle::Forte : CastStyle::Normal;
        clock.reset_after_action(who, kSpd, CardSpeedClass::Basico, style);
    }

    // Primeiro ciclo identico ao Caso 2, e o padrao continua estavel (verificado
    // exaustivamente pra 13 acoes - trace completo, nao so o inicio).
    std::vector<ActorId> expected{kP1, kP2, kP2, kP1, kP2, kP2, kP1,
                                   kP2, kP1, kP2, kP2, kP1, kP2};
    REQUIRE(order == expected);

    // A maior sequencia consecutiva de P2 fica em 2 (nunca 3+) - o achado acima.
    int longest_p2_streak = 0;
    int current_streak = 0;
    for (ActorId a : order) {
        if (a == kP2) {
            ++current_streak;
            longest_p2_streak = std::max(longest_p2_streak, current_streak);
        } else {
            current_streak = 0;
        }
    }
    REQUIRE(longest_p2_streak == 2);
}

// ---------------------------------------------------------------------------------
// Caso 6 (P1 Forte + P2 Agil, assimetrico): P1-Forte -> P2-Agil -> P2 -> P2 -> P1 ->
// P2.
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock Caso 6: P1 Forte vs P2 Agil produz padrao assimetrico",
          "[action_clock][caso6]") {
    ActionClock clock = make_two_actor_clock();
    std::vector<ActorId> order;
    for (int i = 0; i < 6; ++i) {
        ActorId who = clock.advance();
        order.push_back(who);
        CastStyle style = (who == kP1) ? CastStyle::Forte : CastStyle::Agil;
        clock.reset_after_action(who, kSpd, CardSpeedClass::Basico, style);
    }
    std::vector<ActorId> expected{kP1, kP2, kP2, kP2, kP1, kP2};
    REQUIRE(order == expected);
}

// ---------------------------------------------------------------------------------
// Desempate D3 completo: next_action_at asc -> last_acted_at asc -> SPD desc ->
// preferencia asc. Testado em isolamento (1 dimensao por teste).
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock desempate: last_acted_at menor (esperou mais) vence mesmo com "
          "preferencia pior",
          "[action_clock][tiebreak]") {
    ActionClock clock;
    // P1 "preferido" (preference_order menor) mas agiu mais recentemente.
    clock.add_actor(kP1, 100, kSpd, /*preference_order=*/0, /*is_protected=*/false,
                     /*initial_last_acted_at=*/0);
    clock.add_actor(kP2, 100, kSpd, /*preference_order=*/1, /*is_protected=*/false,
                     /*initial_last_acted_at=*/-10);
    REQUIRE(clock.advance() == kP2);
}

TEST_CASE("ActionClock desempate: SPD maior vence quando next_action_at e last_acted_at "
          "empatam",
          "[action_clock][tiebreak]") {
    ActionClock clock;
    clock.add_actor(kP1, 100, /*spd=*/5, /*preference_order=*/0, /*is_protected=*/false,
                     /*initial_last_acted_at=*/0);
    clock.add_actor(kP2, 100, /*spd=*/20, /*preference_order=*/1, /*is_protected=*/false,
                     /*initial_last_acted_at=*/0);
    REQUIRE(clock.advance() == kP2);
}

TEST_CASE("ActionClock desempate: preferencia (menor preference_order) e o desempate "
          "final",
          "[action_clock][tiebreak]") {
    ActionClock clock;
    clock.add_actor(kP1, 100, kSpd, /*preference_order=*/3, /*is_protected=*/false,
                     /*initial_last_acted_at=*/0);
    clock.add_actor(kP2, 100, kSpd, /*preference_order=*/1, /*is_protected=*/false,
                     /*initial_last_acted_at=*/0);
    REQUIRE(clock.advance() == kP2);
}

// ---------------------------------------------------------------------------------
// Starvation-guard (D1 hibrido): o ator protegido NUNCA fica mais que
// MAX_STARVE_TICKS sem agir, mesmo espamando Forte com SPD baixo (delta natural
// 320 > MAX_STARVE_TICKS de teste 250).
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock starvation-guard: catch-up bump trava o gap do protegido",
          "[action_clock][starvation]") {
    constexpr int kMaxStarve = 250;
    ActionClock clock(kMaxStarve);
    // P1 protegido (party), SPD baixo, sempre Forte (delta natural sem guard = 320).
    clock.add_actor(kP1, 200, /*spd=*/5, /*preference_order=*/0, /*is_protected=*/true);
    // P2 rapido (inimigo), sem protecao.
    clock.add_actor(kP2, 10, /*spd=*/100, /*preference_order=*/1, /*is_protected=*/false);

    for (int i = 0; i < 60; ++i) {
        ActorId who = clock.advance();
        // Invariante checada A CADA advance(): mesmo se P1 nao venceu esta rodada, o
        // guard ja garantiu que o gap agendado dele nao ultrapassa o maximo.
        const ActionClockEntry& p1_entry = clock.entry_of(kP1);
        REQUIRE(p1_entry.next_action_at - p1_entry.last_acted_at <= kMaxStarve);

        CastStyle style = (who == kP1) ? CastStyle::Forte : CastStyle::Normal;
        clock.reset_after_action(who, (who == kP1) ? 5 : 100, CardSpeedClass::Basico,
                                  style);
    }
}

// ---------------------------------------------------------------------------------
// PendingResolution (cast interpretado): agenda, dispara no tick certo, cancelavel
// antes.
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock PendingResolution: dispara exatamente no resolve_at agendado",
          "[action_clock][pending]") {
    ActionClock clock;
    clock.add_actor(kP1, 100, kSpd, 0);

    ActorId who = clock.advance();  // global_tick = 100
    int pending_id = clock.schedule_pending(kP1, /*resolve_at=*/300);
    REQUIRE(clock.collect_due_pending().empty());

    clock.reset_after_action(who, kSpd, CardSpeedClass::Basico, CastStyle::Normal);
    who = clock.advance();  // global_tick = 200
    REQUIRE(clock.collect_due_pending().empty());
    REQUIRE_FALSE(clock.is_pending_fired(pending_id));

    clock.reset_after_action(who, kSpd, CardSpeedClass::Basico, CastStyle::Normal);
    who = clock.advance();  // global_tick = 300 -> devido
    std::vector<int> due = clock.collect_due_pending();
    REQUIRE(due.size() == 1);
    REQUIRE(due[0] == pending_id);
    REQUIRE(clock.is_pending_fired(pending_id));
    REQUIRE_FALSE(clock.is_pending_cancelled(pending_id));

    // Uma segunda coleta nao re-dispara a mesma pendencia (one-shot).
    REQUIRE(clock.collect_due_pending().empty());
}

TEST_CASE("ActionClock PendingResolution: cancel_pending impede o disparo",
          "[action_clock][pending][cancel]") {
    ActionClock clock;
    clock.add_actor(kP1, 100, kSpd, 0);

    ActorId who = clock.advance();  // tick 100
    int pending_id = clock.schedule_pending(kP1, /*resolve_at=*/300);
    clock.cancel_pending(kP1);
    REQUIRE(clock.is_pending_cancelled(pending_id));

    clock.reset_after_action(who, kSpd, CardSpeedClass::Basico, CastStyle::Normal);
    who = clock.advance();  // 200
    clock.reset_after_action(who, kSpd, CardSpeedClass::Basico, CastStyle::Normal);
    who = clock.advance();  // 300 -> seria devido, mas foi cancelada antes
    std::vector<int> due = clock.collect_due_pending();
    REQUIRE(due.empty());
    REQUIRE_FALSE(clock.is_pending_fired(pending_id));
}

// ---------------------------------------------------------------------------------
// Determinismo: mesma entrada/script produz a MESMA ordem e os MESMOS ticks sempre.
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock e deterministico: mesma configuracao produz o mesmo trace",
          "[action_clock][determinism]") {
    auto run = []() {
        ActionClock clock = make_two_actor_clock();
        std::vector<std::pair<ActorId, int>> trace;
        std::vector<CastStyle> script{CastStyle::Agil,  CastStyle::Normal, CastStyle::Forte,
                                       CastStyle::Normal, CastStyle::Agil,  CastStyle::Forte,
                                       CastStyle::Normal, CastStyle::Agil};
        for (int i = 0; i < 8; ++i) {
            ActorId who = clock.advance();
            trace.emplace_back(who, clock.global_tick());
            clock.reset_after_action(who, kSpd, CardSpeedClass::Basico,
                                      script[static_cast<std::size_t>(i)]);
        }
        return trace;
    };

    REQUIRE(run() == run());
}

// ---------------------------------------------------------------------------------
// Ticks INTEIROS: o estado guardado e sempre int (nunca float persistente); a formula
// arredonda (lround) na hora do reset, mesmo quando a divisao nao e exata.
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock ticks sao inteiros: campos int + arredondamento na formula",
          "[action_clock][ticks]") {
    static_assert(std::is_same_v<decltype(ActionClockEntry::next_action_at), int>);
    static_assert(std::is_same_v<decltype(ActionClockEntry::last_acted_at), int>);

    ActionClock clock;
    clock.add_actor(kP1, 100, /*spd=*/3, /*preference_order=*/0);
    ActorId who = clock.advance();
    int tick_before = clock.global_tick();
    // 1000/3 = 333.333...; Normal mult 1.0 -> round(333.333) = 333.
    clock.reset_after_action(who, 3, CardSpeedClass::Basico, CastStyle::Normal);
    REQUIRE(clock.entry_of(kP1).next_action_at - tick_before == 333);

    ActionClock clock2;
    clock2.add_actor(kP1, 100, /*spd=*/7, /*preference_order=*/0);
    who = clock2.advance();
    tick_before = clock2.global_tick();
    // 1000/7 = 142.857...; Agil 1a (mult 0.55) -> 142.857*0.55 = 78.571 -> round = 79.
    clock2.reset_after_action(who, 7, CardSpeedClass::Basico, CastStyle::Agil);
    REQUIRE(clock2.entry_of(kP1).next_action_at - tick_before == 79);
}

// ---------------------------------------------------------------------------------
// cardSpeedMult (combat-flavor.md secao 1-2, unificado pelo ADR-017): compilada 0.85,
// interpretada 1.35, basico 1.00.
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock card_speed_mult resolve a tabela do ADR-017", "[action_clock][cards]") {
    REQUIRE(ActionClock::card_speed_mult(CardSpeedClass::Compilada) == 0.85f);
    REQUIRE(ActionClock::card_speed_mult(CardSpeedClass::Interpretada) == 1.35f);
    REQUIRE(ActionClock::card_speed_mult(CardSpeedClass::Basico) == 1.00f);
}

// ---------------------------------------------------------------------------------
// Fail-fast: acessar ator inexistente ou avancar clock vazio lanca excecao (nao
// undefined behavior silencioso).
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock lanca excecao em clock vazio ou ator ausente",
          "[action_clock][errors]") {
    ActionClock empty_clock;
    REQUIRE_THROWS_AS(empty_clock.advance(), std::logic_error);

    ActionClock clock;
    clock.add_actor(kP1, 100, kSpd, 0);
    REQUIRE_THROWS_AS(clock.entry_of(kP2), std::invalid_argument);
    REQUIRE_THROWS_AS(clock.reset_after_action(kP2, kSpd, CardSpeedClass::Basico,
                                                CastStyle::Normal),
                      std::invalid_argument);
}

// ---------------------------------------------------------------------------------
// Cobertura de mutation testing (achados do qa-engineer, revisao adversarial
// independente): os 17 casos acima usam CardSpeedClass::Basico (mult 1.0) em TODA
// chamada de reset_after_action - onde x1.0 e /1.0 coincidem, entao uma inversao
// (x -> /) ou uma troca de linha na tabela de card_speed_mult DENTRO da formula de
// reset nunca e exercitada de verdade (card_speed_mult() isolado ja e testado acima,
// mas o valor pode nunca ser MULTIPLICADO corretamente no calculo do delta). Este teste
// fecha esse buraco usando Compilada/Interpretada (mult != 1.0) e conferindo o delta
// exato.
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock card_speed_mult e MULTIPLICADO (nao invertido) dentro da formula "
          "de reset",
          "[action_clock][cards][formula]") {
    // SPD=10, estilo Normal (styleMult=1.0), Compilada (0.85):
    // delta = lround(1000/10 * 0.85 * 1.0) = lround(85.0) = 85.
    ActionClock clock;
    clock.add_actor(kP1, 100, /*spd=*/10, /*preference_order=*/0);
    ActorId who = clock.advance();
    int tick_before = clock.global_tick();
    clock.reset_after_action(who, 10, CardSpeedClass::Compilada, CastStyle::Normal);
    REQUIRE(clock.entry_of(kP1).next_action_at - tick_before == 85);

    // Mesma SPD/estilo, Interpretada (1.35):
    // delta = lround(1000/10 * 1.35 * 1.0) = lround(135.0) = 135.
    ActionClock clock2;
    clock2.add_actor(kP1, 100, /*spd=*/10, /*preference_order=*/0);
    who = clock2.advance();
    tick_before = clock2.global_tick();
    clock2.reset_after_action(who, 10, CardSpeedClass::Interpretada, CastStyle::Normal);
    REQUIRE(clock2.entry_of(kP1).next_action_at - tick_before == 135);
}

// ---------------------------------------------------------------------------------
// global_tick() em valor ABSOLUTO: todos os testes acima checam DELTAS relativos
// (next_action_at - tick_before), que cancelam um offset sistematico no avanco do
// relogio (ex.: global_tick_ = winner.next_action_at - 1, ou +1). Este teste fecha o
// buraco checando o valor absoluto de global_tick() antes e depois de advance().
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock global_tick() reflete o valor absoluto exato do next_action_at "
          "do vencedor",
          "[action_clock][global_tick]") {
    ActionClock clock;
    // Valor deliberadamente nao-redondo (nao multiplo de BASE_CLOCK/SPD), pra nao
    // coincidir por acidente com nenhum offset de +-1.
    clock.add_actor(kP1, /*initial_next_action_at=*/137, kSpd, 0);
    REQUIRE(clock.global_tick() == 0);  // "0 antes da primeira chamada" (contrato do header).

    ActorId who = clock.advance();
    REQUIRE(who == kP1);
    REQUIRE(clock.global_tick() == 137);
}

// ---------------------------------------------------------------------------------
// Desempate D3 com 2 dimensoes em CONFLITO direto: os testes de tiebreak acima isolam
// uma dimensao por vez (as demais empatadas), entao uma troca na ORDEM dos criterios
// (ex.: comparar SPD antes de last_acted_at) pode passar despercebida se a dimensao
// testada isoladamente "ganhar de qualquer jeito". Aqui P1 esperou mais (last_acted_at
// menor, deveria vencer por essa regra) MAS tem SPD menor (perderia se SPD fosse
// comparado primeiro); P2 e o oposto em ambas as dimensoes. So um desempate que
// realmente aplica last_acted_at ANTES de SPD (ADR-017 D3: next_action_at asc ->
// last_acted_at asc -> SPD desc -> preferencia) acerta o vencedor.
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock desempate D3: last_acted_at tem prioridade sobre SPD quando as "
          "duas dimensoes conflitam",
          "[action_clock][tiebreak][d3-order]") {
    ActionClock clock;
    // P1: esperou mais (last_acted_at = -10, menor) mas SPD baixo (5).
    clock.add_actor(kP1, 100, /*spd=*/5, /*preference_order=*/0, /*is_protected=*/false,
                     /*initial_last_acted_at=*/-10);
    // P2: agiu ha pouco (last_acted_at = 0, maior) mas SPD alto (20).
    clock.add_actor(kP2, 100, /*spd=*/20, /*preference_order=*/1, /*is_protected=*/false,
                     /*initial_last_acted_at=*/0);
    REQUIRE(clock.advance() == kP1);
}

// ---------------------------------------------------------------------------------
// PendingResolution na fronteira exata: os testes de pending acima so amostram em
// multiplos do delta natural (100/200/300), entao um disparo antecipado em exatamente 1
// tick (ex.: mutante que troca `resolve_at <= global_tick()` por
// `resolve_at <= global_tick() + 1`) passaria despercebido. Agenda resolve_at = (proximo
// tick natural de P1) + 1; um segundo ator (kP2) so existe pra fornecer esse tick exato
// como proxima parada do relogio, sem interferir na acao de P1.
// ---------------------------------------------------------------------------------
TEST_CASE("ActionClock PendingResolution: nao dispara 1 tick antes do resolve_at "
          "agendado",
          "[action_clock][pending][boundary]") {
    ActionClock clock;
    clock.add_actor(kP1, 100, kSpd, 0);
    // kP2 parado exatamente em 201 (= proximo tick natural de P1 [200] + 1) - so pra dar
    // ao relogio uma parada exata em 201 mais adiante, sem agir de verdade.
    clock.add_actor(kP2, 201, /*spd=*/1, /*preference_order=*/1);

    ActorId who = clock.advance();  // global_tick = 100 (kP1, 100 < 201)
    REQUIRE(who == kP1);

    // proximo tick natural de kP1 (SPD=10, Normal): 100 + round(1000/10) = 200.
    int pending_id = clock.schedule_pending(kP1, /*resolve_at=*/201);

    clock.reset_after_action(who, kSpd, CardSpeedClass::Basico, CastStyle::Normal);
    // kP1.next_action_at agora = 200.

    who = clock.advance();  // global_tick = 200 (kP1 de novo, 200 < 201) - o tick NATURAL,
                             // exatamente 1 antes do resolve_at agendado (201).
    REQUIRE(who == kP1);
    REQUIRE(clock.global_tick() == 200);
    REQUIRE(clock.collect_due_pending().empty());
    REQUIRE_FALSE(clock.is_pending_fired(pending_id));

    // Empurra kP1 bem pra frente (SPD=1 -> delta=1000) pra nao vencer de novo antes de
    // kP2, e avanca mais uma vez: agora kP2 (parado em 201) vence e o relogio alcanca o
    // resolve_at exatamente.
    clock.reset_after_action(who, /*spd=*/1, CardSpeedClass::Basico, CastStyle::Normal);

    who = clock.advance();  // global_tick = 201 -> devido
    REQUIRE(who == kP2);
    REQUIRE(clock.global_tick() == 201);
    std::vector<int> due = clock.collect_due_pending();
    REQUIRE(due.size() == 1);
    REQUIRE(due[0] == pending_id);
    REQUIRE(clock.is_pending_fired(pending_id));
}
