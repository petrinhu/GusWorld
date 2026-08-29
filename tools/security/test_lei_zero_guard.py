# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Testes do gate --staged/--range do lei-zero-guard (tools/security/lei_zero_guard.py).

Nasceu de um defeito medido em CI (run 33226855294): o passo de regressao
passou imprimindo ZERO linhas -- um log de varredura bem-sucedida e um log
de varredura que quebrou e nao olhou arquivo nenhum eram identicos. Prova
tres coisas, na ordem da ordem de servico:

  1. MORDE: violacao plantada no diff -> exit 2.
  2. PASSA AUDIVEL: diff limpo -> exit 0 E a saida contem a contagem de
     arquivos varridos (nao passa em silencio).
  3. NAO FALHA ABERTA: `git diff` falha (SHA invalido no --range) -> exit 2,
     com a mensagem nomeando a falha do git -- nunca um passe silencioso
     disfarcado de "diff vazio, nada a varrer".

Usa repositorios git de verdade em diretorio temporario (nao mocka
subprocess) porque o proprio comportamento do `git diff` diante de SHA
invalido e o fato sob teste. Roda com stdlib puro (unittest), sem pytest
(GODS_LAWS.md L-28: nao instala nada em nenhuma das cinco plataformas).

Uso: python3 tools/security/test_lei_zero_guard.py -v
"""
from __future__ import annotations
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

GUARD = Path(__file__).resolve().parent / "lei_zero_guard.py"


def git(cwd: Path, *args: str) -> subprocess.CompletedProcess:
    return subprocess.run(
        ["git", *args], cwd=cwd, capture_output=True, text=True, check=True
    )


def repo_git_novo(cwd: Path) -> None:
    git(cwd, "init", "-q")
    git(cwd, "config", "user.email", "teste@example.invalid")
    git(cwd, "config", "user.name", "Teste Lei Zero")


def commitar(cwd: Path, nome: str, conteudo: str) -> None:
    (cwd / nome).write_text(conteudo, encoding="utf-8")
    git(cwd, "add", "--", nome)
    git(cwd, "commit", "-q", "-m", f"commit de fixture: {nome}")


def rodar_guard(cwd: Path, *args: str) -> subprocess.CompletedProcess:
    return subprocess.run(
        [sys.executable, str(GUARD), *args],
        cwd=cwd,
        capture_output=True,
        text=True,
    )


class TestGateStagedMorde(unittest.TestCase):
    def test_violacao_plantada_reprova(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo = Path(tmp)
            repo_git_novo(repo)
            commitar(repo, "base.txt", "arquivo base, sem nada de errado\n")
            # planta a violacao: SDL e proibido (PROIBIDOS em lei_zero_guard.py)
            (repo / "engine.txt").write_text(
                "linkamos contra a SDL para abrir a janela\n", encoding="utf-8"
            )
            git(repo, "add", "--", "engine.txt")

            resultado = rodar_guard(repo, "--staged")

            self.assertEqual(
                resultado.returncode, 2,
                f"deveria reprovar; stdout={resultado.stdout!r} "
                f"stderr={resultado.stderr!r}",
            )
            self.assertIn("SDL", resultado.stdout + resultado.stderr)
            self.assertIn("BLOQUEADO", resultado.stdout + resultado.stderr)


class TestGatePassaAudivel(unittest.TestCase):
    def test_diff_limpo_aprova_e_declara_escopo(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo = Path(tmp)
            repo_git_novo(repo)
            commitar(repo, "base.txt", "arquivo base, sem nada de errado\n")
            (repo / "regra.txt").write_text(
                "regra de dano do combate por turno\n", encoding="utf-8"
            )
            git(repo, "add", "--", "regra.txt")

            resultado = rodar_guard(repo, "--staged")

            self.assertEqual(
                resultado.returncode, 0,
                f"deveria aprovar; stdout={resultado.stdout!r} "
                f"stderr={resultado.stderr!r}",
            )
            saida = resultado.stdout + resultado.stderr
            self.assertTrue(
                saida.strip(),
                "passou em silencio -- um portao mudo e indistinguivel de um "
                "portao quebrado que nao varreu nada",
            )
            self.assertIn("lei-zero-guard", saida)
            self.assertIn("1", saida)  # 1 arquivo do diff varrido
            self.assertIn("0", saida)  # 0 violacoes

    def test_range_limpo_tambem_declara_escopo(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo = Path(tmp)
            repo_git_novo(repo)
            commitar(repo, "base.txt", "linha um\n")
            base_sha = git(repo, "rev-parse", "HEAD").stdout.strip()
            commitar(repo, "seguinte.txt", "regra de cura no hospital\n")
            head_sha = git(repo, "rev-parse", "HEAD").stdout.strip()

            resultado = rodar_guard(repo, "--range", f"{base_sha} {head_sha}")

            self.assertEqual(
                resultado.returncode, 0,
                f"deveria aprovar; stdout={resultado.stdout!r} "
                f"stderr={resultado.stderr!r}",
            )
            saida = resultado.stdout + resultado.stderr
            self.assertTrue(saida.strip(), "range limpo passou em silencio")
            self.assertIn("lei-zero-guard", saida)


class TestGateNaoFalhaAberta(unittest.TestCase):
    def test_range_com_sha_invalido_reprova_nomeando_a_falha_do_git(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo = Path(tmp)
            repo_git_novo(repo)
            commitar(repo, "base.txt", "linha um\n")
            head_sha = git(repo, "rev-parse", "HEAD").stdout.strip()

            resultado = rodar_guard(
                repo, "--range", f"sha-que-nao-existe-nem-de-longe {head_sha}"
            )

            self.assertEqual(
                resultado.returncode, 2,
                f"SHA invalido tem de reprovar (fail-secure), nunca passar; "
                f"stdout={resultado.stdout!r} stderr={resultado.stderr!r}",
            )
            saida = resultado.stdout + resultado.stderr
            self.assertIn("git", saida.lower())
            self.assertIn(
                "diff", saida.lower(),
                "mensagem tem que nomear o comando git que falhou",
            )
            # nao pode ser o passe silencioso disfarcado de "0 violacoes"
            self.assertNotIn("0 violacao", saida)

    def test_staged_em_diretorio_sem_repo_git_reprova(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo = Path(tmp)  # deliberadamente SEM git init

            resultado = rodar_guard(repo, "--staged")

            self.assertEqual(
                resultado.returncode, 2,
                f"sem repo git, o git falha -- gate tem de reprovar, nao "
                f"tratar como diff vazio; stdout={resultado.stdout!r} "
                f"stderr={resultado.stderr!r}",
            )
            saida = resultado.stdout + resultado.stderr
            self.assertNotIn("0 violacao", saida)


if __name__ == "__main__":
    unittest.main(verbosity=2)
