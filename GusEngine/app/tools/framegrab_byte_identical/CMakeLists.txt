# FRAMEGRAB-7-SITIOS - projeto CMake STANDALONE (mesma forma de
# app/tools/framegrab_ordem/, motivo identico: nao tocar em app/tools/CMakeLists.txt).
#
# Diferenca em relacao a framegrab_ordem/CMakeLists.txt: este probe LINKA
# libgusengine_platform.a de verdade (gl3_load_functions/gl3_read_backbuffer_rgba
# PRODUCAO, gl3_loader.cpp - a UNICA TU dona de GLAD_GL_IMPLEMENTATION, ADR-010 F3) em
# vez de definir seu proprio glad - por isso NAO define GLAD_GL_IMPLEMENTATION na propria
# TU (framegrab_byte_identical_probe.cpp so DECLARA via RmlUi_Include_GL3.h sem a macro;
# a definicao vem do .o de gl3_loader.cpp dentro do .a, resolvida pelo linker).
#
# Uso (sempre sob o lock de build da maquina):
#   export TMPDIR=/var/tmp
#   flock /var/tmp/gusworld-build.lock -c '
#     cmake -S GusEngine/app/tools/framegrab_byte_identical -B GusEngine/build/framegrab_byte_identical_scratch -G Ninja &&
#     cmake --build GusEngine/build/framegrab_byte_identical_scratch'
#   GusEngine/app/tools/framegrab_byte_identical/run_framegrab_byte_identical.sh

cmake_minimum_required(VERSION 3.21)
project(framegrab_byte_identical_probe LANGUAGES C CXX)

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
      "framegrab_byte_identical_probe: build/linux-release ainda nao foi compilado (falta "
      "${_GUSENGINE_RELEASE}/_deps/glintfx-build/libglintfx.a) - rode 'cmake --preset "
      "linux-release && cmake --build --preset linux-release' na raiz do GusEngine primeiro.")
endif()
if(NOT EXISTS "${_GUSENGINE_RELEASE}/platform/libgusengine_platform.a")
  message(FATAL_ERROR
      "framegrab_byte_identical_probe: falta ${_GUSENGINE_RELEASE}/platform/libgusengine_platform.a "
      "- rode o build completo do preset linux-release primeiro.")
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
# gusengine_platform.a: SO o .o de gl3_loader.cpp e puxado (link estatico resolve por
# simbolo, nao arquiva inteiro) - gl3_load_functions()/gl3_read_backbuffer_rgba(),
# zero outra dependencia da lib (audio/render2d/etc. ficam de fora, nao referenciadas).
add_library(gusengine_platform_imported STATIC IMPORTED)
set_target_properties(gusengine_platform_imported PROPERTIES
    IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/platform/libgusengine_platform.a")

find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_package(Threads REQUIRED)

add_executable(framegrab_byte_identical_probe framegrab_byte_identical_probe.cpp)

target_include_directories(framegrab_byte_identical_probe PRIVATE
    # RmlUi_Include_GL3.h (glad GL3.3 core, so DECLARACOES aqui - ver comentario acima)
    "${_GUSENGINE_ROOT}/platform/rmlui"
    # gus/platform/rmlui/gl3_loader.hpp (header limpo, sem GL/glad)
    "${_GUSENGINE_ROOT}/platform/include"
    "${_GUSENGINE_ROOT}/third_party/stb"
    "${_GUSENGINE_RELEASE}/_deps/sdl3-build/include-revision"
    "${_GUSENGINE_RELEASE}/_deps/sdl3-src/include"
    "${_GUSENGINE_RELEASE}/_deps/glintfx-src/glintfx/include"
    "${_GUSENGINE_RELEASE}/_deps/glintfx-build/include"
)

target_compile_features(framegrab_byte_identical_probe PRIVATE cxx_std_20)

# gusengine_platform_imported ANTES de rmlui/glintfx (resolve gl3_load_functions/
# gl3_read_backbuffer_rgba primeiro; ordem de link estatico importa no ld do Linux).
target_link_libraries(framegrab_byte_identical_probe PRIVATE
    gusengine_platform_imported
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
