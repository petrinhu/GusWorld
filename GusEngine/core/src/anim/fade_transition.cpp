// gus/core/src/anim/fade_transition.cpp
//
// Implementacao do POCO fade_overlay_alpha. Ver header. Travado por
// GusEngine/tests/fade_transition_test.cpp (TEST-FIRST).

#include "gus/core/anim/fade_transition.hpp"

namespace gus::core::anim {

float fade_overlay_alpha(FadeDirection direction, float elapsed_seconds,
                          float duration_seconds) noexcept {
    // duration_seconds<=0: transicao "instantanea" - devolve o estado FINAL da fase
    // direto, sem dividir por zero (t seria indefinido).
    if (duration_seconds <= 0.0f) {
        return direction == FadeDirection::kOut ? 1.0f : 0.0f;
    }

    float t = elapsed_seconds / duration_seconds;
    if (t < 0.0f) {
        t = 0.0f;
    } else if (t > 1.0f) {
        t = 1.0f;
    }

    // kOut: 0 -> 1 (escurecendo). kIn: 1 -> 0 (clareando) - espelho de kOut.
    return direction == FadeDirection::kOut ? t : (1.0f - t);
}

}  // namespace gus::core::anim
