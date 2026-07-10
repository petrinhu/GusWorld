// GusEngine/app/tests/difficulty_menu_rml_test.cpp
//
// Catch2 (TEST-FIRST) de build_difficulty_menu_rml (MODOS-MORTE Fase 0). SO
// checagem ESTRUTURAL da string RML gerada (ids presentes, rotulos traduzidos
// substituidos, item focado, splash de confirmacao) - MESMO espirito de
// title_menu_rml_test.cpp (nao valida pixel/renderizacao real).

#include <catch2/catch_test_macros.hpp>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/difficulty_menu.hpp"
#include "gus/app/screens/difficulty_menu_rml.hpp"

using namespace gus::app::screens;
using gus::app::i18n::Translator;

namespace {

Translator make_translator() {
    Translator tr;
    tr.load_from_content(
        "## SAVE_DIFFICULTY_TITLE\nEscolha a dificuldade\n\n"
        "## SAVE_DIFFICULTY_LEGEND\nEssa escolha vale pra sempre\n\n"
        "## SAVE_DIFFICULTY_FACIL_LABEL\nFacil\n\n"
        "## SAVE_DIFFICULTY_FACIL_DESC\nVolta pro ultimo save\n\n"
        "## SAVE_DIFFICULTY_MEDIO_LABEL\nMedio (recomendado)\n\n"
        "## SAVE_DIFFICULTY_MEDIO_DESC\nAcorda no Hospital\n\n"
        "## SAVE_DIFFICULTY_DIFICIL_LABEL\nDificil\n\n"
        "## SAVE_DIFFICULTY_DIFICIL_DESC\nAcorda longe e fraco\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_TITLE_PREFIX\nConfirmar dificuldade: [\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_BODY\nNao da pra trocar depois\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_YES\nConfirmar\n\n"
        "## SAVE_DIFFICULTY_CONFIRM_NO\nCancelar\n\n"
        "## SAVE_DIFFICULTY_FOOTER_HINT\nCima/Baixo navega - Enter seleciona\n\n");
    return tr;
}

}  // namespace

TEST_CASE("build_difficulty_menu_rml: lista mostra os 3 itens, Medio focado por "
          "default",
          "[difficulty_menu_rml]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);

    const std::string rml = build_difficulty_menu_rml(state, make_translator());
    REQUIRE(rml.find("Escolha a dificuldade") != std::string::npos);
    REQUIRE(rml.find("Facil") != std::string::npos);
    REQUIRE(rml.find("Medio (recomendado)") != std::string::npos);
    REQUIRE(rml.find("Dificil") != std::string::npos);
    REQUIRE(rml.find("Volta pro ultimo save") != std::string::npos);
    REQUIRE(rml.find("Essa escolha vale pra sempre") != std::string::npos);
    REQUIRE(rml.find("id=\"difficulty-item-1\"") != std::string::npos);

    const auto pos1 = rml.find("id=\"difficulty-item-1\"");
    const auto div1 = rml.rfind("<div class=\"", pos1);
    REQUIRE(rml.substr(div1, pos1 - div1).find("sel") != std::string::npos);

    // Facil (indice 0) NAO deve estar focado.
    const auto pos0 = rml.find("id=\"difficulty-item-0\"");
    const auto div0 = rml.rfind("<div class=\"", pos0);
    REQUIRE(rml.substr(div0, pos0 - div0).find("sel") == std::string::npos);
}

TEST_CASE("build_difficulty_menu_rml: foco muda pro item selecionado (Facil)",
          "[difficulty_menu_rml]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    state.selected = static_cast<int>(DifficultyMenuItem::Facil);

    const std::string rml = build_difficulty_menu_rml(state, make_translator());
    const auto pos0 = rml.find("id=\"difficulty-item-0\"");
    const auto div0 = rml.rfind("<div class=\"", pos0);
    REQUIRE(rml.substr(div0, pos0 - div0).find("sel") != std::string::npos);
}

TEST_CASE("build_difficulty_menu_rml: splash de confirmacao substitui a lista, "
          "mostra o item ESCOLHIDO no titulo",
          "[difficulty_menu_rml]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    state.selected = static_cast<int>(DifficultyMenuItem::Dificil);
    (void)difficulty_menu_key_down(state, SDLK_RETURN);  // abre o splash
    REQUIRE(state.confirming);

    const std::string rml = build_difficulty_menu_rml(state, make_translator());
    REQUIRE(rml.find("Confirmar dificuldade: [Dificil]") != std::string::npos);
    REQUIRE(rml.find("Nao da pra trocar depois") != std::string::npos);
    REQUIRE(rml.find("id=\"difficulty-confirm-0\"") != std::string::npos);
    REQUIRE(rml.find("id=\"difficulty-confirm-1\"") != std::string::npos);
    // default seguro: Cancelar (indice 1) focado.
    const auto pos1 = rml.find("id=\"difficulty-confirm-1\"");
    const auto div1 = rml.rfind("<div class=\"", pos1);
    REQUIRE(rml.substr(div1, pos1 - div1).find("focused") != std::string::npos);
}

TEST_CASE("build_difficulty_menu_rml: pressed_index marca o item certo com a "
          "classe 'pressed'",
          "[difficulty_menu_rml]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);

    const std::string rml =
        build_difficulty_menu_rml(state, make_translator(), /*pressed_index=*/2);
    const auto pos = rml.find("id=\"difficulty-item-2\"");
    const auto div_start = rml.rfind("<div class=\"", pos);
    REQUIRE(rml.substr(div_start, pos - div_start).find("pressed") != std::string::npos);
}
