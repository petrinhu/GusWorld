// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

namespace gusworld::test {

// No argument runs every case. --list prints the case names. --case NAME runs
// one case by exact name. Returns 0 when everything passed, 1 when a case
// failed, and 2 on a usage error, an unknown name, or an empty registry.
int run_main(int argc, char** argv);

} // namespace gusworld::test
