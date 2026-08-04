// SPDX-License-Identifier: Apache-2.0
// gus/domain/world/scene_prop.cpp
//
// Implementação do catálogo de peças de cenário. Ver o header para a lei do
// átomo aplicada ao mundo. Travado por domain/tests/scene_prop_test.cpp
// (TEST-FIRST).

#include "gus/domain/world/scene_prop.hpp"

namespace gus::domain::world {

const ScenePropDef& scene_prop_def(ScenePropKind kind) noexcept {
    const int i = static_cast<int>(kind);
    if (i < 0 || i >= kScenePropKindCount) {
        // Guarda de robustez: valor fora da tabela (dado corrompido, cast
        // inválido) degrada para a primeira linha em vez de ler fora do array.
        return kMasterSceneProps[0];
    }
    return kMasterSceneProps[i];
}

gus::core::spatial::Aabb scene_prop_footprint(const ScenePropPlacement& placement,
                                               const ScenePropDef& def,
                                               float tile_size,
                                               float scale) noexcept {
    const float s = scale > 0.0f ? scale : 1.0f;
    const float ts = tile_size > 0.0f ? tile_size : 0.0f;

    gus::core::spatial::Aabb rect;
    rect.w = def.width_tiles() * ts * s;
    rect.h = def.height_tiles() * ts * s;
    // Centro em X sobre a célula; base do desenho na base da célula. Toda peça
    // cresce PARA CIMA a partir do chão - por isso a escala nunca a descola dele.
    rect.x = (static_cast<float>(placement.cell_x) + 0.5f) * ts - rect.w * 0.5f;
    rect.y = (static_cast<float>(placement.cell_y) + 1.0f) * ts - rect.h;
    return rect;
}

gus::core::spatial::Aabb scene_prop_footprint(const ScenePropPlacement& placement,
                                               float tile_size,
                                               float scale) noexcept {
    return scene_prop_footprint(placement, scene_prop_def(placement.kind), tile_size,
                                scale);
}

gus::core::spatial::Aabb scene_prop_solid(const gus::core::spatial::Aabb& footprint,
                                           const ScenePropDef& def, float tile_size,
                                           float scale) noexcept {
    gus::core::spatial::Aabb solid{};
    if (!def.solid()) {
        return solid;  // atravessável: caixa degenerada, o chamador não a usa
    }
    const float s = scale > 0.0f ? scale : 1.0f;
    const float ts = tile_size > 0.0f ? tile_size : 0.0f;
    solid.w = def.solid_w_tiles * ts * s;
    solid.h = def.solid_h_tiles * ts * s;
    // MESMA ancoragem do corpo sólido de NPC/inimigo (centro em X, base do
    // desenho): uma convenção só de "onde o corpo encosta no chão" em todo o jogo.
    solid.x = footprint.x + footprint.w * 0.5f - solid.w * 0.5f;
    solid.y = (footprint.y + footprint.h) - solid.h;
    return solid;
}

}  // namespace gus::domain::world
