# Testes da camada platform/ (Qt Test). Headless: rodam sem abrir janela e sem
# GPU (so exercitam a logica testavel da fronteira Qt - traducao de input,
# matematica de projecao do render). Complementam os testes Catch2 de core/+domain/.
#
# Cada arquivo e um executavel QTEST_MAIN proprio (padrao Qt Test) registrado no
# ctest via add_test. AUTOMOC ligado (os .moc dos QObject de teste).
#
# Rodam com QT_QPA_PLATFORM=offscreen (sem display) - configurado no ENVIRONMENT
# de cada teste, pra valer tanto no CI quanto no ctest local.

set(CMAKE_AUTOMOC ON)

# Lista de (nome do teste, fonte). Cada um vira target + add_test.
set(GUSENGINE_PLATFORM_TESTS
    key_translation_test
    input_mapper_test
    viewport_transform_test
    render2d_rhi_test
)

foreach(_t IN LISTS GUSENGINE_PLATFORM_TESTS)
    add_executable(${_t} ${_t}.cpp)
    target_link_libraries(${_t}
        PRIVATE
            gusengine_platform
            gusengine_core
            gusengine_domain
            Qt6::Test
    )
    target_compile_features(${_t} PRIVATE cxx_std_20)
    add_test(NAME ${_t} COMMAND ${_t})
    # Headless: sem isto, QTEST_MAIN tenta abrir a plataforma de janela e falha no CI.
    set_tests_properties(${_t} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endforeach()
