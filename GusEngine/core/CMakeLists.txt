# gusengine_core - POCO C++ puro, ZERO Qt.
#
# INVARIANTE (engine-design.md secao 2): este target NAO pode linkar Qt.
# NAO adicione Qt6::* em target_link_libraries(gusengine_core ...). Nenhum
# header aqui pode incluir <Q...>. O CI audita includes Qt nesta pasta.
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
)

target_include_directories(gusengine_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(gusengine_core PUBLIC cxx_std_20)

# Sem target_link_libraries(... Qt6::*) por design. Camada pura.
