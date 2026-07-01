// gus/platform/render2d/render2d_gl3.hpp
//
// Render2dGl3: implementacao de IRenderer sobre OpenGL 3.3 core (ADR-009 adendo GL3).
// Substitui o Render2dSdl como backend da ARENA quando a janela usa contexto GL (para
// coexistir com o HUD RmlUi-GL3, que precisa de shaders). A API publica e a MESMA
// (IRenderer): a arena, o battle_scene e os testes NAO mudam - so o backend interno
// passa de SDL_Renderer para GL.
//
// PIXEL-ART CRISP: as texturas de sprite usam GL_NEAREST (min/mag), preservando o
// pixel-perfect da arena (D1). O atlas de fonte tambem e NEAREST. Projecao mundo->pixel
// reusa a viewport_transform POCO (mesma matematica do Render2dSdl), convertida para
// coordenadas de clip [-1,1] na CPU; um unico shader desenha quads coloridos e
// texturizados (cor + textura * tint, premultiplied alpha).
//
// COEXISTENCIA: o Render2dGl3 desenha a arena PRIMEIRO (no contexto GL corrente); o HUD
// (RmlUiHud sobre RenderInterface_GL3) compoe DEPOIS, no MESMO contexto; o swap
// (SDL_GL_SwapWindow) e UNICO, feito pelo dono do frame. Como o RmlUi-GL3 salva/restaura
// o estado GL no BeginFrame/EndFrame, o Render2dGl3 mantem seu proprio programa/VAO e
// nao depende do estado deixado pelo HUD.
//
// MODO HEADLESS (gl_active == false): espelha o Render2dSdl(nullptr). NENHUMA funcao GL e
// chamada (nao ha contexto no CI); todos os draws viram no-op contabilizado e
// load_texture devolve kInvalidTexture. Prova que a cadeia monta e roda sem GPU.
//
// O .cpp inclui o loader glad (via RmlUi_Include_GL3.h) e, junto do gl3_loader.cpp (dono da
// impl do glad, ADR-010 F3), e uma das TUs que tocam GL. O HEADER permanece limpo (sem GL
// nem SDL): app/ ve so a interface IRenderer.

#ifndef GUS_PLATFORM_RENDER2D_RENDER2D_GL3_HPP
#define GUS_PLATFORM_RENDER2D_RENDER2D_GL3_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "gus/platform/render2d/font_atlas.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

namespace gus::platform::render2d {

class Render2dGl3 : public IRenderer {
public:
    // gl_active: true = ha um contexto GL corrente (a janela ja criou e fez make-current
    // ANTES de construir o renderer); false = headless (CI/smoke sem GPU): tudo no-op.
    explicit Render2dGl3(bool gl_active) noexcept;
    ~Render2dGl3() override;

    Render2dGl3(const Render2dGl3&) = delete;
    Render2dGl3& operator=(const Render2dGl3&) = delete;

    // IRenderer.
    void begin_frame(const gus::core::spatial::Rect& camera_world, int pixel_w,
                     int pixel_h) override;
    void draw_filled_rect(const gus::core::spatial::Rect& world_rect,
                          const DrawColor& color) override;
    void draw_rect_outline(const gus::core::spatial::Rect& world_rect,
                           const DrawColor& color, float thickness_world) override;
    [[nodiscard]] TextureId load_texture(const char* path) override;
    void draw_textured_rect(const gus::core::spatial::Rect& world_rect,
                            TextureId texture, const UvRect& uv,
                            const DrawColor& tint) override;
    [[nodiscard]] ContentBbox texture_content_bbox(
        TextureId texture) const override;
    void draw_text(const char* text, float x, float y, float px_size,
                   const DrawColor& color, bool bold) override;
    void end_frame() override;

    [[nodiscard]] int last_draw_count() const noexcept { return last_draw_count_; }

    // Present diferido (ADR-009): por padrao end_frame NAO faz swap (o dono do frame
    // chama present() depois do HUD). set_defer_present espelha o Render2dSdl, mas no GL o
    // SWAP e do contexto da janela (SDL_GL_SwapWindow), feito pela casca - aqui present()
    // so marca o ponto; o swap real fica na casca app/. Mantido para simetria de API.
    void set_defer_present(bool defer) noexcept { defer_present_ = defer; }
    [[nodiscard]] bool defer_present() const noexcept { return defer_present_; }
    void present();

private:
    struct Impl;                  // GL handles (programa/VAO/VBO/texturas); PImpl
    std::unique_ptr<Impl> impl_;  // nullptr em headless (gl_active == false)
    bool gl_active_ = false;

    int draw_count_ = 0;
    int last_draw_count_ = 0;
    bool defer_present_ = false;
};

}  // namespace gus::platform::render2d

#endif  // GUS_PLATFORM_RENDER2D_RENDER2D_GL3_HPP
