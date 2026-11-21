# Estado e comandos do GusWorld: o detalhe

Referência para agentes e para o líder. O essencial deste projeto (as leis, o índice de gatilhos, as decisões fechadas, a autoridade documental, os comandos e as restrições da máquina) está no [`CLAUDE.md`](../../CLAUDE.md) da raiz; este arquivo guarda o detalhe para o qual ele aponta.

## O que é o GusWorld: o detalhe

### Estrutura de mundo

⚠️ A **forma da conectividade** é do `docs/design/mundo-topologia.md` — 13 áreas em grafo, com dobradiças ligando regiões —, e **não** do corte. O radial sobrevive como **ritmo de missão** no `pillars.md`, não como geografia.

### Escopo da campanha

Nenhum conteúdo se corta por não caber num jogo curto, e a duração só se mede quando houver cena jogável. A cerca contra escopo infinito **não** é a duração total: cada uma das treze áreas e cada dungeon têm orçamento próprio de conteúdo, e mecânica nova entra se couber no orçamento da área onde vive.

## Estado atual do repositório (20/11/2026)

**Git e remoto:**

- `git rev-list --count HEAD` responde quantos commits tem o branch local `main`.
- `git ls-remote git@github.com:petrinhu/GusWorld.git main` responde o SHA do `main` no remoto.
- `git rev-list --count origin/main..HEAD` responde quantos commits locais ainda não foram pushados.
- Existem onze workflows de CI em `.github/workflows/` (medido em 20/11/2026), cada um com o `name:` declarado. `ls .github/workflows/*.yml | wc -l` responde quantos são, e `grep -H '^name:' .github/workflows/*.yml` responde o nome de cada um: `Build` (`build.yml`), `Camadas` (`layers.yml`), `LEI ZERO` (`lei-zero.yml`), `version-tag` (`version-tag.yml`), `Segredo no histórico` (`gitleaks.yml`), `Mensagem de commit` (`commit-gate.yml`), `Análise estática` (`cppcheck.yml`), `Análise estática com clang-tidy` (`clang-tidy.yml`), `Sanitizadores` (`sanitizers.yml`), `Portão de CVE` (`cve.yml`) e `Formatação` (`clang-format.yml`). O `lei-zero.yml` nasceu no commit `c2dc58d` de 28/08/2026, disparado em `push`, `pull_request` e `workflow_dispatch`. Ele roda `tools/security/lei_zero_guard.py` duas vezes (sobre o range do push, para regressão, e sobre a árvore inteira, para dívida) e **já dá à GW-32 (push só com o GHA verde) o que checar**. `gh run list --workflow=lei-zero.yml` responde quantas execuções há e a conclusão de cada uma. ⚠️ **Isto não é o CI de build do projeto**: o build é o workflow `Build`. O LEI ZERO verifica só TEXTO, sem compilar nada.

**Código-fonte: EXISTE desde 11/10/2026.** Medido por comando, não por suposição:

- `find . -name CMakeLists.txt -not -path './.git/*' | wc -l` responde quantos arquivos de build há, `tests/` e os fixtures incluídos.
- `find . -name "*.cpp" -not -path './.git/*' | wc -l` responde quantas unidades de tradução há.
- **A espinha de cinco camadas existe como alvos:** `core/`, `content/`, `domain/`, `app/` e `present/`, cada uma com `CMakeLists.txt` próprio e dependência só para baixo (GW-17). ⚠️ **`present/` nasce DESLIGADA**, atrás da opção `GUSWORLD_PRESENT`, porque a GW-06 e a GW-27 a mantêm bloqueada até o GlintFx ter entrada e texto.
- **`tests/` existe**, com o harness próprio em `tests/harness/` (sem framework de terceiro, ADR-025) e os autotestes em `tests/selftest/`. `grep -cE '^add_test' tests/CMakeLists.txt` responde quantos testes são registrados sem condição. Os testes do `-Werror` e dos sanitizadores ficam dentro de `if(...)` e indentados, então essa conta não os inclui. O total depende das opções de configuração, e o número que vale é o do `ctest -N` sobre o build configurado.
- **Não há `include/` nem `src/`**, e isso é desenho, não falta: o código mora nas cinco camadas.
- **Nada do GlintFx entra ainda.** Nenhum `find_package`, nenhum `add_subdirectory` de framework, nenhum diretório `framework/`. O submódulo entra quando `present/` for desbloqueada, por decisão do líder de 11/10/2026.

**O que existe hoje, fora de código de jogo** (contado por `find`/`ls`):

- Arquivos `.md` na raiz (`ls *.md | wc -l` responde quantos há; inclui o `CLAUDE.md`): os manuais listados em "Autoridade documental" do `CLAUDE.md`, mais `GODS_LAWS.md`, `TODO.md`, `inicial.md`, `README.md`, `sinopse.md`, `CHARS.md`, `PLACES.md`.
- `LICENSE` (AGPL-3.0), `NOTICE`, `REUSE.toml`, `.gitattributes`, `.gitignore`, `.bigtech-porte` (`porte=bigtech`), `LICENSES/` (textos integrais SPDX).
- `docs/`: diretórios de primeiro nível (`art`, `audio`, `book`, `design`, `narrative`, `_processo`, `production`, `qa`, `_secret`, `security`, `specs`, `tech`; `find docs -maxdepth 1 -mindepth 1 -type d | wc -l` responde quantos há; sem o `-mindepth 1` o comando conta também o próprio `docs/`, um a mais) mais `docs/licoes-aprendidas.md` na raiz de `docs/`. `docs/_secret/` é área cifrada por `git-crypt` (GW-25) — só o caminho é citável, nunca o conteúdo.
- `docs/tech/adr/`: ADRs (`ls docs/tech/adr/ADR-*.md | wc -l` responde quantos existem). Os números ausentes estão na linha **Não presentes** (números aposentados) de `docs/tech/adr/_INDEX.md`. ⚠️ **Número de ADR não se reaproveita.**
- `tools/ci/`: os portões de CI atomizados, um diretório por portão (`cve`, `gitleaks`, `commit_gate`, `cppcheck`, `clang_tidy`, `sanitizers`), mais `tools/ci/cmake_floor.sh`, `tools/ci/fetch_verified.sh` e `tools/ci/env/`, um script de toolchain por ambiente de build.
- `tools/security/`: o portão 5 da GW-19 (`tools/security/commit_gate.py`, `tools/security/test_commit_gate.py`, `tools/security/termos_proibidos.txt` cifrado) e os ganchos `tools/git-hooks/commit-msg` e `tools/git-hooks/install.py`.
- `tools/tdd/`, `tools/container/` e `.claude/tdd-guard.json`: o guard de TDD em contêiner, a imagem que ele usa e a configuração dele (`ADR-026`); o uso está em "Comandos" do `CLAUDE.md`.
- `resources/` e `assets/`: existem (`ls` confirma), não inventariados aqui — fora do escopo desta tarefa.

## Comandos: o detalhe

### Resultado do guard de TDD e testes de `tools/tdd/`

- O resultado de cada execução fica em `~/.claude/state/tdd-guard/<hash16>/last-run.json` e em `/var/tmp/gusworld-tdd/<hash16>/last.log`.
- Os testes de `tools/tdd/` rodam com `bash tools/tdd/test/run_in_container.sh`. A variável `GUSWORLD_TEST_PIDS` só aperta o teto de pids desse contêiner, nunca o afrouxa.

### Conferência de tag contra a versão

- `python3 tools/ci/check_version_matches_tag.py vA.B.C.D` confere a tag contra a versão declarada. ⚠️ Ele **reprova em clone raso**, de propósito: num clone raso as tags podem não ter sido buscadas, e um casamento aparente seria falso verde.

### Workflows de CI e provas medidas

**Onze workflows existem** (medido em 20/11/2026; a lista de nomes está na seção de estado). Os que provam o build:

- `build.yml` (`Build`) configura com `cmake -S . -B build -DGUSWORLD_WERROR=ON` e roda `ctest` em nove entradas de matriz (`grep -c '^          - name:' .github/workflows/build.yml` responde quantas são). As plataformas são Fedora (`fedora:latest`), Ubuntu, Arch, CachyOS (`cachyos/cachyos:latest`, job próprio, não é Arch renomeado), cada uma com GCC e Clang, e Windows com MSVC (`windows-latest`). Todas as imagens Linux estão em `:latest`.
- Prova medida: run `44737723604`, `headSha` `e9be1a0`, nove jobs com `conclusion` `success`. Cada job mostra `100% tests passed` sobre `Total Tests: 10`. Esse run não é prova do HEAD atual, que é outro commit.
- `sanitizers.yml` (`Sanitizadores`) configura com `-DGUSWORLD_SANITIZER_FIXTURES=ON`, o que acrescenta os testes dos fixtures de sanitizadores. O total desse job não foi medido aqui.
- `cve.yml` (`Portão de CVE`) é o sexto portão, fora dos cinco da GW-19, e roda só no Fedora (TST-3a). Usa grype `0.122.1` com SHA-256 fixo e barra só CVE crítica com correção publicada. Prova medida: run `44755957346`, `headSha` `89ba42e`, `conclusion` `success`, com veredictos `finding` (caso positivo), `zero` (piso) e `accept` (varredura real), e `VERSION_ID=45` no log.

Os demais workflows (`layers.yml`, `lei-zero.yml`, `version-tag.yml`, `gitleaks.yml`, `commit-gate.yml`, `cppcheck.yml`, `clang-tidy.yml` e `clang-format.yml`) não entram nesta prova. Para o estado deles, use `gh run list --workflow=<arquivo>`. O `Build` é o CI de build, e o `LEI ZERO` verifica só texto, sem compilar nada.

## Teste dinâmico em container (GW-35): o detalhe

⚠️ **Subir o container não basta:** `wl_display_connect(NULL)` cai no nome embutido `wayland-0` dentro do `XDG_RUNTIME_DIR`, e desligar a variável **não protege** — a proteção real é trocar o `XDG_RUNTIME_DIR` por diretório próprio `chmod 700` só no processo sob teste, e provar antes de interagir. O compositor aninhado (`kwin_wayland`, `Xvfb`) continua valendo como **piso dentro** do container, nunca como alternativa a ele. O GusWorld tem código e suíte desde 11/10/2026 (sete testes registrados sem condição, medido em 20/11/2026 por `grep -cE '^add_test' tests/CMakeLists.txt`), e a suíte roda no CI, não na sessão do líder. A camada `present/` continua bloqueada pelo GlintFx (GW-06, GW-27), e a lei foi escrita **antes** do primeiro teste, de propósito.

## Pendências

A tabela de pendências do projeto está em `TODO.md` na raiz (`grep -cE '^\| [0-9]' TODO.md` responde quantos itens há), em schema de **10 colunas** (`WSJF`, `ID`, `Onda`, `Grupo`, `Descrição Técnica`, `Prioridade`, `Pré-requisito`, `Dificuldade`, `Status`, `Estado Auditado`, com `WSJF` como primeira coluna, GW-30). A coluna `Onda` vai de `0` até o maior valor em uso, mais `—` para item que não entra em fila de execução: bloqueado sem data, ou prática contínua que por desenho nunca fecha, como o `A11b`. `awk -F'|' '/^\| [0-9]/{gsub(/^[ \t]+|[ \t]+$/,"",$4); print $4}' TODO.md | grep -E '^[0-9]+$' | sort -n | tail -1` responde o maior valor de `Onda` hoje; `awk -F'|' '/^\| [0-9]/{gsub(/^[ \t]+|[ \t]+$/,"",$4); print $4}' TODO.md | sort -u | wc -l` responde quantos valores distintos a coluna tem (contando o `—`).

Não há mais nenhum status `🟡 Parcial`: o único era o `A11`, que em 25/08/2026 foi **fatiado em `A11a` e `A11b`** por decisão do líder sob a GW-33 — ele misturava o gabarito SPDX, que fecha uma vez, com a prática de aplicá-lo, que nunca fecha, e nessa forma travava o grafo do primeiro commit.

O rastreio dos 28 itens do `inicial.md` — condição da GW-21 para apagá-lo — vive dentro do próprio `TODO.md`, não duplicado aqui.

## Bus entre projetos: `gusworld_ia_autocomm`

Canal assíncrono entre as sessões do líder e o filho dele. Clone: `<vault>/gusworld_ia_autocomm/`, repo **privado** `petrinhu/gusworld_ia_autocomm`. **O protocolo canônico é o `PROTOCOL.md` do clone**; leia lá antes de usar, e trate a GW-07 e a GW-31 de `GODS_LAWS.md` como o resumo operacional.

| Slug | Projeto |
|---|---|
| **`gusworld`** | **este projeto**, o jogo |
| `glintfx` | `Projects/GlintFx`, o framework do qual o GusWorld depende (LEI ZERO) |
| `site` | `petrinhu/site_gusworld`, a revista retro que conta o progresso do jogo |
| `mapeditor` | `petrinhu/gusworld_mapeditor`, editor de mapas |
| Gus Dragon (`Dragon-Drv`) | colaborador humano, filho do líder; manda ideias por issue, `.txt` no `inbox/` ou discussion (GW-07, GW-16) |

Fluxo: ler o que está solto em `inbox/gusworld/`, agir, `git mv` para `inbox/gusworld/archive/`, commit `read: <arquivo>`, push. Enviar: um `.md` em `inbox/<destinatario>/` com frontmatter `de`/`para`/`assunto`/`data`, sempre depois de `git pull`. **Pedido vai sem classificação de prioridade** (quem recebe classifica).

Estado em 24/08/2026 (`ls inbox/gusworld/`): **vazia**, só `archive/` e `.gitkeep`.

**Aviso proativo ao Gus Dragon é obrigação permanente (GW-31):** sempre que o líder aprovar, rejeitar ou mudar algo que seja ideia dele, ou fechar item de alta prioridade da tabela, avisar **sem que ele pergunte** — na discussion 7 (o catálogo de bugs que ele mantém), com timestamp e uma das três classificações que ele fixou (**Bug Consertado**, **Bug Funcional**, **Bug Possível**).

---

**Última revisão:** 2026-11-20. Decisão do líder por `AskUserQuestion` (opção "Cortar para ponteiros"): o `CLAUDE.md` da raiz fica com o essencial, e o detalhe mora neste arquivo.
