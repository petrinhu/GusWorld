# Pipeline: Da Ideia ao Release 1.0 do GusWorld

> Fases do pipeline aplicáveis a um **jogo FOSS single-player offline** (LEI ZERO do `GODS_LAWS.md`: liga só em GlintFx e no sistema operacional). Sem backend, sem API, sem mobile, sem conta de usuário, sem monetização. Ancorado na **GW-10**: `main` só orquestra; C-level audita e arquiteta sempre com modelo `fable`; agente operacional implementa sempre com `sonnet`. Hub: `Standards.md`.

---

## Visão geral do pipeline

```mermaid
flowchart TD
    F0["0. Ideação"] --> F1["1. Discovery"] --> F2["2. Definição"] --> F3["3. Design"] --> F4["4. Arquitetura"] --> F5["5. Setup Eng"]
    F5 --> F6["6. Desenvolvimento\n(camadas da GW-17)"]
    F6 --> F7["7. QA"]
    F6 --> F8["8. Proteção de dado e licença"]
    F7 --> F9["9. Beta"]
    F8 --> F9
    F9 --> F10["10. Comunidade"] --> F11["11. Release 1.0"] --> F12["12. Pós"]
    F12 -. aprendizado .-> F1
```

Cada fase tem loop interno (uma descoberta volta ao Discovery, um bug volta ao Dev). O pipeline é iterativo dentro de cada fase, sequencial entre macro-fases.

### Mapa fase, C-level e agent

| Fase | C-level (`fable`) | Agents operacionais (`sonnet`) |
|---|---|---|
| 0. Ideação | Celso (CEO) | - (resolvida: `inicial.md` + `GODS_LAWS.md`) |
| 1. Discovery | Caetano (CTO) | `software-architect` (canon já existe em `docs/narrative/`) |
| 2. Definição | Caetano (CTO) | `software-architect` |
| 3. Design | Caetano (CTO) | - (interface só nasce com o GlintFx, GW-27) |
| 4. Arquitetura | Caetano (CTO) | `software-architect`, `security-engineer` |
| 5. Setup Eng | Caetano (CTO) | `devops-sre`, `tech-lead` |
| 6. Desenvolvimento | Caetano (CTO) | `backend-engineer` (C++23) |
| 7. QA | Caetano (CTO) | `qa-engineer` |
| 8. Proteção de dado e licença | Narciso (CISO) + Cláudio (CLO) | `security-engineer`, `compliance-legal` |
| 9. Beta | Caetano (CTO) | `qa-engineer`, `technical-writer` |
| 10. Comunidade | Celso (CEO) | `technical-writer` (comunicação via bus, GW-07) |
| 11. Release 1.0 | Celso (CEO) coordena | `devops-sre`, `tech-lead`, `qa-engineer` |
| 12. Pós | Caetano (CTO) | `qa-engineer`, `technical-writer` |

Antes de iniciar uma fase nova, o Cósimo (Chief of Staff) confirma que toda fase tem C-level nomeado e que nenhum C-level fica dormente por presunção de agente: o porte é COMPLETO e a constelação inteira tem mandato neste projeto (ver `ORG.md` secao 2). A tabela acima lista o C-level tecnicamente responsável por cada fase; os demais C-levels (Capitolino, Cosmo, Camilo, Cândido, Caio, Confúcio, Cícero) têm mandato próprio, descrito no `ORG.md` secao 2, mesmo quando não lideram uma fase específica. Quando uma fase ou um mandato parecer não se aplicar, o caso vai ao líder por `AskUserQuestion`, nunca vira corte silencioso.

---

## Fase 0: Ideação

Já concluída. A ideia, a hipótese de valor e o escopo do MVP estão no `inicial.md` (documento fundador, 28 itens) e nas leis do `GODS_LAWS.md`. Não se reabre sem o líder.

---

## Fase 1: Discovery

Para um jogo FOSS de autor, discovery não é pesquisa de mercado: é o **canon já escrito**. O corpus em `docs/narrative/`, `docs/design/`, `sinopse.md`, `CHARS.md` e `PLACES.md` é o insumo (GW-01: vale para lore e design, não para código). Onde o canon contradiz uma lei vigente, o trabalho que depende dele **para** até o líder atualizar o canon (GW-13).

**Entregável:** lista de contradições de canon abertas (ver GW-13), atualizada conforme aparecem.

---

## Fase 2: Definição

O equivalente de PRD é a soma de `inicial.md` + `GODS_LAWS.md` + os documentos de design em `docs/design/`. Escopo do "MVP": núcleo de regra de jogo jogável por replay determinístico (GW-17), antes mesmo de existir tela (GW-06).

**Métricas de sucesso não são de aquisição/retenção** (não há usuário externo nem telemetria, GW-08): são de **qualidade técnica**, os cinco portões da GW-19, e de fidelidade ao canon.

---

## Fase 3: Design (jogo, não UI)

Design de sistema de jogo (combate, deck, mapa, progressão) deriva da prosa do canon (GW-01) e das mecânicas em `docs/design/mecanicas/`. **Design de interface não é escrito ainda**: nenhuma marcação, folha de estilo ou tela piloto nasce antes de o GlintFx traduzir marcação (GW-27). Perspectiva fixada: 3/4 top-down, quatro direções cardeais, câmera que não gira (GW-26).

---

## Fase 4: Arquitetura Técnica

**Stack:** C++23 (GW-03), zero dependência além do GlintFx e do sistema operacional (LEI ZERO).

**Padrão arquitetural:** espinha de cinco camadas com dependência só para baixo e gate de CI (`core/ → content/ → domain/ → app/ → present/`), núcleo determinístico por comando e evento (GW-17). Nada de monólito (GW-04), nada de dublê de plataforma (GW-05).

**Modelagem de dado:** POCOs de domínio, serializados em envelope binário próprio, nunca texto (GW-18, GW-25).

**Segurança by design:** Narciso (CISO) desenha a proteção de save/config/mapa/catálogo (GW-25); primitivas criptográficas vêm só do GlintFx.

**ADRs:** decisão de arquitetura registrada como lei no `GODS_LAWS.md` quando vem do líder; documento técnico de suporte fica em `docs/design/`.

---

## Fase 5: Setup de Engenharia

**Repositório:** `git`, Conventional Commits, mensagem em pt-br (GW-22). `.gitignore`/`.gitattributes` escritos antes do primeiro commit (GW-15): `resources/livros/` e `resources/glb/` fora do git, Git LFS para o resto do binário pesado.

**CI:** matriz de cinco plataformas desde o primeiro commit (GW-09, GW-20): Fedora 44 pinado (primário), Ubuntu, Arch, CachyOS, Windows. Fundação de build nasce legitimamente sem TDD; a partir do primeiro módulo de comportamento, não há mais essa saída (GW-19, GW-20).

**Observabilidade:** não aplicável (sem serviço em produção, sem telemetria).

---

## Fase 6: Desenvolvimento

Sem trilhas frontend/backend/mobile: **uma trilha só**, organizada pelas cinco camadas da GW-17. Núcleo de regra (`core/`, `content/`, `domain/`) nasce primeiro, em TDD estrito (GW-06, GW-19); `app/` e `present/` só quando o GlintFx tiver janela, contexto gráfico, entrada e texto.

**Consequência assumida (GW-06):** enquanto isso, o jogo não roda, não tem tela, não tem demo. O que existe é suíte de teste verde e núcleo provado. Relatar progresso visual inexistente é falta grave.

**Agent:** `backend-engineer` (C++23), sempre `sonnet`, sob orquestração de Caetano (CTO, `fable`).

---

## Fase 7: QA e Testes

TDD estrito a partir do primeiro módulo com comportamento (GW-19): vermelho, verde, refatorar, sem pular etapa. Suíte de replay determinístico (semente + comandos reproduz o mesmo estado, GW-17) como teste permanente. Detalhe completo em `TESTES.md`.

**Sem meta numérica de cobertura** (GW-19): cobertura é consequência do TDD, não alvo.

---

## Fase 8: Proteção de Dado e Licença

Substitui "Segurança e Compliance" genérico: não há LGPD/GDPR/HIPAA aplicável (sem dado de usuário coletado, jogo offline, GW-08). O que existe:

- **Proteção de save/config/mapa/catálogo** (GW-18, GW-25): envelope binário, cifra autenticada, validador semântico, sem prometer "impossível editar".
- **Licenciamento** (GW-08): código AGPL-3.0-or-later com ressalva de uso offline; assets e lore com todos os direitos reservados; marcação REUSE/SPDX desde o primeiro commit.

**Liderança:** Narciso (CISO) na proteção de dado, Cláudio (CLO) no licenciamento, os dois sempre `fable`.

---

## Fase 9: Beta

Playtesting interno (o próprio líder e, quando aplicável, o filho dele como "Gus Dragon", GW-16) substitui o beta fechado de usuários externos. Feedback do Gus Dragon chega pelo bus (GW-07), nunca por canal de suporte comercial.

**Documentação para o jogador:** `README` com a ressalva de uso offline (GW-08), changelog público por release (GW-23).

---

## Fase 10: Comunidade

Substitui GTM comercial: sem pricing, sem funil pago, sem ASO. O que existe é comunicação simples de projeto FOSS (release notes no GitHub, resposta a issue) e o pipe de ideia do Gus Dragon (GW-07: absorver, ack automático, discutir viabilidade com o líder, postar resultado honesto, arquivar).

---

## Fase 11: Release 1.0

**Versionamento:** `vA.B.C.D`, mesma convenção do GlintFx (GW-23). Tag e release exigem aval explícito do líder no contexto, sempre (GW-23).

Detalhe completo do checklist de release irreversível em `DEPLOY_CHECKLIST.md`.

**Liderança:** Celso (CEO) coordena o go/no-go; Caetano (CTO) confirma que os portões da GW-19 estão fechados antes da tag, e o `internal-auditor` confirma o dossiê de auditoria (`AUDITORIAS.md`) só quando a release toca segurança, privacidade ou deploy irreversível (L-35 global).

---

## Fase 12: Pós-Lançamento

Estabilização por bug reportado (via bus com Gus Dragon ou issue pública), sem hypercare de produção porque não há serviço rodando. Planejamento do próximo `A.B` com o líder. Auditoria periódica conforme `AUDITORIAS.md`.

---

## Anti-padrões que este pipeline recusa

1. Escrever camada `present/` antes do GlintFx existir (GW-06, GW-27).
2. Dublê de plataforma: mock, stub ou `#ifdef` fingindo o GlintFx (GW-05).
3. Meta numérica de cobertura de teste (GW-19).
4. Prometer "impossível editar" em comunicação pública (GW-25).
5. Aplicar pipeline comercial (GTM pago, LGPD, mobile store) a um jogo FOSS single-player sem esses domínios.
6. Deixar C-level dormente por presunção de agente: o porte é COMPLETO (`ORG.md` secao 2) e a constelação inteira tem mandato aqui; quando uma fase parecer não se aplicar a um C-level, o caso vai ao líder por `AskUserQuestion`, nunca vira corte silencioso.
