// gus/platform/input/input_mapper.hpp
//
// InputMapper (M1, platform/input): mantem o conjunto de teclas pressionadas e
// resolve quais ACOES estao ativas, consultando um InputRemapConfig (o mapa
// logico puro de domain/). E a ponte entre o backend de evento Qt e o dado puro.
//
// FLUXO: o app traduz QKeyEvent -> keycode Godot (key_translation) e chama
// press()/release(); por frame, le movement_dx()/movement_dy()/run_active() para
// montar o deslocamento. A regra "qual tecla -> qual acao" vive no InputRemapConfig
// (dado, nao codigo), entao remap futuro nao mexe aqui.
//
// HEADER limpo (sem <Q...>): so depende de domain/ (POCO) e da STL. Testavel
// headless. Inclui input_binding.hpp por valor (InputRemapConfig).
//
// ESCOPO M1 (decisao tecnica documentada): o casamento de tecla->acao considera
// apenas o KEYCODE (nao os modifiers Ctrl/Shift/Alt do KeyBinding). Suficiente e
// correto para o movimento do overworld, cujos bindings de fabrica (WASD, setas,
// Shift de correr) NAO usam modifier. Acoes com modifier (combos tipo Ctrl+algo)
// pedem um casamento sensivel a modifier; isso entra quando uma acao assim existir
// (fora do M1). Cross-ref: engine-design.md secao 3.

#ifndef GUS_PLATFORM_INPUT_INPUT_MAPPER_HPP
#define GUS_PLATFORM_INPUT_INPUT_MAPPER_HPP

#include <string_view>
#include <unordered_set>
#include <vector>

#include "gus/domain/input/input_binding.hpp"

namespace gus::platform::input {

class InputMapper {
public:
    // Constroi a partir do mapa logico ativo (ex.: default_controls()). Copia o
    // config (barato, vetor de structs) e pre-indexa os keycodes das acoes de
    // movimento para consulta O(1) por frame.
    explicit InputMapper(gus::domain::input::InputRemapConfig config);

    // Registra/limpa uma tecla (keycode JA no esquema Godot, vindo de
    // key_translation). press de keycode 0 (sentinela "sem binding") e no-op.
    // Idempotente: e um conjunto, nao um contador (auto-repeat do SO nao duplica;
    // um release zera a tecla).
    void press(long long keycode);
    void release(long long keycode);

    // Esvazia o estado (ex.: ao perder foco da janela, pra nao "grudar" tecla).
    void clear() noexcept;

    // true se ALGUMA tecla bound a esta acao esta pressionada (casamento por
    // keycode; ver nota de escopo no topo).
    [[nodiscard]] bool is_action_active(std::string_view action_name) const;

    // Componentes cardinais cruos do movimento (cada um em {-1,0,1}); teclas
    // opostas se cancelam. +X direita, +Y BAIXO (tile_grid.hpp). A normalizacao
    // da diagonal (feel) e do chamador (RF-3), nao daqui.
    [[nodiscard]] int movement_dx() const;
    [[nodiscard]] int movement_dy() const;

    // Atalho: move_run (Shift no esquema de fabrica) ativo.
    [[nodiscard]] bool run_active() const;

private:
    // Keycodes que disparam uma acao (vazio se a acao nao existe no config).
    [[nodiscard]] std::vector<long long> keycodes_for(std::string_view action_name) const;
    [[nodiscard]] bool any_pressed(const std::vector<long long>& codes) const;

    gus::domain::input::InputRemapConfig config_;
    std::unordered_set<long long> pressed_;

    // Indices pre-resolvidos das 5 acoes de movimento (cache; recalculado no ctor).
    std::vector<long long> left_;
    std::vector<long long> right_;
    std::vector<long long> up_;     // move_forward
    std::vector<long long> down_;   // move_backward
    std::vector<long long> run_;
};

}  // namespace gus::platform::input

#endif  // GUS_PLATFORM_INPUT_INPUT_MAPPER_HPP
