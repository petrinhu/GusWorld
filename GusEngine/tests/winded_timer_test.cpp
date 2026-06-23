// GusEngine/tests/winded_timer_test.cpp
//
// Catch2 do POCO core::player::WindedTimer: o TIMER DE FOLEGO (corpo) do Gus,
// SEPARADO da Carga do aparato (decisao do lider 2026-06-23). Acumula folego
// enquanto o Gus CORRE; ao PARAR de correr, se passou do limiar, fica ATIVO por
// um minimo (5 s) que ESCALA com quanto tempo correu (ate um teto, 8 s), e decai
// com o tempo parado ate zerar - INDEPENDENTE da Carga ja ter recarregado.
// TEST-FIRST. Matematica pura: ZERO SDL/IO, roda headless.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/core/player/winded_timer.hpp"

using Catch::Matchers::WithinAbs;
using gus::core::player::WindedConfig;
using gus::core::player::WindedTimer;

namespace {
// Config canonica do M1 (numeros do lider 2026-06-23): minimo 5 s, teto 8 s, e o
// teto e atingido apos run_for_max_winded = 8 s de corrida sustentada (escala linear).
WindedConfig canon() {
    return WindedConfig{/*min_winded_seconds=*/5.0f, /*max_winded_seconds=*/8.0f,
                        /*run_for_max_winded=*/8.0f,
                        /*run_threshold_seconds=*/2.0f};
}
}  // namespace

TEST_CASE("WindedTimer comeca inativo e sem folego acumulado", "[winded]") {
    WindedTimer w(canon());
    REQUIRE_FALSE(w.is_winded());
    REQUIRE_THAT(w.remaining(), WithinAbs(0.0f, 1e-4f));
    REQUIRE_THAT(w.run_accumulated(), WithinAbs(0.0f, 1e-4f));
}

TEST_CASE("Correr acumula folego (define quanto vai ofegar)", "[winded]") {
    WindedTimer w(canon());
    w.tick_running(3.0f);  // correu 3 s
    REQUIRE_THAT(w.run_accumulated(), WithinAbs(3.0f, 1e-4f));
    // Correndo ainda nao esta "ofegante" (so ao PARAR).
    REQUIRE_FALSE(w.is_winded());
}

TEST_CASE("Parar abaixo do limiar de corrida NAO ofega (corrida curta)",
          "[winded]") {
    // run_threshold 2 s: correr menos que isso e parar nao dispara ofegancia.
    WindedTimer w(canon());
    w.tick_running(1.5f);  // corrida curta (< 2 s)
    w.tick_stopped(0.001f);  // primeiro tick parado: avalia o gatilho
    REQUIRE_FALSE(w.is_winded());
    REQUIRE_THAT(w.remaining(), WithinAbs(0.0f, 1e-4f));
}

TEST_CASE("Parar apos correr o suficiente ofega pelo MINIMO (5 s)", "[winded]") {
    // Correu pouco acima do limiar (3 s): a escala da ~3/8 do range, mas o PISO e 5 s.
    WindedTimer w(canon());
    w.tick_running(3.0f);
    w.tick_stopped(0.0f);  // dispara o gatilho sem consumir tempo
    REQUIRE(w.is_winded());
    // 5 + (8-5) * (3/8) = 5 + 3*0.375 = 6.125; >= 5 (minimo respeitado).
    REQUIRE(w.remaining() >= 5.0f - 1e-4f);
}

TEST_CASE("Correr ate o teto de corrida ofega pelo TETO (8 s)", "[winded]") {
    WindedTimer w(canon());
    w.tick_running(8.0f);  // corrida == run_for_max_winded
    w.tick_stopped(0.0f);
    REQUIRE(w.is_winded());
    REQUIRE_THAT(w.remaining(), WithinAbs(8.0f, 1e-3f));
}

TEST_CASE("Correr ALEM do teto satura no teto (8 s), nao passa", "[winded]") {
    WindedTimer w(canon());
    w.tick_running(20.0f);  // muito mais que o teto de corrida
    w.tick_stopped(0.0f);
    REQUIRE_THAT(w.remaining(), WithinAbs(8.0f, 1e-3f));
}

TEST_CASE("A duracao do folego ESCALA com o tempo de corrida (mais corre, mais ofega)",
          "[winded]") {
    WindedTimer a(canon());
    WindedTimer b(canon());
    a.tick_running(3.0f);
    b.tick_running(6.0f);
    a.tick_stopped(0.0f);
    b.tick_stopped(0.0f);
    REQUIRE(b.remaining() > a.remaining());  // correu mais -> ofega mais
    // Ambos respeitam piso e teto.
    REQUIRE(a.remaining() >= 5.0f - 1e-4f);
    REQUIRE(b.remaining() <= 8.0f + 1e-4f);
}

TEST_CASE("Ativo, o folego DECAI com o tempo parado ate zerar", "[winded]") {
    WindedTimer w(canon());
    w.tick_running(8.0f);   // teto: 8 s de folego
    w.tick_stopped(0.0f);   // dispara
    REQUIRE_THAT(w.remaining(), WithinAbs(8.0f, 1e-3f));
    w.tick_stopped(3.0f);   // parado 3 s
    REQUIRE_THAT(w.remaining(), WithinAbs(5.0f, 1e-3f));
    REQUIRE(w.is_winded());
    w.tick_stopped(10.0f);  // parado o bastante pra zerar (clamp em 0)
    REQUIRE_THAT(w.remaining(), WithinAbs(0.0f, 1e-4f));
    REQUIRE_FALSE(w.is_winded());
}

TEST_CASE("Folego ativo INDEPENDE da Carga: continua ofegante mesmo recarregado",
          "[winded]") {
    // O ponto da decisao do lider: o folego do CORPO nao olha a Carga do aparato.
    // Aqui o WindedTimer fica ativo por >5 s; o consumidor (sim) ja sabe que a Carga
    // recarrega rapido - este POCO segue ofegante de qualquer jeito.
    WindedTimer w(canon());
    w.tick_running(8.0f);
    w.tick_stopped(0.0f);
    // Passados 2 s parado (a Carga ja teria subido muito a 13/s), ainda ofegante.
    w.tick_stopped(2.0f);
    REQUIRE(w.is_winded());
    REQUIRE(w.remaining() > 0.0f);
}

TEST_CASE("Voltar a correr zera o decaimento e re-acumula corrida", "[winded]") {
    WindedTimer w(canon());
    w.tick_running(3.0f);
    w.tick_stopped(0.0f);     // ofega ~6.125 s
    const float r0 = w.remaining();
    REQUIRE(r0 > 0.0f);
    w.tick_running(2.0f);     // voltou a correr: limpa o folego ativo e acumula de novo
    REQUIRE_FALSE(w.is_winded());          // correndo nao esta ofegante
    REQUIRE_THAT(w.remaining(), WithinAbs(0.0f, 1e-4f));
    REQUIRE_THAT(w.run_accumulated(), WithinAbs(2.0f, 1e-4f));
}

TEST_CASE("dt nao-positivo nao muda nada", "[winded]") {
    WindedTimer w(canon());
    w.tick_running(5.0f);
    w.tick_stopped(0.0f);
    const float before = w.remaining();
    w.tick_stopped(0.0f);
    w.tick_stopped(-1.0f);
    w.tick_running(-2.0f);
    REQUIRE_THAT(w.remaining(), WithinAbs(before, 1e-4f));
}

TEST_CASE("ctor saneia numeros degenerados (teto < piso, tempos negativos)",
          "[winded]") {
    // teto menor que o piso -> teto vira o piso; tempos negativos -> minimos seguros.
    WindedTimer w(WindedConfig{/*min=*/5.0f, /*max=*/2.0f, /*run_for_max=*/-1.0f,
                               /*run_threshold=*/-3.0f});
    w.tick_running(100.0f);
    w.tick_stopped(0.0f);
    REQUIRE(w.is_winded());
    REQUIRE(w.remaining() >= 5.0f - 1e-4f);  // piso preservado
}
