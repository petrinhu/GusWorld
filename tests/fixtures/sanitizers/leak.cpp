// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// DELIBERATE DEFECT. This file exists only to prove the LeakSanitizer gate
// (sanitizers job, L-19 portao 2). Do not add a delete below: without the leak
// the proof has nothing to catch.
#include <cstddef>
#include <cstdio>

namespace {

// The block is never freed, and the pointer does not outlive this function: no
// global or static holds it, so LeakSanitizer finds it unreachable at exit.
int allocate_and_forget(int argc) {
  const auto count = static_cast<std::size_t>(16 * argc);
  int* block = new int[count];
  block[0] = argc;
  return block[0];  // Deliberate: the block is dropped and never deleted.
}

}  // namespace

int main(int argc, char** argv) {
  (void)argv;
  std::printf("%d\n", allocate_and_forget(argc));
  return 0;
}
