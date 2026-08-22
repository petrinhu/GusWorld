# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Gate de commit-msg do GusWorld.

Barra commit cuja MENSAGEM contenha termo da lista de termos proibidos, que
vive cifrada com git-crypt em tools/security/termos_proibidos.txt (mesma
chave do resto do repositorio, GODS_LAWS.md L-25).

Principio-mae, GODS_LAWS.md L-19: "gate que nao consegue medir tem de
FALHAR, nunca aprovar em silencio". Sem a chave do git-crypt destravada
nesta copia de trabalho, o arquivo de termos e ilegivel -- e o gate
BLOQUEIA o commit em vez de deixar passar sem checar nada.

Documentacao completa (sem citar nenhum termo da lista): ver
docs/security/commit-gate.md.

Uso normal (via hook commit-msg, ver tools/git-hooks/commit-msg):
    python3 commit_gate.py <arquivo-com-a-mensagem-do-commit>

Escape explicito para o lider, se o gate barrar por engano (nunca um
--no-verify silencioso): definir a variavel de ambiente
GUSWORLD_COMMIT_GATE_OVERRIDE=1 antes do commit. O override e sempre
visivel: o gate imprime o que teria bloqueado, mesmo liberando o commit.
"""
from __future__ import annotations

import os
import re
import sys
import unicodedata
from pathlib import Path
from typing import Dict, List, Optional, Pattern, Tuple

TERMOS_PATH = Path(__file__).resolve().parent / "termos_proibidos.txt"
OVERRIDE_ENV = "GUSWORLD_COMMIT_GATE_OVERRIDE"

# Cabecalho binario que o git-crypt grava no blob quando o arquivo esta
# cifrado e a chave nao foi usada para decifra-lo (smudge filter nao
# rodou). Presenca deste cabecalho no arquivo lido do disco de trabalho
# significa "a chave nao esta destravada aqui", nao "o arquivo esta vazio".
GITCRYPT_MAGIC = b"\x00GITCRYPT\x00"


class TermosIndisponiveis(Exception):
    """A lista de termos nao pode ser lida com confianca.

    Cobre: arquivo ausente, ainda cifrado (sem chave), vazio, binario
    ilegivel como texto, ou com uma linha fora do formato esperado. Em
    qualquer um destes casos o chamador tem que tratar como FALHA do
    gate (bloqueia o commit), nunca como "lista vazia, nada a bloquear".
    """


def strip_accents(texto: str) -> str:
    """Remove marca diacritica (acento, til, cedilha) preservando a letra base."""
    decomposto = unicodedata.normalize("NFKD", texto)
    return "".join(c for c in decomposto if not unicodedata.combining(c))


def normalize(texto: str) -> str:
    """Forma canonica pra comparacao: sem acento, sem caixa, espaco unico.

    Colapsar toda sequencia de espaco (inclusive quebra de linha) num
    espaco so evita o falso negativo de frase partida entre linhas
    (licao registrada em 2026-07-27: busca literal em texto extraido
    perde frase que o extrator quebrou ao meio).
    """
    sem_acento = strip_accents(texto).casefold()
    return re.sub(r"\s+", " ", sem_acento)


def carregar_termos(caminho: Path = TERMOS_PATH) -> List[Tuple[str, str]]:
    """Le o arquivo de termos e devolve lista de pares (classe, termo).

    Levanta TermosIndisponiveis se o arquivo nao puder ser lido com
    confianca (ver docstring da excecao). Formato esperado:

        ## NOME_DA_CLASSE
        termo um
        outro termo
        # comentario, ignorado

        ## OUTRA_CLASSE
        ...
    """
    if not caminho.exists():
        raise TermosIndisponiveis(f"arquivo nao encontrado: {caminho}")

    bruto = caminho.read_bytes()

    if bruto.startswith(GITCRYPT_MAGIC):
        raise TermosIndisponiveis(
            "arquivo ainda cifrado por git-crypt (chave nao destravada "
            "nesta copia de trabalho)"
        )

    try:
        texto = bruto.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise TermosIndisponiveis(
            f"conteudo ilegivel como utf-8 ({exc}); pode ser blob cifrado "
            "com cabecalho nao reconhecido"
        ) from exc

    if not texto.strip():
        raise TermosIndisponiveis("arquivo vazio")

    termos: List[Tuple[str, str]] = []
    classe_atual: Optional[str] = None
    viu_alguma_classe = False

    for numero_linha, linha in enumerate(texto.splitlines(), start=1):
        linha_limpa = linha.strip()
        if not linha_limpa:
            continue
        if linha_limpa.startswith("## "):
            classe_atual = linha_limpa[3:].strip()
            viu_alguma_classe = True
            continue
        if linha_limpa.startswith("#"):
            continue
        if classe_atual is None:
            raise TermosIndisponiveis(
                f"linha {numero_linha}: termo fora de qualquer secao "
                "'## NOME_DA_CLASSE'"
            )
        termos.append((classe_atual, linha_limpa))

    if not viu_alguma_classe or not termos:
        raise TermosIndisponiveis(
            "nenhuma classe ou termo valido encontrado no arquivo"
        )

    return termos


def construir_padroes(
    termos: List[Tuple[str, str]]
) -> List[Tuple[str, str, Pattern[str]]]:
    """Compila cada termo num regex com fronteira de palavra nos dois lados.

    A comparacao roda sobre o texto ja normalizado (normalize()), entao
    o padrao tambem usa o termo normalizado.

    NAO usa `\\b` do Python: `\\b` e relativo (compara o par de caracteres
    dos dois lados da posicao), e quebra quando o termo COMECA ou TERMINA
    com um caractere que ja nao e "palavra" (ex.: um caminho absoluto tipo
    "/caminho/de/maquina" comecando com "/"). Medido ao vivo nesta suite
    com um termo real da classe TECNICO_MAQUINA: `\\b` deixava de casar
    quando o termo vinha precedido de espaco, porque espaco e "/" sao os
    dois nao-palavra, e o par nao produz transicao.

    Em vez disso usa lookaround absoluto: nao pode haver caractere de
    palavra imediatamente ANTES do inicio do termo, nem imediatamente
    DEPOIS do fim. Isso barra "eu" dentro de "seu"/"meu" e "rml" dentro de
    "firmly"/"Stormlight" do mesmo jeito, e cobre termo com barra, hifen
    ou ponto na borda sem caso especial.
    """
    padroes = []
    for classe, termo in termos:
        termo_norm = normalize(termo)
        if not termo_norm:
            continue
        padrao = re.compile(r"(?<!\w)" + re.escape(termo_norm) + r"(?!\w)")
        padroes.append((classe, termo, padrao))
    return padroes


def encontrar_violacoes(
    mensagem: str, padroes: List[Tuple[str, str, Pattern[str]]]
) -> List[Tuple[str, str]]:
    """Devolve lista de (classe, termo_original) que casaram na mensagem."""
    msg_norm = normalize(mensagem)
    achados = []
    for classe, termo_original, padrao in padroes:
        if padrao.search(msg_norm):
            achados.append((classe, termo_original))
    return achados


def _formatar_bloqueio(achados: List[Tuple[str, str]]) -> str:
    linhas = ["commit_gate: BLOQUEADO -- mensagem de commit contem termo proibido:"]
    for classe, termo in achados:
        linhas.append(f'  - classe [{classe}]: "{termo}"')
    return "\n".join(linhas)


def main(
    argv: List[str],
    termos_path: Path = TERMOS_PATH,
    env: Optional[Dict[str, str]] = None,
) -> int:
    """Ponto de entrada do hook commit-msg.

    argv[1] e o caminho do arquivo com a mensagem de commit (contrato
    padrao do hook commit-msg do git). `termos_path` e `env` sao pontos
    de injecao pra teste; em uso real ficam no default (arquivo real do
    projeto, os.environ do processo).
    """
    if env is None:
        env = os.environ

    if len(argv) < 2:
        print(
            "commit_gate: uso: commit_gate.py <arquivo-de-mensagem-de-commit>",
            file=sys.stderr,
        )
        return 2

    msg_path = Path(argv[1])
    try:
        mensagem = msg_path.read_text(encoding="utf-8", errors="replace")
    except OSError as exc:
        print(
            f"commit_gate: nao consegui ler a mensagem de commit ({exc})",
            file=sys.stderr,
        )
        return 2

    override = env.get(OVERRIDE_ENV, "").strip()

    try:
        termos = carregar_termos(termos_path)
    except TermosIndisponiveis as exc:
        if override:
            print(
                "commit_gate: AVISO -- lista de termos indisponivel "
                f"({exc}); commit PERMITIDO só porque {OVERRIDE_ENV} esta "
                "definida. Nada foi verificado nesta mensagem.",
                file=sys.stderr,
            )
            return 0
        print(
            "commit_gate: BLOQUEADO -- nao consigo medir (lista de termos "
            f"proibidos indisponivel: {exc}).\n"
            "Isto NAO significa que a mensagem esta limpa: o gate falha em "
            "vez de aprovar em silencio (GODS_LAWS.md L-19).\n"
            "Destrave a chave do git-crypt nesta copia de trabalho "
            "(git-crypt unlock <arquivo-de-chave>) e tente de novo.\n"
            f"Para forcar o commit por sua conta e risco, defina "
            f"{OVERRIDE_ENV}=1 antes do commit.",
            file=sys.stderr,
        )
        return 1

    padroes = construir_padroes(termos)
    achados = encontrar_violacoes(mensagem, padroes)

    if not achados:
        return 0

    if override:
        print(
            _formatar_bloqueio(achados)
            + f"\n\ncommit_gate: AVISO -- commit PERMITIDO mesmo assim, "
            f"{OVERRIDE_ENV} esta definida. Revise a mensagem antes de "
            "publicar (push exige autorizacao a parte de qualquer forma).",
            file=sys.stderr,
        )
        return 0

    print(
        _formatar_bloqueio(achados)
        + "\n\nReescreva a mensagem sem o(s) termo(s) acima, ou, se for "
        f"engano do gate, defina {OVERRIDE_ENV}=1 para este commit.",
        file=sys.stderr,
    )
    return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
