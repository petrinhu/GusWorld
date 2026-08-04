// SPDX-License-Identifier: Apache-2.0
// gus/app/src/screens/city_props.cpp
//
// Implementação do vestir da cidade. Ver o header. Travado por
// app/tests/city_props_test.cpp (TEST-FIRST).

#include "gus/app/screens/city_props.hpp"

#include <string>

#include <glintfx/log.hpp>  // FW-LOG: o log da casa vem do glintfx, nunca do SDL

#include "gus/core/asset_paths.hpp"
#include "gus/platform/assets/asset_source.hpp"

namespace gus::app::screens {

namespace {

// Caminho da arte de uma peça: pasta da área + arquivo da linha do catálogo, os
// dois vindos do header central (caminho de asset nunca literal aqui).
std::string prop_asset_path(const gus::domain::world::ScenePropDef& def) {
    const std::string id = std::string(gus::core::assets::kWorldPropsDistritosDir) +
                           "/" + std::string(def.sprite_file);
    return gus::platform::assets::FilesystemAssetSource().resolve_path(id);
}

}  // namespace

std::vector<gus::domain::world::ScenePropPlacement> resolve_city_props(
    const gus::core::spatial::TileGrid& grid, const CityPropRow* rows, int count) {
    std::vector<gus::domain::world::ScenePropPlacement> out;
    if (rows == nullptr || count <= 0) {
        return out;
    }

    // Limites do mapa, checados à parte de propósito: is_blocked devolve true para
    // célula de fora (a borda é parede implícita), então a régua da peça-parede
    // aceitaria o lado de fora do mundo se confiasse só nela.
    const auto in_bounds = [&grid](int cx, int cy) {
        return cx >= 0 && cx < grid.width() && cy >= 0 && cy < grid.height();
    };

    out.reserve(static_cast<std::size_t>(count));
    int descartadas = 0;
    for (int i = 0; i < count; ++i) {
        const int cx = rows[i].cell_x;
        const int cy = rows[i].cell_y;
        const bool cabe =
            in_bounds(cx, cy) && (rows[i].cell_rule == PropCellRule::WallCell
                                      ? grid.is_blocked(cx, cy)
                                      : !grid.is_blocked(cx, cy));
        if (!cabe) {
            ++descartadas;
            continue;
        }
        out.push_back({rows[i].kind, cx, cy});
    }
    if (descartadas > 0) {
        // via PLANA: número, texto interno de diagnóstico.
        glintfx::log(glintfx::LogLevel::Warn,
                     ("city_props: " + std::to_string(descartadas) +
                      " peca(s) de cenario caiu(ram) fora do mapa ou dentro de "
                      "parede - descartada(s) da vestimenta desta cidade.")
                         .c_str());
    }
    return out;
}

ScenePropTextures load_scene_prop_textures(
    gus::platform::render2d::IRenderer& renderer) {
    ScenePropTextures out;
    int ausentes = 0;
    for (int i = 0; i < gus::domain::world::kScenePropKindCount; ++i) {
        const gus::domain::world::ScenePropDef& def =
            gus::domain::world::kMasterSceneProps[i];
        const std::string path = prop_asset_path(def);
        out.by_kind[i] = renderer.load_texture(path.c_str());
        if (out.by_kind[i] == gus::platform::render2d::kInvalidTexture) {
            ++ausentes;
        }
    }
    if (ausentes > 0) {
        glintfx::log(glintfx::LogLevel::Warn,
                     ("city_props: " + std::to_string(ausentes) +
                      " arte(s) de cenario ausente(s)/ilegivel(is) - a(s) peca(s) "
                      "correspondente(s) nao entra(m) no mundo.")
                         .c_str());
    }
    return out;
}

std::vector<ScenePropInstance> build_scene_prop_instances(
    const gus::domain::world::ScenePropPlacement* placements, int count,
    float tile_size, float scale, const ScenePropTextures& textures) {
    std::vector<ScenePropInstance> out;
    if (placements == nullptr || count <= 0) {
        return out;
    }
    out.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        const gus::domain::world::ScenePropPlacement& p = placements[i];
        const gus::platform::render2d::TextureId tex = textures.at(p.kind);
        if (tex == gus::platform::render2d::kInvalidTexture) {
            // Peça sem arte NÃO entra: invisível que bloqueia é parede fantasma.
            continue;
        }
        const gus::domain::world::ScenePropDef& def =
            gus::domain::world::scene_prop_def(p.kind);
        ScenePropInstance inst;
        inst.footprint =
            gus::domain::world::scene_prop_footprint(p, def, tile_size, scale);
        inst.solid =
            gus::domain::world::scene_prop_solid(inst.footprint, def, tile_size, scale);
        inst.tex = tex;
        inst.ground = def.ground;
        out.push_back(inst);
    }
    return out;
}

}  // namespace gus::app::screens
