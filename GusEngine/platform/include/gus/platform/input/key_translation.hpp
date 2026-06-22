// gus/platform/input/key_translation.hpp
//
// Traducao de keycode Qt (Qt::Key) -> keycode no esquema Godot Key enum (M1).
// Camada platform/ (fronteira Qt), mas o HEADER e limpo: recebe um int (o valor
// de Qt::Key ja extraido do QKeyEvent), sem incluir <Q...>, pra ser trivialmente
// testavel headless. A tabela vive no .cpp.
//
// POR QUE: o esquema de fabrica (domain/input/controls_restore.cpp) foi portado
// do Godot e usa os codigos do enum Godot Key (congelados nos saves V4). O Qt
// produz Qt::Key, que diverge para teclas nomeadas. Esta funcao casa os dois, sem
// tocar no mapa logico puro (que continua engine-agnostic). Cross-ref:
// engine-design.md secao 3 ("o mapa acao->tecla e puro; so o backend muda").

#ifndef GUS_PLATFORM_INPUT_KEY_TRANSLATION_HPP
#define GUS_PLATFORM_INPUT_KEY_TRANSLATION_HPP

namespace gus::platform::input {

// Converte um valor de Qt::Key (int) para o keycode equivalente no esquema Godot
// usado por default_controls(). Letras/numeros ASCII passam (Qt == ASCII ==
// Godot); letras minusculas sao normalizadas para maiuscula (o esquema usa
// maiuscula). Teclas nomeadas relevantes (setas, Shift, Enter/Return, Escape,
// Tab, Space) sao mapeadas explicitamente. Tecla fora da tabela e nao-ASCII ->
// 0 (sentinela "sem binding"). Pura, deterministica.
[[nodiscard]] long long qt_key_to_godot_keycode(int qt_key) noexcept;

}  // namespace gus::platform::input

#endif  // GUS_PLATFORM_INPUT_KEY_TRANSLATION_HPP
