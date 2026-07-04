// gus/core/src/anim/glitch_dissolve.cpp
//
// Implementacao do POCO glitch_*. Ver header. Travado por
// GusEngine/tests/glitch_dissolve_test.cpp (TEST-FIRST).

#include "gus/core/anim/glitch_dissolve.hpp"

namespace gus::core::anim {

namespace {

// Hash inteiro simples (splitmix-like, sem <random>/estado) que mistura (col,row)
// + um "salt" pra produzir 2 sequencias INDEPENDENTES a partir das mesmas
// coordenadas (o limiar de acender e o deslocamento nao devem correlacionar - ver
// comentario do header). Puramente aritmetico, sem UB (unsigned wraparound e
// definido).
[[nodiscard]] unsigned int mix_hash(int col, int row, unsigned int salt) noexcept {
    unsigned int h = static_cast<unsigned int>(col) * 0x9E3779B1u;
    h ^= static_cast<unsigned int>(row) * 0x85EBCA6Bu;
    h ^= salt;
    h ^= h >> 15;
    h *= 0x2C1B3C6Du;
    h ^= h >> 12;
    h *= 0x297A2D39u;
    h ^= h >> 15;
    return h;
}

}  // namespace

float glitch_cell_threshold(int col, int row) noexcept {
    constexpr unsigned int kThresholdSalt = 0xA5A5A5A5u;
    constexpr unsigned int kBucket = 100000u;
    const unsigned int h = mix_hash(col, row, kThresholdSalt);
    return static_cast<float>(h % kBucket) / static_cast<float>(kBucket);
}

float glitch_block_alpha(int col, int row, float alpha) noexcept {
    if (alpha <= 0.0f) {
        return 0.0f;
    }
    if (alpha >= 1.0f) {
        return 1.0f;
    }
    return alpha >= glitch_cell_threshold(col, row) ? 1.0f : 0.0f;
}

bool glitch_cell_is_wavefront(int col, int row, float alpha, float band) noexcept {
    if (alpha <= 0.0f || alpha >= 1.0f || band <= 0.0f) {
        return false;
    }
    const float threshold = glitch_cell_threshold(col, row);
    return alpha >= threshold && (alpha - threshold) <= band;
}

float glitch_cell_shift_fraction(int col, int row) noexcept {
    constexpr unsigned int kShiftSalt = 0x5F356495u;
    constexpr unsigned int kBucket = 200001u;
    const unsigned int h = mix_hash(col, row, kShiftSalt);
    // Mapeia [0,kBucket) -> [0,2) -> [-1,1].
    return (static_cast<float>(h % kBucket) / static_cast<float>(kBucket) * 2.0f) -
           1.0f;
}

}  // namespace gus::core::anim
