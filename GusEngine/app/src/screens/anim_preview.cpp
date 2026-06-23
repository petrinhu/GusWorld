// gus/app/src/screens/anim_preview.cpp
//
// Ver header. Casca SDL do viewer de animacao. Reusa Render2dSdl (atras de
// IRenderer) e o mesmo padrao de loop da SdlWindow (poll -> update -> render),
// mas com poll de TECLA direto (Tab/setas/Esc) e AnimClock dirigindo o quadro.

#include "gus/app/screens/anim_preview.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

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
        gus::platform::render2d::Render2dSdl renderer(sdl_renderer);

        // 3) Estado do viewer: anim corrente, frames carregados, relogio.
        int anim_idx = 0;
        gus::core::anim::AnimClock clock(
            static_cast<int>(catalog[anim_idx].frames.size()), 10.0f);
        std::vector<gus::platform::render2d::TextureId> frames =
            load_frames(renderer, catalog[anim_idx]);

        auto switch_anim = [&](int new_idx) {
            const int n = static_cast<int>(catalog.size());
            anim_idx = ((new_idx % n) + n) % n;  // wrap nos dois sentidos
            frames = load_frames(renderer, catalog[anim_idx]);
            clock.set_frame_count(static_cast<int>(catalog[anim_idx].frames.size()));
        };

        set_hud(window, catalog[anim_idx], clock.frame(), clock.frame_count(),
                clock.fps(), anim_idx, static_cast<int>(catalog.size()));

        // 4) Loop: poll de evento -> avanca relogio com dt real -> render.
        bool running = true;
        bool have_last = false;
        unsigned long long last_ns = 0;
        while (running) {
            SDL_Event ev;
            while (SDL_PollEvent(&ev)) {
                if (ev.type == SDL_EVENT_QUIT) {
                    running = false;
                } else if (ev.type == SDL_EVENT_KEY_DOWN) {
                    switch (ev.key.key) {
                        case SDLK_ESCAPE:
                            running = false;
                            break;
                        case SDLK_TAB:
                        case SDLK_RIGHT:
                            switch_anim(anim_idx + 1);
                            break;
                        case SDLK_LEFT:
                            switch_anim(anim_idx - 1);
                            break;
                        case SDLK_UP:
                            clock.nudge_fps(+1.0f);
                            break;
                        case SDLK_DOWN:
                            clock.nudge_fps(-1.0f);
                            break;
                        default:
                            break;
                    }
                }
            }
            if (!running) {
                break;
            }

            const unsigned long long now_ns = SDL_GetTicksNS();
            float dt = 0.0f;
            if (have_last) {
                dt = static_cast<float>(now_ns - last_ns) / 1.0e9f;
            }
            have_last = true;
            last_ns = now_ns;
            clock.advance(dt);

            // Render: camera = a propria tela em PIXELS (mundo == pixel aqui),
            // origem no canto superior-esquerdo, +Y pra baixo (igual ao backend).
            int pw = kWindowW, ph = kWindowH;
            SDL_GetCurrentRenderOutputSize(sdl_renderer, &pw, &ph);
            const gus::core::spatial::Rect cam{0.0f, 0.0f, static_cast<float>(pw),
                                               static_cast<float>(ph)};
            renderer.begin_frame(cam, pw, ph);

            // Fundo neutro: um quad que cobre a tela inteira.
            renderer.draw_filled_rect(cam, kBg);

            const int frame_i = clock.frame();
            gus::platform::render2d::TextureId tex =
                (frame_i >= 0 && frame_i < static_cast<int>(frames.size()))
                    ? frames[frame_i]
                    : gus::platform::render2d::kInvalidTexture;

            if (tex != gus::platform::render2d::kInvalidTexture) {
                // Preserva o aspect do PNG: usa o canvas real (alpha-bbox carrega
                // canvas_w/h). Escala pra ocupar kFitFraction da menor dimensao da
                // tela e centraliza.
                const gus::platform::render2d::ContentBbox bb =
                    renderer.texture_content_bbox(tex);
                float src_w = bb.valid() ? static_cast<float>(bb.canvas_w) : 1.0f;
                float src_h = bb.valid() ? static_cast<float>(bb.canvas_h) : 1.0f;

                const float fit = kFitFraction *
                                  static_cast<float>(std::min(pw, ph));
                const float scale = fit / std::max(src_w, src_h);
                const float dw = src_w * scale;
                const float dh = src_h * scale;
                const float dx = (static_cast<float>(pw) - dw) * 0.5f;
                const float dy = (static_cast<float>(ph) - dh) * 0.5f;

                const gus::core::spatial::Rect dst{dx, dy, dw, dh};
                const gus::platform::render2d::UvRect uv{0.0f, 0.0f, 1.0f, 1.0f};
                const gus::platform::render2d::DrawColor white{1.0f, 1.0f, 1.0f,
                                                               1.0f};
                renderer.draw_textured_rect(dst, tex, uv, white);
            }

            renderer.end_frame();

            set_hud(window, catalog[anim_idx], frame_i, clock.frame_count(),
                    clock.fps(), anim_idx, static_cast<int>(catalog.size()));
        }
    }  // Render2dSdl destruido antes de destruir o renderer SDL

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

}  // namespace gus::app::screens
