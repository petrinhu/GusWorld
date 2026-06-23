// GusEngine/platform/tests/input_mapper_test.cpp
//
// Catch2 do InputMapper (platform/input): estado de teclas pressionadas -> acoes
// logicas ativas -> vetor de movimento cardinal. TEST-FIRST. Headless (POCO): nao
// abre janela; alimenta keycodes (ja no esquema Godot) e consulta o resultado.
//
// O InputMapper consome um InputRemapConfig (de default_controls(), o esquema de
// fabrica puro). E a ponte: o app traduz SDL_Keycode -> keycode Godot (via
// key_translation) e chama press/release; o mapper diz quais ACOES estao ativas.
// A regra de qual tecla e qual acao continua no dado puro (domain/), nao no codigo.
//
// CONVENCAO DE EIXO (segue tile_grid.hpp: +X direita, +Y BAIXO, top-down):
//   move_left -> dx=-1   move_right -> dx=+1
//   move_forward (W, "cima") -> dy=-1   move_backward (S, "baixo") -> dy=+1
// O vetor e CARDINAL CRU; a normalizacao da diagonal (feel) nao e do mapper.

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/input/controls_restore.hpp"
#include "gus/platform/input/input_mapper.hpp"

using gus::domain::input::default_controls;
using gus::platform::input::InputMapper;

namespace {
// Keycodes Godot (esquema de fabrica) das teclas exercitadas.
constexpr long long kW = 'W';
constexpr long long kA = 'A';
constexpr long long kS = 'S';
constexpr long long kD = 'D';
constexpr long long kShift = 4194325;
}  // namespace

TEST_CASE("InputMapper: inicial sem teclas nao move", "[input_mapper]") {
    InputMapper m(default_controls());
    REQUIRE(m.movement_dx() == 0);
    REQUIRE(m.movement_dy() == 0);
    REQUIRE_FALSE(m.is_action_active("move_right"));
    REQUIRE_FALSE(m.run_active());
}

TEST_CASE("InputMapper: D anda para a direita", "[input_mapper]") {
    InputMapper m(default_controls());
    m.press(kD);
    REQUIRE(m.is_action_active("move_right"));
    REQUIRE(m.movement_dx() == 1);
    REQUIRE(m.movement_dy() == 0);
}

TEST_CASE("InputMapper: A anda para a esquerda", "[input_mapper]") {
    InputMapper m(default_controls());
    m.press(kA);
    REQUIRE(m.movement_dx() == -1);
}

TEST_CASE("InputMapper: W anda para cima (Y negativo)", "[input_mapper]") {
    InputMapper m(default_controls());
    m.press(kW);
    REQUIRE(m.is_action_active("move_forward"));
    REQUIRE(m.movement_dy() == -1);
}

TEST_CASE("InputMapper: S anda para baixo (Y positivo)", "[input_mapper]") {
    InputMapper m(default_controls());
    m.press(kS);
    REQUIRE(m.movement_dy() == 1);
}

TEST_CASE("InputMapper: diagonal soma cardinal cru", "[input_mapper]") {
    InputMapper m(default_controls());
    m.press(kW);
    m.press(kD);
    REQUIRE(m.movement_dx() == 1);
    REQUIRE(m.movement_dy() == -1);
}

TEST_CASE("InputMapper: teclas opostas cancelam", "[input_mapper]") {
    InputMapper m(default_controls());
    m.press(kA);
    m.press(kD);
    REQUIRE(m.movement_dx() == 0);
}

TEST_CASE("InputMapper: release para o movimento", "[input_mapper]") {
    InputMapper m(default_controls());
    m.press(kD);
    REQUIRE(m.movement_dx() == 1);
    m.release(kD);
    REQUIRE(m.movement_dx() == 0);
    REQUIRE_FALSE(m.is_action_active("move_right"));
}

TEST_CASE("InputMapper: Shift ativa run", "[input_mapper]") {
    InputMapper m(default_controls());
    m.press(kShift);
    REQUIRE(m.run_active());
    m.release(kShift);
    REQUIRE_FALSE(m.run_active());
}

TEST_CASE("InputMapper: tecla sem binding nao afeta movimento", "[input_mapper]") {
    InputMapper m(default_controls());
    m.press(0);
    m.press('K');  // K nao e movimento no esquema de fabrica
    REQUIRE(m.movement_dx() == 0);
    REQUIRE(m.movement_dy() == 0);
}

TEST_CASE("InputMapper: press repetido e idempotente", "[input_mapper]") {
    InputMapper m(default_controls());
    m.press(kD);
    m.press(kD);
    m.press(kD);
    REQUIRE(m.movement_dx() == 1);
    m.release(kD);  // um release zera (conjunto, nao contador)
    REQUIRE(m.movement_dx() == 0);
}

TEST_CASE("InputMapper: clear esvazia tudo", "[input_mapper]") {
    InputMapper m(default_controls());
    m.press(kW);
    m.press(kD);
    m.press(kShift);
    m.clear();
    REQUIRE(m.movement_dx() == 0);
    REQUIRE(m.movement_dy() == 0);
    REQUIRE_FALSE(m.run_active());
}
