# F5-BK.AUDIT.FULL T3 — CRONOLOGIA CROSS-DOC

**Data:** 2026-05-22
**Issues totais:** 31 (16 CRÍTICOS + 11 MÉDIOS + 4 LEVES)
**Modo:** read-only sampling estratégico cross-doc (31+ docs)

## CRÍTICOS (16)

### T3-CR-01. Davi Berenger DD-013 bilhete paradoxo
DD-013 in-world-docs "Cauã tem 8" — se escrito -13 (ano morte Davi consensus pre-cascata), Cauã 0 ano = impossível (Cauã nasc -13). Solução: DD-013 escrito -5 (Davi 16 morre, Cauã 8). Fix: in-world-docs:361 ano referência.

### T3-CR-02. INCOHERENCES.md:78 §C6 Salvador -16 vs -13 não-retrofitted
CHARS canon -13, INCOHERENCES ainda stale "-16". Fix: atualizar C6 marcando RESOLVIDO -13.

### T3-CR-03. Pyotor Vance idade drift 40 vs 37
lore-bible:521 = 40. BIBLE-V2-INDICE:35 = ~37. CHARS:47 sem idade. Yakov=36 (irmão 4 anos a menos) → Pyotor=40 consistente. Fix: padronizar 40 + ADD CHARS.

### T3-CR-04. Patrício Vance obsoleto remanescente 5+ docs
lore-bible:520 + timeline:115 + in-world-docs:797 + era-2:691,953 + BIBLE-V2-CAPA:67. Cascata T1-CR-01 não retrofitted. Fix: substituir Pyotor + retrofit profissão (médico-cyber itinerante).

### T3-CR-05. **Bento idade paradoxo Patch-Zero -7**
Bento 14 ano 0 → nasc -14 → em -7 = 7 anos. Mas lore-bible:499 + timeline:128 + characters/bento:99 + factions:203,223 = "Bento 4 anos cena Patch-Zero -7". Aritmética dupla quebrada. Decisão one-way door:
- Hipótese A: Bento 7 (não 4) em -7 — fix 5+ docs
- Hipótese B: Cena Patch-Zero em -10 (não -7) — preserva "Bento 4 wound" mas altera primeira-aparição-Patch-Zero canon
- Hipótese C: Bento 11 ano 0 (não 14) — re-cascata enorme

### T3-CR-06. timeline.md L114-121 entries duplicadas dev-lixo
L115+L119 marcadas "revisado para -13 abaixo" — entries obsoletas em doc canônico publicável. Fix: remover.

### T3-CR-07. Adila Murmúrio idade ambígua
CHARS implícito 40 hoje (Sonja morta -34 + Adila 6 = nasc -40). BIBLE-V2-APENDICE-H:22 sugere "Adila 32 conto 3". Inconsistência 8 anos. Fix: investigar antologia/03.

### T3-CR-08. Iara deserção idade timeline 10 vs 11
timeline:139 "Iara 10 anos descobre" vs :140 "deserta aos 11" vs characters/iara:27,107 "desertou aos 11". Iara nasc -12, em -1 = 11. Fix: ajustar timeline:139 → "10-11 anos".

### T3-CR-09. Linhagem Berenger Pythia: 3 vs 4 gerações
timeline:200 "Iremar → 3 gerações → Salvador". 02-caua-volt:7 "Salvador 4ª geração linhagem Iremar". Fix: explicitar "Iremar (1ª) → 3 intermediárias → Salvador (4ª) → Davi+Cauã (5ª)".

### T3-CR-10. Linhagem Próspero Vance 65 anos/geração biologicamente impossível
CHARS:204 "11 gerações Próspero (-720 a Era 3)". 720/11 = 65 anos/gen. Decisão one-way door:
- Declarar institucional não-cronológica (título cumulativo)
- Canonizar gerações intermediárias preenchidas

### T3-CR-11. BIBLE-V2-INDICE:205 "Patrício Tércio" → "Padim Tércio Almagre"
Padim = título religioso/honorífico canon CHARS:125. Drift "Patrício" confusão com lista negra obsoleta. Fix: substituir.

### T3-CR-12. characters/dante-grid.md:162 "Aos 9" vs canon "Aos 8"
Timeline + lore-bible + factions + dante-grid:166 + BIBLE-V2-INDICE:208 + deep/factions/fir:15 = "Vorto recruta Dante aos 8 (-5)". Drift apenas dante-grid:162 "Aos 9". Fix: substituir.

### T3-CR-13. Bartolo DD-017 paradoxo Linda -16 vs -12 (C5 persiste)
Bartolo carta -16 refere Linda nascida -12 (4 anos pré-nascimento). Decisão one-way door:
- Hipótese A: DD-017 data "entre -14 e -12" (Linda gestante/recém-nascida)
- Hipótese B: Bartolo info Sterling pré-natal (Knowledge alta gate)

### T3-CR-14. era-2-boom-tecnico:953 "Gargi ramo Boroshova matrilinear" não canon
Linhagem Boroshova-Vance Era 1 (CHARS:202) culmina via patrilinear Pyotor, NÃO matrilinear Gargi. Fix: remover atribuição OU canonizar decisão.

### T3-CR-15. lore-bible:522 Yakov sem retrofit 5 tecnologias canon
INCOHERENCES C9 retrofit parcial. CHARS:48 detalha (radar GPR + sísmica + tomografia muônica + magnetotelúrica + espectrometria microbiana + 89% mortalidade reduzida). Fix: enriquecer lore-bible.

### T3-CR-16. Verônica Atelaiá data morte canonizar ~-30
Nasc -78, ativa -45, Tarsila assume -25. Verônica morta entre -45 e -25. Fix: canonizar ~-30 em CHARS:73 + antologia/13.

## MÉDIOS (11)

T3-MD-01 lore-bible Patrício 520 + Pyotor 521 duplicado | T3-MD-02 Dante 8 vs 9 drift dante-grid:162 | T3-MD-03 Antoneto/Dmitri 40 anos/gen alto | T3-MD-04 Vespa 24 vs 25 drift 1 ano | T3-MD-05 Aldebrando 13.5 cross-link | T3-MD-06 lore-bible:482 "40 anos" tratado vs aritmética 42 | T3-MD-07 hiato Cult 28 anos -34 a -6 | T3-MD-08 factions Edilma sem retrofit explícito C10 | T3-MD-09 Batida Casa Antenas -50 sem entry timeline | T3-MD-10 Sérgio Brimber 24 vs 28 drift 4 anos | T3-MD-11 Catedral Atelaiá contaminação -7 vs saque -3 dois eventos distintos.

## LEVES (4)

T3-LV-01 timeline L74+L76 duplicadas | T3-LV-02 Calendário equinócio 1 set vs astronômico 22 set | T3-LV-03 Tradicoes Era 2 OK | T3-LV-04 Memo Yakov OK.

## STATUS

**BLOQUEADO** canon pré-tradução. 16 críticos T3 + cascata T1+T2 não-fixed.

**Decisões one-way door pendentes (5 P1):** T3-CR-05 Bento -7 + T3-CR-01 Davi DD-013 + T3-CR-10 Próspero linhagem + T3-CR-13 Bartolo DD-017 + T3-CR-16 Verônica morte.

**Acumulado audit T1+T2+T3:** 98 issues (53 críticos + 33 médios + 12 leves).

**Next:** T4 Voz Stephenson cross-doc.
