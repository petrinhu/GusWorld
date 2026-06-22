// fixed_random.hpp (test helper)
//
// Duplo deterministico da porta IRandomSource (gus/domain/combat/random_source.hpp),
// portado de engine/tests/turn_combat/FixedRandom.cs. POCO puro, ZERO Qt, ZERO I/O.
//
// A formula de UseCard (secao 11) usa duas fontes de aleatoriedade da porta:
//   - next_double(): variancia Knowledge Decay -> rolled = base * (1 + (v*2*x - v))
//   - next(100):     chance de critico         -> is_crit = next(100) < crit_chance
//
// FixedRandom crava ambas:
//   - next_double == 0.5 => contribuicao de variancia ZERO (rolled == base_damage).
//   - next_double == 1.0 => variancia maxima (+v).
//   - next_double == 0.0 => variancia minima (-v).
//   - next_int fixa o roll de crit (0 => sempre crita se crit_chance>0; 99 => quase nunca).
//
// Determinismo independente de plataforma (NAO dependemos de PRNG interno). Espelha
// FixedRandom.cs 1:1: Next(maxValue) = maxValue<=0 ? 0 : min(next_int, maxValue-1).
//
// Cross-ref: engine/tests/turn_combat/FixedRandom.cs; docs/design/mecanicas/combat.md secao 11.

#ifndef GUS_DOMAIN_TESTS_FIXED_RANDOM_HPP
#define GUS_DOMAIN_TESTS_FIXED_RANDOM_HPP

#include <algorithm>

#include "gus/domain/combat/random_source.hpp"

namespace gus::domain::tests {

// Random de valor fixo pra testes deterministicos da formula de dano (secao 11).
class FixedRandom final : public gus::domain::combat::IRandomSource {
public:
    explicit FixedRandom(double next_double = 0.5, int next_int = 0)
        : next_double_(next_double), next_int_(next_int) {}

    // Espelha Random.NextDouble() do C#: valor em [0,1) (aqui fixo).
    double next_double() override { return next_double_; }

    // Espelha Random.Next(maxValue): inteiro em [0, max_value). Clamp identico ao C#.
    int next(int max_value) override {
        return max_value <= 0 ? 0 : std::min(next_int_, max_value - 1);
    }

private:
    double next_double_;
    int next_int_;
};

}  // namespace gus::domain::tests

#endif  // GUS_DOMAIN_TESTS_FIXED_RANDOM_HPP
