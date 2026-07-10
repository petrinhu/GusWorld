// GusEngine/app/tests/title_menu_rml_test.cpp
//
// Catch2 (TEST-FIRST) de build_title_menu_rml (SAVE-LOAD-UI etapa 4). SO checagem
// ESTRUTURAL da string RML gerada (ids presentes, rotulos traduzidos
// substituidos, item focado/desabilitado com a classe certa, mini-dialogo de
// Novo Jogo) - MESMO espirito de save_load_menu_rml_test.cpp/system_menu_rml_test.cpp
// (nao valida pixel/renderizacao real).

#include <catch2/catch_test_macros.hpp>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/title_menu.hpp"
#include "gus/app/screens/title_menu_rml.hpp"

using namespace gus::app::screens;
using gus::app::i18n::Translator;

namespace {

Translator make_translator() {
    Translator tr;
    tr.load_from_content(
        "## MENU_CONTINUE\nContinuar\n\n"
        "## MENU_NEW_GAME\nNovo Jogo\n\n"
        "## MENU_QUIT\nSair\n\n"
        "## TITLE_LOGO_PREFIX\nGus\n\n"
        "## TITLE_LOGO_SUFFIX\nWorld\n\n"
        "## TITLE_SUBTITLE\nvertical slice\n\n"
        "## TITLE_FOOTER_HINT\nCima/Baixo navega - Enter seleciona\n\n"
        "## TITLE_NEW_GAME_CONFIRM\nComecar novo jogo?\n\n"
        "## TITLE_NEW_GAME_CONFIRM_YES\nSim, comecar\n\n"
        "## TITLE_NEW_GAME_CONFIRM_NO\nCancelar\n\n");
    return tr;
}

}  // namespace

TEST_CASE("build_title_menu_rml: com save existente, Continuar focado e HABILITADO",
          "[title_menu_rml]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);

    const std::string rml = build_title_menu_rml(state, make_translator());
    REQUIRE(rml.find("Gus") != std::string::npos);
    REQUIRE(rml.find("World") != std::string::npos);
    REQUIRE(rml.find("vertical slice") != std::string::npos);
    REQUIRE(rml.find("Continuar") != std::string::npos);
    REQUIRE(rml.find("Novo Jogo") != std::string::npos);
    REQUIRE(rml.find("Sair") != std::string::npos);
    REQUIRE(rml.find("id=\"title-item-0\"") != std::string::npos);

    const auto pos = rml.find("id=\"title-item-0\"");
    const auto div_start = rml.rfind("<div class=\"", pos);
    const std::string classes = rml.substr(div_start, pos - div_start);
    REQUIRE(classes.find("sel") != std::string::npos);
    REQUIRE(classes.find("disabled") == std::string::npos);
}

TEST_CASE("build_title_menu_rml: SEM save nenhum, Continuar marcado 'disabled' e "
          "Novo Jogo e quem fica focado",
          "[title_menu_rml]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/false);

    const std::string rml = build_title_menu_rml(state, make_translator());
    const auto pos0 = rml.find("id=\"title-item-0\"");
    const auto div0 = rml.rfind("<div class=\"", pos0);
    REQUIRE(rml.substr(div0, pos0 - div0).find("disabled") != std::string::npos);

    const auto pos1 = rml.find("id=\"title-item-1\"");
    const auto div1 = rml.rfind("<div class=\"", pos1);
    REQUIRE(rml.substr(div1, pos1 - div1).find("sel") != std::string::npos);
}

TEST_CASE("build_title_menu_rml: mini-dialogo de Novo Jogo substitui a lista",
          "[title_menu_rml]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    state.selected = static_cast<int>(TitleMenuItem::NewGame);
    (void)title_menu_key_down(state, SDLK_RETURN);  // abre o dialogo (ha save)
    REQUIRE(state.confirming_new_game);

    const std::string rml = build_title_menu_rml(state, make_translator());
    REQUIRE(rml.find("Comecar novo jogo?") != std::string::npos);
    REQUIRE(rml.find("id=\"title-confirm-0\"") != std::string::npos);
    REQUIRE(rml.find("id=\"title-confirm-1\"") != std::string::npos);
    // default seguro: "Nao" focado.
    const auto pos_no = rml.find("id=\"title-confirm-1\"");
    const auto div_no = rml.rfind("<div class=\"", pos_no);
    REQUIRE(rml.substr(div_no, pos_no - div_no).find("focused") != std::string::npos);
}

TEST_CASE("build_title_menu_rml: pressed_index marca o item certo com a classe "
          "'pressed'",
          "[title_menu_rml]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);

    const std::string rml =
        build_title_menu_rml(state, make_translator(), /*pressed_index=*/2);
    const auto pos = rml.find("id=\"title-item-2\"");
    const auto div_start = rml.rfind("<div class=\"", pos);
    REQUIRE(rml.substr(div_start, pos - div_start).find("pressed") != std::string::npos);
}
