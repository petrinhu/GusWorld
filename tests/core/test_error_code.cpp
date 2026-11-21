// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
#include <gusworld/core/error_code.hpp>

#include "check.hpp"
#include "test_registry.hpp"

#include <cstdint>
#include <string_view>
#include <type_traits>
#include <utility>

// The underlying type and the numeric value are part of the save format
// (ADR-027): a change to either breaks every save written before it.
static_assert(std::is_same_v<std::underlying_type_t<gusworld::core::error_code>, std::uint8_t>);
static_assert(std::to_underlying(gusworld::core::error_code::invalid_id) == 1);

// Logging paths call the name function on the way to a diagnostic, and must not
// be able to throw there.
static_assert(noexcept(gusworld::core::error_code_name(gusworld::core::error_code::invalid_id)));

GUSWORLD_TEST(error_code_name_of_invalid_id) {
    GUSWORLD_CHECK(gusworld::core::error_code_name(gusworld::core::error_code::invalid_id) ==
                   std::string_view{"invalid_id"});
}

// Out of range must not reuse a valid name. Together with the case above, this
// rules out a name function that answers the same string for every value.
GUSWORLD_TEST(error_code_name_out_of_range_is_unknown) {
    GUSWORLD_CHECK(gusworld::core::error_code_name(static_cast<gusworld::core::error_code>(255)) ==
                   std::string_view{"unknown_error_code"});
}

// The zero value is deliberately unnamed (see error_code.hpp): a zeroed error
// must not pass for a known code.
GUSWORLD_TEST(error_code_name_zero_is_unknown) {
    GUSWORLD_CHECK(gusworld::core::error_code_name(static_cast<gusworld::core::error_code>(0)) ==
                   std::string_view{"unknown_error_code"});
}
