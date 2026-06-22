// gus/app/game_window.hpp
//
// GameWindow: a CASCA Qt do M1 (janela + loop + bombeamento de input). Vive em
// app/ por ser o SHELL DE APLICACAO (event loop, eventos de tecla, ciclo de frame
// do QRhi com swapchain), nao um modulo de engine reutilizavel - o renderer
// reutilizavel (Render2dRhi atras de IRenderer) e a regra de jogo (OverworldSim)
// estao FORA daqui. Mantida o mais fina possivel: nenhuma regra de jogo, so
// orquestracao Qt. O irredutivel (que nao da pra unit-testar sem janela/GPU) e
// coberto pelo smoke offscreen do main (--smoke).
//
// Esta e a UNICA classe que junta tudo no caminho de janela real: mede o tempo,
// roda o FixedTimestep (passo fixo + alpha), alimenta o OverworldSim com (dx,dy)
// do InputMapper, e desenha via Render2dRhi no swapchain.
//
// Inclui <Q...> e <rhi/qrhi.h> (camada app/, Qt permitido).

#ifndef GUS_APP_GAME_WINDOW_HPP
#define GUS_APP_GAME_WINDOW_HPP

#include <memory>

#include <QElapsedTimer>
#include <QWindow>

#include <rhi/qrhi.h>

#include "gus/app/screens/overworld_sim.hpp"
#include "gus/core/time/fixed_timestep.hpp"
#include "gus/platform/input/input_mapper.hpp"
#include "gus/platform/render2d/render2d_rhi.hpp"

namespace gus::app {

class GameWindow : public QWindow {
    Q_OBJECT

public:
    explicit GameWindow(QWindow* parent = nullptr);
    ~GameWindow() override;

protected:
    // Inicializa RHI/swapchain no primeiro expose e dispara o loop de render.
    void exposeEvent(QExposeEvent* e) override;
    // requestUpdate() agenda um UpdateRequest; aqui renderizamos um frame.
    bool event(QEvent* e) override;
    // Teclado -> InputMapper (via key_translation). Ignora auto-repeat (idempotente).
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    // Perder foco: solta todas as teclas pra nao "grudar" movimento.
    void focusOutEvent(QFocusEvent* e) override;

private:
    bool init_rhi();           // cria QRhi (OpenGL) + swapchain. false se falhar.
    void render_frame();       // um frame: dt -> ticks fixos -> render alpha -> agenda
    void release_swapchain();  // libera recursos do swapchain (resize/teardown)

    std::unique_ptr<QRhi> rhi_;
    std::unique_ptr<QRhiSwapChain> swapchain_;
    std::unique_ptr<QRhiRenderPassDescriptor> rp_;
    std::unique_ptr<gus::platform::render2d::Render2dRhi> renderer_;

    std::unique_ptr<gus::app::screens::OverworldSim> sim_;
    gus::platform::input::InputMapper input_;
    gus::core::time::FixedTimestep clock_;
    QElapsedTimer frame_timer_;

    bool initialized_ = false;
    bool have_last_time_ = false;
    qint64 last_ns_ = 0;
};

}  // namespace gus::app

#endif  // GUS_APP_GAME_WINDOW_HPP
