// gus/platform/src/render2d/text_metrics.cpp
//
// Implementacao das metricas de texto puras (ver header). Aritmetica monospace, sem
// SDL nem stb_truetype.

#include "gus/platform/render2d/text_metrics.hpp"

namespace gus::platform::render2d {

int text_char_count(std::string_view text) noexcept {
    return static_cast<int>(text.size());
}

float text_width(std::string_view text, float px_size) noexcept {
    if (px_size <= 0.0f) {
        return 0.0f;
    }
    return static_cast<float>(text_char_count(text)) * kMonoAdvanceRatio * px_size;
}

float text_height(float px_size) noexcept {
    return px_size > 0.0f ? px_size : 0.0f;
}

float glyph_advance(float px_size) noexcept {
    return px_size > 0.0f ? kMonoAdvanceRatio * px_size : 0.0f;
}

}  // namespace gus::platform::render2d
