// gus/domain/src/input/controls_hash.cpp
//
// Hash 128 dos controles (ADR-007 item 2). POCO puro, ZERO Qt. Trunca o SHA-256
// (core/crypto) da forma canonica do controls.json. Ver header. Travado por
// tests/controls_hash_test.cpp.

#include "gus/domain/input/controls_hash.hpp"

#include <string>
#include <vector>

#include "gus/core/crypto/sha256.hpp"
#include "gus/domain/input/controls_json.hpp"

namespace gus::domain::input {

std::array<std::uint8_t, kControlsHashSize> controls_hash128(
    const InputRemapConfig& cfg) {
    // Forma CANONICA (compacta, ordem do ActionRegistry): o hash NAO depende de
    // formatacao cosmetica do arquivo de disco.
    const std::string canonical = serialize_controls_canonical(cfg);
    const auto full = gus::core::crypto::sha256(
        std::vector<std::uint8_t>(canonical.begin(), canonical.end()));

    std::array<std::uint8_t, kControlsHashSize> hash{};
    for (std::size_t i = 0; i < kControlsHashSize; ++i) hash[i] = full[i];
    return hash;
}

bool controls_were_modified(
    const std::array<std::uint8_t, kControlsHashSize>& current,
    const std::array<std::uint8_t, kControlsHashSize>& saved) {
    return current != saved;  // comparacao trivial; nao e seguranca (sem timing-safety)
}

}  // namespace gus::domain::input
