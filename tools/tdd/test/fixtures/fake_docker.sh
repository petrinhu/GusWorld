#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Fake docker for the invoker tests. Logs every call to $FAKE_DOCKER_LOG and
# answers with the FAKE_* variables. On "exec" it stores its stdin (the tar) in
# $FAKE_DOCKER_STDIN, prints $FAKE_EXEC_OUT and exits with $FAKE_EXEC_RC.
printf '%s\n' "$*" >> "${FAKE_DOCKER_LOG:?}"
case "$1" in
    info)
        exit "${FAKE_DOCKER_INFO_RC:-0}"
        ;;
    image)
        printf 'sha256:%s\n' "${FAKE_IMAGE_ID:-imagemfake}"
        exit "${FAKE_IMAGE_RC:-0}"
        ;;
    container)
        if [ "$2" = inspect ]; then
            case "$*" in
                *'{{.Image}}'*) printf '%s\n' "${FAKE_CTR_IMAGE:-sha256:imagemfake}" ;;
                *) printf 'true\n' ;;
            esac
            exit "${FAKE_CTR_RC:-0}"
        fi
        ;;
    start)
        exit 0
        ;;
    exec)
        cat > "${FAKE_DOCKER_STDIN:?}"
        printf '%s\n' "${FAKE_EXEC_OUT:-}"
        exit "${FAKE_EXEC_RC:-0}"
        ;;
esac
exit 99
