#!/usr/bin/env python3
"""test_production_scope.py - prova que o GATE(production-scope) MORDE.

Fecha os FUROS 1 e 4 medidos pela auditoria dos gates (GATES-HARDEN,
2026-08-06):

  FURO 1 - `GusEngine/app/main.cpp` (a UNICA fonte do executavel) nao estava em
  NENHUM gate de camada, porque a lista so apontava pra DIRETORIOS. Testes:
  `test_main_cpp_esta_no_escopo_real` (o de hoje) e
  `test_arquivo_solto_sem_cobertura_reprova` (o mecanismo, em arvore de brinquedo).

  FURO 4 - a lista de camadas e ESCRITA A MAO: uma 5a camada nasceria fora dela
  e os 3 gates zero-tolerancia diriam "OK" sem nunca ter olhado pra ela. Teste:
  `test_camada_nova_sem_cobertura_reprova`.

Cada teste de reprova tem seu par de CRESCIMENTO LEGITIMO (arquivo novo DENTRO
de src/, subdir novo dentro de src/): gate que nao pode nascer vermelho tambem
nao pode barrar trabalho legitimo.

Roda 100% em tmp_path - nenhum outro agente na mesma arvore e afetado.

Uso: `python3 -m pytest tools/tests/test_production_scope.py -q`
"""

import os
import sys

import pytest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import production_scope as scope  # noqa: E402


def _write(path: str, content: str = "// vazio\n") -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)


def _fake_engine(tmp_path, layers=("core", "domain", "platform", "app"),
                 loose=()):
    """Monta uma arvore de brinquedo com N camadas e devolve (engine, dirs, files)."""
    engine = str(tmp_path / "GusEngine")
    dirs = []
    for layer in layers:
        _write(os.path.join(engine, layer, "src", "a.cpp"))
        _write(os.path.join(engine, layer, "include", "a.hpp"))
        dirs.append(os.path.join(engine, layer, "src"))
        dirs.append(os.path.join(engine, layer, "include"))
    files = []
    for rel in loose:
        _write(os.path.join(engine, rel))
        files.append(os.path.join(engine, rel))
    return engine, dirs, files


def _patch(monkeypatch, tmp_path, engine, dirs, files):
    monkeypatch.setattr(scope, "ROOT", str(tmp_path))
    monkeypatch.setattr(scope, "ENGINE", engine)
    monkeypatch.setattr(scope, "ALL_PRODUCTION_DIRS", dirs)
    monkeypatch.setattr(scope, "ALL_PRODUCTION_FILES", files)


# ------------------------------------------------- FURO 4: camada nao coberta


def test_camada_nova_sem_cobertura_reprova(tmp_path, monkeypatch):
    """A 5a camada existe no disco e NAO esta na lista escrita a mao.

    ANTES desta fatia nao havia auditoria nenhuma: os 3 gates zero-tolerancia
    varriam as 4 camadas conhecidas e imprimiam OK, sem nunca olhar pra 5a."""
    engine, dirs, files = _fake_engine(tmp_path)
    # 5a camada no disco, ausente de `dirs` (= ausente de ALL_PRODUCTION_DIRS).
    _write(os.path.join(engine, "netcode", "src", "socket.cpp"))
    _patch(monkeypatch, tmp_path, engine, dirs, files)

    problems = scope.audit_scope()
    assert any("netcode" in p for p in problems), problems
    assert scope.main() == 1


def test_include_de_camada_sem_cobertura_reprova(tmp_path, monkeypatch):
    engine, dirs, files = _fake_engine(tmp_path)
    dirs = [d for d in dirs if not d.endswith(os.path.join("app", "include"))]
    _patch(monkeypatch, tmp_path, engine, dirs, files)

    problems = scope.audit_scope()
    assert any("include de camada NAO coberto" in p for p in problems), problems
    assert scope.main() == 1


def test_include_coberto_por_subdir_mais_fundo_passa(tmp_path, monkeypatch):
    """CRESCIMENTO LEGITIMO: `app/include/gus/app` (o caso REAL do repo) cobre
    `app/include`. O gate nao pode exigir a raiz do include na lista."""
    engine, dirs, files = _fake_engine(tmp_path)
    dirs = [d for d in dirs if not d.endswith(os.path.join("app", "include"))]
    fundo = os.path.join(engine, "app", "include", "gus", "app")
    _write(os.path.join(fundo, "tela.hpp"))
    dirs.append(fundo)
    _patch(monkeypatch, tmp_path, engine, dirs, files)

    assert scope.audit_scope() == []
    assert scope.main() == 0


# --------------------------------------------- FURO 1: arquivo solto na raiz


def test_arquivo_solto_sem_cobertura_reprova(tmp_path, monkeypatch):
    """O mecanismo do furo do `main.cpp`: .cpp na RAIZ da camada, fora de src/."""
    engine, dirs, files = _fake_engine(tmp_path)
    _write(os.path.join(engine, "app", "main.cpp"), "#include <SDL3/SDL.h>\n")
    _patch(monkeypatch, tmp_path, engine, dirs, files)  # files = [] (o estado ANTIGO)

    problems = scope.audit_scope()
    assert any("main.cpp" in p for p in problems), problems
    assert scope.main() == 1


def test_arquivo_solto_declarado_passa(tmp_path, monkeypatch):
    engine, dirs, files = _fake_engine(tmp_path, loose=("app/main.cpp",))
    _patch(monkeypatch, tmp_path, engine, dirs, files)

    assert scope.audit_scope() == []
    assert scope.main() == 0
    assert os.path.join(engine, "app", "main.cpp") in list(
        scope.iter_production_files()
    ), "arquivo solto declarado tem de ser VARRIDO, nao so aceito pela auditoria"


def test_entrada_morta_em_production_files_reprova(tmp_path, monkeypatch):
    """Arraste de excecao: o arquivo saiu do disco e a lista nao encolheu."""
    engine, dirs, _ = _fake_engine(tmp_path)
    _patch(monkeypatch, tmp_path, engine, dirs,
           [os.path.join(engine, "app", "sumiu.cpp")])

    problems = scope.audit_scope()
    assert any("desatualizada" in p for p in problems), problems
    assert scope.main() == 1


# ------------------------------------------------------- crescimento legitimo


def test_arquivo_novo_dentro_de_src_nao_reprova(tmp_path, monkeypatch):
    """O caso comum do dia a dia: fatia nova acrescenta .cpp em src/ (inclusive
    em subdiretorio novo). Isso NAO pode barrar."""
    engine, dirs, files = _fake_engine(tmp_path)
    _write(os.path.join(engine, "domain", "src", "combat", "novo.cpp"))
    _write(os.path.join(engine, "domain", "include", "gus", "novo.hpp"))
    _patch(monkeypatch, tmp_path, engine, dirs, files)

    assert scope.audit_scope() == []
    assert scope.main() == 0


def test_escopo_colapsado_nao_imprime_ok_falso(tmp_path, monkeypatch):
    """Sentinela: arvore sem nenhuma camada NAO pode virar 'OK, 0 problemas'."""
    engine = str(tmp_path / "GusEngine")
    os.makedirs(engine, exist_ok=True)
    _patch(monkeypatch, tmp_path, engine, [], [])

    assert scope.audit_scope() != []
    assert scope.main() == 1


# ------------------------------------------------------ o repo REAL, de hoje


def test_main_cpp_esta_no_escopo_real():
    """FURO 1 no repo de verdade: `GusEngine/app/main.cpp` tem de ser varrido.

    Este e o teste que ficava VERMELHO antes desta fatia - o arquivo nao estava
    em nenhum dir de ALL_PRODUCTION_DIRS e ALL_PRODUCTION_FILES nao existia."""
    varridos = {os.path.abspath(p) for p in scope.iter_production_files()}
    main_cpp = os.path.join(scope.ENGINE, "app", "main.cpp")
    assert os.path.isfile(main_cpp)
    assert os.path.abspath(main_cpp) in varridos


def test_repo_real_nasce_verde():
    """Requisito (b) da casa: o gate nasce VERDE na arvore corrigida."""
    assert scope.audit_scope() == []


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-v"]))
