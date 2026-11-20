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

1. **Imagem própria.** `tools/container/Containerfile.fedora`, com
   `FROM fedora:latest` e `--pull`. O contexto de build é um tar com quatro
   arquivos: o `Containerfile`, `tools/ci/env/fedora.sh`, `_common.sh` e
   `tools/ci/cmake_floor.sh`. Instala `python3-pyyaml`, e o usuário é `builder`.
   A tag é `gusworld-ci-fedora:<sha12 do contexto>`, calculada por
   `tools/container/env_hash.sh`.
2. **Invólucro no host.** `test_command` passa a ser `bash tools/tdd/run_suite.sh`.
   O invólucro mapeia o código de saída: `0` vira `0`, `1` vira `1`, e qualquer
   outro código vira `126`. O tar da árvore de trabalho sai de uma lista branca,
   `tools/tdd/build_paths.txt`, que deixa de fora as quatro áreas cifradas por
   git-crypt.
3. **Contêiner de vida longa, sem volume (opção A).** A árvore entra por
   `docker exec -i` num contêiner `gusworld-tdd-<hash16>`. A opção B seria um
   volume nomeado, não adotada agora.
4. **Dentro do contêiner.** `tools/tdd/in_container.sh` faz a trava por `mkdir`
   (34 s), define `XDG_RUNTIME_DIR` com permissão 700, confere que `ctest -N`
   tem valor maior que zero e roda build e `ctest` (89 s).
5. **Prazos.** `timeout_sec` de 144 s. O trabalho interno cabe em 89 s, e a trava
   em 34 s.
6. **Contêiner frio.** Quando o contêiner não existe, o guard devolve `126`, que
   o runner trata como "não rodou". O aquecimento é feito por
   `tools/tdd/prepare.sh`, em segundo plano.
7. **Hardening.** `--network none`, `--cap-drop ALL`, `no-new-privileges`,
   `--user builder`, e nenhum `-v`.
8. **Aviso visível.** Uma falha de infraestrutura continua caindo no fail-open,
   e passa a chegar ao agente por um hook de aviso (`tools/tdd/notice.py`, via
   `PreToolUse` no `.claude/settings.json` do projeto, com `additionalContext`).
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
  até o aquecimento. A imagem é puxada de `fedora:latest`, e cada rebuild pede
  autorização de novo (L-51).
- **Limites declarados:** edição por Bash escapa do guard. Falha de infraestrutura
  continua em fail-open, agora com aviso. A verificação de que o guard morde de
  verdade é a prova da fatia 6, não presunção.
- **Dependências de outros itens:** o `D1` passa a depender de `C8` e `C10`.
  O `C8` depende do `C3`, o harness de teste próprio (`ADR-025`).

## Cross-refs

- [ADR-025](ADR-025-harness-teste-proprio-minimo.md) (harness de teste próprio)
- [ADR-024](ADR-024-ci-cinco-plataformas-cachyos-container.md) (contêiner Fedora na matriz de CI)
- `../../../GODS_LAWS.md` L-19 (cinco portões), L-35 (teste dinâmico só em contêiner), L-43 (critério pré-registrado), L-51 (instalação pede permissão)
- `TODO.md`, itens `C3`, `C8` e `D1`
