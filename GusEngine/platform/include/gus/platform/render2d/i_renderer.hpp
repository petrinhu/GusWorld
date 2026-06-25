// gus/platform/render2d/i_renderer.hpp
//
// IRenderer: interface de desenho 2D. ABSTRACAO que isola o backend grafico
// (SDL_Renderer hoje, via Render2dSdl; antes Qt RHI - ver ADR-008) do resto do
// jogo: trocar de backend deve ser ~1 arquivo, e nada alem de platform/render2d/
// enxerga SDL. O app (OverworldSim) fala SO com esta interface.
//
// HEADER limpo (sem <SDL...>): usa tipos proprios (DrawColor + Rect de
// core/spatial), pra que app/ e os testes consumam a interface sem arrastar SDL. A
// impl concreta (Render2dSdl) vive no .cpp da camada platform/, que pode tocar SDL.
//
// CONVENCAO: as coordenadas dos retangulos sao em unidades de MUNDO (nao pixels);
// quem projeta para a tela e o renderer (via a camera passada em begin_frame e a
// matematica de viewport_transform). Cores em [0,1] (RGBA linear simples; sem
// gestao de espaco de cor no M1 - placeholder). 2D puro: sem profundidade, a
// ordem de emissao e a ordem de desenho (painter's order).

#ifndef GUS_PLATFORM_RENDER2D_I_RENDERER_HPP
#define GUS_PLATFORM_RENDER2D_I_RENDERER_HPP

#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect

namespace gus::platform::render2d {

// Cor RGBA em [0,1]. Placeholder: sem sRGB/linear explicito no M1.
struct DrawColor {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

// Handle opaco de textura (sprite). 0 = invalido/ausente. O backend mapeia o
// handle para o recurso de GPU; o app so guarda o numero. Devolvido por
// load_texture e consumido por draw_textured_rect.
using TextureId = unsigned int;
inline constexpr TextureId kInvalidTexture = 0;

// Regiao de uma textura em coordenadas normalizadas [0,1] (UV). Cobre uma sub-
// imagem (ex.: 1 frame de uma folha) ou a textura inteira ({0,0,1,1}). Origem
// (0,0) = canto superior-esquerdo da textura (mesmo eixo do mundo: +V para baixo).
struct UvRect {
    float u = 0.0f;
    float v = 0.0f;
    float w = 1.0f;
    float h = 1.0f;
};

// Caixa do CONTEUDO nao-transparente de uma textura, em PIXELS (o alpha-bbox).
// Medida pelo backend ao carregar (decodifica o alpha do PNG). Serve pra ancorar
// o sprite pelos PES REAIS (a margem inferior transparente = canvas_h - (top+h)):
// ver app/screens/sprite_anchor.hpp e o M1-BUG.SUL. valid() = false quando o
// backend nao decodifica (headless/Null) ou a textura e invalida - o chamador
// degrada pro anchor legado (margem 0). Origem (0,0) = topo-esquerda do canvas.
struct ContentBbox {
    int canvas_w = 0;  // largura total do canvas (px)
    int canvas_h = 0;  // altura total do canvas (px)
    int left = 0;      // 1o pixel nao-transparente (x)
    int top = 0;       // 1a linha nao-transparente (y)
    int width = 0;     // largura do conteudo (px); 0 = nada/tudo transparente
    int height = 0;    // altura do conteudo (px)

    // false = sem medicao confiavel (headless/textura invalida): use anchor legado.
    [[nodiscard]] bool valid() const noexcept {
        return canvas_w > 0 && canvas_h > 0 && width > 0 && height > 0;
    }

    // Margem inferior TRANSPARENTE em px (espaco vazio abaixo do conteudo). E o que
    // o anchor desconta pra colar o pe real na base da AABB. 0 se invalido.
    [[nodiscard]] int bottom_margin() const noexcept {
        if (!valid()) {
            return 0;
        }
        const int content_bottom = top + height;
        const int m = canvas_h - content_bottom;
        return m > 0 ? m : 0;
    }
};

// Interface de render 2D. O ciclo e: begin_frame -> N draws -> end_frame.
class IRenderer {
public:
    virtual ~IRenderer() = default;

    // Abre o frame. camera_world e a janela visivel em mundo (de clamp_camera);
    // pixel_w/h sao o tamanho do alvo em pixels (pra aspect/escala do backend).
    virtual void begin_frame(const gus::core::spatial::Rect& camera_world,
                             int pixel_w, int pixel_h) = 0;

    // Retangulo de mundo preenchido (ex.: celula de parede).
    virtual void draw_filled_rect(const gus::core::spatial::Rect& world_rect,
                                  const DrawColor& color) = 0;

    // Contorno de retangulo de mundo (ex.: hitbox do jogador), espessura em
    // unidades de mundo. O backend pode aproximar a espessura.
    virtual void draw_rect_outline(const gus::core::spatial::Rect& world_rect,
                                   const DrawColor& color,
                                   float thickness_world) = 0;

    // Carrega uma textura de um arquivo de imagem (PNG com alpha). Devolve um
    // TextureId valido, ou kInvalidTexture se falhar (arquivo ausente, backend
    // sem suporte a textura - ex.: smoke offscreen com backend Null). O chamador
    // DEVE tolerar kInvalidTexture (degradar para o desenho de retangulo). Pode
    // ser chamado FORA de um frame (carregamento preguicoso/inicializacao).
    // Idempotente por caminho: o mesmo path devolve o mesmo handle (cache).
    [[nodiscard]] virtual TextureId load_texture(const char* path) = 0;

    // Desenha um quad de MUNDO texturizado (sprite). uv recorta a sub-imagem
    // (folha de frames). tint multiplica a cor amostrada (use branco opaco
    // {1,1,1,1} para "sem tinta"). Alpha-blend pelo alpha da textura * tint.a.
    // Se texture == kInvalidTexture, e um no-op (nada e desenhado): cabe ao
    // chamador desenhar um fallback antes. NEAREST sampling (pixel-art crisp).
    virtual void draw_textured_rect(const gus::core::spatial::Rect& world_rect,
                                    TextureId texture, const UvRect& uv,
                                    const DrawColor& tint) = 0;

    // Caixa do CONTEUDO nao-transparente (alpha > 0) de uma textura ja carregada,
    // medida no decode do PNG e cacheada por TextureId. Serve pra ANCORAR o sprite
    // pelos PES REAIS: o anchor desconta a margem inferior transparente
    // (ContentBbox::bottom_margin) pra colar o pe na base da AABB (M1-BUG.SUL),
    // SEM numero magico - cada sprite/direcao mede a propria sobra. Devolve um
    // ContentBbox com valid()==false quando o handle e invalido ou o backend nao
    // decodifica (headless/Null): o chamador degrada pro anchor legado (margem 0).
    [[nodiscard]] virtual ContentBbox texture_content_bbox(
        TextureId texture) const = 0;

    // Desenha TEXTO (M5, incremento 3.5) na fonte monospace embarcada (Pixel Operator
    // Mono). pos (x,y) = canto superior-esquerdo da 1a celula, em coordenadas de MUNDO
    // (na BattleScreen o mundo == px logico 640x360, entao e px de tela logico).
    // px_size = altura da celula em unidades de mundo; o backend escala o glifo do atlas
    // (NEAREST, crisp). color tinge o glifo (alpha incluso). bold usa a face Bold
    // (enfase: criticos/sistema no log). Se a fonte nao carregou (headless/CI sem
    // assets), e um NO-OP seguro (o chamador desenhou um fallback antes - barras/marcas).
    virtual void draw_text(const char* text, float x, float y, float px_size,
                           const DrawColor& color, bool bold) = 0;

    // Fecha o frame (submete ao backend / swap).
    virtual void end_frame() = 0;
};

}  // namespace gus::platform::render2d

#endif  // GUS_PLATFORM_RENDER2D_I_RENDERER_HPP
