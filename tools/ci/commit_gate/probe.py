# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Proves that the mutated copy is the module the suite loads, by importing it and calling
# normalize() once. Single responsibility: the import probe. It does not run the suite.
# Usage: python3 -I probe.py <copy-of-tools-security-dir>
import sys

dir_copy = sys.argv[1]
sys.path.insert(0, dir_copy)
import commit_gate  # noqa: E402  (import after sys.path, deliberate)

print("module file:", commit_gate.__file__)
out = commit_gate.normalize("AINDA Compila")
print("normalize('AINDA Compila') =", repr(out))
if not commit_gate.__file__.startswith(dir_copy):
    sys.exit("the import did not resolve to the mutated copy")
if out != "AINDA Compila":
    sys.exit("the mutation is not in the loaded module")
print("the mutated copy is the module the suite loads")
