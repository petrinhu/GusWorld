# gusengine_domain - POCO portado do C#, ZERO Qt.
#
# INVARIANTE (engine-design.md secao 2): este target NAO pode linkar Qt.
# NAO adicione Qt6::* em target_link_libraries(gusengine_domain ...). Nenhum
# header aqui pode incluir <Q...>. O CI audita includes Qt nesta pasta.
#
# Subdominios previstos (engine-design.md secao 2, portados em M3+):
#   save/        HMAC-SHA256 + migrators forward-only + backup chain
#   i18n/        loader .md + validador de paridade
#   progression/ EnemyKnowledgeTracker + XpDifferential
#   templates/   CharacterTemplate / EnemyTemplate
#   combat/      motor turn_combat preservado e portado (RF-4 e so apresentacao)
# No M0 so existe o stub de identidade do dominio; nenhuma regra portada ainda.
#
# Depende de core/ (utils puros). Continua 100% headless.

add_library(gusengine_domain STATIC
    src/domain_info.cpp
    src/i18n/md_translation_loader.cpp
    src/i18n/translation_parity_validator.cpp
)

target_include_directories(gusengine_domain
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(gusengine_domain
    PUBLIC gusengine_core
)

target_compile_features(gusengine_domain PUBLIC cxx_std_20)

# Sem target_link_libraries(... Qt6::*) por design. Camada pura portada.

# Testes do dominio (Catch2 headless). BUILD_TESTING vem do include(CTest) na
# raiz; os casos sao registrados no mesmo ctest dos testes do M0.
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
