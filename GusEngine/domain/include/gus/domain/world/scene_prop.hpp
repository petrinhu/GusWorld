// SPDX-License-Identifier: Apache-2.0
// gus/domain/world/scene_prop.hpp
//
// CATÁLOGO DE PEÇAS DE CENÁRIO (DEMO-CIDADE-VESTIDA fatia B1). POCO puro, ZERO
// SDL/GL/RmlUi/IO (invariante de domain/, engine-design.md seção 2).
//
// LEI DO ÁTOMO (ADR-019 no nível de conteúdo, ADR-020 decisão 5 no nível de
// mundo): objeto novo no mundo é DADO - uma LINHA na tabela kMasterSceneProps -,
// nunca um método par-a-par no código. Não existe (e não pode passar a existir)
// nenhuma função "desenha a casa" ou "colide com o poste": existe UMA fórmula de
// geometria que vale para toda linha da tabela. O dia em que uma peça exigir
// função própria, ela não é peça de cenário: é sistema, e nasce em módulo próprio
// (ADR-020 decisão 1).
//
// O QUE É DADO AQUI, e por isso é ajustável sem reescrever nada:
//   - qual arte (arquivo, via core::assets - caminho nunca hardcoded);
//   - o tamanho, DERIVADO da dimensão do PNG a 32 px/tile (não chutado);
//   - a caixa que BLOQUEIA passagem (largura e altura em tiles, ancorada na base)
//     ou a ausência dela (peça atravessável);
//   - se a peça é pintada NO CHÃO (antes do Y-sort) ou fica de pé no mundo.
//
// UNIDADES: `tile_size` e as AABBs devolvidas estão em UNIDADES DE MUNDO; as
// células de ScenePropPlacement são índices de célula do mapa. A escala visual
// global (`scale`) é botão do líder e vem de fora (app/), não daqui.
//
// Cross-ref: docs/tech/adr/ADR-020-pecas-componiveis-modulo-e-mundo-data-driven.md;
//            gus/core/asset_paths.hpp (os arquivos); domain/tests/scene_prop_test.cpp.

#ifndef GUS_DOMAIN_WORLD_SCENE_PROP_HPP
#define GUS_DOMAIN_WORLD_SCENE_PROP_HPP

#include <string_view>

#include "gus/core/asset_paths.hpp"
#include "gus/core/spatial/grid_collision.hpp"  // Aabb

namespace gus::domain::world {

// Identidade de cada peça desenhada para os Distritos Inferiores. A ORDEM é
// contrato com kMasterSceneProps (travado por teste): índice do enum == índice da
// linha, o que torna a busca O(1) sem switch nenhum.
enum class ScenePropKind : int {
    CasaCibergoticaA = 0,
    CasaCibergoticaB,
    PosteNeonCiano,
    FonteLatao,
    PortaoSulFechado,
    PlacaLore,
    TerminalHack,
    CoverBox,
    BoardPuzzle,
    HoloSterling,
};

inline constexpr int kScenePropKindCount = 10;

// Resolução de autoria da arte de cenário: 32 px equivalem a 1 TILE do mapa.
// MEDIDO nos PNGs entregues (casa 96x96 = 3x3, poste 32x80 = 1x2.5, placa 32x48 =
// 1x1.5, terminal 32x40 = 1x1.25, portão 64x48 = 2x1.5, fonte 96x96 = 3x3,
// tabuleiro 112x80 = 3.5x2.5, caixa 48x48 = 1.5x1.5, holograma 80x96 = 2.5x3) - as
// dez peças caem em múltiplos exatos, então a razão não é convenção arbitrária: é
// a grade em que a arte foi realmente desenhada. Peça nova fora dessa grade
// aparece torta, e é sinal de arte errada, não de constante errada.
inline constexpr float kScenePropArtPixelsPerTile = 32.0f;

// Uma LINHA do catálogo. Tudo que distingue uma peça de outra mora aqui.
struct ScenePropDef {
    ScenePropKind kind{};

    // Arquivo da arte, RELATIVO à pasta da área (core::assets::kWorldPropsDistritosDir).
    std::string_view sprite_file{};

    // Dimensão do PNG em pixels - a fonte do tamanho em tiles (ver width_tiles()).
    float art_px_w = 0.0f;
    float art_px_h = 0.0f;

    // Caixa que BLOQUEIA o jogador, em TILES, ancorada no centro-base do desenho
    // (mesma ancoragem do corpo sólido de NPC/inimigo). Zero em qualquer eixo
    // significa peça ATRAVESSÁVEL - o chamador nem chega a montar obstáculo.
    float solid_w_tiles = 0.0f;
    float solid_h_tiles = 0.0f;

    // true = pintada NO CHÃO junto com os tiles (nunca entra no Y-sort, nunca
    // esconde ninguém). Invariante travado por teste: peça de chão não bloqueia.
    bool ground = false;

    [[nodiscard]] constexpr float width_tiles() const noexcept {
        return art_px_w / kScenePropArtPixelsPerTile;
    }
    [[nodiscard]] constexpr float height_tiles() const noexcept {
        return art_px_h / kScenePropArtPixelsPerTile;
    }
    [[nodiscard]] constexpr bool solid() const noexcept {
        return solid_w_tiles > 0.0f && solid_h_tiles > 0.0f;
    }
};

// ===========================================================================
//  TABELA MESTRA - a cidade inteira se veste a partir daqui.
//
//  As caixas sólidas seguem a regra "bloqueia a BASE, não a silhueta": o que
//  bloqueia é a área que a peça ocupa NO CHÃO, e não o retângulo alto do desenho.
//  É a mesma leitura já canonizada para o corpo sólido de NPC/inimigo
//  (overworld_tuning.hpp::npc_solid_box_tiles) e o padrão de Zelda/Stardew: o
//  jogador anda POR TRÁS do telhado (o Y-sort resolve a profundidade) e é barrado
//  só na parede da frente. Bloquear a silhueta inteira travaria o corredor vários
//  tiles acima da porta.
// ===========================================================================
inline constexpr ScenePropDef kMasterSceneProps[kScenePropKindCount] = {
    // Casa cibergótica A (3x3): fachada inteira barra; o telhado (2 tiles de cima)
    // é só desenho, o jogador passa atrás.
    {ScenePropKind::CasaCibergoticaA, gus::core::assets::kPropCasaCibergoticaAFile,
     96.0f, 96.0f, 3.0f, 1.0f, false},
    // Casa cibergótica B (3x4): mais alta, mesma base de 1 tile.
    {ScenePropKind::CasaCibergoticaB, gus::core::assets::kPropCasaCibergoticaBFile,
     96.0f, 128.0f, 3.0f, 1.0f, false},
    // Poste de neon (1x2.5): só o pé do poste barra - é fino, contornável.
    {ScenePropKind::PosteNeonCiano, gus::core::assets::kPropPosteNeonCianoFile,
     32.0f, 80.0f, 0.6f, 0.5f, false},
    // Fonte de latão (3x3): o tanque barra, a água/borda superior não.
    {ScenePropKind::FonteLatao, gus::core::assets::kPropFonteLatoFile, 96.0f, 96.0f,
     2.4f, 1.2f, false},
    // Portão sul FECHADO (2x1.5): existe para barrar - bloqueia a largura inteira.
    {ScenePropKind::PortaoSulFechado, gus::core::assets::kPropPortaoSulFechadoFile,
     64.0f, 48.0f, 2.0f, 1.0f, false},
    // Placa de lore (1x1.5): poste fino, barra pouco (o jogador chega perto para ler).
    {ScenePropKind::PlacaLore, gus::core::assets::kPropPlacaLoreFile, 32.0f, 48.0f,
     0.8f, 0.4f, false},
    // Terminal de hack (1x1.25): gabinete no chão, barra a base cheia.
    {ScenePropKind::TerminalHack, gus::core::assets::kPropTerminalHackFile, 32.0f,
     40.0f, 0.9f, 0.6f, false},
    // Caixa de cobertura (1.5x1.5): objeto sólido inteiro, é o ponto do jogo em
    // que "se esconder atrás" precisa mesmo barrar.
    {ScenePropKind::CoverBox, gus::core::assets::kPropCoverBoxFile, 48.0f, 48.0f,
     1.4f, 1.0f, false},
    // Tabuleiro de puzzle (3.5x2.5): DESENHO DE CHÃO - o jogador pisa em cima
    // (é onde o puzzle acontece), então nunca bloqueia e nunca entra no Y-sort.
    {ScenePropKind::BoardPuzzle, gus::core::assets::kPropBoardPuzzleFile, 112.0f,
     80.0f, 0.0f, 0.0f, true},
    // Holograma do Sterling (2.5x3): projeção. Atravessável de propósito - luz não
    // barra ninguém, e o susto de atravessar o vilão é leitura correta do mundo.
    {ScenePropKind::HoloSterling, gus::core::assets::kPropHoloSterlingFile, 80.0f,
     96.0f, 0.0f, 0.0f, false},
};

// Linha do catálogo de `kind`. Índice direto (o enum É o índice, travado por
// teste). Valor fora da tabela degrada para a primeira linha em vez de ler fora do
// array - guarda de robustez, não caminho legítimo.
[[nodiscard]] const ScenePropDef& scene_prop_def(ScenePropKind kind) noexcept;

// ONDE a peça fica. É isto que o level designer escreve: qual peça, em que célula.
// CONVENÇÃO: a célula é onde a peça PISA - o desenho fica centrado em X sobre ela e
// com a BASE na base da célula, subindo dali. Nenhuma outra âncora é suportada, de
// propósito (âncora única = não existe peça "meio tile acima" por engano).
struct ScenePropPlacement {
    ScenePropKind kind{};
    int cell_x = 0;
    int cell_y = 0;
};

// Retângulo DESENHADO da peça, em unidades de mundo. `scale` é a escala visual
// global (botão único do líder, ver OverworldTuning::scene_prop_scale): multiplica
// o tamanho SEM tirar a peça do chão (a base fica onde estava).
[[nodiscard]] gus::core::spatial::Aabb scene_prop_footprint(
    const ScenePropPlacement& placement, const ScenePropDef& def, float tile_size,
    float scale) noexcept;

// Idem, buscando a linha do catálogo pela própria placement (conveniência).
[[nodiscard]] gus::core::spatial::Aabb scene_prop_footprint(
    const ScenePropPlacement& placement, float tile_size, float scale) noexcept;

// Caixa que BLOQUEIA, derivada do retângulo desenhado: centrada em X sobre ele,
// base na base dele - a MESMA ancoragem do corpo sólido de NPC/inimigo, para não
// existirem duas convenções de "onde o corpo encosta no chão". Peça atravessável
// devolve caixa degenerada (w == h == 0), que o chamador simplesmente não usa.
[[nodiscard]] gus::core::spatial::Aabb scene_prop_solid(
    const gus::core::spatial::Aabb& footprint, const ScenePropDef& def,
    float tile_size, float scale) noexcept;

}  // namespace gus::domain::world

#endif  // GUS_DOMAIN_WORLD_SCENE_PROP_HPP
