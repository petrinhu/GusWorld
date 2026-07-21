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
    # MENU-INICIAL (ACHADO 1): fresh_new_game_save_data - "estado fresco" de save
    # Novo Jogo, nomeado/testado (ate esta onda so implicito em Maestro::save_{}).
    new_game_test.cpp
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
    # DECK-4: persistencia do sistema de deck/mao no save (V5->V6, CardCollectionState
    # + credits + hand_selection por personagem, EM SUBSTITUICAO ao `deck` legado).
    save_v6_test.cpp
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
    # Executor techMagic (ADR-016 manifesto item 5, decisoes do lider 2026-07-15): Planck/
    # Quantum-Lock - EffectKind::DamageQuantize quantiza o canal COMUM em 3 degraus fixos
    # (piso/centro/teto, chances 25%/50%/25%), paridade preview<->real, fronteiras do
    # sorteio de degrau, determinismo/contagem de RNG (substitui next_double por next(100),
    # mesma contagem), escopo por-ator, critico/falha intactos, colapso graceful.
    techmagic_quantize_test.cpp
    # Executor techMagic (ADR-016 step 8, manifesto item 6, decisoes D1-D4 do lider
    # 2026-07-15): John Dee/Black-Mirror - EffectKind::RevealIntent (buff StatusId::Scrying
    # + dump read-only de IntentPreview via log_intent_for/dump_reveal_intent), re-dump na
    # fronteira de rodada (CombatStateMachine::process_scrying_hooks), D2 (intent CAOTICO
    # do Patch-Zero retorna ruido, nunca revela), fail-soft no brain ausente (assimetria vs
    # Gambito manual), Scan aprimorado (status/fila/intent, D1-ii), stub posse-only (D1-i) e
    # o TESTE-REI de determinismo (HP/dano/ordem/RNG byte-identicos com e sem Scrying).
    techmagic_reveal_test.cpp
    # Executor techMagic (ADR-016 manifesto item 12, decisao do lider 2026-07-16, AMB-08):
    # Maxwell/Spectra-Wave ("Onda Unificada") - dano-base PURO (effects vazio, ZERO
    # EffectKind novo) via o MESMO caminho Grupo/AoE do Newton. AoE em todos os inimigos
    # vivos, paridade preview<->real (estimate_card_damage) nas fronteiras do canal COMUM, e
    # o modo-aliado como no-op puro (dissipa via a regra geral "fogo amigo desligado", sem
    # EffectSpec pra rodar).
    techmagic_maxwell_test.cpp
    # Executor techMagic (CARD-ENGINE-MANIFESTO item 7, AMB-09, decisao do lider
    # 2026-07-16): Hayek/Free-Order - EffectKind::DiversityBonus (marker fora do
    # dispatcher, mesmo padrao de DamageQuantize/Planck). Ledger novo round_actions_
    # (granularidade de ACAO via resolve_action, primo do round_hits_ do Pythagoras); M2
    # (UseCard refinado por CardFamily); forward-only anti auto-inflacao; degraus
    # 2/3/4+ distintas = +5%/-1pp, +8%/-2pp, +13%/-3pp; beneficia o LADO INTEIRO
    # enquanto o dono estiver vivo/equipado; paridade preview<->real (carta E ataque
    # basico); determinismo de RNG identico com/sem Hayek (CountingRandom); piso de
    # falha 0.
    techmagic_hayek_test.cpp
    # Executor techMagic (CARD-ENGINE-MANIFESTO item 9, decisao do lider 2026-07-16):
    # Mises/Calc-Edge - EffectKind::ApEfficiency (marker fora do dispatcher, mesmo padrao
    # de DamageQuantize/DiversityBonus). Face 1 (+1 AP no turno do dono equipado, nao
    # persiste); Face 2 ("comando central": atores CombatActor::central_command() do lado
    # oposto ao dono da Mises sofrem atraso de fila via regroup_round_by_side + desconto
    # -13% de dano, carta E ataque basico, gemeo preview<->real); tag sem Mises ativa e
    # Mises sem tag = no-op (com log); determinismo de RNG (CountingRandom); handler
    # no-op no dispatcher.
    techmagic_mises_test.cpp
    # Executor techMagic (CARD-ENGINE-MANIFESTO item 8, ultimo step do manifesto, AMB-11):
    # von Neumann/Fork + Giordano Bruno/Echo-Self - EffectKind::CloneAlly (status-based,
    # StatusId::Eco) + EffectKind::TokenRefund (marker fora do dispatcher). handle_clone_ally
    # (OnCast, side_filter AllyOnly dissipa em inimigo) + techMagic::replicate_eco (fim de
    # CADA turno PROPRIO do portador, last_action_.actor == portador, take_damage PURO) +
    # CombatStateMachine::process_eco_turn_end_hook (os 2 sitios de fim-de-turno, ANTES de
    # expire_elapsed_statuses) + token_refund_equipped_on_side (gate specials_cast_ de
    # resolve_use_card). TESTE-REI: composicao Ada (RepeatLastAction/OnAllyTurnEnd) + Eco no
    # mesmo ator/rodada nao recursa nem dobra indevidamente (os dois usam take_damage puro,
    # nenhum toca last_action_/round_hits_).
    techmagic_clone_test.cpp
    # ADR-017 (pedido do Gus + velocidade-por-carta): ActionClock ISOLADO. Prova a
    # formula de reset (BASE_CLOCK/SPD x cardSpeedMult x styleMult), os 3 estilos +
    # decaimento do Agil consecutivo, o desempate D3, o starvation-guard (D1 hibrido) e
    # a PendingResolution contra os 6 casos de exemplo do pedido do Gus. NAO liga a
    # CombatStateMachine/InitiativeQueue de producao (cutover e onda pos-M7).
    action_clock_test.cpp
    # DECK-1 (fundacao do sistema de deck/mao, deck-mao-sistema.md secao 7/8c):
    # CardCollection - XOR de container, deck morto inerte, one-way por ausencia de
    # API, guard de tier pra classe protegida (Especial/Super), capacidade 34/55/89.
    card_collection_test.cpp
    # DECK-2 (mao/loadout como selecao, deck-mao-sistema.md secao 3/4/7/8c): HandLoadout
    # - anti-dup (so guarda instance_id), so-deck-ativo, teto de comuns, slot especial
    # dedicado do Gus (kGusSpecialHandSlots), hand_capacity() (teto/delta Bento),
    # atomicidade de set_selection/swap_card, revalidacao de IDs orfaos.
    hand_loadout_test.cpp
    # DECK-3 (transacoes atomicas, deck-mao-sistema.md secao 6.1/6.2/7/8c):
    # sell/upload/acquire/craft - garantia forte (valida tudo, so entao muta container+
    # wallet), idempotencia (double-sell/double-upload nao credita 2x), guard de tier
    # herdado (Especial/Super nunca saem do ativo), craft desacoplado de materiais via
    # MaterialConsumer (capacidade checada ANTES de consumir).
    deck_transactions_test.cpp
    # DECK-5 (revisao ADVERSARIAL INDEPENDENTE, qa-engineer): property-based testing
    # "a mao" (property_gen.hpp) sobre os invariantes anti-exploit do deck/mao inteiro
    # (secao 7: XOR de container, sem duplicata, mao so-do-ativo pos-revalidate,
    # carteira nao-negativa/conservada, classe protegida fora do morto, roundtrip do
    # save V6) sob sequencias aleatorias de sell/upload/acquire/craft/add/discard/
    # hand-mutations. Mutation testing "com dente" (mutantes revertidos) documentado
    # no relatorio da onda DECK-5, nao no codigo.
    deck_invariants_property_test.cpp
    # REGRESSAO de lifetime (heap-use-after-free): tier_of (TierLookup) recebia uma
    # REFERENCIA pro card_id dentro de active(); um tier_of que realoque active_ (add_to_
    # active) fazia essa referencia pendurar (UAF deterministico sob ASan, crash
    # intermitente ASLR-dependente no build normal). Casos com tier_of adversarial que
    # realoca o buffer ANTES de ler o card_id - crashavam pre-fix, passam pos-fix (copia
    # estavel do card_id + reancoragem do iterador antes do erase).
    deck_tier_callback_lifetime_test.cpp
    # CARTAS-COMUNS-ENGINE (TODO.md, decisoes do lider 2026-07-16): 2 pecas de engine pras
    # cartas COMUNS (record-base secao 7 + formula divisiva secao 11, ZERO EffectKind/
    # StatusId novo - NAO toca o executor techMagic/ADR-016). PECA 1 SynergyStatus
    # (Finalizador Opcao A, generaliza mult_expose pra qualquer StatusId, +40% fixo sem
    # stack, gemeo preview<->real, paridade com Planck/Quantum-Lock). PECA 2 recarga de
    # AP/mana (Eletrico-utilidade "Tavus-Overclock", trava 1x/turno resetada no refresh de
    # TurnStart, custo sempre pago antes, Silence bloqueia).
    cartas_comuns_engine_test.cpp
    # CARDS-HARDWARE-ENGINE incremento 1 (CARDS-HW-1, TODO.md): camada FISICA de
    # carta possuida (origem ROM/EPROM/pirata, bateria CR2032, integridade/virus
    # oculto, conector RSB) - CardPhysicalState::validate() + funcoes puras
    # derivadas (SoH, conector, classe-de-hardware) + tabela de constantes
    # (bateria por dificuldade, contaminacao por classe).
    card_hardware_test.cpp
    # CARDS-HW-1: persistencia do save V6->V7 (CardInstance::physical em cada
    # CardInstance ativo/morto) - migrator puro, fixture serialize_save_v6
    # congelando o layout antigo, roundtrip, defesa em profundidade (ordinal fora
    # de faixa em payload forjado).
    save_v7_test.cpp
    # ATOM-1 (decomposicao atomica ao nivel de modulo, generaliza ADR-019):
    # cobertura FOCADA de cada peca componivel de CardPhysicalState, isolada do
    # agregado (que continua coberto, zero diff, por card_hardware_test.cpp).
    card_provenance_test.cpp
    battery_state_test.cpp
    integrity_state_test.cpp
    hardware_class_test.cpp
    # CARDS-HW-2 fatia 1 (VIRUS EM COMBATE, TODO.md): ponte instancia->combate
    # (CardIntegrityRef/integrity_ledger, CombatAction::card_instance_id) + os 3
    # payloads desta fatia (LogicBomb pre-cast/Worm+ZipBomb pos-cast) no
    # resolve_use_card. docs/design/mecanicas/cartas-spec-logica.md secao 1/4/5.2.
    card_virus_combat_test.cpp
    # CARDS-HW-2 fatia A (CARDS-HW-2A, TODO.md): servico de diagnostico/cura do
    # Turing (diagnose()/attempt_cure(), roll 62/38%, guard tier protegido + guard
    # nao-diagnosticado, AMB-T1 RESOLVIDA - queima = SUCATA) + mapeamento puro
    # outcome->chave i18n. docs/design/mecanicas/cartas-spec-logica.md secao 6.
    turing_service_test.cpp
    # CARDS-HW-2 fatia B (CARDS-HW-2B, TODO.md): carta `urandom` (carta-caos do Gus) -
    # redireciona pra um efeito JA EXISTENTE no catalogo (record-base OU techMagic::execute
    # conforme o tier da carta sorteada), sorteado por peso (original x pirata, backfire
    # 1/3 exato na pirata). docs/design/mecanicas/cartas-spec-logica.md secao 7.
    urandom_test.cpp
    # CARDS-HW-2C (Backdoor SIGNAL-ONLY, TODO.md): CombatState::leaked_intel(), populado
    # pela CombatStateMachine a partir do integrity_ledger (owner_actor_id de carta
    # Backdoor-infectada). SO o sinal - nenhum brain pondera ainda (UtilityBrain e onda
    # futura); ScriptedBrain permanece byte-identico com/sem o sinal.
    # docs/design/mecanicas/cartas-spec-logica.md secao 4.2.
    backdoor_signal_test.cpp
    # CARDS-HW-3B (TODO.md): rolagem de contaminacao NA AQUISICAO - a fatia que ACENDE
    # o sistema de virus (roll_contamination_on_acquisition() + pick_weighted_payload(),
    # gus/domain/deck/contamination_service.hpp). docs/design/mecanicas/
    # cartas-spec-logica.md secao 5.1 (AMB-07 RESOLVIDA); cartas-numeros-proposta.md
    # secao 3/3a.
    contamination_service_test.cpp
    # CARDS-HW-3C (TODO.md): logica de dominio do Adware Sterling - flag de catalogo
    # (Card::has_adware, opt-in) + AdwareExposureTracker (gus/domain/combat/
    # adware_sterling.hpp, AMB-11 resolvida pelo lider) + gate pre-cast em
    # resolve_use_card (ANTES do debito de recurso, secao 1). docs/design/mecanicas/
    # cartas-spec-logica.md secao 1/4.1/9. A tela glintfx (overlay 5s + X) e fatia
    # futura de UI, FORA deste teste.
    adware_sterling_test.cpp
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
        gus_warnings
)

target_compile_features(gusengine_domain_tests PRIVATE cxx_std_20)

# Registra os casos no ctest. Rodar com: ctest --output-on-failure
catch_discover_tests(gusengine_domain_tests)
