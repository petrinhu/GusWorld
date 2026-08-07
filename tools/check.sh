#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# check.sh - build + smoke + gate + suite do GusEngine, num comando.
#
# Espelha o "=== build + smoke + gate + suite ===" do PokemonTCGViewer,
# adaptado a stack do GusWorld (C++20 + CMake + Ninja + Catch2 + SDL3).
# Roda a cada mudanca de codigo (via hook PostToolUse) e tambem a mao.
#
# POS REPIVOT ADR-008: a camada de plataforma e SDL3 (nao mais Qt). O smoke roda o
# app em modo headless com os drivers DUMMY do SDL (sem display/audio), e o gate de
# arquitetura audita que core/ e domain/ nao incluem Qt NEM SDL.
#
# ESTAGIOS (cada um imprime seu exit; o script sai != 0 se QUALQUER um falhar):
#   BUILD : cmake --build (incremental; no-op ~0.02s, rebuild real ~1.5s)
#   SMOKE : roda o gusworld_app --smoke (N ticks + 1 render headless, sai 0)
#   GATE  : (a) arquitetura - core/ e domain/ NAO incluem Qt NEM SDL
#           (b) paridade i18n - tabela por locale (tools/i18n_parity.py)
#           (j) SPDX obrigatorio - tools/spdx_header_required.py
#   SUITE : ctest (resumo "100% tests passed ... out of N")
#
# Idempotente. Reaproveita o build dir existente (so reconfigura no 1o uso).
# So Linux por enquanto (preset linux-release; Windows e fase posterior).
#
# ARGUMENTOS:
#   --gates-only  roda SO o estagio GATE (pula BUILD/SMOKE/SUITE) e devolve o rc
#                 dele. Existe para o CI (.github/workflows/ci.yml) invocar ESTE
#                 arquivo em vez de reimplementar gate por gate no YAML - ver
#                 CI-RODA-OS-GATES (2026-08-06). O CI ja tem step proprio de
#                 Build, Test e Smoke; o que faltava era o GATE, e ele custa
#                 ~2,7s medidos (14 scripts Python ~1,8s + pytest ~0,8s + dois
#                 greps), contra minutos do build - por isso vale rodar o
#                 estagio INTEIRO em vez de escolher um subconjunto.
#                 ATENCAO: o gate (h) (ctest_timeout_required) mede por
#                 INTROSPECAO REAL do ctest, entao --gates-only ainda exige um
#                 build dir CONFIGURADO (no CI roda depois do step de Build). Sem
#                 ele, aquele gate REPROVA - de proposito, nunca passa em
#                 silencio.
#
# Variaveis de ambiente uteis:
#   CHECK_QUIET=1 silencia o log de build/ctest (so mostra os marcadores e a
#                 tabela); usado pelo hook para nao poluir o transcript.
#   CHECK_MIN_I18N=<n>  alem da estrutura, exige cobertura de conteudo >= n%.

set -euo pipefail

# Raiz do repo = pai de tools/ (resolve symlink do proprio script).
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
ENGINE="$ROOT/GusEngine"
PRESET="linux-release"
APP_BIN="$ENGINE/build/$PRESET/app/gusworld_app"

QUIET="${CHECK_QUIET:-0}"
MIN_I18N="${CHECK_MIN_I18N:-}"

# ---------------------------------------------------------------- ARGUMENTOS
# Um unico modo alternativo: --gates-only (ver cabecalho). Argumento desconhecido
# ABORTA com rc=2 em vez de ser ignorado - um CI que invoque `check.sh --gate`
# (nome errado) tem de falhar barulhento, nunca rodar a coisa toda em silencio
# nem, pior, rodar nada e sair 0.
GATES_ONLY=0
for _arg in "$@"; do
    case "$_arg" in
        --gates-only) GATES_ONLY=1 ;;
        -h|--help)
            echo "uso: check.sh [--gates-only]"
            echo "  (sem argumento) BUILD + SMOKE + GATE + SUITE"
            echo "  --gates-only    so o estagio GATE (exige build dir configurado)"
            exit 0 ;;
        *)
            echo "check.sh: argumento desconhecido '$_arg' (use --help)" >&2
            exit 2 ;;
    esac
done

# Redireciona stdout de comandos verbosos quando QUIET=1.
run_log() {
    if [ "$QUIET" = "1" ]; then "$@" >/dev/null; else "$@"; fi
}

if [ "$GATES_ONLY" = "1" ]; then
    echo "=== gate (--gates-only: BUILD, SMOKE e SUITE NAO executados aqui) ==="
else
    echo "=== build + smoke + gate + suite ==="
fi

# ---------------------------------------------------------------- BUILD
# Reconfigura so se o build dir ainda nao existe (1a vez). Depois e incremental.
BUILD=0
if [ "$GATES_ONLY" = "1" ]; then
    echo "BUILD=pulado (--gates-only)"
else
    if [ ! -f "$ENGINE/build/$PRESET/build.ninja" ] \
       && [ ! -f "$ENGINE/build/$PRESET/Makefile" ]; then
        run_log cmake --preset "$PRESET" -S "$ENGINE" || true
    fi
    set +e
    ( cd "$ENGINE" && run_log cmake --build --preset "$PRESET" )
    BUILD=$?
    set -e
    echo "BUILD=$BUILD"
    if [ "$BUILD" != "0" ]; then
        # build quebrado: smoke/suite nao fazem sentido; gate de arquitetura ainda
        # roda (e estatico) para ja apontar se a causa foi Qt vazando em core/domain.
        echo "(build falhou; pulando SMOKE e SUITE)"
    fi
fi

# ---------------------------------------------------------------- SMOKE
# O app ABRE JANELA no modo normal (entraria no loop e travaria o check sem
# display). Por isso o smoke roda o modo HEADLESS do app:
#   --smoke  => inicializa tudo, roda N ticks do loop logico + 1 render OFFSCREEN
#              (Render2dSdl em modo headless, sem GPU/display), imprime resumo e
#              sai 0.
# SDL_VIDEODRIVER=dummy / SDL_AUDIODRIVER=dummy => o SDL nao tenta abrir display
# nem audio (vale no CI sem X/Wayland). timeout como cinto de seguranca: se algum
# dia o smoke travar, falha rapido em vez de pendurar o hook/CI.
SMOKE=0
if [ "$GATES_ONLY" = "1" ]; then
    echo "SMOKE=pulado (--gates-only)"
elif [ "$BUILD" = "0" ]; then
    if [ -x "$APP_BIN" ]; then
        set +e
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
            run_log timeout 60 "$APP_BIN" --smoke
        SMOKE=$?
        set -e
    else
        echo "(smoke: $APP_BIN inexistente)"
        SMOKE=1
    fi
    echo "SMOKE=$SMOKE"
fi

# ---------------------------------------------------------------- GATE
# (a) Arquitetura: core/ e domain/ sao POCO; nenhum #include <Q...>, <SDL...>, <RmlUi...>
#     NEM <glintfx...> (ADR-008 + ADR-009 + ADR-010 F3: SDL/RmlUi/glintfx vivem SO em
#     platform/ + app/). O match tambem pega os namespaces Rml:: e glintfx:: vazando pra
#     core/domain. Mesma logica do .github/workflows/ci.yml (sincronia de proposito).
GATE_ARCH=0
if grep -rnE '#[[:space:]]*include[[:space:]]*<(Q[A-Za-z]|SDL|RmlUi|glintfx)' \
        "$ENGINE/core" "$ENGINE/domain" 2>/dev/null; then
    echo "GATE(arch): include de Qt, SDL, RmlUi ou glintfx encontrado em core/ ou domain/ (POCO)."
    GATE_ARCH=1
elif grep -rnE '\b(Rml|glintfx)::' "$ENGINE/core" "$ENGINE/domain" 2>/dev/null; then
    echo "GATE(arch): uso do namespace Rml:: ou glintfx:: encontrado em core/ ou domain/ (POCO)."
    GATE_ARCH=1
else
    [ "$QUIET" = "1" ] || \
        echo "GATE(arch): OK (sem Qt, SDL, RmlUi nem glintfx em core/ ou domain/)."
fi

# (a2) EXCLUSIVIDADE DO GLINTFX (decisao do lider, 2026-07-29): o RmlUi e servido
#      EXCLUSIVAMENTE pelo glintfx. Nenhuma camada nossa fala com a API do RmlUi
#      direto - nem app/, nem platform/. Se falta alguma coisa, o caminho e PEDIR ao
#      glintfx (regua de fronteira, docs/tech/glintfx-boundary.md), nao contornar por
#      dentro. Medido em 2026-07-29: o codigo ja estava 100% em conformidade (zero uso
#      de Rml:: fora de comentario, 36 arquivos consumindo glintfx), entao este gate
#      CONGELA um estado que ja existia por disciplina, em vez de pedir migracao.
#      ATENCAO ao falso positivo: RmlUi_Include_GL3.h e o header do GLAD (nome
#      historico enganoso, ver THIRD-PARTY-LICENSES.md), NAO e API do RmlUi - por isso
#      o padrao abaixo casa <RmlUi/...> e Rml::, e nao a string "RmlUi" solta.
GATE_EXCL=0
if grep -rnE '#[[:space:]]*include[[:space:]]*[<"]RmlUi/' \
        "$ENGINE/platform" "$ENGINE/app" 2>/dev/null; then
    echo "GATE(excl): include da API do RmlUi encontrado fora do glintfx."
    echo "            O RmlUi e servido EXCLUSIVAMENTE pelo glintfx (decisao do lider 2026-07-29)."
    echo "            Falta algo? PECA ao glintfx (docs/tech/glintfx-boundary.md), nao contorne."
    GATE_EXCL=1
elif grep -rnE --include='*.cpp' --include='*.hpp' --include='*.h' '\bRml::' \
        "$ENGINE/platform" "$ENGINE/app" 2>/dev/null \
        | grep -vE '^[^:]+:[0-9]+:[[:space:]]*(//|\*|/\*|#)' | grep -q .; then
    echo "GATE(excl): uso do namespace Rml:: fora de comentario em platform/ ou app/."
    echo "            O RmlUi e servido EXCLUSIVAMENTE pelo glintfx (decisao do lider 2026-07-29)."
    GATE_EXCL=1
else
    [ "$QUIET" = "1" ] || \
        echo "GATE(excl): OK (RmlUi so via glintfx; nenhuma camada fala com a API dele direto)."
fi

# (b) Paridade i18n: tabela por locale (faltando/extra/dup reprovam; % so exibe).
set +e
if [ -n "$MIN_I18N" ]; then
    python3 "$ROOT/tools/i18n_parity.py" --min "$MIN_I18N"
else
    python3 "$ROOT/tools/i18n_parity.py"
fi
GATE_I18N=$?
set -e

# (c) Ratchet de nao-regressao SDL em app/ (docs/tech/plano-camadas-sdl.md):
#     platform/ e a UNICA fronteira que deveria tocar SDL3 (CLAUDE.md), mas
#     app/ ainda tem 32 arquivos de producao (src/+include/gus/app/, fora
#     tests/tools/) que tocam SDL3 direto - migracao em andamento por fatias.
#     O teto (script tools/sdl_layer_ratchet.py) so pode CAIR: falha se o
#     numero de arquivos infratores SUBIR. bash+grep cru conta comentario
#     como uso real (a mesma armadilha que deixou o GATE(arch) original sem
#     cobrir app/ - ver plano, secao 1), por isso o parser e Python.
set +e
python3 "$ROOT/tools/sdl_layer_ratchet.py"
GATE_SDL_RATCHET=$?
set -e

# (d) Zero-tolerancia de categoria FECHADA (fatia 1 log-e-relogio, 2026-07-30,
#     docs/tech/plano-camadas-sdl.md secao 8): diferente do ratchet acima (que
#     ainda acomoda transicao categoria-a-categoria), SDL_Log e SDL_GetTicksNS
#     ja saíram por completo de app/ de producao (glintfx::log*/FW-LOG e
#     glintfx::monotonic_now_ns/FW-CLOCK) - qualquer reintroducao reprova na
#     hora, sem teto numerico. SDL_Delay fica de FORA (ver
#     tools/sdl_log_clock_zero.py): continua em uso real, categoria ainda nao
#     migrada.
set +e
python3 "$ROOT/tools/sdl_log_clock_zero.py"
GATE_LOG_CLOCK_ZERO=$?
set -e

# (e) Zero-tolerancia CROSS-CAMADA (Fatia S3, docs/tech/plano-migracao-total.md
#     secao 5): stbi_load/stbi_image_free (decode) saíram de vez da producao
#     inteira (core+domain+platform+app) na Fatia S1 (commit 9387155); este
#     gate CONGELA o estado. stbi_write fica de fora de proposito (bloqueado
#     pela IMG-ENCODE do glintfx) - ver o docstring de
#     tools/stb_image_zero.py pro motivo, mesmo raciocinio do SDL_Delay acima.
set +e
python3 "$ROOT/tools/stb_image_zero.py"
GATE_STBI_ZERO=$?
set -e

# (f) Zero-tolerancia CROSS-CAMADA (Fatia S3): ma_* (miniaudio direto) e
#     <miniaudio...> ja saíram da producao inteira desde 2026-07-22 (commit
#     d79d880, audio_engine.cpp -> glintfx::Audio); este gate TRAVA a
#     migracao (o miniaudio ja mudou de dono uma vez - saiu do nosso
#     third_party/, voltou por dentro do glintfx - o gate garante que nao
#     volta pelo outro lado). Ver tools/audio_ma_zero.py.
set +e
python3 "$ROOT/tools/audio_ma_zero.py"
GATE_AUDIO_ZERO=$?
set -e

# (g) Lista fechada de FetchContent (Fatia S3): GusEngine/CMakeLists.txt
#     declara EXATAMENTE {SDL3, RmlUi, glintfx, Catch2} - dependencia nova
#     (transitiva ou direta) so entra por decisao EXPLICITA (o proprio
#     miniaudio ja entrou "de gratis" por dentro do glintfx uma vez; este
#     gate obriga a proxima a virar uma linha em ALLOWED, nao um acidente).
#     Ver tools/fetchcontent_manifest.py.
set +e
python3 "$ROOT/tools/fetchcontent_manifest.py"
GATE_FETCHCONTENT=$?
set -e

# (h) Zero-tolerancia PERMANENTE ao proprio buraco do timeout (GATES-S3,
#     2026-07-30, pedido do orquestrador apos o hang de 8h54min): TODO teste
#     registrado no ctest precisa ter a propriedade TIMEOUT, medido por
#     INTROSPECAO REAL (`ctest --show-only=json-v1`), NUNCA por grep no
#     CMakeLists.txt - um grep so prova que a SINTAXE existe em algum lugar,
#     nao que aquele teste especifico recebeu a propriedade (foi exatamente
#     essa lacuna que deixou 3 dos 4 alvos sem TIMEOUT por 6 dias sem
#     ninguem perceber). Fecha o buraco pra SEMPRE: no dia em que nascer um
#     5o alvo de teste sem TIMEOUT, reprova na hora. Ver
#     tools/ctest_timeout_required.py (inclusive o requisito de NUNCA passar
#     em silencio se a introspecao falhar por qualquer motivo).
set +e
python3 "$ROOT/tools/ctest_timeout_required.py"
GATE_CTEST_TIMEOUT=$?
set -e

# (i) Zero-tolerancia PERMANENTE (FRAMEGRAB-7-SITIOS, 2026-07-30): os 7 sitios
#     de captura de backbuffer via gus::platform::rmlui::gl3_read_backbuffer_rgba
#     (leitura crua GL/glad) migraram pra glintfx::UiLayer::capture_frame()
#     (battle_preview.cpp x4, difficulty_menu_loop.cpp, title_menu_loop.cpp,
#     save_load_menu_loop.cpp). Este gate CONGELA o resultado: qualquer
#     chamada NOVA em producao (app/src + app/include/gus/app, mesmo escopo
#     dos gates acima) reprova na hora, exceto o unico site allowlisted e
#     documentado (sdl_window.cpp - roda antes de qualquer UiLayer existir).
#     Ver tools/gl3_readbackbuffer_zero.py (inclusive a auto-auditoria que
#     reprova se a allowlist ficar desatualizada).
set +e
python3 "$ROOT/tools/gl3_readbackbuffer_zero.py"
GATE_GL3_READBACKBUFFER=$?
set -e

# (j) SPDX obrigatorio (GATE-SPDX, 2026-08-01): todo .cpp/.hpp/.h/.c
#     RASTREADO (git ls-files - nunca find/os.walk, ver docstring do script)
#     precisa do cabecalho "SPDX-License-Identifier: Apache-2.0" perto do
#     topo, exceto o que estiver na allowlist versionada (terceiro com
#     licenca propria - tools/spdx_allowlist.txt). Fecha a lacuna que a
#     onda LICENSE-APACHE deixou: nada impedia o proximo arquivo de nascer
#     sem cabecalho, ou o antigo GPL de voltar. Ver tools/spdx_header_required.py
#     (inclusive a auto-auditoria que reprova se a allowlist ficar
#     desatualizada).
set +e
python3 "$ROOT/tools/spdx_header_required.py"
GATE_SPDX=$?
set -e

# (k) Ordem de destruicao dos membros (CALLBACK-DTOR-ORDER, 2026-08-05): nas 4
#     telas que registram callback no glintfx::UiLayer, `ui_` tem de ser
#     declarado DEPOIS de tudo que a lambda alcanca (logo destruido ANTES).
#     Existe como GATE, e nao so como comentario, porque desfazer a ordem NAO
#     deixa nenhum teste vermelho: o cenario exige uma excecao escapando de
#     enter()/tick()/handle_event() (o exit() esta solto, fora de guard RAII,
#     em screen_state.cpp), que o codigo de hoje nao lanca. Sem gate, a proxima
#     reorganizacao de membros reabre o use-after-free em silencio - foi
#     exatamente essa a forma do bug que o glintfx levou ao vivo (achado so
#     pelo ASan). Ver tools/callback_dtor_order.py (inclusive a auto-auditoria
#     que reprova se a tabela de membros ficar desatualizada por rename).
set +e
python3 "$ROOT/tools/callback_dtor_order.py"
GATE_CALLBACK_DTOR=$?
set -e

# (l) Desarme do callback no destrutor (CALLBACK-DTOR-DESARME, 2026-08-05): a
#     SEGUNDA linha de defesa, que se SOMA a (k) sem substitui-la. Cada tela que
#     registra callback no UiLayer tem de desarma-lo (`set_*_callback(nullptr)`)
#     como PRIMEIRA instrucao do proprio destrutor - foi o conserto que o glintfx
#     adotou no bug real deles, e o contrato publico da v0.30.0 diz explicitamente
#     que "nenhum consumidor e chamado de volta depois de pedir para ser
#     desligado". Existe como gate pelo MESMO motivo de (k) - apagar o desarme nao
#     deixa teste vermelho -, e cobre o furo MEDIDO de (k): aquele gate nao pega um
#     membro NOVO declarado depois de `ui_`, e o desarme e insensivel a ordem de
#     membro. Este gate DESCOBRE os registros (git ls-files, nao lista escrita a
#     mao) e reprova tela nova sem desarme, tabela obsoleta, e familia de callback
#     nao classificada. Ver tools/callback_dtor_disarm.py.
set +e
python3 "$ROOT/tools/callback_dtor_disarm.py"
GATE_CALLBACK_DISARM=$?
set -e

# (m) Zero-tolerancia CROSS-CAMADA (PNG-DECODE-ADOPT, 2026-08-06): `decode_image_file`
#     do glintfx faz dispatch PNG/JPG/TGA pelo sniffing do stb_image, e a perna TGA e
#     FROUXA - medido pela auditoria da W1 e reconferido: 82 bytes de header TGA
#     forjado com nome ".png" decodificam "COM SUCESSO" (ok == true, nao falha!) numa
#     RGBA8 4096x4096 toda alpha-zero: 64 MB alocados a partir de 82 bytes de arquivo,
#     amplificacao de ~818.000x, superficie de ataque por asset malicioso. Os 3 call
#     sites (app_icon.cpp + render2d_{gl3,sdl}.cpp) migraram pra `decode_png_file`
#     (glintfx v0.30.0+, ja no pin que usamos), que checa a assinatura PNG antes do
#     stb_image e RECUSA o forjado - drop-in, mesmo contrato. Este gate CONGELA o
#     estado, allowlist VAZIA. Adotar API que a lib ja expoe pra este fim NAO e
#     contornar lacuna (o oposto do que a lei de 2026-07-29 proibe). `decode_image_
#     memory` fica de FORA de proposito (sem irma so-PNG no glintfx = sem substituto
#     pra oferecer; zero uso em producao hoje) - mesmo criterio que deixou stbi_write
#     fora do (e) e SDL_Delay fora do (d). Ver tools/decode_image_file_zero.py
#     (inclusive a sentinela de escopo que reprova se a varredura colapsar pra zero
#     arquivo, em vez de imprimir um OK falso).
set +e
python3 "$ROOT/tools/decode_image_file_zero.py"
GATE_DECODE_IMAGE_ZERO=$?
set -e

# (n) Auto-auditoria do ESCOPO de producao (GATES-HARDEN, 2026-08-06): a lista de
#     diretorios que os gates (e), (f) e (m) varrem e ESCRITA A MAO
#     (tools/production_scope.py) - uma 5a camada, ou um arquivo de codigo solto na
#     raiz de uma camada, nasceria FORA dela e os tres diriam "OK" sem nunca ter
#     olhado pra ela ("um verificador com ponto cego confirma o que se espera
#     dele"). Nao e hipotese: `GusEngine/app/main.cpp` - a UNICA fonte do
#     executavel - estava exatamente nessa situacao, e carregava um
#     `#include <SDL3/SDL.h>` orfao que nenhum gate de camada via. Este gate
#     ENUMERA o disco (GusEngine/*/src, */include e a raiz de cada camada) e
#     reprova se algo estiver fora da lista - mesmo desenho do (j), que enumera
#     por `git ls-files` em vez de procurar onde ja se sabe.
set +e
python3 "$ROOT/tools/production_scope.py"
GATE_PRODUCTION_SCOPE=$?
set -e

# (o) OS PROPRIOS GATES SOB TESTE (GATES-HARDEN, 2026-08-06): `tools/tests/`
#     existe desde a fatia GATE-SPDX e NAO ERA EXECUTADO POR NINGUEM - nem aqui,
#     nem no .github/workflows/ci.yml. E o mesmo defeito que os gates existem pra
#     impedir, aplicado a eles mesmos: "gate correto e DESLIGADO e gate que nao
#     existe". Cada furo fechado nesta fatia tem um teste que prova o
#     VERMELHO-ANTES (a arvore antiga passava) e o VERDE-DEPOIS, mais o caso de
#     CRESCIMENTO LEGITIMO (arquivo novo em src/, camada nova sem dependencia
#     nova) - se o gate so soubesse reprovar, ele barraria trabalho honesto. Sem
#     esta linha, nada disso rodava.
#     Sem pytest instalado o estagio REPROVA (nao pula em silencio): "OK" tem de
#     significar "rodei os testes", nunca "nao consegui rodar e deixei passar".
set +e
if python3 -c "import pytest" 2>/dev/null; then
    python3 -m pytest "$ROOT/tools/tests" -q --no-header
    GATE_TOOLS_TESTS=$?
else
    echo "GATE(tools-tests): FALHA - pytest nao encontrado; os testes dos gates"
    echo "  (tools/tests/) NAO rodaram. Instale com 'python3 -m pip install pytest'."
    echo "  Nao pulamos em silencio: um gate que nao roda nao protege nada."
    GATE_TOOLS_TESTS=1
fi
set -e

# (n) Zero-tolerancia ESTREITA no canal ui_log_ (I18N-UILOG, 2026-08-06): texto de
#     TELA nao se escreve dentro de `ui_log_.push_back` - a fonte e o catalogo
#     (resources/translations/*.md), via BattleScene::push_ui_log(kind, "CHAVE").
#     Superficie MEDIDA e FECHADA: 3 call sites, todos em battle_scene.cpp, e o
#     gate NASCE VERDE no mesmo commit que os fia pelo translator. Sao 3 regras
#     (literal proibido; sentinela de escopo que reprova se a varredura colapsar
#     pra zero site; e chave literal obrigatoriamente existente nos DOIS
#     catalogos - a regra que fecha o furo de escrever o texto solto por dentro
#     do helper). NAO e um detector de "literal acentuado": medimos que os 4
#     ofensores reais eram TODOS sem acento, entao essa heuristica erraria 4 de
#     4. O que este gate nao cobre (literal em builder de RML) fica com os testes
#     de origem-no-catalogo - ver o docstring de tools/ui_log_key_only.py.
set +e
python3 "$ROOT/tools/ui_log_key_only.py"
GATE_UI_LOG_KEY=$?
set -e

# (p) GATE(tools-isolation) (FATIA1-LOG-CLOCK, 2026-08-07): ate esta fatia,
#     "GATE(tools-isolation)" era citado como existente em TRES lugares (TODO.md,
#     o docstring do sdl_layer_ratchet.py e docs/tech/plano-camadas-sdl.md) e nao
#     existia em NENHUM ponto executavel - a fronteira app/tools/ so era
#     protegida ESTRUTURALMENTE (CMakeLists.txt standalone), sem gate ativo. Este
#     gate torna a citacao verdadeira: reprova se GusEngine/CMakeLists.txt ou
#     GusEngine/app/CMakeLists.txt ganharem `add_subdirectory(tools)` de verdade
#     (fora de comentario) - o dia em que isso acontecer invalida de uma vez tres
#     exececoes documentadas em outros gates (sdl_layer_ratchet.py,
#     sdl_log_clock_zero.py, fetchcontent_manifest.py). Ver tools/tools_isolation_zero.py.
set +e
python3 "$ROOT/tools/tools_isolation_zero.py"
GATE_TOOLS_ISOLATION=$?
set -e

GATE=0
[ "$GATE_ARCH" = "0" ] && [ "$GATE_EXCL" = "0" ] && [ "$GATE_I18N" = "0" ] \
    && [ "$GATE_SDL_RATCHET" = "0" ] && [ "$GATE_LOG_CLOCK_ZERO" = "0" ] \
    && [ "$GATE_STBI_ZERO" = "0" ] && [ "$GATE_AUDIO_ZERO" = "0" ] \
    && [ "$GATE_FETCHCONTENT" = "0" ] && [ "$GATE_CTEST_TIMEOUT" = "0" ] \
    && [ "$GATE_GL3_READBACKBUFFER" = "0" ] && [ "$GATE_SPDX" = "0" ] \
    && [ "$GATE_CALLBACK_DTOR" = "0" ] && [ "$GATE_CALLBACK_DISARM" = "0" ] \
    && [ "$GATE_DECODE_IMAGE_ZERO" = "0" ] \
    && [ "$GATE_PRODUCTION_SCOPE" = "0" ] \
    && [ "$GATE_TOOLS_TESTS" = "0" ] \
    && [ "$GATE_UI_LOG_KEY" = "0" ] \
    && [ "$GATE_TOOLS_ISOLATION" = "0" ] \
    || GATE=1
echo "GATE=$GATE"

# ---------------------------------------------------------------- SUITE
# Log completo do ctest vai pra um caminho FIXO dentro do build dir (sobrescrito a
# cada rodada, nao precisa de remocao): antes ia pra /tmp/.gusworld_ctest.$$ e era
# descartado via `gio trash` a cada execucao do hook PostToolUse - centenas de
# arquivos por sessao de trabalho poluindo o Lixo do usuario (AC-E6).
CTEST_LOG="$ENGINE/build/$PRESET/last_ctest.log"
SUITE=0
if [ "$GATES_ONLY" = "1" ]; then
    echo "SUITE=pulado (--gates-only)"
elif [ "$BUILD" = "0" ]; then
    set +e
    # XDG_RUNTIME_DIR ISOLADO (2026-07-24, achado do glintfx via bus - o furo que sobrou
    # no 1o conserto): forcar SDL_VIDEODRIVER=x11 e unsetar WAYLAND_DISPLAY **NAO ISOLA**.
    # `wl_display_connect(NULL)` - que QUALQUER dependencia do processo pode chamar, nao so
    # o nosso codigo - cai no nome EMBUTIDO "wayland-0" e resolve dentro do
    # $XDG_RUNTIME_DIR; unsetar a variavel de backend so tira o OVERRIDE, o fallback pelo
    # nome fixo continua achando o socket da sessao VIVA do usuario (eles levaram a mordida:
    # janela na sessao real por 2-3 min com o display aninhado ja provado). O conserto que
    # fecha e apontar o XDG_RUNTIME_DIR pra um dir PROPRIO e VAZIO (sem socket wayland-0 pra
    # o fallback encontrar), so no processo sob teste. Fica dentro do build dir (descartavel,
    # nunca contem socket) em vez de mktemp, o que evita de quebra a armadilha de limpeza que
    # eles relataram (trap ... RETURN nao dispara quando `set -e` aborta pela falha do
    # proprio comando - justamente o caminho de teste FALHANDO).
    _suite_xdg="$ENGINE/build/$PRESET/.suite_xdg_runtime"
    mkdir -p "$_suite_xdg" && chmod 700 "$_suite_xdg"

    # DISPLAY ISOLADO (2026-07-24, F4-GL-TESTS-SILENT-DEGRADE / achado do QA F4-1b.4):
    # alguns testes de interacao (app/tests/*_interaction_test) abrem contexto GL REAL
    # via glintfx::UiLayer. Sem isolar, o ctest herda o DISPLAY=:0 / WAYLAND_DISPLAY da
    # sessao VIVA do usuario e os testes GL rodam NELA (janelas HIDDEN, mas mesma
    # superficie que feedback_nunca_stress_janela_sessao_viva proibe em absoluto).
    # Solucao: subir um Xvfb dedicado num display LIVRE e forcar o SDL a usar X11 nele
    # (env -u WAYLAND_DISPLAY tira o wayland-0 do caminho; SDL_VIDEODRIVER=x11 impede
    # o SDL de escolher wayland). Sem Xvfb no host -> SDL_VIDEODRIVER=offscreen (os
    # testes GL degradam pra 0 assercoes, mas NAO tocam a sessao do usuario). SMOKE ja
    # usa dummy; aqui o GL precisa rodar de verdade, por isso Xvfb (framebuffer real),
    # nao dummy. GUSWORLD_SUITE_DISPLAY=:N sobrepoe (ex.: CI que ja tem Xvfb proprio).
    _suite_xvfb_pid=""
    _suite_disp="${GUSWORLD_SUITE_DISPLAY:-}"
    if [ -z "$_suite_disp" ] && command -v Xvfb >/dev/null 2>&1; then
        for _n in 99 100 101 102 103 104 105 106 107 108; do
            [ -e "/tmp/.X11-unix/X$_n" ] || { _suite_disp=":$_n"; break; }
        done
        if [ -n "$_suite_disp" ]; then
            Xvfb "$_suite_disp" -screen 0 1024x768x24 >/dev/null 2>&1 &
            _suite_xvfb_pid=$!
            sleep 1
        fi
    fi
    # TIMEOUT POR TESTE (2026-07-30, achado da noite de S3): SEM isto, um teste
    # pendurado NUNCA falha - ele so PARA de responder, e o ctest espera pra
    # sempre. Aconteceu de verdade: "AudioEngine set_music_volume/set_sfx_volume
    # com device real" (GusEngine/platform/tests/, gusengine_platform_tests) -
    # NA HORA do hang, so gusengine_app_tests tinha TIMEOUT property
    # (F4-CTEST-NO-TIMEOUT/GusEngine/app/tests/CMakeLists.txt:184); platform/tests,
    # domain/tests (GusEngine/domain/tests/CMakeLists.txt) e tests/ (core,
    # GusEngine/tests/CMakeLists.txt) NAO TINHAM NENHUM, e foi corrigido no MESMO
    # commit desta fatia (GATES-S3) - agora os 4 alvos tem PROPERTIES TIMEOUT
    # proprio, medido por alvo (30/20/20/60, ver cada CMakeLists.txt). O processo
    # pendurou com a thread principal em `pthread_join`, esperando uma thread do
    # PipeWire presa em `epoll_wait`. Custo medido: 8h54min de maquina perdidos,
    # dois agentes bloqueados sem saber por que (sem erro, sem log, sem fim). Nao
    # reproduzi depois - isolado passa em 0,03s, a suite inteira passa em ~22s: e
    # intermitente (ja reportado ao glintfx pelo bus).
    #
    # --timeout 120 AQUI e a SEGUNDA camada (defesa em profundidade), nao a
    # unica: cobre quem invoca `ctest --preset` direto na linha de comando (README,
    # outros agentes) SEM passar pelas PROPERTIES por-alvo acima - mas as
    # PROPERTIES ja bastam pra qualquer invocacao de ctest (viajam COM o teste,
    # gravadas no CTestTestfile.cmake). 120s e generoso o bastante (bem acima de
    # qualquer um dos 4 tetos por-alvo, nunca dispara antes deles) pra nunca
    # reprovar teste legitimo, e teto o bastante pra transformar um hang de 9
    # HORAS em, no pior caso (teste sem PROPERTIES por algum motivo futuro), 2
    # MINUTOS. NAO protege execucao DIRETA do binario de teste (fora do ctest) -
    # NAO conserta o deadlock em si (isso e bug do glintfx/PipeWire, ja
    # reportado); e cinto de seguranca, nao conserto.
    CTEST_TIMEOUT="${CHECK_CTEST_TIMEOUT:-120}"

    # FIX (2026-07-14): capturar o exit do ctest LOGO APOS o pipe ctest|tee (o `|| true`
    # de um grep depois resetava PIPESTATUS e mascarava falha -> verde cego).
    if [ -n "$_suite_disp" ]; then
        ( cd "$ENGINE" && env -u WAYLAND_DISPLAY DISPLAY="$_suite_disp" SDL_VIDEODRIVER=x11 \
            XDG_RUNTIME_DIR="$_suite_xdg" XDG_SESSION_TYPE=x11 \
            DBUS_SESSION_BUS_ADDRESS="unix:path=$_suite_xdg/no-dbus" \
            ctest --preset "$PRESET" --timeout "$CTEST_TIMEOUT" 2>&1 ) | tee "$CTEST_LOG" >/dev/null
    else
        # sem display isolado: NUNCA rodar GL no :0 vivo -> offscreen (GL degrada).
        # O XDG_RUNTIME_DIR proprio vai TAMBEM aqui: sem ele, uma dependencia que chame
        # wl_display_connect(NULL) ainda acharia o wayland-0 da sessao viva mesmo com o
        # DISPLAY vazio (o fallback e por nome fixo dentro do runtime dir, nao por DISPLAY).
        ( cd "$ENGINE" && env -u WAYLAND_DISPLAY DISPLAY= SDL_VIDEODRIVER=offscreen \
            XDG_RUNTIME_DIR="$_suite_xdg" XDG_SESSION_TYPE=x11 \
            DBUS_SESSION_BUS_ADDRESS="unix:path=$_suite_xdg/no-dbus" \
            ctest --preset "$PRESET" --timeout "$CTEST_TIMEOUT" 2>&1 ) | tee "$CTEST_LOG" >/dev/null
    fi
    SUITE=${PIPESTATUS[0]}
    [ -n "$_suite_xvfb_pid" ] && kill "$_suite_xvfb_pid" 2>/dev/null
    grep -E "tests passed|tests failed|Total Test" "$CTEST_LOG" || true
    set -e
    echo "SUITE=$SUITE"
fi

# ---------------------------------------------------------------- VEREDITO
# Em --gates-only o veredito olha SO o GATE, EXPLICITAMENTE - e nao "os outros
# ficaram 0 mesmo, entao a conta da certo". A diferenca importa no dia em que
# nascer um estagio novo: com a composicao explicita, esquecer de trata-lo aqui
# aparece; com o zero-por-omissao, ele passaria em silencio (o mesmo defeito que
# o GATE_TOOLS_TESTS teve em 2026-08-06, ver tools/tests/test_check_sh_gates_ligados.py).
RC=0
if [ "$GATES_ONLY" = "1" ]; then
    [ "$GATE" = "0" ] || RC=1
else
    [ "$BUILD" = "0" ] || RC=1
    [ "$SMOKE" = "0" ] || RC=1
    [ "$GATE"  = "0" ] || RC=1
    [ "$SUITE" = "0" ] || RC=1
fi
echo "=== resultado: $([ $RC = 0 ] && echo OK || echo FALHOU) (rc=$RC) ==="
exit $RC
