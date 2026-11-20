#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Red proof of the clang-format gate (TODO item C11, L-36 global): the judge must be SEEN
# rejecting a badly formatted file before its green on the real tree counts.
# Two cases, both in a directory outside the checkout:
#   red:   a file with 2-space indentation and a line over 100 columns must be REJECTED,
#          with exit 1 AND the id clang-format-violations in the log (exit 1 alone is not
#          enough: an invalid argument also returns 1).
#   green: the output of clang-format itself on that file must be ACCEPTED. Without this
#          case a judge that rejects everything would pass the red proof.
# The .clang-format of the repository is copied next to the files, so the policy under
# test is the real one. Nothing here reformats the tree: the files live in the work dir.
# Usage: proof.sh <work-dir>
set -eu -o pipefail

work="${1:?work dir path missing}"
here="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "$here/../../.." && pwd)"

mkdir -p "$work"
cp "$root/.clang-format" "$work/.clang-format"

cat > "$work/bad.cpp" <<'CPP'
int sum_of_many(int first_value, int second_value, int third_value, int fourth_value, int fifth_value, int sixth_value, int seventh_value) {
  return first_value + second_value + third_value + fourth_value + fifth_value + sixth_value + seventh_value;
}
CPP

if ! clang-format --style=file "$work/bad.cpp" > "$work/clean.cpp"; then
  echo "FORMAT PROOF FAILED: clang-format could not produce the reference output"
  exit 1
fi

printf '%s\n' "$work/bad.cpp" > "$work/bad.list"
printf '%s\n' "$work/clean.cpp" > "$work/clean.list"

rc=0
bash "$here/judge.sh" "$work/bad.list" "$work/bad.log" || rc=$?
if [ "$rc" -ne 1 ] || ! grep -q 'clang-format-violations' "$work/bad.log"; then
  echo "FORMAT PROOF FAILED: the judge did not reject the badly formatted file (exit ${rc}, id clang-format-violations needed)"
  exit 1
fi
echo "PROOF PASS (red): the badly formatted file was rejected, exit ${rc}, id clang-format-violations in the log"

rc=0
bash "$here/judge.sh" "$work/clean.list" "$work/clean.log" || rc=$?
if [ "$rc" -ne 0 ]; then
  echo "FORMAT PROOF FAILED: the judge rejected the output of clang-format itself (exit ${rc})"
  exit 1
fi
echo "PROOF PASS (green): the output of clang-format is accepted"
