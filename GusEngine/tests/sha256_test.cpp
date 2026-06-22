// sha256_test.cpp
//
// Spec executavel (Catch2 v3) do SHA-256 PROPRIO de core/ (ADR-006). TEST-FIRST
// contra os VETORES OFICIAIS do FIPS 180-4 / NIST: a prova de correcao da crypto
// propria. Sem dependencia externa, sem Qt (core/ e POCO puro).
//
// Vetores (FIPS 180-4 Appendix B + NIST CAVS / "SHA-256 Examples"):
//   - "abc"
//   - "" (string vazia)
//   - msg de 448 bits "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
//   - "abcdefgh...stu" de 896 bits (two-block ext) opcional
//   - 1.000.000 de 'a' (long message, ate verde)
//
// Cross-ref: docs/tech/adr/ADR-006-crypto-hmac-formato-domain.md.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>
#include <vector>

#include "gus/core/crypto/sha256.hpp"

using gus::core::crypto::sha256;
using gus::core::crypto::sha256_hex;

namespace {

// Converte string ASCII em bytes (sem o terminador nulo).
std::vector<std::uint8_t> bytes_of(const std::string& s) {
    return std::vector<std::uint8_t>(s.begin(), s.end());
}

}  // namespace

// ---- Vetores FIPS 180-4 / NIST --------------------------------------------

TEST_CASE("sha256: vetor 'abc' (FIPS 180-4 B.1)", "[core][crypto][sha256]") {
    REQUIRE(sha256_hex(bytes_of("abc")) ==
            "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

TEST_CASE("sha256: string vazia (NIST)", "[core][crypto][sha256]") {
    REQUIRE(sha256_hex(bytes_of("")) ==
            "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST_CASE("sha256: vetor de 448 bits (FIPS 180-4 B.2)",
          "[core][crypto][sha256]") {
    const std::string msg =
        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    REQUIRE(sha256_hex(bytes_of(msg)) ==
            "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
}

TEST_CASE("sha256: vetor de 896 bits / dois blocos (NIST)",
          "[core][crypto][sha256]") {
    const std::string msg =
        "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
        "hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";
    REQUIRE(sha256_hex(bytes_of(msg)) ==
            "cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1");
}

TEST_CASE("sha256: um milhao de 'a' (FIPS 180-4 long message)",
          "[core][crypto][sha256]") {
    const std::vector<std::uint8_t> msg(1'000'000, static_cast<std::uint8_t>('a'));
    REQUIRE(sha256_hex(msg) ==
            "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
}

// ---- Propriedades basicas --------------------------------------------------

TEST_CASE("sha256: digest tem 32 bytes", "[core][crypto][sha256]") {
    const auto digest = sha256(bytes_of("qualquer"));
    REQUIRE(digest.size() == 32u);
}

TEST_CASE("sha256: deterministico para a mesma entrada",
          "[core][crypto][sha256]") {
    REQUIRE(sha256(bytes_of("repetivel")) == sha256(bytes_of("repetivel")));
}

TEST_CASE("sha256: entradas diferentes geram digests diferentes",
          "[core][crypto][sha256]") {
    REQUIRE(sha256(bytes_of("A")) != sha256(bytes_of("B")));
}

TEST_CASE("sha256: hex tem 64 chars lowercase", "[core][crypto][sha256]") {
    const auto hex = sha256_hex(bytes_of("hex"));
    REQUIRE(hex.size() == 64u);
    for (char c : hex) {
        REQUIRE(((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')));
    }
}
