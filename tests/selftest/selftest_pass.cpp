// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
#include "check.hpp"
#include "test_registry.hpp"

namespace {

// Read at run time, so the checks below are not constant expressions. A
// constant condition would trip MSVC warning C4127 under /W4 /WX.
volatile int selftest_two = 2;

} // namespace

// Self-tests of the harness itself, not game logic. Both must pass: they prove
// the green path that the CTest gate depends on.

GUSWORLD_TEST(selftest_pass_check) { GUSWORLD_CHECK(selftest_two + 2 == 4); }

GUSWORLD_TEST(selftest_pass_second) { GUSWORLD_CHECK(selftest_two * 2 == 4); }
