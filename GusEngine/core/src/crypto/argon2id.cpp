// gus/core/src/crypto/argon2id.cpp
//
// Implementacao do Argon2id proprio (ADR-015 decisao 3). Ver header para o
// contrato. Delega ao crypto_argon2 do Monocypher vendorizado (third_party/); a
// work area (memoria alocada para o custo memory-hard) e zerada (secure_zero)
// antes de liberar, mesmo criterio de "nao deixar segredo/material intermediario
// vivo na RAM" que o wipe do Hardcore usa (ADR-015 secao 5).

#include "gus/core/crypto/argon2id.hpp"

#include <vector>

#include "gus/core/crypto/secure_zero.hpp"
#include "monocypher.h"

namespace gus::core::crypto {

std::array<std::uint8_t, kArgon2idKeySize> derive_key_argon2id(
    const std::uint8_t* pass, std::size_t pass_size, const std::uint8_t* salt,
    std::size_t salt_size, std::uint32_t memory_kib, std::uint32_t passes,
    std::uint32_t lanes) {
    // Argon2 opera em blocos de 1024 bytes ("Argon2 operates on 1024 byte blocks",
    // monocypher.c): nb_blocks EM UNIDADES DE 1 KiB = memory_kib diretamente.
    std::vector<std::uint8_t> work_area(static_cast<std::size_t>(memory_kib) * 1024);

    crypto_argon2_config config{};
    config.algorithm = CRYPTO_ARGON2_ID;
    config.nb_blocks = memory_kib;
    config.nb_passes = passes;
    config.nb_lanes = lanes;

    crypto_argon2_inputs inputs{};
    inputs.pass = pass;
    inputs.salt = salt;
    inputs.pass_size = static_cast<std::uint32_t>(pass_size);
    inputs.salt_size = static_cast<std::uint32_t>(salt_size);

    std::array<std::uint8_t, kArgon2idKeySize> out{};
    crypto_argon2(out.data(), static_cast<std::uint32_t>(out.size()),
                 work_area.data(), config, inputs, crypto_argon2_no_extras);

    secure_zero(work_area.data(), work_area.size());
    return out;
}

}  // namespace gus::core::crypto
