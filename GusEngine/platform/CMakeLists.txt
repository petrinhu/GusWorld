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
    # Tela Controles (remap, M2): I/O real de ~/.gusworld/<perfil>_controls.json.
    src/fs/controls_file_store.cpp
    # M2-SAVE-IO (ADR-012 Onda 2): I/O real de ~/.gusworld/saves/*.sav (save V4
    # binario, consumido via load_save nao-lancante).
    src/fs/save_file_store.cpp
    # ASSETS-VFS-F1 (ADR-013): o "porteiro" unico de leitura de asset (AssetSource +
    # FilesystemAssetSource). Fase 1 = so indirecao sobre o filesystem, consolidando a
    # cadeia `env > macro > CWD` que estava duplicada em 6+ resolvers (font_atlas,
    # translator, player_sprites_loader, anim_catalog, sdl_window, battle_preview,
    # system_menu_loop).
    src/assets/asset_source.cpp
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
#
# ASSETS-VFS-F1 (ADR-013): as 4 macros abaixo (ASSETS/SFX/MUSIC/TRANSLATIONS_DIR) MIGRAM
# pra ca (antes so em app/CMakeLists.txt) porque a cadeia de resolucao `env > macro > CWD`
# por familia de asset agora mora DENTRO de FilesystemAssetSource (platform/assets/), nao
# mais duplicada em cada consumidor de app/. Valores IDENTICOS aos ja definidos em
# app/CMakeLists.txt (mesma raiz ${CMAKE_SOURCE_DIR} = GusEngine/ nos dois alvos - definir
# nos dois nao diverge, e o mesmo padrao ja usado por GUSWORLD_FONTS_DIR).
# ASSETS-VFS-F1b: GUSWORLD_DIALOGUES_DIR migra pra ca pela MESMA razao (familia DIALOGUES
# nova no dispatcher, retrofit de npc_dialogue_catalog.cpp).
# ASSETS-VFS-F1c: GUSWORLD_MAPS_DIR migra pra ca pela MESMA razao (familia MAPS nova no
# dispatcher, retrofit de city_scene.cpp::resolve_distritos_inferiores_gmap). Valor
# IDENTICO ao ja definido em app/CMakeLists.txt (${CMAKE_SOURCE_DIR} = GusEngine/ nos dois
# alvos - deixado tambem la, mesmo precedente de GUSWORLD_TRANSLATIONS_DIR/
# GUSWORLD_DIALOGUES_DIR, so unused agora que city_scene.cpp delega pro porteiro).
target_compile_definitions(gusengine_platform
    PRIVATE GUSWORLD_FONTS_DIR="${CMAKE_SOURCE_DIR}/assets/fonts"
    # Raiz resources/ do repo (irma de GusEngine/) - familia GENERICA (sprites/images/vfx).
    PRIVATE GUSWORLD_ASSETS_DIR="${CMAKE_SOURCE_DIR}/../resources"
    # Kit CC0 de SFX, na raiz do repo (irma de resources/ e GusEngine/, M6 ADR-011).
    PRIVATE GUSWORLD_SFX_DIR="${CMAKE_SOURCE_DIR}/../assets/sfx"
    # Musica CC0, irma de assets/sfx/ (M6 ADR-011).
    PRIVATE GUSWORLD_MUSIC_DIR="${CMAKE_SOURCE_DIR}/../assets/music"
    # Catalogos de traducao (game/translations/), raiz = pai de GusEngine/.
    PRIVATE GUSWORLD_TRANSLATIONS_DIR="${CMAKE_SOURCE_DIR}/../game/translations"
    # Grafos de dialogo (game/dialogues/), raiz-irma de GUSWORLD_TRANSLATIONS_DIR acima
    # (M7-DIALOGO, ASSETS-VFS-F1b).
    PRIVATE GUSWORLD_DIALOGUES_DIR="${CMAKE_SOURCE_DIR}/../game/dialogues"
    # Mapas .gmap compilados, DENTRO de GusEngine/ (versionados com a engine, ASSETS-VFS-F1c).
    PRIVATE GUSWORLD_MAPS_DIR="${CMAKE_SOURCE_DIR}/assets/maps/compiled"
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
