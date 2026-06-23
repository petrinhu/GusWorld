// gus/app/src/sdl_window.cpp
//
// Implementacao da SdlWindow - casca SDL de janela + loop PROPRIO + input. Ver
// header. Caminho irredutivel (janela/renderer/loop): coberto pelo smoke headless
// do main (--smoke, SDL_VIDEODRIVER=dummy). A regra de jogo (OverworldSim) e o
// renderer (Render2dSdl) sao testados a parte.

#include "gus/app/sdl_window.hpp"

#include "gus/app/screens/player_sprites_loader.hpp"
#include "gus/app/screens/test_overworld.hpp"

namespace gus::app {

namespace {
constexpr int kWindowW = 1280;
constexpr int kWindowH = 720;
}  // namespace

SdlWindow::SdlWindow() : clock_(1.0 / 60.0, 5) {
    // Tuning unico da cena (velocidade + corner-assist + ganchos). O lider ajusta
    // em test_overworld.hpp / overworld_tuning.hpp, sem tocar aqui.
    sim_ = std::make_unique<gus::app::screens::OverworldSim>(
        gus::app::screens::make_test_map(),
        gus::app::screens::kTestPlayerStart,
        gus::app::screens::make_test_tuning());
}

SdlWindow::~SdlWindow() {
    render2d_.reset();
    if (renderer_ != nullptr) {
        SDL_DestroyRenderer(renderer_);
    }
    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
    }
}

bool SdlWindow::init() {
    // Cria janela + renderer num passo (SDL3 helper). vsync ligado por padrao
    // (suave; o lider pode decidir desligar - ver relatorio).
    if (!SDL_CreateWindowAndRenderer("GusWorld", kWindowW, kWindowH,
                                     SDL_WINDOW_RESIZABLE, &window_, &renderer_)) {
        SDL_Log("SDL_CreateWindowAndRenderer falhou: %s", SDL_GetError());
        return false;
    }
    SDL_SetRenderVSync(renderer_, 1);  // 1 = sincroniza com o refresh

    render2d_ =
        std::make_unique<gus::platform::render2d::Render2dSdl>(renderer_);

    // Abre gamepad ja conectado + arma hot-plug.
    input_.open_gamepads();

    // Carrega os sprites do Caua e os entrega ao sim (handles resolvidos pelo
    // renderer). Se faltar arquivo, o set fica incompleto e o sim cai pro contorno.
    const std::string assets = gus::app::screens::resolve_caua_sprites_dir();
    sim_->set_player_sprites(
        gus::app::screens::load_caua_sprites(*render2d_, assets));
    return true;
}

void SdlWindow::run() {
    bool running = true;
    while (running) {
        // 1) INPUT: drena os eventos SDL (teclado + gamepad). false = fechar.
        running = input_.pump_events();
        if (!running) {
            break;
        }

        // 2) dt real desde o ultimo frame (segundos), via relogio monotonico do SDL.
        const unsigned long long now_ns = SDL_GetTicksNS();
        double dt = 0.0;
        if (have_last_time_) {
            dt = static_cast<double>(now_ns - last_ns_) / 1.0e9;
        }
        have_last_time_ = true;
        last_ns_ = now_ns;

        // 3) UPDATE: N passos fixos (FixedTimestep) com a intencao cardinal fundida.
        const gus::core::time::FrameSteps steps = clock_.advance(dt);
        const int dx = input_.dx();
        const int dy = input_.dy();
        const bool run = input_.run();
        for (int i = 0; i < steps.ticks; ++i) {
            sim_->step_fixed(dx, dy, run, static_cast<float>(clock_.fixed_dt()));
        }

        // 4) RENDER: 1 frame, interpolado pelo alpha residual. Viewport em PIXELS.
        int pw = kWindowW, ph = kWindowH;
        SDL_GetCurrentRenderOutputSize(renderer_, &pw, &ph);
        sim_->render(*render2d_, static_cast<float>(pw), static_cast<float>(ph),
                     static_cast<float>(steps.alpha));
    }
}

}  // namespace gus::app
