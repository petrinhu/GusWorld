// GusEngine/app/tests/system_menu_test.cpp
//
// Catch2 (TEST-FIRST) da logica PURA do menu de sistema (pausa + config de som/
// video/lingua/save, arvore hierarquica, MENU-PAUSA-CONFIG-SOM, M7-COSTURA): POCO
// 100% testavel sem SDL/janela/glintfx - mesmo espirito de
// app/tests/battle_key_routing_test.cpp e maestro_logic_test.cpp. A UI (RML/RCSS)
// e a integracao com AudioEngine/persistencia SO CONSOMEM este estado; aqui so a
// navegacao/selecao/ajuste de volume da arvore inteira:
//
//   Pause (Continuar/Salvar/Configuracoes/Sair)
//     Save            -> placeholder
//     ConfigCategories (Audio/Video/Lingua/Voltar)
//       Audio         -> Musica/SFX/Voltar (sliders)
//       Video         -> placeholder
//       Language      -> placeholder

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include <SDL3/SDL.h>  // SDL_Keycode - mesmo padrao de battle_key_down (app/ ja usa SDL)

#include "gus/app/screens/system_menu.hpp"
#include "gus/domain/input/action_registry.hpp"
#include "gus/domain/input/controls_restore.hpp"

using namespace gus::app::screens;

namespace {

// Helpers de navegacao (evitam repetir a sequencia de teclas em todo TEST_CASE).
void goto_config_categories(SystemMenuState& state) {
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);    // Pause: foco = Configuracoes (2)
    system_menu_key_down(state, SDLK_RETURN);  // entra em ConfigCategories
}

void goto_audio(SystemMenuState& state) {
    goto_config_categories(state);
    system_menu_key_down(state, SDLK_RETURN);  // ConfigCategories: foco=Audio(0) -> entra
}

void goto_controls(SystemMenuState& state) {
    goto_config_categories(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);    // ConfigCategories: foco=Controles(2)
    system_menu_key_down(state, SDLK_RETURN);  // entra em Controls
}

}  // namespace

TEST_CASE("SystemMenuState: comeca Hidden (menu fechado)", "[system_menu]") {
    SystemMenuState state;
    REQUIRE(state.screen == SystemMenuScreen::Hidden);
}

TEST_CASE("system_menu_open: abre em Pause com foco inicial em Continuar (item 0)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    REQUIRE(state.screen == SystemMenuScreen::Pause);
    REQUIRE(state.pause_selected == 0);
}

TEST_CASE("parent_screen_of: cada tela da arvore aponta pro pai correto",
          "[system_menu]") {
    REQUIRE(parent_screen_of(SystemMenuScreen::Save) == SystemMenuScreen::Pause);
    REQUIRE(parent_screen_of(SystemMenuScreen::ConfigCategories) ==
            SystemMenuScreen::Pause);
    REQUIRE(parent_screen_of(SystemMenuScreen::Audio) ==
            SystemMenuScreen::ConfigCategories);
    REQUIRE(parent_screen_of(SystemMenuScreen::Video) ==
            SystemMenuScreen::ConfigCategories);
    REQUIRE(parent_screen_of(SystemMenuScreen::Controls) ==
            SystemMenuScreen::ConfigCategories);
    REQUIRE(parent_screen_of(SystemMenuScreen::Language) ==
            SystemMenuScreen::ConfigCategories);
}

// ---------------------------------------------------------------- Pause

TEST_CASE("Pause: UP/DOWN navega os 4 itens com WRAP (Continuar/Salvar/"
          "Configuracoes/Sair)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    REQUIRE(state.pause_selected == 0);

    system_menu_key_down(state, SDLK_DOWN);
    REQUIRE(state.pause_selected == 1);
    system_menu_key_down(state, SDLK_DOWN);
    REQUIRE(state.pause_selected == 2);
    system_menu_key_down(state, SDLK_DOWN);
    REQUIRE(state.pause_selected == 3);
    system_menu_key_down(state, SDLK_DOWN);  // wrap pro topo
    REQUIRE(state.pause_selected == 0);

    system_menu_key_down(state, SDLK_UP);  // wrap pro fim
    REQUIRE(state.pause_selected == 3);
}

TEST_CASE("Pause: ENTER em Continuar (item 0) devolve action Continue",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(action == SystemMenuAction::Continue);
}

TEST_CASE("Pause: ESC (volta ao jogo) devolve action Continue - RAIZ da arvore, "
          "footer 'ESC volta ao jogo'",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);  // move pro item 1 (Salvar)
    const SystemMenuAction action = system_menu_key_down(state, SDLK_ESCAPE);
    REQUIRE(action == SystemMenuAction::Continue);
}

TEST_CASE("Pause: ENTER em Salvar (item 1) abre a tela Save (placeholder)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);  // item 1 = Salvar
    const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::Save);
}

TEST_CASE("Pause: ENTER em Configuracoes (item 2) abre ConfigCategories, foco "
          "inicial = Audio",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);  // item 2 = Configuracoes
    const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::ConfigCategories);
    REQUIRE(state.config_categories_selected == 0);  // foco inicial = Audio
}

TEST_CASE("Pause: ENTER em Sair (item 3) devolve action RequestQuit sem fechar "
          "o menu sozinho (o chamador decide encerrar o programa)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);  // item 3 = Sair
    const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(action == SystemMenuAction::RequestQuit);
}

TEST_CASE("Pause: A/D nao fazem nada (sem eixo horizontal na tela Pause)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    const int before = state.pause_selected;
    REQUIRE(system_menu_key_down(state, SDLK_A) == SystemMenuAction::None);
    REQUIRE(system_menu_key_down(state, SDLK_D) == SystemMenuAction::None);
    REQUIRE(state.pause_selected == before);
}

// ---------------------------------------------------------------- Save (placeholder)

TEST_CASE("Save (placeholder): ESC ou ENTER voltam pro Pause preservando a "
          "selecao anterior de Pause",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);    // Pause: foco = Salvar (1)
    system_menu_key_down(state, SDLK_RETURN);  // entra em Save
    REQUIRE(state.screen == SystemMenuScreen::Save);

    const SystemMenuAction action = system_menu_key_down(state, SDLK_ESCAPE);
    REQUIRE(action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::Pause);
    REQUIRE(state.pause_selected == 1);  // ainda em Salvar, nao resetou
}

TEST_CASE("Save (placeholder): UP/DOWN/LEFT/RIGHT nao fazem nada (sem controle, "
          "so 'em breve' + Voltar)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_RETURN);  // Save
    REQUIRE(system_menu_key_down(state, SDLK_UP) == SystemMenuAction::None);
    REQUIRE(system_menu_key_down(state, SDLK_DOWN) == SystemMenuAction::None);
    REQUIRE(system_menu_key_down(state, SDLK_LEFT) == SystemMenuAction::None);
    REQUIRE(state.screen == SystemMenuScreen::Save);
}

// ---------------------------------------------------------------- ConfigCategories

TEST_CASE("ConfigCategories: UP/DOWN navega os 5 itens com WRAP (Audio/Video/"
          "Controles/Lingua/Voltar)",
          "[system_menu]") {
    SystemMenuState state;
    goto_config_categories(state);
    REQUIRE(state.config_categories_selected == 0);

    system_menu_key_down(state, SDLK_DOWN);
    REQUIRE(state.config_categories_selected == 1);
    system_menu_key_down(state, SDLK_DOWN);
    REQUIRE(state.config_categories_selected == 2);
    system_menu_key_down(state, SDLK_DOWN);
    REQUIRE(state.config_categories_selected == 3);
    system_menu_key_down(state, SDLK_DOWN);
    REQUIRE(state.config_categories_selected == 4);
    system_menu_key_down(state, SDLK_DOWN);  // wrap
    REQUIRE(state.config_categories_selected == 0);
}

TEST_CASE("ConfigCategories: ENTER em Controles (item 2, M2) abre a tela "
          "Controls, foco inicial = primeira action",
          "[system_menu]") {
    SystemMenuState state;
    goto_config_categories(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);  // item 2 = Controles
    const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::Controls);
    REQUIRE(state.controls_selected == 0);
    REQUIRE_FALSE(state.controls_capturing);
    REQUIRE_FALSE(state.controls_confirming_restore);
}

TEST_CASE("ConfigCategories: ENTER em Audio (item 0) abre a tela Audio, foco "
          "inicial = Musica",
          "[system_menu]") {
    SystemMenuState state;
    goto_config_categories(state);
    const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::Audio);
    REQUIRE(state.audio_selected == 0);
}

TEST_CASE("ConfigCategories: ENTER em Video (item 1) abre a tela Video "
          "(placeholder)",
          "[system_menu]") {
    SystemMenuState state;
    goto_config_categories(state);
    system_menu_key_down(state, SDLK_DOWN);  // item 1 = Video
    const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::Video);
}

TEST_CASE("ConfigCategories: ENTER em Lingua (item 3) abre a tela Language "
          "(placeholder)",
          "[system_menu]") {
    SystemMenuState state;
    goto_config_categories(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);  // item 3 = Lingua
    const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::Language);
}

TEST_CASE("ConfigCategories: ESC ou ENTER em Voltar (item 4) voltam pro Pause "
          "preservando a selecao anterior de Pause (Configuracoes)",
          "[system_menu]") {
    SystemMenuState state;
    goto_config_categories(state);
    const SystemMenuAction esc_action = system_menu_key_down(state, SDLK_ESCAPE);
    REQUIRE(esc_action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::Pause);
    REQUIRE(state.pause_selected == 2);  // ainda em Configuracoes

    // De novo, desta vez confirmando com ENTER no item Voltar.
    goto_config_categories(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);  // item 4 = Voltar
    const SystemMenuAction back_action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(back_action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::Pause);
}

// ---------------------------------------------------------------- Video/Language (placeholder)

TEST_CASE("Video (placeholder): ESC volta pra ConfigCategories preservando a "
          "selecao anterior (Video)",
          "[system_menu]") {
    SystemMenuState state;
    goto_config_categories(state);
    system_menu_key_down(state, SDLK_DOWN);    // foco = Video (1)
    system_menu_key_down(state, SDLK_RETURN);  // entra em Video
    REQUIRE(state.screen == SystemMenuScreen::Video);

    const SystemMenuAction action = system_menu_key_down(state, SDLK_ESCAPE);
    REQUIRE(action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::ConfigCategories);
    REQUIRE(state.config_categories_selected == 1);  // ainda em Video
}

TEST_CASE("Language (placeholder): ENTER volta pra ConfigCategories",
          "[system_menu]") {
    SystemMenuState state;
    goto_config_categories(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);    // foco = Lingua (3)
    system_menu_key_down(state, SDLK_RETURN);  // entra em Language
    REQUIRE(state.screen == SystemMenuScreen::Language);

    const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::ConfigCategories);
}

// ---------------------------------------------------------------- Audio (sliders)

TEST_CASE("Audio: UP/DOWN navega os 3 itens com WRAP (Musica/SFX/Voltar)",
          "[system_menu]") {
    SystemMenuState state;
    goto_audio(state);
    REQUIRE(state.audio_selected == 0);

    system_menu_key_down(state, SDLK_DOWN);
    REQUIRE(state.audio_selected == 1);
    system_menu_key_down(state, SDLK_DOWN);
    REQUIRE(state.audio_selected == 2);
    system_menu_key_down(state, SDLK_DOWN);  // wrap
    REQUIRE(state.audio_selected == 0);
}

TEST_CASE("Audio: LEFT/RIGHT no item Musica ajusta music_volume clampado [0,1]",
          "[system_menu]") {
    SystemMenuState state;
    goto_audio(state);
    REQUIRE(state.music_volume == Catch::Approx(1.0f));

    const SystemMenuAction a1 = system_menu_key_down(state, SDLK_LEFT);
    REQUIRE(a1 == SystemMenuAction::VolumeChanged);
    REQUIRE(state.music_volume < 1.0f);

    for (int i = 0; i < 30; ++i) system_menu_key_down(state, SDLK_LEFT);
    REQUIRE(state.music_volume == Catch::Approx(0.0f));

    for (int i = 0; i < 30; ++i) system_menu_key_down(state, SDLK_RIGHT);
    REQUIRE(state.music_volume == Catch::Approx(1.0f));
}

TEST_CASE("Audio: LEFT/RIGHT no item SFX (indice 1) ajusta sfx_volume, NAO "
          "music_volume",
          "[system_menu]") {
    SystemMenuState state;
    goto_audio(state);
    system_menu_key_down(state, SDLK_DOWN);  // foco = SFX (1)
    REQUIRE(state.audio_selected == 1);

    system_menu_key_down(state, SDLK_LEFT);
    REQUIRE(state.sfx_volume < 1.0f);
    REQUIRE(state.music_volume == Catch::Approx(1.0f));  // intocado
}

TEST_CASE("Audio: LEFT/RIGHT no item Voltar (indice 2) NAO produz VolumeChanged "
          "(nao e um slider)",
          "[system_menu]") {
    SystemMenuState state;
    goto_audio(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);  // foco = Voltar (2)
    REQUIRE(state.audio_selected == 2);

    const SystemMenuAction action = system_menu_key_down(state, SDLK_LEFT);
    REQUIRE(action != SystemMenuAction::VolumeChanged);
    REQUIRE(state.music_volume == Catch::Approx(1.0f));
    REQUIRE(state.sfx_volume == Catch::Approx(1.0f));
}

TEST_CASE("Audio: A/D ajustam volume igual LEFT/RIGHT (WASD completo)",
          "[system_menu]") {
    SystemMenuState state;
    goto_audio(state);
    REQUIRE(state.music_volume == Catch::Approx(1.0f));

    const SystemMenuAction a_action = system_menu_key_down(state, SDLK_A);
    REQUIRE(a_action == SystemMenuAction::VolumeChanged);
    REQUIRE(state.music_volume < 1.0f);
    const float after_a = state.music_volume;

    const SystemMenuAction d_action = system_menu_key_down(state, SDLK_D);
    REQUIRE(d_action == SystemMenuAction::VolumeChanged);
    REQUIRE(state.music_volume > after_a);
    REQUIRE(state.music_volume == Catch::Approx(1.0f));
}

TEST_CASE("Audio: ESC ou ENTER em Voltar voltam pra ConfigCategories "
          "preservando a selecao anterior (Audio)",
          "[system_menu]") {
    SystemMenuState state;
    goto_audio(state);
    const SystemMenuAction action = system_menu_key_down(state, SDLK_ESCAPE);
    REQUIRE(action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::ConfigCategories);
    REQUIRE(state.config_categories_selected == 0);  // ainda em Audio

    goto_audio(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);  // foco = Voltar
    const SystemMenuAction back_action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(back_action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::ConfigCategories);
}

// ---------------------------------------------------------------- Controls (M2)

TEST_CASE("controls_action_name_at/controls_group_at: as 30 posicoes cobrem "
          "EXATAMENTE o ActionRegistry (sem repetir, sem faltar), grupos em "
          "ordem nao-decrescente 0..3",
          "[system_menu][controls]") {
    std::vector<std::string> seen;
    int last_group = -1;
    for (int i = 0; i < kControlsActionCount; ++i) {
        const std::string_view name = controls_action_name_at(i);
        REQUIRE_FALSE(name.empty());
        REQUIRE(gus::domain::input::ActionRegistry::get_by_name(name) != nullptr);
        seen.emplace_back(name);

        const int group = controls_group_at(i);
        REQUIRE(group >= 0);
        REQUIRE(group <= 3);
        REQUIRE(group >= last_group);  // nao-decrescente (agrupado)
        last_group = group;
    }
    // Sem repeticao.
    std::sort(seen.begin(), seen.end());
    REQUIRE(std::adjacent_find(seen.begin(), seen.end()) == seen.end());
    // Cobre TODAS as actions do registry (mesma cardinalidade + mesmo conteudo).
    REQUIRE(static_cast<int>(seen.size()) ==
            gus::domain::input::ActionRegistry::count());
    for (const auto& def : gus::domain::input::ActionRegistry::actions()) {
        REQUIRE(std::binary_search(seen.begin(), seen.end(), def.action_name));
    }
}

TEST_CASE("controls_action_name_at/controls_group_at: indice fora do intervalo "
          "e defensivo (vazio/-1)",
          "[system_menu][controls]") {
    REQUIRE(controls_action_name_at(-1).empty());
    REQUIRE(controls_action_name_at(kControlsActionCount).empty());
    REQUIRE(controls_group_at(-1) == -1);
    REQUIRE(controls_group_at(kControlsActionCount) == -1);
}

TEST_CASE("Controls: UP/DOWN navega os 32 itens com WRAP (30 actions + "
          "Restaurar padrao + Voltar)",
          "[system_menu][controls]") {
    SystemMenuState state;
    goto_controls(state);
    REQUIRE(state.screen == SystemMenuScreen::Controls);
    REQUIRE(state.controls_selected == 0);

    for (int i = 1; i < kControlsItemCount; ++i) {
        system_menu_key_down(state, SDLK_DOWN);
        REQUIRE(state.controls_selected == i);
    }
    system_menu_key_down(state, SDLK_DOWN);  // wrap
    REQUIRE(state.controls_selected == 0);

    system_menu_key_down(state, SDLK_UP);  // wrap pro fim
    REQUIRE(state.controls_selected == kControlsBackIndex);
}

TEST_CASE("Controls: ESC na navegacao normal volta pra ConfigCategories "
          "preservando a selecao (Controles)",
          "[system_menu][controls]") {
    SystemMenuState state;
    goto_controls(state);
    const SystemMenuAction action = system_menu_key_down(state, SDLK_ESCAPE);
    REQUIRE(action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::ConfigCategories);
    REQUIRE(state.config_categories_selected == static_cast<int>(ConfigCategoryItem::Controls));
}

TEST_CASE("Controls: ENTER numa action entra em modo CAPTURA (nao muda de "
          "tela)",
          "[system_menu][controls]") {
    SystemMenuState state;
    goto_controls(state);
    const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(action == SystemMenuAction::None);
    REQUIRE(state.controls_capturing);
    REQUIRE(state.screen == SystemMenuScreen::Controls);
}

TEST_CASE("Controls: ENQUANTO capturando, system_menu_key_down normal e "
          "no-op defensivo (o CHAMADOR deve rotear pra "
          "system_menu_controls_capture_key)",
          "[system_menu][controls]") {
    SystemMenuState state;
    goto_controls(state);
    system_menu_key_down(state, SDLK_RETURN);  // entra em captura
    REQUIRE(state.controls_capturing);

    const SystemMenuAction action = system_menu_key_down(state, SDLK_DOWN);
    REQUIRE(action == SystemMenuAction::None);
    REQUIRE(state.controls_selected == 0);  // nao navegou
    REQUIRE(state.controls_capturing);      // continua capturando
}

TEST_CASE("system_menu_controls_capture_key: Esc CANCELA a captura sem mudar "
          "o config",
          "[system_menu][controls]") {
    SystemMenuState state;
    goto_controls(state);
    state.controls_config = gus::domain::input::default_controls();
    const gus::domain::input::InputRemapConfig before = state.controls_config;
    system_menu_key_down(state, SDLK_RETURN);  // entra em captura (action 0 = move_forward)
    REQUIRE(state.controls_capturing);

    const SystemMenuAction action =
        system_menu_controls_capture_key(state, /*is_escape=*/true, /*godot_keycode=*/0);
    REQUIRE(action == SystemMenuAction::None);
    REQUIRE_FALSE(state.controls_capturing);
    REQUIRE(state.controls_config == before);
}

TEST_CASE("system_menu_controls_capture_key: tecla LIVRE aplica o remap e "
          "devolve ControlsChanged",
          "[system_menu][controls]") {
    SystemMenuState state;
    goto_controls(state);
    state.controls_config = gus::domain::input::default_controls();
    system_menu_key_down(state, SDLK_RETURN);  // captura action 0 = move_forward (default 'W')
    REQUIRE(state.controls_capturing);

    const SystemMenuAction action =
        system_menu_controls_capture_key(state, /*is_escape=*/false, /*godot_keycode=*/'K');
    REQUIRE(action == SystemMenuAction::ControlsChanged);
    REQUIRE_FALSE(state.controls_capturing);
    REQUIRE_FALSE(state.controls_last_action_swapped);

    const auto& actions = state.controls_config.actions;
    const auto it = std::find_if(actions.begin(), actions.end(), [](const auto& a) {
        return a.action_name == "move_forward";
    });
    REQUIRE(it != actions.end());
    REQUIRE(it->keys.front().keycode == 'K');
}

TEST_CASE("system_menu_controls_capture_key: CONFLITO troca com a outra "
          "action e preenche o aviso de swap",
          "[system_menu][controls]") {
    SystemMenuState state;
    goto_controls(state);
    state.controls_config = gus::domain::input::default_controls();
    system_menu_key_down(state, SDLK_RETURN);  // captura action 0 = move_forward ('W')

    // move_backward e 'S' por default (action 1 da lista curada).
    const SystemMenuAction action =
        system_menu_controls_capture_key(state, /*is_escape=*/false, /*godot_keycode=*/'S');
    REQUIRE(action == SystemMenuAction::ControlsChanged);
    REQUIRE(state.controls_last_action_swapped);
    REQUIRE(state.controls_last_swapped_with_action == "move_backward");
    REQUIRE_FALSE(state.controls_last_swapped_with_label_key.empty());

    const auto& actions = state.controls_config.actions;
    auto find_action = [&](const char* name) {
        return std::find_if(actions.begin(), actions.end(),
                             [&](const auto& a) { return a.action_name == name; });
    };
    REQUIRE(find_action("move_forward")->keys.front().keycode == 'S');
    REQUIRE(find_action("move_backward")->keys.front().keycode == 'W');
}

TEST_CASE("system_menu_controls_capture_key: keycode SENTINELA (0, tecla sem "
          "correspondente) e ignorado - permanece capturando",
          "[system_menu][controls]") {
    SystemMenuState state;
    goto_controls(state);
    state.controls_config = gus::domain::input::default_controls();
    const gus::domain::input::InputRemapConfig before = state.controls_config;
    system_menu_key_down(state, SDLK_RETURN);

    const SystemMenuAction action =
        system_menu_controls_capture_key(state, /*is_escape=*/false, /*godot_keycode=*/0);
    REQUIRE(action == SystemMenuAction::None);
    REQUIRE(state.controls_capturing);  // continua capturando
    REQUIRE(state.controls_config == before);
}

TEST_CASE("system_menu_controls_capture_key: chamado FORA do modo captura e "
          "no-op defensivo",
          "[system_menu][controls]") {
    SystemMenuState state;
    goto_controls(state);
    state.controls_config = gus::domain::input::default_controls();
    const gus::domain::input::InputRemapConfig before = state.controls_config;
    REQUIRE_FALSE(state.controls_capturing);

    const SystemMenuAction action =
        system_menu_controls_capture_key(state, /*is_escape=*/false, /*godot_keycode=*/'K');
    REQUIRE(action == SystemMenuAction::None);
    REQUIRE(state.controls_config == before);
}

TEST_CASE("Controls: ENTER em Restaurar padrao (kControlsRestoreIndex) abre o "
          "prompt de confirmacao SEM mudar o config ainda",
          "[system_menu][controls]") {
    SystemMenuState state;
    goto_controls(state);
    state.controls_config = gus::domain::input::default_controls();
    // Rebind move_forward pra 'K' - deve SOBREVIVER ate a confirmacao de fato.
    system_menu_key_down(state, SDLK_RETURN);
    system_menu_controls_capture_key(state, false, 'K');

    state.controls_selected = kControlsRestoreIndex;
    const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(action == SystemMenuAction::None);
    REQUIRE(state.controls_confirming_restore);
    REQUIRE(state.controls_restore_confirm_selected == 1);  // default seguro = Nao

    const auto& actions = state.controls_config.actions;
    const auto it = std::find_if(actions.begin(), actions.end(), [](const auto& a) {
        return a.action_name == "move_forward";
    });
    REQUIRE(it->keys.front().keycode == 'K');  // ainda nao restaurou
}

TEST_CASE("Controls: confirmar restaurar (Sim) aplica default_controls() e "
          "devolve ControlsChanged; Esc/Nao CANCELA sem mudar nada",
          "[system_menu][controls]") {
    // Caminho Nao/Esc: cancela, config customizado sobrevive.
    {
        SystemMenuState state;
        goto_controls(state);
        state.controls_config = gus::domain::input::default_controls();
        system_menu_key_down(state, SDLK_RETURN);
        system_menu_controls_capture_key(state, false, 'K');
        const gus::domain::input::InputRemapConfig customizado = state.controls_config;

        state.controls_selected = kControlsRestoreIndex;
        system_menu_key_down(state, SDLK_RETURN);  // abre o prompt
        REQUIRE(state.controls_confirming_restore);

        const SystemMenuAction esc_action = system_menu_key_down(state, SDLK_ESCAPE);
        REQUIRE(esc_action == SystemMenuAction::None);
        REQUIRE_FALSE(state.controls_confirming_restore);
        REQUIRE(state.controls_config == customizado);  // intacto
    }

    // Caminho Sim: LEFT/RIGHT alterna pra Sim (indice 0), ENTER confirma.
    {
        SystemMenuState state;
        goto_controls(state);
        state.controls_config = gus::domain::input::default_controls();
        system_menu_key_down(state, SDLK_RETURN);
        system_menu_controls_capture_key(state, false, 'K');  // customiza

        state.controls_selected = kControlsRestoreIndex;
        system_menu_key_down(state, SDLK_RETURN);  // abre o prompt (default = Nao)
        REQUIRE(state.controls_restore_confirm_selected == 1);
        system_menu_key_down(state, SDLK_LEFT);  // alterna pra Sim (0)
        REQUIRE(state.controls_restore_confirm_selected == 0);

        const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
        REQUIRE(action == SystemMenuAction::ControlsChanged);
        REQUIRE_FALSE(state.controls_confirming_restore);
        REQUIRE(state.controls_config == gus::domain::input::default_controls());
    }
}

TEST_CASE("Controls: ENTER em Voltar (kControlsBackIndex) volta pra "
          "ConfigCategories",
          "[system_menu][controls]") {
    SystemMenuState state;
    goto_controls(state);
    state.controls_selected = kControlsBackIndex;
    const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::ConfigCategories);
}

TEST_CASE("system_menu_click_option (Controls): clicar numa action foca+entra "
          "em captura; clicar em Restaurar/Voltar confirma na hora",
          "[system_menu][controls]") {
    SystemMenuState state;
    goto_controls(state);

    SystemMenuState action_click = state;
    REQUIRE(system_menu_click_option(action_click, 3) == SystemMenuAction::None);
    REQUIRE(action_click.controls_selected == 3);
    REQUIRE(action_click.controls_capturing);

    SystemMenuState restore_click = state;
    REQUIRE(system_menu_click_option(restore_click, kControlsRestoreIndex) ==
            SystemMenuAction::None);
    REQUIRE(restore_click.controls_confirming_restore);

    SystemMenuState back_click = state;
    REQUIRE(system_menu_click_option(back_click, kControlsBackIndex) ==
            SystemMenuAction::Navigated);
    REQUIRE(back_click.screen == SystemMenuScreen::ConfigCategories);
}

TEST_CASE("system_menu_click_option (Controls): durante confirmar-restaurar, "
          "indice reinterpretado como Sim(0)/Nao(1)",
          "[system_menu][controls]") {
    SystemMenuState state;
    goto_controls(state);
    state.controls_config = gus::domain::input::default_controls();
    state.controls_selected = kControlsRestoreIndex;
    system_menu_key_down(state, SDLK_RETURN);  // abre o prompt
    REQUIRE(state.controls_confirming_restore);

    SystemMenuState click_yes = state;
    const SystemMenuAction yes_action = system_menu_click_option(click_yes, 0);
    REQUIRE(yes_action == SystemMenuAction::ControlsChanged);
    REQUIRE_FALSE(click_yes.controls_confirming_restore);

    SystemMenuState click_no = state;
    const SystemMenuAction no_action = system_menu_click_option(click_no, 1);
    REQUIRE(no_action == SystemMenuAction::None);
    REQUIRE_FALSE(click_no.controls_confirming_restore);
}

// ---------------------------------------------------------------- Genericos

TEST_CASE("system_menu_close: fecha de qualquer tela (mesmo funda na arvore) "
          "para Hidden",
          "[system_menu]") {
    SystemMenuState state;
    goto_audio(state);
    system_menu_close(state);
    REQUIRE(state.screen == SystemMenuScreen::Hidden);
}

TEST_CASE("Teclas fora do conjunto conhecido (ex.: SDLK_Q) nao mudam estado nem "
          "produzem action",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    const int before = state.pause_selected;
    const SystemMenuAction action = system_menu_key_down(state, SDLK_Q);
    REQUIRE(action == SystemMenuAction::None);
    REQUIRE(state.pause_selected == before);
}

// ---------------------------------------------------------------- Mouse (click_option)

TEST_CASE("system_menu_click_option (Pause): clicar numa pill seleciona E "
          "confirma na hora (equivalente a focar + ENTER)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);

    SystemMenuState continuar = state;
    REQUIRE(system_menu_click_option(continuar, 0) == SystemMenuAction::Continue);

    SystemMenuState save = state;
    const SystemMenuAction save_action = system_menu_click_option(save, 1);
    REQUIRE(save_action == SystemMenuAction::Navigated);
    REQUIRE(save.screen == SystemMenuScreen::Save);

    SystemMenuState config = state;
    const SystemMenuAction cfg_action = system_menu_click_option(config, 2);
    REQUIRE(cfg_action == SystemMenuAction::Navigated);
    REQUIRE(config.screen == SystemMenuScreen::ConfigCategories);
    REQUIRE(config.config_categories_selected == 0);  // foco inicial = Audio

    SystemMenuState sair = state;
    REQUIRE(system_menu_click_option(sair, 3) == SystemMenuAction::RequestQuit);

    // indice invalido: no-op defensivo, nao muda pause_selected.
    SystemMenuState invalido = state;
    const int before = invalido.pause_selected;
    REQUIRE(system_menu_click_option(invalido, 99) == SystemMenuAction::None);
    REQUIRE(invalido.pause_selected == before);
}

TEST_CASE("system_menu_click_option (ConfigCategories): clicar em qualquer "
          "categoria confirma na hora (Audio/Video/Controles/Lingua/Voltar)",
          "[system_menu]") {
    SystemMenuState state;
    goto_config_categories(state);

    SystemMenuState audio = state;
    const SystemMenuAction audio_action = system_menu_click_option(audio, 0);
    REQUIRE(audio_action == SystemMenuAction::Navigated);
    REQUIRE(audio.screen == SystemMenuScreen::Audio);

    SystemMenuState video = state;
    REQUIRE(system_menu_click_option(video, 1) == SystemMenuAction::Navigated);
    REQUIRE(video.screen == SystemMenuScreen::Video);

    SystemMenuState controls = state;
    REQUIRE(system_menu_click_option(controls, 2) == SystemMenuAction::Navigated);
    REQUIRE(controls.screen == SystemMenuScreen::Controls);

    SystemMenuState lang = state;
    REQUIRE(system_menu_click_option(lang, 3) == SystemMenuAction::Navigated);
    REQUIRE(lang.screen == SystemMenuScreen::Language);

    SystemMenuState back = state;
    REQUIRE(system_menu_click_option(back, 4) == SystemMenuAction::Navigated);
    REQUIRE(back.screen == SystemMenuScreen::Pause);
}

TEST_CASE("system_menu_click_option (Audio): clicar em Musica/SFX SO FOCA "
          "(nao muda volume); clicar em Voltar confirma (Navigated)",
          "[system_menu]") {
    SystemMenuState state;
    goto_audio(state);

    const SystemMenuAction sfx_action = system_menu_click_option(state, 1);
    REQUIRE(sfx_action == SystemMenuAction::None);  // so foca, nao e VolumeChanged
    REQUIRE(state.audio_selected == 1);
    REQUIRE(state.music_volume == Catch::Approx(1.0f));  // intocado
    REQUIRE(state.sfx_volume == Catch::Approx(1.0f));    // intocado (so focou)

    const SystemMenuAction back_action = system_menu_click_option(state, 2);
    REQUIRE(back_action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::ConfigCategories);
}

TEST_CASE("system_menu_click_option (placeholder): so kPlaceholderBackIndex "
          "confirma; qualquer outro indice e no-op",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_RETURN);  // Save
    REQUIRE(state.screen == SystemMenuScreen::Save);

    SystemMenuState invalido = state;
    REQUIRE(system_menu_click_option(invalido, 5) == SystemMenuAction::None);
    REQUIRE(invalido.screen == SystemMenuScreen::Save);

    const SystemMenuAction action =
        system_menu_click_option(state, kPlaceholderBackIndex);
    REQUIRE(action == SystemMenuAction::Navigated);
    REQUIRE(state.screen == SystemMenuScreen::Pause);
}

TEST_CASE("system_menu_click_option: Hidden (menu fechado) e no-op defensivo",
          "[system_menu]") {
    SystemMenuState state;  // screen == Hidden por default
    REQUIRE(system_menu_click_option(state, 0) == SystemMenuAction::None);
}

TEST_CASE("system_menu_set_slider_ratio: converte fracao de mouse [0,1] em "
          "volume, alimentando VolumeChanged via drag/click (item 0=musica, "
          "1=sfx)",
          "[system_menu]") {
    SystemMenuState state;
    goto_audio(state);

    system_menu_set_slider_ratio(state, /*item=*/0, /*ratio=*/0.3f);
    REQUIRE(state.music_volume == Catch::Approx(0.3f));

    system_menu_set_slider_ratio(state, /*item=*/1, /*ratio=*/0.8f);
    REQUIRE(state.sfx_volume == Catch::Approx(0.8f));

    // Fora de [0,1] clampa (arrastar o mouse alem da borda do track).
    system_menu_set_slider_ratio(state, /*item=*/0, /*ratio=*/-0.5f);
    REQUIRE(state.music_volume == Catch::Approx(0.0f));
    system_menu_set_slider_ratio(state, /*item=*/1, /*ratio=*/5.0f);
    REQUIRE(state.sfx_volume == Catch::Approx(1.0f));
}

// -------------------------------------------------------- HOVER (mouse, SOM)

namespace {

// 4 caixas empilhadas verticalmente, sem sobreposicao (mesmo espirito de
// pills reais - 100dp de altura cada, y crescente), pra testar
// system_menu_hover_index sem depender de layout RCSS de verdade.
void make_boxes_into(SystemMenuHoverBox (&boxes)[kSystemMenuMaxHoverItems]) {
    for (int i = 0; i < kSystemMenuMaxHoverItems; ++i) {
        boxes[i] = SystemMenuHoverBox{/*found=*/true, /*x=*/10.0f,
                                       /*y=*/static_cast<float>(i * 100),
                                       /*w=*/200.0f, /*h=*/100.0f};
    }
}

}  // namespace

TEST_CASE("system_menu_hover_index: Pause (4 itens) acha o indice sob o mouse",
          "[system_menu][hover]") {
    SystemMenuState state;
    system_menu_open(state);  // screen=Pause
    SystemMenuHoverBox boxes[kSystemMenuMaxHoverItems];
    make_boxes_into(boxes);

    REQUIRE(system_menu_hover_index(state, 50.0f, 50.0f, boxes) == 0);   // dentro do item 0
    REQUIRE(system_menu_hover_index(state, 50.0f, 150.0f, boxes) == 1);  // item 1
    REQUIRE(system_menu_hover_index(state, 50.0f, 250.0f, boxes) == 2);  // item 2
    REQUIRE(system_menu_hover_index(state, 50.0f, 350.0f, boxes) == 3);  // item 3
}

TEST_CASE("system_menu_hover_index: mouse fora de qualquer caixa devolve -1",
          "[system_menu][hover]") {
    SystemMenuState state;
    system_menu_open(state);
    SystemMenuHoverBox boxes[kSystemMenuMaxHoverItems];
    make_boxes_into(boxes);

    REQUIRE(system_menu_hover_index(state, 9999.0f, 9999.0f, boxes) == -1);
    REQUIRE(system_menu_hover_index(state, -5.0f, -5.0f, boxes) == -1);
}

TEST_CASE("system_menu_hover_index: found=false conta como fora (MESMO contrato "
          "do hit-test de clique)",
          "[system_menu][hover]") {
    SystemMenuState state;
    system_menu_open(state);
    SystemMenuHoverBox boxes[kSystemMenuMaxHoverItems];
    make_boxes_into(boxes);
    boxes[0].found = false;  // elemento ainda nao resolvido pelo glintfx

    // Mouse dentro da GEOMETRIA do item 0, mas found=false -> nao bate; cai
    // pro proximo item que bater (nenhum aqui, pois y=50 so cai no item 0).
    REQUIRE(system_menu_hover_index(state, 50.0f, 50.0f, boxes) == -1);
}

TEST_CASE("system_menu_hover_index: Audio (3 itens) e placeholder (1 item) "
          "respeitam o count da tela atual",
          "[system_menu][hover]") {
    SystemMenuHoverBox boxes[kSystemMenuMaxHoverItems];
    make_boxes_into(boxes);

    SystemMenuState audio_state;
    goto_audio(audio_state);  // screen=Audio, kAudioItemCount=3
    REQUIRE(system_menu_hover_index(audio_state, 50.0f, 250.0f, boxes) == 2);
    // boxes[3] existe geometricamente mas Audio so testa os 3 primeiros.
    REQUIRE(system_menu_hover_index(audio_state, 50.0f, 350.0f, boxes) == -1);

    SystemMenuState save_state;
    system_menu_open(save_state);
    save_state.screen = SystemMenuScreen::Save;  // placeholder, kPlaceholderItemCount=1
    REQUIRE(system_menu_hover_index(save_state, 50.0f, 50.0f, boxes) == 0);
    REQUIRE(system_menu_hover_index(save_state, 50.0f, 150.0f, boxes) == -1);
}

TEST_CASE("system_menu_hover_index: Hidden (menu fechado) sempre devolve -1",
          "[system_menu][hover]") {
    SystemMenuState state;  // screen == Hidden por default
    SystemMenuHoverBox boxes[kSystemMenuMaxHoverItems];
    make_boxes_into(boxes);
    REQUIRE(system_menu_hover_index(state, 50.0f, 50.0f, boxes) == -1);
}

TEST_CASE("system_menu_hover_entered_new_item: dispara so ao ENTRAR num item "
          "NOVO e valido",
          "[system_menu][hover]") {
    // -1 (nenhum) -> item 0: entrou, dispara.
    REQUIRE(system_menu_hover_entered_new_item(-1, 0) == true);
    // item 0 -> item 0 (parado no mesmo item): NAO redispara.
    REQUIRE(system_menu_hover_entered_new_item(0, 0) == false);
    // item 0 -> item 1: mudou de item, dispara.
    REQUIRE(system_menu_hover_entered_new_item(0, 1) == true);
    // item 1 -> -1 (saiu pra fora): NAO dispara (so ENTRAR soa).
    REQUIRE(system_menu_hover_entered_new_item(1, -1) == false);
    // -1 -> -1 (fora, continua fora): NAO dispara.
    REQUIRE(system_menu_hover_entered_new_item(-1, -1) == false);
    // Sair (vira -1) e voltar pro MESMO item de antes: redispara (a "memoria"
    // e so o ULTIMO indice visto, o -1 no meio zera o historico). Sequencia:
    // hovered=2 -> sai (novo=-1, nao dispara) -> reentra no MESMO item 2
    // (novo=2 != previous=-1, dispara de novo).
    REQUIRE(system_menu_hover_entered_new_item(2, -1) == false);  // saiu do item 2
    REQUIRE(system_menu_hover_entered_new_item(-1, 2) == true);  // reentrou no 2: dispara
}
