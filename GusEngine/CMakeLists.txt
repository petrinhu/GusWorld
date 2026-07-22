# GusEngine - raiz do build (pos repivot SDL3, ADR-008).
#
# Arquitetura de 4 camadas (engine-design.md secao 2, atualizada por ADR-008):
#   core/      POCO C++ puro, ZERO Qt E ZERO SDL, headless, 100% testavel
#   domain/    POCO portado do C#, ZERO Qt E ZERO SDL (save/i18n/progression/...)
#   platform/  fronteira SDL3, UNICA camada (com app/) que conhece SDL
#   app/       executavel GusWorld-specific (casca SDL: janela + loop proprio)
#
# INVARIANTE AUDITAVEL (ADR-008): core/ e domain/ NAO incluem nem linkam Qt NEM
# SDL. So platform/ e app/ tocam SDL. Os targets gusengine_core e gusengine_domain
# JAMAIS recebem SDL3::* em target_link_libraries. O CI audita via grep de includes
# <Q...> e <SDL...> nessas pastas.

cmake_minimum_required(VERSION 3.21)

project(GusEngine
    VERSION 0.1.0
    DESCRIPTION "Engine propria do GusWorld (C++/SDL3, pivot pos-Qt)"
    LANGUAGES CXX
)

# Monocypher (ADR-015 Onda 1, SAVE-CRYPTO-V2-VENDOR) e C99 puro, nao C++: habilita
# a linguagem C so para compilar esse UNICO vendor (third_party/monocypher/). Nao
# afeta o resto do projeto (100% C++20).
enable_language(C)

# C++20 idiomatico, sem extensoes de compilador (portavel Linux/Windows).
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Exporta compile_commands.json para clang-format/clangd/IDE.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# ===========================================================================
#  Dependencias de plataforma via FetchContent (build reprodutivel Linux+Win).
#  Decisao (ADR-008 + relatorio): SDL3 e RmlUi nao sao header-only como as libs de
#  third_party/; entram por FetchContent com PIN de tag, compiladas do fonte. Isso
#  evita depender do que cada distro empacota e fixa o ABI. Catch2 ja seguia esse
#  padrao. Primeira config baixa o fonte; depois fica cacheado em build/_deps.
# ===========================================================================

# --- SDL3 (janela + render2d + input/gamepad). Pin = release-3.4.10 (casa com a
#     libSDL3 do sistema atual; estavel). Linkada ESTATICA (binario menor, deploy
#     trivial, licenca zlib permite). Desliga testes/exemplos do SDL.
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.4.10
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(SDL3)

# --- RmlUi (UI/HUD do jogador, ADR-009). Pin = 6.3, SHA exato do glintfx (ADR-010 F0):
#     2cd28864ae25ed345b70598751703a5433b12356. Alinhar a ABI do RmlUi com a versao que
#     o glintfx fixa e PRE-REQUISITO da adocao do embed mode (UiLayer) - ver ADR-010.
#     Desliga samples; FONT_ENGINE=freetype (ADR-009 DECISAO 1, ratificada): caminho
#     suportado do RmlUi, com font-effect (glow/outline) de graca; a fonte real (Pixel
#     Operator Mono) e rasterizada pelo FreeType. RmlUi usa find_package(Freetype) (NAO
#     FetchContent), entao o FreeType vem do sistema (Linux: libfreetype-dev; cross-compile
#     Windows = ponto de atencao R-font, plano B = custom stb). Static (BUILD_SHARED_LIBS=OFF).
#     Linkado por platform/rmlui/ + app/ (NUNCA por core/ nem domain/ - GATE).
#     NOTA: pin por SHA e INCOMPATIVEL com GIT_SHALLOW (shallow so busca o tip do ref);
#     por isso o GIT_SHALLOW foi removido deste Declare (fetch completo necessario p/ o SHA).
#
#     PATCH_COMMAND (APP-ASAN-RMLUI-UBSAN, 2026-07-10): o glintfx aplica
#     glintfx/patches/rmlui-2cd28864-teardown-ub.patch (2 UBs genuinos de ordem-de-teardown
#     no proprio RmlUi - vptr:ElementDocument + vptr:*WidgetScroll.cpp*/vptr:*RemoveEventListener*,
#     ver docs/embed-integration.md secao 18 do glintfx) via PATCH_COMMAND na PROPRIA
#     FetchContent_Declare(RmlUi ...) dele - mas o glintfx so e fetchado DEPOIS deste bloco
#     (linha ~120 abaixo). Como o FetchContent e idempotente-por-nome (a 1a Declare/
#     MakeAvailable de um nome vence; populacoes seguintes so reusam a arvore ja populada,
#     sem rerodar PATCH_COMMAND), a NOSSA Declare(RmlUi) - que vem primeiro - e quem decide
#     se o patch entra ou nao: sem PATCH_COMMAND aqui, o PATCH_COMMAND do glintfx nunca
#     chega a rodar (a populacao ja aconteceu do nosso lado, sem patch), confirmado por
#     leitura direta do rmlui-src fetchado (Element.cpp identico ao upstream com UB).
#     Fix: vendorizar a MESMA copia do patch em GusEngine/patches/ (consumo legitimo -
#     nao e modificar a lib glintfx, so aplicar o patch dela do nosso lado, exatamente como
#     a secao 18 do embed-integration.md prescreve para "host declara RmlUi primeiro") e
#     repetir aqui o mesmo idiom de probe idempotente que o glintfx usa (git apply
#     --reverse --check | git apply) para o PATCH_COMMAND sobreviver a reconfiguracoes
#     limpas que encontram o source ja populado E ja patcheado.
#     Reverificar sempre que o GIT_TAG acima for bumpado (o patch pode deixar de aplicar
#     limpo, ja estar corrigido a montante, ou ainda ser necessario - mesma disciplina do
#     glintfx/patches/README.md).
set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_FONT_ENGINE "freetype" CACHE STRING "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(_gusengine_rmlui_patch ${CMAKE_CURRENT_SOURCE_DIR}/patches/rmlui-2cd28864-teardown-ub.patch)
FetchContent_Declare(
    RmlUi
    GIT_REPOSITORY https://github.com/mikke89/RmlUi.git
    GIT_TAG        2cd28864ae25ed345b70598751703a5433b12356
    PATCH_COMMAND  sh -c "git apply --reverse --check '${_gusengine_rmlui_patch}' >/dev/null 2>&1 || git apply '${_gusengine_rmlui_patch}'"
)
FetchContent_MakeAvailable(RmlUi)

# --- glintfx (ADR-010 F3: UNICO motor de UI/HUD via EMBED mode, glintfx::UiLayer). Compilado
#     e linkado SEMPRE (o backend RmlUi vendorizado foi aposentado neste F3). Sem CMake option:
#     nao ha mais caminho alternativo. two-way door via historico git.
#
#     GLINTFX_BACKEND_GLFW=OFF (FORCE, ANTES do FetchContent) => embed-only: o glintfx NAO
#     chama find_package(glfw3), NAO linka glfw e NAO compila App/window_glfw/RmlUi_Platform_GLFW.
#     O glintfx FIXA o RmlUi no MESMO SHA ja declarado acima
#     (2cd28864ae25ed345b70598751703a5433b12356) => o FetchContent_MakeAvailable e idempotente
#     por nome: o RmlUi do GusEngine (declarado PRIMEIRO) vence e o glintfx o reusa (sem conflito
#     de versao). Por isso este bloco vem DEPOIS do MakeAvailable(RmlUi) de proposito.
#     Linkado SO em app/ (GATE de 4 camadas: nunca core/ nem domain/).
#
#     APP-ASAN-RMLUI-UBSAN (2026-07-10): esta mesma idempotencia-por-nome e o motivo pelo
#     qual o PATCH_COMMAND do glintfx (dentro do proprio glintfx/CMakeLists.txt, no bloco
#     FetchContent_Declare(RmlUi ...) DELE) nunca chega a rodar aqui - quando o glintfx e
#     fetchado (MakeAvailable(glintfx) abaixo), o RmlUi ja foi POPULADO pela nossa Declare
#     de cima, entao a Declare/MakeAvailable(RmlUi) interna do glintfx e um no-op (reusa a
#     arvore ja populada, sem rerodar PATCH_COMMAND algum). Por isso o patch tem que estar
#     no PATCH_COMMAND da NOSSA Declare(RmlUi) (ver acima) - vendorizado em
#     GusEngine/patches/rmlui-2cd28864-teardown-ub.patch a partir da copia do glintfx -
#     nao no bloco do glintfx. O SHA/patch dos dois lados e o mesmo, entao o resultado final
#     (arvore RmlUi patcheada) e identico ao que o glintfx entregaria sozinho; so muda QUEM
#     efetivamente aplica.
set(GLINTFX_BACKEND_GLFW OFF CACHE BOOL "" FORCE)  # embed-only (sem GLFW)
# SOURCE_SUBDIR glintfx: o CMakeLists.txt do projeto vive no subdir glintfx/ (a raiz do repo
# NAO tem CMakeLists.txt); e o que o consumer-example oficial do glintfx aponta. Sem isto,
# MakeAvailable popula mas NAO adiciona o subdir => alvo glintfx::glintfx nao existe.
#
# GLINTFX-INTEGRACAO F1 (2026-07-21): a partir da v0.12.0 o glintfx atomizou em modulos
# opt-in GLINTFX_MODULE_* (framework 2D, ADR-0015 do lado deles). Fixando EXPLICITAMENTE
# os 3 que usamos e desligando o resto, em vez de depender do default de cada flag:
#   - GLINTFX_MODULE_TEXT ON: reservada mas FORCADA ON nesta versao (FATAL_ERROR se OFF -
#     ainda nao ha caminho OFF real, ver comentario da propria option() no CMakeLists do
#     glintfx). Setar ON aqui e so documentar a dependencia, nao muda nada.
#   - GLINTFX_MODULE_FX ON: usamos polygon()/image-tint() como decorator RCSS (recolor de
#     carta) - sem este modulo os 3 TUs de decorator (decorator_polygon/image_tint/ripple)
#     saem do binario e o RCSS correspondente vira no-op silencioso (fail-high documentado
#     do lado deles, mas silencioso pra nos = regressao visual). Ja e o default ON; fixado
#     aqui por clareza/estabilidade contra bump futuro que troque o default.
#   - GLINTFX_MODULE_AUDIO ON (GLINTFX-INTEGRACAO F2, 2026-07-22): a partir desta onda o
#     audio do jogo vem do modulo glintfx::Audio (glintfx_audio, OBJECT library dobrada no
#     alvo glintfx STATIC - ver comentario do bump abaixo) em vez do miniaudio vendorizado
#     nosso (third_party/miniaudio, aposentado deste bloco em diante - continua no disco ate
#     a limpeza fisica do M9). platform/src/audio/audio_engine.cpp (fachada AudioEngine,
#     header intocado) consome glintfx::Audio por baixo do PImpl.
#   - GLINTFX_MODULE_GAMEPAD/I18N OFF: nao usamos ainda (i18n = translator C++ proprio no
#     domain; gamepad = fora de escopo). Real ON/OFF (nao reservada) - default de fabrica e
#     ON pros 2; desligar aqui evita linkar codigo/dep que nao usamos (ex.: nao expor evdev).
#   - GLINTFX_MODULE_WINDOW OFF: alias de GLINTFX_BACKEND_GLFW (ja fixado OFF acima desde
#     o embed-only original); o glintfx v0.16 espelha o valor de BACKEND_GLFW dentro de
#     MODULE_WINDOW automaticamente (ver comentario "alias target" no CMakeLists deles) -
#     nao precisa ser setado 2x, so registrado aqui pra quem procurar a flag pelo nome novo.
#   - "UI" e "INPUT" NAO SAO flags reais nesta versao (nomes RESERVADOS pela ADR-0015 (b/c)
#     do lado deles, ainda nao introduzidos - "UI" e o produto inteiro de hoje sem gate,
#     "INPUT" foi decidido D1 a nunca existir como nome guarda-chuva, gamepad ganhou atomo
#     proprio GLINTFX_MODULE_GAMEPAD em vez de cair dentro de MODULE_INPUT). Nada a setar.
#
# GLINTFX_OWN_FONT_ENGINE OFF (opt-out do font-flip, ADR-009): desde a v0.10 o default de
# RUNTIME do UiLayer/App virou FontEngine::Own (motor de fonte proprio clean-room do
# glintfx, nao FreeType) - ver UiLayerConfig::font_engine em glintfx/include/glintfx/
# ui_layer.hpp. Nos usamos FreeType (PixelOperatorMono, goldens calibrados por DPI); o
# opt-out documentado do lado deles e de UMA linha em UiLayerConfig::font_engine, mas isso
# tocaria nosso codigo de consumo (fora do escopo desta bump, "consumo intocado"). Em vez
# disso, desligamos o motor proprio em TEMPO DE BUILD: com GLINTFX_OWN_FONT_ENGINE=OFF o
# modulo nem entra no binario, e o proprio Bootstrap::init() deles cai no fallback pra
# FreeType SEMPRE que alguem (nos, ou codigo futuro que nunca sete font_engine) pedir
# FontEngine::Own - fail-safe, nao fail-silent (loga 1 warning por init(), documentado no
# doc-comment de FontEngine). Resultado: rasterizacao byte-identica sem tocar 1 linha de
# app/, mais forte que so setar o campo runtime (cobre qualquer call site futuro tambem).
# Trade-off registrado: pos-M8, o item GLINTFX-FONTFLIP-VALIDATE (A/B dos 2 motores) vai
# precisar religar esta flag ON + reconfigure pra experimentar o motor proprio de novo -
# nao e regressao, e o preco de "fail-safe por padrao" ate la.
set(GLINTFX_MODULE_TEXT      ON  CACHE BOOL "" FORCE)
set(GLINTFX_MODULE_FX        ON  CACHE BOOL "" FORCE)
set(GLINTFX_MODULE_AUDIO     ON  CACHE BOOL "" FORCE)
set(GLINTFX_MODULE_GAMEPAD   OFF CACHE BOOL "" FORCE)
set(GLINTFX_MODULE_I18N      OFF CACHE BOOL "" FORCE)
set(GLINTFX_OWN_FONT_ENGINE  OFF CACHE BOOL "" FORCE)
#
# BUMP v0.2.4 -> v0.2.5 (decisao Caetano/CTO 2026-07-02, higiene pre-M7): a v0.2.5 entrega
# UiLayer::set_click_callback (hit-test de elemento NATIVO do glintfx, reportando o id do
# elemento clicado) - mata a divida do hit-test manual dos 6 pills de verbo do cockpit
# (gus/app/screens/battle_cockpit_pills.hpp, aposentado neste bump). O 5-arg set_viewport
# (letterbox) tambem veio nesta versao mas FICA NO BACKLOG (GLINTFX-LETTERBOX no TODO.md;
# so o callback de clique entra agora). get_element_box (2a API nova) tambem NAO e usado
# pro hit-test de clique aqui - so pro reposicionamento do cursor sintetico do
# GUSWORLD_BATTLE_HOVER_SELFTEST (diagnostico preexistente que dependia da geometria
# manual aposentada; ver battle_preview.cpp).
#
# BUMP v0.3.1 -> v0.4.0 (M2/GLINTFX-SCROLL, commit glintfx 7ee1bc7): a v0.4.0 entrega
# SCROLL de verdade em embed mode - UiEvent::Type::MouseWheel (forward da roda do
# mouse pro elemento em HOVER) e UiLayer::scroll_element_into_view(id, align_with_top)
# (scroll programatico pra acompanhar selecao de teclado). Antes so ~6 das 30 acoes
# da tela Controles eram alcancaveis (sem scroll, a lista transbordava e nao rolava).
# Ver system_menu_loop.cpp (wheel forwarding + scroll_element_into_view no
# handle_mouse_motion/navegacao) - controls_row_visible_in_list (BUG-A, filtro de
# hit-test por recorte visivel) RECONCILIADO com a geometria JA ROLADA que
# get_element_box passa a devolver.
#
# BUMP v0.9.2 -> v0.16.0 (GLINTFX-INTEGRACAO F1, decisao do lider 2026-07-21, saida do
# freeze framework-2D): gap de 6 minors (v0.10-v0.16), aditivo - CHANGELOG do glintfx nao
# remove API existente ate aqui. Verificado antes do bump (leitura do CMakeLists deles
# apos fetch): pin do RmlUi continua no MESMO SHA 2cd28864 (nosso patch teardown-UB segue
# aplicavel, reverificado - diff byte-a-byte contra patches/rmlui-2cd28864-teardown-ub.patch
# deles = identico); alvo glintfx::glintfx (ALIAS) continua existindo (consumo em
# app/CMakeLists.txt intocado). Consumo real: ZERO ajuste de codigo em app/ - so este
# bloco de CMake (GIT_TAG + os GLINTFX_MODULE_*/GLINTFX_OWN_FONT_ENGINE explicitos acima,
# ver os comentarios deles pro raciocinio completo por flag: TEXT/FX ON, AUDIO/GAMEPAD/
# I18N/WINDOW OFF, motor de fonte proprio OFF pra manter FreeType byte-identico). Ganhos
# que vieram de graca com o bump (nao pedidos, so herdados): GLSYM-COLLISION resolvido
# desde a v0.11.2 (globais do loader prefixados glx_, ver memoria reference_glintfx_api.md
# secao GLSYM-COLLISION - preocupacao de link-order que tinhamos documentada como risco
# latente agora fechada, sem nada a fazer do nosso lado) + hardening geral das versoes
# intermediarias. Build local + suite completa (2394 testes) verdes no pin novo antes
# deste commit (ver relatorio da onda GLINTFX-INTEGRACAO-F1).
#
# REVERT v0.16.0 -> v0.9.2 (2026-07-21, commit 17ea2a0): o CI Windows (MSVC) pegou o
# que o build local Linux nao pegava - o CMakeLists do glintfx setava -Wall -Wextra sem
# guard de compilador, e o MSVC rejeita essas flags GCC/Clang com erro D8021. Revertido
# pra v0.9.2 ate o upstream consertar.
#
# RE-BUMP v0.9.2 -> v0.18.0 (GLINTFX-INTEGRACAO F1, re-tentativa 2026-07-22): o glintfx
# lancou v0.17.0 e v0.18.0 consertando os dois problemas que o CI Windows achou no bump
# anterior - (1) guard de generator expression em torno de -Wall -Wextra (so aplica em
# GCC/Clang, nao MSVC) e (2) um segundo bug encontrado no processo: windows.h define
# macros min/max que colidem com std::min/std::max, resolvido com NOMINMAX +
# WIN32_LEAN_AND_MEAN em escopo de diretorio do lado deles. O glintfx agora tem job de
# CI Windows MSVC real buildando o CMake principal deles, verde. Revalidado nesta versao
# antes do re-bump (ver relatorio da onda GLINTFX-INTEGRACAO-F1 re-bump): guard MSVC
# confirmado presente; flags GLINTFX_MODULE_*/GLINTFX_OWN_FONT_ENGINE usadas aqui
# continuam existindo sem renomeacao; pin do RmlUi continua no MESMO SHA 2cd28864 (patch
# teardown-UB segue aplicavel). AUDIO continua OFF nesta fase (troca de audio e a F2,
# pacote separado). Build local + suite completa (2394 testes) verdes no pin novo antes
# deste commit.
#
# GLINTFX-INTEGRACAO F2 (2026-07-22, mesmo pin v0.18.0 - so a flag muda): liga
# GLINTFX_MODULE_AUDIO=ON (ver o comentario da propria flag, acima, pro raciocinio
# completo). A v0.18.0 trouxe as 4 features de audio pedidas (loop/fade nativos,
# play_oneshot polifonico com cap+voice-stealing, is_playing) que faltavam quando o
# GLINTFX-INTEGRACAO-F1 ainda deixava o modulo OFF. Consumo real:
# platform/src/audio/audio_engine.cpp reescrito para falar com glintfx::Audio (ver o
# cabecalho daquele arquivo); platform/CMakeLists.txt agora linka glintfx::glintfx e
# nao inclui mais third_party/miniaudio (aposentado deste bloco em diante, so a
# remocao FISICA do vendored fica pro M9). Suite completa (2394 testes, incluindo os
# 21 de platform/tests/audio_engine_test.cpp intactos) verde antes deste commit.
# SHIM TEMPORARIO DE PORTABILIDADE (GLINTFX-INTEGRACAO-F2, 2026-07-22) - libm no MSVC.
# O glintfx v0.18.0 linka a libm do Unix no modulo de audio SEM guard de plataforma:
#   glintfx/CMakeLists.txt:1151
#   target_link_libraries(glintfx_audio PRIVATE Threads::Threads ${CMAKE_DL_LIBS} m)
# (curiosidade: a linha 1152, logo abaixo, JA tem o guard por compilador que pedimos no
# report anterior - o "m" e que passou batido). No MSVC nao existe "m.lib" (a math vive
# na CRT), e como glintfx_audio e STATIC e dobrado no alvo glintfx, a dependencia
# propaga pro link FINAL do consumidor: quem quebra somos nos, com
#   LINK : fatal error LNK1181: cannot open input file 'm.lib'
# O CI Windows do MSVC deles nao pega isso porque COMPILA os alvos mas nao LINKA um
# consumidor com o modulo de audio ligado (sugestao ja enviada a eles pelo bus).
#
# O shim: um alvo INTERFACE vazio chamado "m" so no Windows. O CMake resolve o NOME "m"
# para este alvo em vez de procurar m.lib no disco; no Unix o alvo nem e criado e o -lm
# de verdade continua valendo. Precisa vir ANTES do MakeAvailable (e quando o
# target_link_libraries deles e avaliado).
# REMOVER quando o glintfx taggear o guard de plataforma (dor reportada pelo bus em
# 2026-07-22); a remocao sai no mesmo commit do bump que trouxer o fix.
if(WIN32 AND NOT TARGET m)
    add_library(m INTERFACE)
endif()

FetchContent_Declare(
    glintfx
    GIT_REPOSITORY https://codeberg.org/petrinhu/glintfx
    GIT_TAG        v0.18.0
    SOURCE_SUBDIR  glintfx
)
FetchContent_MakeAvailable(glintfx)

# --- Testes (Catch2 v3). core/+domain/+platform/+app/ testados headless. ---
include(CTest)        # define BUILD_TESTING (ON por padrao)
enable_testing()

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.7.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)
# disponibiliza o helper catch_discover_tests (integra Catch2 ao ctest)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

# --- Vendor com build proprio (ADR-015 Onda 1: Monocypher e 2 arquivos C, nao
#     header-only como o resto de third_party/; precisa de um alvo compilado). ---
add_subdirectory(third_party/monocypher)

# --- gus_warnings (onda HUD-STATUS-ICONS-STALE, Fase A) ---------------------
# INTERFACE target so com flags de warning, SEM fontes proprias. Linkado PRIVATE
# nos alvos do PROJETO (core/domain/platform/app + seus executaveis de teste),
# NUNCA em third_party/ nem nos alvos vindos de FetchContent (SDL3/RmlUi/glintfx/
# Catch2/monocypher) - o box2d vendorizado (third_party/box2d/), por exemplo, tem
# switch sem `default:` que dispararia o warning e nao e nosso codigo pra corrigir.
#
# Fase A (HUD-STATUS-ICONS-STALE) foi so a REDE: warning-only, pra inventariar os
# switches nao-exaustivos do projeto (enumeration value ... not handled in
# switch). Fase B (SA-1/gameplay_engineer) consertou os casos encontrados
# (battle_hud_model.cpp: 20 StatusId; balance_harness.hpp: case StatusTick).
# Fase D (promocao, esta mudanca): com o projeto ja limpo, -Wswitch vira ERRO
# (Werror=switch/we4062) - switch nao-exaustivo novo QUEBRA o build a partir daqui,
# em vez de so avisar.
add_library(gus_warnings INTERFACE)
target_compile_options(gus_warnings INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Werror=switch>
    $<$<CXX_COMPILER_ID:MSVC>:/we4062>
)

# --- Camadas ---
# Ordem importa: core e base de domain; platform usa core; app usa todos.
add_subdirectory(core)
add_subdirectory(domain)
add_subdirectory(platform)
add_subdirectory(app)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
