// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <string_view>

// check.hpp - the assertion of the in-house test harness (ADR-025).
//
// GUSWORLD_CHECK records the failure first, then throws case_check_failed.
// The throw unwinds only the test case that failed; the runner catches it
// per case, so one failing case never stops the rest of the suite.

namespace gusworld::test {

// Deliberately not derived from std::exception: a catch (const std::exception&)
// elsewhere in a test must not swallow it and hide the failure it reports.
struct case_check_failed {};

void record_check_failure(std::string_view file, int line, std::string_view expr);
[[nodiscard]] int failure_count();
void reset_failure_count();

} // namespace gusworld::test

#define GUSWORLD_CHECK(cond)                                                                       \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            ::gusworld::test::record_check_failure(__FILE__, __LINE__, #cond);                     \
            throw ::gusworld::test::case_check_failed{};                                           \
        }                                                                                          \
    } while (false)
