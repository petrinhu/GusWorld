// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
#include "check.hpp"

#include <iostream>

namespace gusworld::test {

namespace {

int recorded_failures = 0;

} // namespace

// Writes the failure before the caller throws, so the report survives even if
// the unwind is lost somewhere along the way.
void record_check_failure(std::string_view file, int line, std::string_view expr) {
    ++recorded_failures;
    std::cerr << file << ':' << line << ": CHECK failed: " << expr << '\n';
}

int failure_count() { return recorded_failures; }

void reset_failure_count() { recorded_failures = 0; }

} // namespace gusworld::test
