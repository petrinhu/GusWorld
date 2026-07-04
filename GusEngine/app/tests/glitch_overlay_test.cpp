// GusEngine/app/tests/glitch_overlay_test.cpp
//
// Catch2 do desenho do GLITCH DIGITAL/"PROCESSANDO" (M7-COSTURA Inc 2b) - a funcao
// draw_glitch_overlay substitui o retangulo preto liso da transicao cidade<->
// batalha. Headless: usa um IRenderer FALSO (mesmo padrao de overworld_sim_test.
// cpp) que so registra as chamadas de draw_filled_rect, sem GPU/janela. Prova o
// CONTRATO de desenho (quantas celulas, extremos seguros do envelope) - o VISUAL
// em si (cores, timing "ao vivo") o lider/Gus Dragon validam jogando.

#include <catch2/catch_test_macros.hpp>

#include "gus/app/glitch_overlay.hpp"
#include "gus/core/anim/glitch_dissolve.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

using gus::core::spatial::Rect;
using gus::platform::render2d::ContentBbox;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::IRenderer;
using gus::platform::render2d::kInvalidTexture;
using gus::platform::render2d::TextureId;
using gus::platform::render2d::UvRect;

namespace {

// IRenderer falso: so registra draw_filled_rect (o unico primitivo que
// draw_glitch_overlay usa - factibilidade em QUALQUER backend, ver o header).
class FakeRenderer : public IRenderer {
public:
    struct Cmd {
        Rect rect;
        DrawColor color;
    };

    void begin_frame(const Rect&, int, int) override {}
    void draw_filled_rect(const Rect& world_rect, const DrawColor& c) override {
        cmds.push_back({world_rect, c});
    }
    void draw_rect_outline(const Rect&, const DrawColor&, float) override {}
    TextureId load_texture(const char*) override { return kInvalidTexture; }
    void draw_textured_rect(const Rect&, TextureId, const UvRect&,
                            const DrawColor&) override {}
    ContentBbox texture_content_bbox(TextureId) const override {
        return ContentBbox{};
    }
    void draw_text(const char*, float, float, float, const DrawColor&,
                   bool) override {}
    void end_frame() override {}

    std::vector<Cmd> cmds;
};

constexpr Rect kScreen{0.0f, 0.0f, 960.0f, 540.0f};  // battle_screen_rect() real

}  // namespace

TEST_CASE("draw_glitch_overlay: alpha<=0 nao desenha nada (tela limpa)",
          "[glitch_overlay]") {
    FakeRenderer r;
    gus::app::draw_glitch_overlay(r, kScreen, 0.0f);
    REQUIRE(r.cmds.empty());
}

TEST_CASE("draw_glitch_overlay: alpha>=1 cobre a grade INTEIRA (invariante de "
          "seguranca da troca de renderer)",
          "[glitch_overlay]") {
    FakeRenderer r;
    gus::app::draw_glitch_overlay(r, kScreen, 1.0f);
    REQUIRE(r.cmds.size() ==
            static_cast<std::size_t>(gus::core::anim::kGlitchGridCols *
                                      gus::core::anim::kGlitchGridRows));
    // Toda celula opaca (alpha==1) - nenhum "buraco" deixaria a troca de renderer
    // visivel.
    for (const auto& cmd : r.cmds) {
        REQUIRE(cmd.color.a >= 0.99f);
    }
}

TEST_CASE("draw_glitch_overlay: no pico (alpha=1) NENHUMA celula fica deslocada "
          "(x bate exatamente com a grade regular)",
          "[glitch_overlay]") {
    FakeRenderer r;
    gus::app::draw_glitch_overlay(r, kScreen, 1.0f);
    const float cell_w = kScreen.w / static_cast<float>(gus::core::anim::kGlitchGridCols);
    for (const auto& cmd : r.cmds) {
        const float col_f = (cmd.rect.x - kScreen.x) / cell_w;
        const float rounded = static_cast<float>(static_cast<int>(col_f + 0.5f));
        REQUIRE(std::abs(col_f - rounded) < 0.01f);
    }
}

TEST_CASE("draw_glitch_overlay: alpha intermediario desenha MENOS que a grade "
          "inteira (dissolve em blocos, nao um retangulo unico)",
          "[glitch_overlay]") {
    FakeRenderer r;
    gus::app::draw_glitch_overlay(r, kScreen, 0.5f);
    const int total = gus::core::anim::kGlitchGridCols * gus::core::anim::kGlitchGridRows;
    REQUIRE(r.cmds.size() < static_cast<std::size_t>(total));
    // ...mas alguma coisa ja apareceu (nao e um "tudo ou nada").
    REQUIRE_FALSE(r.cmds.empty());
}

TEST_CASE("draw_glitch_overlay: todas as celulas desenhadas ficam DENTRO do "
          "screen_rect (nenhum bloco vaza pra fora da tela)",
          "[glitch_overlay]") {
    FakeRenderer r;
    gus::app::draw_glitch_overlay(r, kScreen, 0.7f);
    REQUIRE_FALSE(r.cmds.empty());
    for (const auto& cmd : r.cmds) {
        REQUIRE(cmd.rect.x + cmd.rect.w >= kScreen.x);
        REQUIRE(cmd.rect.x <= kScreen.x + kScreen.w);
        REQUIRE(cmd.rect.y >= kScreen.y - 1.0f);
        REQUIRE(cmd.rect.y + cmd.rect.h <= kScreen.y + kScreen.h + 1.0f);
    }
}
