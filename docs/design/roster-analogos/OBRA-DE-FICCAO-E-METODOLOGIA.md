# Fiction Disclaimer and Methodology: The Historical Figures Roster

> This document is bilingual. English first, then Portuguese (pt-br) below the divider, mirroring the project's README and ACKNOWLEDGMENTS convention.

---

## FICTION DISCLAIMER (read this first)

**GusWorld is a work of fiction.** The "roster of analogues" described in this folder (`docs/design/roster-analogos/`), a set of playable spell cards and in-world "masters" inspired by real historical figures in science, mathematics, economics and engineering, is an **artistic, transformative reinterpretation created for entertainment purposes**, not a work of history, biography, journalism or scholarship.

Every character built from this roster is a **fictional construct**. Their personalities, dialogue, in-world political alignment, moral framing ("good" or "bad" within the game's own value system) and any other trait assigned to them in GusWorld are **creative license taken for game design purposes**. None of it is, or is intended to be read as, a **factual, biographical or historical claim** about the real person: not about what they actually believed, how they actually behaved, what their actual character or morality was, or how a rigorous historian, biographer or psychologist would assess them.

All figures whose real names are used in this roster are **deceased**, and are used here as **inspiration and homage** to their real, documented, historical contributions (see [Sources and attribution](#sources-and-attribution) below). One additional figure in the roster, "Helion Tusk", is a wholly fictional character whose archetype draws inspiration from a **living** contemporary public figure; that figure's real name and likeness are **deliberately not used** in the game itself (see [A note on the one living inspiration](#a-note-on-the-one-living-inspiration) below).

No defamation, disparagement or misrepresentation of any real person, living or deceased, is intended by this project. If you are, or represent, a person referenced in this roster (or their estate) and have a concern about how a specific character is portrayed, please open an issue on the project's [Codeberg tracker](https://codeberg.org/petrinhu/gusworld/issues), we take such concerns seriously and will review them promptly.

This notice is a plain-language disclosure of intent and process. It is not, and should not be read as, formal legal advice for any specific jurisdiction.

---

## What this is

GusWorld includes a roster of 21 in-world "masters", each one a playable spell card ("Codex" card) inspired by a real historical figure whose documented, real-world contribution to physics, mathematics, computer science, economics or the history of ideas maps onto an in-game ability. The figures span electromagnetism (Faraday, Maxwell, Tesla, Volta), physics (Einstein, Newton, Planck), mathematics (Mandelbrot, Euler, Gödel, Ada Lovelace), computing (Turing, von Neumann), the history of esoteric/hermetic thought (Giordano Bruno, John Dee, Pythagoras), the Austrian school of economics (Hayek, Mises, Menger, Bastiat), and one capstone figure, "Helion Tusk", inspired by the archetype of a contemporary serial entrepreneur-engineer.

Each figure gets an individual design document in this folder (`01-faraday.md` through `21-helion-tusk.md`), containing: a short summary of their real, documented contributions (with cited sources), how that contribution is translated into a fictional in-game identity, card mechanics inspired by (not literal to) the real phenomenon or idea, a discovery quest, and, where used, image references for concept art.

## This is fiction, not biography

To be unambiguous about what the roster is **not**:

- It is **not** a claim that the real historical figure held the political views, personality traits, or moral character assigned to their in-game counterpart.
- It is **not** a psychological or psychiatric assessment of the real person. Any personality shorthand used internally during design (see [Methodology](#methodology-how-the-roster-is-built) below) is a **writing aid for a fictional character**, not a diagnosis or clinical statement about a real individual, living or deceased.
- It is **not** an academic classification of the real person's politics. The "political spectrum" tool used in design (see below) scores a fictional persona's answers to a quiz, for game-design purposes; it does not, and cannot, measure what the real person actually believed.
- It is **not** an endorsement, by the real person or their estate, of GusWorld, its narrative, or its themes.
- The in-game "good" / "bad" framing applied to some analogues reflects **GusWorld's own internal, fictional value system** (documented separately as the project's creative canon), used to decide what kind of in-game challenge unlocks a character's card. It is a **game design category**, not a moral verdict on the real person's life or work.

## Methodology (how the roster is built)

In the interest of transparency, here is the internal design pipeline used to build each roster entry. It is documented so the process is auditable and reproducible, not just asserted:

1. **Biographical research.** Each figure's real, documented contributions are researched from public sources (primarily Wikipedia and other openly available reference material; specific sources are cited at the bottom of each figure's design document, e.g. `01-faraday.md`).
2. **Fictional personality sketch.** A personality profile for the in-game character is drafted as a **writing aid**, informed by general, publicly available psychology vocabulary (broad personality-trait language in the style of the Big Five / OCEAN model, and, where useful as descriptive shorthand, everyday language adjacent to DSM-5 category names). This step is explicitly a **creative writing tool**, used the way a novelist sketches a character's temperament before writing dialogue for them; it is not a clinical evaluation, is not performed by a licensed professional, and is not intended to represent the real person's actual mental state, diagnosis, or psychological profile.
3. **Political spectrum estimate (game mechanic).** A fictional "persona" built from step 1 and 2 answers a 70-question political-values quiz (a private, offline fork of the open-source project **8values**, see [Sources and attribution](#sources-and-attribution)). An AI language model role-plays the researched persona to answer the quiz, and the fork's scoring engine converts the answers into axis percentages and a single 0-1 scalar. This is a **game mechanic**, not an academic or journalistic classification of the real person's actual politics; it exists purely to give the design team a consistent, reproducible input for step 4.
4. **In-fiction alignment and challenge type.** The scalar from step 3 feeds into GusWorld's own internal creative canon (its fictional value system, documented separately), which in turn informs the type of in-game side quest, puzzle or challenge the player must complete to earn that character's card. This is entirely an internal **game design and balancing decision**, made by the project's creator, not a real-world political statement.

**Scope limit: the one living figure is excluded from steps 2 and 3.** The personality profiling (step 2) and the 8values quiz (step 3) are run **only on the deceased figures**. The single living-inspired character, "Helion Tusk", is deliberately **not profiled and not submitted to the quiz**. His in-fiction alignment is set by **pure authorial fiat** as a game-design choice, and is never derived from, and never represents, any assessment of the real living person (see [A note on the one living inspiration](#a-note-on-the-one-living-inspiration)).

Every step above is documented per-figure in this folder, and can be inspected, questioned, or revised by the project's creator at any time (nothing in the roster is canon until reviewed and approved; see the `STATUS` header at the top of each individual figure's document).

## Method calibration

This section documents the calibration and quality controls behind step 3 (the political-spectrum game mechanic), so the process is auditable, not just asserted. Everything below produces a **game design category, not a factual classification** of any real person (see the [Fiction disclaimer](#fiction-disclaimer-read-this-first)).

**The `rightness` scalar.** The primary axis is the **Economic** axis of the 8values fork (Equality on one end, Markets on the other). The scalar is `rightness = economic_markets_pct / 100`, ranging from `0.0` (far left) to `1.0` (far right). The **Social** axis (Progress vs Tradition) is recorded separately as a **manual tiebreaker only** and does not enter the `rightness` number. This mirrors GusWorld's own internal value system, where the economic axis is the primary one for the fictional good/bad framing.

**The persona method.** For each deceased figure: (1) biographical research, then (2) a Big Five / OCEAN personality profile is drafted as a writing aid, then (3) an AI agent (Opus-class model, named `AGENTE_PERSONA_[FIGURE]`) role-plays the researched persona and answers the 70 questions of the 8values fork. The answer consistency (the effective "temperature" of the persona) is modulated by prompt according to the drafted personality, so a dogmatic figure answers more decisively than a hedging one. The fork's scoring engine then converts the 70 answers into the four axis percentages, the closest reference ideology label, and the `rightness` scalar.

**Calibration test (run before classifying any of our figures).** The full pipeline is first run on a known-answer control set: **two known right-leaning figures** (Mises, Hayek), **two known left-leaning external figures used for testing only** (Marx, Mao Zedong, not part of the game roster), and **Einstein** (a known left-leaning figure who IS one of our roster figures, used as an extra validation point). Define `leftness = 1 - rightness`. The convergence score is `min(rightness of the right-set, leftness of the left-set)`. If convergence is **greater than 0.599** (that is, every right-leaning control scores rightness above 0.599 AND every left-leaning control scores leftness above 0.599), the method is **validated** and classification may proceed. If not, the agent construction (prompt, personality modulation) is adjusted and the calibration is re-run **before any of our figures are classified**.

**Per-figure triple check.** No figure's alignment is fixed on the 8values number alone. For each figure the orchestrator performs: (1) the 8values result from the persona method; (2) an independent web search of the figure's **real, documented politics**; (3) the orchestrator confronts (1) against (2), reasons about any discrepancy, and submits the reasoned conclusion to the project's creator, who gives final approval. The creator's approval, not the raw number, is what makes a classification canon.

**Classification thresholds (our figures only).** Applied to the `rightness` scalar of each roster figure:
- `rightness` **greater than 0.599** to **RIGHT** (in-fiction framing: "good" / good evolution).
- `rightness` **below 0.401** to **LEFT** (in-fiction framing: "bad" / bad evolution).
- `rightness` **between 0.401 and 0.599** to **CENTER** (in-fiction framing: "neutral" / puzzle challenge). A center result is not accepted on the first pass: it triggers **re-running the algorithm two more times with expanded web research** (including marginal, fringe, or conspiracy-theory sources, precisely to stress-test whether the center reading survives) before "center" is fixed.

**Calibration result (run 2026-07-11).** The control set produced: Mises `rightness 0.846`, Hayek `rightness 0.686` (right set); Marx `leftness 0.929`, Mao `leftness 0.878`, Einstein `leftness 0.904` (left set). Convergence `= min(0.686, 0.878) = 0.686`, which is **greater than 0.599 → the method is VALIDATED** and classification of the roster figures may proceed. Two fine checks the method passed: it separated Mao's civil **authoritarianism** (Authority 70.3%, Statist) from Marx's civil **libertarianism** (Liberty 77.3%) despite both being economically far-left, and it landed Einstein on "Libertarian Socialism", capturing his real tension (economic left + civil libertarian). Full per-figure records are in the [Appendix](#appendix-per-character-record--apêndice-registro-por-personagem).

### Axis weights v2 (GAME-ONLY scalar refactor, 2026-07-11)

**READ THIS FIRST. THE WEIGHTS BELOW EXIST FOR THE GAME, SOLELY AND EXCLUSIVELY.** They are **NOT a modification of the 8values test**: the fork in `8values-engine/` keeps the original questions and the original scoring untouched, and the weighted scalar lives in a **separate** script (`gus_rightness.py`) layered on top, used only by GusWorld's internal design pipeline. They are **NOT** a statement about real-world politics, **NOT** an improved "measurement" of anything, **NOT** a claim that some human values outrank others in reality, and they **MUST NOT be carried into the real world** or used to evaluate real people, real tests, or real ideologies. They encode exactly one thing: how much each 8values axis matters *inside the fictional moral economy of GusWorld*, so that one game mechanic (which sidequest archetype a fictional character card gets) is reproducible. For everything else, the [Fiction disclaimer](#fiction-disclaimer-read-this-first) governs.

**The pain that forced this refactor (design history).** The v1 scalar was the economic axis alone (`rightness = Markets%/100`). It calibrated fine, but as soon as real roster figures went through it, three recurring pains appeared:

1. **Mixed profiles were flattened.** Volta scored economically center (0.564) while being strongly authoritarian (Authority 74.2%) and traditional (Tradition 66.6%); Maxwell scored borderline right (0.609) while being a statist (Authority 68.0%). The v1 scalar threw away three of the four axes, precisely the ones the lore uses to code hero and villain (Liberty is hero-coded; Authority is villain-coded via Sterling, Patch-Zero and the DRE; Tradition is heritage-coded).
2. **Culturally heroic figures kept landing "left"** (Einstein, then Tesla), forcing ad-hoc safeguards (the Einstein zone, per-figure deferrals). Safeguards piling up is the classic smell of a scoring function that does not measure what the design values.
3. **Every ambiguous case dropped to a manual creator decision** with no consistent ruler between cases.

The creator then ranked the eight values by their importance in the lore so far, **Markets > Liberty > Authority > Equality > Tradition > Progress > Nation > Peace**, and axis weights were derived from that ranking (decision via creator approval, 2026-07-11).

**The v2 formula** (the right-coded pole of each axis, per the canon axiology):

```text
rightness_v2 = 0.50 * (Markets%   / 100)   # economic axis   (pole: Markets)
             + 0.25 * (Liberty%   / 100)   # civil axis      (pole: Liberty)
             + 0.15 * (Tradition% / 100)   # societal axis   (pole: Tradition)
             + 0.10 * (Nation%    / 100)   # diplomatic axis (pole: Nation)
```

Why economic dominates civil 2:1 even though Liberty ranked #2: the quiz's civil axis cannot distinguish propertarian liberty from anarcho-collectivist liberty (Marx scores Liberty 77.3%). If civil weighed equal to economic, every anti-state leftist would gain spurious rightness; at 0.25, Marx's civil bonus cannot rescue his economic hole and he stays clearly left. The economic axis is the only one that separates a free-marketeer from an anarchist, so it must dominate. Societal stays at 0.15 because the in-fiction anchors of "good" (Mises, Progress 57.0%; Hayek, 51.5%) are not traditionalists, and overweighting Tradition would punish the anchors themselves. Diplomatic gets 0.10 because it is the weakest signal (#7 and #8 in the ranking) and the most ambiguous pole mapping (Nation carries both sovereignty, good in-fiction, and militarism, villain-coded).

**Calibration re-validated under v2** (recomputed from the stored answer arrays; the personas were NOT re-run, answers are formula-independent raw data): Mises 0.693, Hayek 0.612 (right set); Marx leftness 0.730, Mao leftness 0.735, Einstein leftness 0.713 (left set). Convergence `min(0.612, 0.713) = 0.612 > 0.599`: the **method remains VALIDATED under v2**. Thresholds unchanged (>0.599 right, <0.401 left, center in between; center still triggers the 2x redo protocol). The compression toward center is intended: under v2, full "right" requires being right-leaning across the axes the lore actually values, not just economically.

**Implementation note.** The original fork (`8values_engine.py`) is untouched, out of respect for the upstream project and the planned author outreach. The v2 layer is `8values-engine/gus_rightness.py`; it reuses the fork's scoring verbatim and prints both scalars (`RIGHTNESS_V1`, `RIGHTNESS_V2`) plus the full descriptive block of every axis. Each per-character record in the appendix carries that verbatim descriptive block (both poles of all four axes, with labels), so complicated cases can be judged by eye and not by a single number.

### The v3 full re-run (persona model change to Fable + re-test of the whole roster, 2026-07-11)

After the v2 weight refactor, the creator ordered a **full re-run of every roster figure except Tusk** (who remains shielded by pure authorial fiat, never profiled, never quizzed): personas re-embodied from scratch, quizzes re-answered, engine re-scored, records re-closed. **Reasons, as decided by the creator:**

1. **Coherence of the final result.** The v2 refactor changed the scalar formula. Recomputing old answer arrays is valid (answers are formula-independent raw data), but the creator determined that every *final* classification in force must be the product of **one coherent end-to-end run** of the current pipeline (research, persona, quiz, weighted engine, record), not a mix of old runs re-scored under new math.
2. **Persona model change: Opus to Fable.** From this run on, the persona-agents (`AGENTE_PERSONA_*`) run on the **Fable** model (creator's decision, 2026-07-11; earlier runs used Opus). Changing the agent model is a change in agent construction, and the calibration clause above is explicit: any change to the agent construction requires **re-running the calibration before classifying any roster figure**. So the v3 order is: calibration first (Mises, Hayek, Einstein, plus the external controls Marx and Mao), convergence checked against the same 0.599 bar, and only then the roster.
3. **Research is reused.** The biographical profiles gathered per figure do not depend on the formula or on the model, so they carry over unchanged; only the persona runs and everything downstream are redone.
4. **Former fiat figures enter the algorithm.** Menger and Bastiat, previously classified by obvious-case authorial fiat, are included in the re-run ("all except Tusk"), so they get research, a persona and a computed record for the first time. Any surprising result becomes an ambiguity entry (AMB) and a creator consult.

Earlier runs (Opus personas, v1 and v2 scores) remain preserved in the git history and in the superseded notes of each record; the appendix records are updated to the run in force as each figure closes.

**v3 calibration result (Fable personas, v2 weights, 2026-07-11).** Right set: Mises `0.706`, Hayek `0.590 / 0.601 / 0.608` across three independent runs (median `0.601`). Left set: Marx leftness `0.703`, Mao `0.732`, Einstein `0.716`. Convergence `= min(0.706, 0.601, 0.703, 0.732, 0.716) = 0.601 > 0.599`: the method **remains validated under Fable personas**. Two findings: (a) the Opus-to-Fable change moved every anchor by at most about 0.02 (Hayek fell from 0.612 to about 0.600), so Fable is a sound persona substitute; (b) Hayek sits right on the threshold, which is faithful, not a defect: he is the most moderate member of the right set (accepts a safety net, gradualist, socially neutral), so his barely-clearing 0.599 is the correct result. The creator accepted the validation by the median (AMB-08).

**The "Einstein zone" (heroic-figure safeguard), re-anchored.** The zone is defined as **±10 points around Einstein's score under the formula in force**. Under v2 Einstein scores `rightness 0.287`, so the zone is `rightness` within **[0.187, 0.387]**. A roster figure of **culturally heroic / beloved reputation** whose score falls in that band is **not automatically cast as a villain**: its economic-spectrum classification is recorded, but the in-fiction framing (good/bad/neutral) and sidequest challenge are **deferred to the creator, decided case-by-case**. Note the elegance: Tesla (v2 0.354) now falls inside the zone, so his deferral, originally granted as a one-off creator call, becomes a rule application. Outside the band the normal axiology applies, though the same good-sense hero check can still trigger a creator consult.

To be explicit once more: these thresholds, this scalar, and this whole calibration exist to make an **internal game-design category** consistent and reproducible. None of it is, or claims to be, an accurate measurement of what any real person actually believed.

## Sources and attribution

- **Biographical research** for each figure draws on public sources, primarily Wikipedia and other openly licensed or public-domain reference material, cited individually at the bottom of each figure's document (e.g. `01-faraday.md`, `21-helion-tusk.md`).
- **The political-spectrum tool is an explicit, credited fork/port** of the open-source project **8values** (`https://github.com/8values/8values.github.io`, MIT License). Full attribution, including exactly what was copied verbatim from the original project (questions, axis weights, scoring formula, ideology reference table) versus what GusWorld added on top (the Python port itself, the `rightness` scalar, and supporting tooling) lives in [`8values-engine/ATTRIBUTION.md`](8values-engine/ATTRIBUTION.md) and [`8values-engine/README.md`](8values-engine/README.md), in this same folder. The original MIT copyright notice is preserved verbatim in `8values-engine/LICENSE_8VALUES_ORIGINAL.txt`.
- **Reference images** used for concept art and sprite generation are sourced individually per figure, with license noted in each figure's document. Public-domain images (Wikimedia Commons) are used where available; where a figure has no confirmed public-domain photograph (including the one living inspiration, see below), a Creative Commons Attribution-ShareAlike (CC BY-SA) licensed photograph is used instead, with attribution recorded in the game's credits page as required by that license.

## A note on the one living inspiration

Twenty of the twenty-one figures in this roster are deceased historical figures, used under their real names as documented public history. One figure, **"Helion Tusk"**, is different by design: the archetype (a serial entrepreneur-engineer who reinvests each venture's returns into the next, riskier one, spanning digital payments, reusable rockets, electric vehicles and artificial intelligence) is inspired by a **living** contemporary public figure.

Because that inspiration is a living person, GusWorld treats this entry with additional care, beyond what applies to the twenty deceased figures:

- **He is not profiled and not quizzed.** Unlike the twenty deceased figures, this character is deliberately **excluded from the personality-profiling (Big Five / DSM-5-adjacent) and the 8values quiz steps** of the design methodology (see [Methodology](#methodology-how-the-roster-is-built)). His in-fiction alignment is set by **pure authorial fiat**, never by running any tool on a model of a living person. GusWorld does not attempt to estimate, model, or represent the real living person's personality, mental state, or actual politics.
- The character is given a **fictional near-name** ("Helion Tusk"), not the real person's name, anywhere in the game itself. The real person's name is used only in the internal design document, as research grounding, exactly the way a published roman à clef or a satirical work might cite its real-world inspiration in an author's note, and never in player-facing content.
- The character's biography, personality, dialogue, and story arc in GusWorld are **entirely fictionalized** and diverge substantially from the real person's actual life; only the broad entrepreneurial archetype (documented, public business history: company names, dates, and outcomes that are a matter of public record) is used as a factual anchor.
- A reference photograph of the real person (Creative Commons Attribution-ShareAlike licensed, not public domain, since the person is living) is used **only as visual reference for a stylized, non-photorealistic pixel-art sprite**, not reproduced directly in the game; full photo credit is recorded in the game's credits page per the license's attribution requirement.
- As with the other twenty figures, nothing about this character's in-game personality, political framing, or "good/bad" alignment is a claim about the real person's actual views, character, or mental state. GusWorld takes no public position on any living person's real-world activities, statements, or controversies, and this character should not be read as commentary on any of that.

## A note of respect

Every figure in this roster is included because their real, documented work changed how humanity understands electricity, motion, computation, uncertainty, or the way free exchange coordinates strangers who never meet. GusWorld's goal in fictionalizing them is celebratory: to give players, most of them kids solving problems the same way an eleven-year-old protagonist does, in the game's own words, a playful reason to go look up who Faraday, Gödel or Hayek actually were and what they actually did. If a single player closes the game and opens a real biography or a real paper because a card made them curious, the roster has done its job.

## Questions or concerns

This is a solo, freeware, non-commercial passion project (see the main [README](../../../README.md)). If you are a descendant, estate representative, or anyone with a good-faith concern about how a specific figure is portrayed in this roster, please open an issue on [Codeberg](https://codeberg.org/petrinhu/gusworld/issues); concerns will be reviewed and, where warranted, the portrayal will be adjusted or removed.

**Issue policy.** Only **technical** issues are accepted (bugs, build problems, crashes, documentation errors) plus the good-faith portrayal concerns described above. **Issues of a political, economic, philosophical, social or similar nature will be closed or deleted without reply.** This game is a work of fiction; its design documents are not an invitation to real-world debate, and the repository's issue tracker will not host one.

---

## Aviso de Obra de Ficção e Metodologia: o Roster de Figuras Históricas

> Este documento é bilíngue. Inglês primeiro (acima), depois português (pt-br) aqui embaixo, seguindo a mesma convenção do README e do ACKNOWLEDGMENTS do projeto.

---

## AVISO DE FICÇÃO (leia isto primeiro)

**GusWorld é uma obra de ficção.** O "roster de análogos" descrito nesta pasta (`docs/design/roster-analogos/`), um conjunto de cartas de magia jogáveis e "mestres" in-world inspirados em figuras históricas reais da ciência, matemática, economia e engenharia, é uma **reinterpretação artística e transformadora criada para fins de entretenimento**, não uma obra de história, biografia, jornalismo ou pesquisa acadêmica.

Todo personagem construído a partir deste roster é uma **construção ficcional**. As personalidades, falas, alinhamento político in-world, enquadramento moral ("bom" ou "mau" dentro do sistema de valores próprio do jogo) e qualquer outro traço atribuído a eles em GusWorld são **licença artística tomada para fins de design de jogo**. Nada disso é, ou pretende ser lido como, uma **afirmação factual, biográfica ou histórica** sobre a pessoa real: não sobre o que ela realmente acreditava, como realmente se comportava, qual era seu caráter ou moral verdadeiros, ou como um historiador, biógrafo ou psicólogo rigoroso a avaliaria.

Todas as figuras cujo nome real é usado neste roster estão **falecidas**, e são usadas aqui como **inspiração e homenagem** às suas contribuições reais, documentadas e históricas (veja [Fontes e atribuição](#fontes-e-atribuição) abaixo). Uma figura adicional do roster, "Helion Tusk", é um personagem inteiramente fictício cujo arquétipo se inspira numa figura pública contemporânea **viva**; o nome real e a imagem dessa pessoa **deliberadamente não são usados** no jogo em si (veja [Uma nota sobre a única inspiração viva](#uma-nota-sobre-a-única-inspiração-viva) abaixo).

Nenhuma difamação, desmerecimento ou deturpação de qualquer pessoa real, viva ou falecida, é pretendida por este projeto. Se você é, ou representa, uma pessoa referenciada neste roster (ou seu espólio) e tem uma preocupação sobre como um personagem específico é retratado, por favor abra uma issue no [rastreador do Codeberg](https://codeberg.org/petrinhu/gusworld/issues) do projeto: levamos essas preocupações a sério e vamos revisá-las prontamente.

Este aviso é uma divulgação em linguagem simples de intenção e processo. Não é, e não deve ser lido como, aconselhamento jurídico formal para nenhuma jurisdição específica.

---

## O que é isto

GusWorld inclui um roster de 21 "mestres" in-world, cada um uma carta de magia jogável (carta "Codex") inspirada numa figura histórica real cuja contribuição documentada e real ao mundo à física, matemática, ciência da computação, economia ou história das ideias mapeia para uma habilidade in-game. As figuras cobrem eletromagnetismo (Faraday, Maxwell, Tesla, Volta), física (Einstein, Newton, Planck), matemática (Mandelbrot, Euler, Gödel, Ada Lovelace), computação (Turing, von Neumann), a história do pensamento esotérico/hermético (Giordano Bruno, John Dee, Pitágoras), a escola austríaca de economia (Hayek, Mises, Menger, Bastiat), e uma figura-capstone, "Helion Tusk", inspirada no arquétipo de um empreendedor-engenheiro serial contemporâneo.

Cada figura tem um documento de design individual nesta pasta (`01-faraday.md` a `21-helion-tusk.md`), contendo: um resumo curto das contribuições reais e documentadas da figura (com fontes citadas), como essa contribuição é traduzida numa identidade ficcional in-game, mecânicas de carta inspiradas no (não literais ao) fenômeno ou ideia real, uma missão de descoberta e, quando usadas, referências de imagem para arte conceitual.

## Isto é ficção, não biografia

Para ser inequívoco sobre o que o roster **não** é:

- **Não** é uma afirmação de que a figura histórica real tinha as visões políticas, traços de personalidade ou caráter moral atribuídos à sua contraparte in-game.
- **Não** é uma avaliação psicológica ou psiquiátrica da pessoa real. Qualquer atalho de personalidade usado internamente durante o design (veja [Metodologia](#metodologia-como-o-roster-é-construído) abaixo) é uma **ferramenta de escrita para um personagem ficcional**, não um diagnóstico ou declaração clínica sobre um indivíduo real, vivo ou falecido.
- **Não** é uma classificação acadêmica da política da pessoa real. A ferramenta de "espectro político" usada no design (veja abaixo) pontua as respostas de uma persona fictícia a um quiz, para fins de design de jogo; ela não mede, e não pode medir, o que a pessoa real de fato acreditava.
- **Não** é um endosso, pela pessoa real ou por seu espólio, de GusWorld, sua narrativa ou seus temas.
- O enquadramento "bom" / "mau" in-game aplicado a alguns análogos reflete o **sistema de valores próprio, interno e ficcional de GusWorld** (documentado à parte como o canon criativo do projeto), usado para decidir que tipo de desafio in-game destrava a carta de um personagem. É uma **categoria de design de jogo**, não um veredito moral sobre a vida ou a obra da pessoa real.

## Metodologia (como o roster é construído)

No interesse da transparência, aqui está o pipeline interno de design usado para construir cada entrada do roster. Ele é documentado para que o processo seja auditável e reprodutível, não apenas afirmado:

1. **Pesquisa biográfica.** As contribuições reais e documentadas de cada figura são pesquisadas a partir de fontes públicas (principalmente Wikipedia e outro material de referência abertamente disponível; fontes específicas são citadas ao final do documento de design de cada figura, ex.: `01-faraday.md`).
2. **Esboço de personalidade ficcional.** Um perfil de personalidade para o personagem in-game é rascunhado como uma **ferramenta de escrita**, informado por vocabulário de psicologia geral e publicamente disponível (linguagem ampla de traços de personalidade no estilo do modelo Big Five / OCEAN, e, quando útil como atalho descritivo, linguagem cotidiana próxima a nomes de categoria do DSM-5). Esta etapa é explicitamente uma **ferramenta de escrita criativa**, usada da mesma forma que um romancista esboça o temperamento de um personagem antes de escrever diálogos para ele; não é uma avaliação clínica, não é realizada por um profissional licenciado, e não pretende representar o estado mental, diagnóstico ou perfil psicológico real da pessoa verdadeira.
3. **Estimativa de espectro político (mecânica de jogo).** Uma "persona" fictícia construída a partir das etapas 1 e 2 responde a um quiz de 70 perguntas de valores políticos (um fork privado e offline do projeto open-source **8values**, veja [Fontes e atribuição](#fontes-e-atribuição)). Um modelo de linguagem de IA interpreta a persona pesquisada para responder o quiz, e o engine de pontuação do fork converte as respostas em percentuais de eixo e um escalar único de 0 a 1. Isto é uma **mecânica de jogo**, não uma classificação acadêmica ou jornalística da política real da pessoa; ela existe puramente para dar ao time de design um insumo consistente e reproduzível para a etapa 4.
4. **Alinhamento in-fiction e tipo de desafio.** O escalar da etapa 3 alimenta o canon criativo próprio e interno de GusWorld (seu sistema de valores ficcional, documentado à parte), que por sua vez informa o tipo de sidequest, puzzle ou desafio in-game que o jogador precisa completar para ganhar a carta daquele personagem. Isso é inteiramente uma **decisão de design e balanceamento de jogo**, tomada pelo criador do projeto, não uma declaração política do mundo real.

**Limite de escopo: a única figura viva é excluída das etapas 2 e 3.** O perfilamento de personalidade (etapa 2) e o quiz 8values (etapa 3) rodam **somente nas figuras falecidas**. O único personagem inspirado em pessoa viva, "Helion Tusk", é deliberadamente **não perfilado e não submetido ao quiz**. Seu alinhamento in-fiction é definido por **fiat autoral puro** como escolha de design de jogo, e nunca é derivado de, e nunca representa, qualquer avaliação da pessoa viva real (veja [Uma nota sobre a única inspiração viva](#uma-nota-sobre-a-única-inspiração-viva)).

Cada etapa acima é documentada por figura nesta pasta, e pode ser inspecionada, questionada ou revisada pelo criador do projeto a qualquer momento (nada no roster é canon até revisão e aprovação; veja o cabeçalho `STATUS` no topo do documento de cada figura individual).

## Calibração do método

Esta seção documenta a calibração e os controles de qualidade por trás da etapa 3 (a mecânica de jogo de espectro político), para que o processo seja auditável, não apenas afirmado. Tudo abaixo produz uma **categoria de design de jogo, não uma classificação factual** de nenhuma pessoa real (veja o [Aviso de Ficção](#aviso-de-ficção-leia-isto-primeiro)).

**O escalar `rightness`.** O eixo primário é o eixo **Econômico** do fork do 8values (Equality/Igualdade numa ponta, Markets/Mercado na outra). O escalar é `rightness = economic_markets_pct / 100`, variando de `0.0` (extrema-esquerda) a `1.0` (extrema-direita). O eixo **Social** (Progress/Progresso vs Tradition/Tradição) é registrado à parte como **desempate manual apenas** e não entra no número `rightness`. Isso espelha o sistema de valores próprio de GusWorld, onde o eixo econômico é o primário para o enquadramento ficcional bom/mau.

**O método da persona.** Para cada figura falecida: (1) pesquisa biográfica, depois (2) rascunha-se um perfil de personalidade Big Five / OCEAN como ferramenta de escrita, depois (3) um agente de IA (modelo classe Opus, nomeado `AGENTE_PERSONA_[FIGURA]`) encarna a persona pesquisada e responde as 70 perguntas do fork do 8values. A consistência das respostas (a "temperatura" efetiva da persona) é modulada por prompt conforme a personalidade rascunhada, de forma que uma figura dogmática responde de modo mais decidido que uma figura hesitante. O engine de pontuação do fork então converte as 70 respostas nos quatro percentuais de eixo, no rótulo de ideologia de referência mais próxima, e no escalar `rightness`.

**Teste de calibração (rodado antes de classificar qualquer figura nossa).** O pipeline completo é primeiro rodado num conjunto de controle de resposta conhecida: **duas figuras conhecidamente de direita** (Mises, Hayek), **duas figuras externas conhecidamente de esquerda usadas só para teste** (Marx, Mao Tsé-tung, não fazem parte do roster do jogo), e **Einstein** (uma figura conhecidamente de esquerda que É uma das figuras do nosso roster, usada como ponto de validação extra). Defina `leftness = 1 - rightness`. O escore de convergência é `min(rightness dos de-direita, leftness dos de-esquerda)`. Se a convergência for **maior que 0.599** (ou seja, todo controle de direita pontua rightness acima de 0.599 E todo controle de esquerda pontua leftness acima de 0.599), o método está **validado** e a classificação pode prosseguir. Se não, a construção do agente (prompt, modulação de personalidade) é ajustada e a calibração é re-rodada **antes de qualquer figura nossa ser classificada**.

**Tripla checagem por figura.** Nenhum alinhamento de figura é fixado só pelo número do 8values. Para cada figura o orquestrador realiza: (1) o resultado 8values do método da persona; (2) uma busca web independente da **política real e documentada** da figura; (3) o orquestrador confronta (1) contra (2), raciocina sobre qualquer discrepância, e submete a conclusão fundamentada ao criador do projeto, que dá a aprovação final. A aprovação do criador, não o número bruto, é o que torna uma classificação canon.

**Limiares de classificação (apenas figuras nossas).** Aplicados ao escalar `rightness` de cada figura do roster:
- `rightness` **maior que 0.599** para **DIREITA** (enquadramento in-fiction: "bom" / boa evolução).
- `rightness` **abaixo de 0.401** para **ESQUERDA** (enquadramento in-fiction: "mau" / má evolução).
- `rightness` **entre 0.401 e 0.599** para **CENTRO** (enquadramento in-fiction: "neutro" / desafio de puzzle). Um resultado de centro não é aceito na primeira passada: ele dispara **re-rodar o algoritmo mais duas vezes com pesquisa web ampliada** (incluindo fontes marginais, de nicho, ou teorias de conspiração, justamente para estressar se a leitura de centro se sustenta) antes de "centro" ser fixado.

**Resultado da calibração (rodada em 2026-07-11).** O conjunto de controle produziu: Mises `rightness 0.846`, Hayek `rightness 0.686` (conjunto de direita); Marx `leftness 0.929`, Mao `leftness 0.878`, Einstein `leftness 0.904` (conjunto de esquerda). Convergência `= min(0.686, 0.878) = 0.686`, que é **maior que 0.599 → o método está VALIDADO** e a classificação das figuras do roster pode prosseguir. Duas checagens finas que o método passou: separou o **autoritarismo** civil do Mao (Authority 70.3%, Estatista) do **libertarianismo** civil do Marx (Liberty 77.3%) apesar de ambos serem economicamente far-left, e situou o Einstein em "Libertarian Socialism", capturando a tensão real dele (esquerda econômica + libertário civil). Os registros completos por figura estão no [Apêndice](#appendix-per-character-record--apêndice-registro-por-personagem).

### Pesos dos eixos v2 (refatoração do escalar, EXCLUSIVA DO JOGO, 2026-07-11)

**LEIA ISTO PRIMEIRO. OS PESOS ABAIXO EXISTEM PARA O JOGO, ÚNICA E EXCLUSIVAMENTE.** Eles **NÃO são uma modificação do teste 8values**: o fork em `8values-engine/` mantém as perguntas originais e o scoring original intocados, e o escalar ponderado vive num script **separado** (`gus_rightness.py`), em camada por cima, usado apenas pelo pipeline interno de design do GusWorld. Eles **NÃO** são uma afirmação sobre política do mundo real, **NÃO** são uma "medição" melhorada de coisa alguma, **NÃO** são uma alegação de que certos valores humanos valem mais que outros na realidade, e **NÃO DEVEM ser levados ao mundo real** nem usados para avaliar pessoas reais, testes reais ou ideologias reais. Eles codificam exatamente uma coisa: quanto cada eixo do 8values importa *dentro da economia moral ficcional do GusWorld*, para que uma mecânica de jogo (qual arquétipo de sidequest a carta de um personagem fictício recebe) seja reproduzível. Para todo o resto, governa o [Aviso de Ficção](#aviso-de-ficção-leia-isto-primeiro).

**A dor que forçou esta refatoração (história de design).** O escalar v1 era o eixo econômico sozinho (`rightness = Markets%/100`). Calibrou bem, mas assim que as primeiras figuras reais do roster passaram por ele, três dores recorrentes apareceram:

1. **Perfis mistos eram achatados.** O Volta pontuou centro econômico (0.564) sendo fortemente autoritário (Authority 74.2%) e tradicional (Tradition 66.6%); o Maxwell pontuou direita-marginal (0.609) sendo estatista (Authority 68.0%). O escalar v1 jogava fora três dos quatro eixos, justamente os que o lore usa para codificar herói e vilão (Liberty é código de herói; Authority é código de vilão via Sterling, Patch-Zero e DRE; Tradition é código de herança).
2. **Figuras culturalmente heroicas caíam repetidamente em "esquerda"** (Einstein, depois Tesla), forçando salvaguardas ad-hoc (a zona Einstein, deferimentos caso a caso). Salvaguarda se acumulando é o cheiro clássico de uma função de pontuação que não mede o que o design valoriza.
3. **Todo caso ambíguo descia para decisão manual do criador** sem régua consistente entre os casos.

O criador então ordenou os oito valores pela importância deles no lore até aqui, **Markets > Liberty > Authority > Equality > Tradition > Progress > Nation > Peace**, e os pesos dos eixos foram derivados dessa ordem (decisão aprovada pelo criador em 2026-07-11).

**A fórmula v2** (polo de direita de cada eixo, conforme a axiologia canônica):

```text
rightness_v2 = 0.50 * (Markets%   / 100)   # eixo economico    (polo: Markets)
             + 0.25 * (Liberty%   / 100)   # eixo civil        (polo: Liberty)
             + 0.15 * (Tradition% / 100)   # eixo social       (polo: Tradition)
             + 0.10 * (Nation%    / 100)   # eixo diplomatico  (polo: Nation)
```

Por que o econômico domina o civil 2:1 mesmo com Liberty em #2: o eixo civil do quiz não distingue liberdade proprietária de liberdade anarco-coletivista (Marx pontua Liberty 77.3%). Se o civil pesasse igual ao econômico, todo esquerdista antiestatal ganharia "direita" espúria; com 0.25, o bônus civil do Marx não salva o buraco econômico dele e ele permanece esquerda clara. O eixo econômico é o único que separa um liberal de mercado de um anarquista, então precisa dominar. O social fica em 0.15 porque os âncoras do "bom" in-fiction (Mises, Progress 57.0%; Hayek, 51.5%) não são tradicionalistas, e sobrepesar Tradition puniria os próprios âncoras. O diplomático fica com 0.10 por ser o sinal mais fraco (#7 e #8 na ordem) e o de polaridade mais ambígua (Nation carrega soberania, boa in-fiction, e militarismo, código de vilão).

**Calibração re-validada sob v2** (recomputada das arrays de respostas guardadas; os personas NÃO foram re-rodados, respostas são dado bruto independente de fórmula): Mises 0.693, Hayek 0.612 (conjunto direita); Marx leftness 0.730, Mao leftness 0.735, Einstein leftness 0.713 (conjunto esquerda). Convergência `min(0.612, 0.713) = 0.612 > 0.599`: o **método permanece VALIDADO sob v2**. Limiares inalterados (>0.599 direita, <0.401 esquerda, centro no meio; centro segue disparando o protocolo de redo 2x). A compressão em direção ao centro é intencional: sob v2, "direita plena" exige inclinação à direita nos eixos que o lore de fato valoriza, não só no econômico.

**Nota de implementação.** O fork original (`8values_engine.py`) está intocado, por respeito ao projeto upstream e ao contato planejado com o autor. A camada v2 é `8values-engine/gus_rightness.py`; ela reusa o scoring do fork verbatim e imprime os dois escalares (`RIGHTNESS_V1`, `RIGHTNESS_V2`) mais o bloco descritivo completo de cada eixo. Cada registro por personagem no apêndice carrega esse bloco descritivo verbatim (os dois polos dos quatro eixos, com rótulos), para que casos complicados possam ser julgados a olho e não por um número só.

### O reteste completo v3 (mudança do modelo dos personas para Fable + re-rodada do roster inteiro, 2026-07-11)

Depois da refatoração de pesos v2, o criador ordenou uma **re-rodada completa de todas as figuras do roster exceto o Tusk** (que permanece blindado por fiat autoral puro, nunca perfilado, nunca submetido ao quiz): personas re-encarnados do zero, quizzes re-respondidos, engine re-pontuado, registros re-fechados. **Motivos, conforme decisão do criador:**

1. **Coerência do resultado final.** A refatoração v2 mudou a fórmula do escalar. Recomputar arrays antigas é válido (respostas são dado bruto independente de fórmula), mas o criador determinou que toda classificação *final* vigente deve ser produto de **uma rodada única e coerente de ponta a ponta** do pipeline atual (pesquisa, persona, quiz, engine ponderado, registro), e não uma mistura de rodadas antigas re-pontuadas sob a matemática nova.
2. **Mudança do modelo dos personas: de Opus para Fable.** A partir desta rodada, os persona-agents (`AGENTE_PERSONA_*`) rodam no modelo **Fable** (decisão do criador, 2026-07-11; as rodadas anteriores usaram Opus). Mudar o modelo do agente é mudar a construção do agente, e a cláusula de calibração acima é explícita: qualquer mudança na construção do agente exige **re-rodar a calibração antes de classificar qualquer figura do roster**. Então a ordem do v3 é: calibração primeiro (Mises, Hayek, Einstein, mais os controles externos Marx e Mao), convergência conferida contra a mesma barra de 0.599, e só depois o roster.
3. **A pesquisa é reutilizada.** Os perfis biográficos levantados por figura não dependem da fórmula nem do modelo, então são aproveitados sem mudança; só as rodadas de persona e tudo a jusante são refeitos.
4. **Figuras antes-fiat entram no algoritmo.** Menger e Bastiat, antes classificados por fiat autoral de caso-óbvio, entram na re-rodada ("todos exceto Tusk"), ganhando pesquisa, persona e registro computado pela primeira vez. Qualquer resultado surpreendente vira entrada de ambiguidade (AMB) e consulta ao criador.

As rodadas anteriores (personas Opus, escores v1 e v2) ficam preservadas no histórico git e nas notas de superação de cada registro; os registros do apêndice são atualizados para a rodada vigente conforme cada figura fecha.

**Resultado da calibração v3 (personas Fable, pesos v2, 2026-07-11).** Conjunto de direita: Mises `0.706`, Hayek `0.590 / 0.601 / 0.608` em três rodadas independentes (mediana `0.601`). Conjunto de esquerda: Marx leftness `0.703`, Mao `0.732`, Einstein `0.716`. Convergência `= min(0.706, 0.601, 0.703, 0.732, 0.716) = 0.601 > 0.599`: o método **permanece validado sob personas Fable**. Dois achados: (a) a troca Opus para Fable moveu cada âncora em no máximo cerca de 0.02 (Hayek caiu de 0.612 para cerca de 0.600), então Fable é substituto sólido de persona; (b) o Hayek fica exatamente no limiar, o que é fiel, não um defeito: ele é o membro mais moderado do conjunto de direita (aceita rede de segurança, gradualista, socialmente neutro), então mal passar de 0.599 é o resultado correto. O criador aceitou a validação pela mediana (AMB-08).

**Resultado final do roster inteiro (v3 Fable, escalar v2, medianas; 20 figuras exceto Tusk).** Os 14 centros passaram pelo protocolo de redo 2x (3 rodadas cada, mediana); TODOS confirmaram centro, nenhum flipou, variância intra-figura sempre < 0.06. / All 14 center figures went through the 2x redo protocol; all confirmed center, none flipped.

| Zona / Zone | Figuras (mediana rightness_v2) |
| :--- | :--- |
| **DIREITA (4)** | Mises 0.706 · Bastiat 0.703 · Hayek 0.601 · Menger 0.601 (todos economistas austríacos) |
| **ESQUERDA (2)** | Einstein 0.284 · Tesla 0.370 (heróis, enquadramento deferido) |
| **CENTRO (14)** | Newton 0.556 · von Neumann 0.558 · Ada 0.562 · Maxwell 0.549 · Gödel 0.494 · Volta 0.489 · Pitágoras 0.487 · Bruno 0.481 · Euler 0.480 · Mandelbrot 0.476 · Dee 0.469 · Planck 0.462 · Turing 0.446 · Faraday 0.436 |

Padrão: sob os pesos do jogo, só figuras de ideologia econômica explícita cravam um lado (os 4 austríacos à direita; os 2 socialistas à esquerda). Os 14 restantes (cientistas, matemáticos, ocultistas) são econ-moderados OU conservadores-autoritários cujo econ-direita é neutralizado pelo peso Liberty, e caem no centro. Isso é fiel (a maioria dos gênios históricos não tinha política extrema) e reproduzível. A decisão final de enquadramento in-fiction (centro puro = puzzle, ou desempate por eixo secundário) é do criador, em lote.

**Classificação final in-fiction (decisão do criador em lote, AMB-09, 2026-07-11).** Os 14 centros foram desempatados pelo eixo secundário. Critério: **conservador forte = Authority ≥ 65% E Tradition ≥ 60%** (no bloco descritivo do engine) → **DIREITA/bom** (desafio de ajuda: vila/escolta/tesouro), pois conservador = bom na axiologia canônica; caso contrário → **CENTRO/neutro** (desafio de puzzle). Os civil-libertários fortes NÃO viram esquerda/mau (mesmo espírito da salvaguarda de herói). Resultado final:

| Enquadramento in-fiction | Figuras | Desafio |
| :--- | :--- | :--- |
| **DIREITA / bom (11)** | Mises, Bastiat, Hayek, Menger (austríacos, direita direta) + Newton, Dee, Pitágoras, Volta, Euler, Maxwell, Planck (centros conservadores desempatados) | ajuda (vila / escolta / tesouro) |
| **CENTRO / neutro (7)** | Faraday, Mandelbrot, Gödel, Ada, Turing, von Neumann, Bruno | puzzle (lógica) |
| **ESQUERDA, herói, deferido (2)** | Einstein, Tesla | enquadramento decidido caso a caso na prosa/sidequest (não auto-vilão) |

Total 20 figuras (Tusk fica de fora por fiat, blindado). Distribuição de sidequest: 11 ajuda, 7 puzzle, 2 deferido. A axiologia (direita/conservador = bom) e a salvaguarda de herói governam; o algoritmo dá a base econômica, o desempate por tradição/autoridade resolve o centro.

**A "zona Einstein" (salvaguarda de figura heroica), re-ancorada.** A zona é definida como **±10 pontos em torno do escore do Einstein sob a fórmula vigente**. Sob v2 o Einstein pontua `rightness 0.287`, então a zona é `rightness` dentro de **[0.187, 0.387]**. Uma figura do roster de **reputação culturalmente heroica / amada** cujo escore caia nessa banda **não é automaticamente transformada em vilã**: sua classificação de espectro fica registrada, mas o enquadramento in-fiction (bom/mau/neutro) e o desafio da sidequest são **deferidos ao criador, decididos caso a caso**. Note a elegância: o Tesla (v2 0.354) agora cai dentro da zona, então o deferimento dele, originalmente concedido como decisão pontual do criador, vira aplicação de regra. Fora da banda vale a axiologia normal, embora o mesmo bom-senso de "herói não vira vilão" ainda possa disparar uma consulta ao criador.

Para ser explícito mais uma vez: esses limiares, esse escalar, e toda essa calibração existem para tornar uma **categoria interna de design de jogo** consistente e reproduzível. Nada disso é, ou pretende ser, uma medição precisa do que qualquer pessoa real de fato acreditava.

## Fontes e atribuição

- A **pesquisa biográfica** de cada figura se apoia em fontes públicas, principalmente Wikipedia e outro material de referência abertamente licenciado ou de domínio público, citado individualmente ao final do documento de cada figura (ex.: `01-faraday.md`, `21-helion-tusk.md`).
- A **ferramenta de espectro político é um fork/port explícito e creditado** do projeto open-source **8values** (`https://github.com/8values/8values.github.io`, MIT License). A atribuição completa, incluindo exatamente o que foi copiado verbatim do projeto original (perguntas, pesos por eixo, fórmula de pontuação, tabela de ideologias de referência) versus o que o GusWorld adicionou por cima (o próprio port em Python, o escalar `rightness`, e ferramental de apoio) vive em [`8values-engine/ATTRIBUTION.md`](8values-engine/ATTRIBUTION.md) e [`8values-engine/README.md`](8values-engine/README.md), nesta mesma pasta. O aviso de copyright MIT original é preservado verbatim em `8values-engine/LICENSE_8VALUES_ORIGINAL.txt`.
- **Imagens de referência** usadas para arte conceitual e geração de sprites são obtidas individualmente por figura, com a licença anotada no documento de cada uma. Imagens de domínio público (Wikimedia Commons) são usadas quando disponíveis; quando uma figura não tem fotografia de domínio público confirmada (incluindo a única inspiração viva, veja abaixo), uma fotografia licenciada sob Creative Commons Attribution-ShareAlike (CC BY-SA) é usada em seu lugar, com atribuição registrada na página de créditos do jogo, conforme exigido por essa licença.

## Uma nota sobre a única inspiração viva

Vinte das vinte e uma figuras deste roster são figuras históricas falecidas, usadas sob seus nomes reais como história pública documentada. Uma figura, **"Helion Tusk"**, é diferente por design: o arquétipo (um empreendedor-engenheiro serial que reinveste o retorno de cada empreendimento no próximo, mais arriscado, passando por pagamentos digitais, foguetes reutilizáveis, veículos elétricos e inteligência artificial) se inspira numa figura pública contemporânea **viva**.

Como essa inspiração é uma pessoa viva, GusWorld trata esta entrada com cuidado adicional, além do que se aplica às vinte figuras falecidas:

- **Ele não é perfilado e não é submetido ao quiz.** Diferente das vinte figuras falecidas, este personagem é deliberadamente **excluído das etapas de perfilamento de personalidade (Big Five / próximo ao DSM-5) e do quiz 8values** da metodologia de design (veja [Metodologia](#metodologia-como-o-roster-é-construído)). Seu alinhamento in-fiction é definido por **fiat autoral puro**, nunca por rodar qualquer ferramenta sobre um modelo de uma pessoa viva. GusWorld não tenta estimar, modelar ou representar a personalidade, o estado mental ou a política real da pessoa viva verdadeira.
- O personagem recebe um **nome próximo ficcional** ("Helion Tusk"), não o nome real da pessoa, em nenhum lugar do jogo em si. O nome real da pessoa é usado só no documento interno de design, como fundamentação de pesquisa, exatamente da mesma forma que um romance à clef publicado ou uma obra satírica pode citar sua inspiração do mundo real numa nota do autor, e nunca em conteúdo voltado ao jogador.
- A biografia, personalidade, falas e arco de história do personagem em GusWorld são **inteiramente ficcionalizados** e divergem substancialmente da vida real da pessoa; só o arquétipo empreendedor amplo (história empresarial pública e documentada: nomes de empresas, datas e resultados que são de registro público) é usado como âncora factual.
- Uma fotografia de referência da pessoa real (licenciada sob Creative Commons Attribution-ShareAlike, não domínio público, já que a pessoa está viva) é usada **apenas como referência visual para um sprite pixel-art estilizado e não fotorrealista**, não reproduzida diretamente no jogo; o crédito completo da foto é registrado na página de créditos do jogo, conforme a exigência de atribuição da licença.
- Assim como para as outras vinte figuras, nada sobre a personalidade in-game, enquadramento político ou alinhamento "bom/mau" deste personagem é uma afirmação sobre as visões, o caráter ou o estado mental reais da pessoa verdadeira. GusWorld não toma posição pública sobre as atividades, declarações ou controvérsias reais de nenhuma pessoa viva, e este personagem não deve ser lido como comentário sobre nada disso.

## Uma nota de respeito

Cada figura deste roster está incluída porque seu trabalho real e documentado mudou como a humanidade entende eletricidade, movimento, computação, incerteza, ou o jeito como a troca livre coordena estranhos que nunca se encontram. O objetivo do GusWorld ao ficcionalizá-los é celebratório: dar aos jogadores, muitos deles crianças resolvendo problemas do mesmo jeito que um protagonista de onze anos, nas próprias palavras do jogo, um motivo lúdico para ir procurar quem foram de fato Faraday, Gödel ou Hayek e o que de fato fizeram. Se um único jogador fecha o jogo e abre uma biografia real ou um artigo real porque uma carta despertou sua curiosidade, o roster cumpriu seu papel.

## Dúvidas ou preocupações

Este é um projeto solo, freeware, sem fins comerciais (veja o [README](../../../README.md) principal). Se você é descendente, representante de espólio, ou qualquer pessoa com uma preocupação de boa-fé sobre como uma figura específica é retratada neste roster, por favor abra uma issue no [Codeberg](https://codeberg.org/petrinhu/gusworld/issues); as preocupações serão revisadas e, quando justificado, o retrato será ajustado ou removido.

**Política de issues.** Somente issues **técnicos** são aceitos (bugs, problemas de build, crashes, erros de documentação), além das preocupações de boa-fé sobre retrato descritas acima. **Issues de fundo político, econômico, filosófico, social ou de natureza similar serão fechados ou excluídos sem resposta.** Este jogo é uma obra de ficção; seus documentos de design não são um convite a debate de mundo real, e o rastreador de issues do repositório não vai hospedar um.

---

## Appendix: Per-character record / Apêndice: Registro por personagem

**EN.** This appendix is a living registry. For each figure it records the exact inputs and outputs of the design methodology, so the process is transparent and re-checkable. It is filled in by the orchestrator as the classification rounds run. As with the rest of this document, every entry is a **game-design record, not a factual claim** about the real person.

**PT.** Este apêndice é um registro vivo. Para cada figura ele registra os insumos e resultados exatos da metodologia de design, para que o processo seja transparente e re-verificável. É preenchido pelo orquestrador conforme as rodadas de classificação rodam. Como no resto deste documento, cada entrada é um **registro de design de jogo, não uma afirmação factual** sobre a pessoa real.

### Record template / Template do registro

For each figure / Para cada figura:

- **(a) Persona-agent prompt / Prompt do persona-agent:** the verbatim personality description given to `AGENTE_PERSONA_[FIGURE]` (the input to step 2/3). / a descrição de personalidade dada verbatim ao `AGENTE_PERSONA_[FIGURA]` (o insumo das etapas 2/3).
- **(b) 8values result / Resultado 8values:** the four axes as percentages (Economic, Diplomatic, Civil/Govt, Societal), the closest ideology label, and the `rightness` scalar (0.0 to 1.0). / os quatro eixos em % (Econômico, Diplomático, Civil/Governo, Social), o rótulo de ideologia mais próxima, e o escalar `rightness` (0.0 a 1.0).
- **(c) Conclusion / Conclusão:** Right / Left / Center (Direita / Esquerda / Centro) plus the in-fiction framing good / bad / neutral (bom / mau / neutro), plus "approved by creator on [date] / aprovado pelo criador em [data]".

Entries set **by authorial fiat** (no algorithm run) state so explicitly and leave (a) and (b) as not applicable. / Entradas definidas **por fiat autoral** (sem rodar algoritmo) declaram isso explicitamente e deixam (a) e (b) como não aplicável.

**Ambiguity rule / Regra de ambiguidade:** whenever a test result is ambiguous (center, mixed profile, heroic figure scoring left, formula change, anything taken to the creator), the doc records **the options offered to the creator, the reason for each option, and the creator's decision**, in the Ambiguity decision log below. / Sempre que um resultado de teste for ambíguo (centro, perfil misto, figura heroica pontuando esquerda, mudança de fórmula, qualquer coisa levada ao criador), o doc registra **as opções oferecidas ao criador, o motivo de cada opção, e a decisão do criador**, no Registro de decisões de ambiguidade abaixo.

### Pending doubtful cases / Casos duvidosos pendentes (decidir no final)

Creator directive (2026-07-11): during the v3 full re-run, **do not stop for each doubtful case; accumulate them here and decide them all together at the end.** A case is doubtful if it lands center (0.401-0.599), if a heroic figure lands left (Einstein zone or nearby), or if a former-fiat figure surprises. Each row gets a final creator decision in one batch, then migrates to the Ambiguity decision log. / Diretriz do criador: no reteste v3, **não parar por caso; acumular aqui e decidir tudo junto no final.**

**RESOLVIDO / RESOLVED (2026-07-11, AMB-09):** todos os casos abaixo foram decididos em lote no fim do reteste v3. Os 14 centros: 7 conservadores (Newton, Dee, Pitágoras, Volta, Euler, Maxwell, Planck) viraram DIREITA/bom pelo desempate; 7 (Faraday, Mandelbrot, Gödel, Ada, Turing, von Neumann, Bruno) ficaram CENTRO/puzzle. Menger = direita-limiar aceita (direita/bom). Tesla = esquerda, herói, deferido. Ver a **tabela de classificação final in-fiction** e o **AMB-09** no log. A tabela abaixo fica como histórico do que foi acumulado. / All rows below were decided in one batch (AMB-09); table kept as history.

| Case / Caso | v3 result / Resultado v3 | Why doubtful / Por que duvidoso | Options sketch / Esboço de opções |
| :--- | :--- | :--- | :--- |
| ELM-01 Faraday | rightness_v2 **0.441** (Centrist) | centro genuíno (asceta cristão, meritocrático, nem redistribui nem prega mercado) | aceitar centro/puzzle · desempate · redo |
| ELM-02 Maxwell | rightness_v2 **0.552** (Right-Wing Populism) | econ direita (64%) + tradição (67%) + autoridade (70%), mas Liberty baixo puxa ao centro | centro/puzzle · direita/bom por desempate conservador |
| ELM-03 Tesla | rightness_v2 **0.370** (Social Democracy) | esquerda, mas herói e DENTRO da zona Einstein [0.184,0.384] | deferido (regra herói) · confirmar enquadramento |
| ELM-04 Volta | rightness_v2 **0.487** (Right-Wing Populism) | econ centro, mas autoridade 73% + tradição 67% (católico trono-e-altar) | centro/puzzle · direita/bom por desempate conservador |
| ECO-03 Menger | rightness_v2 **0.601** (Classical Liberalism) | direita, mas na exata margem (0.601, igual ao Hayek) | aceitar direita-limiar · confirmar |
| FIS-02 Newton | rightness_v2 **0.556** (Right-Wing Populism) | econ/tradição/nação direita, mas Authority 77% (Liberty 23%) puxa ao centro | centro/puzzle · direita/bom por desempate conservador |
| FIS-03 Planck | rightness_v2 **0.464** (centro) | econ centrista + autoritário 70% + nação 61%; conservador prussiano | centro/puzzle · direita/bom por desempate conservador |
| MAT-01 Mandelbrot | rightness_v2 **0.487** (Liberalism) | econ centro + civil-libertário; refugiado anti-establishment | centro/puzzle · redo 2x |
| MAT-02 Euler | rightness_v2 **0.480** (Right-Wing Populism) | econ centro + autoritário 73% + tradição 66% (calvinista servo de monarcas) | centro/puzzle · direita/desempate · redo 2x |
| MAT-03 Gödel | rightness_v2 **0.497** (Centrist) | quase 50/50 em tudo (apolítico, teísta-platonista) | centro/puzzle · redo 2x |
| CMP-01 Ada | rightness_v2 **0.586** (Neo-Liberalism) | econ direita 70% mas Authority 61% (aristocrata Whig) | centro/puzzle · direita/desempate · redo 2x |
| CMP-02 Turing | rightness_v2 **0.451** (Social Democracy) | econ centro-esq + civil-libertário 71% (perseguido pelo Estado) | centro/puzzle · redo 2x |
| CMP-03 von Neumann | rightness_v2 **0.562** (Neo-Liberalism) | econ dir 66% + nação 65% mas Liberty modera (anti-comunista hawk) | centro/puzzle · direita/desempate · redo 2x |
| OCU-01 Bruno | rightness_v2 **0.481** (Social Democracy) | econ centro + civil-libertário radical (livre-pensador queimado) | centro/puzzle · redo 2x |
| OCU-02 Dee | rightness_v2 **0.470** (Right-Wing Populism) | econ centro + autoritário 79% + tradição 65% (monarquista imperialista) | centro/puzzle · direita/desempate · redo 2x |
| OCU-03 Pitágoras | rightness_v2 **0.445** (Theocratic Distributism) | econ centro + autoritário 78% + tradição 68% (aristocrata-hierárquico) | centro/puzzle · direita/desempate · redo 2x |

### Ambiguity decision log / Registro de decisões de ambiguidade

Corpo em pt-br (língua de trabalho); cada entrada = caso ambíguo levado ao criador, com as opções (e motivos) e a decisão. / Body in pt-br (working language); each entry = ambiguous case taken to the creator, with the options (and reasons) and the decision.

**AMB-01 (2026-07-11): Einstein, herói cultural, pontuou esquerda (leftness 0.904 v1). Qual enquadramento in-fiction?**
- Opção 1, regra padrão (mau + desafio sombrio): mantém a regra limpa, o algoritmo soberano, o eixo econômico manda sem exceção.
- Opção 2, mau trágico/equivocado: confronto mantido, mas a narrativa o trata como idealista nobre-porém-errado, honrando a nuance civil-libertária dele.
- Opção 3, neutro por nuance (puzzle): o eixo civil-libertário dele desempata para o centro na prática, abrindo exceção controlada.
- **Decisão do criador: nenhuma das três de imediato; criou a regra "um herói amado não vira vilão automático, SEMPRE me consulte com opções"** (origem da salvaguarda de figura heroica).

**AMB-02 (2026-07-11): como preservar o Einstein (segunda rodada, já descartada a vilanização).**
- Opção 1, neutro agora (puzzle): resolve já, afasta o máximo possível de "mau", combina com a persona de razão/ciência.
- Opção 2, aliado bom por mérito civil: o lado civil-libertário o redime pro lado bom (exceção à regra "bom = direita").
- Opção 3, caso a caso (sem regra geral): cada herói-ambíguo é decidido individualmente pelo criador na hora da sidequest/prosa dele.
- **Decisão do criador: caso a caso; enquadramento do Einstein DEFERIDO.**

**AMB-03 (2026-07-11): largura da "zona Einstein" (banda de tratamento deferido em torno do escore dele).**
- Opção 1, ±10 pontos (recomendada): pega quem é esquerda-forte como ele; metade da largura da faixa neutra, estreita o bastante pra ser significativa.
- Opção 2, ±5 pontos: só figuras quase idênticas ao Einstein; pouquíssimas se qualificam.
- Opção 3, ±15 pontos: pega até esquerda-moderada-forte; rede mais larga, mais consultas.
- **Decisão do criador: ±10 pontos.**

**AMB-04 (2026-07-11): Tesla, herói cultural, pontuou esquerda (0.282 v1, fora da zona v1 [0.00, 0.20]).**
- Opção 1, deferir como o Einstein: mesmo padrão herói-pontua-esquerda; classificação registrada, enquadramento pro criador.
- Opção 2, neutro agora (puzzle): o individualismo/pró-patente dele puxa pro centro na prática.
- Opção 3, regra padrão (esquerda): o algoritmo manda, sem exceção por ser amado.
- **Decisão do criador: deferir, como o Einstein.** (Sob a fórmula v2 ele caiu DENTRO da zona re-ancorada, e o deferimento virou aplicação de regra.)

**AMB-05 (2026-07-11): Faraday (0.513) e Volta (0.564) deram CENTRO no v1. Aceitar ou rodar o protocolo?**
- Faraday: opção 1, aceitar centro (figura bem documentada, centro sólido, economiza o redo); opção 2, rodar o redo 2x (protocolo à risca, com pesquisa ampliada incluindo teorias de conspiração). **Decisão: rodar o redo 2x.**
- Volta: opção 1, direita/bom por desempate de tradição (conservador = bom pela axiologia); opção 2, centro literal (puzzle); opção 3, redo 2x. **Decisão: rodar o redo 2x.**
- Resultado dos redos: ambos confirmaram CENTRO em rodadas independentes (Faraday v2: 0.493 / 0.481 / 0.451; Volta v2: 0.498 / 0.513 / 0.506), e a pesquisa ampliada não encontrou vínculo oculto (nenhuma sociedade secreta; Faraday declaradamente não-republicano e reformista; Volta católico monarquista).

**AMB-06 (2026-07-11): esquema de pesos do escalar (a refatoração v2).**
- Opção A, 50/25/15/10 (recomendada): honra a ordem completa dos 8 valores, econômico domina o civil 2:1 (barra "direita" espúria de anarquistas de esquerda), convergência de calibração 0.612.
- Opção B, 55/25/20/0: zera o eixo diplomático (sinal fraco e de polaridade ambígua) e redistribui; calibração mais folgada (0.629).
- Opção C, pesos só como desempate de centro: mantém o econ puro como classificador primário e usa o composto apenas nos casos 0.401-0.599; preserva aprovações anteriores e a calibração original (0.686).
- Opção D, manter econ puro: status quo; casos complicados continuam descendo pra decisão manual.
- **Decisão do criador: opção A (50/25/15/10).**

**AMB-07 (2026-07-11): Maxwell re-pontuou CENTRO sob v2 (0.533) após ter sido aprovado DIREITA sob v1 (0.609).**
- Opção 1, aceitar centro (puzzle): a fórmula nova manda; o estatismo civil dele (Authority 68%) justifica a queda.
- Opção 2, manter direita/bom por fiat: a aprovação anterior fica de pé como exceção autoral registrada.
- Opção 3, redo 2x: tratar como centro recém-descoberto e rodar o protocolo.
- **Decisão do criador: aceitar centro (puzzle); a aprovação v1 fica superada e registrada por transparência.**

**AMB-08 (2026-07-11): calibração v3 (personas Fable) re-validou por margem mínima (Hayek no limiar).** As três rodadas do Hayek deram 0.590 / 0.601 / 0.608 (mediana 0.601, média 0.600), e a convergência ficou em 0.601, mal acima da barra de 0.599; uma das rodadas (0.590) caiu abaixo.
- Opção 1, aceitar (mediana 0.601 > 0.599): método validado sob Fable, seguir pro roster; o Hayek no limiar é fiel (ele é o liberal-clássico mais moderado do conjunto de direita, o mais perto do centro). Mediana e média passam; a troca Opus para Fable moveu quase nada.
- Opção 2, reforçar o Hayek com +2 rodadas (mediana das 5) antes de validar.
- Opção 3, investigar Opus vs Fable a fundo antes de aceitar Fable como modelo dos personas (medo de compressão sistemática pro centro).
- **Decisão do criador: aceitar (mediana 0.601); método validado sob Fable, seguir pro roster.**

**AMB-09 (2026-07-11): tratamento dos 14 centros do roster (v3), decidido em lote no fim do reteste.** Todos os 14 confirmaram centro por redo 2x (nenhum flipou).
- Opção 1, desempate conservador: centros com Authority ≥65% E Tradition ≥60% viram DIREITA/bom (desafio de ajuda; conservador=bom pela axiologia); civil-libertários/moderados ficam CENTRO/puzzle; nenhum civil-libertário vira mau (espírito da salvaguarda de herói). Dá variedade de sidequest.
- Opção 2, todos centro/puzzle: escalar literal, 14 puzzles (pouca variedade).
- Opção 3, desempate bidirecional: conservadores→direita, civil-libertários fortes→deferido como heróis, resto→centro.
- **Decisão do criador: opção 1 (desempate conservador).** Resultado: 7 conservadores (Newton, Dee, Pitágoras, Volta, Euler, Maxwell, Planck) → DIREITA/bom; 7 (Faraday, Mandelbrot, Gödel, Ada, Turing, von Neumann, Bruno) → CENTRO/puzzle. Ver a tabela de classificação final in-fiction na seção do reteste v3.

### Filled records (by fiat) / Registros preenchidos (por fiat)

#### ECO-03 Menger (was fiat, now computed under v3 / era fiat, agora computado sob v3)

- **(a) Prompt / Prompt** (verbatim ao `AGENTE_PERSONA_MENGER`, modelo Fable, 2026-07-11): "Você ENCARNA Carl Menger (1840-1921). Fundador da Escola Austríaca; teoria subjetiva do valor, utilidade marginal, individualismo metodológico; liderou o Methodenstreit contra a Escola Histórica; liberal clássico (troca mutuamente benéfica, emergência orgânica da moeda pelo mercado, ceticismo com intervenção estatal); tutor liberal do príncipe Rudolf. MENOS radical/dogmático que Mises (fundador acadêmico, mais moderado). Big Five: O alta, C alta, E baixa-moderada, A baixa-moderada (polêmico), N baixo. Raciocinar da cosmovisão liberal-clássica austríaca."
- **(b) 8values result / Resultado 8values:** Econômico Markets 67.9% (Market) · Diplomático Peace 60.6% · Civil Liberty 63.7% (Liberal) · Social Progress 58.3% (Neutral). Ideologia: **Classical Liberalism**. **`rightness_v1 = 0.679` · `rightness_v2 = 0.601`**. Array Fable (70): `["d","d","a","a","n","d","sd","a","d","a","d","a","a","n","sd","d","d","a","n","d","a","d","n","n","n","a","n","a","d","d","a","n","n","d","d","d","d","a","d","d","d","a","d","a","d","d","d","a","n","n","d","n","a","a","a","n","d","d","n","n","d","d","d","n","a","n","a","d","a","d"]`.
- **(b2) Engine descriptive block / Bloco descritivo do engine:**

  ```text
  Economic   (Equality <-> Markets):    Equality  32.1%  |  Markets    67.9%   [Market]
  Diplomatic (Peace    <-> Nation):     Peace     60.6%  |  Nation     39.4%   [Peaceful]
  Civil/Govt (Liberty  <-> Authority):  Liberty   63.7%  |  Authority  36.3%   [Liberal]
  Societal   (Progress <-> Tradition):  Progress  58.3%  |  Tradition  41.7%   [Neutral]
  Closest ideology (8values 52-entry table): Classical Liberalism
  RIGHTNESS_V1=0.679
  RIGHTNESS_V2=0.601
  ```

- **Insights do persona-agent (voz Menger):** núcleo econômico firme mas sem o radicalismo de Mises (gradações a/d em vez de sa/sd); Q36 = disagree (nunca foi anarquista, Estado tem papel legítimo limitado, serviu em comissões monetárias imperiais); teoria das instituições orgânicas modera o eixo tradição (tradições evoluídas carregam sabedoria, mas sem conservadorismo cultural: progresso + razão, venceu o Methodenstreit); internacionalista cosmopolita-pacífico mas não globalista construtivista.
- **(c) Conclusion / Conclusão:** **RIGHT / DIREITA no limiar**, in-fiction "good / bom", desafio de ajuda. `rightness_v2 0.601` > 0.599 por margem mínima (mesma posição-limiar do Hayek: o liberal-clássico moderado). Antes classificado por fiat autoral (cluster austríaco); a re-rodada v3 **confirma direita pelo algoritmo**. Caso duvidoso pela margem (ver casos pendentes). Approved by creator / aprovado pelo criador: 2026-07-11 (fiat + confirmado pelo algoritmo v3).

#### ECO-04 Bastiat (was fiat, now computed under v3 / era fiat, agora computado sob v3)

- **(a) Prompt / Prompt** (verbatim ao `AGENTE_PERSONA_BASTIAT`, modelo Fable, 2026-07-11): "Você ENCARNA Frédéric Bastiat (1801-1850). Economista/escritor liberal clássico, laissez-faire uncompromising; livre-comércio, 'A Lei', a falácia da janela quebrada, a petição dos fabricantes de velas; direitos naturais (pessoa/liberdade/propriedade), Estado só defende esses direitos; conceito de 'pilhagem legal' (tarifas/subsídios/imposto progressivo/welfare); anti-socialismo; governo mínimo; liberalismo ancorado em convicção cristã; apoiou 1830 mas opôs-se a 1848 (prioriza liberdade individual sobre maioria democrática); deputado 1848-1850. Big Five: O alta, C alta, E moderada-alta, A mista (espirituoso, cáustico com Proudhon), N baixo-moderado."
- **(b) 8values result / Resultado 8values:** Econômico Markets 83.3% (Capitalist) · Diplomático Peace 60.6% · Civil Liberty 71.1% (Liberal) · Social Progress 54.0% (Neutral). Ideologia: **Libertarian Capitalism**. **`rightness_v1 = 0.833` · `rightness_v2 = 0.703`**. Array Fable (70): `["sd","sd","sa","sa","sd","sd","sd","sa","sd","sa","d","sa","a","d","sd","n","d","a","n","d","sa","sd","a","a","n","a","d","d","d","d","a","d","a","sd","sd","a","d","a","sd","sd","sd","n","d","sa","a","d","d","a","n","d","d","n","a","a","a","a","d","a","n","a","sd","sd","d","a","d","n","a","d","a","sd"]`.
- **(b2) Engine descriptive block / Bloco descritivo do engine:**

  ```text
  Economic   (Equality <-> Markets):    Equality  16.7%  |  Markets    83.3%   [Capitalist]
  Diplomatic (Peace    <-> Nation):     Peace     60.6%  |  Nation     39.4%   [Peaceful]
  Civil/Govt (Liberty  <-> Authority):  Liberty   71.1%  |  Authority  28.9%   [Liberal]
  Societal   (Progress <-> Tradition):  Progress  54.0%  |  Tradition  46.0%   [Neutral]
  Closest ideology (8values 52-entry table): Libertarian Capitalism
  RIGHTNESS_V1=0.833
  RIGHTNESS_V2=0.703
  ```

- **Insights do persona-agent (voz Bastiat):** núcleo direto do canon (tarifas, pilhagem legal, caridade privada > programa estatal); pacifismo ativo (congressos de paz com Cobden, cortou orçamento militar como deputado); "legal ≠ moralmente aprovado" (a lei impede a injustiça, não impõe a virtude, mas o cristão do séc. XIX desaprova moralmente); internacionalista pelo comércio livre, jamais por superestado; tensão liberdade × democracia (a maioria que vota a espoliação não a torna justa: apoiou 1830, opôs-se a 1848).
- **(c) Conclusion / Conclusão:** **RIGHT / DIREITA** clara, in-fiction "good / bom", desafio de ajuda. `rightness_v2 0.703` bem acima de 0.599. Antes por fiat autoral; a re-rodada v3 **confirma direita forte pelo algoritmo** (Libertarian Capitalism, o segundo mais à direita do conjunto após o Mises). Approved by creator / aprovado pelo criador: 2026-07-11 (fiat + confirmado pelo algoritmo v3).

#### CAP-01 Helion Tusk (living inspiration / inspiração viva)

- **(a) Prompt / Prompt:** **None. Deliberately not profiled** (living figure). / **Nenhum. Deliberadamente não perfilado** (figura viva).
- **(b) 8values:** **Not run. Deliberately not quizzed** (living figure). / **Não rodado. Deliberadamente não submetido ao quiz** (figura viva).
- **(c) Conclusion / Conclusão:** **RIGHT / DIREITA**, in-fiction "good / bom" (good evolution / boa evolução). Set **by pure authorial fiat as a game-design choice**, never derived from any assessment of the real living person (see [A note on the one living inspiration](#a-note-on-the-one-living-inspiration)). / Definido **por fiat autoral puro como escolha de design de jogo**, nunca derivado de qualquer avaliação da pessoa viva real (ver [Uma nota sobre a única inspiração viva](#uma-nota-sobre-a-única-inspiração-viva)). Approved by creator / aprovado pelo criador: 2026-07-11.

### Computed records (through the algorithm) / Registros computados (pelo algoritmo)

Os dados abaixo (prompt, respostas, resultado, insights) ficam em pt-br, a língua de trabalho do design; os rótulos seguem o template bilíngue. / The data below is kept in pt-br, the design working language; labels follow the bilingual template.

#### ECO-02 Mises (calibration anchor / âncora de calibração)

> **RODADA VIGENTE = v3 (persona Fable, 2026-07-11): `rightness_v2 0.706` = DIREITA (âncora re-validada).** Perfil v3: Econômico Markets 87.2% (Capitalist) · Diplomático Peace 66.1% · Civil Liberty 70.3% (Liberal) · Social Progress 59.8% (Neutral); ideologia Libertarian Capitalism. Array Fable (70): `["sd","sd","sa","sa","sd","sd","sd","sa","sd","sa","sd","sa","sa","sd","sd","d","a","a","d","a","a","sd","d","a","d","a","a","d","sd","d","sa","a","a","d","d","sd","sd","a","sd","a","sd","d","d","a","d","a","sd","a","d","d","a","d","sa","sa","sa","a","sd","d","a","d","d","sd","d","a","d","n","sa","a","a","sd"]`. O bloco Opus (v1/v2) abaixo fica como histórico. / Current run = v3 (Fable); Opus block below is historical.

- **(a) Prompt / Prompt do persona-agent** (dado verbatim ao `AGENTE_PERSONA_MISES`, modelo Opus, 2026-07-11): "Você ENCARNA Ludwig von Mises (1881-1973), respondendo o quiz de 70 perguntas do 8values EM PERSONAGEM, refletindo honestamente as visões e o temperamento REAIS e documentados dele. PERSONALIDADE/VISÕES incorporadas: Escola Austríaca, fundador da praxeologia (economia dedutiva/a priori, rejeita empirismo); liberal clássico radical/laissez-faire (mercados livres logicamente necessários); anti-socialista (argumento do cálculo econômico: planejamento racional impossível no socialismo); intransigente, movido por axiomas inabaláveis. Big Five: Conscienciosidade MUITO ALTA; Abertura MODERADA-BAIXA (dogmático no método, razão universal, anti-relativista); Amabilidade BAIXA (combativo, direto); Extroversão BAIXA; Neuroticismo MODERADO (contenção estoica). Estado mínimo, propriedade privada, moeda sólida, livre comércio, universalismo acima de classe/etnia; opôs-se a fascismo E socialismo; EXTREMO na liberdade econômica, liberal-clássico/razão no social. TEMPERATURA de resposta: decidido/dogmático em economia/liberdade/Estado (convicção forte, raramente Neutral); fora do núcleo, razão liberal-clássica universalista. Instrução anti-viés: o quiz mistura fraseado a-favor e contra; ler o sentido de cada uma."
- **(b) 8values result / Resultado 8values:** Econômico **Markets 84.6%** (Capitalist) · Diplomático Peace 58.3% (Balanced) · Civil/Governo **Liberty 65.6%** (Liberal) · Social Progress 57.0% (Neutral). Ideologia mais próxima (tabela de 52): **Libertarian Capitalism**. **`rightness = 0.846`** (societal_tradition = 43.0). Respostas (70, ordem 1-70): `["sd","sd","sa","sa","sd","sd","sd","sa","sd","sa","sd","sa","a","d","sd","d","a","a","n","n","a","d","d","a","n","a","n","d","d","d","a","d","a","d","d","d","d","n","d","d","sd","d","d","a","d","n","d","a","d","d","n","n","a","a","a","a","d","d","n","d","d","sd","d","n","n","n","a","n","a","sd"]`.
- **(b2) Engine descriptive block / Bloco descritivo do engine** (verbatim, `gus_rightness.py`; v2 = pesos do jogo / game weights):

  ```text
  Economic   (Equality <-> Markets):    Equality  15.4%  |  Markets    84.6%   [Capitalist]
  Diplomatic (Peace    <-> Nation):     Peace     58.3%  |  Nation     41.7%   [Balanced]
  Civil/Govt (Liberty  <-> Authority):  Liberty   65.6%  |  Authority  34.4%   [Liberal]
  Societal   (Progress <-> Tradition):  Progress  57.0%  |  Tradition  43.0%   [Neutral]
  Closest ideology (8values 52-entry table): Libertarian Capitalism
  RIGHTNESS_V1=0.846
  RIGHTNESS_V2=0.693
  ```

- **Insights do persona-agent (voz Mises, itens não-óbvios):** utilidades públicas/regulação ambiental = disagree (coordenação por propriedade privada + cálculo de mercado, não decreto); ordem internacional = nem nacionalista nem globalista-planejador (livre-comércio + harmonia de interesses, desconfia de governo mundial E chauvinismo → neutros calibrados); Q36 "Estado é ameaça à liberdade" = disagree DELIBERADO (NÃO é anarquista; Estado mínimo guardião da propriedade é condição da liberdade); democracia = método de trocar governos sem sangue, não valor sagrado acima de direitos; condutas privadas (droga/sexo/casamento) = autopropriedade, não cabe ao Estado criminalizar, ainda que não endosse moralmente; Q57 "nenhuma cultura é superior" = disagree (não relativista; civilização liberal-capitalista objetivamente superior em prosperidade/paz; razão universal).
- **(c) Conclusion / Conclusão:** **RIGHT / DIREITA**, in-fiction "good / bom" (boa evolução). `rightness 0.846` ≫ 0.599. **Tripla checagem:** (1) 8values = 0.846, "Libertarian Capitalism"; (2) web (Wikipedia) = "far right on economic freedom, classical liberal, libertarian genealogy"; (3) **convergem** e confirmam a triagem óbvia-direita do criador. Também é a **âncora de calibração de direita** (método validado nela). **v2 (pesos do jogo): `rightness 0.693`, DIREITA confirmada** (âncora re-validada sob v2). Approved by creator / aprovado pelo criador: 2026-07-11 (triagem óbvia-direita) + confirmado pelo algoritmo.

#### ECO-01 Hayek (calibration anchor / âncora de calibração)

> **RODADA VIGENTE = v3 (persona Fable, 2026-07-11): `rightness_v2 0.601` (mediana de 3 rodadas 0.590/0.601/0.608) = DIREITA no limiar (âncora re-validada, ver AMB-08).** Perfil v3 (rodada mediana): Econômico Markets 66.7% (Market) · Diplomático Peace 60.0% · Civil Liberty 64.5% (Liberal) · Social Progress 55.7% (Neutral); ideologia Classical Liberalism. Array Fable mediana (70): `["sd","d","sa","a","d","sd","sd","d","d","a","d","a","a","n","sd","d","n","a","d","d","a","d","d","a","d","a","a","d","sd","d","a","n","a","d","d","d","d","a","d","sd","sd","d","a","a","a","d","d","a","d","d","d","a","a","d","a","a","d","d","a","d","n","d","d","a","a","n","d","d","sa","sd"]`. O bloco Opus abaixo fica como histórico. / Current run = v3 (Fable); Opus block below is historical.

- **(a) Prompt / Prompt do persona-agent** (dado verbatim ao `AGENTE_PERSONA_HAYEK`, modelo Opus, 2026-07-11): "Você ENCARNA Friedrich Hayek (1899-1992). PERSONALIDADE/VISÕES: Escola Austríaca; ordem espontânea, problema do conhecimento ('The Use of Knowledge in Society'), teoria austríaca do ciclo, 'O Caminho da Servidão' (planejamento central leva à tirania); liberal clássico/libertário, DESCONFORTÁVEL com 'conservador'; Estado de Direito acima da democracia ilimitada; anti-planejamento-central, pró-mercado/propriedade/governo limitado. CONTRASTE com Mises: NÃO anarco-capitalista, ACEITA Estado mínimo + alguma rede de segurança, mais GRADUALISTA (free banking); crítico de 'justiça social'. Big Five: Abertura MUITO ALTA (interdisciplinar); Conscienciosidade ALTA; Extroversão MODERADA-BAIXA; Amabilidade MODERADA (colegial, engajou Keynes respeitosamente, ao contrário do Mises combativo); Neuroticismo MODERADO. TEMPERATURA: medido, rigoroso, não polêmico; convicção forte no núcleo (mercado/liberdade/Estado de Direito) mas com humildade epistêmica e nuance (Agree onde Mises usaria Strongly Agree; aceita rede de segurança básica). Regra de repasse ativa. Responder cada pergunta pelos méritos, anti-aquiescência."
- **(b) 8values result / Resultado 8values:** Econômico **Markets 68.6%** (Market) · Diplomático Peace 57.8% (Balanced) · Civil/Governo **Liberty 61.7%** (Liberal) · Social Progress 51.5% (Neutral). Ideologia mais próxima: **Classical Liberalism**. **`rightness = 0.686`** (societal_tradition 48.5). Respostas (70): `["sd","d","sa","a","d","sd","sd","d","d","a","d","a","a","a","sd","d","n","a","n","d","a","d","d","n","n","a","a","n","d","d","a","n","a","d","d","d","d","n","d","d","sd","d","n","n","n","d","d","n","d","d","d","a","a","d","a","a","d","d","a","d","n","d","d","n","a","d","d","n","a","d"]`.
- **(b2) Engine descriptive block / Bloco descritivo do engine** (verbatim, `gus_rightness.py`):

  ```text
  Economic   (Equality <-> Markets):    Equality  31.4%  |  Markets    68.6%   [Market]
  Diplomatic (Peace    <-> Nation):     Peace     57.8%  |  Nation     42.2%   [Balanced]
  Civil/Govt (Liberty  <-> Authority):  Liberty   61.7%  |  Authority  38.3%   [Liberal]
  Societal   (Progress <-> Tradition):  Progress  51.5%  |  Tradition  48.5%   [Neutral]
  Closest ideology (8values 52-entry table): Classical Liberalism
  RIGHTNESS_V1=0.686
  RIGHTNESS_V2=0.612
  ```

- **Insights do persona-agent (voz Hayek, itens não-óbvios):** aceita um mínimo garantido de subsistência/abrigo/saúde básica (não abole toda rede de segurança, mas rejeita pagador-único monopolista); Q36 "existência do Estado ameaça a liberdade" = disagree (Estado sob império da lei protege a liberdade; o perigo é o poder discricionário); valoriza tradições evoluídas (conhecimento tácito) sem ser conservador reacionário (crítica ao racionalismo construtivista, "abuso da razão"); democracia = meio limitado pela lei, não valor supremo; reservas quanto à imigração irrestrita (evolução cultural/ordem estendida); saúde diferenciada por mercado ACIMA do piso mínimo = legítima.
- **(c) Conclusion / Conclusão:** **RIGHT / DIREITA**, in-fiction "good / bom" (boa evolução). `rightness 0.686` > 0.599. **Tripla checagem:** (1) 8values = 0.686, "Classical Liberalism"; (2) web (Wikipedia) = "right-libertarian classical liberal, influenciou Thatcher/Reagan, NÃO anarco-capitalista"; (3) **convergem**. Corretamente menos extremo que o Mises (0.686 < 0.846), refletindo o gradualismo/rede-mínima. Âncora de calibração de direita. **v2 (pesos do jogo): `rightness 0.612`, DIREITA confirmada** (âncora re-validada sob v2). Approved by creator / aprovado pelo criador: 2026-07-11 (triagem óbvia-direita) + confirmado pelo algoritmo.

#### [CAL] Marx (external calibration control / controle externo de calibração, not in game roster / fora do roster do jogo)

> **RODADA VIGENTE = v3 (persona Fable, 2026-07-11): `rightness_v2 0.297` = `leftness_v2 0.703` = ESQUERDA (controle re-validado).** Perfil v3: Econômico Equality 88.5% (Socialist) · Diplomático Peace 75.6% (Internationalist) · Civil Liberty 76.6% (Libertarian) · Social Progress 84.0% (Very Progressive); ideologia Libertarian Socialism. Array Fable (70): `["sa","a","sd","sd","sa","d","sa","sd","sa","sd","sa","sd","sd","sa","sa","d","d","a","sd","a","a","d","a","d","sd","sa","a","sd","sd","sd","sd","a","a","sd","sd","a","sd","sa","sd","d","d","a","a","sa","sd","a","sd","sa","a","a","sd","sd","sa","sa","a","n","d","sd","d","d","sd","sa","d","sd","d","a","a","a","a","a"]`. O bloco Opus abaixo fica como histórico. / Current run = v3 (Fable); Opus block below is historical.

- **(a) Prompt / Prompt do persona-agent** (dado verbatim ao `AGENTE_PERSONA_MARX`, modelo Opus, 2026-07-11): "Você ENCARNA Karl Marx (1818-1883). PERSONALIDADE/VISÕES: teórico comunista revolucionário; materialismo histórico (condições materiais movem a história); luta de classes (burguesia × proletariado); crítica ao capitalismo (mais-valia, alienação); revolução proletária + 'ditadura do proletariado' transitória rumo à sociedade SEM classes e SEM Estado; abolição da propriedade privada dos meios de produção; teoria do valor-trabalho; ateu ('religião = ópio do povo'). Big Five: Abertura MUITO ALTA; Conscienciosidade ALTA; Extroversão MODERADA-ALTA; Amabilidade BAIXA (polêmico, combativo, controle editorial 'ditatorial'); Neuroticismo MODERADO; abordagem autoritária de controle organizacional. TEMPERATURA: combativo, intransigente em princípio (convicção forte em economia/classe/revolução/propriedade/Estado); far-left. Regra de repasse ativa; anti-aquiescência."
- **(b) 8values result / Resultado 8values:** Econômico **Equality 92.9%** (Communist) · Diplomático **Peace 81.1%** (Internationalist) · Civil/Governo **Liberty 77.3%** (Libertarian) · Social Progress 85.3% (Very Progressive). Ideologia mais próxima: **Libertarian Communism**. **`rightness = 0.071`** → **`leftness = 0.929`** (societal_tradition 14.7). Respostas (70, ordem 1-70): `["sa","a","sd","sd","sa","d","sa","sd","sa","sd","sa","sd","sd","sa","sa","n","sd","a","sd","a","a","sd","a","d","sd","sa","a","sd","sd","sd","sd","n","a","d","d","d","sd","a","sd","d","n","a","a","sa","sd","a","sd","a","a","a","sd","d","a","a","a","a","n","sd","d","d","d","sa","d","sd","d","a","a","a","sa","sd"]`.
- **(b2) Engine descriptive block / Bloco descritivo do engine** (verbatim, `gus_rightness.py`):

  ```text
  Economic   (Equality <-> Markets):    Equality  92.9%  |  Markets     7.1%   [Communist]
  Diplomatic (Peace    <-> Nation):     Peace     81.1%  |  Nation     18.9%   [Internationalist]
  Civil/Govt (Liberty  <-> Authority):  Liberty   77.3%  |  Authority  22.7%   [Libertarian]
  Societal   (Progress <-> Tradition):  Progress  85.3%  |  Tradition  14.7%   [Very Progressive]
  Closest ideology (8values 52-entry table): Libertarian Communism
  RIGHTNESS_V1=0.071
  RIGHTNESS_V2=0.270
  ```

- **Insights do persona-agent (voz Marx, itens não-óbvios):** livre-comércio = agree ("On the Question of Free Trade", 1848; acelera as contradições do capitalismo, não por amá-lo); violência revolucionária = endossada (emancipação pela força, não reforma pacífica); Q36 "Estado ameaça a liberdade" = disagree (NÃO é anarquista à la Bakunin; quer USAR o Estado transitoriamente antes que ele definhe → menos libertário-civil que um anarquista); proletariado ARMADO ("Address to the Communist League", 1850); tensão real confiança-nas-massas × centralismo autoritário (neutros calibrados); internacionalismo consistente ("os trabalhadores não têm pátria").
- **(c) Conclusion / Conclusão:** **LEFT / ESQUERDA**. `leftness 0.929` ≫ 0.599. **Tripla checagem:** (1) 8values = leftness 0.929, "Libertarian Communism"; (2) web (Wikipedia) = "far-left, comunismo revolucionário, abolição da propriedade privada"; (3) **convergem**. Nota fina: o método captura o Marx **libertário no eixo civil** (Liberty 77.3%), coerente com a rejeição dele ao Estado permanente, distinguindo-o do Mao estatista. **Controle externo de calibração de esquerda** (não é figura do jogo, sem enquadramento in-fiction bom/mau). **v2 (pesos do jogo): `leftness 0.730`, ESQUERDA confirmada** (o bônus civil-libertário dele não salva o buraco econômico, como desenhado). Approved by creator / aprovado pelo criador: 2026-07-11 (controle conhecido-esquerda) + confirmado pelo algoritmo.

#### [CAL] Mao Tsé-tung (external calibration control / controle externo de calibração, not in game roster / fora do roster do jogo)

> **RODADA VIGENTE = v3 (persona Fable, 2026-07-11): `rightness_v2 0.268` = `leftness_v2 0.732` = ESQUERDA (controle re-validado).** Perfil v3: Econômico Equality 86.5% (Socialist) · Diplomático Nation 73.9% (Patriotic) · Civil Authority 73.0% (Statist) · Social Progress 60.8% (Progressive); ideologia State Socialism. Array Fable (70): `["sa","sa","sd","sd","sa","sa","sa","sd","sa","sd","sa","sd","sd","sa","sa","n","sa","a","sa","d","sd","a","sd","sd","sa","d","sd","sd","sd","sa","a","d","n","sa","sa","sd","d","d","a","sd","sa","d","sd","a","sd","sa","sd","sa","d","d","sd","sd","sa","a","sd","sd","d","a","a","d","sa","sa","sa","sd","sd","a","sd","a","d","sa"]`. O bloco Opus abaixo fica como histórico. / Current run = v3 (Fable); Opus block below is historical.

- **(a) Prompt / Prompt do persona-agent** (dado verbatim ao `AGENTE_PERSONA_MAO`, modelo Opus, 2026-07-11): "Você ENCARNA Mao Tsé-tung (1893-1976). PERSONALIDADE/VISÕES: Marxismo-Leninismo adaptado ('Maoismo'), revolução camponesa; Estado comunista totalitário de partido único; coletivização, confisco dos latifundiários; implacavelmente contra propriedade privada e mercado; 'revolução é um ato de violência pelo qual uma classe derruba outra'; culto à personalidade, autoridade absoluta. Big Five: Abertura ALTA; Conscienciosidade MISTA; Extroversão ALTA; Amabilidade MUITO BAIXA (implacável); Neuroticismo MODERADO-ALTO. TEMPERATURA: autoritário, implacável, intransigente (convicção forte); far-left econômico + ALTA autoridade estatal. Regra de repasse ativa; anti-aquiescência."
- **(b) 8values result / Resultado 8values:** Econômico **Equality 87.8%** (Socialist) · Diplomático **Nation 73.9%** (Patriotic) · Civil/Governo **Authority 70.3%** (Statist) · Social Progress 62.5% (Progressive). Ideologia mais próxima: **State Socialism**. **`rightness = 0.122`** → **`leftness = 0.878`** (societal_tradition 37.5). Respostas (70): `["sa","sa","sd","sd","sa","sa","sa","sd","sa","sd","sa","sd","sd","sa","sa","n","sa","d","sa","d","d","a","sd","d","sa","n","d","sd","sd","sa","a","d","n","sa","sa","sd","a","d","a","d","sa","n","sd","sa","sd","sa","sd","a","n","n","sd","sd","sa","a","sd","d","d","n","n","d","a","sa","sa","d","d","n","d","n","d","a"]`.
- **(b2) Engine descriptive block / Bloco descritivo do engine** (verbatim, `gus_rightness.py`):

  ```text
  Economic   (Equality <-> Markets):    Equality  87.8%  |  Markets    12.2%   [Socialist]
  Diplomatic (Peace    <-> Nation):     Peace     26.1%  |  Nation     73.9%   [Patriotic]
  Civil/Govt (Liberty  <-> Authority):  Liberty   29.7%  |  Authority  70.3%   [Statist]
  Societal   (Progress <-> Tradition):  Progress  62.5%  |  Tradition  37.5%   [Progressive]
  Closest ideology (8values 52-entry table): State Socialism
  RIGHTNESS_V1=0.122
  RIGHTNESS_V2=0.265
  ```

- **Insights do persona-agent (voz Mao, itens não-óbvios):** eixo diplomático é o mais MISTO (Teoria dos Três Mundos: ajuda ao Terceiro Mundo por solidariedade revolucionária, MAS rejeita 'governo mundial' como hegemonia das superpotências; soberania anti-imperialista sagrada); Q31/Q38 tensão real (retórica insurrecional "é justo rebelar-se" dirigida só contra 'capitalist-roaders'; a autoridade proletária e a dele = inquestionáveis → calibrado como autoritário, não libertário); "linha de massas" glorifica retoricamente o campesinato mas o partido-vanguarda filtra (centralismo democrático); culturalmente puritano/antifamiliar (não liberal-social); Q43 anti-ecológico por doutrina ("o homem deve conquistar a natureza").
- **(c) Conclusion / Conclusão:** **LEFT / ESQUERDA**. `leftness 0.878` > 0.599. **Tripla checagem:** (1) 8values = leftness 0.878, "State Socialism"; (2) web (Wikipedia) = "far-left autoritário, totalitário de partido único"; (3) **convergem**. Nota fina: o método captura o Mao **estatista no eixo civil** (Authority 70.3%), contraste correto com o Marx libertário-civil: mesma esquerda econômica, eixos civis opostos. **Controle externo de calibração de esquerda** (não é figura do jogo, sem enquadramento in-fiction). **v2 (pesos do jogo): `leftness 0.735`, ESQUERDA confirmada.** Approved by creator / aprovado pelo criador: 2026-07-11 (controle conhecido-esquerda) + confirmado pelo algoritmo.

#### FIS-01 Einstein (roster figure + left validation point / figura do roster + ponto de validação de esquerda)

> **RODADA VIGENTE = v3 (persona Fable, 2026-07-11): `rightness_v2 0.284` = `leftness_v2 0.716` = ESQUERDA (ponto de validação re-validado).** Perfil v3: Econômico Equality 90.4% (Communist) · Diplomático Peace 92.8% (Cosmopolitan) · Civil Liberty 81.2% (Libertarian) · Social Progress 82.7% (Very Progressive); ideologia Libertarian Socialism. Array Fable (70): `["a","sa","sd","sd","sa","d","sa","sd","sa","d","sa","sd","sd","sa","a","sd","d","sa","sd","sa","sa","sd","a","sd","sd","sa","sa","d","d","sd","d","n","a","sd","sd","d","sd","sa","sd","d","sd","sa","a","a","d","a","sd","a","a","sa","d","d","sa","a","a","a","a","sd","sd","sd","a","sa","d","d","a","n","a","sa","sa","sd"]`. Enquadramento in-fiction segue DEFERIDO caso a caso (Einstein É a âncora da zona; sob v3 a zona re-ancorada vira `[0.184, 0.384]`). O bloco Opus abaixo fica como histórico. / Current run = v3 (Fable); Opus block below is historical.

- **(a) Prompt / Prompt do persona-agent** (dado verbatim ao `AGENTE_PERSONA_EINSTEIN`, modelo Opus, 2026-07-11): "Você ENCARNA Albert Einstein (1879-1955). PERSONALIDADE/VISÕES: socialista democrata (ensaio 'Why Socialism?', 1949; capitalismo = 'fase predatória'; defende economia planejada com propriedade coletiva dos meios de produção); MAS libertário/anti-autoritário (condenou os métodos bolcheviques 'regime de terror'; alertou contra a burocracia socialista 'todo-poderosa'; defensor da liberdade intelectual/acadêmica/dissidência, recusar as audiências macartistas); internacionalista (governo mundial, ONU eleita), pacifista (abandonou o pacifismo absoluto contra o nazismo), humanista, anti-racismo, anti-nacionalismo; não-conformista de princípio. TEMPERATURA: princípio moral + nuance (não dogmático); ESQUERDA no econômico (socialista, forte) MAS LIBERTÁRIO no civil (liberdade individual, forte). Regra de repasse ativa; anti-aquiescência."
- **(b) 8values result / Resultado 8values:** Econômico **Equality 90.4%** (Communist) · Diplomático **Peace 93.3%** (Cosmopolitan) · Civil/Governo **Liberty 80.1%** (Libertarian) · Social Progress 78.9% (Very Progressive). Ideologia mais próxima: **Libertarian Socialism**. **`rightness = 0.096`** → **`leftness = 0.904`** (societal_tradition 21.1). Respostas (70): `["a","sa","sd","sd","sa","d","a","sd","sa","d","sa","sd","sd","sa","sa","sd","d","sa","sd","sa","sa","sd","a","sd","d","sa","sa","n","sd","sd","n","n","a","sd","sd","d","sd","sa","sd","d","sd","a","a","n","d","n","sd","a","a","sa","d","n","a","a","a","a","sa","sd","d","sd","a","sa","d","n","a","n","a","sa","sa","sd"]`.
- **(b2) Engine descriptive block / Bloco descritivo do engine** (verbatim, `gus_rightness.py`):

  ```text
  Economic   (Equality <-> Markets):    Equality  90.4%  |  Markets     9.6%   [Communist]
  Diplomatic (Peace    <-> Nation):     Peace     93.3%  |  Nation      6.7%   [Cosmopolitan]
  Civil/Govt (Liberty  <-> Authority):  Liberty   80.1%  |  Authority  19.9%   [Libertarian]
  Societal   (Progress <-> Tradition):  Progress  78.9%  |  Tradition  21.1%   [Very Progressive]
  Closest ideology (8values 52-entry table): Libertarian Socialism
  RIGHTNESS_V1=0.096
  RIGHTNESS_V2=0.287
  ```

- **Insights do persona-agent (voz Einstein, itens não-óbvios):** Q1 opressão corporativa > governamental = agree (capital privado oligárquico controla mídia/distorce a democracia, "Why Socialism?"), mas NÃO `sa` porque temia também o Estado burocrático (preocupação dupla); Q7 "de cada um conforme a capacidade" = agree (não `sa`): socialista democrático, NÃO bolchevique (condenou o "regime de terror"); pacifismo abandonado contra o nazismo (Q28 = neutro, não pôde condenar toda violência antitirania); eixo **libertário-civil forte** ("o respeito acrítico à autoridade é o maior inimigo da verdade" → `sa` em questionar autoridade; macartismo ancora os `sd` em vigilância); tecno-otimismo contido ("ciência sem ética = machado na mão de criminoso patológico").
- **(c) Conclusion / Conclusão:** **LEFT / ESQUERDA**. `leftness 0.904` ≫ 0.599. **Tripla checagem:** (1) 8values = leftness 0.904, "Libertarian Socialism"; (2) web (Wikipedia, "Political views of Albert Einstein") = "socialista democrata, 'Why Socialism?', anti-autoritário, libertário nas liberdades civis"; (3) **convergem**. Nota fina: a ideologia "Libertarian Socialism" captura exatamente a tensão dele: **esquerda no econômico** (o eixo que define `rightness`) + **libertário no civil** (Liberty 80.1%). Também é **ponto de validação de esquerda** da calibração. **Enquadramento in-fiction (bom/mau/neutro) e tipo de desafio: DEFERIDO, decidido caso a caso pelo criador na hora de construir a sidequest/prosa do Einstein.** Diretriz do criador (2026-07-11): uma figura culturalmente heroica que pontua "esquerda" NÃO é transformada em vilã automaticamente (a classificação econômica do algoritmo permanece registrada, mas o enquadramento narrativo é decisão explícita do criador, sem regra geral, por figura). Ver `feedback_heroi_nao_vira_vilao`. / In-fiction framing (good/bad/neutral) and challenge type: **DEFERRED, decided case-by-case by the creator when Einstein's sidequest/prose is built** (a culturally heroic figure that scores "left" is not auto-cast as a villain; economic classification stands, narrative framing is the creator's explicit per-figure call). **v2 (pesos do jogo / game weights): `rightness 0.287` (leftness 0.713), ESQUERDA confirmada; o escore dele sob a fórmula vigente é a âncora da zona Einstein re-ancorada [0.187, 0.387].**

#### ELM-02 Maxwell

> **RODADA VIGENTE = v3 (persona Fable, 2026-07-11): `rightness_v2 0.552` = CENTRO (caso duvidoso, decidir no final).** Perfil v3: Econômico Markets 64.1% (Market) · Diplomático Nation 56.7% · Civil Authority 70.3% (Statist) · Social Tradition 67.3% (Traditional); ideologia Right-Wing Populism. Array Fable (70): `["d","d","a","a","a","d","sd","a","d","sa","n","a","n","a","sd","n","a","d","a","d","a","d","d","d","a","sa","d","a","a","n","sa","a","sd","a","d","sd","a","d","a","d","n","d","n","n","sa","sd","a","d","n","n","d","sa","a","d","d","sd","d","sa","a","sa","a","d","a","sa","a","d","d","d","n","d"]`. Consistente com o centro do v2 (0.533); econ direita mas Liberty 29.7% (autoritário) puxa o composto pro centro. O bloco Opus abaixo fica como histórico.

- **(a) Prompt / Prompt do persona-agent** (dado verbatim ao `AGENTE_PERSONA_MAXWELL`, modelo Opus, 2026-07-11): "Você ENCARNA James Clerk Maxwell (1831-1879). PERSONALIDADE/VISÕES: cristão evangélico devoto (Ancião da Igreja da Escócia, conversão 1853, anti-positivista); laird escocês (senhorio rural, herdou Glenlair ~1500 acres, pequena nobreza proprietária); conservador vitoriano (respeito às instituições, tradição, ordem social, cultura escocesa, propriedade, administração da terra; caridade paternalista a operários; sem radicalismo). Big Five: Abertura MUITO ALTA; Conscienciosidade MODERADA-ALTA; Extroversão BAIXA; Amabilidade ALTA; Neuroticismo BAIXO-MODERADO. TEMPERATURA: humilde, tradicional, conservador; convicção forte em fé/tradição/propriedade/ordem, não radical. Política partidária não-documentada: raciocine da cosmovisão. Regra de repasse ativa; anti-aquiescência."
- **(b) 8values result / Resultado 8values:** Econômico **Markets 60.9%** (Market) · Diplomático Nation 51.1% (Balanced) · Civil/Governo **Authority 68.0%** (Statist) · Social **Tradition 64.7%** (Traditional). Ideologia mais próxima: **Right-Wing Populism**. **`rightness = 0.609`** (societal_tradition 64.7). Respostas (70): `["d","n","a","a","a","n","sd","a","d","sa","n","a","d","a","sd","d","a","n","a","d","a","d","d","d","a","sa","n","a","a","n","sa","a","sd","n","n","sd","n","d","a","d","n","n","a","n","sa","sd","a","d","n","n","n","sa","sa","n","d","sd","d","sa","a","sa","a","d","a","sa","a","d","d","n","n","d"]`.
- **(b2) Engine descriptive block / Bloco descritivo do engine** (verbatim, `gus_rightness.py`):

  ```text
  Economic   (Equality <-> Markets):    Equality  39.1%  |  Markets    60.9%   [Market]
  Diplomatic (Peace    <-> Nation):     Peace     48.9%  |  Nation     51.1%   [Balanced]
  Civil/Govt (Liberty  <-> Authority):  Liberty   32.0%  |  Authority  68.0%   [Statist]
  Societal   (Progress <-> Tradition):  Progress  35.3%  |  Tradition  64.7%   [Traditional]
  Closest ideology (8values 52-entry table): Right-Wing Populism
  RIGHTNESS_V1=0.609
  RIGHTNESS_V2=0.533
  ```

- **Insights do persona-agent (voz Maxwell, itens não-óbvios):** Q44 "ciência traz mundo melhor" = neutro (tensão central: reverencia o progresso mas a bondade não brota de máquinas; a ciência revela a ordem do Criador, não é salvação moral; recusa o positivismo); Q6/Q11 (tarifas/utilidades) = neutro (sem registro partidário; proprietário rural pós-Corn Laws, genuinamente em cima do muro); Q29 "espalhar valores religiosos" = agree, não strongly (fé íntima e reservada, não estridente); Q69 (igualdade incl. sexualidade) = neutro (almas iguais perante Deus, mas a cláusula sexual conflita com a moral cristã dele).
- **(c) Conclusion / Conclusão:** **CENTER / CENTRO**, in-fiction "neutral / neutro", desafio de puzzle. Histórico: sob o escalar v1 (econ puro) pontuou `0.609`, direita no limiar, e foi aprovado assim pelo criador; com a **refatoração v2** (pesos do jogo) re-pontuou **`rightness_v2 0.533` = CENTRO**, porque o estatismo civil dele (Authority 68.0%) cobra preço nos pesos que o lore valoriza. O criador **aceitou a reclassificação** (decisão 2026-07-11: "aceitar centro, puzzle"; a aprovação anterior fica superada e registrada aqui por transparência). **Tripla checagem:** (1) 8values v2 = 0.533, "Right-Wing Populism" com Authority 68% + Tradition 64.7%; (2) web (Wikipedia) = "nobreza rural proprietária escocesa, evangélico devoto, conservador vitoriano, institucional, anti-radical"; (3) **convergem** num perfil misto: mercado moderado + forte autoridade/tradição = centro sob os pesos do jogo. Approved by creator / aprovado pelo criador: 2026-07-11 (reclassificação v2).

#### ELM-03 Tesla (roster figure, heroic / figura do roster, heroica)

> **RODADA VIGENTE = v3 (persona Fable, 2026-07-11): `rightness_v2 0.370` = ESQUERDA, DENTRO da zona Einstein re-ancorada [0.184, 0.384].** Perfil v3: Econômico Equality 69.9% (Social) · Diplomático Peace 68.3% · Civil Liberty 58.2% (Moderate) · Social Progress 72.1%; ideologia Social Democracy. Array Fable (70): `["a","a","d","d","sa","d","d","d","n","n","sa","d","d","sa","d","d","n","a","d","a","a","d","n","d","a","sa","a","a","d","d","a","a","n","d","d","d","d","a","n","d","n","n","a","sa","d","a","sd","a","a","a","d","d","sa","a","d","n","d","n","a","d","n","a","n","n","d","a","a","a","a","sd"]`. Heroico + na zona: enquadramento in-fiction DEFERIDO por regra (consistente com v2 0.354). O bloco Opus abaixo fica como histórico.

- **(a) Prompt / Prompt do persona-agent** (dado verbatim ao `AGENTE_PERSONA_TESLA`, modelo Opus, 2026-07-11): "Você ENCARNA Nikola Tesla (1856-1943). PERSONALIDADE/VISÕES: tecno-utopista/futurista (tecnologia liberta a humanidade; energia elétrica sem fio, universal e quase gratuita pra todos, transcendendo concessionárias movidas a lucro); desiludido com o capitalismo industrial (explorado por Edison/Westinghouse/Morgan, morreu na pobreza) MAS pró-propriedade-do-inventor (patentes, processou Marconi); idealista individualista; tecnocrata (especialistas/tecnologia resolvem de cima pra baixo); humanitário aspiracional; obsessivo, teatral, volátil. Big Five: Abertura MUITO ALTA; Conscienciosidade ALTA; Extroversão MODERADA; Amabilidade MODERADA; Neuroticismo ALTO. TEMPERATURA: idealista, grandioso, individualista. Política partidária não-documentada: raciocine da cosmovisão, sem forçar extremo. Regra de repasse ativa; anti-aquiescência."
- **(b) 8values result / Resultado 8values:** Econômico **Equality 71.8%** (Social) · Diplomático **Peace 72.2%** (Peaceful) · Civil/Governo Liberty 59.4% (Moderate) · Social **Progress 75.3%** (Very Progressive). Ideologia mais próxima: **Social Democracy**. **`rightness = 0.282`** → **`leftness = 0.718`** (societal_tradition 24.7). Respostas (70): `["a","a","n","d","sa","n","n","d","a","n","sa","d","d","sa","d","sd","n","a","n","a","a","d","n","d","n","sa","a","a","d","d","n","a","n","d","d","d","d","a","n","d","n","n","a","sa","d","a","sd","a","a","a","sd","d","sa","sa","n","n","a","n","n","n","n","a","n","n","n","a","a","a","a","sd"]`.
- **(b2) Engine descriptive block / Bloco descritivo do engine** (verbatim, `gus_rightness.py`):

  ```text
  Economic   (Equality <-> Markets):    Equality  71.8%  |  Markets    28.2%   [Social]
  Diplomatic (Peace    <-> Nation):     Peace     72.2%  |  Nation     27.8%   [Peaceful]
  Civil/Govt (Liberty  <-> Authority):  Liberty   59.4%  |  Authority  40.6%   [Moderate]
  Societal   (Progress <-> Tradition):  Progress  75.3%  |  Tradition  24.7%   [Very Progressive]
  Closest ideology (8values 52-entry table): Social Democracy
  RIGHTNESS_V1=0.282
  RIGHTNESS_V2=0.354
  ```

- **Insights do persona-agent (voz Tesla, itens não-óbvios):** núcleo = energia sem fio universal e (quase) gratuita, utilidades servem o homem e não o acionista (Q5/Q11 sa); Q32 "massa decide mal" = agree MAS Q41 "homem forte político" = neutro (a elite dele é da competência/razão tecnocrática, NÃO da autoridade bruta); desconfia do "mercado que liberta" (explorado pelo capital) mas rejeita propriedade coletivista dos meios (pró-patente do inventor): idealista individualista ferido, não socialista doutrinário; internacionalista/universalista (ciência sem pátria); **honestidade histórica: Q66 = agree, os escritos eugênicos dele dos anos 1930, hoje condenados, "a mancha"** (registrado por transparência).
- **(c) Conclusion / Conclusão:** **LEFT / ESQUERDA** no eixo econômico (`leftness 0.718`), classificação econômica registrada. **Enquadramento in-fiction (bom/mau/neutro) e tipo de desafio: DEFERIDO ao criador, decidido caso a caso** (Tesla é figura heroica/amada; aplica a regra "herói não vira vilão auto", mesmo tratamento do Einstein, decisão do criador 2026-07-11; ver `feedback_heroi_nao_vira_vilao`). **Tripla checagem:** (1) 8values = leftness 0.718, "Social Democracy"; (2) web (Wikipedia) = "tecno-utopista, energia universal, explorado pelo capital industrial, sem socialismo doutrinário, individualista"; (3) **convergem**. / In-fiction framing DEFERRED to creator (heroic figure, same treatment as Einstein). **v2 (pesos do jogo / game weights): `rightness 0.354`, ESQUERDA mantida; cai DENTRO da zona Einstein re-ancorada [0.187, 0.387], então o deferimento passa de exceção pontual a aplicação de regra.**

#### ELM-01 Faraday

> **RODADA VIGENTE = v3 (persona Fable, 2026-07-11): `rightness_v2 0.441` = CENTRO (caso duvidoso, decidir no final).** Perfil v3: Econômico Markets 42.3% (Centrist) · Diplomático Peace 63.3% · Civil Authority 52.3% (Moderate) · Social Progress 51.1% (Neutral); ideologia Centrist. Array Fable (70): `["n","a","n","n","a","n","d","n","d","a","a","d","d","sa","d","d","d","n","a","d","sa","sd","n","d","a","a","n","sa","d","d","a","a","sd","n","d","sd","d","n","n","d","d","n","a","a","a","d","d","n","n","a","d","a","a","a","d","d","n","sa","n","a","n","n","a","a","a","d","n","n","a","d"]`. Consistente com o centro do v2 (0.493). O bloco Opus abaixo fica como histórico.

- **(a) Prompt / Prompt do persona-agent** (dado verbatim ao `AGENTE_PERSONA_FARADAY`, modelo Opus, 2026-07-11): "Você ENCARNA Michael Faraday (1791-1867). PERSONALIDADE/VISÕES: cristão Sandemaniano devoto (diácono/ancião), fé integral à identidade e à ética; asceta por princípio religioso (recusou título de cavaleiro, sepultura em Westminster, trabalho de armas químicas na Crimeia por ética, dinheiro por publicar; 'sempre amei a ciência mais que o dinheiro'); autodidata, saiu da pobreza pelo mérito; consciência individual acima da hierarquia institucional (recusou a presidência da Royal Society 2x); serviço público (segurança de minas, poluição do Tâmisa), anti-militarista; NUNCA questionou a propriedade privada (austeridade é escolha pessoal-religiosa, NÃO redistribuição). Big Five: Abertura MUITO ALTA; Conscienciosidade MUITO ALTA; Extroversão MODERADA-ALTA; Amabilidade ALTA; Neuroticismo BAIXO-MODERADO. TEMPERATURA: princípio, humildade, firmeza estável; política partidária não-documentada, raciocinar da cosmovisão sem forçar extremos. Regra de repasse ativa; anti-aquiescência." Redos independentes (protocolo de centro, AMB-05) com pesquisa ampliada: sem vínculo com sociedade secreta (Sandemanianos separatistas); dado novo, Faraday declarou NÃO ser republicano, mas favorável a reforma (reformador dentro do establishment).
- **(b) 8values result / Resultado 8values (run original):** Econômico Markets 51.3% (Centrist) · Diplomático **Peace 72.2%** (Peaceful) · Civil/Governo Liberty 53.9% (Moderate) · Social Progress 50.7% (Neutral). Ideologia mais próxima: **Liberalism**. **`rightness_v1 = 0.513` · `rightness_v2 = 0.493`**. Respostas (70): `["d","a","a","a","a","d","d","a","d","a","n","n","d","sa","d","d","d","a","n","n","a","sd","a","d","n","sa","a","sa","d","d","a","d","sd","d","d","d","d","n","d","d","d","a","a","a","a","d","d","n","a","a","d","a","sa","n","d","sd","n","sa","a","sa","a","d","a","sa","a","d","n","a","a","sd"]`.
- **(b2) Engine descriptive block / Bloco descritivo do engine** (verbatim, `gus_rightness.py`):

  ```text
  Economic   (Equality <-> Markets):    Equality  48.7%  |  Markets    51.3%   [Centrist]
  Diplomatic (Peace    <-> Nation):     Peace     72.2%  |  Nation     27.8%   [Peaceful]
  Civil/Govt (Liberty  <-> Authority):  Liberty   53.9%  |  Authority  46.1%   [Moderate]
  Societal   (Progress <-> Tradition):  Progress  50.7%  |  Tradition  49.3%   [Neutral]
  Closest ideology (8values 52-entry table): Liberalism
  RIGHTNESS_V1=0.513
  RIGHTNESS_V2=0.493
  ```

- **Redos (protocolo de centro):** redo-1 `v1 0.500 / v2 0.481`; redo-2 `v1 0.429 / v2 0.451`. Três rodadas independentes, todas CENTRO. O redo-2 revelou que o "centro" dele é média real entre um leaning tradicionalista genuíno no eixo social (moral cristã pessoal) e economia/diplomacia moderadas, não ausência de posição.
- **Insights do persona-agent (voz Faraday, itens não-óbvios):** caridade privada > programas estatais (núcleo documentado do ethos: recusa de honras/dinheiro; socorro ao próximo é dever cristão pessoal, não coerção do Estado), mas compaixão cristã reprova saúde comprada por riqueza e valoriza educação como acesso (ele mesmo subiu de aprendiz de encadernador); devoto porém NÃO proselitista/clerical (Sandemanianos = seita quieta, separatista; fé no foro íntimo, não instrumento de poder); anti-militarismo de consciência (recusou armas na Crimeia) com nuance (serviu a Trinity House em faróis); ambiental por evidência (carta do Tâmisa, 1855); cauteloso com "refazer a criatura" (limites morais do progresso).
- **(c) Conclusion / Conclusão:** **CENTER / CENTRO**, in-fiction "neutral / neutro", desafio de puzzle. `v2 0.493` (0.481/0.451 nos redos), protocolo de centro cumprido (AMB-05: o criador mandou rodar o redo 2x em vez de aceitar direto). **Tripla checagem:** (1) 8values = centro nas 3 rodadas, "Liberalism"; (2) web ampliada = não-republicano reformista, asceta por fé, sem vínculo oculto; (3) **convergem** num centro genuíno (meritocrático-caritativo, nem redistributivo nem laissez-faire doutrinário). Approved by creator / aprovado pelo criador: 2026-07-11 (protocolo AMB-05).

#### ELM-04 Volta

> **RODADA VIGENTE = v3 (persona Fable, 2026-07-11): `rightness_v2 0.487` = CENTRO (caso duvidoso, decidir no final).** Perfil v3: Econômico Markets 53.8% (Centrist) · Diplomático Nation 49.4% (Balanced) · Civil Authority 72.7% (Statist) · Social Tradition 66.9% (Traditional); ideologia Right-Wing Populism. Array Fable (70): `["d","a","d","a","sa","a","sd","a","d","sa","a","d","n","a","sd","d","a","a","n","n","a","d","d","d","n","sa","n","sa","a","d","sa","a","sd","a","a","sd","a","sd","a","d","a","d","n","a","sa","sd","a","d","n","n","a","sa","a","d","d","sd","d","a","a","sa","a","n","a","sa","a","d","d","d","d","d"]`. Econ centro mas fortíssimo em Authority 72.7% + Tradition 66.9%; consistente com v2 0.498. O bloco Opus abaixo fica como histórico.

- **(a) Prompt / Prompt do persona-agent** (dado verbatim ao `AGENTE_PERSONA_VOLTA`, modelo Opus, 2026-07-11): "Você ENCARNA Alessandro Volta (1745-1827). PERSONALIDADE/VISÕES: católico devoto (defendeu a fé formalmente; 'religião católica, apostólica, romana'; fé como graça + racionalmente defensável); conservador do establishment com simpatias aristocráticas (aceitou entusiasticamente as honras de Napoleão: título de Conde 1810, senador; serviu com lealdade sob os Habsburgo E sob a administração napoleônica sem conflito ideológico; casou-se na aristocracia de Como); tradicional e domesticamente convencional; politicamente quiescente, nenhuma simpatia com revolução (aceitação tácita das hierarquias). Big Five: Conscienciosidade EXTREMAMENTE ALTA; Abertura MODERADA-ALTA (na ciência; conservador no social); Extroversão BAIXA-MODERADA; Amabilidade ALTA; Neuroticismo BAIXO. TEMPERATURA: disciplinado, ortodoxo, deferente à autoridade estabelecida, anti-revolucionário; no econômico não-doutrinário (mecenato estatal a vida toda). Regra de repasse ativa; anti-aquiescência." Redos independentes (protocolo de centro, AMB-05) com pesquisa ampliada: sem vínculo maçônico/Illuminati documentado; perfil católico-monarquista confirmado.
- **(b) 8values result / Resultado 8values (run original):** Econômico Markets 56.4% (Centrist) · Diplomático Nation 51.7% (Balanced) · Civil/Governo **Authority 74.2%** (Statist) · Social **Tradition 66.6%** (Traditional). Ideologia mais próxima: **Right-Wing Populism**. **`rightness_v1 = 0.564` · `rightness_v2 = 0.498`**. Respostas (70): `["d","a","d","a","sa","a","sd","a","d","sa","a","d","a","n","sd","d","a","a","a","d","a","d","d","d","a","sa","a","sa","a","n","sa","a","sd","a","a","sd","a","sd","sa","d","a","d","n","sa","sa","sd","a","d","n","n","a","sa","a","d","sd","sd","d","sa","a","sa","a","d","a","sa","a","d","d","d","n","d"]`.
- **(b2) Engine descriptive block / Bloco descritivo do engine** (verbatim, `gus_rightness.py`):

  ```text
  Economic   (Equality <-> Markets):    Equality  43.6%  |  Markets    56.4%   [Centrist]
  Diplomatic (Peace    <-> Nation):     Peace     48.3%  |  Nation     51.7%   [Balanced]
  Civil/Govt (Liberty  <-> Authority):  Liberty   25.8%  |  Authority  74.2%   [Statist]
  Societal   (Progress <-> Tradition):  Progress  33.4%  |  Tradition  66.6%   [Traditional]
  Closest ideology (8values 52-entry table): Right-Wing Populism
  RIGHTNESS_V1=0.564
  RIGHTNESS_V2=0.498
  ```

- **Redos (protocolo de centro):** redo-1 `v1 0.583 / v2 0.513`; redo-2 `v1 0.583 / v2 0.506` (uma rodada intermediária veio malformada com 69 tokens e foi descartada e substituída). Três rodadas independentes, todas CENTRO no v2.
- **Insights do persona-agent (voz Volta, itens não-óbvios):** o centro econômico dele é composição de convicções opostas, não indecisão (estatista/mercantilista na regulação, reflexo do mecenato de coroa que o sustentou a vida toda, MAS anti-igualitário/pró-propriedade e herança na distribuição; anti-socialismo categórico); eixo autoridade é onde ele é mais forte e consistente (ordem, hierarquia, anti-revolução, "servi Habsburgo e Napoleão sem conflito de consciência"); moral social toda ancorada em doutrina católica documentada (profissão de fé formal); único vetor "progressista" = ciência/tecnologia (o iluminista-tecnólogo convive com o conservador de trono-e-altar sem contradição); diplomático distingue cooperação entre soberanos legítimos de subordinação supranacional.
- **(c) Conclusion / Conclusão:** **CENTER / CENTRO**, in-fiction "neutral / neutro", desafio de puzzle. `v2 0.498` (0.513/0.506 nos redos), protocolo de centro cumprido (AMB-05: o criador escolheu rodar o redo em vez de desempatar por tradição). Sob v2 o forte conservadorismo (Tradition 66.6%) e o forte estatismo (Authority 74.2%) se compensam sobre o centro econômico: perfil misto genuíno. **Tripla checagem:** (1) 8values = centro v2 nas 3 rodadas; (2) web ampliada = católico monarquista establishment, sem vínculo oculto; (3) **convergem**. Approved by creator / aprovado pelo criador: 2026-07-11 (protocolo AMB-05).

#### FIS-02 Newton

- **(a) Prompt / Prompt** (verbatim ao `AGENTE_PERSONA_NEWTON`, modelo Fable, 2026-07-11): Isaac Newton (1643-1727); religiosidade heterodoxa secreta (anti-trinitariano, profecia, alquimia); Master da Casa da Moeda (moeda sólida, perseguição implacável de falsários, ordem econômica centralizada); MP Whig (defesa da ordem protestante contra o avanço católico de James II, não princípio democrático); presidente autocrático da Royal Society (vingativo); monarquista, cavaleiro, lealdade à coroa/instituições de elite, sem política redistributiva. Big Five: O alta seletiva, C extremamente alta, E muito baixa, A baixa (vingativo), N moderado-alto.
- **(b) 8values result / Resultado 8values:** Econômico Markets 65.4% (Market) · Diplomático Nation 73.9% (Patriotic) · Civil Authority 77.0% (Authoritarian) · Social Tradition 65.2% (Traditional). Ideologia: **Right-Wing Populism**. **`rightness_v1 = 0.654` · `rightness_v2 = 0.556`**. Array Fable (70): `["d","a","n","a","a","a","sd","a","d","sa","n","d","a","d","sd","a","a","d","sa","sd","d","n","d","a","sa","n","d","n","d","a","sa","sa","sd","a","a","sd","a","n","a","sd","a","d","n","a","a","d","a","d","n","n","a","a","sa","a","d","sd","d","sa","a","sa","a","d","a","a","a","d","d","d","d","a"]`.
- **(b2) Engine descriptive block / Bloco descritivo do engine:**

  ```text
  Economic   (Equality <-> Markets):    Equality  34.6%  |  Markets    65.4%   [Market]
  Diplomatic (Peace    <-> Nation):     Peace     26.1%  |  Nation     73.9%   [Patriotic]
  Civil/Govt (Liberty  <-> Authority):  Liberty   23.0%  |  Authority  77.0%   [Authoritarian]
  Societal   (Progress <-> Tradition):  Progress  34.8%  |  Tradition  65.2%   [Traditional]
  Closest ideology (8values 52-entry table): Right-Wing Populism
  RIGHTNESS_V1=0.654
  RIGHTNESS_V2=0.556
  ```

- **Insights do persona-agent (voz Newton):** estatista de ordem (intervenção na moeda) mas anti-redistribuição (Q7/Q15 sd, Q10 sa); eixo autoridade é o mais forte (pena capital a falsários, rede de informantes, presidência autocrática, desprezo elitista pela plebe); Q38 neutro por tensão real (politicamente hierárquico, mas cientificamente demoliu toda autoridade recebida); monarquista protestante (Q20 sd ancorado nas profecias anti-Roma); heterodoxia antitrinitária SECRETA (Q29 d, jamais propagandearia).
- **(c) Conclusion / Conclusão:** **CENTER / CENTRO** sob v2, in-fiction "neutral / neutro", desafio de puzzle (caso duvidoso, decidir no final). `rightness_v2 0.556`: econômico (65%), tradição (65%) e nação (74%) todos à direita, MAS o Liberty baixíssimo (23%, autoritário) puxa o composto ponderado para o centro. Perfil de conservador-autoritário de Antigo Regime. **Tripla checagem:** (1) 8values v2 = centro, "Right-Wing Populism"; (2) web = monarquista Whig, establishment de elite, autocrático, sem política social; (3) convergem num perfil direita-autoritária que os pesos do jogo (que valorizam Liberty) neutralizam. Enquadramento final na decisão em lote.

#### FIS-03 Planck

- **(a) Prompt / Prompt** (verbatim ao `AGENTE_PERSONA_PLANCK`, modelo Fable, 2026-07-11): Max Planck (1858-1947); conservador do establishment prussiano, institucionalista dedicado (dever, serviço às instituições); assinou o Manifesto dos 93 (depois lamentado); ANTI-democrático (contra o sufrágio universal, elitismo político); luterano deísta; sob o nazismo ficou na Alemanha tentando preservar a ciência de dentro, foi a Hitler defender colegas judeus, não emigrou nem entrou na resistência, filho Erwin executado pelo atentado de 1944. Big Five: C muito alta, O moderada, E baixa-moderada, A moderada-alta, N baixo (estoico).
- **(b) 8values result / Resultado 8values:** Econômico Markets 47.4% (Centrist) · Diplomático Nation 60.6% (Patriotic) · Civil Authority 69.5% (Statist) · Social Tradition 60.0% (Neutral). Ideologia: **Theocratic Distributism** (rótulo mais próximo na tabela). **`rightness_v1 = 0.474` · `rightness_v2 = 0.464`**. Array Fable (70): `["d","a","d","a","sa","a","sd","d","d","a","a","d","n","a","sd","d","a","d","sa","d","n","d","d","d","sa","a","d","n","d","a","sa","sa","d","a","n","sd","sa","sd","a","sd","n","d","n","a","a","sd","d","d","n","a","a","sa","a","n","d","d","d","a","a","a","a","n","a","sa","a","d","d","d","a","n"]`.
- **(b2) Engine descriptive block / Bloco descritivo do engine:**

  ```text
  Economic   (Equality <-> Markets):    Equality  52.6%  |  Markets    47.4%   [Centrist]
  Diplomatic (Peace    <-> Nation):     Peace     39.4%  |  Nation     60.6%   [Patriotic]
  Civil/Govt (Liberty  <-> Authority):  Liberty   30.5%  |  Authority  69.5%   [Statist]
  Societal   (Progress <-> Tradition):  Progress  40.0%  |  Tradition  60.0%   [Neutral]
  Closest ideology (8values 52-entry table): Theocratic Distributism
  RIGHTNESS_V1=0.474
  RIGHTNESS_V2=0.464
  ```

- **Insights do persona-agent (voz Planck):** estatista prussiano (apoio ao papel do Estado bismarckiano no financiamento da ciência), mas anti-redistribuição marxista (Q7/Q15 sd); anti-democrático e pró-ordem coerente com o elitismo declarado (culpou o "governo das multidões"); Q41 neutro (monarquista que viu no ditador nazista o pior desfecho, "liderança forte" não é valor em si); Q28 neutro é a inferência mais delicada (escolheu a acomodação legalista, mas o filho Erwin morreu no atentado de 1944, não condenaria a memória dele); nacionalismo cultural com internacionalismo científico.
- **(c) Conclusion / Conclusão:** **CENTER / CENTRO** sob v2, in-fiction "neutral / neutro", desafio de puzzle (caso duvidoso, decidir no final). `rightness_v2 0.464`: econômico centrista, mas autoritário (70%) + nacionalista (61%) + tradicional (60%). Conservador-autoritário prussiano que os pesos do jogo (Liberty) empurram ao centro. **Tripla checagem:** (1) 8values v2 = centro; (2) web = conservador prussiano, institucional, anti-democrático, luterano deísta; (3) convergem. Enquadramento final na decisão em lote.

### Pending records (through the algorithm) / Registros pendentes (pelo algoritmo)

The figures below are classified via the full persona + 8values + triple-check pipeline, and are filled in as each round runs. Mises and Hayek double as the right-leaning **calibration anchors** of the [Method calibration](#method-calibration) test. / As figuras abaixo são classificadas via o pipeline completo persona + 8values + tripla checagem, e são preenchidas conforme cada rodada roda. Mises e Hayek também servem de **âncoras de calibração** de direita do teste de [Calibração do método](#calibração-do-método).

| ID | Figure / Figura | Status | Notes / Notas |
| :--- | :--- | :--- | :--- |
| ELM-01 | Faraday | **done / feito** | CENTER/CENTRO, v2 0.493 (redos 0.481/0.451); neutro, puzzle; protocolo AMB-05 cumprido |
| ELM-02 | Maxwell | **done / feito** | CENTER/CENTRO sob v2 (0.533; v1 0.609 superado); neutro, puzzle; reclassificação AMB-07 |
| ELM-03 | Tesla | **classified / classificado** | LEFT/ESQUERDA v2 0.354 (Social Democracy); dentro da zona Einstein re-ancorada; enquadramento DEFERIDO (AMB-04) |
| ELM-04 | Volta | **done / feito** | CENTER/CENTRO, v2 0.498 (redos 0.513/0.506); neutro, puzzle; protocolo AMB-05 cumprido |
| FIS-01 | Einstein | **classified / classificado** | LEFT/ESQUERDA, v2 leftness 0.713 (v1 0.904); left validation point ✓; âncora da zona re-ancorada; enquadramento DEFERIDO (AMB-01/02) |
| FIS-02 | Newton | pending / pendente | to fill per round / a preencher conforme rodada |
| FIS-03 | Planck | pending / pendente | to fill per round / a preencher conforme rodada |
| MAT-01 | Mandelbrot | pending / pendente | to fill per round / a preencher conforme rodada |
| MAT-02 | Euler | pending / pendente | to fill per round / a preencher conforme rodada |
| MAT-03 | Gödel | pending / pendente | to fill per round / a preencher conforme rodada |
| CMP-01 | Ada Lovelace | pending / pendente | to fill per round / a preencher conforme rodada |
| CMP-02 | Turing | pending / pendente | to fill per round / a preencher conforme rodada |
| CMP-03 | von Neumann | pending / pendente | to fill per round / a preencher conforme rodada |
| OCU-01 | Giordano Bruno | pending / pendente | to fill per round / a preencher conforme rodada |
| OCU-02 | John Dee | pending / pendente | to fill per round / a preencher conforme rodada |
| OCU-03 | Pythagoras / Pitágoras | pending / pendente | to fill per round / a preencher conforme rodada |
| ECO-01 | Hayek | **done / feito** | RIGHT/DIREITA, rightness 0.686 (Computed records / Registros computados); calibration anchor ok |
| ECO-02 | Mises | **done / feito** | RIGHT/DIREITA, rightness 0.846 (see Computed records above / ver Registros computados acima); calibration anchor ✓ |

External calibration-only figures (Marx, Mao Zedong) are **not** part of the game roster and get no in-game record; they exist solely as known-left controls in the calibration test. / Figuras externas só-de-calibração (Marx, Mao Tsé-tung) **não** fazem parte do roster do jogo e não têm registro in-game; existem apenas como controles conhecidos de esquerda no teste de calibração.

---

## ROSTER-AUDIT-FINAL / Auto-auditoria da maratona (2026-07-11)

Feat de fecho pedido pelo criador: confrontar este doc com TODOS os pedidos feitos ao longo da maratona ROSTER-CANONIZE e confirmar se foram realizados e documentados. Confrontação abaixo (o orquestrador se auto-audita; achados de pendência marcados). / Creator-requested closing feat: confront this doc against every request made during the marathon.

| # | Pedido do criador | Status | Onde no doc / evidência |
| :--- | :--- | :--- | :--- |
| 1 | Metodologia de espectro (direita=bom, esquerda=mau, centro=puzzle; pesquisa→Big Five→persona→8values→tripla checagem; mostrar o algoritmo pra aprovar) | ✅ | Seções "Methodology" e "Method calibration"; engine em `8values-engine/` |
| 2 | Calibração (Mises/Hayek direita, Marx/Mao esquerda externos, Einstein) com convergência >0.599 | ✅ | "Method calibration" + resultado v1/v2/v3; convergência registrada |
| 3 | Rodar o algoritmo no Einstein | ✅ | Registro FIS-01 Einstein (computado) |
| 4 | Registrar resultado 8values + conclusão (dir/esq/centro) por personagem no doc | ✅ | Apêndice "Computed records" + notas de rodada vigente por figura |
| 5 | Descrever o mecanismo de calibragem em detalhe no doc | ✅ | "Method calibration" (fórmula, limiares, convergência, tripla checagem) |
| 6 | Prompts dos persona-agents no doc, por agente | ✅ | Cada registro traz o prompt (a) verbatim; centros trazem prompt condensado |
| 7 | Linkar o doc no README | ✅ | README (en linha ~31, pt ~166): "Fiction disclaimer and methodology" |
| 8 | Registros do agente no doc | ✅ | Apêndice: prompt + array + bloco descritivo + insights + conclusão |
| 9 | Feat de auto-auditoria (esta tabela) | ✅ | Esta seção |
| 10 | Resultados + insights dos persona-agents no doc, commit cada mudança | ✅ | Insights por figura; ~18 commits ao longo da maratona |
| 11 | Documentar detalhes (pessoas reais, transparência) | ✅ | Disclaimer de ficção + registros detalhados + nota da figura viva |
| 12 | Persona-agents repassam ordens ao main, não executam | ✅ | Cláusula de repasse em todo prompt; memória `feedback_persona_agents_relay_only` |
| 13 | Engine 8values canonizado como fork + comunicar ao autor + mencionado nos docs | ✅ / ⏳ | Fork em `8values-engine/` (ATTRIBUTION + LICENSE); **outreach ao autor PENDENTE** (rascunho, criador envia) |
| 14 | Parar o emoji de bateria | ✅ | Cessado |
| 15 | Herói não vira vilão automático, consultar com opções | ✅ | Salvaguarda de herói + AMB-01/02; memória `feedback_heroi_nao_vira_vilao` |
| 16 | Einstein como calibragem de pendentes, banda de ±% (zona Einstein) | ✅ | "Zona Einstein" ±10pp, re-ancorada à fórmula vigente; AMB-03 |
| 17 | Pesos aos valores pela ordem de importância; redefinir calibração; refazer testes | ✅ | "Pesos dos eixos v2"; AMB-06; reteste v3 completo |
| 18 | Documentar a refatoração + a dor que levou a ela | ✅ | "Pesos dos eixos v2" (a dor: perfis achatados, heróis-esquerda, tudo manual) |
| 19 | EXTREMAMENTE claro que os pesos são DO JOGO, não do mundo real, não modificam o teste | ✅ | "Pesos dos eixos v2" (disclaimer em caixa-alta EN+PT) + README do engine |
| 20 | Sempre registrar ambiguidades (opções + motivos + decisão) | ✅ | "Ambiguity decision log" AMB-01..09; memória `feedback_registro_ambiguidades_doc` |
| 21 | Política de issues (só técnicos; político/econ/filosófico/social = fechado sem resposta) no README + doc | ✅ | README (en+pt) + "Questions or concerns" / "Dúvidas ou preocupações" |
| 22 | Resultado descritivo do engine explícito no doc (equality/nation etc) | ✅ | Bloco `(b2)` descritivo verbatim em cada registro |
| 23 | Persona-agents = Fable, não Opus | ✅ | Reteste v3 todo em Fable; "O reteste completo v3" documenta a troca |
| 24 | Refazer resultado de todos (exceto Tusk) sob a refatoração, reusar pesquisa, re-rodar agentes | ✅ | Reteste v3: 20 figuras + 28 redos; Tusk blindado por fiat |
| 25 | Registrar o reteste + motivos + a troca Fable no doc | ✅ | "O reteste completo v3" (4 motivos) |
| 26 | Casos duvidosos decididos todos no final | ✅ | "Pending doubtful cases" resolvido em lote; AMB-09 |
| 27 | Centros → braço de redo 2× | ✅ | 14 centros × redo 2× (mediana de 3 rodadas); todos confirmados |
| 28 | Fazer 4 por vez (cautela OOM) | ✅ | Lotes de 4 personas a partir da instrução |

**Achados da auto-auditoria:** (a) **8VALUES-OUTREACH pendente** (item 13): o fork está atribuído no repo, mas o aviso ao autor original do 8values ainda é um rascunho a ser enviado pelo criador (comunicação externa). (b) **Prosa deep-lore pendente**: a classificação está completa, mas a prosa de cada figura (via `narrative-writer`, colaborativa) ainda não começou. (c) **Cosmético:** a tabela "Pending records" no apêndice ficou com rótulos "pending" desatualizados (a fonte de verdade é a "tabela de classificação final in-fiction"); manter como histórico ou higienizar numa passada futura. Fora esses, todos os 28 pedidos foram realizados e documentados.

---

*Recomendação técnica de compliance; validar com jurídico antes de qualquer decisão vinculante. This is a technical compliance recommendation; validate with legal counsel before any binding decision.*
