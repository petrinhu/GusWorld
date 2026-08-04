// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/city_scene_test.cpp
//
// Catch2 (headless) da montagem da CENA DA CIDADE a partir de um TileMap (M4-visual).
// Prova, sem janela:
//   - spawn_player_aabb: hitbox ~0.6 tile CENTRADA na celula de spawn();
//   - make_city_scene: colisao = to_tile_grid (so Parede bloqueia), spawn certo,
//     TileMap guardado pro render por TileKind, e o Gus colide com Parede;
//   - color_for_tile: cada TileKind -> sua cor da paleta (graybox);
//   - o render por TileMap pinta TODAS as celulas (nao so as bloqueadas).
// I/O de arquivo (.gmap do disco) NAO entra aqui: e exercitado pelo smoke do main.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cstdint>
#include <cstdlib>  // setenv/unsetenv (seam de env do resolve_tile_palette)

#include "gus/app/screens/city_scene.hpp"
#include "gus/app/screens/overworld_sim.hpp"
#include "gus/app/screens/tile_palette.hpp"
#include "gus/core/spatial/grid_collision.hpp"
#include "gus/domain/map/tile_map.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

using Catch::Matchers::WithinAbs;
using gus::app::screens::color_for_tile;
using gus::app::screens::make_city_scene;
using gus::app::screens::make_city_tuning;
using gus::app::screens::OverworldSim;
using gus::app::screens::spawn_player_aabb;
using gus::app::screens::TilePalette;
using gus::core::spatial::Aabb;
using gus::core::spatial::Rect;
using gus::domain::map::Cell;
using gus::domain::map::TileKind;
using gus::domain::map::TileMap;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::IRenderer;
using gus::platform::render2d::TextureId;
using gus::platform::render2d::UvRect;

namespace {

std::uint16_t k(TileKind t) { return static_cast<std::uint16_t>(t); }

// Mapa pequeno e coerente: bordas de Parede, um Marco, spawn no meio. tile_size 2.0
// (igual ao .gmap real) pra apanhar bugs de escala.
TileMap small_walled_map() {
    TileMap m(5, 4, 2.0f);
    for (std::int32_t x = 0; x < 5; ++x) {
        m.set(x, 0, k(TileKind::Parede));
        m.set(x, 3, k(TileKind::Parede));
    }
    for (std::int32_t y = 0; y < 4; ++y) {
        m.set(0, y, k(TileKind::Parede));
        m.set(4, y, k(TileKind::Parede));
    }
    m.set(2, 1, k(TileKind::Marco));
    m.set(2, 2, k(TileKind::Chao));
    m.set_spawn(Cell{2, 2});
    m.validate();
    return m;
}

// IRenderer falso minimo: conta os filled-rects e guarda cor/retangulo.
class CountingRenderer : public IRenderer {
public:
    struct Fill {
        Rect rect;
        DrawColor color;
    };
    void begin_frame(const Rect&, int, int) override { fills.clear(); }
    void draw_filled_rect(const Rect& r, const DrawColor& c) override {
        fills.push_back({r, c});
    }
    void draw_rect_outline(const Rect&, const DrawColor&, float) override {}
    TextureId load_texture(const char*) override { return 0; }
    void draw_textured_rect(const Rect&, TextureId, const UvRect&,
                            const DrawColor&) override {}
    gus::platform::render2d::ContentBbox texture_content_bbox(
        TextureId) const override {
        return {};
    }
    void draw_text(const char*, float, float, float, const DrawColor&,
                   bool) override {}
    void end_frame() override {}

    std::vector<Fill> fills;
};

}  // namespace

TEST_CASE("spawn_player_aabb centra a hitbox na celula de spawn", "[city_scene]") {
    const TileMap m = small_walled_map();
    const Aabb a = spawn_player_aabb(m);

    // tile_size 2.0, hitbox = 0.6*2.0 = 1.2. Centro da celula (2,2) = (2.5, 2.5)*2.0
    // = (5.0, 5.0). Canto sup-esq = centro - meia-hitbox = 5.0 - 0.6 = 4.4.
    REQUIRE_THAT(a.w, WithinAbs(1.2f, 1e-4f));
    REQUIRE_THAT(a.h, WithinAbs(1.2f, 1e-4f));
    REQUIRE_THAT(a.x, WithinAbs(4.4f, 1e-4f));
    REQUIRE_THAT(a.y, WithinAbs(4.4f, 1e-4f));
}

TEST_CASE("make_city_scene nasce no spawn e guarda o TileMap", "[city_scene]") {
    const TileMap m = small_walled_map();
    OverworldSim sim = make_city_scene(m, make_city_tuning());

    // Nasce centrado no spawn.
    const Aabb p = sim.player();
    REQUIRE_THAT(p.x, WithinAbs(4.4f, 1e-4f));
    REQUIRE_THAT(p.y, WithinAbs(4.4f, 1e-4f));

    // Guardou o TileMap (pro render por TileKind) e a colisao casa o tile_size real.
    REQUIRE(sim.tile_map().has_value());
    REQUIRE(sim.grid().tile_size() == 2.0f);
    REQUIRE(sim.grid().width() == 5);
    REQUIRE(sim.grid().height() == 4);
    // Parede da borda bloqueia; o Marco (2,1) e andavel; o spawn (2,2) e andavel.
    REQUIRE(sim.grid().is_blocked(0, 0));
    REQUIRE_FALSE(sim.grid().is_blocked(2, 1));
    REQUIRE_FALSE(sim.grid().is_blocked(2, 2));
}

TEST_CASE("o Gus colide com a Parede do mapa real", "[city_scene]") {
    const TileMap m = small_walled_map();
    OverworldSim sim = make_city_scene(m, make_city_tuning());

    // Empurra forte pra ESQUERDA por varios ticks: a borda (coluna 0 = Parede) trava.
    const float dt = 1.0f / 60.0f;
    for (int i = 0; i < 600; ++i) {
        sim.step_fixed(/*dx=*/-1, /*dy=*/0, /*run=*/true, dt);
    }
    // Nao atravessa a coluna 0 (Parede): x do canto fica >= a borda interna da
    // coluna 1 (1*tile_size = 2.0). Com folga numerica.
    REQUIRE(sim.player().x >= 2.0f - 1e-3f);
}

TEST_CASE("color_for_tile mapeia cada TileKind pra sua cor", "[tile_palette]") {
    TilePalette pal;  // defaults aprovados
    REQUIRE(color_for_tile(pal, k(TileKind::Chao)).g == pal.chao.g);
    REQUIRE(color_for_tile(pal, k(TileKind::Parede)).b == pal.parede.b);
    REQUIRE(color_for_tile(pal, k(TileKind::Marco)).r == pal.marco.r);
    REQUIRE(color_for_tile(pal, k(TileKind::Entrada)).g == pal.entrada.g);
    REQUIRE(color_for_tile(pal, k(TileKind::Saida)).b == pal.saida.b);
    // id reservado ao futuro (sem TileKind) cai no Chao (andavel).
    REQUIRE(color_for_tile(pal, 9999).r == pal.chao.r);
}

TEST_CASE("render do mapa real pinta TODAS as celulas por TileKind", "[city_scene]") {
    const TileMap m = small_walled_map();  // 5x4 = 20 celulas
    OverworldSim sim = make_city_scene(m, make_city_tuning());

    CountingRenderer r;
    // Viewport gigante em mundo: a camera clampa, mas todas as 20 celulas cabem na
    // janela (mapa 10x8 em mundo). Espera 20 fills (uma por celula), nao so paredes.
    sim.render(r, /*viewport_w=*/1000.0f, /*viewport_h=*/1000.0f, /*alpha=*/0.0f);
    REQUIRE(r.fills.size() == 20);

    // A celula de Marco (2,1) pintou na cor de Marco da paleta.
    const TilePalette pal = sim.tile_palette();
    bool found_marco = false;
    for (const auto& f : r.fills) {
        // Centro da celula (2,1): x=(2.5)*2=5.0, y=(1.5)*2=3.0.
        if (f.rect.x <= 5.0f && f.rect.x + f.rect.w > 5.0f && f.rect.y <= 3.0f &&
            f.rect.y + f.rect.h > 3.0f) {
            REQUIRE(f.color.r == pal.marco.r);
            REQUIRE(f.color.g == pal.marco.g);
            found_marco = true;
        }
    }
    REQUIRE(found_marco);
}

// ===========================================================================
//  DUAS LEITURAS DO MAPA (DEMO-CIDADE-VESTIDA fatia E, achado A2 do laudo visual)
//
//  As cores de Marco/Entrada/Saida eram a LEGENDA do blockout, util enquanto nao
//  havia arte. Com a cidade vestida elas viraram defeito: o ambar vaza por baixo e
//  ao redor de cada peca (o poste tem 23 px de largura numa celula de 43 px) e as
//  ancoras sem peca ficam como quadrados chapados. Apagar a legenda consertaria a
//  tela e mataria a ferramenta de quem traca o nivel - por isso sao DUAS leituras
//  do MESMO dado, nao uma cor trocada.
// ===========================================================================

TEST_CASE("na leitura de PRODUCAO a marcacao de blockout nao pinta", "[tile_palette]") {
    const TilePalette pal;  // default = producao
    REQUIRE(pal.reading == gus::app::screens::TileReading::Production);
    // Marco/Entrada/Saida saem na cor do Chao: sobra chao e parede, e quem marca o
    // lugar passa a ser a ARTE da peca.
    REQUIRE(color_for_tile(pal, k(TileKind::Marco)).r == pal.chao.r);
    REQUIRE(color_for_tile(pal, k(TileKind::Marco)).g == pal.chao.g);
    REQUIRE(color_for_tile(pal, k(TileKind::Marco)).b == pal.chao.b);
    REQUIRE(color_for_tile(pal, k(TileKind::Entrada)).g == pal.chao.g);
    REQUIRE(color_for_tile(pal, k(TileKind::Saida)).b == pal.chao.b);
    // Chao e Parede continuam distintos: a leitura de traçado do mapa sobrevive.
    REQUIRE(color_for_tile(pal, k(TileKind::Parede)).b != pal.chao.b);
}

TEST_CASE("a leitura de BLOCKOUT devolve a legenda inteira", "[tile_palette]") {
    const TilePalette pal = gus::app::screens::blockout_palette();
    REQUIRE(pal.reading == gus::app::screens::TileReading::Blockout);
    REQUIRE(color_for_tile(pal, k(TileKind::Marco)).r == gus::app::screens::kLegendMarco.r);
    REQUIRE(color_for_tile(pal, k(TileKind::Entrada)).g ==
            gus::app::screens::kLegendEntrada.g);
    REQUIRE(color_for_tile(pal, k(TileKind::Saida)).b == gus::app::screens::kLegendSaida.b);
    // E os tres sao DISTINTOS do chao - senao a legenda nao legenda nada.
    REQUIRE(color_for_tile(pal, k(TileKind::Marco)).r != pal.chao.r);
    REQUIRE(color_for_tile(pal, k(TileKind::Entrada)).g != pal.chao.g);
    REQUIRE(color_for_tile(pal, k(TileKind::Saida)).b != pal.chao.b);
    // Chao e Parede sao os MESMOS nas duas leituras: so a legenda muda.
    const TilePalette prod;
    REQUIRE(pal.chao.r == prod.chao.r);
    REQUIRE(pal.parede.b == prod.parede.b);
}

TEST_CASE("o render usa a leitura corrente na celula de Marco", "[city_scene]") {
    const TileMap m = small_walled_map();  // Marco em (2,1)
    OverworldSim sim = make_city_scene(m, make_city_tuning());
    CountingRenderer r;

    // Centro da celula (2,1) em mundo: x=5.0, y=3.0.
    const auto cor_do_marco = [&]() {
        for (const auto& f : r.fills) {
            if (f.rect.x <= 5.0f && f.rect.x + f.rect.w > 5.0f && f.rect.y <= 3.0f &&
                f.rect.y + f.rect.h > 3.0f) {
                return f.color;
            }
        }
        return DrawColor{-1.0f, -1.0f, -1.0f, -1.0f};
    };

    // PRODUCAO (default): a ancora some no chao.
    sim.render(r, 1000.0f, 1000.0f, 0.0f);
    REQUIRE(cor_do_marco().r == sim.tile_palette().chao.r);

    // BLOCKOUT: o ambar volta, sem tocar em nada alem da paleta.
    sim.set_tile_palette(gus::app::screens::blockout_palette());
    sim.render(r, 1000.0f, 1000.0f, 0.0f);
    REQUIRE(cor_do_marco().r == gus::app::screens::kLegendMarco.r);
}

// ===========================================================================
//  A CELULA QUE UMA PECA VESTE (achado A3 do laudo visual)
//
//  A caixa de cobertura NAO fica ao lado de um obstaculo: ela E a celula de
//  Parede. Pintar essa celula com a cor de graybox da parede deixa a barricada
//  "pairando sobre um buraco" escuro. Na leitura de producao a celula vestida sai
//  na cor do Chao - quem faz o papel de parede ali e a arte. A colisao NAO muda:
//  a celula continua Parede na TileGrid.
// ===========================================================================

namespace {

// Peca de pe plantada na celula (cx,cy) de um mapa de tile_size ts.
gus::app::screens::ScenePropInstance prop_em(int cx, int cy, float ts,
                                             TextureId tex) {
    gus::app::screens::ScenePropInstance p;
    p.footprint = Aabb{static_cast<float>(cx) * ts, static_cast<float>(cy) * ts, ts, ts};
    p.solid = p.footprint;
    p.tex = tex;
    p.ground = false;
    p.cell_x = cx;
    p.cell_y = cy;
    return p;
}

}  // namespace

TEST_CASE("a celula de Parede que uma peca veste nao pinta na producao",
          "[city_scene]") {
    const TileMap m = small_walled_map();  // borda inteira de Parede; (0,1) e Parede
    OverworldSim sim = make_city_scene(m, make_city_tuning());
    CountingRenderer r;

    // Centro da celula (0,1) em mundo: x=1.0, y=3.0.
    const auto cor_da_celula = [&]() {
        for (const auto& f : r.fills) {
            if (f.rect.x <= 1.0f && f.rect.x + f.rect.w > 1.0f && f.rect.y <= 3.0f &&
                f.rect.y + f.rect.h > 3.0f) {
                return f.color;
            }
        }
        return DrawColor{-1.0f, -1.0f, -1.0f, -1.0f};
    };

    // SEM peca: parede pinta parede.
    sim.render(r, 1000.0f, 1000.0f, 0.0f);
    REQUIRE(cor_da_celula().b == sim.tile_palette().parede.b);

    // COM peca desenhavel em cima: a celula sai na cor do chao.
    sim.add_scene_prop(prop_em(0, 1, 2.0f, /*tex=*/7));
    sim.render(r, 1000.0f, 1000.0f, 0.0f);
    REQUIRE(cor_da_celula().b == sim.tile_palette().chao.b);
    // A vizinha (0,2), tambem Parede e SEM peca, segue parede: a supressao e da
    // celula vestida, nao da coluna.
    bool achou_vizinha = false;
    for (const auto& f : r.fills) {
        if (f.rect.x <= 1.0f && f.rect.x + f.rect.w > 1.0f && f.rect.y <= 5.0f &&
            f.rect.y + f.rect.h > 5.0f) {
            REQUIRE(f.color.b == sim.tile_palette().parede.b);
            achou_vizinha = true;
        }
    }
    REQUIRE(achou_vizinha);

    // BLOCKOUT: quem traca o nivel volta a ver a parede de verdade.
    sim.set_tile_palette(gus::app::screens::blockout_palette());
    sim.render(r, 1000.0f, 1000.0f, 0.0f);
    REQUIRE(cor_da_celula().b == sim.tile_palette().parede.b);
}

TEST_CASE("peca SEM arte nao esconde a parede que ela deveria vestir",
          "[city_scene]") {
    // REGRA DE DEGRADACAO, irma da que ja vale para o bloqueio: peca sem textura
    // nao entra no mundo, entao suprimir a parede por causa dela deixaria um
    // obstaculo INVISIVEL - o jogador travaria contra chao pintado.
    const TileMap m = small_walled_map();
    OverworldSim sim = make_city_scene(m, make_city_tuning());
    CountingRenderer r;

    sim.add_scene_prop(
        prop_em(0, 1, 2.0f, /*tex=*/gus::platform::render2d::kInvalidTexture));
    sim.render(r, 1000.0f, 1000.0f, 0.0f);
    for (const auto& f : r.fills) {
        if (f.rect.x <= 1.0f && f.rect.x + f.rect.w > 1.0f && f.rect.y <= 3.0f &&
            f.rect.y + f.rect.h > 3.0f) {
            REQUIRE(f.color.b == sim.tile_palette().parede.b);
        }
    }
}

TEST_CASE("clear_scene_props devolve a parede a celula", "[city_scene]") {
    const TileMap m = small_walled_map();
    OverworldSim sim = make_city_scene(m, make_city_tuning());
    CountingRenderer r;

    sim.add_scene_prop(prop_em(0, 1, 2.0f, /*tex=*/7));
    sim.clear_scene_props();
    sim.render(r, 1000.0f, 1000.0f, 0.0f);
    for (const auto& f : r.fills) {
        if (f.rect.x <= 1.0f && f.rect.x + f.rect.w > 1.0f && f.rect.y <= 3.0f &&
            f.rect.y + f.rect.h > 3.0f) {
            REQUIRE(f.color.b == sim.tile_palette().parede.b);
        }
    }
}

// NOME SEM VIRGULA de proposito: o Catch2 trata virgula como SEPARADOR de filtro,
// entao um nome com virgula nao casa por nome nenhum e o binario sai com "No tests
// ran" - que e facil de ler como sucesso.
TEST_CASE("resolve_tile_palette da producao por default e blockout so por env",
          "[city_scene]") {
    // O default e o caminho SEGURO: sem env var (ou com valor escrito errado) o
    // jogo pinta a leitura de producao. A legenda so aparece se for pedida.
    ::unsetenv("GUSWORLD_TILE_PALETTE");
    REQUIRE(gus::app::screens::resolve_tile_palette().reading ==
            gus::app::screens::TileReading::Production);

    ::setenv("GUSWORLD_TILE_PALETTE", "blockout", 1);
    REQUIRE(gus::app::screens::resolve_tile_palette().reading ==
            gus::app::screens::TileReading::Blockout);
    REQUIRE(gus::app::screens::resolve_tile_palette().marco.r ==
            gus::app::screens::kLegendMarco.r);

    // Valor QUALQUER (typo, lixo, string vazia) NAO liga a legenda por acidente.
    ::setenv("GUSWORLD_TILE_PALETTE", "blockuot", 1);
    REQUIRE(gus::app::screens::resolve_tile_palette().reading ==
            gus::app::screens::TileReading::Production);
    ::setenv("GUSWORLD_TILE_PALETTE", "", 1);
    REQUIRE(gus::app::screens::resolve_tile_palette().reading ==
            gus::app::screens::TileReading::Production);
    ::setenv("GUSWORLD_TILE_PALETTE", "BLOCKOUT", 1);
    REQUIRE(gus::app::screens::resolve_tile_palette().reading ==
            gus::app::screens::TileReading::Production);

    ::unsetenv("GUSWORLD_TILE_PALETTE");
}
