# mingw-w64.cmake - toolchain de cross-compile Linux -> Windows x86_64 (MinGW-w64).
#
# WIN-CROSS-VALIDATE (2026-07-14): usado para validar, DENTRO de um container
# efemero (imagem ghcr.io/catthehacker/ubuntu:act-latest, a mesma da CI Forgejo
# local "claudio"), que o GusEngine (C++20/SDL3) cross-compila pra Windows sem
# tocar o host. Ver GusEngine/tools/winbuild_container.sh (script reprodutivel
# que consome este toolchain) e o preset "windows-mingw" em CMakePresets.json.
#
# -posix (NAO -win32): o par -posix dos pacotes Ubuntu
# g++-mingw-w64-x86-64-posix/gcc-mingw-w64-x86-64-posix traz winpthreads, exigido
# por std::thread/std::mutex/std::condition_variable (usados no GusEngine). O par
# -win32 (default do alternatives em algumas imagens) NAO tem threading POSIX e
# falha ao linkar <thread>. Fixar aqui em vez de confiar no `update-alternatives`
# do container evita variar por imagem.
#
# CMAKE_FIND_ROOT_PATH: raiz de sysroot MinGW (/usr/x86_64-w64-mingw32, onde o
# Ubuntu poe as libs mingw) + WINDEPS_PREFIX (env var setada pelo script de build
# ANTES do configure do GusEngine, apontando pro prefixo onde o FreeType
# cross-compilado from-source foi instalado - Ubuntu NAO empacota freetype-mingw,
# entao RmlUi/find_package(Freetype) so acha se WINDEPS_PREFIX estiver no path).
#
# MODE_PROGRAM NEVER: permite usar binarios do HOST (cmake, ninja, git) durante o
# configure/build, mas MODE_LIBRARY/INCLUDE/PACKAGE ficam ONLY (so acham libs e
# headers dentro do sysroot MinGW, nunca vazam pra libs Linux do host - proteje
# contra falso-positivo de link, ex. achar uma libfoo.so do host por engano).

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER   x86_64-w64-mingw32-gcc-posix)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++-posix)
set(CMAKE_RC_COMPILER  x86_64-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32 $ENV{WINDEPS_PREFIX})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
