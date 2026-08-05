// SPDX-License-Identifier: Apache-2.0
// GusEngine/tests/depth_sort_test.cpp
//
// Spec executavel (Catch2 v3) da ORDEM DE DESENHO POR OCLUSAO (DEMO-CIDADE-VESTIDA
// fatia I). TEST-FIRST: prova a ordenacao pura, SEM GL/janela - overworld_sim.cpp
// consome sort_by_occlusion pra decidir a SEQUENCIA de draw_* a cada quadro
// (jogador, NPCs, inimigos e pecas de cenario entram na MESMA lista).
//
// CONTRATO exercitado:
//   - sem cruzamento LATERAL dos desenhos nao ha restricao de ordem;
//   - separacao limpa em profundidade: quem esta mais ao norte desenha antes;
//   - CONTENCAO: quem tem a fatia de chao DENTRO da do outro desenha DEPOIS (fica
//     na frente) - e o caso relatado pelo lider, o personagem nivelado com a base
//     de uma peca cujo desenho e muito maior que a celula;
//   - empate exato preserva a ordem de entrada (estavel, sem tremer entre quadros);
//   - a saida e SEMPRE uma permutacao completa, inclusive com grafo CICLICO e com
//     coordenada nao-finita;
//   - quebra de ciclo DETERMINISTICA (mesma entrada, mesma saida);
//   - ponteiro nulo / count <= 1 sao no-op seguro.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

#include "gus/core/spatial/depth_sort.hpp"

using gus::core::spatial::DepthEntry;
using gus::core::spatial::DepthSortScratch;
using gus::core::spatial::occlusion_relation;
using gus::core::spatial::resolve_draw_order;
using gus::core::spatial::sort_by_occlusion;

namespace {

// Desenhavel "de personagem": a fatia de chao e a caixa dos pes, e o desenho e
// largo o bastante pra cruzar com os vizinhos do teste.
DepthEntry actor(float base_y, float depth, float left, float right, int id) {
    DepthEntry e;
    e.ground_back = base_y - depth;
    e.ground_front = base_y;
    e.draw_left = left;
    e.draw_right = right;
    e.id = id;
    return e;
}

// Posicao de `id` na sequencia de desenho (-1 se nao esta na lista).
int index_of(const DepthEntry* entries, int count, int id) {
    for (int i = 0; i < count; ++i) {
        if (entries[i].id == id) return i;
    }
    return -1;
}

}  // namespace

TEST_CASE("occlusion: desenhos que nao se cruzam lateralmente nao se restringem",
          "[core][spatial][depth_sort]") {
    // Um bem ao norte do outro, mas em colunas separadas: nenhum esconde nenhum,
    // logo NAO existe aresta. A ordem entre eles fica por conta da chave (Y), o que
    // e indiferente - mas a AUSENCIA de aresta e o que impede o grafo de encher de
    // restricao inventada (e, com ela, de inventar ciclo).
    const DepthEntry a = actor(10.0f, 2.0f, /*left=*/0.0f, /*right=*/5.0f, 1);
    const DepthEntry b = actor(90.0f, 2.0f, /*left=*/20.0f, /*right=*/25.0f, 2);
    REQUIRE(occlusion_relation(a, b) == 0);
    REQUIRE(occlusion_relation(b, a) == 0);

    // Encostar borda com borda tambem nao cruza: nao ha pixel em comum.
    const DepthEntry c = actor(90.0f, 2.0f, /*left=*/5.0f, /*right=*/9.0f, 3);
    REQUIRE(occlusion_relation(a, c) == 0);
}

TEST_CASE("occlusion: separacao limpa - quem esta ao norte desenha antes (atras)",
          "[core][spatial][depth_sort]") {
    const DepthEntry norte = actor(20.0f, 2.0f, 0.0f, 10.0f, 1);
    const DepthEntry sul = actor(50.0f, 2.0f, 0.0f, 10.0f, 2);
    REQUIRE(occlusion_relation(norte, sul) == -1);  // norte antes = atras
    REQUIRE(occlusion_relation(sul, norte) == 1);

    // A relacao e ANTISSIMETRICA em todos os pares deste teste.
    REQUIRE(occlusion_relation(norte, sul) == -occlusion_relation(sul, norte));
}

TEST_CASE("occlusion: fatia de chao CONTIDA na do outro desenha DEPOIS (na frente)",
          "[core][spatial][depth_sort]") {
    // A REGRA QUE CONSERTA O DEFEITO RELATADO. A peca ocupa uma fatia de chao
    // funda; o personagem esta nivelado com ela (os pes caem DENTRO dessa fatia).
    // Um corpo nao some para dentro de parede: ou esta atras dela, ou na frente.
    DepthEntry peca;
    peca.ground_back = 40.0f;
    peca.ground_front = 44.0f;
    peca.draw_left = 11.0f;
    peca.draw_right = 22.0f;
    peca.id = 1;

    DepthEntry gente;
    gente.ground_back = 42.4f;
    gente.ground_front = 43.6f;  // MENOR que o da peca, e ainda assim na frente
    gente.draw_left = 18.0f;
    gente.draw_right = 24.0f;
    gente.id = 2;

    REQUIRE(occlusion_relation(peca, gente) == -1);  // peca antes = atras
    REQUIRE(occlusion_relation(gente, peca) == 1);

    // Prova de que NAO e a chave escalar decidindo: o Y-sort de antes olharia
    // 44.0 > 43.6 e poria a peca por ultimo (por cima do personagem).
    REQUIRE(peca.ground_front > gente.ground_front);
}

TEST_CASE("occlusion: fatias IDENTICAS nao geram contencao (ninguem contem ninguem)",
          "[core][spatial][depth_sort]") {
    const DepthEntry a = actor(30.0f, 2.0f, 0.0f, 10.0f, 1);
    const DepthEntry b = actor(30.0f, 2.0f, 5.0f, 15.0f, 2);
    REQUIRE(occlusion_relation(a, b) == 0);  // empate exato: ordem de entrada decide
}

TEST_CASE("occlusion: fatias que se interpenetram sem conter decidem pela borda da frente",
          "[core][spatial][depth_sort]") {
    // [10,14] e [12,16]: cruzam, nenhuma cabe na outra. Decide quem tem a borda de
    // chao mais perto da camera.
    DepthEntry a;
    a.ground_back = 10.0f;
    a.ground_front = 14.0f;
    a.draw_left = 0.0f;
    a.draw_right = 10.0f;
    a.id = 1;
    DepthEntry b;
    b.ground_back = 12.0f;
    b.ground_front = 16.0f;
    b.draw_left = 0.0f;
    b.draw_right = 10.0f;
    b.id = 2;
    REQUIRE(occlusion_relation(a, b) == -1);
    REQUIRE(occlusion_relation(b, a) == 1);
}

TEST_CASE("sort_by_occlusion: caso real do overworld - jogador, NPC e inimigo",
          "[core][spatial][depth_sort]") {
    DepthSortScratch scratch;
    // Todos na mesma faixa lateral (se cobrem), fatias de chao separadas.
    DepthEntry entries[3] = {
        actor(40.0f, 1.0f, 0.0f, 10.0f, /*id=*/0),  // inimigo
        actor(42.0f, 1.0f, 0.0f, 10.0f, /*id=*/1),  // npc
        actor(5.0f, 1.0f, 0.0f, 10.0f, /*id=*/2),   // jogador, bem acima
    };
    sort_by_occlusion(entries, 3, scratch);
    REQUIRE(entries[0].id == 2);  // jogador desenha primeiro (fica atras)
    REQUIRE(entries[1].id == 0);
    REQUIRE(entries[2].id == 1);

    // Jogador vindo do SUL: passa a desenhar por ultimo (fica na frente).
    DepthEntry sul[3] = {
        actor(40.0f, 1.0f, 0.0f, 10.0f, 0),
        actor(42.0f, 1.0f, 0.0f, 10.0f, 1),
        actor(99.0f, 1.0f, 0.0f, 10.0f, 2),
    };
    sort_by_occlusion(sul, 3, scratch);
    REQUIRE(sul[0].id == 0);
    REQUIRE(sul[1].id == 1);
    REQUIRE(sul[2].id == 2);
}

TEST_CASE("sort_by_occlusion: empate exato preserva a ordem de entrada (estavel)",
          "[core][spatial][depth_sort]") {
    DepthSortScratch scratch;
    DepthEntry entries[3] = {
        actor(30.0f, 1.0f, 0.0f, 10.0f, 0),
        actor(30.0f, 1.0f, 0.0f, 10.0f, 1),
        actor(30.0f, 1.0f, 0.0f, 10.0f, 2),
    };
    sort_by_occlusion(entries, 3, scratch);
    REQUIRE(entries[0].id == 0);
    REQUIRE(entries[1].id == 1);
    REQUIRE(entries[2].id == 2);
}

TEST_CASE("sort_by_occlusion: peca larga esconde quem esta atras e NAO quem esta ao lado",
          "[core][spatial][depth_sort]") {
    // A cena que motivou a fatia, com os dois personagens de uma vez: a casa cobre
    // 5,5 celulas de largura. Quem esta ao NORTE da base dela some atras; quem esta
    // NIVELADO com a base aparece na frente. A chave escalar nao consegue dar as
    // duas respostas ao mesmo tempo, porque so tem um numero por desenhavel.
    DepthSortScratch scratch;
    DepthEntry casa;
    casa.ground_back = 40.34f;
    casa.ground_front = 44.0f;
    casa.draw_left = 11.51f;
    casa.draw_right = 22.49f;
    casa.id = 100;

    const DepthEntry atras = actor(39.6f, 1.2f, 18.0f, 24.0f, /*id=*/1);
    const DepthEntry ao_lado = actor(43.6f, 1.2f, 18.0f, 24.0f, /*id=*/2);

    DepthEntry entries[3] = {casa, atras, ao_lado};
    sort_by_occlusion(entries, 3, scratch);

    const int i_casa = index_of(entries, 3, 100);
    const int i_atras = index_of(entries, 3, 1);
    const int i_lado = index_of(entries, 3, 2);
    REQUIRE(i_atras < i_casa);  // ao norte da base: escondido pela casa
    REQUIRE(i_casa < i_lado);   // nivelado com a base: aparece na frente
}

TEST_CASE("sort_by_occlusion: ponteiro nulo e count <= 1 sao no-op seguro",
          "[core][spatial][depth_sort]") {
    DepthSortScratch scratch;
    sort_by_occlusion(nullptr, 0, scratch);
    sort_by_occlusion(nullptr, 5, scratch);  // count > 0 com ponteiro nulo

    DepthEntry single[1] = {actor(7.0f, 1.0f, 0.0f, 1.0f, 42)};
    sort_by_occlusion(single, 1, scratch);
    REQUIRE(single[0].id == 42);

    DepthEntry zero[1] = {actor(7.0f, 1.0f, 0.0f, 1.0f, 42)};
    sort_by_occlusion(zero, 0, scratch);
    REQUIRE(zero[0].id == 42);

    DepthEntry negativo[1] = {actor(7.0f, 1.0f, 0.0f, 1.0f, 42)};
    sort_by_occlusion(negativo, -1, scratch);
    REQUIRE(negativo[0].id == 42);
}

TEST_CASE("sort_by_occlusion: coordenada nao-finita ainda devolve permutacao completa",
          "[core][spatial][depth_sort]") {
    // Dado corrompido (NaN vindo de uma divisao ruim rio acima) nao pode travar o
    // render nem sumir com desenhavel: a saida continua tendo os mesmos N ids.
    DepthSortScratch scratch;
    const float nan_v = std::nan("");
    DepthEntry entries[3] = {
        actor(10.0f, 1.0f, 0.0f, 10.0f, 1),
        actor(nan_v, 1.0f, 0.0f, 10.0f, 2),
        actor(30.0f, 1.0f, nan_v, 10.0f, 3),
    };
    sort_by_occlusion(entries, 3, scratch);

    std::vector<int> ids{entries[0].id, entries[1].id, entries[2].id};
    std::sort(ids.begin(), ids.end());
    REQUIRE(ids == std::vector<int>{1, 2, 3});
}

// ===========================================================================
//  QUEBRA DE CICLO. Com a regra de aresta acima o grafo e ACICLICO por
//  construcao (ver o teste de busca logo abaixo), entao o unico jeito honesto de
//  exercitar a quebra e alimentar resolve_draw_order com um grafo ciclico MONTADO
//  A MAO - que e exatamente por que ela esta no contrato do header, e nao
//  escondida no anonimo. Caminho de excecao que nao da pra exercitar e caminho
//  que nao esta testado.
// ===========================================================================

TEST_CASE("resolve_draw_order: ciclo de 3 nos ainda devolve permutacao completa",
          "[core][spatial][depth_sort][ciclo]") {
    // 0 -> 1 -> 2 -> 0
    constexpr int n = 3;
    std::vector<unsigned char> behind(n * n, 0);
    behind[0 * n + 1] = 1;
    behind[1 * n + 2] = 1;
    behind[2 * n + 0] = 1;
    const float key[n] = {30.0f, 10.0f, 20.0f};
    int order[n] = {-1, -1, -1};

    resolve_draw_order(behind.data(), key, n, order);

    std::vector<int> saida(order, order + n);
    std::sort(saida.begin(), saida.end());
    REQUIRE(saida == std::vector<int>{0, 1, 2});
}

TEST_CASE("resolve_draw_order: a quebra de ciclo e DETERMINISTICA",
          "[core][spatial][depth_sort][ciclo]") {
    // Mesma entrada, mesma saida - sempre. Sem isto a cena TREME entre quadros: o
    // mesmo trio de desenhaveis sairia numa ordem a cada 16 ms.
    constexpr int n = 3;
    const float key[n] = {30.0f, 10.0f, 20.0f};
    std::vector<int> primeira;
    for (int rodada = 0; rodada < 5; ++rodada) {
        std::vector<unsigned char> behind(n * n, 0);
        behind[0 * n + 1] = 1;
        behind[1 * n + 2] = 1;
        behind[2 * n + 0] = 1;
        int order[n] = {-1, -1, -1};
        resolve_draw_order(behind.data(), key, n, order);
        const std::vector<int> saida(order, order + n);
        if (rodada == 0) {
            primeira = saida;
        } else {
            REQUIRE(saida == primeira);
        }
    }
    // E a quebra escolhe pela MENOR chave: o no 1 (key 10) sai primeiro.
    REQUIRE(primeira[0] == 1);
}

TEST_CASE("resolve_draw_order: quebra de ciclo respeita quem esta FORA do ciclo",
          "[core][spatial][depth_sort][ciclo]") {
    // 4 nos: 0 -> 1 -> 2 -> 0 (ciclo) e 3 preso atras do 0 (aresta 3 -> 0). O 3 tem
    // de sair ANTES do 0 mesmo com a quebra em cena: a quebra corta so as arestas
    // que entram no eleito, nunca as dos vizinhos sadios.
    constexpr int n = 4;
    std::vector<unsigned char> behind(n * n, 0);
    behind[0 * n + 1] = 1;
    behind[1 * n + 2] = 1;
    behind[2 * n + 0] = 1;
    behind[3 * n + 0] = 1;
    const float key[n] = {30.0f, 10.0f, 20.0f, 99.0f};
    int order[n] = {-1, -1, -1, -1};

    resolve_draw_order(behind.data(), key, n, order);

    std::vector<int> pos(n, -1);
    for (int i = 0; i < n; ++i) {
        pos[static_cast<std::size_t>(order[i])] = i;
    }
    REQUIRE(pos[3] < pos[0]);  // a aresta sadia foi respeitada
    std::vector<int> saida(order, order + n);
    std::sort(saida.begin(), saida.end());
    REQUIRE(saida == std::vector<int>{0, 1, 2, 3});
}

TEST_CASE("resolve_draw_order: ponteiro nulo e count <= 0 sao no-op seguro",
          "[core][spatial][depth_sort]") {
    const float key[1] = {0.0f};
    int order[1] = {7};
    unsigned char behind[1] = {0};
    resolve_draw_order(nullptr, key, 1, order);
    resolve_draw_order(behind, nullptr, 1, order);
    resolve_draw_order(behind, key, 1, nullptr);
    resolve_draw_order(behind, key, 0, order);
    resolve_draw_order(behind, key, -3, order);
    REQUIRE(order[0] == 7);  // nada foi tocado
}

namespace {

// Universo de fatias de chao de uma grade fina, TODAS se cruzando lateralmente -
// que e o pior caso para o grafo, porque o cruzamento lateral so REMOVE aresta.
std::vector<DepthEntry> grade_de_fatias() {
    std::vector<DepthEntry> universo;
    for (int back = 0; back <= 6; ++back) {
        for (int len = 0; len <= 6; ++len) {
            DepthEntry e;
            e.ground_back = static_cast<float>(back);
            e.ground_front = static_cast<float>(back + len);
            e.draw_left = 0.0f;
            e.draw_right = 100.0f;
            e.id = static_cast<int>(universo.size());
            universo.push_back(e);
        }
    }
    return universo;
}

}  // namespace

TEST_CASE("occlusion_relation: a relacao e ANTISSIMETRICA (busca exaustiva)",
          "[core][spatial][depth_sort][ciclo]") {
    // A INVARIANTE que sustenta tudo: se `a` vem antes de `b`, entao `b` vem depois
    // de `a`, ponto. Quebrar isto significa gravar as DUAS arestas opostas na
    // matriz - um ciclo de 2, que so aparece na tela como desenhavel piscando de
    // ordem. Foi exatamente o que aconteceu na primeira versao desta regra, com
    // fatias de comprimento ZERO na mesma altura (peca atravessavel), e foi a busca
    // exaustiva que pegou. Teste barato, e pega classe inteira de defeito.
    const std::vector<DepthEntry> u = grade_de_fatias();
    const int n = static_cast<int>(u.size());
    REQUIRE(n == 49);

    int violacoes = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            const int ij = occlusion_relation(u[static_cast<std::size_t>(i)],
                                              u[static_cast<std::size_t>(j)]);
            const int ji = occlusion_relation(u[static_cast<std::size_t>(j)],
                                              u[static_cast<std::size_t>(i)]);
            if (ij != -ji) {
                ++violacoes;
            }
        }
    }
    REQUIRE(violacoes == 0);
}

TEST_CASE("occlusion_relation: a regra de aresta NAO fecha ciclo (busca exaustiva)",
          "[core][spatial][depth_sort][ciclo]") {
    // Prova empirica da afirmacao do header: em todo trio de fatias de chao de uma
    // grade fina (todos se cruzando lateralmente, que e o pior caso - o cruzamento
    // so REMOVE aresta), nunca aparece 0->1->2->0. Se um dia a regra mudar e passar
    // a fechar volta, ESTE teste cai primeiro, e a quebra de ciclo acima deixa de
    // ser guarda e vira caminho normal.
    const std::vector<DepthEntry> universo = grade_de_fatias();
    const int n = static_cast<int>(universo.size());
    REQUIRE(n == 49);

    int ciclos = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (occlusion_relation(universo[static_cast<std::size_t>(i)],
                                   universo[static_cast<std::size_t>(j)]) >= 0) {
                continue;  // precisa de i -> j
            }
            for (int k = 0; k < n; ++k) {
                if (occlusion_relation(universo[static_cast<std::size_t>(j)],
                                       universo[static_cast<std::size_t>(k)]) >= 0) {
                    continue;  // precisa de j -> k
                }
                if (occlusion_relation(universo[static_cast<std::size_t>(k)],
                                       universo[static_cast<std::size_t>(i)]) < 0) {
                    ++ciclos;  // k -> i fecharia a volta
                }
            }
        }
    }
    REQUIRE(ciclos == 0);
}
