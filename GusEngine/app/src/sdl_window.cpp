// gus/app/src/sdl_window.cpp
//
// Implementacao da SdlWindow - casca SDL de janela + loop PROPRIO + input. Ver
// header. Caminho irredutivel (janela/renderer/loop): coberto pelo smoke headless
// do main (--smoke, SDL_VIDEODRIVER=dummy). A regra de jogo (OverworldSim) e o
// renderer (Render2dSdl) sao testados a parte.

#include "gus/app/sdl_window.hpp"

#include <cstdlib>  // std::getenv (resolve_assets_subdir_local)
#include <string>

#include "gus/app/screens/anim_catalog.hpp"  // resolve_gus_sprites_dir
#include "gus/app/screens/city_loader.hpp"   // load_city_or_fallback
#include "gus/app/screens/player_sprites_loader.hpp"
#include "gus/core/asset_paths.hpp"  // kRetratosDir/kRetratoInimigoFile (marcador do inimigo)

// Raiz resources/ do repo, embutida pelo CMake (mesma macro que anim_catalog.cpp/
// battle_preview.cpp resolvem - PRIVATE no CMakeLists do target app, ver GusEngine/app/
// CMakeLists.txt). Guard defensivo caso este .cpp compile fora desse target um dia.
#ifndef GUSWORLD_ASSETS_DIR
#define GUSWORLD_ASSETS_DIR ""
#endif

namespace gus::app {

namespace {
constexpr int kWindowW = 1280;
constexpr int kWindowH = 720;

std::string join_asset_path(const std::string& a, const std::string& b) {
    if (a.empty()) {
        return b;
    }
    if (a.back() == '/') {
        return a + b;
    }
    return a + "/" + b;
}

// Resolve um SUB-CAMINHO relativo de asset (do header central, ex.: kRetratosDir ou
// kVfxBootPixelDir) pela MESMA receita de resolve_gus_sprites_dir (anim_catalog.cpp):
// env GUSWORLD_ASSETS > macro de compilacao GUSWORLD_ASSETS_DIR > relativo ao CWD
// (resources/). Generalizada de "resolve_retratos_dir_local" (M7-COSTURA Inc 2c: o
// boot pixelizado precisa da MESMA receita de resolucao, so muda o sub-caminho) - a
// FONTE do sub-caminho continua so a constante do chamador, nao hardcoded aqui.
std::string resolve_assets_subdir_local(std::string_view rel) {
    const std::string sub(rel);
    if (const char* env = std::getenv("GUSWORLD_ASSETS")) {
        if (env[0] != '\0') {
            return join_asset_path(env, sub);
        }
    }
    const std::string compiled = GUSWORLD_ASSETS_DIR;
    if (!compiled.empty()) {
        return join_asset_path(compiled, sub);
    }
    return join_asset_path("resources", sub);
}
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
    load_boot_pixel_frames();
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
    load_boot_pixel_frames();
    return true;
}

void SdlWindow::load_enemy_marker_texture() {
    if (!enemy_marker_aabb_.has_value()) {
        return;  // nenhum marcador definido ainda (uso standalone/sem Maestro)
    }
    const std::string path = join_asset_path(
        resolve_assets_subdir_local(gus::core::assets::kRetratosDir),
        std::string(gus::core::assets::kRetratoInimigoFile));
    enemy_marker_tex_ = render2d_->load_texture(path.c_str());
    if (enemy_marker_tex_ != gus::platform::render2d::kInvalidTexture) {
        sim_->set_enemy_marker(*enemy_marker_aabb_, enemy_marker_tex_);
    } else {
        // Asset ausente/headless: degrada sem deixar um TextureId obsoleto no sim_.
        sim_->clear_enemy_marker();
    }
}

void SdlWindow::load_boot_pixel_frames() {
    boot_overlay_.load(*render2d_,
                        resolve_assets_subdir_local(gus::core::assets::kVfxBootPixelDir));
}

void SdlWindow::set_enemy_marker(const gus::core::spatial::Aabb& aabb) {
    enemy_marker_aabb_ = aabb;
    load_enemy_marker_texture();
}

void SdlWindow::clear_enemy_marker() {
    enemy_marker_aabb_.reset();
    sim_->clear_enemy_marker();
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
    // Idem pro marcador de inimigo (M7-COSTURA Inc 2): so recarrega se ja havia um
    // definido (no-op seguro se enemy_marker_aabb_ nunca foi setada).
    load_enemy_marker_texture();
    // Idem pro boot pixelizado (M7-COSTURA Inc 2c): os 20 TextureId antigos tambem
    // nao sobrevivem a troca de SDL_Renderer.
    load_boot_pixel_frames();
    return true;
}

bool SdlWindow::step() { return step_with_fade(0.0f); }

bool SdlWindow::step_with_fade(float overlay_alpha,
                                gus::core::anim::FadeDirection direction) {
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
        // DUNGEON-SCALING (fix do letterbox ao maximizar, achado do lider ao vivo
        // 2026-07-03): o ZOOM (quanto mundo a camera mostra) fica FIXO no viewport
        // LOGICO de referencia (kWindowW x kWindowH - a resolucao onde
        // camera_zoom_px_per_tile foi calibrado, ver overworld_tuning.hpp); pw/ph REAIS
        // (que crescem ao maximizar) so entram como o TAMANHO DE TELA pra onde esse
        // enquadramento e esticado (render()/screen_px_w-h, novo). Antes, pw/ph reais
        // alimentavam TAMBEM o zoom: maximizar pedia mais MUNDO (nao mais TELA); quando
        // a visao pedida ficava maior que o mapa (60x40 unidades), clamp_camera nao
        // tinha pra onde rolar e a area alem do mapa ficava sem tile nenhum - o clear
        // color escuro aparecendo como as "margens pretas" relatadas (nao era um
        // letterbox de fato, SDL_SetRenderLogicalPresentation nunca era chamado). Agora
        // a cidade estica pro tamanho real da janela, mesma tecnica da batalha
        // (Render2dGl3: camera fixa 960x540 D1 mapeada pros pixels reais).
        const float kLogicalViewportW = static_cast<float>(kWindowW);
        const float kLogicalViewportH = static_cast<float>(kWindowH);
        // M7-COSTURA Inc 2: overlay_alpha>0 adia o present (mesmo mecanismo do HUD
        // RmlUi, ADR-009) pra desenhar o retangulo preto DEPOIS da cena e ANTES do
        // swap. overlay_alpha<=0 nao mexe em defer_present - BYTE-IDENTICO ao step()
        // de sempre (nenhuma chamada extra, nenhum present() manual).
        const bool has_overlay = overlay_alpha > 0.0f;
        if (has_overlay) {
            render2d_->set_defer_present(true);
        }
        sim_->render(*render2d_, kLogicalViewportW, kLogicalViewportH,
                     static_cast<float>(steps.alpha), static_cast<float>(pw),
                     static_cast<float>(ph));
        if (has_overlay) {
            const float clamped = overlay_alpha > 1.0f ? 1.0f : overlay_alpha;
            // camera_view() so devolve o RETANGULO DE MUNDO (nao mexe em pixel de
            // tela) - precisa da MESMA dupla (kLogicalViewportW/H) que render() usou
            // pro zoom, senao o overlay full-screen do boot pixelizado desalinharia do
            // enquadramento que a cena acabou de desenhar.
            const gus::core::spatial::CameraView cam =
                sim_->camera_view(kLogicalViewportW, kLogicalViewportH);
            // M7-COSTURA Inc 2c: sequencia de frames pre-renderizada (boot
            // pixelizado) no lugar do glitch procedural (aposentado - o lider VETOU
            // o visual ao vivo, "pareceu bug"). A CIDADE so toca 2 das 4 pernas da
            // transicao inteira (ver gus/core/anim/boot_pixel_sequence.hpp): kOut =
            // kToBattleDarkening (t=clamped, o MESMO valor que ja era o alpha
            // original pra essa perna); kIn = kFromBattleRevealing (t=1-clamped,
            // pois fade_overlay_alpha(kIn,...) ja devolve 1-t). MESMA janela
            // temporal, MESMO invariante de seguranca (extremo continua cobrindo
            // tudo, opaco). Ver gus/app/boot_pixel_overlay.hpp.
            const bool going_to_battle =
                direction == gus::core::anim::FadeDirection::kOut;
            const auto leg = going_to_battle
                                  ? gus::core::anim::BootPixelLeg::kToBattleDarkening
                                  : gus::core::anim::BootPixelLeg::kFromBattleRevealing;
            const float t = going_to_battle ? clamped : (1.0f - clamped);
            boot_overlay_.draw(*render2d_, cam.rect, leg, t);
            render2d_->present();
            render2d_->set_defer_present(false);  // restaura o default pro step() normal
        }
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

const gus::app::screens::OverworldTuning& SdlWindow::tuning() const noexcept {
    return sim_->tuning();
}

bool SdlWindow::consume_escape_pressed() noexcept {
    return input_.consume_escape_pressed();
}

}  // namespace gus::app
