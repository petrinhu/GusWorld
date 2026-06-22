// gus/platform/render2d/render2d_rhi.hpp
//
// Render2dRhi: a implementacao concreta de IRenderer sobre Qt RHI (M1). ESTE e o
// UNICO ponto que toca QRhi (risco R1 do design: API semi-privada; encapsulada
// aqui, trocar de backend e ~1 arquivo). O resto do jogo fala so com IRenderer.
//
// COMO FUNCIONA: acumula os quads desenhados no frame como vertices JA EM NDC (a
// projecao mundo->NDC e a tested viewport_transform, na CPU, sem GPU), num buffer
// dinamico, e grava UM render pass com UM draw por frame (poucos quads no M1).
// Vertice = vec2 pos_ndc + vec4 cor (6 floats). Retangulo preenchido = 2
// triangulos (6 verts); contorno = 4 faixas finas (24 verts). 2D puro, sem
// profundidade.
//
// CONTEXTO DE FRAME: o owner (janela ou harness de smoke) controla o
// begin/endFrame do QRhi (swapchain no modo janela, offscreen no modo --smoke) e
// informa o command buffer + render target via set_frame_context() ANTES de
// begin_frame(). Assim o mesmo renderer serve janela e offscreen sem saber qual.
//
// Inclui <rhi/qrhi.h> (camada platform/, Qt/QRhi permitido). NAO testavel sem um
// QRhi, mas o backend Null (render2d_rhi_test) exercita toda a API headless.

#ifndef GUS_PLATFORM_RENDER2D_RENDER2D_RHI_HPP
#define GUS_PLATFORM_RENDER2D_RENDER2D_RHI_HPP

#include <vector>

#include <rhi/qrhi.h>

#include "gus/platform/render2d/i_renderer.hpp"

namespace gus::platform::render2d {

class Render2dRhi : public IRenderer {
public:
    // rhi: o device QRhi (NAO assumido owner; vive enquanto o renderer viver). Os
    // recursos GPU (pipeline/buffer/srb) sao criados preguicosamente no 1o frame,
    // contra o render pass descriptor do render target informado.
    explicit Render2dRhi(QRhi* rhi);
    ~Render2dRhi() override;

    Render2dRhi(const Render2dRhi&) = delete;
    Render2dRhi& operator=(const Render2dRhi&) = delete;

    // Informa o contexto do frame corrente (command buffer + alvo de render).
    // Chamado pelo owner DEPOIS de beginFrame/beginOffscreenFrame e ANTES de
    // begin_frame(). O renderer NAO controla o ciclo de frame do QRhi.
    void set_frame_context(QRhiCommandBuffer* cb, QRhiRenderTarget* rt) noexcept;

    // IRenderer.
    void begin_frame(const gus::core::spatial::Rect& camera_world, int pixel_w,
                     int pixel_h) override;
    void draw_filled_rect(const gus::core::spatial::Rect& world_rect,
                          const DrawColor& color) override;
    void draw_rect_outline(const gus::core::spatial::Rect& world_rect,
                           const DrawColor& color, float thickness_world) override;
    void end_frame() override;

    // Numero de vertices acumulados no ultimo end_frame (debug/teste).
    [[nodiscard]] int last_vertex_count() const noexcept { return last_vertex_count_; }

private:
    // Cria pipeline + buffer + srb uma vez (idempotente), contra o render pass do
    // render target atual. Devolve false se algo falhar.
    bool ensure_resources(QRhiRenderPassDescriptor* rp);
    // Empilha um quad (4 cantos NDC, ordem do build_quad_ndc) como 2 triangulos.
    void push_quad_world(const gus::core::spatial::Rect& world_rect,
                         const DrawColor& color);

    QRhi* rhi_ = nullptr;
    QRhiCommandBuffer* cb_ = nullptr;
    QRhiRenderTarget* rt_ = nullptr;

    // Recursos GPU (owner; liberados no dtor). std::unique_ptr com deletar do Qt.
    std::unique_ptr<QRhiBuffer> vbuf_;
    std::unique_ptr<QRhiShaderResourceBindings> srb_;
    std::unique_ptr<QRhiGraphicsPipeline> pipeline_;
    bool resources_ready_ = false;
    quint32 vbuf_capacity_bytes_ = 0;

    // Acumulador CPU de vertices do frame (intercalado: x,y,r,g,b,a por vertice).
    std::vector<float> verts_;
    gus::core::spatial::Rect camera_{};
    int pixel_w_ = 0;
    int pixel_h_ = 0;
    int last_vertex_count_ = 0;
};

}  // namespace gus::platform::render2d

#endif  // GUS_PLATFORM_RENDER2D_RENDER2D_RHI_HPP
