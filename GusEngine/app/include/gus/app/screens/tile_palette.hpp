// SPDX-License-Identifier: Apache-2.0
// gus/app/screens/tile_palette.hpp
//
// ===========================================================================
//  PALETA dos tiles do overworld. PONTO UNICO de cor por TileKind enquanto NAO ha
//  arte de tileset. O lider ajusta os RGBA aqui vendo o mapa no display - sem
//  reescrever logica de render.
// ===========================================================================
//
// Header-only (dados puros). O OverworldSim recebe uma TilePalette e pinta cada
// celula da grade com a cor do seu TileKind. Quando vier o tileset de verdade,
// isto sai (o render passa a amostrar textura por tile).
//
// DUAS LEITURAS DO MESMO MAPA (DEMO-CIDADE-VESTIDA fatia E, achado A2 do laudo
// visual). As cores de Marco/Entrada/Saida nasceram como LEGENDA de blockout,
// para o traçado ser legivel enquanto nao havia arte nenhuma. Com a cidade
// vestida elas viraram defeito medido: o ambar do Marco vaza por baixo e ao redor
// de cada peca (um poste tem 23 px de largura numa celula de 43 px, entao sobra
// dos dois lados) e as ancoras que ainda nao receberam peca ficam como quadrados
// chapados no meio da rua - o laudo mediu o equivalente a 23 celulas inteiras de
// ambar a mostra. Apagar a legenda consertaria a tela e mataria junto a
// ferramenta de quem traça o nivel, que le o mapa POR essas cores.
//
// Entao a legenda nao foi apagada: ela virou uma LEITURA, e existem duas.
//   TileReading::Production (default) - Marco/Entrada/Saida saem na cor do Chao.
//       Sobra chao e parede; quem marca o lugar passa a ser a ARTE da peca. E o
//       que o jogador ve, e e o default de proposito: nenhum caminho de
//       inicializacao consegue ESQUECER de limpar a tela.
//   TileReading::Blockout - a legenda inteira de volta, exatamente como era.
//       Ligada pela env var GUSWORLD_TILE_PALETTE=blockout (ver
//       city_scene.hpp::resolve_tile_palette), mesmo padrao de GUSWORLD_MAPS e
//       GUSWORLD_RENDER2D_BACKEND.
//
// CORES (RGBA em [0,1]; o lider validou os valores no display 2026-06-23):
//   Chao    cinza-medio   - piso andavel (fundo neutro).
//   Parede  escura-azulada- solido (igual a wall_color legada do M1, continuidade).
//   Marco   ambar         - LEGENDA: ponto de interesse (landmark).
//   Entrada verde         - LEGENDA: por onde se entra na area.
//   Saida   azul          - LEGENDA: por onde se sai da area.
// Chao e Parede sao os MESMOS nas duas leituras: o que muda e so a legenda.
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

// Qual das duas leituras do mapa a paleta serve (ver o cabecalho).
enum class TileReading : int {
    Production = 0,  // o que o jogador ve: so chao e parede, a arte marca o lugar
    Blockout = 1,    // a ferramenta de quem traça o nivel: a legenda inteira
};

// Cores de piso e solido. Iguais nas DUAS leituras - o traçado do mapa (onde ha
// massa construida e onde se anda) tem de ser legivel sempre.
inline constexpr gus::platform::render2d::DrawColor kTileChao{0.35f, 0.35f, 0.38f,
                                                              1.0f};
inline constexpr gus::platform::render2d::DrawColor kTileParede{0.18f, 0.20f, 0.28f,
                                                                1.0f};

// Cores da LEGENDA de blockout. So aparecem na leitura Blockout; na de producao
// estes tres TileKinds saem na cor do Chao.
inline constexpr gus::platform::render2d::DrawColor kLegendMarco{0.85f, 0.62f, 0.18f,
                                                                 1.0f};
inline constexpr gus::platform::render2d::DrawColor kLegendEntrada{0.25f, 0.70f,
                                                                   0.35f, 1.0f};
inline constexpr gus::platform::render2d::DrawColor kLegendSaida{0.25f, 0.45f, 0.85f,
                                                                 1.0f};

// Cor de cada TileKind. Os defaults sao a leitura de PRODUCAO (a legenda nao
// pinta): assim a tela limpa e o estado de repouso do jogo, e mostrar a legenda e
// um ato deliberado - nunca o contrario.
struct TilePalette {
    gus::platform::render2d::DrawColor chao = kTileChao;
    gus::platform::render2d::DrawColor parede = kTileParede;
    gus::platform::render2d::DrawColor marco = kTileChao;
    gus::platform::render2d::DrawColor entrada = kTileChao;
    gus::platform::render2d::DrawColor saida = kTileChao;

    TileReading reading = TileReading::Production;
};

// A paleta de BLOCKOUT: a legenda de volta, sobre o mesmo chao e a mesma parede.
[[nodiscard]] inline constexpr TilePalette blockout_palette() noexcept {
    TilePalette p;
    p.marco = kLegendMarco;
    p.entrada = kLegendEntrada;
    p.saida = kLegendSaida;
    p.reading = TileReading::Blockout;
    return p;
}

// A paleta de PRODUCAO, explicita (identica ao default - existe para o chamador
// poder DIZER qual leitura quer, em vez de depender de saber qual e o default).
[[nodiscard]] inline constexpr TilePalette production_palette() noexcept {
    return TilePalette{};
}

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
