// gus/core/spatial/grid_collision.hpp
//
// Colisao AABB-desliza-na-grade (M4) - POCO C++ puro, ZERO Qt, ZERO I/O.
// Movimento cinematico puro (sem impulso/fisica), deterministico.
//
// FEEL (decidido pelo lider, nao reabrir): a caixa do personagem DESLIZA ao
// longo das paredes. A resolucao e SEPARADA POR EIXO: tenta mover em X e resolve
// contra as celulas bloqueadas sobrepostas; depois tenta mover em Y a partir do
// X ja resolvido. Bateu numa parede num eixo, continua se movendo no outro
// (estilo Zelda ALttP / Stardew). Nao e "trava ao bater".
//
// CONTRATO:
//   - Aabb: x,y = canto SUPERIOR-ESQUERDO em unidades de mundo; w,h = largura e
//     altura (>= 0). Eixo +X direita, +Y baixo (ver tile_grid.hpp).
//   - resolve_move(grid, box, dx, dy) devolve a posicao final ja resolvida e
//     flags hit_x/hit_y (true se o movimento naquele eixo foi limitado por uma
//     parede). A caixa de entrada NAO e modificada.
//   - Pre-condicao: assume-se que a posicao de entrada nao esta DENTRO de uma
//     parede (estado valido). Borda do mapa e parede (TileGrid::is_blocked).

#ifndef GUS_CORE_SPATIAL_GRID_COLLISION_HPP
#define GUS_CORE_SPATIAL_GRID_COLLISION_HPP

namespace gus::core::spatial {

class TileGrid;

struct Aabb {
    float x = 0.0f;  // canto superior-esquerdo
    float y = 0.0f;
    float w = 0.0f;  // largura
    float h = 0.0f;  // altura
};

struct MoveResult {
    Aabb box;            // posicao final (w/h preservados)
    bool hit_x = false;  // movimento em X foi limitado por parede
    bool hit_y = false;  // movimento em Y foi limitado por parede
};

// ---------------------------------------------------------------------------
// Obstaculos PONTUAIS (M7-COSTURA/M7-DIALOGO, colisao solida de personagem) -
// EXTENSAO de resolve_move/resolve_move_with_corner_assist abaixo.
// ---------------------------------------------------------------------------
//
// CONTEXTO (playtest ao vivo do lider): o Gus atravessava POR CIMA de NPCs/inimigos
// parados (Bertoldo sumia embaixo do sprite do jogador) - so existia a hitbox de
// TRIGGER (dialogo/combate), nunca uma colisao FISICA que bloqueasse ocupar a mesma
// posicao. Padrao canonico (Zelda/Stardew): o corpo do NPC bloqueia o movimento como
// uma "parede pontual" pequena; o jogador contorna pelos tiles adjacentes.
//
// Estes obstaculos NAO fazem parte do tilemap estatico (a TileGrid do mapa/.gmap
// continua intocada) - sao AABBs adicionais, avulsas, passadas a cada chamada.
// Tratados EXATAMENTE como uma parede (mesma resolucao por eixo, "desliza" ao
// redor): um obstaculo que bloquearia o deslocamento clampa o alvo daquele eixo pra
// encostar na borda do obstaculo, escolhendo (entre TODOS os bloqueadores - paredes
// da grade E obstaculos) o que resulta no MENOR deslocamento (o mais restritivo
// vence, ordem-independente). Como sao AABBs livres (nao alinhados a celula), a
// borda de encosto e a borda REAL do obstaculo (obstacle.x/obstacle.x+obstacle.w),
// nao uma face de celula.
struct ObstacleSpan {
    const Aabb* items = nullptr;  // vetor de AABBs de obstaculo (nao possui os dados).
    int count = 0;                // numero de itens validos em `items`.
};

// Resolve um deslocamento desejado (dx,dy) contra a grade, deslizando nas
// paredes (resolucao por eixo: X primeiro, depois Y). Deterministico.
// `obstacles` (opcional, default vazio): obstaculos PONTUAIS adicionais (ver
// ObstacleSpan acima) tratados como paredes extras naquele eixo. Vazio (default)
// reproduz o comportamento LEGADO byte-identico (todo chamador/teste existente,
// que nao passa obstaculos, continua identico).
MoveResult resolve_move(const TileGrid& grid, const Aabb& box, float dx, float dy,
                        ObstacleSpan obstacles = {}) noexcept;

// ---------------------------------------------------------------------------
// Corner-correction (corner-assist) - EXTENSAO do resolve_move (M1, feel do lider)
// ---------------------------------------------------------------------------
//
// FEEL (decidido pelo lider): quando o jogador anda contra uma parede e SO A
// QUINA pega (esta levemente desalinhado com uma abertura adjacente, e a MAIOR
// PARTE da largura perpendicular passaria pela abertura), o movimento o empurra
// lateralmente o suficiente pra alinhar e contornar (estilo Stardew/Zelda/Celeste).
// So ajuda quando HA abertura (celula lateral/diagonal livre); NUNCA atravessa
// parede solida. E uma EXTENSAO: o resolve_move acima fica intacto.
//
// COMO FUNCIONA (resumo): se o eixo do movimento foi bloqueado, tenta um pequeno
// empurrao PERPENDICULAR (ate max_assist_fraction * tile), nos dois sentidos,
// pegando o MENOR que faz o eixo principal deixar de bater (preferindo o lado com
// abertura). O empurrao perpendicular e validado contra a grade (nao atravessa
// parede). Se nenhum empurrao dentro do limite destrava, comporta como resolve_move.

struct CornerAssistOptions {
    // Liga/desliga o corner-assist. Desligado (ou max_assist_fraction == 0)
    // reproduz exatamente o resolve_move atual.
    bool enabled = true;

    // Quanto "perdoa" a quina, como FRACAO do tile (0..1). Default ~0.35: empurra
    // ate ~1/3 do tile pra alinhar; alem disso, considera que a quina pega demais
    // (a maior parte do corpo esta obstruido) e NAO ajuda. O lider ajusta vendo.
    float max_assist_fraction = 0.35f;
};

// Igual ao resolve_move, mas com corner-assist (ver acima). Deterministico.
// Com opts.enabled == false, identico a resolve_move(grid, box, dx, dy, obstacles).
// `obstacles` (opcional, default vazio): MESMOS obstaculos pontuais de resolve_move
// (ver ObstacleSpan acima) - o corner-assist tambem NAO empurra atraves deles (o
// empurrao perpendicular e validado contra grade E obstaculos).
MoveResult resolve_move_with_corner_assist(const TileGrid& grid, const Aabb& box,
                                           float dx, float dy,
                                           const CornerAssistOptions& opts,
                                           ObstacleSpan obstacles = {}) noexcept;

}  // namespace gus::core::spatial

#endif  // GUS_CORE_SPATIAL_GRID_COLLISION_HPP
