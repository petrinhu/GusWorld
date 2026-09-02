# ORG.md - Constelação de Agents no GusWorld

> A constelação bigtech (C-levels + agents operacionais, em `~/.claude/agents/`) é infraestrutura global, já provisionada. Este manual descreve só **como o GusWorld usa esse recorte**, sob a **L-10** do `GODS_LAWS.md`. Onde este manual e a L-10 divergirem, a L-10 vence. Hub: `Standards.md`. Manuais irmãos: `CONTRACT.md`, `TESTES.md`, `AGILE.md`, `DEPLOY_CHECKLIST.md`, `AUDITORIAS.md`, `TOOLING.md`.

---

## 0. Autoridade suprema

**O líder é soberano deste projeto.** A constelação propõe e executa, mas a palavra final é sempre dele. Decisão de arquitetura, escopo, stack, go/no-go, deploy irreversível e qualquer escolha difícil de reverter passa por `AskUserQuestion`, com 2 a 3 alternativas e a recomendada primeiro (L-11). Nenhum agente decide nada sozinho.

---

## 1. Como este projeto usa a constelação (L-10)

**Verbatim da lei:** *"main apenas orquestra, interage comigo e dispara agentes. Auditorias apenas com clevel bigtech fable, trabalhadores sonnet bigtech."*

| Tipo de trabalho | Agente | Modelo |
|---|---|---|
| Auditoria | C-level da constelação bigtech | **`fable`, sempre** |
| Criação de projeto e arquitetura | C-level da constelação bigtech | **`fable`, sempre** |
| Implementação | agente operacional bigtech | **`sonnet`** |

**O que continua sendo do `main`:** decidir o que delegar e em que ordem; escrever a ordem de serviço; **re-verificar o entregável** (relatório de agente não é prova); levar decisão ao líder; falar com o líder.

**Papéis distintos:** quem implementa não é quem revisa, e nenhum dos dois é o `main` que re-verifica.

---

## 2. C-levels ativos neste projeto

O GusWorld é jogo FOSS single-player offline, freeware (`README.md`, seção "Apoie o projeto"), sem servidor, sem conta de usuário, sem telemetria e sem receita. **O porte é COMPLETO por decisão do líder em 21/08/2026 (L-10, `GODS_LAWS.md`): a constelação inteira tem mandato aqui.** O que muda de um produto comercial para este jogo é o conteúdo do mandato de cada C-level, descrito na tabela abaixo. Quando uma fase ou um mandato parecer não se aplicar, o caso vai ao líder por `AskUserQuestion` (L-11); nunca vira corte silencioso. Onde um agente delegado tem fronteira imposta por lei (LEI ZERO, L-02, L-06, L-27), a fronteira está anotada na célula.

| Agent | Cargo | Onde entra no GusWorld |
|---|---|---|
| **Celso** | CEO | Arbitragem entre os demais quando um trade-off atravessa domínios; coordena o go/no-go da release 1.0 (`REL-1` do `TODO.md`, L-23); leva toda decisão ao líder (L-11) |
| **Capitolino** | CPO | Produto deste projeto é o design do jogo: GDD e pilares (`docs/design/gdd.md`, `docs/design/pillars.md`), mecânicas, cartas, economia, níveis, narrativa e onboarding, dentro da cerca da L-29. Já assina as análises de pacing e os protocolos de simulação em `docs/design/mecanicas/` (ex.: `analise-pacing-fase-a.md:4`). Delega `lead-game-designer`, `economy-designer`, `level-designer`, `narrative-designer`, `narrative-writer`, `revisor-textual`, `ux-writer`, `art-director`, `audio-designer-composer`, `game-animator` e `3d-artist-rigger` (este último só a pedido nominal do líder: o pipeline 3D é dele, L-02). Nenhuma interface ou HUD antes de o GlintFx traduzir marcação (L-27) |
| **Caetano** | CTO | Arquitetura de camadas (L-17), decisões de stack (L-03), CI de 5 plataformas (L-09/L-20), os cinco portões da L-19 e o pedido de função ao GlintFx pelo bus (L-07). Delega `software-architect`, `tech-lead`, `backend-engineer` (C++23), `devops-sre`, `qa-engineer` e `engine-graphics-programmer` (este só para redigir pedido técnico ao GlintFx pelo bus; render, janela e laço são do framework, LEI ZERO) |
| **Cosmo** | COO | Cadência das ondas do `TODO.md` (limite de WIP, um build pesado por vez), sincronia dos quatro repositórios ligados pelo bus (`gusworld`, `glintfx`, `site`, `mapeditor`), o bloqueio sem data do GlintFx como risco operacional número um, e o ritmo da fase 11. Já consolidou a reordenação de 25/08/2026 (`TODO.md:64`). Delega `game-producer`, `scrum-master` e `release-manager` |
| **Camilo** | CMO | Comunicação pública do projeto FOSS, sem marketing pago: a revista retro `petrinhu/site_gusworld` (slug `site` do bus), release notes por versão (L-23), a política de issues e a seção de apoio do `README.md`, `OFFLINE-NOTICE.md` e `AI-DISCLOSURE.md` como texto público, e o canal com o Gus Dragon na discussion 7 (L-31). A distribuição em loja (Steam, Steam Deck) é decisão do líder adiada pela L-29 (`C-10` e a nota do `C-08`). Delega `community-manager`, `pr-comms`, `content-seo` e `product-marketing-manager` |
| **Narciso** | CISO | Proteção de save/config/mapa/catálogo (L-18, L-25), portões 4 e 5 da L-19 (`gitleaks`, `commit_gate.py`), via `security-engineer` |
| **Cândido** | CDO | O jogo não coleta dado (sem telemetria, `README.md`, seção "Licença"). O dado que é ativo aqui é o de simulação e balanceamento: harness de auto-resolve (`D14`), calibração por simulação do ato final (`D31`, ordem do líder), análises de pacing com artefato congelado e critério pré-registrado antes do dado existir. Governa reprodutibilidade (sorteio contado, L-17), congelamento e pré-registro. Delega `data-scientist` e, em conjunto com Capitolino, `economy-designer` |
| **Caio** | CAIO | IA como capability do produto não existe (jogo offline, sem modelo embarcado). O mandato aqui é IA na produção: o `AI-DISCLOSURE.md` bilíngue, a frota de agents que trabalha no projeto (frontmatter atualizado contra o canon, L-13), o tiering `fable`/`sonnet` da L-10 e a governança de uso de modelo. Delega `applied-ai-engineer` |
| **Confúcio** | CFO | Sem receita, mas com custo real: tokens de IA (`README.md`, seção "Apoie o projeto"), quota de Git LFS para os 1266 binários do `B8` (L-15), minutos de CI de cinco plataformas (L-09/L-20) e as ferramentas do pipeline de arte listadas em `AI-DISCLOSURE.md`. Lente de custo antes de decisão de infra; destino das doações. Delega `devops-sre` (custo de CI e LFS). Estimativa de tempo continua proibida (L-08 global); custo em dinheiro e em quota é outra coisa |
| **Cícero** | CRO | Decisão do líder em 02/09/2026: ativo, sem tarefa despachada, até a distribuição virar real. O jogo é freeware sem intenção comercial (`README.md`) e o `C-14` (L-29) corta conteúdo pago, e nenhum trabalho de receita foi encontrado no corpus. Qualquer proposta de distribuição em loja passa por ele em conjunto com Camilo |
| **Cláudio** | CLO | Licenciamento AGPL + REUSE/SPDX, regime de asset e do catálogo fatiado por natureza jurídica (L-08, L-25), homenagem e nome de menor (L-16), via `compliance-legal` e `internal-auditor` |
| **Cósimo** | Chief of Staff | O porte está classificado pelo líder como COMPLETO (L-10) e não se reclassifica por agente. Confirma, a cada onda, que toda fase tem C-level nomeado e que nenhum ficou dormente por presunção; quando uma fase parecer não se aplicar, leva ao líder por `AskUserQuestion` (L-11); mantém este manual e o mapa de fases de `pipeline_release_1.0.md` coerentes com a lei |

O `internal-auditor` (agent operacional, mas com mandato de dossiê formal) monta o dossiê de auditoria da L-19 antes da 1.0, reportando a Caetano, Narciso e Cláudio conforme o capítulo.

---

## 3. Agents operacionais usados neste projeto

Por domínio, com o C-level que delega entre parênteses. Todo agente abaixo trabalha sob a L-10 (`sonnet` na implementação) e recebe no prompt o caminho absoluto de `GODS_LAWS.md` e as leis cujo gatilho casa com a tarefa.

**Engenharia (Caetano):** `software-architect`, `tech-lead`, `backend-engineer` (C++23), `devops-sre` (CI de 5 plataformas), `qa-engineer`. `engine-graphics-programmer` só redige pedido técnico ao GlintFx pelo bus (L-07); render, janela e laço são do framework (LEI ZERO). `gameplay_engineer` implementa mecânica de jogo na camada `app/`, consumindo POCOs e serviços do `backend-engineer`.

**Design de jogo (Capitolino):** `lead-game-designer` (regras, core loop, balanceamento, GDD), `economy-designer` (fontes, ralos, crafting e curvas; compartilhado com Cândido no dado de simulação), `level-designer` (topologia das 13 áreas e das dungeons, `docs/design/mundo-topologia.md`, L-26).

**Narrativa (Capitolino):** `narrative-designer` (arquitetura: beats, árvore de diálogo, lore-spec), `narrative-writer` (prosa e verso finais, inclusive os dois livros-companheiros e o Diário), `revisor-textual` (auditoria de texto contra `TEXTREVIEW.md`, `CHARS.md`, `PLACES.md`), `ux-writer` (microcópia, só quando o GlintFx traduzir marcação, L-27). Designer não escreve prosa polida e writer não projeta esqueleto; o fluxo é designer, aprovação do líder, handoff, writer. Conteúdo aprovado do Gus original só muda com nova autorização do líder (L-37).

**Arte e áudio (Capitolino):** `art-director` (`docs/art/style-guide.md`, pixel-art 2D, L-02), `game-animator` (quatro direções cardeais desenhadas à mão, sem espelhamento, L-26), `audio-designer-composer` (sem voz nenhuma no jogo, `C-04` da L-29), `3d-artist-rigger` (só a pedido nominal do líder: o pipeline 3D é dele, L-02).

**Produção e cadência (Cosmo):** `game-producer`, `scrum-master` (ver `AGILE.md`), `release-manager` (fase 11, `REL-1`, sempre com aval explícito do líder para tag, L-23).

**Comunidade e comunicação (Camilo):** `community-manager` (bus, issues, discussion 7 com o Gus Dragon, L-31 e L-34), `pr-comms` (release notes, texto público), `content-seo` (revista `site_gusworld`), `product-marketing-manager` (posicionamento do projeto FOSS, sem pricing).

**Dado de simulação (Cândido):** `data-scientist` (análise estatística dos harnesses `D14` e `D31`, critério fixado antes do dado existir).

**IA na produção (Caio):** `applied-ai-engineer`.

**Proteção, conformidade e auditoria (Narciso e Cláudio):** `security-engineer`, `compliance-legal`, `internal-auditor` (dossiê da L-19, `AUD-5`).

**Documentação (Caetano, com Camilo no texto público):** `technical-writer` (documentação do projeto, wiki `H1` e guia para iniciante `H2`, nunca inline).

---

## 4. Política de ferramentas dos agents

Regra para o campo `tools:` de qualquer agent que trabalhe neste projeto.

| Ferramenta | Quem recebe |
|---|---|
| **Read** | Todos, sempre. |
| **Grep, Glob** | Todos, sempre (navegação e busca). |
| **Write** | Quem **produz artefato** (código, doc, relatório de auditoria). |
| **Edit** | Quem **mantém/revisa artefato existente** in-place. |
| **Bash** | Quem **executa** (build, teste, CI local, diagnóstico). |
| **WebFetch, WebSearch** | Quem **pesquisa fonte externa** (ex.: Narciso pesquisando mecanismo anti-adulteração para a L-18/L-25). |
| **TodoWrite** | Quem **planeja tarefa multi-passo**. |

**Exceção read-only documentada:** agente de diagnóstico puro (nunca corrige/instala/edita) fica sem Write e sem Edit, por desenho.
