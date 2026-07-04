// gus/core/src/anim/boot_pixel_sequence.cpp
//
// Implementacao dos POCO boot_pixel_frame_index/boot_pixel_safety_alpha. Ver header.
// Travado por GusEngine/tests/boot_pixel_sequence_test.cpp (TEST-FIRST).

#include "gus/core/anim/boot_pixel_sequence.hpp"

namespace gus::core::anim {

namespace {

[[nodiscard]] float clamp01(float v) noexcept {
    if (v < 0.0f) {
        return 0.0f;
    }
    if (v > 1.0f) {
        return 1.0f;
    }
    return v;
}

[[nodiscard]] int round_to_int(float v) noexcept {
    return static_cast<int>(v + 0.5f);
}

}  // namespace

int boot_pixel_frame_index(BootPixelLeg leg, float t, int frame_count) noexcept {
    if (frame_count <= 0) {
        return 0;  // "asset vazio" - resposta bem definida, sem divisao por zero.
    }
    if (frame_count == 1) {
        return 0;  // 1 frame so: sempre ele, qualquer t/leg.
    }

    const float ct = clamp01(t);
    const int half = frame_count / 2;               // 1a metade: [0, half-1]
    const int second_half_size = frame_count - half; // 2a metade: [half, frame_count-1]
    const int last = frame_count - 1;

    int idx = 0;
    switch (leg) {
        case BootPixelLeg::kToBattleDarkening:
            // [0, half-1] ASCENDENTE.
            idx = round_to_int(ct * static_cast<float>(half - 1));
            break;
        case BootPixelLeg::kToBattleRevealing:
            // [half, last] ASCENDENTE - termina no ULTIMO frame.
            idx = half + round_to_int(ct * static_cast<float>(second_half_size - 1));
            break;
        case BootPixelLeg::kFromBattleDarkening:
            // [last, half] DESCENDENTE - espelha kToBattleRevealing.
            idx = last - round_to_int(ct * static_cast<float>(second_half_size - 1));
            break;
        case BootPixelLeg::kFromBattleRevealing:
            // [half-1, 0] DESCENDENTE - espelha kToBattleDarkening, termina no 1o frame.
            idx = (half - 1) - round_to_int(ct * static_cast<float>(half - 1));
            break;
    }

    if (idx < 0) {
        idx = 0;
    } else if (idx > last) {
        idx = last;
    }
    return idx;
}

float boot_pixel_safety_alpha(BootPixelLeg leg, float t) noexcept {
    const float ct = clamp01(t);
    switch (leg) {
        case BootPixelLeg::kToBattleDarkening:
        case BootPixelLeg::kFromBattleDarkening:
            return ct;  // escurecendo: cresce com t (igual fade_overlay_alpha kOut).
        case BootPixelLeg::kToBattleRevealing:
        case BootPixelLeg::kFromBattleRevealing:
            return 1.0f - ct;  // revelando: decresce com t (igual fade_overlay_alpha kIn).
    }
    return 0.0f;  // inalcancavel (enum exaustivo acima) - guard defensivo pro compilador.
}

}  // namespace gus::core::anim
