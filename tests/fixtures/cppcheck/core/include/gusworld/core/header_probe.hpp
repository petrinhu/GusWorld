// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// DELIBERATE DEFECT. This header exists only to prove that the cppcheck gate reaches a
// header under <layer>/include/ (GW-19 portao 3, cppcheck job). It is passed alone to
// scan.sh with, and the list self-check in enumerate.sh covers the enumeration. It is not
// a CMake target. Do not fix the defect below: without it the proof has nothing to catch.

#ifndef GUSWORLD_FIXTURE_CPPCHECK_HEADER_PROBE_HPP
#define GUSWORLD_FIXTURE_CPPCHECK_HEADER_PROBE_HPP

// Deliberate: `value` is read before any assignment. cppcheck reports this as uninitvar,
// an error-severity finding that is on by default.
inline int uninitialized_header_probe() {
    int value;
    return value;
}

#endif // GUSWORLD_FIXTURE_CPPCHECK_HEADER_PROBE_HPP
