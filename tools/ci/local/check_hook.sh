#!/bin/sh
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Half (b) of the fifth gate (GW-19, commit message gate): fails when the commit-msg
# hook is not installed for the repository. A CI clone never carries the hook, so this
# check is local by design (lead's decision, 12/10/2026; TODO item C7).
#
# Both conditions must hold:
#   1. <git-common-dir>/hooks/commit-msg exists, is executable and equals the source
#      tools/git-hooks/commit-msg when that source exists (a stale copy fails).
#   2. If core.hooksPath is set, git running the hooks from that path reaches the
#      repository hook. Checked by behaviour: a throwaway repository gets a marker
#      hook, and `git hook run commit-msg` through the configured path must run it.
#      A global hooksPath that shadows the local hooks without chaining fails here.
#
# Usage: check_hook.sh <repository-directory>
# Exit 0 only when both hold. Writes only a probe directory under TMPDIR (or /var/tmp).
# The probe directory is not removed here: its path is printed, and it is removed by hand.
set -eu

repo="${1:?repository directory missing}"

fail() {
  echo "HOOK CHECK FAILED: $*"
  exit 1
}

gd=$(git -C "$repo" rev-parse --git-common-dir 2>/dev/null) || fail "not a git repository: $repo"
case "$gd" in
  /*|[A-Za-z]:*) ;;
  *) gd="$repo/$gd" ;;
esac

local_hook="$gd/hooks/commit-msg"
if [ ! -f "$local_hook" ]; then
  fail "local hook missing: $local_hook (run: python3 tools/git-hooks/install.py)"
fi
if [ ! -x "$local_hook" ]; then
  fail "local hook without the execute bit: $local_hook"
fi
source_hook="$repo/tools/git-hooks/commit-msg"
if [ -f "$source_hook" ] && ! cmp -s "$source_hook" "$local_hook"; then
  fail "installed hook differs from tools/git-hooks/commit-msg: run python3 tools/git-hooks/install.py"
fi
echo "local hook present and executable: $local_hook"

hooks_path=$(git -C "$repo" config --get core.hooksPath || true)
if [ -z "$hooks_path" ]; then
  echo "core.hooksPath not set: git reads $gd/hooks directly"
  echo "HOOK CHECK PASSED"
  exit 0
fi
echo "core.hooksPath is set: $hooks_path"

base="${TMPDIR:-/var/tmp}"
[ -d "$base" ] || base=/tmp
probe=$(mktemp -d "$base/gw-hookprobe.XXXXXX") || fail "cannot create the probe directory under $base"
echo "probe repository (remove by hand when done): $probe"
git init -q "$probe" || fail "cannot init the probe repository"
marker="$probe/marker"
cat > "$probe/.git/hooks/commit-msg" <<EOF
#!/bin/sh
echo reached > "$marker"
exit 0
EOF
chmod +x "$probe/.git/hooks/commit-msg"
msg="$probe/msg.txt"
echo "probe" > "$msg"

# --ignore-missing keeps an absent hook silent, so the marker decides: no marker, no chain.
git -C "$probe" -c core.hooksPath="$hooks_path" hook run --ignore-missing commit-msg -- "$msg" \
  || fail "git hook run failed through core.hooksPath=$hooks_path"
if [ ! -f "$marker" ]; then
  fail "core.hooksPath=$hooks_path does not chain to $gd/hooks/commit-msg (the repository hook did not run)"
fi
echo "chain reached the repository hook through core.hooksPath"
echo "HOOK CHECK PASSED"
