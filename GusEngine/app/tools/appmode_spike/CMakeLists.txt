# SPDX-License-Identifier: Apache-2.0
# PROBE de EXPERIMENTO (app/tools/appmode_spike/ - scratch por proposito, mas
# VERSIONADO desde 2026-08-06/CI-RODA-OS-GATES: ficou untracked por dias e era a
# unica evidencia da F3 de GLINTFX-INTEGRACAO, que a auditoria da W1 julgou
# NAO-AUDITAVEL justamente porque o codigo so existia fora do git e os diretorios
# de evidencia tinham sido apagados. Commitar e impedir perda; o irmao
# ../framegrab_ordem/ e ../glintfx_input_e2e/ ja seguiam esse padrao). NAO
# add_subdirectory do GusEngine/CMakeLists.txt principal, NAO add_subdirectory do
# app/tools/CMakeLists.txt existente - aquele forca GLINTFX_BACKEND_GLFW=OFF, que e
# INCOMPATIVEL com glintfx::App (app.hpp linha 7: #error se BACKEND_GLFW!=ON).
#
# F3-SPIKE-1 (GLINTFX-INTEGRACAO-F3 fatia 1): prova DESCARTAVEL de App mode do
# glintfx v0.18.0. Nada aqui toca producao (nenhum arquivo tracked e editado).
#
# DIFERENCA-CHAVE vs os probes irmaos (poc_gl_single_context_probe.cpp e afins, ver
# ../CMakeLists.txt): aqueles IMPORTAM o .a JA COMPILADO do glintfx em
# GLINTFX_BACKEND_GLFW=OFF (embed-only, config de producao). Aqui o glintfx (E o
# RmlUi que ele fixa por baixo) sao compilados FROM-SOURCE NESTE projeto, com a flag
# ON, numa binaryDir propria (GusEngine/build/appmode_spike_scratch) - reusando os
# fontes JA FETCHADOS pelo preset linux-release via SOURCE_DIR (offline, sem
# re-clone), MESMA tecnica de reuso que ../CMakeLists.txt (msaa_glintfx_probe) usa,
# so com GLINTFX_BACKEND_GLFW=ON no lugar de OFF.
#
# O resto da engine (gusengine_app/platform/domain/core, monocypher, SDL3) e
# IMPORTADO dos .a JA COMPILADOS pelo preset linux-release, EXATAMENTE como
# poc_gl_single_context_probe faz (../CMakeLists.txt linhas 93-168) - MENOS o
# glintfx e o RmlUi importados de la (glintfx_imported, rmlui_imported,
# rmlui_debugger_imported): aqueles vieram da build embed-only (GLFW=OFF) de
# producao e colidiriam em simbolo com o RmlUi::Core/glintfx GLFW=ON compilados
# fresco aqui (2 copias do MESMO RmlUi::Core linkadas juntas no mesmo binario).
# SDL3 SEGUE importado do .a de producao sem problema (nosso .cpp nunca chama SDL_*
# nem inclui headers SDL3 - so os .a de producao referenciam SDL3 internamente).
#
# glfw (system, glfw-devel) e adicionado como dependencia PROPRIA (nao so via
# glintfx::glintfx) porque o probe chama as funcoes GLOBAIS
# glfwGetProcAddress/glfwGetCurrentContext/glfwGetKey/glfwGetCursorPos diretamente -
# o WORKAROUND documentado no relatorio da onda pro GAP "API publica do App nao tem
# canal de input pro host".
#
# Uso:
#   cmake --preset linux-release && cmake --build --preset linux-release   # 1x (pre-requisito)
#   export TMPDIR=/var/tmp   # build pesado - NAO usar tmpfs (/tmp), ver memoria do lider
#   cmake -S GusEngine/app/tools/appmode_spike -B GusEngine/build/appmode_spike_scratch -G Ninja
#   cmake --build GusEngine/build/appmode_spike_scratch
#   DISPLAY=:99 GusEngine/build/appmode_spike_scratch/appmode_spike_probe

cmake_minimum_required(VERSION 3.21)
project(appmode_spike_probe LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)

set(_GUSENGINE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
set(_GUSENGINE_RELEASE "${_GUSENGINE_ROOT}/build/linux-release")
set(_GUSENGINE_DEPS "${_GUSENGINE_RELEASE}/_deps")

if(NOT EXISTS "${_GUSENGINE_RELEASE}/app/libgusengine_app.a")
    message(FATAL_ERROR
        "appmode_spike_probe: build/linux-release ainda nao foi compilado (falta "
        "${_GUSENGINE_RELEASE}/app/libgusengine_app.a) - rode 'cmake --preset "
        "linux-release && cmake --build --preset linux-release' na raiz do GusEngine "
        "primeiro.")
endif()
if(NOT EXISTS "${_GUSENGINE_DEPS}/rmlui-src")
    message(FATAL_ERROR
        "appmode_spike_probe: ${_GUSENGINE_DEPS}/rmlui-src nao existe - o preset "
        "linux-release precisa ter sido configurado/buildado ao menos 1x (fetcha "
        "RmlUi/glintfx offline-reusaveis daqui pra frente).")
endif()
if(NOT EXISTS "${_GUSENGINE_DEPS}/glintfx-src")
    message(FATAL_ERROR
        "appmode_spike_probe: ${_GUSENGINE_DEPS}/glintfx-src nao existe (idem "
        "rmlui-src acima).")
endif()

# --- RmlUi (SOURCE_DIR reuse, offline). DECLARADO ANTES do glintfx (idioma
# FetchContent by-name-wins, MESMO truque do GusEngine/CMakeLists.txt principal,
# linhas 93-103/105-121): a nossa Declare()/MakeAvailable() abaixo e quem POPULA e
# COMPILA a arvore RmlUi (fresca, nesta binaryDir propria - NAO reusa objetos do
# preset linux-release); quando o FetchContent_Declare(RmlUi ...) INTERNO do
# glintfx/CMakeLists.txt rodar mais abaixo (via FetchContent_MakeAvailable(glintfx)),
# vira no-op (mesmo nome "RmlUi" ja populado por nos - fonte ja e o MESMO SOURCE_DIR,
# ja patcheado pelo PATCH_COMMAND do preset linux-release na 1a vez que aquele
# preset rodou).
set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_FONT_ENGINE "freetype" CACHE STRING "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(RmlUi SOURCE_DIR "${_GUSENGINE_DEPS}/rmlui-src")
FetchContent_MakeAvailable(RmlUi)

# --- glintfx (SOURCE_DIR reuse, offline). GLINTFX_BACKEND_GLFW=ON e OBRIGATORIO
# (glintfx::App exige - app.hpp linha 7, #error caso contrario). Mesmos
# GLINTFX_MODULE_TEXT/FX/OWN_FONT_ENGINE de producao (GusEngine/CMakeLists.txt
# linhas 177-182, paridade deliberada); AUDIO/GAMEPAD/I18N ficam OFF aqui (nao
# usados por este probe - reduz escopo de compilacao dentro do timebox da onda).
set(GLINTFX_BACKEND_GLFW    ON  CACHE BOOL "" FORCE)
set(GLINTFX_MODULE_TEXT     ON  CACHE BOOL "" FORCE)
set(GLINTFX_MODULE_FX       ON  CACHE BOOL "" FORCE)
set(GLINTFX_MODULE_AUDIO    OFF CACHE BOOL "" FORCE)
set(GLINTFX_MODULE_GAMEPAD  OFF CACHE BOOL "" FORCE)
set(GLINTFX_MODULE_I18N     OFF CACHE BOOL "" FORCE)
set(GLINTFX_OWN_FONT_ENGINE OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glintfx
    SOURCE_DIR "${_GUSENGINE_DEPS}/glintfx-src"
    SOURCE_SUBDIR glintfx
)
FetchContent_MakeAvailable(glintfx)

# find_package PROPRIO (nao basta o find_package interno do glintfx/CMakeLists.txt -
# os targets IMPORTED que ele cria via find_package/config-mode, ex. o `glfw` de
# glfw3Config.cmake via add_library(glfw SHARED IMPORTED) SEM GLOBAL, so ficam
# visiveis na propria arvore de diretorio do glintfx + subdiretorios DELE - nao no
# NOSSO escopo de diretorio, que e o PAI daquela subarvore. Precisamos dos nossos
# proprios targets pra poder referenciar `glfw`/OpenGL::GL/Freetype::Freetype no
# nosso proprio target_link_libraries abaixo. RmlUi::Core e glintfx::glintfx NAO
# tem este problema - sao targets REGULARES (nao IMPORTED), visibilidade global no
# projeto CMake inteiro por construcao.
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_package(Threads REQUIRED)

# --- Os .a JA COMPILADOS pelo preset linux-release (producao) - MESMA receita de
# poc_gl_single_context_probe (../CMakeLists.txt linhas 96-123), MENOS
# rmlui_imported/rmlui_debugger_imported/glintfx_imported (ver comentario de topo
# deste arquivo pro motivo).
add_library(gusengine_app_imported STATIC IMPORTED)
set_target_properties(gusengine_app_imported PROPERTIES
    IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/app/libgusengine_app.a")
add_library(gusengine_platform_imported STATIC IMPORTED)
set_target_properties(gusengine_platform_imported PROPERTIES
    IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/platform/libgusengine_platform.a")
add_library(gusengine_domain_imported STATIC IMPORTED)
set_target_properties(gusengine_domain_imported PROPERTIES
    IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/domain/libgusengine_domain.a")
add_library(gusengine_core_imported STATIC IMPORTED)
set_target_properties(gusengine_core_imported PROPERTIES
    IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/core/libgusengine_core.a")
add_library(monocypher_imported STATIC IMPORTED)
set_target_properties(monocypher_imported PROPERTIES
    IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/third_party/monocypher/libmonocypher.a")
add_library(sdl3_imported STATIC IMPORTED)
set_target_properties(sdl3_imported PROPERTIES
    IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/_deps/sdl3-build/libSDL3.a")

add_executable(appmode_spike_probe appmode_spike_probe.cpp)

target_include_directories(appmode_spike_probe PRIVATE
    "${_GUSENGINE_ROOT}/core/include"
    "${_GUSENGINE_ROOT}/domain/include"
    "${_GUSENGINE_ROOT}/platform/include"
    "${_GUSENGINE_ROOT}/app/include"
    # RmlUi_Include_GL3.h (glad GL3.3 core vendorizado PROPRIO do GusEngine, nao o
    # backend do RmlUi) - MESMA tecnica dos probes irmaos (../CMakeLists.txt linhas
    # 63-67/137-139): so pra termos glGetError/GL_NO_ERROR disponiveis. NAO define
    # GLAD_GL_IMPLEMENTATION aqui (a impl mora dentro de gusengine_platform_imported,
    # ver platform/rmlui/gl3_loader.cpp) - so declaracoes, zero simbolo duplicado.
    "${_GUSENGINE_ROOT}/platform/rmlui"
)

target_compile_features(appmode_spike_probe PRIVATE cxx_std_20)

# MESMAS macros de app/CMakeLists.txt (gusengine_app) - usadas como fallback quando
# GUSWORLD_ASSETS/GUSWORLD_FONTS nao estao setadas no env (MESMA receita dos probes
# irmaos, ../CMakeLists.txt linhas 249-252).
target_compile_definitions(appmode_spike_probe PRIVATE
    GUSWORLD_ASSETS_DIR="${_GUSENGINE_ROOT}/../resources"
    GUSWORLD_FONTS_DIR="${_GUSENGINE_ROOT}/assets/fonts"
)

# Ordem de link: os .a de producao PRIMEIRO (MESMA ordem/duplicacao comprovada dos
# probes irmaos - resolve refs circulares no link estatico de arquivo unico, sem
# --start-group/--end-group, ver ../CMakeLists.txt linhas 146-150) + glintfx/glfw NO
# FIM (glintfx::glintfx e um alvo REAL compilado fresco aqui - nao IMPORTED - entao
# o CMake ja conhece o grafo transitivo dele (RmlUi::Core/OpenGL::GL/
# Freetype::Freetype/glfw ja vao juntos automaticamente); ainda assim listamos
# OpenGL/Freetype/Threads explicitamente por defesa-em-profundidade, MESMO habito
# dos probes irmaos com os alvos IMPORTED).
target_link_libraries(appmode_spike_probe PRIVATE
    gusengine_app_imported
    gusengine_core_imported
    gusengine_domain_imported
    gusengine_platform_imported
    sdl3_imported
    gusengine_domain_imported
    gusengine_core_imported
    monocypher_imported
    glintfx::glintfx
    glfw
    OpenGL::GL
    Freetype::Freetype
    Threads::Threads
    ${CMAKE_DL_LIBS}
    m
)
