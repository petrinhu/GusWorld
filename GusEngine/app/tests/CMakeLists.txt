# Testes da camada app/ (Catch2 v3, headless). O OverworldSim e desenhado num
# IRenderer FALSO (sem GPU), e o movimento/colisao/camera/interpolacao/sprite sao
# exercitados deterministicamente. A LOGICA de animacao (sprite_animation) e POCO
# pura. NAO linka Qt: a MAIORIA roda sem abrir janela nenhuma.
#
# EXCECAO: save_load_menu_interaction_test.cpp ABRE uma janela SDL ESCONDIDA +
# contexto GL real (Xvfb :99 ou equivalente) pra exercitar glintfx::UiLayer de
# verdade (harness de POSICIONAMENTO/INTERACAO, SAVE-LOAD-UI) - degrada seguro
# (0 assercoes, NUNCA falha) se GL/display estiver indisponivel no host que roda
# o ctest, ver o comentario no topo daquele arquivo.
#
# Um executavel agregado + catch_discover_tests, no padrao de core/+domain/.

add_executable(gusengine_app_tests
    overworld_sim_test.cpp
    sprite_animation_test.cpp
    sprite_anchor_test.cpp
    anim_catalog_test.cpp
    player_sprites_layout_test.cpp
    city_scene_test.cpp
    battle_layout_test.cpp
    battle_cockpit_verb_ids_test.cpp
    # COCKPIT-SFX-HOVER-CLIQUE: POCO generico de "som de hover" de botoes.
    ui_hover_test.cpp
    battle_hud_model_test.cpp
    battle_menu_test.cpp
    battle_log_model_test.cpp
    battle_floaters_test.cpp
    battle_anim_test.cpp
    battle_sprite_anim_test.cpp
    battle_pacing_test.cpp
    battle_scene_test.cpp
    battle_key_routing_test.cpp
    translator_test.cpp
    maestro_logic_test.cpp
    boot_pixel_overlay_test.cpp
    # M7-DIALOGO (NPC-MVP): overlay funcional simples + I/O do grafo do Bertoldo.
    npc_dialogue_overlay_test.cpp
    npc_dialogue_catalog_test.cpp
    npc_dialogue_save_roundtrip_test.cpp
    # DIALOGO-TERMINAL: gerador de RML da caixa quente (retrato real).
    npc_dialogue_rml_test.cpp
    # DIALOGO-TERMINAL (opcoes clicaveis + numeradas): fatia PURA exposta do
    # loop GL (npc_dialogue_digit_for_key), MESMO racional de
    # battle_key_routing_test.cpp.
    npc_dialogue_loop_gl_test.cpp
    # MENU-PAUSA-CONFIG-SOM (M7-COSTURA): logica pura do menu de pausa + config de som.
    system_menu_test.cpp
    system_menu_rml_test.cpp
    # SAVE-LOAD-UI (PI9): logica pura da tela de salvar/carregar + gerador de RML.
    save_load_menu_test.cpp
    save_load_menu_rml_test.cpp
    # SAVE-LOAD-UI etapa 4 (TELA DE TITULO): logica pura de navegacao/confirmacao +
    # gerador de RML.
    title_menu_test.cpp
    title_menu_rml_test.cpp
    # MODOS-MORTE Fase 0 (decisao do lider 2026-07-10): tela de selecao de
    # dificuldade - logica pura de navegacao/confirmacao + gerador de RML.
    difficulty_menu_test.cpp
    difficulty_menu_rml_test.cpp
    # SAVE-LOAD-UI (retoque ao vivo do lider, bugs 1-9 + feature "Apagar"): harness
    # HEADLESS com GL real (Xvfb :99) de POSICIONAMENTO/INTERACAO - degrada seguro
    # (0 assercoes) se GL/display indisponivel. ui_box_assertions.hpp e so header
    # (helpers genericos), nao entra aqui.
    save_load_menu_interaction_test.cpp
)

target_link_libraries(gusengine_app_tests
    PRIVATE
        gusengine_app
        gusengine_core
        gusengine_domain
        gusengine_platform
        Catch2::Catch2WithMain
)

# save_load_menu_interaction_test.cpp inclui <glintfx/ui_layer.hpp> direto (harness
# GL) - gusengine_app ja linka glintfx::glintfx PRIVATE (nao propaga includes pra
# quem so linka gusengine_app), entao o teste precisa do link PROPRIO aqui, MESMA
# fronteira "so em app/" do GATE de arquitetura (glintfx nunca em core/domain/).
target_link_libraries(gusengine_app_tests PRIVATE glintfx::glintfx)

# GUSWORLD_FONTS_DIR: o harness de interacao precisa injetar a MESMA fonte real
# que save_load_menu_loop.cpp injeta em producao (achado empirico: SEM a fonte
# real, o fallback do RmlUi tem metricas DIFERENTES -> altura de linha diferente
# -> a posicao medida pelo harness diverge da posicao real que o loop usa,
# fazendo o teste medir um alvo ERRADO). MESMO valor/receita de app/CMakeLists.txt
# e platform/CMakeLists.txt (CMAKE_SOURCE_DIR = raiz GusEngine/, NAO
# CMAKE_CURRENT_SOURCE_DIR - as fontes vivem em GusEngine/assets/fonts).
target_compile_definitions(gusengine_app_tests
    PRIVATE GUSWORLD_FONTS_DIR="${CMAKE_SOURCE_DIR}/assets/fonts"
)

target_compile_features(gusengine_app_tests PRIVATE cxx_std_20)

catch_discover_tests(gusengine_app_tests)
