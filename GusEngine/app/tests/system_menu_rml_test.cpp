// GusEngine/app/tests/system_menu_rml_test.cpp
//
// Catch2 (TEST-FIRST) de build_system_menu_rml (MENU-PAUSA-CONFIG-SOM): SO
// checagem ESTRUTURAL da string RML gerada (ids presentes, rotulos traduzidos
// substituidos, percentuais de volume corretos, item selecionado marcado). NAO
// valida pixel/renderizacao real (isso exige GPU - o lider valida ao vivo); aqui
// e regressao barata de "a string nao teve um bug de substituicao/indice".

#include <catch2/catch_test_macros.hpp>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/system_menu.hpp"
#include "gus/app/screens/system_menu_rml.hpp"

using namespace gus::app::screens;
using gus::app::i18n::Translator;

namespace {

// Catalogo MINIMO injetado direto (mesmo padrao de translator_test.cpp) - nao
// depende do arquivo .md real em disco.
Translator make_translator() {
    Translator tr;
    tr.load_from_content(
        "## MENU_CONTINUE\nContinuar\n\n"
        "## SETTINGS_TITLE\nConfiguracoes\n\n"
        "## MENU_QUIT\nSair\n\n"
        "## SETTINGS_BACK\nVoltar\n\n"
        "## MENU_SYSTEM_KICKER\nSistema\n\n"
        "## MENU_PAUSE_TITLE\nPausado\n\n"
        "## SETTINGS_MUSIC_VOLUME\nVolume da Musica\n\n"
        "## SETTINGS_SFX_VOLUME\nVolume dos Efeitos (SFX)\n\n");
    return tr;
}

}  // namespace

TEST_CASE("build_system_menu_rml: tela Pause contem os 3 verb-pills traduzidos",
          "[system_menu_rml]") {
    SystemMenuState state;
    system_menu_open(state);
    const Translator tr = make_translator();

    const std::string rml = build_system_menu_rml(state, tr);
    REQUIRE(rml.find("Continuar") != std::string::npos);
    REQUIRE(rml.find("Configuracoes") != std::string::npos);
    REQUIRE(rml.find("Sair") != std::string::npos);
    REQUIRE(rml.find("Pausado") != std::string::npos);
}

TEST_CASE("build_system_menu_rml: item selecionado de Pause ganha a classe "
          "'focused' (glow do mock)",
          "[system_menu_rml]") {
    SystemMenuState state;
    system_menu_open(state);  // foco = Continuar (item 0)
    const Translator tr = make_translator();

    const std::string rml = build_system_menu_rml(state, tr);
    // A pill de Continuar deve carregar 'focused'; a de Sair, nao.
    const auto continue_pos = rml.find("id=\"pause-item-0\"");
    const auto quit_pos = rml.find("id=\"pause-item-2\"");
    REQUIRE(continue_pos != std::string::npos);
    REQUIRE(quit_pos != std::string::npos);
    // A regiao ao redor do item 0 contem 'focused'; ao redor do item 2, nao.
    const std::string around_continue =
        rml.substr(continue_pos > 40 ? continue_pos - 40 : 0, 80);
    const std::string around_quit = rml.substr(quit_pos > 40 ? quit_pos - 40 : 0, 80);
    REQUIRE(around_continue.find("focused") != std::string::npos);
    REQUIRE(around_quit.find("focused") == std::string::npos);
}

TEST_CASE("build_system_menu_rml: tela Config mostra os percentuais corretos de "
          "volume e o item Voltar",
          "[system_menu_rml]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Config;
    state.music_volume = 0.72f;
    state.sfx_volume = 0.45f;
    const Translator tr = make_translator();

    const std::string rml = build_system_menu_rml(state, tr);
    REQUIRE(rml.find("Volume da Musica") != std::string::npos);
    REQUIRE(rml.find("Volume dos Efeitos (SFX)") != std::string::npos);
    REQUIRE(rml.find("72%") != std::string::npos);
    REQUIRE(rml.find("45%") != std::string::npos);
    REQUIRE(rml.find("Voltar") != std::string::npos);
}

TEST_CASE("build_system_menu_rml: Hidden devolve um documento vazio/minimo (sem "
          "os 2 painéis) - o chamador so chama isto quando screen != Hidden, mas "
          "a funcao e defensiva",
          "[system_menu_rml]") {
    SystemMenuState state;  // screen == Hidden por default
    const Translator tr = make_translator();
    const std::string rml = build_system_menu_rml(state, tr);
    REQUIRE(rml.find("Continuar") == std::string::npos);
    REQUIRE(rml.find("Volume da Musica") == std::string::npos);
}

TEST_CASE("build_system_menu_rml: ids estaveis dos sliders para hit-test de "
          "mouse (get_element_box, mesma receita GLINTFX-CLICK do cockpit)",
          "[system_menu_rml]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Config;
    const Translator tr = make_translator();
    const std::string rml = build_system_menu_rml(state, tr);
    REQUIRE(rml.find("id=\"slider-track-0\"") != std::string::npos);  // musica
    REQUIRE(rml.find("id=\"slider-track-1\"") != std::string::npos);  // sfx
}

TEST_CASE("build_system_menu_rml: ids estaveis das PILLS/campos/Voltar para "
          "clique de mouse (MENU-PAUSA-CONFIG-SOM, click-to-activate)",
          "[system_menu_rml]") {
    SystemMenuState pause_state;
    pause_state.screen = SystemMenuScreen::Pause;
    const Translator tr = make_translator();
    const std::string pause_rml = build_system_menu_rml(pause_state, tr);
    REQUIRE(pause_rml.find("id=\"pause-item-0\"") != std::string::npos);
    REQUIRE(pause_rml.find("id=\"pause-item-1\"") != std::string::npos);
    REQUIRE(pause_rml.find("id=\"pause-item-2\"") != std::string::npos);

    SystemMenuState config_state;
    config_state.screen = SystemMenuScreen::Config;
    const std::string config_rml = build_system_menu_rml(config_state, tr);
    REQUIRE(config_rml.find("id=\"config-item-0\"") != std::string::npos);  // musica
    REQUIRE(config_rml.find("id=\"config-item-1\"") != std::string::npos);  // sfx
    REQUIRE(config_rml.find("id=\"config-back\"") != std::string::npos);
}
