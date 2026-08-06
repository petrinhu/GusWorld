#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""ui_log_key_only.py - GATE(ui-log-key-only) do tools/check.sh.

O DEFEITO QUE MOTIVOU (I18N-UILOG, 2026-08-06): o canal `ui_log_` da BattleScene
carregava texto de TELA escrito direto no .cpp, fora do tradutor, numa tela cujo
resto e 100% traduzido - jogador em ingles lia portugues. Eram 3 call sites, e
duas das frases ja tinham sido reescritas no codigo sem nenhum teste travando
nada (uma delas foi parar em producao com erro de ortografia: "modulo" sem
acento, numa frase declarada "texto final").

AS 3 REGRAS (todas zero-tolerancia, superficie MEDIDA e FECHADA):

  R1  Nenhum `ui_log_.push_back(...)` com LITERAL de string na expressao. O
      texto que o jogador le vem do catalogo (resources/translations/*.md), e o
      unico push_back legitimo e o de dentro do helper BattleScene::push_ui_log,
      que empilha o resultado do translator (variavel, nao literal).

  R2  SENTINELA DE ESCOPO: tem de existir pelo menos UM `ui_log_.push_back(`
      no escopo varrido. Zero significa que o membro foi renomeado/removido -
      e um gate que varre nada imprime "OK" para sempre sem proteger coisa
      nenhuma (o modo de falha mais covarde que existe). Zero REPROVA, com a
      instrucao de atualizar ou aposentar este gate de proposito.

  R3  Toda chave passada a `push_ui_log(kind, "CHAVE")` tem de ser um literal
      de string E existir como `## CHAVE` nos DOIS catalogos. Esta e a regra que
      fecha o furo obvio de R1: sem ela, bastaria escrever
      `push_ui_log(Kind, "texto solto em pt-br")` pra o literal voltar por
      dentro do helper. Chave nao-literal (variavel/expressao) tambem reprova:
      o valor deixaria de ser auditavel estaticamente, que e todo o ponto.

O QUE ESTE GATE **NAO** COBRE (declarado, nao omitido):

  - Literal de texto em OUTROS builders de tela (o `[BLOQUEADO]` que esta mesma
    fatia tirou de difficulty_menu_rml.cpp, por exemplo, nasceu num `body <<`
    de RML, nao num ui_log_). Cobrir "qualquer literal em builder de UI" exigiria
    distinguir texto de jogador de id de elemento/classe CSS/tag RML - todos
    literais legitimos e MUITO mais numerosos - e o gate viraria ruido. Quem
    protege esse terreno sao os testes de ORIGEM-NO-CATALOGO (tecnica do
    sentinela: catalogo de teste com valor unico, que so aparece na saida se o
    builder de fato consultou o translator), em
    app/tests/difficulty_menu_rml_test.cpp e app/tests/battle_scene_test.cpp.

  - Literal ACENTUADO como heuristica de "isto e texto de jogador": MEDIDO e
    DESCARTADO nesta fatia - os 4 literais ofensores reais (as 3 frases de stub
    do ui_log_ + o badge) sao TODOS sem acento. Um detector de acento erraria
    4 de 4. Gate que nao pega o defeito que motivou sua criacao e teatro.

Reusa strip_comments/PRODUCTION_DIRS de sdl_layer_ratchet.py (mesmo parser de
caractere, mesmo motivo: os comentarios desta fatia CITAM as frases antigas de
proposito, e um grep ingenuo reprovaria justamente a documentacao do conserto).
"""

import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sdl_layer_ratchet import PRODUCTION_DIRS, ROOT, strip_comments  # noqa: E402

CATALOGS = [
    os.path.join(ROOT, "resources", "translations", "pt_br.md"),
    os.path.join(ROOT, "resources", "translations", "en_intl.md"),
]

_PUSH_BACK_RE = re.compile(r"ui_log_\s*\.\s*push_back\s*\(")
_HELPER_RE = re.compile(r"push_ui_log\s*\(")
# A DECLARACAO (no .hpp) e a DEFINICAO (no .cpp) do proprio helper casam o padrao
# acima sem serem chamadas. Casadas aqui pra serem descontadas por POSICAO (nao por
# "tem 'void' por perto", heuristica que uma chamada dentro de uma lambda `void`
# derrubaria).
_HELPER_DECL_RE = re.compile(
    r"\bvoid\s+(?:[A-Za-z_][A-Za-z0-9_]*\s*::\s*)?push_ui_log\s*\(")
# Chave i18n valida: MAIUSCULA/digito/underscore, como todas as do catalogo.
_KEY_RE = re.compile(r'^"([A-Z0-9_]+)"$')


def _balanced_args(text: str, open_paren_idx: int) -> str:
    """Devolve o texto entre o '(' em open_paren_idx e o ')' que o fecha,
    contando aninhamento de parenteses e ignorando parenteses dentro de string
    (`LogLine{...}` e chamadas aninhadas sao comuns nestes call sites)."""
    depth = 0
    i = open_paren_idx
    n = len(text)
    in_string = False
    out = []
    while i < n:
        c = text[i]
        if in_string:
            if c == "\\":
                out.append(text[i:i + 2])
                i += 2
                continue
            if c == '"':
                in_string = False
            out.append(c)
            i += 1
            continue
        if c == '"':
            in_string = True
            out.append(c)
            i += 1
            continue
        if c == "(":
            depth += 1
            if depth == 1:
                i += 1
                continue
        elif c == ")":
            depth -= 1
            if depth == 0:
                return "".join(out)
        out.append(c)
        i += 1
    return "".join(out)  # nao fechou (arquivo truncado?): devolve o que deu


def _split_top_level(args: str) -> list:
    """Separa argumentos por virgula de nivel ZERO (fora de (), {}, <> de
    template e strings)."""
    parts = []
    cur = []
    depth = 0
    in_string = False
    i = 0
    n = len(args)
    while i < n:
        c = args[i]
        if in_string:
            if c == "\\":
                cur.append(args[i:i + 2])
                i += 2
                continue
            if c == '"':
                in_string = False
            cur.append(c)
            i += 1
            continue
        if c == '"':
            in_string = True
        elif c in "({[<":
            depth += 1
        elif c in ")}]>":
            depth -= 1
        elif c == "," and depth == 0:
            parts.append("".join(cur).strip())
            cur = []
            i += 1
            continue
        cur.append(c)
        i += 1
    if "".join(cur).strip():
        parts.append("".join(cur).strip())
    return parts


def _catalog_keys(path: str) -> set:
    keys = set()
    if not os.path.isfile(path):
        return keys
    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            if line.startswith("## "):
                keys.add(line[3:].strip())
    return keys


def _production_sources():
    for base in PRODUCTION_DIRS:
        if not os.path.isdir(base):
            continue
        for dirpath, _dirnames, filenames in os.walk(base):
            for fn in sorted(filenames):
                if fn.endswith(".cpp") or fn.endswith(".hpp"):
                    yield os.path.join(dirpath, fn)


def main() -> int:
    catalogs = {p: _catalog_keys(p) for p in CATALOGS}
    missing_catalog = [p for p, k in catalogs.items() if not k]
    if missing_catalog:
        print("GATE(ui-log-key-only): FALHA - catalogo ilegivel/vazio: "
              + ", ".join(os.path.relpath(p, ROOT) for p in missing_catalog))
        return 1

    push_back_sites = 0
    helper_sites = 0
    violations = []  # (regra, caminho, detalhe)
    files_scanned = 0

    for path in _production_sources():
        files_scanned += 1
        rel = os.path.relpath(path, ROOT)
        with open(path, encoding="utf-8", errors="replace") as f:
            code = strip_comments(f.read())

        # R1: ui_log_.push_back com literal de string na expressao.
        for m in _PUSH_BACK_RE.finditer(code):
            push_back_sites += 1
            args = _balanced_args(code, m.end() - 1)
            if '"' in args:
                violations.append(
                    ("R1", rel, " ".join(args.split())[:120]))

        # R3: chave do helper tem de ser literal E existir nos 2 catalogos.
        decl_ends = {m.end() for m in _HELPER_DECL_RE.finditer(code)}
        for m in _HELPER_RE.finditer(code):
            if m.end() in decl_ends:
                continue  # declaracao/definicao do proprio helper, nao chamada
            helper_sites += 1
            args = _split_top_level(_balanced_args(code, m.end() - 1))
            if len(args) != 2:
                violations.append(
                    ("R3", rel, f"esperava (kind, \"CHAVE\"), veio: {args}"))
                continue
            key_match = _KEY_RE.match(args[1])
            if key_match is None:
                violations.append(
                    ("R3", rel,
                     f"chave NAO e literal de string em MAIUSCULAS: {args[1][:80]}"))
                continue
            key = key_match.group(1)
            for cat, keys in catalogs.items():
                if key not in keys:
                    violations.append(
                        ("R3", rel,
                         f"chave {key} ausente de {os.path.relpath(cat, ROOT)}"))

    # R2: sentinela de escopo.
    if push_back_sites == 0:
        print("GATE(ui-log-key-only): FALHA (R2, sentinela de escopo) - nenhum "
              f"`ui_log_.push_back(` encontrado em {files_scanned} arquivo(s) de "
              "producao varrido(s).")
        print("  O membro foi renomeado/removido, ou o escopo da varredura quebrou. "
              "Um gate que varre ZERO site imprime OK para sempre sem proteger nada: "
              "atualize o padrao aqui, ou aposente este gate DE PROPOSITO (e diga por "
              "que no docstring).")
        return 1

    if violations:
        print(f"GATE(ui-log-key-only): FALHA - {len(violations)} violacao(oes) "
              f"em {files_scanned} arquivo(s) de producao varrido(s).")
        for regra, rel, detalhe in violations:
            print(f"    - [{regra}] {rel}: {detalhe}")
        print("  R1: texto de tela NAO se escreve em ui_log_.push_back - use "
              "BattleScene::push_ui_log(kind, \"CHAVE_I18N\").")
        print("  R3: a chave tem de ser literal em MAIUSCULAS e existir nos DOIS "
              "catalogos (resources/translations/pt_br.md e en_intl.md).")
        return 1

    print(f"GATE(ui-log-key-only): OK ({push_back_sites} `ui_log_.push_back` sem "
          f"literal e {helper_sites} chamada(s) de push_ui_log com chave existente nos "
          f"2 catalogos, em {files_scanned} arquivo(s) de producao varrido(s)).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
