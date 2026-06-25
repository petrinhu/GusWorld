# gusengine_platform - fronteira SDL3 (pos repivot ADR-008). UNICA camada (com
# app/) que conhece SDL. Toda dependencia de SDL fica confinada aqui e em app/, de
# modo que core/ e domain/ permanecam puros (sem Qt E sem SDL) e reutilizaveis.
#
# Subdominios (engine-design.md secao 2, atualizado por ADR-008):
#   render2d/ IRenderer sobre SDL_Renderer (sprite via SDL_Texture, quads das
#             paredes); viewport_transform POCO (mundo->pixel) testavel sem GPU.
#   input/    eventos SDL (teclado + gamepad) -> intencao cardinal (consome o
#             InputMapper/key_translation; gamepad_mapping POCO).

add_library(gusengine_platform STATIC
    src/platform_info.cpp
    src/input/key_translation.cpp
    src/input/input_mapper.cpp
    src/input/gamepad_mapping.cpp
    src/input/sdl_input.cpp
    src/render2d/viewport_transform.cpp
    src/render2d/text_metrics.cpp
    src/render2d/font_atlas.cpp
    src/render2d/render2d_sdl.cpp
)

target_include_directories(gusengine_platform
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    # stb_image + stb_truetype (vendorizado header-only): load_texture e o atlas de
    # fonte (font_atlas) no Render2dSdl.
    PRIVATE ${CMAKE_SOURCE_DIR}/third_party/stb
)

# Pasta dos assets de fonte da engine (.ttf embarcados no repo, dentro de assets/fonts/).
# Embutida em compilacao pra o resolver achar a fonte rodando do build dir; override em
# runtime via env GUSWORLD_ASSETS (mesma logica do resolver de mapas/sprites).
target_compile_definitions(gusengine_platform
    PRIVATE GUSWORLD_FONTS_DIR="${CMAKE_SOURCE_DIR}/assets/fonts"
)

# domain/ entra aqui: input_mapper/sdl_input consomem o mapa logico puro. SDL3 e a
# camada de plataforma. SDL3::SDL3 e o target estatico vindo do FetchContent.
target_link_libraries(gusengine_platform
    PUBLIC
        gusengine_core
        gusengine_domain
        SDL3::SDL3
)

target_compile_features(gusengine_platform PUBLIC cxx_std_20)

# Testes (Catch2, headless) da camada platform/.
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
