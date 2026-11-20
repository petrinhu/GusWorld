# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Restores change detection after the tar (ADR-026). tar keeps the host mtimes, so a
# file whose content changed can carry an older mtime than the build products, and
# cmake and ninja would not rebuild it: a false green. This script hashes every file of
# the tree, compares with the manifest of the previous run, and touches exactly the
# files whose sha256 differs or that are new. Unchanged files keep their mtime, so the
# build stays incremental. The manifest is replaced only after every touch succeeded.
# usage: touch_changed.sh <tree> <manifest>
set -uo pipefail

tree="${1:?usage: touch_changed.sh <tree> <manifest>}"
manifest="${2:?usage: touch_changed.sh <tree> <manifest>}"
new="$manifest.new"
changed="$manifest.changed"

[ -f "$manifest" ] || : > "$manifest" || exit 1

cd "$tree" || exit 1
find . -type f -print0 | sort -z | xargs -0 -r sha256sum > "$new" || exit 1

# A line of sha256sum is "<64 hex>  <path>": the path starts at column 67.
awk 'NR == FNR { seen[$0] = 1; next } !($0 in seen) { print substr($0, 67) }' \
    "$manifest" "$new" > "$changed" || exit 1

while IFS= read -r path; do
    touch -- "$path" || exit 1
done < "$changed"

mv -f "$new" "$manifest" || exit 1
