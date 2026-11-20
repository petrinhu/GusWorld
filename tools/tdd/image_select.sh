#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Chooses the local image of the test container by its own label (A4, MÉDIA-8). For
# each local gusworld-ci-fedora image, the org.gusworld.base.digest label is hashed with
# the current context (env_hash.sh <digest>). The image whose tag equals the first 12
# digits of that hash is the one that matches the current context. Nothing is built.
# If several images match, the most recent by Created wins. If none does, the image is
# stale: the script exits 1 and the invoker refuses the run.
# Output, on success: IMAGE_REF=<ref> and IMAGE_ID=<id>, one per line.
# usage: image_select.sh <repo_root>
set -uo pipefail

root="${1:?usage: image_select.sh <repo_root>}"
refs="$(timeout 20 docker image ls --format '{{.Repository}}:{{.Tag}}' 2>/dev/null)" || {
    echo "docker image ls falhou" >&2
    exit 1
}

candidates=()
while IFS= read -r ref; do
    case "$ref" in
        gusworld-ci-fedora:*) ;;
        *) continue ;;
    esac
    digest="$(timeout 20 docker image inspect --format '{{index .Config.Labels "org.gusworld.base.digest"}}' "$ref" 2>/dev/null)" || continue
    case "$digest" in
        sha256:*) ;;
        *) continue ;;
    esac
    expected="gusworld-ci-fedora:$("$root/tools/container/env_hash.sh" "$digest" | cut -c1-12)"
    [ "$ref" = "$expected" ] || continue
    created="$(timeout 20 docker image inspect --format '{{.Created}}' "$ref" 2>/dev/null)" || continue
    id="$(timeout 20 docker image inspect --format '{{.Id}}' "$ref" 2>/dev/null)" || continue
    [ -n "$id" ] || continue
    candidates+=("$created|$ref|$id")
done <<< "$refs"

if [ "${#candidates[@]}" -eq 0 ]; then
    echo "imagem defasada: nenhuma imagem local bate com o contexto atual, rode build_image.sh" >&2
    exit 1
fi

best="$(printf '%s\n' "${candidates[@]}" | sort -r | sed -n '1p')"
IFS='|' read -r _created ref id <<< "$best"
printf 'IMAGE_REF=%s\n' "$ref"
printf 'IMAGE_ID=%s\n' "$id"
