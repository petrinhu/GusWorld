// argon2id_test.cpp
//
// Spec executavel (Catch2 v3) do Argon2id PROPRIO sobre o Monocypher vendorizado
// (ADR-015 decisao 1/3). KDF memory-hard: chave do AEAD do save v2.
//
// Oraculo:
//   (a) determinismo: mesma entrada (pass+salt+params) -> mesma chave sempre;
//   (b) sensibilidade: pass ou salt diferente -> chave diferente (separacao de
//       dominio / nao ha atalho que ignore parte da entrada);
//   (c) vetor congelado (golden, gerado do PROPRIO Monocypher vendorizado 4.0.3,
//       commit ab2b16dd619ad5f6979a4fbe69cfa324a6fcc35f, via programa standalone
//       linkando monocypher.c direto - nao ha vetor oficial RFC 9106 publico com
//       estes parametros de teste especificos, entao o oraculo e o proprio
//       vendored, nao um numero de RFC. Ver ADR-015 "caveat honesto");
//   (d) parametros de PRODUCAO (piso OWASP m=19456 KiB/t=2/p=1) produzem chave de
//       32 bytes (prova que o wrapper aceita os parametros reais, nao so os
//       reduzidos usados nos outros casos para velocidade de CI).
//
// Cross-ref: docs/tech/adr/ADR-015-save-security-v2-offline.md decisao 3,
//            gus/core/crypto/argon2id.hpp, RFC 9106.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <cstring>
#include <string>

#include "gus/core/crypto/argon2id.hpp"

using gus::core::crypto::derive_key_argon2id;
using gus::core::crypto::kArgon2idKeySize;
using gus::core::crypto::kArgon2idOwaspLanes;
using gus::core::crypto::kArgon2idOwaspMemoryKib;
using gus::core::crypto::kArgon2idOwaspPasses;

namespace {

// Parametros REDUZIDOS (8 KiB, 1 pass) so para velocidade dos casos que nao
// precisam do custo real de producao (determinismo/sensibilidade/golden). O caso
// "parametros OWASP" abaixo usa os DEFAULTS reais.
constexpr std::uint32_t kFastMemoryKib = 8;
constexpr std::uint32_t kFastPasses = 1;
constexpr std::uint32_t kFastLanes = 1;

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

// ---- (a) determinismo -------------------------------------------------------

TEST_CASE("argon2id: mesma entrada produz sempre a mesma chave",
          "[core][crypto][argon2id]") {
    const std::string pass = "segredo-base-de-teste-gusworld";
    const std::string salt = "gusworld-save-v2-salt-fixo-teste";
    const auto k1 = derive_key_argon2id(as_bytes(pass), pass.size(), as_bytes(salt),
                                        salt.size(), kFastMemoryKib, kFastPasses,
                                        kFastLanes);
    const auto k2 = derive_key_argon2id(as_bytes(pass), pass.size(), as_bytes(salt),
                                        salt.size(), kFastMemoryKib, kFastPasses,
                                        kFastLanes);
    REQUIRE(k1 == k2);
    REQUIRE(k1.size() == kArgon2idKeySize);
}

// ---- (b) sensibilidade -------------------------------------------------------

TEST_CASE("argon2id: pass diferente muda a chave", "[core][crypto][argon2id]") {
    const std::string salt = "gusworld-save-v2-salt-fixo-teste";
    const auto k1 = derive_key_argon2id(as_bytes(std::string("segredo-a")), 9,
                                        as_bytes(salt), salt.size(), kFastMemoryKib,
                                        kFastPasses, kFastLanes);
    const auto k2 = derive_key_argon2id(as_bytes(std::string("segredo-b")), 9,
                                        as_bytes(salt), salt.size(), kFastMemoryKib,
                                        kFastPasses, kFastLanes);
    REQUIRE(k1 != k2);
}

TEST_CASE("argon2id: salt diferente muda a chave (separacao de dominio)",
          "[core][crypto][argon2id]") {
    const std::string pass = "segredo-base-de-teste-gusworld";
    const auto k1 = derive_key_argon2id(as_bytes(pass), pass.size(),
                                        as_bytes(std::string("contexto-save")), 13,
                                        kFastMemoryKib, kFastPasses, kFastLanes);
    const auto k2 = derive_key_argon2id(as_bytes(pass), pass.size(),
                                        as_bytes(std::string("contexto-outro")), 14,
                                        kFastMemoryKib, kFastPasses, kFastLanes);
    REQUIRE(k1 != k2);
}

// ---- (c) vetor congelado (golden, gerado do Monocypher vendorizado) ---------

TEST_CASE("argon2id: vetor congelado (golden, Monocypher 4.0.3 vendorizado)",
          "[core][crypto][argon2id][golden]") {
    // Gerado via programa standalone linkando third_party/monocypher/monocypher.c
    // direto (mesmo binario vendorizado neste repo, commit
    // ab2b16dd619ad5f6979a4fbe69cfa324a6fcc35f), com os MESMOS parametros deste
    // caso. Nao e um vetor oficial de RFC (RFC 9106 nao publica vetores para
    // parametros arbitrarios de teste); e a prova de que o WRAPPER chama o
    // Monocypher corretamente (regressao: se o wrapper mudar de biblioteca ou de
    // convencao de chamada, este teste pega o desvio).
    const std::string pass = "segredo-base-de-teste-gusworld";
    const std::string salt = "gusworld-save-v2-salt-fixo-teste";
    const auto k = derive_key_argon2id(as_bytes(pass), pass.size(), as_bytes(salt),
                                       salt.size(), kFastMemoryKib, kFastPasses,
                                       kFastLanes);
    const std::string expected = from_hex(
        "ce01aa56ce0ac49e594ec11d39a47f6fef1bab641cf459a2d6eaa2c9fd89a0e8");
    REQUIRE(std::memcmp(k.data(), expected.data(), k.size()) == 0);
}

// ---- (d) parametros de PRODUCAO (piso OWASP) --------------------------------

TEST_CASE("argon2id: parametros OWASP de producao (m=19456 KiB/t=2/p=1)",
          "[core][crypto][argon2id]") {
    const std::string pass = "segredo-base-embutido-producao";
    const std::string salt = "gusworld-save-v2";
    const auto k = derive_key_argon2id(as_bytes(pass), pass.size(), as_bytes(salt),
                                       salt.size());  // defaults = piso OWASP
    REQUIRE(k.size() == kArgon2idKeySize);
    REQUIRE(kArgon2idOwaspMemoryKib == 19456);
    REQUIRE(kArgon2idOwaspPasses == 2);
    REQUIRE(kArgon2idOwaspLanes == 1);
}
