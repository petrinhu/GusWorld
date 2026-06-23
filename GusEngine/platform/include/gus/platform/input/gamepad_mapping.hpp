// gus/platform/input/gamepad_mapping.hpp
//
// LOGICA pura de mapeamento de gamepad -> intencao cardinal (dx,dy em {-1,0,1}),
// POCO sem SDL (pos repivot ADR-008: o SDL trouxe gamepad nativo de classe-AAA que
// o Qt6 nao tinha). O SdlInput preenche um GamepadState a partir dos SDL_Event /
// SDL_GetGamepadAxis; estas funcoes convertem esse estado em intencao cardinal -
// o MESMO contrato (dx,dy) do teclado, pra alimentar o overworld igual.
//
// HEADER limpo (sem <SDL...>): so int/bool. Testavel headless sem hardware.
//
// DEADZONE: o stick analogico parado nunca fica exatamente em 0 (ruido). Abaixo do
// limiar, o eixo conta como neutro. Acima, vira +-1 (digital). O d-pad e digital
// puro. D-pad e stick somam e clampam em {-1,0,1} (mesmo sentido nao vira 2;
// opostos cancelam).

#ifndef GUS_PLATFORM_INPUT_GAMEPAD_MAPPING_HPP
#define GUS_PLATFORM_INPUT_GAMEPAD_MAPPING_HPP

namespace gus::platform::input {

// Limiar de deadzone do stick analogico (faixa SDL Sint16 [-32768, 32767]). ~25%
// do curso. Abaixo disso o eixo conta como neutro (ruido do stick parado).
inline constexpr int kStickDeadzone = 8000;

// Estado cru do gamepad num frame (preenchido pelo SdlInput a partir do SDL). O
// stick esquerdo move; o d-pad move (digital); botoes/triggers de acao entram
// quando houver acao (fora do M1). run e tratado a parte (botao mapeado).
struct GamepadState {
    int stick_x = 0;       // eixo X do stick esquerdo (SDL Sint16; +direita)
    int stick_y = 0;       // eixo Y do stick esquerdo (SDL Sint16; +baixo)
    bool dpad_left = false;
    bool dpad_right = false;
    bool dpad_up = false;
    bool dpad_down = false;
    bool run_button = false;  // botao de correr (ex.: B/Circle) segurado
    bool connected = false;   // ha gamepad ativo?
};

// Intencao horizontal do gamepad: -1 esquerda, +1 direita, 0 neutro. D-pad + stick
// (com deadzone) somados e clampados; opostos cancelam.
[[nodiscard]] int gamepad_dx(const GamepadState& g) noexcept;

// Intencao vertical: -1 cima (frente), +1 baixo. SDL e mundo usam +Y baixo, entao
// o sinal do eixo passa direto. D-pad + stick (com deadzone), clampado.
[[nodiscard]] int gamepad_dy(const GamepadState& g) noexcept;

}  // namespace gus::platform::input

#endif  // GUS_PLATFORM_INPUT_GAMEPAD_MAPPING_HPP
