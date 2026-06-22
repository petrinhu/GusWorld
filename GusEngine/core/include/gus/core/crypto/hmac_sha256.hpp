// gus/core/crypto/hmac_sha256.hpp
//
// HMAC-SHA256 PROPRIO (RFC 2104 / RFC 4231) sobre o SHA-256 de sha256.hpp.
// Dominio publico, deterministico, ZERO dependencia externa, ZERO Qt.
//
// Uso: selo anti-tamper dos envelopes binarios de templates/save (ADR-006). A
// correcao e travada TEST-FIRST contra os 7 test cases do RFC 4231
// (tests/hmac_sha256_test.cpp). Integridade casual local, nao sigilo: a chave
// mora no binario (extraivel por decompile), o aceitavel para save single-player.
//
// Cross-ref: docs/tech/adr/ADR-006-crypto-hmac-formato-domain.md, RFC 4231.

#ifndef GUS_CORE_CRYPTO_HMAC_SHA256_HPP
#define GUS_CORE_CRYPTO_HMAC_SHA256_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "gus/core/crypto/sha256.hpp"

namespace gus::core::crypto {

// HMAC-SHA256(key, message) -> tag de 32 bytes (RFC 2104). Chave de qualquer
// tamanho: chaves > 64 bytes sao reduzidas por SHA-256 (RFC 4231), chaves
// menores sao zero-padded ate o bloco. Deterministico.
[[nodiscard]] std::array<std::uint8_t, kSha256DigestSize> hmac_sha256(
    const std::uint8_t* key, std::size_t key_size,
    const std::uint8_t* message, std::size_t message_size);

// Sobrecarga conveniente para vectors de bytes.
[[nodiscard]] std::array<std::uint8_t, kSha256DigestSize> hmac_sha256(
    const std::vector<std::uint8_t>& key,
    const std::vector<std::uint8_t>& message);

// Tag HMAC-SHA256 em hex lowercase (64 chars).
[[nodiscard]] std::string hmac_sha256_hex(
    const std::vector<std::uint8_t>& key,
    const std::vector<std::uint8_t>& message);

// Comparacao em TEMPO CONSTANTE de duas tags de 32 bytes (anti timing-oracle no
// ponto de verificacao). Espelha CryptographicOperations.FixedTimeEquals do C#.
[[nodiscard]] bool fixed_time_equals(
    const std::array<std::uint8_t, kSha256DigestSize>& a,
    const std::array<std::uint8_t, kSha256DigestSize>& b);

}  // namespace gus::core::crypto

#endif  // GUS_CORE_CRYPTO_HMAC_SHA256_HPP
