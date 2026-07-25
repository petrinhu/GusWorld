# Testes da camada platform/ (Catch2 v3, headless). Pos repivot ADR-008: a logica
# testavel da fronteira SDL roda sem abrir janela e sem GPU - traducao de input
# (teclado SDL + gamepad), fusao de intencao, matematica de projecao do render, e o
# ciclo do backend Render2dSdl em modo headless (renderer nulo). O irredutivel
# (janela/render SDL real) e coberto pelo smoke do app.
#
# Um executavel agregado + catch_discover_tests (cada TEST_CASE vira um teste no
# ctest), no mesmo padrao de core/+domain/. NAO linka Qt.

add_executable(gusengine_platform_tests
    key_translation_test.cpp
    # F4-2.1/F4-2.2 (Onda F4, costura de input): traducao bidirecional glintfx::Key <->
    # keycode Godot + captura de remap do backend glintfx.
    key_translation_glintfx_test.cpp
    input_mapper_test.cpp
    gamepad_mapping_test.cpp
    sdl_input_test.cpp
    # F4-2.3 (Onda F4, costura de input): adaptador de teclado do backend novo
    # (glintfx) por polling de nivel, provider injetavel (headless).
    glintfx_input_test.cpp
    viewport_transform_test.cpp
    render2d_sdl_test.cpp
    render2d_gl3_test.cpp
    alpha_bbox_test.cpp
    text_metrics_test.cpp
    font_atlas_test.cpp
    audio_engine_test.cpp
    # MENU-PAUSA-CONFIG-SOM (M7-COSTURA): I/O real de settings.json em disco.
    settings_file_store_test.cpp
    # Tela Controles (remap, M2): I/O real de "<perfil>_controls.json" em disco.
    controls_file_store_test.cpp
    # M2-SAVE-IO (ADR-012 Onda 2): I/O real do save em disco (~/.gusworld/saves).
    save_file_store_test.cpp
    # ASSETS-VFS-F1 (ADR-013): AssetSource/FilesystemAssetSource, por familia de asset.
    asset_source_test.cpp
)

# F4-2.4 (Onda F4, costura de input): adaptador de GAMEPAD do backend novo (glintfx, evdev
# cru Linux) para o mesmo GamepadState - provider injetavel (headless) + teste "sem
# dispositivo" com glintfx::Gamepads real (dev_dir vazio injetado). Registrado SO em Linux,
# MESMO guard `if(UNIX AND NOT APPLE)` de GLINTFX_MODULE_GAMEPAD no CMakeLists raiz - o
# header glintfx_gamepad_input.hpp inclui <glintfx/gamepad.hpp>, que tem #error fora de
# Linux (glintfx::Gamepads requer GLINTFX_MODULE_GAMEPAD=ON). Ligar isto incondicionalmente
# quebraria o build no CI Windows (MSVC).
if(UNIX AND NOT APPLE)
    target_sources(gusengine_platform_tests PRIVATE glintfx_gamepad_input_test.cpp)
endif()

# ADR-010 F3: os testes do HUD RmlUi VENDORIZADO (rmlui_hud_test.cpp, 4 TEST_CASE) foram
# aposentados junto do backend vendorizado - o glintfx e o unico motor de UI/HUD agora.

target_link_libraries(gusengine_platform_tests
    PRIVATE
        gusengine_platform
        gusengine_core
        gusengine_domain
        Catch2::Catch2WithMain
        gus_warnings
        # F4-2.1: key_translation_glintfx_test.cpp inclui key_translation_glintfx.hpp,
        # que por sua vez inclui <glintfx/ui_event.hpp> (glintfx::Key e PUBLICO na
        # assinatura das funcoes) - o link PRIVATE de glintfx::glintfx dentro de
        # gusengine_platform (ver platform/CMakeLists.txt) NAO propaga include dirs pra
        # este alvo (usage requirements PRIVATE nao atravessam consumidor), entao este
        # TU precisa do proprio link. MESMO padrao que app/CMakeLists.txt ja usa pra
        # gusengine_app (glintfx::glintfx PRIVATE, direto no alvo que inclui o header).
        glintfx::glintfx
)

target_compile_features(gusengine_platform_tests PRIVATE cxx_std_20)

# ASSETS-VFS-F1/ASSETS-VFS-F1b/ASSETS-VFS-F1c: espelha as MESMAS 6 macros do alvo
# gusengine_platform (compile definitions sao PRIVATE, nao propagam por link) - so pra
# asset_source_test.cpp poder computar o valor esperado do fallback "sem env" sem
# depender do layout real do repo.
target_compile_definitions(gusengine_platform_tests
    PRIVATE GUSWORLD_ASSETS_DIR="${CMAKE_SOURCE_DIR}/../resources"
    PRIVATE GUSWORLD_SFX_DIR="${CMAKE_SOURCE_DIR}/../assets/sfx"
    PRIVATE GUSWORLD_MUSIC_DIR="${CMAKE_SOURCE_DIR}/../assets/music"
    PRIVATE GUSWORLD_TRANSLATIONS_DIR="${CMAKE_SOURCE_DIR}/../resources/translations"
    PRIVATE GUSWORLD_DIALOGUES_DIR="${CMAKE_SOURCE_DIR}/../resources/dialogues"
    PRIVATE GUSWORLD_MAPS_DIR="${CMAKE_SOURCE_DIR}/assets/maps/compiled"
)

catch_discover_tests(gusengine_platform_tests)
