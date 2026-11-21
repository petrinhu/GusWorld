// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <gusworld/core/error_code.hpp>

#include <source_location>

namespace gusworld::core {

// A failure value: what went wrong, and the line that raised it. Equality reads
// only the code. The origin changes at every call site, so comparing it would
// make two identical failures unequal and tests fragile.
struct error {
    // The default argument is evaluated at the caller, so `where` names the
    // line that built the error, not a line in this header.
    explicit error(error_code c, std::source_location at = std::source_location::current()) noexcept
        : code{c}, where{at} {}

    error_code code;
    std::source_location where;

    friend bool operator==(const error &lhs, const error &rhs) noexcept {
        return lhs.code == rhs.code;
    }
};

} // namespace gusworld::core
