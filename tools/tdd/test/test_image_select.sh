# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# The image is chosen by its own label (A4, MÉDIA-8): for each local image of the
# gusworld-ci-fedora family, the digest label is hashed with the current context, and
# the image whose tag matches that hash is the one. No match means a stale image.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

SEL="$TDD_DIR/image_select.sh"
require_file "image_select.sh existe" "$SEL"
[ -f "$SEL" ] || suite_done

FX="$(make_fixture)"
BIN="$(make_fakebin yes)"
export FAKE_DOCKER_LOG="$BIN/docker.log"
export FAKE_DOCKER_STDIN="$BIN/stdin.tar"
export PATH="$BIN:$PATH"
setup_fake_image "$FX"
tag="${FAKE_IMAGE_LS#gusworld-ci-fedora:}"

out="$(bash "$SEL" "$FX" 2>/dev/null)"
expect_rc "imagem da propria etiqueta: rc 0" 0 bash "$SEL" "$FX"
expect_eq "imagem da propria etiqueta: referencia" "IMAGE_REF=gusworld-ci-fedora:$tag" "$(printf '%s\n' "$out" | sed -n '1p')"
expect_eq "imagem da propria etiqueta: id" "IMAGE_ID=sha256:imagemfake" "$(printf '%s\n' "$out" | sed -n '2p')"

# A local image whose tag does not match its label's digest is stale.
expect_rc "imagem defasada: rc 1" 1 env FAKE_IMAGE_LS="gusworld-ci-fedora:000000000000" bash "$SEL" "$FX"
env FAKE_IMAGE_LS="gusworld-ci-fedora:000000000000" bash "$SEL" "$FX" > "$BIN/stale.out" 2>&1
expect_rc "imagem defasada: mensagem diz defasada" 0 grep -q 'defasada' "$BIN/stale.out"

# No gusworld image at all: stale as well.
expect_rc "sem imagem local: rc 1" 1 bash -c "FAKE_IMAGE_LS='' bash '$SEL' '$FX' >/dev/null 2>&1"

# A digest label that is missing or malformed is not a candidate.
expect_rc "rotulo sem digest: rc 1" 1 bash -c "FAKE_IMAGE_DIGEST='<no value>' FAKE_IMAGE_LS='$FAKE_IMAGE_LS' bash '$SEL' '$FX' >/dev/null 2>&1"

suite_done
