// combat_state_machine_test.cpp
//
// Spec executavel (Catch2 v3) da FSM de combate (secao 3): transicoes, loop de AP em
// ActionSelect, recarga de AP/mana em TurnStart (ramp + cap + sem carry-over), CheckEnd
// (3 condicoes), CombatEnd outcome. Portado de
// engine/tests/turn_combat/CombatStateMachineTests.cs. POCO puro, ZERO Qt, headless.
//
// A FSM e POCO testavel sem Godot. A selecao de acao e injetada via std::function
// (CombatActionProvider) pra exercitar o loop interno de forma deterministica. Os atores
// vivem no escopo do teste (locais); a FSM/fila guardam ponteiros NAO-DONOS (mesmo padrao
// de InitiativeQueue/CombatState ja portados).
//
// Cross-ref: engine/tests/turn_combat/CombatStateMachineTests.cs;
//            docs/design/mecanicas/combat.md secao 3/4/5.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"  // kMinDamage (piso do dano/previa)
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"

using namespace gus::domain::combat;

namespace {

CombatActor hero(const std::string& id = "gus", int hp = 30, int spd = 20, int atk = 8,
                 int def = 2) {
    return CombatActor(id, id, hp, atk, def, spd, CardFamily::Eletrico, /*player=*/true);
}

CombatActor foe(const std::string& id = "enemy", int hp = 30, int spd = 10, int atk = 6,
                int def = 1) {
    return CombatActor(id, id, hp, atk, def, spd, CardFamily::Cinetico, /*player=*/false);
}

CombatAction always_pass(CombatActor&, const CombatState&) { return CombatAction::pass(); }

std::vector<std::string> order_ids(const CombatStateMachine& sm) {
    std::vector<std::string> ids;
    for (const CombatActor* a : sm.queue().order())
        ids.push_back(a->id());
    return ids;
}

bool log_has(const CombatStateMachine& sm, CombatActionType action,
             const std::string& needle) {
    for (const auto& e : sm.log()) {
        if (e.action == action && e.message.find(needle) != std::string::npos)
            return true;
    }
    return false;
}

}  // namespace

// ----- Setup / fase inicial -----

TEST_CASE("fsm: SetupPhase monta fila por spd e inicia em TurnStart resolvido",
          "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20);
    CombatActor e = foe("enemy", 30, 10);
    CombatStateMachine sm({&h, &e}, always_pass);

    REQUIRE(order_ids(sm) == std::vector<std::string>{"gus", "enemy"});
    REQUIRE(sm.outcome() == CombatOutcome::Ongoing);
}

// ----- TurnStart: AP e mana -----

TEST_CASE("fsm: TurnStart recarrega ap para 3", "[domain][combat][fsm]") {
    CombatActor h = hero();
    CombatActor e = foe();
    CombatStateMachine sm({&h, &e}, always_pass);

    sm.begin_turn();
    REQUIRE(sm.active_actor()->ap() == 3);
    REQUIRE(sm.active_actor()->max_ap() == 3);
}

TEST_CASE("fsm: TurnStart mana ramp 2 mais turno_index", "[domain][combat][fsm]") {
    CombatActor h = hero();
    CombatActor e = foe();
    CombatStateMachine sm({&h, &e}, always_pass);

    sm.begin_turn();
    REQUIRE(sm.active_actor()->max_mana() == 2);
    REQUIRE(sm.active_actor()->mana() == 2);
}

TEST_CASE("fsm: mana sobe com o avanco das rodadas", "[domain][combat][fsm]") {
    CombatActor h = hero();
    CombatStateMachine sm({&h}, always_pass);

    sm.begin_turn();
    REQUIRE(h.max_mana() == 2);
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();

    sm.begin_turn();
    REQUIRE(h.max_mana() == 3);
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();

    sm.begin_turn();
    REQUIRE(h.max_mana() == 4);
}

TEST_CASE("fsm: mana capa em 8", "[domain][combat][fsm]") {
    CombatActor h = hero();
    CombatStateMachine sm({&h}, always_pass);

    for (int i = 0; i < 8; ++i) {
        sm.begin_turn();
        sm.run_active_turn_to_end();
        sm.advance_to_next_actor();
    }
    sm.begin_turn();
    REQUIRE(h.max_mana() == 8);
}

TEST_CASE("fsm: mana nao tem carry-over recarrega ao maximo", "[domain][combat][fsm]") {
    CombatActor h = hero();
    CombatStateMachine sm({&h}, always_pass);

    sm.begin_turn();
    h.spend_mana(2);
    REQUIRE(h.mana() == 0);
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();

    sm.begin_turn();
    REQUIRE(h.mana() == h.max_mana());
    REQUIRE(h.mana() == 3);
}

// ----- ActionSelect: loop de AP -----

TEST_CASE("fsm: ActionSelect consome 3 attacks ate ap zerar", "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/10);
    CombatActor e = foe("enemy", 100, 10, 6, /*def=*/0);
    int attacks = 0;

    CombatStateMachine sm({&h, &e}, [&](CombatActor&, const CombatState&) {
        ++attacks;
        return CombatAction::attack(e.id());
    });
    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(attacks == 3);
    REQUIRE(h.ap() == 0);
    REQUIRE(e.hp() == 70);
}

TEST_CASE("fsm: pass encerra turno imediatamente sem gastar ap",
          "[domain][combat][fsm]") {
    CombatActor h = hero();
    CombatActor e = foe();
    CombatStateMachine sm({&h, &e}, always_pass);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(h.ap() == 3);
}

TEST_CASE("fsm: para quando provider devolve pass no meio", "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, 10);
    CombatActor e = foe("enemy", 100, 10, 6, 0);
    int count = 0;
    CombatStateMachine sm({&h, &e}, [&](CombatActor&, const CombatState&) {
        ++count;
        return count == 1 ? CombatAction::attack(e.id()) : CombatAction::pass();
    });
    sm.begin_turn();
    sm.run_active_turn_to_end();

    REQUIRE(h.ap() == 2);
    REQUIRE(e.hp() == 90);
}

// ----- Ataque basico stub (clamp min 1) -----

TEST_CASE("fsm: attack stub dano e atk menos def clamp minimo 1",
          "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/5);
    CombatActor e = foe("enemy", 50, 10, 6, /*def=*/99);
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return &a == &h ? CombatAction::attack(e.id()) : CombatAction::pass();
    });
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(e.hp() == 47);  // 3 ataques de clamp 1
}

// ----- CheckEnd: 3 condicoes -----

TEST_CASE("fsm: CheckEnd victory quando todos inimigos mortos",
          "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/1000);
    CombatActor e = foe("enemy", 5, 10, 6, /*def=*/0);
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return &a == &h ? CombatAction::attack(e.id()) : CombatAction::pass();
    });
    sm.run_until_end();
    REQUIRE(sm.outcome() == CombatOutcome::Victory);
}

TEST_CASE("fsm: CheckEnd defeat quando party inteira morta",
          "[domain][combat][fsm]") {
    CombatActor h = hero("gus", /*hp=*/1, /*spd=*/1, 8, /*def=*/0);
    CombatActor e = foe("enemy", 30, /*spd=*/99, /*atk=*/50, 1);
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return a.is_player_side() ? CombatAction::pass() : CombatAction::attack(h.id());
    });
    sm.run_until_end();
    REQUIRE(sm.outcome() == CombatOutcome::Defeat);
}

TEST_CASE("fsm: CheckEnd fled quando fuga bem-sucedida", "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, /*spd=*/100);
    CombatActor e = foe("enemy", 30, /*spd=*/1);
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return a.is_player_side() ? CombatAction::flee() : CombatAction::pass();
    });
    sm.run_until_end();
    REQUIRE(sm.outcome() == CombatOutcome::Fled);
}

// ----- CombatEnd -----

TEST_CASE("fsm: CombatEnd produz resultado com outcome e log",
          "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/1000);
    CombatActor e = foe("enemy", 5, 10, 6, /*def=*/0);
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return &a == &h ? CombatAction::attack(e.id()) : CombatAction::pass();
    });
    const CombatResult result = sm.run_until_end();
    REQUIRE(result.outcome == CombatOutcome::Victory);
    REQUIRE_FALSE(result.log.empty());
    REQUIRE(sm.phase() == CombatPhase::CombatEnd);
}

TEST_CASE("fsm: inimigo morto e removido da fila", "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/1000);
    CombatActor e = foe("enemy", 5, 10, 6, /*def=*/0);
    CombatStateMachine sm({&h, &e}, [&](CombatActor& a, const CombatState&) {
        return &a == &h ? CombatAction::attack(e.id()) : CombatAction::pass();
    });
    sm.run_until_end();
    const auto ids = order_ids(sm);
    REQUIRE(std::find(ids.begin(), ids.end(), "enemy") == ids.end());
    REQUIRE_FALSE(log_has(sm, CombatActionType::Pass, "nunca"));  // sanity (no-op)
}

// ----- Previa de dano do ataque basico (modo-mira §3.5): PURA e read-only -----
// Espelha resolve_basic_attack (dano bruto = max(kMinDamage, atk - def)) + a absorcao de
// Shield (absorb_with_shield) SEM mutar nada. A UI mostra este numero no modo-mira antes
// de confirmar. Ver preview_basic_attack_damage.

namespace {
// Aplica um Shield de pool `magnitude` num ator (mesmo shape do resolve_defend).
void give_shield(CombatActor& a, int magnitude) {
    a.add_status(StatusEffect{StatusId::Shield, magnitude, /*duration=*/1,
                              StackRule::Replace, CardFamily::Eletrico});
}
}  // namespace

TEST_CASE("preview: (a) SEM shield o dano previsto e atk - def", "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/8, /*def=*/2);
    CombatActor e = foe("enemy", 30, 10, /*atk=*/6, /*def=*/3);
    CombatStateMachine sm({&h, &e}, always_pass);

    // raw = max(1, 8 - 3) = 5; sem Shield => perda de HP prevista = 5.
    REQUIRE(sm.preview_basic_attack_damage(h, e) == 5);

    // READ-ONLY: a previa NAO tocou o alvo nem a maquina (HP cheio, sem status, sem log).
    REQUIRE(e.hp() == 30);
    REQUIRE(e.status_effects().empty());
    REQUIRE(sm.log().empty());
}

TEST_CASE("preview: (b) shield PARCIAL reduz a perda prevista", "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/10, /*def=*/2);
    CombatActor e = foe("enemy", 30, 10, /*atk=*/6, /*def=*/2);
    give_shield(e, /*magnitude=*/3);
    CombatStateMachine sm({&h, &e}, always_pass);

    // raw = max(1, 10 - 2) = 8; Shield 3 absorve => perda prevista = 8 - 3 = 5.
    REQUIRE(sm.preview_basic_attack_damage(h, e) == 5);

    // READ-ONLY: o Shield NAO foi consumido pela previa (magnitude segue 3) e o HP segue cheio.
    const int idx = e.index_of_status(StatusId::Shield);
    REQUIRE(idx >= 0);
    REQUIRE(e.status_effects()[static_cast<std::size_t>(idx)].magnitude == 3);
    REQUIRE(e.hp() == 30);
}

TEST_CASE("preview: (c) shield que ABSORVE TUDO zera a perda prevista",
          "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/7, /*def=*/2);
    CombatActor e = foe("enemy", 30, 10, /*atk=*/6, /*def=*/2);
    give_shield(e, /*magnitude=*/50);
    CombatStateMachine sm({&h, &e}, always_pass);

    // raw = max(1, 7 - 2) = 5; Shield 50 >= 5 => perda prevista = max(0, 5 - 50) = 0.
    REQUIRE(sm.preview_basic_attack_damage(h, e) == 0);

    // READ-ONLY: Shield intacto (magnitude 50), HP cheio.
    const int idx = e.index_of_status(StatusId::Shield);
    REQUIRE(idx >= 0);
    REQUIRE(e.status_effects()[static_cast<std::size_t>(idx)].magnitude == 50);
    REQUIRE(e.hp() == 30);
}

TEST_CASE("preview: (d) piso kMinDamage quando atk < def", "[domain][combat][fsm]") {
    CombatActor h = hero("gus", 30, 20, /*atk=*/5, /*def=*/2);
    CombatActor e = foe("enemy", 30, 10, /*atk=*/6, /*def=*/99);
    CombatStateMachine sm({&h, &e}, always_pass);

    // atk 5 < def 99 => raw = max(kMinDamage, 5 - 99) = kMinDamage (1); sem Shield => 1.
    REQUIRE(sm.preview_basic_attack_damage(h, e) == combat_constants::kMinDamage);
    REQUIRE(sm.preview_basic_attack_damage(h, e) == 1);

    // Piso + Shield: raw 1 absorvido por um Shield de 1 => perda prevista = 0.
    give_shield(e, /*magnitude=*/1);
    REQUIRE(sm.preview_basic_attack_damage(h, e) == 0);
}

// ============================================================================
// Janela de Comando da Party (comando livre sobre o CTB, modelo 1B) — combat.md §4.1.
//
// Extensao ADITIVA ao motor: a FORMA da FSM (SetupPhase -> TurnStart -> ActionSelect <->
// ActionResolve -> TurnEnd -> CheckEnd) NAO muda; muda a SELECAO de ator quando e a vez
// do bloco da party. A SPD continua decidindo (a) qual LADO abre a rodada e (b) o membro
// PRE-SELECIONADO ao abrir a Janela. A selecao de ator e INPUT (jogador/host), NAO consome
// RNG (§4.1 / §11). Os testes de transicao existentes seguem validos: forcar o topo e o
// caso particular do conjunto elegivel ter 1 elemento (default sem select = motor pre-1B).
// ============================================================================

namespace {
std::vector<std::string> ids_of(const std::vector<CombatActor*>& v) {
    std::vector<std::string> out;
    out.reserve(v.size());
    for (const CombatActor* a : v) out.push_back(a->id());
    return out;
}
}  // namespace

// ----- (a) SetupPhase / inicio de rodada: SPD do lado decide quem ABRE -----

TEST_CASE("1B: round_opening_side abre pela party quando ela e mais rapida",
          "[domain][combat][party1b]") {
    CombatActor volt = hero("volt", 55, /*spd=*/13);
    CombatActor gus = hero("gus", 34, /*spd=*/9);
    CombatActor e = foe("enemy", 55, /*spd=*/8);
    CombatStateMachine sm({&volt, &gus, &e}, always_pass);
    REQUIRE(sm.round_opening_side() == CombatSide::Party);
}

TEST_CASE("1B: round_opening_side abre pelo enemy quando o inimigo e mais rapido",
          "[domain][combat][party1b]") {
    CombatActor gus = hero("gus", 34, /*spd=*/9);
    CombatActor volt = hero("volt", 55, /*spd=*/8);
    CombatActor e = foe("enemy", 55, /*spd=*/20);
    CombatStateMachine sm({&gus, &volt, &e}, always_pass);
    REQUIRE(sm.round_opening_side() == CombatSide::Enemy);
}

TEST_CASE("1B: round_opening_side empate favorece a party (regra documentada)",
          "[domain][combat][party1b]") {
    CombatActor gus = hero("gus", 34, /*spd=*/10);
    CombatActor e = foe("enemy", 55, /*spd=*/10);
    CombatStateMachine sm({&gus, &e}, always_pass);
    REQUIRE(sm.round_opening_side() == CombatSide::Party);
}

// ----- (b)+(c) PendingPartyActors: completo no inicio, SPD desc, pre-selecionado -----

TEST_CASE("1B: pending_party_actors comeca com toda a party viva, SPD desc; pre-sel = maior SPD",
          "[domain][combat][party1b]") {
    CombatActor volt = hero("volt", 55, /*spd=*/13);
    CombatActor gus = hero("gus", 34, /*spd=*/9);
    CombatActor jaci = hero("jaci", 55, /*spd=*/7);
    CombatActor e = foe("enemy", 999, /*spd=*/5);
    CombatStateMachine sm({&jaci, &gus, &volt, &e}, always_pass);  // ordem de entrada baguncada

    // Ordenado por SPD desc (front = pre-selecionado), independente da ordem de entrada.
    REQUIRE(ids_of(sm.pending_party_actors()) ==
            std::vector<std::string>{"volt", "gus", "jaci"});
    CombatActor* pre = sm.preselected_party_actor();
    REQUIRE(pre != nullptr);
    REQUIRE(pre->id() == "volt");  // maior SPD entre os elegiveis
}

// ----- (b) PendingPartyActors esvazia conforme a party age (dirigido no default) -----

TEST_CASE("1B: pending esvazia conforme a party age e passa para o enemy-block",
          "[domain][combat][party1b]") {
    CombatActor volt = hero("volt", 55, /*spd=*/13);
    CombatActor gus = hero("gus", 34, /*spd=*/9);
    CombatActor e = foe("enemy", 999, /*spd=*/5);
    CombatStateMachine sm({&volt, &gus, &e}, always_pass);

    REQUIRE(ids_of(sm.pending_party_actors()) == std::vector<std::string>{"volt", "gus"});

    // volt age (default: current() = topo = pre-selecionado quando ele esta no cursor):
    sm.begin_turn();
    REQUIRE(sm.active_actor()->id() == "volt");
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();

    REQUIRE(ids_of(sm.pending_party_actors()) == std::vector<std::string>{"gus"});
    CombatActor* pre = sm.preselected_party_actor();
    REQUIRE(pre != nullptr);
    REQUIRE(pre->id() == "gus");

    // gus age:
    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();

    REQUIRE(sm.pending_party_actors().empty());          // bloco da party esvaziou
    REQUIRE(sm.preselected_party_actor() == nullptr);
    REQUIRE(sm.active_actor()->id() == "enemy");          // enemy-block em seguida
}

// ----- (d)+(e) Comando LIVRE: override do pre-selecionado; enemy-block em SPD -----

TEST_CASE("1B: comando livre escolhe a ordem da party (override) e o enemy-block segue SPD",
          "[domain][combat][party1b]") {
    CombatActor volt = hero("volt", 55, /*spd=*/13);
    CombatActor gus = hero("gus", 34, /*spd=*/9);
    CombatActor e1 = foe("e1", 999, /*spd=*/8);
    CombatActor e2 = foe("e2", 999, /*spd=*/5);
    std::vector<std::string> seq;
    CombatStateMachine sm({&volt, &gus, &e1, &e2},
                          [&](CombatActor& a, const CombatState&) {
                              seq.push_back(a.id());  // registra quem AGE
                              return CombatAction::pass();
                          });

    REQUIRE(sm.round_opening_side() == CombatSide::Party);

    // Bloco da party: pre-selecionado = volt (maior SPD); jogador faz OVERRIDE para gus.
    CombatActor* pre = sm.preselected_party_actor();
    REQUIRE(pre != nullptr);
    REQUIRE(pre->id() == "volt");
    sm.select_party_actor(&gus);  // escolhe o NAO-pre-selecionado
    sm.begin_turn();
    REQUIRE(sm.active_actor()->id() == "gus");  // override respeitado pela FSM
    sm.run_active_turn_to_end();
    REQUIRE_FALSE(sm.check_end());
    sm.advance_to_next_actor();

    REQUIRE(ids_of(sm.pending_party_actors()) == std::vector<std::string>{"volt"});
    sm.select_party_actor(&volt);
    sm.begin_turn();
    REQUIRE(sm.active_actor()->id() == "volt");
    sm.run_active_turn_to_end();
    REQUIRE_FALSE(sm.check_end());
    sm.advance_to_next_actor();

    REQUIRE(sm.pending_party_actors().empty());  // bloco da party terminou

    // Bloco dos inimigos: ordem de SPD entre eles (e1 spd8 antes de e2 spd5), IA intacta.
    REQUIRE(sm.active_actor()->id() == "e1");
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE_FALSE(sm.check_end());
    sm.advance_to_next_actor();
    REQUIRE(sm.active_actor()->id() == "e2");
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE_FALSE(sm.check_end());
    sm.advance_to_next_actor();

    // Rodada completa: 4 avancos -> round_index incrementou UMA vez; cada ator agiu 1x.
    REQUIRE(sm.queue().round_index() == 1);
    // Party na ordem ESCOLHIDA (gus, volt); inimigos em SPD (e1, e2).
    REQUIRE(seq == std::vector<std::string>{"gus", "volt", "e1", "e2"});
}

TEST_CASE("1B: select_party_actor rejeita ator inelegivel (fail-fast)",
          "[domain][combat][party1b]") {
    CombatActor gus = hero("gus", 34, /*spd=*/9);
    CombatActor e = foe("enemy", 55, /*spd=*/5);
    CombatStateMachine sm({&gus, &e}, always_pass);
    // inimigo nao e membro elegivel da party:
    REQUIRE_THROWS_AS(sm.select_party_actor(&e), std::invalid_argument);
    // ator fora do combate tambem:
    CombatActor estranho = hero("estranho", 10, 3);
    REQUIRE_THROWS_AS(sm.select_party_actor(&estranho), std::invalid_argument);
}

// ----- (f) Nova rodada recomputa quem abre por SPD -----

TEST_CASE("1B: round_opening_side recomputa quando a SPD muda (Haste flippa o lado)",
          "[domain][combat][party1b]") {
    CombatActor gus = hero("gus", 999, /*spd=*/9);
    CombatActor volt = hero("volt", 999, /*spd=*/8);
    CombatActor e = foe("enemy", 999, /*spd=*/15);
    CombatStateMachine sm({&gus, &volt, &e}, always_pass);

    // Inicio: inimigo mais rapido (15 > 9) -> enemy abre.
    REQUIRE(sm.round_opening_side() == CombatSide::Enemy);

    // Haste +10 em gus (9 -> 19): o tick de TurnStart aplica e recomputa a fila por SPD.
    gus.add_status(
        StatusEffect{StatusId::Haste, 10, 3, StackRule::Replace, CardFamily::Eletrico});
    sm.select_party_actor(&gus);  // gus e elegivel (party, vivo, ainda-nao-agiu)
    sm.begin_turn();              // TurnStart do gus: tick Haste -> recompute_by_speed
    REQUIRE(gus.spd() == 19);

    // Recomputado: agora a party abre (19 > 15).
    REQUIRE(sm.round_opening_side() == CombatSide::Party);
}

// ----- (h) AutoResolveBrain headless: run_until_end multi-party no DEFAULT (sem select) -----

TEST_CASE("1B: run_until_end headless com party multi-membro roda no default (sem select)",
          "[domain][combat][party1b]") {
    CombatActor volt = hero("volt", 55, /*spd=*/13, /*atk=*/1000);
    CombatActor gus = hero("gus", 34, /*spd=*/9, /*atk=*/1000);
    CombatActor e = foe("enemy", 5, /*spd=*/5, /*atk=*/1);
    // Provider sub-otimo (estilo AutoResolveBrain §19.6): party ataca, inimigo passa.
    // NENHUM select_party_actor: a FSM headless usa o default (drive por cursor), intacto.
    CombatStateMachine sm({&volt, &gus, &e}, [](CombatActor& a, const CombatState&) {
        return a.is_player_side() ? CombatAction::attack("enemy") : CombatAction::pass();
    });
    const CombatResult r = sm.run_until_end();
    REQUIRE(r.outcome == CombatOutcome::Victory);
}

// ============================================================================
// Regroup por lado na fronteira da rodada (comando livre 1B fase 2) — combat.md §4.1.
//
// Canon: "A rodada e 'um lado age todo, depois o outro', com a SPD decidindo a ordem dos
// LADOS." Quando o inimigo abre com SPDs INTERCALADAS entre os lados, a ordem bruta de SPD
// alterna os lados; o regroup por lado (std::stable_partition) agrupa "quem abre inteiro,
// depois o outro" SEM re-sortear por SPD dentro do lado (preserva empurroes de Gambito =>
// Gambito-safe). Aplicado NA FRONTEIRA da rodada: construcao (rodada 0) e wrap de advance
// (rodadas 1+). ADITIVO: nao consome RNG, nao muda a FORMA da FSM, nao mexe no comando livre.
// ============================================================================

// ----- (a) Inimigo abre com SPD intercalada -> reagrupa por lado -----

TEST_CASE("1B-regroup: (a) inimigo abre com SPD intercalada -> fila reagrupa por lado",
          "[domain][combat][party1b][regroup]") {
    // Cenario EXATO do encontro-demo: SPDs inimigo3=13, caua=12, inimigo1=11, gus=9,
    // inimigo4=8, jaci=7, inimigo2=6. A ordem bruta de SPD intercala os lados
    // (inimigo3, caua, inimigo1, gus, ...). Como inimigo3 (13) tem a MAIOR SPD, o inimigo
    // ABRE. Reagrupado: TODOS os inimigos (na ordem relativa de SPD 13,11,8,6), depois TODA
    // a party (12,9,7). O regroup NAO re-sorteia por SPD dentro do lado (aqui coincide com
    // SPD desc porque a ordem relativa ja era SPD desc; a prova de stable_partition esta em (b)).
    CombatActor inimigo3 = foe("inimigo3", 999, /*spd=*/13);
    CombatActor caua = hero("caua", 55, /*spd=*/12);
    CombatActor inimigo1 = foe("inimigo1", 999, /*spd=*/11);
    CombatActor gus = hero("gus", 34, /*spd=*/9);
    CombatActor inimigo4 = foe("inimigo4", 999, /*spd=*/8);
    CombatActor jaci = hero("jaci", 55, /*spd=*/7);
    CombatActor inimigo2 = foe("inimigo2", 999, /*spd=*/6);
    // Ordem de ENTRADA embaralhada de proposito: a fila ordena por SPD; o regroup agrupa por lado.
    CombatStateMachine sm({&gus, &inimigo2, &caua, &inimigo4, &jaci, &inimigo1, &inimigo3},
                          always_pass);

    REQUIRE(sm.round_opening_side() == CombatSide::Enemy);  // inimigo3 (13) e o mais rapido
    REQUIRE(order_ids(sm) == std::vector<std::string>{"inimigo3", "inimigo1", "inimigo4",
                                                      "inimigo2", "caua", "gus", "jaci"});
    REQUIRE(sm.queue().cursor() == 0);
    REQUIRE(sm.queue().round_index() == 0);
    REQUIRE(sm.active_actor()->id() == "inimigo3");  // primeiro do lado que abre
}

// ----- (b) Gambito-Reordenar sobrevive ao regroup da rodada seguinte (stable_partition) -----

TEST_CASE("1B-regroup: (b) Gambito-Reordenar DENTRO da rodada SOBREVIVE ao regroup seguinte",
          "[domain][combat][party1b][regroup]") {
    // Prova direta de Gambito-safe no nivel da FSM. A party abre. Na rodada 0, um
    // Gambito-Reordenar empurra o inimigo LENTO e2 -2 (pra FRENTE), passando o membro de party
    // p2 => a fila fica INTERCALADA [p1, e2, p2, e1] E os inimigos ficam FORA da ordem de SPD
    // (e2 spd10 antes de e1 spd25). Na fronteira da rodada 1 o regroup por lado tem trabalho
    // REAL (desintercalar) MAS nao pode desfazer o empurrao: stable_partition reagrupa
    // party-first preservando a ordem relativa => inimigos ficam [e2, e1]. Um SORT restauraria
    // [e1, e2] (SPD desc) e apagaria o Gambito (bug que o regroup Gambito-safe evita).
    CombatActor p1 = hero("p1", 999, /*spd=*/30);
    CombatActor p2 = hero("p2", 999, /*spd=*/20);
    CombatActor e1 = foe("e1", 999, /*spd=*/25);
    CombatActor e2 = foe("e2", 999, /*spd=*/10);
    bool pushed = false;
    CombatStateMachine sm({&p1, &p2, &e1, &e2},
                          [&](CombatActor& a, const CombatState&) -> CombatAction {
                              if (a.id() == "p1" && !pushed) {
                                  pushed = true;
                                  return CombatAction::gambit_reorder("e2", -2);  // -2 = adianta
                              }
                              return CombatAction::pass();
                          });

    // Rodada 0 reagrupada (party abre, 30>25): party [p1,p2], inimigos [e1,e2].
    REQUIRE(order_ids(sm) == std::vector<std::string>{"p1", "p2", "e1", "e2"});

    // p1 (primeiro a agir) empurra e2 -2 -> INTERCALA a fila: [p1, e2, p2, e1].
    sm.begin_turn();
    sm.run_active_turn_to_end();
    REQUIRE(pushed);
    REQUIRE(order_ids(sm) == std::vector<std::string>{"p1", "e2", "p2", "e1"});  // intercalado
    REQUIRE_FALSE(sm.check_end());
    sm.advance_to_next_actor();

    // Completa a rodada 0 (e2, p2, e1 passam) ate a fronteira da rodada 1.
    for (int i = 0; i < 3; ++i) {
        sm.begin_turn();
        sm.run_active_turn_to_end();
        REQUIRE_FALSE(sm.check_end());
        sm.advance_to_next_actor();
    }

    // Fronteira da rodada 1: regroup DESINTERCALA (party-first) mas PRESERVA o empurrao =>
    // inimigos continuam [e2, e1] (lento antes do rapido), nao [e1, e2].
    REQUIRE(sm.queue().round_index() == 1);
    REQUIRE(order_ids(sm) == std::vector<std::string>{"p1", "p2", "e2", "e1"});
    REQUIRE(sm.queue().cursor() == 0);
    REQUIRE(sm.active_actor()->id() == "p1");
}

// ----- (c) Party abre com SPD intercalada: regroup + comando livre convivem (nao regride) -----

TEST_CASE("1B-regroup: (c) party abre intercalada -> reagrupa E o comando livre segue intacto",
          "[domain][combat][party1b][regroup]") {
    // Party abre (p_fast=20 e o mais rapido) mas as SPDs intercalam com o inimigo e_mid=15.
    // O regroup agrupa party-first ([p_fast, p_slow], depois [e_mid, e_slow]) E o comando
    // livre da Fase 1 continua identico por cima: override do pre-selecionado, enemy-block em SPD.
    CombatActor p_fast = hero("p_fast", 999, /*spd=*/20);
    CombatActor e_mid = foe("e_mid", 999, /*spd=*/15);
    CombatActor p_slow = hero("p_slow", 999, /*spd=*/10);
    CombatActor e_slow = foe("e_slow", 999, /*spd=*/5);
    std::vector<std::string> seq;
    CombatStateMachine sm({&p_fast, &e_mid, &p_slow, &e_slow},
                          [&](CombatActor& a, const CombatState&) {
                              seq.push_back(a.id());
                              return CombatAction::pass();
                          });

    // Reagrupado: party [p_fast, p_slow] antes dos inimigos [e_mid, e_slow].
    REQUIRE(sm.round_opening_side() == CombatSide::Party);
    REQUIRE(order_ids(sm) == std::vector<std::string>{"p_fast", "p_slow", "e_mid", "e_slow"});

    // Comando livre (Fase 1) intacto: pending por SPD desc, override do pre-selecionado.
    REQUIRE(ids_of(sm.pending_party_actors()) == std::vector<std::string>{"p_fast", "p_slow"});
    sm.select_party_actor(&p_slow);  // override (pre-selecionado era p_fast)
    sm.begin_turn();
    REQUIRE(sm.active_actor()->id() == "p_slow");
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();

    REQUIRE(ids_of(sm.pending_party_actors()) == std::vector<std::string>{"p_fast"});
    sm.select_party_actor(&p_fast);
    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();

    REQUIRE(sm.pending_party_actors().empty());  // bloco da party terminou

    // Enemy-block em ordem de SPD (e_mid 15 antes de e_slow 5).
    REQUIRE(sm.active_actor()->id() == "e_mid");
    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();
    REQUIRE(sm.active_actor()->id() == "e_slow");
    sm.begin_turn();
    sm.run_active_turn_to_end();
    sm.advance_to_next_actor();

    // Party na ordem ESCOLHIDA (p_slow, p_fast); inimigos em SPD (e_mid, e_slow).
    REQUIRE(seq == std::vector<std::string>{"p_slow", "p_fast", "e_mid", "e_slow"});
    REQUIRE(sm.queue().round_index() == 1);
    REQUIRE(sm.queue().cursor() == 0);
}

// ----- (d) cursor volta a 0 e round_index avanca em CADA fronteira (regroup estavel) -----

TEST_CASE("1B-regroup: (d) cursor=0 e round_index correto em cada fronteira; ordem estavel",
          "[domain][combat][party1b][regroup]") {
    // Inimigo abre intercalado; sem Gambito, o regroup mantem a mesma ordem agrupada a cada
    // rodada. Confirma o contrato de cursor (volta a 0 na fronteira) e a contagem de rodada.
    CombatActor e_fast = foe("e_fast", 999, /*spd=*/20);
    CombatActor p_mid = hero("p_mid", 999, /*spd=*/15);
    CombatActor e_slow = foe("e_slow", 999, /*spd=*/10);
    CombatActor p_slow = hero("p_slow", 999, /*spd=*/5);
    CombatStateMachine sm({&e_fast, &p_mid, &e_slow, &p_slow}, always_pass);

    const std::vector<std::string> grouped{"e_fast", "e_slow", "p_mid", "p_slow"};
    REQUIRE(order_ids(sm) == grouped);  // rodada 0 reagrupada (inimigo abre)

    for (int round = 1; round <= 2; ++round) {
        for (int i = 0; i < 4; ++i) {
            sm.begin_turn();
            sm.run_active_turn_to_end();
            sm.advance_to_next_actor();
        }
        REQUIRE(sm.queue().round_index() == round);
        REQUIRE(sm.queue().cursor() == 0);
        REQUIRE(order_ids(sm) == grouped);  // ordem agrupada estavel entre rodadas
        REQUIRE(sm.active_actor()->id() == "e_fast");
    }
}
