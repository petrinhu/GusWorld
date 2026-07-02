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
    input_mapper_test.cpp
    gamepad_mapping_test.cpp
    sdl_input_test.cpp
    viewport_transform_test.cpp
    render2d_sdl_test.cpp
    render2d_gl3_test.cpp
    alpha_bbox_test.cpp
    text_metrics_test.cpp
    font_atlas_test.cpp
    audio_engine_test.cpp
)

# ADR-010 F3: os testes do HUD RmlUi VENDORIZADO (rmlui_hud_test.cpp, 4 TEST_CASE) foram
# aposentados junto do backend vendorizado - o glintfx e o unico motor de UI/HUD agora.

target_link_libraries(gusengine_platform_tests
    PRIVATE
        gusengine_platform
        gusengine_core
        gusengine_domain
        Catch2::Catch2WithMain
)

target_compile_features(gusengine_platform_tests PRIVATE cxx_std_20)

catch_discover_tests(gusengine_platform_tests)
