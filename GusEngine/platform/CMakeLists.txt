# gusengine_platform - fronteira SDL3 (pos repivot ADR-008). UNICA camada (com
# app/) que conhece SDL. Toda dependencia de SDL fica confinada aqui e em app/, de
# modo que core/ e domain/ permanecam puros (sem Qt E sem SDL) e reutilizaveis.
#
# Subdominios (engine-design.md secao 2, atualizado por ADR-008):
#   render2d/ IRenderer sobre SDL_Renderer (sprite via SDL_Texture, quads das
#             paredes); viewport_transform POCO (mundo->pixel) testavel sem GPU.
#   input/    eventos SDL (teclado + gamepad) -> intencao cardinal (consome o
#             InputMapper/key_translation; gamepad_mapping POCO).
#   audio/    AudioEngine sobre miniaudio (M6 F1, ADR-011): device + mixer minimo
#             (buses music/sfx) + API play_sfx/play_music/stop_music/set_master_volume.
#             Degradacao graciosa (sem device -> no-op) e null-device pra testes/CI.

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
    # NAO e backend de RmlUi; a arena precisa dele SEMPRE => fica.
    rmlui/gl3_loader.cpp
    # AudioEngine (M6 F1, ADR-011): UNICA TU que define MINIAUDIO_IMPLEMENTATION
    # (miniaudio.h e single-header vendorizado em third_party/miniaudio/).
    src/audio/audio_engine.cpp
    # MENU-PAUSA-CONFIG-SOM (M7-COSTURA): I/O real de ~/.gusworld/settings.json.
    src/fs/settings_file_store.cpp
)

# ADR-010 F3: o backend RmlUi vendorizado (RmlUi_Renderer_GL3.cpp / RmlUi_Platform_SDL.cpp /
# rmlui_hud.cpp) foi APOSENTADO - o glintfx (linkado em app/) provê o RenderInterface_GL3. O
# RmlUi_Renderer_GL3 vendorizado era o dono do GLAD_GL_IMPLEMENTATION; como ele saiu, o
# gl3_loader.cpp (loader da arena, que fica SEMPRE) assume a IMPLEMENTACAO do glad via
# GUSWORLD_OWN_GLAD_IMPL (antes incluia so as declaracoes). O glintfx usa gl3w (tabela de
# ponteiros PROPRIA), entao nao ha conflito de simbolos GL.
target_compile_definitions(gusengine_platform PRIVATE GUSWORLD_OWN_GLAD_IMPL=1)

# OpenGL do sistema (libGL): o Render2dGl3 (arena) linka GL. O glad (loader header-only) ja
# vem com o RmlUi_Include_GL3.h; so o link de GL e necessario.
find_package(OpenGL REQUIRED)

# miniaudio (M6 F1, ADR-011): no Linux o backend (ALSA/PulseAudio/JACK, decodificados em
# runtime via dlopen) exige -lpthread -ldl -lm (miniaudio.h, secao "Building", linha ~505:
# "The Linux build only requires linking to -ldl, -lpthread and -lm"). libm ja vem implicita
# com o driver de link do C++ na pratica, mas listamos por completude/portabilidade.
find_package(Threads REQUIRED)

target_include_directories(gusengine_platform
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    # stb_image + stb_truetype (vendorizado header-only): load_texture e o atlas de
    # fonte (font_atlas) no Render2dSdl; tambem o LoadTexture do backend RmlUi (PATCH
    # GUSWORLD usa stb_image em vez de SDL_image).
    PRIVATE ${CMAKE_SOURCE_DIR}/third_party/stb
    # Header do loader glad (RmlUi_Include_GL3.h), incluido pelo gl3_loader.cpp e pelo
    # Render2dGl3 (arena). ADR-010 F3: o unico consumidor deste dir e a ARENA (o backend
    # RmlUi vendorizado, que tambem o usava, foi aposentado).
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/rmlui
    # miniaudio (vendorizado header-only, M6 F1 ADR-011): so audio_engine.cpp inclui
    # miniaudio.h (e e a UNICA TU com MINIAUDIO_IMPLEMENTATION) - PRIVATE.
    PRIVATE ${CMAKE_SOURCE_DIR}/third_party/miniaudio
)

# Pasta dos assets de fonte da engine (.ttf embarcados no repo, dentro de assets/fonts/).
# Embutida em compilacao pra o resolver achar a fonte rodando do build dir; override em
# runtime via env GUSWORLD_ASSETS (mesma logica do resolver de mapas/sprites).
# GUSWORLD_FONTS_DIR e usado pelo font_atlas (Render2dSdl); o glintfx (app/) carrega a Pixel
# Operator Mono via @font-face no RCSS a partir da mesma pasta.
target_compile_definitions(gusengine_platform
    PRIVATE GUSWORLD_FONTS_DIR="${CMAKE_SOURCE_DIR}/assets/fonts"
)

# domain/ entra aqui: input_mapper/sdl_input consomem o mapa logico puro. SDL3 e a camada de
# plataforma. SDL3::SDL3 e o target estatico vindo do FetchContent. RmlUi::RmlUi entra aqui
# porque o header do loader glad (RmlUi_Include_GL3.h) vive na arvore do RmlUi; o glintfx
# (app/) e quem linka o nucleo do RmlUi de fato (ADR-010 F3).
target_link_libraries(gusengine_platform
    PUBLIC
        gusengine_core
        gusengine_domain
        SDL3::SDL3
        RmlUi::RmlUi
        # OpenGL do sistema: Render2dGl3 (arena). PUBLIC porque o gl3_loader (dono do glad
        # neste alvo) expoe simbolos GL que o app tambem resolve.
        OpenGL::GL
)

# miniaudio (M6 F1, ADR-011): PRIVATE porque so audio_engine.cpp toca essas libs
# diretamente (o resto de gusengine_platform/app nao inclui miniaudio.h).
target_link_libraries(gusengine_platform PRIVATE Threads::Threads ${CMAKE_DL_LIBS})

target_compile_features(gusengine_platform PUBLIC cxx_std_20)

# Testes (Catch2, headless) da camada platform/.
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
