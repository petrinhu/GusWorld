// GusEngine/app/tests/system_menu_rml_test.cpp
//
// Catch2 (TEST-FIRST) de build_system_menu_rml (MENU-PAUSA-CONFIG-SOM, arvore): SO
// checagem ESTRUTURAL da string RML gerada (ids presentes, rotulos traduzidos
// substituidos, percentuais de volume corretos, item selecionado/pressionado
// marcado). NAO valida pixel/renderizacao real (isso exige GPU - o lider valida
// ao vivo); aqui e regressao barata de "a string nao teve um bug de substituicao/
// indice", cobrindo as 7 telas (Pause/Save/ConfigCategories/Audio/Video/Language +
// Hidden).

#include <catch2/catch_test_macros.hpp>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/system_menu.hpp"
#include "gus/app/screens/system_menu_rml.hpp"
#include "gus/domain/input/controls_restore.hpp"

using namespace gus::app::screens;
using gus::app::i18n::Translator;

namespace {

// Catalogo MINIMO injetado direto (mesmo padrao de translator_test.cpp) - nao
// depende do arquivo .md real em disco.
Translator make_translator() {
    Translator tr;
    tr.load_from_content(
        "## MENU_CONTINUE\nContinuar\n\n"
        "## MENU_SAVE_GAME\nSalvar\n\n"
        "## SETTINGS_TITLE\nConfiguracoes\n\n"
        "## MENU_QUIT\nSair\n\n"
        "## SETTINGS_BACK\nVoltar\n\n"
        "## SETTINGS_AUDIO\nAudio\n\n"
        "## SETTINGS_VIDEO\nVideo\n\n"
        "## SETTINGS_LANGUAGE\nIdioma\n\n"
        "## MENU_SYSTEM_KICKER\nSistema\n\n"
        "## MENU_PAUSE_TITLE\nPausado\n\n"
        // Placeholders posicionais de proposito (espelha o catalogo real pt_br.md,
        // AUD-M7-COSTURA ACH-2): a fixture ANTIGA nao tinha "{0}/{1}" e por isso nao
        // pegava o bug de interpolacao faltando no call-site.
        "## MENU_PAUSE_HINT\n{0} confirma, {1} volta ao jogo\n\n"
        "## MENU_PLACEHOLDER_TEXT\nEm breve.\n\n"
        "## SETTINGS_MUSIC_VOLUME\nVolume da Musica\n\n"
        "## SETTINGS_SFX_VOLUME\nVolume dos Efeitos (SFX)\n\n"
        // Tela Controles (M2).
        "## SETTINGS_CONTROLS\nControles\n\n"
        "## SETTINGS_RESET_DEFAULTS\nRestaurar padroes\n\n"
        "## CONTROLS_HINT\nSelecione uma acao\n\n"
        "## CONTROLS_CAPTURE_PROMPT\nPressione uma tecla...\n\n"
        "## CONTROLS_COL_ACTION\nAcao\n\n"
        "## CONTROLS_COL_KEYBOARD\nTeclado\n\n"
        "## CONTROLS_COL_GAMEPAD\nControle\n\n"
        "## CONTROLS_NAV_HINT\nnavega\n\n"
        "## CONTROLS_GROUP_MOVEMENT\nMovimento\n\n"
        "## CONTROLS_GROUP_WORLD\nMundo\n\n"
        "## CONTROLS_GROUP_COMBAT\nCombate\n\n"
        "## CONTROLS_GROUP_MENU_DIALOGUE\nMenu e Dialogo\n\n"
        "## CONTROLS_SWAP_NOTICE\n(!) trocou com: {0}\n\n"
        "## CONTROLS_RESTORE_CONFIRM_TITLE\nTem certeza?\n\n"
        "## CONTROLS_RESTORE_CONFIRM_YES\nSim restaurar\n\n"
        "## CONTROLS_RESTORE_CONFIRM_NO\nCancelar\n\n"
        "## CONTROLS_NO_BINDING\nsem tecla\n\n"
        "## ACTION_MOVE_FORWARD\nAndar para frente\n\n"
        "## ACTION_MOVE_BACKWARD\nAndar para tras\n\n");
    return tr;
}

}  // namespace

// ---------------------------------------------------------------- Pause

TEST_CASE("build_system_menu_rml: tela Pause contem os 4 verb-pills traduzidos "
          "(Continuar/Salvar/Configuracoes/Sair)",
          "[system_menu_rml]") {
    SystemMenuState state;
    system_menu_open(state);
    const Translator tr = make_translator();

    const std::string rml = build_system_menu_rml(state, tr);
    REQUIRE(rml.find("Continuar") != std::string::npos);
    REQUIRE(rml.find("Salvar") != std::string::npos);
    REQUIRE(rml.find("Configuracoes") != std::string::npos);
    REQUIRE(rml.find("Sair") != std::string::npos);
    REQUIRE(rml.find("Pausado") != std::string::npos);
}

TEST_CASE("build_system_menu_rml: footer-hint de Pause interpola os placeholders "
          "posicionais de MENU_PAUSE_HINT (AUD-M7-COSTURA ACH-2) - sem isso o "
          "jogador veria '{0}/{1}' crus no rodape",
          "[system_menu_rml]") {
    SystemMenuState state;
    system_menu_open(state);
    const Translator tr = make_translator();

    const std::string rml = build_system_menu_rml(state, tr);
    REQUIRE(rml.find("{0}") == std::string::npos);
    REQUIRE(rml.find("{1}") == std::string::npos);
    REQUIRE(rml.find("Enter confirma, Esc volta ao jogo") != std::string::npos);
}

TEST_CASE("build_system_menu_rml: item selecionado de Pause ganha a classe "
          "'focused' (glow do mock)",
          "[system_menu_rml]") {
    SystemMenuState state;
    system_menu_open(state);  // foco = Continuar (item 0)
    const Translator tr = make_translator();

    const std::string rml = build_system_menu_rml(state, tr);
    const auto continue_pos = rml.find("id=\"pause-item-0\"");
    const auto quit_pos = rml.find("id=\"pause-item-3\"");
    REQUIRE(continue_pos != std::string::npos);
    REQUIRE(quit_pos != std::string::npos);
    const std::string around_continue =
        rml.substr(continue_pos > 40 ? continue_pos - 40 : 0, 80);
    const std::string around_quit = rml.substr(quit_pos > 40 ? quit_pos - 40 : 0, 80);
    REQUIRE(around_continue.find("focused") != std::string::npos);
    REQUIRE(around_quit.find("focused") == std::string::npos);
}

TEST_CASE("build_system_menu_rml: pressed_index marca APENAS o item indicado "
          "com a classe 'pressed' (tela Pause)",
          "[system_menu_rml]") {
    SystemMenuState state;
    system_menu_open(state);
    const Translator tr = make_translator();

    const std::string rml = build_system_menu_rml(state, tr, /*pressed_index=*/2);
    const auto pressed_pos = rml.find("id=\"pause-item-2\"");
    const auto other_pos = rml.find("id=\"pause-item-0\"");
    REQUIRE(pressed_pos != std::string::npos);
    REQUIRE(other_pos != std::string::npos);
    const std::string around_pressed =
        rml.substr(pressed_pos > 40 ? pressed_pos - 40 : 0, 80);
    const std::string around_other =
        rml.substr(other_pos > 40 ? other_pos - 40 : 0, 80);
    REQUIRE(around_pressed.find("pressed") != std::string::npos);
    REQUIRE(around_other.find("pressed") == std::string::npos);
}

TEST_CASE("build_system_menu_rml: pressed_index default (-1) nao marca nenhum "
          "item",
          "[system_menu_rml]") {
    SystemMenuState state;
    system_menu_open(state);
    const Translator tr = make_translator();
    const std::string rml = build_system_menu_rml(state, tr);
    // 'pressed' aparece no <style> (seletores .verb-pill.pressed/.btn-back.pressed,
    // SEMPRE presentes no RCSS compartilhado) - o que este teste garante e que
    // NENHUM elemento do <body> ganhou a classe (procura pelo padrao exato
    // 'pressed"' fechando aspas de atributo class, que so aparece quando
    // pressed_class() concatena a classe - CSS puro nunca tem aspas logo apos
    // 'pressed').
    REQUIRE(rml.find("pressed\"") == std::string::npos);
}

// ---------------------------------------------------------------- Save (placeholder)

TEST_CASE("build_system_menu_rml: tela Save mostra o titulo Salvar, o texto "
          "placeholder e o Voltar",
          "[system_menu_rml]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Save;
    const Translator tr = make_translator();
    const std::string rml = build_system_menu_rml(state, tr);
    REQUIRE(rml.find("Salvar") != std::string::npos);
    REQUIRE(rml.find("Em breve.") != std::string::npos);
    REQUIRE(rml.find("Voltar") != std::string::npos);
    REQUIRE(rml.find("id=\"placeholder-back\"") != std::string::npos);
}

// ---------------------------------------------------------------- ConfigCategories

TEST_CASE("build_system_menu_rml: tela ConfigCategories mostra Audio/Video/"
          "Idioma/Voltar com ids estaveis",
          "[system_menu_rml]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::ConfigCategories;
    const Translator tr = make_translator();
    const std::string rml = build_system_menu_rml(state, tr);
    REQUIRE(rml.find("Audio") != std::string::npos);
    REQUIRE(rml.find("Video") != std::string::npos);
    REQUIRE(rml.find("Idioma") != std::string::npos);
    REQUIRE(rml.find("id=\"category-item-0\"") != std::string::npos);
    REQUIRE(rml.find("id=\"category-item-1\"") != std::string::npos);
    REQUIRE(rml.find("id=\"category-item-2\"") != std::string::npos);
    REQUIRE(rml.find("id=\"category-item-3\"") != std::string::npos);  // Voltar
}

TEST_CASE("build_system_menu_rml: item selecionado de ConfigCategories ganha "
          "'focused'",
          "[system_menu_rml]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::ConfigCategories;
    state.config_categories_selected = 1;  // Video
    const Translator tr = make_translator();
    const std::string rml = build_system_menu_rml(state, tr);
    const auto video_pos = rml.find("id=\"category-item-1\"");
    const auto audio_pos = rml.find("id=\"category-item-0\"");
    REQUIRE(video_pos != std::string::npos);
    REQUIRE(audio_pos != std::string::npos);
    const std::string around_video = rml.substr(video_pos > 40 ? video_pos - 40 : 0, 80);
    const std::string around_audio = rml.substr(audio_pos > 40 ? audio_pos - 40 : 0, 80);
    REQUIRE(around_video.find("focused") != std::string::npos);
    REQUIRE(around_audio.find("focused") == std::string::npos);
}

// ---------------------------------------------------------------- Audio (sliders)

TEST_CASE("build_system_menu_rml: tela Audio mostra os percentuais corretos de "
          "volume e o item Voltar",
          "[system_menu_rml]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Audio;
    state.music_volume = 0.72f;
    state.sfx_volume = 0.45f;
    const Translator tr = make_translator();

    const std::string rml = build_system_menu_rml(state, tr);
    REQUIRE(rml.find("Volume da Musica") != std::string::npos);
    REQUIRE(rml.find("Volume dos Efeitos (SFX)") != std::string::npos);
    REQUIRE(rml.find("72%") != std::string::npos);
    REQUIRE(rml.find("45%") != std::string::npos);
    REQUIRE(rml.find("Voltar") != std::string::npos);
    REQUIRE(rml.find("id=\"slider-track-0\"") != std::string::npos);
    REQUIRE(rml.find("id=\"slider-track-1\"") != std::string::npos);
    REQUIRE(rml.find("id=\"audio-item-0\"") != std::string::npos);
    REQUIRE(rml.find("id=\"audio-item-1\"") != std::string::npos);
    REQUIRE(rml.find("id=\"audio-item-2\"") != std::string::npos);  // Voltar
}

// ---------------------------------------------------------------- Controls (M2)

TEST_CASE("build_system_menu_rml: tela Controls mostra o titulo, cabecalhos de "
          "coluna, os 4 grupos e o rotulo/keycap de uma action com tecla",
          "[system_menu_rml][controls]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Controls;
    state.controls_config = gus::domain::input::default_controls();
    const Translator tr = make_translator();

    const std::string rml = build_system_menu_rml(state, tr);
    REQUIRE(rml.find("Controles") != std::string::npos);         // titulo (SETTINGS_CONTROLS)
    REQUIRE(rml.find("Acao") != std::string::npos);
    REQUIRE(rml.find("Teclado") != std::string::npos);
    REQUIRE(rml.find("Controle") != std::string::npos);
    REQUIRE(rml.find("Movimento") != std::string::npos);
    REQUIRE(rml.find("Mundo") != std::string::npos);
    REQUIRE(rml.find("Combate") != std::string::npos);
    REQUIRE(rml.find("Menu e Dialogo") != std::string::npos);
    REQUIRE(rml.find("Andar para frente") != std::string::npos);  // ACTION_MOVE_FORWARD
    REQUIRE(rml.find(">W<") != std::string::npos);                // keycap do default de move_forward
    REQUIRE(rml.find("id=\"controls-item-0\"") != std::string::npos);
    REQUIRE(rml.find("id=\"controls-item-29\"") != std::string::npos);  // ultima action
    REQUIRE(rml.find("Restaurar padroes") != std::string::npos);
    REQUIRE(rml.find("sysmenu-panel\" class=\"wide\"") != std::string::npos);
}

TEST_CASE("build_system_menu_rml: item selecionado de Controls ganha 'sel'",
          "[system_menu_rml][controls]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Controls;
    state.controls_config = gus::domain::input::default_controls();
    state.controls_selected = 3;
    const Translator tr = make_translator();
    const std::string rml = build_system_menu_rml(state, tr);

    const auto pos = rml.find("id=\"controls-item-3\"");
    REQUIRE(pos != std::string::npos);
    const std::string around = rml.substr(pos > 30 ? pos - 30 : 0, 60);
    REQUIRE(around.find("sel") != std::string::npos);
}

TEST_CASE("build_system_menu_rml: item CAPTURANDO mostra o prompt de captura "
          "em vez do keycap, com a classe 'capturing'",
          "[system_menu_rml][controls]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Controls;
    state.controls_config = gus::domain::input::default_controls();
    state.controls_selected = 0;
    state.controls_capturing = true;
    const Translator tr = make_translator();
    const std::string rml = build_system_menu_rml(state, tr);

    REQUIRE(rml.find("Pressione uma tecla...") != std::string::npos);
    const auto pos = rml.find("id=\"controls-item-0\"");
    REQUIRE(pos != std::string::npos);
    const std::string around = rml.substr(pos > 30 ? pos - 30 : 0, 60);
    REQUIRE(around.find("capturing") != std::string::npos);
}

TEST_CASE("build_system_menu_rml: aviso de TROCA (swap) aparece so na action "
          "selecionada, com o rotulo da OUTRA action",
          "[system_menu_rml][controls]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Controls;
    state.controls_config = gus::domain::input::default_controls();
    state.controls_selected = 0;
    state.controls_last_action_swapped = true;
    state.controls_last_swapped_with_action = "move_backward";
    state.controls_last_swapped_with_label_key = "ACTION_MOVE_BACKWARD";
    const Translator tr = make_translator();
    const std::string rml = build_system_menu_rml(state, tr);

    REQUIRE(rml.find("trocou com: Andar para tras") != std::string::npos);
}

TEST_CASE("build_system_menu_rml: sem tecla nenhuma, mostra CONTROLS_NO_BINDING",
          "[system_menu_rml][controls]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Controls;
    state.controls_config = gus::domain::input::default_controls();
    state.controls_config.actions.front().keys.clear();  // remove a tecla da 1a action
    const Translator tr = make_translator();
    const std::string rml = build_system_menu_rml(state, tr);

    REQUIRE(rml.find("sem tecla") != std::string::npos);
}

TEST_CASE("build_system_menu_rml: controls_confirming_restore SUBSTITUI a "
          "lista pelo prompt 'tem certeza?' com Sim/Nao",
          "[system_menu_rml][controls]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Controls;
    state.controls_config = gus::domain::input::default_controls();
    state.controls_confirming_restore = true;
    state.controls_restore_confirm_selected = 1;  // Nao focado
    const Translator tr = make_translator();
    const std::string rml = build_system_menu_rml(state, tr);

    REQUIRE(rml.find("Tem certeza?") != std::string::npos);
    REQUIRE(rml.find("Sim restaurar") != std::string::npos);
    REQUIRE(rml.find("Cancelar") != std::string::npos);
    REQUIRE(rml.find("id=\"controls-confirm-0\"") != std::string::npos);
    REQUIRE(rml.find("id=\"controls-confirm-1\"") != std::string::npos);
    // A lista de actions NAO aparece (foi substituida).
    REQUIRE(rml.find("id=\"controls-item-0\"") == std::string::npos);

    const auto pos = rml.find("id=\"controls-confirm-1\"");
    const std::string around = rml.substr(pos > 30 ? pos - 30 : 0, 60);
    REQUIRE(around.find("focused") != std::string::npos);  // Nao e o default focado
}

// ---------------------------------------------------------------- Video/Language (placeholder)

TEST_CASE("build_system_menu_rml: tela Video mostra o titulo Video e o texto "
          "placeholder",
          "[system_menu_rml]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Video;
    const Translator tr = make_translator();
    const std::string rml = build_system_menu_rml(state, tr);
    REQUIRE(rml.find("Video") != std::string::npos);
    REQUIRE(rml.find("Em breve.") != std::string::npos);
    REQUIRE(rml.find("id=\"placeholder-back\"") != std::string::npos);
}

TEST_CASE("build_system_menu_rml: tela Language mostra o titulo Idioma e o "
          "texto placeholder",
          "[system_menu_rml]") {
    SystemMenuState state;
    state.screen = SystemMenuScreen::Language;
    const Translator tr = make_translator();
    const std::string rml = build_system_menu_rml(state, tr);
    REQUIRE(rml.find("Idioma") != std::string::npos);
    REQUIRE(rml.find("Em breve.") != std::string::npos);
}

// ---------------------------------------------------------------- Hidden

TEST_CASE("build_system_menu_rml: Hidden devolve um documento vazio/minimo (sem "
          "nenhum painel) - o chamador so chama isto quando screen != Hidden, "
          "mas a funcao e defensiva",
          "[system_menu_rml]") {
    SystemMenuState state;  // screen == Hidden por default
    const Translator tr = make_translator();
    const std::string rml = build_system_menu_rml(state, tr);
    REQUIRE(rml.find("Continuar") == std::string::npos);
    REQUIRE(rml.find("Volume da Musica") == std::string::npos);
    REQUIRE(rml.find("Em breve.") == std::string::npos);
}
