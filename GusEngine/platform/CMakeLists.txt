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
    # Backend OpenGL 3.3 da ARENA (ADR-009 adendo GL3): coexiste com o HUD RmlUi-GL3 no
    # mesmo contexto GL. Mesma API IRenderer do Render2dSdl (a arena/testes nao mudam).
    src/render2d/render2d_gl3.cpp
    # Loader GL (glad) da ARENA (Render2dGl3): gl3_load_functions + gl3_read_backbuffer.
    # NAO e backend de RmlUi; a arena precisa dele nos DOIS builds (OFF e ON) => fica SEMPRE.
    rmlui/gl3_loader.cpp
)

# --- RmlUi VENDORIZADO (ADR-009): backend GL3 oficial + sistema SDL3 + wrapper. SO no build
#     OFF (default). ADR-010 R-dup-backend (decisao do lider, Opcao 2): quando
#     GUSWORLD_GLINTFX=ON, o backend do glintfx assume e estes TUs NAO entram - assim o
#     RmlUi_Renderer_GL3 deixa de ficar DUPLICADO (uma copia nossa + a do glintfx, que so
#     linkavam por ordem de link, fragil). No ON nao ha caminho de HUD vendorizado em runtime
#     (o fallback do caminho antigo passa a ser BUILDAR COM GUSWORLD_GLINTFX=OFF).
#       RmlUi_Renderer_GL3.cpp -> backend GL3 (shaders gradiente/box-shadow/blur/mask) E dono
#                                 do GLAD_GL_IMPLEMENTATION (a impl do glad da arena vive aqui).
#       RmlUi_Platform_SDL.cpp -> sistema/eventos SDL3 do HUD vendorizado (so deste caminho).
#       rmlui_hud.cpp          -> wrapper PImpl que dirige o RmlUi via RenderInterface_GL3.
#     Confinado a platform/rmlui/ (com app/); core/+domain/ nao tocam (GATE).
if(NOT GUSWORLD_GLINTFX)
    target_sources(gusengine_platform PRIVATE
        rmlui/RmlUi_Renderer_GL3.cpp
        rmlui/RmlUi_Platform_SDL.cpp
        rmlui/rmlui_hud.cpp
    )
else()
    # ON: o RmlUi_Renderer_GL3 vendorizado SAIU - e era o dono do GLAD_GL_IMPLEMENTATION. A
    # arena (render2d_gl3) e o gl3_loader ainda usam o glad => o gl3_loader (que fica nos dois
    # builds) assume a IMPLEMENTACAO do glad (antes incluia so as declaracoes). O glintfx usa
    # gl3w (tabela de ponteiros PROPRIA), entao nao ha conflito de simbolos GL.
    target_compile_definitions(gusengine_platform PRIVATE GUSWORLD_OWN_GLAD_IMPL=1)
endif()

# OpenGL do sistema (libGL): o backend GL3 do RmlUi e o Render2dGl3 linkam GL. O glad
# (loader header-only) ja vem com o RmlUi_Include_GL3.h; so o link de GL e necessario.
find_package(OpenGL REQUIRED)

target_include_directories(gusengine_platform
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    # stb_image + stb_truetype (vendorizado header-only): load_texture e o atlas de
    # fonte (font_atlas) no Render2dSdl; tambem o LoadTexture do backend RmlUi (PATCH
    # GUSWORLD usa stb_image em vez de SDL_image).
    PRIVATE ${CMAKE_SOURCE_DIR}/third_party/stb
    # Headers vendorizados do backend SDL3 do RmlUi (RmlUi_Renderer_SDL.h /
    # RmlUi_Platform_SDL.h), incluidos pelo wrapper rmlui_hud.cpp.
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/rmlui
)

# Pasta dos assets de fonte da engine (.ttf embarcados no repo, dentro de assets/fonts/).
# Embutida em compilacao pra o resolver achar a fonte rodando do build dir; override em
# runtime via env GUSWORLD_ASSETS (mesma logica do resolver de mapas/sprites).
# GUSWORLD_FONTS_DIR e usado tanto pelo font_atlas (Render2dSdl) quanto pelo RmlUiHud
# (FreeType carrega a Pixel Operator Mono daqui).
# RMLUI_SDL_VERSION_MAJOR=3: o backend oficial e generico SDL2/SDL3 e seleciona o caminho
# SDL3 por esta macro (ver RmlUi_Renderer_SDL.h / RmlUi_Platform_SDL.h).
target_compile_definitions(gusengine_platform
    PRIVATE GUSWORLD_FONTS_DIR="${CMAKE_SOURCE_DIR}/assets/fonts"
    PRIVATE RMLUI_SDL_VERSION_MAJOR=3
)

# domain/ entra aqui: input_mapper/sdl_input consomem o mapa logico puro. SDL3 e a
# camada de plataforma. SDL3::SDL3 e o target estatico vindo do FetchContent. RmlUi::RmlUi
# entra aqui (UI/HUD, ADR-009): o wrapper rmlui_hud + o backend vendorizado linkam o nucleo
# do RmlUi (que ja puxa o FreeType do sistema via find_package na config raiz).
target_link_libraries(gusengine_platform
    PUBLIC
        gusengine_core
        gusengine_domain
        SDL3::SDL3
        RmlUi::RmlUi
        # OpenGL do sistema: backend GL3 do RmlUi + Render2dGl3. PUBLIC porque o backend
        # vendorizado (compilado neste alvo) expoe simbolos GL que o app tambem resolve.
        OpenGL::GL
)

target_compile_features(gusengine_platform PUBLIC cxx_std_20)

# Testes (Catch2, headless) da camada platform/.
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
