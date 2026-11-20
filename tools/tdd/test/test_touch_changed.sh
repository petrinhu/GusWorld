# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Change detection after the tar (a false green otherwise). tar keeps the host mtimes,
# so a file whose content changed can carry an older mtime than the build products,
# and cmake/ninja would not rebuild it. touch_changed.sh touches exactly the files
# whose sha256 differs from the previous manifest, and leaves the others alone.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

TC="$TDD_DIR/touch_changed.sh"
require_file "touch_changed.sh existe" "$TC"
[ -f "$TC" ] || suite_done

T="$(mktemp -d /tmp/tdd-touch-XXXXXX)"
mkdir "$T/tree"
printf 'alpha\n' > "$T/tree/a.txt"
printf 'beta\n' > "$T/tree/b.txt"
M="$T/manifest"

# First run: no manifest yet, so every file is new and is touched.
expect_rc "primeira execucao: rc 0" 0 bash "$TC" "$T/tree" "$M"
expect_rc "primeira execucao: manifesto gravado" 0 test -s "$M"

# Both files get an OLD mtime, as a tar of older host files would give. Only a.txt changes.
touch -d '2000-01-01 00:00:00 UTC' "$T/tree/a.txt" "$T/tree/b.txt"
printf 'alpha changed\n' > "$T/tree/a.txt"
touch -d '2000-01-01 00:00:00 UTC' "$T/tree/a.txt"
b_before="$(stat -c %Y "$T/tree/b.txt")"
expect_rc "segunda execucao: rc 0" 0 bash "$TC" "$T/tree" "$M"
expect_eq "arquivo alterado recebe mtime novo" 1 "$(( $(stat -c %Y "$T/tree/a.txt") > 946684800 ? 1 : 0 ))"
expect_eq "arquivo nao alterado mantem o mtime" "$b_before" "$(stat -c %Y "$T/tree/b.txt")"

# A new file is touched too.
printf 'gamma\n' > "$T/tree/c.txt"
touch -d '2000-01-01 00:00:00 UTC' "$T/tree/c.txt"
expect_rc "terceira execucao com arquivo novo: rc 0" 0 bash "$TC" "$T/tree" "$M"
expect_eq "arquivo novo recebe mtime novo" 1 "$(( $(stat -c %Y "$T/tree/c.txt") > 946684800 ? 1 : 0 ))"

suite_done
