// gus/core/src/crypto/sha256.cpp
//
// Implementacao SHA-256 (FIPS 180-4). Ver header para o contrato. Dominio
// publico, deterministica, ZERO dependencia externa, ZERO Qt. Travada contra os
// vetores oficiais em tests/sha256_test.cpp.

#include "gus/core/crypto/sha256.hpp"

#include <cstring>

namespace gus::core::crypto {

namespace {

// Constantes de round K[0..63] = parte fracionaria das raizes cubicas dos 64
// primeiros primos (FIPS 180-4 secao 4.2.2).
constexpr std::uint32_t kK[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu,
    0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u, 0xd807aa98u, 0x12835b01u,
    0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u,
    0xc19bf174u, 0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau, 0x983e5152u,
    0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u,
    0x06ca6351u, 0x14292967u, 0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu,
    0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u,
    0xd6990624u, 0xf40e3585u, 0x106aa070u, 0x19a4c116u, 0x1e376c08u,
    0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu,
    0x682e6ff3u, 0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u};

[[nodiscard]] inline std::uint32_t rotr(std::uint32_t x, unsigned n) {
    return (x >> n) | (x << (32u - n));
}

// Processa um bloco de 64 bytes, atualizando o estado h[0..7] (FIPS 180-4 6.2.2).
void process_block(std::uint32_t h[8], const std::uint8_t* block) {
    std::uint32_t w[64];
    for (int i = 0; i < 16; ++i) {
        w[i] = (static_cast<std::uint32_t>(block[i * 4]) << 24) |
               (static_cast<std::uint32_t>(block[i * 4 + 1]) << 16) |
               (static_cast<std::uint32_t>(block[i * 4 + 2]) << 8) |
               (static_cast<std::uint32_t>(block[i * 4 + 3]));
    }
    for (int i = 16; i < 64; ++i) {
        const std::uint32_t s0 =
            rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        const std::uint32_t s1 =
            rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
    std::uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];

    for (int i = 0; i < 64; ++i) {
        const std::uint32_t s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
        const std::uint32_t ch = (e & f) ^ (~e & g);
        const std::uint32_t t1 = hh + s1 + ch + kK[i] + w[i];
        const std::uint32_t s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
        const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        const std::uint32_t t2 = s0 + maj;

        hh = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    h[0] += a;
    h[1] += b;
    h[2] += c;
    h[3] += d;
    h[4] += e;
    h[5] += f;
    h[6] += g;
    h[7] += hh;
}

}  // namespace

std::array<std::uint8_t, kSha256DigestSize> sha256(const std::uint8_t* data,
                                                   std::size_t size) {
    // Estado inicial h[0..7] = parte fracionaria das raizes quadradas dos 8
    // primeiros primos (FIPS 180-4 secao 5.3.3).
    std::uint32_t h[8] = {0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
                          0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u};

    // Processa todos os blocos completos de 64 bytes.
    std::size_t offset = 0;
    while (size - offset >= kSha256BlockSize) {
        process_block(h, data + offset);
        offset += kSha256BlockSize;
    }

    // Bloco final com padding (FIPS 180-4 secao 5.1.1): byte 0x80, zeros, e o
    // comprimento da mensagem em BITS como big-endian de 64 bits.
    std::uint8_t tail[2 * kSha256BlockSize] = {0};
    const std::size_t rem = size - offset;
    std::memcpy(tail, data + offset, rem);
    tail[rem] = 0x80;

    // Se nao couber o length de 8 bytes no bloco atual (rem >= 56), usa 2 blocos.
    const std::size_t tail_blocks = (rem >= kSha256BlockSize - 8) ? 2 : 1;
    const std::size_t tail_size = tail_blocks * kSha256BlockSize;

    const std::uint64_t bit_len = static_cast<std::uint64_t>(size) * 8u;
    for (int i = 0; i < 8; ++i) {
        tail[tail_size - 1 - i] =
            static_cast<std::uint8_t>((bit_len >> (8 * i)) & 0xffu);
    }

    for (std::size_t b = 0; b < tail_blocks; ++b) {
        process_block(h, tail + b * kSha256BlockSize);
    }

    std::array<std::uint8_t, kSha256DigestSize> digest{};
    for (int i = 0; i < 8; ++i) {
        digest[i * 4] = static_cast<std::uint8_t>((h[i] >> 24) & 0xffu);
        digest[i * 4 + 1] = static_cast<std::uint8_t>((h[i] >> 16) & 0xffu);
        digest[i * 4 + 2] = static_cast<std::uint8_t>((h[i] >> 8) & 0xffu);
        digest[i * 4 + 3] = static_cast<std::uint8_t>(h[i] & 0xffu);
    }
    return digest;
}

std::array<std::uint8_t, kSha256DigestSize> sha256(
    const std::vector<std::uint8_t>& data) {
    return sha256(data.data(), data.size());
}

std::string sha256_hex(const std::vector<std::uint8_t>& data) {
    static constexpr char kHex[] = "0123456789abcdef";
    const auto digest = sha256(data);
    std::string out;
    out.reserve(digest.size() * 2);
    for (std::uint8_t byte : digest) {
        out.push_back(kHex[byte >> 4]);
        out.push_back(kHex[byte & 0x0f]);
    }
    return out;
}

}  // namespace gus::core::crypto
