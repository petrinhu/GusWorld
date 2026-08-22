# ADR-001: Pivot Lore-to-Engine — pausa de deep-lore como gating, retomada paralela orgânica

**Status:** Accepted
**Data:** 2026-05-19
**Decisores:** petrinhu (criador supremo) + software-architect (consultivo)
**Substitui:** —
**Substituído por:** —

---

## Contexto

GusWorld está no fim da Fase 1 (Concepção). Estado factual em 2026-05-19:

- **Entregue:** Era 1 §§1-10 deep-lore (~286k pal, contagem antiga) + R2 Facções (22k pal) + R3 Settings deep (25k pal). Total deep-lore canônico: **~365k pal**.
- **Falta no plano original (F1-DL.4 → F1-DL.9):** ~111k pal — Characters deep (~35k), Antagonistas+NPCs (~12k), Magic (~10k), Ontologia (~8k), Stinger (~4k), Antologia Vol 2 (~42k).
- **Decisão fechada:** dois volumes-livro pós-release (F5-BK.1, F5-BK.2) dependem de F1-DL.9 completo + F1-DL.REFAC (refactor fluidez + retrofit dos motivos cerimoniais fundadores + densificação §10).
- **Restrição operacional:** F2-S.1 (instalar Godot 4) está formalmente bloqueada por F1-DL.9 no TODO.md atual ("PAUSADA até deep-lore F1-DL.9 completa").

### Forças em jogo

1. **Fadiga criativa real do criador supremo.** Produzir lore literário denso (voz Stephenson/Sterling, 7 parâmetros narrative-writer, RAG queries, audits) por sessões consecutivas tem retorno marginal decrescente em qualidade. Sinal: user explicitamente reportou cansaço pós-§10 + R2 + R3.
2. **Risco de tunneling.** Continuar lore sem nenhum código rodando = arquitetura permanece teoria. Pillar 3 (triângulo de hardware) ainda não foi validado em protótipo nenhum. Câmera orbital 3/4 (uma decisão visual central) nunca foi tocada em Godot.
3. **Janela de momentum perdida.** Há 6+ meses planejados pra Fase 2 (vertical slice). Cada semana adicional em lore atrasa o primeiro contato com perf real, com export pipeline, com playtester externo.
4. **Lore restante ≠ blocking pra vertical slice.** F1-DL.4-9 produz material pro **livro** (F5-BK.1/2) e pra **profundidade canônica de personagens secundários**. Vertical slice precisa de: Gus + 1 NPC introdutório + 1 inimigo comum + 1 puzzle Vetor do Gambito + 1 área pequena Distritos Inferiores. **Esse material já existe canonicamente** (party.md, gus.md, factions.md, environments/ Bloco F).
5. **Two-way door.** Pivot pra engenharia agora **não destrói** o trabalho de lore feito nem impede retomá-lo. Lore vira atividade paralela orgânica — escreve quando estiver inspirado, não em sprint forçado.
6. **One-way door inverso (perigoso):** continuar 111k pal antes de tocar Godot pode resultar em descobrir, em mês 3 de F2, que uma decisão arquitetural (ex: turn-based hooks) precisa retrofit em 20 docs de lore. Caro.

### O que NÃO é o problema

- **Qualidade de lore.** Os 365k entregues estão validados, audit-approved, canônicos. Era 1 fechada em §10 com voz Verônica Atelaiá / Eco Adso consolidada.
- **Falta de visão criativa.** Pillars, GDD, arco principal, 3 endings, 8 settings, 7 facções — todos validados em sessão colaborativa.
- **Risco de virar projeto técnico genérico.** Motivos numéricos recorrentes + cerimoniais fundadores canon mantidos no TEXTO de qualquer lore futuro. Pillar narrativo intacto.

---

## Decisão

**Pivot imediato pra Engineering Fase 2.** Deep-lore F1-DL.4-9 + F1-DL.REFAC reclassificados de **gating sequencial** pra **trabalho paralelo orgânico** (ritmo livre, sem dependência de F2).

**Reclassificações operacionais:**

1. **F2-S.1 (Instalar Godot 4):** dependência `Pré-requisito: F1-DL.9` **removida**. Pré-requisito atualizado pra `—` (nenhum). Liberado pra execução imediata.
2. **F1-DL.4-9 + F1-DL.REFAC:** prioridade rebaixada de `Alta` pra `Média` no TODO.md. Continuam canônicos no escopo do projeto, mas **não bloqueiam Fase 2**. Bloqueiam apenas F5-BK.1/F5-BK.2 (consolidação livro pós-release).
3. **Cadência sugerida deep-lore paralelo:** 1 sessão de lore a cada 2-3 sessões de código, quando user estiver com energia criativa. Sem meta de palavras por sprint.
4. **Salvaguarda canônica:** qualquer feature de F2 que dependa de detalhe canônico ainda não escrito (ex: spec exato de uma carta rúnica, comportamento de um inimigo específico) **pausa pra micro-sessão de lore pontual** (~500-2000 pal), não pra ciclo completo F1-DL.X. Princípio: lore-on-demand pra desbloquear código, não code-on-hold pra completar lore.

---

## Opções consideradas

### Opção A — Manter plano original (F1-DL.4-9 antes de F2-S.1)

**Prós:**
- Lore 100% canônica antes de qualquer decisão técnica final.
- Volume 1 do livro vira entregável pré-código (marco psicológico).
- Zero risco de descobrir conflito lore/código em mês 3.

**Contras:**
- ~3-6 meses adicionais antes de **primeira linha de Godot rodando**.
- Fadiga real do user (sinal explícito 2026-05-19) — qualidade marginal cai.
- Vertical slice (meta 4-6 meses CLAUDE.md) escorrega pra 9-12 meses.
- Decisões one-way door técnicas (engine modular real, save schema, câmera orbital) seguem sem validação empírica.
- Aumenta probabilidade de scope creep narrativo (cada release narrativa puxa mais expansão).

**Veredito:** rejeitada. Custo de iteração solo > benefício de canon completo antecipado.

### Opção B — Pivot duro: parar lore, só voltar pós-v1.0.0

**Prós:**
- Foco máximo em engineering. Vertical slice em 4-6 meses como planejado.
- Lore restante feita "com peso técnico" — sabendo o que o código precisa.
- F5-BK.1/2 vira pós-release sem ambiguidade.

**Contras:**
- Quebra o fluxo criativo natural do user. Lore às vezes desbloqueia ideias técnicas (Pillar 2 magia=software emergiu de lore, não de design técnico).
- Voz Stephenson/Sterling consolidada em §10 pode esfriar em 6-12 meses de pausa total.
- Decisão binária (tudo ou nada) é frágil — single dev solo precisa de válvula de escape criativa quando código trava.

**Veredito:** rejeitada. Binário demais. Lore como hobby paralelo serve melhor solo indie.

### Opção C — Pivot soft: lore paralelo orgânico (ESCOLHIDA)

**Prós:**
- Engineering Fase 2 começa **agora** (semana 2026-05-19).
- Lore continua canônica e progride no ritmo natural (sem deadline de sprint).
- Lore-on-demand cobre micro-lacunas que aparecem em código (ex: nome exato de um inimigo blockout).
- Reversível: se lore travar engineering em algum ponto específico, faz micro-sessão; se engineering travar e user quiser criar, dispara ciclo deep-lore.
- F5-BK.1/2 mantidos no escopo sem urgência — qualidade > prazo de livro.

**Contras:**
- Risco de lore restante nunca ser completada (mitigado: F1-DL.4-9 não é blocking pra v1.0.0 do jogo, só pro livro).
- Coordenação mental dupla (código + lore) tem custo de context-switch (mitigado: alternância intencional, não simultânea).
- Versionar canon parcial é mais complexo (mitigado: TODO.md + CHARS.md + PLACES.md já são canônicos; novos personagens em lore paralela entram no inventário).

**Veredito:** aceita. Melhor balanço pra solo indie com fadiga real e meta de vertical slice em 4-6 meses.

### Opção D — Reduzir escopo de lore restante (cortar F1-DL.9 Antologia Vol 2 ~42k pal)

**Prós:**
- F1-DL.4-9 cai de ~111k pra ~69k pal. Mais alcançável paralelo.
- Antologia Vol 2 é o item mais longo e mais "extra" (14 contos in-character).

**Contras:**
- Decisão de scope criativo, não arquitetural — fora do mandato deste ADR.
- Antologia Vol 2 é diferenciador real do projeto (poucos jogos indie têm 2-volume bíblia narrativa).

**Veredito:** deferida. Opção C engloba D — em modo paralelo orgânico, user decide ciclo a ciclo se Antologia entra ou não. ADR não precisa congelar isso.

---

## Consequências

### Positivas

- **Momentum técnico imediato.** F2-S.1 desbloqueada. Próxima sessão pode instalar Godot + criar `project.godot`.
- **Validação empírica acelerada de pillars técnicos.** Câmera orbital 3/4 (Pillar 5 setting bipartido) testada em semanas, não meses. Triângulo de hardware (Pillar 3) sai do papel.
- **Risco arquitetural reduzido.** Decisões one-way door (save schema, modular engine layout, signal bus) confrontadas com código real cedo. ADRs subsequentes (ADR-002+) nascem de evidência, não conjectura.
- **Fadiga criativa respeitada.** User produz lore quando inspirado, código quando inspirado. Solo indie sustentável.
- **Vertical slice volta à meta 4-6 meses.** Sem 3-6 meses de lore pré-código, janela original do CLAUDE.md restaurada.
- **Lore-on-demand preserva qualidade.** Quando F2 puxar um detalhe canônico faltante (ex: comportamento exato de inimigo da Selve em combate), micro-sessão de 500-2000 pal cobre — não precisa do bloco completo de 35k.

### Negativas (custos aceitos)

- **Livro Vol 1 atrasa.** F5-BK.1 depende de F1-DL.9 + F1-DL.REFAC; ambos agora "Média prioridade orgânica". Livro provavelmente sai 3-9 meses pós-v1.0.0 do jogo, não junto.
- **Lore parcial em produção.** Companion specs deep (F1-DL.4) podem não estar prontos quando o segundo companion entrar em blockout. Mitigação: party.md + character-spec-*.md canônicos cobrem o essencial pra implementação; deep-lore adiciona profundidade literária, não mecânica.
- **Context-switch cost.** Alternar entre código GDScript e prose Eco Adso tem custo cognitivo. Mitigação: sessões inteiras dedicadas, não micro-alternância (alternância intra-sessão proibida).
- **Decisão depende de disciplina.** "Paralelo orgânico" pode virar "lore nunca" se user nunca tiver inspiração. Mitigação: revisitar ADR em F2-M.1 (vertical slice done) — se F1-DL.4-9 não avançou nada, decisão consciente entre acelerar lore ou cortar Antologia Vol 2.

### Riscos / pontos de atenção

1. **Risco baixo:** descobrir em F2 que lore canônica atual contradiz arquitetura prática. **Mitigação:** lore Era 1-3 já é narrativa pura (não-mecânica); contradições mecânicas seriam em F1-DL.6 (Magic) e F1-DL.5 (Antagonistas+NPCs). Esses podem virar primeiros candidatos a sessões paralelas se F2-G.5/F2-G.6 (combate + puzzle) precisarem.
2. **Risco médio:** F1-DL.REFAC (refactor fluidez §§6-10 + densificação §10 +17k pal) vira eternamente adiado. **Mitigação:** F1-DL.REFAC só bloqueia F5-BK.1 (livro), não jogo. Aceitar atraso ou cortar §10 do livro Vol 1.
3. **Risco médio-alto:** sem cadência forçada, user pode entrar em "modo só código" e nunca voltar ao lore, gerando frustração tardia. **Mitigação:** marcador em F2-M.1 (vertical slice done) — revisão obrigatória de status de F1-DL.* nesse milestone.
4. **Risco baixo:** decisões técnicas em Godot 4 forçarem reescritas em lore (ex: turn-based combat real revelar que carta canônica X é injogável). **Mitigação:** lore atual canônica é descritiva ("Compilação Rúnica combina cartas com tags compatíveis"), não prescritiva ("carta X faz Y dano em Z range"). Espaço de manobra mecânica preservado.

---

## Reversibilidade

**Two-way door.** Decisão totalmente reversível:

- Se em F2 (próximas semanas) user descobrir que lore-vibes são insubstituíveis pra resolver bloqueio criativo técnico, basta dispatch de ciclo deep-lore conforme workflow existente (memo `feedback_deep_lore_colaborativo_rag_visivel`).
- Se em F2-M.1 (vertical slice done) F1-DL.4-9 estiver completamente parada, decisão re-avaliada com dados reais (quanto tempo F2 levou? Vale dedicar 2-3 meses pra completar lore antes de F3?).
- Hard reverse path: re-promover F1-DL.4 a `Alta` + bloquear próxima fase do jogo. Custo: 1 update no TODO.md + 1 sessão de planejamento. Quase zero.

Decisão única não-reversível embutida: a memória organizacional do user de que "lore restante NÃO é gating de código". Isso é cultura, não código. Aceitar.

---

## Ações imediatas (implementação deste ADR)

1. **Atualizar TODO.md:**
   - F2-S.1: `Pré-requisito: F1-DL.9` → `Pré-requisito: —`. Descrição: remover "PAUSADA até deep-lore F1-DL.9 completa".
   - F1-DL.4 → F1-DL.9: prioridade `Alta` → `Média`. Status mantido `⏳ Pendente`.
   - F1-DL.REFAC: prioridade `Alta` → `Média`.
   - Notas TODO.md: adicionar entry citando ADR-001.

2. **Atualizar CLAUDE.md (raiz gusworld):**
   - Seção "Estado atual": "**Fim de Fase 1 (Concepção) — REVISÃO 1. Início de Fase 2 (Engineering) — 2026-05-19. Deep-lore restante paralelo orgânico (ver ADR-001).**"
   - Seção "Próximos passos (Fase 2)": validar que enumeração reflete realidade pós-pivot.

3. **Não atualizar:** memos `~/.claude/projects/.../memory/` salvo se user pedir. ADR é canônico no repo; memos são state-keeping de sessão.

---

**Fim do ADR-001.**
