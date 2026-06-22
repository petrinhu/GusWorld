// key_derivation_test.cpp
//
// Spec executavel (Catch2 v3) do KDF simples de core/crypto (ADR-006 T2.2). A chave
// de integridade do HMAC do save passa a ser DERIVADA de um segredo-base + contexto,
// em vez de embutida CRUA. Transparente ao formato (nao muda layout do envelope), so
// muda o VALOR da chave (e portanto do HMAC).
//
// Contrato (decisao do lider, ADR-006 "so a CHAVE e sensivel"):
//   derive_key(base, base_size, ctx, ctx_size) = SHA-256( base || 0x00 || ctx ),
//   devolvendo os 32 bytes do digest como vetor de chave. O byte separador 0x00
//   impede colisao de dominio entre (base="ab", ctx="c") e (base="a", ctx="bc").
//
// Propriedades exigidas:
//   (a) deterministico: mesma (base, ctx) -> mesma chave, sempre;
//   (b) sensivel ao contexto: contextos distintos -> chaves distintas (separacao
//       de dominio: a mesma base gera chaves diferentes para save vs templates);
//   (c) sensivel a base: bases distintas -> chaves distintas;
//   (d) separador anti-colisao: (base="ab", ctx="") != (base="a", ctx="b");
//   (e) tamanho de chave = 32 bytes (digest SHA-256 cheio).
//
// NAO e seguranca de sigilo (o segredo-base vive no binario, ADR-006); e soberania
// da ORIGEM da chave: a chave nao aparece literal no codigo, e derivada.
//
// Cross-ref: docs/tech/adr/ADR-006-crypto-hmac-formato-domain.md (T2.2),
//            gus/core/crypto/key_derivation.hpp, FIPS 180-4 (SHA-256).

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>
#include <vector>

#include "gus/core/crypto/key_derivation.hpp"
#include "gus/core/crypto/sha256.hpp"

using gus::core::crypto::derive_key;
using gus::core::crypto::kSha256DigestSize;
using gus::core::crypto::sha256;

namespace {

std::vector<std::uint8_t> bytes_of(const std::string& s) {
    return std::vector<std::uint8_t>(s.begin(), s.end());
}

std::vector<std::uint8_t> kd(const std::string& base, const std::string& ctx) {
    const auto b = bytes_of(base);
    const auto c = bytes_of(ctx);
    return derive_key(b.data(), b.size(), c.data(), c.size());
}

}  // namespace

// ---- (e) tamanho de chave = digest SHA-256 cheio (32 bytes) ----------------

TEST_CASE("kdf: chave derivada tem 32 bytes (digest SHA-256)",
          "[core][crypto][kdf]") {
    REQUIRE(kd("segredo-base", "save-v4-hmac").size() == kSha256DigestSize);
}

// ---- (a) determinismo ------------------------------------------------------

TEST_CASE("kdf: mesma base + contexto produz a MESMA chave (deterministico)",
          "[core][crypto][kdf]") {
    REQUIRE(kd("segredo-base", "save-v4-hmac") == kd("segredo-base", "save-v4-hmac"));
}

// ---- (b) separacao de dominio por contexto ---------------------------------

TEST_CASE("kdf: contextos diferentes da MESMA base geram chaves diferentes",
          "[core][crypto][kdf]") {
    REQUIRE(kd("segredo-base", "save-v4-hmac") != kd("segredo-base", "templates-hmac"));
}

// ---- (c) sensivel a base ---------------------------------------------------

TEST_CASE("kdf: bases diferentes geram chaves diferentes",
          "[core][crypto][kdf]") {
    REQUIRE(kd("segredo-A", "ctx") != kd("segredo-B", "ctx"));
}

// ---- (d) separador anti-colisao de fronteira -------------------------------

TEST_CASE("kdf: separador impede colisao de fronteira base/contexto",
          "[core][crypto][kdf]") {
    // Sem separador, ("ab","") e ("a","b") concatenariam ao mesmo "ab".
    REQUIRE(kd("ab", "") != kd("a", "b"));
}

// ---- vetor congelado: a derivacao e exatamente SHA-256(base || 0x00 || ctx) -

TEST_CASE("kdf: deriva exatamente SHA-256(base || 0x00 || contexto)",
          "[core][crypto][kdf]") {
    const std::string base = "segredo-base";
    const std::string ctx = "save-v4-hmac";
    std::vector<std::uint8_t> preimage(base.begin(), base.end());
    preimage.push_back(0x00);
    preimage.insert(preimage.end(), ctx.begin(), ctx.end());
    const auto expected = sha256(preimage);

    const auto key = kd(base, ctx);
    REQUIRE(key.size() == expected.size());
    REQUIRE(std::equal(key.begin(), key.end(), expected.begin()));
}
