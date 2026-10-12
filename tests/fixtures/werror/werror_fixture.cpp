// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// DELIBERATE WARNINGS. This file exists only to prove the warnings-as-errors
// gate (GUSWORLD_WERROR, L-19 portao 1). Do not fix the two warnings below:
// without them the proof has nothing to catch.
#include <cstddef>

namespace {

// Deliberate: the parameter `unused` is never read (-Wunused-parameter, from -Wextra).
int unused_parameter_probe(int used, int unused) {
  return used;
}

// Deliberate: signed against unsigned comparison (-Wsign-compare, from -Wextra).
bool sign_compare_probe(int value, std::size_t size) {
  return value < size;
}

}  // namespace

int werror_fixture_entry() {
  return unused_parameter_probe(1, 2) + (sign_compare_probe(1, 2) ? 1 : 0);
}
