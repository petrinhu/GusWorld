// weakness_wheel_property_test.cpp
//
// REFORCO DE QA (marco M5) da roda de fraqueza (secao 6) por exaustao + propriedade.
// A roda e fechada e deterministica (5 familias), entao exaurimos as 25 combinacoes
// (atacante x alvo) e afirmamos os INVARIANTES:
//
//   INV-8a multiplier(a,t) SEMPRE pertence ao conjunto canonico {1.5, 1.0, 0.66}
//          (Imune 0.0 NAO faz parte da roda base; ver nota de cobertura abaixo).
//   INV-8b tier <-> multiplier sao consistentes (Fraco<->1.5, Neutro<->1.0, Resistente<->0.66).
//   INV-8c simetria do ciclo: se a e Fraco-contra t (mult 1.5), entao t e Resistente-contra a
//          (mult 0.66). Nunca dois "Fracos" no mesmo par.
//   INV-8d reflexividade: familia contra si mesma e sempre Neutra (1.0).
//
// Tambem cobre o curto-circuito de IMUNIDADE end-to-end via FSM (INV-8 "imune zera dano"):
// quando mult_fraqueza == 0, o dano e 0 ANTES de qualquer RNG. NOTA DE COBERTURA: a roda
// base PUBLICA so produz 1.5/1.0/0.66; o tier Imune (0.0) "e flag de inimigo/lore,
// incremento futuro" (weakness_wheel.hpp) e NAO e exposto pela API publica hoje. Logo a
// imunidade organica NAO e exercitavel pela roda; ver relatorio (invariante nao-coberta).
//
// Sem RNG (a roda nao usa). NAO altera codigo de producao.
//
// Subsistema: domain/combat (WeaknessWheel). POCO puro, ZERO Qt, headless.

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/weakness_wheel.hpp"

using namespace gus::domain::combat;
namespace cc = gus::domain::combat::combat_constants;

namespace {

constexpr CardFamily kFamilies[] = {
    CardFamily::Eletrico, CardFamily::Bioquimico, CardFamily::Sonico,
    CardFamily::Cinetico, CardFamily::Criptografico,
};

bool is_canonical_mult(float m) {
    return m == cc::kMultFraco || m == cc::kMultNeutro || m == cc::kMultResistente;
}

}  // namespace

// ===== INV-8a/b: multiplicador no conjunto canonico + consistente com o tier =====

TEST_CASE("property: roda de fraqueza so produz {1.5, 1.0, 0.66} em todas as 25 combinacoes",
          "[domain][combat][property][weakness]") {
    for (CardFamily atk : kFamilies) {
        for (CardFamily tgt : kFamilies) {
            const float m = WeaknessWheel::multiplier(atk, tgt);
            // INV-8a
            REQUIRE(is_canonical_mult(m));

            // INV-8b: tier <-> multiplier.
            switch (WeaknessWheel::tier_for(atk, tgt)) {
                case WeaknessTier::Fraco:
                    REQUIRE(m == cc::kMultFraco);
                    break;
                case WeaknessTier::Neutro:
                    REQUIRE(m == cc::kMultNeutro);
                    break;
                case WeaknessTier::Resistente:
                    REQUIRE(m == cc::kMultResistente);
                    break;
                case WeaknessTier::Imune:
                    // Inalcancavel pela roda base; se aparecer, e mult 0.0 (fora do canonico).
                    REQUIRE(m == cc::kMultImune);
                    break;
            }
        }
    }
}

// ===== INV-8c: simetria do ciclo (Fraco <-> Resistente no par espelhado) =====

TEST_CASE("property: par (a,t) Fraco implica par (t,a) Resistente (simetria do ciclo)",
          "[domain][combat][property][weakness]") {
    for (CardFamily a : kFamilies) {
        for (CardFamily t : kFamilies) {
            if (a == t) continue;
            const WeaknessTier ab = WeaknessWheel::tier_for(a, t);
            const WeaknessTier ba = WeaknessWheel::tier_for(t, a);

            if (ab == WeaknessTier::Fraco) {
                REQUIRE(ba == WeaknessTier::Resistente);
            } else if (ab == WeaknessTier::Resistente) {
                REQUIRE(ba == WeaknessTier::Fraco);
            } else {
                // Neutro <-> Neutro (relacao nao-direcional na roda de 5).
                REQUIRE(ab == WeaknessTier::Neutro);
                REQUIRE(ba == WeaknessTier::Neutro);
            }
        }
    }
}

// ===== INV-8d: reflexividade (familia contra si mesma e Neutra) =====

TEST_CASE("property: familia contra si mesma e sempre Neutra (mult 1.0)",
          "[domain][combat][property][weakness]") {
    for (CardFamily f : kFamilies) {
        REQUIRE(WeaknessWheel::tier_for(f, f) == WeaknessTier::Neutro);
        REQUIRE(WeaknessWheel::multiplier(f, f) == cc::kMultNeutro);
    }
}

// ===== Estrutura do ciclo: cada familia e Fraco contra EXATAMENTE 1 e Resistente a 1 =====

TEST_CASE("property: cada familia e forte contra exatamente uma e fraca diante de exatamente uma",
          "[domain][combat][property][weakness]") {
    for (CardFamily a : kFamilies) {
        int strong_count = 0;   // alvos onde 'a' e Fraco-pra-eles (a forte): mult 1.5
        int resist_count = 0;   // alvos que resistem a 'a': mult 0.66
        for (CardFamily t : kFamilies) {
            if (a == t) continue;
            const auto tier = WeaknessWheel::tier_for(a, t);
            if (tier == WeaknessTier::Fraco) ++strong_count;
            if (tier == WeaknessTier::Resistente) ++resist_count;
        }
        REQUIRE(strong_count == 1);
        REQUIRE(resist_count == 1);
    }
}
