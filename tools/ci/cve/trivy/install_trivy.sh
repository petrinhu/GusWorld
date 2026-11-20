#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Installs trivy pinned by SHA-256 into a directory. Single responsibility: fetch, check, place.
# Usage: install_trivy.sh <destination-dir>
# The hash was read from the official checksums file of release v0.76.2:
#   https://github.com/aquasecurity/trivy/releases/download/v0.76.2/trivy_0.76.2_checksums.txt
#   line 7: bc0fc726e017b8927a62b2d3531d7a84ee01769daf5c99ed866dced54089fdbf  trivy_0.76.2_Linux-64bit.tar.gz
# Versions 0.69.4, 0.69.5 and 0.69.6 are vetoed by the leader (20/11/2026) and must never be pinned here.
# No third-party action is used, and no moving label. Any other tarball is rejected.
set -eu

dest="${1:?destination directory missing}"
version="0.76.2"
sha256="bc0fc726e017b8927a62b2d3531d7a84ee01769daf5c99ed866dced54089fdbf"
tarball="trivy_${version}_Linux-64bit.tar.gz"
base="https://github.com/aquasecurity/trivy/releases/download/v${version}"
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

work="$(mktemp -d "${RUNNER_TEMP:-/var/tmp}/trivy-download.XXXXXX")"
# The download directory is removed on every exit, success or failure, by its own path.
trap 'rm -rf "$work"' EXIT
cd "$work"
bash "$here/../../fetch_verified.sh" "$base/$tarball" "$sha256" "$work/$tarball"
mkdir -p "$dest"
tar -xzf "$work/$tarball" -C "$dest" trivy
test -x "$dest/trivy"
echo "trivy $version installed in $dest"
