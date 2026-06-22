# GusEngine - raiz do build (pivot C++/Qt6, marco M0 andaime)
#
# Arquitetura de 4 camadas (engine-design.md secao 2):
#   core/      POCO C++ puro, ZERO Qt, headless, 100% testavel
#   domain/    POCO portado do C#, ZERO Qt (save/i18n/progression/templates/combat)
#   platform/  fronteira Qt, UNICA camada que conhece Qt
#   app/       executavel GusWorld-specific
#
# INVARIANTE AUDITAVEL (engine-design.md secao 2):
#   core/ e domain/ NAO incluem nem linkam Qt. So platform/ e app/ tocam Qt.
#   Targets gusengine_core e gusengine_domain JAMAIS recebem Qt6::* em
#   target_link_libraries. O CI audita via grep de includes <Q...> nessas pastas.
#
# Escopo M0: apenas andaime. 1 stub por camada para dar targets ao build.
# Nenhuma logica de jogo portada aqui (porte e marco M3+).
#
# NOTA: CMAKE_PREFIX_PATH apontando para o Qt 6.8 vem do CMakePresets.json,
# que e mantido pelo devops-sre (fora do escopo desta frente). Aqui so se
# referencia o Qt via find_package; confia-se no prefix path do preset.

cmake_minimum_required(VERSION 3.21)

project(GusEngine
    VERSION 0.1.0
    DESCRIPTION "Engine propria do GusWorld (C++/Qt6, pivot pos-Godot)"
    LANGUAGES CXX
)

# C++20 idiomatico, sem extensoes de compilador (portavel Linux/Windows).
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Exporta compile_commands.json para clang-format/clangd/IDE.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt 6.8 LTS. O devops instala o Qt e ajusta CMAKE_PREFIX_PATH no preset;
# aqui so se exige a versao e os componentes usados pela camada platform/app.
# ShaderTools: fornece qt6_add_shaders (compila GLSL -> .qsb via qsb) pro render2d.
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Quick Multimedia Test ShaderTools)

# Politica recomendada do Qt6: nao auto-rodar moc/uic/rcc nas libs sem Qt
# (core/domain). Quem precisa (platform/app) liga AUTOMOC localmente.
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

# --- Testes (Catch2 v3 via FetchContent) ---
# core/ e domain/ sao testados headless com Catch2 (a spec executavel portada).
# Qt Test entra em M1+ (quando houver camada Qt para testar); a porta fica
# aberta (Qt6::Test ja achado acima), mas no M0 o teste e Catch2.
include(CTest)        # define BUILD_TESTING (ON por padrao)
enable_testing()

include(FetchContent)
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
