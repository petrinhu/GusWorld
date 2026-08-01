// SPDX-License-Identifier: Apache-2.0
// gus/app/src/screens/anim_preview.cpp
//
// Ver header. Casca SDL do viewer de animacao. Reusa Render2dSdl (atras de
// IRenderer) e o mesmo padrao de loop da SdlWindow (poll -> update -> render),
// mas com poll de TECLA direto (Tab/setas/Esc) e AnimClock dirigindo o quadro.
//
// F4-1b.6 (onda F4 "casca SDL -> App mode do glintfx", fatia 1b.6 - ferramenta
// INTERNA de dev, nao tela de jogo): o while(true){SDL_PollEvent...} PROPRIO
// que este arquivo tinha foi convertido pra AnimPreviewScreen (gus::app::
// ScreenState) + gus::app::run_screen_state (gus/app/screen_state.hpp), MESMA
// tecnica de DifficultyScreen (difficulty_menu_loop.cpp, F4-1b) - a CASCA
// (SDL_Init, janela, SDL_Renderer, SDL_Quit) fica no WRAPPER
// (run_anim_preview(), unico chamador: main.cpp via --anim-preview); o estado
// do viewer (catalogo, indice de anim, AnimClock, frames carregados) vira
// MEMBRO de AnimPreviewScreen. O roteamento de evento (Tab/setas com WRAP nos
// 2 sentidos, Cima/Baixo=nudge_fps, Esc, Quit) foi extraido pra a funcao livre
// PURA anim_preview_step (declarada no .hpp, testada headless SEM SDL_Init/
// IRenderer/AnimClock real em anim_preview_step_test.cpp) - ela NAO troca de
// anim de fato (isso e I/O de GPU via IRenderer::load_texture, efeito
// colateral), so devolve a DECISAO ("mude pro indice N, JA com wrap"); quem
// aplica e AnimPreviewScreen::handle_event.
//
// Ferramenta STANDALONE (nao tela de jogo): tem sua PROPRIA janela/SDL_Renderer,
// zero input compartilhado com o overworld/Maestro - por isso `sync_hook` de
// run_screen_state fica no default (nullptr), MESMO comportamento de sempre
// (nao ha estado de input persistente nenhum pra sincronizar aqui).

#include "gus/app/screens/anim_preview.hpp"

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

#include "gus/app/screen_state.hpp"
#include "gus/app/screens/anim_catalog.hpp"
#include "gus/core/anim/anim_clock.hpp"
#include "gus/core/spatial/camera_clamp.hpp"  // Rect
#include "gus/platform/render2d/render2d_sdl.hpp"

namespace gus::app::screens {

namespace {

constexpr int kWindowW = 960;
constexpr int kWindowH = 960;
// Fracao da menor dimensao da janela que o sprite (no maior eixo) deve ocupar.
constexpr float kFitFraction = 0.8f;
// Fundo neutro (cinza-medio), pra ver bordas claras E escuras do sprite.
constexpr gus::platform::render2d::DrawColor kBg{0.18f, 0.18f, 0.20f, 1.0f};

// Carrega (ou recupera do cache) os TextureId de TODOS os frames de uma anim.
std::vector<gus::platform::render2d::TextureId> load_frames(
    gus::platform::render2d::IRenderer& renderer, const AnimEntry& anim) {
    std::vector<gus::platform::render2d::TextureId> ids;
    ids.reserve(anim.frames.size());
    for (const std::string& path : anim.frames) {
        ids.push_back(renderer.load_texture(path.c_str()));
    }
    return ids;
}

// Atualiza o titulo da janela com o HUD (nome + frame + fps). Texto via titulo,
// pra nao precisar de fonte/atlas (desenhar texto seria custoso pro escopo).
void set_hud(SDL_Window* window, const AnimEntry& anim, int idx, int count,
             float fps, int anim_pos, int anim_total) {
    char buf[256];
    SDL_snprintf(buf, sizeof(buf),
                 "GusWorld AnimPreview  [%d/%d]  %s  |  frame %d/%d  |  %.1f fps  "
                 "(Tab/<- -> anim, Cima/Baixo fps, Esc sai)",
                 anim_pos + 1, anim_total, anim.label.c_str(), idx + 1, count, fps);
    SDL_SetWindowTitle(window, buf);
}

// Wrap nos DOIS sentidos (indice negativo E indice >= n) - MESMA formula do
// switch_anim lambda antigo. n <= 0 nunca deveria acontecer em producao (o
// wrapper so cria a tela com catalog_ nao-vazio), mas devolve 0 defensivamente
// em vez de dividir por zero.
int wrap_anim_idx(int idx, int n) noexcept {
    if (n <= 0) return 0;
    return ((idx % n) + n) % n;
}

}  // namespace

// F4-1b.6: implementacao de anim_preview_step (declarada no .hpp) - extracao
// BEHAVIOR-PRESERVING do corpo do while(true) antigo (MESMO roteamento de
// tecla: Tab/seta-direita/seta-esquerda trocam de anim com wrap, Cima/Baixo
// ajustam fps, Esc sai, Quit fecha a janela) - so devolvendo a DECISAO em vez
// de aplicar o efeito (load_frames/AnimClock::set_frame_count/nudge_fps) na
// hora.
AnimPreviewStepResult anim_preview_step(const AnimPreviewState& state,
                                        const SDL_Event& ev) noexcept {
    AnimPreviewStepResult result;

    if (ev.type == SDL_EVENT_QUIT) {
        result.window_closed = true;
        return result;
    }

    if (ev.type == SDL_EVENT_KEY_DOWN) {
        switch (ev.key.key) {
            case SDLK_ESCAPE:
                result.finished = true;
                break;
            case SDLK_TAB:
            case SDLK_RIGHT:
                result.switch_anim = true;
                result.new_anim_idx = wrap_anim_idx(state.anim_idx + 1, state.catalog_size);
                break;
            case SDLK_LEFT:
                result.switch_anim = true;
                result.new_anim_idx = wrap_anim_idx(state.anim_idx - 1, state.catalog_size);
                break;
            case SDLK_UP:
                result.fps_delta = 1.0f;
                break;
            case SDLK_DOWN:
                result.fps_delta = -1.0f;
                break;
            default:
                break;
        }
        return result;
    }

    return result;  // tipo de evento nao roteado por este viewer - no-op TOTAL
}

namespace {

// F4-1b.6: o ScreenState de PRODUCAO do viewer (unico chamador:
// run_anim_preview(), abaixo) - MESMO padrao de DifficultyScreen
// (difficulty_menu_loop.cpp, F4-1b). Todo o estado que antes vivia em
// variaveis locais fechadas pela lambda switch_anim (anim_idx, clock, frames)
// agora e MEMBRO; enter() cria o Render2dSdl + carrega a 1a anim, exit() o
// libera (MESMA ordem do while(true) antigo: Render2dSdl destruido ANTES do
// SDL_Renderer real, que o WRAPPER so destroi depois que run_screen_state()
// retorna).
class AnimPreviewScreen final : public gus::app::ScreenState {
public:
    AnimPreviewScreen(SDL_Window* window, SDL_Renderer* sdl_renderer,
                      std::vector<AnimEntry> catalog)
        : window_(window), sdl_renderer_(sdl_renderer), catalog_(std::move(catalog)) {}

    void enter() override {
        renderer_.emplace(sdl_renderer_);
        clock_.emplace(
            static_cast<int>(catalog_[static_cast<std::size_t>(anim_idx_)].frames.size()),
            10.0f);
        frames_ = load_frames(*renderer_, catalog_[static_cast<std::size_t>(anim_idx_)]);

        set_hud(window_, catalog_[static_cast<std::size_t>(anim_idx_)], clock_->frame(),
                clock_->frame_count(), clock_->fps(), anim_idx_,
                static_cast<int>(catalog_.size()));
    }

    void handle_event(const SDL_Event& ev) override {
        const AnimPreviewState state{anim_idx_, static_cast<int>(catalog_.size())};
        const AnimPreviewStepResult step = anim_preview_step(state, ev);

        if (step.window_closed) {
            window_closed_ = true;
            return;
        }
        if (step.finished) {
            finished_ = true;
            return;
        }
        if (step.switch_anim) {
            switch_anim_(step.new_anim_idx);
        }
        if (step.fps_delta != 0.0f) {
            clock_->nudge_fps(step.fps_delta);
        }
    }

    void tick(float dt) override {
        if (finished_ || window_closed_) {
            return;  // defensivo: run_screen_state() nao chama tick() quando
                      // finished()/window_closed() ja e true, mas guarda mesmo assim.
        }

        clock_->advance(dt);

        // Render: camera = a propria tela em PIXELS (mundo == pixel aqui),
        // origem no canto superior-esquerdo, +Y pra baixo (igual ao backend).
        int pw = kWindowW, ph = kWindowH;
        SDL_GetCurrentRenderOutputSize(sdl_renderer_, &pw, &ph);
        const gus::core::spatial::Rect cam{0.0f, 0.0f, static_cast<float>(pw),
                                           static_cast<float>(ph)};
        renderer_->begin_frame(cam, pw, ph);

        // Fundo neutro: um quad que cobre a tela inteira.
        renderer_->draw_filled_rect(cam, kBg);

        const int frame_i = clock_->frame();
        gus::platform::render2d::TextureId tex =
            (frame_i >= 0 && frame_i < static_cast<int>(frames_.size()))
                ? frames_[static_cast<std::size_t>(frame_i)]
                : gus::platform::render2d::kInvalidTexture;

        if (tex != gus::platform::render2d::kInvalidTexture) {
            // Preserva o aspect do PNG: usa o canvas real (alpha-bbox carrega
            // canvas_w/h). Escala pra ocupar kFitFraction da menor dimensao da
            // tela e centraliza.
            const gus::platform::render2d::ContentBbox bb =
                renderer_->texture_content_bbox(tex);
            float src_w = bb.valid() ? static_cast<float>(bb.canvas_w) : 1.0f;
            float src_h = bb.valid() ? static_cast<float>(bb.canvas_h) : 1.0f;

            const float fit = kFitFraction * static_cast<float>(std::min(pw, ph));
            const float scale = fit / std::max(src_w, src_h);
            const float dw = src_w * scale;
            const float dh = src_h * scale;
            const float dx = (static_cast<float>(pw) - dw) * 0.5f;
            const float dy = (static_cast<float>(ph) - dh) * 0.5f;

            const gus::core::spatial::Rect dst{dx, dy, dw, dh};
            const gus::platform::render2d::UvRect uv{0.0f, 0.0f, 1.0f, 1.0f};
            const gus::platform::render2d::DrawColor white{1.0f, 1.0f, 1.0f, 1.0f};
            renderer_->draw_textured_rect(dst, tex, uv, white);
        }

        renderer_->end_frame();

        set_hud(window_, catalog_[static_cast<std::size_t>(anim_idx_)], frame_i,
                clock_->frame_count(), clock_->fps(), anim_idx_,
                static_cast<int>(catalog_.size()));
    }

    [[nodiscard]] bool finished() const override { return finished_; }

    void exit() override {
        // Render2dSdl destruido ANTES do SDL_Renderer real (o WRAPPER so
        // destroi sdl_renderer_ DEPOIS que run_screen_state() retorna, ou
        // seja, depois deste exit() - MESMA ordem do while(true) antigo).
        renderer_.reset();
    }

    [[nodiscard]] bool window_closed() const override { return window_closed_; }

private:
    void switch_anim_(int new_idx) {
        anim_idx_ = new_idx;
        frames_ = load_frames(*renderer_, catalog_[static_cast<std::size_t>(anim_idx_)]);
        clock_->set_frame_count(
            static_cast<int>(catalog_[static_cast<std::size_t>(anim_idx_)].frames.size()));
    }

    SDL_Window* window_;
    SDL_Renderer* sdl_renderer_;
    std::vector<AnimEntry> catalog_;

    int anim_idx_ = 0;
    bool window_closed_ = false;
    bool finished_ = false;

    // std::optional (nao um objeto direto): enter()/exit() controlam o ciclo
    // de vida, MESMA tecnica de DifficultyScreen::ui_/backdrop_.
    std::optional<gus::platform::render2d::Render2dSdl> renderer_;
    std::optional<gus::core::anim::AnimClock> clock_;
    std::vector<gus::platform::render2d::TextureId> frames_;
};

}  // namespace

int run_anim_preview() {
    // 1) Cataloga as animacoes ANTES de abrir janela (se nao houver nada, nem abre).
    const std::string gus_dir = resolve_gus_sprites_dir();
    std::vector<AnimEntry> catalog = build_gus_anim_catalog(gus_dir);
    if (catalog.empty()) {
        std::cerr << "AnimPreview: nenhuma animacao encontrada em " << gus_dir
                  << " (defina GUSWORLD_ASSETS ou rode da raiz do repo).\n";
        return 2;
    }

    std::cout << "AnimPreview: " << catalog.size()
              << " animacoes em " << gus_dir << ":\n";
    for (std::size_t i = 0; i < catalog.size(); ++i) {
        std::cout << "  [" << (i + 1) << "] " << catalog[i].label << " ("
                  << catalog[i].frames.size() << " frames)\n";
    }

    // 2) SDL + janela + renderer (mesma casca do jogo).
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "AnimPreview: SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Window* window = nullptr;
    SDL_Renderer* sdl_renderer = nullptr;
    if (!SDL_CreateWindowAndRenderer("GusWorld AnimPreview", kWindowW, kWindowH,
                                     SDL_WINDOW_RESIZABLE, &window, &sdl_renderer)) {
        std::cerr << "AnimPreview: SDL_CreateWindowAndRenderer falhou: "
                  << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderVSync(sdl_renderer, 1);

    {
        // 3) Estado do viewer (catalogo, anim corrente, frames, relogio) vive
        // em AnimPreviewScreen; o LOOP UNICO (1 pump por frame) e
        // gus::app::run_screen_state - sem sync_hook (ferramenta standalone,
        // zero input persistente pra sincronizar).
        AnimPreviewScreen screen(window, sdl_renderer, std::move(catalog));
        gus::app::run_screen_state(screen);
    }  // Render2dSdl (dentro do screen) ja destruido pelo exit() antes daqui

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

}  // namespace gus::app::screens
