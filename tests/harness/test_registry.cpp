// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
#include "test_registry.hpp"

namespace gusworld::test {

namespace {

// Function-local static: built on first use, so a registrar running during
// static initialization of another translation unit never sees a vector that
// is not constructed yet.
std::vector<test_case>& registry() {
    static std::vector<test_case> cases;
    return cases;
}

} // namespace

void register_case(test_case c) { registry().push_back(c); }

const std::vector<test_case>& all_cases() { return registry(); }

case_registrar::case_registrar(std::string_view name, case_fn fn) {
    register_case(test_case{name, fn});
}

} // namespace gusworld::test
