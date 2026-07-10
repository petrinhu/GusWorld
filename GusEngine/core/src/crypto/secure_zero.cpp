// gus/core/src/crypto/secure_zero.cpp
//
// Implementacao da zeragem segura (ADR-015 secao 5). Ver header para o contrato.
// Delega inteiramente ao crypto_wipe do Monocypher vendorizado (third_party/).

#include "gus/core/crypto/secure_zero.hpp"

#include "monocypher.h"

namespace gus::core::crypto {

void secure_zero(void* data, std::size_t size) noexcept { crypto_wipe(data, size); }

}  // namespace gus::core::crypto
