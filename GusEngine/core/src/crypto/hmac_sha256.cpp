// gus/core/src/crypto/hmac_sha256.cpp
//
// Implementacao HMAC-SHA256 (RFC 2104) sobre o SHA-256 proprio. Ver header para
// o contrato. Dominio publico, deterministica, ZERO dep externa, ZERO Qt.
// Travada contra os 7 test cases do RFC 4231 em tests/hmac_sha256_test.cpp.

#include "gus/core/crypto/hmac_sha256.hpp"

#include <array>
#include <cstring>

namespace gus::core::crypto {

std::array<std::uint8_t, kSha256DigestSize> hmac_sha256(
    const std::uint8_t* key, std::size_t key_size, const std::uint8_t* message,
    std::size_t message_size) {
    // Bloco de chave normalizado para kSha256BlockSize (RFC 2104): chave maior
    // que o bloco e reduzida por SHA-256; depois zero-padded.
    std::array<std::uint8_t, kSha256BlockSize> key_block{};  // zero-inicializado
    if (key_size > kSha256BlockSize) {
        const auto hashed = sha256(key, key_size);
        std::memcpy(key_block.data(), hashed.data(), hashed.size());
    } else {
        std::memcpy(key_block.data(), key, key_size);
    }

    // ipad/opad (RFC 2104).
    std::array<std::uint8_t, kSha256BlockSize> i_pad{};
    std::array<std::uint8_t, kSha256BlockSize> o_pad{};
    for (std::size_t i = 0; i < kSha256BlockSize; ++i) {
        i_pad[i] = static_cast<std::uint8_t>(key_block[i] ^ 0x36u);
        o_pad[i] = static_cast<std::uint8_t>(key_block[i] ^ 0x5cu);
    }

    // inner = SHA256( i_pad || message ).
    std::vector<std::uint8_t> inner_in;
    inner_in.reserve(kSha256BlockSize + message_size);
    inner_in.insert(inner_in.end(), i_pad.begin(), i_pad.end());
    inner_in.insert(inner_in.end(), message, message + message_size);
    const auto inner = sha256(inner_in);

    // outer = SHA256( o_pad || inner ).
    std::vector<std::uint8_t> outer_in;
    outer_in.reserve(kSha256BlockSize + inner.size());
    outer_in.insert(outer_in.end(), o_pad.begin(), o_pad.end());
    outer_in.insert(outer_in.end(), inner.begin(), inner.end());
    return sha256(outer_in);
}

std::array<std::uint8_t, kSha256DigestSize> hmac_sha256(
    const std::vector<std::uint8_t>& key,
    const std::vector<std::uint8_t>& message) {
    return hmac_sha256(key.data(), key.size(), message.data(), message.size());
}

std::string hmac_sha256_hex(const std::vector<std::uint8_t>& key,
                            const std::vector<std::uint8_t>& message) {
    static constexpr char kHex[] = "0123456789abcdef";
    const auto tag = hmac_sha256(key, message);
    std::string out;
    out.reserve(tag.size() * 2);
    for (std::uint8_t byte : tag) {
        out.push_back(kHex[byte >> 4]);
        out.push_back(kHex[byte & 0x0f]);
    }
    return out;
}

bool fixed_time_equals(const std::array<std::uint8_t, kSha256DigestSize>& a,
                       const std::array<std::uint8_t, kSha256DigestSize>& b) {
    // XOR acumulado: o tempo nao depende de onde o primeiro byte difere.
    std::uint8_t diff = 0;
    for (std::size_t i = 0; i < kSha256DigestSize; ++i) {
        diff = static_cast<std::uint8_t>(diff | (a[i] ^ b[i]));
    }
    return diff == 0;
}

}  // namespace gus::core::crypto
