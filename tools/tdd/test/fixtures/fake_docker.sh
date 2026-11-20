#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Fake docker for the invoker tests. Logs every call to $FAKE_DOCKER_LOG and
# answers with the FAKE_* variables:
#   image ls                 -> $FAKE_IMAGE_LS (one reference per line)
#   image inspect (label)    -> $FAKE_IMAGE_DIGEST
#   image inspect (.Created) -> $FAKE_IMAGE_CREATED
#   image inspect (other)    -> sha256:$FAKE_IMAGE_ID
#   container inspect        -> the image id, the running flag, or the HostConfig line
#   exec                     -> stores stdin in $FAKE_DOCKER_STDIN, sleeps
#                               $FAKE_EXEC_SLEEP if set, prints $FAKE_EXEC_OUT,
#                               exits with $FAKE_EXEC_RC
# Every *_RC variable sets the exit code of its subcommand.
printf '%s\n' "$*" >> "${FAKE_DOCKER_LOG:?}"
case "$1" in
    info)
        exit "${FAKE_DOCKER_INFO_RC:-0}"
        ;;
    image)
        case "$2" in
            ls)
                printf '%s\n' "${FAKE_IMAGE_LS:-}"
                exit "${FAKE_IMAGE_LS_RC:-0}"
                ;;
            inspect)
                case "$*" in
                    *org.gusworld.base.digest*) printf '%s\n' "${FAKE_IMAGE_DIGEST:-<no value>}" ;;
                    *.Created*) printf '%s\n' "${FAKE_IMAGE_CREATED:-2026-11-20T17:31:18Z}" ;;
                    *) printf 'sha256:%s\n' "${FAKE_IMAGE_ID:-imagemfake}" ;;
                esac
                exit "${FAKE_IMAGE_RC:-0}"
                ;;
        esac
        ;;
    container)
        if [ "$2" = inspect ]; then
            case "$*" in
                *HostConfig*) printf '%s\n' "${FAKE_CTR_HOSTCONF:-none|0|0|false|4294967296|4294967296|233|4000000000|[ALL]|[]|[no-new-privileges]|builder||private|||0|0}" ;;
                *State.Running*) printf '%s\n' "${FAKE_CTR_RUNNING:-true}" ;;
                *) printf '%s\n' "${FAKE_CTR_IMAGE:-sha256:imagemfake}" ;;
            esac
            exit "${FAKE_CTR_RC:-0}"
        fi
        ;;
    start)
        exit 0
        ;;
    exec)
        cat > "${FAKE_DOCKER_STDIN:?}"
        if [ -n "${FAKE_EXEC_SLEEP:-}" ]; then
            sleep "$FAKE_EXEC_SLEEP"
        fi
        printf '%s\n' "${FAKE_EXEC_OUT:-}"
        exit "${FAKE_EXEC_RC:-0}"
        ;;
esac
exit 99
