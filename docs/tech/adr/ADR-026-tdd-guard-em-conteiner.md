# ADR-026: Guard de TDD roda a suíte em contêiner, e falha de infraestrutura avisa o agente

| | |
|---|---|
| **Status** | Proposed (respostas do líder de 20/11/2026; decisões técnicas a ratificar) |
| **Data** | 2026-11-20 |
| **Decisor** | petrinhu (líder supremo), por `AskUserQuestion`; desenho técnico do CTO (Caetano) |
| **Reversibilidade** | Two-way door: a imagem, a tag e o mecanismo de volume ou de `docker exec` trocam sem mudar o contrato do guard. O que muda o contrato (prazos, veredito de cada código de saída) exige nova ratificação. |

## Contexto

O guard de TDD do projeto (`.claude/tdd-guard.json`, item `C8` do `TODO.md`)
chama `ctest --test-dir build` na sessão do líder. Duas premissas erradas
estavam escritas no `C3` e no `C8`: a de que comando quebrado faz fail-open
com aviso, e a de que o guard fica armado por existir.

Fatos medidos em 20/11/2026 pelo CTO:

- `~/.claude/state/tdd-guard/43fdb329b8cd4907/last-run.json` tem `exit_code 1`,
  com o tail `Failed to change working directory to ".../GusWorld/build": No such file or directory`.
- O runner do plugin só grava `ran=false` para os códigos 126, 127 e 9009.
  Qualquer outro código diferente de zero vira `has_red=true`.
- Portanto o guard dá **falso vermelho** hoje. Um falso vermelho permite a
  edição de produção, ou seja, **libera produção**. A semântica "fail-open com
  aviso" escrita no `C3` e no `C8` é falsa.
- Stderr de hook com saída zero não chega ao agente. Hoje não há canal de
  aviso que cheque o agente.
- Há dois tetos de 180 s em conflito: `timeout_sec` do projeto e o `hooks.json`
  do plugin.
- O `TESTES.md`, na linha 197, ensinava `docker run -v "$PWD":/work:Z`. Isso
  contradiz o `CLAUDE.md` do projeto: o volume de host não funciona com o
  SELinux em `Enforcing`, e o `:Z` reetiqueta arquivos do host.
- O `.claude/tdd-guard.json` diz que não há C++ (`_hoje_e_inerte`), e há 17
  arquivos `.cpp`. Esse arquivo só muda na fatia 7.

## Decisão

O guard passa a rodar a suíte dentro de contêiner, e a falha de infraestrutura
passa a avisar o agente.

1. **Imagem própria.** `tools/container/build_image.sh` monta a imagem por
   instalação e commit, sem `docker build`. Primeiro `base_probe.sh` faz
   `docker pull fedora:latest`, lê o digest da linha `Digest:` da saída do pull e
   fixa a base por esse digest. Depois `container_install.sh` roda
   `docker run --memory 4g --memory-swap 4g --pids-limit 233 --cpus 4
   --security-opt no-new-privileges <base@digest>` com o contexto por tar na
   entrada padrão. O contexto tem quatro arquivos: `install_toolchain.sh`,
   `tools/ci/env/fedora.sh`, `_common.sh` e `tools/ci/cmake_floor.sh`. O
   `install_toolchain.sh` é a receita: instala a toolchain, o `python3-pyyaml` e o
   usuário `builder`, e grava o `rpm -q` em `/etc/gusworld/packages.txt`. A rede
   fica aberta só nessa etapa. Ao fim, `docker commit` grava a tag e o contêiner
   temporário sai por `trap`. A tag é `gusworld-ci-fedora:<sha12>`, e o hash cobre
   o conteúdo do contexto mais o digest da base (`env_hash.sh <base-digest>`). Uma
   base nova gera tag nova, e uma tag existente é recusada, nunca sobrescrita.
   O `Containerfile.fedora` saiu: a receita única é o `install_toolchain.sh`.
   Todo script que baixa ou constrói passa antes por `rebuild_authorized.sh`, que
   exige a linha de autorização de rebuild do dia no `docs/decisoes-do-lider.md`
   commitado (`git show HEAD`).
2. **Invólucro no host.** `test_command` passa a ser `bash tools/tdd/run_suite.sh`.
   O invólucro mapeia o código de saída: `0` vira `0`, `1` vira `1`, e qualquer
   outro código vira `126`. O tar da árvore de trabalho sai de uma lista branca,
   `tools/tdd/build_paths.txt`, que deixa de fora as quatro áreas cifradas por
   git-crypt.
3. **Contêiner de vida longa, sem volume (opção A).** A árvore entra por
   `docker exec -i` num contêiner `gusworld-tdd-<hash16>`. A opção B seria um
   volume nomeado, não adotada agora.
4. **Dentro do contêiner.** `tools/tdd/in_container.sh` faz a trava por `ln -s`
   (34 s): o alvo do link é o pid do dono, e um dono morto expira na hora. Define
   `XDG_RUNTIME_DIR` com permissão 700, confere que `ctest -N` tem valor maior que
   zero e roda build e `ctest` (89 s). A extração e o `touch_changed.sh` também
   contam no mesmo orçamento de 89 s, cada um sob `timeout` com o que sobra (revisão,
   MÉDIA-3). Ao fim de cada execução, a linha `tdd-tempo: N` diz quantos segundos
   passaram desde a trava; é o relógio que o prazo limita, e o veredito vem logo depois.
5. **Prazos.** `timeout_sec` de 144 s. O trabalho interno cabe em 89 s, e a trava
   em 34 s. A variável `GUSWORLD_TDD_BUDGET`, que o `run_suite.sh` repassa ao
   `docker exec` só quando o host a define, **só aperta** o orçamento: aceita inteiro
   positivo menor que 89, e qualquer outro valor é ignorado. Nunca afrouxa.
   **Nota de desenho:** apertar o prazo faz o guard devolver `126` (não rodou) nos
   trabalhos que passam do novo limite. Isso é fail-open por construção: o guard não
   libera produção por prazo apertado, e a falha aparece no motivo do `last-reason`.
   A fonte única dos valores do contêiner (memória, swap, pids, CPUs, usuário, rede)
   é `tools/tdd/container_spec.sh`, lido por `prepare.sh`, `container_ready.sh` e pelos
   testes. O `container_ready.sh` confere o HostConfig com esses valores exatos
   (incluindo CapAdd vazio e PidMode, IpcMode, UTSMode e UsernsMode vazios ou
   privados) antes de qualquer `docker start`.
6. **Contêiner frio.** Quando o contêiner não existe, o guard devolve `126`, que
   o runner trata como "não rodou". O aquecimento é feito por
   `tools/tdd/prepare.sh`, em segundo plano.
7. **Hardening.** `--network none`, `--cap-drop ALL`, `no-new-privileges`,
   `--user builder`, e nenhum `-v`.
8. **Aviso visível (revisada em 20/11/2026).** O aviso de guard que não rodou não
   reimplementa o guard. O `tools/tdd/notice.py`, instalado como `PreToolUse` no
   `.claude/settings.json` do projeto, chama `tdd_guard.evaluate` do plugin bigtech
   sobre a carga do hook, e fala só onde o guard abre (código 0 com mensagem: falha
   de infraestrutura ou de I/O), ou quando o próprio aviso não consegue confiar no
   plugin (registro, leitura, permissão ou import), caso em que avisa com texto
   fixo. Com código 2 (bloqueio) fica mudo, porque o guard já fala. O motivo vem de
   `last-reason` só se o diretório e o arquivo forem do uid do processo, não forem
   links e não forem graváveis por grupo ou outros, e só se não forem mais antigos
   que o `ts` do estado; caso contrário, vem do `reason` do estado ou da mensagem do
   guard. Todo texto que chega ao agente passa por limpeza de controle e ANSI e por
   corte de 200 caracteres. O aviso nunca bloqueia: toda saída do script é 0.
9. **Fatias.** Canon N4 (esta fatia), imagem N3, invólucro N2, preparo e medida
   N3, aviso N3, prova do guard N2 pelo `qa-engineer`, virada N2 e fecho N4.
   O critério de medição da fatia 4 é pré-registrado: 5 execuções sem mudança,
   5 incrementais e 3 com reconfiguração. Cabe se o interno ficar em até 89 s e
   a parede em até 123 s.

## Respostas do líder (20/11/2026, AskUserQuestion, verbatim)

- *"Autorizo com pyyaml (Recomendado)"*: libera o download da imagem própria com
  `fedora:latest`, a toolchain do `fedora.sh` e o `python3-pyyaml`. Cada rebuild
  futuro pergunta de novo.
- *"Docker, que já existe (Recomendado)"*: o runtime é o Docker já instalado.
- *"Hook de aviso visível (Recomendado)"*: o fail-open por infraestrutura
  continua, com um aviso que chega ao agente.
- *"Apagar na virada (Recomendado)"*: o link pendurado `build` sai na fatia 7.

## Decisões técnicas a ratificar

Decididas pelo orquestrador, sem ratificação registrada ainda:

- **Prazos 89/34/144:** trabalho interno de 89 s, trava de 34 s, `timeout_sec`
  de 144 s. Medidos na fatia 4 antes de virar fixo.
- **Opção A:** `docker exec -i` em contêiner de vida longa, sem volume.
- **Só GCC** no contêiner do guard. O Clang fica para o CI.
- **Sem `-Werror` no laço** do guard. O `-Werror` continua no CI, portão 1 da L-19.
- **`test_globs` novos** no `tdd-guard.json`, para a fatia 7.
- **Edição por Bash escapa do guard.** Fica como limite declarado. A regra é que
  produção e teste se editam só por Write e Edit, e as ordens de serviço dizem isso.
- **Limpeza pós-prova.** Depois da prova do `qa-engineer`, apaga-se o fixture
  `.tdd-proof/`. O resto fica.

## Alternativas consideradas

- **Volume de host (`-v`).** Rejeitada: o SELinux em `Enforcing` bloqueia
  `container_t`, e o `:Z` reetiqueta o host. A `L-60` do global manda não mexer
  em política.
- **Volume nomeado (opção B).** Não escolhida agora. Fica como alternativa se a
  opção A medir custo alto de cópia da árvore.
- **Manter o guard como está.** Rejeitada: libera produção com um falso vermelho
  medido.

## Consequências e limites

- **Positivas:** o guard deixa de liberar produção por falha de comando. A suíte
  roda fora da sessão viva do líder (L-35), e o aviso chega ao agente.
- **Custos:** cada teste copia a árvore pelo tar. O contêiner frio devolve `126`
  até o aquecimento. A imagem é puxada de `fedora:latest` e fixada por digest; cada rebuild pede
  autorização nova, registrada no índice de decisões (L-51).
- **Limites declarados:** edição por Bash escapa do guard. Falha de infraestrutura
  continua em fail-open, agora com aviso. A verificação de que o guard morde de
  verdade é a prova da fatia 6, não presunção.
- **Acoplamento ao nome interno do plugin:** o `notice.py` depende de
  `tdd_guard.evaluate` e de `tdd_common` (`find_project_root`, `read_state`,
  `extract_file_path`) do plugin bigtech, nomes internos e não versionados. Um
  `evaluate` público e versionado foi pedido ao plugin, registrado na INBOX do
  acoplamento (`TODO.md`, commit `d18a7b8`). Até lá, o teste de contrato
  `tools/tdd/test/test_notice_contract.sh` roda o plugin real e falha se o nome mudar.
- **Checagem de dono só no código:** o `notice.py` confere `st_uid == os.getuid()`
  no diretório e no arquivo do `last-reason`, e no diretório do plugin. Os testes do
  contêiner cobrem só o modo gravável por grupo ou outros (0777 e 0666), porque o
  contêiner roda como `builder` e não cria arquivo de outro dono. A checagem de dono
  é garantia de código, sem teste de contêiner.
- **Dependências de outros itens:** o `D1` passa a depender de `C8` e `C10`.
  O `C8` depende do `C3`, o harness de teste próprio (`ADR-025`).

## Cross-refs

- [ADR-025](ADR-025-harness-teste-proprio-minimo.md) (harness de teste próprio)
- [ADR-024](ADR-024-ci-cinco-plataformas-cachyos-container.md) (contêiner Fedora na matriz de CI)
- `../../../GODS_LAWS.md` L-19 (cinco portões), L-35 (teste dinâmico só em contêiner), L-43 (critério pré-registrado), L-51 (instalação pede permissão)
- `TODO.md`, itens `C3`, `C8` e `D1`
