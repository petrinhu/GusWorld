#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Prints the SHA-256 that names the image: one line, 64 hex digits. The tag is
# gusworld-ci-fedora:<first 12 digits>. The hash covers the content and the
# in-context name of each context file, in the fixed order of context_list.sh,
# AND the digest of the base image (A4): a new base gives a new tag.
# Usage: env_hash.sh <base-digest>, for example sha256:...
set -euo pipefail

base_digest="${1:?usage: env_hash.sh <base-digest>}"
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=context_list.sh
. "$here/context_list.sh"

{
    for rel in "${CONTEXT_FILES[@]}"; do
        digest="$(sha256sum < "$REPO_ROOT/$rel" | cut -d' ' -f1)"
        printf '%s  %s\n' "$digest" "$(basename "$rel")"
    done
    printf '%s  base-digest\n' "$base_digest"
} | sha256sum | cut -d' ' -f1
