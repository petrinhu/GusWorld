// SPDX-License-Identifier: Apache-2.0
// patrol_route_test.cpp
//
// Spec executável (Catch2 v3) da RONDA EM ROTA FIXA
// (gus/domain/world/patrol_route.hpp, DEMO-CIDADE-VESTIDA fatia B3): o inimigo
// anda em rota fixa, ida e volta, SEM reagir ao jogador (decisão do líder, estilo
// Chrono Trigger). A rota é DADO (lista de pontos + velocidade + pausa), e o
// avanço é uma função pura e determinística - nada aqui sabe de sprite, de tela
// nem de quem está patrulhando.
//
// Unidade: os pontos são CÉLULAS do mapa e a velocidade é em TILES/s. Quem
// consome converte para unidades de mundo multiplicando pelo tile_size (o domínio
// não conhece a escala do mapa).

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/world/patrol_route.hpp"

using gus::domain::world::advance_patrol;
using gus::domain::world::kMaxPatrolWaypoints;
using gus::domain::world::PatrolRoute;
using gus::domain::world::PatrolSample;
using gus::domain::world::PatrolState;
using gus::domain::world::sample_patrol;

namespace {

constexpr bool near_eq(float a, float b, float eps = 0.0005f) noexcept {
    const float d = a - b;
    return (d < 0.0f ? -d : d) <= eps;
}

// Rota horizontal simples: (0,0) -> (4,0). Sem pausa, 1 tile/s.
PatrolRoute make_line_route() {
    PatrolRoute r;
    r.waypoints[0] = {0.0f, 0.0f};
    r.waypoints[1] = {4.0f, 0.0f};
    r.count = 2;
    r.speed_tiles_per_sec = 1.0f;
    r.pause_seconds = 0.0f;
    return r;
}

}  // namespace

TEST_CASE("patrol: rota default é inválida e não move nada",
          "[domain][world][patrol]") {
    PatrolRoute r;
    REQUIRE_FALSE(r.valid());
    PatrolState s;
    const PatrolSample out = advance_patrol(r, s, 1.0f);
    REQUIRE(near_eq(out.x, 0.0f));
    REQUIRE(near_eq(out.y, 0.0f));
}

TEST_CASE("patrol: rota de um ponto só deixa o ator parado nele",
          "[domain][world][patrol]") {
    PatrolRoute r;
    r.waypoints[0] = {7.0f, 3.0f};
    r.count = 1;
    r.speed_tiles_per_sec = 2.0f;
    REQUIRE_FALSE(r.valid());  // ronda exige pelo menos 2 pontos

    PatrolState s;
    const PatrolSample out = advance_patrol(r, s, 5.0f);
    REQUIRE(near_eq(out.x, 7.0f));
    REQUIRE(near_eq(out.y, 3.0f));
}

TEST_CASE("patrol: anda na velocidade pedida ao longo da perna",
          "[domain][world][patrol]") {
    const PatrolRoute r = make_line_route();
    REQUIRE(r.valid());
    PatrolState s;

    PatrolSample out = advance_patrol(r, s, 1.0f);  // 1 tile/s por 1 s
    REQUIRE(near_eq(out.x, 1.0f));
    REQUIRE(near_eq(out.y, 0.0f));

    out = advance_patrol(r, s, 1.5f);
    REQUIRE(near_eq(out.x, 2.5f));
}

TEST_CASE("patrol: chega no fim e volta - ida e volta sem reagir a ninguém",
          "[domain][world][patrol]") {
    // O comportamento canônico da decisão do líder: ping-pong. Nunca teleporta de
    // volta ao início (isso seria loop de ciclo, não ida e volta).
    const PatrolRoute r = make_line_route();
    PatrolState s;

    PatrolSample out = advance_patrol(r, s, 4.0f);  // exatamente o fim
    REQUIRE(near_eq(out.x, 4.0f));

    out = advance_patrol(r, s, 1.0f);  // já de volta
    REQUIRE(near_eq(out.x, 3.0f));

    out = advance_patrol(r, s, 3.0f);  // volta ao ponto de partida
    REQUIRE(near_eq(out.x, 0.0f));

    out = advance_patrol(r, s, 1.0f);  // e sai de novo para a ida
    REQUIRE(near_eq(out.x, 1.0f));
}

TEST_CASE("patrol: um passo grande atravessa a virada sem estourar a rota",
          "[domain][world][patrol]") {
    // Um dt enorme (lag spike, janela minimizada) não pode fazer o ator sair da
    // rota nem entrar em laço infinito: ele consome a distância pelas pernas.
    const PatrolRoute r = make_line_route();
    PatrolState s;

    const PatrolSample out = advance_patrol(r, s, 6.0f);  // 4 de ida + 2 de volta
    REQUIRE(near_eq(out.x, 2.0f));
    REQUIRE(near_eq(out.y, 0.0f));
}

TEST_CASE("patrol: passo gigante nunca sai do intervalo da rota",
          "[domain][world][patrol]") {
    const PatrolRoute r = make_line_route();
    PatrolState s;
    const PatrolSample out = advance_patrol(r, s, 10000.0f);
    REQUIRE(out.x >= -0.0005f);
    REQUIRE(out.x <= 4.0005f);
}

TEST_CASE("patrol: pontos repetidos não penduram o avanço",
          "[domain][world][patrol]") {
    // Perna de comprimento ZERO é erro de autoria plausível (dois pontos iguais
    // na tabela). O avanço tem que atravessá-la, não girar em falso.
    PatrolRoute r;
    r.waypoints[0] = {2.0f, 2.0f};
    r.waypoints[1] = {2.0f, 2.0f};
    r.waypoints[2] = {5.0f, 2.0f};
    r.count = 3;
    r.speed_tiles_per_sec = 1.0f;
    PatrolState s;

    const PatrolSample out = advance_patrol(r, s, 1.0f);
    REQUIRE(near_eq(out.y, 2.0f));
    REQUIRE(out.x >= 2.0f);
    REQUIRE(out.x <= 5.0f);
}

TEST_CASE("patrol: pausa segura o ator no ponto antes de virar",
          "[domain][world][patrol]") {
    PatrolRoute r = make_line_route();
    r.pause_seconds = 2.0f;
    PatrolState s;

    PatrolSample out = advance_patrol(r, s, 4.0f);  // chega na ponta
    REQUIRE(near_eq(out.x, 4.0f));
    REQUIRE(out.paused);

    out = advance_patrol(r, s, 1.0f);  // ainda pausado (faltava 1 s)
    REQUIRE(near_eq(out.x, 4.0f));
    REQUIRE(out.paused);

    out = advance_patrol(r, s, 1.5f);  // 1 s fecha a pausa e sobra 0.5 s de marcha
    REQUIRE(near_eq(out.x, 3.5f));
    REQUIRE_FALSE(out.paused);
}

TEST_CASE("patrol: velocidade zero ou negativa deixa o ator parado",
          "[domain][world][patrol]") {
    PatrolRoute r = make_line_route();
    r.speed_tiles_per_sec = 0.0f;
    REQUIRE_FALSE(r.valid());
    PatrolState s;
    PatrolSample out = advance_patrol(r, s, 3.0f);
    REQUIRE(near_eq(out.x, 0.0f));

    r.speed_tiles_per_sec = -4.0f;
    REQUIRE_FALSE(r.valid());
    PatrolState s2;
    out = advance_patrol(r, s2, 3.0f);
    REQUIRE(near_eq(out.x, 0.0f));
}

TEST_CASE("patrol: dt zero ou negativo não avança nem corrompe o estado",
          "[domain][world][patrol]") {
    const PatrolRoute r = make_line_route();
    PatrolState s;
    advance_patrol(r, s, 1.0f);
    const PatrolState antes = s;

    PatrolSample out = advance_patrol(r, s, 0.0f);
    REQUIRE(near_eq(out.x, 1.0f));
    out = advance_patrol(r, s, -5.0f);
    REQUIRE(near_eq(out.x, 1.0f));
    REQUIRE(s.from_index == antes.from_index);
    REQUIRE(s.to_index == antes.to_index);
    REQUIRE(near_eq(s.traveled_tiles, antes.traveled_tiles));
}

TEST_CASE("patrol: sample_patrol lê a posição sem mexer no estado",
          "[domain][world][patrol]") {
    const PatrolRoute r = make_line_route();
    PatrolState s;
    advance_patrol(r, s, 2.5f);
    const PatrolState antes = s;

    const PatrolSample lido = sample_patrol(r, s);
    REQUIRE(near_eq(lido.x, 2.5f));
    REQUIRE(s.from_index == antes.from_index);
    REQUIRE(near_eq(s.traveled_tiles, antes.traveled_tiles));
}

TEST_CASE("patrol: rota em L anda pelos dois eixos",
          "[domain][world][patrol]") {
    PatrolRoute r;
    r.waypoints[0] = {0.0f, 0.0f};
    r.waypoints[1] = {3.0f, 0.0f};
    r.waypoints[2] = {3.0f, 4.0f};
    r.count = 3;
    r.speed_tiles_per_sec = 1.0f;
    PatrolState s;

    PatrolSample out = advance_patrol(r, s, 3.0f);
    REQUIRE(near_eq(out.x, 3.0f));
    REQUIRE(near_eq(out.y, 0.0f));

    out = advance_patrol(r, s, 2.0f);
    REQUIRE(near_eq(out.x, 3.0f));
    REQUIRE(near_eq(out.y, 2.0f));
}

TEST_CASE("patrol: rota respeita o teto de pontos declarado",
          "[domain][world][patrol]") {
    // count acima do teto é dado corrompido: a rota vira inválida em vez de ler
    // além do array.
    PatrolRoute r;
    r.count = kMaxPatrolWaypoints + 1;
    r.speed_tiles_per_sec = 1.0f;
    REQUIRE_FALSE(r.valid());
    PatrolState s;
    const PatrolSample out = advance_patrol(r, s, 1.0f);
    REQUIRE(near_eq(out.x, 0.0f));
}

TEST_CASE("patrol: estado com índice corrompido não lê fora da rota",
          "[domain][world][patrol]") {
    const PatrolRoute r = make_line_route();
    PatrolState s;
    s.from_index = 99;
    s.to_index = -4;
    const PatrolSample out = advance_patrol(r, s, 1.0f);
    REQUIRE(out.x >= -0.0005f);
    REQUIRE(out.x <= 4.0005f);
}
