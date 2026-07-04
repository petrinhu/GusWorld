// gus/app/tests/ui_hover_test.cpp
//
// COCKPIT-SFX-HOVER-CLIQUE: trava a fatia PURA/generica do "som de hover" de botoes
// (ui_hover.hpp), reusada pelo MENU DE SISTEMA (que delega a ela) e pelo COCKPIT DA
// BATALHA (pills de verbo). Espelha os testes de hover de system_menu_test.cpp, mas
// sobre a API generica (count explicito, sem SystemMenuState). Headless: zero glintfx/
// GL/janela.

#include <catch2/catch_test_macros.hpp>

#include "gus/app/screens/ui_hover.hpp"

using gus::app::screens::ui_hover_entered_new_item;
using gus::app::screens::ui_hover_index;
using gus::app::screens::UiHoverBox;

namespace {

// 4 botoes empilhados verticalmente (x=0..100, cada um 100dp de altura), MESMO
// layout sintetico dos testes do menu.
void fill_column(UiHoverBox boxes[6], int count) {
    for (int i = 0; i < count; ++i) {
        boxes[i] = UiHoverBox{true, 0.0f, static_cast<float>(i) * 100.0f, 100.0f, 100.0f};
    }
}

}  // namespace

TEST_CASE("ui_hover_index: acha o indice do botao sob o mouse", "[ui_hover]") {
    UiHoverBox boxes[6];
    fill_column(boxes, 4);
    REQUIRE(ui_hover_index(50.0f, 50.0f, boxes, 4) == 0);
    REQUIRE(ui_hover_index(50.0f, 150.0f, boxes, 4) == 1);
    REQUIRE(ui_hover_index(50.0f, 250.0f, boxes, 4) == 2);
    REQUIRE(ui_hover_index(50.0f, 350.0f, boxes, 4) == 3);
}

TEST_CASE("ui_hover_index: mouse fora de qualquer caixa devolve -1", "[ui_hover]") {
    UiHoverBox boxes[6];
    fill_column(boxes, 4);
    REQUIRE(ui_hover_index(9999.0f, 9999.0f, boxes, 4) == -1);
    REQUIRE(ui_hover_index(-5.0f, -5.0f, boxes, 4) == -1);
}

TEST_CASE("ui_hover_index: found=false conta como fora", "[ui_hover]") {
    UiHoverBox boxes[6];
    fill_column(boxes, 4);
    boxes[1].found = false;  // botao 1 sem geometria (nao carregado)
    REQUIRE(ui_hover_index(50.0f, 150.0f, boxes, 4) == -1);  // cairia no 1, mas found=false
    REQUIRE(ui_hover_index(50.0f, 50.0f, boxes, 4) == 0);    // os outros seguem normais
}

TEST_CASE("ui_hover_index: count menor limita a varredura", "[ui_hover]") {
    UiHoverBox boxes[6];
    fill_column(boxes, 4);
    // count=2: so botoes 0 e 1 sao testaveis; o ponto do botao 2 nao bate.
    REQUIRE(ui_hover_index(50.0f, 250.0f, boxes, 2) == -1);
    REQUIRE(ui_hover_index(50.0f, 150.0f, boxes, 2) == 1);
}

TEST_CASE("ui_hover_index: count<=0 ou boxes nulo devolve -1 (defensivo)", "[ui_hover]") {
    UiHoverBox boxes[6];
    fill_column(boxes, 4);
    REQUIRE(ui_hover_index(50.0f, 50.0f, boxes, 0) == -1);
    REQUIRE(ui_hover_index(50.0f, 50.0f, boxes, -3) == -1);
    REQUIRE(ui_hover_index(50.0f, 50.0f, nullptr, 4) == -1);
}

TEST_CASE("ui_hover_entered_new_item: dispara so ao ENTRAR num botao novo/valido",
          "[ui_hover]") {
    // -1 -> 0: entrou no 0 (dispara)
    REQUIRE(ui_hover_entered_new_item(-1, 0) == true);
    // 0 -> 0: parado no mesmo botao (NAO redispara)
    REQUIRE(ui_hover_entered_new_item(0, 0) == false);
    // 0 -> 1: mudou de botao (dispara)
    REQUIRE(ui_hover_entered_new_item(0, 1) == true);
    // 1 -> -1: saiu pra fora de tudo (NAO dispara: so ENTRAR soa)
    REQUIRE(ui_hover_entered_new_item(1, -1) == false);
    // -1 -> -1: continua fora (NAO dispara)
    REQUIRE(ui_hover_entered_new_item(-1, -1) == false);
    // sair e VOLTAR pro mesmo botao (com -1 no meio) redispara
    REQUIRE(ui_hover_entered_new_item(2, -1) == false);  // saiu do 2
    REQUIRE(ui_hover_entered_new_item(-1, 2) == true);   // reentrou no 2: dispara
}

// PROVA "N hovers -> N plays, sem repique": simula uma varredura de mouse sobre 6
// botoes na SEQUENCIA 0,1,2,3,4,5 (6 entradas NOVAS) + 2 amostras PARADAS no ultimo
// botao (nao contam) + sair e voltar ao 0 (conta) = 7 disparos. Mesmo edge-detect que
// o cockpit usa headless (GUSWORLD_BATTLE_UI_SFX_SELFTEST) e o mouse real.
TEST_CASE("ui_hover: varredura por 6 botoes conta 1 disparo por ENTRADA nova",
          "[ui_hover]") {
    int prev = -1;
    int plays = 0;
    auto visit = [&](int cur) {
        if (ui_hover_entered_new_item(prev, cur)) ++plays;
        prev = cur;
    };
    for (int i = 0; i < 6; ++i) visit(i);  // 0..5: 6 entradas novas
    visit(5);                              // parado no 5: nao conta
    visit(5);                              // parado de novo: nao conta
    visit(-1);                             // saiu pra fora: nao conta
    visit(0);                              // reentrou no 0: conta
    REQUIRE(plays == 7);
}
