#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Regenera a PARTE MEDIDA de docs/art/sprites-inventory.md a partir do disco.

Existe porque docs/art/sprites-inventory.md e mantido a mao, e em 31/08/2026 uma
reforma mediu oito divergencias entre o que o documento dizia e o que
resources/sprites/ de fato tinha. O documento derivou porque nada o mantinha
honesto -- este script tira essa carga de quem escreve a mao.

O que este script cobre, e SO isto (GODS_LAWS.md L-04/L-33 -- atomo com fronteira
propria, nao dono de tudo): por pasta de nivel 1 de resources/sprites/,
contagem de arquivos no topo, presenca e tamanho de walk/ e de anims/, e as
dimensoes distintas de imagem encontradas recursivamente. Julgamento sobre o
que cada pasta significa (quem e personagem, quem e pasta especial, o que
esta pendente de geracao) NAO e mecanico -- continua escrito a mao na PARTE DE
JULGAMENTO do documento, e este script nunca toca nela.

Fronteira entre a parte gerada e a parte escrita a mao (GODS_LAWS.md L-05
global -- marcador explicito, nunca heuristica): o par de comentarios HTML
MARCADOR_INICIO/MARCADOR_FIM abaixo. So o texto ENTRE eles e substituido. Na
primeira execucao, sem o par ainda no arquivo, o bloco novo entra logo antes
do separador "---" que precede "## PARTE DE JULGAMENTO" (o proprio documento
ja usa esse cabecalho como fronteira entre medido e julgado); nas execucoes
seguintes o marcador já presente decide o lugar, sem depender mais desse
cabecalho.

Ferramenta de apoio (GODS_LAWS.md LEI ZERO, reforma de 28/08/2026): roda na
maquina de quem constroi, le o disco, escreve um documento -- nunca embarca no
binario do jogo, logo nao e dependencia proibida.

Uso:
    python3 tools/art/sprites_inventory_gen.py           # regenera e grava
    python3 tools/art/sprites_inventory_gen.py --check   # so avisa se o bloco no
                                                          # disco esta desatualizado
                                                          # (saida 1 = desatualizado)
    python3 tools/art/sprites_inventory_gen.py --sprites-root DIR --doc ARQUIVO
                                                          # aponta para outra arvore
                                                          # e outro documento (usado
                                                          # pelo teste de sabotagem)
    python3 tools/art/sprites_inventory_gen.py --permite-vazio
                                                          # nao reprova se a arvore
                                                          # de sprites nao tiver
                                                          # nenhuma pasta de nivel 1
                                                          # (use so quando vazio for
                                                          # mesmo o estado esperado)

Comparacao de desatualizado (--check) ignora o instante de geracao: o bloco no
disco so conta como desatualizado quando o que descreve o disco muda (tabela,
contagens, dimensoes), nunca so porque um minuto passou desde a ultima geracao.

Saida: 0 = ok (gravado, ou --check confirmou que ja esta em dia)
       1 = --check achou o bloco desatualizado, ou os numeros de cobertura nao
           fecharam (analisados + falharam != encontrados -- bug deste script)
       2 = erro de uso (documento sem o cabeçalho-ancora na primeira execucao,
           arvore de sprites inexistente, varredura vazia sem --permite-vazio,
           etc.)
       3 = falha de leitura encontrada na varredura (pasta ilegivel ou imagem
           corrompida/truncada) -- distinto de "desatualizado": o disco tem um
           problema proprio, que o texto do documento so registra, nao resolve
"""
from __future__ import annotations

import argparse
import datetime as _dt
import re
import sys
from collections import Counter
from pathlib import Path
from typing import Optional

try:
    from PIL import Image
except ImportError:  # pragma: no cover - falha reportada, nunca escondida
    Image = None

REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SPRITES_ROOT = REPO_ROOT / "resources" / "sprites"
DEFAULT_DOC_PATH = REPO_ROOT / "docs" / "art" / "sprites-inventory.md"

IMAGE_EXTS = {".png", ".jpg", ".jpeg", ".gif"}

MARCADOR_INICIO = "<!-- INICIO BLOCO GERADO por tools/art/sprites_inventory_gen.py -- NAO EDITAR A MAO ENTRE ESTAS MARCAS -->"
MARCADOR_FIM = "<!-- FIM BLOCO GERADO -->"
ANCORA_JULGAMENTO = "## PARTE DE JULGAMENTO"

# O bloco gerado embute "em **DD/MM/AAAA, HH:MM**" (o instante da geracao) --
# isso e informativo, mas nao pode entrar na comparacao de "esta desatualizado",
# senao o portao fica vermelho um minuto depois de qualquer geracao, sem que
# nada no disco tenha mudado. Este regex normaliza so essa parte antes de
# comparar; o texto gravado no documento continua com o instante real.
_TIMESTAMP_NO_CABECALHO_RE = re.compile(r"\*\*\d{2}/\d{2}/\d{4}, \d{2}:\d{2}\*\*")


def _normaliza_para_comparacao(bloco: str) -> str:
    return _TIMESTAMP_NO_CABECALHO_RE.sub("**TIMESTAMP**", bloco)


def _rel_ou_abs(caminho: Path) -> str:
    """Caminho relativo ao repo quando possivel; absoluto quando `caminho` fica
    fora da arvore do repositorio (--sprites-root apontando para outra arvore,
    como no teste de sabotagem). `Path.relative_to` lanca ValueError nesse caso
    -- sem este helper, essa excecao escapava de dentro de um bloco que so
    devia tratar falha de leitura de imagem/pasta."""
    try:
        return str(caminho.relative_to(REPO_ROOT))
    except ValueError:
        return str(caminho)


class ErroDeUso(Exception):
    """Situacao que impede o script de agir com confianca -- nunca segue calado."""


class Cobertura:
    """Conta encontrados/analisados/falharam de um tipo de item (GODS_LAWS.md L-36:
    ferramenta que processa em lote e morre no meio esconde cobertura perdida --
    aqui o laco e item a item e os tres numeros sempre fecham entre si)."""

    def __init__(self, nome: str):
        self.nome = nome
        self.encontrados = 0
        self.analisados = 0
        self.falhas: list[str] = []

    def registra_encontrado(self) -> None:
        self.encontrados += 1

    def registra_sucesso(self) -> None:
        self.analisados += 1

    def registra_falha(self, descricao: str) -> None:
        self.falhas.append(descricao)

    def fecha_ou_lanca(self) -> None:
        if self.analisados + len(self.falhas) != self.encontrados:
            raise ErroDeUso(
                f"cobertura de '{self.nome}' nao fecha: "
                f"{self.encontrados} encontrados != "
                f"{self.analisados} analisados + {len(self.falhas)} falharam "
                "-- bug deste script, nao do disco"
            )

    def linha_relatorio(self) -> str:
        return (
            f"{self.nome}: {self.encontrados} encontrados, "
            f"{self.analisados} analisados, {len(self.falhas)} falharam"
        )


def _conta_arquivos_topo(pasta: Path) -> int:
    return sum(1 for f in pasta.iterdir() if f.is_file())


def _conta_arquivos_recursivo(pasta: Path) -> int:
    return sum(1 for f in pasta.rglob("*") if f.is_file())


def _dimensoes_recursivo(pasta: Path, cobertura_imgs: Cobertura) -> "Counter[str]":
    """Le a dimensao de cada imagem sob `pasta`, um arquivo por vez, num unico
    processo Python (GODS_LAWS.md L-11 global: proibido gastar um processo por
    item varrido -- nao se chama `identify` em laco, PIL le tudo dentro deste
    processo so)."""
    dims: Counter[str] = Counter()
    for arq in sorted(pasta.rglob("*")):
        if not arq.is_file() or arq.suffix.lower() not in IMAGE_EXTS:
            continue
        cobertura_imgs.registra_encontrado()
        try:
            with Image.open(arq) as img:
                largura, altura = img.size
                img.load()  # forca a leitura completa dos dados -- Image.open
                            # e preguicoso e so le o cabecalho, entao um arquivo
                            # truncado no meio dos dados passaria com dimensoes
                            # certas e falha nenhuma sem este load() explicito
            dims[f"{largura}x{altura}"] += 1
            cobertura_imgs.registra_sucesso()
        except Exception as exc:  # arquivo corrompido, truncado, formato invalido
            cobertura_imgs.registra_falha(f"{_rel_ou_abs(arq)}: {exc}")
    return dims


def _formata_dimensoes(dims: "Counter[str]") -> str:
    if not dims:
        return "(nenhuma imagem)"
    partes = [f"{dim} (x{n})" for dim, n in sorted(dims.items(), key=lambda kv: (-kv[1], kv[0]))]
    return ", ".join(partes)


def inventaria(sprites_root: Path) -> tuple[list[dict], Cobertura, Cobertura]:
    if not sprites_root.is_dir():
        raise ErroDeUso(f"arvore de sprites nao encontrada: {sprites_root}")

    cobertura_pastas = Cobertura("pastas de nivel 1")
    cobertura_imgs = Cobertura("imagens")

    linhas: list[dict] = []
    for pasta in sorted(p for p in sprites_root.iterdir() if p.is_dir()):
        cobertura_pastas.registra_encontrado()
        try:
            walk_dir = pasta / "walk"
            anims_dir = pasta / "anims"
            linha = {
                "nome": pasta.name,
                "arquivos_topo": _conta_arquivos_topo(pasta),
                "walk": _conta_arquivos_recursivo(walk_dir) if walk_dir.is_dir() else None,
                "anims": _conta_arquivos_recursivo(anims_dir) if anims_dir.is_dir() else None,
                "total": _conta_arquivos_recursivo(pasta),
                "dimensoes": _formata_dimensoes(_dimensoes_recursivo(pasta, cobertura_imgs)),
            }
            linhas.append(linha)
            cobertura_pastas.registra_sucesso()
        except Exception as exc:  # permissao negada, symlink quebrado, etc.
            cobertura_pastas.registra_falha(f"{_rel_ou_abs(pasta)}: {exc}")

    cobertura_pastas.fecha_ou_lanca()
    cobertura_imgs.fecha_ou_lanca()
    return linhas, cobertura_pastas, cobertura_imgs


def _fmt_presenca(valor: Optional[int]) -> str:
    if valor is None:
        return "não"
    return f"sim ({valor})"


def monta_bloco(linhas: list[dict], cobertura_pastas: Cobertura, cobertura_imgs: Cobertura, agora: str, sprites_root_rel: str) -> str:
    cabecalho = (
        f"### Inventário automático por pasta (gerado por ferramenta, não editar à mão)\n\n"
        f"Gerado por `python3 tools/art/sprites_inventory_gen.py` em **{agora}** "
        f"(America/Recife), lendo `{sprites_root_rel}` no disco. Cobre só quatro fatos "
        f"mecânicos por pasta de nível 1: contagem de arquivos no topo, presença e "
        f"tamanho de `walk/` e de `anims/` (busca só no nível 1 de cada pasta, como o "
        f"resto deste documento), e as dimensões distintas de imagem encontradas "
        f"recursivamente. O que cada pasta *significa* (quem é personagem, quem é "
        f"pasta especial, o que está pendente de geração) não é fato mecânico e "
        f"continua na PARTE DE JULGAMENTO, escrito à mão.\n\n"
        f"Varredura: {cobertura_pastas.linha_relatorio()}. {cobertura_imgs.linha_relatorio()}.\n\n"
    )
    tabela = [
        "| pasta | arquivos no topo | `walk/` | `anims/` | total recursivo | dimensões distintas (recursivo) |",
        "|---|---|---|---|---|---|",
    ]
    for linha in linhas:
        tabela.append(
            f"| `{linha['nome']}` | {linha['arquivos_topo']} | {_fmt_presenca(linha['walk'])} | "
            f"{_fmt_presenca(linha['anims'])} | {linha['total']} | {linha['dimensoes']} |"
        )
    return cabecalho + "\n".join(tabela) + "\n"


def aplica_no_documento(doc_texto: str, bloco_novo: str) -> tuple[str, bool]:
    """Devolve (texto novo, mudou?). Substitui entre os marcadores se existirem;
    senao insere um par novo logo antes do "---" que precede a PARTE DE
    JULGAMENTO.

    "Mudou" compara o bloco ignorando o instante de geracao (ver
    _normaliza_para_comparacao): se so o relogio andou e o que descreve o
    disco e igual, o documento no disco e mantido tal como esta -- inclusive
    com o instante antigo -- em vez de regravar so para trocar o timestamp."""
    bloco_marcado = f"{MARCADOR_INICIO}\n\n{bloco_novo}\n{MARCADOR_FIM}"

    if MARCADOR_INICIO in doc_texto:
        if MARCADOR_FIM not in doc_texto:
            raise ErroDeUso(
                "documento tem o marcador de INICIO sem o de FIM -- "
                "arquivo corrompido a mao, corrija antes de regenerar"
            )
        inicio = doc_texto.index(MARCADOR_INICIO)
        fim = doc_texto.index(MARCADOR_FIM) + len(MARCADOR_FIM)
        if fim <= inicio:
            raise ErroDeUso("marcador de FIM aparece antes do de INICIO -- arquivo corrompido a mao")
        bloco_atual = doc_texto[inicio:fim]
        if _normaliza_para_comparacao(bloco_atual) == _normaliza_para_comparacao(bloco_marcado):
            return doc_texto, False
        texto_novo = doc_texto[:inicio] + bloco_marcado + doc_texto[fim:]
        return texto_novo, True

    if ANCORA_JULGAMENTO not in doc_texto:
        raise ErroDeUso(
            f'primeira execucao: nao achei nem o marcador nem o cabecalho ancora "{ANCORA_JULGAMENTO}" '
            "-- documento nao tem a forma esperada, nao vou adivinhar onde inserir"
        )

    pos_ancora = doc_texto.index(ANCORA_JULGAMENTO)
    antes = doc_texto[:pos_ancora]
    depois = doc_texto[pos_ancora:]

    linhas_antes = antes.splitlines(keepends=True)
    idx_separador = None
    for i in range(len(linhas_antes) - 1, -1, -1):
        if linhas_antes[i].strip() == "---":
            idx_separador = i
            break
    if idx_separador is None:
        raise ErroDeUso(
            'primeira execucao: achei "## PARTE DE JULGAMENTO" mas nenhum separador "---" antes dele '
            "-- documento nao tem a forma esperada, nao vou adivinhar onde inserir"
        )

    antes_do_separador = "".join(linhas_antes[:idx_separador])
    separador_em_diante = "".join(linhas_antes[idx_separador:])
    texto_novo = antes_do_separador + bloco_marcado + "\n\n" + separador_em_diante + depois
    return texto_novo, True


def main(argv: Optional[list[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--sprites-root", type=Path, default=DEFAULT_SPRITES_ROOT)
    parser.add_argument("--doc", type=Path, default=DEFAULT_DOC_PATH)
    parser.add_argument("--check", action="store_true", help="nao grava; sai 1 se o bloco estiver desatualizado")
    parser.add_argument(
        "--permite-vazio",
        action="store_true",
        help="nao reprova quando a arvore de sprites nao tem nenhuma pasta de nivel 1 "
        "(por padrao, varredura vazia e tratada como varredura quebrada, GODS_LAWS.md L-36 global)",
    )
    args = parser.parse_args(argv)

    if Image is None:
        print("ERRO: Pillow (PIL) nao esta disponivel neste Python -- nao executado, falta a biblioteca (L-28/L-51: nao instalar sem autorizacao).", file=sys.stderr)
        return 2

    try:
        if not args.doc.is_file():
            raise ErroDeUso(f"documento nao encontrado: {args.doc}")
        doc_texto = args.doc.read_text(encoding="utf-8")

        linhas, cobertura_pastas, cobertura_imgs = inventaria(args.sprites_root)
        if cobertura_pastas.encontrados == 0 and not args.permite_vazio:
            raise ErroDeUso(
                f"varredura vazia: nenhuma pasta de nivel 1 encontrada em {args.sprites_root} -- "
                "zero e sinal de varredura quebrada, nao de disco limpo (GODS_LAWS.md L-36 global). "
                "Se vazio e mesmo o estado esperado, repita com --permite-vazio."
            )
        agora = _dt.datetime.now().strftime("%d/%m/%Y, %H:%M")
        try:
            sprites_root_rel = str(args.sprites_root.resolve().relative_to(REPO_ROOT))
        except ValueError:
            sprites_root_rel = str(args.sprites_root)
        bloco_novo = monta_bloco(linhas, cobertura_pastas, cobertura_imgs, agora, sprites_root_rel)
        texto_novo, mudou = aplica_no_documento(doc_texto, bloco_novo)
    except ErroDeUso as exc:
        print(f"ERRO: {exc}", file=sys.stderr)
        return 2

    print(cobertura_pastas.linha_relatorio())
    print(cobertura_imgs.linha_relatorio())
    for falha in cobertura_pastas.falhas:
        print(f"  falha (pasta): {falha}", file=sys.stderr)
    for falha in cobertura_imgs.falhas:
        print(f"  falha (imagem): {falha}", file=sys.stderr)

    if args.check:
        if mudou:
            print(f"DESATUALIZADO: {args.doc} nao bate com o disco em resources/sprites/.")
            codigo = 1
        else:
            print(f"em dia: {args.doc} bate com o disco.")
            codigo = 0
    else:
        if mudou:
            args.doc.write_text(texto_novo, encoding="utf-8")
            print(f"gravado: {args.doc}")
        else:
            print(f"sem mudanca: {args.doc} ja estava em dia.")
        codigo = 0

    # Falha de leitura (pasta ilegivel, imagem corrompida/truncada) e um problema
    # do DISCO, distinto de o documento estar desatualizado -- a falha ja fica
    # visivel no texto (Varredura: ... falharam) e nas linhas "falha (...)" acima,
    # mas isso sozinho nunca mudava o codigo de saida, e quem automatiza nao le
    # texto. Sai 3 sempre que houver falha, mesmo com --check "em dia" ou com o
    # documento gravado com sucesso.
    if cobertura_pastas.falhas or cobertura_imgs.falhas:
        total_falhas = len(cobertura_pastas.falhas) + len(cobertura_imgs.falhas)
        print(f"FALHA DE LEITURA: {total_falhas} falha(s) na varredura (ver acima) -- disco com problema proprio.", file=sys.stderr)
        return 3

    return codigo


if __name__ == "__main__":
    sys.exit(main())
