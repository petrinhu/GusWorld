// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <cstdint>
#include <string_view>

namespace gusworld::core {

// The underlying type and the values are part of the save format (ADR-027), so
// a value is never renumbered. Value 0 has no name on purpose: a zeroed error
// must not look like a known code.
enum class error_code : std::uint8_t {
    invalid_id = 1,
};

// Stable name for logs and diagnostics. A value outside the enum gets
// "unknown_error_code" instead of undefined behaviour.
[[nodiscard]] std::string_view error_code_name(error_code code) noexcept;

} // namespace gusworld::core
