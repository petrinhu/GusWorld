// GusEngine/app/tests/npc_dialogue_rml_test.cpp
//
// Catch2 (TEST-FIRST) de npc_dialogue_portrait_file/build_npc_dialogue_rml
// (DIALOGO-TERMINAL, caixa quente com retrato real): SO checagem ESTRUTURAL da
// string RML gerada (ids presentes, retrato flat referenciado, nome/fala
// traduzidos, opcao selecionada marcada) - MESMO racional de
// system_menu_rml_test.cpp ("NAO valida pixel/renderizacao real - isso exige GPU,
// o lider valida ao vivo").
//
// GENERICIDADE (pedido explicito do lider): o resolvedor de retrato NAO pode ser
// hardcoded pro Bertoldo - prova com DOIS speaker_id (bertoldo, que tem excecao
// cadastrada, E gus, que usa o default "retrato_<id>.png" e ja tem arquivo real em
// resources/sprites/icons-m5/retratos/retrato_gus.png).

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/npc_dialogue_rml.hpp"
#include "gus/domain/dialogue/dialogue_graph.hpp"

using namespace gus::app::screens;
using gus::app::i18n::Translator;
using gus::domain::dialogue::DialogueNode;
using gus::domain::dialogue::DialogueOption;

namespace {

Translator make_translator() {
    Translator tr;
    tr.load_from_content(
        "## ACTOR_BERTOLDO\nSeu Bertoldo\n\n"
        "## ACTOR_GUS\nGus\n\n"
        "## DIALOGUE_CONTINUE\n(continuar)\n\n"
        "## DIALOGUE_NPC_INTRO_N0_GREET\nCedo pra rua, moco.\n\n"
        "## DIALOGUE_NPC_INTRO_CHOICE_CURIOSO\nO desenho ali nao fecha.\n\n"
        "## DIALOGUE_NPC_INTRO_CHOICE_PRAGMATICO\nE perigoso?\n\n");
    return tr;
}

DialogueNode make_linear_node(const std::string& speaker_id) {
    DialogueNode n;
    n.id = "n0_greet";
    n.speaker_id = speaker_id;
    n.text_key = "DIALOGUE_NPC_INTRO_N0_GREET";
    n.next_node_id = "n1_hook";
    return n;
}

DialogueNode make_choice_node(const std::string& speaker_id) {
    DialogueNode n;
    n.id = "n1_hook";
    n.speaker_id = speaker_id;
    n.text_key = "DIALOGUE_NPC_INTRO_N0_GREET";
    n.options = {
        DialogueOption{"DIALOGUE_NPC_INTRO_CHOICE_CURIOSO", "n2a", std::nullopt},
        DialogueOption{"DIALOGUE_NPC_INTRO_CHOICE_PRAGMATICO", "n2b", std::nullopt},
    };
    return n;
}

}  // namespace

// ---------------------------------------------------------- npc_dialogue_portrait_file

TEST_CASE("npc_dialogue_portrait_file: bertoldo usa a EXCECAO cadastrada "
          "(retrato_seu_bertoldo_caim.png - retrato_bertoldo.png NAO existe em disco)",
          "[npc_dialogue_rml]") {
    REQUIRE(npc_dialogue_portrait_file("bertoldo") == "retrato_seu_bertoldo_caim.png");
}

TEST_CASE("npc_dialogue_portrait_file: GENERICIDADE - gus usa o DEFAULT "
          "'retrato_<id>.png' (arquivo real ja existente, SEM excecao cadastrada)",
          "[npc_dialogue_rml]") {
    REQUIRE(npc_dialogue_portrait_file("gus") == "retrato_gus.png");
}

TEST_CASE("npc_dialogue_portrait_file: GENERICIDADE - qualquer speaker_id novo "
          "(sem excecao cadastrada) cai no MESMO default",
          "[npc_dialogue_rml]") {
    REQUIRE(npc_dialogue_portrait_file("caua") == "retrato_caua.png");
    REQUIRE(npc_dialogue_portrait_file("jaci") == "retrato_jaci.png");
    REQUIRE(npc_dialogue_portrait_file("npc_futuro_qualquer") ==
            "retrato_npc_futuro_qualquer.png");
}

// ---------------------------------------------------------------- build_npc_dialogue_rml

TEST_CASE("build_npc_dialogue_rml: no LINEAR (bertoldo) - nome traduzido, fala, "
          "retrato referenciado, hint de continuar",
          "[npc_dialogue_rml]") {
    const Translator tr = make_translator();
    const DialogueNode node = make_linear_node("bertoldo");
    const std::string portrait = npc_dialogue_portrait_file(node.speaker_id);

    const std::string rml = build_npc_dialogue_rml(node, tr, /*selected_option=*/-1,
                                                     portrait);

    REQUIRE(rml.find("Seu Bertoldo") != std::string::npos);
    REQUIRE(rml.find("Cedo pra rua, moco.") != std::string::npos);
    REQUIRE(rml.find("retrato_seu_bertoldo_caim.png") != std::string::npos);
    REQUIRE(rml.find("decorator: image(") != std::string::npos);
    REQUIRE(rml.find("(continuar)") != std::string::npos);
    // Estrutura da caixa quente (mock aprovado 05-dialogo-bertoldo-retrato-real.html).
    REQUIRE(rml.find("npcdlg-box") != std::string::npos);
    REQUIRE(rml.find("warm-corner") != std::string::npos);
    REQUIRE(rml.find("npcdlg-portrait") != std::string::npos);
}

TEST_CASE("build_npc_dialogue_rml: GENERICIDADE - speaker_id='gus' funciona "
          "IGUAL (nome/retrato proprios, MESMA estrutura de caixa)",
          "[npc_dialogue_rml]") {
    const Translator tr = make_translator();
    const DialogueNode node = make_linear_node("gus");
    const std::string portrait = npc_dialogue_portrait_file(node.speaker_id);
    REQUIRE(portrait == "retrato_gus.png");

    const std::string rml = build_npc_dialogue_rml(node, tr, /*selected_option=*/-1,
                                                     portrait);

    REQUIRE(rml.find("Gus") != std::string::npos);
    REQUIRE(rml.find("retrato_gus.png") != std::string::npos);
    REQUIRE(rml.find("npcdlg-box") != std::string::npos);
}

TEST_CASE("build_npc_dialogue_rml: no de ESCOLHA marca a opcao SELECIONADA "
          "(> ) e as demais SEM marcador",
          "[npc_dialogue_rml]") {
    const Translator tr = make_translator();
    const DialogueNode node = make_choice_node("bertoldo");

    const std::string rml_sel0 = build_npc_dialogue_rml(
        node, tr, /*selected_option=*/0, npc_dialogue_portrait_file("bertoldo"));
    REQUIRE(rml_sel0.find("warm-choice selected") != std::string::npos);
    REQUIRE(rml_sel0.find("O desenho ali nao fecha.") != std::string::npos);
    REQUIRE(rml_sel0.find("E perigoso?") != std::string::npos);

    // Selecao troca de opcao -> a MARCACAO "selected" segue o indice (nao fica
    // presa na 1a opcao).
    const std::string rml_sel1 = build_npc_dialogue_rml(
        node, tr, /*selected_option=*/1, npc_dialogue_portrait_file("bertoldo"));
    const std::size_t sel_pos = rml_sel1.find("warm-choice selected");
    const std::size_t pragmatico_pos = rml_sel1.find("E perigoso?");
    REQUIRE(sel_pos != std::string::npos);
    REQUIRE(pragmatico_pos != std::string::npos);
    // A opcao selecionada (classe "selected") vem ANTES do texto da 2a opcao no
    // MESMO <div> (a marcacao precede o label na mesma linha gerada).
    REQUIRE(sel_pos < pragmatico_pos);
}

TEST_CASE("build_npc_dialogue_rml: no de ESCOLHA nao mostra o hint de "
          "'continuar' (so nos LINEARES mostram)",
          "[npc_dialogue_rml]") {
    const Translator tr = make_translator();
    const DialogueNode node = make_choice_node("bertoldo");
    const std::string rml = build_npc_dialogue_rml(
        node, tr, /*selected_option=*/0, npc_dialogue_portrait_file("bertoldo"));
    REQUIRE(rml.find("(continuar)") == std::string::npos);
}
