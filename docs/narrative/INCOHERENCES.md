# Incoerências Canon GusWorld

**Audit Plano 2 (F1-DL.REFAC) — 2026-05-20**

Mapeamento sistemático de conflitos canon detectados via cross-validation deep-lore vs canon central (lore-bible/timeline/factions/in-world-docs/CHARS/PLACES).

---

## TOP 10 CRÍTICAS

### C1. Pai do Gus: Patrício Vance vs Pyotor Vance (ONE-WAY DOOR)

**Conflito triplo: nome + profissão + dinâmica família**

- `lore-bible.md:520`: "**Patrício Vance** (40) — pai Gus. Engenheiro de manutenção em estação de transmissão."
- `timeline.md:115`: "Vênea Vance + **Patrício Vance** (pai, engenheiro de manutenção)."
- `in-world-docs.md` DD-019:797: "**Patrício Vance** (pai ausente)."
- `CHARS.md:47`: "**Pyotor Vance** — Pai do Gus, **médico-cyber itinerante**."
- `deep/antologia/10-pyotor-vance-pai.md` + `deep/stinger/post-credits-deep.md`: **Pyotor Vance**.
- Memos: `project_familia_vance_canonica` + `project_nome_gus_canon`: **Pyotor**.

**Decisão canon CHARS/deep-lore/memo é Pyotor + médico-cyber + separação amigável -5.** Canon central NÃO foi retrofitted.

**Cascata fix:** lore-bible §14 + timeline + in-world-docs DD-019.

---

### C2. Catedral Menor de São Camilo vs Atelaiá -80/-7 (numeração conflitante)

- `lore-bible.md:479` §13: "**-80** Tragédia da Catedral Menor de **São Camilo**. **17 mestres morrem** em uma noite."
- `timeline.md:76`: "-80 — Atelaiá registra: queda parcial de catedral menor por 'anomalia recorrente'. **17 mestres morrem**."
- `in-world-docs.md:103` DD-003: Diário Atelaiá: "Estávamos em **treze**. Saímos em **quatro**." Cabeçalho: Catedral Menor de **São Camilo**. → **13 mestres, 9 mortos** (não 17).
- `factions.md:223`: "**-7** Contaminação da Catedral Menor de **Atelaiá**. Hilário Tepenkov morre. Bento (4) sobrevive."

**Conflito:** numeração mortos -80 (13-4=9 sobrev. vs 17 mortos). Nomes catedrais distintas em -80 (São Camilo) e -7 (Atelaiá), OK.

**Cascata fix:** unificar numeração (sugestão: DD-003 in-character pode ser unreliable narrator, mas lore-bible/timeline devem alinhar — 17 mortos = canon estrutural).

---

### C3. Cauã idade DD-013 "Cauã tem 8" em -13 quebra cronologia

- `timeline.md:110`: Cauã nasce **-13**.
- `in-world-docs.md` DD-013 (Davi bilhete final -13): "**o Cauã tem 8**."
- Se Cauã nasceu -13 e Davi morreu -13, Cauã tinha **0 anos**, não 8.

**Hipóteses:**
- Davi morreu em ano diferente (~ -8?), não -13.
- DD-013 contém "erro" autoral do Davi (unreliable).
- Cauã nasceu em ano diferente (~-21?).

**Cascata fix:** decisão criador supremo qual versão preserva.

---

### C4. Bartolo Penkin: pai vs tio do Mateus

- `CHARS.md:83`: "Bartolo Penkin — **Pai** do Mateus Penkin, auditor canônico."
- `environments/06` + `in-world-docs.md` DD-017: "Casa atualmente habitada por Mateus Penkin (**sobrinho** [de Bartolo])."

**Conflito direto.**

---

### C5. DD-017 Bartolo "cuida da Linda quando ela tiver idade" escrito -16

- Bartolo desapareceu **-16**.
- Linda nasceu **-12** (4 anos depois).
- DD-017 página 80: "cuida da Linda quando ela tiver idade. Você sabe quem ela é."
- Bartolo não podia se referir à Linda antes dela nascer.

**Hipóteses:** clarividência narrativa esticada OU Bartolo desapareceu mais tarde.

---

### C6. Salvador Berenger (pai Cauã) — **RESOLVIDO 2026-05-22** (canon F5-BK.AUDIT T3-CR-02)

**Resolução canon:** Salvador Berenger morreu **-13** (acidente tráfego Periferia mesmo ano nascimento Cauã). CHARS:62 atualizado. R-AUDIT-04 documentou Conto 02 reescrito com Cauã narrando Salvador via memória oral materna (Cauã não conheceu pai pessoalmente). timeline.md retrofit aplicado (entry -13: Cauã nasce + Salvador morre). lore-bible + factions + deep-lore alinhados em cascata. Cross-ref AUDIT-T1-NOMES:279.

**Status:** RESOLVIDO. Sem ação pendente.

---

### C7. Idades Sterling Corp diretores divergem

| Diretor | lore-bible | CHARS |
|---|---|---|
| Octávia Penedo | 52 | 55 |
| Theodoro Calveri | 57 | 52 |
| Solange Vix | 49 | 46 |

**Cascata fix:** decidir versão canônica + propagar.

---

### C8. Mãe Bento (Atelaiana de Sevra Chevalier) canon CHARS sem retrofit

- `CHARS.md:60`: "Atelaiana de Sevra Chevalier — Mãe biológica viva do Bento, irmã consanguínea de Mestre Lavínia Sevra. Partiu da Catedral três meses após o parto."
- `lore-bible.md §14` + `timeline.md`: mãe Bento NÃO nomeada.

**Cascata fix:** retrofit lore-bible §14 + characters/bento-requiem.md.

---

### C9. Tio Yakov Vance canon CHARS sem retrofit

- `CHARS.md:48`: "Yakov Vance — Irmão mais novo de Pyotor (4 anos a menos), engenheiro+geólogo 'maior mineradora do reino'. Apresentou xadrez ao Gus."
- `lore-bible.md / timeline.md`: **zero menção Yakov**.

**Cascata fix:** retrofit lore-bible §14.

---

### C10. Edilma Alencar: viva reclusa vs presa Caverna dos Perdidos

- `lore-bible.md:521` + `factions.md` §1 (-8): Edilma "sobrevive, hoje vive **reclusa em apartamento subsidiado**".
- `PLACES.md:76` Caverna dos Perdidos (Dragon Victory canon): "Família Dante (mãe **Edilma Alencar) presa lá** pra chantagear Dante (canon arco Dante Beat Ten reveal)."

**Conflito:** estado canônico atual da Edilma.

**Cascata fix:** distinguir aparência pública (reclusa em apartamento) vs estado real (sequestrada por Sterling brass na Caverna).

---

## INCOERÊNCIAS ADICIONAIS (médias)

- **C11. Mara Bento dupla entrada CHARS** (§6 operadora rádio + §7 pesquisadora acústica)
- **C12. Vespa Calderón dupla entrada CHARS** (§6 + §7)
- **C13. Verônica Atelaiá morta -30 vs Sterling classifica tratado -25** (Sterling classifica tratado 5 anos pós-morte autor)
- **C14. Tatauín Branca idade 22 (factions) vs "veterana" (CHARS)**
- **C15. Datação vilarejos: -720 fundação física (PLACES) vs -45 formalização institucional (timeline)** — distinção válida mas não explícita

---

## PATTERN SISTÊMICO DE DRIFT

1. **Deep-lore R4/R5/R6/R7/R8/R9 não retrofitted em canon central.** Decisões posteriores (Pyotor Vance, Salvador Berenger, Atelaiana mãe Bento, Yakov tio, Edilma sequestrada, idades Sterling) ficaram só em CHARS+PLACES+memos.
2. **Idades NPCs divergem** entre lore-bible (canon original) e CHARS (atualizado deep-lore). 3+ casos confirmados.
3. **In-world-docs DD-003, DD-013, DD-017** contêm datas/relações que não casam com timeline. Pode ser unreliable narrator deliberado OU drift genuíno.
4. **Cronologia secundária Berenger/Penkin** com inconsistências internas visíveis em cross-reading.

---

## CASCATA FIX PROPOSTA (Plano 3)

Para cada CRÍTICA C1-C10 + relevantes adicionais, decisão criador supremo necessária:
- Versão a preservar canônica
- Doc origem (qual fonte é correta)
- Cascata fix (quais docs atualizar)

**Total estimado:** ~15-20 decisões one-way door + ~30-50 edits cascata.

---

**Versão inicial:** 2026-05-20. F1-DL.REFAC Plano 2C. Não modificar sem reaudit.

---

## NOTAS DE RESOLUÇÃO CANON SUPREMO (2026-05-20)

### C2. Tragédia Catedral Menor de São Camilo -80: 17 mortos canon, DD-003 unreliable

**Decisão criador supremo:** lore-bible §13 + timeline -80 mantêm **17 mestres mortos** como canon estrutural. DD-003 (Diário Atelaiá Chevalier, "estávamos em treze, saímos em quatro" = 13 presentes, 9 mortos, 4 sobreviventes) é **unreliable narrator deliberado**: Atelaiá omitiu 8 mortos por trauma e/ou proteção (poupar leitor in-world, ocultar identidade de mortos cujos descendentes ainda estão vivos, ou registro parcial da câmara em que esteve, não do total da catedral). Player sensível percebe a fratura em cross-reading; gate Knowledge alta pode expor a discrepância como pista de leitura crítica das fontes in-world.

Nenhuma cascata fix em lore-bible/timeline. Apenas reconhecimento explícito do recurso narrativo.

---

## PILOTO TEXTREVIEW LOTE 1 — ORDEM RECURSIVA (2026-05-22)

**Contexto:** F5-BK.AUDIT.PILOTO executado pós-F1-DL.ORDEM-EXPAND. Revisor-textual + TEXTREVIEW.md protocolo dominante aplicado a `docs/narrative/deep/factions/ordem-recursiva.md` (11815 pal, Lote 1 = §1 + §3 + §8).

**Achados:** 13 CRÍTICOS + 6 MÉDIOS + 4 LEVES detectados (output 4-pack: Dicionário + Inconsistências + Tabelas + Mermaid).

**9 decisões one-way door resolvidas pelo criador supremo:**

| # | Decisão | Resolução |
|---|---|---|
| 1 | Canonizar 7 NPCs novos | Canonizar todos 7 (3 matrilineares + 3 patrilineares + Aldebrando já canon) |
| 2 | Colisão homônima Hilário (4 personagens) | Renomear patrilinear Chevalier; substituir Hilário por nome único etnia mista não-africana |
| 3 | Gerações matrilineares R10 (5) vs Ordem (6) | 6 gerações com Antoneta + sobrenomes compostos matrilineares |
| 4 | Gerações patrilineares timeline (4) vs Ordem (5) | 4 gerações intermediárias canon. Nomes: Antoneto + Dmitri + Casimiro + Aldebrando-pai-Bento |
| 5 | Aritmética §1 L13 "43 anos pós Hiato" | "572 anos após Êxodo" (Êxodo -720 + 572 = -148). Correção rerun TEXTREVIEW: original decisão "552" continha erro aritmético; valor canon final = 572. |
| 6 | Polis-Vermelha "captura política -45" (R10 §6.2 NÃO tem isso) | Remover -45 + usar "deriva séculos" alinhado R10 canon |
| 7+8 | 3 festas regionais novas | Canonizar 3 festas em tradicoes-cultura.md (Calibração 8 fev + Federação 8 mai + Coletivista 13 fev) |
| 9 | Nomenclatura 5 catedrais cross-doc | PLACES como canonical: São Camilo + Quarta + Quinta ativas; São Vargas + Atelaiá saqueadas -3 |

**Cascata canon executada (2026-05-22):**

1. ✅ CHARS.md §8c: +6 NPCs (Antoneta Argéndia-Chevalier + Tarsila Atelaiá-Verônica + Felícia Tarsila + Antoneto + Dmitri + Casimiro Chevalier) + 2 linhagens canonizadas (matrilinear cronistas 6 gerações + patrilinear Chevalier 6 totais 4 intermediárias)
2. ✅ tradicoes-cultura.md: +3 festas regionais (Calibração + Federação + Coletivista)
3. ✅ timeline.md: 5 eventos novos linhagem (-110 Antoneta nasce, -95 Antoneto, -78 expandido linhagem matrilinear, -55 Dmitri, -25 Casimiro+Tarsila, -8 Felícia) + Linhagens cruzadas atualizada (Chevalier 4 gerações nomeadas + nova Cronistas matrilineares Atelaiá)
4. ⏳ ordem-recursiva.md: prose deep-lore fixes pendentes (dispatchar narrative-writer):
   - §1 L13 aritmética
   - §3 catedrais ativas (3 ativas + 2 saqueadas alinhar PLACES)
   - §8.4 sobrenomes Tarsila/Felícia (matrilinear composto)
   - §9.5 patrilineares (4 nomes: Antoneto + Dmitri + Casimiro + Aldebrando)
   - §8.7 + §9.2 + §11.1 remover "Polis-Vermelha -45" → "deriva séculos"
5. ⏳ era-2-boom-tecnico.md (R10): drift sobrenomes Tarsila/Felícia alinhar

**Status piloto:** PROTOCOLO VALIDADO. ROI revisão proativa confirmado empiricamente. 13 críticos que cascateariam pra docs futuros + custo audit final 3-5× maior se adiados.

**Próximo passo:** rerun TEXTREVIEW Lote 1 pós-fixes prose pra validar 0 críticos remanescentes.
