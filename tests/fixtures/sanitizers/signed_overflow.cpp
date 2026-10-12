// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// DELIBERATE DEFECT. This file exists only to prove the UndefinedBehaviorSanitizer
// gate (sanitizers job, L-19 portao 2). Do not fix the overflow below: without it
// the proof has nothing to catch.
#include <cstdio>
#include <limits>

namespace {

// argc is 1 under ctest, so the sum leaves the range of int. The value is not a
// constant, so static analysis cannot flag the overflow before the sanitizer runs.
int add_argc_to_int_max(int argc) {
  int value = std::numeric_limits<int>::max();
  value += argc;  // Deliberate: signed integer overflow.
  return value;
}

}  // namespace

int main(int argc, char** argv) {
  (void)argv;
  std::printf("%d\n", add_argc_to_int_max(argc));
  return 0;
}
