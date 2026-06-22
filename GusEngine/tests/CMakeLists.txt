# Testes headless (Catch2 v3) de core/ + domain/.
#
# Estes testes NAO linkam Qt: exercitam apenas as camadas puras, do mesmo jeito
# que a logica do jogo roda hoje sem abrir o Godot. Qt Test (camada platform/)
# entra em M1+, quando houver janela/input/audio para testar.
#
# Integra ao ctest via catch_discover_tests (cada TEST_CASE vira um teste).

add_executable(gusengine_core_tests
    core_smoke_test.cpp
    sha256_test.cpp
    hmac_sha256_test.cpp
)

target_link_libraries(gusengine_core_tests
    PRIVATE
        gusengine_core
        gusengine_domain
        Catch2::Catch2WithMain
)

target_compile_features(gusengine_core_tests PRIVATE cxx_std_20)

# Registra os casos no ctest. Rodar com: ctest --output-on-failure
catch_discover_tests(gusengine_core_tests)
