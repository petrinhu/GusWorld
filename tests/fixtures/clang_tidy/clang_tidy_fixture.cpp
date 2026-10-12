// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// DELIBERATE DEFECT. This file exists only to prove the clang-tidy gate (L-19 portao 3,
// clang-tidy job). It is not a CMake target and no build compiles it: the gate scans it
// by path, with its own flags and without the compile database. Do not fix the defect
// below: without it the proof has nothing to catch.

#define DELIBERATE_SCALE(value) value * 2

namespace {

// Deliberate: the macro argument `value` is not enclosed in parentheses, so the
// expansion of DELIBERATE_SCALE(x + 1) is x + 1 * 2. clang-tidy reports this as
// bugprone-macro-parentheses, which is enabled in .clang-tidy.
int deliberate_macro_probe(int x) {
  return DELIBERATE_SCALE(x + 1);
}

}  // namespace

int clang_tidy_fixture_entry() {
  return deliberate_macro_probe(1);
}
