// aead_xchacha20poly1305_test.cpp
//
// Spec executavel (Catch2 v3) do AEAD XChaCha20-Poly1305 PROPRIO sobre o
// Monocypher vendorizado (ADR-015 decisao 1/2). Envelope GDS3 do save.
//
// Oraculo (ADR-015 "Riscos/atencao" (a) - bug de integracao do AEAD tem impacto
// MAIOR que bug de HMAC, quebra confidencialidade, nao so integridade):
//   (a) roundtrip: aead_unlock(aead_lock(x)) == x (vazio e grande);
//   (b) tamper-detection: flip de 1 byte em ciphertext/tag/nonce/AAD -> unlock
//       REJEITA (rc=false), sem crash;
//   (c) nonce UNICO: N chamadas de generate_nonce() nao repetem (CSPRNG, o bug de
//       crypto #1 - reuso de nonce quebra confidencialidade E abre forjamento);
//   (d) vetor congelado (golden, gerado do PROPRIO Monocypher vendorizado 4.0.3,
//       commit ab2b16dd619ad5f6979a4fbe69cfa324a6fcc35f, via programa standalone -
//       nao ha RFC numerado para a extensao XChaCha do AEAD simples do Monocypher,
//       "caveat honesto" do ADR).
//
// Cross-ref: docs/tech/adr/ADR-015-save-security-v2-offline.md decisao 1/2,
//            gus/core/crypto/aead_xchacha20poly1305.hpp.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <cstring>
#include <set>
#include <string>
#include <vector>

#include "gus/core/crypto/aead_xchacha20poly1305.hpp"

using gus::core::crypto::aead_lock;
using gus::core::crypto::aead_unlock;
using gus::core::crypto::generate_nonce;
using gus::core::crypto::kAeadKeySize;
using gus::core::crypto::kAeadNonceSize;
using gus::core::crypto::kAeadTagSize;

namespace {

std::array<std::uint8_t, kAeadKeySize> fixed_key() {
    std::array<std::uint8_t, kAeadKeySize> k{};
    for (std::size_t i = 0; i < k.size(); ++i) k[i] = static_cast<std::uint8_t>(i);
    return k;
}

std::array<std::uint8_t, kAeadNonceSize> fixed_nonce() {
    std::array<std::uint8_t, kAeadNonceSize> n{};
    for (std::size_t i = 0; i < n.size(); ++i)
        n[i] = static_cast<std::uint8_t>(0x40 + i);
    return n;
}

const std::uint8_t* as_bytes(const std::string& s) {
    return reinterpret_cast<const std::uint8_t*>(s.data());
}

std::string from_hex(const std::string& hex) {
    std::string out;
    out.reserve(hex.size() / 2);
    auto nib = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return -1;
    };
    for (std::size_t i = 0; i < hex.size(); i += 2)
        out.push_back(static_cast<char>((nib(hex[i]) << 4) | nib(hex[i + 1])));
    return out;
}

}  // namespace

// ---- (a) roundtrip -----------------------------------------------------------

TEST_CASE("aead: roundtrip preserva o plaintext", "[core][crypto][aead]") {
    const auto key = fixed_key();
    const auto nonce = generate_nonce();
    const std::string plain = "gusworld save v2 payload de teste, roundtrip AEAD.";
    const std::string ad = "GDS3-AAD-slot0-rollback0";

    const auto locked = aead_lock(key, nonce, as_bytes(ad), ad.size(),
                                  as_bytes(plain), plain.size());

    std::vector<std::uint8_t> decrypted;
    const bool ok = aead_unlock(decrypted, key, nonce, as_bytes(ad), ad.size(),
                                locked.cipher_text.data(), locked.tag, plain.size());
    REQUIRE(ok);
    REQUIRE(decrypted.size() == plain.size());
    REQUIRE(std::memcmp(decrypted.data(), plain.data(), plain.size()) == 0);
}

TEST_CASE("aead: roundtrip de plaintext VAZIO", "[core][crypto][aead]") {
    const auto key = fixed_key();
    const auto nonce = generate_nonce();
    const auto locked = aead_lock(key, nonce, nullptr, 0, nullptr, 0);
    REQUIRE(locked.cipher_text.empty());

    std::vector<std::uint8_t> decrypted;
    const bool ok =
        aead_unlock(decrypted, key, nonce, nullptr, 0, nullptr, locked.tag, 0);
    REQUIRE(ok);
    REQUIRE(decrypted.empty());
}

TEST_CASE("aead: roundtrip de plaintext GRANDE (payload de save realista)",
          "[core][crypto][aead]") {
    const auto key = fixed_key();
    const auto nonce = generate_nonce();
    std::vector<std::uint8_t> plain(64 * 1024);
    for (std::size_t i = 0; i < plain.size(); ++i)
        plain[i] = static_cast<std::uint8_t>(i * 37 + 11);
    const std::string ad = "GDS3-AAD-slot2-rollback0";

    const auto locked =
        aead_lock(key, nonce, as_bytes(ad), ad.size(), plain.data(), plain.size());

    std::vector<std::uint8_t> decrypted;
    const bool ok = aead_unlock(decrypted, key, nonce, as_bytes(ad), ad.size(),
                                locked.cipher_text.data(), locked.tag, plain.size());
    REQUIRE(ok);
    REQUIRE(decrypted == plain);
}

// ---- (b) tamper-detection ----------------------------------------------------

TEST_CASE("aead: flip no ciphertext rejeita", "[core][crypto][aead]") {
    const auto key = fixed_key();
    const auto nonce = generate_nonce();
    const std::string plain = "conteudo secreto do save";
    auto locked = aead_lock(key, nonce, nullptr, 0, as_bytes(plain), plain.size());

    locked.cipher_text[0] ^= 0xFF;
    std::vector<std::uint8_t> decrypted;
    const bool ok = aead_unlock(decrypted, key, nonce, nullptr, 0,
                                locked.cipher_text.data(), locked.tag, plain.size());
    REQUIRE_FALSE(ok);
}

TEST_CASE("aead: flip na tag rejeita", "[core][crypto][aead]") {
    const auto key = fixed_key();
    const auto nonce = generate_nonce();
    const std::string plain = "conteudo secreto do save";
    auto locked = aead_lock(key, nonce, nullptr, 0, as_bytes(plain), plain.size());

    locked.tag[0] ^= 0xFF;
    std::vector<std::uint8_t> decrypted;
    const bool ok = aead_unlock(decrypted, key, nonce, nullptr, 0,
                                locked.cipher_text.data(), locked.tag, plain.size());
    REQUIRE_FALSE(ok);
}

TEST_CASE("aead: nonce errado (diferente do usado no lock) rejeita",
          "[core][crypto][aead]") {
    const auto key = fixed_key();
    const auto nonce = generate_nonce();
    const std::string plain = "conteudo secreto do save";
    const auto locked = aead_lock(key, nonce, nullptr, 0, as_bytes(plain), plain.size());

    auto wrong_nonce = nonce;
    wrong_nonce[0] ^= 0xFF;
    std::vector<std::uint8_t> decrypted;
    const bool ok = aead_unlock(decrypted, key, wrong_nonce, nullptr, 0,
                                locked.cipher_text.data(), locked.tag, plain.size());
    REQUIRE_FALSE(ok);
}

TEST_CASE("aead: AAD divergente rejeita (troca de slot_id/rollback_ctr)",
          "[core][crypto][aead]") {
    const auto key = fixed_key();
    const auto nonce = generate_nonce();
    const std::string plain = "conteudo secreto do save";
    const std::string ad_original = "GDS3-slot2-rollback0";
    const std::string ad_trocado = "GDS3-slot5-rollback0";  // slot trocado

    const auto locked = aead_lock(key, nonce, as_bytes(ad_original), ad_original.size(),
                                  as_bytes(plain), plain.size());

    std::vector<std::uint8_t> decrypted;
    const bool ok =
        aead_unlock(decrypted, key, nonce, as_bytes(ad_trocado), ad_trocado.size(),
                    locked.cipher_text.data(), locked.tag, plain.size());
    REQUIRE_FALSE(ok);
}

TEST_CASE("aead: chave errada rejeita", "[core][crypto][aead]") {
    const auto key = fixed_key();
    const auto nonce = generate_nonce();
    const std::string plain = "conteudo secreto do save";
    const auto locked = aead_lock(key, nonce, nullptr, 0, as_bytes(plain), plain.size());

    auto wrong_key = key;
    wrong_key[0] ^= 0xFF;
    std::vector<std::uint8_t> decrypted;
    const bool ok = aead_unlock(decrypted, wrong_key, nonce, nullptr, 0,
                                locked.cipher_text.data(), locked.tag, plain.size());
    REQUIRE_FALSE(ok);
}

// ---- (c) nonce unico (CSPRNG, bug de crypto #1) ------------------------------

TEST_CASE("aead: generate_nonce() nao repete em N chamadas seguidas",
          "[core][crypto][aead]") {
    std::set<std::string> seen;
    constexpr int kCalls = 200;
    for (int i = 0; i < kCalls; ++i) {
        const auto nonce = generate_nonce();
        const std::string key(reinterpret_cast<const char*>(nonce.data()),
                              nonce.size());
        REQUIRE(seen.find(key) == seen.end());  // nunca visto antes
        seen.insert(key);
    }
    REQUIRE(seen.size() == static_cast<std::size_t>(kCalls));
}

TEST_CASE("aead: dois locks do MESMO plaintext com nonces diferentes produzem "
          "ciphertexts diferentes (sem reuso de keystream)",
          "[core][crypto][aead]") {
    const auto key = fixed_key();
    const std::string plain = "mesmo conteudo, duas escritas seguidas";

    const auto locked1 =
        aead_lock(key, generate_nonce(), nullptr, 0, as_bytes(plain), plain.size());
    const auto locked2 =
        aead_lock(key, generate_nonce(), nullptr, 0, as_bytes(plain), plain.size());

    REQUIRE(locked1.cipher_text != locked2.cipher_text);
    REQUIRE(locked1.tag != locked2.tag);
}

// ---- (d) vetor congelado (golden, gerado do Monocypher vendorizado) ---------

TEST_CASE("aead: vetor congelado (golden, Monocypher 4.0.3 vendorizado)",
          "[core][crypto][aead][golden]") {
    // Gerado via programa standalone linkando third_party/monocypher/monocypher.c
    // direto (mesmo binario vendorizado neste repo, commit
    // ab2b16dd619ad5f6979a4fbe69cfa324a6fcc35f), com os MESMOS key/nonce/ad/plain
    // deste caso. Nao existe um numero de RFC dedicado a extensao XChaCha do AEAD
    // simples do Monocypher (RFC 8439 e so o ChaCha20-Poly1305 IETF de nonce 12
    // bytes) - "caveat honesto" do ADR-015: e a prova de que o WRAPPER chama a
    // biblioteca corretamente, nao um vetor de padrao externo.
    const auto key = fixed_key();
    const auto nonce = fixed_nonce();
    const std::string ad = "GDS3-AAD-slot0-rollback0";
    const std::string plain = "gusworld save v2 payload de teste, roundtrip AEAD.";

    const auto locked =
        aead_lock(key, nonce, as_bytes(ad), ad.size(), as_bytes(plain), plain.size());

    const std::string expected_cipher_hex =
        "b34c7607bf921572af87e6c8cabc13a0b2caccbd7f3632fe4a5598657d6150e473d5221"
        "c3ae7ea748bc09d3c2a9b9fb0df45";
    const std::string expected_mac_hex = "2bd3940910b9ca2dddb28462bf712cfc";

    const auto expected_cipher = from_hex(expected_cipher_hex);
    REQUIRE(locked.cipher_text.size() == expected_cipher.size());
    REQUIRE(std::memcmp(locked.cipher_text.data(), expected_cipher.data(),
                        expected_cipher.size()) == 0);

    const auto expected_mac = from_hex(expected_mac_hex);
    REQUIRE(expected_mac.size() == kAeadTagSize);
    REQUIRE(std::memcmp(locked.tag.data(), expected_mac.data(), kAeadTagSize) == 0);
}
