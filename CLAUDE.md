> **LEI ZERO, ACIMA DE TUDO:** raciocine sobre o GusWorld como jogo que assenta **só no GlintFx e no sistema operacional**, nunca como se pudesse ter janela própria, contexto gráfico próprio, laço de quadro próprio ou workaround por cima do framework. Deixe janela, laço principal, entrada, áudio e desenho com o **GlintFx** e escreva **só a lógica do jogo**. Se uma função ainda não existe no GlintFx, mande o pedido ao bus e espere parado, sem contornar a fronteira. Corpo na LEI ZERO e na GW-05 do `GODS_LAWS.md`.

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Leis: leia `GODS_LAWS.md` antes de agir

Siga as ordens expressas do líder em [`GODS_LAWS.md`](GODS_LAWS.md), que vencem este arquivo, os manuais e qualquer preferência sua. Antes do primeiro comando de cada tarefa, varra o índice de gatilhos de lá e leia inteira a lei cujo gatilho casa, antes de agir. O protocolo de uso (subagent, relato, lei nova) também mora lá. Não copie o índice para cá, porque índice duplicado envelhece em silêncio.

## O que é o GusWorld

Jogo **2D pixel-art**, single-player e offline, em **C++23**: executável final, não biblioteca, assentado sobre o framework [GlintFx](../GlintFx) (LEI ZERO). Estas decisões são do líder e têm o corpo no `GODS_LAWS.md`; leve a ele toda mudança nelas:

- perspectiva 3/4 top-down fixa, grade quadrada, sprite em quatro direções cardeais desenhadas à mão e locomoção em diagonal exibindo o cardeal mais próximo (GW-26);
- sem mundo aberto e sem mundo persistente (`C-02`), com a geografia no grafo de 13 áreas com dobradiças do `mundo-topologia.md`;
- progressão de cartas por progresso narrativo e por crafting;
- campanha longa por desenho, da ordem de meses de jogo, sem número de horas fixado (`C-15`, levantada pelo Gus Dragon na issue 11 do barramento); a cerca completa do que o jogo **não** é (14 cortes, `C-01` a `C-16`, sem reaproveitar `C-03` e `C-13`) está na GW-29;
- cinco plataformas no CI desde o primeiro commit (GW-09, GW-20), espinha de cinco camadas `core/ → content/ → domain/ → app/ → present/` com gate de CI (GW-17), dados em envelope binário selado e nunca em texto (GW-18, GW-25), e núcleo de regra em TDD antes da camada `present/` (GW-06);
- código sob AGPL-3.0-or-later; assets e lore com todos os direitos reservados a Petrus Alves da Silva Costa, com carve-out de marca e conteúdo de fã não comercial (GW-08);
- porte bigtech **COMPLETO**: pipeline de 12 fases e toda a constelação C-level ativos, sem fase dormente por presunção de agente (GW-10).

Antes de desenhar mapa, conectividade entre áreas ou escopo de conteúdo, leia [`docs/claude/estado-e-comandos.md`](docs/claude/estado-e-comandos.md), seção "O que é o GusWorld: o detalhe". Antes de afirmar quantos commits, arquivos, testes, workflows ou ADRs existem, ou o que há em `docs/` e em `tools/`, leia a seção "Estado atual do repositório" do mesmo arquivo.

## Manuais

Leia o manual relevante ANTES de decidir. Os manuais da raiz vencem qualquer preferência sua e perdem para o `GODS_LAWS.md`. `Standards.md` indexa os manuais de processo; a tabela estende o índice aos de licença e de canon.

| Manual | Leia antes de... |
|---|---|
| `CONTRACT.md` | escrever, modificar ou revisar código (SOLID, Clean Code, regras C++); onde a GW-22 diverge dele, siga a GW-22 |
| `TESTES.md` | planejar ou executar teste, análise estática, fuzzing, sanitizer ou auditoria de suíte, no TDD estrito e nos cinco portões da GW-19 |
| `AUDITORIAS.md` | conduzir auditoria técnica, ancorada nos cinco portões da GW-19 e na proteção de dado da GW-25 |
| `AGILE.md` | planejar cadência, ondas, priorização e backlog |
| `DEPLOY_CHECKLIST.md` | qualquer operação irreversível: tag, release pública, quebra de compatibilidade de save (GW-23) |
| `TOOLING.md` | improvisar em shell: qual ferramenta FOSS canônica usar por domínio |
| `ORG.md`, `pipeline_release_1.0.md`, `lideranca_pipeline_release.md` | decidir quem lidera o quê na constelação bigtech, qual C-level ativar e em qual das 12 fases (GW-10) |
| `ASSETS-LICENSE.md` | versionar, licenciar ou redistribuir asset (arte, música, som, sprite, texto de sabor, prosa in-game), na fronteira entre código e asset (GW-08, GW-25) |
| `CHARS.md` | criar, editar ou citar personagem do jogo: inventário canônico, imutável sem aprovação do líder |
| `PLACES.md` | criar, editar ou citar lugar do jogo: inventário canônico, imutável sem aprovação do líder |
| `sinopse.md` | trabalho de lore, narrativa ou worldbuilding que precise do panorama canônico do mundo |
| `AI-DISCLOSURE.md` | escrever ou revisar nota pública sobre uso de IA no projeto |
| `OFFLINE-NOTICE.md` | escrever texto público sobre a licença e o caráter offline do jogo |
| `THIRD-PARTY-LICENSES.md` | adicionar, remover ou documentar dependência de terceiro |
| `TEXTREVIEW.md` | revisar ou editar o texto dos dois livros-companheiros |
| `Standards.md` | começar: é o índice dos manuais de processo |

Ao despachar subagent, inclua no prompt da task o caminho absoluto do manual, porque subagent não herda este contexto.

## Comandos

Rode build e suíte só em container, nunca na sessão do líder (GW-35); sem container, relate "não executado, falta o container".

- **Configure e compile:** `cmake -S . -B <dir> -G Ninja -DGUSWORLD_WERROR=ON`, depois `cmake --build <dir>`. O `-Werror` fica atrás de opção porque é o gancho do portão 1 da GW-19, e só o CI o liga.
- **Suíte:** `ctest --test-dir <dir> --output-on-failure`. Teste o VALOR em todo passo que conta teste, nunca só o código de saída, porque `ctest -N` num build sem teste diz `Total Tests: 0` e sai com código zero.
- **Arquivos no container:** passe-os por `tar` na entrada padrão e leia o resultado na saída padrão, porque o SELinux desta máquina está `Enforcing` e bloqueia volume do host em `container_t`, e a L-58 global manda não mexer na política.
- **Verificar uma fatia:** extraia do blob com `git archive`, como manda a L-27 global, sem cópia de trabalho em subpasta, porque o filtro de descriptografia do `git-crypt` não resolve no worktree novo e o checkout aborta nos arquivos de `docs/_secret/`.
- **Toolchain no container:** instale-a em segundo plano, porque a instalação estoura o limite de primeiro plano.

Guard de TDD, que roda a suíte em contêiner (`C8`; desenho, números e limites no `ADR-026`):

- O `.claude/tdd-guard.json` chama `bash tools/tdd/run_suite.sh`, com `timeout_sec` de 144 s, e o script roda a suíte no contêiner `gusworld-tdd-<hash16>`, nunca na sessão do líder (GW-35). Ele sai com 0 (verde), 1 (vermelho) ou 126 (não rodou); o 126 é fail-open, e a edição passa com o aviso do plugin e o do `tools/tdd/notice.py`.
- Aqueça o contêiner com `bash tools/tdd/prepare.sh`, em segundo plano, porque com o contêiner frio o guard recebe 126.
- Siga o protocolo de rajada combinado com a sessão `manutencao`: antes de editar C++, avise "abrindo rajada do guard" e só edite se houver vaga; no fim, avise "fechando rajada" com o pico medido. Peça o preparo frio à parte.
- Como subagente, cite no relatório, verbatim, o aviso do guard que receber, porque o aviso de tela (`systemMessage`) não chega ao líder quando quem edita é subagente.

Portões locais de texto, que não dependem de build: `tools/git-hooks/install.py` instala `tools/security/commit_gate.py` como gancho `commit-msg` e `tools/security/lei_zero_guard.py` como `pre-commit`; `python3 tools/security/test_commit_gate.py` e `python3 tools/security/test_lei_zero_guard.py` rodam os autotestes dos dois.

Mantenha cada portão de CI em `tools/ci/<portão>/` como script de uma responsabilidade: o workflow só orquestra, e cada veredicto sai de um script.

Antes de ler o resultado de uma execução do guard, rodar os testes de `tools/tdd/`, conferir tag contra a versão, afirmar o que um workflow de CI prova ou citar prova medida de execução, leia [`docs/claude/estado-e-comandos.md`](docs/claude/estado-e-comandos.md), seção "Comandos: o detalhe".

## Restrições da máquina

- Compile build pesado em `/var/tmp`, com `export TMPDIR=/var/tmp`, porque `/tmp` aqui é tmpfs (sai da RAM) e o link de C++ grande falha com "no space on device".
- Meça espaço em disco com `btrfs filesystem usage /` (sem `sudo`), lendo `Device unallocated` e o `min` de `Free (estimated)`, porque o `df` mente em btrfs.
- Antes de rodar teste com janela, entrada ou tela, leia o detalhe do `XDG_RUNTIME_DIR` e do compositor aninhado em [`docs/claude/estado-e-comandos.md`](docs/claude/estado-e-comandos.md), seção "Teste dinâmico em container (GW-35): o detalhe".
- Deixe a verificação de entregável visual ao `qa-engineer`, independente de quem implementou, com o orquestrador reconferindo o relatório do QA, quando houver interface ou tela.

## Pendências

Antes de ler, contar ou editar a tabela de pendências (`TODO.md` na raiz), leia o schema dela e os comandos que contam itens e ondas em [`docs/claude/estado-e-comandos.md`](docs/claude/estado-e-comandos.md), seção "Pendências".

## Bus entre projetos: `gusworld_ia_autocomm`

Antes de ler ou escrever no bus, leia o clone, o protocolo, os slugs, o fluxo e o aviso proativo ao Gus Dragon em [`docs/claude/estado-e-comandos.md`](docs/claude/estado-e-comandos.md), seção "Bus entre projetos".
