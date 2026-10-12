// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// DELIBERATE DEFECT. This file exists only to prove the static analysis gate
// (cppcheck job, L-19 portao 3). It is not a CMake target and no build compiles
// it: the gate scans it by path. Do not fix the defect below: without it the proof
// has nothing to catch.

namespace {

// Deliberate: `value` is read before any assignment. cppcheck reports this as
// uninitvar, an error-severity finding that is on by default.
int uninitialized_read_probe() {
  int value;
  return value;
}

}  // namespace

int cppcheck_fixture_entry() {
  return uninitialized_read_probe();
}
