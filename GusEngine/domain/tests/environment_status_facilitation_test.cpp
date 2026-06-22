// environment_status_facilitation_test.cpp
//
// Spec executavel (Catch2 v3) do canal 2 (secao 18.1): ambientes FACILITAM um status do
// enum EXISTENTE (secao 9), nunca criam status novo. Portada de
// engine/tests/turn_combat/environments/EnvironmentStatusFacilitationTests.cs (xUnit =
// SPEC) e da fonte EnvironmentCatalog.cs / EnvironmentModifier.cs. POCO puro, ZERO Qt,
// headless. Marco M5 (chunk 3).
//
// O incremento (facilitated_status) soma magnitude/duracao ao StatusEffect da
// familia-casa, via environment_catalog (lookup) + environment_modifier (apply_to).
//
// Cross-ref: engine/tests/turn_combat/environments/EnvironmentStatusFacilitationTests.cs;
//            docs/design/mecanicas/combat.md secao 18.1 (canal 2)/18.2/9.

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/environment_catalog.hpp"
#include "gus/domain/combat/environment_enums.hpp"

using namespace gus::domain::combat;

namespace {
StatusEffect status(StatusId id, int mag, int dur) {
    StatusEffect s{};
    s.id = id;
    s.magnitude = mag;
    s.duration = dur;
    s.stack_rule = StackRule::StackMagnitude;
    s.family_origin = CardFamily::Sonico;
    return s;
}
}  // namespace

// ---- Neblina -> Disrupt +1 magnitude (secao 18.2) ---------------------------------

TEST_CASE("environment_status_facilitation: Neblina aumenta magnitude do Disrupt em 1",
          "[domain][combat][environment]") {
    const auto fs = EnvironmentCatalog::get(EnvironmentId::Neblina).facilitated_status.value();
    const auto base = status(StatusId::Disrupt, /*mag=*/20, /*dur=*/1);

    const auto facilitado = fs.apply_to(base);
    REQUIRE(facilitado.id == StatusId::Disrupt);
    REQUIRE(facilitado.magnitude == 21);  // +1 mag
    REQUIRE(facilitado.duration == 1);    // dur inalterada
}

// ---- Chuva -> Stun +1 duracao (secao 18.2) ----------------------------------------

TEST_CASE("environment_status_facilitation: Chuva aumenta duracao do Stun em 1",
          "[domain][combat][environment]") {
    const auto fs = EnvironmentCatalog::get(EnvironmentId::Chuva).facilitated_status.value();
    const auto base = status(StatusId::Stun, /*mag=*/0, /*dur=*/1);

    const auto facilitado = fs.apply_to(base);
    REQUIRE(facilitado.duration == 2);  // +1 dur
    REQUIRE(facilitado.magnitude == 0);
}

// ---- O ambiente so facilita o SEU status (nao toca outros) ------------------------

TEST_CASE("environment_status_facilitation: apply_to e no-op para status de id diferente",
          "[domain][combat][environment]") {
    const auto fs = EnvironmentCatalog::get(EnvironmentId::Neblina).facilitated_status.value();
    const auto poison = status(StatusId::Poison, /*mag=*/5, /*dur=*/3);

    const auto resultado = fs.apply_to(poison);
    REQUIRE(resultado == poison);  // inalterado
}

// ---- PavimentoTesselado -> Expose magnitude 13 (Fibonacci canon, secao 18.4) ------

TEST_CASE("environment_status_facilitation: PavimentoTesselado facilita Expose magnitude 13",
          "[domain][combat][environment]") {
    const auto fs = EnvironmentCatalog::get(EnvironmentId::PavimentoTesselado).facilitated_status.value();
    REQUIRE(fs.id == StatusId::Expose);

    const auto base = status(StatusId::Expose, /*mag=*/0, /*dur=*/1);
    const auto facilitado = fs.apply_to(base);
    REQUIRE(facilitado.magnitude == 13);  // 0 + 13 (Fibonacci)
}

// ---- Root = Slow extremo (secao 18.1): Vinhas facilita Slow, NAO status novo -------

TEST_CASE("environment_status_facilitation: Vinhas Root e Slow nao status novo",
          "[domain][combat][environment]") {
    const auto fs = EnvironmentCatalog::get(EnvironmentId::Vinhas).facilitated_status.value();
    REQUIRE(fs.id == StatusId::Slow);  // "Root" = Slow de magnitude extrema
}
