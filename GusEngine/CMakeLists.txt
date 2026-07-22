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
FetchContent_Declare(
    glintfx
    GIT_REPOSITORY https://codeberg.org/petrinhu/glintfx
    GIT_TAG        v0.9.2
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
