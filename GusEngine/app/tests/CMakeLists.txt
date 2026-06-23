# Testes da camada app/ (Catch2 v3, headless). O OverworldSim e desenhado num
# IRenderer FALSO (sem GPU), e o movimento/colisao/camera/interpolacao/sprite sao
# exercitados deterministicamente. A LOGICA de animacao (sprite_animation) e POCO
# pura. NAO linka Qt nem SDL: tudo roda sem abrir janela.
#
# Um executavel agregado + catch_discover_tests, no padrao de core/+domain/.

add_executable(gusengine_app_tests
    overworld_sim_test.cpp
    sprite_animation_test.cpp
    sprite_anchor_test.cpp
    anim_catalog_test.cpp
    player_sprites_layout_test.cpp
)

target_link_libraries(gusengine_app_tests
    PRIVATE
        gusengine_app
        gusengine_core
        gusengine_domain
        gusengine_platform
        Catch2::Catch2WithMain
)

target_compile_features(gusengine_app_tests PRIVATE cxx_std_20)

catch_discover_tests(gusengine_app_tests)
