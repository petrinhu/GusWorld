// environment_cap_property_test.cpp
//
// REFORCO DE QA (marco M5) do CAP de mult_ambiente (secao 11/18) por FUZZ das combinacoes
// de camadas. A curadoria de transicoes (secao 18.6) impede ORGANICAMENTE 2 fontes x1.5 da
// MESMA familia; o CAP [0.44, 2.25] e a TRAVA DE SEGURANCA NUMERICA que deve segurar
// QUALQUER combinacao, inclusive as que a curadoria nunca montaria. Aqui ATACAMOS o cap:
// empilhamos subconjuntos aleatorios de TODO o catalogo (terreno+clima+periodo, ate dezenas
// de camadas repetidas) em todas as 5 familias e afirmamos o INVARIANTE:
//
//   INV-7  mult_ambiente(family, active) ESTA SEMPRE em [0.44, 2.25], para qualquer
//          combinacao/cardinalidade de ambientes do catalogo e qualquer familia.
//   INV-7b conjunto vazio (ou so None) => exatamente 1.0 (retrocompat: combate inalterado).
//   INV-7c o mesmo cap vale pela superficie da FSM (mult_ambiente_for via set_environment).
//
// property-based "a mao" (property_gen.hpp): LCG semeado por indice de caso. NAO altera
// codigo de producao. Um caso fora do cap aqui e BUG -> reportar ao lider com a seed.
//
// Subsistema: domain/combat (EnvironmentCatalog + CombatStateMachine). POCO puro, ZERO Qt.

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/environment_catalog.hpp"
#include "gus/domain/combat/environment_enums.hpp"
#include "gus/domain/combat/environment_modifier.hpp"
#include "property_gen.hpp"

using namespace gus::domain::combat;
namespace cc = gus::domain::combat::combat_constants;
using gus::domain::tests::Lcg;

namespace {

constexpr int kCasesPerProperty = 4000;

constexpr CardFamily kFamilies[] = {
    CardFamily::Eletrico, CardFamily::Bioquimico, CardFamily::Sonico,
    CardFamily::Cinetico, CardFamily::Criptografico,
};

// Todos os ids do catalogo (lidos do proprio catalogo: nada hardcoded).
std::vector<EnvironmentId> all_ids() {
    std::vector<EnvironmentId> ids;
    for (const auto& [id, env] : EnvironmentCatalog::all()) {
        (void)env;
        ids.push_back(id);
    }
    return ids;
}

bool within_cap(float m) {
    return m >= cc::kMultAmbienteCapMin && m <= cc::kMultAmbienteCapMax;
}

}  // namespace

// ===== INV-7: cap [0.44, 2.25] sob qualquer pilha de camadas =====

TEST_CASE("property: mult_ambiente fica em [0.44, 2.25] para qualquer combinacao do catalogo",
          "[domain][combat][property][environment]") {
    const std::vector<EnvironmentId> ids = all_ids();
    REQUIRE(ids.size() > 1);  // sanidade: catalogo nao-vazio

    for (int c = 0; c < kCasesPerProperty; ++c) {
        Lcg g(0xE1000000u + static_cast<unsigned>(c));

        // Pilha aleatoria: 0..24 camadas amostradas COM repeticao (stress de cap alem do
        // que a curadoria organica permitiria: varias x1.5 da mesma familia empilhadas).
        const int layer_count = g.in_range(0, 24);
        std::vector<EnvironmentModifier> active;
        active.reserve(static_cast<std::size_t>(layer_count));
        for (int i = 0; i < layer_count; ++i) {
            const auto id = ids[static_cast<std::size_t>(g.in_range(0, static_cast<int>(ids.size()) - 1))];
            active.push_back(EnvironmentCatalog::get(id));
        }

        for (CardFamily fam : kFamilies) {
            const float m = EnvironmentCatalog::mult_ambiente(fam, active);
            REQUIRE(within_cap(m));
        }
    }
}

// ===== INV-7b: vazio / so None => 1.0 exato =====

TEST_CASE("property: mult_ambiente sem camadas (ou so None) e exatamente 1.0",
          "[domain][combat][property][environment]") {
    for (CardFamily fam : kFamilies) {
        const std::vector<EnvironmentModifier> empty;
        REQUIRE(EnvironmentCatalog::mult_ambiente(fam, empty) == cc::kMultAmbienteDefault);

        const std::vector<EnvironmentModifier> only_none{EnvironmentCatalog::none(),
                                                         EnvironmentCatalog::none()};
        REQUIRE(EnvironmentCatalog::mult_ambiente(fam, only_none) == cc::kMultAmbienteDefault);
    }
}

// ===== INV-7c: mesmo cap pela superficie da FSM (set_environment / mult_ambiente_for) =====

TEST_CASE("property: mult_ambiente_for da FSM respeita o cap sob conjuntos aleatorios",
          "[domain][combat][property][environment]") {
    const std::vector<EnvironmentId> ids = all_ids();

    for (int c = 0; c < 800; ++c) {
        Lcg g(0xE2000000u + static_cast<unsigned>(c));

        CombatActor hero("gus", "gus", 50, 8, 2, 20, CardFamily::Eletrico, /*player=*/true);
        CombatActor foe("enemy", "enemy", 500, 6, 1, 10, CardFamily::Cinetico, /*player=*/false);
        CombatActionProvider pass = [](CombatActor&, const CombatState&) {
            return CombatAction::pass();
        };
        CombatStateMachine sm({&hero, &foe}, pass);

        // Conjunto de ambiente aleatorio (1..6 ids), aplicado pela superficie publica.
        const int n = g.in_range(1, 6);
        std::vector<EnvironmentId> set;
        set.reserve(static_cast<std::size_t>(n));
        for (int i = 0; i < n; ++i)
            set.push_back(ids[static_cast<std::size_t>(g.in_range(0, static_cast<int>(ids.size()) - 1))]);
        sm.set_environment(set);

        for (CardFamily fam : kFamilies) {
            const float m = sm.mult_ambiente_for(fam);
            REQUIRE(within_cap(m));
        }
    }
}
