// gus/core/src/crypto/key_derivation.cpp
//
// Implementacao do KDF simples (ADR-006 T2.2). Ver header para o contrato. Dominio
// publico, deterministico, ZERO dependencia externa, ZERO Qt. Travado por
// tests/key_derivation_test.cpp (determinismo, separacao de dominio, vetor
// congelado = SHA-256(base || 0x00 || context)).

#include "gus/core/crypto/key_derivation.hpp"

#include "gus/core/crypto/sha256.hpp"

namespace gus::core::crypto {

std::vector<std::uint8_t> derive_key(const std::uint8_t* base, std::size_t base_size,
                                     const std::uint8_t* context,
                                     std::size_t context_size) {
    // Preimage = base || 0x00 || context. O separador 0x00 da separacao de dominio
    // sem colisao de fronteira (ver header).
    std::vector<std::uint8_t> preimage;
    preimage.reserve(base_size + 1 + context_size);
    preimage.insert(preimage.end(), base, base + base_size);
    preimage.push_back(0x00);
    preimage.insert(preimage.end(), context, context + context_size);

    const auto digest = sha256(preimage);
    return std::vector<std::uint8_t>(digest.begin(), digest.end());
}

}  // namespace gus::core::crypto
