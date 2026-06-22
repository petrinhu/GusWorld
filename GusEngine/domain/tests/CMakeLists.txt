# Testes headless (Catch2 v3) de domain/ (subsistemas portados do C#).
#
# Espelha o padrao de GusEngine/tests/ do M0: executavel Catch2 +
# catch_discover_tests (cada TEST_CASE vira um teste no ctest). NAO linka Qt:
# domain/ e POCO puro, exercitado sem abrir janela (como a logica do jogo roda
# hoje sem Godot).
#
# Marco M3: spec executavel portada do i18n (loader .md + paridade) e do
# progression (EnemyKnowledgeTracker + XpDifferential). Outros subsistemas
# (save/templates/combat) ganham seus alvos de teste aqui conforme forem portados.

add_executable(gusengine_domain_tests
    md_translation_loader_test.cpp
    translation_parity_validator_test.cpp
    xp_differential_test.cpp
    enemy_knowledge_tracker_test.cpp
    character_template_test.cpp
    template_serializer_test.cpp
    template_source_test.cpp
    canonical_templates_test.cpp
    save_slots_test.cpp
    save_serializer_test.cpp
    save_serializer_fuzz_test.cpp
    save_migrators_test.cpp
    save_backup_test.cpp
    template_serializer_fuzz_test.cpp
    combat_constants_test.cpp
    combat_enums_test.cpp
    environment_enums_test.cpp
    combat_records_test.cpp
)

target_link_libraries(gusengine_domain_tests
    PRIVATE
        gusengine_domain
        Catch2::Catch2WithMain
)

target_compile_features(gusengine_domain_tests PRIVATE cxx_std_20)

# Registra os casos no ctest. Rodar com: ctest --output-on-failure
catch_discover_tests(gusengine_domain_tests)
