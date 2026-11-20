# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Shared tail of the CI environment scripts (fedora.sh, ubuntu.sh, arch.sh).
# Sourced with `.` by each distro script after its install step; it is not run
# on its own. It does three things and nothing else:
#   1. default CC and CXX to cc and c++, and export them with CMAKE_GENERATOR=Ninja
#      to the next steps via $GITHUB_ENV;
#   2. print /etc/os-release and the tool versions, so the CI log proves what ran.
#
# The compiler comes from the workflow matrix when the workflow has one (the
# build job sets CC and CXX per entry, GCC or Clang). A workflow without a
# matrix gets the default: cc and c++. Set CC and CXX in the environment to
# override it.
#
# It inherits `set -eu` from the calling script and sets none of its own.

CC="${CC:-cc}"
CXX="${CXX:-c++}"

{
    echo "CC=${CC}"
    echo "CXX=${CXX}"
    echo "CMAKE_GENERATOR=Ninja"
} >> "$GITHUB_ENV"

cat /etc/os-release
"$CC" --version
"$CXX" --version
cmake --version
ninja --version
git --version
