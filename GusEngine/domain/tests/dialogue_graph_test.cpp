// dialogue_graph_test.cpp
//
// Spec executavel (Catch2 v3) do grafo de dialogo POCO (ADR-014): DialogueGraph,
// DialogueNode, DialogueOption, FlagCondition, FlagEffect + DialogueGraph::validate()
// fail-fast. POCO puro, ZERO Qt/SDL/I-O.
//
// Oraculo:
//   (a) grafo minimo valido nao lanca;
//   (b) entry_node_id ausente/inexistente -> invalid_argument;
//   (c) next_node_id/option apontando para no inexistente -> invalid_argument;
//   (d) no de escolha com exatamente 1 opcao -> invalid_argument;
//   (e) no orfao (inalcancavel a partir do entry) -> invalid_argument;
//   (f) "@exit" e um destino valido, nao precisa existir como chave;
//   (g) FlagCondition/FlagEffect: igualdade por valor.
//
// Subsistema: domain/dialogue. POCO puro, ZERO Qt/SDL, headless.

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

#include "gus/domain/dialogue/dialogue_graph.hpp"

using namespace gus::domain::dialogue;

namespace {

DialogueNode make_linear_node(std::string id, std::string next) {
    DialogueNode n;
    n.id = id;
    n.speaker_id = "bertoldo";
    n.text_key = "DLG_" + id;
    n.next_node_id = std::move(next);
    return n;
}

}  // namespace

TEST_CASE("DialogueGraph: grafo linear minimo valido nao lanca", "[dialogue][graph]") {
    DialogueGraph g;
    g.dialogue_id = "npc_intro";
    g.entry_node_id = "n0";
    g.nodes["n0"] = make_linear_node("n0", std::string(kExitNodeId));
    REQUIRE_NOTHROW(g.validate());
}

TEST_CASE("DialogueGraph: entry_node_id vazio -> invalid_argument",
          "[dialogue][graph][malformado]") {
    DialogueGraph g;
    g.nodes["n0"] = make_linear_node("n0", std::string(kExitNodeId));
    REQUIRE_THROWS_AS(g.validate(), std::invalid_argument);
}

TEST_CASE("DialogueGraph: entry_node_id inexistente -> invalid_argument",
          "[dialogue][graph][malformado]") {
    DialogueGraph g;
    g.entry_node_id = "fantasma";
    g.nodes["n0"] = make_linear_node("n0", std::string(kExitNodeId));
    REQUIRE_THROWS_AS(g.validate(), std::invalid_argument);
}

TEST_CASE("DialogueGraph: next_node_id de no linear inexistente -> invalid_argument",
          "[dialogue][graph][malformado]") {
    DialogueGraph g;
    g.entry_node_id = "n0";
    g.nodes["n0"] = make_linear_node("n0", "nao_existe");
    REQUIRE_THROWS_AS(g.validate(), std::invalid_argument);
}

TEST_CASE("DialogueGraph: no linear sem next_node_id (beco sem saida) -> invalid_argument",
          "[dialogue][graph][malformado]") {
    DialogueGraph g;
    g.entry_node_id = "n0";
    g.nodes["n0"] = make_linear_node("n0", "");
    REQUIRE_THROWS_AS(g.validate(), std::invalid_argument);
}

TEST_CASE("DialogueGraph: opcao apontando para no inexistente -> invalid_argument",
          "[dialogue][graph][malformado]") {
    DialogueGraph g;
    g.entry_node_id = "n0";
    DialogueNode n0;
    n0.id = "n0";
    n0.speaker_id = "bertoldo";
    n0.text_key = "DLG_N0";
    DialogueOption opt_a;
    opt_a.label_key = "OPT_A";
    opt_a.next_node_id = "n1";
    DialogueOption opt_b;
    opt_b.label_key = "OPT_B";
    opt_b.next_node_id = "fantasma";
    n0.options = {opt_a, opt_b};
    g.nodes["n0"] = n0;
    g.nodes["n1"] = make_linear_node("n1", std::string(kExitNodeId));
    REQUIRE_THROWS_AS(g.validate(), std::invalid_argument);
}

TEST_CASE("DialogueGraph: no de escolha com exatamente 1 opcao -> invalid_argument",
          "[dialogue][graph][malformado]") {
    DialogueGraph g;
    g.entry_node_id = "n0";
    DialogueNode n0;
    n0.id = "n0";
    n0.speaker_id = "bertoldo";
    n0.text_key = "DLG_N0";
    DialogueOption opt_a;
    opt_a.label_key = "OPT_A";
    opt_a.next_node_id = std::string(kExitNodeId);
    n0.options = {opt_a};
    g.nodes["n0"] = n0;
    REQUIRE_THROWS_AS(g.validate(), std::invalid_argument);
}

TEST_CASE("DialogueGraph: no orfao (inalcancavel do entry) -> invalid_argument",
          "[dialogue][graph][malformado]") {
    DialogueGraph g;
    g.entry_node_id = "n0";
    g.nodes["n0"] = make_linear_node("n0", std::string(kExitNodeId));
    // n1 existe mas nada aponta pra ele.
    g.nodes["n1"] = make_linear_node("n1", std::string(kExitNodeId));
    REQUIRE_THROWS_AS(g.validate(), std::invalid_argument);
}

TEST_CASE("DialogueGraph: 2 opcoes convergindo pro MESMO next_node_id e valido (Bertoldo)",
          "[dialogue][graph]") {
    DialogueGraph g;
    g.entry_node_id = "n1_hook";
    DialogueNode hook;
    hook.id = "n1_hook";
    hook.speaker_id = "bertoldo";
    hook.text_key = "DLG_HOOK";
    DialogueOption a;
    a.label_key = "OPT_A";
    a.next_node_id = "n3_reconverge";
    DialogueOption b;
    b.label_key = "OPT_B";
    b.next_node_id = "n3_reconverge";
    hook.options = {a, b};
    g.nodes["n1_hook"] = hook;
    g.nodes["n3_reconverge"] = make_linear_node("n3_reconverge", std::string(kExitNodeId));
    REQUIRE_NOTHROW(g.validate());
}

TEST_CASE("DialogueGraph: register_override vazio herda default_register do grafo (contrato)",
          "[dialogue][graph][register]") {
    DialogueGraph g;
    g.default_register = DialogueRegister::Warm;
    g.entry_node_id = "n0";
    DialogueNode n0 = make_linear_node("n0", std::string(kExitNodeId));
    REQUIRE_FALSE(n0.register_override.has_value());
    g.nodes["n0"] = n0;
    REQUIRE_NOTHROW(g.validate());
    REQUIRE(n0.register_override.value_or(g.default_register) == DialogueRegister::Warm);
}

TEST_CASE("FlagCondition/FlagEffect: igualdade por valor", "[dialogue][flags]") {
    FlagCondition c1{"npc_intro.met", true};
    FlagCondition c2{"npc_intro.met", true};
    FlagCondition c3{"npc_intro.met", false};
    REQUIRE(c1 == c2);
    REQUIRE_FALSE(c1 == c3);

    FlagEffect e1{"npc_intro.choice_curioso", true};
    FlagEffect e2{"npc_intro.choice_curioso", true};
    FlagEffect e3{"npc_intro.choice_seco", true};
    REQUIRE(e1 == e2);
    REQUIRE_FALSE(e1 == e3);
}
