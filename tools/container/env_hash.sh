#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Prints the SHA-256 that names the image: one line, 64 hex digits. The tag is
# gusworld-ci-fedora:<first 12 digits>. The hash covers the content and the
# in-context name of each file, in the fixed order of context_list.sh, so the
# same input always gives the same output.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=context_list.sh
. "$here/context_list.sh"

for rel in "${CONTEXT_FILES[@]}"; do
    digest="$(sha256sum < "$REPO_ROOT/$rel" | cut -d' ' -f1)"
    printf '%s  %s\n' "$digest" "$(basename "$rel")"
done | sha256sum | cut -d' ' -f1
