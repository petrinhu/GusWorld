// GusEngine/app/tests/npc_dialogue_overlay_test.cpp
//
// Catch2 (headless, SEM SDL) da logica de interacao do overlay de dialogo do NPC
// (M7-DIALOGO, NPC-MVP): npc_dialogue_move_selection (navegacao com wrap),
// npc_dialogue_overlay_lines (montagem de texto + fallback de speaker + cursor),
// apply_npc_dialogue_input (advance/choose roteados pelo tipo de no, SEM tocar
// SDL). O desenho de fato (SdlWindow::render_dialogue_overlay_frame) e o poll de
// evento (npc_dialogue_loop.cpp) sao irredutiveis, sem unidade direta (mesmo
// racional de battle_preview/system_menu_loop).

#include <catch2/catch_test_macros.hpp>

#include <map>
#include <string>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/npc_dialogue_overlay.hpp"
#include "gus/domain/dialogue/dialogue_runtime.hpp"
#include "gus/domain/dialogue/dialogue_text.hpp"

using namespace gus::app::screens;
using gus::app::i18n::Translator;
using gus::domain::dialogue::DialogueGraph;
using gus::domain::dialogue::DialogueRuntime;
using gus::domain::dialogue::parse_text_to_dialogue_graph;

namespace {

// Mesmo grafo do Bertoldo (5 nos + reconvergencia) exercitado pelos testes de
// dialogue_text_test.cpp/dialogue_runtime_test.cpp, aqui com as chaves reais do
// npc_intro_bertoldo.dlg.txt (game/dialogues/) - prova a MESMA forma sem depender
// de I/O de disco (POCO puro, ver npc_dialogue_catalog_test.cpp pro parse do
// arquivo real).
const std::string kSrc =
    "#meta dialogue_id npc_intro_bertoldo\n"
    "#meta default_register warm\n"
    "#meta entry n0_greet\n"
    "\n"
    "@node n0_greet\n"
    "speaker: bertoldo\n"
    "text: DIALOGUE_NPC_INTRO_N0_GREET\n"
    "on_enter: npc_intro.met=true\n"
    "-> n1_hook\n"
    "\n"
    "@node n1_hook\n"
    "speaker: bertoldo\n"
    "text: DIALOGUE_NPC_INTRO_N1_HOOK\n"
    "- [DIALOGUE_NPC_INTRO_CHOICE_CURIOSO] -> n2a_curioso "
    "flag:npc_intro.choice_curioso=true\n"
    "- [DIALOGUE_NPC_INTRO_CHOICE_PRAGMATICO] -> n2b_pragmatico "
    "flag:npc_intro.choice_pragmatico=true\n"
    "- [DIALOGUE_NPC_INTRO_CHOICE_SECO] -> n2c_seco "
    "flag:npc_intro.choice_seco=true\n"
    "\n"
    "@node n2a_curioso\n"
    "speaker: bertoldo\n"
    "text: DIALOGUE_NPC_INTRO_N2A_CURIOSO\n"
    "-> n3_reconverge\n"
    "\n"
    "@node n2b_pragmatico\n"
    "speaker: bertoldo\n"
    "text: DIALOGUE_NPC_INTRO_N2B_PRAGMATICO\n"
    "-> n3_reconverge\n"
    "\n"
    "@node n2c_seco\n"
    "speaker: bertoldo\n"
    "text: DIALOGUE_NPC_INTRO_N2C_SECO\n"
    "-> n3_reconverge\n"
    "\n"
    "@node n3_reconverge\n"
    "speaker: bertoldo\n"
    "text: DIALOGUE_NPC_INTRO_N3_RECONVERGE\n"
    "-> @exit\n";

Translator make_translator() {
    Translator t;
    t.load_from_content(
        "## ACTOR_BERTOLDO\nBertoldo\n\n"
        "## DIALOGUE_CONTINUE\n(continuar)\n\n"
        "## DIALOGUE_NPC_INTRO_N0_GREET\nsaudacao\n\n"
        "## DIALOGUE_NPC_INTRO_N1_HOOK\ngancho\n\n"
        "## DIALOGUE_NPC_INTRO_CHOICE_CURIOSO\ncurioso\n\n"
        "## DIALOGUE_NPC_INTRO_CHOICE_PRAGMATICO\npragmatico\n\n"
        "## DIALOGUE_NPC_INTRO_CHOICE_SECO\nseco\n\n"
        "## DIALOGUE_NPC_INTRO_N3_RECONVERGE\nadeus\n");
    return t;
}

}  // namespace

// ---- npc_dialogue_move_selection --------------------------------------------

TEST_CASE("npc_dialogue_move_selection: desce e sobe dentro do range",
          "[npc_dialogue][overlay]") {
    CHECK(npc_dialogue_move_selection(0, +1, 3) == 1);
    CHECK(npc_dialogue_move_selection(1, +1, 3) == 2);
    CHECK(npc_dialogue_move_selection(1, -1, 3) == 0);
}

TEST_CASE("npc_dialogue_move_selection: WRAP nas duas pontas", "[npc_dialogue][overlay]") {
    CHECK(npc_dialogue_move_selection(2, +1, 3) == 0);  // ultimo -> 0
    CHECK(npc_dialogue_move_selection(0, -1, 3) == 2);  // 0 -> ultimo
}

TEST_CASE("npc_dialogue_move_selection: option_count<=0 e defensivo (devolve 0)",
          "[npc_dialogue][overlay]") {
    CHECK(npc_dialogue_move_selection(5, +1, 0) == 0);
    CHECK(npc_dialogue_move_selection(5, -1, -1) == 0);
}

// ---- npc_dialogue_overlay_lines ----------------------------------------------

TEST_CASE("npc_dialogue_overlay_lines: no LINEAR fecha com DIALOGUE_CONTINUE",
          "[npc_dialogue][overlay]") {
    const DialogueGraph g = parse_text_to_dialogue_graph(kSrc);
    const Translator tr = make_translator();
    const auto lines = npc_dialogue_overlay_lines(g.nodes.at("n0_greet"), tr, 0);
    REQUIRE(lines.size() == 3u);
    CHECK(lines[0] == "Bertoldo:");
    CHECK(lines[1] == "saudacao");
    CHECK(lines[2] == "(continuar)");
}

TEST_CASE("npc_dialogue_overlay_lines: no de ESCOLHA lista as 3 opcoes com cursor "
          "na SELECIONADA",
          "[npc_dialogue][overlay]") {
    const DialogueGraph g = parse_text_to_dialogue_graph(kSrc);
    const Translator tr = make_translator();
    const auto lines = npc_dialogue_overlay_lines(g.nodes.at("n1_hook"), tr, 1);
    REQUIRE(lines.size() == 5u);  // speaker + fala + 3 opcoes
    CHECK(lines[0] == "Bertoldo:");
    CHECK(lines[1] == "gancho");
    CHECK(lines[2] == "  curioso");
    CHECK(lines[3] == "> pragmatico");  // selected_option=1
    CHECK(lines[4] == "  seco");
}

TEST_CASE("npc_dialogue_overlay_lines: chave ACTOR_ ausente cai pro id em minusculo",
          "[npc_dialogue][overlay]") {
    const DialogueGraph g = parse_text_to_dialogue_graph(kSrc);
    Translator tr;  // vazio: nenhuma chave ACTOR_BERTOLDO carregada
    tr.load_from_content("## DIALOGUE_NPC_INTRO_N0_GREET\nx\n\n## DIALOGUE_CONTINUE\ny\n");
    const auto lines = npc_dialogue_overlay_lines(g.nodes.at("n0_greet"), tr, 0);
    CHECK(lines[0] == "bertoldo:");  // fallback: id minusculo, nao a chave crua
}

// ---- apply_npc_dialogue_input -------------------------------------------------

TEST_CASE("apply_npc_dialogue_input: Confirm em no LINEAR avanca (nao escolhe)",
          "[npc_dialogue][overlay]") {
    const DialogueGraph g = parse_text_to_dialogue_graph(kSrc);
    std::map<std::string, bool> flags;
    DialogueRuntime rt(g, flags);
    rt.enter();  // n0_greet (linear) - aplica on_enter (npc_intro.met=true)
    REQUIRE(flags.at("npc_intro.met") == true);
    REQUIRE(rt.current().id == "n0_greet");

    const int sel = apply_npc_dialogue_input(rt, NpcDialogueInputAction::Confirm, 0);
    CHECK(sel == 0);
    CHECK(rt.current().id == "n1_hook");  // avancou pro no de escolha
}

TEST_CASE("apply_npc_dialogue_input: MoveUp/MoveDown navegam SO em no de escolha "
          "(no LINEAR e no-op, selecao intacta)",
          "[npc_dialogue][overlay]") {
    const DialogueGraph g = parse_text_to_dialogue_graph(kSrc);
    std::map<std::string, bool> flags;
    DialogueRuntime rt(g, flags);
    rt.enter();  // n0_greet, LINEAR
    const int after_down =
        apply_npc_dialogue_input(rt, NpcDialogueInputAction::MoveDown, 0);
    CHECK(after_down == 0);  // no-op: n0_greet nao tem opcoes
}

TEST_CASE("apply_npc_dialogue_input: escolher o ramo PRAGMATICO seta SO a flag dele, "
          "converge e chega ao @exit",
          "[npc_dialogue][overlay]") {
    const DialogueGraph g = parse_text_to_dialogue_graph(kSrc);
    std::map<std::string, bool> flags;
    DialogueRuntime rt(g, flags);
    rt.enter();
    apply_npc_dialogue_input(rt, NpcDialogueInputAction::Confirm, 0);  // -> n1_hook
    REQUIRE(rt.current().id == "n1_hook");

    // Navega ate a opcao 1 (pragmatico) com MoveDown, depois Confirm.
    int sel = apply_npc_dialogue_input(rt, NpcDialogueInputAction::MoveDown, 0);
    CHECK(sel == 1);
    sel = apply_npc_dialogue_input(rt, NpcDialogueInputAction::Confirm, sel);
    CHECK(sel == 0);  // reset ao entrar no no seguinte
    CHECK(rt.current().id == "n2b_pragmatico");
    CHECK(flags.at("npc_intro.choice_pragmatico") == true);
    CHECK_FALSE(flags.count("npc_intro.choice_curioso"));
    CHECK_FALSE(flags.count("npc_intro.choice_seco"));

    apply_npc_dialogue_input(rt, NpcDialogueInputAction::Confirm, 0);  // -> n3_reconverge
    CHECK(rt.current().id == "n3_reconverge");
    apply_npc_dialogue_input(rt, NpcDialogueInputAction::Confirm, 0);  // -> @exit
    CHECK(rt.finished());
}

TEST_CASE("apply_npc_dialogue_input: no-op apos finished() (nunca chama advance/"
          "choose num runtime ja encerrado)",
          "[npc_dialogue][overlay]") {
    const DialogueGraph g = parse_text_to_dialogue_graph(kSrc);
    std::map<std::string, bool> flags;
    DialogueRuntime rt(g, flags);
    rt.enter();
    apply_npc_dialogue_input(rt, NpcDialogueInputAction::Confirm, 0);  // n1_hook
    apply_npc_dialogue_input(rt, NpcDialogueInputAction::Confirm, 0);  // n2a_curioso
    apply_npc_dialogue_input(rt, NpcDialogueInputAction::Confirm, 0);  // n3_reconverge
    apply_npc_dialogue_input(rt, NpcDialogueInputAction::Confirm, 0);  // @exit
    REQUIRE(rt.finished());
    // Mais uma acao apos finished(): nao lanca (nao chama current()/advance/choose).
    CHECK(apply_npc_dialogue_input(rt, NpcDialogueInputAction::Confirm, 0) == 0);
}
