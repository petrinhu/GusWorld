// GusEngine/app/tests/system_menu_test.cpp
//
// Catch2 (TEST-FIRST) da logica PURA do menu de sistema (pausa + config de som,
// MENU-PAUSA-CONFIG-SOM, M7-COSTURA): POCO 100% testavel sem SDL/janela/glintfx -
// mesmo espirito de app/tests/battle_key_routing_test.cpp e maestro_logic_test.cpp.
// A UI (RML/RCSS) e a integracao com AudioEngine/persistencia SO CONSOMEM este
// estado; aqui so a navegacao/selecao/ajuste de volume.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <SDL3/SDL.h>  // SDL_Keycode - mesmo padrao de battle_key_down (app/ ja usa SDL)

#include "gus/app/screens/system_menu.hpp"

using namespace gus::app::screens;

TEST_CASE("SystemMenuState: comeca Hidden (menu fechado)", "[system_menu]") {
    SystemMenuState state;
    REQUIRE(state.screen == SystemMenuScreen::Hidden);
}

TEST_CASE("system_menu_open: abre em Pause com foco inicial em Continuar (item 0, "
          "mock: 'foco inicial CONTINUAR')",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    REQUIRE(state.screen == SystemMenuScreen::Pause);
    REQUIRE(state.pause_selected == 0);
}

TEST_CASE("Pause: UP/DOWN navega os 3 itens com WRAP (Continuar/Configuracoes/Sair)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    REQUIRE(state.pause_selected == 0);

    system_menu_key_down(state, SDLK_DOWN);
    REQUIRE(state.pause_selected == 1);
    system_menu_key_down(state, SDLK_DOWN);
    REQUIRE(state.pause_selected == 2);
    system_menu_key_down(state, SDLK_DOWN);  // wrap pro topo
    REQUIRE(state.pause_selected == 0);

    system_menu_key_down(state, SDLK_UP);  // wrap pro fim
    REQUIRE(state.pause_selected == 2);
}

TEST_CASE("Pause: ENTER em Continuar (item 0) devolve action Continue",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(action == SystemMenuAction::Continue);
}

TEST_CASE("Pause: ESC (volta ao jogo) devolve action Continue - mesmo efeito de "
          "confirmar Continuar (footer do mock: 'ESC volta ao jogo')",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);  // move pro item 1 (Configuracoes)
    const SystemMenuAction action = system_menu_key_down(state, SDLK_ESCAPE);
    REQUIRE(action == SystemMenuAction::Continue);
}

TEST_CASE("Pause: ENTER em Configuracoes (item 1) abre a tela de Config",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);  // item 1 = Configuracoes
    const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(action == SystemMenuAction::OpenSettings);
    REQUIRE(state.screen == SystemMenuScreen::Config);
    REQUIRE(state.config_selected == 0);  // foco inicial = Musica
}

TEST_CASE("Pause: ENTER em Sair (item 2) devolve action RequestQuit sem fechar "
          "o menu sozinho (o chamador decide encerrar o programa)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);  // item 2 = Sair
    const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(action == SystemMenuAction::RequestQuit);
}

TEST_CASE("Config: UP/DOWN navega os 3 itens com WRAP (Musica/SFX/Voltar)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_RETURN);  // entra em Config
    REQUIRE(state.config_selected == 0);

    system_menu_key_down(state, SDLK_DOWN);
    REQUIRE(state.config_selected == 1);
    system_menu_key_down(state, SDLK_DOWN);
    REQUIRE(state.config_selected == 2);
    system_menu_key_down(state, SDLK_DOWN);  // wrap
    REQUIRE(state.config_selected == 0);
}

TEST_CASE("Config: LEFT/RIGHT no item Musica ajusta music_volume clampado [0,1]",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_RETURN);  // Config, foco = Musica (0)
    REQUIRE(state.music_volume == Catch::Approx(1.0f));

    const SystemMenuAction a1 = system_menu_key_down(state, SDLK_LEFT);
    REQUIRE(a1 == SystemMenuAction::VolumeChanged);
    REQUIRE(state.music_volume < 1.0f);

    // Repete LEFT ate esbarrar no piso 0.0 (nunca fica negativo).
    for (int i = 0; i < 30; ++i) system_menu_key_down(state, SDLK_LEFT);
    REQUIRE(state.music_volume == Catch::Approx(0.0f));

    for (int i = 0; i < 30; ++i) system_menu_key_down(state, SDLK_RIGHT);
    REQUIRE(state.music_volume == Catch::Approx(1.0f));
}

TEST_CASE("Config: LEFT/RIGHT no item SFX (indice 1) ajusta sfx_volume, NAO "
          "music_volume",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_RETURN);  // Config
    system_menu_key_down(state, SDLK_DOWN);    // foco = SFX (1)
    REQUIRE(state.config_selected == 1);

    system_menu_key_down(state, SDLK_LEFT);
    REQUIRE(state.sfx_volume < 1.0f);
    REQUIRE(state.music_volume == Catch::Approx(1.0f));  // intocado
}

TEST_CASE("Config: LEFT/RIGHT no item Voltar (indice 2) NAO produz VolumeChanged "
          "(nao e um slider)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_RETURN);  // Config
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);  // foco = Voltar (2)
    REQUIRE(state.config_selected == 2);

    const SystemMenuAction action = system_menu_key_down(state, SDLK_LEFT);
    REQUIRE(action != SystemMenuAction::VolumeChanged);
    REQUIRE(state.music_volume == Catch::Approx(1.0f));
    REQUIRE(state.sfx_volume == Catch::Approx(1.0f));
}

TEST_CASE("Config: ENTER/ESC em Voltar (ou ESC de qualquer foco) volta pra Pause "
          "(BackToPause), preservando a selecao anterior de Pause",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);  // Pause: foco = Configuracoes (1)
    system_menu_key_down(state, SDLK_RETURN);  // entra em Config

    const SystemMenuAction action = system_menu_key_down(state, SDLK_ESCAPE);
    REQUIRE(action == SystemMenuAction::BackToPause);
    REQUIRE(state.screen == SystemMenuScreen::Pause);
    REQUIRE(state.pause_selected == 1);  // ainda em Configuracoes, nao resetou
}

TEST_CASE("Config: ENTER no item Voltar tambem devolve BackToPause (nao so ESC)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_RETURN);  // Config
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_DOWN);  // foco = Voltar

    const SystemMenuAction action = system_menu_key_down(state, SDLK_RETURN);
    REQUIRE(action == SystemMenuAction::BackToPause);
    REQUIRE(state.screen == SystemMenuScreen::Pause);
}

TEST_CASE("system_menu_close: fecha de qualquer tela (Pause ou Config) para Hidden",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_RETURN);  // Config
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

TEST_CASE("Config: A/D ajustam volume igual LEFT/RIGHT (WASD completo)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_RETURN);  // Config, foco = Musica (0)
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

TEST_CASE("Pause: A/D nao fazem nada (sem eixo horizontal na tela Pause)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    const int before = state.pause_selected;
    REQUIRE(system_menu_key_down(state, SDLK_A) == SystemMenuAction::None);
    REQUIRE(system_menu_key_down(state, SDLK_D) == SystemMenuAction::None);
    REQUIRE(state.pause_selected == before);
}

TEST_CASE("system_menu_click_option (Pause): clicar numa pill seleciona E "
          "confirma na hora (equivalente a focar + ENTER)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);

    SystemMenuState continuar = state;
    REQUIRE(system_menu_click_option(continuar, 0) == SystemMenuAction::Continue);

    SystemMenuState config = state;
    const SystemMenuAction cfg_action = system_menu_click_option(config, 1);
    REQUIRE(cfg_action == SystemMenuAction::OpenSettings);
    REQUIRE(config.screen == SystemMenuScreen::Config);
    REQUIRE(config.config_selected == 0);  // foco inicial = Musica

    SystemMenuState sair = state;
    REQUIRE(system_menu_click_option(sair, 2) == SystemMenuAction::RequestQuit);

    // indice invalido: no-op defensivo, nao muda pause_selected.
    SystemMenuState invalido = state;
    const int before = invalido.pause_selected;
    REQUIRE(system_menu_click_option(invalido, 99) == SystemMenuAction::None);
    REQUIRE(invalido.pause_selected == before);
}

TEST_CASE("system_menu_click_option (Config): clicar em Musica/SFX SO FOCA "
          "(nao muda volume); clicar em Voltar confirma (BackToPause)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_RETURN);  // Config, foco = Musica (0)

    const SystemMenuAction sfx_action = system_menu_click_option(state, 1);
    REQUIRE(sfx_action == SystemMenuAction::None);  // so foca, nao e VolumeChanged
    REQUIRE(state.config_selected == 1);
    REQUIRE(state.music_volume == Catch::Approx(1.0f));  // intocado
    REQUIRE(state.sfx_volume == Catch::Approx(1.0f));    // intocado (so focou)

    const SystemMenuAction back_action = system_menu_click_option(state, 2);
    REQUIRE(back_action == SystemMenuAction::BackToPause);
    REQUIRE(state.screen == SystemMenuScreen::Pause);
}

TEST_CASE("system_menu_click_option: Hidden (menu fechado) e no-op defensivo",
          "[system_menu]") {
    SystemMenuState state;  // screen == Hidden por default
    REQUIRE(system_menu_click_option(state, 0) == SystemMenuAction::None);
}

TEST_CASE("system_menu_mouse_slider_ratio: converte fracao de mouse [0,1] em "
          "volume, alimentando VolumeChanged via drag/click (item 0=musica, "
          "1=sfx)",
          "[system_menu]") {
    SystemMenuState state;
    system_menu_open(state);
    system_menu_key_down(state, SDLK_DOWN);
    system_menu_key_down(state, SDLK_RETURN);  // Config

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
