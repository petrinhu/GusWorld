// GusEngine/app/tests/difficulty_menu_test.cpp
//
// Catch2 (TEST-FIRST) de difficulty_menu.hpp (MODOS-MORTE Fase 0, TELA DE
// SELECAO DE DIFICULDADE). POCO 100% testavel sem SDL_Init/janela/glintfx -
// mesmo espirito de title_menu_test.cpp. Cobre: foco inicial em Medio,
// navegacao com wrap (todos os 3 itens sempre selecionaveis), Enter abre o
// splash (Aviso #2), splash confirma/cancela, mapeamento indice->
// DifficultyLevel, e o GAP preenchido (ESC na lista -> Cancelled).

#include <catch2/catch_test_macros.hpp>

#include "gus/app/screens/difficulty_menu.hpp"

using namespace gus::app::screens;
using gus::domain::save::DifficultyLevel;

// ---------------------------------------------------------------- difficulty_level_for_item

TEST_CASE("difficulty_level_for_item: mapeia os 3 itens na ordem da spec",
          "[difficulty_menu]") {
    REQUIRE(difficulty_level_for_item(static_cast<int>(DifficultyMenuItem::Facil)) ==
            DifficultyLevel::Facil);
    REQUIRE(difficulty_level_for_item(static_cast<int>(DifficultyMenuItem::Medio)) ==
            DifficultyLevel::Medio);
    REQUIRE(difficulty_level_for_item(static_cast<int>(DifficultyMenuItem::Dificil)) ==
            DifficultyLevel::Dificil);
}

// ---------------------------------------------------------------- difficulty_menu_open

TEST_CASE("difficulty_menu_open: foco inicial = Medio (default canonico §2.1)",
          "[difficulty_menu]") {
    DifficultyMenuState state;
    state.selected = static_cast<int>(DifficultyMenuItem::Dificil);  // suja de proposito
    state.confirming = true;
    difficulty_menu_open(state);
    REQUIRE(state.selected == static_cast<int>(DifficultyMenuItem::Medio));
    REQUIRE_FALSE(state.confirming);
    REQUIRE(state.confirm_selected == 1);  // default seguro
}

// ---------------------------------------------------------------- navegacao (todos selecionaveis)

TEST_CASE("difficulty_menu_key_down: Baixo/Cima visitam os 3 itens com wrap",
          "[difficulty_menu]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    REQUIRE(state.selected == static_cast<int>(DifficultyMenuItem::Medio));

    REQUIRE(difficulty_menu_key_down(state, SDLK_DOWN) == DifficultyMenuAction::None);
    REQUIRE(state.selected == static_cast<int>(DifficultyMenuItem::Dificil));
    REQUIRE(difficulty_menu_key_down(state, SDLK_DOWN) == DifficultyMenuAction::None);
    REQUIRE(state.selected == static_cast<int>(DifficultyMenuItem::Facil));  // wrap
    REQUIRE(difficulty_menu_key_down(state, SDLK_UP) == DifficultyMenuAction::None);
    REQUIRE(state.selected == static_cast<int>(DifficultyMenuItem::Dificil));
}

TEST_CASE("difficulty_menu_key_down: WASD espelha as setas", "[difficulty_menu]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    REQUIRE(difficulty_menu_key_down(state, SDLK_S) == DifficultyMenuAction::None);
    REQUIRE(state.selected == static_cast<int>(DifficultyMenuItem::Dificil));
    REQUIRE(difficulty_menu_key_down(state, SDLK_W) == DifficultyMenuAction::None);
    REQUIRE(state.selected == static_cast<int>(DifficultyMenuItem::Medio));
}

// ---------------------------------------------------------------- Enter abre o splash (Aviso #2)

TEST_CASE("difficulty_menu_key_down: Enter na lista abre o splash (nao devolve "
          "Chosen ainda)",
          "[difficulty_menu]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    state.selected = static_cast<int>(DifficultyMenuItem::Facil);
    REQUIRE(difficulty_menu_key_down(state, SDLK_RETURN) == DifficultyMenuAction::None);
    REQUIRE(state.confirming);
    REQUIRE(state.confirm_selected == 1);  // default seguro = Cancelar
    REQUIRE(state.selected == static_cast<int>(DifficultyMenuItem::Facil));  // preservado
}

// ---------------------------------------------------------------- splash confirmar/cancelar

TEST_CASE("difficulty_menu_key_down: splash - Enter com Cancelar (default) fecha, "
          "permanece na lista",
          "[difficulty_menu]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    (void)difficulty_menu_key_down(state, SDLK_RETURN);  // abre o splash
    REQUIRE(difficulty_menu_key_down(state, SDLK_RETURN) == DifficultyMenuAction::None);
    REQUIRE_FALSE(state.confirming);
}

TEST_CASE("difficulty_menu_key_down: splash - alternar pra Confirmar devolve Chosen",
          "[difficulty_menu]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    state.selected = static_cast<int>(DifficultyMenuItem::Dificil);
    (void)difficulty_menu_key_down(state, SDLK_RETURN);  // abre o splash

    REQUIRE(difficulty_menu_key_down(state, SDLK_LEFT) == DifficultyMenuAction::None);
    REQUIRE(state.confirm_selected == 0);
    REQUIRE(difficulty_menu_key_down(state, SDLK_RETURN) == DifficultyMenuAction::Chosen);
    REQUIRE_FALSE(state.confirming);
    REQUIRE(difficulty_level_for_item(state.selected) == DifficultyLevel::Dificil);
}

TEST_CASE("difficulty_menu_key_down: splash - Esc equivale a Cancelar (seguranca)",
          "[difficulty_menu]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    (void)difficulty_menu_key_down(state, SDLK_RETURN);  // abre o splash
    REQUIRE(difficulty_menu_key_down(state, SDLK_ESCAPE) == DifficultyMenuAction::None);
    REQUIRE_FALSE(state.confirming);
}

// ---------------------------------------------------------------- GAP: ESC na lista

TEST_CASE("difficulty_menu_key_down: ESC na LISTA (fora do splash) devolve "
          "Cancelled (aborta Novo Jogo, volta pra titulo)",
          "[difficulty_menu]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    REQUIRE(difficulty_menu_key_down(state, SDLK_ESCAPE) == DifficultyMenuAction::Cancelled);
}

// ---------------------------------------------------------------- mouse (click)

TEST_CASE("difficulty_menu_click_option: clicar num item foca + abre o splash",
          "[difficulty_menu]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    REQUIRE(difficulty_menu_click_option(state, static_cast<int>(DifficultyMenuItem::Facil)) ==
            DifficultyMenuAction::None);
    REQUIRE(state.selected == static_cast<int>(DifficultyMenuItem::Facil));
    REQUIRE(state.confirming);
}

TEST_CASE("difficulty_menu_click_option: no splash, index reinterpreta como "
          "Confirmar/Cancelar",
          "[difficulty_menu]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    (void)difficulty_menu_key_down(state, SDLK_RETURN);  // abre o splash

    REQUIRE(difficulty_menu_click_option(state, 0) == DifficultyMenuAction::Chosen);
    REQUIRE_FALSE(state.confirming);
}

TEST_CASE("difficulty_menu_click_option: index fora do intervalo e no-op",
          "[difficulty_menu]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    REQUIRE(difficulty_menu_click_option(state, 99) == DifficultyMenuAction::None);
    REQUIRE(state.selected == static_cast<int>(DifficultyMenuItem::Medio));  // intocado
}

// ---------------------------------------------------------------- difficulty_keyboard_focus_index

TEST_CASE("difficulty_keyboard_focus_index: fora do splash devolve state.selected",
          "[difficulty_menu]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    REQUIRE(difficulty_keyboard_focus_index(state) ==
            static_cast<int>(DifficultyMenuItem::Medio));
    (void)difficulty_menu_key_down(state, SDLK_DOWN);
    REQUIRE(difficulty_keyboard_focus_index(state) ==
            static_cast<int>(DifficultyMenuItem::Dificil));
}

TEST_CASE("difficulty_keyboard_focus_index: no splash devolve confirm_selected",
          "[difficulty_menu]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    (void)difficulty_menu_key_down(state, SDLK_RETURN);  // abre o splash (confirm_selected=1)
    REQUIRE(difficulty_keyboard_focus_index(state) == 1);
    (void)difficulty_menu_key_down(state, SDLK_LEFT);  // alterna pra Confirmar (0)
    REQUIRE(difficulty_keyboard_focus_index(state) == 0);
}

// ---------------------------------------------------------------- difficulty_hover_index

namespace {

void make_difficulty_boxes_into(UiHoverBox (&boxes)[kDifficultyItemCount]) {
    for (int i = 0; i < kDifficultyItemCount; ++i) {
        boxes[i] = UiHoverBox{/*found=*/true, /*x=*/10.0f,
                               /*y=*/static_cast<float>(i * 100), /*w=*/200.0f,
                               /*h=*/100.0f};
    }
}

}  // namespace

TEST_CASE("difficulty_hover_index: fora do splash, acha o indice sob o mouse "
          "entre os 3 itens",
          "[difficulty_menu][hover]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    UiHoverBox boxes[kDifficultyItemCount];
    make_difficulty_boxes_into(boxes);

    REQUIRE(difficulty_hover_index(state, 50.0f, 50.0f, boxes) == 0);
    REQUIRE(difficulty_hover_index(state, 50.0f, 150.0f, boxes) == 1);
    REQUIRE(difficulty_hover_index(state, 50.0f, 250.0f, boxes) == 2);
}

TEST_CASE("difficulty_hover_index: mouse fora de qualquer caixa devolve -1",
          "[difficulty_menu][hover]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    UiHoverBox boxes[kDifficultyItemCount];
    make_difficulty_boxes_into(boxes);

    REQUIRE(difficulty_hover_index(state, 9999.0f, 9999.0f, boxes) == -1);
}

TEST_CASE("difficulty_hover_index: no splash, SO as 2 primeiras caixas contam",
          "[difficulty_menu][hover]") {
    DifficultyMenuState state;
    difficulty_menu_open(state);
    (void)difficulty_menu_key_down(state, SDLK_RETURN);  // abre o splash

    UiHoverBox boxes[kDifficultyItemCount];
    make_difficulty_boxes_into(boxes);  // boxes[2] tambem "found", mas fora do count=2

    REQUIRE(difficulty_hover_index(state, 50.0f, 50.0f, boxes) == 0);    // pill Confirmar
    REQUIRE(difficulty_hover_index(state, 50.0f, 150.0f, boxes) == 1);   // pill Cancelar
    REQUIRE(difficulty_hover_index(state, 50.0f, 250.0f, boxes) == -1);  // ignorada
}
