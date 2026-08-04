// SPDX-License-Identifier: Apache-2.0
// gus/app/screens/city_props.hpp
//
// VESTIR A CIDADE (DEMO-CIDADE-VESTIDA fatia B1): a ponte entre o DADO do mundo
// (gus::domain::world::ScenePropPlacement, que é só "qual peça, em que célula") e
// as instâncias que o OverworldSim desenha e faz colidir.
//
// Três responsabilidades, todas puras exceto o carregamento de textura:
//   1. resolver onde cada peça vai (validando contra o mapa REAL);
//   2. carregar a arte de cada peça do catálogo (a única parte que toca disco,
//      via IRenderer, com o caminho vindo de core/asset_paths.hpp);
//   3. montar as instâncias com a geometria calculada pelo domínio.
//
// POR QUE A TABELA É RELATIVA AO SPAWN (e é PROVISÓRIA): o mapa dos Distritos
// Inferiores está sendo redesenhado de 30x20 para 90x60 em paralelo a esta fatia.
// Uma tabela de células ABSOLUTAS escrita agora seria jogada fora amanhã, e pior,
// apontaria para dentro de paredes novas sem ninguém perceber. Amarrar ao spawn é
// a mesma técnica que o posicionamento do inimigo fixo e do Bertoldo já usam
// (pick_fixed_enemy_position), e é imune à troca de mapa. Quando o level design
// entregar as posições definitivas, elas entram como ScenePropPlacement absoluto
// e esta tabela relativa sai - o resto do caminho não muda.
//
// Cross-ref: gus/domain/world/scene_prop.hpp (o catálogo, uma linha por peça);
//            gus/app/screens/world_entities.hpp (a instância);
//            app/tests/city_props_test.cpp.

#ifndef GUS_APP_SCREENS_CITY_PROPS_HPP
#define GUS_APP_SCREENS_CITY_PROPS_HPP

#include <vector>

#include "gus/app/screens/world_entities.hpp"
#include "gus/core/spatial/grid_collision.hpp"  // Aabb
#include "gus/core/spatial/tile_grid.hpp"
#include "gus/domain/world/scene_prop.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

namespace gus::app::screens {

// Uma linha da vestimenta provisória: peça + deslocamento em CÉLULAS a partir da
// célula onde o jogador nasce.
struct SpawnRelativePropRow {
    gus::domain::world::ScenePropKind kind{};
    int offset_tiles_x = 0;
    int offset_tiles_y = 0;
};

// ===========================================================================
//  VESTIMENTA PROVISÓRIA DOS DISTRITOS INFERIORES
//
//  Uma linha por peça posta na rua. Layout pensado para o demo ser LEGÍVEL, não
//  para ser o level design final: duas casas fechando um lado da praça, a fonte
//  como ponto central, o poste iluminando o caminho do jogador, a placa de lore
//  perto de onde ele nasce (é o primeiro texto que ele encontra), o terminal de
//  hack e a caixa de cobertura na direção do inimigo, o portão sul fechando o
//  fundo, e o tabuleiro de puzzle no chão a caminho do Bertoldo.
//
//  Os offsets do inimigo fixo (-5,+4) e do Bertoldo (-5,+13) estão RESERVADOS -
//  nenhuma peça ocupa nem encosta neles, senão a arte cobriria o personagem com
//  quem o jogador precisa esbarrar.
// ===========================================================================
//  ⚠️ OS OFFSETS FORAM MEDIDOS CONTRA O MAPA REAL, não estimados. As duas
//  primeiras versões desta tabela foram escritas "no olho" e perderam 4 das 10
//  peças - e o jogo não reclamava, só ficava com menos casa. O mapa vigente tem o
//  spawn em (15,1), dentro de um corredor de DUAS células de largura que só abre
//  na linha 4; qualquer peça espalhada para os lados perto do spawn cai em
//  parede. Daí a formação: desce o corredor primeiro, e só então se abre na praça.
//  O smoke headless imprime "N de M peças couberam no mapa" justamente para esse
//  erro aparecer no CI, e não num playtest.
inline constexpr SpawnRelativePropRow kDistritosInferioresDressing[] = {
    // Saindo do corredor para a praça: poste de um lado, placa do outro.
    {gus::domain::world::ScenePropKind::PosteNeonCiano, -1, 3},
    {gus::domain::world::ScenePropKind::PlacaLore, 1, 3},
    {gus::domain::world::ScenePropKind::TerminalHack, 5, 3},
    // A praça propriamente dita.
    {gus::domain::world::ScenePropKind::CoverBox, -10, 4},
    {gus::domain::world::ScenePropKind::FonteLatao, 3, 5},
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, -7, 6},
    {gus::domain::world::ScenePropKind::CasaCibergoticaB, -3, 6},
    {gus::domain::world::ScenePropKind::HoloSterling, 7, 6},
    {gus::domain::world::ScenePropKind::BoardPuzzle, -3, 8},
    // O portão fecha a passagem estreita que leva à metade sul do mapa.
    {gus::domain::world::ScenePropKind::PortaoSulFechado, 0, 9},
};

inline constexpr int kDistritosInferioresDressingCount =
    static_cast<int>(std::size(kDistritosInferioresDressing));

// Textura já resolvida de cada peça do catálogo, indexada pelo ScenePropKind.
struct ScenePropTextures {
    gus::platform::render2d::TextureId
        by_kind[gus::domain::world::kScenePropKindCount] = {};

    [[nodiscard]] gus::platform::render2d::TextureId at(
        gus::domain::world::ScenePropKind kind) const noexcept {
        const int i = static_cast<int>(kind);
        if (i < 0 || i >= gus::domain::world::kScenePropKindCount) {
            return gus::platform::render2d::kInvalidTexture;
        }
        return by_kind[i];
    }
};

// Converte a tabela relativa em células absolutas, DESCARTANDO o que não cabe no
// mapa carregado: fora dos limites ou dentro de parede. Descartar é de propósito
// (e avisa no terminal): uma casa dentro de uma parede é o erro mais difícil de
// enxergar no playtest, porque a arte fica meio enterrada e parece proposital.
// `rows == nullptr` ou `count <= 0` devolve lista vazia.
[[nodiscard]] std::vector<gus::domain::world::ScenePropPlacement>
resolve_spawn_relative_props(const gus::core::spatial::TileGrid& grid,
                             const gus::core::spatial::Aabb& player_spawn,
                             const SpawnRelativePropRow* rows, int count);

// Carrega a arte de TODAS as peças do catálogo no renderer corrente. O caminho de
// cada uma é montado a partir do header central de assets (pasta da área + o
// arquivo da linha do catálogo) - nunca literal espalhado. Arte ausente devolve
// textura inválida naquele slot, e a peça simplesmente não entra no mundo.
[[nodiscard]] ScenePropTextures load_scene_prop_textures(
    gus::platform::render2d::IRenderer& renderer);

// Monta as instâncias prontas para o OverworldSim. Peça sem textura é OMITIDA (e
// não adicionada invisível): peça invisível que bloqueia vira parede fantasma, e
// o jogador trava contra algo que não vê.
[[nodiscard]] std::vector<ScenePropInstance> build_scene_prop_instances(
    const gus::domain::world::ScenePropPlacement* placements, int count,
    float tile_size, float scale, const ScenePropTextures& textures);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_CITY_PROPS_HPP
