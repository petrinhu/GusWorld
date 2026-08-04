// SPDX-License-Identifier: Apache-2.0
// gus/domain/world/patrol_route.hpp
//
// RONDA EM ROTA FIXA (DEMO-CIDADE-VESTIDA fatia B3). POCO puro, ZERO SDL/GL/IO
// (invariante de domain/).
//
// DECISÃO DO LÍDER (2026-08-04): o inimigo anda em ROTA FIXA, IDA E VOLTA, SEM
// reagir ao jogador - estilo Chrono Trigger. Não é perseguição, não é aleatório.
// Consequência de projeto que vale registrar: como a ronda ignora o jogador, ela
// é uma função só do TEMPO e da rota, logo é pura, determinística e testável sem
// mundo nenhum. Perseguição exigiria conhecer a posição do jogador e traria a
// necessidade de busca de caminho; nada disso existe aqui de propósito.
//
// A ROTA É DADO (ADR-020 decisão 5): lista de pontos + velocidade + pausa. Ronda
// nova = outra linha de dado, nunca um comportamento novo em código.
//
// UNIDADES: os pontos são CÉLULAS do mapa (float, para permitir meio-tile) e a
// velocidade é em TILES/s. O domínio não conhece tile_size: quem consome converte
// para unidades de mundo. Isso mantém a rota legível para quem a escreve (o level
// designer pensa em células) e imune a mudança de escala do mapa.
//
// Cross-ref: domain/tests/patrol_route_test.cpp;
//            gus/app/screens/overworld_sim.hpp (quem avança a ronda no passo fixo).

#ifndef GUS_DOMAIN_WORLD_PATROL_ROUTE_HPP
#define GUS_DOMAIN_WORLD_PATROL_ROUTE_HPP

namespace gus::domain::world {

// Teto de pontos por rota. Array FIXO (sem heap): uma ronda de vigia de cidade não
// precisa de mais que isso, e o teto mantém o ator copiável e cache-friendly. Se um
// dia precisar de rota longa, o teto sobe aqui - é dado, não estrutura.
inline constexpr int kMaxPatrolWaypoints = 8;

// Um ponto da rota, em CÉLULAS do mapa.
struct PatrolWaypoint {
    float x = 0.0f;
    float y = 0.0f;
};

// A rota inteira. Default = rota inválida (nenhum ponto), que significa "ator
// parado" - o default seguro, igual ao que acontece hoje com NPC/inimigo fixo.
struct PatrolRoute {
    PatrolWaypoint waypoints[kMaxPatrolWaypoints]{};
    int count = 0;

    // Velocidade da ronda, em TILES/s. Referência de calibragem: o Gus caminha a
    // 4.5 tiles/s (OverworldTuning::walk_speed_tiles_per_sec) - a ronda deve ser
    // visivelmente mais lenta, senão o jogador não consegue contorná-la.
    float speed_tiles_per_sec = 0.0f;

    // Pausa (s) AO CHEGAR em cada ponto da rota, antes de seguir. Zero = ronda
    // contínua. A pausa é o que dá leitura de "ele está olhando ao redor" em vez
    // de "ele está deslizando num trilho".
    float pause_seconds = 0.0f;

    // Ronda só existe com pelo menos 2 pontos e velocidade positiva. Qualquer
    // outra coisa (inclusive count corrompido acima do teto) é ator PARADO.
    [[nodiscard]] constexpr bool valid() const noexcept {
        return count >= 2 && count <= kMaxPatrolWaypoints &&
               speed_tiles_per_sec > 0.0f;
    }
};

// Estado MUTÁVEL da ronda de um ator (a rota em si é constante). Vive junto do
// ator, não da rota: dois inimigos podem compartilhar a mesma rota em pontos
// diferentes dela.
struct PatrolState {
    int from_index = 0;             // ponto de onde saiu
    int to_index = 1;               // ponto para onde vai
    float traveled_tiles = 0.0f;    // já percorrido NESTA perna
    float pause_left = 0.0f;        // pausa restante (0 = andando)
    bool forward = true;            // true = indo (0 -> n-1); false = voltando
};

// Posição corrente da ronda, em CÉLULAS.
struct PatrolSample {
    float x = 0.0f;
    float y = 0.0f;
    bool paused = false;  // true = parado no ponto cumprindo a pausa
};

// Avança a ronda por `dt` segundos e devolve a posição resultante. Determinística.
//
// IDA E VOLTA (ping-pong): ao chegar no último ponto o sentido inverte e o ator
// refaz o caminho ponto a ponto - nunca teleporta de volta ao início (isso seria
// rota cíclica, que não é o que o líder pediu).
//
// ROBUSTEZ (todos travados por teste, porque dado de mundo é escrito à mão e erra):
//   - rota inválida / dt <= 0 -> nada muda;
//   - dt gigante (lag spike, janela minimizada) -> consome as pernas em laço com
//     teto de iterações, nunca sai do intervalo da rota nem pendura;
//   - pontos repetidos (perna de comprimento zero) -> atravessa em vez de girar;
//   - índices corrompidos no estado -> reancorados na rota.
PatrolSample advance_patrol(const PatrolRoute& route, PatrolState& state,
                            float dt) noexcept;

// Lê a posição corrente SEM avançar (render/inspeção/teste).
[[nodiscard]] PatrolSample sample_patrol(const PatrolRoute& route,
                                          const PatrolState& state) noexcept;

}  // namespace gus::domain::world

#endif  // GUS_DOMAIN_WORLD_PATROL_ROUTE_HPP
