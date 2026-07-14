# PROBE EFEMERO (app/tools/ e UNTRACKED - nada aqui e commitado). Projeto CMake
# STANDALONE (nao e add_subdirectory do GusEngine/CMakeLists.txt principal - nao toca
# em NENHUM arquivo tracked) so para buildar o msaa_glintfx_probe.
#
# Reusa os fontes JA FETCHADOS pelo preset linux-release (SOURCE_DIR override no
# FetchContent_Declare pula o clone git inteiramente - offline, instantaneo) mas
# compila os objetos NUMA binaryDir propria (nao reusa os .a do linux-release porque
# misturar arvores de build de 2 CMake projects diferentes e fragil). Mesmos pins/opcoes
# do GusEngine/CMakeLists.txt (SDL3 release-3.4.10, RmlUi SHA do glintfx, glintfx v0.9.0
# GLINTFX_BACKEND_GLFW=OFF) para ABI identica a producao.
#
# Uso:
#   cmake -S GusEngine/app/tools -B GusEngine/build/msaa_probe_scratch -G Ninja
#   cmake --build GusEngine/build/msaa_probe_scratch --target msaa_glintfx_probe

cmake_minimum_required(VERSION 3.21)
project(msaa_glintfx_probe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)

# Raiz do checkout linux-release ja fetchado (mesma maquina, mesmo commit) - reusa fonte,
# NAO binario (cada projeto CMake tem sua propria binaryDir).
set(_GUSENGINE_DEPS "${CMAKE_CURRENT_SOURCE_DIR}/../../build/linux-release/_deps")

set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    SDL3
    SOURCE_DIR "${_GUSENGINE_DEPS}/sdl3-src"
)
FetchContent_MakeAvailable(SDL3)

set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_FONT_ENGINE "freetype" CACHE STRING "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    RmlUi
    SOURCE_DIR "${_GUSENGINE_DEPS}/rmlui-src"
)
FetchContent_MakeAvailable(RmlUi)

set(GLINTFX_BACKEND_GLFW OFF CACHE BOOL "" FORCE)  # embed-only, MESMA config da producao
FetchContent_Declare(
    glintfx
    SOURCE_DIR "${_GUSENGINE_DEPS}/glintfx-src"
    SOURCE_SUBDIR glintfx
)
FetchContent_MakeAvailable(glintfx)

add_executable(msaa_glintfx_probe msaa_glintfx_probe.cpp)

target_include_directories(msaa_glintfx_probe PRIVATE
    # RmlUi_Include_GL3.h (glad GL3.3 core vendorizado) - MESMO header que
    # platform/rmlui/gl3_loader.cpp usa em producao, so pra carregar os ponteiros GL
    # que o PROBIE usa (backdrop clear + resolve-blit MSAA->non-MSAA + glReadPixels).
    # O glintfx tem SEU PROPRIO loader interno (gl3w/glloader) - zero conflito de simbolo
    # (ver comentario em platform/rmlui/gl3_loader.cpp).
    "${CMAKE_CURRENT_SOURCE_DIR}/../../platform/rmlui"
    # stb_image_write.h (vendorizado, header-only) - grava os 3 PNGs de evidencia.
    "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/stb"
)

target_link_libraries(msaa_glintfx_probe PRIVATE SDL3::SDL3 glintfx::glintfx)
target_compile_features(msaa_glintfx_probe PRIVATE cxx_std_20)
