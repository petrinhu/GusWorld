// SPDX-License-Identifier: GPL-3.0-or-later
// GusEngine/platform/tests/glintfx_gamepad_input_test.cpp
//
// Catch2 do GlintfxGamepadInput (platform/input, F4-2.4): adaptador de GAMEPAD do backend
// novo (glintfx::Gamepads, evdev cru Linux) para o MESMO GamepadState que o jogo ja usa.
// TEST-FIRST.
//
// PROVIDERS FALSOS (FakePad, abaixo): headless de verdade - nenhum device evdev, nenhum
// uinput, nenhum privilegio de /dev/input. O teste dirige o "hardware" via
// FakePad::set_axis()/set_button()/set_connected() e so entao chama
// GlintfxGamepadInput::read() (button()/axis() do glintfx.Gamepads ja sao leituras de nivel,
// entao NAO ha edge/poll pra sintetizar aqui, ao contrario do GlintfxInput por teclado).
//
// Este arquivo cobre (ver o pedido da fatia, INBOX F4-2.4):
//   1. Eixos -> dx/dy: stick em cada direcao produz o GamepadState/gamepad_dx/dy corretos.
//   2. Deadzone: valor logo ABAIXO do limite nao move; logo ACIMA move (pega erro de
//      conversao de escala float[-1,1] -> int[-32767,32767]).
//   3. D-pad -> direcao (botoes normalizados direto, sem hat pra decodificar).
//   4. Botao run = East (mutante que trocasse pra South tem que morrer).
//   5. Sem dispositivo: Gamepads real com dev_dir vazio (headless, sem hardware) da zero
//      pads e nao crasha - usa o SEGUNDO ctor (const glintfx::Gamepads&).
//   6. Sinal dos eixos: um teste que morre se alguem inverter stick_y (evdev/SDL podem ter
//      convencoes de sinal diferentes no eixo Y - aqui NAO tem, glintfx usa o mesmo
//      ABS_Y positivo-pra-baixo do kernel que o SDL ja usa, mas o teste prova isso).

#include <filesystem>
#include <map>
#include <system_error>

#include <catch2/catch_test_macros.hpp>

#include <glintfx/gamepad.hpp>

#include "gus/platform/input/gamepad_mapping.hpp"
#include "gus/platform/input/glintfx_gamepad_input.hpp"

using gus::platform::input::GamepadState;
using gus::platform::input::gamepad_dx;
using gus::platform::input::gamepad_dy;
using gus::platform::input::GlintfxGamepadInput;
using Axis = glintfx::GamepadAxis;
using Button = glintfx::GamepadButton;

namespace {

// FakePad: providers dirigidos manualmente pelo teste - sem device evdev/uinput nenhum.
// axis_ comeca em 0.0f (stick centrado) pra cada eixo nao setado explicitamente; button_
// comeca false; connected_ comeca true (a maioria dos testes quer um pad "ligado" pronto
// pra ler eixo/botao - o caso "sem dispositivo" tem seu proprio TEST_CASE via Gamepads real).
class FakePad {
public:
    void set_connected(bool connected) { connected_ = connected; }
    void set_axis(Axis a, float value) { axis_[a] = value; }
    void set_button(Button b, bool pressed) { button_[b] = pressed; }

    [[nodiscard]] GlintfxGamepadInput::ConnectedProvider connected_provider() {
        return [this] { return connected_; };
    }
    [[nodiscard]] GlintfxGamepadInput::AxisProvider axis_provider() {
        return [this](Axis a) {
            const auto it = axis_.find(a);
            return it == axis_.end() ? 0.0f : it->second;
        };
    }
    [[nodiscard]] GlintfxGamepadInput::ButtonProvider button_provider() {
        return [this](Button b) {
            const auto it = button_.find(b);
            return it == button_.end() ? false : it->second;
        };
    }

    [[nodiscard]] GlintfxGamepadInput make_input() {
        return GlintfxGamepadInput(connected_provider(), axis_provider(), button_provider());
    }

private:
    bool connected_ = true;
    std::map<Axis, float> axis_;
    std::map<Button, bool> button_;
};

}  // namespace

// --- 1. Eixos -> dx/dy -------------------------------------------------------------------

TEST_CASE("GlintfxGamepadInput: stick parado (0.0f) nao move", "[glintfx_gamepad]") {
    FakePad pad;
    GlintfxGamepadInput in = pad.make_input();
    const GamepadState state = in.read();
    REQUIRE(state.connected == true);
    REQUIRE(gamepad_dx(state) == 0);
    REQUIRE(gamepad_dy(state) == 0);
}

TEST_CASE("GlintfxGamepadInput: stick para a direita (LeftX=1.0f) move +X",
          "[glintfx_gamepad]") {
    FakePad pad;
    pad.set_axis(Axis::LeftX, 1.0f);
    GlintfxGamepadInput in = pad.make_input();
    const GamepadState state = in.read();
    REQUIRE(gamepad_dx(state) == 1);
    REQUIRE(gamepad_dy(state) == 0);
}

TEST_CASE("GlintfxGamepadInput: stick para a esquerda (LeftX=-1.0f) move -X",
          "[glintfx_gamepad]") {
    FakePad pad;
    pad.set_axis(Axis::LeftX, -1.0f);
    GlintfxGamepadInput in = pad.make_input();
    const GamepadState state = in.read();
    REQUIRE(gamepad_dx(state) == -1);
}

// --- 6. Sinal dos eixos (Y) --------------------------------------------------------------
// glintfx usa a MESMA convencao ABS_Y do kernel/SDL: positivo = baixo. Um mutante que
// inverta o sinal na conversao (stick_y = -to_stick_axis(...)) morre nos dois REQUIRE
// abaixo - cima tem que dar dy=-1 (frente), baixo tem que dar dy=+1.
TEST_CASE("GlintfxGamepadInput: stick para cima (LeftY=-1.0f) move -Y (frente)",
          "[glintfx_gamepad][sinal]") {
    FakePad pad;
    pad.set_axis(Axis::LeftY, -1.0f);
    GlintfxGamepadInput in = pad.make_input();
    const GamepadState state = in.read();
    REQUIRE(gamepad_dy(state) == -1);
}

TEST_CASE("GlintfxGamepadInput: stick para baixo (LeftY=1.0f) move +Y",
          "[glintfx_gamepad][sinal]") {
    FakePad pad;
    pad.set_axis(Axis::LeftY, 1.0f);
    GlintfxGamepadInput in = pad.make_input();
    const GamepadState state = in.read();
    REQUIRE(gamepad_dy(state) == 1);
}

// --- 2. Deadzone (pega erro de conversao de escala) ---------------------------------------
// kStickDeadzone = 8000 na faixa int [-32768,32767] (gamepad_mapping.hpp). 0.24f normalizado
// converte pra ~7864 (ABAIXO do limite - nao move); 0.26f converte pra ~8519 (ACIMA - move).
// Se a conversao de escala estivesse ERRADA (ex.: esquecendo o *32767 e gravando o float
// truncado direto num int), 0.26f viraria 0 (int(0.26f) == 0) e este teste FALHARIA -
// exatamente o "erro de conversao de escala" que o pedido da fatia avisa.
TEST_CASE("GlintfxGamepadInput: ruido logo ABAIXO da deadzone nao move",
          "[glintfx_gamepad][deadzone]") {
    FakePad pad;
    pad.set_axis(Axis::LeftX, 0.24f);
    pad.set_axis(Axis::LeftY, -0.24f);
    GlintfxGamepadInput in = pad.make_input();
    const GamepadState state = in.read();
    REQUIRE(gamepad_dx(state) == 0);
    REQUIRE(gamepad_dy(state) == 0);
}

TEST_CASE("GlintfxGamepadInput: valor logo ACIMA da deadzone move",
          "[glintfx_gamepad][deadzone]") {
    FakePad pad;
    pad.set_axis(Axis::LeftX, 0.26f);
    GlintfxGamepadInput in = pad.make_input();
    const GamepadState state = in.read();
    REQUIRE(gamepad_dx(state) == 1);
}

// --- 3. D-pad -> direcao -------------------------------------------------------------------
// glintfx JA normaliza o hat cru (ABS_HAT0X/Y) em 4 booleanos - sem hat pra decodificar aqui.

TEST_CASE("GlintfxGamepadInput: d-pad esquerda move -X", "[glintfx_gamepad][dpad]") {
    FakePad pad;
    pad.set_button(Button::DpadLeft, true);
    GlintfxGamepadInput in = pad.make_input();
    const GamepadState state = in.read();
    REQUIRE(state.dpad_left == true);
    REQUIRE(state.dpad_right == false);
    REQUIRE(gamepad_dx(state) == -1);
}

TEST_CASE("GlintfxGamepadInput: d-pad direita+esquerda cancelam (mesmo teste que o SDL)",
          "[glintfx_gamepad][dpad]") {
    FakePad pad;
    pad.set_button(Button::DpadLeft, true);
    pad.set_button(Button::DpadRight, true);
    GlintfxGamepadInput in = pad.make_input();
    const GamepadState state = in.read();
    REQUIRE(gamepad_dx(state) == 0);
}

TEST_CASE("GlintfxGamepadInput: d-pad cima/baixo mapeiam pros campos corretos (sem swap)",
          "[glintfx_gamepad][dpad]") {
    FakePad pad;
    pad.set_button(Button::DpadUp, true);
    GlintfxGamepadInput in = pad.make_input();
    const GamepadState state_up = in.read();
    REQUIRE(state_up.dpad_up == true);
    REQUIRE(state_up.dpad_down == false);
    REQUIRE(gamepad_dy(state_up) == -1);

    FakePad pad2;
    pad2.set_button(Button::DpadDown, true);
    GlintfxGamepadInput in2 = pad2.make_input();
    const GamepadState state_down = in2.read();
    REQUIRE(state_down.dpad_down == true);
    REQUIRE(gamepad_dy(state_down) == 1);
}

// --- 4. Botao run = East (preserva o mapeamento do SdlInput) ------------------------------

TEST_CASE("GlintfxGamepadInput: botao East ativa run_button", "[glintfx_gamepad][run]") {
    FakePad pad;
    pad.set_button(Button::East, true);
    GlintfxGamepadInput in = pad.make_input();
    const GamepadState state = in.read();
    REQUIRE(state.run_button == true);
}

// Mutante-killer: South pressionado (SEM East) NAO pode ativar run_button - um sabotador
// que trocasse East por South no adaptador passaria no teste anterior "por coincidencia" se
// este aqui nao existisse.
TEST_CASE("GlintfxGamepadInput: botao South (sem East) NAO ativa run_button",
          "[glintfx_gamepad][run]") {
    FakePad pad;
    pad.set_button(Button::South, true);
    GlintfxGamepadInput in = pad.make_input();
    const GamepadState state = in.read();
    REQUIRE(state.run_button == false);
}

// --- desconectado: zera tudo, nunca consulta os providers de eixo/botao -------------------

TEST_CASE("GlintfxGamepadInput: desconectado devolve GamepadState zerado mesmo com "
          "providers de eixo/botao 'sujos'",
          "[glintfx_gamepad]") {
    FakePad pad;
    pad.set_connected(false);
    pad.set_axis(Axis::LeftX, 1.0f);
    pad.set_button(Button::East, true);
    GlintfxGamepadInput in = pad.make_input();
    const GamepadState state = in.read();
    REQUIRE(state.connected == false);
    REQUIRE(state.stick_x == 0);
    REQUIRE(state.run_button == false);
    REQUIRE(gamepad_dx(state) == 0);
}

// --- 5. Sem dispositivo (Gamepads REAL, dev_dir vazio, headless) --------------------------
// Espelha gamepad_device_sanity.cpp do proprio glintfx (dev_dir injetavel pra teste, sem
// tocar /dev/input real nenhum) - usa o SEGUNDO ctor de GlintfxGamepadInput (const
// glintfx::Gamepads&), a conveniencia de producao, com uma Gamepads real inicializada num
// diretorio temporario VAZIO.

TEST_CASE("GlintfxGamepadInput(const glintfx::Gamepads&): dir vazio (sem device) da "
          "GamepadState zerado, sem crash",
          "[glintfx_gamepad][sem-device]") {
    namespace fs = std::filesystem;
    const fs::path dir = fs::temp_directory_path() / "glintfx-gamepad-input-test-empty";
    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);
    REQUIRE(!ec);

    glintfx::Gamepads pads;
    glintfx::GamepadsConfig cfg;
    const std::string dir_str = dir.string();
    cfg.dev_dir = dir_str.c_str();
    cfg.hotplug = false;  // este teste e so varredura (mesmo padrao do device_sanity deles)

    REQUIRE(pads.init(cfg));
    REQUIRE(pads.connected_count() == 0);
    pads.poll();  // nao pode crashar mesmo com zero pads

    GlintfxGamepadInput in(pads, /*pad_index=*/0);
    const GamepadState state = in.read();
    REQUIRE(state.connected == false);
    REQUIRE(state.stick_x == 0);
    REQUIRE(state.stick_y == 0);
    REQUIRE(state.dpad_left == false);
    REQUIRE(state.run_button == false);
    REQUIRE(gamepad_dx(state) == 0);
    REQUIRE(gamepad_dy(state) == 0);

    pads.shutdown();

    fs::remove_all(dir, ec);
}

// Nao-regressao: pad_index fora de faixa (kMaxPads=8) tambem tem que devolver zerado, sem
// crash - mesmo fail-high do glintfx::Gamepads::connected() pra indice invalido.
TEST_CASE("GlintfxGamepadInput(const glintfx::Gamepads&): pad_index fora de faixa nao crasha",
          "[glintfx_gamepad][sem-device]") {
    namespace fs = std::filesystem;
    const fs::path dir = fs::temp_directory_path() / "glintfx-gamepad-input-test-oob";
    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);
    REQUIRE(!ec);

    glintfx::Gamepads pads;
    glintfx::GamepadsConfig cfg;
    const std::string dir_str = dir.string();
    cfg.dev_dir = dir_str.c_str();
    cfg.hotplug = false;

    REQUIRE(pads.init(cfg));

    GlintfxGamepadInput in(pads, /*pad_index=*/glintfx::Gamepads::kMaxPads + 1);
    const GamepadState state = in.read();
    REQUIRE(state.connected == false);

    pads.shutdown();
    fs::remove_all(dir, ec);
}
