// gus/app/src/game_window.cpp
//
// Implementacao da GameWindow (M1) - casca Qt de janela + loop + input. Ver
// header. Caminho irredutivel (janela/GPU/event loop): nao unit-testavel; coberto
// pelo smoke offscreen do main (--smoke), que roda a mesma logica de step/render
// sem janela. A regra de jogo e o renderer sao testados a parte.

#include "gus/app/game_window.hpp"

#include <QExposeEvent>
#include <QFocusEvent>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QOffscreenSurface>
#include <QOpenGLContext>

#include <rhi/qrhi.h>

#include "gus/app/screens/test_overworld.hpp"
#include "gus/domain/input/controls_restore.hpp"
#include "gus/platform/input/key_translation.hpp"

namespace gus::app {

GameWindow::GameWindow(QWindow* parent)
    : QWindow(parent),
      input_(gus::domain::input::default_controls()),
      clock_(1.0 / 60.0, 5) {
    // OpenGL como backend de janela do M1 (ampla compatibilidade, inclui iGPU -
    // risco R3). Trocar de backend e ~1 ponto aqui + no init_rhi.
    setSurfaceType(QSurface::OpenGLSurface);
    setTitle(QStringLiteral("GusWorld"));
    resize(1280, 720);

    sim_ = std::make_unique<gus::app::screens::OverworldSim>(
        gus::app::screens::make_test_map(),
        gus::app::screens::kTestPlayerStart,
        gus::app::screens::kTestWalkTilesPerSec);
}

GameWindow::~GameWindow() {
    // Ordem importa: solta recursos do swapchain antes do QRhi morrer.
    release_swapchain();
    renderer_.reset();
    rhi_.reset();
}

bool GameWindow::init_rhi() {
    if (rhi_) {
        return true;
    }
    // QRhi sobre OpenGL. Precisa de uma fallback surface offscreen pro contexto.
    QRhiGles2InitParams params;
    params.fallbackSurface = QRhiGles2InitParams::newFallbackSurface();
    params.window = this;
    rhi_.reset(QRhi::create(QRhi::OpenGLES2, &params));
    if (!rhi_) {
        return false;
    }

    swapchain_.reset(rhi_->newSwapChain());
    swapchain_->setWindow(this);
    rp_.reset(swapchain_->newCompatibleRenderPassDescriptor());
    swapchain_->setRenderPassDescriptor(rp_.get());

    renderer_ = std::make_unique<gus::platform::render2d::Render2dRhi>(rhi_.get());
    return true;
}

void GameWindow::release_swapchain() {
    rp_.reset();
    swapchain_.reset();
}

void GameWindow::exposeEvent(QExposeEvent* /*e*/) {
    if (isExposed() && !initialized_) {
        if (init_rhi()) {
            initialized_ = true;
            frame_timer_.start();
            render_frame();  // dispara o loop (cada frame agenda o proximo)
        }
    } else if (isExposed() && initialized_) {
        render_frame();
    }
}

bool GameWindow::event(QEvent* e) {
    if (e->type() == QEvent::UpdateRequest) {
        if (isExposed() && initialized_) {
            render_frame();
        }
        return true;
    }
    return QWindow::event(e);
}

void GameWindow::keyPressEvent(QKeyEvent* e) {
    if (e->isAutoRepeat()) {
        return;  // o estado e um conjunto; auto-repeat nao agrega
    }
    const long long code =
        gus::platform::input::qt_key_to_godot_keycode(e->key());
    input_.press(code);
}

void GameWindow::keyReleaseEvent(QKeyEvent* e) {
    if (e->isAutoRepeat()) {
        return;
    }
    const long long code =
        gus::platform::input::qt_key_to_godot_keycode(e->key());
    input_.release(code);
}

void GameWindow::focusOutEvent(QFocusEvent* /*e*/) {
    input_.clear();  // evita tecla "grudada" quando a janela perde foco
}

void GameWindow::render_frame() {
    if (!initialized_ || !swapchain_) {
        return;
    }

    // Reajusta o swapchain ao tamanho atual da superficie (resize/primeiro frame).
    const QSize outputSize = swapchain_->surfacePixelSize();
    if (outputSize.isEmpty()) {
        requestUpdate();
        return;
    }
    if (!swapchain_->currentPixelSize().isValid() ||
        swapchain_->currentPixelSize() != outputSize) {
        swapchain_->createOrResize();
    }

    // dt real desde o ultimo frame (segundos), com o relogio monotonico da janela.
    const qint64 now_ns = frame_timer_.nsecsElapsed();
    double dt = 0.0;
    if (have_last_time_) {
        dt = static_cast<double>(now_ns - last_ns_) / 1.0e9;
    }
    have_last_time_ = true;
    last_ns_ = now_ns;

    // Passo fixo: roda N updates logicos de 1/60s e devolve o alpha de interpolacao.
    const gus::core::time::FrameSteps steps = clock_.advance(dt);
    const int dx = input_.movement_dx();
    const int dy = input_.movement_dy();
    const bool run = input_.run_active();
    for (int i = 0; i < steps.ticks; ++i) {
        sim_->step_fixed(dx, dy, run, static_cast<float>(clock_.fixed_dt()));
    }

    // Render do frame no swapchain.
    if (rhi_->beginFrame(swapchain_.get()) != QRhi::FrameOpSuccess) {
        requestUpdate();
        return;
    }
    QRhiCommandBuffer* cb = swapchain_->currentFrameCommandBuffer();
    QRhiRenderTarget* rt = swapchain_->currentFrameRenderTarget();
    renderer_->set_frame_context(cb, rt);

    const float vw = static_cast<float>(outputSize.width());
    const float vh = static_cast<float>(outputSize.height());
    sim_->render(*renderer_, vw, vh, static_cast<float>(steps.alpha));

    rhi_->endFrame(swapchain_.get());

    requestUpdate();  // agenda o proximo frame (loop continuo enquanto exposto)
}

}  // namespace gus::app
