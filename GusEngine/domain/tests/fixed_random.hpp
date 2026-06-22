// fixed_random.hpp (test helper)
//
// Duplo deterministico da porta IRandomSource (gus/domain/combat/random_source.hpp),
// portado de engine/tests/turn_combat/FixedRandom.cs. POCO puro, ZERO Qt, ZERO I/O.
//
// A formula de UseCard (secao 11) usa duas fontes de aleatoriedade da porta, NESTA ordem:
//   - next(100):     sorteio de canal -> roll<fumble=FALHA; roll<fumble+crit=CRIT; senao COMUM
//   - next_double(): variancia Knowledge -> rolled = base * (1 + (v*2*x - v)); SO no canal COMUM
//
// FixedRandom crava ambas:
//   - next_int fixa o sorteio de canal (roll = min(next_int, 99)):
//       roll = 0   => FALHA  (quando fumbleChance > 0; ex. kills baixos)
//       roll = fumbleChance (ex. 5 com kills=0) => CRIT (cai na faixa [fumble, fumble+crit))
//       roll = 99  => COMUM  (default; topo da faixa, fora de FALHA/CRIT no caso comum)
//   - next_double (so consumido no canal COMUM):
//       0.5 => contribuicao de variancia ZERO (rolled == base_damage).
//       1.0 => variancia maxima (+v).
//       0.0 => variancia minima (-v).
// Default = (0.5, 99): canal COMUM com variancia ZERO (dano == danoBase deterministico).
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
    explicit FixedRandom(double next_double = 0.5, int next_int = 99)
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
