<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

> **RESGATADO em 25/08/2026 do `gusworld_legacy` pelo `technical-writer`, na varredura profunda de narrativa. ACHADOS JÁ RESOLVIDOS, conferidos contra a árvore atual em 25/08/2026.** Este dossiê (`F5-BK.AUDIT.FULL T2 v2`, 22/05/2026) lista 2 críticos residuais que sobreviveram à cascata da v1 (ver `AUDIT-T2-LUGARES.md`):
>
> - **CR-T2v2-01** (`diary/entries-mapas-timeline.md`, "Câmara do Equinócio Acústico descoberta por Joaquim em -1" vs canon "-3"): **resolvido**. `diary/entries-mapas-timeline.md:329` e `:339` já dizem "descoberta Joaquim -3" / "descoberta -3", batendo com `PLACES.md:91`.
> - **CR-T2v2-02** (quadrante do vilarejo Pelicano Branco: "oeste-sul" em `deep/settings/02-selve-sombria.md` vs "centro-sul" em `deep/settings/08-selve-profunda.md`, colidindo com "Pelicano Roxo a oeste"): **resolvido**. Os dois arquivos hoje dizem "Pelicano Branco centro-sul" e "Pelicano Roxo a oeste" de forma consistente (`02-selve-sombria.md:25`, `08-selve-profunda.md:11`).
>
> **Os itens MÉDIOS e LEVES abaixo não foram reconferidos individualmente.** Nada aqui foi declarado morto por este agente (L-14 do `GODS_LAWS.md`); a confirmação acima é factual (arquivo:linha).
>
> **O que segue é o texto original do legacy, sem edição de conteúdo.**

---

# F5-BK.AUDIT.FULL T2 v2 — LUGARES CANONIZAÇÃO CROSS-DOC

**Data:** 2026-05-22 (refazer com TEXTREVIEW v2)
**Issues totais:** 12 (2 CRÍTICOS reais + 5 MÉDIOS + 3 LEVES)
**Eficácia cascata T2 v1:** 93% confirmada (zero drift lista negra Sylvania-Catedrais/Vyr-Aldeia/Polis Vermelha s/hífen/Cidade Janelarum)

---

## STATUS DE CASCATA (revisão F1-DL.TRACKER-CLOSE 2026-05-30)

> Marcador de bookkeeping. NÃO altera o canon.

| Crítico | Status no canon | Nota |
|---|---|---|
| CR-T2v2-01 Câmara Equinócio Acústico datação -3 vs -1 | PENDENTE (fix diary) | Canon -3 (PLACES:91 + environments/07 + factions); drift em `diary/entries-mapas-timeline` 2 ocorrências, não verificado/corrigido. Fix trivial sed. |
| CR-T2v2-02 Quadrante Pelicano Branco "oeste-sul" vs "centro-sul" | PENDENTE (decisão criador) | One-way door entre `deep/settings/02` e `deep/settings/08`; sem decisão registrada. |

**Relação com incoerências factuais C1-C15:** C15 (datação vilarejos -720 físico vs -45 institucional) está RESOLVIDO em INCOHERENCES.md (camadas distintas canon). O lugar "Caverna dos Perdidos" (relevante a C10 Edilma) foi verificado alinhado cross-doc (L-T2v2-01 confirmado). Os 2 críticos T2 são de datação/quadrante em prosa deep-lore + diary, **PENDENTES**. Status BLOQUEADO original mantido para os 2 críticos.

## CRÍTICOS REAIS (2)

### CR-T2v2-01. Câmara do Equinócio Acústico datação -3 vs -1
`PLACES:91 + environments/07:44+118 + factions` canonizam descoberta Joaquim Bartolomeu **-3**. Drift `diary/entries-mapas-timeline:329+339` "Câmara descoberta Joaquim -1". Fix: sed -1→-3 diary 2 ocorrências.

### CR-T2v2-02. Quadrante Pelicano Branco "oeste-sul" vs "centro-sul"
`deep/settings/02-selve-sombria:25` = "Pelicano Branco oeste-sul" + colisão com "Pelicano Roxo oeste".
`deep/settings/08-selve-profunda:11` = "Pelicano Branco centro-sul". 
Decisão one-way door criador supremo.

## FALSOS POSITIVOS (2)

- CR-T2v2-03 Trilha placas latão: 50 (Pioneiros) vs 117 (Sementes) — trilhas DISTINTAS canon ✅
- CR-T2v2-04 Cabo cobre Era 2 ano -65 — alinhado timeline ✅

## MÉDIOS (5)

- M-T2v2-01 Polis-Vermelha -2 caída: consistente cross-doc ✅ pillar
- M-T2v2-02 Tartaruga-Fractal "sul" + "sul-mais-distante": variação descritiva válida ✅
- M-T2v2-03 Eco sintático Stephenson canon §3.4 v2 COPYDESK (densidade alta sustentada legítima vs sustentável fadiga §6.3+§9.3 era-2)
- M-T2v2-04 Praça da Compilação cross-doc alinhada ✅ pillar
- M-T2v2-05 Heliópolis-Nova "submissa há mais tempo" lore-bible:382 ambíguo — retrofit "desde Era 3 (~-25)"

## LEVES (3 verificação)

L-T2v2-01 Caverna dos Perdidos descrição alinhada cross-doc 100% | L-T2v2-02 Setor Tavus -150/-140 alinhado | L-T2v2-03 Catedral-Mãe 47 ocorrências consistentes

## COPYDESK §3 v2

- §3.4 Eco "coletivista" sustentado 5x parágrafo ordem-recursiva:144 + era-2:983
- §3.4 Eco "articula-se em registro de" pervasivo era-2 §6.3+§9.3 (>20x cada)
- §3.2 Concordância PLACES:75-76 "Família Dante mãe Edilma presa lá" anacoluto
- §3.3 Regência ordem-recursiva:182 "à Família Tavus Vance" — análise canon

## LIMPEZA confirmada

Sylvania-Catedrais + Vyr-Aldeia + Polis Vermelha s/hífen + Cidade Janelarum = **0 ocorrências cross-doc**. Cascata T2 v1 efetiva 100% lista negra.

## STATUS

**BLOQUEADO** 2 críticos pendentes (1 trivial diary + 1 decisão criador).

**Quote canon supremo aplicado (§1 v2):** "Se tiver de escolher entre regras de revisão e o canon, escolha o canon."

**Comparação T2 v1 → v2:** T2 v1 25 issues + cascata aplicada; T2 v2 12 issues residuais (2 reais críticos + 10 médios/leves descritivos).
