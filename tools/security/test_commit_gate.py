# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Testes do gate de commit-msg (tools/security/commit_gate.py).

GODS_LAWS.md L-19 (TDD estrito: este arquivo nasceu ANTES da implementacao,
e a primeira execucao dele falhou por ausencia do modulo -- ver a saida
colada no relatorio de execucao) e restricao (d) da ordem de servico
("falso positivo tem custo real... prove com teste que nao casa dentro de
palavra").

Roda com stdlib puro (unittest), sem pytest, para nao depender de instalar
nada em nenhuma das cinco plataformas (GODS_LAWS.md L-28).

Uso: python3 tools/security/test_commit_gate.py -v
"""
import stat
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import commit_gate  # noqa: E402  (import depois do sys.path, deliberado)


TERMOS_FIXTURE = """\
# arquivo de termos de teste, nao e o real do projeto
## SEGREDO_JOGO
easter egg
pigpen

## SPOILER_ENREDO
carta de pyotor
ainda compila

## DADOS_PESSOAIS
site_consultorio
crm
nome-fictício-de-teste-menor

## TECNICO_MAQUINA
/home/petrus
drpetrus.top
"""


def escrever_termos(diretorio: Path, conteudo: str = TERMOS_FIXTURE) -> Path:
    caminho = diretorio / "termos_proibidos.txt"
    caminho.write_text(conteudo, encoding="utf-8")
    return caminho


class TestNormalizacao(unittest.TestCase):
    def test_remove_acento_e_caixa(self):
        self.assertEqual(commit_gate.normalize("AINDA Compila"), "ainda compila")
        self.assertEqual(commit_gate.normalize("Prontuário"), "prontuario")
        self.assertEqual(commit_gate.normalize("47ª proposição"), "47a proposicao")

    def test_colapsa_espaco_e_quebra_de_linha(self):
        # Lição registrada (2026-07-27): frase quebrada entre linhas engana
        # busca literal; o gate tem que normalizar quebras de linha em espaço
        # antes de comparar.
        self.assertEqual(
            commit_gate.normalize("ainda\ncompila"), "ainda compila"
        )


class TestCarregarTermos(unittest.TestCase):
    def test_carrega_classes_e_termos(self):
        with tempfile.TemporaryDirectory() as tmp:
            caminho = escrever_termos(Path(tmp))
            termos = commit_gate.carregar_termos(caminho)
            classes = {c for c, _ in termos}
            self.assertEqual(
                classes,
                {
                    "SEGREDO_JOGO",
                    "SPOILER_ENREDO",
                    "DADOS_PESSOAIS",
                    "TECNICO_MAQUINA",
                },
            )
            self.assertIn(("SEGREDO_JOGO", "pigpen"), termos)

    def test_arquivo_ausente_falha(self):
        with tempfile.TemporaryDirectory() as tmp:
            caminho = Path(tmp) / "nao-existe.txt"
            with self.assertRaises(commit_gate.TermosIndisponiveis):
                commit_gate.carregar_termos(caminho)

    def test_arquivo_ainda_cifrado_falha(self):
        # Simula o estado real de um arquivo git-crypt sem a chave carregada:
        # o blob comeca com o cabecalho binario GITCRYPT.
        with tempfile.TemporaryDirectory() as tmp:
            caminho = Path(tmp) / "termos_proibidos.txt"
            caminho.write_bytes(b"\x00GITCRYPT\x00" + b"\x01\x02\x03lixo-binario")
            with self.assertRaises(commit_gate.TermosIndisponiveis):
                commit_gate.carregar_termos(caminho)

    def test_arquivo_vazio_falha(self):
        with tempfile.TemporaryDirectory() as tmp:
            caminho = Path(tmp) / "termos_proibidos.txt"
            caminho.write_text("   \n\n", encoding="utf-8")
            with self.assertRaises(commit_gate.TermosIndisponiveis):
                commit_gate.carregar_termos(caminho)

    def test_termo_fora_de_secao_falha(self):
        with tempfile.TemporaryDirectory() as tmp:
            caminho = Path(tmp) / "termos_proibidos.txt"
            caminho.write_text("termo-orfao-sem-classe\n", encoding="utf-8")
            with self.assertRaises(commit_gate.TermosIndisponiveis):
                commit_gate.carregar_termos(caminho)


class TestDeteccao(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)
        caminho = escrever_termos(Path(self._tmp.name))
        termos = commit_gate.carregar_termos(caminho)
        self.padroes = commit_gate.construir_padroes(termos)

    def test_mensagem_limpa_passa(self):
        msg = "corrige o calculo de dano do combate por turno"
        self.assertEqual(commit_gate.encontrar_violacoes(msg, self.padroes), [])

    def test_barra_termo_classe_segredo_jogo(self):
        msg = "documenta o easter egg pro changelog"
        achados = commit_gate.encontrar_violacoes(msg, self.padroes)
        self.assertTrue(any(c == "SEGREDO_JOGO" for c, _ in achados))

    def test_barra_termo_classe_spoiler_enredo(self):
        msg = "ajusta timing da cena carta de pyotor"
        achados = commit_gate.encontrar_violacoes(msg, self.padroes)
        self.assertTrue(any(c == "SPOILER_ENREDO" for c, _ in achados))

    def test_barra_termo_classe_dados_pessoais(self):
        msg = "corrige script copiado sem querer do site_consultorio"
        achados = commit_gate.encontrar_violacoes(msg, self.padroes)
        self.assertTrue(any(c == "DADOS_PESSOAIS" for c, _ in achados))

    def test_barra_termo_classe_tecnico_maquina(self):
        msg = "remove caminho hardcoded /home/petrus do script de build"
        achados = commit_gate.encontrar_violacoes(msg, self.padroes)
        self.assertTrue(any(c == "TECNICO_MAQUINA" for c, _ in achados))

    def test_barra_ignorando_acento_e_caixa(self):
        msg = "corrige DRPETRUS.TOP no manifest de exemplo"
        achados = commit_gate.encontrar_violacoes(msg, self.padroes)
        self.assertTrue(any(c == "TECNICO_MAQUINA" for c, _ in achados))

    def test_nao_barra_termo_dentro_de_palavra_comum_real(self):
        # "crm" (classe DADOS_PESSOAIS) embutido dentro de uma palavra
        # portuguesa plausivel formada com o aumentativo "-zinho"; mesma
        # familia de erro do "grep -i 'eu '" que casava dentro de "seu"/
        # "meu", e do "rml" raspado de dentro de "Stormlight"/"firmly".
        msg = "cria consultoriozinho de teste pra fixture da suite"
        achados = commit_gate.encontrar_violacoes(msg, self.padroes)
        self.assertEqual(achados, [], f"falso positivo: {achados}")

    def test_nao_barra_termo_dentro_de_palavra_comum_sintetico(self):
        # Prova adicional, sintetica: "crm" dentro de um identificador maior
        # nao pode disparar o gate.
        msg = "renomeia variavel microcrmzao para nome_descritivo"
        achados = commit_gate.encontrar_violacoes(msg, self.padroes)
        self.assertEqual(achados, [], f"falso positivo: {achados}")

    def test_nao_barra_palavra_comum_do_jogo_que_nao_esta_na_lista(self):
        # Confirma que termos genericos de vocabulario legitimo do jogo
        # (Pillar 4: economia do hospital) nao estao bloqueados -- nao
        # aparecem na fixture de teste nem devem aparecer na lista real
        # (ver docs/security/commit-gate.md).
        msg = "balanceia custo de cura no hospital e ajusta paciente-teste"
        self.assertEqual(commit_gate.encontrar_violacoes(msg, self.padroes), [])


class TestMainFimAFim(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)
        self.tmp_path = Path(self._tmp.name)
        self.termos_path = escrever_termos(self.tmp_path)

    def _escrever_msg(self, texto: str) -> Path:
        caminho = self.tmp_path / "COMMIT_EDITMSG"
        caminho.write_text(texto, encoding="utf-8")
        return caminho

    def test_main_aprova_mensagem_limpa(self):
        msg_path = self._escrever_msg("corrige teste de combate\n")
        codigo = commit_gate.main(
            ["commit_gate.py", str(msg_path)], termos_path=self.termos_path, env={}
        )
        self.assertEqual(codigo, 0)

    def test_main_bloqueia_mensagem_com_termo_proibido(self):
        msg_path = self._escrever_msg("expõe o pigpen no changelog\n")
        codigo = commit_gate.main(
            ["commit_gate.py", str(msg_path)], termos_path=self.termos_path, env={}
        )
        self.assertNotEqual(codigo, 0)

    def test_main_falha_fechado_sem_chave_do_git_crypt(self):
        cifrado = self.tmp_path / "termos_cifrado.txt"
        cifrado.write_bytes(b"\x00GITCRYPT\x00\x01\x02\x03")
        msg_path = self._escrever_msg("mensagem qualquer\n")
        codigo = commit_gate.main(
            ["commit_gate.py", str(msg_path)], termos_path=cifrado, env={}
        )
        self.assertNotEqual(
            codigo, 0, "gate aprovou em silencio sem conseguir medir (viola L-19)"
        )

    def test_main_escape_hatch_explicito_libera_mesmo_bloqueado(self):
        msg_path = self._escrever_msg("expõe o pigpen no changelog\n")
        codigo = commit_gate.main(
            ["commit_gate.py", str(msg_path)],
            termos_path=self.termos_path,
            env={commit_gate.OVERRIDE_ENV: "1"},
        )
        self.assertEqual(codigo, 0)

    def test_main_escape_hatch_libera_mesmo_sem_conseguir_medir(self):
        cifrado = self.tmp_path / "termos_cifrado.txt"
        cifrado.write_bytes(b"\x00GITCRYPT\x00\x01\x02\x03")
        msg_path = self._escrever_msg("mensagem qualquer\n")
        codigo = commit_gate.main(
            ["commit_gate.py", str(msg_path)],
            termos_path=cifrado,
            env={commit_gate.OVERRIDE_ENV: "1"},
        )
        self.assertEqual(codigo, 0)


if __name__ == "__main__":
    unittest.main(verbosity=2)
