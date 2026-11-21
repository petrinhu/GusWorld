// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Red proof for headers (GW-19 portao 3, clang-tidy job). It includes a header with a
// deliberate defect, so the gate must reach that header through the include and report it.
// Not a CMake target; no build compiles it. Scanned by path with its own flags, like
// clang_tidy_fixture.cpp.

#include "core/include/gusworld/core/header_probe.hpp"

int clang_tidy_header_fixture_entry() { return deliberate_header_probe(1); }
