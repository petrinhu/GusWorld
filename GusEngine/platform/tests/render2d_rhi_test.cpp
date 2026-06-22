// GusEngine/platform/tests/render2d_rhi_test.cpp
//
// Qt Test do Render2dRhi (M1, platform/render2d) com o backend NULL do QRhi.
// TEST-FIRST. Headless e SEM GPU: o backend Null exercita TODA a API do QRhi
// (cria pipeline, buffer, render pass, grava o pass) sem precisar de Vulkan/GL
// nem de display - exatamente o que torna o caminho irredutivel de GPU coberto
// por teste/smoke (risco R1: trocar de backend e ~1 ponto).
//
// O Render2dRhi implementa IRenderer (a abstracao que isola o QRhi do resto). O
// teste prova: cria o renderer sobre um QRhi, roda um frame OFFSCREEN
// (begin/draw/end) sem crashar, e que a contagem de vertices acumulados bate com
// o que foi desenhado (1 quad preenchido = 6 vertices; 1 contorno = 4 faixas =
// 24 vertices). A matematica de projecao em si ja e testada em
// viewport_transform_test; aqui o foco e o ciclo da API RHI.

#include <QtTest/QtTest>

#include <memory>

#include <rhi/qrhi.h>

#include "gus/core/spatial/camera_clamp.hpp"
#include "gus/platform/render2d/render2d_rhi.hpp"

using gus::core::spatial::Rect;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::Render2dRhi;

class Render2dRhiTest : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<QRhi> rhi_;

private slots:
    void initTestCase() {
        // Backend Null: implementa a API inteira sem device real. Ideal pra CI.
        QRhiNullInitParams params;
        rhi_.reset(QRhi::create(QRhi::Null, &params));
        QVERIFY(rhi_ != nullptr);
    }

    void cleanupTestCase() { rhi_.reset(); }

    void cria_sobre_rhi_sem_crashar() {
        Render2dRhi r(rhi_.get());
        QVERIFY(true);  // construcao nao deve lancar
    }

    void frame_offscreen_completo_nao_crasha() {
        // Alvo offscreen: textura + render target. Mesma estrutura que o app usa
        // no modo --smoke (sem swapchain).
        std::unique_ptr<QRhiTexture> tex(
            rhi_->newTexture(QRhiTexture::RGBA8, QSize(64, 64), 1,
                             QRhiTexture::RenderTarget));
        QVERIFY(tex->create());
        std::unique_ptr<QRhiTextureRenderTarget> rt(
            rhi_->newTextureRenderTarget({tex.get()}));
        std::unique_ptr<QRhiRenderPassDescriptor> rp(
            rt->newCompatibleRenderPassDescriptor());
        rt->setRenderPassDescriptor(rp.get());
        QVERIFY(rt->create());

        Render2dRhi r(rhi_.get());

        QRhiCommandBuffer* cb = nullptr;
        QCOMPARE(rhi_->beginOffscreenFrame(&cb), QRhi::FrameOpSuccess);
        QVERIFY(cb != nullptr);

        // O owner informa o contexto do frame (cb + render target) ao renderer.
        r.set_frame_context(cb, rt.get());

        Rect cam{0.0f, 0.0f, 64.0f, 64.0f};
        r.begin_frame(cam, 64, 64);
        r.draw_filled_rect(Rect{8.0f, 8.0f, 16.0f, 16.0f},
                           DrawColor{0.2f, 0.2f, 0.3f, 1.0f});
        r.draw_rect_outline(Rect{30.0f, 30.0f, 10.0f, 10.0f},
                            DrawColor{0.2f, 0.9f, 0.9f, 1.0f}, 2.0f);
        r.end_frame();

        rhi_->endOffscreenFrame();

        // 1 quad preenchido (6 verts) + 1 contorno (4 faixas * 6 = 24 verts).
        QCOMPARE(r.last_vertex_count(), 30);
    }

    void frame_vazio_e_valido() {
        // Um frame sem nenhum draw (so begin/end) deve ser valido (limpa a tela).
        std::unique_ptr<QRhiTexture> tex(
            rhi_->newTexture(QRhiTexture::RGBA8, QSize(32, 32), 1,
                             QRhiTexture::RenderTarget));
        QVERIFY(tex->create());
        std::unique_ptr<QRhiTextureRenderTarget> rt(
            rhi_->newTextureRenderTarget({tex.get()}));
        std::unique_ptr<QRhiRenderPassDescriptor> rp(
            rt->newCompatibleRenderPassDescriptor());
        rt->setRenderPassDescriptor(rp.get());
        QVERIFY(rt->create());

        Render2dRhi r(rhi_.get());
        QRhiCommandBuffer* cb = nullptr;
        QCOMPARE(rhi_->beginOffscreenFrame(&cb), QRhi::FrameOpSuccess);
        r.set_frame_context(cb, rt.get());
        Rect cam{0.0f, 0.0f, 32.0f, 32.0f};
        r.begin_frame(cam, 32, 32);
        r.end_frame();
        rhi_->endOffscreenFrame();
        QCOMPARE(r.last_vertex_count(), 0);
    }
};

QTEST_MAIN(Render2dRhiTest)
#include "render2d_rhi_test.moc"
