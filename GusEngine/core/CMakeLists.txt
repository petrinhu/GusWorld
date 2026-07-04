# gusengine_core - POCO C++ puro, ZERO Qt E ZERO SDL (ADR-008).
#
# INVARIANTE (engine-design.md sec 2 + ADR-008): este target NAO pode linkar Qt
# NEM SDL. NAO adicione Qt6::*/SDL3::* em target_link_libraries(gusengine_core).
# Nenhum header aqui pode incluir <Q...> nem <SDL...>. O CI audita ambos nesta pasta.
#
# Subdominios previstos (engine-design.md secao 2): time/ rng/ ecs_lite/
# resource/ events/. No M0 so existia o stub de versao + um util (clamp); os
# subdominios sao populados em marcos posteriores. M4 adiciona spatial/
# (grade + colisao AABB-desliza + clamp de camera), matematica de engine pura.

add_library(gusengine_core STATIC
    src/version.cpp
    src/math_util.cpp
    src/crypto/sha256.cpp
    src/crypto/hmac_sha256.cpp
    src/crypto/key_derivation.cpp
    src/spatial/tile_grid.cpp
    src/spatial/grid_collision.cpp
    src/spatial/camera_clamp.cpp
    src/time/fixed_timestep.cpp
    src/anim/anim_clock.cpp
    src/anim/breath_oscillator.cpp
    src/anim/fade_transition.cpp
    # M7-COSTURA Inc 2c: sequencia de frames pre-renderizada (boot pixelizado) no
    # lugar do glitch procedural (aposentado - o lider VETOU o visual, "pareceu bug").
    src/anim/boot_pixel_sequence.cpp
    src/player/stamina.cpp
    src/player/winded_timer.cpp
)

target_include_directories(gusengine_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(gusengine_core PUBLIC cxx_std_20)

# Sem target_link_libraries(... Qt6::*/SDL3::*) por design. Camada pura.
