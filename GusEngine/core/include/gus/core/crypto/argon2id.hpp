// gus/core/crypto/argon2id.hpp
//
// Argon2id (RFC 9106) PROPRIO sobre o Monocypher vendorizado (ADR-015 decisao
// 1/3). KDF memory-hard: deriva uma chave de 32 bytes de um segredo (pass) + um
// sal (salt), com custo de memoria/CPU parametrizavel. Uso: chave do AEAD do save
// v2 (envelope GDS3); a derivacao roda so em save/load (poucas vezes por sessao,
// NUNCA por frame) - o custo de dezenas de ms e irrelevante nesse ponto.
//
// PISO recomendado pela OWASP Password Storage Cheat Sheet (ADR-015 decisao 3,
// ratificada pelo lider 2026-07-10): m=19456 KiB (19 MiB), t=2, p=1. `p` fica
// travado em 1 porque o Monocypher nao implementa paralelismo real (`nb_lanes` so
// particiona o work area, sem threads) - e o mesmo valor que este piso ja pede.
//
// Cross-ref: docs/tech/adr/ADR-015-save-security-v2-offline.md decisao 3, RFC 9106,
//            GusEngine/third_party/monocypher/monocypher.h (crypto_argon2).

#ifndef GUS_CORE_CRYPTO_ARGON2ID_HPP
#define GUS_CORE_CRYPTO_ARGON2ID_HPP

#include <array>
#include <cstddef>
#include <cstdint>

namespace gus::core::crypto {

inline constexpr std::size_t kArgon2idKeySize = 32;

// Piso OWASP (ADR-015 decisao 3): memoria em KiB, passes de CPU, lanes (=1, sem
// paralelismo real no Monocypher).
inline constexpr std::uint32_t kArgon2idOwaspMemoryKib = 19456;  // 19 MiB
inline constexpr std::uint32_t kArgon2idOwaspPasses = 2;
inline constexpr std::uint32_t kArgon2idOwaspLanes = 1;

// Deriva uma chave de 32 bytes via Argon2id (RFC 9106). `pass` e o segredo de
// entrada (ex.: segredo-base embutido, ou segredo-base+fingerprint de maquina no
// slot Hardcore, ADR-015 decisao 3); `salt` da separacao de dominio/contexto
// (recomendado >= 16 bytes; o Monocypher aceita qualquer tamanho > 0).
// Deterministico: mesma entrada -> mesma chave sempre. Aloca e zera (secure_zero)
// a work area internamente (memory_kib * 1024 bytes).
[[nodiscard]] std::array<std::uint8_t, kArgon2idKeySize> derive_key_argon2id(
    const std::uint8_t* pass, std::size_t pass_size, const std::uint8_t* salt,
    std::size_t salt_size, std::uint32_t memory_kib = kArgon2idOwaspMemoryKib,
    std::uint32_t passes = kArgon2idOwaspPasses,
    std::uint32_t lanes = kArgon2idOwaspLanes);

}  // namespace gus::core::crypto

#endif  // GUS_CORE_CRYPTO_ARGON2ID_HPP
