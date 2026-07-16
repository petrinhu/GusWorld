// counting_random.hpp (test helper)
//
// Duplo deterministico da porta IRandomSource (gus/domain/combat/random_source.hpp) que,
// alem de cravar valores fixos (como FixedRandom), CONTA quantas vezes cada metodo foi
// chamado. Serve pros testes de DETERMINISMO/consumo de RNG do caminho Grupo/AoE do motor
// techMagic (invariante sagrado: N alvos consomem exatamente N sorteios de canal + N de
// variancia, nem mais nem menos - um consumo espurio numa carta especifica seria bug).
//
// Extraido de techmagic_newton_test.cpp (classe local CountingRandom, teste de determinismo
// N-alvos) pra ser COMPARTILHADO entre os arquivos de carta que exercitam o mesmo caminho
// Grupo (Newton, Maxwell e - onda em curso - Hayek/Mises). Evita duplicar a classe em cada
// arquivo. Mesmo diretorio/estilo de fixed_random.hpp.
//
// Comportamento cravado (identico ao antigo CountingRandom local do Newton):
//   - next_double() -> 0.5 fixo (variancia Knowledge ZERO; rolled == danoBase), conta em
//     next_double_calls.
//   - next(max) -> min(99, max-1) (roll do canal = 99 => COMUM, sem fumble/crit no caso
//     comum), conta em next_calls; max<=0 devolve 0 (mesmo clamp de FixedRandom.cs).
//
// Cross-ref: fixed_random.hpp (duplo fixo sem contagem); techmagic_newton_test.cpp
//            (determinismo N-alvos, teste-gemeo); docs/design/mecanicas/combat.md secao 11.

#ifndef GUS_DOMAIN_TESTS_COUNTING_RANDOM_HPP
#define GUS_DOMAIN_TESTS_COUNTING_RANDOM_HPP

#include <algorithm>

#include "gus/domain/combat/random_source.hpp"

namespace gus::domain::tests {

// Random de valor fixo que CONTA consumos (canal + variancia) por metodo.
class CountingRandom final : public gus::domain::combat::IRandomSource {
public:
    double next_double() override {
        ++next_double_calls;
        return 0.5;
    }
    int next(int max_value) override {
        ++next_calls;
        return max_value <= 0 ? 0 : std::min(99, max_value - 1);
    }

    int next_calls = 0;
    int next_double_calls = 0;
};

}  // namespace gus::domain::tests

#endif  // GUS_DOMAIN_TESTS_COUNTING_RANDOM_HPP
