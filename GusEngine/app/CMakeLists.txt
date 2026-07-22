# gusworld_app - executavel GusWorld-specific + a lib gusengine_app (pos repivot
# SDL3, ADR-008).
#
# A logica de tela (OverworldSim) e a casca de janela/loop (SdlWindow) viram uma
# LIB estatica (gusengine_app) pra ser testavel headless (Catch2) e linkavel pelo
# executavel. O executavel (gusworld_app) e so o main.cpp (modo janela + --smoke).

# --- lib gusengine_app: logica de tela + casca de janela/loop (testavel) ---
add_library(gusengine_app STATIC
    src/screens/overworld_sim.cpp
    src/screens/sprite_animation.cpp
    src/screens/player_sprites_loader.cpp
    src/screens/anim_catalog.cpp
    src/screens/anim_preview.cpp
    src/screens/city_scene.cpp
    src/screens/city_loader.cpp
    src/screens/battle_layout.cpp
    src/screens/battle_cockpit_verb_ids.cpp
    # COCKPIT-SFX-HOVER-CLIQUE: POCO generico de "som de hover" de botoes (menu E
    # cockpit delegam pra ca - ver gus/app/screens/ui_hover.hpp).
    src/screens/ui_hover.cpp
    src/screens/battle_hud_model.cpp
    src/screens/battle_menu.cpp
    src/screens/battle_log_model.cpp
    src/screens/battle_floaters.cpp
    src/screens/battle_anim.cpp
    src/screens/battle_sprite_anim.cpp
    src/screens/battle_pacing.cpp
    src/screens/battle_scene.cpp
    # AC-E11 A4 (TU-split, ADR-019): render() + draw_bar/draw_pips + paleta/flavor
    # extraidos de battle_scene.cpp - ver gus/app/screens/battle_scene.hpp (header
    # intocado, mesma classe BattleScene).
    src/screens/battle_scene_render.cpp
    # AC-E11 A5 (TU-split, ADR-019): modo-mira/target selection (§3.5, D3) extraido
    # de battle_scene.cpp - ver gus/app/screens/battle_scene.hpp.
    src/screens/battle_scene_aim.cpp
    # AC-E11 A5 (TU-split, ADR-019): escolha de ator / Janela de Comando da Party
    # (§4.1) extraido de battle_scene.cpp - ver gus/app/screens/battle_scene.hpp.
    src/screens/battle_scene_picker.cpp
    # AC-E11 A1 (ADR-019): roteamento de input (teclado/mouse/SDL->glintfx) extraido de
    # battle_preview.cpp - ver gus/app/screens/battle_input.hpp.
    src/screens/battle_input.cpp
    # AC-E11 A3 (ADR-019): resolucao de paths/loaders de asset extraida de
    # battle_preview.cpp - ver gus/app/screens/battle_assets.hpp.
    src/screens/battle_assets.cpp
    # AC-E11 A2 (ADR-019): montagem do RML/RCSS do cockpit extraida de
    # battle_preview.cpp - ver gus/app/screens/battle_cockpit_rml.hpp.
    src/screens/battle_cockpit_rml.cpp
    src/screens/battle_preview.cpp
    # MENU-PAUSA-CONFIG-SOM (M7-COSTURA): logica pura do menu de pausa + config de som.
    src/screens/system_menu.cpp
    src/screens/system_menu_rml.cpp
    # MENU-PAUSA-CONFIG-SOM (INTEGRACAO FINAL): o loop interativo GL/glintfx (Esc na
    # cidade E na batalha abrem o mesmo menu - ver system_menu_loop.hpp).
    src/screens/system_menu_loop.cpp
    # SAVE-LOAD-UI (PI9, peca que desbloqueia o M7): logica pura da tela de
    # salvar/carregar (slots + preview XP/capitulo/timestamp + confirmacao de
    # sobrescrita) - ver gus/app/screens/save_load_menu.hpp.
    src/screens/save_load_menu.cpp
    src/screens/save_load_menu_rml.cpp
    # SAVE-LOAD-UI etapa 6 (wiring REAL): o loop interativo GL/glintfx que faz o
    # I/O de disco de fato (has_save/save_game/load_game) - ver
    # gus/app/screens/save_load_menu_loop.hpp.
    src/screens/save_load_menu_loop.cpp
    # SAVE-LOAD-UI etapa 4 (TELA DE TITULO): logica pura + gerador de RML + o
    # loop GL/glintfx dono do proprio contexto (owning, MESMA receita de
    # system_menu_loop.cpp::run_system_menu_loop_owning_gl) - ver
    # gus/app/screens/title_menu_loop.hpp.
    src/screens/title_menu.cpp
    src/screens/title_menu_rml.cpp
    src/screens/title_menu_loop.cpp
    # MODOS-MORTE Fase 0 (decisao do lider 2026-07-10): tela de selecao de
    # dificuldade no fluxo de Novo Jogo - logica pura + RML + loop ANINHADO no
    # MESMO contexto GL da tela de titulo (MESMA receita de
    # run_save_load_menu_loop_gl_current) - ver gus/app/screens/difficulty_menu.hpp.
    src/screens/difficulty_menu.cpp
    src/screens/difficulty_menu_rml.cpp
    src/screens/difficulty_menu_loop.cpp
    src/i18n/translator.cpp
    # M7-DIALOGO (NPC-MVP): I/O do grafo de dialogo do Bertoldo (.dlg.txt) -
    # fronteira app/, domain/dialogue permanece ZERO I/O.
    src/dialogue/npc_dialogue_catalog.cpp
    src/screens/npc_dialogue_overlay.cpp
    # DIALOGO-TERMINAL (caixa quente com retrato real, RCSS/glintfx): gerador de
    # RML puro + o loop GL dedicado (MESMA receita de contexto GL owning do
    # system_menu_loop.cpp, ver npc_dialogue_loop_gl.hpp).
    src/screens/npc_dialogue_rml.cpp
    src/screens/npc_dialogue_loop_gl.cpp
    src/screens/npc_dialogue_loop.cpp
    src/sdl_window.cpp
    # APP-ICON: aplica o icone do app (opcao C do mock, Gus combate + glow cyan) na
    # SDL_Window - chamado por SdlWindow::init() E por Maestro::init() (a dona da
    # UNICA janela do modo normal, ver maestro.hpp/app_icon.hpp).
    src/app_icon.cpp
    # M7-COSTURA (ADR-012 Onda 1): maestro leve cidade<->batalha (ver maestro.hpp).
    src/maestro_logic.cpp
    src/maestro.cpp
    # M7-COSTURA Inc 2c: sequencia de frames pre-renderizada (boot pixelizado) no
    # lugar do glitch procedural (aposentado - o lider VETOU o visual ao vivo,
    # "pareceu bug"; ver gus/app/boot_pixel_overlay.hpp).
    src/boot_pixel_overlay.cpp
    # Diagnostico opcional de device de audio real (M6 F1, ADR-011 item 5).
    # GUSWORLD_AUDIO_SMOKE=1: ver app/include/gus/app/audio_smoke.hpp.
    src/audio_smoke.cpp
)

target_include_directories(gusengine_app
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    # stb_image_write (vendorizado header-only): captura de frame em PNG para o smoke
    # visual do ADR-009 (battle_preview). IMPLEMENTACAO definida em battle_preview.cpp.
    PRIVATE ${CMAKE_SOURCE_DIR}/third_party/stb
)

# Caminho ABSOLUTO da pasta resources/ do repo (raiz = pai de GusEngine/), embutido
# para o loader de sprites resolver os assets do Caua quando rodando do build dir
# (a pasta resources/ vive FORA do git, junto do codigo). Override em runtime via
# env GUSWORLD_ASSETS. ${CMAKE_SOURCE_DIR} = GusEngine/ (raiz do projeto CMake).
target_compile_definitions(gusengine_app
    PRIVATE GUSWORLD_ASSETS_DIR="${CMAKE_SOURCE_DIR}/../resources"
    # ASSETS-VFS-F1c: GUSWORLD_MAPS_DIR NAO fica mais aqui - o resolver de mapas migrou
    # pro porteiro (FilesystemAssetSource, familia MAPS em platform/), que carrega a macro
    # do gusengine_platform. city_scene.cpp agora delega pro porteiro, entao nenhuma TU de
    # app/ lia esta def (era leftover inofensivo, removido na higiene pos-verificacao).
    # Pasta dos catalogos de traducao (resources/translations/) DENTRO do repo (raiz =
    # pai de GusEngine/; M8 decommission moveu de game/translations/ via git mv). O
    # Translator de UI le pt_br.md daqui rodando do build dir; override em runtime via
    # env GUSWORLD_TRANSLATIONS (caminho completo do .md).
    PRIVATE GUSWORLD_TRANSLATIONS_DIR="${CMAKE_SOURCE_DIR}/../resources/translations"
    # Pasta dos grafos de dialogo (.dlg.txt, M7-DIALOGO) DENTRO do repo (raiz = pai
    # de GusEngine/), MESMA receita/raiz-irma de GUSWORLD_TRANSLATIONS_DIR acima
    # (M8 decommission moveu de game/dialogues/ via git mv).
    # Override em runtime via env GUSWORLD_DIALOGUES (caminho completo do .dlg.txt).
    PRIVATE GUSWORLD_DIALOGUES_DIR="${CMAKE_SOURCE_DIR}/../resources/dialogues"
    # Pasta das fontes (.ttf) DENTRO de GusEngine/ (mesma macro do platform/). ADR-010
    # F2a: o cockpit BAKED do glintfx registra a "Pixel Operator Mono" via @font-face no
    # RCSS (o glintfx::UiLayer NAO expoe Rml::LoadFontFace - so o doc carrega fonte). Sem
    # esta macro o app/ nao saberia o caminho absoluto da fonte. Override: env GUSWORLD_FONTS.
    PRIVATE GUSWORLD_FONTS_DIR="${CMAKE_SOURCE_DIR}/assets/fonts"
    # Pasta do kit CC0 de SFX (M6 F2/F3, ADR-011), na RAIZ do repo (irma de resources/ e
    # GusEngine/ - "assets/" de origem/source do JOGO, diferente de GusEngine/assets/ que
    # e asset DA ENGINE). O battle_preview (owner do AudioEngine) le hit_digital_
    # provisorio.wav daqui rodando do build dir; override em runtime via env GUSWORLD_SFX.
    PRIVATE GUSWORLD_SFX_DIR="${CMAKE_SOURCE_DIR}/../assets/sfx"
    # Pasta da musica CC0 (M6 F4, ADR-011), irma de assets/sfx/ (mesma raiz "assets/" do
    # JOGO). O battle_preview le cidade_tema_provisorio.mp3 daqui; override em runtime
    # via env GUSWORLD_MUSIC.
    PRIVATE GUSWORLD_MUSIC_DIR="${CMAKE_SOURCE_DIR}/../assets/music"
)

# SDL3 entra aqui (a casca SdlWindow + o main tocam SDL diretamente).
target_link_libraries(gusengine_app
    PUBLIC
        gusengine_core
        gusengine_domain
        gusengine_platform
        SDL3::SDL3
)

target_compile_features(gusengine_app PUBLIC cxx_std_20)

# ADR-010 F3: glintfx::UiLayer (embed mode) e o UNICO motor de UI/HUD. Linkado SEMPRE, SO
# aqui em app/ (GATE de 4 camadas: nunca core/ nem domain/). Sem flag - o backend RmlUi
# vendorizado foi aposentado (two-way door via historico git).
target_link_libraries(gusengine_app PRIVATE glintfx::glintfx)

# gus_warnings (HUD-STATUS-ICONS-STALE Fase A): -Wswitch/w14062 warning-only,
# so nos alvos do projeto (raiz do CMakeLists.txt principal define o target).
target_link_libraries(gusengine_app PRIVATE gus_warnings)

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
        SDL3::SDL3
        gus_warnings
)

target_compile_features(gusworld_app PRIVATE cxx_std_20)

# Testes da camada app/ (Catch2, headless).
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
