# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Mutates one file: the scratch copy of commit_gate.py, never the tracked file. It removes
# the case folding from normalize(), then proves that the mutation applied. Single
# responsibility: the mutation, with its anchor and its counts. It runs no test.
# Usage: python3 -I mutate.py <path-of-the-copy>
import sys

if len(sys.argv) != 2:
    sys.exit("usage: mutate.py <path-of-the-copy>")
path = sys.argv[1]


def count_casefold_lines(text):
    # Counts the lines that carry casefold(), the same count as grep -c.
    return sum(1 for line in text.splitlines() if "casefold()" in line)


with open(path, encoding="utf-8") as fh:
    text = fh.read()
before = count_casefold_lines(text)
print("casefold() occurrences before the mutation:", before)
if before != 1:
    sys.exit("mutation anchor is not unique: refusing to mutate")
anchor = "sem_acento = strip_accents(texto).casefold()"
found = text.count(anchor)
if found != 1:
    sys.exit(f"anchor found {found} times, expected exactly 1")
with open(path, "w", encoding="utf-8") as fh:
    fh.write(text.replace(anchor, "sem_acento = strip_accents(texto)", 1))
print("mutation applied to the copy:", path)
# The count after the mutation reads the file again from disk, not the text in memory.
with open(path, encoding="utf-8") as fh:
    after = count_casefold_lines(fh.read())
print("casefold() occurrences after the mutation:", after)
if after != 0:
    sys.exit("mutation did not apply: refusing to run the red proof")
