# GusEngine - raiz do build (pos repivot SDL3, ADR-008).
#
# Arquitetura de 4 camadas (engine-design.md secao 2, atualizada por ADR-008):
#   core/      POCO C++ puro, ZERO Qt E ZERO SDL, headless, 100% testavel
#   domain/    POCO portado do C#, ZERO Qt E ZERO SDL (save/i18n/progression/...)
#   platform/  fronteira SDL3, UNICA camada (com app/) que conhece SDL
#   app/       executavel GusWorld-specific (casca SDL: janela + loop proprio)
#
# INVARIANTE AUDITAVEL (ADR-008): core/ e domain/ NAO incluem nem linkam Qt NEM
# SDL. So platform/ e app/ tocam SDL. Os targets gusengine_core e gusengine_domain
# JAMAIS recebem SDL3::* em target_link_libraries. O CI audita via grep de includes
# <Q...> e <SDL...> nessas pastas.

cmake_minimum_required(VERSION 3.21)

project(GusEngine
    VERSION 0.1.0
    DESCRIPTION "Engine propria do GusWorld (C++/SDL3, pivot pos-Qt)"
    LANGUAGES CXX
)

# C++20 idiomatico, sem extensoes de compilador (portavel Linux/Windows).
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Exporta compile_commands.json para clang-format/clangd/IDE.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# ===========================================================================
#  Dependencias de plataforma via FetchContent (build reprodutivel Linux+Win).
#  Decisao (ADR-008 + relatorio): SDL3 e RmlUi nao sao header-only como as libs de
#  third_party/; entram por FetchContent com PIN de tag, compiladas do fonte. Isso
#  evita depender do que cada distro empacota e fixa o ABI. Catch2 ja seguia esse
#  padrao. Primeira config baixa o fonte; depois fica cacheado em build/_deps.
# ===========================================================================

# --- SDL3 (janela + render2d + input/gamepad). Pin = release-3.4.10 (casa com a
#     libSDL3 do sistema atual; estavel). Linkada ESTATICA (binario menor, deploy
#     trivial, licenca zlib permite). Desliga testes/exemplos do SDL.
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.4.10
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(SDL3)

# --- RmlUi (UI do jogador, fase 3 do ADR-008). APENAS CONFIGURADO aqui, NAO usado
#     ainda (nenhum target linka). Pin = 6.2. Desliga samples; FONT_ENGINE=none
#     para nao exigir FreeType no sistema enquanto a UI nao existe (a fonte real e
#     plugada na fase 3). Fica disponivel (target RmlUi::RmlUi) para quando a UI for
#     construida.
set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_FONT_ENGINE "none" CACHE STRING "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    RmlUi
    GIT_REPOSITORY https://github.com/mikke89/RmlUi.git
    GIT_TAG        6.2
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(RmlUi)

# --- Testes (Catch2 v3). core/+domain/+platform/+app/ testados headless. ---
include(CTest)        # define BUILD_TESTING (ON por padrao)
enable_testing()

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.7.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)
# disponibiliza o helper catch_discover_tests (integra Catch2 ao ctest)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

# --- Camadas ---
# Ordem importa: core e base de domain; platform usa core; app usa todos.
add_subdirectory(core)
add_subdirectory(domain)
add_subdirectory(platform)
add_subdirectory(app)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
