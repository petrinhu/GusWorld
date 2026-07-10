// secure_zero_test.cpp
//
// Spec executavel (Catch2 v3) da zeragem segura anti-otimizador (ADR-015 secao 5,
// item 4 do wipe do Hardcore). Wrapper proprio sobre crypto_wipe do Monocypher
// vendorizado.
//
// Cross-ref: docs/tech/adr/ADR-015-save-security-v2-offline.md,
//            gus/core/crypto/secure_zero.hpp.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <vector>

#include "gus/core/crypto/secure_zero.hpp"

using gus::core::crypto::secure_zero;

TEST_CASE("secure_zero: zera todos os bytes do buffer", "[core][crypto][secure_zero]") {
    std::vector<std::uint8_t> buf(64, 0xAB);
    secure_zero(buf.data(), buf.size());
    for (const auto b : buf) REQUIRE(b == 0);
}

TEST_CASE("secure_zero: buffer grande (chave derivada + payload)",
          "[core][crypto][secure_zero]") {
    std::vector<std::uint8_t> buf(4096, 0xFF);
    secure_zero(buf.data(), buf.size());
    REQUIRE(std::all_of(buf.begin(), buf.end(), [](std::uint8_t b) { return b == 0; }));
}

TEST_CASE("secure_zero: size=0 e um no-op seguro (nao acessa nada)",
          "[core][crypto][secure_zero]") {
    std::uint8_t sentinel = 0x42;
    secure_zero(&sentinel, 0);
    REQUIRE(sentinel == 0x42);
}
