// GusEngine/tests/fixed_timestep_test.cpp
//
// Spec executavel (Catch2 v3) do loop de tempo fixo (M1). TEST-FIRST: estes
// casos DEFINEM o contrato do acumulador de delta, criterio de SAIDA do M1.
//
// ALGORITMO (classico "Fix Your Timestep", Gaffer on Games): o update logico
// roda a passo FIXO (dt = 1/60 s por padrao). Cada frame, o tempo real decorrido
// (frame_dt) entra num acumulador; enquanto o acumulador comporta um passo fixo,
// consome-se um tick e subtrai-se dt do acumulador. O residuo vira alpha em
// [0,1) (= acumulador/dt) pra interpolacao do render (movimento suave). Clamp
// anti "spiral of death": no maximo max_ticks_per_frame ticks por frame; o
// excedente e DESCARTADO (o jogo desacelera, nao engasga em cascata).
//
// CONTRATO exercitado:
//   - advance(frame_dt) -> FrameSteps { int ticks; double alpha }, acumulando;
//   - dt fracionario acumula entre frames (3 frames de 0.01 com dt=1/60 -> 1 tick
//     quando passa de 1/60 ~= 0.01666);
//   - alpha = accumulator/fixed_dt sempre em [0,1) APOS consumir os ticks;
//   - clamp: frame_dt gigante nao gera ticks ilimitados (limita a max_ticks e
//     descarta o resto, zerando o acumulador para nao "dever" tempo).
//   - deterministico: tempo entra como parametro, ZERO relogio real.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/core/time/fixed_timestep.hpp"

using gus::core::time::FixedTimestep;
using gus::core::time::FrameSteps;
using Catch::Matchers::WithinAbs;

namespace {
constexpr double kEps = 1e-9;
constexpr double k60 = 1.0 / 60.0;  // ~0.0166667
}  // namespace

TEST_CASE("fixed_timestep: defaults canonicos 60 Hz", "[core][time]") {
    FixedTimestep ts;  // dt = 1/60, max_ticks = 5 (defaults)
    REQUIRE_THAT(ts.fixed_dt(), WithinAbs(k60, kEps));
    REQUIRE(ts.max_ticks_per_frame() == 5);
    REQUIRE_THAT(ts.accumulator(), WithinAbs(0.0, kEps));
    REQUIRE_THAT(ts.alpha(), WithinAbs(0.0, kEps));
}

TEST_CASE("fixed_timestep: um frame de exatamente dt gera exatamente 1 tick",
          "[core][time]") {
    FixedTimestep ts(k60, 5);
    FrameSteps s = ts.advance(k60);
    REQUIRE(s.ticks == 1);
    // Acumulador zera (consumiu o passo inteiro); alpha = 0.
    REQUIRE_THAT(ts.accumulator(), WithinAbs(0.0, kEps));
    REQUIRE_THAT(s.alpha, WithinAbs(0.0, kEps));
}

TEST_CASE("fixed_timestep: meio passo nao gera tick e guarda alpha",
          "[core][time]") {
    FixedTimestep ts(k60, 5);
    FrameSteps s = ts.advance(k60 * 0.5);
    REQUIRE(s.ticks == 0);
    // Acumulou metade -> alpha ~= 0.5, residuo no acumulador.
    REQUIRE_THAT(s.alpha, WithinAbs(0.5, 1e-6));
    REQUIRE_THAT(ts.accumulator(), WithinAbs(k60 * 0.5, 1e-9));
}

TEST_CASE("fixed_timestep: acumulo fracionario entre frames vira tick",
          "[core][time]") {
    // 3 frames de 0.01 s = 0.03 s. Com dt ~= 0.01667, isso e 1 passo cheio
    // (0.01667) + residuo 0.01333. Prova o acumulo fracionario (criterio de
    // SAIDA do board): nenhum frame isolado de 0.01 atinge dt, mas a soma sim.
    FixedTimestep ts(k60, 5);

    FrameSteps a = ts.advance(0.01);
    REQUIRE(a.ticks == 0);  // 0.01 < 0.01667

    FrameSteps b = ts.advance(0.01);
    REQUIRE(b.ticks == 1);  // 0.02 >= 0.01667 -> 1 tick, sobra 0.00333

    FrameSteps c = ts.advance(0.01);
    REQUIRE(c.ticks == 0);  // 0.01333 < 0.01667 ainda nao fecha outro

    // Acumulador final = 0.03 - 1*dt.
    REQUIRE_THAT(ts.accumulator(), WithinAbs(0.03 - k60, 1e-9));
    REQUIRE_THAT(ts.alpha(), WithinAbs((0.03 - k60) / k60, 1e-6));
}

TEST_CASE("fixed_timestep: dois passos e meio num frame gera 2 ticks",
          "[core][time]") {
    FixedTimestep ts(k60, 5);
    FrameSteps s = ts.advance(k60 * 2.5);
    REQUIRE(s.ticks == 2);
    REQUIRE_THAT(s.alpha, WithinAbs(0.5, 1e-6));  // residuo 0.5*dt
    REQUIRE_THAT(ts.accumulator(), WithinAbs(k60 * 0.5, 1e-9));
}

TEST_CASE("fixed_timestep: alpha sempre no intervalo 0 ate 1 apos consumir ticks",
          "[core][time]") {
    FixedTimestep ts(k60, 10);
    // Varre varios dt fracionarios e confirma o invariante do alpha.
    const double dts[] = {0.0, 0.001, 0.007, 0.0166, 0.0167, 0.05, 0.123};
    for (double dt : dts) {
        FrameSteps s = ts.advance(dt);
        REQUIRE(s.alpha >= 0.0);
        REQUIRE(s.alpha < 1.0);
        REQUIRE_THAT(ts.alpha(), WithinAbs(s.alpha, kEps));
    }
}

TEST_CASE("fixed_timestep: clamp anti spiral-of-death limita ticks e zera dover",
          "[core][time]") {
    // Frame gigante (1 segundo) com max_ticks = 5: sem clamp, seriam 60 ticks
    // (cascata). Com clamp, no maximo 5 ticks, e o acumulador residual NAO pode
    // exceder dt (o tempo excedente e descartado, senao o proximo frame herda a
    // divida e a cascata so adia). alpha continua em [0,1).
    FixedTimestep ts(k60, 5);
    FrameSteps s = ts.advance(1.0);
    REQUIRE(s.ticks == 5);
    REQUIRE(ts.accumulator() < ts.fixed_dt());  // sem divida acumulada
    REQUIRE(s.alpha >= 0.0);
    REQUIRE(s.alpha < 1.0);
}

TEST_CASE("fixed_timestep: apos clamp o proximo frame normal nao tem divida",
          "[core][time]") {
    // Depois de um pico que estoura o clamp, um frame normal deve voltar a 1
    // tick (e nao "pagar" a divida do pico). Prova que o clamp zera o excedente.
    FixedTimestep ts(k60, 3);
    (void)ts.advance(1.0);    // pico: 3 ticks, excedente descartado
    FrameSteps s = ts.advance(k60);
    REQUIRE(s.ticks == 1);    // frame normal -> 1 tick exato, sem heranca
    REQUIRE_THAT(ts.accumulator(), WithinAbs(0.0, 1e-9));
}

TEST_CASE("fixed_timestep: frame_dt negativo ou zero nao gera tick nem quebra",
          "[core][time]") {
    // Robustez: dt <= 0 (ex. relogio recuado/pausa) nao deve gerar ticks nem
    // baixar o acumulador abaixo de zero.
    FixedTimestep ts(k60, 5);
    (void)ts.advance(k60 * 0.5);    // acumula meio passo
    FrameSteps z = ts.advance(0.0);
    REQUIRE(z.ticks == 0);
    REQUIRE_THAT(ts.accumulator(), WithinAbs(k60 * 0.5, 1e-9));  // inalterado
    FrameSteps n = ts.advance(-0.01);
    REQUIRE(n.ticks == 0);
    REQUIRE(ts.accumulator() >= 0.0);                            // nunca negativo
    REQUIRE_THAT(ts.accumulator(), WithinAbs(k60 * 0.5, 1e-9));  // dt<0 ignorado
}

TEST_CASE("fixed_timestep: reset volta ao estado inicial", "[core][time]") {
    FixedTimestep ts(k60, 5);
    (void)ts.advance(k60 * 0.7);
    REQUIRE(ts.accumulator() > 0.0);
    ts.reset();
    REQUIRE_THAT(ts.accumulator(), WithinAbs(0.0, kEps));
    REQUIRE_THAT(ts.alpha(), WithinAbs(0.0, kEps));
}

TEST_CASE("fixed_timestep: construtor satura parametros invalidos",
          "[core][time]") {
    // dt <= 0 e max_ticks <= 0 sao invalidos; o construtor satura para defaults
    // coerentes (nao explode, nao deixa o objeto num estado quebrado).
    FixedTimestep bad(0.0, 0);
    REQUIRE(bad.fixed_dt() > 0.0);
    REQUIRE(bad.max_ticks_per_frame() >= 1);
    // E ainda funciona: um passo do dt saturado gera 1 tick.
    FrameSteps s = bad.advance(bad.fixed_dt());
    REQUIRE(s.ticks == 1);
}
