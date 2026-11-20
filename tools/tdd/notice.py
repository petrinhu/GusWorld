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
import re
import stat
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
# /var/tmp is writable by every local user, so last-reason is text from outside the
# trust boundary. The motive shown to the agent is capped and printable only.
UNTRUSTED_MOTIVE = "motivo indisponível"
MAX_MOTIVE = 200
READ_LIMIT = 4096
ANSI_SEQUENCE = re.compile(r"\x1b\[[0-9;?]*[ -/]*[@-~]")


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


def trusted_stat(info, want_dir):
    """True when the entry is a directory or a regular file (as wanted), owned by this
    uid, and not writable by group or others."""
    kind_ok = stat.S_ISDIR(info.st_mode) if want_dir else stat.S_ISREG(info.st_mode)
    return (kind_ok and info.st_uid == os.getuid()
            and not info.st_mode & (stat.S_IWGRP | stat.S_IWOTH))


def read_last_reason(digest):
    """The run_suite.sh reason text. "" when the file is absent (the normal case with no
    last run), and None when a check fails: the folder or the file is a symlink, has
    another owner, or is writable by group or others. lstat and O_NOFOLLOW keep a
    symlink from being followed."""
    path = REASON_ROOT / digest / "last-reason"
    try:
        if not trusted_stat(os.lstat(path.parent), want_dir=True):
            return None
        fd = os.open(path, os.O_RDONLY | os.O_NOFOLLOW)
    except FileNotFoundError:
        return ""
    except OSError:
        return None
    try:
        with os.fdopen(fd, "rb") as f:
            if not trusted_stat(os.fstat(f.fileno()), want_dir=False):
                return None
            raw = f.read(READ_LIMIT)
    except OSError:
        return None
    return raw.decode("utf-8", errors="replace")


def clean_text(text):
    """One line, printable characters only, with ANSI sequences removed."""
    without_ansi = ANSI_SEQUENCE.sub("", str(text))
    collapsed = " ".join(without_ansi.split())
    return " ".join("".join(ch for ch in collapsed if ch.isprintable()).split())


def motive(state, digest):
    """The run_suite.sh reason when it is a no-run record; else the state's reason. The
    motive is capped at MAX_MOTIVE characters. An untrusted last-reason gives the fixed
    UNTRUSTED_MOTIVE, and the notice still warns."""
    last = read_last_reason(digest)
    if last is None:
        return UNTRUSTED_MOTIVE
    last = clean_text(last)
    if last.startswith(NO_EXEC_PREFIX):
        text = last[len(NO_EXEC_PREFIX):]
    else:
        text = clean_text(state.get("reason") or "")
    return text[:MAX_MOTIVE] or UNKNOWN


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
