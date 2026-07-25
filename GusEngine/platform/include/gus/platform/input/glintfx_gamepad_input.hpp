// gus/platform/input/glintfx_gamepad_input.hpp
//
// GlintfxGamepadInput (platform/input, F4-2.4): adaptador de GAMEPAD do backend novo
// (glintfx::Gamepads, evdev cru Linux) para o MESMO GamepadState que o jogo ja usa (ver
// gamepad_mapping.hpp) - irmao do GlintfxInput (adaptador de TECLADO, F4-2.3). Aqui NAO ha
// deteccao de borda pra fazer: button()/axis() do glintfx::Gamepads ja sao LEITURAS DE NIVEL
// (o mesmo "estado AGORA" que SdlInput::pump_events ja le do SDL_Gamepad a cada quadro),
// entao read() e uma traducao PURA sem estado interno - ao contrario do GlintfxInput::poll(),
// que precisa lembrar o "pressed anterior" de cada tecla pra sintetizar press()/release().
//
// SEAM (o ponto arquitetural desta fatia): traduz pro MESMO GamepadState que o SdlInput ja
// preenche hoje (platform/src/input/sdl_input.cpp, pump_events()) - gamepad_dx()/gamepad_dy()
// (gamepad_mapping.hpp/.cpp) continuam INTOCADOS, e o resto do gameplay (consumo de
// gamepad_dx/dy) nem sabe que existe um segundo backend. Quando o backend XInput/GameInput do
// glintfx sair pro Windows (item GAMEPAD-WINDOWS-POS-V1, ja pedido a eles), ele so precisa
// plugar no MESMO GamepadState - zero mudanca em gameplay.
//
// PROVIDER INJETAVEL (mesmo espirito da F4-2.3, GlintfxInput::KeyStateProvider): o CORE deste
// adaptador (o ctor de 3 providers) NAO depende de glintfx::Gamepads pra ser testado - so de
// 3 std::function sobre os enums PORTATEIS de gamepad_types.hpp (GamepadAxis/GamepadButton -
// header sem NENHUMA restricao de plataforma, ao contrario de glintfx/gamepad.hpp que tem o
// #error Linux-only). Isto permite testar toda a logica de conversao/mapeamento (deadzone,
// sinal de eixo, d-pad, run=East) com FAKES puros, sem device evdev, sem uinput, sem
// privilegio de /dev/input nenhum. O segundo ctor (const glintfx::Gamepads&) e so uma
// CONVENIENCIA que embrulha os 3 providers em cima de uma instancia real - usado pelo teste
// "sem dispositivo" (GamepadsConfig::dev_dir injetavel, headless) e pela integracao real
// futura (fora do escopo desta fatia - F4-2.5/F4-3, o amarrado com o App).
//
// FAIXA DE EIXO (ATENCAO - o ponto que mais gera bug de conversao): glintfx::Gamepads::axis()
// devolve float NORMALIZADO [-1,1] (ver glintfx/gamepad.hpp, doc-comment de axis() e
// docs/gamepad.md "sticks [-1,1]") - DIFERENTE da faixa Sint16 do SDL ([-32768,32767]) que
// GamepadState::stick_x/stick_y e kStickDeadzone (gamepad_mapping.hpp, calibrado em ~8000)
// foram desenhados pra usar. read() CONVERTE float[-1,1] -> int[-32767,32767] (a MESMA faixa
// simetrica que axis_sign()/kStickDeadzone ja assumem, ver to_stick_axis() no .cpp) ANTES de
// gravar em GamepadState - gamepad_dx()/gamepad_dy() funcionam SEM MUDANCA NENHUMA, nem no
// struct nem no consumo a jusante.
//
// DEADZONE: o glintfx NAO impoe deadzone nenhuma no proprio modulo (decisao documentada dos
// autores, docs/gamepad.md "Limits of this slice": "deadzone e decisao de design de jogo, nao
// do framework") - exatamente como o SdlInput hoje tambem nao aplica deadzone antes de gravar
// em GamepadState.stick_x/y. A deadzone continua sendo aplicada 1x so, no MESMO lugar de
// sempre (axis_sign()/kStickDeadzone, gamepad_mapping.cpp), depois da conversao de faixa
// acima - nao duplicada aqui.
//
// D-PAD: o glintfx JA normaliza o hat cru do evdev (ABS_HAT0X/Y) em 4 BOOLEANOS
// (GamepadButton::DpadUp/Down/Left/Right, ver gamepad_types.hpp) - este adaptador NAO precisa
// decodificar hat nenhum, so ler os 4 botoes normalizados direto (mais simples do que o
// SDL_GAMEPAD_BUTTON_DPAD_* que o SdlInput ja usa, mesma forma final).
//
// RUN = LESTE (East): preserva o MESMO botao fisico que SdlInput::pump_events ja mapeia
// (SDL_GAMEPAD_BUTTON_EAST, "B/Circle") - GamepadButton::East e a mesma tecla, so com o nome
// da nomenclatura kernel gamepad.rst (South/East/West/North) que o glintfx usa em vez do nome
// SDL_GameControllerButton. Preservado de proposito: um mutante que trocasse pra South (ou
// qualquer outro botao de face) tem que morrer no teste de run_button abaixo.
//
// LIFECYCLE do ctor real (const glintfx::Gamepads&): este adaptador NAO possui a instancia
// injetada por referencia (mesmo espirito do provider injetavel do GlintfxInput) -
// init()/poll()/shutdown() de glintfx::Gamepads sao responsabilidade do CHAMADOR (mesma nota
// "Coexistencia com o App" da doc do glintfx: Gamepads nao tem ordem de construcao amarrada a
// nada, e poll() vai no MESMO hook por-quadro que o resto do input). O CHAMADOR roda
// pads.poll() 1x por quadro (drena o evdev), e SO ENTAO chama GlintfxGamepadInput::read() pra
// traduzir o estado ja decodificado.

#ifndef GUS_PLATFORM_INPUT_GLINTFX_GAMEPAD_INPUT_HPP
#define GUS_PLATFORM_INPUT_GLINTFX_GAMEPAD_INPUT_HPP

#include <functional>

#include <glintfx/gamepad.hpp>        // glintfx::Gamepads (ctor de conveniencia, Linux-only)
#include <glintfx/gamepad_types.hpp>  // glintfx::GamepadAxis/GamepadButton (portateis)

#include "gus/platform/input/gamepad_mapping.hpp"

namespace gus::platform::input {

class GlintfxGamepadInput {
public:
    // Providers de estado do gamepad: devolvem o valor AGORA. Chamados a cada read(), nunca
    // guardados como side effect por este adaptador - mesma disciplina do
    // GlintfxInput::KeyStateProvider.
    using ConnectedProvider = std::function<bool()>;
    using AxisProvider = std::function<float(glintfx::GamepadAxis)>;    // [-1,1] sticks
    using ButtonProvider = std::function<bool(glintfx::GamepadButton)>;

    // Ctor de INJECAO PURA (headless, sem device evdev/uinput nenhum) - usado pelos testes de
    // logica de conversao/mapeamento (deadzone, sinal, d-pad, run=East) via fakes.
    GlintfxGamepadInput(ConnectedProvider connected, AxisProvider axis, ButtonProvider button) noexcept;

    // Ctor de CONVENIENCIA: embrulha uma glintfx::Gamepads JA CONSTRUIDA (e, no uso real, ja
    // init()'d/poll()'d pelo CHAMADOR - ver LIFECYCLE no cabecalho do arquivo) nos 3 providers
    // acima. pads e capturada POR REFERENCIA dentro dos providers (std::function com lambda
    // que captura `&pads`) - a instancia PRECISA sobreviver a este GlintfxGamepadInput, mesmo
    // contrato de referencia externa que o provider do GlintfxInput ja assume. pad_index: slot
    // do glintfx (M1: 1 jogador, 1 gamepad -> 0, mesmo "so o primeiro pad" que
    // SdlInput::open_gamepads ja usa).
    explicit GlintfxGamepadInput(const glintfx::Gamepads& pads, int pad_index = 0) noexcept;

    // Traduz o estado ATUAL (via os providers) pro MESMO GamepadState que SdlInput::
    // pump_events ja preenche - traducao PURA, sem estado proprio. Pode ser chamado quantas
    // vezes o chamador quiser, sempre devolvendo o mesmo resultado (idempotente) ate os
    // providers mudarem de resposta. Gamepad desconectado (connected() == false): GamepadState{}
    // zerado, SEM consultar os providers de eixo/botao - mesmo fail-high de "sem crash" que o
    // resto do glintfx ja segue.
    [[nodiscard]] GamepadState read() const noexcept;

private:
    ConnectedProvider connected_;
    AxisProvider axis_;
    ButtonProvider button_;
};

}  // namespace gus::platform::input

#endif  // GUS_PLATFORM_INPUT_GLINTFX_GAMEPAD_INPUT_HPP
