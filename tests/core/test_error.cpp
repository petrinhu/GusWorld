// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <gusworld/core/error.hpp>

#include "check.hpp"
#include "test_registry.hpp"

#include <cstdint>
#include <type_traits>

using gusworld::core::error;
using gusworld::core::error_code;

// A failure is passed by value through every layer, so a throwing copy would
// turn an error return into an exception path.
static_assert(std::is_nothrow_copy_constructible_v<error>);

// The two errors are built on different lines on purpose: if equality read the
// origin, these would differ and the case would fail.
GUSWORLD_TEST(error_equality_ignores_origin) {
    const error first{error_code::invalid_id};
    const error second{error_code::invalid_id};
    GUSWORLD_CHECK(first == second);
}

GUSWORLD_TEST(error_exposes_its_code) {
    const error e{error_code::invalid_id};
    GUSWORLD_CHECK(e.code == error_code::invalid_id);
}

// The default argument must capture the call site, not a line inside the header.
GUSWORLD_TEST(error_where_is_the_construction_line) {
    const error e{error_code::invalid_id};
    GUSWORLD_CHECK(e.where.line() == static_cast<std::uint_least32_t>(__LINE__ - 1));
}
