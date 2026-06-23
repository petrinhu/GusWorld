// GusEngine/app/main.cpp
// Ponto de entrada do gusworld_app (overworld jogavel). Pos repivot ADR-008: a
// casca de plataforma e SDL3 (nos possuimos o loop), nao mais Qt.
//
// DOIS MODOS:
//   (1) normal (sem args): SDL_Init -> SdlWindow (janela + renderer + loop proprio)
//       -> o lider joga (move o Caua com WASD/setas/GAMEPAD, desliza nas paredes,
//       camera presa ao mapa, animacao de walk por distancia).
//   (2) --smoke[=N]: modo HEADLESS pro CI/hook. Inicializa tudo, roda N ticks do
//       loop logico (default 120) com input roteirizado, faz 1 render OFFSCREEN
//       (Render2dSdl em modo headless - renderer nulo, sem display nem GPU),
//       imprime um resumo e sai 0 SEM entrar no loop interativo. E o que o
//       tools/check.sh executa com SDL_VIDEODRIVER=dummy / SDL_AUDIODRIVER=dummy.
//
// O smoke exercita a MESMA cena (test_overworld.hpp), o MESMO passo de simulacao
// (OverworldSim::step_fixed) e o MESMO renderer (Render2dSdl) que a janela - so
// troca o loop interativo por um for de N ticks e a janela por um render headless.

#include <SDL3/SDL.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

#include "gus/app/sdl_window.hpp"
#include "gus/app/screens/anim_catalog.hpp"  // resolve_gus_sprites_dir
#include "gus/app/screens/anim_preview.hpp"
#include "gus/app/screens/overworld_sim.hpp"
#include "gus/app/screens/player_sprites_loader.hpp"
#include "gus/app/screens/test_overworld.hpp"
#include "gus/core/spatial/grid_collision.hpp"
#include "gus/core/time/fixed_timestep.hpp"
#include "gus/core/version.hpp"
#include "gus/platform/render2d/render2d_sdl.hpp"

namespace {

// Parseia "--smoke" ou "--smoke=N". Devolve true se o modo smoke foi pedido e
// escreve o numero de ticks em out_ticks (default 120). N invalido/ausente -> 120.
// True se "--anim-preview" estiver entre os argumentos. Modo VIEWER: abre uma
// janela que mostra as animacoes do Gus em loop (catalogo varrido em runtime),
// pro lider VER a arte rodando na nossa engine. Reusa o mesmo backend (Render2dSdl).
bool parse_anim_preview(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]) == "--anim-preview") {
            return true;
        }
    }
    return false;
}

bool parse_smoke(int argc, char** argv, int& out_ticks) {
    out_ticks = 120;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if (arg == "--smoke") {
            return true;
        }
        if (arg.rfind("--smoke=", 0) == 0) {
            const std::string n(arg.substr(8));
            try {
                const int v = std::stoi(n);
                if (v > 0) {
                    out_ticks = v;
                }
            } catch (...) {
                // mantem o default
            }
            return true;
        }
    }
    return false;
}

// Roda o smoke HEADLESS: N ticks fixos com input roteirizado (anda pra direita) +
// 1 render no Render2dSdl em modo headless (renderer nulo). Sem display/GPU.
// Devolve 0 se tudo ok.
int run_smoke(int ticks) {
    // Render headless: renderer nulo -> draws viram no-op contabilizado, sprites
    // degradam para o contorno. Prova que a cadeia monta e roda offscreen.
    gus::platform::render2d::Render2dSdl renderer(nullptr);
    gus::app::screens::OverworldSim sim(
        gus::app::screens::make_test_map(),
        gus::app::screens::kTestPlayerStart,
        gus::app::screens::make_test_tuning());

    // Exercita o caminho de SPRITE do GUS tambem no headless: o loader degrada
    // (renderer nulo -> kInvalidTexture) e o sim cai pro contorno. Nao crasha.
    const std::string assets = gus::app::screens::resolve_gus_sprites_dir();
    sim.set_player_sprites(gus::app::screens::load_gus_sprites(renderer, assets));

    gus::core::time::FixedTimestep clock(1.0 / 60.0, 5);
    const float dt = static_cast<float>(clock.fixed_dt());
    for (int i = 0; i < ticks; ++i) {
        sim.step_fixed(/*dx=*/1, /*dy=*/0, /*run=*/false, dt);
    }

    // Um render offscreen (exercita o caminho de render, sem display).
    sim.render(renderer, 256.0f, 256.0f, /*alpha=*/0.0f);

    const gus::core::spatial::Aabb& p = sim.player();
    std::cout << "GusEngine " << gus::core::engine_version()
              << " smoke OK (SDL): " << ticks << " ticks, jogador em (" << p.x
              << ", " << p.y << "), " << renderer.last_draw_count()
              << " primitivos desenhados\n";
    return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
    // Modo VIEWER de animacao: --anim-preview. Cuida do proprio SDL_Init/Quit.
    if (parse_anim_preview(argc, argv)) {
        return gus::app::screens::run_anim_preview();
    }

    int ticks = 0;
    const bool smoke = parse_smoke(argc, argv, ticks);

    if (smoke) {
        // Headless: nao precisa nem inicializar video. Roda, valida e sai 0.
        return run_smoke(ticks);
    }

    // Modo normal: inicializa SDL (video + gamepad), abre a janela e roda o loop.
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }

    int rc = 0;
    {
        gus::app::SdlWindow window;
        if (!window.init()) {
            std::cerr << "Falha ao inicializar a janela SDL.\n";
            rc = 1;
        } else {
            window.run();  // o lider joga; retorna ao fechar a janela
        }
    }  // window destruido antes do SDL_Quit

    SDL_Quit();
    return rc;
}
