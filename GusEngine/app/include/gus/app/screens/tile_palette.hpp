// gus/app/screens/tile_palette.hpp
//
// ===========================================================================
//  PALETA GRAYBOX dos tiles do overworld (M4-visual). PONTO UNICO de cor por
//  TileKind enquanto NAO ha arte de tileset. O lider ajusta os RGBA aqui vendo o
//  mapa no display - sem reescrever logica de render.
// ===========================================================================
//
// Header-only (dados puros). O OverworldSim recebe uma TilePalette e pinta cada
// celula da grade com a cor do seu TileKind (graybox legivel pro blockout). Quando
// vier o tileset de verdade, isto sai (o render passa a amostrar textura por tile).
//
// CORES (RGBA em [0,1], placeholder; o lider valida no display 2026-06-23):
//   Chao    cinza-medio   - piso andavel (fundo neutro do blockout).
//   Parede  escura-azulada- solido (igual a wall_color legada do M1, continuidade).
//   Marco   ambar         - ponto de interesse (landmark) salta ao olho.
//   Entrada verde         - por onde se entra na area.
//   Saida   azul          - por onde se sai da area.
// Ids RESERVADOS ao futuro (>= kMaxReservado, sem TileKind) caem no fallback de
// Chao (sao andaveis por is_tile_blocking): nao quebra o render de mapas futuros.
//
// Cross-ref: gus/domain/map/tile_map.hpp (TileKind), gus/app/screens/overworld_sim.hpp
//            (consumidor), gus/app/screens/overworld_tuning.hpp (feel de movimento).

#ifndef GUS_APP_SCREENS_TILE_PALETTE_HPP
#define GUS_APP_SCREENS_TILE_PALETTE_HPP

#include <cstdint>

#include "gus/domain/map/tile_map.hpp"  // TileKind, kMaxReservado
#include "gus/platform/render2d/i_renderer.hpp"  // DrawColor

namespace gus::app::screens {

// Cor de cada TileKind no graybox. Defaults aprovados pelo lider (ajustaveis aqui).
struct TilePalette {
    gus::platform::render2d::DrawColor chao{0.35f, 0.35f, 0.38f, 1.0f};
    gus::platform::render2d::DrawColor parede{0.18f, 0.20f, 0.28f, 1.0f};
    gus::platform::render2d::DrawColor marco{0.85f, 0.62f, 0.18f, 1.0f};
    gus::platform::render2d::DrawColor entrada{0.25f, 0.70f, 0.35f, 1.0f};
    gus::platform::render2d::DrawColor saida{0.25f, 0.45f, 0.85f, 1.0f};
};

// Cor de um tile-id cru segundo a paleta. Ids sem TileKind conhecido (reservados ao
// futuro) caem no Chao (andaveis), espelhando is_tile_blocking. Total para uint16_t.
[[nodiscard]] inline gus::platform::render2d::DrawColor color_for_tile(
    const TilePalette& palette, std::uint16_t tile_id) noexcept {
    using gus::domain::map::TileKind;
    switch (static_cast<TileKind>(tile_id)) {
        case TileKind::Parede:
            return palette.parede;
        case TileKind::Marco:
            return palette.marco;
        case TileKind::Entrada:
            return palette.entrada;
        case TileKind::Saida:
            return palette.saida;
        case TileKind::Chao:
        default:
            return palette.chao;  // Chao + ids reservados ao futuro
    }
}

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_TILE_PALETTE_HPP
