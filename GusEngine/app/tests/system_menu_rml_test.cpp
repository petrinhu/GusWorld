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
        "## MENU_PAUSE_HINT\nEnter confirma, ESC volta\n\n"
        "## MENU_PLACEHOLDER_TEXT\nEm breve.\n\n"
        "## SETTINGS_MUSIC_VOLUME\nVolume da Musica\n\n"
        "## SETTINGS_SFX_VOLUME\nVolume dos Efeitos (SFX)\n\n");
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
