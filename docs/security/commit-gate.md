<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Gate de commit-msg contra vazamento de segredo

Este documento **não cita nenhum termo da lista protegida**. Ele explica o
mecanismo, não o conteúdo.

## O problema que isto resolve

`git-crypt` cifra o **conteúdo** de arquivo marcado em `.gitattributes`. Ele
não cifra o **nome** do arquivo, o **nome do diretório**, nem a **mensagem
de commit**. Uma mensagem de commit pode citar em texto puro, para sempre,
num histórico de repositório público, algo que o próprio conteúdo cifrado
existe para proteger.

O `gitleaks` (GODS_LAWS.md L-19, quarto portão de qualidade) cobre padrão de
credencial. Ele não cobre nome de personagem secreto, rótulo que expõe o
mecanismo de um segredo de design do jogo, spoiler de enredo ou dado
pessoal do líder — essa é a lacuna que este gate cobre.

## Como funciona

1. **Gancho `commit-msg`**, local ao repositório, roda **antes** do commit
   ser criado. Se ele recusa (código de saída diferente de zero), o commit
   não acontece — a mensagem problemática nunca chega a existir como
   objeto git.
2. O gancho (`tools/git-hooks/commit-msg`) é um delegador fino em `/bin/sh`
   que só localiza a raiz do repositório e chama o script de verdade,
   `tools/security/commit_gate.py`, em Python puro. A lógica inteira
   (normalização, comparação, decisão de bloquear) mora no `.py`, para
   rodar idêntica nas cinco plataformas da matriz de CI (GODS_LAWS.md
   L-09, L-20) sem depender de utilitário de shell específico de sistema.
3. O script lê `tools/security/termos_proibidos.txt` — a lista de termos
   proibidos, dividida em **quatro classes**:
   - segredo de mecanismo de jogo (rótulo, não o elemento velado em si);
   - spoiler estrutural de enredo (cena e frase exclusiva de material
     ainda não publicado);
   - dado pessoal e de saúde do líder e da família, fora do escopo do
     jogo, e outros projetos pessoais dele sem relação com o GusWorld;
   - caminho de máquina e identificador técnico.
4. Esse arquivo de termos está **cifrado com git-crypt**, com a mesma
   chave do resto do repositório (GODS_LAWS.md L-25). Em texto puro, a
   lista **seria** o vazamento — bastaria lê-la para saber, de uma vez,
   tudo que o projeto trata como segredo. Cifrar a lista, e não só os
   documentos que ela protege, é a decisão do líder que fecha essa
   lacuna.
5. Antes de comparar, o script **normaliza** mensagem e termo: remove
   acento, ignora maiúscula/minúscula, e colapsa toda sequência de espaço
   (inclusive quebra de linha) num espaço só — para uma frase que o git
   ou o editor quebrem em duas linhas continuar detectável.
6. A comparação usa **fronteira de palavra nos dois lados do termo
   inteiro**, nunca substring solta. Termo curto embutido dentro de uma
   palavra maior (o mesmo tipo de erro já registrado nesta casa: busca
   solta casando dentro de palavra comum) **não** dispara o gate. Isto é
   testado, não só afirmado — ver `tools/security/test_commit_gate.py`.

## Por que a lista não bloqueia palavra genérica do vocabulário do jogo

Ao montar a lista, medimos a ocorrência de cada candidato a termo contra o
corpus real de `docs/` antes de fechar a lista. Palavra genérica de
vocabulário legítimo e recorrente de mecânica de jogo (um pilar de design
inteiro gira em torno de um cenário que usa esse vocabulário no sentido
comum, sem relação nenhuma com o que este gate protege) foi
**deliberadamente excluída**, porque bloqueá-la travaria toda mensagem de
commit que tocasse esse pilar, sem proteger nada de fato secreto. Pelo
mesmo motivo, nome de entidade/lugar/personagem já estabelecido como canon
público (presente em fartura em `CHARS.md`, `sinopse.md` e dezenas de
documentos de design) também não entra: bloquear vocabulário de
desenvolvimento do dia a dia não é o objetivo do gate, e gera exatamente o
tipo de fadiga de alarme que teria feito o líder desligar a proteção.

Ao acrescentar termo novo à lista, aplique o mesmo critério: **o termo
tem uso legítimo e frequente fora do segredo que ele protege?** Se sim,
prefira uma frase mais específica (várias palavras, exclusiva do contexto
secreto) a uma palavra solta.

## Escape hatch para o líder

O gate nunca deve ser contornado por um `--no-verify` silencioso — isso
some com o rastro de que algo foi pulado. O escape é **explícito e
visível**: a variável de ambiente

```
GUSWORLD_COMMIT_GATE_OVERRIDE=1
```

definida antes do `git commit` libera o commit **mesmo que o gate tenha
encontrado termo proibido**, ou mesmo que ele **não tenha conseguido
medir** (lista indisponível). Em qualquer um dos dois casos, o gate
imprime no terminal exatamente o que teria bloqueado (ou por que não
conseguiu verificar) antes de liberar — a visibilidade não desaparece só
porque o commit foi adiante.

Isto não dispensa a regra separada de autorização de `push`/tag/merge em
`main`: o override só afeta o commit local.

## Falha explícita quando não consegue medir

Se a chave do `git-crypt` não estiver destravada nesta cópia de trabalho
(clone novo, ou chave nunca importada), o arquivo de termos aparece como
blob cifrado ilegível. O gate **não** interpreta isso como "lista vazia,
nada a bloquear" — ele **recusa o commit** com uma mensagem que diz
exatamente isso: não é prova de que a mensagem está limpa, é prova de que
ninguém checou. Mesma lógica se o arquivo estiver ausente, vazio, ou
corrompido de outra forma. Isto é a aplicação direta de GODS_LAWS.md L-19:
"gate que não consegue medir tem de FALHAR, nunca aprovar em silêncio".

## Como isto convive com o `core.hooksPath` compartilhado

Este repositório herda `core.hooksPath` de uma configuração **global**
(`~/.claude/githooks`), compartilhada com outros projetos do líder. Essa
pasta global já tem um mecanismo de repasse (`_chain.sh`) desenhado
exatamente para este cenário: cada shim global (`commit-msg`,
`pre-commit`, etc.) primeiro roda a rotina própria daquela pasta e depois
delega para `$(git rev-parse --git-common-dir)/hooks/<nome>` **se esse
arquivo local existir e for executável** — ou seja, para o hook próprio
deste repositório, dentro do seu `.git/hooks/`.

**Isto significa que o gate deste projeto não precisa tocar em nada
compartilhado.** Ele só precisa existir em `.git/hooks/commit-msg` deste
repositório, e o encadeamento já embutido no `hooksPath` global cuida do
resto. Verificado antes de implementar: `~/.claude/githooks/commit-msg`
já era um shim de repasse puro (sem lógica própria) quando este gate foi
construído — não havia nada para sobrescrever ou quebrar.

**O que este trabalho NÃO fez, de propósito:** não alterou nenhum arquivo
dentro de `~/.claude/githooks/`, não mudou `core.hooksPath` (nem local nem
global), e não presume nada sobre outro projeto que compartilhe essa
pasta.

### Instalação

`.git/hooks/` não é versionado pelo git (é metadado local do clone). Por
isso o hook "de verdade" vive versionado em `tools/git-hooks/commit-msg`,
e é copiado (nunca symlink, para funcionar igual no Windows) para
`.git/hooks/commit-msg` por:

```
python3 tools/git-hooks/install.py
```

Rodar isso uma vez por clone (ou de novo, a qualquer momento, para
reinstalar depois de editar o hook fonte — o instalador é idempotente).

## Degradação conhecida em cada plataforma

- **Linux e macOS:** funciona diretamente; `/bin/sh` e `python3` são
  esperados no `PATH`.
- **Windows (Git for Windows):** o hook sem extensão roda através do
  `sh` embutido no Git for Windows (comportamento padrão de qualquer
  instalação normal, que traz Git Bash/MSYS2 junto). Se essa instalação
  não tiver `python3` nem `python` acessível de dentro desse shell, o
  script imprime um erro explícito pedindo para instalar Python
  (GODS_LAWS.md L-28: nada se instala sem perguntar ao líder) — mas só
  imprime esse erro se o próprio `sh` conseguir rodar; um Git instalado
  sem esse shell embutido (incomum, mas possível em instalação mínima)
  simplesmente não dispara o hook nenhum, sem aviso, porque nesse caso é
  o próprio git que não consegue executar o arquivo. **Isto ainda não foi
  testado em CI Windows real** — pendência a confirmar na matriz de CI
  (GODS_LAWS.md L-20).
- **CI (todas as cinco plataformas):** o CI nunca tem a chave do
  `git-crypt`, e não roda hook de commit (não há commit sendo criado em
  pipeline de build/teste). O gate é, por desenho, uma proteção de
  **tempo de desenvolvimento local**, não um portão de CI. A proteção de
  CI contra segredo em texto puro continua sendo o `gitleaks` (L-19,
  quarto portão) mais a própria cifra do `git-crypt` sobre o conteúdo.

## Como acrescentar um termo

1. Com a chave do `git-crypt` destravada nesta cópia de trabalho, abra
   `tools/security/termos_proibidos.txt`.
2. Acrescente a linha na seção (`## NOME_DA_CLASSE`) certa. Uma linha por
   termo ou frase.
3. Antes de salvar como definitivo, pergunte: **esse termo tem uso
   legítimo e comum fora do que estou protegendo?** Se a resposta for
   sim, prefira uma frase mais longa e específica.
4. `git add tools/security/termos_proibidos.txt` e commit — o filtro do
   `git-crypt` cifra o conteúdo automaticamente no objeto gravado (isto
   já está marcado em `.gitattributes`; não precisa marcar de novo).

### Nota para o líder: nome de batismo do filho

`GODS_LAWS.md` L-16 proíbe versionar o nome civil real do filho do líder
em qualquer lugar do repositório, inclusive como exemplo de teste. Quem
implementou este gate não conhece esse nome e não o inseriu na lista.
Se o líder quiser blindar esse nome contra mensagem de commit, o único
lugar onde ele pode existir é dentro do arquivo cifrado de termos, e só
ele deve adicioná-lo lá.

## Pendência registrada para o líder/orquestrador

O caminho `tools/security/termos_proibidos.txt` é um **quarto caminho de
cifra**, além dos três que `GODS_LAWS.md` L-25 já registra
(`docs/_secret/**`, `docs/narrative/deep/stinger/**`, `docs/book/**`).
A ordem de serviço que originou este gate proibiu explicitamente tocar em
`GODS_LAWS.md`, então essa lei **ainda não reflete** o quarto caminho —
falta ao líder (ou a quem ele delegar) acrescentar a linha correspondente
na tabela do L-25.

## Limitação conhecida, para não prometer o que o gate não entrega

Este gate cobre **mensagem de commit**. Ele não varre o **conteúdo** de
arquivo (isso é papel do `gitleaks` e de revisão humana), não cobre nome
de branch, nem título de pull request. Se o mesmo termo precisar de
proteção nesses outros lugares, é trabalho separado.
