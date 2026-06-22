// GusEngine/platform/tests/viewport_transform_test.cpp
//
// Qt Test da matematica de projecao mundo -> NDC do render2d (M1). TEST-FIRST.
// Headless: nao toca GPU; valida so as contas que o shader/vertex-buffer vao usar.
//
// CONTEXTO: a camera e ORTOGRAFICA FIXA (decisao do lider, sem zoom/lerp). O
// CameraView.rect (em unidades de MUNDO, vindo de clamp_camera) define a janela
// visivel; ela mapeia para o cubo NDC [-1,1] x [-1,1] do Qt RHI. O offset da
// camera = -rect.{x,y}; a escala normaliza pra [-1,1].
//
// INVERSAO DE Y (ponto critico): no MUNDO +Y aponta para BAIXO (tile_grid.hpp);
// em NDC +Y aponta para CIMA. Logo a projecao INVERTE Y. Um ponto no TOPO do
// mundo visivel (y == rect.y) cai em ndc_y = +1; no fundo (y == rect.y+rect.h)
// cai em ndc_y = -1. Errar isso poe o mapa de cabeca pra baixo: por isso este
// teste existe.
//
// CONTRATO exercitado:
//   - world_to_ndc(px, py, cam_rect) -> NdcPoint {x,y};
//   - canto sup-esq do rect -> (-1, +1); canto inf-dir -> (+1, -1); centro -> (0,0);
//   - build_quad_ndc(world_rect, cam_rect) -> 4 cantos NDC do quad (ordem
//     conhecida) que o renderer manda pro vertex buffer.

#include <QtTest/QtTest>

#include "gus/core/spatial/camera_clamp.hpp"  // Rect (mundo)
#include "gus/platform/render2d/viewport_transform.hpp"

using gus::core::spatial::Rect;
using gus::platform::render2d::build_quad_ndc;
using gus::platform::render2d::NdcPoint;
using gus::platform::render2d::QuadNdc;
using gus::platform::render2d::world_to_ndc;

namespace {
constexpr float kEps = 1e-4f;
}

class ViewportTransformTest : public QObject {
    Q_OBJECT

private slots:
    void centro_do_rect_vira_origem_ndc() {
        // Camera cobrindo mundo [100..300] x [50..250]; centro (200,150).
        Rect cam{100.0f, 50.0f, 200.0f, 200.0f};
        NdcPoint c = world_to_ndc(200.0f, 150.0f, cam);
        QVERIFY(qAbs(c.x - 0.0f) < kEps);
        QVERIFY(qAbs(c.y - 0.0f) < kEps);
    }

    void canto_superior_esquerdo_vira_menos1_mais1() {
        Rect cam{100.0f, 50.0f, 200.0f, 200.0f};
        NdcPoint p = world_to_ndc(100.0f, 50.0f, cam);
        QVERIFY(qAbs(p.x - (-1.0f)) < kEps);
        QVERIFY(qAbs(p.y - (1.0f)) < kEps);  // topo do mundo -> +1 (Y invertido)
    }

    void canto_inferior_direito_vira_mais1_menos1() {
        Rect cam{100.0f, 50.0f, 200.0f, 200.0f};
        NdcPoint p = world_to_ndc(300.0f, 250.0f, cam);
        QVERIFY(qAbs(p.x - (1.0f)) < kEps);
        QVERIFY(qAbs(p.y - (-1.0f)) < kEps);  // fundo do mundo -> -1
    }

    void y_invertido_metade_de_cima_e_positivo() {
        // Ponto na metade superior (y menor que o centro) deve ter ndc_y > 0.
        Rect cam{0.0f, 0.0f, 100.0f, 100.0f};
        NdcPoint p = world_to_ndc(50.0f, 25.0f, cam);  // acima do centro
        QVERIFY(p.y > 0.0f);
        QVERIFY(qAbs(p.y - 0.5f) < kEps);  // 25 esta a 1/4 do topo -> +0.5
    }

    void quad_gera_quatro_cantos_corretos() {
        // World rect [10..20] x [10..30] (w=10,h=20) numa camera [0..100]^2.
        Rect cam{0.0f, 0.0f, 100.0f, 100.0f};
        Rect world{10.0f, 10.0f, 10.0f, 20.0f};
        QuadNdc q = build_quad_ndc(world, cam);

        // ndc_x: 10 -> -0.8 ; 20 -> -0.6
        // ndc_y(topo y=10) -> 1 - 2*(10/100) = 0.8 ; ndc_y(fundo y=30) -> 1 - 2*(30/100) = 0.4
        // Ordem documentada: [0]=sup-esq, [1]=sup-dir, [2]=inf-dir, [3]=inf-esq.
        QVERIFY(qAbs(q.corners[0].x - (-0.8f)) < kEps);
        QVERIFY(qAbs(q.corners[0].y - (0.8f)) < kEps);
        QVERIFY(qAbs(q.corners[1].x - (-0.6f)) < kEps);
        QVERIFY(qAbs(q.corners[1].y - (0.8f)) < kEps);
        QVERIFY(qAbs(q.corners[2].x - (-0.6f)) < kEps);
        QVERIFY(qAbs(q.corners[2].y - (0.4f)) < kEps);
        QVERIFY(qAbs(q.corners[3].x - (-0.8f)) < kEps);
        QVERIFY(qAbs(q.corners[3].y - (0.4f)) < kEps);
    }

    void rect_degenerado_da_camera_nao_divide_por_zero() {
        // Camera com w/h zero (estado degenerado): nao deve gerar NaN/inf.
        Rect cam{0.0f, 0.0f, 0.0f, 0.0f};
        NdcPoint p = world_to_ndc(5.0f, 5.0f, cam);
        QVERIFY(qIsFinite(p.x));
        QVERIFY(qIsFinite(p.y));
    }
};

QTEST_MAIN(ViewportTransformTest)
#include "viewport_transform_test.moc"
