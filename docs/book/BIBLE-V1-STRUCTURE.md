# GusWorld — Bíblia Worldbuilding, Volume 1

> **Status:** Estrutura TOC unificada (proposta de consolidação editorial). Não é livro redigido; é blueprint de organização do canon existente em forma de obra de referência publicável.
>
> **Autor das fontes:** criador supremo + squad criativo (`narrative-designer`, `narrative-writer`, `lead-game-designer`, `art-director`) ao longo de 2026-05-12 a 2026-05-21.
>
> **Audiência-alvo:** colecionador do jogo (post-release), pesquisador de worldbuilding, autor (referência interna canon), tradutor (en-intl pós v1.0.0).
>
> **Volume 2 (separado):** Antologia narrativa em 14 contos in-character (`docs/narrative/deep/antologia/*`), publicada em livro próprio.
>
> **Convenção de medida:** ~300 palavras por página A4 corpo 11/12 com margens normais. Estimativas declaradas em palavras (fonte canônica) + páginas derivadas.

---

## Sumário executivo

| Métrica | Valor |
|---|---|
| Partes | 9 + Apêndices |
| Capítulos | 18 |
| Sub-capítulos | ~72 |
| Palavras totais estimadas (corpo) | ~480k |
| Palavras totais estimadas (apêndices) | ~85k |
| **Palavras totais Vol 1** | **~565k** |
| **Páginas totais Vol 1** | **~1.880 pp** (corpo ~1.600 + apêndices ~280) |
| Apêndices | 7 (A-G) |
| Fontes consolidadas | 78 arquivos `.md` canon |

> **Nota sobre tamanho:** o volume cresceu além de estimativa original (~133k pal) porque Era 1 sozinha foi expandida pra ~218k pal (REFAC-2). Sugestão editorial: considerar **divisão em dois tomos físicos** (Tomo I = Partes I-IV, ~620 pp; Tomo II = Partes V-IX + Apêndices, ~1.260 pp), ou impressão em formato grande com papel fino tipo bíblia.

---

## Mapa visual da obra

```
VOLUME 1 — BÍBLIA WORLDBUILDING
│
├── PARTE I       FRONTISPÍCIO                    (~28k pal /  ~95 pp)
├── PARTE II      COSMOLOGIA                      (~44k pal / ~145 pp)
├── PARTE III     AS TRÊS ERAS                    (~250k pal / ~835 pp)  [bloco maior]
├── PARTE IV      MAGIA E LINGUAGEM               (~36k pal / ~120 pp)
├── PARTE V       PERSONAGENS                     (~90k pal / ~300 pp)
├── PARTE VI      SETTINGS                        (~58k pal / ~195 pp)
├── PARTE VII     FACÇÕES                         (~44k pal / ~145 pp)
├── PARTE VIII    LEITMOTIVS E ESTÉTICA           (~14k pal /  ~45 pp)
├── PARTE IX      STINGER E LEGADO               (~9k pal /  ~30 pp)
│
└── APÊNDICES (A-G)                               (~85k pal / ~285 pp)
```

---

# PARTE I — FRONTISPÍCIO

**Função:** abrir o livro, declarar premissa, dar ao leitor as cinco regras de leitura (pillars) e o manifesto autoral que sustenta tudo.

**Estimativa parte:** ~28k pal / ~95 pp

---

## Capítulo 1 — Sinopse e Visão Geral

| # | Sub-capítulo | Fonte | Palavras | Páginas | Nota revisão |
|---|---|---|---|---|---|
| 1.1 | Sinopse canônica | `sinopse.md` | ~3k | ~10 | imutável — base canon |
| 1.2 | Premissa em três princípios | `docs/narrative/lore-bible.md` §1-2 | ~1.5k | ~5 | extrair + leve adaptação editorial |
| 1.3 | Theme central: a inteligência mais alta serve à vida | `lore-bible.md` §2 | ~0.5k | ~2 | extrair (1 frase + parágrafo expansão) |
| 1.4 | Como ler esta bíblia (manual do leitor) | **novo, redacional** | ~2k | ~7 | encomendar a `narrative-writer` (estilo Stephenson/Sterling) |
| 1.5 | Cronologia macro de produção (2026-05-12 a release) | **novo, editorial** | ~1k | ~3 | resumo histórico do projeto |

**Subtotal Cap 1:** ~8k pal / ~27 pp

---

## Capítulo 2 — Pillars Canônicos e Manifesto Autoral

| # | Sub-capítulo | Fonte | Palavras | Páginas | Nota revisão |
|---|---|---|---|---|---|
| 2.1 | Cinco pillars testáveis | `docs/design/pillars.md` | ~6k | ~20 | extrair íntegra (canon validado) |
| 2.2 | Anti-pillars (boundaries) | `pillars.md` §Anti-pillars | ~3k | ~10 | extrair íntegra |
| 2.3 | Manifesto Pillar 2 expandido (magia = software, natureza = matemática) | `deep/magic/natureza-matematica-rigida-deep.md` (extrato + abertura ensaio) | ~5k | ~17 | encomendar extrato editorial a `narrative-writer` |
| 2.4 | Easter eggs pervasivos canônicos (Fibonacci + maçonaria) | `CLAUDE.md` §Easter eggs + memórias `project_fibonacci_easter_egg` + `project_eastereggs_maconaria_canonica` | ~4k | ~13 | escrever apresentação autoral discreta (não didática); revelação parcial |
| 2.5 | Convenções editoriais (em-dash, terminologia, voz autoral) | `deep/_INDEX.md` §Convenções autorais | ~2k | ~7 | extrair + adaptar pra prefacio editorial |

**Subtotal Cap 2:** ~20k pal / ~67 pp

**Subtotal Parte I:** ~28k pal / ~94 pp

---

# PARTE II — COSMOLOGIA

**Função:** dar ao leitor o esqueleto ontológico antes de mergulhar nas eras. Sem cosmologia, o leitor não entende por que magia e biologia operam pelas mesmas regras.

**Estimativa parte:** ~44k pal / ~145 pp

---

## Capítulo 3 — Sistema Formal Computável

| # | Sub-capítulo | Fonte | Palavras | Páginas | Nota revisão |
|---|---|---|---|---|---|
| 3.1 | Pós-apocalipse digital sem meta-simulação | `lore-bible.md` §4 + `deep/ontologia/cosmologia-deep.md` | ~5k | ~17 | consolidar duas fontes; remover duplicação |
| 3.2 | A tese de Neo-Sylvania: o mundo já era programável | `deep/eras/era-1-pre-codigo.md` §1 (frontispício) — extrato | ~3k | ~10 | extrato editorial (1ª pessoa do cronista) |
| 3.3 | Substratos suportados (silício, latão, proteína, mineral ressonante) | `deep/ontologia/tech-3-eras-deep.md` §1-3 | ~5k | ~17 | extrair com adaptação |
| 3.4 | Limites do sistema (custo, escopo, tipo, terminação) | `deep/magic/glyph-token-conjuro-codex-deep.md` §sistema formal | ~4k | ~13 | extrair |

**Subtotal Cap 3:** ~17k pal / ~57 pp

---

## Capítulo 4 — Natureza como Matemática Rígida

| # | Sub-capítulo | Fonte | Palavras | Páginas | Nota revisão |
|---|---|---|---|---|---|
| 4.1 | Fibonacci como assinatura biótica universal | `deep/magic/natureza-matematica-rigida-deep.md` §Fibonacci | ~4k | ~13 | extrair |
| 4.2 | Fractais, recursão, ruído coerente | `natureza-matematica-rigida-deep.md` §fractais + ruído | ~5k | ~17 | extrair |
| 4.3 | Anomalias e bugs (quando a Selve cospe runtime error) | `natureza-matematica-rigida-deep.md` §anomalias | ~4k | ~13 | extrair |
| 4.4 | Selve Sombria como compilador vivo (case study) | `deep/settings/02-selve-sombria.md` + `08-selve-profunda.md` (extratos) | ~5k | ~17 | extrato editorial |
| 4.5 | Bestiary computacional (sequências como espécies) | **novo, derivado** de `deep/eras/era-1-pre-codigo.md` §bestiary + `diary/entries-fichas-bestiary.md` | ~9k | ~30 | consolidar fichas em texto enciclopédico |

**Subtotal Cap 4:** ~27k pal / ~90 pp

**Subtotal Parte II:** ~44k pal / ~147 pp

---

# PARTE III — AS TRÊS ERAS

**Função:** coluna vertebral histórica da obra. Bloco maior do livro (~835 pp). Era 1 é desproporcional em peso porque é a fundação esquecida que ressoa em todo o resto.

**Estimativa parte:** ~250k pal / ~835 pp

---

## Capítulo 5 — Era 1: Pré-Código (Neo-Sylvania)

> **Tom:** ensaio arqueológico em prosa densa Stephenson/Sterling. O cronista da Era 2 fala em primeira pessoa metódica. Bloco mais longo do livro.

| # | Sub-capítulo | Fonte | Palavras | Páginas | Nota revisão |
|---|---|---|---|---|---|
| 5.1 | Frontispício | `deep/eras/era-1-pre-codigo.md` §1 | ~7k | ~23 | extrair íntegra |
| 5.2 | Cronologia material (8 horizontes datados) | `era-1-pre-codigo.md` §2 | ~20k | ~67 | extrair íntegra |
| 5.3 | Arqueologia material (cripto-glifo, catedrais, sementes-relíquia) | `era-1-pre-codigo.md` §3 | ~22k | ~73 | extrair íntegra |
| 5.4 | Sistema técnico vivo (Asmódico ancestral, hidráulica, fungo em malha) | `era-1-pre-codigo.md` §4 | ~24k | ~80 | extrair íntegra |
| 5.5 | Ética material e frugalidade calculada | `era-1-pre-codigo.md` §5 | ~22k | ~73 | extrair íntegra |
| 5.6 | Famílias-Pilastra, Pelicano Branco, linhagens fundadoras | `era-1-pre-codigo.md` §6 + memória `project_familia_vance_canonica` | ~25k | ~83 | extrair íntegra + reforçar canon Vance |
| 5.7 | O Regime Vyrcátrix (centralização e ruína) | `era-1-pre-codigo.md` §7 | ~24k | ~80 | extrair íntegra |
| 5.8 | Helíaco Vyr (eco de Hiram Abiff, lenda fundadora) | `era-1-pre-codigo.md` §8 + `deep/antologia/14-heliaco-vyr-era-1.md` (entrada autoral) | ~25k | ~83 | consolidar (cap 5.8 = ensaio; antologia = conto pessoal) |
| 5.9 | O Êxodo de -720 e o silêncio que sobrou | `era-1-pre-codigo.md` §9 | ~24k | ~80 | extrair íntegra |
| 5.10 | Eco no presente (como Era 1 ressoa em Era 3) | `era-1-pre-codigo.md` §10 + tese Velhusto (cross-ref Pillar 2) | ~25k | ~83 | extrair íntegra |

**Subtotal Cap 5:** ~218k pal / ~725 pp

> **Nota crítica de revisão:** Era 1 §§6-8 receberam retrofit em 2026-05-21 (Plano 1 + 3 REFAC) integrando easter eggs maçom canon (Pigpen ↔ cripto-glifo, Helíaco Vyr ↔ Hiram Abiff, pavimento tesselado, acaceiro). Validar com `narrative-designer` que densidade ficou em 10-15% (não didática, não evidente).

---

## Capítulo 6 — Era 2: Era do Compilador

| # | Sub-capítulo | Fonte | Palavras | Páginas | Nota revisão |
|---|---|---|---|---|---|
| 6.1 | A retomada pragmática | `lore-bible.md` §3.2 + `timeline.md` §Era 2 | ~3k | ~10 | consolidar |
| 6.2 | A invenção do C-Arcane | `deep/magic/4-linguagens-deep.md` §C-Arcane história | ~3k | ~10 | extrato editorial |
| 6.3 | Cooperativismo, código aberto, Tomo da Pilha Sobrecarregada | `lore-bible.md` §3.2 + `comic-reliefs.md` cena 7 | ~2k | ~7 | consolidar tom respeitoso |
| 6.4 | A fundação de GusWorld City (Gustaf I, ~-150) | `timeline.md` + memória `project_nome_gus_canon` + extratos `deep/antologia/10-pyotor-vance-pai.md` | ~4k | ~13 | escrever cap conector (Gustaf I → VII, 7 gerações) |
| 6.5 | A FIR em sua boa-fé original | `factions.md` §FIR + `deep/factions/fir.md` §história | ~3k | ~10 | extrato |
| 6.6 | A Ordem Recursiva como guardiã das catedrais | `deep/factions/ordem-recursiva.md` + `deep/settings/03-catedrais.md` | ~3k | ~10 | consolidar |
| 6.7 | Verônica Atelaiá e os cronistas comparativos | `deep/antologia/13-veronica-atelaia.md` + `era-1-pre-codigo.md` §codificação institucional Atelaiá Chevalier | ~2k | ~7 | conector entre Era 1 e Era 2 |

**Subtotal Cap 6:** ~20k pal / ~67 pp

---

## Capítulo 7 — Era 3: Era Sterling (presente do jogo)

| # | Sub-capítulo | Fonte | Palavras | Páginas | Nota revisão |
|---|---|---|---|---|---|
| 7.1 | A tese de doutorado (DRE — Dynamic-Runtime Evaluation) | `lore-bible.md` §3.3 + `characters/sterling-locke.md` §tese + `deep/antagonists/sterling-locke-deep.md` §1 | ~3k | ~10 | consolidar |
| 7.2 | A subida corporativa de Sterling Locke | `deep/antagonists/sterling-locke-deep.md` §2 + `lore-bible.md` §3.3 | ~4k | ~13 | extrair |
| 7.3 | Apex-Data, Nexus-Cloud, Core-Synth: canibalização de três conglomerados | `characters/sterling-locke.md` §canibalização | ~3k | ~10 | extrair |
| 7.4 | Sterling Corp consolidada + Cúpula Sterling | `deep/antagonists/sterling-locke-deep.md` §3 + `deep/factions/sterling-corp.md` | ~5k | ~17 | consolidar |
| 7.5 | A FIR vassala (mafiosa lavando ativos) | `deep/factions/fir.md` §captura | ~3k | ~10 | extrair |
| 7.6 | Janelarum, terminais públicos, infraestrutura privatizada | `comic-reliefs.md` cena 11 + `lore-bible.md` §3.3 | ~2k | ~7 | consolidar (tom satírico controlado) |
| 7.7 | Operação GRE — Global Runtime Environment | `lore-bible.md` §3.3 + `deep/antagonists/sterling-locke-deep.md` §GRE | ~3k | ~10 | extrair |
| 7.8 | Patch-Zero — o antagonista-sistema | `characters/patch-zero.md` + `deep/antagonists/patch-zero-deep.md` | ~5k | ~17 | consolidar |
| 7.9 | A Selve resistindo (anomalias de runtime no presente) | `lore-bible.md` §3.3 final + `deep/settings/02-selve-sombria.md` §anomalias | ~3k | ~10 | extrato editorial |

**Subtotal Cap 7:** ~31k pal / ~104 pp

**Subtotal Parte III:** ~269k pal / ~896 pp

> **Nota editorial:** se considerar divisão em dois tomos físicos, o corte natural é entre Cap 5 (fecha Tomo I com ~725 pp Era 1) e Cap 6 (abre Tomo II).

---

# PARTE IV — MAGIA E LINGUAGEM

**Função:** explicar formalmente o sistema GTCC (Glyph/Token/Conjuro/Codex) e as quatro linguagens canônicas. Leitor sai daqui sabendo escrever uma instrução básica em C-Arcane.

**Estimativa parte:** ~36k pal / ~120 pp

---

## Capítulo 8 — Sistema GTCC e as Quatro Linguagens

| # | Sub-capítulo | Fonte | Palavras | Páginas | Nota revisão |
|---|---|---|---|---|---|
| 8.1 | Glyph (átomo sintático) | `deep/magic/glyph-token-conjuro-codex-deep.md` §Glyph | ~3k | ~10 | extrair |
| 8.2 | Token (palavra com tipo) | `glyph-token-conjuro-codex-deep.md` §Token | ~3k | ~10 | extrair |
| 8.3 | Conjuro (instrução compilada) | `glyph-token-conjuro-codex-deep.md` §Conjuro | ~3k | ~10 | extrair |
| 8.4 | Codex (programa mágico) | `glyph-token-conjuro-codex-deep.md` §Codex | ~3k | ~10 | extrair |
| 8.5 | C-Arcane (Gus, baixo nível compilado) | `deep/magic/4-linguagens-deep.md` §C-Arcane | ~5k | ~17 | extrair |
| 8.6 | Asmódico (Bento, analógico Neo-Sylvania) | `4-linguagens-deep.md` §Asmódico | ~5k | ~17 | extrair (cross-ref Era 1 §3) |
| 8.7 | Óxido (Iara, Linda — alta segurança) | `4-linguagens-deep.md` §Óxido | ~5k | ~17 | extrair |
| 8.8 | Pythia (Cauã, Jaci — scripting rápido bio-hacking) | `4-linguagens-deep.md` §Pythia | ~5k | ~17 | extrair |
| 8.9 | Comparativa cruzada das quatro linguagens (custo, escopo, runtime) | `4-linguagens-deep.md` §comparativa | ~4k | ~13 | extrair |

**Subtotal Cap 8:** ~36k pal / ~121 pp

**Subtotal Parte IV:** ~36k pal / ~121 pp

---

# PARTE V — PERSONAGENS

**Função:** dossiê completo do elenco canônico. Tom autoral biográfico denso, com 1-2 trechos in-character por personagem (carta, fragmento de diário, depoimento).

**Estimativa parte:** ~90k pal / ~300 pp

---

## Capítulo 9 — A Party (Gus + Seis Companions)

| # | Sub-capítulo | Fonte | Palavras | Páginas | Nota revisão |
|---|---|---|---|---|---|
| 9.1 | Gustaf VII "Gus" Vector Tavus Vance — protagonista | `characters/gus.md` + `deep/characters/gus-dragon.md` + memória `project_dragon_victory_canon` | ~9k | ~30 | consolidar; reforçar canon "Dragon" + Pyotor I Draco Vance |
| 9.2 | Cauã "Volt" Berenger — Striker (Pythia, Dutos Infernais) | `characters/caua-volt.md` + `deep/characters/caua-volt.md` | ~7k | ~23 | consolidar |
| 9.3 | Iara "Lumen" Koslov — Infiltradora (Óxido, Setor Mirage, ex-Cult) | `characters/iara-lumen.md` + `deep/characters/iara-lumen.md` | ~7k | ~23 | consolidar |
| 9.4 | Bento "Requiem" Chevalier — Tanque (Asmódico, exceção Pillar 2) | `characters/bento-requiem.md` + `deep/characters/bento-requiem.md` | ~7k | ~23 | consolidar |
| 9.5 | Linda "Siren" Neumann — Crowd Control (Óxido, Zona do Silêncio) | `characters/linda-siren.md` + `deep/characters/linda-siren.md` | ~7k | ~23 | consolidar |
| 9.6 | Dante "Grid" Alencar — o traidor (Asmódico→C-Arcane, Periferia) | `characters/dante-grid.md` + `deep/characters/dante-grid.md` + memória chantagem Edilma | ~9k | ~30 | tratamento double-layer; cuidado pra não vazar reveal antes do leitor lê arco principal |
| 9.7 | Jaci "Proxy" Vanderbist — Healer biológica (Pythia, Selve Sombria) | `characters/jaci-proxy.md` + `deep/characters/jaci-proxy.md` | ~7k | ~23 | consolidar |
| 9.8 | Dinâmicas da party (matriz linguagens + matriz afetos) | `characters/party.md` | ~3k | ~10 | extrair tabelas + comentário editorial |

**Subtotal Cap 9:** ~56k pal / ~185 pp

---

## Capítulo 10 — Antagonistas

| # | Sub-capítulo | Fonte | Palavras | Páginas | Nota revisão |
|---|---|---|---|---|---|
| 10.1 | Sterling Locke — perfil integral | `characters/sterling-locke.md` + `deep/antagonists/sterling-locke-deep.md` | ~10k | ~33 | consolidar (235 + 61 linhas; doc canônico largo) |
| 10.2 | Pre-lore vilão (formação acadêmica, rejeição, migração) | `characters/prelore_vilao.md` | ~2k | ~7 | extrair íntegra |
| 10.3 | Patch-Zero — antagonista-sistema (anti-padrão + 4 canais) | `characters/patch-zero.md` + `deep/antagonists/patch-zero-deep.md` | ~8k | ~27 | consolidar |
| 10.4 | NPCs antagonistas menores (FIR-vassalos, executivos Sterling Corp) | `deep/antagonists/npcs-antologia.md` | ~4k | ~13 | extrair |

**Subtotal Cap 10:** ~24k pal / ~80 pp

---

## Capítulo 11 — Linhagens Canônicas (cross-eras)

> **Função:** retratar 8 linhagens que atravessam as 3 eras, dando densidade familiar ao mundo. Conector entre Era 1 §6 (Famílias-Pilastra) e Era 3 presente.

| # | Sub-capítulo | Fonte | Palavras | Páginas | Nota revisão |
|---|---|---|---|---|---|
| 11.1 | Vance (Gustaf I → VII, 7 gerações) | `era-1-pre-codigo.md` §6 + `deep/antologia/10-pyotor-vance-pai.md` + `11-gargi-vance-mae.md` + memória `project_familia_vance_canonica` | ~3k | ~10 | conector cross-eras com tabela genealógica |
| 11.2 | Chevalier (Atelaiá → Bento) | `era-1-pre-codigo.md` §6 + `deep/antologia/13-veronica-atelaia.md` | ~1.5k | ~5 | conector |
| 11.3 | Vanderbist (Mariana → Jaci) | `deep/antologia/12-ancia-mariana-vanderbist.md` + `deep/characters/jaci-proxy.md` | ~1.5k | ~5 | conector |
| 11.4 | Berenger (Inácia → Cauã, Davi morto) | `deep/characters/caua-volt.md` §família | ~1k | ~3 | conector |
| 11.5 | Neumann (Linda — origem Zona do Silêncio) | `deep/characters/linda-siren.md` §família | ~1k | ~3 | conector |
| 11.6 | Argéndia, Boroshova, Ferraz (linhagens menores Era 1 residuais) | `CHARS.md` + `era-1-pre-codigo.md` §6 | ~2k | ~7 | conector enciclopédico |

**Subtotal Cap 11:** ~10k pal / ~33 pp

**Subtotal Parte V:** ~90k pal / ~298 pp

---

# PARTE VI — SETTINGS

**Função:** geografia narrativa. Cada setting recebe ensaio denso + extratos in-character.

**Estimativa parte:** ~58k pal / ~195 pp

---

## Capítulo 12 — GusWorld City e os Oito Settings do Bloco F

| # | Sub-capítulo | Fonte | Palavras | Páginas | Nota revisão |
|---|---|---|---|---|---|
| 12.1 | GusWorld City — núcleo metropolitano ciber-gótico | `environments/01-cidade-cyber-gotica.md` + `deep/settings/01-cidade.md` | ~7k | ~23 | consolidar (canon Bloco F + deep) |
| 12.2 | Selve Sombria — floresta-fronteira matemática | `environments/02-selve-sombria.md` + `deep/settings/02-selve-sombria.md` | ~7k | ~23 | consolidar |
| 12.3 | Catedrais Neo-Sylvania — gótico arcaico funcional | `environments/03-catedrais-neo-sylvania.md` + `deep/settings/03-catedrais.md` | ~7k | ~23 | consolidar; cross-ref Era 1 §3 |
| 12.4 | Dutos Infernais — subterrâneo industrial-arcano | `environments/04-dutos-infernais.md` + `deep/settings/04-dutos.md` | ~7k | ~23 | consolidar |
| 12.5 | Setor Mirage — distrito do ofuscamento | `environments/05-setor-mirage.md` + `deep/settings/05-mirage.md` | ~7k | ~23 | consolidar |
| 12.6 | Periferia — bairro oeste sucateiro | `environments/06-periferia.md` + `deep/settings/06-periferia.md` | ~7k | ~23 | consolidar; conector Dante "Grid" |
| 12.7 | Zona do Silêncio — distrito acusticamente morto | `environments/07-zona-do-silencio.md` + `deep/settings/07-silencio.md` | ~5k | ~17 | consolidar |
| 12.8 | Selve Profunda — climax ato 3 | `environments/08-selve-profunda.md` + `deep/settings/08-selve-profunda.md` | ~7k | ~23 | consolidar; cuidado com spoiler controlado |

**Subtotal Cap 12:** ~54k pal / ~178 pp

---

## Capítulo 13 — Cidades-Irmãs e Vilarejos-Fronteira

| # | Sub-capítulo | Fonte | Palavras | Páginas | Nota revisão |
|---|---|---|---|---|---|
| 13.1 | As três cidades-irmãs (referenciadas em `lore-bible.md` §10) | `lore-bible.md` §10 + `PLACES.md` | ~2k | ~7 | escrever cap conector (pouco canon, expansão controlada) |
| 13.2 | Vilarejos-fronteira (Pelicano Branco, outros) | `era-1-pre-codigo.md` §6 + `deep/factions/pelicano-branco.md` + `PLACES.md` | ~2k | ~7 | consolidar |

**Subtotal Cap 13:** ~4k pal / ~14 pp

**Subtotal Parte VI:** ~58k pal / ~192 pp

---

# PARTE VII — FACÇÕES

**Função:** sete facções canon do presente, mais menores. Cada uma com goal, recurso, aliados, inimigos, crença.

**Estimativa parte:** ~44k pal / ~145 pp

---

## Capítulo 14 — As Sete Facções Canon do Presente

| # | Sub-capítulo | Fonte | Palavras | Páginas | Nota revisão |
|---|---|---|---|---|---|
| 14.1 | Sterling Corp — o predador corporativo | `factions.md` §Sterling + `deep/factions/sterling-corp.md` | ~6k | ~20 | consolidar |
| 14.2 | FIR (Federação Industrial de Reciclagem) — vassala mafiosa | `factions.md` §FIR + `deep/factions/fir.md` | ~7k | ~23 | consolidar |
| 14.3 | Ordem Recursiva — guardiãs das catedrais Neo-Sylvania | `factions.md` §Ordem + `deep/factions/ordem-recursiva.md` | ~6k | ~20 | consolidar; cross-ref Era 1 |
| 14.4 | Cult Mirage Reality — manipulação holográfica | `factions.md` §Cult + `deep/factions/cult-mirage.md` | ~7k | ~23 | consolidar |
| 14.5 | Underground do Silêncio — resistência acústica | `factions.md` §Underground + `deep/factions/underground-silencio.md` | ~6k | ~20 | consolidar |
| 14.6 | Pelicano Branco — vilarejo-fronteira (Anciã Mariana Vanderbist) | `factions.md` §Pelicano + `deep/factions/pelicano-branco.md` | ~6k | ~20 | consolidar |
| 14.7 | Facções menores | `factions.md` §menores + `deep/factions/facoes-menores.md` | ~3k | ~10 | consolidar |
| 14.8 | Diagrama de alinhamentos cross-facção (axiologia canon) | **novo, derivado** de memória `project_axiologia_canonica` | ~3k | ~10 | escrever síntese editorial |

**Subtotal Cap 14:** ~44k pal / ~146 pp

**Subtotal Parte VII:** ~44k pal / ~146 pp

---

# PARTE VIII — LEITMOTIVS E ESTÉTICA

**Função:** fechar a obra com a camada artística que costura tudo. Tema musical, símbolo visual, ecos narrativos.

**Estimativa parte:** ~14k pal / ~45 pp

---

## Capítulo 15 — Leitmotivs Musicais e Temas Recorrentes

| # | Sub-capítulo | Fonte | Palavras | Páginas | Nota revisão |
|---|---|---|---|---|---|
| 15.1 | Leitmotivs por personagem/facção | `deep/ontologia/leitmotivs-deep.md` | ~4k | ~13 | extrair |
| 15.2 | Catálogo musical detalhado (compassos Fibonacci 5/8, 13/16; intervalos) | `deep/ontologia/leitmotivs-musicais-detalhados.md` | ~7k | ~23 | extrair |
| 15.3 | Temas narrativos recorrentes (substrato, custo, conservação, traição-perdão) | `deep/ontologia/leitmotivs-deep.md` §temas | ~3k | ~10 | extrair |

**Subtotal Cap 15:** ~14k pal / ~46 pp

**Subtotal Parte VIII:** ~14k pal / ~46 pp

---

# PARTE IX — STINGER E LEGADO

**Função:** ganchos pra futuro (Volume 2 / sequels). Curto, intencional, pós-credits literário.

**Estimativa parte:** ~9k pal / ~30 pp

---

## Capítulo 16 — Sequel Hooks e Pós-Créditos Narrativos

| # | Sub-capítulo | Fonte | Palavras | Páginas | Nota revisão |
|---|---|---|---|---|---|
| 16.1 | Sequel hooks declarados | `deep/stinger/sequel-hooks-deep.md` | ~3k | ~10 | extrair |
| 16.2 | Pós-créditos narrativos (cenas extras pós-ending) | `deep/stinger/post-credits-deep.md` | ~5k | ~17 | extrair |
| 16.3 | Eco final (1-2 páginas autorais de fechamento) | **novo, redacional** | ~0.5k | ~2 | encomendar a `narrative-writer` |

**Subtotal Cap 16:** ~8.5k pal / ~29 pp

**Subtotal Parte IX:** ~8.5k pal / ~29 pp

---

# APÊNDICES

**Função:** material de referência denso. Não-linear. Leitor consulta, não lê em sequência.

**Estimativa apêndices:** ~85k pal / ~285 pp

---

## Apêndice A — Glossário Canon

| Componente | Fonte | Palavras | Páginas | Nota |
|---|---|---|---|---|
| Termos técnicos (Glyph, Token, Conjuro, Codex, GTCC, DRE, GRE, Tavus-Drive, Matriz Ortodôntica, etc.) | `lore-bible.md` + `deep/magic/*` + `CLAUDE.md` §terminologia + memória `project_terminologia` | ~10k | ~33 | compilar ~250-350 entradas; ordem alfabética |
| Termos in-world (cripto-glifo, ashlar bruto/polido, acaceiro, Vyrcátrix, Helíaco Vyr, etc.) | `era-1-pre-codigo.md` + `deep/factions/*` + `deep/settings/*` | ~5k | ~17 | continuar glossário |

**Subtotal Apêndice A:** ~15k pal / ~50 pp

---

## Apêndice B — Índice de Personagens (CHARS.md expandido)

| Componente | Fonte | Palavras | Páginas | Nota |
|---|---|---|---|---|
| Tabela ~150 personagens nomeados | `CHARS.md` íntegra | ~12k | ~40 | extrair tabela; expandir cross-ref pra capítulos da obra |
| Personagens da antologia (cronistas in-character) | `CHARS.md` §cronistas + `deep/antologia/*` autores | ~3k | ~10 | sub-tabela específica |

**Subtotal Apêndice B:** ~15k pal / ~50 pp

---

## Apêndice C — Índice de Lugares (PLACES.md expandido)

| Componente | Fonte | Palavras | Páginas | Nota |
|---|---|---|---|---|
| Tabela ~90 lugares nomeados | `PLACES.md` íntegra | ~8k | ~27 | extrair; mapa-mãe esquemático opcional |
| Sub-locais por setting (interiores de catedrais, becos da Periferia, etc.) | `PLACES.md` §sub-locais + `environments/*` | ~4k | ~13 | continuar |

**Subtotal Apêndice C:** ~12k pal / ~40 pp

---

## Apêndice D — Timeline Canon (3 eras + transições)

| Componente | Fonte | Palavras | Páginas | Nota |
|---|---|---|---|---|
| Cronologia macro 3 eras + transições | `timeline.md` íntegra + `era-1-pre-codigo.md` §2 (8 horizontes) | ~5k | ~17 | consolidar |
| Eventos catalisadores datados | `timeline.md` + cross-ref `lore-bible.md` §3 | ~3k | ~10 | tabela cruzada |

**Subtotal Apêndice D:** ~8k pal / ~27 pp

---

## Apêndice E — Documentos In-Character Descobríveis (DD-001 a DD-020+)

| Componente | Fonte | Palavras | Páginas | Nota |
|---|---|---|---|---|
| 15+ documentos in-character (cartas, fragmentos, transcrições, atas) | `in-world-docs.md` íntegra | ~17k | ~57 | extrair íntegra; ordem por ID; nota de Knowledge Gate (Bronze/Prata/Ouro) |
| Diário do Gus (entradas selecionadas) | `diary/entries-docs-descobriveis.md` + `entries-mapas-timeline.md` + `entries-manuscrito-glossario.md` + `entries-fichas-bestiary.md` | ~5k | ~17 | seleção curada (não-exhaustiva; pleno está in-game) |

**Subtotal Apêndice E:** ~22k pal / ~74 pp

---

## Apêndice F — Inconsistências Documentadas

| Componente | Fonte | Palavras | Páginas | Nota |
|---|---|---|---|---|
| 20+ conflitos canon detectados em audit cross-doc | `INCOHERENCES.md` íntegra | ~2k | ~7 | extrair íntegra; tom de aparátus crítico (transparência editorial) |
| Resoluções aplicadas (F1-DL.REFAC Plano 3 — 12 decisões criador supremo) | commit `2ed93b6` (canon C1-C12) + sessão decisões | ~3k | ~10 | tabela das 12 decisões + diff |

**Subtotal Apêndice F:** ~5k pal / ~17 pp

---

## Apêndice G — Citação-Pedra-de-Toque + Aparátus Crítico

> **Função:** transparência autoral. Declara as bibliografias de inspiração (RAG queries top-k), o estilo Stephenson/Sterling, as licenças, e a metodologia editorial.

| Componente | Fonte | Palavras | Páginas | Nota |
|---|---|---|---|---|
| Citação âncora (passagem-pedra-de-toque) | **novo, autoral** | ~0.5k | ~2 | escolher 1 passagem do livro (sugestão: Era 1 §1 frontispício, primeiro parágrafo) como abertura simbólica do apêndice |
| Metodologia editorial (estilo Stephenson/Sterling, em-dash, terminologia) | `deep/_INDEX.md` §convenções | ~2k | ~7 | extrair |
| RAG queries declaradas (266 livros indexados) | `deep/_INDEX.md` §conformidade RAG + rodapés técnicos `deep/*` | ~3k | ~10 | compilar trace de inspiração (sem quebra de imersão) |
| Influências autorais declaradas (Tolkien, Asimov, Rand, Sun Tzu, Stephenson, Sterling) | `CLAUDE.md` + memórias | ~2k | ~7 | ensaio breve |
| Créditos (squad criativo, agentes IA assistidos, criador supremo) | **novo, editorial** | ~0.5k | ~2 | colofão editorial |

**Subtotal Apêndice G:** ~8k pal / ~28 pp

---

**Subtotal Apêndices A-G:** ~85k pal / ~286 pp

---

# Estatísticas finais consolidadas

| Bloco | Palavras | Páginas | % do volume |
|---|---|---|---|
| Parte I — Frontispício | ~28k | ~94 | 5.0% |
| Parte II — Cosmologia | ~44k | ~147 | 7.8% |
| Parte III — As Três Eras | ~269k | ~896 | 47.6% |
| Parte IV — Magia e Linguagem | ~36k | ~121 | 6.4% |
| Parte V — Personagens | ~90k | ~298 | 16.0% |
| Parte VI — Settings | ~58k | ~192 | 10.3% |
| Parte VII — Facções | ~44k | ~146 | 7.8% |
| Parte VIII — Leitmotivs e Estética | ~14k | ~46 | 2.5% |
| Parte IX — Stinger e Legado | ~9k | ~29 | 1.6% |
| Apêndices A-G | ~85k | ~286 | (referência) |
| **TOTAL VOLUME 1** | **~677k pal** | **~2.255 pp** | 100% |

> **Observação calibração:** estimativa subiu de ~565k (sumário inicial) pra ~677k após detalhamento sub-capítulo. Diferença é margem de consolidação editorial (frase-de-ligação, prefácios curtos por parte/capítulo, transições). Reservar 10-15% de buffer editorial é prática padrão.
>
> **Recomendação editorial reforçada:** dois tomos físicos ou formato bíblia (papel fino 65g/m², ~1.500-1.700 pp em 1 volume com tipografia condensada). Em formato A4 corpo 11/12 padrão, dois tomos é o caminho cômodo.

---

# Pendências antes da consolidação editorial final

Itens que **não existem ainda** e precisam ser produzidos antes do livro fechar:

1. **Cap 1.4 — manual do leitor** (~2k pal) → `narrative-writer`
2. **Cap 2.3 — manifesto Pillar 2 expandido** (~5k pal extrato editorial) → `narrative-writer`
3. **Cap 2.4 — apresentação easter eggs canon** (~4k pal, tom discreto) → `narrative-writer` + validação `narrative-designer`
4. **Cap 4.5 — bestiary computacional consolidado** (~9k pal) → `narrative-writer` consolidando fichas
5. **Cap 6.4 — fundação GusWorld City (Gustaf I→VII)** (~4k pal conector) → `narrative-writer`
6. **Cap 11 — linhagens canônicas cross-eras** (~10k pal, conectores) → `narrative-writer`
7. **Cap 13 — cidades-irmãs e vilarejos-fronteira** (~4k pal) → `narrative-writer`
8. **Cap 14.8 — diagrama axiologia cross-facção** (~3k pal síntese) → `narrative-designer`
9. **Cap 16.3 — eco final autoral** (~0.5k pal) → `narrative-writer`
10. **Apêndice A — glossário canon completo** (~15k pal) → `narrative-designer` consolida; `narrative-writer` pule verbetes
11. **Apêndice F.2 — 12 decisões REFAC tabuladas** (~3k pal) → editorial direto
12. **Apêndice G — colofão, créditos, RAG trace** (~8k pal) → editorial + `narrative-writer`

**Total a produzir antes do livro fechar:** ~67.5k pal (~225 pp) — ~10% do volume.

---

# Cross-refs canon (78 arquivos fonte consolidados)

```
docs/narrative/ (canon Bloco F/G/H/I)
├── sinopse.md                                            [Cap 1.1]
├── lore-bible.md                                         [Cap 1.2, 1.3, 6, 7]
├── arco-principal.md                                     [referência; não usado direto na bíblia]
├── factions.md                                           [Cap 14]
├── foreshadowing.md                                      [referência; não usado direto]
├── in-world-docs.md                                      [Apêndice E]
├── timeline.md                                           [Apêndice D]
├── tradicoes-cultura.md                                  [Cap 6, dispersa]
├── comic-reliefs.md                                      [Cap 6.3, 7.6 referência]
├── INCOHERENCES.md                                       [Apêndice F]
├── characters/                                           [Cap 9, 10]
│   ├── gus.md, party.md, caua-volt.md, iara-lumen.md
│   ├── bento-requiem.md, linda-siren.md, dante-grid.md
│   ├── jaci-proxy.md, sterling-locke.md, patch-zero.md
│   └── prelore_vilao.md                                  [Cap 10.2]
├── environments/                                         [Cap 12]
│   └── 01-cidade.md ... 08-selve-profunda.md (8 settings)
├── diary/                                                [Apêndice E.2]
│   └── _INDEX, ui-spec, knowledge-gates, foreshadow-links,
│       entries-* (4 docs)
└── deep/                                                 [Partes II-IX corpo]
    ├── _INDEX.md
    ├── eras/era-1-pre-codigo.md                          [Cap 5 íntegro]
    ├── factions/ (7 docs)                                [Cap 14]
    ├── settings/ (8 docs)                                [Cap 12]
    ├── characters/ (7 docs)                              [Cap 9]
    ├── magic/ (3 docs)                                   [Parte IV]
    ├── ontologia/ (4 docs)                               [Parte VIII + Cap 3]
    ├── antagonists/ (3 docs)                             [Cap 10]
    ├── stinger/ (2 docs)                                 [Cap 16]
    └── antologia/ (14 contos)                            [Volume 2 separado; extratos em Cap 5.8, 11]

docs/design/
└── pillars.md                                            [Cap 2.1, 2.2]

raiz/
├── CHARS.md                                              [Apêndice B]
├── PLACES.md                                             [Apêndice C]
├── CLAUDE.md                                             [Cap 2.4 easter eggs + Apêndice G]
└── memórias em ~/.claude/.../memory/                     [Cap 9.1 Dragon, 11.1 Vance, 14.8 axiologia]
```

---

# Ordem sugerida de consolidação (workflow)

> **Premissa:** este TOC é o blueprint. Consolidação editorial real do livro acontece em fase pós-release do jogo. Ordem técnica recomendada:

1. **Fase A (pré-release jogo):** congelar canon em `docs/narrative/*` + `deep/*`. Resolver pendências REFAC restantes.
2. **Fase B (release jogo v1.0.0):** publicar jogo. Bíblia segue em standby.
3. **Fase C (pós-release T+3 a T+6 meses):** produzir 12 pendências da seção anterior (~67.5k pal a escrever). Workflow `narrative-designer` (audit) + `narrative-writer` (produção) + criador supremo (aprovação).
4. **Fase D (pós-release T+6 a T+9):** revisão editorial integral (consolidação, transições, prefácios por parte, escolha de citação-pedra-de-toque, layout final).
5. **Fase E (pós-release T+9 a T+12):** diagramação, ilustrações canon (encomenda separada), preparação print-on-demand.
6. **Fase F (T+12+):** publicação Volume 1. Volume 2 (antologia) segue cronograma próprio.

---

**Última revisão:** 2026-05-21. Estrutura proposta pelo `narrative-designer` em modo colaborativo (Auto Mode ativo). Aguarda aprovação do criador supremo para canonização como blueprint editorial.
