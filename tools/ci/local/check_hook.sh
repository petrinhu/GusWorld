#!/bin/sh
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Half (b) of the fifth gate (GW-19, commit message gate), extended by the lead's decision
# of 21/11/2026 (docs/decisoes-do-lider.md): the commit-msg hook AND the pre-commit hook
# (LEI ZERO) must be installed for the repository. A CI clone never carries the hooks, so
# this check is local by design (lead's decision, 12/10/2026; TODO item C7).
#
# For each hook named in HOOKS, both conditions must hold (the spec, as decided by the CTO
# on 21/11/2026; a byte comparison with the source is deliberately NOT made):
#   1. <git-common-dir>/hooks/<hook> exists and is executable.
#   2. If core.hooksPath is set, git running the hooks from that path reaches the
#      repository hook. Checked by behaviour: a throwaway repository gets marker hooks,
#      and `git hook run` through the configured path must run them.
#      A global hooksPath that shadows the local hooks without chaining fails here.
#
# Usage: check_hook.sh <repository-directory>
# Exit 0 only when every hook passes. Writes only a probe directory under TMPDIR (or
# /var/tmp). The probe directory is not removed here: its path is printed, and it is
# removed by hand.
set -eu

repo="${1:?repository directory missing}"
HOOKS="commit-msg pre-commit"

fail() {
  echo "HOOK CHECK FAILED: $*"
  exit 1
}

gd=$(git -C "$repo" rev-parse --git-common-dir 2>/dev/null) || fail "not a git repository: $repo"
case "$gd" in
  /*|[A-Za-z]:*) ;;
  *) gd="$repo/$gd" ;;
esac

for name in $HOOKS; do
  local_hook="$gd/hooks/$name"
  if [ ! -f "$local_hook" ]; then
    fail "local $name hook missing: $local_hook (run: python3 tools/git-hooks/install.py)"
  fi
  if [ ! -x "$local_hook" ]; then
    fail "local $name hook without the execute bit: $local_hook"
  fi
  echo "local $name hook present and executable: $local_hook"
done

hooks_path=$(git -C "$repo" config --get core.hooksPath || true)
if [ -z "$hooks_path" ]; then
  echo "core.hooksPath not set: git reads $gd/hooks directly"
  echo "HOOK CHECK PASSED"
  exit 0
fi
# A relative core.hooksPath is relative to the repository's top level, as git resolves it.
# Resolving it inside the probe would test a different directory and fail for no reason.
repo_abs=$(cd "$repo" && pwd -P)
case "$hooks_path" in
  /*|[A-Za-z]:*|"~"/*) ;;
  *) hooks_path="$repo_abs/$hooks_path" ;;
esac
echo "core.hooksPath is set: $hooks_path"

base="${TMPDIR:-/var/tmp}"
[ -d "$base" ] || base=/tmp
probe=$(mktemp -d "$base/gw-hookprobe.XXXXXX") || fail "cannot create the probe directory under $base"
echo "probe repository (remove by hand when done): $probe"
git init -q "$probe" || fail "cannot init the probe repository"
msg="$probe/msg.txt"
echo "probe" > "$msg"
for name in $HOOKS; do
  printf '#!/bin/sh\necho reached > "%s"\nexit 0\n' "$probe/marker-$name" > "$probe/.git/hooks/$name"
  chmod +x "$probe/.git/hooks/$name"
done

for name in $HOOKS; do
  # --ignore-missing keeps an absent hook silent, so the marker decides: no marker, no chain.
  if [ "$name" = commit-msg ]; then
    git -C "$probe" -c core.hooksPath="$hooks_path" hook run --ignore-missing "$name" -- "$msg" \
      || fail "git hook run $name failed through core.hooksPath=$hooks_path"
  else
    git -C "$probe" -c core.hooksPath="$hooks_path" hook run --ignore-missing "$name" \
      || fail "git hook run $name failed through core.hooksPath=$hooks_path"
  fi
  if [ ! -f "$probe/marker-$name" ]; then
    fail "core.hooksPath=$hooks_path does not chain to $gd/hooks/$name (the repository hook did not run)"
  fi
  echo "chain reached the repository $name hook through core.hooksPath"
done
echo "HOOK CHECK PASSED"
