// property_gen.hpp (test helper)
//
// Gerador deterministico minimo para os testes de PROPRIEDADE/FUZZ do motor de combate
// (marco M5). NAO ha rapidcheck/FsCheck linkado no projeto (so Catch2); fazemos
// property-based "a mao": um LCG (Linear Congruential Generator) semeado por uma seed
// fixa por caso (em geral derivada do indice do caso), exercitado em loop sobre N inputs.
//
// Determinismo TOTAL: nenhuma fonte de entropia real (sem relogio, sem std::random_device).
// O mesmo binario produz exatamente a mesma sequencia em qualquer plataforma/run, requisito
// do dominio POCO puro e da reproducao de qualquer achado.
//
// O LCG segue os parametros de Numerical Recipes (mesma familia do glibc/MMIX), suficiente
// para varrer o espaco de inputs num fuzz dirigido (NAO e PRNG criptografico, nao precisa).
//
// Tambem expoe um IRandomSource (PropertyRandom) cujas saidas next()/next_double() sao
// derivadas do LCG: usado quando a PROPRIEDADE precisa exercitar TODOS os canais de RNG
// do combate (FALHA/CRIT/COMUM) sob muitas sementes, sem cravar um canal especifico como
// o FixedRandom faz.
//
// ZERO Qt, ZERO I/O. Header-only.

#ifndef GUS_DOMAIN_TESTS_PROPERTY_GEN_HPP
#define GUS_DOMAIN_TESTS_PROPERTY_GEN_HPP

#include <cstdint>

#include "gus/domain/combat/random_source.hpp"

namespace gus::domain::tests {

// LCG deterministico (Numerical Recipes: a=1664525, c=1013904223, mod 2^32).
class Lcg {
public:
    explicit Lcg(std::uint32_t seed) : state_(seed) {}

    // Proximo u32 cru.
    std::uint32_t next_u32() {
        state_ = state_ * 1664525u + 1013904223u;
        return state_;
    }

    // Inteiro em [lo, hi] (inclusivo). Se hi <= lo, devolve lo.
    int in_range(int lo, int hi) {
        if (hi <= lo) return lo;
        const std::uint32_t span = static_cast<std::uint32_t>(hi - lo) + 1u;
        return lo + static_cast<int>(next_u32() % span);
    }

    // bool com ~50% (bit alto, evita viés do bit baixo do LCG).
    bool coin() { return (next_u32() >> 31) != 0u; }

    // double em [0,1).
    double unit() {
        return static_cast<double>(next_u32()) / 4294967296.0;  // / 2^32
    }

private:
    std::uint32_t state_;
};

// IRandomSource alimentado por um LCG: cada chamada avanca o gerador. Usado quando a
// propriedade precisa varrer os canais de RNG do combate (FALHA/CRIT/COMUM) sob muitas
// sementes, em vez de cravar um canal (FixedRandom). Determinismo total por seed.
class PropertyRandom final : public gus::domain::combat::IRandomSource {
public:
    explicit PropertyRandom(std::uint32_t seed) : lcg_(seed) {}

    double next_double() override { return lcg_.unit(); }

    int next(int max_value) override {
        if (max_value <= 0) return 0;
        return static_cast<int>(lcg_.next_u32() % static_cast<std::uint32_t>(max_value));
    }

private:
    Lcg lcg_;
};

}  // namespace gus::domain::tests

#endif  // GUS_DOMAIN_TESTS_PROPERTY_GEN_HPP
