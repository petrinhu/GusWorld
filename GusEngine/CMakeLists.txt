# SPDX-License-Identifier: Apache-2.0
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
#     nosso (era third_party/miniaudio; removido do disco na higiene do M9 - o miniaudio
#     que sobra vem vendorizado pelo glintfx em _deps/glintfx-src/glintfx/third_party/
#     miniaudio/). platform/src/audio/audio_engine.cpp (fachada AudioEngine,
#     header intocado) consome glintfx::Audio por baixo do PImpl.
#   - GLINTFX_MODULE_GAMEPAD ON CONDICIONADO A LINUX (F4-2.4, 2026-07-24): o modulo e
#     evdev CRU (`/dev/input/eventN`) - o proprio header publico dele
#     (glintfx/include/glintfx/gamepad.hpp) tem um `#error` fora de
#     GLINTFX_MODULE_GAMEPAD=ON ("Linux-only module... no portable substitute"), e o CMake
#     deles ja auto-desliga a flag em qualquer CMAKE_SYSTEM_NAME != Linux. Ligar isto
#     INCONDICIONALMENTE quebraria o CI Windows (MSVC) no #error - por isso o guard
#     `if(UNIX AND NOT APPLE)` abaixo (nunca `WIN32`/`APPLE`; ver GlintfxGamepadInput,
#     platform/include/gus/platform/input/glintfx_gamepad_input.hpp, o adaptador que
#     consome isto). GLINTFX_MODULE_I18N OFF: nao usamos (i18n = translator C++ proprio
#     no domain) - real ON/OFF (nao reservada), default de fabrica ON; desligar aqui evita
#     linkar codigo/dep que nao usamos.
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
# F4-2.4: ON so em Linux (UNIX AND NOT APPLE exclui macOS de propósito - o modulo e
# evdev cru, especifico do kernel Linux; macOS e UNIX mas nao tem evdev). Em qualquer
# outro CMAKE_SYSTEM_NAME (Windows, macOS) fica OFF - INEGOCIAVEL, ver o comentario
# grande acima ("GLINTFX_MODULE_GAMEPAD ON CONDICIONADO A LINUX") para o raciocinio do
# `#error` guard que isto evita estourar no CI Windows (MSVC).
if(UNIX AND NOT APPLE)
    set(GLINTFX_MODULE_GAMEPAD ON  CACHE BOOL "" FORCE)
else()
    set(GLINTFX_MODULE_GAMEPAD OFF CACHE BOOL "" FORCE)
endif()
set(GLINTFX_MODULE_I18N      OFF CACHE BOOL "" FORCE)
set(GLINTFX_OWN_FONT_ENGINE  OFF CACHE BOOL "" FORCE)
#
# GLINTFX_MODULE_APP OFF (BUMP v0.20.0 -> v0.23.0, flag nova nesta versao, ADR-0015 (b) do
# lado deles): "app" e a fachada App standalone (janela+loop proprios GLFW), o oposto do
# nosso modo (embed-only, casca SDL propria hospedando glintfx::UiLayer). A propria option()
# deles ja default pro valor de GLINTFX_MODULE_WINDOW (que e alias de GLINTFX_BACKEND_GLFW,
# ja FORCADO OFF acima) - ou seja, ja herdariamos OFF de graca por dependencia de DAG deles
# (com FATAL_ERROR se alguem tentasse ligar APP com WINDOW OFF). Fixado aqui EXPLICITO por
# clareza/estabilidade contra bump futuro (mesma disciplina de TEXT/FX/AUDIO/GAMEPAD/I18N
# acima) e porque e o unico jeito de o gate ficar visivel pra quem le este arquivo sem
# precisar saber da regra de default-por-dependencia do lado deles.
set(GLINTFX_MODULE_APP       OFF CACHE BOOL "" FORCE)
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
# nao inclui mais third_party/miniaudio (removido do disco na higiene do M9; o
# miniaudio que sobra vem vendorizado pelo glintfx em _deps/glintfx-src/glintfx/
# third_party/miniaudio/). Suite completa (2394 testes, incluindo os
# 21 de platform/tests/audio_engine_test.cpp intactos) verde antes deste commit.
#
# BUMP v0.18.0 -> v0.18.1 (GLINTFX-LIBM-SHIM, 2026-07-22): o glintfx taggeou o guard de
# plataforma que faltava na linha do "m" (libm) do modulo glintfx_audio - confirmado por
# leitura do CMakeLists deles apos o fetch desta versao:
#   target_link_libraries(glintfx_audio PRIVATE Threads::Threads ${CMAKE_DL_LIBS}
#     $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:m>)
# Com o guard a montante, o shim temporario de portabilidade (alvo INTERFACE "m" vazio
# so no Windows) que tinhamos aqui deixou de ser necessario e foi removido neste mesmo
# commit, como planejado no proprio comentario do shim.
#
# BUMP v0.18.1 -> v0.19.0 (GLINTFX-INTEGRACAO, 2026-07-22): aditiva, CI deles 9/9 verde
# (inclui MSVC). Traz canal de input fisico no App (is_key_down/is_mouse_button_down/
# get_cursor_pos), set_key_callback de borda, ciclo de vida de janela (set_close_
# request_callback com veto, request_close, foco/minimizacao) e throttle do flood de log
# do RmlUi ("No font face defined"). Nada disso e consumido aqui - o input/lifecycle e
# pre-requisito da F4 (migracao da casca SDL pro App mode), decisao separada; so o
# throttle de log tem efeito passivo (nosso embed passa pelo mesmo caminho de log).
# Revalidado apos fetch: flags GLINTFX_MODULE_TEXT/_FX/_AUDIO/_GAMEPAD/_I18N/_WINDOW e
# GLINTFX_BACKEND_GLFW/GLINTFX_OWN_FONT_ENGINE continuam com os mesmos nomes; guard de
# plataforma do libm (GLINTFX-LIBM-SHIM) continua em glintfx_audio; pin do RmlUi continua
# no MESMO SHA 2cd28864 (patch teardown-UB segue aplicavel). Suite completa verde no pin
# novo antes deste commit.
#
# BUMP v0.19.0 -> v0.20.0 (GLINTFX-INTEGRACAO, 2026-07-23): aditiva, CHANGELOG deles nao
# remove API existente. Traz GLINTFX_MODULE_DRAW2D (sprites + batching por-textura sobre
# pipeline GL 3.3 proprio, ADR-0017 do lado deles, ligado por padrao ON) + o carve interno
# de image-decode que o sustenta (D2D-1A, GLINTFX_MODULE_IMAGE segue reservada/forcada ON,
# inalterada) + um fix de isolamento Xvfb/CI (XDG_RUNTIME_DIR por processo de teste - nao
# nos afeta, nosso CI usa xvfb-run com o proprio wrapper deles). NAO adotamos o Draw2D
# agora (decisao do lider): ele desenha em coordenada de TELA mas o jogo inteiro desenha em
# coordenada de MUNDO com camera (platform/render2d/ tem 6 primitivas proprias; Draw2D
# cobre 1 delas). Este bump e consumo intocado - ZERO ajuste em app/.
#
# GLINTFX_MODULE_DRAW2D=OFF (setado abaixo, mesma disciplina de GAMEPAD/I18N: codigo que
# nao consumimos nao deveria entrar no binario) - VERIFICADO NO BINARIO REAL, nao só na
# .a intermediaria (hipotese inicial de "linker de archive ja exclui sozinho" testada e
# REFUTADA por nm): com ON (default), `nm -C gusworld_app` mostra 25 simbolos
# glintfx::Draw2d::*/draw2d_detail::SpriteBatch::* REALMENTE PRESENTES no executavel final
# (nao só na .a), ~40KB a mais (11622032 vs 11582872 bytes) - MESMO SEM NADA em app/
# referenciar Draw2d (confirmado por nm --undefined-only em TODOS os outros .a/.o do link:
# zero referencia a qualquer simbolo Draw2d). Causa raiz isolada com `ld -y` +
# intersecao de simbolos definidos-vs-indefinidos: draw2d.cpp.o e puxado pro link inteiro
# nao por Draw2d em si, mas por UM simbolo COMDAT generico que ele tambem define -
# `std::string::append(const char*)` (instanciacao de template usada em Impl::log_warn) -
# que colide com o mesmo simbolo fraco pedido por outro .o em qualquer lugar do programa;
# ld resolve o primeiro membro de arquivo que define o simbolo pedido e leva o .o inteiro
# junto (arquivo estatico so seleciona por-.o, nao por-simbolo) - o codigo Draw2d que
# sobra e morto/inalcancavel (nada chama draw_sprite/load_texture/etc), mas ocupa espaco
# porque o projeto nao compila com -ffunction-sections/-fdata-sections nem linka com
# --gc-sections (mudanca maior, fora de escopo deste bump). OFF corta a raiz (o .o nem
# entra no build), entao e a unica forma de garantir zero-custo hoje - nao e so
# "disciplina de arvore de build", e reducao REAL, confirmada, do binario que entregamos.
#
# Revalidado apos fetch: flags GLINTFX_MODULE_TEXT/_FX/_AUDIO/_GAMEPAD/_I18N/_WINDOW e
# GLINTFX_BACKEND_GLFW/GLINTFX_OWN_FONT_ENGINE continuam com os mesmos nomes; guard de
# plataforma do libm (GLINTFX-LIBM-SHIM) continua em glintfx_audio; pin do RmlUi continua
# no MESMO SHA 2cd28864 (patch teardown-UB segue aplicavel). Suite completa verde no pin
# novo antes deste commit.
#
# BUMP v0.20.0 -> v0.23.0 (D2D-2-SENTINELA, 2026-07-29): SO ALINHAMENTO DE PIN, sem adotar
# nenhuma API nova (Draw2D camera/transforms/primitivas/scissor/texto sao decisao futura do
# lider, item D2D-2-SENTINELA/GLINTFX-FONTFLIP-VALIDATE - NAO consumidas aqui). Ritual de
# bump revalidado por leitura direta do CMakeLists.txt deles na tag v0.23.0 (baixado via
# `gh api repos/petrinhu/glintfx/contents/glintfx/CMakeLists.txt?ref=v0.23.0`):
#   - Flags GLINTFX_MODULE_TEXT/_FX/_AUDIO/_GAMEPAD/_I18N/_WINDOW e GLINTFX_BACKEND_GLFW/
#     GLINTFX_OWN_FONT_ENGINE continuam com os mesmos nomes e mesma semantica.
#   - Guard de libm em glintfx_audio continua por COMPILADOR
#     ($<$<NOT:$<CXX_COMPILER_ID:MSVC>>:m>), inalterado desde a v0.18.1.
#   - Pin do RmlUi deles continua no MESMO SHA 2cd28864ae25ed345b70598751703a5433b12356 (o
#     patch teardown-UB nosso segue aplicavel - o nosso Declare(RmlUi) vem primeiro e decide,
#     ver comentario grande no bloco RmlUi acima).
#   - Vendors deles (gamecontrollerdb/khronos/miniaudio/stb) conferidos apos o configure
#     novo contra THIRD-PARTY-LICENSES.md - ver o proprio doc pro resultado desta rodada.
#   - GLINTFX_MODULE_APP (flag NOVA nesta faixa, ADR-0015 (b), o agregador a-la-carte da
#     fachada App standalone): FORCADA OFF explicitamente logo abaixo do bloco de flags
#     acima (nao aqui) - ver o comentario proprio dela pro raciocinio (ja herdariamos OFF de
#     graca via GLINTFX_MODULE_WINDOW=OFF, mas fixar explicito documenta a escolha).
#   - GOTCHA do motor de fonte (L1.20-FONTFLIP): `UiLayerConfig::font_engine` (glintfx/
#     include/glintfx/ui_layer.hpp) tem default FontEngine::Own desde a v0.10 - nos NUNCA
#     setamos esse campo em nenhum ponto de construcao de glintfx::UiLayer::Config no
#     GusEngine (grep confirmado, zero ocorrencia). Verificado que Bootstrap::init()
#     (glintfx/src/bootstrap.cpp) cai no fallback fail-safe pra FreeType quando o binario e
#     compilado com GLINTFX_OWN_FONT_ENGINE=OFF (nosso caso, ja fixado acima) mesmo que o
#     chamador peca Own - loga 1 warning por init(), nunca quebra. Mecanismo ja documentado
#     no comentario da propria flag GLINTFX_OWN_FONT_ENGINE (acima, nesta secao) desde antes
#     deste bump; a conferencia desta rodada foi RE-VERIFICAR que continua valendo
#     caractere-a-caractere na v0.23.0 (bootstrap.cpp linhas ~994-1020) - RESULTADO: continua
#     valendo, ZERO mudanca de comportamento, e por isso NENHUM call site de app/ precisou
#     setar FontEngine::FreeType explicito (resultado negativo provado, nao mudanca
#     defensiva no escuro - GLINTFX-FONTFLIP-VALIDATE, a decisao de A/B testar o motor
#     proprio de verdade, segue pendente e e do lider).
# Build local + suite completa (2576 testes) verdes no pin novo antes deste commit.
#
# GLINTFX_MODULE_DRAW2D: OFF -> ON (D2D-2-SENTINELA, 2026-07-29, autorizado pelo lider -
# "o líder autorizou migrar o nosso platform/render2d pro Draw2D do glintfx"): esta fatia
# ADOTA de verdade o Draw2D pela primeira vez, via a TERCEIRA implementacao de IRenderer
# (Render2dGlintfx, platform/render2d/render2d_glintfx.*) que delega ao glintfx::Draw2d 5
# das 6 primitivas (tudo MENOS draw_text - GLINTFX-FONTFLIP-VALIDATE, decisao do lider,
# pendente). ON e INDISPENSAVEL pra este consumo compilar/linkar: o modulo
# `glintfx_draw2d` (OBJECT lib, glintfx/CMakeLists.txt) SO existe com a flag ON - com OFF
# nenhum simbolo glintfx::Draw2d::* e definido em lugar nenhum do binario glintfx, entao
# QUALQUER TU que chame Draw2d::init/begin/draw_sprite/etc falha no LINK (nao no runtime),
# independente de o backend novo ser INSTANCIADO ou nao em tempo de execucao - o custo de
# ~40KB medido no bump anterior (comentario ON->OFF acima) e o preco de trazer o Draw2D
# pro binario AGORA que ha um consumidor real dele (deixou de ser "codigo morto que nao
# deveriamos carregar" pra ser "codigo vivo, so nao selecionado por default" - ver
# GUSWORLD_RENDER2D_BACKEND em platform/CMakeLists.txt, default "gl3", troca reversivel
# pra "glintfx"). O ganho de ~40KB nao muda: o binario cresce de qualquer forma quando
# GLINTFX_MODULE_DRAW2D=ON, seja o backend novo selecionado ou nao (os DOIS backends sao
# SEMPRE compilados, so a instanciacao e condicional) - aceito porque o consumo agora e
# REAL, nao especulativo. NAO reabre a questao do font-engine: o gate widening
# `GLINTFX_OWN_FONT_ENGINE OR GLINTFX_MODULE_DRAW2D` (glintfx/CMakeLists.txt, ADR-0018 (e))
# so controla se os PATHS vendorizados do motor de fonte proprio sao compilados pro
# Draw2D's OWN (nao usado por nos) draw_text/measure_text - o SELETOR runtime do lado ui
# (UiLayerConfig::font_engine, Bootstrap::init()) continua INTOCADO e cai no fallback
# fail-safe pra FreeType do jeito que ja caia (GLINTFX_OWN_FONT_ENGINE continua OFF, ZERO
# mudanca de comportamento do HUD RmlUi). Build local + suite completa verdes com a flag
# ON (`nm` re-confirmado: simbolos Draw2d::* agora presentes E referenciados de verdade).
set(GLINTFX_MODULE_DRAW2D    ON  CACHE BOOL "" FORCE)

# BUMP v0.23.0 -> v0.24.0 (FONT-EXTEND-GLITCH, 2026-07-29): traz UI-FONTFACE
# (glintfx::UiLayer::load_font_face/glintfx::App::load_font_face, glintfx/include/glintfx/
# font_face.hpp) - a API que expoe Rml::LoadFontFace programaticamente, matando o
# contorno que este projeto tinha (injetar `@font-face` no RML por concatenacao de string
# procurando a substring "<style>\n"; ver o commit desta mesma onda que consome a API em
# app/src/screens/). Drop-in a partir da v0.23.0 (nenhuma assinatura existente mudou, so
# API nova aditiva) - ZERO ajuste de codigo neste bloco de CMake, so o GIT_TAG. Ritual de
# bump revalidado por leitura direta do CMakeLists.txt deles na tag v0.24.0 (baixado via
# `gh api repos/petrinhu/glintfx/contents/glintfx/CMakeLists.txt?ref=v0.24.0`):
#   - Flags GLINTFX_MODULE_TEXT/_FX/_AUDIO/_GAMEPAD/_I18N/_WINDOW/_DRAW2D/_APP e
#     GLINTFX_BACKEND_GLFW/GLINTFX_OWN_FONT_ENGINE continuam com os mesmos nomes e mesma
#     semantica (inclusive o FATAL_ERROR de GLINTFX_MODULE_APP sem GLINTFX_MODULE_WINDOW e
#     o de GLINTFX_MODULE_IMAGE/_TEXT=OFF, ainda reservadas/forcadas ON).
#   - Guard de libm em glintfx_audio continua por COMPILADOR
#     ($<$<NOT:$<CXX_COMPILER_ID:MSVC>>:m>), inalterado desde a v0.18.1.
#   - Pin do RmlUi deles continua no MESMO SHA 2cd28864ae25ed345b70598751703a5433b12356 (o
#     patch teardown-UB nosso segue aplicavel - o nosso Declare(RmlUi) vem primeiro e decide,
#     ver comentario grande no bloco RmlUi acima).
#   - Vendors deles (gamecontrollerdb/khronos/miniaudio/stb) conferidos via
#     `gh api repos/petrinhu/glintfx/contents/glintfx/third_party?ref=v0.24.0` - MESMOS 4 do
#     pin anterior, nenhum vendor novo. THIRD-PARTY-LICENSES.md atualizado so na data/pin.
#   - GOTCHA NOVO desta versao (ASSIMETRIA DE MOTOR, ver o proprio doc-comment de
#     font_face.hpp): com `family = nullptr`, FontEngine::Own deriva a familia do STEM do
#     arquivo (`opensans-regular`) enquanto FreeType (o motor que rodamos,
#     GLINTFX_OWN_FONT_ENGINE=OFF) le a tabela 'name' do SFNT (`Open Sans`) - os call sites
#     novos em app/src/screens/ SEMPRE passam `family` explicito ("Pixel Operator Mono",
#     igual ao que o RCSS ja referenciava), o que neutraliza a assimetria nos dois motores.
# Build local + suite completa verdes no pin novo antes deste commit (contagem de testes no
# relatorio da onda FONT-EXTEND-GLITCH).
#
# BUMP v0.24.0 -> v0.25.0 (STB-IMAGE-APP, 2026-07-29): traz IMG-DECODE
# (glintfx::decode_image_file/decode_image_memory, glintfx/include/glintfx/image.hpp) - par de
# free functions que decodifica PNG/JPG/TGA em pixels RGBA8 alpha STRAIGHT (nao-premultiplicado)
# sem exigir contexto GL nenhum, complementando o `Draw2d::create_texture(pixels, w, h,
# PixelFormat)` que ja existia (D2D-TEXPIXELS) e que so cobria a metade "pixels -> textura GPU"
# do pipeline. Consumo real (mesma onda): app/src/app_icon.cpp trocou stbi_load() direto por
# glintfx::decode_image_file() - unico uso GENUINO de decode de stb_image em app/ (os outros 8
# arquivos que o grep inicial listou usam stb_image_write, que e ENCODE/escrita de PNG pra
# screenshot, fora do escopo desta API e nao tocados). Drop-in a partir da v0.24.0 (nenhuma
# assinatura existente mudou, so API nova aditiva) - ZERO ajuste neste bloco de CMake, so o
# GIT_TAG. Ritual de bump revalidado por leitura direta do CMakeLists.txt deles na tag v0.25.0
# (baixado via `gh api repos/petrinhu/glintfx/contents/glintfx/CMakeLists.txt?ref=v0.25.0`):
#   - Flags GLINTFX_MODULE_TEXT/_FX/_AUDIO/_GAMEPAD/_I18N/_WINDOW/_DRAW2D/_APP/_IMAGE e
#     GLINTFX_BACKEND_GLFW/GLINTFX_OWN_FONT_ENGINE continuam com os mesmos nomes e mesma
#     semantica (inclusive os FATAL_ERROR de dependencia entre GLINTFX_MODULE_APP/_WINDOW e o
#     de GLINTFX_MODULE_IMAGE/_TEXT=OFF, ainda reservadas/forcadas ON).
#   - Guard de libm em glintfx_audio continua por COMPILADOR
#     ($<$<NOT:$<CXX_COMPILER_ID:MSVC>>:m>), inalterado desde a v0.18.1.
#   - Pin do RmlUi deles continua no MESMO SHA 2cd28864ae25ed345b70598751703a5433b12356 (o
#     patch teardown-UB nosso segue aplicavel - o nosso Declare(RmlUi) vem primeiro e decide,
#     ver comentario grande no bloco RmlUi acima).
#   - Vendors deles (gamecontrollerdb/khronos/miniaudio/stb) conferidos via
#     `gh api repos/petrinhu/glintfx/contents/glintfx/third_party?ref=v0.25.0` - MESMOS 4 do
#     pin anterior, nenhum vendor novo. THIRD-PARTY-LICENSES.md atualizado so na data/pin.
# Build local + suite completa verdes no pin novo antes deste commit (contagem de testes no
# relatorio da onda STB-IMAGE-APP).
#
# BUMP v0.25.0 -> v0.26.0 (fatia 1 de log-e-relogio, 2026-07-30): traz FW-LOG
# (glintfx::set_log_sink/log/log_info/log_warn/log_error, glintfx/include/glintfx/log.hpp) e
# FW-CLOCK (glintfx::monotonic_now_ns(), glintfx/include/glintfx/clock.hpp) - as duas pecas que
# estavam bloqueando a Fatia 1 do docs/tech/plano-camadas-sdl.md (proibido reimplementar em casa
# o que cabe no glintfx). Consumo real (mesma onda): ~30 SDL_Log() em app/ trocados por
# glintfx::log()/log_info()/log_warn()/log_error() (via plana quando a mensagem embute texto
# externo - caminho de arquivo, erro de terceiro; variadica so quando o formato e literal do
# nosso codigo); ~17 SDL_GetTicksNS() trocados por glintfx::monotonic_now_ns() (assinatura
# identica uint64_t(*)(), troca de 1 token em app/src/screen_state.cpp:69 e nos demais pontos).
# Ambos os headers vivem em glintfx_core (OBJECT library SEMPRE-ON, junto de system_clock.cpp/
# version.cpp - ver comentario deles no CMakeLists deles), sem flag GLINTFX_MODULE_* nova: ZERO
# ajuste de flag neste bloco de CMake, so o GIT_TAG. FW-LOG hardeneado contra CWE-134 (escrita
# arbitraria via `%n`/`%1$n`): `fmt` da API variadica e validado por ALLOWLIST de especificadores
# de conversao (nao blocklist - o proprio glintfx documenta que uma blocklist anterior deles
# deixou passar a forma posicional POSIX `%1$n`, ja corrigido antes desta tag). Ritual de bump
# revalidado por leitura direta do CMakeLists.txt deles na tag v0.26.0 (baixado via
# `gh api repos/petrinhu/glintfx/contents/glintfx/CMakeLists.txt?ref=v0.26.0`):
#   - Flags GLINTFX_MODULE_TEXT/_FX/_AUDIO/_GAMEPAD/_I18N/_WINDOW/_DRAW2D/_APP/_IMAGE e
#     GLINTFX_BACKEND_GLFW/GLINTFX_OWN_FONT_ENGINE continuam com os mesmos nomes e mesma
#     semantica (inclusive os FATAL_ERROR de dependencia entre GLINTFX_MODULE_APP/_WINDOW e o
#     de GLINTFX_MODULE_IMAGE/_TEXT=OFF, ainda reservadas/forcadas ON).
#   - Guard de libm em glintfx_audio continua por COMPILADOR
#     ($<$<NOT:$<CXX_COMPILER_ID:MSVC>>:m>), inalterado desde a v0.18.1.
#   - Pin do RmlUi deles continua no MESMO SHA 2cd28864ae25ed345b70598751703a5433b12356 (o
#     patch teardown-UB nosso segue aplicavel - o nosso Declare(RmlUi) vem primeiro e decide,
#     ver comentario grande no bloco RmlUi acima).
#   - Vendors deles (gamecontrollerdb/khronos/miniaudio/stb) conferidos via
#     `gh api repos/petrinhu/glintfx/contents/glintfx/third_party?ref=v0.26.0` - MESMOS 4 do
#     pin anterior, nenhum vendor novo. THIRD-PARTY-LICENSES.md atualizado so na data/pin.
# Build local + suite completa verdes no pin novo antes deste commit (contagem de testes no
# relatorio da fatia 1 de log-e-relogio).
#
# BUMP v0.26.0 -> v0.27.0 (BUMP-GLINTFX-V027, 2026-07-30): a v0.27.0 paga os 4 defeitos que o
# NOSSO review adversarial da S1 provocou do lado deles - DEC-NOTHROW (decode_image_file/
# decode_image_memory) e TEX-NOTHROW (Draw2d::load_texture/create_texture) agora sao `noexcept`
# com try/catch no corpo, degradando bad_alloc pra retorno invalido (`ok() == false`) em vez de
# lancar atraves da fronteira de API. Somam-se 2 funcoes NOVAS ja nascidas noexcept
# (encode_image_memory/encode_image_file, IMG-ENCODE) = as 6 noexcept desta release.
# Compatibilidade em nivel de FONTE verificada por ENUMERACAO, nao pela palavra deles: diff das
# declaracoes (comentarios removidos) dos 19 headers publicos das duas tags mostra que
# **NENHUMA** assinatura existente mudou alem dos 4 `noexcept` acima - zero remocao, zero troca
# de tipo/parametro. Nosso consumo e imune por dois motivos medidos: (a) ZERO teste da suite
# espera excecao vinda de funcao do glintfx (os 4 arquivos com glintfx E *_THROWS mencionam
# glintfx so em COMENTARIO; o unico catch() perto de glintfx, maestro.cpp:394, envolve
# load_dialogue_graph_from_file, funcao NOSSA de domain/); (b) ZERO lugar do codigo pega o
# ENDERECO dessas funcoes (`&glintfx::decode_image_*`/`decltype`) - `noexcept` faz parte do TIPO
# desde o C++17, entao ponteiro-de-funcao seria a unica forma de o acrescimo quebrar compilacao,
# e nao existe nenhum. Consumo real: ZERO ajuste de codigo, so o GIT_TAG.
#   - ⛔ NAO adotado nesta fatia (fora de escopo, decisao do lider): (1) `Draw2d::load_font`
#     CONTINUA sem `noexcept` na v0.27.0 (draw2d.hpp:948, conferido na tag) e nos o chamamos em
#     platform/src/render2d/render2d_glintfx.cpp:72 - PROIBIDO envolver em try/catch (a lei do
#     projeto proibe contornar lacuna do glintfx); o pedido FONT-NOTHROW ja foi sinalizado pelo
#     bus e a fatia espera o fix deles. (2) `UiLayer::capture_frame()`/`App::capture_frame()`
#     (API nova nesta tag) NAO e adotada aqui, mas por ESCOPO, nao por inadequacao: e readback
#     puro do FBO 0 na regiao do viewport, e SERVE pro nosso fundo congelado desde que chamada
#     ANTES do `render()` da UI. (A primeira redacao deste comentario dizia que ela "le a layer
#     offscreen do RmlUi, nao o FBO 0" - ERRADO, corrigido no commit seguinte: a premissa vinha
#     de ler o "depois de render()" do doc deles como se a ordem fosse REQUISITO, quando era
#     descricao de um caso de uso.) Migrar `capture_frame_to_png` pra ela e fatia propria.
#     (3) O par IMG-ENCODE novo torna o nosso vendor
#     `third_party/stb/stb_image_write.h` tecnicamente removivel (~20 arquivos de app/ o usam
#     pra screenshot) - onda propria, nao esta.
# Ritual de bump revalidado por leitura direta do CMakeLists.txt deles nas DUAS tags (baixados
# via `gh api repos/petrinhu/glintfx/contents/glintfx/CMakeLists.txt?ref=v0.2{6,7}.0` e
# diffados, em vez de conferidos de memoria):
#   - Conjunto de NOMES de flag GLINTFX_* IDENTICO entre v0.26.0 e v0.27.0 (diff do conjunto
#     ordenado de todos os tokens `GLINTFX_[A-Z0-9_]+` dos dois arquivos = vazio): nenhuma flag
#     nova, renomeada ou removida. As 16 `option()`/`set()` deles foram ENUMERADAS (nao grepadas
#     uma a uma): SANITIZE, COVERAGE, BACKEND_GLFW, OWN_FONT_ENGINE, MODULE_WINDOW, MODULE_APP,
#     MODULE_IMAGE, MODULE_TEXT, MODULE_I18N, MODULE_AUDIO, MODULE_GAMEPAD, MODULE_DRAW2D,
#     MODULE_FX, BUILD_LINK_SMOKE, BUILD_DEMOS, BUILD_TESTS - as 9 que setamos acima cobrem
#     tudo que nos concerne (MODULE_IMAGE segue reservada/forcada ON do lado deles;
#     BUILD_DEMOS/_TESTS defaultam pro `_glintfx_top`, FALSE quando consumidos por FetchContent).
#   - Guard de libm em glintfx_audio continua por COMPILADOR
#     ($<$<NOT:$<CXX_COMPILER_ID:MSVC>>:m>, CMakeLists deles linha 1364), inalterado desde a
#     v0.18.1 - linkar `m` no MSVC quebraria o CI Windows.
#   - Pin do RmlUi deles continua no MESMO SHA 2cd28864ae25ed345b70598751703a5433b12356 (linha
#     752 deles) e o patch `patches/rmlui-2cd28864-teardown-ub.patch` da tag v0.27.0 e IDENTICO
#     byte-a-byte ao nosso vendorizado em GusEngine/patches/. Aplicacao conferida na ARVORE
#     BAIXADA, nao no CMake declarativo: `git -C build/linux-release/_deps/rmlui-src apply
#     --reverse --check <patch>` passa (= ja aplicado) E o texto do hunk esta presente de fato
#     em Source/Core/Element.cpp (DestroyAndDeallocate(meta) ANTES de children.clear()).
#   - ⚠️ VENDOR NOVO A NIVEL DE ARQUIVO (o unico achado deste bump): os 4 DIRETORIOS de
#     third_party/ deles continuam os mesmos (gamecontrollerdb/khronos/miniaudio/stb), mas
#     `third_party/stb/` ganhou **stb_image_write.h** (71221 bytes) na v0.27.0, e ele e
#     COMPILADO de verdade (src/stb_image_write_impl.cpp entrou na OBJECT library
#     `glintfx_image`, que NAO tem gate desligavel) => esta no binario que distribuimos.
#     Mesma lib/autor/licenca do stb_image.h ja creditado, logo ZERO obrigacao nova de
#     atribuicao, mas o inventario precisava bater: THIRD-PARTY-LICENSES.md atualizado no mesmo
#     commit. NOTA DE METODO: a receita mecanica que o proprio THIRD-PARTY-LICENSES.md prescrevia
#     (`ls .../glintfx-src/glintfx/third_party/`) e um listing de DIRETORIO e por construcao NAO
#     ENXERGA arquivo novo dentro de vendor que ja existia - responderia "os mesmos 4, nada novo"
#     e passaria batido. A receita foi corrigida pra `find` recursivo naquele doc.
#   - GLINTFX_VERSION conferido no config.hpp GERADO (build/linux-release/_deps/glintfx-build/
#     include/glintfx/config.hpp), nao no CMakeLists declarativo: "0.27.0".
# Build local + gate ASan + suite completa (2596 testes, MESMA contagem do pin anterior - o bump
# nao soma nem remove teste nosso) verdes no pin novo antes deste commit.
#
# BUMP v0.27.0 -> v0.28.0 (BUMP-GLINTFX-V028, 2026-07-30): SO o GIT_TAG, ZERO ajuste de codigo
# de producao. ⚠️ Uma mudanca BREAKING no papel (UILAYER-CTOR-GUARD: o construtor do UiLayer
# agora VALIDA logical_width/logical_height e deixa ok()==false em vez de construir um contexto
# degenerado em silencio), mas ZERO impacto medido aqui: os 18 sitios de construcao de UiLayer
# do repo (6 producao + 3 teste + 9 ferramenta) foram ENUMERADOS, nao amostrados - TODOS usam
# 960/540 literal ou uma constante kW/kH do proprio arquivo, NENHUM le uma dimensao de janela
# ao vivo que pudesse chegar 0x0 (o cenario Wayland-minimizado que o proprio changelog deles
# aponta como o caso real). O achado de producao deles ("Element meta pool not empty on
# shutdown, 75 object(s) leaked", UILAYER-SINGLETON) e o MESMO incidente que ja gerou o guard de
# reentrancia deste projeto em screen_state.cpp (ActiveGuard, ver o comentario la: "o crash real
# de 2 glintfx::UiLayer vivos ao mesmo tempo ja derrubou o lider ao vivo uma vez") - o hazard nao
# e novo pra nos, ja estava mitigado em codigo antes desta tag chegar.
#   - Flags GLINTFX_MODULE_*/GLINTFX_BACKEND_GLFW/GLINTFX_OWN_FONT_ENGINE: conjunto de NOMES
#     IDENTICO entre v0.27.0 e v0.28.0 (diff do grep `GLINTFX_[A-Z0-9_]+` ordenado dos dois
#     CMakeLists.txt deles = vazio). Unica mudanca no CMakeLists deles e cosmetica de
#     organizacao: src/gl_proc.cpp e src/frame_capture.cpp saem do bloco gateado por
#     GLINTFX_MODULE_APP e viram fontes INCONDICIONAIS (GLPROC-EMBED: gl_proc_address() deixou
#     de ser so-modo-App), sem GATE novo nem flag nova pra nos setarmos.
#   - Pin do RmlUi deles continua no MESMO SHA 2cd28864ae25ed345b70598751703a5433b12356 (linha
#     751 do CMakeLists.txt deles na v0.28.0) e o patch
#     patches/rmlui-2cd28864-teardown-ub.patch da tag v0.28.0 e IDENTICO byte-a-byte ao nosso
#     vendorizado (`diff` limpo).
#   - Vendors deles (gamecontrollerdb/khronos/miniaudio/stb): arvore GIT recursiva das duas tags
#     (`gh api .../git/trees/v0.2{7,8}.0?recursive=true`, filtrada por `third_party`) comparada
#     arquivo-a-arquivo, nao so por diretorio (a lacuna de metodo que o proprio
#     THIRD-PARTY-LICENSES.md documentou na v0.27.0) - diff VAZIO, nenhum arquivo novo, nenhum
#     removido. THIRD-PARTY-LICENSES.md NAO precisou de toque nesta fatia.
#   - Seis funcoes ganham `noexcept` nesta release (FONT-NOTHROW/CAPTURE-NOTHROW/
#     GAMEPAD-NOTHROW fecham a familia "never a crash" em 6 membros: Draw2d::load_font,
#     Engine::capture_frame (por tras de UiLayer::capture_frame/App::capture_frame),
#     Gamepads::load_mappings_file) - checado que nosso codigo NUNCA pega o ENDERECO dessas
#     funcoes (`&glintfx::...`/`decltype`) nem espera excecao vinda delas (todo `catch` do
#     repo foi listado; nenhum esta perto de um call site glintfx) - `noexcept` so muda o TIPO
#     pra quem toma ponteiro-de-funcao, e nao existe nenhum caso aqui.
#   - `capture_framebuffer()` (funcao livre nova, CAPTURE-FREE) e `decode_png_file()`
#     (DEC-FORMAT-SURFACE) sao API nova ADITIVA, nao adotada nesta fatia (fora de escopo - o
#     nosso `ui_->capture_frame()` ja e metodo de instancia e ja funciona; migrar pra funcao
#     livre e decisao de fatia propria). `load_gl=false` (o gatilho do GLPROC-CRASH deles) NUNCA
#     e usado no nosso repo - os 18 sitios sao TODOS `load_gl=true` (grep integral, nao
#     amostrado).
#   - GLINTFX_VERSION conferido no config.hpp GERADO (build/linux-release/_deps/glintfx-build/
#     include/glintfx/config.hpp), nao no CMakeLists declarativo: "0.28.0".
# Build local + suite completa (2611 testes) + tools/check.sh (10/10) verdes no pin novo antes
# deste commit (contagem de testes no relatorio da fatia F1).
#
# BUMP v0.28.0 -> v0.29.0 (BUMP-GLINTFX-V029, 2026-07-31): release de LICENCA, nao de codigo. O
# glintfx rotacionou de MPL-2.0 pra Apache-2.0, e a v0.29.0 e a TAG DE CORTE: toda tag ate a
# v0.28.0 inclusive continua MPL-2.0 pra sempre; da v0.29.0 em diante e Apache-2.0. Eles
# declararam "nenhuma API nova, nenhum bugfix, unico delta funcional e version() devolver
# 0.29.0" - NAO aceito pela palavra deles, medido:
#   - Diff do CMakeLists.txt deles nas duas tags (baixado via `gh api
#     repos/petrinhu/glintfx/contents/glintfx/CMakeLists.txt?ref=v0.2{8,9}.0`): 3 hunks, e
#     SO 3: (1) SPDX-License-Identifier MPL-2.0 -> Apache-2.0 na linha 1; (2) project(... VERSION
#     0.28.0 -> 0.29.0); (3) um install(FILES LICENSE NOTICE ... DESTINATION
#     CMAKE_INSTALL_DOCDIR) NOVO, inteiro dentro de `if(CMAKE_PROJECT_NAME STREQUAL
#     PROJECT_NAME)` (linha 1724 deles) - condicao que so e verdadeira quando o glintfx e o
#     projeto TOP-LEVEL, nunca quando consumido via FetchContent/add_subdirectory como aqui.
#     Conjunto de flags GLINTFX_* (grep ordenado, diff) IDENTICO entre as duas tags.
#   - Este repositorio NUNCA chama `cmake --install` pro glintfx (grep confirmado em
#     GusEngine/CMakeLists.txt, */CMakeLists.txt e tools/*.sh: o unico `cmake --install` do
#     projeto e o do FreeType, tools/winbuild_container.sh:71, prefixo proprio) - o install()
#     novo e INERTE pra nos.
#   - Pin do RmlUi deles continua no MESMO SHA 2cd28864ae25ed345b70598751703a5433b12356 (linha
#     752 deles na v0.29.0), byte-a-byte igual ao bloco declarado acima neste arquivo.
#   - Vendors deles (arvore GIT recursiva das duas tags, `gh api .../git/trees/v0.2{8,9}.0
#     ?recursive=true`, filtrada por third_party, comparada arquivo-a-arquivo): os 4 DIRETORIOS
#     continuam os mesmos (gamecontrollerdb/khronos/miniaudio/stb), ZERO arquivo novo, ZERO
#     removido - so o SPDX interno de gamecontrollerdb/README.md e
#     gamecontrollerdb_linux.inc mudou MPL-2.0 -> Apache-2.0 (mesma rotacao, arquivo deles).
#     THIRD-PARTY-LICENSES.md atualizado com o resultado desta conferencia.
#   - Diff de assinatura NAO e suficiente sozinho (licao do bump anterior, onde a mudanca que
#     mais importava nao aparecia em diff nenhum) - por isso os 20 headers publicos de
#     glintfx/include/glintfx/ foram baixados das duas tags e diffados um a um: byte-a-byte
#     IDENTICOS exceto a linha 1 (SPDX), nos 20, sem excecao. CHANGELOG.md deles conferido: a
#     entrada [0.29.0] tem UMA linha, exatamente a rotacao de licenca (ADR-0019 deles, sucede o
#     ADR-0007 MPL-2.0).
#   - LICENSE na raiz do repo deles conferido na tag v0.29.0: "Apache License, Version 2.0"
#     (era "Mozilla Public License Version 2.0" na v0.28.0). NOTICE deles tambem migrado pro
#     cabecalho Apache-2.0 (aviso "AS IS", sem garantias).
#   - GLINTFX_VERSION conferido no config.hpp GERADO (build/linux-release/_deps/glintfx-build/
#     include/glintfx/config.hpp), nao no CMakeLists declarativo: "0.29.0".
# ZERO divergencia do que eles anunciaram: nenhuma API nova, nenhum bugfix, unico delta
# funcional e version()==0.29.0 - confirmado por medicao, nao por confianca. Build local +
# suite completa + tools/check.sh verdes no pin novo antes deste commit (numeros no relatorio
# da fatia BUMP-GLINTFX-V029).
#
# BUMP v0.29.0 -> v0.30.0 (BUMP-GLINTFX-V030, 2026-08-05): SO o GIT_TAG, ZERO ajuste de codigo
# de producao. ⚠️ A NOTA DE ENTRADA E UMA CORRECAO DE ESCOPO, e ela e o achado principal desta
# fatia: a release foi anunciada como "a onda RMLX-0 nao tocou a superficie publica (zero
# mudanca em glintfx/include/ e em *.rcss/*.rml)". Isso e VERDADE SOBRE A ONDA RMLX-0 e FALSO
# SOBRE A TAG v0.30.0 - a tag carrega MUITO mais que a RMLX-0. Medido por enumeracao dos 21
# headers publicos das duas tags, baixados um a um e diffados byte-a-byte (mesmo metodo do bump
# v0.29.0, que existe justamente porque "diff de assinatura nao basta"): 5 headers MUDARAM
# (app.hpp, clock.hpp, ui_layer.hpp com codigo; frame_capture.hpp e gl_proc.hpp so comentario,
# 0 linha nao-comentario) e 1 header NOVO entrou (scroll_types.hpp, 20 -> 21 arquivos). Aceitar
# a frase do anuncio pelo valor de face teria pulado a conferencia inteira. Nada disso quebrou
# nada aqui, mas o motivo e medicao, nao sorte - ver o paragrafo do load_gl abaixo.
#   - Flags GLINTFX_MODULE_*/GLINTFX_BACKEND_GLFW/GLINTFX_OWN_FONT_ENGINE: conjunto de NOMES
#     IDENTICO entre v0.29.0 e v0.30.0 (diff do grep `GLINTFX_[A-Z0-9_]+` ordenado dos dois
#     CMakeLists.txt deles = VAZIO, 19 nomes de cada lado). O CMakeLists deles tem 4 hunks nesta
#     release, e os 4 sao a mesma coisa: os fontes de UI migraram de `src/` pra `src/rml/`
#     (render_gl3/bootstrap/data_binder/font_engine_own/decorator_*/fx/effects_gl3), mais
#     `src/rml/input_map.cpp` NOVO (traducao glintfx::Key -> Rml::Input, extraida de engine.cpp
#     pra aquele arquivo deixar de incluir RmlUi). ZERO flag nova, ZERO gate novo, ZERO valor de
#     default mudado - nada a setar do nosso lado. Confirmado que TEXT/FX/AUDIO seguem ON,
#     GAMEPAD ON so em Linux, I18N/WINDOW/APP OFF, BACKEND_GLFW OFF (embed) e OWN_FONT_ENGINE
#     OFF (FreeType fica), exatamente como os `set(... FORCE)` acima ja fixam.
#   - Guard de plataforma do libm em glintfx_audio: continua por COMPILADOR, nao por plataforma
#     (`target_link_libraries(glintfx_audio PRIVATE ... $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:m>)`,
#     linha 1398 do CMakeLists deles na v0.30.0) - o shim de 3 linhas que a v0.18.0 exigiu por
#     causa do LNK1181 no MinGW continua desnecessario.
#   - Pin do RmlUi deles continua no MESMO SHA 2cd28864ae25ed345b70598751703a5433b12356 (linha
#     752 deles na v0.30.0), igual ao declarado na linha 100 deste arquivo; e o patch
#     patches/rmlui-2cd28864-teardown-ub.patch da tag v0.30.0 e IDENTICO byte-a-byte ao nosso
#     vendorizado em GusEngine/patches/ (md5 4a5f04a1cd8683ee01d48dd3b2da035a nos dois, `diff`
#     limpo) - o PATCH_COMMAND da NOSSA Declare(RmlUi) segue aplicando o mesmo conteudo.
#   - Vendors deles: arvore GIT recursiva das duas tags comparada por SHA DE BLOB + path
#     (`gh api .../git/trees/v0.{29,30}.0?recursive=true`), nao so por path - diff VAZIO nos 19
#     arquivos. Os 4 diretorios seguem os mesmos (gamecontrollerdb/khronos/miniaudio/stb), ZERO
#     arquivo novo, ZERO removido, ZERO byte alterado (nem o SPDX interno, que foi o unico delta
#     do bump anterior). NOTA DE METODO, na mesma linhagem da correcao `ls` -> `find` da v0.27.0
#     e da comparacao arquivo-a-arquivo da v0.28.0: comparar PATH nao prova CONTEUDO. A primeira
#     passada desta fatia comparou so os paths e teria assinado "identico" sem ter olhado um byte
#     de conteudo; a comparacao por SHA de blob e o minimo que responde a pergunta certa.
#     THIRD-PARTY-LICENSES.md NAO precisou de linha nova (nenhuma entrada nova pra classificar
#     entre "entra no binario" e "so build"), so da atualizacao do numero do pin.
#   - Licenca: v0.30.0 segue Apache-2.0 (LICENSE na raiz deles = "Apache License, Version 2.0";
#     NOTICE com o cabecalho Apache e o aviso "AS IS"). A v0.29.0 continua sendo a tag de corte
#     MPL-2.0 -> Apache-2.0; nada mudou de regime nesta.
#   - ⚠️ DEFEITO DELES, registrado e NAO contornado (nao afeta build, nao bloqueia a fatia): a
#     tag v0.30.0 NAO bumpou a versao do projeto. `project(glintfx ... VERSION 0.29.0)` esta
#     literal na linha 3 do CMakeLists.txt DELES na tag v0.30.0, e como o config.hpp.in injeta
#     `#define GLINTFX_VERSION "@PROJECT_VERSION@"`, o config.hpp GERADO aqui
#     (build/linux-release/_deps/glintfx-build/include/glintfx/config.hpp:164) diz "0.29.0"
#     rodando a v0.30.0 - ou seja, `glintfx::version()` MENTE nesta release. Conferido no header
#     GERADO, nao no declarativo (mesma disciplina dos 3 bumps anteriores), e e exatamente por
#     essa disciplina que apareceu. Na mesma linha, o CHANGELOG.md deles na tag NAO tem secao
#     `## [0.30.0]`: todo o conteudo desta release esta sob `## [Unreleased]`. Consequencia
#     pratica pra nos: NENHUMA (nao lemos version() em lugar nenhum do repo), mas quem for
#     depurar por GLINTFX_VERSION vai ler o numero errado. Reportado a eles pelo bus.
#   - API nova, ADITIVA, NAO consumida nesta fatia (bump e alinhamento de pin): `App::
#     is_scancode_down(int)`, `UiLayer/App::set_cursor_callback(std::function<void(const char*)>)`,
#     `UiLayer/App::scroll_element_into_view(const char*, ScrollAlign)` + o header novo
#     `<glintfx/scroll_types.hpp>` que traz o enum ScrollAlign, `UiLayer/App::ready() const
#     noexcept`, e `glintfx::sleep_ms(std::uint64_t)` em <glintfx/clock.hpp>. Nenhuma remocao,
#     nenhuma renomeacao de simbolo, nenhuma mudanca de assinatura existente.
#   - ⚠️ `UiLayerConfig::load_gl` virou `[[deprecated]]` nesta release (SEED-LOADGL-NOME),
#     substituido por `assume_gl_loaded` com POLARIDADE INVERTIDA (`load_gl=false` <=>
#     `assume_gl_loaded=true`), e eles anunciam remocao "nao antes da v0.31". Os campos novos
#     (`assume_gl_loaded`, `gl_proc_resolver`) entraram no FIM da struct, DEPOIS de `font_engine`
#     - a ordem dos 5 campos anteriores (logical_width, logical_height, load_gl, dp_ratio,
#     font_engine) esta PRESERVADA, o que importa porque os nossos ~18 sitios constroem o
#     UiLayerConfig por AGREGADO POSICIONAL (`{960, 540, /*load_gl=*/true, dp_ratio, ...}` - o
#     `/*load_gl=*/` ali e COMENTARIO, nao designated initializer). Medido que o `[[deprecated]]`
#     NAO dispara warning nenhum aqui (rebuild forcado dos 4 arquivos que constroem o config,
#     saida limpa de `deprecat|warning|error`): init posicional nao NOMEIA o membro, entao o
#     atributo nao e acionado. ⚠️ DIVIDA DATADA, que esta init posicional cria pra v0.31: no dia
#     em que `load_gl` for REMOVIDO da struct, todo sitio posicional desloca em silencio um campo
#     pra esquerda (o `true` do slot 3 cai em `dp_ratio`), e o comentario `/*load_gl=*/` vai
#     continuar dizendo a coisa errada em cada um deles sem o compilador ter como reclamar do
#     comentario. Migrar os sitios pra designated initializers ANTES do bump da v0.31 e o
#     conserto barato; deixar pra descobrir no bump e o caro. Nao feito aqui (fora do escopo
#     desta fatia, que e alinhamento de pin) - item proprio no TODO.
# Build local + suite completa (2905 testes, 2905 passando, MESMA contagem da baseline - o bump
# nao soma nem remove teste nosso, e nenhum dos 2 flakes conhecidos de display/audio caiu) +
# tools/check.sh (todos os gates verdes) no pin novo antes deste commit. Checkout do _deps
# CONFERIDO no disco, nao presumido do reconfigure: `git log -1` em _deps/glintfx-src devolve
# 38a301c7cbdd657efcdc631da0a38661bff59c6c (HEAD, tag: v0.30.0), e `glintfx/src/rml/` (a marca
# estrutural desta release) existe na arvore baixada.

FetchContent_Declare(
    glintfx
    # MIGRADO DO CODEBERG PRO GITHUB em 2026-07-25 (o Codeberg passou a nao aceitar
    # codigo escrito com auxilio de IA; o glintfx saiu de la e o repo antigo vira
    # somente-leitura, o que quebraria este FetchContent). Troca conferida por
    # evidencia, nao presumida: a tag v0.20.0 tem o MESMO sha nos dois hosts
    # (9bfaabfa, apontando pro commit 0442dd15 que ja esta no nosso _deps).
    GIT_REPOSITORY https://github.com/petrinhu/glintfx
    GIT_TAG        v0.30.0
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
