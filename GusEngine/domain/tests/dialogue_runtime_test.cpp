// dialogue_runtime_test.cpp
//
// Spec executavel (Catch2 v3) do DialogueRuntime POCO (ADR-014): motor que percorre
// um DialogueGraph JA VALIDADO sobre uma referencia externa a mapa de flags (o
// SaveData::flags real, ou um std::map de teste; domain/dialogue NAO depende de
// domain/save por design).
//
// Oraculo:
//   (a) grafo linear simples: enter() posiciona no entry + aplica on_enter;
//       advance() segue next_node_id; finished() vira true ao alcancar "@exit";
//   (b) no de escolha: choose(i) aplica o FlagEffect DA OPCAO ESCOLHIDA (nao de
//       outras) e segue pro next_node_id certo;
//   (c) 2 opcoes diferentes convergindo pro MESMO next_node_id (cenario Bertoldo)
//       chegam ao mesmo no seguinte, cada uma com seu proprio flag;
//   (d) current()/choose()/advance() antes de enter() ou apos finished() lancam;
//   (e) current_register() = override.value_or(default).
//
// Subsistema: domain/dialogue. POCO puro, ZERO Qt/SDL, headless.

#include <catch2/catch_test_macros.hpp>

#include <map>
#include <stdexcept>
#include <string>

#include "gus/domain/dialogue/dialogue_graph.hpp"
#include "gus/domain/dialogue/dialogue_runtime.hpp"

using namespace gus::domain::dialogue;

namespace {

// Grafo Bertoldo minimo (ADR-014 / dialogue-tree-npc-intro.md):
// n0_greet (linear, on_enter seta npc_intro.met) -> n1_hook (escolha 3 ramos, cada
// um com FlagEffect proprio) -> os 3 convergem em n3 -> @exit.
DialogueGraph make_bertoldo_graph() {
    DialogueGraph g;
    g.dialogue_id = "npc_intro";
    g.default_register = DialogueRegister::Warm;
    g.entry_node_id = "n0_greet";

    DialogueNode n0;
    n0.id = "n0_greet";
    n0.speaker_id = "bertoldo";
    n0.text_key = "DLG_GREET";
    n0.on_enter = FlagEffect{"npc_intro.met", true};
    n0.next_node_id = "n1_hook";

    DialogueNode hook;
    hook.id = "n1_hook";
    hook.speaker_id = "bertoldo";
    hook.text_key = "DLG_HOOK";
    DialogueOption curioso;
    curioso.label_key = "OPT_CURIOSO";
    curioso.next_node_id = "n3";
    curioso.effect = FlagEffect{"npc_intro.choice_curioso", true};
    DialogueOption pragmatico;
    pragmatico.label_key = "OPT_PRAGMATICO";
    pragmatico.next_node_id = "n3";
    pragmatico.effect = FlagEffect{"npc_intro.choice_pragmatico", true};
    DialogueOption seco;
    seco.label_key = "OPT_SECO";
    seco.next_node_id = "n3";
    seco.effect = FlagEffect{"npc_intro.choice_seco", true};
    hook.options = {curioso, pragmatico, seco};

    DialogueNode reconverge;
    reconverge.id = "n3";
    reconverge.speaker_id = "bertoldo";
    reconverge.text_key = "DLG_RECONVERGE";
    reconverge.next_node_id = std::string(kExitNodeId);

    g.nodes["n0_greet"] = n0;
    g.nodes["n1_hook"] = hook;
    g.nodes["n3"] = reconverge;
    g.validate();
    return g;
}

}  // namespace

TEST_CASE("DialogueRuntime: enter() posiciona no entry e aplica on_enter",
          "[dialogue][runtime]") {
    const DialogueGraph g = make_bertoldo_graph();
    std::map<std::string, bool> flags;
    DialogueRuntime rt(g, flags);
    rt.enter();
    REQUIRE(rt.current().id == "n0_greet");
    REQUIRE(flags.at("npc_intro.met") == true);
    REQUIRE_FALSE(rt.finished());
}

TEST_CASE("DialogueRuntime: advance() em no linear segue next_node_id",
          "[dialogue][runtime]") {
    const DialogueGraph g = make_bertoldo_graph();
    std::map<std::string, bool> flags;
    DialogueRuntime rt(g, flags);
    rt.enter();
    rt.advance();
    REQUIRE(rt.current().id == "n1_hook");
}

TEST_CASE("DialogueRuntime: advance() em no de escolha lanca (usar choose)",
          "[dialogue][runtime]") {
    const DialogueGraph g = make_bertoldo_graph();
    std::map<std::string, bool> flags;
    DialogueRuntime rt(g, flags);
    rt.enter();
    rt.advance();  // n1_hook (escolha)
    REQUIRE_THROWS_AS(rt.advance(), std::logic_error);
}

TEST_CASE("DialogueRuntime: choose() aplica o FlagEffect DA OPCAO ESCOLHIDA e reconverge",
          "[dialogue][runtime][convergencia]") {
    const DialogueGraph g = make_bertoldo_graph();

    SECTION("escolha curioso (indice 0)") {
        std::map<std::string, bool> flags;
        DialogueRuntime rt(g, flags);
        rt.enter();
        rt.advance();
        rt.choose(0);
        REQUIRE(rt.current().id == "n3");
        REQUIRE(flags.at("npc_intro.choice_curioso") == true);
        REQUIRE(flags.find("npc_intro.choice_pragmatico") == flags.end());
        REQUIRE(flags.find("npc_intro.choice_seco") == flags.end());
    }

    SECTION("escolha seco (indice 2) -- reconverge no MESMO no que curioso") {
        std::map<std::string, bool> flags;
        DialogueRuntime rt(g, flags);
        rt.enter();
        rt.advance();
        rt.choose(2);
        REQUIRE(rt.current().id == "n3");  // mesmo destino de curioso/pragmatico
        REQUIRE(flags.at("npc_intro.choice_seco") == true);
        REQUIRE(flags.find("npc_intro.choice_curioso") == flags.end());
    }
}

TEST_CASE("DialogueRuntime: choose() com indice fora de alcance lanca out_of_range",
          "[dialogue][runtime]") {
    const DialogueGraph g = make_bertoldo_graph();
    std::map<std::string, bool> flags;
    DialogueRuntime rt(g, flags);
    rt.enter();
    rt.advance();
    REQUIRE_THROWS_AS(rt.choose(99), std::out_of_range);
}

TEST_CASE("DialogueRuntime: finished() vira true ao alcancar @exit; current() lanca depois",
          "[dialogue][runtime]") {
    const DialogueGraph g = make_bertoldo_graph();
    std::map<std::string, bool> flags;
    DialogueRuntime rt(g, flags);
    rt.enter();
    rt.advance();  // n1_hook
    rt.choose(1);  // n3
    REQUIRE_FALSE(rt.finished());
    rt.advance();  // -> @exit
    REQUIRE(rt.finished());
    REQUIRE_THROWS_AS(rt.current(), std::logic_error);
}

TEST_CASE("DialogueRuntime: current()/choose()/advance() antes de enter() lancam",
          "[dialogue][runtime]") {
    const DialogueGraph g = make_bertoldo_graph();
    std::map<std::string, bool> flags;
    DialogueRuntime rt(g, flags);
    REQUIRE_THROWS_AS(rt.current(), std::logic_error);
    REQUIRE_THROWS_AS(rt.advance(), std::logic_error);
    REQUIRE_THROWS_AS(rt.choose(0), std::logic_error);
}

TEST_CASE("DialogueRuntime: current_register() usa override quando presente, senao default",
          "[dialogue][runtime][register]") {
    DialogueGraph g;
    g.dialogue_id = "x";
    g.default_register = DialogueRegister::Warm;
    g.entry_node_id = "n0";
    DialogueNode n0;
    n0.id = "n0";
    n0.speaker_id = "dante";
    n0.text_key = "DLG_N0";
    n0.register_override = DialogueRegister::Terminal;
    n0.next_node_id = "n1";
    DialogueNode n1;
    n1.id = "n1";
    n1.speaker_id = "dante";
    n1.text_key = "DLG_N1";
    n1.next_node_id = std::string(kExitNodeId);
    g.nodes["n0"] = n0;
    g.nodes["n1"] = n1;
    g.validate();

    std::map<std::string, bool> flags;
    DialogueRuntime rt(g, flags);
    rt.enter();
    REQUIRE(rt.current_register() == DialogueRegister::Terminal);  // override
    rt.advance();
    REQUIRE(rt.current_register() == DialogueRegister::Warm);  // herda default
}
