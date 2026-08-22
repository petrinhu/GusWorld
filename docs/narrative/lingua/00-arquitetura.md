# Lingua do GusWorld - Arquitetura (espinha dorsal)

Status: espinha dorsal APROVADA pelo lider 2026-06-23. **Nome canonico da lingua-mae: Sylvarin** (/sil-va-RIN/ = "a fala da Selve"; Sylva [a Selve] + -arin [sufixo de lingua, eco de Sindarin]). Escolhido pelo lider 2026-06-23. Em desenvolvimento COLABORATIVO (brainstorm dirigido + narrative-writer com o RAG-elfico). Detalhes (fonologia, gramatica, lexico, escrita) ainda NAO canonizados - serao desenvolvidos por etapas, cada uma validada pelo lider.

## Visao: 1 familia linguistica, 3 camadas no tempo + 1 escrita secreta transversal

Decisao do lider: "quero tudo - antiga, atual e magica - misturando as de Tolkien com portugues e uma escrita cifrada secreta". A estrutura abaixo amarra isso de forma coerente.

### 1. Camada ANTIGA - a lingua-mae (Era 1, Neo-Sylvania)
- **Fonologia:** base ELFICA (Tolkien, melodica: vogais abertas, consoantes suaves) temperada com sons do PORTUGUES (nasais a-til/o-til, digrafos lh/nh). Resultado: soa como um "portugues elfico arcaico" - exotico mas pronunciavel por um falante de PT.
- **Gramatica:** RIGIDA e regular (Pillar 2, natureza e matematica): paradigmas sem excecao, derivacao por regras formais. Numeros estruturais recorrentes nas classes, declinacoes e contagem.
- **Papel:** lingua dos construtores antigos de Neo-Sylvania. Aparece em inscricoes, docs descobriveis, puzzles. Decifravel via C-Arcane (ja canon).

### 2. Camada ATUAL (Era 3) - o que a party fala
- E o PORTUGUES do jogo COM substrato da lingua-mae: nomes proprios, termos tecnicos e magicos DERIVAM dela (como o ingles carrega latim/grego). A lingua-mae e o "latim" do GusWorld.
- Os IDIOLETOS comicos da party (matriz de linguagens-ancora, party.md) sao sotaques/registros dessa camada - NAO idiomas novos.

### 3. Camada MAGICA - C-Arcane e familia
- A lingua-mae COMPILADA: feiticos sao frases da lingua antiga escritas em sintaxe de programacao (Pillar 1, magia=software). Unidades canon: Glyph / Token / Conjuro / Codex.
- C-Arcane = a lingua antiga em forma de codigo de baixo nivel; outras linguagens magicas sao "dialetos" de nivel mais alto.

### 4. ESCRITA SECRETA (transversal) - o cripto-glifo cifrado da Era 1
- Sistema de escrita CIFRADO (grade 3x3 + X + pontos), ja canon na Era 1 (in-world-docs: GLIFO AGUA/ESPELHO/PASSAGEM).
- A forma sagrada/oculta de grafar a lingua. Base de puzzles e gates Ouro.

## Inspiracoes amarradas
- **Tolkien (elfico):** fonologia melodica + metodo filologico (deriva historica antiga -> atual, como Quenya -> linguas posteriores). Fonte no RAG-elfico (rag_elvish) + cursos no disco.
- **Portugues:** substrato fonetico (a lingua soa familiar) + a lingua viva da Era 3.
- **Tradicao cifrada:** o sistema de escrita secreto da Era 1 (cripto-glifo).

## Pillars amarrados
- Pillar 1 (magia=software): camada magica = lingua compilada.
- Pillar 2 (natureza=matematica): gramatica rigida + numeros estruturais recorrentes nos paradigmas.

## Recursos de pesquisa
- **RAG-elfico** (`resources/livros/rag_elvish`, 1989 chunks): cursos Pedin Edhellen (Sindarin), Quetin i lambe eldaiva (Quenya), Ni-bitha Adunaye (Adunaico), gramaticas, Tengwestie. Consulta isolada do indice principal.
- Cursos em PT+EN no disco: `resources/livros/elvish/`.

## Proximos passos (em desenvolvimento, validar cada um)
1. Fonologia (inventario de sons, fonotatica, estetica) - elfico + PT.
2. Nucleo de gramatica (morfologia, ordem, paradigmas regulares).
3. Lexico-semente (raizes + derivacao; nomes canon ja existentes como ancora: Vance, Tavus, Neo-Sylvania, Vyr...).
4. Sistema de escrita cifrado (formalizar o cripto-glifo).
5. Deriva historica antiga -> atual (empréstimos no PT do jogo).
6. C-Arcane como compilacao da lingua-mae.

## Processo
Deep-lore COLABORATIVO (regra do projeto): queries RAG propostas -> lider aprova -> executa visivel -> lider escolhe inspiracoes -> outline -> lider aprova -> narrative-writer escreve -> audit -> lider canoniza. Nada de narrative-writer autonomo.
