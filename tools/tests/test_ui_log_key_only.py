#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""test_ui_log_key_only.py - prova que o GATE(ui-log-key-only) MORDE.

Um gate so vale se (a) mede de verdade o defeito que motivou sua criacao, (b)
nasce VERDE na arvore corrigida, (c) esta LIGADO no rc do check.sh - e (d) NAO
barra crescimento legitimo. (c) e coberto por test_check_sh_gates_ligados.py
(enumeracao); os outros tres estao aqui.

O defeito medido (I18N-UILOG, 2026-08-06): texto de tela escrito dentro de
`ui_log_.push_back`, fora do tradutor. Cada regra do gate tem aqui o par
VERMELHO (a forma exata do defeito reprova) + VERDE (a forma correta passa):

  R1  literal de string dentro de ui_log_.push_back           -> reprova
      push_back de uma variavel (o helper)                    -> passa
  R2  ZERO call site no escopo (membro renomeado/varredura
      quebrada) -> reprova em vez de imprimir OK sobre o vazio
  R3  chave inexistente no catalogo, chave nao-literal, e
      chave presente em SO UM dos 2 catalogos                 -> reprova
      chave literal presente nos dois                         -> passa

Roda 100% em tmp_path (arvore de brinquedo): nenhum outro agente na mesma
working tree e afetado, e o teste nao depende do conteudo real do jogo.

Uso: `python3 -m pytest tools/tests/test_ui_log_key_only.py -q`
"""

import os
import sys

import pytest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import ui_log_key_only as gate  # noqa: E402

# A forma REAL do call site corrigido em battle_scene.cpp (o push_back que sobrou
# empilha o resultado do translator, nunca um literal).
HELPER_DEF = """\
void BattleScene::push_ui_log(LogLineKind kind, std::string_view key) {
    ui_log_.push_back(LogLine{kind, translator_ != nullptr
                                        ? translator_->tr(std::string(key))
                                        : std::string(key)});
}
"""


def _write(path: str, content: str) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)


def _fake_tree(tmp_path, monkeypatch, sources: dict, pt_keys, en_keys):
    """Monta arvore de brinquedo: 1 dir de producao + os 2 catalogos."""
    src = str(tmp_path / "GusEngine" / "app" / "src")
    for name, content in sources.items():
        _write(os.path.join(src, name), content)
    pt = str(tmp_path / "resources" / "translations" / "pt_br.md")
    en = str(tmp_path / "resources" / "translations" / "en_intl.md")
    _write(pt, "".join(f"## {k}\nvalor de {k}\n\n" for k in pt_keys))
    _write(en, "".join(f"## {k}\n\n" for k in en_keys))
    monkeypatch.setattr(gate, "ROOT", str(tmp_path))
    monkeypatch.setattr(gate, "PRODUCTION_DIRS", [src])
    monkeypatch.setattr(gate, "CATALOGS", [pt, en])
    return src


# ---------------------------------------------------------------- R1

def test_literal_dentro_do_push_back_reprova(tmp_path, monkeypatch):
    """VERMELHO-ANTES: exatamente a forma que existia em battle_scene.cpp."""
    _fake_tree(
        tmp_path, monkeypatch,
        {"battle_scene.cpp": HELPER_DEF + """
void BattleScene::menu_confirm() {
    ui_log_.push_back(LogLine{LogLineKind::System,
                              "COMPILAR: modulo do compilador offline nesta build."});
}
"""},
        pt_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"],
        en_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"])
    assert gate.main() == 1


def test_arvore_corrigida_passa(tmp_path, monkeypatch):
    """VERDE-DEPOIS: o helper + chamadas por chave, como ficou de verdade."""
    _fake_tree(
        tmp_path, monkeypatch,
        {"battle_scene.cpp": HELPER_DEF + """
void BattleScene::menu_confirm() {
    push_ui_log(LogLineKind::System, "COMBAT_LOG_COMPILE_UNAVAILABLE");
}
"""},
        pt_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"],
        en_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"])
    assert gate.main() == 0


def test_literal_em_comentario_NAO_reprova(tmp_path, monkeypatch):
    """Falso positivo que derrubaria a documentacao do proprio conserto: os
    comentarios desta fatia CITAM a frase antiga de proposito."""
    _fake_tree(
        tmp_path, monkeypatch,
        {"battle_scene.cpp": HELPER_DEF + """
// Antes desta fatia a linha era:
//     ui_log_.push_back(LogLine{LogLineKind::System, "COMPILAR: ... offline"});
void BattleScene::menu_confirm() {
    push_ui_log(LogLineKind::System, "COMBAT_LOG_COMPILE_UNAVAILABLE");
}
"""},
        pt_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"],
        en_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"])
    assert gate.main() == 0


# ---------------------------------------------------------------- R2

def test_zero_call_site_reprova(tmp_path, monkeypatch):
    """SENTINELA DE ESCOPO: sem nenhum `ui_log_.push_back` no escopo, o gate
    deixou de medir alguma coisa - tem de gritar, nao imprimir OK sobre o
    vazio (o modo de falha mais covarde de um gate)."""
    _fake_tree(
        tmp_path, monkeypatch,
        {"outra_tela.cpp": "void f() { /* nada a ver com log de UI */ }\n"},
        pt_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"],
        en_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"])
    assert gate.main() == 1


# ---------------------------------------------------------------- R3

def test_chave_inexistente_no_catalogo_reprova(tmp_path, monkeypatch):
    """Chave fantasma cairia pra sempre no fallback (a tela mostraria a CHAVE)."""
    _fake_tree(
        tmp_path, monkeypatch,
        {"battle_scene.cpp": HELPER_DEF + """
void BattleScene::menu_confirm() {
    push_ui_log(LogLineKind::System, "COMBAT_LOG_CHAVE_QUE_NAO_EXISTE");
}
"""},
        pt_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"],
        en_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"])
    assert gate.main() == 1


def test_chave_so_em_um_catalogo_reprova(tmp_path, monkeypatch):
    """Paridade: a chave tem de nascer nos DOIS locales."""
    _fake_tree(
        tmp_path, monkeypatch,
        {"battle_scene.cpp": HELPER_DEF + """
void BattleScene::menu_confirm() {
    push_ui_log(LogLineKind::System, "COMBAT_LOG_COMPILE_UNAVAILABLE");
}
"""},
        pt_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"],
        en_keys=[])
    assert gate.main() == 1


def test_texto_solto_no_lugar_da_chave_reprova(tmp_path, monkeypatch):
    """O FURO OBVIO de R1, fechado por R3: sem esta regra, bastaria escrever o
    texto de tela como se fosse a chave pra o literal voltar por dentro do
    helper, e o gate diria OK."""
    _fake_tree(
        tmp_path, monkeypatch,
        {"battle_scene.cpp": HELPER_DEF + """
void BattleScene::menu_confirm() {
    push_ui_log(LogLineKind::System, "COMPILAR: modulo do compilador offline");
}
"""},
        pt_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"],
        en_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"])
    assert gate.main() == 1


def test_chave_nao_literal_reprova(tmp_path, monkeypatch):
    """Chave vinda de variavel deixa de ser auditavel estaticamente - que e todo
    o ponto do gate."""
    _fake_tree(
        tmp_path, monkeypatch,
        {"battle_scene.cpp": HELPER_DEF + """
void BattleScene::menu_confirm() {
    const std::string k = escolhe_chave();
    push_ui_log(LogLineKind::System, k);
}
"""},
        pt_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"],
        en_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"])
    assert gate.main() == 1


# ---------------------------------------------------------------- crescimento legitimo

def test_call_site_novo_com_chave_valida_passa(tmp_path, monkeypatch):
    """Um gate que so sabe reprovar barra trabalho honesto: mensagem NOVA de log
    de UI, com chave nova nos 2 catalogos, tem de passar sem cerimonia."""
    _fake_tree(
        tmp_path, monkeypatch,
        {"battle_scene.cpp": HELPER_DEF + """
void BattleScene::menu_confirm() {
    push_ui_log(LogLineKind::System, "COMBAT_LOG_COMPILE_UNAVAILABLE");
    push_ui_log(LogLineKind::Status, "COMBAT_LOG_MENSAGEM_NOVISSIMA");
}
"""},
        pt_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE", "COMBAT_LOG_MENSAGEM_NOVISSIMA"],
        en_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE", "COMBAT_LOG_MENSAGEM_NOVISSIMA"])
    assert gate.main() == 0


def test_catalogo_ilegivel_reprova(tmp_path, monkeypatch):
    """Sem catalogo nao ha como validar chave nenhuma: reprova em vez de
    aprovar tudo por vacuidade (`key not in set()` seria sempre falso... e
    `set()` vazio faria TODA chave parecer ausente - nos dois sentidos o
    resultado seria ruido, entao a falta do insumo tem de ser explicita)."""
    src = _fake_tree(
        tmp_path, monkeypatch,
        {"battle_scene.cpp": HELPER_DEF},
        pt_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"],
        en_keys=["COMBAT_LOG_COMPILE_UNAVAILABLE"])
    assert os.path.isdir(src)
    monkeypatch.setattr(gate, "CATALOGS",
                        [str(tmp_path / "resources" / "translations" / "nao_existe.md")])
    assert gate.main() == 1


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-v"]))
