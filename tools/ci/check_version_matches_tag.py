#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Refuses a release tag that does not match the version declared in CMakeLists.txt.

This is a gate, not a report: any doubt is a failure. Three blind spots are
closed on purpose: a missing version line, a tag outside the four-component
format (GODS_LAWS.md L-23), and a shallow clone, where the tags may not have
been fetched and an apparent match would be a false green.

Usage: CI reads GITHUB_REF_NAME; a local run passes the tag as argument:
    python3 tools/ci/check_version_matches_tag.py v0.0.0.1
"""

import os
import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
CMAKE_FILE = REPO_ROOT / "CMakeLists.txt"
DECLARED_RE = re.compile(
    r"^\s*project\(\s*gusworld\s+VERSION\s+(\d+\.\d+\.\d+\.\d+)", re.MULTILINE
)
TAG_RE = re.compile(r"v(\d+\.\d+\.\d+\.\d+)")


def fail(message):
    """Print the reason to stderr and return the failing exit code."""
    print(f"REPROVADO: {message}", file=sys.stderr)
    return 1


def read_declared_version():
    """Return the four-component version declared in CMakeLists.txt, or None."""
    try:
        text = CMAKE_FILE.read_text(encoding="utf-8")
    except OSError:
        return None
    match = DECLARED_RE.search(text)
    return match.group(1) if match else None


def read_tag_name(argv):
    """Return the tag under test: the argument if given, else GITHUB_REF_NAME."""
    if len(argv) > 1:
        return argv[1]
    return os.environ.get("GITHUB_REF_NAME", "")


def is_shallow_clone():
    """Return True when the clone is shallow. Raises if git cannot answer."""
    result = subprocess.run(
        ["git", "rev-parse", "--is-shallow-repository"],
        cwd=REPO_ROOT,
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(f"git could not answer if the clone is shallow: {result.stderr.strip()}")
    return result.stdout.strip() == "true"


def main(argv):
    """Return 0 only when the tag was actually checked and matches the declared version."""
    declared = read_declared_version()
    if declared is None:
        return fail("no 'project(gusworld VERSION A.B.C.D ...)' line in CMakeLists.txt")

    tag = read_tag_name(argv)
    match = TAG_RE.fullmatch(tag)
    if match is None:
        return fail(f"tag {tag!r} is not in the vA.B.C.D format (four components, L-23)")

    try:
        shallow = is_shallow_clone()
    except RuntimeError as error:
        return fail(str(error))
    if shallow:
        return fail(
            "clone is shallow, so the tags may be missing and the check could not look. "
            "Use fetch-depth: 0 and fetch-tags: true."
        )

    if match.group(1) != declared:
        return fail(f"tag {tag} diverges from declared version {declared}")

    print(f"ok: tag {tag} matches declared version {declared}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
