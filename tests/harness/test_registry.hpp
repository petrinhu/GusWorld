// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <string_view>
#include <vector>

// test_registry.hpp - the case registry of the in-house test harness (ADR-025).
//
// GUSWORLD_TEST(name) declares a case and registers it through a static object
// whose constructor runs before main(). A new test file needs no list edited.

namespace gusworld::test {

using case_fn = void (*)();

struct test_case {
    std::string_view name;
    case_fn fn;
};

void register_case(test_case c);
[[nodiscard]] const std::vector<test_case>& all_cases();

struct case_registrar {
    explicit case_registrar(std::string_view name, case_fn fn);
};

} // namespace gusworld::test

#define GUSWORLD_TEST(name)                                                                        \
    static void gusworld_test_fn_##name();                                                         \
    namespace {                                                                                    \
    const ::gusworld::test::case_registrar gusworld_test_reg_##name{#name,                         \
                                                                    &gusworld_test_fn_##name};     \
    }                                                                                              \
    static void gusworld_test_fn_##name()
