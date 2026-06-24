# gusworld_app - executavel GusWorld-specific + a lib gusengine_app (pos repivot
# SDL3, ADR-008).
#
# A logica de tela (OverworldSim) e a casca de janela/loop (SdlWindow) viram uma
# LIB estatica (gusengine_app) pra ser testavel headless (Catch2) e linkavel pelo
# executavel. O executavel (gusworld_app) e so o main.cpp (modo janela + --smoke).

# --- lib gusengine_app: logica de tela + casca de janela/loop (testavel) ---
add_library(gusengine_app STATIC
    src/screens/overworld_sim.cpp
    src/screens/sprite_animation.cpp
    src/screens/player_sprites_loader.cpp
    src/screens/anim_catalog.cpp
    src/screens/anim_preview.cpp
    src/screens/city_scene.cpp
    src/screens/city_loader.cpp
    src/screens/battle_layout.cpp
    src/screens/battle_scene.cpp
    src/screens/battle_preview.cpp
    src/sdl_window.cpp
)

target_include_directories(gusengine_app
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Caminho ABSOLUTO da pasta resources/ do repo (raiz = pai de GusEngine/), embutido
# para o loader de sprites resolver os assets do Caua quando rodando do build dir
# (a pasta resources/ vive FORA do git, junto do codigo). Override em runtime via
# env GUSWORLD_ASSETS. ${CMAKE_SOURCE_DIR} = GusEngine/ (raiz do projeto CMake).
target_compile_definitions(gusengine_app
    PRIVATE GUSWORLD_ASSETS_DIR="${CMAKE_SOURCE_DIR}/../resources"
    # Pasta dos mapas compilados (.gmap) DENTRO do repo (versionada), nao em
    # resources/. O resolver de city_scene usa isto pra achar o .gmap rodando do
    # build dir; override em runtime via env GUSWORLD_MAPS.
    PRIVATE GUSWORLD_MAPS_DIR="${CMAKE_SOURCE_DIR}/assets/maps/compiled"
)

# SDL3 entra aqui (a casca SdlWindow + o main tocam SDL diretamente).
target_link_libraries(gusengine_app
    PUBLIC
        gusengine_core
        gusengine_domain
        gusengine_platform
        SDL3::SDL3
)

target_compile_features(gusengine_app PUBLIC cxx_std_20)

# --- executavel ---
add_executable(gusworld_app
    main.cpp
)

target_link_libraries(gusworld_app
    PRIVATE
        gusengine_app
        gusengine_core
        gusengine_domain
        gusengine_platform
        SDL3::SDL3
)

target_compile_features(gusworld_app PRIVATE cxx_std_20)

# Testes da camada app/ (Catch2, headless).
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
