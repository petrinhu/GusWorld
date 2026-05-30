# Incoerências Canon GusWorld

**Audit Plano 2 (F1-DL.REFAC) — 2026-05-20**

Mapeamento sistemático de conflitos canon detectados via cross-validation deep-lore vs canon central (lore-bible/timeline/factions/in-world-docs/CHARS/PLACES).

---

## TOP 10 CRÍTICAS

### C1. Pai do Gus: Patrício Vance vs Pyotor Vance (ONE-WAY DOOR) [RESOLVIDO 2026-05-30]

**Conflito triplo: nome + profissão + dinâmica família**

- `lore-bible.md:520`: "**Patrício Vance** (40), pai Gus. Engenheiro de manutenção em estação de transmissão."
- `timeline.md:115`: "Gargi Vance + **Patrício Vance** (pai, engenheiro de manutenção)."
- `in-world-docs.md` DD-019:797: "**Patrício Vance** (pai ausente)."
- `CHARS.md:47`: "**Pyotor Vance**, pai do Gus, **médico-cyber itinerante**."
- `deep/antologia/10-pyotor-vance-pai.md` + `deep/stinger/post-credits-deep.md`: **Pyotor Vance**.
- Memos: `project_familia_vance_canonica` + `project_nome_gus_canon`: **Pyotor**.

**Decisão canon CHARS/deep-lore/memo é Pyotor + médico-cyber + separação amigável -5.**

**RESOLVIDO (cascata F1-DL.REFAC):** canon central retrofitted integralmente. Pyotor Vance médico-cyber itinerante (46) canonizado em `lore-bible.md:521` §14 + `timeline.md:120` (-11 Gus nasce) + `in-world-docs.md` DD-019:793-796 (caderneta Gargi + cartas Pyotor). Separação amigável -5 propagada. Verificação 2026-05-30: "Patrício Vance" tem 0 ocorrências em prose canon (lore-bible/timeline/in-world-docs/factions); só remanesce neste tracker como citação do problema original. Cross-ref `AUDIT-T3-CRONOLOGIA-V2` LISTA NEGRA (Patrício Vance eliminado) + `AUDIT-T9-VOZ-NPC-V2` MD-T9v2-03 (Pyotor cross-doc consistente).

---

### C2. Catedral Menor de São Camilo vs Atelaiá -80/-7 (numeração conflitante) [RESOLVIDO 2026-05-20, ver Nota de Resolução abaixo]

- `lore-bible.md:479` §13: "**-80** Tragédia da Catedral Menor de **São Camilo**. **17 mestres morrem** em uma noite."
- `timeline.md:76`: "-80 — Atelaiá registra: queda parcial de catedral menor por 'anomalia recorrente'. **17 mestres morrem**."
- `in-world-docs.md:103` DD-003: Diário Atelaiá: "Estávamos em **treze**. Saímos em **quatro**." Cabeçalho: Catedral Menor de **São Camilo**. → **13 mestres, 9 mortos** (não 17).
- `factions.md:223`: "**-7** Contaminação da Catedral Menor de **Atelaiá**. Hilário Tepenkov morre. Bento (4) sobrevive."

**Conflito:** numeração mortos -80 (13-4=9 sobrev. vs 17 mortos). Nomes catedrais distintas em -80 (São Camilo) e -7 (Atelaiá), OK.

**Cascata fix:** unificar numeração (sugestão: DD-003 in-character pode ser unreliable narrator, mas lore-bible/timeline devem alinhar — 17 mortos = canon estrutural).

---

### C3. Cauã idade DD-013 "Cauã tem 8" em -13 quebra cronologia [RESOLVIDO 2026-05-30]

- `timeline.md:110`: Cauã nasce **-13**.
- `in-world-docs.md` DD-013 (Davi bilhete final -13): "**o Cauã tem 8**."
- Se Cauã nasceu -13 e Davi morreu -13, Cauã tinha **0 anos**, não 8.

**Hipóteses:**
- Davi morreu em ano diferente (~ -8?), não -13.
- DD-013 contém "erro" autoral do Davi (unreliable).
- Cauã nasceu em ano diferente (~-21?).

**RESOLVIDO (cascata F1-DL.REFAC):** canon decidiu **Davi morre em -5** (Acidente da Subestação 7), não em -13. Cauã nasc -13, logo Cauã tinha **8 anos na morte de Davi** (a frase "Cauã tem 8" referencia o momento -5, consistente). Canonizado em `timeline.md:111` (-5 Davi morto aos 16, Cauã 8) + `timeline.md:116` (-13 Cauã nasce). Metadata corrigida em `in-world-docs.md` DD-005:62 ("Cauã tem 8 anos na época da primeira carta, nasc -13, acidente Davi -5") e DD-013:533 (bilhete Davi datado ano -5, "16 anos"). Davi nasc ~-21 (CHARS:56). Cross-ref `AUDIT-T3-CRONOLOGIA-V2` TIMELINE CANON Era 3 (-5 Davi morto) + LISTA NEGRA (-13 Davi morte eliminado) + `AUDIT-T10-AXIOLOGIA-V2` L04 (Davi -5 Subestação alinhado).

---

### C4. Bartolo Penkin: pai vs tio do Mateus [RESOLVIDO 2026-05-30]

- `CHARS.md:83`: "Bartolo Penkin, **Pai** do Mateus Penkin, auditor canônico."
- `environments/06` + `in-world-docs.md` DD-017: "Casa atualmente habitada por Mateus Penkin (**sobrinho** [de Bartolo])."

**Conflito direto.**

**RESOLVIDO (cascata F1-DL.REFAC):** canon decidiu **Bartolo = tio, Mateus = sobrinho**. Canonizado em `CHARS.md:84` ("Tio do Mateus Penkin, irmão paterno do pai biológico, não documentado canon") + `CHARS.md:138` (Mateus "sobrinho de Bartolo Penkin") + `in-world-docs.md` DD-017:715 ("Casa atualmente habitada por Mateus Penkin, sobrinho, fofoqueiro"). Consistente cross-doc. Cross-ref `AUDIT-T3-CRONOLOGIA-V2` MÉDIOS Validação OK.

---

### C5. DD-017 Bartolo "cuida da Linda quando ela tiver idade" escrito -16 [RESOLVIDO 2026-05-30]

- Bartolo desapareceu **-16**.
- Linda nasceu **-12** (4 anos depois).
- DD-017 página 80: "cuida da Linda quando ela tiver idade. Você sabe quem ela é."
- Bartolo não podia se referir à Linda antes dela nascer.

**Hipóteses:** clarividência narrativa esticada OU Bartolo desapareceu mais tarde.

**RESOLVIDO (cascata F1-DL.REFAC):** dupla resolução. (1) Cronologia: Bartolo NÃO desapareceu -16; vazou o audit em -16, manteve perfil baixo por 4 anos e foi eliminado em **-12** (canon `timeline.md:114` + `lore-bible.md:571` + `factions`). A página 80 (gate Knowledge alta) pôde ser escrita até -12, quando Linda (nasc -12) já existia. (2) Subtexto: "Você sabe quem ela é" deixa de ser clarividência afetiva e passa a ser índice documental, Bartolo tinha acesso operacional, como engenheiro QA sênior Apex-Data, a relatório pré-natal de monitoramento de perfil genético-cognitivo de prole de funcionários-chave (nota canônica robusta em `in-world-docs.md` DD-017:745). Cross-ref `AUDIT-T3-CRONOLOGIA-V2` MD-T3v2 Validação OK (Bartolo DD-017 explicação Knowledge consistente).

---

### C6. Salvador Berenger (pai Cauã) — **RESOLVIDO 2026-05-22** (canon F5-BK.AUDIT T3-CR-02)

**Resolução canon:** Salvador Berenger morreu **-13** (acidente tráfego Periferia mesmo ano nascimento Cauã). CHARS:62 atualizado. R-AUDIT-04 documentou Conto 02 reescrito com Cauã narrando Salvador via memória oral materna (Cauã não conheceu pai pessoalmente). timeline.md retrofit aplicado (entry -13: Cauã nasce + Salvador morre). lore-bible + factions + deep-lore alinhados em cascata. Cross-ref AUDIT-T1-NOMES:279.

**Status:** RESOLVIDO. Sem ação pendente.

---

### C7. Idades Sterling Corp diretores divergem [RESOLVIDO 2026-05-30]

| Diretor | lore-bible | CHARS |
|---|---|---|
| Octávia Penedo | 52 | 55 |
| Theodoro Calveri | 57 | 52 |
| Solange Vix | 49 | 46 |

**Cascata fix:** decidir versão canônica + propagar.

**RESOLVIDO (cascata F1-DL.REFAC):** idades unificadas com a versão CHARS (deep-lore R5 npcs-antologia). Octávia Penedo **55** (`lore-bible.md:533` = `CHARS.md:94`); Theodoro Calveri **52** (`lore-bible.md:534` = `CHARS.md:96`); Solange Vix **46** (`lore-bible.md:535` = `CHARS.md:95`). Verificação 2026-05-30: lore-bible §14 alinhado a CHARS em todos os três. Cross-ref `AUDIT-T3-CRONOLOGIA-V2` MÉDIOS (Octávia/Theodoro/Solange trinity consistente) + `AUDIT-T9-VOZ-NPC-V2` MD-T9v2-04 (Calveri CHARS:96 consistente).

---

### C8. Mãe Bento (Atelaiana de Sevra Chevalier) canon CHARS sem retrofit [RESOLVIDO 2026-05-30]

- `CHARS.md:60`: "Atelaiana de Sevra Chevalier, Mãe biológica viva do Bento, irmã consanguínea de Mestre Lavínia Sevra. Partiu da Catedral três meses após o parto."
- `lore-bible.md §14` + `timeline.md`: mãe Bento NÃO nomeada.

**Cascata fix:** retrofit lore-bible §14 + characters/bento-requiem.md.

**RESOLVIDO (cascata F1-DL.REFAC):** retrofit aplicado em `lore-bible.md:523` §14 ("Atelaiana de Sevra Chevalier, mãe biológica viva do Bento, irmã consanguínea de Mestre Lavínia Sevra. Partiu da Catedral três meses após o parto, canon R4 deep-lore"). Alinhado a CHARS:60. Verificação 2026-05-30: nome presente em lore-bible §14.

---

### C9. Tio Yakov Vance canon CHARS sem retrofit [RESOLVIDO 2026-05-30]

- `CHARS.md:48`: "Yakov Vance, Irmão mais novo de Pyotor (4 anos a menos), engenheiro+geólogo 'maior mineradora do reino'. Apresentou xadrez ao Gus."
- `lore-bible.md / timeline.md`: **zero menção Yakov**.

**Cascata fix:** retrofit lore-bible §14.

**RESOLVIDO (cascata F1-DL.REFAC):** retrofit aplicado em `lore-bible.md:522` §14 (Yakov Vance, 42, tio paterno, irmão mais novo de Pyotor 4 anos a menos, engenheiro de software + geólogo na maior mineradora do reino, stack prospecção 5 tecnologias, 89% redução mortalidade Fibonacci, apresenta xadrez ao Gus) + `timeline.md:120` (-11 "Tio paterno Yakov Vance ~25 na época"). Alinhado a CHARS:48. Cross-ref `AUDIT-T3-CRONOLOGIA-V2` LV-T3v2-03 (Pyotor 40 + Yakov 36 alinhado).

---

### C10. Edilma Alencar: viva reclusa vs presa Caverna dos Perdidos [RESOLVIDO 2026-05-30]

- `lore-bible.md:521` + `factions.md` §1 (-8): Edilma "sobrevive, hoje vive **reclusa em apartamento subsidiado**".
- `PLACES.md:76` Caverna dos Perdidos (Dragon Victory canon): "Família Dante (mãe **Edilma Alencar) presa lá** pra chantagear Dante (canon arco Dante Beat Ten reveal)."

**Conflito:** estado canônico atual da Edilma.

**Cascata fix:** distinguir aparência pública (reclusa em apartamento) vs estado real (sequestrada por Sterling brass na Caverna).

**RESOLVIDO (cascata F1-DL.REFAC):** canonizado modelo **dual-state** (aparência pública vs estado real). Aparência pública: reclusa em apartamento subsidiado, fachada FIR rotacionando indicadores de habitação ativa. Estado real (revelação arco Dante Beat Ten + Dragon Victory): sequestrada em -8 por Cassiano Vorto sob ordem Sterling, mantida em cela da Caverna dos Perdidos (nordeste Pântano Markov, Selve Sombria profunda), chantagem psicológica permanente sobre Dante. Canonizado em `lore-bible.md:525` §14 + `factions.md:305` §3 FIR (entry Edilma completa, cross-ref "INCOHERENCES C10 resolvido") + `timeline.md:124` (-8 retrofit dual-state). Cross-ref `PLACES.md:76` (Caverna dos Perdidos) + `characters/dante-grid.md:117`.

---

## INCOERÊNCIAS ADICIONAIS (médias)

- **C11. Mara Bento dupla entrada CHARS** (§6 operadora rádio + §7 pesquisadora acústica). **[RESOLVIDO 2026-05-30]** CHARS unificou em entrada única `CHARS.md:85` (28 anos, operadora rádio analógica Underground sub-célula Norte + pesquisadora acústica clandestina Zona do Silêncio fundidos). Verificação 2026-05-30: 1 só entrada Mara Bento em CHARS.
- **C12. Vespa Calderón dupla entrada CHARS** (§6 + §7). **[RESOLVIDO 2026-05-30]** CHARS tem entrada única `CHARS.md:93` (24 anos, Underground Dutos Infernais + scripter Pythia rebelde, mentora lateral Cauã). Verificação 2026-05-30: 1 só entrada Vespa Calderón em CHARS.
- **C13. Verônica Atelaiá morta -30 vs Sterling classifica tratado -25** (Sterling classifica tratado 5 anos pós-morte autor). **[RESOLVIDO 2026-05-30]** Canon decidiu Verônica **morta -35 aos 43** (canon F5-BK.AUDIT T3-CR-16). Sterling classifica o tratado em -25 (post-mortem, 10 anos após a morte), recurso narrativo coerente (ele rebaixa tratado de autora já falecida). Canonizado em `lore-bible.md:570` ("morta em -35, aos 43 anos, tratado classificado por Sterling em -25 post-mortem") + `CHARS.md:73` ("Morta -35, 43 anos"). **NOTA de verificação 2026-05-30:** os críticos `AUDIT-T1-NOMES-V2` MD-T1v2-03 e `AUDIT-T3-CRONOLOGIA-V2` CR-T3v2-02 datavam de 2026-05-22 e apontavam drift "lore-bible:570 morta há 30 anos / 80 anos" + "sterling-locke-deep:31 ~-1". Reverificação no canon de hoje: "morta há 30 anos" e "80 anos" têm **0 ocorrências** em lore-bible (já corrigido para -35); `deep/antologia/08-sterling-locke.md` na posição citada não contém mais "~-1" (conteúdo realinhado, tratado datado -45 = data do tratado, ativa -45, coerente). Esses dois críticos AUDIT foram cascateados no canon **após** a data do AUDIT; ver marcador de status nos respectivos AUDIT-T*.
- **C14. Tatauín Branca idade 22 (factions) vs "veterana" (CHARS)**. **[RESOLVIDO 2026-05-30]** Falso conflito esclarecido. "Veterana" não significa idade avançada: `CHARS.md:140` canoniza Tatauín **22 anos**, "veterana no sentido de experiência em hierarquia juvenil do vilarejo" (sucessora designada do caderno-semente de Mariana). Coerente.
- **C15. Datação vilarejos: -720 fundação física (PLACES) vs -45 formalização institucional (timeline)**, distinção válida tornada explícita. **[RESOLVIDO 2026-05-30]** Canon canoniza as duas datas como camadas distintas (não drift): -720 fundação física pós-Êxodo (Pioneiro-Fundador Hilário Vanderbist, `CHARS.md:172` §8b) vs -45 formalização como entidade comunitária formal (`timeline.md:83` + `lore-bible.md:481`, Anciã Soraia Vanderbist lidera). Cross-ref `AUDIT-T2-LUGARES-V2` (datação vilarejos alinhada cross-doc).

---

## PATTERN SISTÊMICO DE DRIFT [SANADO 2026-05-30, diagnóstico histórico]

> Os quatro padrões abaixo foram o diagnóstico original (2026-05-20). Verificação 2026-05-30 confirma que a cascata F1-DL.REFAC retrofitou o canon central; os padrões estão SANADOS para C1-C15 (ver veredito por incoerência acima). Mantido como registro histórico do drift detectado.

1. **Deep-lore R4/R5/R6/R7/R8/R9 não retrofitted em canon central.** Decisões posteriores (Pyotor Vance, Salvador Berenger, Atelaiana mãe Bento, Yakov tio, Edilma sequestrada, idades Sterling) ficaram só em CHARS+PLACES+memos. **[SANADO: retrofit aplicado em lore-bible §14 + timeline + factions + in-world-docs.]**
2. **Idades NPCs divergem** entre lore-bible (canon original) e CHARS (atualizado deep-lore). 3+ casos confirmados. **[SANADO: trio Sterling (Octávia 55 / Theodoro 52 / Solange 46) + Verônica (-35/43) alinhados lore-bible vs CHARS, ver C7 + C13.]**
3. **In-world-docs DD-003, DD-013, DD-017** contêm datas/relações que não casam com timeline. Pode ser unreliable narrator deliberado OU drift genuíno. **[SANADO: DD-003 = unreliable narrator canonizado (C2); DD-013/DD-005 = Davi morre -5, Cauã 8 coerente (C3); DD-017 = Bartolo eliminado -12 + índice documental, não clarividência (C4+C5).]**
4. **Cronologia secundária Berenger/Penkin** com inconsistências internas visíveis em cross-reading. **[SANADO: linhagem Berenger canonizada timeline:199 (Salvador morto -13, Davi -5, Cauã -13); Penkin tio/sobrinho fixado.]**

---

## CASCATA FIX PROPOSTA (Plano 3) [EXECUTADA, fechada 2026-05-30]

Para cada CRÍTICA C1-C10 + relevantes adicionais, decisão criador supremo necessária:
- Versão a preservar canônica
- Doc origem (qual fonte é correta)
- Cascata fix (quais docs atualizar)

**Total estimado:** ~15-20 decisões one-way door + ~30-50 edits cascata.

**Status:** EXECUTADA. As decisões C1-C12 do criador supremo foram aplicadas via cascata F1-DL.REFAC ao canon central (lore-bible §13-14 + timeline + factions + in-world-docs + CHARS). C13-C15 (adicionais médias) também resolvidos. Verificação ponto-a-ponto contra o canon em 2026-05-30 (tarefa F1-DL.TRACKER-CLOSE) confirma aplicação. Ver veredito por incoerência nos cabeçalhos acima.

---

## STATUS GLOBAL (fechamento de tracker, F1-DL.TRACKER-CLOSE 2026-05-30)

**15 incoerências catalogadas (C1-C15). 15 RESOLVIDAS. 0 ABERTAS.**

| Faixa | Resolvido | Aberto |
|---|---|---|
| TOP 10 CRÍTICAS (C1-C10) | 10 | 0 |
| Adicionais médias (C11-C15) | 5 | 0 |
| **Total** | **15** | **0** |

**Método:** verificação assertiva ponto-a-ponto contra o canon central (lore-bible.md, timeline.md, factions.md, CHARS.md, in-world-docs.md, characters/). Cada Cx só marcado RESOLVIDO após confirmar a correção fisicamente no doc canônico citado (não marcação cega).

**Achados de verificação:**
- "Patrício Vance" / "Bento (4)" / "-13 Davi morte" / "Chevarier" / "Salvador -16" = 0 ocorrências em prose canon central (só remanescem como citações do problema neste tracker). Lista negra confirmada limpa.
- C13: lore-bible:570 + sterling-locke-deep já corrigidos para -35 / tratado -45, **após** a data dos AUDIT-T1/T3 (2026-05-22). Os críticos AUDIT correspondentes (MD-T1v2-03, CR-T3v2-02) estavam stale; marcados como cascateados nos respectivos AUDIT-T*.

**Distinção importante (não confundir):** este tracker (C1-C15) cobre **incoerências factuais de canon** (nomes, idades, parentescos, datas, estados de personagem), TODAS resolvidas. Os 10 docs `AUDIT-T*-V2` cobrem **categorias de auditoria de prosa/estilo** (voz Stephenson, easter eggs Fibonacci/maçom, em-dash, pillars, axiologia) e contêm críticos PRÓPRIOS, em sua maioria refatorações de prosa deep-lore (era-2 fadiga, maçom literal, em-dash global) que permanecem **PENDENTES** (BLOQUEADO). Fechar este tracker NÃO fecha aqueles refatores. Ver marcador de status no topo de cada AUDIT-T*.

---

**Versão inicial:** 2026-05-20. F1-DL.REFAC Plano 2C. Fechamento de tracker: 2026-05-30 (F1-DL.TRACKER-CLOSE). Não modificar sem reaudit.

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
