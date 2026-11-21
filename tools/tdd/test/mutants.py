#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Mutants of the TDD guard's notice and settings (ADR-026, C8 fatia 6, GW-19 N2). Standard
# library only, and pure text work: it edits a COPY of the tree (made by mutate_guard.sh
# from the committed HEAD), never the live tree. Each mutant must match exactly once, and
# the diff must show the intended change, so a mutant that did not arrive is refused.
# M8 undoes the two halves of the R2 fix (see MULTI): it is a two-line change, and each
# half alone is either equivalent or does not restore the defect.
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
    "M7": (NOTICE, "return mtime >= ts - FRESH_TOLERANCE", "return mtime < ts - FRESH_TOLERANCE"),
}

# Virada mutants (C8 fatia 7). Each changes one line of the guard config or of run_suite.sh,
# and each must be killed by test_virada_config.sh or test_virada_chain.sh.
CONFIG = ".claude/tdd-guard.json"
SUITE = "tools/tdd/run_suite.sh"
VIRADA = {
    "V1": (CONFIG, '"test_command": "bash tools/tdd/run_suite.sh"', '"test_command": "bash tools/tdd/run_suit.sh"'),
    "V2": (CONFIG, '"timeout_sec": 144', '"timeout_sec": 200'),
    "V3": (SUITE, '    record "sem execucao: $1"', '    true'),
    "V4": (CONFIG, '    "tests/**",\n    "tests/**/CMakeLists.txt",\n', '    "tests/**/CMakeLists.txt",\n'),
    "V5": (CONFIG, '"timeout_sec": 144', '"timeout_sec": 180'),
}
MUTANTS.update(VIRADA)

# id -> (file, [(text that must occur once, its replacement), ...]). Each pair is one whole
# line, so the diff must show one removed line per pair. M8 puts back the R2 defect: the
# no-state rule of fresh_enough is dropped, and an unreadable state falls back to the
# last-reason again, instead of the guard's I/O message.
MULTI = {
    "M8": (NOTICE, [
        ("    if not isinstance(state, dict):", "    if False:"),
        ('        return clean_text(msg).rstrip(".")[:MAX_MOTIVE] or UNKNOWN', "        state = None"),
    ]),
}


def apply(root, mid):
    if mid in MULTI:
        rel, pairs = MULTI[mid]
    else:
        rel, old, new = MUTANTS[mid]
        pairs = [(old, new)]
    path = Path(root) / rel
    text = path.read_text(encoding="utf-8")
    mutated = text
    for old, new in pairs:
        hits = mutated.count(old)
        if hits != 1:
            print(f"mutante {mid} recusado: padrao ocorre {hits} vez(es) em {rel}")
            return 1
        mutated = mutated.replace(old, new, 1)
    diff = [line for line in difflib.unified_diff(text.splitlines(), mutated.splitlines(), lineterm="", n=0)
            if line[:1] in "+-" and line[:3] not in ("+++", "---")]
    removed = sum(1 for line in diff if line.startswith("-"))
    added = sum(1 for line in diff if line.startswith("+"))
    if removed != len(pairs) or added > len(pairs):
        print(f"mutante {mid} recusado: diff de -{removed} +{added}, esperado {len(pairs)} linha(s) trocada(s)")
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
        if argv[2] not in MUTANTS and argv[2] not in MULTI:
            print(f"mutante desconhecido: {argv[2]}")
            return 1
        return apply(argv[1], argv[2])
    if len(argv) == 4 and argv[0] == "compare":
        return compare(argv[1], argv[2], int(argv[3]))
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
