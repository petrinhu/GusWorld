// GusEngine/app/tests/boot_pixel_overlay_test.cpp
//
// Catch2 do carregamento/desenho da SEQUENCIA DE FRAMES pre-renderizada (M7-COSTURA
// Inc 2c) - BootPixelOverlay substitui draw_glitch_overlay (o glitch procedural foi
// VETADO pelo lider ao vivo: "pareceu bug"; a sequencia de frames FOI aprovada).
// Headless: IRenderer FALSO (mesmo padrao de overworld_sim_test.cpp/glitch_overlay_
// test.cpp aposentado) - load_texture do fake NAO le disco, so simula sucesso/falha.
// Prova o CONTRATO (quantos frames carrega, invariante de seguranca solida por baixo,
// degradacao segura se faltar asset, a perna/leg certa escolhe o frame certo) - o
// VISUAL de verdade (as imagens reais, o timing "ao vivo") o lider valida jogando.
//
// CUIDADO nos valores de `t` escolhidos abaixo: boot_pixel_safety_alpha(leg,t) e 0
// EXATAMENTE em t=0 pras pernas "Darkening" (a tela ainda nao comecou a cobrir) e em
// t=1 pras pernas "Revealing" (a tela acabou de revelar 100%) - draw() NAO desenha
// NADA nesse alpha<=0 (mesmo invariante do fade/glitch antigos). Os testes que querem
// provar o FRAME mostrado usam um `t` logo AO LADO do extremo (ex.: 0.01 ou 0.99), no
// campo onde o overlay ainda desenha algo.

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "gus/app/boot_pixel_overlay.hpp"
#include "gus/core/anim/boot_pixel_sequence.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

using gus::core::anim::BootPixelLeg;
using gus::core::spatial::Rect;
using gus::platform::render2d::ContentBbox;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::IRenderer;
using gus::platform::render2d::kInvalidTexture;
using gus::platform::render2d::TextureId;
using gus::platform::render2d::UvRect;

namespace {

// IRenderer falso: registra draw_filled_rect + draw_textured_rect; load_texture
// simula sucesso (handle incremental != 0), OU falha a partir do (fail_after+1)-esimo
// load (simula asset ausente/truncado no disco) quando fail_after >= 0.
class FakeRenderer : public IRenderer {
public:
    struct FilledCmd {
        Rect rect;
        DrawColor color;
    };
    struct TexturedCmd {
        Rect rect;
        TextureId tex;
        UvRect uv;
        DrawColor tint;
    };

    explicit FakeRenderer(int fail_after = -1) : fail_after_(fail_after) {}

    void begin_frame(const Rect&, int, int) override {}
    void draw_filled_rect(const Rect& world_rect, const DrawColor& c) override {
        filled.push_back({world_rect, c});
    }
    void draw_rect_outline(const Rect&, const DrawColor&, float) override {}
    TextureId load_texture(const char*) override {
        ++load_calls;
        if (fail_after_ >= 0 && load_calls > fail_after_) {
            return kInvalidTexture;
        }
        return static_cast<TextureId>(load_calls);  // handle "valido" incremental
    }
    void draw_textured_rect(const Rect& world_rect, TextureId tex, const UvRect& uv,
                            const DrawColor& tint) override {
        textured.push_back({world_rect, tex, uv, tint});
    }
    ContentBbox texture_content_bbox(TextureId) const override {
        return ContentBbox{};
    }
    void draw_text(const char*, float, float, float, const DrawColor&,
                   bool) override {}
    void end_frame() override {}

    std::vector<FilledCmd> filled;
    std::vector<TexturedCmd> textured;
    int load_calls = 0;
    int fail_after_;
};

constexpr Rect kScreen{0.0f, 0.0f, 960.0f, 540.0f};  // battle_screen_rect() real

}  // namespace

TEST_CASE("BootPixelOverlay::load: TODOS os frames OK -> ready()==true",
          "[boot_pixel_overlay]") {
    FakeRenderer r;
    gus::app::BootPixelOverlay overlay;
    REQUIRE(overlay.load(r, "resources/vfx/boot_pixel"));
    REQUIRE(overlay.ready());
    REQUIRE(r.load_calls == gus::core::anim::kBootPixelFrameCount);
}

TEST_CASE("BootPixelOverlay::load: 1+ frame falhando -> ready()==false "
          "(degradacao segura, nunca sequencia PARCIAL)",
          "[boot_pixel_overlay]") {
    FakeRenderer r(/*fail_after=*/10);  // os 10 primeiros OK, o resto falha
    gus::app::BootPixelOverlay overlay;
    REQUIRE_FALSE(overlay.load(r, "resources/vfx/boot_pixel"));
    REQUIRE_FALSE(overlay.ready());
}

TEST_CASE("BootPixelOverlay: ready()==false antes de qualquer load()",
          "[boot_pixel_overlay]") {
    gus::app::BootPixelOverlay overlay;
    REQUIRE_FALSE(overlay.ready());
}

TEST_CASE("BootPixelOverlay::draw: kToBattleDarkening em t=0 (nada comecou a "
          "cobrir ainda) nao desenha nada - tela limpa",
          "[boot_pixel_overlay]") {
    FakeRenderer r;
    gus::app::BootPixelOverlay overlay;
    overlay.load(r, "resources/vfx/boot_pixel");
    overlay.draw(r, kScreen, BootPixelLeg::kToBattleDarkening, 0.0f);
    REQUIRE(r.filled.empty());
    REQUIRE(r.textured.empty());
}

TEST_CASE("BootPixelOverlay::draw: kToBattleRevealing em t=1 (revelou 100%) nao "
          "desenha nada - tela limpa",
          "[boot_pixel_overlay]") {
    FakeRenderer r;
    gus::app::BootPixelOverlay overlay;
    overlay.load(r, "resources/vfx/boot_pixel");
    overlay.draw(r, kScreen, BootPixelLeg::kToBattleRevealing, 1.0f);
    REQUIRE(r.filled.empty());
    REQUIRE(r.textured.empty());
}

TEST_CASE("BootPixelOverlay::draw: alpha>0 com frames OK desenha o SOLIDO de "
          "seguranca + o frame textured (nesta ordem, 1 de cada)",
          "[boot_pixel_overlay]") {
    FakeRenderer r;
    gus::app::BootPixelOverlay overlay;
    overlay.load(r, "resources/vfx/boot_pixel");
    overlay.draw(r, kScreen, BootPixelLeg::kToBattleDarkening, 0.5f);
    REQUIRE(r.filled.size() == 1);
    REQUIRE(r.textured.size() == 1);
    REQUIRE(r.textured[0].tex != kInvalidTexture);
}

TEST_CASE("BootPixelOverlay::draw: sem frames (load falhou/nunca chamado) SO o "
          "solido de seguranca desenha (nunca crasha, nunca mostra textura "
          "invalida/sequencia quebrada)",
          "[boot_pixel_overlay]") {
    FakeRenderer r;
    gus::app::BootPixelOverlay overlay;  // load() NUNCA chamado
    overlay.draw(r, kScreen, BootPixelLeg::kToBattleDarkening, 0.5f);
    REQUIRE(r.filled.size() == 1);
    REQUIRE(r.textured.empty());
}

TEST_CASE("BootPixelOverlay::draw: o solido de seguranca cobre a TELA INTEIRA "
          "(nenhuma fresta na troca de renderer)",
          "[boot_pixel_overlay]") {
    FakeRenderer r;
    gus::app::BootPixelOverlay overlay;
    overlay.load(r, "resources/vfx/boot_pixel");
    overlay.draw(r, kScreen, BootPixelLeg::kToBattleDarkening, 0.3f);
    REQUIRE(r.filled[0].rect.x == kScreen.x);
    REQUIRE(r.filled[0].rect.y == kScreen.y);
    REQUIRE(r.filled[0].rect.w == kScreen.w);
    REQUIRE(r.filled[0].rect.h == kScreen.h);
}

TEST_CASE("BootPixelOverlay::draw: kToBattleRevealing perto do fim (t=0.99) "
          "mostra o ULTIMO frame da sequencia (fecha em 'SYSTEM READY.' "
          "exatamente quando a arena esta quase 100% visivel)",
          "[boot_pixel_overlay]") {
    FakeRenderer r;
    gus::app::BootPixelOverlay overlay;
    overlay.load(r, "resources/vfx/boot_pixel");
    overlay.draw(r, kScreen, BootPixelLeg::kToBattleRevealing, 0.99f);
    // O ultimo load_texture bem-sucedido devolveu o handle kBootPixelFrameCount (20,
    // 1-based via FakeRenderer::load_calls) - o frame nesse instante deve ser esse.
    REQUIRE(r.textured[0].tex ==
            static_cast<TextureId>(gus::core::anim::kBootPixelFrameCount));
}

TEST_CASE("BootPixelOverlay::draw: kToBattleDarkening logo no comeco (t=0.01) "
          "mostra o PRIMEIRO frame da sequencia",
          "[boot_pixel_overlay]") {
    FakeRenderer r;
    gus::app::BootPixelOverlay overlay;
    overlay.load(r, "resources/vfx/boot_pixel");
    overlay.draw(r, kScreen, BootPixelLeg::kToBattleDarkening, 0.01f);
    REQUIRE(r.textured[0].tex == static_cast<TextureId>(1));  // 1o frame carregado
}

TEST_CASE("BootPixelOverlay::draw: kFromBattleRevealing quase no fim (t=0.99, "
          "cidade quase totalmente de volta) mostra o PRIMEIRO frame (tela "
          "escura/cursor)",
          "[boot_pixel_overlay]") {
    FakeRenderer r;
    gus::app::BootPixelOverlay overlay;
    overlay.load(r, "resources/vfx/boot_pixel");
    overlay.draw(r, kScreen, BootPixelLeg::kFromBattleRevealing, 0.99f);
    REQUIRE(r.textured[0].tex == static_cast<TextureId>(1));
}
