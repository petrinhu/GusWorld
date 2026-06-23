// GusEngine/app/main.cpp
// Ponto de entrada do gusworld_app (M1: janela + loop + overworld jogavel).
//
// DOIS MODOS:
//   (1) normal (sem args): abre a janela real (GameWindow), entra no event loop
//       (app.exec()) e o lider joga - move o retangulo do jogador com WASD/setas,
//       deslizando nas paredes, camera presa ao mapa.
//   (2) --smoke[=N]: modo HEADLESS pro CI/hook. Inicializa tudo, roda N ticks do
//       loop logico (default 120) com um input roteirizado, faz 1 render
//       OFFSCREEN (backend Null do QRhi - sem display nem GPU), imprime um resumo
//       e sai 0 SEM entrar no event loop. E o que o tools/check.sh executa com
//       QT_QPA_PLATFORM=offscreen pra validar a cadeia inteira sem travar.
//
// O smoke exercita a MESMA cena (test_overworld.hpp), o MESMO passo de simulacao
// (OverworldSim::step_fixed) e o MESMO renderer (Render2dRhi) que o caminho de
// janela - so troca o swapchain por um render target offscreen e o event loop por
// um for de N ticks. Prova que tudo monta e roda end-to-end.

#include <QGuiApplication>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

#include <rhi/qrhi.h>

#include "gus/app/game_window.hpp"
#include "gus/app/screens/overworld_sim.hpp"
#include "gus/app/screens/player_sprites_loader.hpp"
#include "gus/app/screens/test_overworld.hpp"
#include "gus/core/spatial/grid_collision.hpp"
#include "gus/core/time/fixed_timestep.hpp"
#include "gus/core/version.hpp"
#include "gus/platform/render2d/render2d_rhi.hpp"

namespace {

// Parseia "--smoke" ou "--smoke=N". Devolve true se o modo smoke foi pedido e
// escreve o numero de ticks em out_ticks (default 120). N invalido/ausente -> 120.
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

// Roda o smoke offscreen: Null backend + render target de textura + a cena de
// teste. Roda N ticks fixos com input roteirizado (anda pra direita) e desenha 1
// frame offscreen. Devolve 0 se tudo ok, !=0 se algum passo de RHI falhar.
int run_smoke(int ticks) {
    QRhiNullInitParams params;
    std::unique_ptr<QRhi> rhi(QRhi::create(QRhi::Null, &params));
    if (!rhi) {
        std::cerr << "smoke: falhou ao criar QRhi (Null)\n";
        return 1;
    }

    // Alvo offscreen (sem janela/swapchain).
    std::unique_ptr<QRhiTexture> tex(rhi->newTexture(
        QRhiTexture::RGBA8, QSize(256, 256), 1, QRhiTexture::RenderTarget));
    if (!tex->create()) {
        std::cerr << "smoke: falhou ao criar textura\n";
        return 1;
    }
    std::unique_ptr<QRhiTextureRenderTarget> rt(
        rhi->newTextureRenderTarget({tex.get()}));
    std::unique_ptr<QRhiRenderPassDescriptor> rp(
        rt->newCompatibleRenderPassDescriptor());
    rt->setRenderPassDescriptor(rp.get());
    if (!rt->create()) {
        std::cerr << "smoke: falhou ao criar render target\n";
        return 1;
    }

    gus::platform::render2d::Render2dRhi renderer(rhi.get());
    gus::app::screens::OverworldSim sim(
        gus::app::screens::make_test_map(),
        gus::app::screens::kTestPlayerStart,
        gus::app::screens::make_test_tuning());

    // Exercita o caminho de SPRITE tambem no headless: carrega o set do Caua e o
    // entrega ao sim. No backend Null pode nao haver textura real - o loader
    // DEGRADA (slots invalidos) e o sim cai pro contorno; o objetivo do smoke e
    // so provar que load + render-com-sprite-ou-fallback nao crasham offscreen.
    const std::string assets = gus::app::screens::resolve_caua_sprites_dir();
    sim.set_player_sprites(gus::app::screens::load_caua_sprites(renderer, assets));

    gus::core::time::FixedTimestep clock(1.0 / 60.0, 5);

    // Roda N ticks logicos com input roteirizado (direita), simulando 1/60s/tick.
    const float dt = static_cast<float>(clock.fixed_dt());
    for (int i = 0; i < ticks; ++i) {
        sim.step_fixed(/*dx=*/1, /*dy=*/0, /*run=*/false, dt);
    }

    // Um frame offscreen (exercita o caminho de render real, sem display).
    QRhiCommandBuffer* cb = nullptr;
    if (rhi->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess) {
        std::cerr << "smoke: beginOffscreenFrame falhou\n";
        return 1;
    }
    renderer.set_frame_context(cb, rt.get());
    sim.render(renderer, 256.0f, 256.0f, /*alpha=*/0.0f);
    rhi->endOffscreenFrame();

    const gus::core::spatial::Aabb& p = sim.player();
    std::cout << "GusEngine " << gus::core::engine_version()
              << " smoke OK: " << ticks << " ticks, jogador em ("
              << p.x << ", " << p.y << "), "
              << renderer.last_vertex_count() << " vertices desenhados\n";
    return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    int ticks = 0;
    if (parse_smoke(argc, argv, ticks)) {
        // Headless: roda, valida e sai 0 (NUNCA entra no event loop).
        return run_smoke(ticks);
    }

    // Modo normal: janela real + loop. O lider joga.
    gus::app::GameWindow window;
    window.show();
    return app.exec();
}
