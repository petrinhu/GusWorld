// GusEngine/app/tests/npc_dialogue_loop_gl_test.cpp
//
// Catch2 (headless, SEM SDL_Init/janela/GL) de npc_dialogue_digit_for_key
// (DIALOGO-TERMINAL, opcoes clicaveis + numeradas - pedido do lider): MESMO
// racional de battle_key_routing_test.cpp - SDL_Keycode e so um typedef de
// inteiro (SDLK_* sao constantes de compilacao), usa-los aqui NAO exige
// SDL_Init nem janela. O resto de npc_dialogue_loop_gl.cpp e GL-heavy/sem
// unidade de teste direta (ver o header do .cpp) - esta funcao e a UNICA
// fatia pura exposta de la, MESMA razao de ser de battle_digit_for_key ser
// testada isoladamente em battle_key_routing_test.cpp.

#include <catch2/catch_test_macros.hpp>

#include "gus/app/screens/npc_dialogue_loop_gl.hpp"

using gus::app::screens::npc_dialogue_digit_for_key;

TEST_CASE("npc_dialogue_digit_for_key: fileira 1-9 mapeia pro digito correto",
          "[npc_dialogue_loop_gl]") {
    REQUIRE(npc_dialogue_digit_for_key(SDLK_1) == 1);
    REQUIRE(npc_dialogue_digit_for_key(SDLK_2) == 2);
    REQUIRE(npc_dialogue_digit_for_key(SDLK_3) == 3);
    REQUIRE(npc_dialogue_digit_for_key(SDLK_4) == 4);
    REQUIRE(npc_dialogue_digit_for_key(SDLK_5) == 5);
    REQUIRE(npc_dialogue_digit_for_key(SDLK_6) == 6);
    REQUIRE(npc_dialogue_digit_for_key(SDLK_7) == 7);
    REQUIRE(npc_dialogue_digit_for_key(SDLK_8) == 8);
    REQUIRE(npc_dialogue_digit_for_key(SDLK_9) == 9);
}

TEST_CASE("npc_dialogue_digit_for_key: numpad 1-9 mapeia pro MESMO digito da "
          "fileira (KP_1 == 1, etc)",
          "[npc_dialogue_loop_gl]") {
    REQUIRE(npc_dialogue_digit_for_key(SDLK_KP_1) == 1);
    REQUIRE(npc_dialogue_digit_for_key(SDLK_KP_2) == 2);
    REQUIRE(npc_dialogue_digit_for_key(SDLK_KP_3) == 3);
    REQUIRE(npc_dialogue_digit_for_key(SDLK_KP_9) == 9);
}

TEST_CASE("npc_dialogue_digit_for_key: tecla nao-numerica (ou SDLK_0, fora "
          "de 1-9) devolve 0 - guarda de 'nao e atalho de opcao'",
          "[npc_dialogue_loop_gl]") {
    REQUIRE(npc_dialogue_digit_for_key(SDLK_0) == 0);
    REQUIRE(npc_dialogue_digit_for_key(SDLK_KP_0) == 0);
    REQUIRE(npc_dialogue_digit_for_key(SDLK_UP) == 0);
    REQUIRE(npc_dialogue_digit_for_key(SDLK_RETURN) == 0);
    REQUIRE(npc_dialogue_digit_for_key(SDLK_ESCAPE) == 0);
    REQUIRE(npc_dialogue_digit_for_key(SDLK_A) == 0);
}
