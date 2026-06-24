// gus/app/screens/city_scene.hpp
//
// Montagem da CENA DA CIDADE real (Distritos Inferiores) a partir de um TileMap
// carregado do .gmap. Substitui a cena de teste hardcoded (test_overworld.hpp) como
// cena default do gusworld_app (M4-visual).
//
// CAMADA app/: a logica AQUI e PURA (sem SDL, sem I/O de arquivo) - so transforma um
// TileMap (ja em memoria) na AABB inicial do jogador (derivada do spawn) e ajuda a
// montar o OverworldSim. O I/O do .gmap (ler bytes do disco) e o load_map ficam na
// fronteira de quem chama (city_loader/main). A resolucao do CAMINHO do .gmap so monta
// uma STRING (env > macro embutido > relativo ao CWD), igual ao resolver de sprites;
// nao abre arquivo.
//
// SPAWN -> AABB: o TileMap da o spawn em CELULA (cx,cy). O jogador nasce CENTRADO
// nessa celula, com uma hitbox de ~0.6 tile (mesma proporcao da cena de teste do M1,
// pe ancorado depois pelo sprite). Centrar na celula evita nascer colado numa borda.
//
// Cross-ref: gus/domain/map/tile_map.hpp (TileMap/Cell/spawn),
//            gus/app/screens/overworld_sim.hpp (consumidor),
//            gus/app/screens/test_overworld.hpp (cena de teste / fallback do M1).

#ifndef GUS_APP_SCREENS_CITY_SCENE_HPP
#define GUS_APP_SCREENS_CITY_SCENE_HPP

#include <string>

#include "gus/app/screens/overworld_sim.hpp"
#include "gus/app/screens/overworld_tuning.hpp"
#include "gus/core/spatial/grid_collision.hpp"  // Aabb
#include "gus/domain/map/tile_map.hpp"

namespace gus::app::screens {

// REGISTRO area -> UUID de identidade (binding anti map-swap, decisao do lider
// 2026-06-23). O jogo sabe qual map_id ESPERAR por slot/area: este e o registro
// minimo (uma area = um slot, no slice). O valor e o MESMO UUID v4 fixo gravado na
// fonte assets/maps/source/distritos_inferiores.csv (#map_id). Mante-los em sincronia
// e a unica disciplina: a fonte gera o selo, este constante valida o binding. Ao
// adicionar uma area, adicione seu par aqui (ou migre para um manifest se crescer).
inline constexpr const char* kDistritosInferioresMapId =
    "5f9a2c14-8d3b-4e07-9a21-3c6f1d8b2e55";

// Lado da hitbox do jogador como FRACAO do tile (~0.6, igual ao M1: 20px num tile de
// 32). A hitbox sao "os pes"; o sprite vaza pra cima (ver overworld_sim render).
inline constexpr float kPlayerHitboxTileFraction = 0.6f;

// AABB inicial do jogador derivada do spawn do TileMap: hitbox de
// kPlayerHitboxTileFraction*tile_size, CENTRADA na celula spawn() (canto sup-esq
// calculado a partir do centro da celula). Pura, deterministica, testavel.
[[nodiscard]] gus::core::spatial::Aabb spawn_player_aabb(
    const gus::domain::map::TileMap& map);

// Monta o OverworldSim da cidade a partir do TileMap: colisao = map.to_tile_grid()
// (so Parede bloqueia), spawn = spawn_player_aabb(map), tuning informado. O TileMap e
// guardado pelo sim pra pintar por TileKind (graybox). Pura (sem SDL/I/O); os sprites
// do jogador sao plugados depois pelo chamador (set_player_sprites na casca SDL).
[[nodiscard]] OverworldSim make_city_scene(gus::domain::map::TileMap map,
                                           const OverworldTuning& tuning);

// Tuning da cidade: default do OverworldTuning. A velocidade de caminhada fica nos
// TILES/s do default (4.5); o tile_size real (2.0m do .gmap) entra via o grid, entao
// a velocidade em mundo se ajusta sozinha. Ponto unico de feel pro lider mexer.
[[nodiscard]] OverworldTuning make_city_tuning();

// Resolve o caminho do .gmap dos Distritos Inferiores (toca std::filesystem). Ordem,
// igual ao resolver de sprites:
//   1) env GUSWORLD_MAPS  -> <env>/distritos_inferiores.gmap;
//   2) GUSWORLD_MAPS_DIR (embutido em compilacao, = GusEngine/assets/maps/compiled);
//   3) relativo ao CWD ("assets/maps/compiled/distritos_inferiores.gmap").
// Devolve o caminho montado mesmo que o arquivo nao exista (o chamador checa o I/O e
// cai pro fallback). Sem leitura de bytes aqui.
[[nodiscard]] std::string resolve_distritos_inferiores_gmap();

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_CITY_SCENE_HPP
