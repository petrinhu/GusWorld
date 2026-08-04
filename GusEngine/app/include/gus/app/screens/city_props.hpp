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
// A TABELA É ABSOLUTA E VEM DO LEVEL DESIGN (fatia C, a costura). Até a fatia B
// ela era relativa ao spawn, porque o mapa estava sendo redesenhado de 30x20 para
// 90x60 e qualquer célula fixa escrita naquele momento apontaria para dentro de
// uma parede nova. O traçado de 90x60 chegou, e com ele as 34 âncoras da §5 do
// blockout: cada linha daqui é uma daquelas âncoras, transcrita, não escolhida.
//
// Cross-ref: docs/design/levels/blockout-distritos-inferiores.md §2 (convenção de
//            âncora) e §5 (a tabela de peça por âncora);
//            gus/domain/world/scene_prop.hpp (o catálogo, uma linha por peça);
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

// O que a célula da tabela PRECISA ser no mapa para a peça ser aceita.
//
// Existe porque o blockout tem duas convenções de âncora, e uma régua só reprova
// metade das peças: peça de rua (casa, fonte, poste) fica numa célula ANDÁVEL, em
// frente à fachada; caixa de cobertura não fica ao lado de um obstáculo, ela É o
// obstáculo - uma célula de Parede solta que a arte cobre. Validar as duas com a
// régua da primeira descartaria toda cobertura da arena em silêncio.
enum class PropCellRule : int {
    WalkableCell = 0,  // a peça pisa em chão livre (a esmagadora maioria)
    WallCell = 1,      // a peça É a parede daquela célula (caixa de cobertura)
};

// Uma linha da vestimenta: peça + a CÉLULA ABSOLUTA do mapa + a régua de validação.
struct CityPropRow {
    gus::domain::world::ScenePropKind kind{};
    int cell_x = 0;
    int cell_y = 0;
    PropCellRule cell_rule = PropCellRule::WalkableCell;
};

// ===========================================================================
//  VESTIMENTA DOS DISTRITOS INFERIORES (90x60)
//
//  Uma linha por peça, na ORDEM DO PERCURSO (norte para sul), transcrita da §5 do
//  blockout. A coluna "por que ali" mora no doc, não aqui: duplicá-la produziria
//  duas fontes de verdade que divergem na primeira revisão de level design.
//
//  ⚠️ AS CÉLULAS NÃO SÃO ESCOLHIDAS AQUI. São as âncoras (tile Marco) que o
//  traçado já reserva, e um teste as confere contra o .gmap REAL célula a célula
//  ("toda peça de rua pisa numa âncora do level design"). As duas primeiras
//  versões desta tabela foram escritas "no olho" contra o mapa antigo e perderam
//  4 das 10 peças sem o jogo reclamar - só ficava com menos casa.
//
//  Fora desta tabela, de propósito:
//   - as 5 âncoras de MARCAÇÃO sem sprite (pedestal Era 3, banco do Bertoldo,
//     fundo do beco morto, save point, pichação). Duas delas viram POSIÇÃO DE
//     ATOR (ver city_actors.hpp); as outras esperam arte;
//   - o PORTÃO SUL (44,55)/(45,55). A peça do catálogo é sólida e o vão é o único
//     do muro sul: plantá-la hoje sela 28 células (o vestíbulo inteiro, o save
//     point e a saída), e nada no jogo ainda abre portão. O próprio blockout §8
//     pede o vão livre até o bloqueio virar regra de jogo. Decisão do líder
//     pendente; quando houver a lógica de abertura, entram duas linhas aqui.
// ===========================================================================
inline constexpr CityPropRow kDistritosInferioresDressing[] = {
    // --- Chegada: alameda e terraço (R1/R2) ---
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 17, 4},
    {gus::domain::world::ScenePropKind::HoloSterling, 35, 4},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 52, 4},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 72, 4},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 40, 7},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 48, 9},
    // --- Praça da Compilação e os dois bairros (R8/R10/R12) ---
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 33, 15},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 56, 15},
    {gus::domain::world::ScenePropKind::TerminalHack, 69, 15},
    {gus::domain::world::ScenePropKind::CasaCibergoticaB, 73, 17},
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 8, 21},
    {gus::domain::world::ScenePropKind::CasaCibergoticaB, 17, 21},
    {gus::domain::world::ScenePropKind::FonteLatao, 43, 21},
    {gus::domain::world::ScenePropKind::CasaCibergoticaA, 82, 21},
    {gus::domain::world::ScenePropKind::PlacaLore, 36, 24},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 33, 27},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 56, 27},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 13, 28},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 66, 28},
    // --- Pátio de Sucata e arena (R14/R15) ---
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 36, 34},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 54, 34},
    // Cobertura: UMA caixa POR CÉLULA de Parede do bloco, de baixo para cima.
    //
    // A convenção do blockout (§2) é que a barricada É a célula de Parede. Os
    // blocos do Pátio de Sucata e da arena têm DUAS células (a §5 chama de "4
    // grupos verticais 1x2 simétricos"), e uma caixa só cobre 1,09 tile de
    // conteúdo: sobrava meio tile de parede nua ACIMA dela, medido pelo laudo
    // visual da fatia D (achado A3). Duas caixas empilhadas cobrem o bloco inteiro
    // e leem como o que a barricada é - caixotes empilhados -, sem tocar em arte,
    // em escala nem no traçado do mapa. Os blocos de UMA célula (Pátio da FIR)
    // seguem com uma caixa só, pelo mesmo critério.
    {gus::domain::world::ScenePropKind::CoverBox, 6, 36, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 6, 37, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 10, 36, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 10, 37, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 38, 36, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 38, 37, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 52, 36, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 52, 37, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 38, 42, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 38, 43, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 52, 42, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 52, 43, PropCellRule::WallCell},
    // --- Corredor-puzzle e Pátio da FIR (R17/R19) ---
    {gus::domain::world::ScenePropKind::BoardPuzzle, 42, 47},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 75, 48},
    {gus::domain::world::ScenePropKind::CoverBox, 70, 49, PropCellRule::WallCell},
    {gus::domain::world::ScenePropKind::CoverBox, 78, 49, PropCellRule::WallCell},
    // --- Rua de ligação e antepraça (R20/R20b) ---
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 10, 53},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 33, 53},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 78, 53},
    {gus::domain::world::ScenePropKind::PosteNeonCiano, 52, 54},
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

// Confere a tabela contra o mapa CARREGADO e devolve os placements que passaram,
// DESCARTANDO (com aviso no terminal) o que não cabe: célula fora dos limites, ou
// célula que contraria a régua da linha. Descartar é de propósito: uma casa dentro
// de uma parede é o erro mais difícil de enxergar no playtest, porque a arte fica
// meio enterrada e parece proposital.
// `rows == nullptr` ou `count <= 0` devolve lista vazia.
[[nodiscard]] std::vector<gus::domain::world::ScenePropPlacement> resolve_city_props(
    const gus::core::spatial::TileGrid& grid, const CityPropRow* rows, int count);

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
