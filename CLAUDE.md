> **LEI ZERO, ACIMA DE TUDO: O GUSWORLD ASSENTA SÓ EM GLINTFX E NO SISTEMA OPERACIONAL.** Nunca raciocine sobre este projeto como se ele pudesse ter janela própria, contexto gráfico próprio, laço de quadro próprio ou workaround por cima do framework. Janela, laço principal, entrada, áudio e desenho são do **GlintFx**; o GusWorld escreve **só a lógica do jogo**. Se uma função ainda não existe no GlintFx, o pedido vai para o bus e o trabalho espera parado — nunca se contorna a fronteira. Ver LEI ZERO e L-05 em `GODS_LAWS.md`.

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## LEIS CANÔNICAS: leia `GODS_LAWS.md` ANTES de agir

**[`GODS_LAWS.md`](GODS_LAWS.md) contém as ordens expressas do líder e tem precedência sobre este arquivo, sobre os manuais e sobre qualquer preferência sua.** Ele não existe para ser declarado, existe para ser **usado no momento da ação**.

**Como usar, sem exceção:**

1. Antes do primeiro comando de qualquer tarefa, confira se um dos gatilhos abaixo casa com o que você vai fazer. Casou: **abra `GODS_LAWS.md` e leia a lei inteira antes de agir**, não depois.
2. Ao despachar subagent, **cole no prompt da task** o texto das leis cujo gatilho casa com ela, mais o caminho absoluto de `GODS_LAWS.md`. Subagent não herda este contexto.
3. Ao relatar ao líder, diga qual lei aplicou e como.
4. Ordem nova do líder entra em `GODS_LAWS.md` **no instante em que ele a dá**, com data e o texto dele verbatim.
5. Agente nenhum revoga, flexibiliza ou reinterpreta lei. Só o líder.

## Índice de gatilhos

Transcrito de `GODS_LAWS.md` (32 leis, medido em 24/08/2026 por `grep -c "^## L-" GODS_LAWS.md`). Este índice é cópia; a fonte de verdade é sempre o arquivo, nunca esta tabela.

| Lei | Gatilho: dispara quando você vai... | Resumo |
|---|---|---|
| [L-01](GODS_LAWS.md#l-01) | procurar código anterior, "como era antes", ou base para reaproveitar | O código é do zero; o GusWorld anterior não é base |
| [L-02](GODS_LAWS.md#l-02) | tocar arte, câmera, sprite, modelo ou pipeline visual | Jogo é 2D pixel-art; o pipeline 3D é do líder e agente não toca |
| [L-03](GODS_LAWS.md#l-03) | criar build, escolher padrão de linguagem | C++23 |
| [L-04](GODS_LAWS.md#l-04) | escrever função, arquivo, classe, módulo, item ou carta | Proibido monolito; cada elemento do jogo é átomo com POCO próprio |
| [L-05](GODS_LAWS.md#l-05) | precisar de algo que o GlintFx ainda não tem | Proibido dublê de plataforma: ou liga no GlintFx, ou não existe |
| [L-06](GODS_LAWS.md#l-06) | decidir o que construir primeiro | Núcleo puro agora; a camada que desenha só nasce com o GlintFx |
| [L-07](GODS_LAWS.md#l-07) | precisar de função do framework, ou receber ideia do Gus | Bus: pedido só quando o jogo esbarrar de verdade na falta |
| [L-08](GODS_LAWS.md#l-08) | criar `LICENSE`, cabeçalho de arquivo, versionar asset ou publicar | AGPL-3.0-or-later no código; assets em três zonas; livros reservados |
| [L-09](GODS_LAWS.md#l-09) | escrever CI ou declarar suporte de plataforma | Cinco alvos; Fedora 44 primário; CachyOS é próprio, não Arch |
| [L-10](GODS_LAWS.md#l-10) | ir executar qualquer trabalho de produto | Main só orquestra; C-level fable audita e cria; sonnet implementa |
| [L-11](GODS_LAWS.md#l-11) | escolher entre duas abordagens, ou notar qualquer dúvida | Nada é decidido por agente; opções ao líder via `AskUserQuestion` |
| [L-12](GODS_LAWS.md#l-12) | escrever qualquer mensagem ao líder | Timestamp real `[DD/MM/YY - HH:MM:SS]` obtido do `date` |
| [L-13](GODS_LAWS.md#l-13) | tocar em qualquer trabalho que dependa de canon | Canon desatualizado bloqueia: atualiza primeiro, trabalha depois |
| [L-14](GODS_LAWS.md#l-14) | concluir que algo está morto, obsoleto ou descartável | Nada é declarado morto por agente; a decisão é do líder |
| [L-15](GODS_LAWS.md#l-15) | versionar binário, asset pesado ou configurar o repositório | `livros/` e `glb/` fora do git; LFS no resto |
| [L-16](GODS_LAWS.md#l-16) | homenagear pessoa real, ou citar o filho do líder | Homenagem só com aceite; o filho aparece só como "Gus Dragon" |
| [L-17](GODS_LAWS.md#l-17) | criar módulo, arquivo, ou desenhar a forma de um sistema | Espinha de cinco camadas com gate de CI; regra como comando e evento |
| [L-18](GODS_LAWS.md#l-18) | escolher formato de arquivo de mapa, save, configuração ou item | Nada em formato de texto; máxima proteção contra edição |
| [L-19](GODS_LAWS.md#l-19) | commitar, fechar fatia, ou pensar em cobertura e formatação | Cinco portões de qualidade; TDD estrito; sem meta de cobertura |
| [L-20](GODS_LAWS.md#l-20) | escrever CI, ou declarar que uma plataforma funciona | Matriz das cinco plataformas desde o primeiro commit |
| [L-21](GODS_LAWS.md#l-21) | achar que o `inicial.md` cumpriu o papel dele | Só se apaga com as quatro condições satisfeitas e o líder confirmando |
| [L-22](GODS_LAWS.md#l-22) | nomear qualquer coisa, ou escrever comentário e commit | Identificador e comentário em inglês, `snake_case`; commit em pt-br |
| [L-23](GODS_LAWS.md#l-23) | criar tag, publicar release, ou mexer na versão | Versão e tag são `vA.B.C.D`, como no GlintFx |
| [L-24](GODS_LAWS.md#l-24) | ver regra, seção ou documento que o líder revogou | Revogado se APAGA; não se guarda como histórico |
| [L-25](GODS_LAWS.md#l-25) | tocar save, configuração, mapa ou catálogo de conteúdo | Envelope binário selado, teto técnico faseado, cripto vem do GlintFx |
| [L-26](GODS_LAWS.md#l-26) | tocar câmera, mapa, locomoção ou direção de sprite | 3/4 top-down fixa, quatro direções cardeais, grade quadrada |
| [L-27](GODS_LAWS.md#l-27) | pensar em escrever tela, HUD ou marcação de interface | Nenhuma interface se escreve antes de o GlintFx traduzir marcação |
| [L-28](GODS_LAWS.md#l-28) | precisar baixar, instalar ou atualizar qualquer coisa | Pergunte ao líder; nunca falhe calado; `sudo` sempre com `-A` |
| [L-29](GODS_LAWS.md#l-29) | propor recurso, sistema ou escopo novo | Os 16 cortes: a cerca do que o jogo NÃO é |
| [L-30](GODS_LAWS.md#l-30) | escrever, reordenar ou acrescentar item na tabela | Todo item aponta para o documento que o especifica, se existir |
| [L-31](GODS_LAWS.md#l-31) | o líder aprovar, rejeitar ou mudar algo, ou fechar item de alta prioridade | Avisar o Gus Dragon sem ele perguntar |
| [L-32](GODS_LAWS.md#l-32) | fechar uma fatia, fechar uma onda, ou pensar em `git push` | Commit por fatia; push só com verificação automática e testes verdes |

## O que é o GusWorld

Jogo **2D pixel-art**, single-player e offline, em **C++23**, assentado sobre o framework [GlintFx](../GlintFx) (LEI ZERO): o GusWorld não cria janela, não possui contexto gráfico e não roda laço próprio de quadro — isso é do GlintFx, e o GusWorld escreve só a lógica do jogo.

- **Perspectiva:** 3/4 top-down fixa, quatro direções cardeais desenhadas à mão (sem espelhamento), grade quadrada (L-26).
- **Estrutura de mundo:** hub central mais incursões radiais; sem mundo aberto, sem mundo persistente (corte C-02).
- **Progressão de cartas:** obtidas por progresso narrativo, nunca craftadas (corte C-13).
- **Escopo da campanha:** 4 a 8 horas de campanha principal, mais cerca de 2 horas de puzzle opcional (corte C-15).
- **Distribuição:** FOSS, código sob AGPL-3.0-or-later; assets e lore com todos os direitos reservados (L-08).

A cerca completa do que o jogo **não** é (16 cortes, `C-01` a `C-16`) está na L-29.

## Estado atual do repositório (24/08/2026)

Todo número abaixo foi medido nesta data, com o comando indicado; nada veio de memória.

**Git e remoto:**

- `git rev-list --count HEAD`: **16 commits** no branch local `main`.
- `git ls-remote git@github.com:petrinhu/GusWorld.git main`: `1d5051fe120844bf55132cd8b7c5baca9145ddd4`.
- `git rev-parse HEAD` bate com o SHA acima e `git log origin/main..HEAD --oneline` não lista nada: **local e remoto estão sincronizados**, sem commit pendente de push.
- Não há workflow de CI: `find . -path "*/.github/workflows*"` devolve vazio. **Não existe verificação automática configurada ainda** — a L-32 (push só com o GHA verde) ainda não tem o que checar.

**Código-fonte: não existe.** Confirmado por comando, não por suposição:

- `find . -iname CMakeLists.txt` → vazio.
- `find . -maxdepth 3 -iname "*.hpp" -o -maxdepth 3 -iname "*.cpp"` → vazio.
- `find . -maxdepth 2 -iname tests -o -maxdepth 2 -iname test` → vazio, não existe `tests/`.
- Não há `include/` nem `src/`.

**O que existe hoje, fora de código de jogo** (contado por `find`/`ls`):

- **23 arquivos `.md` na raiz** (`ls *.md | wc -l`, inclui este `CLAUDE.md`): os manuais listados em "Autoridade documental" abaixo, mais `GODS_LAWS.md`, `TODO.md`, `inicial.md`, `README.md`, `sinopse.md`, `CHARS.md`, `PLACES.md`.
- `LICENSE` (AGPL-3.0), `NOTICE`, `REUSE.toml`, `.gitattributes`, `.gitignore`, `.bigtech-porte` (`porte=bigtech`), `LICENSES/` (textos integrais SPDX).
- `docs/`: **12 diretórios de primeiro nível** (`art`, `audio`, `book`, `design`, `narrative`, `_processo`, `production`, `qa`, `_secret`, `security`, `specs`, `tech`; medido por `find docs -maxdepth 1 -mindepth 1 -type d | wc -l` — sem o `-mindepth 1` o comando conta o próprio `docs/` e devolve 13) mais `docs/licoes-aprendidas.md` na raiz de `docs/`. `docs/_secret/` é área cifrada por `git-crypt` (L-25) — só o caminho é citável, nunca o conteúdo.
- `docs/tech/adr/`: **20 ADRs** (`ls docs/tech/adr | wc -l`), numerados `001` a `021` com o **`010` ausente** — foi apagado por ordem do líder junto com o modelo de arquitetura que ele registrava (LEI ZERO, L-24).
- `tools/`: o portão 5 da L-19 (`tools/security/commit_gate.py`, `tools/security/test_commit_gate.py`, `tools/security/termos_proibidos.txt` cifrado, `tools/git-hooks/commit-msg`, `tools/git-hooks/install.py`).
- `resources/` e `assets/`: existem (`ls` confirma), não inventariados aqui — fora do escopo desta tarefa.

## Decisões fechadas pelo líder

Estas foram tomadas explicitamente via `AskUserQuestion` (salvo onde indicado) e estão detalhadas em `GODS_LAWS.md`. Mudança em qualquer uma delas é decisão do líder, não de agente.

| Eixo | Decisão |
|---|---|
| Natureza | Jogo 2D pixel-art, single-player, offline — executável final, não biblioteca |
| Linguagem e build | C++23; única dependência de framework é o GlintFx (LEI ZERO) |
| Perspectiva e câmera | 3/4 top-down fixa, quatro direções cardeais desenhadas à mão, sem espelhamento, grade quadrada (L-26) |
| Estrutura de mundo | Hub central mais incursões radiais; sem mundo aberto nem persistente (C-02) |
| Plataformas | Fedora 44 (primário, pinado), Ubuntu, Arch, CachyOS (próprio, não é Arch renomeado), Windows — cinco entradas distintas de CI desde o primeiro commit (L-09, L-20) |
| Arquitetura | Espinha de cinco camadas (`core/ → content/ → domain/ → app/ → present/`), dependência só para baixo, com gate de CI; regra como transição determinística por comando e evento (L-17) |
| Persistência de dado | Nenhum formato de texto: save, configuração, mapa e catálogo de conteúdo em envelope binário único selado (L-18, L-25) |
| Licença do código | AGPL-3.0-or-later, com ressalva explicando o caráter offline do jogo (L-08) |
| Licença de assets e lore | Todos os direitos reservados, com carve-out de marca e permissão de conteúdo de fã não comercial (L-08) |
| Titular do copyright | Petrus Alves da Silva Costa (L-08) |
| Porte do projeto (bigtech) | **COMPLETO** — pipeline de 12 fases e toda a constelação C-level ativos, nenhuma fase dormente por presunção de agente (L-10) |
| Ordem de construção | Núcleo de regra de jogo em TDD estrito agora; a camada `present/` só nasce quando o GlintFx tiver janela, contexto gráfico, entrada e texto (L-06) |

## Autoridade documental

Os manuais na raiz são normativos e vencem qualquer preferência do agente, **e perdem para `GODS_LAWS.md`** em caso de conflito. **Leia o manual relevante ANTES de decidir**, não depois. `Standards.md` é o hub que indexa os manuais de processo; a tabela abaixo estende esse índice aos manuais de licença e de canon citados neste arquivo.

| Manual | Quando é obrigatório ler |
|---|---|
| `CONTRACT.md` | Antes de escrever, modificar ou revisar qualquer código (SOLID, Clean Code, regras C++). Onde a L-22 substitui uma cláusula dele, a L-22 vence |
| `TESTES.md` | Antes de planejar ou executar teste, análise estática, fuzzing, sanitizer, auditoria de suíte — adaptado ao TDD estrito e aos cinco portões da L-19 |
| `AUDITORIAS.md` | Ao conduzir auditoria técnica, ancorada nos cinco portões da L-19 e na proteção de dado da L-25 |
| `AGILE.md` | Planejamento, cadência, ondas, priorização e backlog |
| `DEPLOY_CHECKLIST.md` | Qualquer operação irreversível: tag, release pública, quebra de compatibilidade de save (L-23) |
| `TOOLING.md` | Antes de improvisar em shell — qual ferramenta FOSS canônica usar por domínio |
| `ORG.md`, `pipeline_release_1.0.md`, `lideranca_pipeline_release.md` | Para saber quem lidera o quê na constelação bigtech, qual C-level ativar, as 12 fases (L-10) |
| `ASSETS-LICENSE.md` | Antes de versionar, licenciar ou redistribuir qualquer asset (arte, música, som, sprite, texto de sabor, prosa in-game) — fronteira entre código e asset (L-08, L-25) |
| `CHARS.md` | Antes de criar, editar ou citar qualquer personagem do jogo — inventário canônico, imutável sem aprovação do líder |
| `PLACES.md` | Antes de criar, editar ou citar qualquer lugar do jogo — inventário canônico, imutável sem aprovação do líder |
| `sinopse.md` | Antes de qualquer trabalho de lore, narrativa ou worldbuilding que precise do panorama canônico do mundo |
| `AI-DISCLOSURE.md` | Antes de escrever ou revisar qualquer nota pública sobre uso de IA no projeto |
| `OFFLINE-NOTICE.md` | Antes de escrever texto público sobre a licença e o caráter offline do jogo |
| `THIRD-PARTY-LICENSES.md` | Antes de adicionar, remover ou documentar dependência de terceiro |
| `TEXTREVIEW.md` | Antes de revisar ou editar o texto dos dois livros-companheiros |
| `Standards.md` | Ponto de entrada: índice dos manuais de processo acima |

Ao despachar um subagent, **inclua o caminho absoluto do manual no prompt da task** — subagents não herdam este contexto.

## Comandos

**Não há comando de build, teste ou lint a executar hoje.** Confirmado por comando, não por omissão: sem `CMakeLists.txt`, sem `src/`, sem `include/`, sem `tests/`, sem workflow de CI (ver "Estado atual do repositório" acima). A fundação de build nasce como a primeira fatia de código do projeto (L-20), e esta seção é reescrita nesse momento.

O que roda hoje é o portão 5 da L-19, local:

- `tools/git-hooks/install.py` instala `tools/security/commit_gate.py` como gancho `commit-msg`.
- `python3 tools/security/test_commit_gate.py` roda o teste próprio do gate (não é `ctest`, porque não existe build de C++ ainda).

## Restrições desta máquina que mordem este projeto

- **Build pesado vai para `/var/tmp`, com `export TMPDIR=/var/tmp`.** `/tmp` aqui é tmpfs (sai da RAM); build C++ grande enche o tmpfs e o link falha com "no space on device".
- **Espaço em disco se mede com `btrfs filesystem usage /`** (sem `sudo`), lendo `Device unallocated` e o `min` de `Free (estimated)`. O `df` mente em btrfs.
- **Teste que toca janela, teclado, mouse ou tela:** ainda não existe nenhum teste desse tipo no GusWorld — não há `present/` (L-06, L-27), e a L-19 de `GODS_LAWS.md` deste projeto não registra regra própria de isolamento. Quando `present/` nascer e ligar direto no GlintFx, a prática desta máquina (compositor Wayland aninhado dentro de container, nunca a sessão viva) é a mesma já em uso no GlintFx — mas isto é prática observada, não lei formal deste projeto; confirmar com o líder antes de tratá-la como obrigatória aqui.
- **Verificação de entregável visual é do `qa-engineer`**, independente de quem implementou, com o orquestrador reconferindo o relatório do QA — ainda não se aplica, porque não há interface nem tela.

## Pendências

A tabela de pendências do projeto está em `TODO.md` na raiz: **71 itens** (`grep -cE '^\| [0-9]' TODO.md`), em schema de **10 colunas** (`WSJF`, `ID`, `Onda`, `Grupo`, `Descrição Técnica`, `Prioridade`, `Pré-requisito`, `Dificuldade`, `Status`, `Estado Auditado`, com `WSJF` como primeira coluna, L-30). A coluna `Onda` tem **21 valores distintos** (`0` a `19`, mais `—` para item bloqueado sem data).

Status medidos linha a linha da tabela (excluindo a legenda do cabeçalho, que cita cada status uma vez): **59 `⏳ Pendente`**, **10 `🔍 Pendente verificação`**, **1 `✅ Concluído`**, **1 `🟡 Parcial`** (item `A11`). Soma = 71.

O rastreio dos 28 itens do `inicial.md` — condição da L-21 para apagá-lo — vive dentro do próprio `TODO.md`, não duplicado aqui.

## Bus entre projetos: `gusworld_ia_autocomm`

Canal assíncrono entre as sessões do líder e o filho dele. Clone: `<vault>/gusworld_ia_autocomm/`, repo **privado** `petrinhu/gusworld_ia_autocomm`. **O protocolo canônico é o `PROTOCOL.md` do clone**; leia lá antes de usar, e trate a L-07 e a L-31 de `GODS_LAWS.md` como o resumo operacional.

| Slug | Projeto |
|---|---|
| **`gusworld`** | **este projeto**, o jogo |
| `glintfx` | `Projects/GlintFx`, o framework do qual o GusWorld depende (LEI ZERO) |
| `site` | `petrinhu/site_gusworld`, a revista retro que conta o progresso do jogo |
| `mapeditor` | `petrinhu/gusworld_mapeditor`, editor de mapas |
| Gus Dragon (`Dragon-Drv`) | colaborador humano, filho do líder; manda ideias por issue, `.txt` no `inbox/` ou discussion (L-07, L-16) |

Fluxo: ler o que está solto em `inbox/gusworld/`, agir, `git mv` para `inbox/gusworld/archive/`, commit `read: <arquivo>`, push. Enviar: um `.md` em `inbox/<destinatario>/` com frontmatter `de`/`para`/`assunto`/`data`, sempre depois de `git pull`. **Pedido vai sem classificação de prioridade** (quem recebe classifica).

Estado em 24/08/2026 (`ls inbox/gusworld/`): **vazia**, só `archive/` e `.gitkeep`.

**Aviso proativo ao Gus Dragon é obrigação permanente (L-31):** sempre que o líder aprovar, rejeitar ou mudar algo que seja ideia dele, ou fechar item de alta prioridade da tabela, avisar **sem que ele pergunte** — na discussion 7 (o catálogo de bugs que ele mantém), com timestamp e uma das três classificações que ele fixou (**Bug Consertado**, **Bug Funcional**, **Bug Possível**).
