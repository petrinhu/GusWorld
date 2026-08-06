# SPDX-License-Identifier: Apache-2.0
# F4-2.5 - CMake SCRATCH proprio (TRACKED, ao contrario de ../appmode_spike/, que e
# scratch/nao-commitado). NAO e add_subdirectory do GusEngine/CMakeLists.txt
# principal nem do ../CMakeLists.txt (aquele forca GLINTFX_BACKEND_GLFW=OFF, que e
# INCOMPATIVEL com glintfx::App - app.hpp linha 7, #error caso contrario).
#
# DIFERENCA vs appmode_spike/CMakeLists.txt (MESMA tecnica de reuso de fontes/import
# de .a, ver o comentario de topo la para o "porque" de cada peca): este probe NAO
# usa OverworldSim/city/SDL3/monocypher (fora do escopo - "prova de vida do INPUT",
# nao da cena) e, principalmente, NAO linka glfw diretamente nem inclui
# <GLFW/glfw3.h> - a partir do pin v0.20.0 (GusEngine/CMakeLists.txt) o
# glintfx::App JA EXPOE App::is_key_down(Key) publicamente (HOSTIN-1), fechando o
# GAP-1 que o appmode_spike documentou (workaround GLFW cru via
# glfwGetCurrentContext/glfwGetKey). Import de gusengine_platform_imported/
# domain_imported/core_imported (JA compilados pelo preset linux-release, GLFW=OFF -
# seguro: glintfx_input.cpp/key_translation_glintfx.cpp so usam glintfx::Key como
# TIPO, header-only, nunca referenciam simbolo nenhum de App/GLFW) + glintfx/RmlUi
# compilados FRESCOS aqui com GLFW=ON.
#
# SABOTAGEM (prova de "tem dente", ver report da fatia): key_translation_glintfx.cpp
# e adicionado como fonte PROPRIA do executavel (NAO so importado via
# gusengine_platform_imported) - o objeto compilado aqui, fresco, sombreia o mesmo
# simbolo dentro do .a importado (o linker so extrai um membro de arquivo pra
# resolver simbolo AINDA indefinido; como o nosso proprio objeto ja define
# glintfx_key_to_godot_keycode/godot_keycode_to_glintfx_key/glintfx_capture_key, o
# membro key_translation_glintfx.cpp.o do .a nunca chega a ser puxado). Isto permite
# editar o arquivo TRACKED de verdade (platform/src/input/key_translation_glintfx.cpp),
# rebuildar SO este probe (rapido, escopo scratch), rodar o roteiro sabotado, e
# depois `git checkout --` pra restaurar - sem tocar/rebuildar build/linux-release.
#
# Uso:
#   cmake --preset linux-release && cmake --build --preset linux-release   # 1x (pre-requisito)
#   export TMPDIR=/var/tmp   # build pesado - NAO usar tmpfs (/tmp), ver memoria do lider
#   cmake -S GusEngine/app/tools/glintfx_input_e2e -B GusEngine/build/glintfx_input_e2e_scratch -G Ninja
#   cmake --build GusEngine/build/glintfx_input_e2e_scratch
#   (rodar via run_e2e.sh deste diretorio - NUNCA a mao, ver secao de seguranca la)

cmake_minimum_required(VERSION 3.21)
project(glintfx_input_e2e_probe LANGUAGES C CXX)

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

if(NOT EXISTS "${_GUSENGINE_RELEASE}/platform/libgusengine_platform.a")
    message(FATAL_ERROR
        "glintfx_input_e2e_probe: build/linux-release ainda nao foi compilado (falta "
        "${_GUSENGINE_RELEASE}/platform/libgusengine_platform.a) - rode 'cmake "
        "--preset linux-release && cmake --build --preset linux-release' na raiz do "
        "GusEngine primeiro.")
endif()
if(NOT EXISTS "${_GUSENGINE_DEPS}/rmlui-src")
    message(FATAL_ERROR
        "glintfx_input_e2e_probe: ${_GUSENGINE_DEPS}/rmlui-src nao existe - o preset "
        "linux-release precisa ter sido configurado/buildado ao menos 1x (fetcha "
        "RmlUi/glintfx offline-reusaveis daqui pra frente).")
endif()
if(NOT EXISTS "${_GUSENGINE_DEPS}/glintfx-src")
    message(FATAL_ERROR
        "glintfx_input_e2e_probe: ${_GUSENGINE_DEPS}/glintfx-src nao existe (idem "
        "rmlui-src acima).")
endif()

# --- RmlUi (SOURCE_DIR reuse, offline). MESMA tecnica/ordem de appmode_spike/CMakeLists.txt.
set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_FONT_ENGINE "freetype" CACHE STRING "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(RmlUi SOURCE_DIR "${_GUSENGINE_DEPS}/rmlui-src")
FetchContent_MakeAvailable(RmlUi)

# --- glintfx (SOURCE_DIR reuse, offline). GLINTFX_BACKEND_GLFW=ON e OBRIGATORIO
# (glintfx::App exige - app.hpp linha 7, #error caso contrario).
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

find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_package(Threads REQUIRED)

# --- .a JA COMPILADOS pelo preset linux-release (producao, GLFW=OFF) - so as 3
# camadas POCO/platform de INPUT precisam disto (nenhuma referencia App/GLFW/SDL).
add_library(gusengine_platform_imported STATIC IMPORTED)
set_target_properties(gusengine_platform_imported PROPERTIES
    IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/platform/libgusengine_platform.a")
add_library(gusengine_domain_imported STATIC IMPORTED)
set_target_properties(gusengine_domain_imported PROPERTIES
    IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/domain/libgusengine_domain.a")
add_library(gusengine_core_imported STATIC IMPORTED)
set_target_properties(gusengine_core_imported PROPERTIES
    IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/core/libgusengine_core.a")

add_executable(glintfx_input_e2e_probe
    glintfx_input_e2e_probe.cpp
    # SABOTAGEM (ver comentario de topo deste arquivo): fonte PROPRIA, sombreia o
    # mesmo simbolo dentro do .a importado - editar+rebuildar SO este alvo exercita
    # a traducao de tecla de verdade sem tocar build/linux-release.
    "${_GUSENGINE_ROOT}/platform/src/input/key_translation_glintfx.cpp"
)

target_include_directories(glintfx_input_e2e_probe PRIVATE
    "${_GUSENGINE_ROOT}/core/include"
    "${_GUSENGINE_ROOT}/domain/include"
    "${_GUSENGINE_ROOT}/platform/include"
)

target_compile_features(glintfx_input_e2e_probe PRIVATE cxx_std_20)

target_link_libraries(glintfx_input_e2e_probe PRIVATE
    gusengine_platform_imported
    gusengine_domain_imported
    gusengine_core_imported
    glintfx::glintfx
    OpenGL::GL
    Freetype::Freetype
    Threads::Threads
    ${CMAKE_DL_LIBS}
    m
)
