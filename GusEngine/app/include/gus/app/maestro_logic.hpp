// gus/app/maestro_logic.hpp
//
// Logica PURA da Maestro (M7-COSTURA, ADR-012 Onda 1): POCO 100% testavel SEM SDL/janela
// - so as decisoes que a Maestro toma ao orquestrar cidade<->batalha. A classe Maestro
// (gus/app/maestro.hpp, dona da janela/SDL) so CONSOME estas funcoes; a logica em si fica
// testavel headless, no mesmo padrao das funcoes-livres battle_* (ver
// gus/app/screens/battle_preview.hpp + app/tests/battle_key_routing_test.cpp).
//
// Escopo do Incremento 1 (esqueleto do loop, ver ADR-012 Onda 1):
//   - 1 inimigo FIXO no mapa (AABB+id, dado de app/ - NAO toca o formato .gmap/TileMap,
//     que nao tem nocao de inimigo).
//   - colisao jogador<->inimigo dispara a troca cidade->batalha.
//   - Victory marca o inimigo derrotado (some do mapa); Defeat/Fled/Ongoing (janela
//     fechada no meio) NAO marcam - o inimigo continua la, o jogador tenta de novo. O
//     FLAVOR da derrota (reboot/bark/tela-xadrez, Incremento 3) fica pra depois; aqui e
//     so o roteamento binario "o inimigo some ou nao".

#ifndef GUS_APP_MAESTRO_LOGIC_HPP
#define GUS_APP_MAESTRO_LOGIC_HPP

#include "gus/core/spatial/grid_collision.hpp"  // Aabb
#include "gus/core/spatial/tile_grid.hpp"       // TileGrid (posicionamento do inimigo)
#include "gus/domain/combat/combat_enums.hpp"   // CombatOutcome

namespace gus::app {

// Identidade do encontro (item 1 do escopo do M7: "1 inimigo FIXO"). Nasce parametrizado
// (enum, nao bool) pra crescer sem reescrever o contrato de to_battle()/on_battle_result()
// quando uma onda futura adicionar mais encontros - so este 1 valor e exercitado agora.
enum class EncounterId : int {
    kFixedEnemy1 = 0,
};

// true se os AABBs a e b (canto sup-esq + w/h) se sobrepoem (meio-aberto: bordas
// coincidentes NAO contam como sobreposicao). POCO puro, SEM SDL - a versao PUBLICA/
// testavel do mesmo teste que overworld_sim.cpp ja faz internamente (anonymous
// namespace, sem duplicar a formula "as cegas": a Maestro reusa esta).
[[nodiscard]] bool aabb_overlaps(const gus::core::spatial::Aabb& a,
                                  const gus::core::spatial::Aabb& b) noexcept;

// true se o jogador deve ENTRAR EM BATALHA agora: o inimigo NAO foi derrotado ainda E o
// AABB do jogador sobrepoe o AABB do inimigo. Encapsula a regra "esbarrar dispara
// batalha; inimigo derrotado nunca mais dispara" (item 4 do escopo do Incremento 1).
[[nodiscard]] bool should_trigger_battle(const gus::core::spatial::Aabb& player,
                                          const gus::core::spatial::Aabb& enemy,
                                          bool enemy_defeated) noexcept;

// Roteamento outcome -> acao (item 4 do escopo): Victory marca o inimigo derrotado;
// qualquer outro resultado (Defeat, Fled, ou Ongoing se a janela fechou no meio do
// combate) NAO marca.
[[nodiscard]] bool outcome_marks_enemy_defeated(
    gus::domain::combat::CombatOutcome outcome) noexcept;

// Escolhe a posicao do inimigo FIXO: parte da celula de (player_start.x+w/2,
// player_start.y+h/2) + o offset cardinal (offset_tiles_x, offset_tiles_y) em CELULAS, e
// VARRE em espiral (raio crescente) pela celula ALCANCAVEL A PE mais proxima daquele
// alvo - "alcancavel" = mesma componente conectada do spawn (flood-fill 4-conectado
// sobre TileGrid::is_blocked, a MESMA nocao de "andavel" que o movimento real do
// jogador usa via resolve_move/resolve_move_with_corner_assist). Isto e MAIS FORTE que
// so "celula sem parede": uma celula pode ser chao livre mas estar isolada numa sala
// sem porta (bug real corrigido no M7-COSTURA - o offset caia dentro de uma sala
// fechada de distritos_inferiores.gmap, impossivel de alcancar andando); agora o
// inimigo NUNCA cai numa celula que o jogador nao consiga pisar partindo do proprio
// spawn. Se nao houver NENHUMA celula alcancavel alem do proprio spawn (mapa
// degenerado), cai de volta na propria celula de spawn do jogador (garantidamente
// alcancavel - e onde ele nasceu). Devolve a AABB do inimigo com o MESMO w/h de
// player_start, centrada na celula escolhida. POCO puro (so consome
// TileGrid::is_blocked/world_to_cell/width/height, ja permitido em app/).
[[nodiscard]] gus::core::spatial::Aabb pick_fixed_enemy_position(
    const gus::core::spatial::TileGrid& grid,
    const gus::core::spatial::Aabb& player_start, int offset_tiles_x,
    int offset_tiles_y) noexcept;

}  // namespace gus::app

#endif  // GUS_APP_MAESTRO_LOGIC_HPP
