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

### Filled records (by fiat) / Registros preenchidos (por fiat)

#### ECO-03 Menger

- **(a) Prompt / Prompt:** N/A (by fiat / por fiat).
- **(b) 8values:** N/A (algorithm not run / algoritmo não rodado).
- **(c) Conclusion / Conclusão:** **RIGHT / DIREITA**, in-fiction "good / bom" (good evolution / boa evolução). Set **by authorial fiat**, Austrian-school economics cluster (canon axiology, see `project_axiologia_canonica`). / Definido **por fiat autoral**, cluster de economia da escola austríaca (axiologia canônica, ver `project_axiologia_canonica`). Approved by creator / aprovado pelo criador: 2026-07-11.

#### ECO-04 Bastiat

- **(a) Prompt / Prompt:** N/A (by fiat / por fiat).
- **(b) 8values:** N/A (algorithm not run / algoritmo não rodado).
- **(c) Conclusion / Conclusão:** **RIGHT / DIREITA**, in-fiction "good / bom" (good evolution / boa evolução). Set **by authorial fiat**, Austrian/classical-liberal economics cluster (canon axiology, see `project_axiologia_canonica`). / Definido **por fiat autoral**, cluster de economia austríaca/liberal-clássica (axiologia canônica, ver `project_axiologia_canonica`). Approved by creator / aprovado pelo criador: 2026-07-11.

#### CAP-01 Helion Tusk (living inspiration / inspiração viva)

- **(a) Prompt / Prompt:** **None. Deliberately not profiled** (living figure). / **Nenhum. Deliberadamente não perfilado** (figura viva).
- **(b) 8values:** **Not run. Deliberately not quizzed** (living figure). / **Não rodado. Deliberadamente não submetido ao quiz** (figura viva).
- **(c) Conclusion / Conclusão:** **RIGHT / DIREITA**, in-fiction "good / bom" (good evolution / boa evolução). Set **by pure authorial fiat as a game-design choice**, never derived from any assessment of the real living person (see [A note on the one living inspiration](#a-note-on-the-one-living-inspiration)). / Definido **por fiat autoral puro como escolha de design de jogo**, nunca derivado de qualquer avaliação da pessoa viva real (ver [Uma nota sobre a única inspiração viva](#uma-nota-sobre-a-única-inspiração-viva)). Approved by creator / aprovado pelo criador: 2026-07-11.

### Computed records (through the algorithm) / Registros computados (pelo algoritmo)

Os dados abaixo (prompt, respostas, resultado, insights) ficam em pt-br, a língua de trabalho do design; os rótulos seguem o template bilíngue. / The data below is kept in pt-br, the design working language; labels follow the bilingual template.

#### ECO-02 Mises (calibration anchor / âncora de calibração)

- **(a) Prompt / Prompt do persona-agent** (dado verbatim ao `AGENTE_PERSONA_MISES`, modelo Opus, 2026-07-11): "Você ENCARNA Ludwig von Mises (1881-1973), respondendo o quiz de 70 perguntas do 8values EM PERSONAGEM, refletindo honestamente as visões e o temperamento REAIS e documentados dele. PERSONALIDADE/VISÕES incorporadas: Escola Austríaca, fundador da praxeologia (economia dedutiva/a priori, rejeita empirismo); liberal clássico radical/laissez-faire (mercados livres logicamente necessários); anti-socialista (argumento do cálculo econômico: planejamento racional impossível no socialismo); intransigente, movido por axiomas inabaláveis. Big Five: Conscienciosidade MUITO ALTA; Abertura MODERADA-BAIXA (dogmático no método, razão universal, anti-relativista); Amabilidade BAIXA (combativo, direto); Extroversão BAIXA; Neuroticismo MODERADO (contenção estoica). Estado mínimo, propriedade privada, moeda sólida, livre comércio, universalismo acima de classe/etnia; opôs-se a fascismo E socialismo; EXTREMO na liberdade econômica, liberal-clássico/razão no social. TEMPERATURA de resposta: decidido/dogmático em economia/liberdade/Estado (convicção forte, raramente Neutral); fora do núcleo, razão liberal-clássica universalista. Instrução anti-viés: o quiz mistura fraseado a-favor e contra; ler o sentido de cada uma."
- **(b) 8values result / Resultado 8values:** Econômico **Markets 84.6%** (Capitalist) · Diplomático Peace 58.3% (Balanced) · Civil/Governo **Liberty 65.6%** (Liberal) · Social Progress 57.0% (Neutral). Ideologia mais próxima (tabela de 52): **Libertarian Capitalism**. **`rightness = 0.846`** (societal_tradition = 43.0). Respostas (70, ordem 1-70): `["sd","sd","sa","sa","sd","sd","sd","sa","sd","sa","sd","sa","a","d","sd","d","a","a","n","n","a","d","d","a","n","a","n","d","d","d","a","d","a","d","d","d","d","n","d","d","sd","d","d","a","d","n","d","a","d","d","n","n","a","a","a","a","d","d","n","d","d","sd","d","n","n","n","a","n","a","sd"]`.
- **Insights do persona-agent (voz Mises, itens não-óbvios):** utilidades públicas/regulação ambiental = disagree (coordenação por propriedade privada + cálculo de mercado, não decreto); ordem internacional = nem nacionalista nem globalista-planejador (livre-comércio + harmonia de interesses, desconfia de governo mundial E chauvinismo → neutros calibrados); Q36 "Estado é ameaça à liberdade" = disagree DELIBERADO (NÃO é anarquista; Estado mínimo guardião da propriedade é condição da liberdade); democracia = método de trocar governos sem sangue, não valor sagrado acima de direitos; condutas privadas (droga/sexo/casamento) = autopropriedade, não cabe ao Estado criminalizar, ainda que não endosse moralmente; Q57 "nenhuma cultura é superior" = disagree (não relativista; civilização liberal-capitalista objetivamente superior em prosperidade/paz; razão universal).
- **(c) Conclusion / Conclusão:** **RIGHT / DIREITA**, in-fiction "good / bom" (boa evolução). `rightness 0.846` ≫ 0.599. **Tripla checagem:** (1) 8values = 0.846, "Libertarian Capitalism"; (2) web (Wikipedia) = "far right on economic freedom, classical liberal, libertarian genealogy"; (3) **convergem** e confirmam a triagem óbvia-direita do criador. Também é a **âncora de calibração de direita** (método validado nela). Approved by creator / aprovado pelo criador: 2026-07-11 (triagem óbvia-direita) + confirmado pelo algoritmo.

### Pending records (through the algorithm) / Registros pendentes (pelo algoritmo)

The figures below are classified via the full persona + 8values + triple-check pipeline, and are filled in as each round runs. Mises and Hayek double as the right-leaning **calibration anchors** of the [Method calibration](#method-calibration) test. / As figuras abaixo são classificadas via o pipeline completo persona + 8values + tripla checagem, e são preenchidas conforme cada rodada roda. Mises e Hayek também servem de **âncoras de calibração** de direita do teste de [Calibração do método](#calibração-do-método).

| ID | Figure / Figura | Status | Notes / Notas |
| :--- | :--- | :--- | :--- |
| ELM-01 | Faraday | pending / pendente | card already canonized (`_IDS-CARTAS.md`); classification record to fill / carta já canonizada; registro de classificação a preencher |
| ELM-02 | Maxwell | pending / pendente | to fill per round / a preencher conforme rodada |
| ELM-03 | Tesla | pending / pendente | to fill per round / a preencher conforme rodada |
| ELM-04 | Volta | pending / pendente | to fill per round / a preencher conforme rodada |
| FIS-01 | Einstein | pending / pendente | also a known-left **validation point** in calibration / também **ponto de validação** conhecido de esquerda na calibração |
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
| ECO-01 | Hayek | pending / pendente | right-leaning **calibration anchor** / **âncora de calibração** de direita |
| ECO-02 | Mises | **done / feito** | RIGHT/DIREITA, rightness 0.846 (see Computed records above / ver Registros computados acima); calibration anchor ✓ |

External calibration-only figures (Marx, Mao Zedong) are **not** part of the game roster and get no in-game record; they exist solely as known-left controls in the calibration test. / Figuras externas só-de-calibração (Marx, Mao Tsé-tung) **não** fazem parte do roster do jogo e não têm registro in-game; existem apenas como controles conhecidos de esquerda no teste de calibração.

---

*Recomendação técnica de compliance; validar com jurídico antes de qualquer decisão vinculante. This is a technical compliance recommendation; validate with legal counsel before any binding decision.*
