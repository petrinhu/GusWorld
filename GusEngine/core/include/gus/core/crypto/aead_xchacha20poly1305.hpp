// gus/core/crypto/aead_xchacha20poly1305.hpp
//
// AEAD XChaCha20-Poly1305 PROPRIO sobre o Monocypher vendorizado (ADR-015 decisao
// 1/2). Nonce de 24 bytes (extensao XChaCha do ChaCha20-Poly1305, RFC 8439):
// espaco grande o bastante para gerar aleatorio a cada chamada SEM esquema de
// contador (CSPRNG, NUNCA reusado - reuso de nonce quebra confidencialidade E abre
// forjamento universal do MAC, ver ADR-015 decisao 1 "Riscos/atencao" (a)).
//
// Uso: envelope GDS3 do save (ADR-015 decisao 2) - substitui o HMAC-SHA256 do
// envelope GDS2 (ADR-006) por cifrar+autenticar numa unica operacao. O
// HMAC-SHA256 proprio (hmac_sha256.hpp) NAO e removido: continua servindo outros
// consumidores (controls_hash128 ADR-007, selo de conteudo ADR-012), que nao sao
// sigilo e ficam fora do escopo deste ADR.
//
// Cross-ref: docs/tech/adr/ADR-015-save-security-v2-offline.md decisao 1/2,
//            GusEngine/third_party/monocypher/monocypher.h (crypto_aead_lock/unlock).

#ifndef GUS_CORE_CRYPTO_AEAD_XCHACHA20POLY1305_HPP
#define GUS_CORE_CRYPTO_AEAD_XCHACHA20POLY1305_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gus::core::crypto {

inline constexpr std::size_t kAeadKeySize = 32;
inline constexpr std::size_t kAeadNonceSize = 24;
inline constexpr std::size_t kAeadTagSize = 16;

// Gera um nonce de 24 bytes ALEATORIO via CSPRNG do SO (std::random_device; ver
// .cpp). Chamar UMA VEZ por escrita (envelope/lock): nunca reusar, nunca derivar
// deterministicamente. Nao e a mesma classe de RNG de gameplay (random_source.hpp
// do domain, seedado e reproduzivel de proposito) - aqui a garantia exigida e
// imprevisibilidade, nao reproducao.
[[nodiscard]] std::array<std::uint8_t, kAeadNonceSize> generate_nonce();

// Resultado de aead_lock: cipher_text (mesmo tamanho do plain_text de entrada) +
// tag de 16 bytes (Poly1305).
struct AeadLocked {
    std::vector<std::uint8_t> cipher_text;
    std::array<std::uint8_t, kAeadTagSize> tag{};
};

// Cifra + autentica. `ad` (Additional Authenticated Data) e autenticado mas NAO
// cifrado (fica em claro no envelope, ex.: magic/envelope_ver/slot_id/rollback_ctr
// do GDS3); pode ser vazio (ad=nullptr, ad_size=0).
[[nodiscard]] AeadLocked aead_lock(const std::array<std::uint8_t, kAeadKeySize>& key,
                                   const std::array<std::uint8_t, kAeadNonceSize>& nonce,
                                   const std::uint8_t* ad, std::size_t ad_size,
                                   const std::uint8_t* plain_text,
                                   std::size_t text_size);

// Verifica a tag e decifra em `plain_text_out` (redimensionado para text_size).
// Retorna false (SEM garantir o conteudo de plain_text_out, que fica do tamanho
// certo mas nao deve ser usado) se a tag nao bater (tamper em qualquer um de
// ciphertext/tag/nonce/ad, ou chave errada) - espelha crypto_aead_unlock (0 = ok,
// -1 = falha) do Monocypher, SEM lancar (a camada domain decide o LoadResult).
[[nodiscard]] bool aead_unlock(std::vector<std::uint8_t>& plain_text_out,
                               const std::array<std::uint8_t, kAeadKeySize>& key,
                               const std::array<std::uint8_t, kAeadNonceSize>& nonce,
                               const std::uint8_t* ad, std::size_t ad_size,
                               const std::uint8_t* cipher_text,
                               const std::array<std::uint8_t, kAeadTagSize>& tag,
                               std::size_t text_size);

}  // namespace gus::core::crypto

#endif  // GUS_CORE_CRYPTO_AEAD_XCHACHA20POLY1305_HPP
