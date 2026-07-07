// gus/app/src/sdl_window.cpp
//
// Implementacao da SdlWindow - casca SDL de janela + loop PROPRIO + input. Ver
// header. Caminho irredutivel (janela/renderer/loop): coberto pelo smoke headless
// do main (--smoke, SDL_VIDEODRIVER=dummy). A regra de jogo (OverworldSim) e o
// renderer (Render2dSdl) sao testados a parte.

#include "gus/app/sdl_window.hpp"

#include <string>

#include "gus/app/screens/anim_catalog.hpp"  // resolve_gus_sprites_dir
#include "gus/app/screens/city_loader.hpp"   // load_city_or_fallback
#include "gus/app/screens/player_sprites_loader.hpp"
#include "gus/core/asset_paths.hpp"  // kRetratosDir/kRetratoInimigoFile (marcador do inimigo); kBertoldoSpritesDir/kBertoldoSpriteSouthFile (marcador do Bertoldo)
#include "gus/platform/assets/asset_source.hpp"  // ASSETS-VFS-F1 (ADR-013): porteiro

// stb_image_write: SO o header aqui (mesma receita de stb_image.h em render2d_gl3.cpp -
// ver seu comentario de topo) - a IMPLEMENTACAO (STB_IMAGE_WRITE_IMPLEMENTATION) ja e
// definida UMA vez em screens/battle_preview.cpp, NESTE MESMO alvo estatico
// (gusengine_app, ver CMakeLists.txt) - definir de novo aqui daria symbol duplicado no
// link. Usado por capture_frame_to_png (fundo real congelado do dialogo/menu de pausa,
// M7-DIALOGO/MENU-PAUSA-CONFIG-SOM).
#include "stb_image_write.h"

namespace gus::app {

namespace {
constexpr int kWindowW = 1280;
constexpr int kWindowH = 720;

// Resolve um SUB-CAMINHO relativo de asset (do header central, ex.: kRetratosDir ou
// kVfxBootPixelDir) - familia GENERICA. ASSETS-VFS-F1 (ADR-013): a cadeia `env
// GUSWORLD_ASSETS > macro GUSWORLD_ASSETS_DIR > CWD (resources/)` foi CONSOLIDADA em
// FilesystemAssetSource (mesmo destino de resolve_gus_sprites_dir em anim_catalog.cpp).
// Assinatura INTOCADA.
std::string resolve_assets_subdir_local(std::string_view rel) {
    return gus::platform::assets::FilesystemAssetSource().resolve_path(rel);
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
    // ASSETS-VFS-F1: monta o id RELATIVO completo (subdir + arquivo) e resolve UMA vez
    // (era resolver so o subdir e concatenar o arquivo depois - equivalente, mas agora o
    // porteiro resolve o caminho inteiro de uma vez, mesmo padrao dos demais consumidores).
    const std::string id = std::string(gus::core::assets::kRetratosDir) + "/" +
                            std::string(gus::core::assets::kRetratoInimigoFile);
    const std::string path = resolve_assets_subdir_local(id);
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

void SdlWindow::load_npc_bertoldo_marker_texture() {
    if (!npc_bertoldo_marker_aabb_.has_value()) {
        return;  // nenhum marcador definido ainda (uso standalone/sem Maestro)
    }
    const std::string id = std::string(gus::core::assets::kBertoldoSpritesDir) + "/" +
                            std::string(gus::core::assets::kBertoldoSpriteSouthFile);
    const std::string path = resolve_assets_subdir_local(id);
    npc_bertoldo_marker_tex_ = render2d_->load_texture(path.c_str());
    if (npc_bertoldo_marker_tex_ != gus::platform::render2d::kInvalidTexture) {
        sim_->set_npc_bertoldo_marker(*npc_bertoldo_marker_aabb_,
                                      npc_bertoldo_marker_tex_);
    } else {
        // Asset ausente/headless: degrada sem deixar um TextureId obsoleto no sim_.
        sim_->clear_npc_bertoldo_marker();
    }
}

void SdlWindow::set_npc_bertoldo_marker(const gus::core::spatial::Aabb& aabb) {
    npc_bertoldo_marker_aabb_ = aabb;
    load_npc_bertoldo_marker_texture();
}

void SdlWindow::clear_npc_bertoldo_marker() {
    npc_bertoldo_marker_aabb_.reset();
    sim_->clear_npc_bertoldo_marker();
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
    // Idem pro marcador do Bertoldo (M7-DIALOGO, integracao do sprite): mesmo
    // racional, no-op seguro se npc_bertoldo_marker_aabb_ nunca foi setada.
    load_npc_bertoldo_marker_texture();
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

void SdlWindow::clear_input() noexcept { input_.clear(); }

void SdlWindow::render_dialogue_overlay_frame(const std::vector<std::string>& lines) {
    if (render2d_ == nullptr || renderer_ == nullptr) {
        return;  // renderer liberado (uso incorreto/degradacao segura) - no-op
    }
    int pw = kWindowW, ph = kWindowH;
    SDL_GetCurrentRenderOutputSize(renderer_, &pw, &ph);
    const float kLogicalViewportW = static_cast<float>(kWindowW);
    const float kLogicalViewportH = static_cast<float>(kWindowH);

    // M7-DIALOGO (NPC-MVP): mesmo mecanismo de present-diferido de step_with_fade -
    // desenha a cidade PARADA (alpha=1.0, sem interpolar - sim_ nao avancou
    // step_fixed enquanto o dialogo esta aberto) e compoe o overlay POR CIMA antes
    // do present manual.
    render2d_->set_defer_present(true);
    sim_->render(*render2d_, kLogicalViewportW, kLogicalViewportH, /*alpha=*/1.0f,
                 static_cast<float>(pw), static_cast<float>(ph));

    const gus::core::spatial::CameraView cam =
        sim_->camera_view(kLogicalViewportW, kLogicalViewportH);

    // Caixa de texto SIMPLES (overlay funcional, nao a apresentacao fina RCSS de
    // DIALOGO-TERMINAL): ancorada no rodape do enquadramento visivel ATUAL
    // (cam.rect, o MESMO retangulo de mundo que boot_overlay_ ja usa pra se
    // alinhar ao enquadramento - ver step_with_fade acima).
    const float box_h = cam.rect.h * 0.34f;
    const gus::core::spatial::Rect box{cam.rect.x, cam.rect.y + cam.rect.h - box_h,
                                        cam.rect.w, box_h};
    render2d_->draw_filled_rect(
        box, gus::platform::render2d::DrawColor{0.02f, 0.04f, 0.06f, 0.82f});

    const float px_size = cam.rect.h * 0.028f;
    const float pad = cam.rect.w * 0.02f;
    float ty = box.y + pad;
    for (const std::string& line : lines) {
        render2d_->draw_text(line.c_str(), box.x + pad, ty, px_size,
                              gus::platform::render2d::DrawColor{0.85f, 0.95f, 0.9f,
                                                                  1.0f},
                              /*bold=*/false);
        ty += px_size * 1.4f;
    }

    render2d_->present();
    render2d_->set_defer_present(false);  // restaura o default pro step() normal
}

bool SdlWindow::capture_frame_to_png(const std::string& out_path) {
    if (render2d_ == nullptr || renderer_ == nullptr) {
        return false;  // renderer liberado - nada a capturar (degradacao segura)
    }
    int pw = kWindowW, ph = kWindowH;
    SDL_GetCurrentRenderOutputSize(renderer_, &pw, &ph);
    const float kLogicalViewportW = static_cast<float>(kWindowW);
    const float kLogicalViewportH = static_cast<float>(kWindowH);

    // FUNDO REAL CONGELADO (M7-DIALOGO/MENU-PAUSA-CONFIG-SOM, decisao do lider): a
    // caixa de dialogo do NPC e o menu de pausa passam a mostrar a CENA REAL da
    // cidade (ultimo frame antes de abrir), nao mais a vinheta abstrata - mesmo
    // padrao de Chrono Trigger/Zelda/Stardew Valley (o mundo "pausa" atras da UI).
    //
    // TECNICA: redesenha o MESMO frame (sim_ NAO avanca - nenhum step_fixed aqui,
    // alpha=1.0 sem interpolar, MESMA receita de render_dialogue_overlay_frame
    // acima) com present ADIADO (ADR-009, set_defer_present) e le o backbuffer via
    // SDL_RenderReadPixels ANTES de apresentar - a doc da SDL3 so garante o
    // conteudo do frame ATUAL nessa janela (entre o desenho e o SDL_RenderPresent);
    // ler DEPOIS do swap seria conteudo indefinido em backends com double-buffer
    // real (opengl/vulkan/d3d, onde o backbuffer troca de lugar com o front no
    // present). MESMA tecnica ja provada empiricamente sob Xvfb (ver
    // app/tools/repro_bertoldo.cpp, que le OK sem nenhum present). Depois de ler,
    // presenta o MESMO frame normalmente - byte-identico ao que o jogador ja
    // estava vendo, so desenhado 1x a mais (custo desprezivel: 1 captura pontual
    // por ABERTURA de tela, nao por frame).
    render2d_->set_defer_present(true);
    sim_->render(*render2d_, kLogicalViewportW, kLogicalViewportH, /*alpha=*/1.0f,
                 static_cast<float>(pw), static_cast<float>(ph));

    SDL_Surface* surface = SDL_RenderReadPixels(renderer_, nullptr);
    render2d_->present();
    render2d_->set_defer_present(false);  // restaura o default pro step() normal

    if (surface == nullptr) {
        SDL_Log("SdlWindow: capture_frame_to_png - SDL_RenderReadPixels falhou: %s",
                SDL_GetError());
        return false;
    }
    // Converte pro formato RGBA32 tightly-defined (o formato NATIVO devolvido por
    // SDL_RenderReadPixels varia por backend/driver - normalizar antes de escrever
    // o PNG, mesma receita de repro_bertoldo.cpp).
    SDL_Surface* converted = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);
    if (converted == nullptr) {
        SDL_Log("SdlWindow: capture_frame_to_png - SDL_ConvertSurface falhou: %s",
                SDL_GetError());
        return false;
    }
    const int ok = stbi_write_png(out_path.c_str(), converted->w, converted->h,
                                  /*comp=*/4, converted->pixels, converted->pitch);
    SDL_DestroySurface(converted);
    if (ok == 0) {
        SDL_Log("SdlWindow: capture_frame_to_png - stbi_write_png falhou (%s)",
                out_path.c_str());
        return false;
    }
    return true;
}

}  // namespace gus::app
