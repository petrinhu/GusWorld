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
PLUGIN_NAME = "bigtech"
MARKETPLACE_ID = "petrinhu"
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


def claude_home():
    return Path(os.path.expanduser("~")) / ".claude"


def read_json(path):
    """The parsed JSON object at path, or None when it is absent or unreadable."""
    try:
        with open(path, encoding="utf-8") as f:
            data = json.load(f)
    except (OSError, ValueError):
        return None
    return data if isinstance(data, dict) else None


def marketplace_location():
    """installLocation of the petrinhu marketplace when its source is a `directory`. The
    known_marketplaces.json entry comes first; the settings' extraKnownMarketplaces path
    is the fallback. None when neither applies."""
    known = read_json(claude_home() / "plugins" / "known_marketplaces.json") or {}
    entry = known.get(MARKETPLACE_ID)
    if isinstance(entry, dict) and _is_directory_source(entry.get("source")):
        location = entry.get("installLocation")
        if isinstance(location, str):
            return location
    settings = read_json(claude_home() / "settings.json") or {}
    known_extra = settings.get("extraKnownMarketplaces")
    extra = known_extra.get(MARKETPLACE_ID) if isinstance(known_extra, dict) else None
    if isinstance(extra, dict) and _is_directory_source(extra.get("source")):
        path = (extra.get("source") or {}).get("path")
        if isinstance(path, str):
            return path
    return None


def _is_directory_source(source):
    return isinstance(source, dict) and source.get("source") == "directory"


def bigtech_relative_source(location):
    """The relative `source` of the bigtech entry in the marketplace's marketplace.json,
    such as "./". None when the entry is absent or its source is not relative."""
    data = read_json(Path(location) / ".claude-plugin" / "marketplace.json") or {}
    plugins = data.get("plugins")
    for entry in plugins if isinstance(plugins, list) else []:
        if isinstance(entry, dict) and entry.get("name") == PLUGIN_NAME:
            source = entry.get("source")
            if isinstance(source, str) and not os.path.isabs(source) and source.startswith("."):
                return source
    return None


def marketplace_plugin_root():
    """The bigtech root as Claude Code loads it from a `directory` marketplace: loaded in
    place, so the root is installLocation + the relative source. None when the rule does
    not apply (no directory marketplace, or no relative source for bigtech)."""
    location = marketplace_location()
    if location is None:
        return None
    source = bigtech_relative_source(location)
    if source is None:
        return None
    return os.path.join(location, source)


def installed_plugin_root():
    """installPath of the user-scope bigtech entry in installed_plugins.json."""
    registry = read_json(claude_home() / "plugins" / "installed_plugins.json") or {}
    entries = (registry.get("plugins") or {}).get(PLUGIN_ID)
    for entry in entries if isinstance(entries, list) else []:
        if isinstance(entry, dict) and entry.get("scope") == "user":
            install = entry.get("installPath")
            if isinstance(install, str):
                return install
    raise PluginUnavailable()


def plugin_hooks_dir():
    """The hooks/ of the bigtech plugin that Claude Code runs. This is the single resolver:
    the contract test and the launcher call it too. Every root is realpath()'d before the
    trust check, so a symlinked marketplace is judged by its target (review F2)."""
    root = marketplace_plugin_root()
    if root is None:
        root = installed_plugin_root()
    real = os.path.realpath(root)
    hooks = Path(real) / "hooks"
    try:
        if not (trusted_stat(os.lstat(real), want_dir=True)
                and trusted_stat(os.lstat(hooks), want_dir=True)):
            raise PluginUnavailable()
    except OSError:
        raise PluginUnavailable()
    return hooks


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


# Tolerance of the freshness check, in seconds. run_suite.sh writes last-reason before it
# exits, and the runner stamps ts = int(time.time()) after the subprocess returns. When a
# whole second ticks in between, the fresh last-reason has mtime < ts by under one second,
# and the check wrongly calls it stale. Decided by the CTO (C8 fatia 7, achado 2): a
# 2 s window absorbs that race. A last-reason written 5 s before ts is still stale.
FRESH_TOLERANCE = 2


def fresh_enough(mtime, state):
    """A last-reason older than the state's ts (minus FRESH_TOLERANCE) belongs to an
    earlier run: it is ignored. Without a state there is no ts to compare with, so the
    last-reason is not trusted (R2): no state never lets an old last-reason become the
    motive."""
    if not isinstance(state, dict):
        return False
    ts = state.get("ts")
    if not isinstance(ts, (int, float)) or isinstance(ts, bool):
        return True
    return mtime >= ts - FRESH_TOLERANCE


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
        # R2: the state cannot be read, so the motive is the guard's own I/O message and
        # never a last-reason left by an earlier run.
        return clean_text(msg).rstrip(".")[:MAX_MOTIVE] or UNKNOWN
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
