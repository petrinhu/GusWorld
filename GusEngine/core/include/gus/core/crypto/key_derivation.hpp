// gus/core/crypto/key_derivation.hpp
//
// KDF SIMPLES (derivacao de chave) sobre o SHA-256 proprio de sha256.hpp. Dominio
// publico, deterministico, ZERO dependencia externa, ZERO Qt (core/ e POCO puro).
//
// MOTIVACAO (ADR-006 T2.2): a chave de integridade do HMAC dos envelopes (save) era
// embutida CRUA no codigo. Aqui a chave passa a ser DERIVADA de um segredo-base +
// um contexto (separacao de dominio: save vs templates). Transparente ao formato
// (nao muda layout do envelope), so muda o VALOR da chave (e do HMAC).
//
// ATENCAO (ADR-006: "so a CHAVE e sensivel"): trocar a derivacao MUDA todos os
// HMACs. Como nao ha saves reais (DEV), o re-baseline e indolor; pos-release seria
// breaking. Isto NAO e sigilo (o segredo-base vive no binario): e soberania da
// ORIGEM da chave (a chave nao aparece literal no codigo, e computada).
//
// DEFINICAO: derive_key(base, ctx) = SHA-256( base || 0x00 || ctx ), 32 bytes. O
// byte separador 0x00 impede colisao de fronteira entre (base="ab", ctx="") e
// (base="a", ctx="b").
//
// Cross-ref: docs/tech/adr/ADR-006-crypto-hmac-formato-domain.md (T2.2),
//            gus/core/crypto/sha256.hpp, FIPS 180-4.

#ifndef GUS_CORE_CRYPTO_KEY_DERIVATION_HPP
#define GUS_CORE_CRYPTO_KEY_DERIVATION_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

namespace gus::core::crypto {

// Deriva uma chave de 32 bytes de um segredo-base + um contexto, via
// SHA-256( base || 0x00 || context ). Deterministico. O separador 0x00 da
// separacao de dominio (mesma base, contextos distintos -> chaves distintas; e
// sem colisao de fronteira entre base e contexto).
[[nodiscard]] std::vector<std::uint8_t> derive_key(
    const std::uint8_t* base, std::size_t base_size,
    const std::uint8_t* context, std::size_t context_size);

}  // namespace gus::core::crypto

#endif  // GUS_CORE_CRYPTO_KEY_DERIVATION_HPP
