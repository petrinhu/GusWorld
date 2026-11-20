#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# PreToolUse notice of the TDD guard (ADR-026, decision 8). When the guard's last run
# did not happen (ran=false), it tells the agent through additionalContext and the user
# through systemMessage. It never blocks: every path exits 0, because exit 2 on
# PreToolUse blocks the edit. A run that happened, or no state at all, stays silent.
# Standard library only.
import hashlib
import json
import os
import sys
from pathlib import Path

NO_RUN_TEXT = (
    "TDD guard não rodou (falha de infraestrutura): {reason}. "
    "A edição foi liberada sem teste."
)
# An unreadable state that the guard reads as "no state" blocks production edits.
BLOCK_TEXT = (
    "TDD guard sem estado legível: as edições de produção ficam bloqueadas "
    "até o runner gravar o estado de novo ({reason})."
)
UNREADABLE = "estado ilegível"
BAD_JSON = "JSON inválido"
UNKNOWN = "desconhecido"
# Written by run_suite.sh (ASCII, as the invoker writes it), before the motive.
NO_EXEC_PREFIX = "sem execucao: "
REASON_ROOT = Path("/var/tmp/gusworld-tdd")


class StateUnreadable(Exception):
    """The guard state exists but cannot be used. `blocks` says what the guard does:
    True when it reads the state as absent (invalid JSON or UTF-8, so production edits
    are blocked); False when it fails open (an OS error, or a JSON value that is not an
    object)."""

    def __init__(self, blocks):
        super().__init__("blocks" if blocks else "fail-open")
        self.blocks = blocks


def project_root():
    # This file lives in <root>/tools/tdd/, so the root is three levels up.
    return Path(__file__).resolve().parents[2]


def state_digest(root):
    # Same digest as the plugin's state_path() and as run_suite.sh (sha256 of the
    # realpath of the project root, first 16 hex digits).
    return hashlib.sha256(os.path.realpath(root).encode()).hexdigest()[:16]


def guard_state_path(digest):
    home = Path(os.path.expanduser("~"))
    return home / ".claude" / "state" / "tdd-guard" / digest / "last-run.json"


def read_state(path):
    """Return the state object, None when absent, or raise StateUnreadable."""
    try:
        with open(path, encoding="utf-8") as f:
            data = json.load(f)
    except FileNotFoundError:
        return None
    except ValueError:
        # Bad JSON or bad UTF-8: the guard's read_state returns None, so it blocks.
        raise StateUnreadable(blocks=True)
    except OSError:
        # The guard warns and fails open on an OS error.
        raise StateUnreadable(blocks=False)
    if not isinstance(data, dict):
        # The guard fails open here: its state.get() raises and main() lets the edit go.
        raise StateUnreadable(blocks=False)
    return data


def read_last_reason(digest):
    try:
        with open(REASON_ROOT / digest / "last-reason", encoding="utf-8",
                  errors="replace") as f:
            return f.read()
    except OSError:
        return ""


def one_line(text):
    return " ".join(str(text).split())


def motive(state, digest):
    """The run_suite.sh reason when it is a no-run record; else the state's reason."""
    last = one_line(read_last_reason(digest))
    if last.startswith(NO_EXEC_PREFIX):
        return last[len(NO_EXEC_PREFIX):] or UNKNOWN
    return one_line(state.get("reason") or "") or UNKNOWN


def evaluate(root=None):
    """Return the notice text, or None when the guard ran or left no state."""
    root = root or project_root()
    digest = state_digest(root)
    try:
        state = read_state(guard_state_path(digest))
    except StateUnreadable as e:
        if e.blocks:
            return BLOCK_TEXT.format(reason=BAD_JSON)
        return NO_RUN_TEXT.format(reason=UNREADABLE)
    if state is None or state.get("ran", False):
        return None
    return NO_RUN_TEXT.format(reason=motive(state, digest))


def payload(text):
    return {
        "systemMessage": text,
        "hookSpecificOutput": {
            "hookEventName": "PreToolUse",
            "additionalContext": text,
        },
    }


def main():
    try:
        text = evaluate()
        if text is not None:
            data = json.dumps(payload(text), ensure_ascii=False) + "\n"
            sys.stdout.buffer.write(data.encode("utf-8"))
    except Exception as e:  # never block the edit, whatever happens here
        print(f"tdd notice: erro interno ignorado ({e}).", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
