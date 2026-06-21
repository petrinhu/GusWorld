// gus/core/math_util.cpp
#include "gus/core/math_util.hpp"

namespace gus::core {

int clamp(int value, int lo, int hi) noexcept {
    if (value < lo) {
        return lo;
    }
    if (value > hi) {
        return hi;
    }
    return value;
}

}  // namespace gus::core
