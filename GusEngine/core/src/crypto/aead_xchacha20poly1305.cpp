// gus/core/src/crypto/aead_xchacha20poly1305.cpp
//
// Implementacao do AEAD XChaCha20-Poly1305 proprio (ADR-015 decisao 1/2). Ver
// header para o contrato. Delega ao crypto_aead_lock/crypto_aead_unlock do
// Monocypher vendorizado (third_party/).

#include "gus/core/crypto/aead_xchacha20poly1305.hpp"

#include <random>

#include "monocypher.h"

namespace gus::core::crypto {

std::array<std::uint8_t, kAeadNonceSize> generate_nonce() {
    std::array<std::uint8_t, kAeadNonceSize> nonce{};
    // std::random_device: fonte de entropia do SO (CSPRNG), NAO um PRNG de
    // gameplay seedado. Preenche byte a byte a partir das palavras de 32 bits que
    // o gerador devolve, sem assumir que kAeadNonceSize e multiplo exato do
    // tamanho da palavra (portavel entre implementacoes da stdlib).
    std::random_device rd;
    std::size_t filled = 0;
    while (filled < nonce.size()) {
        const std::random_device::result_type word = rd();
        const auto* bytes = reinterpret_cast<const std::uint8_t*>(&word);
        for (std::size_t i = 0; i < sizeof(word) && filled < nonce.size(); ++i, ++filled)
            nonce[filled] = bytes[i];
    }
    return nonce;
}

AeadLocked aead_lock(const std::array<std::uint8_t, kAeadKeySize>& key,
                     const std::array<std::uint8_t, kAeadNonceSize>& nonce,
                     const std::uint8_t* ad, std::size_t ad_size,
                     const std::uint8_t* plain_text, std::size_t text_size) {
    AeadLocked out;
    out.cipher_text.resize(text_size);
    crypto_aead_lock(out.cipher_text.data(), out.tag.data(), key.data(), nonce.data(),
                     ad, ad_size, plain_text, text_size);
    return out;
}

bool aead_unlock(std::vector<std::uint8_t>& plain_text_out,
                 const std::array<std::uint8_t, kAeadKeySize>& key,
                 const std::array<std::uint8_t, kAeadNonceSize>& nonce,
                 const std::uint8_t* ad, std::size_t ad_size,
                 const std::uint8_t* cipher_text,
                 const std::array<std::uint8_t, kAeadTagSize>& tag,
                 std::size_t text_size) {
    plain_text_out.resize(text_size);
    const int rc = crypto_aead_unlock(plain_text_out.data(), tag.data(), key.data(),
                                      nonce.data(), ad, ad_size, cipher_text,
                                      text_size);
    return rc == 0;
}

}  // namespace gus::core::crypto
