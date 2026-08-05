#!/usr/bin/env python3
"""callback_dtor_disarm.py - GATE(callback-dtor-disarm) do tools/check.sh.

ZERO-TOLERANCIA (nao ratchet). Exige que TODA tela que registra um callback da
familia de evento do glintfx::UiLayer DESARME esse callback como PRIMEIRA
instrucao do proprio destrutor (`set_*_callback(nullptr)`).

IRMAO, NAO SUBSTITUTO, do tools/callback_dtor_order.py
------------------------------------------------------
As duas defesas se somam:
  - CALLBACK-DTOR-ORDER  (gate irmao): `ui_` declarado por ultimo, logo destruido
    primeiro. Estrutural, mas fragil socialmente - depende de o comentario ser
    lido, e a tabela daquele gate so conhece os membros de HOJE: um membro NOVO
    declarado DEPOIS de `ui_` passa batido (fragilidade MEDIDA a mao, nao
    hipotetica - o gate segue verde nesse caso).
  - CALLBACK-DTOR-DESARME (este gate): desarme explicito no destrutor. Local e
    INSENSIVEL a ordem de membro, entao cobre justamente o furo acima.
Nenhum dos dois torna o outro dispensavel. Este gate NAO afrouxa aquele.

POR QUE PRECISA DE GATE (mesmo motivo do irmao)
-----------------------------------------------
Remover o desarme NAO deixa nenhum teste de runtime vermelho: as telas vivem em
namespace anonimo dentro do .cpp, e no caminho normal `exit()` ja fez
`ui_.reset()` antes do destrutor. O unico caminho em que o desarme AGE e uma
excecao escapando de enter()/tick()/handle_event() - o `exit()` esta solto na
ultima linha de run_screen_state() (GusEngine/app/src/screen_state.cpp), FORA de
guard RAII -, e o codigo de hoje nao lanca. Sem gate, o conserto some em silencio
na proxima refatoracao.

(O que TEM teste de runtime e a PREMISSA: GusEngine/app/tests/
callback_disarm_contract_test.cpp prova, com GL real, que passar nullptr de fato
desarma no glintfx do pin - o alarme para um bump futuro quebrar a semantica.)

DUAS DISCIPLINAS (as mesmas do tools/spdx_header_required.py)
-------------------------------------------------------------
 1. ENUMERA, NAO BUSCA. O gate nao confia numa lista escrita a mao de "quem
    registra callback": ele DESCOBRE os registros varrendo os fontes RASTREADOS
    (`git ls-files`, nunca os.walk - trabalho untracked em voo nao entra) e exige
    que o conjunto descoberto BATA com a tabela SITES. Tela NOVA que registre
    hover/clique reprova ate entrar aqui com o desarme; tela da tabela que pare de
    registrar tambem reprova (tabela obsoleta). Uma familia de callback nao
    classificada (nem relevante, nem irrelevante-com-motivo) reprova pedindo
    classificacao - desconhecido nunca passa calado.
 2. DECLARA O QUE VARRE. A saida diz quantos arquivos varreu e quantos registros
    achou. "OK" so pode significar "medi estes N", nunca "nao achei onde procurei".
"""

import os
import re
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(SCRIPT_DIR)

# Extensoes fechadas por construcao - cada uma vira um pathspec do git ls-files.
EXTENSIONS = ("cpp", "hpp")

# Escopo: PRODUCAO da camada app/ - `app/src` + `app/include/gus/app`, excluindo
# `app/tests/` e `app/tools/`. MESMO recorte (e mesma redacao na saida) do gate
# tools/sdl_layer_ratchet.py, pra nao existirem duas nocoes de "producao" na casa.
# app/ e a unica camada que pode tocar glintfx (o GATE de camadas proibe em
# core/domain/), entao o escopo esta fechado por construcao.
#
# POR QUE tests/ e tools/ ficam de fora, e o que isso custa: o conserto e sobre o
# DESTRUTOR DE TELA - um objeto de pilha que pode ser desenrolado por excecao com o
# callback ainda armado. Um teste ou um probe que registra callback a mao no corpo
# de uma funcao nao tem destrutor de tela pra gate nenhum medir (o
# callback_disarm_contract_test.cpp desta mesma fatia REGISTRA e DESARMA de
# proposito - e o objeto do teste, nao um sitio a consertar). CUSTO ACEITO E
# DECLARADO: uma ferramenta em app/tools/ que registre hover/clique numa classe de
# vida longa nao seria pega aqui - mesmo recorte que o gate irmao ja assumiu, com o
# mesmo motivo medido (ver "FORA DE ESCOPO" no docstring de callback_dtor_order.py).
PRODUCTION_PREFIXES = ("GusEngine/app/src", "GusEngine/app/include/gus/app")
SCOPE_LABEL = "app/src + app/include/gus/app, excluindo app/tests/ e app/tools/"

# Familias de callback do glintfx que PODEM disparar durante o teardown do
# UiLayer e portanto EXIGEM desarme. Sao os canais servidos por listener do RmlUi
# anexado ao documento: descarregar o documento (o que ~UiLayer() faz) recalcula a
# hover chain e pode emitir um ultimo evento. Enumeradas do header publico pinado
# (glintfx/include/glintfx/ui_layer.hpp, v0.30.0) - nao inventadas.
TEARDOWN_RELEVANT = {
    "click": "ClickEventListener no documento; dispara no teardown do RmlUi.",
    "click_info": "irmao do click (ClickInfo), mesmo listener de documento.",
    "scroll": "ScrollEventListener no documento.",
    "change": "FORMEV: listener de Change no documento.",
    "submit": "FORMEV: listener de Submit no documento.",
    "focus": "FORMEV: listener de Focus no documento.",
    "blur": "FORMEV: listener de Blur no documento.",
    "hover": "FORMEV: HoverEventListener (Mouseover) no documento.",
    "cursor": ("SystemClock::SetMouseCursor - foi EXATAMENTE este que deu "
               "heap-use-after-free ao vivo no glintfx (AUD-PUB-6g-TEARDOWN). "
               "O ~UiLayer::Impl() deles ja desarma, mas o consumidor desarmar "
               "tambem nao depende de versao."),
}

# Familias classificadas como IRRELEVANTES pro teardown, cada uma com o motivo
# MEDIDO (nao presumido). Fora desta lista e da de cima, o gate reprova pedindo
# classificacao - "classificacao errada nao se corrige medindo".
TEARDOWN_IRRELEVANT = {
    "frame": ("glintfx::App::set_frame_callback: `frame_cb` tem call site UNICO "
              "em Impl::render_frame() (glintfx src/app.cpp), alcancavel so por "
              "render()/snapshot(), nunca pelo teardown (App::~App() e "
              "= default). Medido na fatia CALLBACK-DTOR-ORDER."),
}

# Tabela dos sitios: onde esta o destrutor e o que ele TEM de fazer primeiro.
# A familia exigida NAO e escrita duas vezes - e derivada do proprio literal de
# desarme (regex abaixo) e cruzada com o que a descoberta achou no arquivo.
SITES = {
    "GusEngine/app/src/screens/title_menu_loop.cpp": {
        "dtor": "~TitleScreen() override {",
        "disarm": ["if (ui_) ui_->set_hover_callback(nullptr);"],
    },
    "GusEngine/app/src/screens/difficulty_menu_loop.cpp": {
        "dtor": "~DifficultyScreen() override {",
        "disarm": ["if (ui_) ui_->set_hover_callback(nullptr);"],
    },
    "GusEngine/app/src/screens/system_menu_loop.cpp": {
        "dtor": "~SystemMenuLoopScreen() override {",
        "disarm": ["if (ui_) ui_->set_hover_callback(nullptr);"],
    },
    "GusEngine/app/src/screens/battle_preview.cpp": {
        "dtor": "~BattleScreen() override {",
        "disarm": ["if (ui_) ui_->set_click_callback(nullptr);"],
    },
}

# Chamada REAL de registro/desarme: precisa do `->`/`.` na frente (uma mencao em
# prosa - "ver UiLayer::set_click_callback" - nao casa). \s* entre as pecas cobre
# a chamada quebrada em duas linhas pelo clang-format.
_CALL_RE = re.compile(r"(?:->|\.)\s*set_([a-z_]+?)_callback\s*\(")
# Comentario de linha inteira (o unico caso que precisa ser descartado aqui: uma
# chamada de verdade nunca comeca a linha com // ou *).
_COMMENT_RE = re.compile(r"^\s*(//|\*|/\*)")


def _tracked_sources():
    """Fontes RASTREADOS no escopo, via git ls-files (nunca os.walk)."""
    pathspecs = [f"{pref}/**/*.{ext}"
                 for pref in PRODUCTION_PREFIXES for ext in EXTENSIONS]
    out = subprocess.run(
        ["git", "-C", ROOT, "ls-files", "--"] + pathspecs,
        capture_output=True, text=True, check=True,
    ).stdout
    return sorted(p for p in out.splitlines() if p)


def _code_text(lines):
    """Texto do arquivo sem as linhas que sao puro comentario."""
    return "\n".join("" if _COMMENT_RE.match(ln) else ln for ln in lines)


def _families_called(lines):
    """Familias de callback com chamada REAL neste arquivo."""
    return set(_CALL_RE.findall(_code_text(lines)))


def _read(rel_path):
    with open(os.path.join(ROOT, rel_path), "r", encoding="utf-8") as fh:
        return fh.read().splitlines()


def _dtor_body_head(lines, dtor_literal, n):
    """As primeiras `n` linhas de CODIGO do corpo do destrutor (ou None).

    Pula linhas vazias e comentarios - o comentario grande que explica o desarme
    fica ANTES do destrutor, mas tolerar comentario dentro do corpo evita que o
    gate reprove por motivo cosmetico.
    """
    idx = [i for i, ln in enumerate(lines) if ln.strip() == dtor_literal]
    if len(idx) != 1:
        return None, len(idx)
    head = []
    for ln in lines[idx[0] + 1:]:
        s = ln.strip()
        if not s or _COMMENT_RE.match(ln):
            continue
        head.append(s)
        if len(head) == n:
            break
    return head, 1


def check_site(rel_path, spec, discovered):
    errors = []
    if not os.path.isfile(os.path.join(ROOT, rel_path)):
        return [f"{rel_path}: arquivo NAO existe (tabela SITES desatualizada?)."]
    lines = _read(rel_path)

    # --- cruzamento descoberta x tabela (a parte que impede o gate de mentir) --
    expected = {m.group(1) for d in spec["disarm"]
                for m in [_CALL_RE.search(d)] if m}
    called = discovered.get(rel_path, set()) & set(TEARDOWN_RELEVANT)
    if called != expected:
        errors.append(
            f"{rel_path}: as familias de callback REGISTRADAS no arquivo "
            f"({sorted(called) or 'nenhuma'}) nao batem com as DESARMADAS na "
            f"tabela SITES ({sorted(expected) or 'nenhuma'}). Toda familia "
            "registrada tem de ser desarmada no destrutor - atualize o destrutor "
            "E a tabela no MESMO commit (CALLBACK-DTOR-DESARME)."
        )
        return errors

    # --- o desarme e a PRIMEIRA instrucao do destrutor? -----------------------
    head, n_dtor = _dtor_body_head(lines, spec["dtor"], len(spec["disarm"]))
    if head is None:
        errors.append(
            f"{rel_path}: a linha `{spec['dtor']}` foi encontrada {n_dtor}x "
            "(esperado: exatamente 1). Ou o destrutor sumiu (o desarme foi "
            "apagado), ou a classe/assinatura mudou e a tabela SITES ficou "
            "desatualizada - atualize-a no MESMO commit."
        )
        return errors
    if head != spec["disarm"]:
        errors.append(
            f"{rel_path}: as primeiras instrucoes de `{spec['dtor']}` sao "
            f"{head}, e o esperado e {spec['disarm']}. O desarme tem de ser a "
            "PRIMEIRA coisa do destrutor - qualquer trabalho de teardown antes "
            "dele roda com o callback ainda ARMADO, que e exatamente o que este "
            "conserto evita."
        )
    return errors


def main() -> int:
    sources = _tracked_sources()
    discovered = {}
    unknown = []
    for rel in sources:
        fams = _families_called(_read(rel))
        for fam in sorted(fams):
            if fam not in TEARDOWN_RELEVANT and fam not in TEARDOWN_IRRELEVANT:
                unknown.append((rel, fam))
        relevant = fams & set(TEARDOWN_RELEVANT)
        if relevant:
            discovered[rel] = relevant

    errors = []
    for rel, fam in unknown:
        errors.append(
            f"{rel}: `set_{fam}_callback` NAO esta classificado. Verifique se "
            "ele pode disparar durante o teardown do UiLayer (leia o call site no "
            "glintfx pinado) e adicione-o a TEARDOWN_RELEVANT (exige desarme) ou "
            "a TEARDOWN_IRRELEVANT (com o motivo MEDIDO) em "
            "tools/callback_dtor_disarm.py. Desconhecido nao passa calado."
        )

    # Descoberto e fora da tabela = tela nova sem desarme.
    for rel in sorted(set(discovered) - set(SITES)):
        errors.append(
            f"{rel}: registra {sorted(discovered[rel])} no glintfx::UiLayer mas "
            "NAO esta na tabela SITES de tools/callback_dtor_disarm.py. Toda tela "
            "que registra callback tem de desarma-lo no destrutor "
            "(CALLBACK-DTOR-DESARME) - some o destrutor e a entrada na tabela."
        )
    # Na tabela e sem registro = tabela obsoleta (o gate mediria o nada).
    for rel in sorted(set(SITES) - set(discovered)):
        errors.append(
            f"{rel}: esta na tabela SITES mas NENHUM registro de callback do "
            "UiLayer foi descoberto nele. Se a tela parou de registrar, remova a "
            "entrada; se o registro mudou de forma, ajuste o gate - uma entrada "
            "que nao mede nada e pior que entrada nenhuma."
        )

    for rel, spec in sorted(SITES.items()):
        if rel in discovered:
            errors.extend(check_site(rel, spec, discovered))

    if errors:
        print("GATE(callback-dtor-disarm): FALHA - o desarme do callback no "
              "destrutor (2a linha de defesa do teardown do UiLayer) foi "
              "quebrado:")
        for err in errors:
            print(f"    - {err}")
        print("  Racional completo: o comentario ATENCAO - CALLBACK-DTOR-DESARME "
              "no destrutor de cada tela, e o docstring deste script.")
        return 1

    n_reg = sum(len(v) for v in discovered.values())
    print(f"GATE(callback-dtor-disarm): OK (varri {len(sources)} fonte(s) "
          f"rastreado(s), escopo: {SCOPE_LABEL}; achei {n_reg} registro(s) de "
          f"callback de teardown em {len(discovered)} arquivo(s), todos com "
          "desarme como 1a instrucao do destrutor; "
          f"{len(TEARDOWN_IRRELEVANT)} familia(s) classificada(s) como "
          "irrelevante(s) com motivo medido).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
