#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Proves that take_heavy_slot refuses a planted lock BEFORE it opens the slot (review of
# the heavy_slot finding, 2026-11-20). Runs inside the container only (GW-35): the
# planted links live in the /var/tmp of the container, never in the host's. Each take
# runs in its own bash process, so a slot taken by one case is released when it exits.
set -u

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
heavy_slot="$here/../heavy_slot.sh"
cases=0
fails=0

pass_case() {
    cases=$((cases + 1))
    echo "ok    $1"
}

fail_case() {
    cases=$((cases + 1))
    fails=$((fails + 1))
    echo "FALHA $1${2:+: $2}"
}

# take_once: one take in a fresh process. Sets rc and err (stdout and stderr together).
take_once() {
    err="$(bash -c '. "$1"; take_heavy_slot' _ "$heavy_slot" 2>&1)"
    rc=$?
}

# clean_locks: removes the three slot names of this container, by absolute literal path.
clean_locks() {
    rm -f -- /var/tmp/gusworld-heavy-slot-1.lock /var/tmp/gusworld-heavy-slot-2.lock \
        /var/tmp/gusworld-heavy-slot-3.lock
}

# A. No lock yet: the first slot is taken.
clean_locks
take_once
if [ "$rc" -eq 0 ] && [[ "$err" == *"slot 1 of 3 taken"* ]]; then
    pass_case "sem lock previo, o slot 1 e tomado (rc 0)"
else
    fail_case "sem lock previo, o slot 1 e tomado (rc 0)" "rc $rc, err [$err]"
fi

# B. Own regular lock already there: accepted, the slot is taken.
clean_locks
: > /var/tmp/gusworld-heavy-slot-1.lock
take_once
if [ "$rc" -eq 0 ] && [[ "$err" == *"slot 1 of 3 taken"* ]]; then
    pass_case "lock regular do proprio uid e aceito (rc 0)"
else
    fail_case "lock regular do proprio uid e aceito (rc 0)" "rc $rc, err [$err]"
fi

# C. A symbolic link at the lock path, pointing to a file that does not exist yet.
# The refusal must come before exec, so the target must NOT be created.
clean_locks
ln -s /var/tmp/heavy_slot_victim_absent /var/tmp/gusworld-heavy-slot-1.lock
take_once
if [ "$rc" -ne 0 ] && [[ "$err" == *"REFUSED"* ]] && [ ! -e /var/tmp/heavy_slot_victim_absent ]; then
    pass_case "link simbolico no lock e recusado antes do exec (rc nao zero, alvo intocado)"
else
    fail_case "link simbolico no lock e recusado antes do exec (rc nao zero, alvo intocado)" \
        "rc $rc, err [$err], alvo existe: $([ -e /var/tmp/heavy_slot_victim_absent ] && echo sim || echo nao)"
fi
clean_locks

# D. A symbolic link to a file that exists: refused as well, and the file is not touched.
clean_locks
: > /var/tmp/heavy_slot_victim_present
ln -s /var/tmp/heavy_slot_victim_present /var/tmp/gusworld-heavy-slot-1.lock
take_once
if [ "$rc" -ne 0 ] && [[ "$err" == *"REFUSED"* ]]; then
    pass_case "link simbolico para arquivo existente e recusado (rc nao zero)"
else
    fail_case "link simbolico para arquivo existente e recusado (rc nao zero)" "rc $rc, err [$err]"
fi
clean_locks
rm -f -- /var/tmp/heavy_slot_victim_present

# E. A directory at the lock path is not a regular file: refused with the reason.
clean_locks
mkdir -- /var/tmp/gusworld-heavy-slot-1.lock
take_once
if [ "$rc" -ne 0 ] && [[ "$err" == *"REFUSED"* ]]; then
    pass_case "diretorio no lock e recusado com motivo (rc nao zero)"
else
    fail_case "diretorio no lock e recusado com motivo (rc nao zero)" "rc $rc, err [$err]"
fi
rmdir -- /var/tmp/gusworld-heavy-slot-1.lock 2>/dev/null
clean_locks

# F. A regular lock owned by another uid is refused. The container has no file of another
# uid it can hardlink into /var/tmp (the link crosses devices, and protected_hardlinks
# refuses the root files of the same device), so the owner is reported by a test double:
# a stat in PATH that answers uid 0 only for the %u query. Every other call is the real stat.
clean_locks
: > /var/tmp/gusworld-heavy-slot-1.lock
real_stat="$(command -v stat)"
shim="$(mktemp -d)"
cat > "$shim/stat" <<EOF
#!/usr/bin/env bash
for a in "\$@"; do [ "\$a" = '%u' ] && { echo 0; exit 0; }; done
exec "$real_stat" "\$@"
EOF
chmod 700 "$shim/stat"
err="$(PATH="$shim:$PATH" bash -c '. "$1"; take_heavy_slot' _ "$heavy_slot" 2>&1)"
rc=$?
if [ "$rc" -ne 0 ] && [[ "$err" == *"REFUSED"* ]] && [[ "$err" == *"belongs to uid 0"* ]]; then
    pass_case "lock regular de outro uid e recusado (rc nao zero, motivo com o uid)"
else
    fail_case "lock regular de outro uid e recusado (rc nao zero, motivo com o uid)" "rc $rc, err [$err]"
fi
clean_locks

if [ "$cases" -eq 0 ]; then
    echo "FALHA nenhum caso rodou"
    exit 1
fi
echo "heavy_slot: $cases casos, $fails falhas"
[ "$fails" -eq 0 ]
