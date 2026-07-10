// GusEngine/app/tests/title_menu_test.cpp
//
// Catch2 (TEST-FIRST) de title_menu.hpp (SAVE-LOAD-UI etapa 4, TELA DE TITULO).
// POCO 100% testavel sem SDL_Init/janela/glintfx - mesmo espirito de
// system_menu_test.cpp/save_load_menu_test.cpp. Cobre: selecionabilidade
// (Continuar desabilitado sem save), navegacao com wrap pulando o desabilitado,
// Enter em cada item, e o mini-dialogo de confirmacao de Novo Jogo.

#include <catch2/catch_test_macros.hpp>

#include "gus/app/screens/title_menu.hpp"

using namespace gus::app::screens;

// ---------------------------------------------------------------- title_item_selectable

TEST_CASE("title_item_selectable: Continuar so e selecionavel se any_save_exists",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/false);
    REQUIRE_FALSE(title_item_selectable(state, static_cast<int>(TitleMenuItem::Continue)));
    REQUIRE(title_item_selectable(state, static_cast<int>(TitleMenuItem::NewGame)));
    REQUIRE(title_item_selectable(state, static_cast<int>(TitleMenuItem::Quit)));

    title_menu_open(state, /*any_save_exists=*/true);
    REQUIRE(title_item_selectable(state, static_cast<int>(TitleMenuItem::Continue)));
}

TEST_CASE("title_item_selectable: indice fora do intervalo devolve false (defensivo)",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, true);
    REQUIRE_FALSE(title_item_selectable(state, -1));
    REQUIRE_FALSE(title_item_selectable(state, kTitleItemCount));
}

// ---------------------------------------------------------------- title_menu_open

TEST_CASE("title_menu_open: com save existente, foco inicial = Continuar",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::Continue));
    REQUIRE(state.any_save_exists);
    REQUIRE_FALSE(state.confirming_new_game);
}

TEST_CASE("title_menu_open: SEM save nenhum, Continuar desabilitado - foco inicial "
          "pula pra Novo Jogo",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/false);
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::NewGame));
    REQUIRE_FALSE(state.any_save_exists);
}

// ---------------------------------------------------------------- navegacao (pula o desabilitado)

TEST_CASE("title_menu_key_down: Baixo/Cima pulam Continuar quando desabilitado (wrap)",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/false);
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::NewGame));

    REQUIRE(title_menu_key_down(state, SDLK_DOWN) == TitleMenuAction::None);
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::Quit));

    // DOWN de novo: wrap ate NewGame de novo (pula Continue, indice 0, desabilitado).
    REQUIRE(title_menu_key_down(state, SDLK_DOWN) == TitleMenuAction::None);
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::NewGame));

    // UP a partir de NewGame: wrap pro ULTIMO selecionavel (Quit, pula Continue).
    REQUIRE(title_menu_key_down(state, SDLK_UP) == TitleMenuAction::None);
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::Quit));
}

TEST_CASE("title_menu_key_down: com save existente, navegacao visita os 3 itens",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::Continue));

    REQUIRE(title_menu_key_down(state, SDLK_DOWN) == TitleMenuAction::None);
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::NewGame));
    REQUIRE(title_menu_key_down(state, SDLK_DOWN) == TitleMenuAction::None);
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::Quit));
    REQUIRE(title_menu_key_down(state, SDLK_DOWN) == TitleMenuAction::None);
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::Continue));
}

TEST_CASE("title_menu_key_down: WASD espelha as setas (W=cima, S=baixo)",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    REQUIRE(title_menu_key_down(state, SDLK_S) == TitleMenuAction::None);
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::NewGame));
    REQUIRE(title_menu_key_down(state, SDLK_W) == TitleMenuAction::None);
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::Continue));
}

// ---------------------------------------------------------------- Enter em cada item

TEST_CASE("title_menu_key_down: Enter em Continuar (habilitado) devolve ContinueGame",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    REQUIRE(title_menu_key_down(state, SDLK_RETURN) == TitleMenuAction::ContinueGame);
}

TEST_CASE("title_menu_key_down: Enter em Sair devolve RequestQuit", "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    state.selected = static_cast<int>(TitleMenuItem::Quit);
    REQUIRE(title_menu_key_down(state, SDLK_RETURN) == TitleMenuAction::RequestQuit);
}

TEST_CASE("title_menu_key_down: Enter em Novo Jogo SEM save nenhum comeca DIRETO "
          "(sem mini-dialogo)",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/false);
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::NewGame));
    REQUIRE(title_menu_key_down(state, SDLK_RETURN) == TitleMenuAction::StartNewGame);
    REQUIRE_FALSE(state.confirming_new_game);
}

TEST_CASE("title_menu_key_down: Enter em Novo Jogo COM save existente abre o "
          "mini-dialogo (nao comeca ainda)",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    state.selected = static_cast<int>(TitleMenuItem::NewGame);
    REQUIRE(title_menu_key_down(state, SDLK_RETURN) == TitleMenuAction::None);
    REQUIRE(state.confirming_new_game);
    REQUIRE(state.confirm_selected == 1);  // default seguro = Nao
}

// ---------------------------------------------------------------- mini-dialogo Novo Jogo

TEST_CASE("title_menu_key_down: mini-dialogo - Enter com Nao (default) cancela, "
          "permanece na tela",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    state.selected = static_cast<int>(TitleMenuItem::NewGame);
    (void)title_menu_key_down(state, SDLK_RETURN);  // abre o dialogo

    REQUIRE(title_menu_key_down(state, SDLK_RETURN) == TitleMenuAction::None);
    REQUIRE_FALSE(state.confirming_new_game);
}

TEST_CASE("title_menu_key_down: mini-dialogo - alternar pra Sim e confirmar devolve "
          "StartNewGame",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    state.selected = static_cast<int>(TitleMenuItem::NewGame);
    (void)title_menu_key_down(state, SDLK_RETURN);  // abre o dialogo

    REQUIRE(title_menu_key_down(state, SDLK_LEFT) == TitleMenuAction::None);
    REQUIRE(state.confirm_selected == 0);
    REQUIRE(title_menu_key_down(state, SDLK_RETURN) == TitleMenuAction::StartNewGame);
    REQUIRE_FALSE(state.confirming_new_game);
}

TEST_CASE("title_menu_key_down: mini-dialogo - Esc equivale a Nao (seguranca)",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    state.selected = static_cast<int>(TitleMenuItem::NewGame);
    (void)title_menu_key_down(state, SDLK_RETURN);  // abre o dialogo

    REQUIRE(title_menu_key_down(state, SDLK_ESCAPE) == TitleMenuAction::None);
    REQUIRE_FALSE(state.confirming_new_game);
}

// ---------------------------------------------------------------- mouse (click)

TEST_CASE("title_menu_click_option: clicar em Continuar desabilitado e no-op",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/false);
    REQUIRE(title_menu_click_option(state, static_cast<int>(TitleMenuItem::Continue)) ==
            TitleMenuAction::None);
    REQUIRE(state.selected == static_cast<int>(TitleMenuItem::NewGame));  // foco intocado
}

TEST_CASE("title_menu_click_option: clicar em Continuar habilitado confirma na hora",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    REQUIRE(title_menu_click_option(state, static_cast<int>(TitleMenuItem::Continue)) ==
            TitleMenuAction::ContinueGame);
}

TEST_CASE("title_menu_click_option: no mini-dialogo, index reinterpreta como Sim/Nao",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    state.selected = static_cast<int>(TitleMenuItem::NewGame);
    (void)title_menu_key_down(state, SDLK_RETURN);  // abre o dialogo

    REQUIRE(title_menu_click_option(state, 0) == TitleMenuAction::StartNewGame);
    REQUIRE_FALSE(state.confirming_new_game);
}

// ---------------------------------------------------------- title_keyboard_focus_index (COCKPIT-SFX-HOVER-CLIQUE)

TEST_CASE("title_keyboard_focus_index: fora do mini-dialogo devolve state.selected",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    REQUIRE(title_keyboard_focus_index(state) ==
            static_cast<int>(TitleMenuItem::Continue));

    (void)title_menu_key_down(state, SDLK_DOWN);
    REQUIRE(title_keyboard_focus_index(state) ==
            static_cast<int>(TitleMenuItem::NewGame));
}

TEST_CASE("title_keyboard_focus_index: no mini-dialogo devolve confirm_selected",
          "[title_menu]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    state.selected = static_cast<int>(TitleMenuItem::NewGame);
    (void)title_menu_key_down(state, SDLK_RETURN);  // abre o dialogo (confirm_selected=1)
    REQUIRE(state.confirming_new_game);
    REQUIRE(title_keyboard_focus_index(state) == 1);

    (void)title_menu_key_down(state, SDLK_LEFT);  // alterna pra Sim (indice 0)
    REQUIRE(title_keyboard_focus_index(state) == 0);
}

// ---------------------------------------------------------------- title_hover_index (COCKPIT-SFX-HOVER-CLIQUE)

namespace {

// 3 caixas empilhadas verticalmente, sem sobreposicao (MESMO espirito de
// make_boxes_into em system_menu_test.cpp) - serve tanto a lista de 3 itens
// quanto (usando so as 2 primeiras) o mini-dialogo.
void make_title_boxes_into(UiHoverBox (&boxes)[kTitleItemCount]) {
    for (int i = 0; i < kTitleItemCount; ++i) {
        boxes[i] = UiHoverBox{/*found=*/true, /*x=*/10.0f,
                               /*y=*/static_cast<float>(i * 100), /*w=*/200.0f,
                               /*h=*/100.0f};
    }
}

}  // namespace

TEST_CASE("title_hover_index: fora do mini-dialogo, acha o indice sob o mouse "
          "entre os 3 itens",
          "[title_menu][hover]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    UiHoverBox boxes[kTitleItemCount];
    make_title_boxes_into(boxes);

    REQUIRE(title_hover_index(state, 50.0f, 50.0f, boxes) == 0);
    REQUIRE(title_hover_index(state, 50.0f, 150.0f, boxes) == 1);
    REQUIRE(title_hover_index(state, 50.0f, 250.0f, boxes) == 2);
}

TEST_CASE("title_hover_index: mouse fora de qualquer caixa devolve -1",
          "[title_menu][hover]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    UiHoverBox boxes[kTitleItemCount];
    make_title_boxes_into(boxes);

    REQUIRE(title_hover_index(state, 9999.0f, 9999.0f, boxes) == -1);
}

TEST_CASE("title_hover_index: no mini-dialogo, SO as 2 primeiras caixas contam "
          "(a 3a e ignorada mesmo se 'found')",
          "[title_menu][hover]") {
    TitleMenuState state;
    title_menu_open(state, /*any_save_exists=*/true);
    state.selected = static_cast<int>(TitleMenuItem::NewGame);
    (void)title_menu_key_down(state, SDLK_RETURN);  // abre o dialogo
    REQUIRE(state.confirming_new_game);

    UiHoverBox boxes[kTitleItemCount];
    make_title_boxes_into(boxes);  // boxes[2] tambem "found", mas fora do count=2

    REQUIRE(title_hover_index(state, 50.0f, 50.0f, boxes) == 0);    // pill Sim
    REQUIRE(title_hover_index(state, 50.0f, 150.0f, boxes) == 1);   // pill Nao
    REQUIRE(title_hover_index(state, 50.0f, 250.0f, boxes) == -1);  // boxes[2] ignorada
}
