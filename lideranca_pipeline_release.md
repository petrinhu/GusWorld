# Quem Lidera o Pipeline de Release do GusWorld

> Resposta curta: **nenhum C-level único lidera o pipeline inteiro**, mas neste projeto a maioria dos C-levels fica dormente (ver `ORG.md` secao 2). Caetano (CTO) lidera a engenharia na prática; Celso (CEO) coordena e arbitra; Narciso (CISO) e Cláudio (CLO) entram nas fases de proteção de dado e licença. Este documento mapeia a teoria de liderança C-level e como ela se aplica **especificamente ao GusWorld**, sob a **L-10** do `GODS_LAWS.md`.

---

## 1. Divisão de domínio, aplicada a um jogo FOSS single-player

O GusWorld não tem os três domínios clássicos de produto comercial (o quê construir para o mercado, como construir, como vender) na mesma proporção: não há venda, não há mercado a validar, não há usuário externo com dado a proteger além do próprio jogador local. O que resta:

| C-Level | Agent | Pergunta que responde neste projeto | Fases (`pipeline_release_1.0.md`) |
|---|---|---|---|
| **CTO** | Caetano | Como construir o jogo tecnicamente, sobre o GlintFx? | 1, 2, 4, 5, 6, 7, 9, 12 |
| **CISO** | Narciso | Como proteger save, config, mapa e catálogo? | 4, 8 |
| **CLO** | Cláudio | Como licenciar código, asset e catálogo? | 8 |
| **CEO** | Celso | Como alinhar as três decisões acima e arbitrar trade-off entre elas? | 0, 10, 11 |
| **Chief of Staff** | Cósimo | Este projeto ainda justifica cada C-level ativo? | transversal |

**Dormentes neste projeto** (sem produto comercial, sem GTM pago, sem dado como ativo de negócio, sem IA como capability, sem receita B2B): Capitolino (CPO), Camilo (CMO), Cosmo (COO), Cândido (CDO), Caio (CAIO), Confúcio (CFO), Cícero (CRO). Detalhe e motivo de cada um em `ORG.md` secao 2.

---

## 2. Por que o CIO não lidera aqui, também

A confusão entre CTO e CIO segue não se aplicando: o GusWorld é produto que se distribui a um jogador (mesmo sem venda), não sistema interno de empresa. **Caetano (CTO)** segue sendo a liderança técnica certa; não há agent CIO na constelação.

---

## 3. RACI simplificado (fases do GusWorld)

R = Responsável, A = Aprovador, C = Consultado, I = Informado. Mapa de fase em `pipeline_release_1.0.md`.

| Fase | Responsável | Apoio |
|---|---|---|
| 0. Ideação | Celso (CEO) | - (já resolvida no `inicial.md`) |
| 1. Discovery (canon) | Caetano (CTO) | - |
| 2. Definição | Caetano (CTO) | - |
| 3. Design (jogo, não UI) | Caetano (CTO) | - |
| 4. Arquitetura | Caetano (CTO) | Narciso (CISO) |
| 5. Setup Eng | Caetano (CTO) | - |
| 6. Desenvolvimento | Caetano (CTO) | - |
| 7. QA | Caetano (CTO) | - |
| 8. Proteção de dado e licença | Narciso (CISO) / Cláudio (CLO) | - |
| 9. Beta (playtesting interno) | Caetano (CTO) | - |
| 10. Comunidade | Celso (CEO) | - |
| 11. Release 1.0 | Celso coordena, Caetano executa | `internal-auditor` |
| 12. Pós | Caetano (CTO) | - |

Em toda linha, o trabalho de C-level é sempre `fable` e a implementação é sempre agente operacional `sonnet` (L-10).

---

## 4. Anti-padrões de liderança, aplicados aqui

1. **Ativar C-level sem trabalho real** (Camilo, Cícero, Confúcio, Cândido, Caio) só porque existem na constelação global. Cósimo previne.
2. **CTO decidindo proteção de dado sozinho**, sem Narciso: a L-25 exige o CISO no desenho.
3. **Licenciamento decidido sem Cláudio**: a L-08 é jurídica por natureza (REUSE/SPDX, regime de asset, catálogo fatiado).
4. **`main` executando trabalho de produto** em vez de orquestrar: viola a L-10 diretamente.
5. **Relatório de agente aceito sem re-verificação** do `main`: viola a L-10 ("relatório de agente não é prova").

---

## 5. Resumo executivo

> Neste projeto, a pergunta "quem lidera?" tem resposta curta:
>
> 1. **Estrategicamente:** Celso (CEO), que arbitra entre engenharia (Caetano), proteção de dado (Narciso) e licenciamento (Cláudio).
> 2. **Operacionalmente:** Caetano (CTO) lidera quase todas as fases, porque o GusWorld é, na prática, um projeto de engenharia com dois capítulos jurídico-técnicos (Narciso, Cláudio) e um coordenador (Celso).
> 3. **A palavra final é sempre do líder** (petrus), em qualquer fase, via `AskUserQuestion` (L-11).
