// GusEngine/app/tests/maestro_logic_test.cpp
//
// Catch2 (headless, SEM SDL_Init/janela) da logica PURA da Maestro (M7-COSTURA, ADR-012
// Onda 1): aabb_overlaps, should_trigger_battle, outcome_marks_enemy_defeated e
// pick_fixed_enemy_position. Ver gus/app/maestro_logic.hpp.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "gus/app/maestro_logic.hpp"

using gus::app::aabb_overlaps;
using gus::app::EncounterId;
using gus::app::outcome_marks_enemy_defeated;
using gus::app::pick_fixed_enemy_position;
using gus::app::should_trigger_battle;
using gus::core::spatial::Aabb;
using gus::core::spatial::TileGrid;
using gus::domain::combat::CombatOutcome;

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

TEST_CASE("EncounterId: kFixedEnemy1 existe (item 1 do escopo M7-COSTURA)",
          "[maestro][logic]") {
    constexpr auto id = EncounterId::kFixedEnemy1;
    CHECK(static_cast<int>(id) == 0);
}
