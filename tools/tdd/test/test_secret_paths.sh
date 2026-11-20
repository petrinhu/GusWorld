# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Case (b): a path in a git-crypt area never enters the test tar. The fixture
# plants docs/_secret/x and widens the whitelist to docs/; the pipeline must fail
# closed, with no tar left behind.
. "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

SECRET_LIB="$TDD_DIR/secret_areas.sh"
require_file "secret_areas.sh existe" "$SECRET_LIB"
. "$SECRET_LIB"
require_function "is_secret_path definida" is_secret_path

for p in docs/_secret/x docs/narrative/deep/stinger/cena.md docs/book/livro.md tools/security/termos_proibidos.txt; do
    expect_rc "segredo reconhecido: $p" 0 is_secret_path "$p"
done
expect_rc "core/a.cpp nao e segredo" 1 is_secret_path core/a.cpp

expect_rc "lista branca sem area cifrada" 1 grep -qE '^(docs|tools/security)' "$TDD_DIR/build_paths.txt"

require_file "build_list.sh existe" "$TDD_DIR/build_list.sh"
require_file "make_tar.sh existe" "$TDD_DIR/make_tar.sh"

FX="$(make_fixture)"
echo "docs/" >> "$FX/tools/tdd/build_paths.txt"
# Each refusal is ONE case: exit 1 and no artifact, read together, so an absent
# script (rc 127) can never pass the "no artifact" half on its own.
bash "$FX/tools/tdd/build_list.sh" "$FX" "$FX/list-leak.nul" >/dev/null 2>&1
rc=$?
state=$([ -e "$FX/list-leak.nul" ] && echo presente || echo ausente)
expect_eq "build_list recusa segredo: rc 1, sem lista" "1 ausente" "$rc $state"

# Planted secret fed straight to make_tar: fail closed, no tar produced.
printf 'docs/_secret/x\0' > "$FX/list-direct.nul"
bash "$TDD_DIR/make_tar.sh" "$FX" "$FX/list-direct.nul" "$FX/leak.tar" >/dev/null 2>&1
rc=$?
state=$([ -e "$FX/leak.tar" ] && echo presente || echo ausente)
expect_eq "make_tar recusa segredo: rc 1, sem tar" "1 ausente" "$rc $state"

# Empty list: fail closed too.
: > "$FX/list-empty.nul"
bash "$TDD_DIR/make_tar.sh" "$FX" "$FX/list-empty.nul" "$FX/empty.tar" >/dev/null 2>&1
rc=$?
state=$([ -e "$FX/empty.tar" ] && echo presente || echo ausente)
expect_eq "make_tar recusa lista vazia: rc 1, sem tar" "1 ausente" "$rc $state"

# Positive case on a clean fixture: the tar holds the invoker tree and no secret.
GOOD="$(make_fixture)"
expect_rc "build_list monta lista limpa" 0 bash "$GOOD/tools/tdd/build_list.sh" "$GOOD" "$GOOD/list.nul"
expect_rc "make_tar monta tar limpo" 0 bash "$GOOD/tools/tdd/make_tar.sh" "$GOOD" "$GOOD/list.nul" "$GOOD/src.tar"
tar -tf "$GOOD/src.tar" > "$GOOD/names.txt" 2>/dev/null
tar_rc=$?
# Counts are read only from a tar that listed cleanly; an empty names file from a
# failed tar must not look like "zero secrets".
count_names() {
    if [ "$tar_rc" -eq 0 ]; then grep -c "$1" "$GOOD/names.txt"; else echo "erro-tar"; fi
}
expect_eq "tar limpo legivel" 0 "$tar_rc"
expect_eq "tar tem o CMakeLists.txt" 1 "$(count_names '^CMakeLists.txt$')"
expect_eq "tar tem o run_suite" 1 "$(count_names '^tools/tdd/run_suite.sh$')"
expect_eq "tar sem docs/_secret" 0 "$(count_names '^docs/')"

suite_done
