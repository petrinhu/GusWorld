// GusEngine/tests/stamina_test.cpp
//
// Catch2 do POCO core::player::Stamina: a CARGA DO APARATO do jogador (drena
// correndo, regenera parado RAPIDO e andando DEVAGAR, clampa em [0,max], ofega
// abaixo do limiar em UNIDADE). Numeros CANONICOS (Fibonacci) em
// docs/design/mecanicas/stamina.md. TEST-FIRST. Matematica pura: ZERO SDL/IO,
// roda headless.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/core/player/stamina.hpp"

using Catch::Matchers::WithinAbs;
using gus::core::player::MoveState;
using gus::core::player::Stamina;
using gus::core::player::StaminaConfig;

TEST_CASE("Stamina comeca cheia no max", "[stamina]") {
    Stamina s(StaminaConfig{/*max=*/89.0f});
    REQUIRE_THAT(s.value(), WithinAbs(89.0f, 1e-4f));
    REQUIRE_THAT(s.percent(), WithinAbs(100.0f, 1e-4f));
    REQUIRE_FALSE(s.is_tired());
}

TEST_CASE("Defaults sao os numeros CANONICOS (Fibonacci)", "[stamina]") {
    // Sem override: max 89, drain 8, regen andando 5, regen parado 13, limiar 34.
    Stamina s;
    const StaminaConfig& c = s.config();
    REQUIRE_THAT(c.max, WithinAbs(89.0f, 1e-4f));
    REQUIRE_THAT(c.drain_per_sec, WithinAbs(8.0f, 1e-4f));
    REQUIRE_THAT(c.recover_walk_per_sec, WithinAbs(5.0f, 1e-4f));
    REQUIRE_THAT(c.recover_idle_per_sec, WithinAbs(13.0f, 1e-4f));
    REQUIRE_THAT(c.tired_value, WithinAbs(34.0f, 1e-4f));
}

TEST_CASE("Running (sprint) drena pela taxa * dt", "[stamina]") {
    Stamina s(StaminaConfig{100.0f, /*drain=*/20.0f, /*walk=*/5.0f, /*idle=*/10.0f});
    s.tick(MoveState::Running, 1.0f);  // -20
    REQUIRE_THAT(s.value(), WithinAbs(80.0f, 1e-4f));
    s.tick(MoveState::Running, 0.5f);  // -10
    REQUIRE_THAT(s.value(), WithinAbs(70.0f, 1e-4f));
}

TEST_CASE("Walking regenera DEVAGAR; Idle regenera RAPIDO", "[stamina]") {
    // Seam de 3 estados: parado recupera mais que andando (canon 13 vs 5).
    Stamina s(StaminaConfig{100.0f, /*drain=*/20.0f, /*walk=*/5.0f, /*idle=*/13.0f});
    s.tick(MoveState::Running, 2.0f);  // 100 -> 60
    REQUIRE_THAT(s.value(), WithinAbs(60.0f, 1e-4f));
    s.tick(MoveState::Walking, 1.0f);  // +5 -> 65
    REQUIRE_THAT(s.value(), WithinAbs(65.0f, 1e-4f));
    s.tick(MoveState::Idle, 1.0f);  // +13 -> 78
    REQUIRE_THAT(s.value(), WithinAbs(78.0f, 1e-4f));
    // Andar regenera ESTRITAMENTE menos que parar no mesmo dt.
    Stamina a(StaminaConfig{100.0f, 20.0f, 5.0f, 13.0f});
    Stamina b(StaminaConfig{100.0f, 20.0f, 5.0f, 13.0f});
    a.tick(MoveState::Running, 3.0f);  // dreno igual nos dois
    b.tick(MoveState::Running, 3.0f);
    a.tick(MoveState::Walking, 1.0f);
    b.tick(MoveState::Idle, 1.0f);
    REQUIRE(b.value() > a.value());
}

TEST_CASE("Stamina clampa: nunca passa do max nem fica negativa", "[stamina]") {
    Stamina s(StaminaConfig{89.0f, 1000.0f, 1000.0f, 1000.0f});
    s.tick(MoveState::Running, 10.0f);  // dreno enorme -> piso 0
    REQUIRE_THAT(s.value(), WithinAbs(0.0f, 1e-4f));
    s.tick(MoveState::Idle, 10.0f);  // recuperacao enorme -> teto 89
    REQUIRE_THAT(s.value(), WithinAbs(89.0f, 1e-4f));
}

TEST_CASE("is_tired dispara abaixo do limiar em UNIDADE (estrito)", "[stamina]") {
    // Limiar CANONICO 34 (unidade de Carga, NAO percentual). max 89, drain 8/s.
    Stamina s;  // defaults canon
    // Cai ate EXATAMENTE 34 -> ainda descansado (comparacao estrita <).
    // 89 - 34 = 55 a drenar; a 8/s sao 6.875 s.
    s.tick(MoveState::Running, (89.0f - 34.0f) / 8.0f);  // 89 -> 34
    REQUIRE_THAT(s.value(), WithinAbs(34.0f, 1e-3f));
    REQUIRE_FALSE(s.is_tired());
    // Mais um tiquinho -> abaixo de 34 -> ofegante.
    s.tick(MoveState::Running, 0.1f);  // -0.8 -> 33.2
    REQUIRE(s.is_tired());
}

TEST_CASE("REGRA DE OURO: parado/andando saem da ofegancia (nunca trava)",
          "[stamina]") {
    // Esgota a Carga ate 0 correndo; o explorador que SO anda regenera (5/s) e sai da
    // ofegancia sozinho - andar nunca trava. (Aqui exercitamos o POCO; a trava do
    // sprint mora no consumidor.)
    Stamina s;  // canon: max 89, drain 8, walk 5, idle 13, limiar 34
    s.tick(MoveState::Running, 20.0f);  // 89 -> 0 (clamp)
    REQUIRE_THAT(s.value(), WithinAbs(0.0f, 1e-4f));
    REQUIRE(s.is_tired());
    // SO ANDANDO: a 5/s, sair da ofegancia (>=34) leva ~6.8 s. Roda 8 s -> 40 > 34.
    s.tick(MoveState::Walking, 8.0f);  // +40
    REQUIRE_THAT(s.value(), WithinAbs(40.0f, 1e-4f));
    REQUIRE_FALSE(s.is_tired());
}

TEST_CASE("dt nao-positivo nao muda nada", "[stamina]") {
    Stamina s(StaminaConfig{89.0f, 20.0f, 20.0f, 20.0f});
    s.tick(MoveState::Running, 0.0f);
    s.tick(MoveState::Running, -1.0f);
    s.tick(MoveState::Walking, -2.0f);
    s.tick(MoveState::Idle, 0.0f);
    REQUIRE_THAT(s.value(), WithinAbs(89.0f, 1e-4f));
}

TEST_CASE("ctor saneia max<=0 e taxas negativas", "[stamina]") {
    Stamina s(StaminaConfig{/*max=*/-5.0f, /*drain=*/-20.0f, /*walk=*/-10.0f,
                            /*idle=*/-13.0f});
    REQUIRE(s.max() > 0.0f);  // max saneado pra > 0
    const float before = s.value();
    s.tick(MoveState::Running, 1.0f);  // taxa negativa saneada a 0 -> sem dreno
    s.tick(MoveState::Walking, 1.0f);  // idem regen
    s.tick(MoveState::Idle, 1.0f);     // idem regen
    REQUIRE_THAT(s.value(), WithinAbs(before, 1e-4f));
}

TEST_CASE("ctor clampa tired_value ao [0, max]", "[stamina]") {
    // Limiar maior que o max e clampado pro max (sempre comparavel a value).
    Stamina s(StaminaConfig{50.0f, 8.0f, 5.0f, 13.0f, /*tired_value=*/999.0f});
    REQUIRE_THAT(s.config().tired_value, WithinAbs(50.0f, 1e-4f));
}

TEST_CASE("refill volta ao cheio", "[stamina]") {
    Stamina s(StaminaConfig{89.0f, 50.0f, 5.0f, 13.0f});
    s.tick(MoveState::Running, 1.0f);
    REQUIRE(s.value() < 89.0f);
    s.refill();
    REQUIRE_THAT(s.value(), WithinAbs(89.0f, 1e-4f));
}

TEST_CASE("Sprint cheio esgota em ~11 s (verifica o feel canonico)", "[stamina]") {
    // 89 / 8 = 11.125 s ate zerar correndo continuo.
    Stamina s;  // canon
    s.tick(MoveState::Running, 11.0f);  // -88 -> 1
    REQUIRE(s.value() > 0.0f);
    s.tick(MoveState::Running, 0.2f);  // -1.6 -> piso 0
    REQUIRE_THAT(s.value(), WithinAbs(0.0f, 1e-4f));
}

TEST_CASE("Ofegancia so apos ~7 s de sprint continuo (drains curtos nao ofegam)",
          "[stamina]") {
    // 89 - 34 = 55; a 8/s -> 6.875 s pra cruzar o limiar. Antes disso, descansado.
    Stamina s;  // canon
    s.tick(MoveState::Running, 6.0f);  // 89 -> 41
    REQUIRE_FALSE(s.is_tired());
    s.tick(MoveState::Running, 1.0f);  // 41 -> 33 (< 34)
    REQUIRE(s.is_tired());
}
