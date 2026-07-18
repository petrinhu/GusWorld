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

# =============================================================================
# poc_gl_single_context_probe (FLASH-CTX passo 1, GATE - ver docs/tech/pivot/
# menu-flash-contexto-unico-plano.md secao 5 passo 1). Diferente do
# msaa_glintfx_probe acima (que so precisa de SDL3+glintfx crus), este probe
# chama gus::app::screens::OverworldSim/load_city_or_fallback/
# run_system_menu_loop_gl_current de VERDADE - precisa das 4 camadas da engine
# (gusengine_core/domain/platform/app). Em vez de RECOMPILAR tudo aqui (pesado,
# duplicaria a arvore inteira), IMPORTA os .a JA COMPILADOS pelo preset
# linux-release (mesma maquina/commit - ISOLADO: le artefatos, nao reconfigura
# nem altera build/linux-release). Se o preset ainda nao foi buildado, este
# target falha ao configurar com uma mensagem clara (pre-requisito documentado).
#
# Uso:
#   cmake --preset linux-release && cmake --build --preset linux-release   # 1x
#   cmake -S GusEngine/app/tools -B GusEngine/build/msaa_probe_scratch -G Ninja
#   cmake --build GusEngine/build/msaa_probe_scratch --target poc_gl_single_context_probe
#   DISPLAY=:99 GusEngine/build/msaa_probe_scratch/poc_gl_single_context_probe
set(_GUSENGINE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(_GUSENGINE_RELEASE "${_GUSENGINE_ROOT}/build/linux-release")

if(EXISTS "${_GUSENGINE_RELEASE}/app/libgusengine_app.a")
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
    add_library(rmlui_imported STATIC IMPORTED)
    set_target_properties(rmlui_imported PROPERTIES
        IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/librmlui.a")
    add_library(rmlui_debugger_imported STATIC IMPORTED)
    set_target_properties(rmlui_debugger_imported PROPERTIES
        IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/librmlui_debugger.a")
    add_library(glintfx_imported STATIC IMPORTED)
    set_target_properties(glintfx_imported PROPERTIES
        IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/_deps/glintfx-build/libglintfx.a")
    add_library(sdl3_imported STATIC IMPORTED)
    set_target_properties(sdl3_imported PROPERTIES
        IMPORTED_LOCATION "${_GUSENGINE_RELEASE}/_deps/sdl3-build/libSDL3.a")

    find_package(OpenGL REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(Threads REQUIRED)

    add_executable(poc_gl_single_context_probe poc_gl_single_context_probe.cpp)

    target_include_directories(poc_gl_single_context_probe PRIVATE
        "${_GUSENGINE_ROOT}/core/include"
        "${_GUSENGINE_ROOT}/domain/include"
        "${_GUSENGINE_ROOT}/platform/include"
        "${_GUSENGINE_ROOT}/app/include"
        "${_GUSENGINE_ROOT}/third_party/stb"
        # RmlUi_Include_GL3.h (glad GL3.3 core vendorizado) - MESMA tecnica do
        # msaa_glintfx_probe acima, pra ter glGetError/GL_NO_ERROR disponiveis.
        "${_GUSENGINE_ROOT}/platform/rmlui"
        "${_GUSENGINE_RELEASE}/_deps/sdl3-build/include-revision"
        "${_GUSENGINE_RELEASE}/_deps/sdl3-src/include"
    )

    target_compile_features(poc_gl_single_context_probe PRIVATE cxx_std_20)

    # Ordem do link REPLICA EXATAMENTE a linha (comprovadamente funcional) que o
    # ninja do preset linux-release usa pra linkar gusworld_app (ver
    # build/linux-release/build.ninja, alvo app/gusworld_app) - inclui as
    # duplicatas de domain/core apos platform (resolve refs circulares no link
    # estatico de arquivo unico, sem --start-group/--end-group).
    target_link_libraries(poc_gl_single_context_probe PRIVATE
        gusengine_app_imported
        gusengine_core_imported
        gusengine_domain_imported
        gusengine_platform_imported
        sdl3_imported
        gusengine_domain_imported
        gusengine_core_imported
        monocypher_imported
        rmlui_debugger_imported
        glintfx_imported
        rmlui_imported
        OpenGL::GL
        Freetype::Freetype
        Threads::Threads
        ${CMAKE_DL_LIBS}
        m
    )
    # =========================================================================
    # sysmenu_pause_screenshot_probe (MENU-INICIAL, spot-check visual Xvfb :99):
    # MESMA receita/libs importadas de poc_gl_single_context_probe acima (reusa
    # os .a JA compilados pelo preset linux-release) - so chama
    # build_system_menu_rml (POCO+RML pura, gus/app/screens/system_menu_rml.hpp)
    # com state.screen=Pause, renderiza 1 frame via glintfx::UiLayer e grava PNG
    # (stb_image_write). GUSWORLD_PROBE_CONFIRM=1 renderiza o mini-dialogo "voltar
    # ao menu inicial?" em vez da lista normal - ver o comentario no topo do
    # arquivo .cpp.
    add_executable(sysmenu_pause_screenshot_probe sysmenu_pause_screenshot_probe.cpp)

    target_include_directories(sysmenu_pause_screenshot_probe PRIVATE
        "${_GUSENGINE_ROOT}/core/include"
        "${_GUSENGINE_ROOT}/domain/include"
        "${_GUSENGINE_ROOT}/platform/include"
        "${_GUSENGINE_ROOT}/app/include"
        "${_GUSENGINE_ROOT}/third_party/stb"
        "${_GUSENGINE_ROOT}/platform/rmlui"
        "${_GUSENGINE_RELEASE}/_deps/sdl3-build/include-revision"
        "${_GUSENGINE_RELEASE}/_deps/sdl3-src/include"
        # glintfx::ui_layer.hpp (chamado DIRETO neste probe, MESMA receita de
        # sysmenu_controls_screenshot_probe.cpp) - o alvo importado
        # glintfx_imported abaixo nao carrega INTERFACE_INCLUDE_DIRECTORIES
        # (e um STATIC IMPORTED puro), entao o caminho precisa ser explicito.
        "${_GUSENGINE_RELEASE}/_deps/glintfx-src/glintfx/include"
    )

    target_compile_features(sysmenu_pause_screenshot_probe PRIVATE cxx_std_20)

    # MESMA ordem de link (comprovadamente funcional) de poc_gl_single_context_probe
    # acima.
    target_link_libraries(sysmenu_pause_screenshot_probe PRIVATE
        gusengine_app_imported
        gusengine_core_imported
        gusengine_domain_imported
        gusengine_platform_imported
        sdl3_imported
        gusengine_domain_imported
        gusengine_core_imported
        monocypher_imported
        rmlui_debugger_imported
        glintfx_imported
        rmlui_imported
        OpenGL::GL
        Freetype::Freetype
        Threads::Threads
        ${CMAKE_DL_LIBS}
        m
    )
else()
    message(STATUS
        "poc_gl_single_context_probe/sysmenu_pause_screenshot_probe: "
        "build/linux-release ainda nao foi compilado (falta "
        "${_GUSENGINE_RELEASE}/app/libgusengine_app.a) - rode 'cmake --preset "
        "linux-release && cmake --build --preset linux-release' na raiz do "
        "GusEngine primeiro. Alvos NAO adicionados nesta configuracao.")
endif()
