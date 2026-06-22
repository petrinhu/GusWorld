// hmac_sha256_test.cpp
//
// Spec executavel (Catch2 v3) do HMAC-SHA256 PROPRIO de core/ (ADR-006).
// TEST-FIRST contra os VETORES OFICIAIS do RFC 4231 (test cases 1 a 7): a prova
// de correcao do MAC proprio. Sem dependencia externa, sem Qt.
//
// Chaves e dados em hex, resultados conhecidos (RFC 4231 secao 4). O HMAC-SHA256
// truncado (PRF-128 do TC4231 case 5) nao e exercitado: usamos o digest cheio.
//
// Cross-ref: docs/tech/adr/ADR-006-crypto-hmac-formato-domain.md, RFC 4231.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>
#include <vector>

#include "gus/core/crypto/hmac_sha256.hpp"

using gus::core::crypto::hmac_sha256;
using gus::core::crypto::hmac_sha256_hex;

namespace {

// Decodifica uma string hex em bytes. Aborta o teste em hex malformado.
std::vector<std::uint8_t> from_hex(const std::string& hex) {
    REQUIRE(hex.size() % 2 == 0);
    std::vector<std::uint8_t> out;
    out.reserve(hex.size() / 2);
    auto nib = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    for (std::size_t i = 0; i < hex.size(); i += 2) {
        const int hi = nib(hex[i]);
        const int lo = nib(hex[i + 1]);
        REQUIRE(hi >= 0);
        REQUIRE(lo >= 0);
        out.push_back(static_cast<std::uint8_t>((hi << 4) | lo));
    }
    return out;
}

std::vector<std::uint8_t> repeat(std::uint8_t byte, std::size_t n) {
    return std::vector<std::uint8_t>(n, byte);
}

}  // namespace

// ---- RFC 4231 test cases 1..7 ---------------------------------------------

TEST_CASE("hmac-sha256: RFC 4231 case 1", "[core][crypto][hmac]") {
    const auto key = repeat(0x0b, 20);
    const auto data = from_hex("4869205468657265");  // "Hi There"
    REQUIRE(hmac_sha256_hex(key, data) ==
            "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7");
}

TEST_CASE("hmac-sha256: RFC 4231 case 2", "[core][crypto][hmac]") {
    const auto key = from_hex("4a656665");                  // "Jefe"
    const auto data = from_hex(
        "7768617420646f2079612077616e7420666f72206e6f7468696e673f");  // "what do ya want for nothing?"
    REQUIRE(hmac_sha256_hex(key, data) ==
            "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843");
}

TEST_CASE("hmac-sha256: RFC 4231 case 3", "[core][crypto][hmac]") {
    const auto key = repeat(0xaa, 20);
    const auto data = repeat(0xdd, 50);
    REQUIRE(hmac_sha256_hex(key, data) ==
            "773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe");
}

TEST_CASE("hmac-sha256: RFC 4231 case 4", "[core][crypto][hmac]") {
    const auto key = from_hex("0102030405060708090a0b0c0d0e0f10111213141516171819");
    const auto data = repeat(0xcd, 50);
    REQUIRE(hmac_sha256_hex(key, data) ==
            "82558a389a443c0ea4cc819899f2083a85f0faa3e578f8077a2e3ff46729665b");
}

TEST_CASE("hmac-sha256: RFC 4231 case 5 (digest cheio)",
          "[core][crypto][hmac]") {
    // Case 5 do RFC trunca para 128 bits; aqui validamos o digest SHA-256 cheio
    // cujo prefixo de 128 bits e o do RFC (a0b495f3...).
    const auto key = repeat(0x0c, 20);
    const auto data = from_hex(
        "546573742057697468205472756e636174696f6e");  // "Test With Truncation"
    REQUIRE(hmac_sha256_hex(key, data) ==
            "a3b6167473100ee06e0c796c2955552bfa6f7c0a6a8aef8b93f860aab0cd20c5");
}

TEST_CASE("hmac-sha256: RFC 4231 case 6 (key > block)",
          "[core][crypto][hmac]") {
    // Chave de 131 bytes (> block 64) -> hash da chave (RFC 4231).
    const auto key = repeat(0xaa, 131);
    const auto data = from_hex(
        "54657374205573696e67204c6172676572205468616e20426c6f636b2d53697a"
        "65204b6579202d2048617368204b6579204669727374");
    // "Test Using Larger Than Block-Size Key - Hash Key First"
    REQUIRE(hmac_sha256_hex(key, data) ==
            "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54");
}

TEST_CASE("hmac-sha256: RFC 4231 case 7 (key e data grandes)",
          "[core][crypto][hmac]") {
    const auto key = repeat(0xaa, 131);
    const auto data = from_hex(
        "5468697320697320612074657374207573696e672061206c6172676572207468"
        "616e20626c6f636b2d73697a65206b657920616e642061206c61726765722074"
        "68616e20626c6f636b2d73697a6520646174612e20546865206b6579206e6565"
        "647320746f20626520686173686564206265666f7265206265696e6720757365"
        "642062792074686520484d414320616c676f726974686d2e");
    REQUIRE(hmac_sha256_hex(key, data) ==
            "9b09ffa71b942fcb27635fbcd5b0e944bfdc63644f0713938a7f51535c3a35e2");
}

// ---- Propriedades ----------------------------------------------------------

TEST_CASE("hmac-sha256: tag tem 32 bytes", "[core][crypto][hmac]") {
    const auto tag = hmac_sha256(repeat(0x01, 16), from_hex("00"));
    REQUIRE(tag.size() == 32u);
}

TEST_CASE("hmac-sha256: deterministico", "[core][crypto][hmac]") {
    const auto key = repeat(0x42, 32);
    const auto data = from_hex("deadbeef");
    REQUIRE(hmac_sha256(key, data) == hmac_sha256(key, data));
}

TEST_CASE("hmac-sha256: chave diferente muda a tag",
          "[core][crypto][hmac]") {
    const auto data = from_hex("deadbeef");
    REQUIRE(hmac_sha256(repeat(0x01, 32), data) !=
            hmac_sha256(repeat(0x02, 32), data));
}
