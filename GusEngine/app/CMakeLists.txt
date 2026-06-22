# gusworld_app - executavel GusWorld-specific + a lib gusengine_app que ele usa.
#
# Estrutura (M1): a logica de tela (OverworldSim e a casca de janela/loop) vira
# uma LIB estatica (gusengine_app) pra ser testavel headless (Qt Test) e linkavel
# pelo executavel. O executavel (gusworld_app) e so o main.cpp.
#
# Subdominios (engine-design.md secao 2):
#   screens/ Overworld (a state machine completa Overworld/Battle/Menu e M5)
#   main.cpp ponto de entrada (QGuiApplication + janela + loop, ou --smoke)

# AUTOMOC: a casca de janela usa QObject/sinais; os screens tambem podem.
set(CMAKE_AUTOMOC ON)

# --- lib gusengine_app: logica de tela + casca de janela/loop (testavel) ---
add_library(gusengine_app STATIC
    src/screens/overworld_sim.cpp
    src/game_window.cpp
    # Header com Q_OBJECT listado explicitamente pra o AUTOMOC gerar o moc dele
    # (esta em include/, fora do dir de fontes; sem isto, falta a vtable no link).
    include/gus/app/game_window.hpp
)

target_include_directories(gusengine_app
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(gusengine_app
    PUBLIC
        gusengine_core
        gusengine_domain
        gusengine_platform
)

target_compile_features(gusengine_app PUBLIC cxx_std_20)

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
)

target_compile_features(gusworld_app PRIVATE cxx_std_20)

# Testes da camada app/ (Qt Test, headless).
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
