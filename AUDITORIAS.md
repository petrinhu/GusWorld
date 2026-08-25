# Auditorias - GusWorld

> Índice das auditorias deste projeto. **Fonte de verdade acima de tudo: `GODS_LAWS.md`.** Manuais irmãos: `CONTRACT.md`, `TESTES.md`, `TOOLING.md`. Hub: `Standards.md`.

Auditoria é sempre trabalho de C-level `fable` (`internal-auditor`, ou o CTO/CISO/CLO conforme o assunto), nunca da thread principal e nunca de agente genérico (L-10).

---

## Cadência

Dois níveis, os dois obrigatórios:

1. **Portões automáticos**, a cada commit e a cada fatia fechada (L-19).
2. **Dossiê formal** do `internal-auditor`, portão de release antes da 1.0 (L-19), consolidando os capítulos abaixo.

---

## 1. Portões automáticos de qualidade (L-19)

1. **Zero aviso de compilação** em todo commit, com `-Werror` no CI.
2. **ASan e UBSan** a cada fatia fechada, em build separado.
3. **Análise estática:** `clang-tidy` e `cppcheck` no CI.
4. **Scan de segredo:** `gitleaks`. Cobre a árvore por padrão; para dado sensível, a verificação correta é `git log --all -p | grep -ci <termo>`, com `-i`, nunca `git grep`.
5. **Gate de mensagem de commit:** `tools/security/commit_gate.py`, gancho `commit-msg` **local**, instalado por `tools/git-hooks/install.py`. Existe porque o `git-crypt` cifra o **conteúdo** do arquivo e **não** cifra a mensagem de commit, o nome do arquivo nem o do diretório — sem ele, o texto que a L-25 protege sai pela porta que ninguém trancou. Barra quatro classes: termo da lista cifrada, caminho de área cifrada, dado pessoal e credencial. **Sem a chave, reprova o commit** em vez de aprovar em silêncio.

**Como auditar o portão 5, que é diferente dos outros quatro:** os quatro primeiros deixam rastro no CI, então basta ler o log do job. Este é **local**, e um clone onde `install.py` nunca rodou simplesmente não o tem — **CI verde não prova que ele existe**. A auditoria confere três coisas: que `.git/hooks/commit-msg` está presente e executável; que o teste próprio passa (`python3 tools/security/test_commit_gate.py`); e que um `core.hooksPath` global, se houver, **encadeia** de volta ao gancho local em vez de sombreá-lo — foi o que quase aconteceu nesta máquina, e só um teste de execução real mostrou que o encadeamento funcionava.

**Sem meta numérica de cobertura.** Cobertura é consequência do TDD estrito, não alvo de auditoria.

---

## 2. Auditoria de arquitetura (L-17)

- **Gate de CI:** nenhum `#include <glintfx/` fora de `present/`. Violação derruba o build, não depende de disciplina de quem escreve.
- `present/` só existe quando o GlintFx tiver janela, contexto gráfico, entrada e texto (L-06); até lá, a auditoria de camadas cobre `core/`, `content/`, `domain/` e `app/`.
- **Determinismo do replay** como teste automatizado permanente: mesma semente mais mesma lista de comandos reproduz o mesmo estado final, byte a byte.
- Vigiar as cinco armadilhas nomeadas na L-17 (objeto `Game`/`app/` que vira dono de tudo, barramento genérico, `if` de dificuldade no corpo da regra, determinismo quebrado por detalhe de C++, regra vazando para a interface).

---

## 3. Auditoria de plataforma (L-09, L-20)

- Matriz de **cinco entradas** verde: Fedora 44 (pinado, nunca `:latest`), Ubuntu, Arch, CachyOS, Windows.
- Verde num alvo **não** autoriza declarar outro alvo suportado (CachyOS não é coberto pelo job de Arch).

---

## 4. Auditoria de proteção de dado (L-18, L-25)

- Nenhum dado do jogo (save, configuração, mapa, catálogo de conteúdo) em formato de texto na distribuição.
- Envelope binário único com cifra autenticada, validador semântico e gravação atômica com cadeia de backups.
- Configuração selada com selo inválido cai em padrão de fábrica com aviso, **nunca** recusa abrir o jogo.
- Amarra de máquina só no slot Hardcore, nunca no save normal.
- **Primitivas criptográficas só vêm do GlintFx.** Nunca biblioteca de terceiro, nunca primitiva caseira, nunca API do sistema operacional.
- Comunicação pública nunca promete "impossível editar" ou "à prova de trapaça": o que se garante é detecção de adulteração, não prevenção absoluta.

---

## 5. Auditoria de licença (L-08)

- Marcação **REUSE/SPDX** (`SPDX-FileCopyrightText`, `SPDX-License-Identifier`) em todo arquivo fonte, com arquivo companheiro para asset e binário.
- Regime de asset e lore: **todos os direitos reservados**, com o carve-out de marca (nome, logotipo, trade dress, nomes de personagem) e a permissão de conteúdo de fã preservados.
- Catálogo de conteúdo fatiado por natureza jurídica: número e regra são código sob AGPL-3.0-or-later (compilados no executável); texto de sabor e prosa de carta são asset reservado (pacote binário separado).
- Os dois livros-companheiros permanecem fora do regime do jogo, com direitos reservados próprios.

---

## 6. Dossiê formal (pré-1.0)

O `internal-auditor` consolida os cinco capítulos acima num dossiê único antes do release 1.0, com achado, severidade e remediação. Até lá, os portões automáticos da seção 1 cobrem o dia a dia.
