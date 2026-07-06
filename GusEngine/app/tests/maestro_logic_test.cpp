// GusEngine/app/tests/maestro_logic_test.cpp
//
// Catch2 (headless, SEM SDL_Init/janela) da logica PURA da Maestro (M7-COSTURA, ADR-012
// Onda 1): aabb_overlaps, should_trigger_battle, outcome_marks_enemy_defeated e
// pick_fixed_enemy_position. Ver gus/app/maestro_logic.hpp.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "gus/app/maestro_logic.hpp"

using gus::app::aabb_overlaps;
using gus::app::battle_crossfade_target;
using gus::app::crossfade_music;
using gus::app::EncounterId;
using gus::app::enemy_sprite_footprint_aabb;
using gus::app::outcome_marks_enemy_defeated;
using gus::app::pick_fixed_enemy_position;
using gus::app::should_stop_running_after_battle;
using gus::app::should_trigger_battle;
using gus::app::should_trigger_battle_on_edge;
using gus::core::spatial::Aabb;
using gus::core::spatial::TileGrid;
using gus::domain::combat::CombatOutcome;
using gus::platform::audio::AudioEngine;
using gus::platform::audio::kInvalidSound;
using gus::platform::audio::SoundId;

TEST_CASE("aabb_overlaps: sobreposicao franca", "[maestro][logic]") {
    const Aabb a{0.0f, 0.0f, 10.0f, 10.0f};
    const Aabb b{5.0f, 5.0f, 10.0f, 10.0f};
    CHECK(aabb_overlaps(a, b));
    CHECK(aabb_overlaps(b, a));  // simetrico
}

TEST_CASE("aabb_overlaps: separados nao sobrepoem", "[maestro][logic]") {
    const Aabb a{0.0f, 0.0f, 10.0f, 10.0f};
    const Aabb b{100.0f, 100.0f, 10.0f, 10.0f};
    CHECK_FALSE(aabb_overlaps(a, b));
}

TEST_CASE("aabb_overlaps: bordas coincidentes NAO contam (meio-aberto)",
          "[maestro][logic]") {
    const Aabb a{0.0f, 0.0f, 10.0f, 10.0f};
    const Aabb b{10.0f, 0.0f, 10.0f, 10.0f};  // encosta exatamente em x=10
    CHECK_FALSE(aabb_overlaps(a, b));
}

TEST_CASE("should_trigger_battle: inimigo vivo + sobreposicao -> true",
          "[maestro][logic]") {
    const Aabb player{0.0f, 0.0f, 10.0f, 10.0f};
    const Aabb enemy{5.0f, 5.0f, 10.0f, 10.0f};
    CHECK(should_trigger_battle(player, enemy, /*enemy_defeated=*/false));
}

TEST_CASE("should_trigger_battle: sem sobreposicao -> false", "[maestro][logic]") {
    const Aabb player{0.0f, 0.0f, 10.0f, 10.0f};
    const Aabb enemy{100.0f, 100.0f, 10.0f, 10.0f};
    CHECK_FALSE(should_trigger_battle(player, enemy, /*enemy_defeated=*/false));
}

TEST_CASE("should_trigger_battle: inimigo DERROTADO nunca dispara, mesmo sobreposto",
          "[maestro][logic]") {
    const Aabb player{0.0f, 0.0f, 10.0f, 10.0f};
    const Aabb enemy{5.0f, 5.0f, 10.0f, 10.0f};  // sobreposto
    CHECK_FALSE(should_trigger_battle(player, enemy, /*enemy_defeated=*/true));
}

TEST_CASE("outcome_marks_enemy_defeated: so Victory marca", "[maestro][logic]") {
    CHECK(outcome_marks_enemy_defeated(CombatOutcome::Victory));
    CHECK_FALSE(outcome_marks_enemy_defeated(CombatOutcome::Defeat));
    CHECK_FALSE(outcome_marks_enemy_defeated(CombatOutcome::Fled));
    CHECK_FALSE(outcome_marks_enemy_defeated(CombatOutcome::Ongoing));
}

TEST_CASE("pick_fixed_enemy_position: celula-alvo livre -> senta exatamente nela",
          "[maestro][logic]") {
    // Grade 10x10 tudo livre, tile_size=2.0. Spawn no canto sup-esq (celula 1,1).
    TileGrid grid(10, 10, 2.0f);
    const Aabb player_start{2.0f + 0.5f, 2.0f + 0.5f, 1.0f, 1.0f};  // centro ~ celula(1,1)

    const Aabb enemy = pick_fixed_enemy_position(grid, player_start,
                                                  /*offset_tiles_x=*/3,
                                                  /*offset_tiles_y=*/0);

    // Celula-alvo = (1+3, 1) = (4,1), livre -> senta exatamente centrada nela.
    const float expected_cx = (4.0f + 0.5f) * 2.0f;
    const float expected_cy = (1.0f + 0.5f) * 2.0f;
    CHECK(enemy.w == player_start.w);
    CHECK(enemy.h == player_start.h);
    CHECK(enemy.x == Catch::Approx(expected_cx - enemy.w * 0.5f));
    CHECK(enemy.y == Catch::Approx(expected_cy - enemy.h * 0.5f));
}

TEST_CASE("pick_fixed_enemy_position: celula-alvo bloqueada -> vizinha livre mais proxima",
          "[maestro][logic]") {
    TileGrid grid(10, 10, 2.0f);
    const Aabb player_start{0.5f, 0.5f, 1.0f, 1.0f};  // spawn na celula (0,0)

    // Bloqueia a celula-alvo EXATA (offset 2,0 a partir do spawn -> celula (2,0)).
    grid.set_blocked(2, 0, true);

    const Aabb enemy =
        pick_fixed_enemy_position(grid, player_start, /*offset_tiles_x=*/2,
                                   /*offset_tiles_y=*/0);

    // Nao pode ter caido na celula bloqueada (2,0).
    const float blocked_cx = (2.0f + 0.5f) * 2.0f - enemy.w * 0.5f;
    CHECK(enemy.x != Catch::Approx(blocked_cx));

    // A celula escolhida deve estar LIVRE de fato.
    const int cx = grid.world_to_cell(enemy.x + enemy.w * 0.5f);
    const int cy = grid.world_to_cell(enemy.y + enemy.h * 0.5f);
    CHECK_FALSE(grid.is_blocked(cx, cy));
}

TEST_CASE("pick_fixed_enemy_position: tudo bloqueado ao redor -> cai no spawn (livre)",
          "[maestro][logic]") {
    // Grade pequena so com a celula de spawn livre (todo o resto bloqueado).
    TileGrid grid(5, 5, 2.0f);
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 5; ++x) {
            if (x != 0 || y != 0) {
                grid.set_blocked(x, y, true);
            }
        }
    }
    const Aabb player_start{0.5f, 0.5f, 1.0f, 1.0f};  // celula (0,0), a UNICA livre

    const Aabb enemy =
        pick_fixed_enemy_position(grid, player_start, /*offset_tiles_x=*/3,
                                   /*offset_tiles_y=*/0);

    const int cx = grid.world_to_cell(enemy.x + enemy.w * 0.5f);
    const int cy = grid.world_to_cell(enemy.y + enemy.h * 0.5f);
    CHECK(cx == 0);
    CHECK(cy == 0);
    CHECK_FALSE(grid.is_blocked(cx, cy));
}

TEST_CASE(
    "pick_fixed_enemy_position: alvo cai numa SALA FECHADA isolada -> nao senta "
    "la dentro, cai em celula ALCANCAVEL a pe a partir do spawn",
    "[maestro][logic][regressao-sala-fechada]") {
    // Reproducao minima do bug real (lider, playtest ao vivo em
    // distritos_inferiores.gmap): uma sala fechada (chao livre, mas SEM porta) senta
    // exatamente onde o offset alvo aponta. A sala fechada e livre (is_blocked ==
    // false), entao a espiral antiga (so "sem parede") caia direto nela; a nova
    // (alcancabilidade via flood-fill) tem que desviar.
    //
    // Layout (8 linhas x 12 colunas, tile_size=2.0):
    //   y0: ############   <- borda solida
    //   y1: #.....##...#   <- spawn mora aqui (col1-5, livres); col6/7 = separador
    //                         solido; col8-10 = SALA FECHADA (livre, mas sem porta)
    //   y2: #.....##...#   <- mesmo padrao (sala fechada tem 2 linhas de altura)
    //   y3: #.....######   <- sala fechada SELADA por baixo tambem (sem porta em
    //   y4: #.....######      lugar nenhum: cercada em todos os 4 lados)
    //   y5: #.....######
    //   y6: #.....######
    //   y7: ############
    const TileGrid grid = TileGrid::from_rows(
        {
            "############",
            "#.....##...#",
            "#.....##...#",
            "#.....######",
            "#.....######",
            "#.....######",
            "#.....######",
            "############",
        },
        2.0f);

    // Spawn no meio da sala grande (celula (1,1), dentro da area livre col1-5).
    const Aabb player_start{2.5f, 2.5f, 1.0f, 1.0f};

    // Offset (7,0) a partir do spawn (1,1) -> celula-alvo (8,1): CAI DENTRO da sala
    // fechada (col8-10, y1-2) - exatamente o cenario do bug real.
    const Aabb enemy =
        pick_fixed_enemy_position(grid, player_start, /*offset_tiles_x=*/7,
                                   /*offset_tiles_y=*/0);

    const int cx = grid.world_to_cell(enemy.x + enemy.w * 0.5f);
    const int cy = grid.world_to_cell(enemy.y + enemy.h * 0.5f);

    // NUNCA dentro da sala fechada (col8-10, y1-2).
    const bool inside_sala_fechada = (cx >= 8 && cx <= 10 && cy >= 1 && cy <= 2);
    CHECK_FALSE(inside_sala_fechada);

    // A celula escolhida tem que estar na area GRANDE (alcancavel a pe do spawn):
    // col1-5, y1-6.
    CHECK(cx >= 1);
    CHECK(cx <= 5);
    CHECK(cy >= 1);
    CHECK(cy <= 6);
    CHECK_FALSE(grid.is_blocked(cx, cy));
}

TEST_CASE(
    "pick_fixed_enemy_position: alvo alcancavel so por um DESVIO (contorna parede) "
    "-> ainda assim senta EXATAMENTE no alvo (distancia/direcao do offset "
    "preservadas)",
    "[maestro][logic]") {
    // Layout (5 linhas x 7 colunas, tile_size=2.0): uma parede no meio da sala
    // (coluna 3, linhas 1-2) separa o lado do spawn (col1-2) do lado do alvo
    // (col4-5), mas a linha 3 esta ABERTA - da pra contornar por baixo. O alvo
    // (offset 3,0 a partir do spawn (1,1) -> celula (4,1)) e alcancavel, so que via
    // um caminho mais longo (nao em linha reta).
    //   y0: #######
    //   y1: #..#..#   <- spawn(1,1) do lado esquerdo; parede na col3; alvo(4,1) do
    //                     lado direito
    //   y2: #..#..#   <- mesma parede continua
    //   y3: #.....#   <- linha ABERTA: da pra contornar (col1-5 livres)
    //   y4: #######
    const TileGrid grid = TileGrid::from_rows(
        {
            "#######",
            "#..#..#",
            "#..#..#",
            "#.....#",
            "#######",
        },
        2.0f);

    const Aabb player_start{2.5f, 2.5f, 1.0f, 1.0f};  // celula (1,1)

    const Aabb enemy =
        pick_fixed_enemy_position(grid, player_start, /*offset_tiles_x=*/3,
                                   /*offset_tiles_y=*/0);

    // Celula-alvo = (1+3, 1) = (4,1): alcancavel via desvio (linha y3) -> senta
    // EXATAMENTE nela, como se estivesse em campo aberto (a distancia do caminho nao
    // importa, so a ALCANCABILIDADE).
    const float expected_cx = (4.0f + 0.5f) * 2.0f;
    const float expected_cy = (1.0f + 0.5f) * 2.0f;
    CHECK(enemy.x == Catch::Approx(expected_cx - enemy.w * 0.5f));
    CHECK(enemy.y == Catch::Approx(expected_cy - enemy.h * 0.5f));
}

TEST_CASE(
    "pick_fixed_enemy_position: alvo cai numa ilha LIVRE mas totalmente ISOLADA (sem "
    "NENHUMA conexao) -> nao senta na ilha (bug antigo), cai no fallback (spawn)",
    "[maestro][logic][regressao-sala-fechada]") {
    // Prova direta da regressao: com a espiral ANTIGA (so "sem parede"), esta celula
    // livre-mas-isolada seria escolhida (e o bug relatado pelo lider). Aqui o spawn
    // esta sozinho numa cela 1x1 (sem vizinhos livres) e a celula-alvo do offset e
    // OUTRA cela 1x1 livre, tambem totalmente isolada - nenhuma das duas tem QUALQUER
    // conexao com a outra. O flood-fill garante que so o spawn e "alcancavel" (ele
    // mesmo), entao o resultado tem que ser o fallback (spawn), NUNCA a ilha.
    //   y0: ########
    //   y1: #.####.#   <- spawn na col1 (isolado); ilha livre na col6 (isolada)
    //   y2: ########
    const TileGrid grid = TileGrid::from_rows(
        {
            "########",
            "#.####.#",
            "########",
        },
        2.0f);

    const Aabb player_start{2.5f, 2.5f, 1.0f, 1.0f};  // celula (1,1), a UNICA vizinha

    // Offset (5,0) a partir do spawn (1,1) -> celula-alvo (6,1): a ILHA livre (o bug
    // antigo cairia exatamente aqui, pois a celula e is_blocked==false).
    const Aabb enemy =
        pick_fixed_enemy_position(grid, player_start, /*offset_tiles_x=*/5,
                                   /*offset_tiles_y=*/0);

    const int cx = grid.world_to_cell(enemy.x + enemy.w * 0.5f);
    const int cy = grid.world_to_cell(enemy.y + enemy.h * 0.5f);

    // NAO pode ter caido na ilha isolada (6,1) - essa era a assinatura do bug.
    CHECK_FALSE((cx == 6 && cy == 1));

    // Fallback: cai exatamente na propria celula de spawn (1,1), a unica alcancavel.
    CHECK(cx == 1);
    CHECK(cy == 1);
}

TEST_CASE(
    "pick_fixed_enemy_position: mapa REAL (distritos_inferiores) - offset "
    "(-5,+4) poe o inimigo no SALAO principal esquerdo, NAO no portal "
    "entrada_norte",
    "[maestro][logic][regressao-sala-fechada]") {
    // Reproducao FIEL de GusEngine/assets/maps/source/distritos_inferiores.csv (30x20,
    // tile_size 2.0; '#' == Parede/id 1, '.' == qualquer outro id: Chao/Marco/
    // Entrada/Saida, nenhum deles bloqueia - ver TileKind::is_tile_blocking). Prova
    // dupla, pedida pelo lider:
    //   1) o offset ANTIGO (3,0) - celula-alvo (18,1) - cai numa saleta isolada (canto
    //      sup-direito, cols17-28/rows1-2, so ligada ao resto do mapa por um caminho
    //      longo) e o fallback da espiral (alcancabilidade) senta na celula alcancavel
    //      mais proxima: (15,0), EM CIMA do portal entrada_norte. Feio, mas nao mais um
    //      "sala fechada" (a alcancabilidade ja protegia disso) - so um mau lugar.
    //   2) o offset NOVO (-5,+4) - celula-alvo (10,5) - e Chao aberto, ja alcancavel
    //      DIRETO (a espiral senta exatamente nele, sem fallback), bem no meio do
    //      salao principal esquerdo (onde o jogador cai ao descer do spawn), a 5
    //      celulas (chebyshev) do spawn - nem em cima do jogador, nem longe demais.
    const TileGrid grid = TileGrid::from_rows(
        {
            "##############..##############",
            "#............#..#............#",
            "#............#..#............#",
            "#............#..##########...#",
            "#........................#...#",
            "#........................#...#",
            "#........................#...#",
            "#........................#...#",
            "#................##########..#",
            "#.........................#..#",
            "##############...#############",
            "#.........................#..#",
            "#.......................#.#..#",
            "#...........#.....#.....###..#",
            "#............................#",
            "#............................#",
            "#############.....############",
            "#............................#",
            "#............................#",
            "##############..##############",
        },
        2.0f);

    // Spawn real (#spawn 15 1 no CSV): centro da celula (15,1).
    const Aabb player_start{15.0f * 2.0f + 1.0f, 1.0f * 2.0f + 1.0f, 1.0f, 1.0f};

    // 1) offset ANTIGO (3,0): NAO senta na celula-alvo isolada (18,1) nem sobra la -
    // cai no fallback alcancavel mais proximo, que e o portal (15,0). Documenta POR
    // QUE o offset antigo era ruim (motivou trocar pra (-5,+4)), sem reintroduzir o
    // bug de sala fechada (ja coberto pelos testes sinteticos acima).
    const Aabb enemy_old =
        pick_fixed_enemy_position(grid, player_start, /*offset_tiles_x=*/3,
                                   /*offset_tiles_y=*/0);
    const int old_cx = grid.world_to_cell(enemy_old.x + enemy_old.w * 0.5f);
    const int old_cy = grid.world_to_cell(enemy_old.y + enemy_old.h * 0.5f);
    CHECK(old_cx == 15);
    CHECK(old_cy == 0);  // em cima do portal entrada_norte - por isso foi trocado

    // 2) offset NOVO (-5,+4): senta EXATAMENTE no alvo (10,5) - Chao aberto do salao,
    // alcancavel direto (sem precisar de fallback).
    const Aabb enemy_new =
        pick_fixed_enemy_position(grid, player_start, /*offset_tiles_x=*/-5,
                                   /*offset_tiles_y=*/4);
    const int new_cx = grid.world_to_cell(enemy_new.x + enemy_new.w * 0.5f);
    const int new_cy = grid.world_to_cell(enemy_new.y + enemy_new.h * 0.5f);
    CHECK(new_cx == 10);
    CHECK(new_cy == 5);
    CHECK_FALSE(grid.is_blocked(new_cx, new_cy));

    // Nunca mais no portal, nem na passagem estreita (cols 13-16, rows 0-3) por onde o
    // jogador desce ate o salao.
    const bool on_portal = (new_cx == 15 && new_cy == 0);
    const bool in_passagem = (new_cx >= 13 && new_cx <= 16 && new_cy <= 3);
    CHECK_FALSE(on_portal);
    CHECK_FALSE(in_passagem);
}

TEST_CASE(
    "pick_fixed_enemy_position: mapa REAL (distritos_inferiores) - offset "
    "(-5,+13) do Bertoldo (M7-DIALOGO NPC-MVP) e ALCANCAVEL e DISTINTO da "
    "celula do inimigo fixo (10,5)",
    "[maestro][logic][npc_dialogue]") {
    // MESMA reproducao FIEL do mapa real usada no teste do inimigo fixo acima
    // (GusEngine/assets/maps/source/distritos_inferiores.csv, 30x20, tile_size 2.0).
    // Prova que a MESMA tecnica de posicionamento (pick_fixed_enemy_position +
    // flood-fill de alcancabilidade) reusada para o Bertoldo (offset DIFERENTE,
    // kNpcBertoldoOffsetTilesX/Y em maestro.cpp) senta numa celula andavel, e nao
    // colide com a celula do inimigo fixo (nem com o portal/passagem estreita).
    const TileGrid grid = TileGrid::from_rows(
        {
            "##############..##############",
            "#............#..#............#",
            "#............#..#............#",
            "#............#..##########...#",
            "#........................#...#",
            "#........................#...#",
            "#........................#...#",
            "#........................#...#",
            "#................##########..#",
            "#.........................#..#",
            "##############...#############",
            "#.........................#..#",
            "#.......................#.#..#",
            "#...........#.....#.....###..#",
            "#............................#",
            "#............................#",
            "#############.....############",
            "#............................#",
            "#............................#",
            "##############..##############",
        },
        2.0f);

    // Spawn real (#spawn 15 1 no CSV): centro da celula (15,1) - MESMO ponto de
    // partida do inimigo fixo.
    const Aabb player_start{15.0f * 2.0f + 1.0f, 1.0f * 2.0f + 1.0f, 1.0f, 1.0f};

    const Aabb npc_bertoldo =
        pick_fixed_enemy_position(grid, player_start, /*offset_tiles_x=*/-5,
                                   /*offset_tiles_y=*/13);
    const int npc_cx = grid.world_to_cell(npc_bertoldo.x + npc_bertoldo.w * 0.5f);
    const int npc_cy = grid.world_to_cell(npc_bertoldo.y + npc_bertoldo.h * 0.5f);

    // Alcancavel (chao livre - flood-fill nunca cai fora do mapa/numa parede).
    CHECK_FALSE(grid.is_blocked(npc_cx, npc_cy));

    // DISTINTO da celula do inimigo fixo (10,5) - os dois marcadores nao disputam
    // a mesma area (mesmo offset -5,+4 usado pelo teste do inimigo acima).
    const Aabb enemy = pick_fixed_enemy_position(grid, player_start,
                                                  /*offset_tiles_x=*/-5,
                                                  /*offset_tiles_y=*/4);
    const int enemy_cx = grid.world_to_cell(enemy.x + enemy.w * 0.5f);
    const int enemy_cy = grid.world_to_cell(enemy.y + enemy.h * 0.5f);
    const bool same_cell_as_enemy = (npc_cx == enemy_cx) && (npc_cy == enemy_cy);
    CHECK_FALSE(same_cell_as_enemy);

    // Nunca no portal nem na passagem estreita (cols13-16, rows0-3).
    const bool on_portal = (npc_cx == 15 && npc_cy == 0);
    const bool in_passagem = (npc_cx >= 13 && npc_cx <= 16 && npc_cy <= 3);
    CHECK_FALSE(on_portal);
    CHECK_FALSE(in_passagem);
}

TEST_CASE("EncounterId: kFixedEnemy1 existe (item 1 do escopo M7-COSTURA)",
          "[maestro][logic]") {
    constexpr auto id = EncounterId::kFixedEnemy1;
    CHECK(static_cast<int>(id) == 0);
}

// ============================================================================
// BUG-1 (playtest ao vivo do lider, M7-COSTURA): "a tela de batalha so ativou
// quando toquei o inimigo pelo sul". Ver o comentario de enemy_sprite_footprint_aabb
// em maestro_logic.hpp pra causa raiz. Estes testes travam o FIX: o AABB derivado
// coincide com o retangulo que o marcador visual desenha (mesma formula) e
// should_trigger_battle dispara por QUALQUER direcao de contato.
// ============================================================================

TEST_CASE("enemy_sprite_footprint_aabb: quad quadrado, base=base do anchor, "
          "centrado em X sobre o anchor",
          "[maestro][logic][bug1]") {
    // anchor tipico de pick_fixed_enemy_position: AABB minusculo (0.6 tile,
    // tile_size=2.0 -> 1.2 unidades) centrado na celula (5,5).
    const Aabb anchor{5.0f * 2.0f + 1.0f - 0.6f, 5.0f * 2.0f + 1.0f - 0.6f, 1.2f, 1.2f};
    const float sprite_height_tiles = 2.75f;
    const float tile_size = 2.0f;

    const Aabb fp = enemy_sprite_footprint_aabb(anchor, sprite_height_tiles, tile_size);

    const float expected_side = sprite_height_tiles * tile_size;  // 5.5
    CHECK(fp.w == Catch::Approx(expected_side));
    CHECK(fp.h == Catch::Approx(expected_side));
    // base do quad == base do anchor (bottom_fraction=0, sem foot-inset).
    CHECK(fp.y + fp.h == Catch::Approx(anchor.y + anchor.h));
    // centrado em X sobre o anchor.
    CHECK(fp.x + fp.w * 0.5f == Catch::Approx(anchor.x + anchor.w * 0.5f));
}

TEST_CASE("enemy_sprite_footprint_aabb: IDEMPOTENTE - realimentar o resultado na "
          "MESMA formula de desenho reproduz o MESMO retangulo (hitbox == sprite "
          "visivel, prova matematica de coincidencia)",
          "[maestro][logic][bug1]") {
    const Aabb anchor{12.3f, -4.7f, 1.2f, 1.2f};
    const float sprite_height_tiles = 2.75f;
    const float tile_size = 2.0f;

    const Aabb fp = enemy_sprite_footprint_aabb(anchor, sprite_height_tiles, tile_size);
    // Realimenta fp como se fosse o proprio anchor - overworld_sim.cpp faz exatamente
    // isto ao desenhar (ea = enemy_aabb_, ja o footprint apos o fix).
    const Aabb fp2 = enemy_sprite_footprint_aabb(fp, sprite_height_tiles, tile_size);

    CHECK(fp2.x == Catch::Approx(fp.x));
    CHECK(fp2.y == Catch::Approx(fp.y));
    CHECK(fp2.w == Catch::Approx(fp.w));
    CHECK(fp2.h == Catch::Approx(fp.h));
}

TEST_CASE("should_trigger_battle: com o AABB do footprint (FIX bug1), o jogador "
          "dispara a batalha encostando por N, S, L OU O - nao so pelo sul",
          "[maestro][logic][bug1]") {
    // Reproduz a geometria real: anchor minusculo (0.6 tile = 1.2 unidades,
    // tile_size=2.0) centrado na celula (10,10); jogador do MESMO tamanho do
    // anchor (a hitbox real do jogador, city_scene.cpp kPlayerHitboxTileFraction).
    const float tile_size = 2.0f;
    const float sprite_height_tiles = 2.75f;
    const float player_side = 0.6f * tile_size;  // 1.2 (mesma fracao do jogo real)

    const Aabb anchor{10.0f * tile_size + tile_size * 0.5f - player_side * 0.5f,
                       10.0f * tile_size + tile_size * 0.5f - player_side * 0.5f,
                       player_side, player_side};
    const Aabb enemy = enemy_sprite_footprint_aabb(anchor, sprite_height_tiles, tile_size);

    const float cx = enemy.x + enemy.w * 0.5f;  // centro X do sprite visivel
    const float cy = enemy.y + enemy.h * 0.5f;  // centro Y do sprite visivel

    // Jogador chegando por cada direcao cardinal, parado BEM no meio do sprite
    // visivel (onde o lider efetivamente esbarraria olhando pro androide na tela) -
    // ANTES do fix, so o SUL (jogador colado na base = perto do anchor antigo)
    // disparava; os outros 3 ficavam fantasmas (sem overlap).
    const Aabb player_from_north{cx - player_side * 0.5f, enemy.y - player_side * 0.5f,
                                  player_side, player_side};
    const Aabb player_from_south{cx - player_side * 0.5f,
                                  enemy.y + enemy.h - player_side * 0.5f, player_side,
                                  player_side};
    const Aabb player_from_west{enemy.x - player_side * 0.5f, cy - player_side * 0.5f,
                                 player_side, player_side};
    const Aabb player_from_east{enemy.x + enemy.w - player_side * 0.5f,
                                 cy - player_side * 0.5f, player_side, player_side};
    const Aabb player_center{cx - player_side * 0.5f, cy - player_side * 0.5f,
                              player_side, player_side};

    CHECK(should_trigger_battle(player_from_north, enemy, /*enemy_defeated=*/false));
    CHECK(should_trigger_battle(player_from_south, enemy, /*enemy_defeated=*/false));
    CHECK(should_trigger_battle(player_from_west, enemy, /*enemy_defeated=*/false));
    CHECK(should_trigger_battle(player_from_east, enemy, /*enemy_defeated=*/false));
    CHECK(should_trigger_battle(player_center, enemy, /*enemy_defeated=*/false));
}

TEST_CASE("should_trigger_battle: com o AABB ANTIGO (anchor cru, sem o fix), o "
          "jogador vindo do NORTE/LESTE/OESTE NAO dispara - documenta o bug real "
          "que motivou o fix (regressao)",
          "[maestro][logic][bug1][regressao]") {
    // MESMO anchor minusculo do teste acima, mas usado DIRETO como hitbox (o
    // comportamento ANTES do fix - o que a Maestro fazia ate M7-COSTURA Inc 1).
    const float tile_size = 2.0f;
    const float sprite_height_tiles = 2.75f;
    const float player_side = 0.6f * tile_size;

    const Aabb anchor{10.0f * tile_size + tile_size * 0.5f - player_side * 0.5f,
                       10.0f * tile_size + tile_size * 0.5f - player_side * 0.5f,
                       player_side, player_side};
    // O footprint so serve aqui pra achar onde o SPRITE VISIVEL fica (pra
    // posicionar o jogador "olhando pro androide na tela", como o lider faria) -
    // o hitbox testado abaixo e o `anchor` cru, nao o footprint.
    const Aabb visible = enemy_sprite_footprint_aabb(anchor, sprite_height_tiles,
                                                      tile_size);
    const float cx = visible.x + visible.w * 0.5f;

    // Jogador no meio do sprite visivel, vindo do norte (varios tiles acima do
    // anchor minusculo) - visualmente "tocando" o androide, mas o anchor antigo
    // (colado nos "pes", perto da base) fica longe demais: SEM overlap.
    const Aabb player_from_north{cx - player_side * 0.5f, visible.y - player_side * 0.5f,
                                  player_side, player_side};
    CHECK_FALSE(should_trigger_battle(player_from_north, anchor,
                                       /*enemy_defeated=*/false));

    // Vindo do SUL (aproximando por baixo, sobrepondo a base do anchor - onde o
    // hitbox antigo realmente morava): DISPARA - e por isso que o lider so
    // conseguia entrar em batalha encostando por ali.
    const Aabb player_from_south{anchor.x, anchor.y + anchor.h * 0.5f, anchor.w,
                                  anchor.h};
    CHECK(should_trigger_battle(player_from_south, anchor, /*enemy_defeated=*/false));
}

// ============================================================================
// BUG-3 (playtest ao vivo do lider, M7-COSTURA): "cliquei no X pra fechar durante a
// batalha; a janela reabriu na dungeon e em poucos ms virou batalha de novo; precisei
// pkill". Trava o CONTRATO de roteamento do run() da Maestro: SO o quit_requested vindo
// da batalha encerra o loop da cidade; qualquer outro motivo de retorno (Victory/
// Defeat/Fled/Ongoing) mantem o loop rodando.
// ============================================================================

TEST_CASE("should_stop_running_after_battle: quit_requested=true -> encerra o loop da "
          "cidade (fix bug3: fechar a janela na batalha nao pode reabrir a cidade)",
          "[maestro][logic][bug3]") {
    CHECK(should_stop_running_after_battle(/*battle_requested_quit=*/true));
}

TEST_CASE("should_stop_running_after_battle: false (Victory/Defeat/Fled/Ongoing) -> o "
          "loop da cidade CONTINUA (comportamento de sempre, nao regride)",
          "[maestro][logic][bug3]") {
    CHECK_FALSE(should_stop_running_after_battle(/*battle_requested_quit=*/false));
}

// ============================================================================
// BUG-6 (playtest ao vivo do lider, M7-COSTURA): "apertei fugir, apareceu rapidamente a
// dungeon (eu estava tocando o inimigo) e automaticamente reabriu a arena". A batalha
// deve disparar so na TRANSICAO nao-overlap -> overlap (rising edge), nao ENQUANTO o
// jogador permanece dentro da hitbox - senao a fuga/derrota (que NAO remove o inimigo, e
// devolve o jogador em cima dele) re-dispara em loop. should_trigger_battle_on_edge e a
// funcao PURA; o Maestro guarda o estado do frame anterior (was_overlapping_enemy_).
// ============================================================================

TEST_CASE("should_trigger_battle_on_edge: dispara SO no rising edge (fora->dentro), "
          "NAO enquanto permanece dentro da hitbox",
          "[maestro][logic][bug6]") {
    // Sequencia de frames de overlap conforme o jogador ANDA ate o inimigo e fica em
    // cima: fora, fora, ENTRA, dentro, dentro, SAI, fora, RE-ENTRA.
    // Espera: dispara SO nos 2 rising edges (entrar e re-entrar), nunca no "dentro".
    struct Frame { bool overlap; bool expect_trigger; };
    const Frame frames[] = {
        {false, false},  // longe
        {false, false},  // longe
        {true, true},    // ENTROU -> dispara
        {true, false},   // ainda dentro -> NAO redispara (o BUG-6 disparava aqui)
        {true, false},   // ainda dentro -> NAO
        {false, false},  // saiu
        {false, false},  // longe
        {true, true},    // RE-ENTROU -> dispara de novo (encontro fixo correto)
    };
    bool was = false;  // estado inicial do Maestro (jogador nasce longe do inimigo)
    for (const Frame& f : frames) {
        const bool trig = should_trigger_battle_on_edge(f.overlap, was);
        CHECK(trig == f.expect_trigger);
        was = f.overlap;  // o Maestro atualiza o estado a cada frame
    }
}

TEST_CASE("should_trigger_battle_on_edge: apos FUGA/DERROTA (inimigo permanece, jogador "
          "volta EM CIMA), o trigger fica SUPRIMIDO ate SAIR e RE-ENTRAR",
          "[maestro][logic][bug6]") {
    // Reproduz o roteamento REAL do Maestro::run() (BUG-6): o jogador ENTRA na hitbox
    // (dispara a batalha), FOGE, e volta pra cidade AINDA sobre o inimigo (overlap segue
    // true, pois fuga/derrota NAO removem o inimigo). O Maestro, apos a batalha
    // nao-vitoriosa, seta was_overlapping = should_trigger_battle(...) = TRUE (jogador em
    // cima). Provamos que dai em diante o overlap continuo NAO redispara ate virar false
    // e true de novo.
    bool was = false;

    // 1) jogador ENTRA na hitbox -> rising edge -> dispara.
    CHECK(should_trigger_battle_on_edge(/*overlap=*/true, was) == true);
    // Maestro roda a batalha (FUGA) e, como o inimigo permanece e o jogador segue em
    // cima, atualiza was = should_trigger_battle(...) = true.
    was = true;

    // 2) proximos frames: jogador AINDA sobre o inimigo (overlap true) -> NUNCA redispara.
    for (int i = 0; i < 5; ++i) {
        CHECK(should_trigger_battle_on_edge(/*overlap=*/true, was) == false);
        was = true;  // segue em cima
    }

    // 3) jogador SAI da hitbox (anda pra longe) - sem disparo, e o estado vira false.
    CHECK(should_trigger_battle_on_edge(/*overlap=*/false, was) == false);
    was = false;

    // 4) jogador RE-ENCOSTA - agora SIM redispara (rising edge legitimo do encontro fixo).
    CHECK(should_trigger_battle_on_edge(/*overlap=*/true, was) == true);
}

// ============================================================================
// M7-COSTURA Inc 2 (ADR-012 decisao 5 + paga a divida do ADR-011 "fade entre telas"):
// crossfade_music - o MECANISMO que cruza a musica cronometrado com o fade preto (a
// tela fica 100% preta no instante em que o chamador dispara isto). Testado headless
// com AudioEngine em modo null-device (device_active=false) - mesma receita de
// platform/tests/audio_engine_test.cpp (WAV sintetico em tempfile).
// ============================================================================

namespace {
// Mesmo gerador minimo de WAV PCM16 mono do audio_engine_test.cpp (duplicado de
// proposito: este e um teste da camada app/, que nao deveria depender de um util de
// teste de platform/ so por isto - a duplicacao e barata, 1 tom curto).
std::string write_crossfade_test_tone_wav(const std::filesystem::path& path) {
    constexpr int kSampleRate = 22050;
    constexpr float kDurationS = 0.1f;
    constexpr float kFreqHz = 440.0f;
    const auto num_samples = static_cast<std::uint32_t>(kSampleRate * kDurationS);
    std::vector<std::int16_t> samples(num_samples);
    for (std::uint32_t i = 0; i < num_samples; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(kSampleRate);
        const float s = 0.2f * std::sin(2.0f * 3.14159265f * kFreqHz * t);
        samples[i] = static_cast<std::int16_t>(s * 32767.0f);
    }
    const std::uint32_t data_bytes = num_samples * sizeof(std::int16_t);
    const std::uint32_t byte_rate = static_cast<std::uint32_t>(kSampleRate) * 2;
    const std::uint16_t block_align = 2;
    const std::uint16_t bits_per_sample = 16;
    const std::uint32_t riff_size = 36 + data_bytes;

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    auto w32 = [&out](std::uint32_t v) { out.write(reinterpret_cast<const char*>(&v), 4); };
    auto w16 = [&out](std::uint16_t v) { out.write(reinterpret_cast<const char*>(&v), 2); };
    out.write("RIFF", 4);
    w32(riff_size);
    out.write("WAVE", 4);
    out.write("fmt ", 4);
    w32(16);
    w16(1);
    w16(1);
    w32(static_cast<std::uint32_t>(kSampleRate));
    w32(byte_rate);
    w16(block_align);
    w16(bits_per_sample);
    out.write("data", 4);
    w32(data_bytes);
    out.write(reinterpret_cast<const char*>(samples.data()),
              static_cast<std::streamsize>(data_bytes));
    return path.string();
}
}  // namespace

TEST_CASE("crossfade_music: engine nullptr e no-op seguro (nunca crasha)",
          "[maestro][logic][audio][m7_costura]") {
    crossfade_music(nullptr, /*next_id=*/1, /*loop=*/true, /*fade_seconds=*/0.5f);
    SUCCEED("no-op com engine nulo nao crashou");
}

TEST_CASE("crossfade_music: dispara stop_music + play_music da PROXIMA faixa "
          "(mecanismo do fade preto, M7-COSTURA Inc 2)",
          "[maestro][logic][audio][m7_costura]") {
    AudioEngine engine(/*device_active=*/false);
    const auto tmp =
        std::filesystem::temp_directory_path() / "gusworld_test_crossfade_music.wav";
    write_crossfade_test_tone_wav(tmp);

    const SoundId id = engine.load_music(tmp.string().c_str());
    REQUIRE(id != kInvalidSound);

    // Faixa ja tocando ANTES do crossfade (simula a musica corrente da cidade/batalha).
    engine.play_music(id, /*loop=*/true, /*fade_in_seconds=*/0.0f);
    REQUIRE(engine.music_is_playing());
    REQUIRE(engine.music_play_count() == 1);

    // O crossfade cruza pra faixa dada (aqui, a MESMA - o kit CC0 desta onda so tem 1
    // faixa, ver asset_paths.hpp/kCityThemeFile) - stop_music(fade) + play_music(fade).
    crossfade_music(&engine, id, /*loop=*/true, /*fade_seconds=*/0.6f);

    // stop_music NAO decrementa music_play_count (so conta plays); play_music CONTA
    // mais 1 (a faixa que o crossfade acabou de tocar) e a musica segue tocando.
    CHECK(engine.music_play_count() == 2);
    CHECK(engine.music_is_playing());

    std::filesystem::remove(tmp);
}

TEST_CASE("crossfade_music: kInvalidSound como next_id nao incrementa music_play_count "
          "(degradacao segura - kit sem faixa carregada)",
          "[maestro][logic][audio][m7_costura]") {
    AudioEngine engine(/*device_active=*/false);
    REQUIRE(engine.music_play_count() == 0);

    crossfade_music(&engine, /*next_id=*/kInvalidSound, /*loop=*/true,
                     /*fade_seconds=*/0.5f);

    CHECK(engine.music_play_count() == 0);
    CHECK_FALSE(engine.music_is_playing());
}

// ============================================================================
// M7-COSTURA Inc 3 (liga a faixa de batalha REAL, Arena_GusWorld.mp3): agora que a
// Maestro carrega DUAS faixas (city_music_id_ + battle_music_id_), o crossfade
// cidade->batalha em Maestro::to_battle() precisa mirar a faixa da ARENA de verdade,
// nao mais a mesma faixa da cidade. battle_crossfade_target e o POCO que decide isso -
// testado headless (SoundId e so um inteiro tipado, sem precisar de AudioEngine real).
// ============================================================================

TEST_CASE("battle_crossfade_target: prefere o id da BATALHA quando o load deu certo "
          "(as duas faixas sao DIFERENTES - Inc 3, Arena_GusWorld.mp3)",
          "[maestro][logic][audio][m7_costura]") {
    constexpr SoundId kCityId = 1;
    constexpr SoundId kBattleId = 2;
    CHECK(battle_crossfade_target(kBattleId, kCityId) == kBattleId);
    // As duas faixas continuam DISTINTAS (a prova de que o crossfade cruza pra uma
    // faixa de verdade, nao pra "ela mesma" como antes de Arena_GusWorld.mp3 existir).
    CHECK(battle_crossfade_target(kBattleId, kCityId) != kCityId);
}

TEST_CASE("battle_crossfade_target: kInvalidSound como battle_id cai de volta pro "
          "id da cidade (degradacao segura - load da arena falhou)",
          "[maestro][logic][audio][m7_costura]") {
    constexpr SoundId kCityId = 1;
    CHECK(battle_crossfade_target(kInvalidSound, kCityId) == kCityId);
}

TEST_CASE("battle_crossfade_target: os dois ids invalidos devolve invalido "
          "(degradacao segura ate o fim da cadeia - crossfade_music/play_music ja "
          "no-opam com kInvalidSound, nunca crasha)",
          "[maestro][logic][audio][m7_costura]") {
    CHECK(battle_crossfade_target(kInvalidSound, kInvalidSound) == kInvalidSound);
}
