// gus/platform/src/render2d/render2d_rhi.cpp
//
// Implementacao do Render2dRhi (M1). Ver header. Caminho irredutivel de GPU;
// exercitado headless por platform/tests/render2d_rhi_test.cpp (backend Null) e
// pelo smoke offscreen do app. A LOGICA de projecao (build_quad_ndc) e tested a
// parte (viewport_transform_test).

#include "gus/platform/render2d/render2d_rhi.hpp"

#include <QColor>
#include <QFile>
#include <QImage>

#include <rhi/qshader.h>  // QShader (camada platform/, QRhi/private permitido)

#include "gus/platform/render2d/viewport_transform.hpp"

namespace gus::platform::render2d {

namespace {

constexpr int kFloatsPerVertex = 6;  // x,y (NDC) + r,g,b,a
constexpr int kStrideBytes = kFloatsPerVertex * static_cast<int>(sizeof(float));

// Caminho TEXTURIZADO: x,y (NDC) + u,v + r,g,b,a (tint) = 8 floats por vertice.
constexpr int kSpriteFloatsPerVertex = 8;
constexpr int kSpriteStrideBytes =
    kSpriteFloatsPerVertex * static_cast<int>(sizeof(float));

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

bool Render2dRhi::ensure_sprite_resources(QRhiRenderPassDescriptor* rp) {
    if (sprite_resources_ready_) {
        return true;
    }
    if (rhi_ == nullptr || rp == nullptr) {
        return false;
    }

    sprite_vbuf_capacity_bytes_ = 256u * kSpriteStrideBytes;
    sprite_vbuf_.reset(rhi_->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer,
                                       sprite_vbuf_capacity_bytes_));
    if (!sprite_vbuf_->create()) {
        return false;
    }

    // Sampler NEAREST (pixel-art crisp, sem suavizar) + clamp (sem repetir borda).
    sampler_.reset(rhi_->newSampler(QRhiSampler::Nearest, QRhiSampler::Nearest,
                                    QRhiSampler::None, QRhiSampler::ClampToEdge,
                                    QRhiSampler::ClampToEdge));
    if (!sampler_->create()) {
        return false;
    }

    const QShader vs = load_shader(QStringLiteral(":/gusengine/shaders/sprite.vert.qsb"));
    const QShader fs = load_shader(QStringLiteral(":/gusengine/shaders/sprite.frag.qsb"));
    if (!vs.isValid() || !fs.isValid()) {
        return false;
    }

    // O pipeline precisa de UM srb de layout-compativel pra criar (qualquer textura
    // serve de molde; cada textura tem o seu srb real em Texture::srb). Como ainda
    // pode nao haver textura criada, usamos um srb com a 1a textura disponivel, ou
    // adiamos: aqui exigimos que ao menos uma textura ja exista (load_texture roda
    // antes do 1o draw). Se nenhuma existir, nao da pra criar o pipeline ainda.
    QRhiShaderResourceBindings* layout_srb = nullptr;
    for (auto& t : textures_) {
        if (t.srb) {
            layout_srb = t.srb.get();
            break;
        }
    }
    if (layout_srb == nullptr) {
        return false;  // sem textura ainda: tenta de novo quando houver
    }

    sprite_pipeline_.reset(rhi_->newGraphicsPipeline());
    sprite_pipeline_->setShaderStages({
        {QRhiShaderStage::Vertex, vs},
        {QRhiShaderStage::Fragment, fs},
    });

    QRhiVertexInputLayout layout;
    layout.setBindings({{kSpriteStrideBytes}});
    layout.setAttributes({
        {0, 0, QRhiVertexInputAttribute::Float2, 0},  // pos_ndc
        {0, 1, QRhiVertexInputAttribute::Float2,
         2 * static_cast<int>(sizeof(float))},  // uv
        {0, 2, QRhiVertexInputAttribute::Float4,
         4 * static_cast<int>(sizeof(float))},  // tint
    });
    sprite_pipeline_->setVertexInputLayout(layout);
    sprite_pipeline_->setTopology(QRhiGraphicsPipeline::Triangles);
    sprite_pipeline_->setCullMode(QRhiGraphicsPipeline::None);

    // Alpha blend (fundo transparente do PNG recortado pelo alpha da textura).
    QRhiGraphicsPipeline::TargetBlend blend;
    blend.enable = true;
    blend.srcColor = QRhiGraphicsPipeline::SrcAlpha;
    blend.dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
    blend.srcAlpha = QRhiGraphicsPipeline::One;
    blend.dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
    sprite_pipeline_->setTargetBlends({blend});

    sprite_pipeline_->setShaderResourceBindings(layout_srb);
    sprite_pipeline_->setRenderPassDescriptor(rp);
    if (!sprite_pipeline_->create()) {
        return false;
    }

    sprite_resources_ready_ = true;
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

TextureId Render2dRhi::load_texture(const char* path) {
    if (path == nullptr || rhi_ == nullptr) {
        return kInvalidTexture;
    }
    const std::string key(path);

    // Cache por caminho (idempotente): mesmo arquivo -> mesmo handle.
    const auto it = texture_by_path_.find(key);
    if (it != texture_by_path_.end()) {
        return it->second;
    }

    // Carrega os pixels do disco (QImage). Falha de arquivo -> textura invalida
    // (o chamador degrada para o retangulo). RGBA8 premultiplicado? Nao: usamos
    // Format_RGBA8888 (nao-premult) e o blend SrcAlpha/OneMinusSrcAlpha combina.
    QImage img(QString::fromStdString(key));
    if (img.isNull()) {
        return kInvalidTexture;  // arquivo ausente/ilegivel: degrada
    }
    img = img.convertToFormat(QImage::Format_RGBA8888);

    // Cria o recurso de GPU. O backend Null (smoke offscreen) cria texturas
    // "fantasma" que funcionam pra API mas nao desenham nada: ainda assim
    // devolvemos um handle valido (o smoke so exercita o caminho, sem checar pixel).
    std::unique_ptr<QRhiTexture> tex(rhi_->newTexture(
        QRhiTexture::RGBA8, QSize(img.width(), img.height()), 1));
    if (tex == nullptr || !tex->create()) {
        return kInvalidTexture;  // backend sem suporte a textura: degrada
    }

    // O sampler vive em ensure_sprite_resources; se ainda nao existe, cria agora
    // (precisamos dele pro srb desta textura). Idempotente.
    if (!sampler_) {
        sampler_.reset(rhi_->newSampler(QRhiSampler::Nearest, QRhiSampler::Nearest,
                                        QRhiSampler::None, QRhiSampler::ClampToEdge,
                                        QRhiSampler::ClampToEdge));
        if (!sampler_->create()) {
            return kInvalidTexture;
        }
    }

    std::unique_ptr<QRhiShaderResourceBindings> srb(rhi_->newShaderResourceBindings());
    srb->setBindings({
        QRhiShaderResourceBinding::sampledTexture(
            0, QRhiShaderResourceBinding::FragmentStage, tex.get(), sampler_.get()),
    });
    if (!srb->create()) {
        return kInvalidTexture;
    }

    Texture entry;
    entry.tex = std::move(tex);
    entry.srb = std::move(srb);
    entry.uploaded = false;
    entry.pending = std::move(img);

    textures_.push_back(std::move(entry));
    const TextureId id = static_cast<TextureId>(textures_.size());  // 1-based
    texture_by_path_[key] = id;
    return id;
}

void Render2dRhi::draw_textured_rect(const gus::core::spatial::Rect& world_rect,
                                     TextureId texture, const UvRect& uv,
                                     const DrawColor& tint) {
    if (texture == kInvalidTexture || texture > textures_.size()) {
        return;  // sem textura: no-op (o chamador desenha o fallback)
    }
    const QuadNdc q = build_quad_ndc(world_rect, camera_);
    // UV por canto, na MESMA ordem dos cantos NDC ([0]=SE [1]=SD [2]=ID [3]=IE).
    // V cresce para baixo (origem no topo), igual ao mundo: SE=(u,v), ID=(u+w,v+h).
    const float u0 = uv.u, v0 = uv.v, u1 = uv.u + uv.w, v1 = uv.v + uv.h;
    const float cu[4] = {u0, u1, u1, u0};
    const float cv[4] = {v0, v0, v1, v1};

    const int first = static_cast<int>(sprite_verts_.size() / kSpriteFloatsPerVertex);
    const int order[6] = {0, 1, 2, 0, 2, 3};
    for (int i : order) {
        sprite_verts_.push_back(q.corners[i].x);
        sprite_verts_.push_back(q.corners[i].y);
        sprite_verts_.push_back(cu[i]);
        sprite_verts_.push_back(cv[i]);
        sprite_verts_.push_back(tint.r);
        sprite_verts_.push_back(tint.g);
        sprite_verts_.push_back(tint.b);
        sprite_verts_.push_back(tint.a);
    }
    sprite_draws_.push_back({texture, first, 6});
}

void Render2dRhi::begin_frame(const gus::core::spatial::Rect& camera_world,
                              int pixel_w, int pixel_h) {
    camera_ = camera_world;
    pixel_w_ = pixel_w;
    pixel_h_ = pixel_h;
    verts_.clear();
    sprite_verts_.clear();
    sprite_draws_.clear();
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

    // --- SPRITES: upload de pixels pendentes + buffer de vertices ----------
    // Tenta criar os recursos de sprite (pode falhar se nenhuma textura existir
    // ainda - nesse caso simplesmente nao ha sprite a desenhar neste frame).
    const bool sprite_ok = !sprite_draws_.empty() && ensure_sprite_resources(rp);
    if (sprite_ok) {
        // Sobe os pixels de cada textura ainda nao enviada (uma vez por textura).
        for (auto& t : textures_) {
            if (!t.uploaded && !t.pending.isNull() && t.tex) {
                QRhiTextureSubresourceUploadDescription sub(t.pending);
                QRhiTextureUploadDescription up({0, 0, sub});
                batch->uploadTexture(t.tex.get(), up);
                t.uploaded = true;
                t.pending = QImage();  // libera os pixels da CPU
            }
        }
        const quint32 s_needed =
            static_cast<quint32>(sprite_verts_.size() * sizeof(float));
        if (s_needed > sprite_vbuf_capacity_bytes_) {
            sprite_vbuf_capacity_bytes_ = s_needed * 2u;
            sprite_vbuf_.reset(rhi_->newBuffer(
                QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer,
                sprite_vbuf_capacity_bytes_));
            sprite_vbuf_->create();
        }
        if (s_needed > 0) {
            batch->updateDynamicBuffer(sprite_vbuf_.get(), 0, s_needed,
                                       sprite_verts_.data());
        }
    }

    // Pass: limpa pra um fundo placeholder e desenha os quads acumulados.
    const QColor clear(24, 26, 38);  // cinza-azulado bem escuro (chao "vazio")
    cb_->beginPass(rt_, clear, {1.0f, 0}, batch);

    const QRhiViewport vp(0, 0, static_cast<float>(rt_->pixelSize().width()),
                          static_cast<float>(rt_->pixelSize().height()));

    // 1) Quads coloridos (paredes, fallback) primeiro.
    if (needed > 0 && resources_ready_) {
        cb_->setGraphicsPipeline(pipeline_.get());
        cb_->setViewport(vp);
        cb_->setShaderResources(srb_.get());
        const QRhiCommandBuffer::VertexInput vinput(vbuf_.get(), 0);
        cb_->setVertexInput(0, 1, &vinput);
        cb_->draw(static_cast<quint32>(last_vertex_count_));
    }

    // 2) Sprites por cima (painter order), um draw por comando trocando o srb da
    //    textura. Offset de vertice em bytes no MESMO buffer de sprite.
    if (sprite_ok && sprite_resources_ready_) {
        cb_->setGraphicsPipeline(sprite_pipeline_.get());
        cb_->setViewport(vp);
        for (const SpriteDraw& d : sprite_draws_) {
            if (d.texture == kInvalidTexture || d.texture > textures_.size()) {
                continue;
            }
            QRhiShaderResourceBindings* srb = textures_[d.texture - 1].srb.get();
            if (srb == nullptr) {
                continue;
            }
            cb_->setShaderResources(srb);
            const quint32 offset =
                static_cast<quint32>(d.first_vertex) * kSpriteStrideBytes;
            const QRhiCommandBuffer::VertexInput vinput(sprite_vbuf_.get(), offset);
            cb_->setVertexInput(0, 1, &vinput);
            cb_->draw(static_cast<quint32>(d.vertex_count));
        }
    }

    cb_->endPass();
}

}  // namespace gus::platform::render2d
