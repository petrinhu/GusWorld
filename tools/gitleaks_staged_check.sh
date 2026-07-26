#!/usr/bin/env bash
# tools/gitleaks_staged_check.sh
#
# Recomendacao no 3 do TST-8 (docs/auditoria/TST-8-secret-scan-2026-07-25.md
# secao 7): gate de secret-scan no pre-commit, rodando SO nos arquivos staged.
# Chamado pelo pre-commit LOCAL do repo (.git/hooks/pre-commit), que o shim
# global de core.hooksPath (~/.claude/githooks/pre-commit -> _chain.sh)
# encadeia automaticamente. Mesmo padrao do irmao crash_journal_check.sh.
#
# Este e o UNICO gate da familia que BLOQUEIA: segredo commitado nao se
# desfaz com um commit de conserto - o objeto fica no historico, e limpar
# exige reescrita + force-push (foi o que aconteceu com os frames de tela em
# 2026-07-18). Custo de bloquear um commit falso-positivo: segundos. Custo de
# deixar passar um de verdade: rotacao de credencial + reescrita de historico.
#
# --- por que SO os staged, nunca a arvore ----------------------------------
# `gitleaks dir .` varre 1,40 GB e leva ~47s nesta maquina (medido no TST-8,
# secao 8). Gate de 47s por commit vira habito de pular. `gitleaks git
# --staged` le so o diff do indice: sub-segundo. Como consequencia, este gate
# NAO cobre arquivo que ja estava no repo antes - isso e trabalho da varredura
# de arvore/historico, deliberada e com humano lendo a saida.
#
# --- subcomando: serie 8.30 --------------------------------------------------
# `gitleaks detect` (sintaxe da 8.18 e anteriores, ainda citada por muito
# tutorial) FOI REMOVIDO. Na 8.30 existem apenas: git, dir, stdin, version.
# `--staged` e flag do subcomando `git` (`gitleaks git --help` confirma:
# "scan staged commits (good for pre-commit)"). Se um dia a serie mudar de
# novo, conferir com --help antes de editar - nao presumir.
#
# --- auto-desativacao (nao impor varredura a outros projetos) --------------
# Em tese este script fica so no GusWorld, mas a familia de hooks desta
# maquina tem uma perna GLOBAL (core.hooksPath aponta pra ~/.claude/githooks,
# que vale pra TODOS os repos). Se alguem copiar este arquivo pra la, ele
# precisa ser inofensivo onde ninguem pediu: sem .gitleaks.toml na raiz, sai 0
# em silencio. Sem essa guarda, adotar o gate aqui imporia varredura a
# qualquer repositorio da maquina.
#
# --- gitleaks ausente: avisa e PASSA ---------------------------------------
# Hook que bloqueia commit porque a ferramenta nao esta instalada e pior que
# hook nenhum: quebra a maquina de quem clonou o repo sem pedir nada em troca.
# Falha aberta SO nesse caso, e falando alto.
#
# --- escape ----------------------------------------------------------------
#   GUSWORLD_SKIP_GITLEAKS=1 git commit ...     (uma execucao, so isso)
# De proposito NAO ha marker file como no crash_journal_check.sh: marker
# desliga o gate ate alguem lembrar de apagar, e gate de seguranca cego em
# silencio e o pior estado possivel. Falso positivo recorrente se resolve no
# .gitleaks.toml (allowlist de tres eixos, ver o cabecalho de la), nao no
# escape.
#
# Uso: sem argumentos, pelo hook. Rodavel a mao (idempotente, so leitura).

set -uo pipefail
# SEM -e: o unico caminho de saida deste script sao os `exit` explicitos
# abaixo. Um `command -v` falhando nao pode virar exit code indefinido num
# gate que bloqueia commit.

ROOT="$(git rev-parse --show-toplevel 2>/dev/null)" || exit 0
[ -n "$ROOT" ] || exit 0

# --- escape (uma execucao) --------------------------------------------------
if [ "${GUSWORLD_SKIP_GITLEAKS:-0}" = "1" ]; then
    echo "gitleaks: pulado por GUSWORLD_SKIP_GITLEAKS=1" >&2
    exit 0
fi

# --- auto-desativacao: repo sem .gitleaks.toml nao pediu este gate ---------
CONFIG="$ROOT/.gitleaks.toml"
[ -f "$CONFIG" ] || exit 0

# --- ferramenta ausente: avisa e passa -------------------------------------
if ! command -v gitleaks >/dev/null 2>&1; then
    echo "" >&2
    echo "AVISO (nao bloqueia): gitleaks nao esta no PATH - o secret-scan de" >&2
    echo "pre-commit NAO rodou neste commit. Instale para reativar o gate:" >&2
    echo "  Fedora/RHEL : sudo dnf install gitleaks" >&2
    echo "  macOS       : brew install gitleaks" >&2
    echo "  Windows     : winget install gitleaks" >&2
    echo "" >&2
    exit 0
fi

# --- nada staged: nao ha o que varrer --------------------------------------
# Cobre `git commit --amend` sem mudanca e o merge commit sem conflito.
# Se este comando falhar por outro motivo (repo sem HEAD, por ex.), o rc != 0
# nos leva a varrer - direcao segura.
if git diff --cached --quiet 2>/dev/null; then
    exit 0
fi

# --- a varredura ------------------------------------------------------------
# --redact=80: mostra so 20% do achado. Da pra identificar QUAL string sem
# imprimir a credencial inteira no terminal (e no scrollback, e no log da
# sessao do agente). Arquivo, linha e regra saem completos, que e o que
# importa pra localizar.
# -v e OBRIGATORIO aqui: sem ele a saida e so "leaks found: 1", sem arquivo,
# linha nem regra - inacionavel. --log-level warn corta o ruido de INF sem
# tirar o bloco do achado (que sai por outro canal, nao pelo logger).
# --no-color porque a saida vai pra stderr de um hook, que muitas vezes
# termina em log/arquivo, e ali codigo ANSI so atrapalha.
OUT="$(gitleaks git --staged --no-banner --redact=80 -v --no-color \
        --log-level warn -c "$CONFIG" "$ROOT" 2>&1)"
RC=$?

if [ $RC -eq 0 ]; then
    exit 0
fi

echo "" >&2
echo "############################################################" >&2
echo "# COMMIT BLOQUEADO: gitleaks achou segredo no que esta      #" >&2
echo "# STAGED (gitleaks exit $RC).                                #" >&2
echo "############################################################" >&2
echo "$OUT" | sed 's/^/  /' >&2
echo "" >&2
echo "O que fazer:" >&2
echo "  1. E segredo de verdade? Tire do arquivo e ROTACIONE a credencial" >&2
echo "     (ela ja passou pelo seu disco; trate como comprometida)." >&2
echo "  2. E falso positivo? Adicione entrada no .gitleaks.toml com os TRES" >&2
echo "     eixos (targetRules + paths + regexes, condition=\"AND\"). Nunca" >&2
echo "     isente um caminho ou uma regra inteira - ver o cabecalho de la." >&2
echo "  3. Precisa passar AGORA e resolver depois (excepcional):" >&2
echo "     GUSWORLD_SKIP_GITLEAKS=1 git commit ..." >&2
echo "" >&2

exit 1
