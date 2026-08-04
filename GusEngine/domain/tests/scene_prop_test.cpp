// SPDX-License-Identifier: Apache-2.0
// scene_prop_test.cpp
//
// Spec executável (Catch2 v3) do catálogo de PEÇAS DE CENÁRIO
// (gus/domain/world/scene_prop.hpp, DEMO-CIDADE-VESTIDA fatia B1): a peça de
// cenário é DADO (uma linha na tabela kMasterSceneProps), nunca uma função nova
// por peça (lei do átomo, ADR-019/ADR-020). Este teste trava exatamente isso: a
// geometria (retângulo desenhado + caixa sólida) é derivada da MESMA fórmula para
// TODA peça, e o único jeito de acrescentar uma peça é acrescentar uma linha.
//
// Cross-ref: docs/tech/adr/ADR-020-pecas-componiveis-modulo-e-mundo-data-driven.md;
//            gus/app/screens/overworld_sim.hpp (consumidor: desenha e colide).

#include <catch2/catch_test_macros.hpp>

#include <string_view>

#include "gus/domain/world/scene_prop.hpp"

using gus::core::spatial::Aabb;
using gus::domain::world::kMasterSceneProps;
using gus::domain::world::kScenePropArtPixelsPerTile;
using gus::domain::world::kScenePropKindCount;
using gus::domain::world::scene_prop_def;
using gus::domain::world::scene_prop_footprint;
using gus::domain::world::scene_prop_solid;
using gus::domain::world::ScenePropKind;
using gus::domain::world::ScenePropPlacement;

namespace {

constexpr bool near_eq(float a, float b, float eps = 0.0005f) noexcept {
    const float d = a - b;
    return (d < 0.0f ? -d : d) <= eps;
}

}  // namespace

TEST_CASE("scene_prop: a tabela mestra cobre TODAS as peças do enum sem buraco",
          "[domain][world][scene_prop]") {
    // A tabela é a fonte da verdade: uma linha por peça, na ordem do enum. Se
    // alguém acrescentar um ScenePropKind e esquecer a linha (ou vice-versa),
    // este teste reprova ANTES de o jogo tentar desenhar uma peça sem dado.
    REQUIRE(static_cast<int>(std::size(kMasterSceneProps)) == kScenePropKindCount);
    for (int i = 0; i < kScenePropKindCount; ++i) {
        REQUIRE(static_cast<int>(kMasterSceneProps[i].kind) == i);
    }
}

TEST_CASE("scene_prop: toda peça tem arquivo de arte e dimensão de arte positiva",
          "[domain][world][scene_prop]") {
    for (const auto& def : kMasterSceneProps) {
        REQUIRE_FALSE(def.sprite_file.empty());
        REQUIRE(def.art_px_w > 0.0f);
        REQUIRE(def.art_px_h > 0.0f);
    }
}

TEST_CASE("scene_prop: nenhum arquivo de arte se repete entre peças",
          "[domain][world][scene_prop]") {
    for (int i = 0; i < kScenePropKindCount; ++i) {
        for (int j = i + 1; j < kScenePropKindCount; ++j) {
            REQUIRE(kMasterSceneProps[i].sprite_file !=
                    kMasterSceneProps[j].sprite_file);
        }
    }
}

TEST_CASE("scene_prop: tamanho em tiles vem da arte a 32 px por tile",
          "[domain][world][scene_prop]") {
    // A arte foi desenhada a 32 px/tile (casa 96x96 = 3x3 tiles). O tamanho em
    // tiles é DERIVADO da arte, nunca um número chutado por peça.
    const auto& casa = scene_prop_def(ScenePropKind::CasaCibergoticaA);
    REQUIRE(near_eq(casa.width_tiles(), casa.art_px_w / kScenePropArtPixelsPerTile));
    REQUIRE(near_eq(casa.height_tiles(), casa.art_px_h / kScenePropArtPixelsPerTile));
    REQUIRE(near_eq(casa.width_tiles(), 3.0f));
    REQUIRE(near_eq(casa.height_tiles(), 3.0f));
}

TEST_CASE("scene_prop: scene_prop_def devolve a linha do kind pedido",
          "[domain][world][scene_prop]") {
    for (int i = 0; i < kScenePropKindCount; ++i) {
        const auto kind = static_cast<ScenePropKind>(i);
        REQUIRE(scene_prop_def(kind).kind == kind);
    }
}

TEST_CASE("scene_prop: kind fora da tabela degrada para a primeira linha",
          "[domain][world][scene_prop]") {
    // Guarda de robustez (nunca deref fora do array): valor inválido cai na
    // primeira linha em vez de ler memória alheia. Não é caminho legítimo.
    const auto invalido = static_cast<ScenePropKind>(kScenePropKindCount + 7);
    REQUIRE(scene_prop_def(invalido).kind == kMasterSceneProps[0].kind);
    const auto negativo = static_cast<ScenePropKind>(-3);
    REQUIRE(scene_prop_def(negativo).kind == kMasterSceneProps[0].kind);
}

TEST_CASE("scene_prop: o retângulo desenhado tem a base centrada na célula",
          "[domain][world][scene_prop]") {
    // Convenção de autoria (a única que o level designer precisa saber): a célula
    // da placement é onde a peça PISA - base do desenho na base da célula, e o
    // desenho centrado em X sobre ela. O resto (altura) sobe a partir dali.
    const float ts = 2.0f;
    const ScenePropPlacement p{ScenePropKind::CasaCibergoticaA, 10, 5};
    const Aabb rect = scene_prop_footprint(p, ts, /*scale=*/1.0f);

    REQUIRE(near_eq(rect.w, 3.0f * ts));
    REQUIRE(near_eq(rect.h, 3.0f * ts));
    // centro em X = centro da célula 10.
    REQUIRE(near_eq(rect.x + rect.w * 0.5f, (10.0f + 0.5f) * ts));
    // base do desenho = base da célula 5.
    REQUIRE(near_eq(rect.y + rect.h, (5.0f + 1.0f) * ts));
}

TEST_CASE("scene_prop: a escala global multiplica o desenho mantendo a base",
          "[domain][world][scene_prop]") {
    // A escala é o botão ÚNICO do líder para engordar/afinar TODAS as peças de uma
    // vez. Ela nunca desloca a peça do chão: a base fica exatamente onde estava.
    const float ts = 2.0f;
    const ScenePropPlacement p{ScenePropKind::CasaCibergoticaA, 4, 4};
    const Aabb um = scene_prop_footprint(p, ts, 1.0f);
    const Aabb dois = scene_prop_footprint(p, ts, 2.0f);

    REQUIRE(near_eq(dois.w, um.w * 2.0f));
    REQUIRE(near_eq(dois.h, um.h * 2.0f));
    REQUIRE(near_eq(dois.y + dois.h, um.y + um.h));                    // mesma base
    REQUIRE(near_eq(dois.x + dois.w * 0.5f, um.x + um.w * 0.5f));      // mesmo centro X
}

TEST_CASE("scene_prop: a caixa sólida fica ancorada na base do desenho",
          "[domain][world][scene_prop]") {
    // MESMA ancoragem do corpo sólido de NPC/inimigo (centro em X, base = base do
    // desenho): o jogador contorna pela frente e passa POR TRÁS pelo lado de cima,
    // que é o que o Y-sort desenha corretamente.
    const float ts = 2.0f;
    const ScenePropPlacement p{ScenePropKind::CasaCibergoticaA, 6, 6};
    const auto& def = scene_prop_def(p.kind);
    const Aabb rect = scene_prop_footprint(p, ts, 1.0f);
    const Aabb solid = scene_prop_solid(rect, def, ts, 1.0f);

    REQUIRE(solid.w > 0.0f);
    REQUIRE(solid.h > 0.0f);
    REQUIRE(near_eq(solid.x + solid.w * 0.5f, rect.x + rect.w * 0.5f));
    REQUIRE(near_eq(solid.y + solid.h, rect.y + rect.h));
    // A caixa de bloqueio NUNCA é o retângulo desenhado inteiro (senão a casa
    // travaria o corredor vários tiles acima da porta).
    REQUIRE(solid.h < rect.h);
}

TEST_CASE("scene_prop: peça atravessável devolve caixa sólida de área zero",
          "[domain][world][scene_prop]") {
    // Holograma e tabuleiro no chão NÃO bloqueiam: a caixa sólida sai degenerada
    // (w==0 && h==0) e o chamador simplesmente não a passa como obstáculo.
    const float ts = 2.0f;
    const ScenePropPlacement p{ScenePropKind::HoloSterling, 3, 3};
    const auto& def = scene_prop_def(p.kind);
    REQUIRE_FALSE(def.solid());
    const Aabb solid = scene_prop_solid(scene_prop_footprint(p, ts, 1.0f), def, ts, 1.0f);
    REQUIRE(near_eq(solid.w, 0.0f));
    REQUIRE(near_eq(solid.h, 0.0f));
}

TEST_CASE("scene_prop: peça de chão nunca é sólida",
          "[domain][world][scene_prop]") {
    // Invariante do dado: o que é pintado NO CHÃO (antes do Y-sort) não pode
    // bloquear passagem - senão vira parede invisível sob os pés do jogador.
    for (const auto& def : kMasterSceneProps) {
        if (def.ground) {
            REQUIRE_FALSE(def.solid());
        }
    }
}

TEST_CASE("scene_prop: caixa sólida nunca é mais larga que o desenho",
          "[domain][world][scene_prop]") {
    // Vale para TODA linha da tabela (enumeração fechada, não amostragem): o
    // bloqueio não pode passar da silhueta desenhada em nenhum eixo.
    const float ts = 2.0f;
    for (int i = 0; i < kScenePropKindCount; ++i) {
        const ScenePropPlacement p{static_cast<ScenePropKind>(i), 8, 8};
        const auto& def = scene_prop_def(p.kind);
        const Aabb rect = scene_prop_footprint(p, ts, 1.0f);
        const Aabb solid = scene_prop_solid(rect, def, ts, 1.0f);
        REQUIRE(solid.w <= rect.w + 0.0005f);
        REQUIRE(solid.h <= rect.h + 0.0005f);
    }
}

TEST_CASE("scene_prop: tile_size degenerado não explode a geometria",
          "[domain][world][scene_prop]") {
    const ScenePropPlacement p{ScenePropKind::PosteNeonCiano, 2, 2};
    const Aabb rect = scene_prop_footprint(p, /*tile_size=*/0.0f, 1.0f);
    REQUIRE(near_eq(rect.w, 0.0f));
    REQUIRE(near_eq(rect.h, 0.0f));
}
