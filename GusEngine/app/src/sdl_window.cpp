// gus/app/src/sdl_window.cpp
//
// Implementacao da SdlWindow - casca SDL de janela + loop PROPRIO + input. Ver
// header. Caminho irredutivel (janela/renderer/loop): coberto pelo smoke headless
// do main (--smoke, SDL_VIDEODRIVER=dummy). A regra de jogo (OverworldSim) e o
// renderer (Render2dSdl) sao testados a parte.

#include "gus/app/sdl_window.hpp"

#include "gus/app/screens/anim_catalog.hpp"  // resolve_gus_sprites_dir
#include "gus/app/screens/city_loader.hpp"   // load_city_or_fallback
#include "gus/app/screens/player_sprites_loader.hpp"

namespace gus::app {

namespace {
constexpr int kWindowW = 1280;
constexpr int kWindowH = 720;
}  // namespace

SdlWindow::SdlWindow() : clock_(1.0 / 60.0, 5) {
    // CENA DEFAULT = cidade REAL (Distritos Inferiores) carregada do .gmap selado. O
    // I/O + load_map + fallback ficam no city_loader (fronteira app/); se o .gmap
    // faltar/estiver invalido, ele cai na cena de teste do M1 sem crashar. O feel
    // (velocidade/corner/zoom) vem do tuning da cidade (city_scene.hpp); as cores dos
    // tiles, da TilePalette (graybox). O lider ajusta nesses pontos unicos.
    gus::app::screens::CityLoadOutcome city =
        gus::app::screens::load_city_or_fallback();
    sim_ = std::make_unique<gus::app::screens::OverworldSim>(std::move(city.sim));
}

SdlWindow::~SdlWindow() {
    render2d_.reset();
    if (renderer_ != nullptr) {
        SDL_DestroyRenderer(renderer_);
    }
    // M7-COSTURA: so destroi a janela se ESTA instancia a criou (init()). Em modo
    // anexado (init_attached, usado pela Maestro) a janela e da Maestro - o dtor
    // desta SdlWindow NUNCA a toca.
    if (owns_window_ && window_ != nullptr) {
        SDL_DestroyWindow(window_);
    }
}

void SdlWindow::load_player_sprites() {
    // PLAYER = GUS (default). Carrega o walk de 7 quadros por direcao + o breathing
    // idle de 5 quadros e os entrega ao sim (handles resolvidos pelo renderer). Se
    // faltar arquivo, o set fica incompleto e o sim cai pro contorno (fallback).
    const std::string assets = gus::app::screens::resolve_gus_sprites_dir();
    sim_->set_player_sprites(
        gus::app::screens::load_gus_sprites(*render2d_, assets));
}

bool SdlWindow::init() {
    // Cria janela + renderer num passo (SDL3 helper). vsync ligado por padrao
    // (suave; o lider pode decidir desligar - ver relatorio).
    if (!SDL_CreateWindowAndRenderer("GusWorld", kWindowW, kWindowH,
                                     SDL_WINDOW_RESIZABLE, &window_, &renderer_)) {
        SDL_Log("SDL_CreateWindowAndRenderer falhou: %s", SDL_GetError());
        return false;
    }
    owns_window_ = true;
    SDL_SetRenderVSync(renderer_, 1);  // 1 = sincroniza com o refresh

    render2d_ =
        std::make_unique<gus::platform::render2d::Render2dSdl>(renderer_);

    // Abre gamepad ja conectado + arma hot-plug.
    input_.open_gamepads();

    load_player_sprites();
    return true;
}

bool SdlWindow::init_attached(SDL_Window* window) {
    window_ = window;
    owns_window_ = false;  // a janela e da Maestro - o dtor NAO a destroi

    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (renderer_ == nullptr) {
        SDL_Log("SDL_CreateRenderer (attached) falhou: %s", SDL_GetError());
        return false;
    }
    SDL_SetRenderVSync(renderer_, 1);

    render2d_ =
        std::make_unique<gus::platform::render2d::Render2dSdl>(renderer_);

    input_.open_gamepads();
    load_player_sprites();
    return true;
}

void SdlWindow::release_renderer() {
    render2d_.reset();
    if (renderer_ != nullptr) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
}

bool SdlWindow::reacquire_renderer() {
    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (renderer_ == nullptr) {
        SDL_Log("SDL_CreateRenderer (reacquire) falhou: %s", SDL_GetError());
        return false;
    }
    SDL_SetRenderVSync(renderer_, 1);
    render2d_ =
        std::make_unique<gus::platform::render2d::Render2dSdl>(renderer_);
    // Os TextureId anteriores nao sobrevivem ao SDL_Renderer destruido - recarrega.
    load_player_sprites();
    return true;
}

bool SdlWindow::step() {
    // 1) INPUT: drena os eventos SDL (teclado + gamepad). false = fechar.
    if (!input_.pump_events()) {
        return false;
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

    // 4) RENDER: 1 frame, interpolado pelo alpha residual. Viewport em PIXELS. So
    // desenha se o renderer esta vivo (release_renderer o esvazia durante a batalha -
    // a Maestro nao chama step() nesse intervalo, mas o guard e defensivo/barato).
    if (render2d_ != nullptr && renderer_ != nullptr) {
        int pw = kWindowW, ph = kWindowH;
        SDL_GetCurrentRenderOutputSize(renderer_, &pw, &ph);
        sim_->render(*render2d_, static_cast<float>(pw), static_cast<float>(ph),
                     static_cast<float>(steps.alpha));
    }
    return true;
}

void SdlWindow::run() {
    while (step()) {
        // corpo vazio: step() ja fez poll+update+render de 1 frame.
    }
}

const gus::core::spatial::Aabb& SdlWindow::player_aabb() const noexcept {
    return sim_->player();
}

const gus::core::spatial::TileGrid& SdlWindow::grid() const noexcept {
    return sim_->grid();
}

}  // namespace gus::app
