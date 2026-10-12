// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// DELIBERATE DEFECT. This file exists only to prove the AddressSanitizer gate
// (sanitizers job, L-19 portao 2). Do not fix the out-of-bounds write below:
// without it the proof has nothing to catch.
#include <cstddef>
#include <cstdio>

namespace {

// The size comes from argc, a value the compiler cannot see, so static analysis
// cannot flag the write before the sanitizer gets to run.
int write_one_past_the_end(int argc) {
  const auto count = static_cast<std::size_t>(argc);
  int* block = new int[count];
  block[count] = 7;  // Deliberate: one element past the end of the heap block.
  const int first = block[0];
  delete[] block;
  return first;
}

}  // namespace

int main(int argc, char** argv) {
  (void)argv;
  std::printf("%d\n", write_one_past_the_end(argc));
  return 0;
}
