// GusEngine/tests/depth_sort_test.cpp
//
// Spec executavel (Catch2 v3) do Y-SORT (M7-COSTURA/M7-DIALOGO, ordem de desenho por
// profundidade). TEST-FIRST: prova a ORDENACAO pura, SEM GL/janela - overworld_sim.cpp
// consome sort_by_depth pra decidir a SEQUENCIA de draw_* a cada frame (jogador/NPC/
// inimigo entram na MESMA lista ordenavel).
//
// CONTRATO exercitado:
//   - menor depth_key desenha PRIMEIRO (fica atras); maior desenha por ULTIMO (fica
//     na frente) - "quem esta mais embaixo na tela (Y maior) fica na frente";
//   - ordenacao ESTAVEL (empate exato preserva ordem de entrada);
//   - count<=1 e ponteiro nulo sao no-op seguro;
//   - N=3 (jogador+NPC+inimigo, o caso real do overworld) em TODAS as permutacoes de
//     entrada relevantes (jogador acima/abaixo/entre os dois marcadores).

#include <catch2/catch_test_macros.hpp>

#include "gus/core/spatial/depth_sort.hpp"

using gus::core::spatial::DepthEntry;
using gus::core::spatial::sort_by_depth;

TEST_CASE("sort_by_depth: menor Y primeiro (atras), maior Y por ultimo (frente)",
          "[core][spatial][depth_sort]") {
    DepthEntry entries[3] = {
        {50.0f, /*id=*/0},  // "meio"
        {10.0f, /*id=*/1},  // mais "em cima" -> deve ficar PRIMEIRO
        {90.0f, /*id=*/2},  // mais "embaixo" -> deve ficar por ULTIMO
    };
    sort_by_depth(entries, 3);

    REQUIRE(entries[0].id == 1);
    REQUIRE(entries[0].depth_key == 10.0f);
    REQUIRE(entries[1].id == 0);
    REQUIRE(entries[1].depth_key == 50.0f);
    REQUIRE(entries[2].id == 2);
    REQUIRE(entries[2].depth_key == 90.0f);
}

TEST_CASE("sort_by_depth: caso real overworld - jogador ACIMA dos dois marcadores",
          "[core][spatial][depth_sort]") {
    // id 0 = inimigo, 1 = npc, 2 = jogador (ordem de insercao = ordem legada antiga:
    // inimigo, npc, jogador). Jogador com Y MENOR (mais "em cima" na tela) deve
    // desenhar PRIMEIRO (atras dos dois marcadores).
    DepthEntry entries[3] = {
        {40.0f, 0},  // inimigo
        {42.0f, 1},  // npc
        {5.0f, 2},   // jogador, bem acima
    };
    sort_by_depth(entries, 3);

    REQUIRE(entries[0].id == 2);  // jogador desenha primeiro (fica atras)
    REQUIRE(entries[1].id == 0);
    REQUIRE(entries[2].id == 1);
}

TEST_CASE("sort_by_depth: caso real overworld - jogador ABAIXO dos dois marcadores",
          "[core][spatial][depth_sort]") {
    // Jogador com Y MAIOR (mais "embaixo") deve desenhar por ULTIMO (na frente) -
    // o INVERSO da ordem fixa legada (inimigo, npc, jogador SEMPRE por cima).
    DepthEntry entries[3] = {
        {40.0f, 0},  // inimigo
        {42.0f, 1},  // npc
        {99.0f, 2},  // jogador, bem abaixo (mais perto da camera)
    };
    sort_by_depth(entries, 3);

    REQUIRE(entries[0].id == 0);
    REQUIRE(entries[1].id == 1);
    REQUIRE(entries[2].id == 2);  // jogador desenha por ultimo (fica na frente)
}

TEST_CASE("sort_by_depth: jogador ENTRE os dois marcadores (passando do lado)",
          "[core][spatial][depth_sort]") {
    // O caso do bug reportado: Gus aproximando do Bertoldo pelo NORTE (Y menor que
    // o NPC) - o jogador deve ficar ATRAS do NPC (Y do NPC > Y do jogador). Se o
    // jogador tivesse Y MAIOR (aproximando pelo SUL), ficaria na FRENTE.
    DepthEntry entries[2] = {
        {50.0f, /*id=*/100},  // npc (Bertoldo), base em Y=50
        {20.0f, /*id=*/200},  // jogador vindo do NORTE, Y=20 (acima do NPC)
    };
    sort_by_depth(entries, 2);
    REQUIRE(entries[0].id == 200);  // jogador atras
    REQUIRE(entries[1].id == 100);  // npc na frente

    // Inverte: jogador vindo do SUL (Y maior que o NPC) -> jogador na frente.
    DepthEntry entries_south[2] = {
        {50.0f, /*id=*/100},  // npc
        {80.0f, /*id=*/200},  // jogador vindo do SUL, abaixo do NPC
    };
    sort_by_depth(entries_south, 2);
    REQUIRE(entries_south[0].id == 100);  // npc atras
    REQUIRE(entries_south[1].id == 200);  // jogador na frente
}

TEST_CASE("sort_by_depth: empate exato preserva a ordem de entrada (estavel)",
          "[core][spatial][depth_sort]") {
    DepthEntry entries[3] = {
        {30.0f, 0},
        {30.0f, 1},
        {30.0f, 2},
    };
    sort_by_depth(entries, 3);
    REQUIRE(entries[0].id == 0);
    REQUIRE(entries[1].id == 1);
    REQUIRE(entries[2].id == 2);
}

TEST_CASE("sort_by_depth: count<=1 e ponteiro nulo sao no-op seguro",
          "[core][spatial][depth_sort]") {
    sort_by_depth(nullptr, 0);
    sort_by_depth(nullptr, 5);  // count>0 com ponteiro nulo tambem nao deve crashar

    DepthEntry single[1] = {{7.0f, 42}};
    sort_by_depth(single, 1);
    REQUIRE(single[0].id == 42);

    DepthEntry empty[1] = {{7.0f, 42}};
    sort_by_depth(empty, 0);
    REQUIRE(empty[0].id == 42);  // count=0: nao mexeu em nada

    DepthEntry negative[1] = {{7.0f, 42}};
    sort_by_depth(negative, -1);  // count negativo: no-op seguro
    REQUIRE(negative[0].id == 42);
}
