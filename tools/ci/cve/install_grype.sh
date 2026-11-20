#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Installs grype pinned by SHA-256 into a directory. Single responsibility: fetch, check, place.
# Usage: install_grype.sh <destination-dir>
# The hash was read from the official checksums file of release v0.122.1
# (grype_0.122.1_checksums.txt) and matches the tarball. Any other tarball is rejected.
# No third-party action is used, and no moving label.
set -eu

dest="${1:?destination directory missing}"
version="0.122.1"
sha256="f1ba7e2917ee1395618de20a98da0aa7c0fb1788927797651047c50502a37538"
tarball="grype_${version}_linux_amd64.tar.gz"
base="https://github.com/anchore/grype/releases/download/v${version}"

work="$(mktemp -d "${RUNNER_TEMP:-/var/tmp}/grype-download.XXXXXX")"
cd "$work"
curl -fsSLO "$base/$tarball"
echo "$sha256  $tarball" | sha256sum -c -
mkdir -p "$dest"
tar -xzf "$tarball" -C "$dest" grype
test -x "$dest/grype"
echo "grype $version installed in $dest"
