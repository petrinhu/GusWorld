// gus/domain/progression/xp_differential.cpp
//
// Implementacao da formula pura de XP differential por zona. Ver header para o
// contrato, a formula canonica (combat.md secao 11) e a decisao do clamp 1.0.

#include "gus/domain/progression/xp_differential.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace gus::domain::progression {

double xp_factor(int player_zone, int enemy_zone) {
    if (player_zone < 0) {
        throw std::invalid_argument("xp_factor: player_zone deve ser >= 0.");
    }
    if (enemy_zone < 0) {
        throw std::invalid_argument("xp_factor: enemy_zone deve ser >= 0.");
    }

    // gap positivo so quando player esta ACIMA do inimigo; abaixo => gap 0
    // (sem bonus reverso).
    const int gap = std::max(0, player_zone - enemy_zone);
    const double factor = 1.0 - gap * kPenaltyPerZone;
    return std::clamp(factor, 0.0, 1.0);
}

int xp_award(int base_xp, int player_zone, int enemy_zone) {
    if (base_xp < 0) {
        throw std::invalid_argument("xp_award: base_xp deve ser >= 0.");
    }

    const double factor = xp_factor(player_zone, enemy_zone);  // valida as zonas

    // round-half-away-from-zero deterministico: std::llround arredonda o caso
    // .5 para longe de zero (igual a MidpointRounding.AwayFromZero do C#).
    return static_cast<int>(std::llround(base_xp * factor));
}

}  // namespace gus::domain::progression
