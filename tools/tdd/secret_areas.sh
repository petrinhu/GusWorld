# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# The four git-crypt areas that never enter the test tar (ADR-026, decision 2):
#   docs/_secret/, docs/narrative/deep/stinger/, docs/book/, tools/security/termos_proibidos.txt
# Each area is matched both as the directory itself (with or without a trailing slash)
# and as anything under it, so a directory entry is caught as well as its files (MÉDIA-6).
# Sourced by build_list.sh and make_tar.sh. It runs nothing on its own.
# The tar is built from a whitelist, so these paths are excluded by construction;
# this check is the second lock: a secret path that reaches the list aborts the run.

is_secret_path() {
    local p="${1%/}"
    case "$p" in
        docs/_secret|docs/narrative/deep/stinger|docs/book|tools/security/termos_proibidos.txt)
            return 0
            ;;
        docs/_secret/*|docs/narrative/deep/stinger/*|docs/book/*)
            return 0
            ;;
    esac
    return 1
}
