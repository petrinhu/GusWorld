// gus/platform/src/render2d/render2d_rhi.cpp
//
// Implementacao do Render2dRhi (M1). Ver header. Caminho irredutivel de GPU;
// exercitado headless por platform/tests/render2d_rhi_test.cpp (backend Null) e
// pelo smoke offscreen do app. A LOGICA de projecao (build_quad_ndc) e tested a
// parte (viewport_transform_test).

#include "gus/platform/render2d/render2d_rhi.hpp"

#include <QColor>
#include <QFile>

#include <rhi/qshader.h>  // QShader (camada platform/, QRhi/private permitido)

#include "gus/platform/render2d/viewport_transform.hpp"

namespace gus::platform::render2d {

namespace {

constexpr int kFloatsPerVertex = 6;  // x,y (NDC) + r,g,b,a
constexpr int kStrideBytes = kFloatsPerVertex * static_cast<int>(sizeof(float));

// Carrega um QShader serializado (.qsb) de um recurso Qt embutido.
QShader load_shader(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        return QShader();
    }
    return QShader::fromSerialized(f.readAll());
}

}  // namespace

Render2dRhi::Render2dRhi(QRhi* rhi) : rhi_(rhi) {}

Render2dRhi::~Render2dRhi() = default;

void Render2dRhi::set_frame_context(QRhiCommandBuffer* cb,
                                    QRhiRenderTarget* rt) noexcept {
    cb_ = cb;
    rt_ = rt;
}

bool Render2dRhi::ensure_resources(QRhiRenderPassDescriptor* rp) {
    if (resources_ready_) {
        return true;
    }
    if (rhi_ == nullptr || rp == nullptr) {
        return false;
    }

    // Buffer dinamico inicial (cresce sob demanda em end_frame). Comeca com
    // espaco pra ~256 vertices (cabe o mapa de teste folgado).
    vbuf_capacity_bytes_ = 256u * kStrideBytes;
    vbuf_.reset(rhi_->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer,
                                vbuf_capacity_bytes_));
    if (!vbuf_->create()) {
        return false;
    }

    // Sem recursos de shader (nenhum uniform/textura no M1): srb vazio.
    srb_.reset(rhi_->newShaderResourceBindings());
    if (!srb_->create()) {
        return false;
    }

    const QShader vs = load_shader(QStringLiteral(":/gusengine/shaders/quad.vert.qsb"));
    const QShader fs = load_shader(QStringLiteral(":/gusengine/shaders/quad.frag.qsb"));
    if (!vs.isValid() || !fs.isValid()) {
        return false;
    }

    pipeline_.reset(rhi_->newGraphicsPipeline());
    pipeline_->setShaderStages({
        {QRhiShaderStage::Vertex, vs},
        {QRhiShaderStage::Fragment, fs},
    });

    QRhiVertexInputLayout layout;
    layout.setBindings({{kStrideBytes}});
    layout.setAttributes({
        {0, 0, QRhiVertexInputAttribute::Float2, 0},  // pos_ndc
        {0, 1, QRhiVertexInputAttribute::Float4,
         2 * static_cast<int>(sizeof(float))},  // cor
    });
    pipeline_->setVertexInputLayout(layout);
    pipeline_->setTopology(QRhiGraphicsPipeline::Triangles);
    pipeline_->setCullMode(QRhiGraphicsPipeline::None);

    // Blend alpha (cores placeholder com alpha; contorno por cima das paredes).
    QRhiGraphicsPipeline::TargetBlend blend;
    blend.enable = true;
    blend.srcColor = QRhiGraphicsPipeline::SrcAlpha;
    blend.dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
    blend.srcAlpha = QRhiGraphicsPipeline::One;
    blend.dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
    pipeline_->setTargetBlends({blend});

    pipeline_->setShaderResourceBindings(srb_.get());
    pipeline_->setRenderPassDescriptor(rp);
    if (!pipeline_->create()) {
        return false;
    }

    resources_ready_ = true;
    return true;
}

void Render2dRhi::push_quad_world(const gus::core::spatial::Rect& world_rect,
                                  const DrawColor& color) {
    const QuadNdc q = build_quad_ndc(world_rect, camera_);
    // 2 triangulos a partir dos cantos [0]=SE [1]=SD [2]=ID [3]=IE:
    //   (0,1,2) e (0,2,3).
    const int order[6] = {0, 1, 2, 0, 2, 3};
    for (int i : order) {
        verts_.push_back(q.corners[i].x);
        verts_.push_back(q.corners[i].y);
        verts_.push_back(color.r);
        verts_.push_back(color.g);
        verts_.push_back(color.b);
        verts_.push_back(color.a);
    }
}

void Render2dRhi::begin_frame(const gus::core::spatial::Rect& camera_world,
                              int pixel_w, int pixel_h) {
    camera_ = camera_world;
    pixel_w_ = pixel_w;
    pixel_h_ = pixel_h;
    verts_.clear();
}

void Render2dRhi::draw_filled_rect(const gus::core::spatial::Rect& world_rect,
                                   const DrawColor& color) {
    push_quad_world(world_rect, color);
}

void Render2dRhi::draw_rect_outline(const gus::core::spatial::Rect& world_rect,
                                    const DrawColor& color, float thickness_world) {
    // Contorno = 4 faixas finas (cima, baixo, esquerda, direita) em mundo.
    const float x = world_rect.x;
    const float y = world_rect.y;
    const float w = world_rect.w;
    const float h = world_rect.h;
    const float t = thickness_world;
    push_quad_world({x, y, w, t}, color);              // borda superior
    push_quad_world({x, y + h - t, w, t}, color);      // borda inferior
    push_quad_world({x, y, t, h}, color);              // borda esquerda
    push_quad_world({x + w - t, y, t, h}, color);      // borda direita
}

void Render2dRhi::end_frame() {
    last_vertex_count_ = static_cast<int>(verts_.size() / kFloatsPerVertex);

    if (cb_ == nullptr || rt_ == nullptr) {
        return;  // sem contexto de frame: nada a gravar (estado degenerado)
    }
    QRhiRenderPassDescriptor* rp = rt_->renderPassDescriptor();
    if (!ensure_resources(rp)) {
        return;  // recursos indisponiveis (ex.: shader ausente): nao crasha
    }

    QRhiResourceUpdateBatch* batch = rhi_->nextResourceUpdateBatch();

    const quint32 needed = static_cast<quint32>(verts_.size() * sizeof(float));
    if (needed > 0) {
        // Cresce o buffer se faltar espaco (raro: o mapa de teste cabe sempre).
        if (needed > vbuf_capacity_bytes_) {
            vbuf_capacity_bytes_ = needed * 2u;
            vbuf_.reset(rhi_->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer,
                                        vbuf_capacity_bytes_));
            vbuf_->create();
        }
        batch->updateDynamicBuffer(vbuf_.get(), 0, needed, verts_.data());
    }

    // Pass: limpa pra um fundo placeholder e desenha os quads acumulados.
    const QColor clear(24, 26, 38);  // cinza-azulado bem escuro (chao "vazio")
    cb_->beginPass(rt_, clear, {1.0f, 0}, batch);
    if (needed > 0 && resources_ready_) {
        cb_->setGraphicsPipeline(pipeline_.get());
        cb_->setViewport(QRhiViewport(0, 0, static_cast<float>(rt_->pixelSize().width()),
                                      static_cast<float>(rt_->pixelSize().height())));
        cb_->setShaderResources(srb_.get());
        const QRhiCommandBuffer::VertexInput vinput(vbuf_.get(), 0);
        cb_->setVertexInput(0, 1, &vinput);
        cb_->draw(static_cast<quint32>(last_vertex_count_));
    }
    cb_->endPass();
}

}  // namespace gus::platform::render2d
