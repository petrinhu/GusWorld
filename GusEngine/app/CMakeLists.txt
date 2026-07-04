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
    src/screens/battle_cockpit_verb_ids.cpp
    src/screens/battle_hud_model.cpp
    src/screens/battle_menu.cpp
    src/screens/battle_log_model.cpp
    src/screens/battle_floaters.cpp
    src/screens/battle_anim.cpp
    src/screens/battle_sprite_anim.cpp
    src/screens/battle_pacing.cpp
    src/screens/battle_scene.cpp
    src/screens/battle_preview.cpp
    src/i18n/translator.cpp
    src/sdl_window.cpp
    # M7-COSTURA (ADR-012 Onda 1): maestro leve cidade<->batalha (ver maestro.hpp).
    src/maestro_logic.cpp
    src/maestro.cpp
    # M7-COSTURA Inc 2b: GLITCH DIGITAL/"PROCESSANDO" no lugar do fade preto liso
    # (ver glitch_overlay.hpp).
    src/glitch_overlay.cpp
    # Diagnostico opcional de device de audio real (M6 F1, ADR-011 item 5).
    # GUSWORLD_AUDIO_SMOKE=1: ver app/include/gus/app/audio_smoke.hpp.
    src/audio_smoke.cpp
)

target_include_directories(gusengine_app
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    # stb_image_write (vendorizado header-only): captura de frame em PNG para o smoke
    # visual do ADR-009 (battle_preview). IMPLEMENTACAO definida em battle_preview.cpp.
    PRIVATE ${CMAKE_SOURCE_DIR}/third_party/stb
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
    # Pasta dos catalogos de traducao (game/translations/) DENTRO do repo (raiz = pai
    # de GusEngine/). O Translator de UI le pt_br.md daqui rodando do build dir;
    # override em runtime via env GUSWORLD_TRANSLATIONS (caminho completo do .md).
    PRIVATE GUSWORLD_TRANSLATIONS_DIR="${CMAKE_SOURCE_DIR}/../game/translations"
    # Pasta das fontes (.ttf) DENTRO de GusEngine/ (mesma macro do platform/). ADR-010
    # F2a: o cockpit BAKED do glintfx registra a "Pixel Operator Mono" via @font-face no
    # RCSS (o glintfx::UiLayer NAO expoe Rml::LoadFontFace - so o doc carrega fonte). Sem
    # esta macro o app/ nao saberia o caminho absoluto da fonte. Override: env GUSWORLD_FONTS.
    PRIVATE GUSWORLD_FONTS_DIR="${CMAKE_SOURCE_DIR}/assets/fonts"
    # Pasta do kit CC0 de SFX (M6 F2/F3, ADR-011), na RAIZ do repo (irma de resources/ e
    # GusEngine/ - "assets/" de origem/source do JOGO, diferente de GusEngine/assets/ que
    # e asset DA ENGINE). O battle_preview (owner do AudioEngine) le hit_digital_
    # provisorio.wav daqui rodando do build dir; override em runtime via env GUSWORLD_SFX.
    PRIVATE GUSWORLD_SFX_DIR="${CMAKE_SOURCE_DIR}/../assets/sfx"
    # Pasta da musica CC0 (M6 F4, ADR-011), irma de assets/sfx/ (mesma raiz "assets/" do
    # JOGO). O battle_preview le cidade_tema_provisorio.mp3 daqui; override em runtime
    # via env GUSWORLD_MUSIC.
    PRIVATE GUSWORLD_MUSIC_DIR="${CMAKE_SOURCE_DIR}/../assets/music"
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

# ADR-010 F3: glintfx::UiLayer (embed mode) e o UNICO motor de UI/HUD. Linkado SEMPRE, SO
# aqui em app/ (GATE de 4 camadas: nunca core/ nem domain/). Sem flag - o backend RmlUi
# vendorizado foi aposentado (two-way door via historico git).
target_link_libraries(gusengine_app PRIVATE glintfx::glintfx)

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
