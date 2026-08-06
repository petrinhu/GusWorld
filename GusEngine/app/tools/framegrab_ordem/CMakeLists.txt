# SPDX-License-Identifier: Apache-2.0
# FRAMEGRAB-ORDEM - projeto CMake STANDALONE (nao e add_subdirectory do GusEngine:
# app/CMakeLists.txt so faz add_subdirectory(tests)). Subdiretorio PROPRIO (mesma forma de
# app/tools/glintfx_input_e2e/) pra nao tocar em app/tools/CMakeLists.txt, que esta com
# edicoes em voo de outro agente nesta mesma arvore.
#
# Nao compila NENHUMA camada da GusEngine: o probe usa glad header-only (GLAD_GL_IMPLEMENTATION
# na propria TU, MESMA receita de app/tools/msaa_glintfx_probe.cpp), entao so precisa de
# SDL3 + glintfx + RmlUi. IMPORTA os .a JA compilados pelo preset linux-release (mesma
# maquina/commit, pin glintfx v0.27.0) em vez de recompilar - le artefatos, nao reconfigura
# nem altera build/linux-release.
#
# Uso (sempre sob o lock de build da maquina):
#   export TMPDIR=/var/tmp
#   flock /var/tmp/gusworld-build.lock -c '
#     cmake -S GusEngine/app/tools/framegrab_ordem -B GusEngine/build/framegrab_ordem_scratch -G Ninja &&
#     cmake --build GusEngine/build/framegrab_ordem_scratch'
#   GusEngine/app/tools/framegrab_ordem/run_framegrab.sh   # NUNCA rodar o binario direto

cmake_minimum_required(VERSION 3.21)
project(framegrab_ordem_probe LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(_GUSENGINE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
set(_GUSENGINE_RELEASE "${_GUSENGINE_ROOT}/build/linux-release")

if(NOT EXISTS "${_GUSENGINE_RELEASE}/_deps/glintfx-build/libglintfx.a")
  message(FATAL_ERROR
      "framegrab_ordem_probe: build/linux-release ainda nao foi compilado (falta "
      "${_GUSENGINE_RELEASE}/_deps/glintfx-build/libglintfx.a) - rode 'cmake --preset "
      "linux-release && cmake --build --preset linux-release' na raiz do GusEngine primeiro.")
endif()

add_library(glintfx_imported STATIC IMPORTED)
set_target_properties(glintfx_imported PROPERTIES
    IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/_deps/glintfx-build/libglintfx.a")
add_library(rmlui_imported STATIC IMPORTED)
set_target_properties(rmlui_imported PROPERTIES
    IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/librmlui.a")
add_library(rmlui_debugger_imported STATIC IMPORTED)
set_target_properties(rmlui_debugger_imported PROPERTIES
    IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/librmlui_debugger.a")
add_library(sdl3_imported STATIC IMPORTED)
set_target_properties(sdl3_imported PROPERTIES
    IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/_deps/sdl3-build/libSDL3.a")

find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_package(Threads REQUIRED)

add_executable(framegrab_ordem_probe framegrab_ordem_probe.cpp)

target_include_directories(framegrab_ordem_probe PRIVATE
    # RmlUi_Include_GL3.h (glad GL3.3 core, HEADER_ONLY) - MESMO header que
    # platform/rmlui/gl3_loader.cpp usa em producao.
    "${_GUSENGINE_ROOT}/platform/rmlui"
    # stb_image_write.h (vendorizado, header-only) - grava os PNGs de evidencia.
    "${_GUSENGINE_ROOT}/third_party/stb"
    "${_GUSENGINE_RELEASE}/_deps/sdl3-build/include-revision"
    "${_GUSENGINE_RELEASE}/_deps/sdl3-src/include"
    # glintfx/ui_layer.hpp + config.hpp gerado (o alvo IMPORTED puro nao carrega
    # INTERFACE_INCLUDE_DIRECTORIES, entao os caminhos vao explicitos - MESMA nota de
    # app/tools/CMakeLists.txt).
    "${_GUSENGINE_RELEASE}/_deps/glintfx-src/glintfx/include"
    "${_GUSENGINE_RELEASE}/_deps/glintfx-build/include"
)

target_compile_features(framegrab_ordem_probe PRIVATE cxx_std_20)

# MESMA ordem de link (comprovadamente funcional) dos probes de app/tools/CMakeLists.txt,
# menos as 4 libs da GusEngine (este probe nao usa nenhuma camada da engine).
target_link_libraries(framegrab_ordem_probe PRIVATE
    sdl3_imported
    rmlui_debugger_imported
    glintfx_imported
    rmlui_imported
    OpenGL::GL
    Freetype::Freetype
    Threads::Threads
    ${CMAKE_DL_LIBS}
    m
)
