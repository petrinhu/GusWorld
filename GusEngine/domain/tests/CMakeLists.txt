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
    tile_map_test.cpp
    map_serializer_test.cpp
    map_csv_test.cpp
    template_serializer_fuzz_test.cpp
    combat_constants_test.cpp
    combat_enums_test.cpp
    environment_enums_test.cpp
    combat_records_test.cpp
    weakness_wheel_test.cpp
    combat_actor_test.cpp
    initiative_queue_test.cpp
    # COMBATE-FILA-CURSOR-FIX (decisao do lider 2026-07-15, A1/A2/A3): primitivas SEGURAS
    # de reordenacao intra-rodada (reorder_pending/delay_current) que NUNCA cruzam o
    # cursor - fecham a raiz dos bugs GambitReorder-duplo e Knockback-pula-vizinho.
    initiative_queue_pending_test.cpp
    combat_state_test.cpp
    environment_modifier_test.cpp
    environment_catalog_test.cpp
    environment_stacking_test.cpp
    environment_status_facilitation_test.cpp
    environment_clock_test.cpp
    environment_transitions_test.cpp
    random_source_test.cpp
    combo_table_test.cpp
    scripted_brain_test.cpp
    placeholder_cards_test.cpp
    combat_state_machine_test.cpp
    combat_formula_test.cpp
    combat_inc2_test.cpp
    combat_inc3_test.cpp
    combat_status_effects_test.cpp
    combat_status_fixes_test.cpp
    universal_compiler_defense_test.cpp
    environment_damage_test.cpp
    environment_fsm_test.cpp
    # Reforco de QA M5: property-based + fuzz das invariantes do motor de combate
    # (recomendado pela auditoria AUD-M5-MOTOR-2026-06-22). Sem dependencia externa:
    # property-based "a mao" via LCG deterministico (property_gen.hpp).
    combat_actor_property_test.cpp
    weakness_wheel_property_test.cpp
    environment_cap_property_test.cpp
    combat_formula_property_test.cpp
    initiative_queue_property_test.cpp
    # Marco M2 (metade de logica pura): input_remap (ActionRegistry/InputBinding)
    # portado C#->C++ POCO. O backend de evento Qt (platform/input/) NAO entra aqui.
    input_remap_test.cpp
    # Marco M2 (ADR-007): persistencia de controles (json proprio + hash128 + diff/
    # restore + sanitize) e SAVE V4 (backup + slot-id selado + KDF + detect-respond).
    controls_json_test.cpp
    controls_hash_test.cpp
    controls_diff_test.cpp
    controls_name_sanitize_test.cpp
    controls_restore_test.cpp
    # Tela Controles (remap, M2): swap-on-conflict PURO ("avisa e troca").
    controls_remap_apply_test.cpp
    save_v4_test.cpp
    # SAVE-LOAD-UI etapa 5 (AUTOSAVE): predicado puro de politica por local
    # (cidade sempre ON; dungeon so ON com PEM descoberto OU carta Faraday).
    save_policy_test.cpp
    # MENU-PAUSA-CONFIG-SOM (M7-COSTURA): settings.json proprio (mesmo padrao JSON
    # dep-free de controls_json.hpp), so volume musica/SFX nesta onda.
    system_settings_json_test.cpp
    # M7-DIALOGO (ADR-014): grafo/validate(), parser do formato-texto, runtime.
    dialogue_graph_test.cpp
    dialogue_text_test.cpp
    dialogue_runtime_test.cpp
    # COMBATE-TEORIA-JOGOS [1]: harness de balanceamento OFFLINE (N batalhas headless
    # via o motor ja existente + ScriptedBrain; agregacao win-rate/duracao/dominancia).
    # balance_harness.hpp e header-only (ver target_include_directories abaixo).
    balance_harness_test.cpp
    # Executor techMagic (ADR-016, MVP step 2): dispatcher OnCast/OnDamageDealt/
    # OnDamageReceived/Always (ApplyStatus/Leech/Reflect) via a CombatStateMachine.
    techmagic_executor_test.cpp
    # Executor techMagic (ADR-016, MVP step 3): ledger cross-ator + hook OnRoundEnd +
    # handler HypotenuseCombo (Pythagoras, decisoes Q1-Q4 do lider).
    techmagic_roundend_test.cpp
    # Executor techMagic (ADR-016, MVP step 4, TECHMAGIC-EXECUTOR): catalogo das 10 cartas
    # especiais dos mestres (campos + execucao sem logic_error + paridade i18n).
    master_cards_test.cpp
    # Executor techMagic (ADR-016, MVP step 5, TECHMAGIC-EXECUTOR): EffectKind::
    # RepeatLastAction (Mandelbrot/Fractal-Echo OnCast + Ada/Re-Run OnAllyTurnEnd,
    # decisoes Q1-Q4 do lider 2026-07-14).
    techmagic_repeat_test.cpp
    # Executor techMagic (ADR-016, MVP step 6, TECHMAGIC-EXECUTOR): EffectKind::ChainDamage
    # (Tesla) - cadeia de dano decrescente que salta pros proximos inimigos vivos.
    techmagic_chain_test.cpp
    # Executor techMagic (ADR-016, MVP step 7, TECHMAGIC-EXECUTOR): EffectKind::DelayAction
    # (Einstein/Time-Dilate) - empurra a acao do alvo pro fim da fila da rodada (ou N
    # posicoes), via InitiativeQueue::reorder_actor (mesma primitiva do Gambito).
    techmagic_delay_test.cpp
    # Executor techMagic (ADR-016 Balde B, PR1, decisoes do lider 2026-07-15, F-1..F-4):
    # Faraday/EM-Shield - StatusId::BlindagemEM, SideFilter (side_filter em EffectSpec) e o
    # portao de imunidade CombatActor::try_add_status (choke point compartilhado com o
    # PR2/Newton).
    techmagic_faraday_test.cpp
    # Executor techMagic (ADR-016 Balde B, PR2, decisoes do lider 2026-07-15, N-1..N-4):
    # Newton/Poco Gravitacional - Stun AoE EnemyOnly + Reflect-status AllyOnly (modo-aliado)
    # + Reflect-por-status honrado em apply_damage_with_hooks (soma com a passiva equipada,
    # N-4). Cobre tambem o achado-dominó de resolve_targets(Grupo) (lado hardcoded) e a
    # regra geral "fogo amigo desligado" (Parte A) via regressao em combat_state_machine_test.cpp.
    techmagic_newton_test.cpp
    # Executor techMagic (ADR-016 Balde B, PR3, decisoes do lider 2026-07-15, G-1..G-3):
    # Godel/Null-Proof - StatusId::NullProof (buff-trunfo), item 11 GENERICO
    # (ignores_weakness_wheel wiring real) + o pierce no ATACANTE (fura Resistente/Imune,
    # so consome quando ha algo a furar), paridade preview (estimate_card_damage) e o
    # domino com ChainDamage/Tesla (dano pierced alimenta a cadeia).
    techmagic_godel_test.cpp
)

# Os testes do combate incluem helpers de teste headers-only (fixed_random.hpp).
target_include_directories(gusengine_domain_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# master_cards_test.cpp le os .md de traducao pra checar as chaves CARD_EXEC_<FIGURA>_NAME
# nos 2 locales. Caminho ABSOLUTO injetado aqui (CMAKE_SOURCE_DIR = GusEngine; o repo e o
# pai), robusto ao CWD do ctest (catch_discover_tests roda o binario do build dir).
target_compile_definitions(gusengine_domain_tests PRIVATE
    GUSWORLD_TRANSLATIONS_DIR="${CMAKE_SOURCE_DIR}/../game/translations")

target_link_libraries(gusengine_domain_tests
    PRIVATE
        gusengine_domain
        Catch2::Catch2WithMain
)

target_compile_features(gusengine_domain_tests PRIVATE cxx_std_20)

# Registra os casos no ctest. Rodar com: ctest --output-on-failure
catch_discover_tests(gusengine_domain_tests)
