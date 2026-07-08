# Deep-Lore: GusWorld

> **Status:** Canônico (camada deep-lore). Paralelo aos blocos F/G/H/I existentes. **Expande sem contradizer.**
>
> **Escopo:** lore profunda destinada a consolidação em livro pós-release (Volume 1 = bíblia worldbuilding + Volume 2 = antologia narrativa). Cobre eras, facções, settings, personagens, magia, ontologia, stinger.
>
> **Estilo literário canônico:** **Stephenson/Sterling cyberpunk denso** (prosa densa, neologismos técnicos, justaposição jargão computacional + lirismo). Referências autorais: Snow Crash, Cryptonomicon, Schismatrix.
>
> **Cross-ref imutáveis:**
> - Pillars: `docs/design/pillars.md`
> - Canon Bloco G: `docs/narrative/lore-bible.md`, `timeline.md`, `factions.md`, `in-world-docs.md`, `tradicoes-cultura.md`
> - Canon Bloco F: `docs/narrative/environments/*` (8 settings)
> - Canon Bloco H: `docs/narrative/diary/*` (Diário Gus)
> - Canon Bloco I: `docs/narrative/foreshadowing.md` (130 plants)
> - Character canon: `docs/narrative/characters/*` (10 docs) + `Resources/gusworld/character-spec-*.md` (8 specs visuais)

---

## Princípio do deep-lore

Camada **complementar e não-substitutiva**. Cada doc deep:

1. **Expande** profundidade autoral além do necessário pro jogo (referência para autor + livro final).
2. **Respeita absolutamente** decisões canônicas existentes (pillars, terminologia, nomes, roster, antagonistas, endings).
3. **Não toca** docs canon vigentes (Bloco F/G/H/I + characters) sem aprovação explícita do criador supremo.
4. **Calibra revelações ao player**: autor sabe tudo; Diário (Bloco H) + in-world-docs (Bloco G) controlam exposição gradual via Knowledge Progression (gates Bronze/Prata/Ouro).
5. **Construído com auxílio do RAG** (266 livros indexados em `resources/livros/rag/`): agente faz auto-queries semânticas (5-8 por doc, top-k=5) extraindo padrões/conceitos de referência (Tolkien worldbuilding, Asimov sci-fi, Rand filosofia, Sun Tzu estratégia, etc.). **Não copia trechos**; adapta ao canon GusWorld.

---

## Estrutura da pasta

```
docs/narrative/deep/
├── _INDEX.md                       # este doc
├── eras/                           # 3 eras + transições (~18k palavras)
│   ├── era-1-pre-codigo.md         # Neo-Sylvania (~5k)
│   ├── era-2-compilador.md         # Era do Compilador (~5k)
│   ├── era-3-sterling.md           # Era Sterling presente (~5k)
│   └── transicoes-entre-eras.md    # Quedas, hiatos, restos (~3k)
├── factions/                       # 6 facções principais + menores (~20k)
│   ├── sterling-corp.md            # (~3k)
│   ├── fir.md                      # Federação Industrial de Reciclagem (~3k)
│   ├── ordem-recursiva.md          # (~3k)
│   ├── cult-mirage-reality.md      # (~3k)
│   ├── underground-silencio.md     # (~3k)
│   ├── pelicano-branco.md          # (~3k)
│   └── facoes-menores.md           # (~2k)
├── settings/                       # 8 locais deep (~26k)
│   ├── 01-cidade.md                # (~4k crítico)
│   ├── 02-selve-sombria.md         # (~4k crítico)
│   ├── 03-catedrais.md             # (~3k)
│   ├── 04-dutos.md                 # (~3k)
│   ├── 05-mirage.md                # (~3k)
│   ├── 06-periferia.md             # (~3k)
│   ├── 07-silencio.md              # (~2k)
│   └── 08-selve-profunda.md        # (~4k crítico, climax)
├── characters/                     # Gus + 6 companions + Sterling + PZ + NPCs (~47k)
│   ├── gus-dragon.md               # Gus "Dragon" Vector Tavus Vance (~5k crítico)
│   ├── caua-volt.md                # Cauã "Volt" Berenger (~5k)
│   ├── iara-lumen.md               # Iara "Lumen" Koslov (~5k)
│   ├── bento-requiem.md            # Bento "Requiem" Chevalier (~5k)
│   ├── linda-siren.md              # Linda "Siren" Neumann (~5k)
│   ├── dante-grid.md               # Dante "Grid" Alencar, TRAIDOR (~5k crítico)
│   ├── jaci-proxy.md               # Jaci "Proxy" Vanderbist (~5k)
│   ├── sterling-locke.md           # Antagonista (~5k crítico)
│   ├── patch-zero.md               # Antagonista-sistema (~4k)
│   └── npcs-secundarios.md         # Antologia de NPCs nomeados (~3k)
├── magic/                          # Sistema mágico formal (~10k)
│   ├── glyph-token-conjuro-codex.md   # Metafísica formal (~3k)
│   ├── linguagens-magicas.md          # C-Arcane/Asmódico/Óxido/Pythia deep (~4k)
│   └── natureza-matematica.md         # Pillar 2 expansão fractais/sequência recorrente/ruído (~3k)
├── ontologia/                      # Conceito macro (~8k)
│   ├── tecnologia-3-eras.md        # Hardware/substrato 3 eras (~3k)
│   ├── cosmologia-formal-deep.md   # Cosmovisão de superfície: gramática computável, vetor central, Patch-Zero (~3k)
│   │                               #   companheiro: ../eras/cosmologia-origem-deep.md (origem/verdade enterrada: Estilhaçamento + ecos convergentes)
│   └── leitmotivs-temas.md         # Themes recorrentes (~2k)
├── stinger/                        # Sequel hooks (~4k)
│   ├── sequel-hooks.md             # (~2k)
│   └── post-credits-narrativos.md  # (~2k)
└── antologia/                      # Volume 2, 14 contos in-character (~42k)
    ├── _PLANO.md                   # outline dos 14 contos
    ├── ANT-001 ... ANT-014.md      # 1 conto por arco (1 por companion + 2 antag + 3 era + 2 NPC)
```

**Total estimado Volume 1:** ~133k palavras (~500 páginas A4).
**Total estimado Volume 2:** ~42k palavras.

---

## Convenções autorais (canônicas para deep-lore)

### Estilo Stephenson/Sterling cyberpunk denso
- Prosa densa, neologismos técnicos in-universe livres (mas alinhados ao canon: não inventar termo novo que contradiga Glyph/Token/Conjuro/Codex)
- Justaposição jargão computacional + lirismo (ex: "o compilador respira como pulmão de cinza")
- Cláusulas longas com aposição rica, sem temer subordinação
- Voltagem técnica + voltagem emocional alternando intencionalmente
- Diferente de Tolkien grandioso (sem tom épico-mítico sustentado); mantém GusWorld terreno
- Diferente de Asimov direto; permite ambiguidade lírica

### Híbrido voz autoral + in-character
- Cada seção principal: **voz autoral neutra** (3ª pessoa, observador externo informado)
- **2-3 trechos in-character por seção** (carta, fragmento de diário, transcrição, depoimento) marcados como citação destacada
- Trechos in-character assinados por personagens canônicos (cronistas in-universe: Hugo Tirol, Inácia Berenger, Padrinho Tiago, Verônica Atelaiá, etc.); **não inventar cronista novo sem aprovação**

### Em-dash (PROIBIDO desde 2026-05-16)
- `docs/narrative/deep/` **NÃO permite em-dash horizontal (U+2014)** em nenhum contexto. Regra unificada pós cascata global 2026-05-23.
- Substituição obrigatória por vírgula, dois pontos, parênteses, ponto-e-vírgula, ou ponto + frase nova.
- **Única exceção restante:** `docs/narrative/in-world-docs.md` (ver memo `project_em_dash_excecao.md` atualizado).

### Terminologia obrigatória
- **Glyph / Token / Conjuro / Codex** (NUNCA "runa")
- **C-Arcane / Asmódico / Óxido / Pythia** (4 linguagens canônicas; set híbrido mítico)
- **FIR** (NUNCA "Sindicato dos Ferro-Velhos")
- **Sterling Locke** (antagonista, NUNCA Iolanda ou outros)
- **Patch-Zero** (antagonista-sistema)
- **Sem palavrão** (Pillar 4.3 absoluto)
- **Sem adultos como party** (peers 11-14 only)
- **Codinomes canon** (Dragon/Volt/Lumen/Requiem/Siren/Grid/Proxy)

### Conformidade RAG
- Cada doc deep deve declarar 5-8 queries RAG executadas no rodapé técnico (não no doc final consolidado)
- Citação opcional discreta em rodapé pra trace de inspiração (sem quebrar imersão)
- Adaptação obrigatória; nunca copy-paste do RAG

---

## Decisões macro registradas

| Item | Decisão |
|---|---|
| Forma livro final | Bíblia (vol 1) + Antologia (vol 2) |
| Estratégia RAG | Auto-query priori por arco (5-8 queries, top-k=5) |
| Profundidade | Variável por categoria (Gus/Sterling/companions 5k; NPCs/menores 1.5-2k) |
| Ordem produção | Top-down: lore (eras) → facções → settings → personagens → magic → ontologia → stinger → antologia |
| Estilo literário | Stephenson/Sterling cyberpunk denso |
| Era 1 tratamento | Completa pro autor, calibrada pro player (gate Ouro) |
| Cronograma | Deep-lore prioritário (paralelo orgânico à Fase 2: vertical slice na engine própria C++20 + SDL3, em andamento) |
| Estrutura Vol 1 | Híbrida (voz autoral + trechos in-character) |
| Idioma | pt-br (i18n estrutura ready desde dia 1; tradução en-intl pós-release v1.0.0) |
| Em-dash | Permitido nesta camada (exceção formal) |

---

## Workflow agents

1. **`narrative-writer`** = produção. Gera prose long-form aplicando 7 parâmetros prévios + RAG queries + estilo Stephenson/Sterling. Modo autônomo autorizado APÓS user fechar decisões macro de cada rodada.
2. **`narrative-designer`** = validação. Audita coerência cross-canon (vs Bloco F/G/H/I + pillars + character specs). Identifica contradições antes de canonizar. **Lore guardian.**
3. **User** = aprovação final. Decisões one-way doors (pillars, antagonistas, endings, render style, nomes canônicos) **sempre exigem confirmação user**, mesmo em modo autônomo.

Ordem por rodada:
```
user fecha decisões macro da rodada
   → narrative-writer produz N docs em paralelo (até 3 por rodada)
      → narrative-designer audita coerência
         → user aprova ou pede revisão
            → canon
```

---

## Status (rodadas)

| Rodada | Conteúdo | Status |
|---|---|---|
| R1 (Eras) | era-1, era-2, era-3, transicoes (4 docs) | ✅ Concluído (2026-05-26) |
| R2 (Facções) | 6 principais + menores (7 docs, ~20k) | ⏳ Pendente |
| R3 (Settings) | 8 settings deep (8 docs, ~26k) | ⏳ Pendente |
| R4 (Characters) | Gus + 6 companions (7 docs, ~35k) | ⏳ Pendente |
| R5 (Antagonistas + NPCs) | Sterling + Patch-Zero + NPCs antologia (3 docs, ~12k) | ⏳ Pendente |
| R6 (Magic) | Glyph/Token + linguagens + nat-mat (3 docs, ~10k) | ⏳ Pendente |
| R7 (Ontologia) | Tech/cosmologia/leitmotivs (3 docs, ~8k) | ⏳ Pendente |
| R8 (Stinger) | Sequel hooks + post-credits (2 docs, ~4k) | ⏳ Pendente |
| R9 (Antologia Vol 2) | 14 contos in-character (14 docs, ~42k) | ⏳ Pendente |
| R10 (Consolidação livro) | Vol 1 + Vol 2 com prefacio, sumário, glossário, índice | ⏳ Pendente |

---

## Cross-refs

- [[../pillars]]: 5 pillars canônicos
- [[../lore-bible]]: Bloco G fundação
- [[../arco-principal]]: 8 beats Kishōtenketsu + 3 endings
- [[../foreshadowing]]: 130 plants
- [[../characters/_INDEX]] (se houver) ou docs/narrative/characters/: 10 docs canon
- [[../environments/_INDEX]]: Bloco F 9 docs
- [[../diary/_INDEX]]: Bloco H 8 docs
- Memórias: [[../../../../memory/project_pillars_canonicos]] [[../../../../memory/project_personagens]] [[../../../../memory/feedback_nomes_personagens_canonicos]] [[../../../../memory/project_terminologia]] [[../../../../memory/project_i18n_canonico]]

---

**Última revisão:** 2026-05-16. Camada deep-lore inicializada. Aguardando R1 (Eras).
