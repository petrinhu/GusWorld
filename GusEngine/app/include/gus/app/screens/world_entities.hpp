// SPDX-License-Identifier: Apache-2.0
// gus/app/screens/world_entities.hpp
//
// O QUE EXISTE NO MUNDO ALÉM DO CHÃO (DEMO-CIDADE-VESTIDA fatia B1/B2/B3).
//
// Até esta fatia o overworld desenhava células de cor chapada mais DOIS SLOTS
// FIXOS de personagem (um inimigo, um NPC), cada um com seus próprios campos,
// suas próprias funções e sua própria linha no Y-sort. Acrescentar o terceiro
// personagem custava reescrever os três lugares; acrescentar uma casa não era
// possível de forma nenhuma.
//
// Aqui ficam os dois tipos de coisa que o OverworldSim posiciona, desenha e faz
// colidir, e os DOIS são lista:
//   - ScenePropInstance: peça de cenário (casa, poste, fonte, portão...);
//   - WorldActor:        personagem (NPC ou inimigo), opcionalmente em ronda.
//
// Estas structs são a forma JÁ RESOLVIDA das peças: o dado do mundo
// (gus::domain::world) diz o tamanho e o bloqueio, a casca (SdlWindow) resolve a
// textura, e o que chega aqui é retângulo em unidades de mundo mais um TextureId.
// Nenhuma delas conhece SDL, arquivo ou o catálogo - por isso o OverworldSim
// continua testável sem janela.
//
// Cross-ref: gus/domain/world/scene_prop.hpp (o catálogo, uma linha por peça);
//            gus/domain/world/patrol_route.hpp (a ronda como dado);
//            gus/app/screens/city_props.hpp (quem traduz dado em instância).

#ifndef GUS_APP_SCREENS_WORLD_ENTITIES_HPP
#define GUS_APP_SCREENS_WORLD_ENTITIES_HPP

#include "gus/core/spatial/grid_collision.hpp"  // Aabb
#include "gus/domain/world/patrol_route.hpp"
#include "gus/platform/render2d/i_renderer.hpp"  // TextureId

namespace gus::app::screens {

// Peça de cenário pronta para entrar no mundo.
struct ScenePropInstance {
    // Retângulo DESENHADO, em unidades de mundo (já com a escala aplicada).
    gus::core::spatial::Aabb footprint{};

    // Caixa que BLOQUEIA o jogador. Área zero = peça atravessável.
    gus::core::spatial::Aabb solid{};

    // Textura já resolvida pela casca. Inválida = arte ausente/headless.
    gus::platform::render2d::TextureId tex =
        gus::platform::render2d::kInvalidTexture;

    // true = pintada NO CHÃO, junto com os tiles, fora do Y-sort (tabuleiro de
    // puzzle, marcações de calçada). Nunca esconde ninguém e nunca bloqueia.
    bool ground = false;

    // A CÉLULA que a peça pisa (a mesma do ScenePropPlacement que a originou).
    // A geometria já vem resolvida acima, então isto não serve para desenhar:
    // serve para o sim saber QUAL célula esta peça VESTE, e não pintar a marcação
    // de graybox por baixo dela (achado A3 do laudo visual - a caixa de cobertura
    // É a célula de Parede, e pintar a parede atrás dela a deixa "pairando sobre
    // um buraco" escuro). Negativo = peça sem célula de origem (montagem manual em
    // teste), que nunca veste nada.
    int cell_x = -1;
    int cell_y = -1;

    [[nodiscard]] bool drawable() const noexcept {
        return tex != gus::platform::render2d::kInvalidTexture;
    }

    // REGRA DE DEGRADAÇÃO (travada por teste): peça que não é desenhável também
    // não bloqueia. Arte ausente vira "a peça não está lá", nunca uma parede
    // invisível - o jogador travaria contra algo que não vê, e o defeito passaria
    // batido no playtest justamente por ser invisível.
    [[nodiscard]] bool blocks() const noexcept {
        return drawable() && solid.w > 0.0f && solid.h > 0.0f;
    }
};

// Papel do personagem no mundo. O OverworldSim NÃO trata os dois de forma
// diferente (desenha e colide igual): o papel existe para quem decide o que
// acontece ao encostar (a Maestro abre diálogo num, entra em batalha no outro).
enum class WorldActorRole : int {
    Npc = 0,
    Enemy = 1,
};

// O que o chamador informa para pôr um personagem no mundo.
struct WorldActorSpec {
    WorldActorRole role = WorldActorRole::Npc;

    // Posição LÓGICA (onde o personagem pisa). O quad desenhado é derivado dela:
    // quadrado de sprite_height_tiles, centrado em X, com a base na base desta
    // caixa - a mesma fórmula de sempre dos marcadores, agora valendo para N.
    gus::core::spatial::Aabb anchor{};

    gus::platform::render2d::TextureId tex =
        gus::platform::render2d::kInvalidTexture;

    // Altura do quad desenhado, em tiles. <= 0 usa a altura do jogador
    // (OverworldTuning::player_sprite_height_tiles) - é dado por ator porque cada
    // retrato tem margem transparente própria (o Bertoldo já provou isso).
    float sprite_height_tiles = 0.0f;

    // Ronda opcional. Rota inválida (o default) = personagem parado.
    gus::domain::world::PatrolRoute route{};
};

// O personagem já no mundo, com o estado que o passo fixo mantém.
struct WorldActor {
    WorldActorRole role = WorldActorRole::Npc;

    gus::core::spatial::Aabb anchor{};       // posição corrente
    gus::core::spatial::Aabb prev_anchor{};  // posição do passo anterior
    gus::core::spatial::Aabb solid{};        // corpo sólido derivado da âncora
    gus::platform::render2d::TextureId tex =
        gus::platform::render2d::kInvalidTexture;
    float sprite_height_tiles = 0.0f;

    gus::domain::world::PatrolRoute route{};
    gus::domain::world::PatrolState patrol{};

    // Âncora no instante em que a rota foi armada, e o primeiro ponto dela. A
    // ronda é aplicada como DESLOCAMENTO em relação a esse par - assim armar uma
    // rota nunca teleporta o ator, mesmo que a célula escrita na tabela não bata
    // exatamente com onde a Maestro o posicionou.
    gus::core::spatial::Aabb route_origin_anchor{};
    gus::domain::world::PatrolWaypoint route_origin_point{};

    // false = removido (derrotado, ou nunca ocupado). O slot PERMANECE na lista:
    // handle de ator é índice, e índice não pode escorregar quando um vizinho sai.
    bool active = false;

    [[nodiscard]] bool drawable() const noexcept {
        return active && tex != gus::platform::render2d::kInvalidTexture;
    }
    [[nodiscard]] bool blocks() const noexcept {
        return drawable() && solid.w > 0.0f && solid.h > 0.0f;
    }
    [[nodiscard]] bool patrolling() const noexcept {
        return active && route.valid();
    }
};

// Handle inválido devolvido/aceito pelas APIs de ator (no-op seguro).
inline constexpr int kInvalidWorldActor = -1;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_WORLD_ENTITIES_HPP
