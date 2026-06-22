// gus/core/crypto/sha256.hpp
//
// SHA-256 PROPRIO (FIPS 180-4), implementacao de dominio publico, deterministica,
// ZERO dependencia externa, ZERO Qt (core/ e POCO puro, invariante de camadas,
// engine-design.md secao 2 + ADR-006).
//
// Uso aqui: bloco de construcao do HMAC-SHA256 anti-tamper de templates/save. A
// correcao e travada TEST-FIRST contra os vetores oficiais FIPS 180-4 / NIST
// (tests/sha256_test.cpp). NAO e crypto de sigilo: integridade casual local.
//
// Cross-ref: docs/tech/adr/ADR-006-crypto-hmac-formato-domain.md, FIPS 180-4.

#ifndef GUS_CORE_CRYPTO_SHA256_HPP
#define GUS_CORE_CRYPTO_SHA256_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace gus::core::crypto {

// Tamanho do digest SHA-256 em bytes (256 bits).
inline constexpr std::size_t kSha256DigestSize = 32;

// Tamanho do bloco de compressao SHA-256 em bytes (512 bits). Usado pelo HMAC.
inline constexpr std::size_t kSha256BlockSize = 64;

// Digest SHA-256 (32 bytes) de uma sequencia de bytes. Deterministico.
[[nodiscard]] std::array<std::uint8_t, kSha256DigestSize> sha256(
    const std::uint8_t* data, std::size_t size);

// Sobrecarga conveniente para vector de bytes.
[[nodiscard]] std::array<std::uint8_t, kSha256DigestSize> sha256(
    const std::vector<std::uint8_t>& data);

// Digest SHA-256 em hex lowercase (64 chars).
[[nodiscard]] std::string sha256_hex(const std::vector<std::uint8_t>& data);

}  // namespace gus::core::crypto

#endif  // GUS_CORE_CRYPTO_SHA256_HPP
