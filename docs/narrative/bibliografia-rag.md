# Bibliografia RAG da lore do GusWorld

> **Tipo (Diátaxis):** Reference. **Audiência:** criador (petrus) + squad narrativo (`narrative-designer`, `narrative-writer`).
> **Owner:** narrativa. **Last-reviewed:** 2026-06-23. **Versão:** corpus RAG v1 (manifest 2026-06).
> **Reportar imprecisão:** abrir item no `TODO.md` (tag `F1-DL`) ou avisar no canal de narrativa.

Este documento cataloga o material indexado nos RAGs (Retrieval-Augmented Generation) locais do projeto. São **dois índices isolados**: o **RAG principal** (~306 obras, ~163.443 chunks) com a bibliografia geral de lore, e o **RAG élfico** (`rag_elvish`, 1.989 chunks) com o material de língua élfica de Tolkien que inspira a conlang **Sylvarin** (ver §13). O RAG é uma camada de **busca semântica de inspiração** para a deep-lore: serve para cruzar referências, achar passagens temáticas e enriquecer worldbuilding durante o design narrativo.

**Importante: isto é bibliografia de inspiração e referência, não fonte de cópia.** Nenhum texto destes livros é reproduzido no jogo. O RAG ajuda o squad a *pensar com* essas obras (encontrar uma analogia de economia austríaca, um padrão de cronologia tolkieniana, uma imagem cyberpunk), nunca a *plagiar*. A lore canônica do GusWorld é original e escrita pelo squad, validada pelo criador.

---

## Como o RAG funciona

O corpus vive em `resources/livros/` (pasta inteira gitignored, não versionada) e é indexado pelo CLI `rag`.

- **Embedding:** modelo `bge-m3` servido localmente via Ollama. Converte cada trecho de livro em vetor semântico.
- **Reranker:** `bge-reranker-v2-m3` reordena os candidatos por relevância à consulta.
- **Banco vetorial:** `resources/livros/rag/chunks.lance` (Lance) + metadados em `resources/livros/rag/manifest.json` (fonte do inventário principal). O corpus élfico (§13 a §15) é um **segundo índice isolado** em `resources/livros/rag_elvish` (1.989 chunks), consultado à parte para não poluir as queries de lore geral.
- **Wrapper:** usar sempre `rag-safe query "..."` (serializa via `flock`, 1 consulta por vez; respeita os limites de hardware da máquina).

Consulta típica durante design de lore:

```bash
rag-safe query "ecologia planetária moldando religião e profecia em deserto" --json --top-k 8
```

### Limites práticos da busca (empírico)

O gate de score `>= 0.499` do reranker **não mede tema certo, mede tipo textual do corpus.** Resumo da memória `reference_rag_scoring_oom`:

- **Crava alto (0.7 a 0.98):** texto expositivo/argumentativo. Economia austríaca, "As 48 Leis do Poder", Orwell didático, Umberto Eco discursivo, "Without Conscience", Dan Brown expondo conspirações.
- **Reprova (0.01 a 0.45) mesmo on-tema:** prosa narrativa pura. Bíblia, Mad Max, Gibson, Dick. A imagética não cross-encoda bem contra consultas temáticas em português.

**Consequência de método:** para temas com forte match expositivo, deixar o RAG sugerir. Para imagética narrativa (cyberpunk, queda, catástrofe, êxodo), **escrever do canon do jogo** (timeline, lore-bible, era-1 §8); o RAG é camada bônus, não fonte única.

Procedimento canônico de queries (memória `project_alvo_palavras_pos_era_1`): 10 consultas densas (15 a 30+ palavras cada), todas com score `>= 0.499`, até 30 tentativas para atingir 10 boas.

---

## Inventário por autor / saga / tema

Total: **306 obras** em 12 grupos. Para cada grupo: títulos (sagas grandes resumidas) + como inspira a lore do GusWorld, com referência aos 5 pillars e às memórias canônicas.

### 1. Tolkien, Legendarium e filologia (42 obras)

Saga e materiais filológicos de J.R.R. (e Christopher) Tolkien:

- **Núcleo:** *The Hobbit* (+ Enhanced Edition), *The Lord of the Rings* (Fellowship / Two Towers / Return of the King, + Illustrated Edition), *The Silmarillion* (+ Illustrated), *Unfinished Tales*, *The Children of Húrin*.
- **History of Middle-earth Vol. 01 a 12** (série completa: Book of Lost Tales 1-2, Lays of Beleriand, Shaping of Middle-earth, Lost Road, Return of the Shadow, Treason of Isengard, War of the Ring, Sauron Defeated, Morgoth's Ring, War of the Jewels, Peoples of Middle-earth).
- **Menores / contos:** *Tales from the Perilous Realm*, *Farmer Giles of Ham*, *Roverandom*, *Leaf by Niggle*, *Smith / Tom Bombadil*, *Bilbo's Last Song*, *Letters from Father Christmas*, *The Legend of Sigurd and Gudrún*, *The Homecoming of Beorhtnoth*, *Pictures by J.R.R. Tolkien*.
- **Filologia (chave para lore de língua):** *A Middle English Vocabulary*, *Songs for the Philologists*, *Finn and Hengest*, *The Lays of Beleriand*.

**Como inspira:** modelo máximo de **worldbuilding profundo com camadas de tempo** (cronologia em eras, versões in-world divergentes de um mesmo mito), alimenta a estrutura de 3 eras do GusWorld e os docs descobríveis (`in-world-docs.md`). A vertente filológica é a referência direta para a **lore de língua/conlang** que o criador quer construir: como inventar idiomas com história interna e deriva fonética coerente. Conecta ao pillar **Magia = software** (linguagens rúnicas compiladas como artefatos com gramática e versão) e aos padrões numéricos recorrentes em proporções/datações.

### 2. Asimov, ficção científica e ciência popular (171 obras)

O maior bloco do corpus. Subgrupos:

- **Foundation** (7: Foundation a Forward the Foundation): **psico-história** e profecia estatística.
- **Robot / Robôs** (Caves of Steel, Naked Sun, Robots of Dawn, Robots and Empire; *I, Robot*, *The Complete Robot*, *Robot Dreams*, *Robot Visions*, *The Positronic Man*, *Nós, robôs*): leis formais governando agentes; máquina como sistema lógico.
- **Galactic Empire** (3), **Lucky Starr** (6), **Norby Chronicles** (6), **Black Widowers** (6 coletâneas de mistério dedutivo), **Future History** (vol. 1 a 20).
- **Romances avulsos** (Nightfall, The Gods Themselves, The End of Eternity, Nemesis, Fantastic Voyage 1-2, The Ugly Little Boy, etc.) e **coletâneas de contos** (Nine Tomorrows, Gold, Azazel, The Complete Stories, The Early Asimov 1-3, etc.).
- **Ciência popular, série "How did we find out about..."** (~37 títulos: átomos, DNA, números, computadores, buracos negros, genes, cérebro, fotossíntese, etc.).
- **Livros científicos** (Understanding Physics 1-3, Asimov on Numbers, Realm of Algebra, The Human Body, The World of Carbon/Nitrogen, guias de Shakespeare e da Bíblia, etc.) + biografias.

**Como inspira:** espinha dorsal de dois pillars. **Magia = software** (feitiço como script/lei formal, à la Três Leis da Robótica). **Natureza é matemática rígida** (a ciência popular de Asimov sobre números, álgebra e física é a fonte para tratar fauna/flora da Selve Sombria como sequências e funções recursivas, com anomalias = bugs). A **psico-história** de Foundation inspira a noção de previsão estatística e destino computável usada na axiologia (boas vs más evoluções). Os mistérios dedutivos (Black Widowers) modelam o **protagonista prodígio que resolve por lógica**, não por força (pillar idade 11).

### 3. Frank Herbert + Brian Herbert/Anderson, saga Dune (22 obras)

- **Originais de Frank Herbert (6):** *Dune*, *Dune Messiah*, *Children of Dune*, *God Emperor of Dune*, *Heretics of Dune*, *Chapterhouse: Dune*.
- **Continuações (Brian Herbert / Kevin J. Anderson):** Prelude to Dune (House Atreides/Harkonnen/Corrino), Great Schools of Dune (Sisterhood/Mentats/Navigators + Red Plague), Heroes of Dune (Paul/Winds), Hunters of Dune, Sandworms of Dune, Tales of Dune, The Road to Dune.
- **Apoio:** *Eye* (contos), *CliffsNotes on Dune*, *Dreamer of Dune* (biografia).

**Como inspira:** **ecologia como força narrativa** (o deserto que molda cultura e religião) é o modelo para a Selve Sombria e o setting bipartido (megacidade x natureza-matemática). **Religião-tecnologia e profecia** (Bene Gesserit, missionaria protectiva) inspiram facções que instrumentalizam fé e dado, alimenta a axiologia (coletivismo/messianismo evoluindo para ruim, à la culto que se consolida). Mentats (computação humana proibida) ecoam o pillar **Magia = software** e o loop hardware-acoplado dos óculos táticos/Tavus-Drive.

### 4. George R. R. Martin, As Crônicas de Gelo e Fogo (10 obras)

- **Saga principal:** Guerra dos Tronos, A Fúria dos Reis, A Tormenta das Espadas, O Festim dos Corvos, A Dança dos Dragões.
- **Complementares:** *Fogo & Sangue* Vol. 1, *O Cavaleiro dos Sete Reinos*, *A Princesa e a Rainha* (Negros x Verdes), *O Príncipe de Westeros e outras histórias*, *O Mundo de Gelo e Fogo*.

**Como inspira:** **política de facções e cronistas in-world.** As 7 facções do GusWorld (`factions.md`) bebem da intriga dinástica e da moral cinzenta de Westeros. *Fogo & Sangue* e *O Mundo de Gelo e Fogo* são o modelo direto para os **docs descobríveis** (`in-world-docs.md`): crônicas escritas por narradores parciais, com versões conflitantes do mesmo evento, reforça os gates Ouro e a história em camadas das 3 eras.

### 5. Cyberpunk, setting ciber-gótico (~15 obras, parte no grupo "avulsos")

Núcleo do setting da megacidade. Distribuído entre pastas e raiz:

- **William Gibson:** *Neuromancer*, *Count Zero*, *Mona Lisa Overdrive* (trilogia Sprawl), *Idoru*, contos (*O Contínuo de Gernsback*).
- **Bruce Sterling:** *Mirrorshades* (antologia, em espanhol), *Piratas de Dados*.
- **Neal Stephenson:** *Snow Crash*.
- **Philip K. Dick:** *Ubik*, *Realidades Adaptadas*, *Androides Sonham com Ovelhas Elétricas?* (Blade Runner) + *blade_runner_script*.
- **Outros:** Richard Morgan *Altered Carbon*; Ian McDonald *Brasyl*; Harlan Ellison *Não tenho boca e preciso gritar* (conto); scripts de *Matrix* e *Akira*; *Mad Max* (Terry Hayes).

**Como inspira:** fonte direta do **setting bipartido**, a megacidade ciber-gótica (pillar 5). Gibson e Sterling dão a textura de corporações, ciberespaço e periferia (Dante Grid, FIR vassalo, C-Arcane). Snow Crash conecta **linguagem como código executável** (vírus linguístico) ao pillar **Magia = software**. Dick alimenta os antagonistas-sistema (Patch-Zero, realidade adulterada). Nota empírica: imagética cyberpunk reprova no reranker, usar como leitura humana e escrever do canon.

### 6. Economia austríaca / liberal, axiologia canônica (~20 obras, parte em "avulsos")

Base teórica da axiologia do projeto (memória `project_axiologia_canonica`).

- **Ludwig von Mises:** *Ação Humana*, *As Seis Lições*, *Liberalismo*, *O Cálculo Econômico sob o Socialismo*, *Uma Crítica ao Intervencionismo*, *O Essencial von Mises*.
- **F. A. Hayek:** *O Caminho da Servidão*, *Desemprego e Política Monetária*, *Desestatização do Dinheiro*.
- **Murray Rothbard:** *A Anatomia do Estado*, *A Ética da Liberdade*, *Governo e Mercado*.
- **Frédéric Bastiat:** *A Lei*. **Milton Friedman:** *Livre para Escolher*. **Adam Smith:** *Riqueza das Nações* (vol. 1 e 2).
- **Companheiros temáticos:** Ayn Rand *A Revolta de Atlas*; *Como destruir esquerdistas em debate*.

**Como inspira:** define a **moral interna do mundo**. Coletivismo/socialismo/globalismo evoluem para ruim; livres trocas, valores conservadores e libertarianismo austríaco evoluem para bom. Sterling Locke = **capitalismo de compadrio** (mau porque distorce o mercado, não porque é capitalista). A Ordem Recursiva consolidada = má evolução; Famílias-Pilastra, Pelicano Branco e a família Vance (descentralizada, funcional) = boa evolução. Estes textos cravam alto no reranker, fonte forte de cruzamento.

### 7. Filosofia / poder, antagonistas e Sterling (~7 obras, em "avulsos")

- Thomas Hobbes *Leviatã*; Maquiavel *O Príncipe*; Sun Tzu *A Arte da Guerra*.
- Robert Greene *As 48 Leis do Poder*; Robert Hare *Without Conscience* (psicopatia); Jack Shafer *Manual de Persuasão do FBI*.
- True-crime de manipulação: *Suzane: assassina e manipulador* (Ullisses Campbell).

**Como inspira:** psicologia dos **antagonistas**. *Without Conscience* e *As 48 Leis do Poder* (ambos cravam alto no reranker) modelam Sterling Locke (predador corporativo) e Patch-Zero (antagonista-sistema). O *Manual de Persuasão* e Maquiavel/Hobbes informam manipulação e legitimidade de poder das facções, contraponto à axiologia liberal (o que acontece quando o poder centraliza).

### 8. Clássicos, Homero (2 obras)

- *Ilíada* e *Odisseia*.

**Como inspira:** arquétipos de **jornada e arco heroico** (`arco-principal.md`, beats Kishōtenketsu). A astúcia de Odisseu (vencer por engenho, não força) reforça o pillar **idade 11 / prodígio analítico**.

### 9. Orwell, distopia política (2 obras, em "avulsos")

- *1984* e *A Revolução dos Bichos*.

**Como inspira:** vigilância, linguagem controlada (Novilíngua) e corrupção de revoluções coletivistas. Liga a **Magia = software** (linguagem que molda o pensável) e à **axiologia** (coletivismo evoluindo para ruim). *1984* crava alto no reranker (didático); *Revolução dos Bichos* reprova (narrativo), ler humano.

### 10. Umberto Eco, semiótica e conspiração erudita (2 obras)

- *O Nome da Rosa* e *O Pêndulo de Foucault*.

**Como inspira:** o **mistério decifrável por erudição** e a conspiração de ordens fechadas. Fonte direta dos **enigmas decifráveis e da escrita cifrada da Era 1**, e dos docs in-world enigmáticos. Eco discursivo crava alto no reranker.

### 11. Dan Brown, cifras e criptografia (7 obras)

- *O Código Da Vinci*, *Anjos e Demônios*, *O Símbolo Perdido*, *Inferno*, *Fortaleza Digital*, *A Chave de Salomão*, *A Conspiração*.

**Como inspira:** ligação direta à **escrita cifrada da Era 1** (o cripto-glifo) e aos enigmas decifráveis. *O Símbolo Perdido* e *Fortaleza Digital* (criptografia) modelam os enigmas e os gates Ouro. Exposição conspiratória crava alto no reranker.

### 12. Illuminatus + diversos, conspiração e apoio (resto de "avulsos")

- Robert Shea & Robert Anton Wilson *The Illuminatus! Trilogy*: paranoia conspiratória pós-moderna (reforça os padrões ocultos pervasivos da lore).
- Arthur C. Clarke *23 Science Fiction Books*: antologia de FC clássica (apoio ao pillar **Magia = software** e natureza-matemática).
- Bíblia King James 1611: referência mítica/genealógica e de linguagem arcaica (cronologias, profecia; reprova no reranker, leitura humana).
- Simon Sinek *O Jogo Infinito*: teoria de jogos de longo prazo (estratégia de facções).
- Pavinato *O Que É a Verdade*: apoio filosófico/epistemológico.

**Como inspira:** camada de **conspiração e padrão oculto** que sustenta os padrões pervasivos da lore, sem siglas nem gestos nomeados.

---

## Corpus élfico (RAG isolado para a conlang Sylvarin)

> **Índice separado.** Este material NÃO está no RAG principal. Vive no segundo índice `resources/livros/rag_elvish` (1.989 chunks), consultado de forma isolada. Os arquivos-fonte ficam em `resources/livros/elvish/` (gitignored). Serve a um único objetivo de design: alimentar a **língua-mãe Sylvarin** (a conlang do GusWorld; ver `docs/narrative/lingua/00-arquitetura.md`).

A Sylvarin é uma família linguística de 3 camadas no tempo (antiga Era 1 / atual Era 3 / mágica C-Arcane) mais uma escrita cifrada transversal. O corpus élfico de Tolkien é a referência filológica para quatro frentes da conlang: **(a)** fonologia élfica melódica temperada com português (o "português élfico arcaico"); **(b)** gramática rígida e regular sem exceção (Pillar 2, natureza é matemática; paradigmas com números recorrentes); **(c)** a escrita cifrada da Era 1 (o cripto-glifo, modo de grafar oculto, à la modos Tengwar); **(d)** a deriva histórica antiga para atual (como Quenya gera línguas posteriores; modelo para o substrato Sylvarin no português da Era 3). **Importante:** nenhum vocábulo élfico de Tolkien entra no jogo; o método (como inventar idioma com história interna e deriva fonética coerente) é o que inspira, não o léxico.

### 13. Cursos de língua élfica (Sindarin, Quenya, Adúnaico)

- **Thorsten Renk - *Pedin Edhellen* (curso de Sindarin, v2.0, EN)** (`en/pedin_edhellen_en_v2.0.pdf`): curso estruturado de Sindarin. Inspira a progressão pedagógica e a fonologia consonantal suave da camada antiga da Sylvarin.
- **Ardalambion Brazil / Valinor (trad. PT) - *Curso de Sindarin*** (`pt/curso-de-sindarin_pt/`, 22 lições + 3 apêndices): versão em português do curso de Sindarin. A leitura em PT ajuda a calibrar o tempero "português élfico" (como o som élfico convive com nasais e dígrafos lh/nh).
- **Thorsten Renk - *Quetin i lambe eldaiva* (curso de Quenya, v2, EN)** (`en/quetin_lambe_eldaiva_en_v2.pdf`): curso estruturado de Quenya. Modelo de gramática regular e de classe nominal; referência para os paradigmas rígidos.
- **Ardalambion Brazil / Valinor (trad. PT) - *Curso de Quenya*** (`pt/curso-de-quenya_pt/`, 20 lições + intro, apêndice, respostas, vocabulário, dicionário Quenya-Português p1/p2): versão em português do curso de Quenya. Fonte da deriva histórica (Quenya como "latim" élfico) que inspira a Sylvarin como latim do GusWorld.
- **Diversos autores - *Ni-bitha Adúnaye* (curso de Adúnaico, EN)** (`en/ni_bitha_adunaye.pdf`): curso da língua dos Homens de Númenor (Adúnaico). Contraste útil: uma língua menos élfica e mais "humana/terrena", referência para registros menos melódicos do substrato atual.

### 14. Gramáticas e artigos filológicos élficos

- **Diversos autores - 8 gramáticas élficas (HTML)** (`en/grammar_*.html`): notas de gramática sobre casos (`grammar_cases`), mutações consonantais (`grammar_mutations`), pronomes (`grammar_pron_rek`, `grammar_quenya_pronouns`), verbos (`grammar_verbs`), e tempos verbais Quenya passado/perfeito (`grammar_quenya_past_tense`, `grammar_quenya_perfect_tense`) e Sindarin passado (`grammar_sindarin_past_tense`). Inspiram os paradigmas formais sem exceção da gramática rígida da Sylvarin (Pillar 2): as mutações consonantais são o modelo direto para a derivação por regra que a conlang exige.
- **Diversos autores - *Tengwestie* (~15 artigos do journal, EN)** (`en/tengwestie_*.pdf`): artigos acadêmicos do periódico de linguística tolkieniana (Tengwestie). Cobrem tópicos como acento eldarin (`eldarinaccent`), plurais e padrões Noldorin (`noldplur`, `noldpat`, `noldintenspref`), tempos compostos do Quenya (`quenyacompoundtenses`), numerais rúmilianos (`rumiliannumerals`), Ilkorin antigo (`earlyilkorin`), reencarnação élfica (`elvishreincarnationgloss`), comentários de topônimos (`limlight`, `limlightcommentary`, `goldpat`, `causquen`, `sindll`), luz e árvore (`lightandtree`) e resenha de *The Nature of Middle-earth* (`natureofmiddleearthreview`). Inspiram o rigor do **método filológico**: como tratar a língua antiga como objeto com história interna, variantes e deriva.
- **Diversos autores - *VT-index* (índice do Vinyar Tengwar, EN)** (`en/VT-index-en.pdf`): índice do periódico Vinyar Tengwar. Mapa de consulta para localizar tópicos filológicos específicos no corpus.

### 15. Escrita: modos Tengwar / cifra

- **Diversos autores - *Tengwar-Português* (modo MTP, PT)** (`pt/tengwar-portugues-mtp.pdf`): modo de transcrição do português em Tengwar (a escrita élfica). Inspira diretamente a **escrita cifrada transversal** da Sylvarin: como mapear sons do português a um sistema de grafia élfico, base conceitual para o cripto-glifo cifrado da Era 1 como "forma sagrada/oculta de grafar a língua".

---

## Contagem por grupo

| # | Grupo | Obras | Pillar / memória que inspira |
|:--|:------|:-----:|:-----------------------------|
| 1 | Tolkien (Legendarium + filologia) | 42 | Worldbuilding profundo; conlang; Magia = software; padrões numéricos |
| 2 | Asimov (FC + ciência popular) | 171 | Magia = software; Natureza = matemática; prodígio lógico |
| 3 | Herbert / Dune | 22 | Ecologia; religião-tecnologia; profecia; axiologia |
| 4 | George R. R. Martin (ASOIAF) | 10 | Política de facções; cronistas in-world |
| 5 | Cyberpunk (Gibson/Sterling/Stephenson/Dick/...) | ~15 | Setting ciber-gótico; linguagem-código |
| 6 | Economia austríaca / liberal | ~20 | Axiologia canônica |
| 7 | Filosofia / poder | ~7 | Antagonistas; Sterling; Patch-Zero |
| 8 | Homero (clássicos) | 2 | Arco heroico; engenho > força |
| 9 | Orwell | 2 | Vigilância; linguagem; axiologia |
| 10 | Umberto Eco | 2 | Semiótica; conspiração; enigmas |
| 11 | Dan Brown | 7 | Cifras; criptografia; gates Ouro |
| 12 | Illuminatus + diversos | ~6 | Conspiração; padrão oculto; apoio |
| | **Total (RAG principal)** | **306** | |

> **Corpus élfico (RAG isolado `rag_elvish`, ver §13 a §15):** cursos de Sindarin (Pedin Edhellen, EN + PT), Quenya (Quetin i lambe eldaiva, EN + PT) e Adúnaico (Ni-bitha Adúnaye, EN); ~15 artigos do journal Tengwestie; 8 gramáticas em HTML; VT-index; modo Tengwar-Português. Inspira a conlang **Sylvarin** (fonologia élfica + português, gramática rígida, escrita cifrada da Era 1, deriva histórica antiga para atual). Não conta no total do RAG principal.

> Os grupos 5, 6, 7, 9 e 12 partilham a categoria física "avulsos" (50 arquivos na raiz de `resources/livros/`); a soma das estimativas (`~`) reflete a classificação temática, não pastas. Os grupos 1 a 4, 8, 10, 11 correspondem a pastas dedicadas. Fonte exata: `resources/livros/rag/manifest.json`.

---

## Ver também

- `docs/narrative/lore-bible.md`: bíblia de lore canônica (destino da inspiração).
- `docs/narrative/timeline.md`: cronologia das 3 eras (modelo tolkieniano/Martin).
- `docs/narrative/in-world-docs.md`: docs descobríveis (modelo Fogo & Sangue / Eco / Dan Brown).
- `docs/narrative/factions.md`: 7 facções (modelo ASOIAF + Dune + axiologia).
- `docs/narrative/lingua/00-arquitetura.md`: arquitetura da conlang Sylvarin (destino do corpus élfico, §13 a §15).
- `resources/livros/howto_rag.md`: how-to de uso do CLI `rag` (não versionado).
- Memórias: `reference_rag_cli`, `reference_rag_scoring_oom`, `project_alvo_palavras_pos_era_1`, `project_axiologia_canonica`, `feedback_rag_query_batch_2`.
