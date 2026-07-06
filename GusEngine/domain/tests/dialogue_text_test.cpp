// dialogue_text_test.cpp
//
// Spec executavel (Catch2 v3) do parser POCO do FORMATO-TEXTO de dialogo (ADR-014):
// string em memoria -> DialogueGraph validado. Ver dialogue_text.hpp para a sintaxe
// (#meta, @node, "speaker:"/"text:"/"register:"/"on_enter:", "->", "- [..] ->").
//
// Oraculo:
//   (a) grafo valido: entry + nos linear/escolha + registro default+override;
//   (b) no orfao -> invalid_argument (via DialogueGraph::validate());
//   (c) opcao mirando no inexistente -> invalid_argument;
//   (d) convergencia de 2+ opcoes pro mesmo next_node_id -- parse ok;
//   (e) diretiva malformada / #meta ausente -> DialogueTextError com a linha.
//
// Subsistema: domain/dialogue. POCO puro, ZERO Qt/SDL, headless (opera sobre string).

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

#include "gus/domain/dialogue/dialogue_text.hpp"

using namespace gus::domain::dialogue;

TEST_CASE("dialogue_text: parse feliz de grafo linear simples", "[dialogue][text]") {
    const std::string src =
        "#meta dialogue_id npc_intro_smoke\n"
        "#meta default_register warm\n"
        "#meta entry n0\n"
        "\n"
        "@node n0\n"
        "speaker: bertoldo\n"
        "text: DLG_N0\n"
        "on_enter: npc_intro.met=true\n"
        "-> @exit\n";
    const DialogueGraph g = parse_text_to_dialogue_graph(src);
    REQUIRE(g.dialogue_id == "npc_intro_smoke");
    REQUIRE(g.default_register == DialogueRegister::Warm);
    REQUIRE(g.entry_node_id == "n0");
    REQUIRE(g.nodes.size() == 1u);
    const auto& n0 = g.nodes.at("n0");
    REQUIRE(n0.speaker_id == "bertoldo");
    REQUIRE(n0.text_key == "DLG_N0");
    REQUIRE(n0.on_enter.has_value());
    REQUIRE(n0.on_enter->flag_key == "npc_intro.met");
    REQUIRE(n0.on_enter->value == true);
    REQUIRE(n0.next_node_id == "@exit");
    REQUIRE_FALSE(n0.register_override.has_value());
}

TEST_CASE("dialogue_text: no de escolha com 3 opcoes convergindo (Bertoldo)",
          "[dialogue][text][convergencia]") {
    const std::string src =
        "#meta dialogue_id npc_intro\n"
        "#meta default_register warm\n"
        "#meta entry n1_hook\n"
        "\n"
        "@node n1_hook\n"
        "speaker: bertoldo\n"
        "text: DLG_N1_HOOK\n"
        "- [DLG_OPT_CURIOSO] -> n2a flag:npc_intro.choice_curioso=true\n"
        "- [DLG_OPT_PRAGMATICO] -> n2b flag:npc_intro.choice_pragmatico=true\n"
        "- [DLG_OPT_SECO] -> n2c flag:npc_intro.choice_seco=true\n"
        "\n"
        "@node n2a\n"
        "speaker: bertoldo\n"
        "text: DLG_N2A\n"
        "-> n3\n"
        "\n"
        "@node n2b\n"
        "speaker: bertoldo\n"
        "text: DLG_N2B\n"
        "-> n3\n"
        "\n"
        "@node n2c\n"
        "speaker: bertoldo\n"
        "text: DLG_N2C\n"
        "-> n3\n"
        "\n"
        "@node n3\n"
        "speaker: bertoldo\n"
        "text: DLG_N3\n"
        "-> @exit\n";
    const DialogueGraph g = parse_text_to_dialogue_graph(src);
    REQUIRE(g.nodes.size() == 5u);
    const auto& hook = g.nodes.at("n1_hook");
    REQUIRE(hook.options.size() == 3u);
    REQUIRE(hook.options[0].label_key == "DLG_OPT_CURIOSO");
    REQUIRE(hook.options[0].next_node_id == "n2a");
    REQUIRE(hook.options[0].effect.has_value());
    REQUIRE(hook.options[0].effect->flag_key == "npc_intro.choice_curioso");
    REQUIRE(hook.options[1].next_node_id == "n2b");
    REQUIRE(hook.options[2].next_node_id == "n2c");
    // 3 ramos convergem pro mesmo n3.
    REQUIRE(g.nodes.at("n2a").next_node_id == "n3");
    REQUIRE(g.nodes.at("n2b").next_node_id == "n3");
    REQUIRE(g.nodes.at("n2c").next_node_id == "n3");
}

TEST_CASE("dialogue_text: register_override por no (Dante asmodico->c-arcane)",
          "[dialogue][text][register]") {
    const std::string src =
        "#meta dialogue_id dante_late_game\n"
        "#meta default_register warm\n"
        "#meta entry n0\n"
        "\n"
        "@node n0\n"
        "speaker: dante\n"
        "text: DLG_DANTE_N0\n"
        "register: terminal\n"
        "-> @exit\n";
    const DialogueGraph g = parse_text_to_dialogue_graph(src);
    REQUIRE(g.nodes.at("n0").register_override.has_value());
    REQUIRE(*g.nodes.at("n0").register_override == DialogueRegister::Terminal);
}

TEST_CASE("dialogue_text: no orfao (nada aponta pra ele) -> invalid_argument via validate()",
          "[dialogue][text][malformado]") {
    const std::string src =
        "#meta dialogue_id x\n"
        "#meta default_register warm\n"
        "#meta entry n0\n"
        "\n"
        "@node n0\n"
        "speaker: bertoldo\n"
        "text: DLG_N0\n"
        "-> @exit\n"
        "\n"
        "@node n1\n"
        "speaker: bertoldo\n"
        "text: DLG_N1\n"
        "-> @exit\n";
    REQUIRE_THROWS_AS(parse_text_to_dialogue_graph(src), std::invalid_argument);
}

TEST_CASE("dialogue_text: opcao mirando no inexistente -> invalid_argument via validate()",
          "[dialogue][text][malformado]") {
    const std::string src =
        "#meta dialogue_id x\n"
        "#meta default_register warm\n"
        "#meta entry n0\n"
        "\n"
        "@node n0\n"
        "speaker: bertoldo\n"
        "text: DLG_N0\n"
        "- [OPT_A] -> fantasma\n"
        "- [OPT_B] -> @exit\n";
    REQUIRE_THROWS_AS(parse_text_to_dialogue_graph(src), std::invalid_argument);
}

TEST_CASE("dialogue_text: #meta ausente -> DialogueTextError", "[dialogue][text][malformado]") {
    REQUIRE_THROWS_AS(parse_text_to_dialogue_graph("@node n0\ntext: X\n-> @exit\n"),
                      DialogueTextError);
}

TEST_CASE("dialogue_text: diretiva fora de bloco @node -> DialogueTextError",
          "[dialogue][text][malformado]") {
    const std::string src =
        "#meta dialogue_id x\n"
        "#meta default_register warm\n"
        "#meta entry n0\n"
        "speaker: bertoldo\n";  // sem @node aberto
    REQUIRE_THROWS_AS(parse_text_to_dialogue_graph(src), DialogueTextError);
}

TEST_CASE("dialogue_text: linhas em branco e comentarios // sao ignorados",
          "[dialogue][text]") {
    const std::string src =
        "// comentario\n"
        "#meta dialogue_id x\n"
        "\n"
        "#meta default_register warm\n"
        "#meta entry n0\n"
        "\n"
        "// outro comentario\n"
        "@node n0\n"
        "speaker: bertoldo\n"
        "text: DLG_N0\n"
        "-> @exit\n";
    REQUIRE_NOTHROW(parse_text_to_dialogue_graph(src));
}
