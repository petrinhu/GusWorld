# Testes da camada app/ (Qt Test). Headless: o OverworldSim e desenhado num
# IRenderer FALSO (sem GPU), e o movimento/colisao/camera/interpolacao sao
# exercitados deterministicamente. Roda com QT_QPA_PLATFORM=offscreen.

set(CMAKE_AUTOMOC ON)

set(GUSENGINE_APP_TESTS
    overworld_sim_test
)

foreach(_t IN LISTS GUSENGINE_APP_TESTS)
    add_executable(${_t} ${_t}.cpp)
    target_link_libraries(${_t}
        PRIVATE
            gusengine_app
            gusengine_core
            gusengine_domain
            gusengine_platform
            Qt6::Test
    )
    target_compile_features(${_t} PRIVATE cxx_std_20)
    add_test(NAME ${_t} COMMAND ${_t})
    set_tests_properties(${_t} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endforeach()
