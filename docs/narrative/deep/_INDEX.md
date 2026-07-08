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
> - Character canon: `docs/narrative/characters/*` (13 docs no disco) + `Resources/gusworld/character-spec-*.md` (8 specs visuais)

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

> Árvore regravada 2026-07-08 (AUD-LORE) a partir do `find docs/narrative/deep/` real. Nomes de arquivo abaixo são os do disco, não os planejados na versão anterior deste índice.

```
docs/narrative/deep/
├── _INDEX.md                            # este doc
├── eras/                                # 3 eras + transições + cosmologia-origem
│   ├── era-1-pre-codigo.md              # Neo-Sylvania, §§1-10 (maior doc da camada, ~318k pal)
│   ├── era-2-boom-tecnico.md            # Era do Compilador
│   ├── era-3-sterling.md                # Era Sterling presente
│   ├── transicoes-entre-eras.md         # Quedas, hiatos, restos
│   └── cosmologia-origem-deep.md        # origem/verdade enterrada: Estilhaçamento + ecos convergentes; companheiro de ontologia/cosmologia-formal-deep.md
├── factions/                            # 6 facções principais + menores
│   ├── sterling-corp.md
│   ├── fir.md                           # Federação Industrial de Reciclagem
│   ├── ordem-recursiva.md
│   ├── cult-mirage.md
│   ├── underground-silencio.md
│   ├── pelicano-branco.md
│   └── facoes-menores.md
├── settings/                            # 8 locais deep
│   ├── 01-cidade.md                     # crítico
│   ├── 02-selve-sombria.md              # crítico
│   ├── 03-catedrais.md
│   ├── 04-dutos.md
│   ├── 05-mirage.md
│   ├── 06-periferia.md
│   ├── 07-silencio.md
│   └── 08-selve-profunda.md             # crítico, climax
├── characters/                          # Gus + companions (Sterling/Patch-Zero/NPCs viraram antagonists/)
│   ├── gus-dragon.md                    # Gus "Dragon" Vector Tavus Vance, crítico
│   ├── caua-volt.md                     # Cauã "Volt" Berenger
│   ├── iara-lumen.md                    # Iara "Lumen" Koslov
│   ├── bento-requiem.md                 # Bento "Requiem" Chevalier
│   ├── linda-siren.md                   # Linda "Siren" Neumann
│   ├── dante-grid.md                    # Dante "Grid" Alencar, TRAIDOR, crítico
│   └── jaci-proxy.md                    # Jaci "Proxy" Vanderbist
├── antagonists/                         # Sterling + Patch-Zero + NPCs antagonistas (pasta antes invisível neste índice)
│   ├── sterling-locke-deep.md           # antagonista principal, crítico
│   ├── patch-zero-deep.md               # antagonista-sistema
│   └── npcs-antagonistas.md             # NPCs antagonistas nomeados (renomeado de npcs-antologia.md, AUD-LORE D11, tira colisão com antologia/)
├── magic/                               # Sistema mágico formal
│   ├── glyph-token-conjuro-codex-deep.md   # Metafísica formal
│   ├── 4-linguagens-deep.md             # C-Arcane/Asmódico/Óxido/Pythia deep
│   └── natureza-matematica-rigida-deep.md  # Pillar 2 expansão fractais/sequência recorrente/ruído
├── ontologia/                           # Conceito macro
│   ├── tech-3-eras-deep.md              # Hardware/substrato 3 eras
│   ├── cosmologia-formal-deep.md        # Cosmovisão de superfície: gramática computável, vetor central, Patch-Zero
│   ├── leitmotivs-deep.md               # Temas recorrentes
│   └── leitmotivs-musicais-detalhados.md   # Bíblia de leitmotivs musicais por personagem/facção
├── stinger/                             # Sequel hooks
│   ├── sequel-hooks-deep.md
│   └── post-credits-deep.md
└── antologia/                           # Volume 2, 14 contos in-character
    ├── 01-gus-gustaf-vii.md
    ├── 02-caua-volt.md
    ├── 03-iara-lumen.md
    ├── 04-bento-requiem.md
    ├── 05-linda-siren.md
    ├── 06-dante-grid.md
    ├── 07-jaci-proxy.md
    ├── 08-sterling-locke.md
    ├── 09-cassiano-vorto.md
    ├── 10-pyotor-vance-pai.md
    ├── 11-gargi-vance-mae.md
    ├── 12-ancia-mariana-vanderbist.md
    ├── 13-veronica-atelaia.md
    └── 14-heliaco-vyr-era-1.md
```

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
| R1 (Eras) | era-1, era-2, era-3, transicoes, cosmologia-origem (5 docs) | ✅ Concluído (2026-05-26) |
| R2 (Facções) | 6 principais + menores (7 docs) | ✅ Concluído (arquivos presentes no disco) |
| R3 (Settings) | 8 settings deep (8 docs) | ✅ Concluído (arquivos presentes no disco) |
| R4 (Characters) | Gus + 6 companions (7 docs) | ✅ Concluído (arquivos presentes no disco) |
| R5 (Antagonistas + NPCs) | Sterling + Patch-Zero + NPCs antagonistas, pasta `antagonists/` (3 docs) | ✅ Concluído (arquivos presentes no disco) |
| R6 (Magic) | Glyph/Token + linguagens + nat-mat (3 docs) | ✅ Concluído (arquivos presentes no disco) |
| R7 (Ontologia) | Tech/cosmologia/leitmotivs (4 docs) | ✅ Concluído (arquivos presentes no disco) |
| R8 (Stinger) | Sequel hooks + post-credits (2 docs) | ✅ Concluído (arquivos presentes no disco) |
| R9 (Antologia Vol 2) | 14 contos in-character (14 docs) | ✅ Concluído (arquivos presentes no disco) |
| R10 (Consolidação livro) | Vol 1 + Vol 2 com capa, prefácio, sumário, glossário, apêndices (`docs/book/`) | ✅ Concluído (arquivos presentes no disco) |

---

## Cross-refs

- [[../pillars]]: 5 pillars canônicos
- [[../lore-bible]]: Bloco G fundação
- [[../arco-principal]]: 8 beats Kishōtenketsu + 3 endings
- [[../foreshadowing]]: 130 plants
- [[../characters/_INDEX]] (se houver) ou docs/narrative/characters/: 13 docs canon no disco
- [[../environments/_INDEX]]: Bloco F 9 docs
- [[../diary/_INDEX]]: Bloco H 8 docs
- Memórias: [[../../../../memory/project_pillars_canonicos]] [[../../../../memory/project_personagens]] [[../../../../memory/feedback_nomes_personagens_canonicos]] [[../../../../memory/project_terminologia]] [[../../../../memory/project_i18n_canonico]]

---

**Última revisão:** 2026-07-08 (AUD-LORE, remediação Onda 1). Árvore regravada a partir do disco real; todas as 10 rodadas R1-R10 têm arquivos presentes.
