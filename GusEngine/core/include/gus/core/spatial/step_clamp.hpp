// gus/core/spatial/step_clamp.hpp
//
// Teto anti-tunneling POR EIXO (TUNNELING-CLAMP-GUARD) - POCO C++ puro, header-
// only, ZERO Qt, ZERO I/O. Matematica pura: um delta cru vira um delta que NUNCA
// excede uma fracao do tile, por eixo. SIMETRICO por construcao: trata |delta|,
// preservando o SINAL - "andar de costas" (velocidade negativa) fica coberto
// pela MESMA formula, sem caso especial (ver clamp_step_axis abaixo).
//
// CONTEXTO (decisao do lider/CTO, NAO reabrir - CLAMP, nao swept/CCD): o
// algoritmo de colisao de grid_collision.hpp/.cpp resolve so a posicao ALVO por
// eixo (target-only, SEM swept/continuous collision detection). Um delta de
// movimento grande o bastante (walk/run com tuning absurdo, lag spike acima do
// que o FixedTimestep ja descarta, corrupcao de estado) pode fazer a caixa
// pular POR CIMA de uma parede fina de 1 celula sem NUNCA sobrepo-la
// (tunneling): a colisao nunca detecta hit porque a caixa nunca ocupa uma
// posicao intermediaria onde a sobreposicao aconteceria. Este modulo e a
// FRONTEIRA que impede o delta cru de chegar grande demais na resolucao: um
// GUARDA, nao uma reescrita do algoritmo de colisao (que continua target-only,
// intacto).
//
// PORQUE 0.95 (limiar UNIVERSAL, cego ao tamanho da caixa que vai chamar
// resolve_move/resolve_move_with_corner_assist):
//   - collision_sweep_invariants_test.cpp (tier ESTRITO) prova matematicamente
//     que o envelope <= 0.95*tile e livre de tunneling para o PIOR CASO (caixa
//     w/h -> 0, incluindo obstaculos pontuais estreitos - ver ObstacleSpan em
//     grid_collision.hpp): quanto MENOR a caixa, menor o corredor que ela
//     "varre" ao se mover, e mais facil pular uma parede fina inteira.
//   - Para a caixa REAL do jogador (kPlayerHitboxTileFraction = 0.6*tile, ver
//     app/screens/city_scene.hpp) a matematica permite um envelope AINDA MAIOR
//     (1.6*tile = 1.0*tile de parede + 0.6*tile da propria largura da caixa,
//     ver corner_assist_sweep_test.cpp) - mas este clamp fica no ponto de
//     FRONTEIRA de grid_collision.cpp, que so tem acesso a grid.tile_size(),
//     NUNCA a w/h da AABB especifica que vai colidir (a mesma funcao serve o
//     jogador, NPCs, obstaculos pontuais, etc.). Por isso usa-se o limiar mais
//     RESTRITIVO e universalmente seguro (0.95), nao o mais permissivo
//     calculado so para a hitbox do jogador.
//   - kMaxStepPerAxisTileFraction < 1.0 de proposito: mesmo o caso limite (um
//     obstaculo/parede de exatamente 1 tile de espessura) fica coberto com
//     folga - um delta de EXATAMENTE 1.0*tile ja arriscaria "encostar exato"
//     na borda seguinte sem span de deteccao.
//
// TELEPORTE LEGITIMO (ex.: carregar um save, trocar de mapa, cutscene) usa
// OverworldSim::set_player_position, que NAO passa por resolve_move nem por
// resolve_move_with_corner_assist - portanto NUNCA passa por este clamp. Este
// teto SO se aplica a um DELTA DE MOVIMENTO por passo de simulacao, nunca a
// uma reposicao intencional do jogo.
//
// VALIDACAO DE FLOAT (licao da auditoria-dominó - nunca assumir input finito):
//   - delta NaN -> devolve 0.0f (nao move; um deslocamento indefinido nao pode
//     virar um numero arbitrario).
//   - delta +-infinito -> clampa em +-teto (o clamp aritmetico normal ja
//     resolve isso: infinito e sempre > teto/< -teto, nenhum caso especial
//     necessario).
//   - tile_size <= 0 OU nao-finito (NaN/infinito) -> devolve 0.0f (sem grade
//     valida, nao ha teto que faca sentido calcular; nao mover e o lado seguro).

#ifndef GUS_CORE_SPATIAL_STEP_CLAMP_HPP
#define GUS_CORE_SPATIAL_STEP_CLAMP_HPP

#include <limits>

namespace gus::core::spatial {

// Fracao MAXIMA (do tile) que um delta de movimento pode avancar POR EIXO
// antes da resolucao de colisao (ver rationale completo no topo do arquivo).
inline constexpr float kMaxStepPerAxisTileFraction = 0.95f;

// Clampa `delta` (unidades de mundo) em +-(kMaxStepPerAxisTileFraction *
// tile_size). Devolve `delta` inalterado se |delta| <= teto (inclusive, sem
// alterar bit a bit deltas legitimos, em QUALQUER sinal). Pura, deterministica,
// sem alocacao.
[[nodiscard]] constexpr float clamp_step_axis(float delta, float tile_size) noexcept {
    // Grade invalida: "tile_size > 0.0f" ja e false pra tile_size <= 0 E pra
    // NaN (qualquer comparacao com NaN e false); tile_size == +infinito precisa
    // de checagem EXPLICITA (e > 0.0f, mas nao produz um teto finito). Sem uma
    // grade coerente nao ha teto que faca sentido: nao move.
    if (!(tile_size > 0.0f) || tile_size == std::numeric_limits<float>::infinity()) {
        return 0.0f;
    }

    // delta NaN: NaN != NaN e sempre true (truque IEEE754 padrao, sem <cmath>,
    // constexpr em qualquer compilador). Um deslocamento indefinido nao move.
    if (delta != delta) {
        return 0.0f;
    }

    const float cap = kMaxStepPerAxisTileFraction * tile_size;
    if (delta > cap) {
        return cap;  // cobre tambem delta == +infinito (sempre > cap finito).
    }
    if (delta < -cap) {
        return -cap;  // cobre tambem delta == -infinito (mesma logica, sinal
                       // negativo - "andar de costas" nao e caso especial).
    }
    return delta;
}

}  // namespace gus::core::spatial

#endif  // GUS_CORE_SPATIAL_STEP_CLAMP_HPP
