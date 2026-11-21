// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <gusworld/core/error_code.hpp>

namespace gusworld::core {

// No default label on purpose: with every enumerator listed, -Wswitch flags a
// new value that forgets its name. The fallback below only serves out-of-range
// values, which the switch cannot match.
std::string_view error_code_name(error_code code) noexcept {
    switch (code) {
    case error_code::invalid_id:
        return "invalid_id";
    }
    return "unknown_error_code";
}

} // namespace gusworld::core
