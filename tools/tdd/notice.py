#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# PreToolUse notice of the TDD guard (ADR-026, decision 8). It does not reimplement the
# guard: it calls the bigtech plugin's own tdd_guard.evaluate on the hook payload, and it
# speaks only where the guard fails open with a warning (code 0 with a message), or where
# the notice itself cannot trust the plugin. When the guard blocks (code 2) the notice
# stays mute, because the guard already speaks. It never blocks: every path exits 0.
# Standard library only; the plugin's own modules are imported from its trusted hooks dir.
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
PLUGIN_TEXT = (
    "plugin bigtech não encontrado ou ilegível: o guard de TDD pode não estar rodando"
)
INTERNAL_TEXT = "aviso do guard falhou internamente"
UNTRUSTED_MOTIVE = "motivo indisponível"
UNKNOWN = "desconhecido"
# Written by run_suite.sh (ASCII, as the invoker writes it), before the motive.
NO_EXEC_PREFIX = "sem execucao: "
PLUGIN_ID = "bigtech@petrinhu"
GUARD_BLOCK = 2
# /var/tmp is writable by every local user: last-reason is text from outside the trust
# boundary, so it is read only when its folder and file pass trusted_stat, and it is
# capped and stripped of control characters before it reaches the agent.
REASON_ROOT = Path("/var/tmp/gusworld-tdd")
MAX_MOTIVE = 200
READ_LIMIT = 4096
ANSI_SEQUENCE = re.compile(r"\x1b\[[0-9;?]*[ -/]*[@-~]")


class PluginUnavailable(Exception):
    """The bigtech plugin cannot be found, read, or trusted."""


def trusted_stat(info, want_dir):
    """True for a directory (or a regular file) owned by this uid and not writable by
    group or others."""
    kind_ok = stat.S_ISDIR(info.st_mode) if want_dir else stat.S_ISREG(info.st_mode)
    return (kind_ok and info.st_uid == os.getuid()
            and not info.st_mode & (stat.S_IWGRP | stat.S_IWOTH))


def plugin_hooks_dir():
    """The hooks/ of the user-scope bigtech plugin, from installed_plugins.json."""
    registry = Path(os.path.expanduser("~")) / ".claude" / "plugins" / "installed_plugins.json"
    try:
        with open(registry, encoding="utf-8") as f:
            entries = json.load(f)["plugins"][PLUGIN_ID]
    except (OSError, ValueError, KeyError, TypeError):
        raise PluginUnavailable()
    for entry in entries if isinstance(entries, list) else []:
        if not isinstance(entry, dict) or entry.get("scope") != "user":
            continue
        install = entry.get("installPath")
        if not isinstance(install, str):
            continue
        hooks = Path(install) / "hooks"
        try:
            if not (trusted_stat(os.lstat(install), want_dir=True)
                    and trusted_stat(os.lstat(hooks), want_dir=True)):
                raise PluginUnavailable()
        except OSError:
            raise PluginUnavailable()
        return hooks
    raise PluginUnavailable()


def load_plugin():
    """The plugin's tdd_common and tdd_guard, from its trusted hooks dir. Bytecode is not
    written into the user's plugin cache."""
    hooks = plugin_hooks_dir()
    sys.dont_write_bytecode = True
    sys.path.insert(0, str(hooks))
    try:
        import tdd_common
        import tdd_guard
    except Exception:
        raise PluginUnavailable()
    return tdd_common, tdd_guard


def state_digest(root):
    # The same digest as the plugin's state_path() and run_suite.sh: sha256 of the realpath
    # of the project root, first 16 hex digits.
    return hashlib.sha256(os.path.realpath(root).encode()).hexdigest()[:16]


def read_last_reason(digest):
    """(text, mtime) of run_suite.sh's last-reason. ("", 0.0) when the file is absent, the
    normal case with no last run. None when a check fails: a symlink, another owner, or
    writable by group or others. lstat and O_NOFOLLOW keep a symlink from being followed."""
    path = REASON_ROOT / digest / "last-reason"
    try:
        if not trusted_stat(os.lstat(path.parent), want_dir=True):
            return None
        fd = os.open(path, os.O_RDONLY | os.O_NOFOLLOW)
    except FileNotFoundError:
        return "", 0.0
    except OSError:
        return None
    try:
        with os.fdopen(fd, "rb") as f:
            info = os.fstat(f.fileno())
            if not trusted_stat(info, want_dir=False):
                return None
            raw = f.read(READ_LIMIT)
    except OSError:
        return None
    return raw.decode("utf-8", errors="replace"), info.st_mtime


def clean_text(text):
    """One line, printable characters only, with ANSI sequences removed."""
    without_ansi = ANSI_SEQUENCE.sub("", str(text))
    kept = "".join(ch for ch in without_ansi if ch.isprintable() or ch.isspace())
    return " ".join(kept.split())


def fresh_enough(mtime, state):
    """A last-reason older than the state's ts belongs to an earlier run: it is ignored."""
    ts = state.get("ts") if isinstance(state, dict) else None
    if not isinstance(ts, (int, float)) or isinstance(ts, bool):
        return True
    return mtime >= ts


def motive(state, msg, root):
    """The motive shown to the agent. A fresh run_suite.sh no-run record wins; then the
    state's reason; with no state at all, the guard's own message. Always capped."""
    got = read_last_reason(state_digest(root))
    if got is None:
        return UNTRUSTED_MOTIVE
    last_text, mtime = got
    last_text = clean_text(last_text)
    if last_text.startswith(NO_EXEC_PREFIX) and fresh_enough(mtime, state):
        found = last_text[len(NO_EXEC_PREFIX):]
    elif isinstance(state, dict):
        found = clean_text(state.get("reason") or "")
    else:
        found = clean_text(msg).rstrip(".")
    return found[:MAX_MOTIVE] or UNKNOWN


def motive_for(data, common, msg):
    """Find the project root the way the guard does, then read the state with the
    plugin's own reader."""
    start = os.path.dirname(os.path.realpath(common.extract_file_path(data))) or os.getcwd()
    root = common.find_project_root(start)
    if root is None:
        return UNKNOWN
    try:
        state = common.read_state(root)
    except OSError:
        state = None
    return motive(state, msg, root)


def evaluate(data, env):
    """The notice text, or None to stay mute. It mirrors the guard's own decision."""
    try:
        common, guard = load_plugin()
    except PluginUnavailable:
        return PLUGIN_TEXT
    try:
        code, msg = guard.evaluate(data, env)
    except Exception:
        return INTERNAL_TEXT
    if code == GUARD_BLOCK or not msg:
        return None
    try:
        return NO_RUN_TEXT.format(reason=motive_for(data, common, msg))
    except Exception:
        return INTERNAL_TEXT


def read_payload():
    try:
        data = json.load(sys.stdin)
    except (OSError, ValueError):
        return {}
    return data if isinstance(data, dict) else {}


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
        text = evaluate(read_payload(), os.environ)
    except Exception:  # an internal error never leaves the agent silent (review A6)
        text = INTERNAL_TEXT
    try:
        if text is not None:
            data = json.dumps(payload(text), ensure_ascii=False) + "\n"
            sys.stdout.buffer.write(data.encode("utf-8"))
    except Exception as e:  # never block the edit, whatever happens here
        print(f"tdd notice: falha ao escrever o aviso ({e}).", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
