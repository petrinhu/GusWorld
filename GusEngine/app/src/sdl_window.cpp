// gus/app/src/sdl_window.cpp
//
// Implementacao da SdlWindow - casca SDL de janela + loop PROPRIO + input. Ver
// header. Caminho irredutivel (janela/renderer/loop): coberto pelo smoke headless
// do main (--smoke, SDL_VIDEODRIVER=dummy). A regra de jogo (OverworldSim) e o
// renderer (Render2dGl3, FLASH-CTX - era Render2dSdl) sao testados a parte.

#include "gus/app/sdl_window.hpp"

#include <string>
#include <utility>  // std::move (set_controls)

#include "gus/app/screens/anim_catalog.hpp"  // resolve_gus_sprites_dir
#include "gus/app/screens/city_loader.hpp"   // load_city_or_fallback
#include "gus/app/screens/player_sprites_loader.hpp"
#include "gus/core/asset_paths.hpp"  // kRetratosDir/kRetratoInimigoFile (marcador do inimigo); kBertoldoSpritesDir/kBertoldoSpriteSouthFile (marcador do Bertoldo)
#include "gus/platform/assets/asset_source.hpp"  // ASSETS-VFS-F1 (ADR-013): porteiro
#include "gus/platform/rmlui/gl3_loader.hpp"  // FLASH-CTX: gl3_load_functions (init() standalone) + gl3_read_backbuffer_rgba (capture_frame_to_png)

// stb_image_write: SO o header aqui (mesma receita de stb_image.h em render2d_gl3.cpp -
// ver seu comentario de topo) - a IMPLEMENTACAO (STB_IMAGE_WRITE_IMPLEMENTATION) ja e
// definida UMA vez em screens/battle_preview.cpp, NESTE MESMO alvo estatico
// (gusengine_app, ver CMakeLists.txt) - definir de novo aqui daria symbol duplicado no
// link. Usado por capture_frame_to_png (fundo real congelado do dialogo/menu de pausa,
// M7-DIALOGO/MENU-PAUSA-CONFIG-SOM).
#include "stb_image_write.h"

#include "gus/app/app_icon.hpp"  // APP-ICON: set_window_icon_if_available

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
    // FLASH-CTX: destroi o Render2dGl3 (libera GL: texturas/programa/VAO/VBO) ENQUANTO o
    // contexto ainda pode estar corrente - so entao (se dono) derruba o contexto e a
    // janela, MESMA ordem de gus/app/screens/battle_preview.cpp (Render2dGl3 destruido
    // antes do SDL_GL_DestroyContext). No modo ANEXADO (init_attached, a Maestro) nem o
    // contexto nem a janela sao desta SdlWindow - so o Render2dGl3 e destruido aqui; a
    // Maestro cuida do resto no PROPRIO dtor.
    render2d_.reset();
    // M7-COSTURA / FLASH-CTX: so destroi contexto+janela se ESTA instancia os criou
    // (init(), caminho STANDALONE). Em modo anexado (init_attached, usado pela Maestro) a
    // janela E o contexto GL sao da Maestro - o dtor desta SdlWindow NUNCA os toca.
    if (owns_window_) {
        if (gl_context_ != nullptr) {
            SDL_GL_DestroyContext(gl_context_);
        }
        if (window_ != nullptr) {
            SDL_DestroyWindow(window_);
        }
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
    // FLASH-CTX (A1): caminho STANDALONE - cria a JANELA + o CONTEXTO GL PROPRIOS (3.3
    // core/doublebuffer/stencil 8, MESMA receita de gus/app/maestro.cpp::init() - ver o
    // comentario grande no header). SDL_WINDOW_OPENGL e obrigatorio aqui (o antigo
    // SDL_CreateWindowAndRenderer criava um SDL_Renderer; agora e a janela CRUA + o
    // contexto que criamos manualmente).
    window_ = SDL_CreateWindow("GusWorld", kWindowW, kWindowH,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window_ == nullptr) {
        SDL_Log("SDL_CreateWindow (standalone GL) falhou: %s", SDL_GetError());
        return false;
    }
    owns_window_ = true;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    gl_context_ = SDL_GL_CreateContext(window_);
    if (gl_context_ == nullptr) {
        SDL_Log("SDL_GL_CreateContext (standalone) falhou: %s", SDL_GetError());
        return false;
    }
    SDL_GL_MakeCurrent(window_, gl_context_);
    SDL_GL_SetSwapInterval(1);  // 1 = sincroniza com o refresh (era SDL_SetRenderVSync)
    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        SDL_Log("SdlWindow::init - gl3_load_functions (glad) falhou.");
        return false;
    }

    render2d_ = std::make_unique<gus::platform::render2d::Render2dGl3>(
        /*gl_active=*/true);

    // APP-ICON: esta SdlWindow e dona da janela (owns_window_) - aplica aqui pela
    // simetria "quem cria a janela decide o icone". NOTA: no modo normal (main.cpp),
    // quem cria a UNICA janela e a Maestro (nao esta SdlWindow, que so entra via
    // init_attached na janela ja existente e ja com icone - ver Maestro::init) -
    // este init() e o caminho STANDALONE (sem Maestro), usado por testes/uso
    // avulso da SdlWindow. Chamar aqui de novo nesse caminho e correto e barato.
    // (NOTA FLASH-CTX: a chamada real fica FORA de escopo desta refatoracao - o
    // comentario ja existia assim antes; init() e dead code sem chamador de producao.)

    // Abre gamepad ja conectado + arma hot-plug.
    input_.open_gamepads();

    load_player_sprites();
    load_boot_pixel_frames();
    return true;
}

bool SdlWindow::init_attached(SDL_Window* window) {
    window_ = window;
    owns_window_ = false;  // a janela e da Maestro - o dtor NAO a destroi

    // FLASH-CTX (A1): NAO cria contexto nem faz make-current - o CHAMADOR (a Maestro,
    // gus/app/maestro.cpp::init()) ja fez isso ANTES de chamar init_attached() (o
    // contexto GL UNICO do processo, vivo do boot ao shutdown). Ver o comentario grande
    // no header. gl_active=true assume que ha um contexto corrente valido; se
    // Render2dGl3::init_gl() falhar (programa GL nao compila/linka), o proprio ctor
    // degrada pra headless (gl_active_ vira false internamente) sem lancar.
    render2d_ = std::make_unique<gus::platform::render2d::Render2dGl3>(
        /*gl_active=*/true);

    // APP-ICON: a janela e da Maestro (dona real, ver init() acima) - ela ja aplica o
    // icone na criacao; nada a fazer aqui alem de nao duplicar.

    input_.open_gamepads();
    load_player_sprites();
    load_boot_pixel_frames();
    return true;
}

void SdlWindow::load_enemy_marker_texture() {
    if (!enemy_marker_aabb_.has_value()) {
        return;  // nenhum marcador definido ainda (uso standalone/sem Maestro)
    }
    // FIX CRASH (SIGSEGV real, playtest ao vivo 2026-07-17: pausa na cidade -> menu
    // Save/Load -> LOAD -> deref de ponteiro nulo aqui). RAIZ ORIGINAL (era SDL_Renderer):
    // o menu de pausa rodava NUM CONTEXTO GL PROPRIO e a Maestro (open_pause_from_city)
    // chamava release_renderer() ANTES de abrir o menu -> render2d_ == nullptr enquanto o
    // menu estava vivo. FLASH-CTX (A1->A3): o Render2dGl3 e o contexto GL sao UNICOS e
    // vivem do boot ao shutdown (nunca ha mais um contexto GL PROPRIO por cima do da
    // Maestro, desde que o A3 removeu a ultima casca owning do call-graph de producao,
    // passo 5 do plano) - render2d_ nunca mais e nulo nem "pausado" depois de
    // init()/init_attached(). sim_ nunca e nulo (criado no ctor), mas guardado por
    // simetria/defesa.
    if (render2d_ == nullptr || sim_ == nullptr) {
        return;
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
    // FIX CRASH (mesma raiz de load_enemy_marker_texture acima, FLASH-CTX A1->A3): o
    // guard hoje so cobre defesa contra ponteiro nulo - o contexto GL nunca mais e
    // "pausado" (ver a nota grande em load_enemy_marker_texture acima).
    if (render2d_ == nullptr || sim_ == nullptr) {
        return;
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
    // FLASH-CTX PODA (A3, passo 6 do plano): NO-OP de verdade - nao ha mais guard
    // renderer_paused_ pra setar (removido; ver o comentario grande no header). A
    // Maestro nao chama mais isto (passo 5); mantido so por compatibilidade de API
    // com call-sites remanescentes fora de app/src/ (deprecated, ver o header).
}

bool SdlWindow::reacquire_renderer() {
    // FLASH-CTX PODA (A3, passo 6 do plano): NO-OP de verdade - nao ha mais nada pra
    // despausar/recarregar (release_renderer() acima tambem virou no-op; a cidade
    // nunca para de desenhar). Devolve true por compatibilidade de API (deprecated,
    // ver o header) - a Maestro nao chama mais isto.
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
    // desenha se render2d_ existe (defensivo/barato - FLASH-CTX A1->A3: nao ha mais
    // um estado "pausado" pra checar - a cidade desenha SEMPRE que step()/step_with_
    // fade() e chamado, batalha/menu/dialogo/titulo agora desenham no MESMO contexto
    // por cima, ver Maestro::run()/to_battle()/open_pause_from_city()).
    if (render2d_ != nullptr) {
        int pw = kWindowW, ph = kWindowH;
        // FLASH-CTX: SDL_GetWindowSizeInPixels (era SDL_GetCurrentRenderOutputSize do
        // SDL_Renderer) - equivalente em GL, MESMA chamada que battle_preview.cpp usa.
        SDL_GetWindowSizeInPixels(window_, &pw, &ph);
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
            render2d_->present();  // FLASH-CTX: no-op no GL (mantido por simetria de API)
            render2d_->set_defer_present(false);  // restaura o default pro step() normal
        }
        // FLASH-CTX (passo 3 do plano): o swap do frame da cidade agora e explicito -
        // Render2dGl3::end_frame() NUNCA apresenta sozinho (diferente do Render2dSdl
        // antigo, que auto-presentava quando defer_present_==false). Cobre os DOIS
        // ramos acima (com/sem overlay): 1 swap por frame, no fim, MESMA disciplina
        // "o dono do frame faz o swap" da batalha (ver battle_preview.cpp).
        SDL_GL_SwapWindow(window_);
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

void SdlWindow::set_player_position(const gus::core::spatial::Aabb& aabb) noexcept {
    sim_->set_player_position(aabb);
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

void SdlWindow::set_controls(gus::domain::input::InputRemapConfig config) {
    input_.set_controls(std::move(config));
}

void SdlWindow::render_dialogue_overlay_frame(const std::vector<std::string>& lines) {
    if (render2d_ == nullptr) {
        return;  // degradacao segura (headless/GL nao compilou) - no-op
    }
    int pw = kWindowW, ph = kWindowH;
    SDL_GetWindowSizeInPixels(window_, &pw, &ph);  // FLASH-CTX: era SDL_GetCurrentRenderOutputSize
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

    render2d_->present();  // FLASH-CTX: no-op no GL (mantido por simetria de API)
    render2d_->set_defer_present(false);  // restaura o default pro step() normal
    // FLASH-CTX: swap explicito (Render2dGl3::end_frame() nunca apresenta sozinho -
    // era present AUTOMATICO do Render2dSdl com defer_present=false; ver step_with_fade).
    SDL_GL_SwapWindow(window_);
}

void SdlWindow::hold_frozen_frame(int frames) {
    if (render2d_ == nullptr) {
        return;  // degradacao segura (headless/GL nao compilou) - nada a segurar
    }
    int pw = kWindowW, ph = kWindowH;
    SDL_GetWindowSizeInPixels(window_, &pw, &ph);  // FLASH-CTX: era SDL_GetCurrentRenderOutputSize
    const float kLogicalViewportW = static_cast<float>(kWindowW);
    const float kLogicalViewportH = static_cast<float>(kWindowH);
    for (int i = 0; i < frames; ++i) {
        // alpha=1.0 sem interpolar: sim_ NAO avancou durante o menu (mesma cena
        // parada que o fundo congelado ja mostrava) - so redesenha+apresenta pra
        // sobrescrever qualquer imagem indefinida/antiga do swapchain com o conteudo
        // CORRETO. FLASH-CTX: Render2dGl3::end_frame() NAO apresenta sozinho (ao
        // contrario do Render2dSdl antigo, que auto-presentava com defer_present=
        // false) - o SWAP EXPLICITO por iteracao (abaixo) e o que de fato cobre CADA
        // imagem do swapchain, MESMO proposito de antes.
        sim_->render(*render2d_, kLogicalViewportW, kLogicalViewportH, /*alpha=*/1.0f,
                     static_cast<float>(pw), static_cast<float>(ph));
        SDL_GL_SwapWindow(window_);
    }
}

bool SdlWindow::capture_frame_to_png(const std::string& out_path) {
    if (render2d_ == nullptr) {
        return false;  // degradacao segura (headless/GL nao compilou) - nada a capturar
    }
    int pw = kWindowW, ph = kWindowH;
    SDL_GetWindowSizeInPixels(window_, &pw, &ph);  // FLASH-CTX: era SDL_GetCurrentRenderOutputSize
    const float kLogicalViewportW = static_cast<float>(kWindowW);
    const float kLogicalViewportH = static_cast<float>(kWindowH);

    // FUNDO REAL CONGELADO (M7-DIALOGO/MENU-PAUSA-CONFIG-SOM, decisao do lider). FLASH-CTX
    // (A1, passo 4 - DEFAULT escolhido, ver o header): a caixa de dialogo do NPC e o menu
    // de pausa passam a mostrar a CENA REAL da cidade (ultimo frame antes de abrir), nao
    // mais a vinheta abstrata - mesmo padrao de Chrono Trigger/Zelda/Stardew Valley (o
    // mundo "pausa" atras da UI).
    //
    // TECNICA (GL): redesenha o MESMO frame (sim_ NAO avanca - nenhum step_fixed aqui,
    // alpha=1.0 sem interpolar, MESMA receita de render_dialogue_overlay_frame acima) e
    // le o BACKBUFFER via gus::platform::rmlui::gl3_read_backbuffer_rgba (glReadPixels +
    // flip vertical pra origem no topo, MESMA funcao que a batalha ja usa pro smoke
    // visual - ver gus/platform/rmlui/gl3_loader.cpp) ANTES do swap - o conteudo do
    // GL_BACK so e garantido ENTRE o desenho e o SDL_GL_SwapWindow (ler DEPOIS do swap
    // seria o FRONT antigo/indefinido, double-buffer real). Depois de ler, apresenta o
    // MESMO frame normalmente (SDL_GL_SwapWindow - o dono do frame faz o swap, mesma
    // disciplina de step_with_fade acima) - byte-identico ao que o jogador ja estava
    // vendo, so desenhado 1x a mais (custo desprezivel: 1 captura pontual por ABERTURA
    // de tela, nao por frame). set_defer_present/present() SAO no-op no GL (mantidos por
    // simetria de API com a era Render2dSdl - nao afetam o resultado).
    render2d_->set_defer_present(true);
    sim_->render(*render2d_, kLogicalViewportW, kLogicalViewportH, /*alpha=*/1.0f,
                 static_cast<float>(pw), static_cast<float>(ph));

    std::vector<unsigned char> pixels(static_cast<std::size_t>(pw) * ph * 4);
    const bool read_ok =
        gus::platform::rmlui::gl3_read_backbuffer_rgba(pw, ph, pixels.data());

    render2d_->present();  // no-op no GL, mantido por simetria
    render2d_->set_defer_present(false);  // restaura o default pro step() normal
    SDL_GL_SwapWindow(window_);  // apresenta o MESMO frame que acabou de ser lido

    if (!read_ok) {
        SDL_Log(
            "SdlWindow: capture_frame_to_png - gl3_read_backbuffer_rgba falhou "
            "(dimensoes invalidas ou glad nao carregado).");
        return false;
    }
    // stb_image_write escreve RGBA32 tightly-packed (4 bytes/pixel, stride = w*4) -
    // gl3_read_backbuffer_rgba ja devolve exatamente esse formato (glReadPixels com
    // GL_RGBA/GL_UNSIGNED_BYTE + GL_PACK_ALIGNMENT=1), sem SDL_ConvertSurface
    // intermediario (esse passo so existia pra normalizar o formato NATIVO/variavel do
    // SDL_RenderReadPixels - nao existe mais no caminho GL).
    const int ok = stbi_write_png(out_path.c_str(), pw, ph, /*comp=*/4, pixels.data(),
                                  pw * 4);
    if (ok == 0) {
        SDL_Log("SdlWindow: capture_frame_to_png - stbi_write_png falhou (%s)",
                out_path.c_str());
        return false;
    }
    return true;
}

}  // namespace gus::app
