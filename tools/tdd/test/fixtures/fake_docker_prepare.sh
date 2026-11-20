#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Fake docker for the prepare tests (ADR-026, decisions 3 and 6). Logs every call to
# $FAKE_DOCKER_LOG, with the arguments joined by spaces, and answers by subcommand:
#   info                          -> rc FAKE_DOCKER_INFO_RC
#   image ls / image inspect      -> as fake_docker.sh (FAKE_IMAGE_LS, FAKE_IMAGE_DIGEST, FAKE_IMAGE_ID)
#   container inspect             -> rc FAKE_CTR_RC (0 exists, 1 absent); prints the HostConfig,
#                                    the running flag or the image id, as fake_docker.sh does
#   create / start / cp           -> rc FAKE_CREATE_RC / FAKE_START_RC / FAKE_CP_RC; cp reads stdin
#   exec <ctr> <command>          -> by the command after the container name:
#       bash -c <script>          -> prints FAKE_EXEC_OUT, rc FAKE_EXEC_RC (the in_container run)
#       test -d /work             -> rc FAKE_WORK_RC
#       test -w /work             -> rc FAKE_WORK_W_RC
#       test -f /work/build/CMakeCache.txt -> rc FAKE_CACHE_RC
#       cmake -S ...              -> rc FAKE_CMAKE_RC
#       cmake --build ...         -> rc FAKE_BUILD_RC
#       id -u / id -g             -> 10001
# Defaults: every rc is 0, except FAKE_WORK_RC (1: /work absent) and FAKE_CACHE_RC
# (1: cold cache). Any other call exits 99, so an unexpected call is a visible failure.
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
                *HostConfig*) printf '%s\n' "${FAKE_CTR_HOSTCONF:-none|0|0|false|4294967296|4294967296|233|4000000000|[ALL]|0|[no-new-privileges]|builder||private|||0|0}" ;;
                *State.Running*) printf 'true\n' ;;
                *) printf '%s\n' "${FAKE_CTR_IMAGE:-sha256:imagemfake}" ;;
            esac
            exit "${FAKE_CTR_RC:-0}"
        fi
        ;;
    create)
        printf 'fakecontainerid\n'
        exit "${FAKE_CREATE_RC:-0}"
        ;;
    start)
        exit "${FAKE_START_RC:-0}"
        ;;
    cp)
        cat > /dev/null
        exit "${FAKE_CP_RC:-0}"
        ;;
    exec)
        shift
        if [ "${1:-}" = -i ]; then
            shift
            # The in_container run is the only exec that carries a tar on stdin.
            cat > "${FAKE_DOCKER_STDIN:-/dev/null}"
        fi
        shift
        case "$*" in
            "bash -c "*)
                printf '%s\n' "${FAKE_EXEC_OUT:-tdd-verdict: 3}"
                exit "${FAKE_EXEC_RC:-3}"
                ;;
            "test -d /work") exit "${FAKE_WORK_RC:-1}" ;;
            "test -w /work") exit "${FAKE_WORK_W_RC:-0}" ;;
            "test -f /work/build/CMakeCache.txt") exit "${FAKE_CACHE_RC:-1}" ;;
            "cmake -S "*) exit "${FAKE_CMAKE_RC:-0}" ;;
            "cmake --build "*) exit "${FAKE_BUILD_RC:-0}" ;;
            "id -u"|"id -g") printf '10001\n'; exit 0 ;;
        esac
        ;;
esac
exit 99
