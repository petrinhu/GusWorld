# gusengine_domain - POCO portado do C#, ZERO Qt E ZERO SDL (ADR-008).
#
# INVARIANTE (engine-design.md sec 2 + ADR-008): este target NAO pode linkar Qt
# NEM SDL. NAO adicione Qt6::*/SDL3::* em target_link_libraries(gusengine_domain).
# Nenhum header aqui pode incluir <Q...> nem <SDL...>. O CI audita ambos nesta pasta.
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
    src/progression/enemy_knowledge_tracker.cpp
    src/progression/xp_differential.cpp
    src/save/save_data.cpp
    src/save/save_slots.cpp
    src/save/save_serializer.cpp
    src/save/save_migrators.cpp
    src/save/save_backup.cpp
    # MENU-INICIAL (ACHADO 1): "estado fresco" de um save Novo Jogo, nomeado/testado -
    # ate esta onda so existia implicito em Maestro::save_{} (default-construction).
    src/save/new_game.cpp
    src/map/tile_map.cpp
    src/map/map_serializer.cpp
    src/map/map_csv.cpp
    src/templates/character_template.cpp
    src/templates/enemy_template.cpp
    src/templates/template_serializer.cpp
    src/templates/template_source.cpp
    src/templates/canonical_templates.cpp
    src/combat/combat_actor.cpp
    src/combat/initiative_queue.cpp
    src/combat/environment_catalog.cpp
    src/combat/environment_transitions.cpp
    src/combat/environment_clock.cpp
    src/combat/combo_table.cpp
    src/combat/scripted_brain.cpp
    src/combat/combat_state_machine.cpp
    # Executor techMagic (ADR-016, MVP step 2): dispatcher OnCast/OnDamageDealt/
    # OnDamageReceived/Always sobre EffectSpec de Card (ApplyStatus/Leech/Reflect).
    src/combat/techmagic.cpp
    # ADR-017 (pedido do Gus + velocidade-por-carta): ActionClock ISOLADO, prova de
    # conceito do escalonador por ticks estilo Pokemon Legends: Arceus. NAO ligado a
    # CombatStateMachine/InitiativeQueue de producao (cutover e onda pos-M7).
    src/combat/action_clock.cpp
    # ATOM-2 (decomposicao atomica ao nivel de modulo, generalizando ADR-019): catalogo
    # de cartas (comuns + especiais dos mestres) movido de src/combat/ pra src/cards/
    # (LAR proprio, cards/ NAO inclui combat/ - gate de camadas). Sem mudanca de
    # comportamento; so o path/namespace.
    src/cards/placeholder_cards.cpp
    src/cards/master_cards.cpp
    src/input/action_registry.cpp
    src/input/controls_name.cpp
    src/input/controls_json.cpp
    src/input/controls_hash.cpp
    src/input/controls_diff.cpp
    src/input/controls_restore.cpp
    # Tela Controles (remap, M2): swap-on-conflict PURO ("avisa e troca").
    src/input/controls_remap_apply.cpp
    # MENU-PAUSA-CONFIG-SOM (M7-COSTURA): settings.json proprio (mesmo padrao JSON
    # dep-free de controls_json.cpp), so volume musica/SFX nesta onda.
    src/settings/system_settings_json.cpp
    # M7-DIALOGO (ADR-014): runtime de dialogo POCO (grafo + parser do
    # formato-texto proprio + motor enter/current/choose/advance/finished).
    src/dialogue/dialogue_graph.cpp
    src/dialogue/dialogue_text.cpp
    src/dialogue/dialogue_runtime.cpp
    # DECK-1: fundacao do sistema de deck/mao (deck-mao-sistema.md). CardCollection e o
    # agregado que garante os invariantes anti-exploit (XOR de container, deck morto
    # inerte, one-way por ausencia de API, guard de tier pra classe protegida).
    src/deck/card_collection.cpp
    # DECK-2: a MAO (loadout) como SELECAO sobre o deck ativo (secao 4/7 inv.2 - a mao
    # nao e container, so guarda instance_id; validada contra CardCollection::active()).
    # hand_capacity() (secao 8c, teto anti-snowball) + slot especial dedicado do Gus
    # (kGusSpecialHandSlots, flag is_universal_compiler) + revalidacao de IDs orfaos.
    src/deck/hand_loadout.cpp
    # DECK-3: transacoes atomicas sell/upload/acquire/craft (secao 6.1/6.2/7/8c) -
    # garantia forte (valida tudo, so entao muta container+wallet), idempotencia
    # (double-sell/double-upload nao credita 2x via TransactionError::InstanceNotInActive),
    # guard de tier herdado de CardCollection::remove_for_sale (Especial/Super nunca
    # saem do ativo), craft desacoplado do inventario de materiais via MaterialConsumer.
    src/deck/deck_transactions.cpp
    # CARDS-HARDWARE-ENGINE incremento 1 (CARDS-HW-1, TODO.md): camada FISICA de
    # carta possuida (CardPhysicalState, AGREGADO das 3 pecas abaixo desde ATOM-1) -
    # CardPhysicalState::validate() delega as pecas. card_hardware_constants.hpp e
    # header-only (sem .cpp proprio).
    src/deck/card_hardware.cpp
    # ATOM-1 (decomposicao atomica ao nivel de modulo, generaliza ADR-019):
    # CardPhysicalState quebrado em 3 pecas componiveis em modulos estreitos.
    src/hardware/card_provenance.cpp
    src/hardware/battery_state.cpp
    src/hardware/hardware_class.cpp
    src/infection/integrity_state.cpp
    # CARDS-HW-2 fatia A (CARDS-HW-2A, TODO.md): servico de diagnostico/cura do
    # Turing (diagnose()/attempt_cure()) - AMB-T1 RESOLVIDA (queima = SUCATA).
    # Mora em deck/ (nao infection/) desde o refactor mecanico de eliminacao de
    # ciclo infection<->deck (CARDS-HW-2A, 2026-07-19) - reusa arestas ja
    # existentes de deck/ para combat/ e para a peca infection/.
    src/deck/turing_service.cpp
)

target_include_directories(gusengine_domain
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(gusengine_domain
    PUBLIC gusengine_core
)

# gus_warnings (HUD-STATUS-ICONS-STALE Fase A): -Wswitch/w14062 warning-only,
# so nos alvos do projeto (raiz do CMakeLists.txt principal define o target).
target_link_libraries(gusengine_domain PRIVATE gus_warnings)

target_compile_features(gusengine_domain PUBLIC cxx_std_20)

# Sem target_link_libraries(... Qt6::*/SDL3::*) por design. Camada pura portada.

# Testes do dominio (Catch2 headless). BUILD_TESTING vem do include(CTest) na
# raiz; os casos sao registrados no mesmo ctest dos testes do M0.
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
