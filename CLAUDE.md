> **LEI ZERO, ACIMA DE TUDO: O GUSWORLD ASSENTA SÓ EM GLINTFX E NO SISTEMA OPERACIONAL.** Nunca raciocine sobre este projeto como se ele pudesse ter janela própria, contexto gráfico próprio, laço de quadro próprio ou workaround por cima do framework. Janela, laço principal, entrada, áudio e desenho são do **GlintFx**; o GusWorld escreve **só a lógica do jogo**. Se uma função ainda não existe no GlintFx, o pedido vai para o bus e o trabalho espera parado — nunca se contorna a fronteira. Ver LEI ZERO e GW-05 em `GODS_LAWS.md`.

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

Transcrito de `GODS_LAWS.md` (38 leis, medido em 20/11/2026 por `grep -c "^## GW-" GODS_LAWS.md`). Este índice é cópia; a fonte de verdade é sempre o arquivo, nunca esta tabela.

| Lei | Gatilho: dispara quando você vai... | Resumo |
|---|---|---|
| [GW-01](GODS_LAWS.md#gw-01) | procurar código anterior, "como era antes", ou base para reaproveitar | O código é do zero; o GusWorld anterior não é base |
| [GW-02](GODS_LAWS.md#gw-02) | tocar arte, câmera, sprite, modelo ou pipeline visual | Jogo é 2D pixel-art; o pipeline 3D é do líder e agente não toca |
| [GW-03](GODS_LAWS.md#gw-03) | criar build, escolher padrão de linguagem | C++23 |
| [GW-04](GODS_LAWS.md#gw-04) | escrever função, arquivo, classe, módulo, item ou carta | Proibido monolito; cada elemento do jogo é átomo com POCO próprio |
| [GW-05](GODS_LAWS.md#gw-05) | precisar de algo que o GlintFx ainda não tem, ou de ferramenta de apoio | Proibido dublê: ou liga no GlintFx, ou não existe. ⚠️ **Ferramenta que só constrói e nunca embarca NÃO é dependência** (LEI ZERO, reforma de 28/08/2026) |
| [GW-06](GODS_LAWS.md#gw-06) | decidir o que construir primeiro | Núcleo puro agora; a camada que desenha só nasce com o GlintFx |
| [GW-07](GODS_LAWS.md#gw-07) | precisar de função do framework, ou receber ideia do Gus | Bus: pedido só quando o jogo esbarrar de verdade na falta |
| [GW-08](GODS_LAWS.md#gw-08) | criar `LICENSE`, cabeçalho de arquivo, versionar asset ou publicar | AGPL-3.0-or-later no código; assets e lore com todos os direitos reservados; livros à parte |
| [GW-09](GODS_LAWS.md#gw-09) | escrever CI ou declarar suporte de plataforma | Cinco alvos, todos em `:latest`; Fedora primário; CachyOS é próprio, não Arch |
| [GW-10](GODS_LAWS.md#gw-10) | ir executar qualquer trabalho de produto | Main só orquestra; C-level audita e planeja; operacional cria e implementa; modelo pelo /modelos_sessao |
| [GW-11](GODS_LAWS.md#gw-11) | fazer **qualquer** pergunta ao líder, inclusive de esclarecimento | `AskUserQuestion` SEMPRE, com opções clicáveis; nunca pergunta em prosa |
| [GW-12](GODS_LAWS.md#gw-12) | escrever qualquer mensagem ao líder | Timestamp real `[DD/MM/YY - HH:MM:SS]` obtido do `date` |
| [GW-13](GODS_LAWS.md#gw-13) | tocar em qualquer trabalho que dependa de canon | Canon desatualizado bloqueia: atualiza primeiro, trabalha depois |
| [GW-14](GODS_LAWS.md#gw-14) | concluir que algo está morto, obsoleto ou descartável | Nada é declarado morto por agente; a decisão é do líder |
| [GW-15](GODS_LAWS.md#gw-15) | versionar binário, asset pesado ou configurar o repositório | `livros/` e `glb/` fora do git; LFS no resto |
| [GW-16](GODS_LAWS.md#gw-16) | homenagear pessoa real, ou citar o filho do líder | Homenagem só com aceite; o filho aparece só como "Gus Dragon" |
| [GW-17](GODS_LAWS.md#gw-17) | criar módulo, arquivo, ou desenhar a forma de um sistema | Espinha de cinco camadas com gate de CI; regra como comando e evento |
| [GW-18](GODS_LAWS.md#gw-18) | escolher formato de arquivo de mapa, save, configuração ou item | Nada em formato de texto; máxima proteção contra edição |
| [GW-19](GODS_LAWS.md#gw-19) | commitar, fechar fatia, pensar em cobertura e formatação, **ou escrever ordem de serviço que vai ser revisada** | Cinco portões de qualidade; TDD estrito; sem meta de cobertura; **rigor da revisão por CAMADA (N1 a N4), declarada no cabeçalho da ordem, e o orçamento de rodadas sai dela, não do revisor** |
| [GW-20](GODS_LAWS.md#gw-20) | escrever CI, ou declarar que uma plataforma funciona | Matriz das cinco plataformas desde o primeiro commit |
| [GW-21](GODS_LAWS.md#gw-21) | achar que o `inicial.md` cumpriu o papel dele | Só se apaga com as quatro condições satisfeitas e o líder confirmando |
| [GW-22](GODS_LAWS.md#gw-22) | nomear qualquer coisa, ou escrever comentário e commit | Identificador e comentário em inglês, `snake_case`; commit em pt-br |
| [GW-23](GODS_LAWS.md#gw-23) | criar tag, publicar release, ou mexer na versão | Versão e tag são `vA.B.C.D`, como no GlintFx |
| [GW-24](GODS_LAWS.md#gw-24) | ver regra, seção ou documento que o líder revogou, ou escrever relatório/reconferência que narraria por que uma versão anterior dele reprovou | Revogado se APAGA; não se guarda como histórico — vale também para histórico de processo (bilhete sobre reprovação anterior) |
| [GW-25](GODS_LAWS.md#gw-25) | tocar save, configuração, mapa ou catálogo de conteúdo | Envelope binário selado, teto técnico faseado, cripto vem do GlintFx |
| [GW-26](GODS_LAWS.md#gw-26) | tocar câmera, mapa, locomoção ou direção de sprite | 3/4 top-down fixa, grade quadrada; sprite em quatro cardeais, locomoção em oito |
| [GW-27](GODS_LAWS.md#gw-27) | pensar em escrever tela, HUD ou marcação de interface | Nenhuma interface se escreve antes de o GlintFx traduzir marcação |
| [GW-28](GODS_LAWS.md#gw-28) | precisar baixar, instalar ou atualizar qualquer coisa | Pergunte ao líder; nunca falhe calado; `sudo` sempre com `-A` |
| [GW-29](GODS_LAWS.md#gw-29) | propor recurso, sistema ou escopo novo | Os 14 cortes: a cerca do que o jogo NÃO é |
| [GW-30](GODS_LAWS.md#gw-30) | escrever, reordenar ou acrescentar item na tabela, ou escrever/editar checklist em qualquer outro documento do projeto | Tabela aponta para o documento que a especifica; TODO.md é a única fonte de checklist, o resto aponta de volta pelo ID, sem caixa de estado própria |
| [GW-31](GODS_LAWS.md#gw-31) | o líder aprovar, rejeitar ou mudar algo, ou fechar item de alta prioridade | Avisar o Gus Dragon sem ele perguntar |
| [GW-32](GODS_LAWS.md#gw-32) | fechar uma fatia, fechar uma onda, ou pensar em `git push` | Commit por fatia; push livre para ramo de trabalho, aceito só com CI verde; merge e tag só com onda fechada e aval |
| [GW-33](GODS_LAWS.md#gw-33) | criar ou revisar unidade de código do jogo, ou pensar em atomizar teste, ferramenta, documento, commit, item ou asset | Átomo só no código do jogo; fora dele, organiza-se como for melhor; no jogo, monolito é acoplamento, não tamanho, e há cinco perguntas na revisão |
| [GW-34](GODS_LAWS.md#gw-34) | ver qualquer coisa vinda do Gus Dragon, em qualquer dos cinco canais | Pedido dele é prioridade e é SEMPRE respondido; ack imediato, interrompe onda |
| [GW-35](GODS_LAWS.md#gw-35) | rodar qualquer teste que EXECUTE código: suíte, fuzzing, sanitizer, mutação, sonda de janela ou entrada | Nenhum teste dinâmico toca a sessão do líder; sempre em container. Sem container, o resultado é "não executado" |
| [GW-36](GODS_LAWS.md#gw-36) | decidir se lore ainda não escrita atrasa uma fatia de engenharia, ou sentir a tentação de esperar completar canon antes de codar | Lore que falta não bloqueia código, puxa-se sob demanda em micro-sessão pontual; distinta da GW-13, que trava por canon que contradiz, não por canon que ainda falta |
| [GW-37](GODS_LAWS.md#gw-37) | editar, revisar ou propor mudança em fala, aparte, revide, cena ou traço de caracterização já aprovado que define o Gus original | Mudança em conteúdo já aprovado do Gus original exige nova autorização explícita do líder; parente da GW-16, sem fundir nela |
| [GW-38](GODS_LAWS.md#gw-38) | desenhar, revisar ou implementar qualquer enigma, puzzle ou câmara de lógica do jogo | Nenhum puzzle cede por ferramenta do jogo em vez do jogador; vale para todo enigma, escrito antes ou depois desta lei |

## O que é o GusWorld

Jogo **2D pixel-art**, single-player e offline, em **C++23**, assentado sobre o framework [GlintFx](../GlintFx) (LEI ZERO): o GusWorld não cria janela, não possui contexto gráfico e não roda laço próprio de quadro — isso é do GlintFx, e o GusWorld escreve só a lógica do jogo.

- **Perspectiva:** 3/4 top-down fixa, grade quadrada; o **sprite** vive em quatro direções cardeais desenhadas à mão (sem espelhamento) e a **locomoção** aceita diagonal, exibindo o cardeal mais próximo (GW-26).
- **Estrutura de mundo:** **sem mundo aberto e sem mundo persistente** (corte `C-02`).
- **Progressão de cartas:** por progresso narrativo **e por crafting**.
- **Escopo da campanha:** **longa por desenho, da ordem de meses de jogo, sem número de horas fixado** (corte `C-15`, decisão do líder de 11/10/2026, levantada pelo Gus Dragon na issue 11 do barramento).
- **Distribuição:** FOSS, código sob AGPL-3.0-or-later; assets e lore com todos os direitos reservados (GW-08).

A cerca completa do que o jogo **não** é (**14 cortes**, numerados `C-01` a `C-16` com os buracos do `C-03` e do `C-13`, revogados em 25/08/2026) está na GW-29. Os números revogados **não se reaproveitam**.

Antes de desenhar mapa, conectividade entre áreas ou escopo de conteúdo, leia o detalhe da estrutura de mundo e do escopo da campanha em [`docs/claude/estado-e-comandos.md`](docs/claude/estado-e-comandos.md), seção "O que é o GusWorld: o detalhe".

## Estado atual do repositório

Antes de afirmar quantos commits, arquivos, testes, workflows ou ADRs existem, ou o que há em `docs/` e em `tools/`, leia o inventário medido por comando em [`docs/claude/estado-e-comandos.md`](docs/claude/estado-e-comandos.md), seção "Estado atual do repositório".

## Decisões fechadas pelo líder

Estas foram tomadas explicitamente via `AskUserQuestion` (salvo onde indicado) e estão detalhadas em `GODS_LAWS.md`. Mudança em qualquer uma delas é decisão do líder, não de agente.

| Eixo | Decisão |
|---|---|
| Natureza | Jogo 2D pixel-art, single-player, offline — executável final, não biblioteca |
| Linguagem e build | C++23; única dependência de framework é o GlintFx (LEI ZERO) |
| Perspectiva e câmera | 3/4 top-down fixa, grade quadrada; sprite em quatro cardeais desenhadas à mão e sem espelhamento, locomoção em oito exibindo o cardeal mais próximo (GW-26) |
| Estrutura de mundo | Sem mundo aberto nem persistente (`C-02`); a **geografia** é o grafo de 13 áreas com dobradiças do `mundo-topologia.md`, não o corte |
| Plataformas | Fedora (primário, em `:latest`), Ubuntu, Arch, CachyOS (próprio, não é Arch renomeado), Windows: cinco entradas distintas de CI desde o primeiro commit, todas acompanhando `:latest` como no GlintFx (GW-09, GW-20) |
| Arquitetura | Espinha de cinco camadas (`core/ → content/ → domain/ → app/ → present/`), dependência só para baixo, com gate de CI; regra como transição determinística por comando e evento (GW-17) |
| Persistência de dado | Nenhum formato de texto: save, configuração, mapa e catálogo de conteúdo em envelope binário único selado (GW-18, GW-25) |
| Licença do código | AGPL-3.0-or-later, com ressalva explicando o caráter offline do jogo (GW-08) |
| Licença de assets e lore | Todos os direitos reservados, com carve-out de marca e permissão de conteúdo de fã não comercial (GW-08) |
| Titular do copyright | Petrus Alves da Silva Costa (GW-08) |
| Porte do projeto (bigtech) | **COMPLETO** — pipeline de 12 fases e toda a constelação C-level ativos, nenhuma fase dormente por presunção de agente (GW-10) |
| Ordem de construção | Núcleo de regra de jogo em TDD estrito agora; a camada `present/` só nasce quando o GlintFx tiver janela, contexto gráfico, entrada e texto (GW-06) |

## Autoridade documental

Os manuais na raiz são normativos e vencem qualquer preferência do agente, **e perdem para `GODS_LAWS.md`** em caso de conflito. **Leia o manual relevante ANTES de decidir**, não depois. `Standards.md` é o hub que indexa os manuais de processo; a tabela abaixo estende esse índice aos manuais de licença e de canon citados neste arquivo.

| Manual | Quando é obrigatório ler |
|---|---|
| `CONTRACT.md` | Antes de escrever, modificar ou revisar qualquer código (SOLID, Clean Code, regras C++). Onde a GW-22 substitui uma cláusula dele, a GW-22 vence |
| `TESTES.md` | Antes de planejar ou executar teste, análise estática, fuzzing, sanitizer, auditoria de suíte — adaptado ao TDD estrito e aos cinco portões da GW-19 |
| `AUDITORIAS.md` | Ao conduzir auditoria técnica, ancorada nos cinco portões da GW-19 e na proteção de dado da GW-25 |
| `AGILE.md` | Planejamento, cadência, ondas, priorização e backlog |
| `DEPLOY_CHECKLIST.md` | Qualquer operação irreversível: tag, release pública, quebra de compatibilidade de save (GW-23) |
| `TOOLING.md` | Antes de improvisar em shell — qual ferramenta FOSS canônica usar por domínio |
| `ORG.md`, `pipeline_release_1.0.md`, `lideranca_pipeline_release.md` | Para saber quem lidera o quê na constelação bigtech, qual C-level ativar, as 12 fases (GW-10) |
| `ASSETS-LICENSE.md` | Antes de versionar, licenciar ou redistribuir qualquer asset (arte, música, som, sprite, texto de sabor, prosa in-game) — fronteira entre código e asset (GW-08, GW-25) |
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

**Há build e há suíte, desde 11/10/2026.** ⚠️ **Mas nenhum deles roda na sessão do líder: a GW-35 exige container, sempre.** Sem container, o resultado honesto é "não executado, falta o container".

O caminho medido que funciona nesta máquina, e por que cada parte dele é assim:

- **Configure e build:** `cmake -S . -B <dir> -G Ninja -DGUSWORLD_WERROR=ON`, depois `cmake --build <dir>`. O `-Werror` fica atrás de opção porque é o gancho do portão 1 da GW-19, e só o CI o liga.
- **Suíte:** `ctest --test-dir <dir> --output-on-failure`. ⚠️ **`ctest -N` num build sem teste diz `Total Tests: 0` e SAI COM CÓDIGO ZERO** (medido em 11/10): todo passo que contar teste tem de testar o VALOR, nunca só o código de saída.
- **Volume de host em container NÃO funciona:** o SELinux desta máquina está `Enforcing` e bloqueia `container_t`. A L-58 do global manda não mexer em política; os arquivos entram por `tar` na entrada padrão e o resultado sai pela saída padrão.
- **Cópia de trabalho em subpasta FALHA neste repositório:** o filtro de descriptografia do `git-crypt` não resolve no worktree novo e o checkout aborta nos arquivos de `docs/_secret/`. Para verificar uma fatia, extraia do blob com `git archive`, que é o que a L-27 do global manda de todo modo.
- **Instalar a toolchain em container estoura o limite de primeiro plano:** rode em segundo plano.

**O guard de TDD roda a suíte em contêiner** (`C8`; desenho, números e limites no `ADR-026`):

- O `.claude/tdd-guard.json` chama `bash tools/tdd/run_suite.sh`, com `timeout_sec` de 144 s, e o script roda a suíte no contêiner `gusworld-tdd-<hash16>`, nunca na sessão do líder (GW-35).
- O script sai com 0 (verde), 1 (vermelho) ou 126 (não rodou). O 126 é fail-open: a edição passa, com o aviso do plugin e o do `tools/tdd/notice.py`.
- Rode `bash tools/tdd/prepare.sh` em segundo plano para aquecer o contêiner. Com o contêiner frio, o guard recebe 126.
- **Protocolo de rajada, combinado com a sessão `manutencao`:** antes de editar C++, a sessão avisa "abrindo rajada do guard" e só edita se houver vaga; no fim, avisa "fechando rajada" com o pico medido. O preparo frio se pede à parte.
- O aviso de tela (`systemMessage`) não chega ao líder quando quem edita é subagente. Por isso todo subagente cita no relatório, verbatim, o aviso do guard que receber.
- Antes de ler o resultado de uma execução do guard ou de rodar os testes de `tools/tdd/`, veja onde ficam em [`docs/claude/estado-e-comandos.md`](docs/claude/estado-e-comandos.md), seção "Comandos: o detalhe".

Os portões locais de texto, que não dependem de build e rodam desde agosto:

- `tools/git-hooks/install.py` instala `tools/security/commit_gate.py` como gancho `commit-msg` e `tools/security/lei_zero_guard.py` como `pre-commit`.
- `python3 tools/security/test_commit_gate.py` e `python3 tools/security/test_lei_zero_guard.py` rodam os autotestes dos dois.

Antes de conferir tag contra a versão, de afirmar o que um workflow de CI prova ou de citar prova medida de execução, leia [`docs/claude/estado-e-comandos.md`](docs/claude/estado-e-comandos.md), seção "Comandos: o detalhe".

Os portões de CI em `tools/ci/<portão>/` são scripts de uma responsabilidade cada. O workflow só orquestra, e cada veredicto sai de um script.

## Restrições desta máquina que mordem este projeto

- **Build pesado vai para `/var/tmp`, com `export TMPDIR=/var/tmp`.** `/tmp` aqui é tmpfs (sai da RAM); build C++ grande enche o tmpfs e o link falha com "no space on device".
- **Espaço em disco se mede com `btrfs filesystem usage /`** (sem `sudo`), lendo `Device unallocated` e o `min` de `Free (estimated)`. O `df` mente em btrfs.
- **Nenhum teste dinâmico roda na sessão do líder — é a GW-35, lei desde 29/08/2026.** Vale para todo teste que EXECUTA código (suíte, integração, fuzzing, sanitizer, mutation testing, benchmark, sonda de janela/teclado/mouse/tela), não só para os que tocam a tela: **sempre em container**. Sem container disponível, o resultado honesto é **"não executado, falta o container"** — nunca rodar fora e reportar sucesso.
- Antes de rodar teste com janela, entrada ou tela, leia o detalhe do `XDG_RUNTIME_DIR` e do compositor aninhado em [`docs/claude/estado-e-comandos.md`](docs/claude/estado-e-comandos.md), seção "Teste dinâmico em container (GW-35): o detalhe".
- **Verificação de entregável visual é do `qa-engineer`**, independente de quem implementou, com o orquestrador reconferindo o relatório do QA — ainda não se aplica, porque não há interface nem tela.

## Pendências

Antes de ler, contar ou editar a tabela de pendências (`TODO.md` na raiz), leia o schema dela e os comandos que contam itens e ondas em [`docs/claude/estado-e-comandos.md`](docs/claude/estado-e-comandos.md), seção "Pendências".

## Bus entre projetos: `gusworld_ia_autocomm`

Antes de ler ou escrever no bus, leia o clone, o protocolo, os slugs, o fluxo e o aviso proativo ao Gus Dragon em [`docs/claude/estado-e-comandos.md`](docs/claude/estado-e-comandos.md), seção "Bus entre projetos".
