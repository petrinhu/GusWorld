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

O GusWorld é jogo FOSS single-player offline, sem servidor, sem conta de usuário, sem receita, sem dado de terceiro coletado. Isso dispensa boa parte da constelação comercial. Ativos e com trabalho real a fazer:

| Agent | Cargo | Onde entra no GusWorld |
|---|---|---|
| **Caetano** | CTO | Arquitetura de camadas (L-17), decisões de stack, CI de 5 plataformas (L-09/L-20), delega `software-architect`, `tech-lead`, `backend-engineer`, `devops-sre`, `qa-engineer` |
| **Narciso** | CISO | Proteção de save/config/mapa/catálogo (L-18, L-25), via `security-engineer` |
| **Cláudio** | CLO | Licenciamento AGPL + REUSE/SPDX, regime de asset e do catálogo fatiado por natureza jurídica (L-08, L-25), via `compliance-legal` |
| **Celso** | CEO | Arbitragem entre os demais quando um trade-off atravessa domínios; leva ao líder |
| **Cósimo** | Chief of Staff | Classifica o porte do projeto e evita ativar C-level sem trabalho real (anti over-engineering) |

**Dormentes por ora, sem trabalho real hoje neste projeto:** Capitolino (CPO, produto/GTM comercial), Camilo (CMO, marketing pago), Cosmo (COO, operação multi-time), Cândido (CDO, dado como ativo de negócio), Caio (CAIO, IA como capability do produto), Confúcio (CFO, orçamento/receita), Cícero (CRO, vendas B2B). Nenhum deles é apagado da constelação global; simplesmente não têm mandato aqui até o líder decidir o contrário.

O `internal-auditor` (agent operacional, mas com mandato de dossiê formal) monta o dossiê de auditoria da L-19 antes da 1.0, reportando a Caetano, Narciso e Cláudio conforme o capítulo.

---

## 3. Agents operacionais usados neste projeto

`software-architect`, `tech-lead`, `backend-engineer` (C++23), `qa-engineer`, `security-engineer`, `compliance-legal`, `internal-auditor`, `devops-sre` (CI de 5 plataformas), `technical-writer` (documentação do projeto). `game-producer` e `scrum-master` apoiam a cadência de sprint (ver `AGILE.md`) quando o líder quiser cerimônia formal.

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
