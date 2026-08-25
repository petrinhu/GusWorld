<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

> **RESGATADO em 25/08/2026 do `gusworld_legacy` pelo `technical-writer`, na varredura profunda de narrativa. ACHADOS PARCIALMENTE VERIFICADOS, conferidos contra a árvore atual em 25/08/2026.** Este dossiê (`F5-BK.AUDIT.FULL T3 v2`, 22/05/2026) lista 4 críticos residuais que sobreviveram à cascata da v1 (ver `AUDIT-T3-CRONOLOGIA.md`), todos sobre a idade de Bento na contaminação por Patch-Zero na Catedral Atelaiá (canon: 7 anos, não 4) e sobre datações ambíguas de Verônica e Sonja.
>
> **O que este agente confirmou nas fontes primárias citadas pelo próprio dossiê** (`factions.md`, `characters/bento-requiem.md`, `in-world-docs.md`, `foreshadowing.md`, `lore-bible.md`): **todas resolvidas**. `factions.md:223` diz "Bento (7) sobrevive"; `characters/bento-requiem.md:29` e `:97` dizem "aos 7 anos (canônico -7)"; `foreshadowing.md` tem **zero** ocorrências de "Bento 4 anos"; `lore-bible.md:570` diz "morta em -35, aos 43 anos" (a datação ambígua de Verônica já não existe).
>
> **Os quatro arquivos `diary/*` que o próprio dossiê sinalizava como não-checados em 30/05/2026 foram conferidos agora, por `grep`:** `diary/foreshadow-links.md` e `diary/entries-fichas-bestiary.md` têm **zero** ocorrência de "4 anos" ligada a Bento — limpos. `diary/entries-mapas-timeline.md:523` diz "Bento (7) sobrevive" — **resolvido**. `diary/entries-manuscrito-glossario.md:425` diz "Bento treinou desde os 4 anos" (aplicado em Patch-Zero aos 7) — **isto pode ser informação distinta e legítima** (idade de início de treino, não a idade no incidente da Catedral Atelaiá) ou pode ser o resíduo do drift original; este agente não decide qual das duas — fica registrado como **achado a confirmar com o líder**, não como conserto pendente óbvio.
>
> **O que segue é o texto original do legacy, sem edição de conteúdo.**

---

# F5-BK.AUDIT.FULL T3 v2 — CRONOLOGIA CROSS-DOC

**Data:** 2026-05-22 (refazer com TEXTREVIEW v2)
**Issues totais:** 17 (4 CRÍTICOS reais + 9 MÉDIOS + 4 LEVES)
**Eficácia cascata T3 v1:** ~80% (16 críticos cobertos, 4 residuais detectados v2)

---

## STATUS DE CASCATA (revisão F1-DL.TRACKER-CLOSE 2026-05-30)

> Marcador de bookkeeping. Verificação contra o canon central. NÃO altera o canon. Forte cascata aplicada **após** este AUDIT (2026-05-22).

| Crítico | Status no canon | Verificação 2026-05-30 |
|---|---|---|
| CR-T3v2-01 Bento "aos 4 anos" cross-doc (8 docs) | **CASCATEADO (fontes primárias)** | `factions.md:223` agora "Bento (7) sobrevive"; `characters/bento-requiem.md:29+97` "aos 7 anos (canônico -7)"; `in-world-docs.md:128` subtexto "Bento aos 7 anos"; `foreshadowing.md` 0 ocorrências de "Bento 4 anos". **Não exaustivamente verificados nesta passada:** `diary/foreshadow-links.md`, `diary/entries-mapas-timeline.md`, `diary/entries-fichas-bestiary.md`, `diary/entries-manuscrito-glossario.md` (diary deep). |
| CR-T3v2-02 Verônica morte lore-bible:570 | **CASCATEADO** | `lore-bible.md:570` "morta em -35, aos 43 anos" (0 ocorrências "morta há 30 anos"/"80 anos"). Ver INCOHERENCES C13. |
| CR-T3v2-03 Sonja "morta há 28 anos" lore-bible:500 | **CASCATEADO** | `lore-bible.md:500` agora datação absoluta "morta em -34 aos 33 anos (canon timeline:86 + CHARS:82 + factions:505)"; ambiguidade sanada. |
| CR-T3v2-04 DD-003 subtexto Bento 4 anos paralelo numérico | **CASCATEADO** | `in-world-docs.md:128` reformulado: paralelo numérico "13 mestres/4 sobreviventes" preservado, idade Bento corrigida para 7. |

**Resumo:** os 4 CRÍTICOS reais foram cascateados no canon central **após** a data deste AUDIT (verificado nas fontes primárias: factions, bento-requiem, lore-bible, in-world-docs, foreshadowing). Resta varredura confirmatória nos arquivos `diary/*` deep (não checados exaustivamente nesta passada de tracker-close). Incoerências factuais relacionadas (C3 Cauã/Davi, C13 Verônica) fechadas em INCOHERENCES.md. Status original BLOQUEADO deve ser revisto para "majoritariamente cascateado, pendente varredura diary".

## CRÍTICOS REAIS (4)

### CR-T3v2-01. Bento "aos 4 anos" cross-doc drift sistêmico 8 docs
Aritmética: Bento nasc -14, contaminação Patch-Zero Catedral Atelaiá -7 = 7 anos. Cascata T3 v1 FIX C8 pegou 6 docs mas faltam:
- characters/bento-requiem.md:29 "aos 4 anos (canônico -7)"
- factions.md:223 "Bento (4)"  
- foreshadowing.md:96+240
- foreshadow-links.md:167
- entries-mapas-timeline.md:523
- entries-fichas-bestiary.md:155
- in-world-docs.md:128 (subtexto DD-003, propaga via paralelo numérico)
- entries-manuscrito-glossario.md:62

### CR-T3v2-02. Verônica morte lore-bible:570 drift residual
"Morta há 30 anos, ~80 anos no momento" vs CHARS:73 canon "Morta -35 (43 anos)". INCOHERENCES C13 não-retrofitted lore-bible §14.

### CR-T3v2-03. Sonja "morta há 28 anos" lore-bible:500 ambíguo
"Há 28 anos" cronologicamente válido SE referência -6 (28 + -6 = -34 morte). Mas leitor não infere imediatamente. Substituir por datação absoluta "-34".

### CR-T3v2-04. DD-003 subtexto Bento 4 anos paralelo numérico
in-world-docs.md:128 subtexto DD-003 "trauma Bento aos 4 anos quando 4 também era a idade dele": drift literário-mnemônico via paralelo numérico 4 sobreviventes ↔ idade Bento. Reformular preservando paralelo numérico sem manter idade incorreta.

## MÉDIOS (9 — 7 validação OK + 2 pendentes)

### Pendentes (2)
- **MD-T3v2-01:** Mestre Loanis Penãoso (Pelicano Branco, morto -8, 55 anos) — ADD CHARS §5
- **MD-T3v2-02:** Solane (criança 8 anos Pelicano Branco) — ADD CHARS §7 ambient

### Validação OK (7)
- MD-T3v2-03 a -09: Lavínia idade + Bartolo DD-017 explicação Knowledge + Aldebrando paternidade + Octávia/Theodoro/Solange trinity + Hilário Tepenkov idade — todos consistentes cross-doc

## LEVES (4 validação OK)

- LV-T3v2-01 Atelaiá → Antoneto plausível canon
- LV-T3v2-02 Iremar-Velho vs Iremar Era 2 distinção canon
- LV-T3v2-03 Pyotor 40 + Yakov 36 alinhado
- LV-T3v2-04 Próspero 11 títulos institucionais resolvido

## COPYDESK §3 v2

- §3.3 Regência lore-bible:500 "Cult Mirage" sem "do"
- §3.4 "trauma do Bento aos 4 anos" anacolútico in-world-docs:128
- §3.5 timeline:78 parênteses redundantes Antoneto/Dmitri intervalo
- §3.2 lore-bible:570 "Verônica (morta há 30 anos, ~80 anos)" ambíguo

## TIMELINE CANON validado cross-doc

Era 1: -820 Anomalia + -720 Êxodo + -700 colapso ✅
Era 2: -150 GusWorld + -148 Ordem + -115 Asmódico + -110 Óxido + -95 Pythia + -80 São Camilo + -78 Selagem + -55 Dmitri + -45 Verônica + -34 Sonja + -25 DRE ✅
Era 3: -21 Davi nasc + -16 audit + -14 Bento + -13 Cauã/Dante/Salvador + -12 Iara/Linda/Bartolo + -11 Gus/Jaci + -8 surto + -7 Patch-Zero + -5 Davi morto + -3 saque + -2 Polis + -1 Iara + -0.5 Aldebrando + -0.25 Patch-Zero escapa ✅

## LISTA NEGRA confirmada eliminada

Patrício Vance + Chevarier + -890 + -16 Salvador + -16 Bartolo + -13 Davi + Sonja "-6" = **0 ocorrências prose** (apenas meta INCOHERENCES + AUDIT preservados).

## STATUS

**BLOQUEADO** 4 críticos residuais drift sistêmico não-tocados.

**Quote canon supremo TEXTREVIEW §1 v2:** "Se tiver de escolher entre regras de revisão e o canon, escolha o canon."

**Comparação T3 v1 → v2:** T3 v1 31 issues + cascata 16 críticos resolvidos; T3 v2 17 issues residuais (4 críticos drift sistêmico não-mapeado v1 + 13 médios/leves).
