// gus/app/sdl_window.hpp
//
// SdlWindow: a CASCA SDL (janela + loop PROPRIO + bombeamento de input). Vive em
// app/ por ser o SHELL DE APLICACAO (cria janela/renderer, possui o loop de frame),
// nao um modulo de engine reutilizavel - o renderer reutilizavel (Render2dSdl atras
// de IRenderer) e a regra de jogo (OverworldSim) estao FORA daqui. Mantida fina:
// nenhuma regra de jogo, so orquestracao SDL.
//
// POS REPIVOT ADR-008: aqui esta a diferenca-chave do Qt - NOS possuimos o loop.
// Nao ha event loop de framework para brigar: o FixedTimestep (core/time, POCO)
// dirige o ritmo (poll de eventos SDL -> N updates fixos -> 1 render -> repeat),
// com SDL_GetTicksNS medindo o dt real. Isso casa exatamente com o modelo de loop
// que o engine-design pediu.
//
// Inclui <SDL3/SDL.h> (camada app/, SDL permitido). O irredutivel (criar janela/
// renderer, apresentar) e coberto pelo smoke headless do main (--smoke com
// SDL_VIDEODRIVER=dummy).

#ifndef GUS_APP_SDL_WINDOW_HPP
#define GUS_APP_SDL_WINDOW_HPP

#include <memory>

#include <SDL3/SDL.h>

#include "gus/app/screens/overworld_sim.hpp"
#include "gus/core/time/fixed_timestep.hpp"
#include "gus/platform/input/sdl_input.hpp"
#include "gus/platform/render2d/render2d_sdl.hpp"

namespace gus::app {

class SdlWindow {
public:
    SdlWindow();
    ~SdlWindow();

    SdlWindow(const SdlWindow&) = delete;
    SdlWindow& operator=(const SdlWindow&) = delete;

    // Cria janela + renderer SDL + carrega os sprites do Caua. Devolve false se o
    // SDL/janela/renderer falharem (o main reporta e sai != 0).
    [[nodiscard]] bool init();

    // Roda o loop ate o usuario fechar a janela (pump_events devolver false). Cada
    // iteracao: poll de input -> N updates fixos (FixedTimestep) -> 1 render. O
    // lider joga: move o Caua com WASD/setas/gamepad, desliza nas paredes, camera
    // presa ao mapa, animacao de walk por distancia.
    void run();

private:
    SDL_Window* window_ = nullptr;      // owner
    SDL_Renderer* renderer_ = nullptr;  // owner

    std::unique_ptr<gus::platform::render2d::Render2dSdl> render2d_;
    std::unique_ptr<gus::app::screens::OverworldSim> sim_;
    gus::platform::input::SdlInput input_;
    gus::core::time::FixedTimestep clock_;
    bool have_last_time_ = false;
    unsigned long long last_ns_ = 0;
};

}  // namespace gus::app

#endif  // GUS_APP_SDL_WINDOW_HPP
