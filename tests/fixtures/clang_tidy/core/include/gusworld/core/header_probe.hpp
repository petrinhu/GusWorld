// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// DELIBERATE DEFECT. This header exists only to prove that the clang-tidy gate sees
// headers under <layer>/include/ (GW-19 portao 3). It sits under a path that matches the
// HeaderFilterRegex of .clang-tidy, and it is reached only through the fixture that
// includes it. Do not fix the defect below: without it the proof has nothing to catch.

#ifndef GUSWORLD_FIXTURE_CLANG_TIDY_HEADER_PROBE_HPP
#define GUSWORLD_FIXTURE_CLANG_TIDY_HEADER_PROBE_HPP

#define DELIBERATE_HEADER_SCALE(value) value * 2

// Deliberate: `value` is not enclosed in parentheses, so DELIBERATE_HEADER_SCALE(x + 1)
// expands to x + 1 * 2. clang-tidy reports this as bugprone-macro-parentheses.
inline int deliberate_header_probe(int x) { return DELIBERATE_HEADER_SCALE(x + 1); }

#endif // GUSWORLD_FIXTURE_CLANG_TIDY_HEADER_PROBE_HPP
