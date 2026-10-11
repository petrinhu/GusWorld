// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
#include "check.hpp"
#include "test_registry.hpp"

#include <stdexcept>

namespace {

// Read at run time, so the check below is not a constant expression (MSVC C4127).
volatile int selftest_two = 2;

} // namespace

// Self-tests that FAIL on purpose. This file lives in its own executable,
// gusworld_harness_selftest_red, which CTest runs through expect_exit.cmake:
// that script demands exit code 1 and these exact verdict lines. The red
// executable is therefore never a red build, but the harness is still proven
// to report every failure kind.

// A failed CHECK must be reported and must not stop the next case.
GUSWORLD_TEST(selftest_red_check) { GUSWORLD_CHECK(selftest_two == 1); }

// A std::exception escaping a case must become a FAIL of that case only.
GUSWORLD_TEST(selftest_red_std_exception) { throw std::runtime_error{"selftest"}; }

// An exception of a non-std type must also become a FAIL of that case only.
GUSWORLD_TEST(selftest_red_foreign_throw) { throw 42; }

// Passes after three failures, proving the suite keeps going past each one.
GUSWORLD_TEST(selftest_red_then_pass) {}
