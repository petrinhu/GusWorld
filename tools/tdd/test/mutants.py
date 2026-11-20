#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Mutants of the TDD guard's notice and settings (ADR-026, C8 fatia 6, L-19 N2). Standard
# library only, and pure text work: it edits a COPY of the tree (made by mutate_guard.sh
# from the committed HEAD), never the live tree. Each mutant must match exactly once, and
# the diff must show the intended change, so a mutant that did not arrive is refused.
# M8 (undo a fix of R2) is not applicable: R2 has no fix yet.
#
# usage:
#   mutants.py apply <tree_root> <id>        exit 1 when the pattern does not match once
#   mutants.py compare <baseline.log> <mutant.log> <suites>
#                                            exit 0 when killed, 1 alive, 2 inconclusive
import difflib
import sys
from pathlib import Path

NOTICE = "tools/tdd/notice.py"
SETTINGS = ".claude/settings.json"

# id -> (file relative to the tree, text that must occur once, its replacement)
MUTANTS = {
    "M1": (NOTICE, "if code == GUARD_BLOCK or not msg:", "if code != GUARD_BLOCK or not msg:"),
    "M2": (NOTICE, '            "additionalContext": text,\n', ""),
    "M3": (NOTICE, '        "systemMessage": text,\n', ""),
    "M4": (NOTICE, '"hookEventName": "PreToolUse"', '"hookEventName": "PostToolUse"'),
    "M5": (SETTINGS, ' || true",', '",'),
    "M6": (SETTINGS, '"matcher": "Write|Edit|MultiEdit"', '"matcher": "Write|Edit"'),
    "M7": (NOTICE, "return mtime >= ts", "return mtime < ts"),
}


def apply(root, mid):
    rel, old, new = MUTANTS[mid]
    path = Path(root) / rel
    text = path.read_text(encoding="utf-8")
    hits = text.count(old)
    if hits != 1:
        print(f"mutante {mid} recusado: padrao ocorre {hits} vez(es) em {rel}")
        return 1
    mutated = text.replace(old, new, 1)
    diff = [line for line in difflib.unified_diff(text.splitlines(), mutated.splitlines(), lineterm="", n=0)
            if line[:1] in "+-" and line[:3] not in ("+++", "---")]
    removed = sum(1 for line in diff if line.startswith("-"))
    added = sum(1 for line in diff if line.startswith("+"))
    if removed != 1 or added > 1:
        print(f"mutante {mid} recusado: diff de -{removed} +{added}, esperado uma linha trocada")
        return 1
    path.write_text(mutated, encoding="utf-8")
    print(f"mutante {mid} em {rel}: -{removed} +{added}")
    return 0


def names(log, prefix):
    """The case names of one line kind: 'ok    name' or 'FALHA name: detail'."""
    found = set()
    for line in Path(log).read_text(encoding="utf-8").splitlines():
        if line.startswith(prefix):
            found.add(line[len(prefix):].split(": ", 1)[0].strip())
    return found


def compare(baseline, mutant, suites):
    text = Path(mutant).read_text(encoding="utf-8")
    if text.count("\ncasos=") + text.startswith("casos=") != suites:
        print(f"inconclusivo: {suites} suites esperadas, sem todas as linhas casos=")
        return 2
    base_fail = names(baseline, "FALHA ")
    base_ok = names(baseline, "ok    ")
    mut_fail = names(mutant, "FALHA ")
    mut_ok = names(mutant, "ok    ")
    new_fail = mut_fail - base_fail
    vanished = base_ok - mut_ok - mut_fail
    killed = sorted(new_fail | vanished)
    if killed:
        print("MORTO por: " + " | ".join(killed))
        return 0
    print("VIVO: nenhum caso que passou na linha de base falhou no mutante")
    return 1


def main(argv):
    if len(argv) == 3 and argv[0] == "apply":
        if argv[2] not in MUTANTS:
            print(f"mutante desconhecido: {argv[2]}")
            return 1
        return apply(argv[1], argv[2])
    if len(argv) == 4 and argv[0] == "compare":
        return compare(argv[1], argv[2], int(argv[3]))
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
