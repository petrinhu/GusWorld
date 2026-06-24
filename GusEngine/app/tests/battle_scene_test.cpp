// GusEngine/app/tests/battle_scene_test.cpp
//
// Catch2 (headless) da BattleScene (M5, incremento 1): prova que a cena LE o estado do
// motor de combate (domain/combat/) e DESENHA o esqueleto num IRenderer FALSO, SEM
// janela nem SDL. Cobre:
//   - o encontro de DEMO monta (party de 3 com Gus + 4 inimigos);
//   - a fila de iniciativa esta ordenada por SPD (queue_len = 7);
//   - gus_party_index acha o Gus pelo is_universal_compiler;
//   - o render emite primitivos pras 4 zonas (fundo + arena + CTB + HUD) e desenha
//     UM retangulo por ator vivo (3 party + 4 inimigos) e por celula CTB ocupada;
//   - degrada pro retangulo quando nao ha retratos (caminho headless).

#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "gus/app/screens/battle_scene.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

using gus::app::screens::BattleScene;
using gus::core::spatial::Rect;
using gus::platform::render2d::ContentBbox;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::IRenderer;
using gus::platform::render2d::TextureId;
using gus::platform::render2d::UvRect;

namespace {

// IRenderer falso: conta filled-rects, outlines e textured-rects e guarda os rects.
class CountingRenderer : public IRenderer {
public:
    struct Item {
        Rect rect;
        DrawColor color;
    };
    void begin_frame(const Rect& cam, int pw, int ph) override {
        fills.clear();
        outlines.clear();
        textured.clear();
        last_cam = cam;
        last_pw = pw;
        last_ph = ph;
        ++begins;
    }
    void draw_filled_rect(const Rect& r, const DrawColor& c) override {
        fills.push_back({r, c});
    }
    void draw_rect_outline(const Rect& r, const DrawColor& c, float) override {
        outlines.push_back({r, c});
    }
    TextureId load_texture(const char*) override { return next_tex_++; }
    void draw_textured_rect(const Rect& r, TextureId, const UvRect&,
                            const DrawColor&) override {
        textured.push_back(r);
    }
    ContentBbox texture_content_bbox(TextureId) const override { return {}; }
    void end_frame() override { ++ends; }

    std::vector<Item> fills;
    std::vector<Item> outlines;
    std::vector<Rect> textured;
    Rect last_cam{};
    int last_pw = 0;
    int last_ph = 0;
    int begins = 0;
    int ends = 0;
    TextureId next_tex_ = 1;
};

// Conta quantos fills caem (centro) dentro da metade ESQUERDA / DIREITA da tela.
int fills_in_x_band(const std::vector<CountingRenderer::Item>& fills, float x0,
                    float x1) {
    int n = 0;
    for (const auto& f : fills) {
        const float cx = f.rect.x + f.rect.w * 0.5f;
        if (cx >= x0 && cx < x1) {
            ++n;
        }
    }
    return n;
}

}  // namespace

TEST_CASE("BattleScene monta o encontro de demo e le a fila do motor",
          "[battle_scene]") {
    BattleScene scene;
    REQUIRE(scene.party_count() == 3);
    REQUIRE(scene.enemy_count() == 4);
    REQUIRE(scene.queue_len() == 7);
    // Ator ativo existe (fila nao vazia) e e o de maior SPD (Drone spd 13 = inimigo3).
    REQUIRE(scene.active_actor() != nullptr);
    REQUIRE(scene.active_actor()->spd() == 13);
}

TEST_CASE("BattleScene acha o Gus na party pelo compilador universal",
          "[battle_scene]") {
    BattleScene scene;
    const int gi = scene.gus_party_index();
    REQUIRE(gi >= 0);
    REQUIRE(gi < scene.party_count());
}

TEST_CASE("BattleScene::render abre/fecha 1 frame com camera logica 640x360",
          "[battle_scene]") {
    BattleScene scene;
    CountingRenderer r;
    scene.render(r, /*viewport_px_w=*/1280.0f, /*viewport_px_h=*/720.0f);
    REQUIRE(r.begins == 1);
    REQUIRE(r.ends == 1);
    // Camera 1:1 no retangulo logico 640x360.
    REQUIRE(r.last_cam.w == 640.0f);
    REQUIRE(r.last_cam.h == 360.0f);
    // begin_frame recebeu os PIXELS REAIS da janela (pro backend escalar).
    REQUIRE(r.last_pw == 1280);
    REQUIRE(r.last_ph == 720);
}

TEST_CASE("BattleScene::render desenha 1 placeholder por ator (esquerda x direita)",
          "[battle_scene]") {
    BattleScene scene;
    CountingRenderer r;
    scene.render(r, 640.0f, 360.0f);

    // Fundo + faixa CTB + 7 atores + 5 celulas CTB + painel + log = varios fills.
    REQUIRE(r.fills.size() >= 7);  // pelo menos um por ator

    // Atores: 3 na metade esquerda (party) e 4 na metade direita (inimigos). Conta
    // SO os fills no tamanho de slot (56x64) pra nao misturar com fundo/HUD.
    auto count_actor_slots = [&](float x0, float x1) {
        int n = 0;
        for (const auto& f : r.fills) {
            if (f.rect.w == 56.0f && f.rect.h == 64.0f) {
                const float cx = f.rect.x + f.rect.w * 0.5f;
                if (cx >= x0 && cx < x1) {
                    ++n;
                }
            }
        }
        return n;
    };
    REQUIRE(count_actor_slots(0.0f, 320.0f) == 3);    // party esquerda
    REQUIRE(count_actor_slots(320.0f, 640.0f) == 4);  // inimigos direita
    (void)fills_in_x_band;  // helper disponivel pra evolucao do teste
}

TEST_CASE("BattleScene::render headless degrada CTB pro retangulo (sem retratos)",
          "[battle_scene]") {
    BattleScene scene;  // sem set_portraits -> sem texturas
    CountingRenderer r;
    scene.render(r, 640.0f, 360.0f);
    // Sem retratos: nenhuma textura desenhada; as celulas CTB viram retangulos.
    REQUIRE(r.textured.empty());
    // 5 celulas CTB ocupadas (fila de 7) desenhadas como retangulos 48x48.
    int ctb_cells = 0;
    for (const auto& f : r.fills) {
        if (f.rect.w == 48.0f && f.rect.h == 48.0f) {
            ++ctb_cells;
        }
    }
    REQUIRE(ctb_cells == 5);
    // O ator ativo recebe um highlight (outline) na arena.
    REQUIRE(r.outlines.size() >= 1);
}
