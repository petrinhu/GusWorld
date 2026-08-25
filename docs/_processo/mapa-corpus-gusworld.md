# Mapa do corpus documental - GusWorld

**Autor:** technical-writer (agente operacional, inventário somente leitura)
**Data do levantamento:** 2026-08-21
**Escopo:** `docs/`, `resources/`, `assets/` do repositório GusWorld
**Método:** varredura de arquivos e `grep` sobre o corpus real. Toda afirmação de FATO traz `arquivo:linha`. Afirmações sem citação são marcadas **[INFERÊNCIA]**.

**Contadores globais (FATO, medidos por `find`/`wc` em 2026-08-21):**
- `docs/`: 243 arquivos `.md`, 57.428 linhas.
- `resources/`: 207 arquivos `.md`, 4.794 linhas (mais ativos binários, ver seção 5).
- PNGs no repositório inteiro: 1.266 arquivos.
- `docs/audio/` e `docs/production/` existem como diretórios mas **não contêm nenhum arquivo** (FATO, `find docs/audio -type f` e `find docs/production -type f` retornam vazio).

Este documento está sendo escrito **seção por seção, incrementalmente**, à medida que a varredura avança. Cada seção abaixo é gravada assim que apurada.

---

## Seção 1 - Índice por tema

### `docs/design/` (81 arquivos, 18.187 linhas) - o maior bloco de regras de jogo

Contém pillars, GDD, especificações de mecânica, produção e pesquisa de referência.

- **Raiz** (`docs/design/*.md`, 8 arquivos): `pillars.md` (os 4 pilares + anti-pillars, documento-mestre de validação de feature), `gdd.md` (GDD de 1 página), `card-frame-spec.md` (moldura visual de carta), `mundo-topologia.md` (13 áreas do mundo), `techmagic.md`, `lore-delivery-model.md`, `gus-abertura.md`, `brainstorm-backlog.md`.
- **`mecanicas/`** (44 arquivos, é a maior subpasta do repo inteiro): sistema de combate (`combat.md`, `battle-screen.md`, `battle-anim.md`, `combat-flavor.md`), sistema de cartas/deck (`deck-mao-sistema.md`, `cartas-*` = 8 arquivos), economia (`economia.md`), exploração (`core-loop-exploracao.md`, `locomotion.md`, `mini-mapa.md`), progressão (`knowledge-progression.md`), morte/dificuldade (`modos-morte.md`), encontros (`encontros-aleatorios.md`), puzzles (`puzzle-gambito.md`), 6 análises de pacing/mira (`analise-*`), 4 propostas (`proposta-*`).
- **`roster-analogos/`** (25 arquivos): 21 fichas de "mestres" (figuras históricas da ciência/economia convertidas em análogos in-world: Faraday, Maxwell, Tesla, Volta, Einstein, Newton, Planck, Mandelbrot, Euler, Gödel, Ada Lovelace, Turing, von Neumann, Giordano Bruno, John Dee, Pitágoras, Hayek, Mises, Menger, Bastiat, Helion Tusk) + 4 arquivos de metodologia/reconciliação/efeitos/IDs de carta.
- **`producao/`** (5 arquivos): `plano_vs.md` e `raid-log.md` (risco), `ci-build-plan.md`, `art-spike-protocol.md` (todos com nota de stack "pós-ADR-008" indicando que descrevem um pipeline Godot/C# já aposentado).
- **`research/`** (2 arquivos): `game-design-refs-2026-07.md` e `game-design-juice-intent-2026-07.md`, calibração de referências de mercado (Sea of Stars, Balatro, Chained Echoes etc.).
- **`levels/`** (1 arquivo): `blockout-distritos-inferiores.md` (grafo de nós do primeiro distrito jogável).
- **`narrativa/`** (1 arquivo): `dialogue-tree-npc-intro.md`.
- **`propostas/`** (1 arquivo): `copy-stubs-combate.md`.

### `docs/narrative/` (134 arquivos, 34.013 linhas) - o maior bloco do corpus inteiro

- **Raiz** (16 arquivos): `lore-bible.md` (bíblia central), `timeline.md` (3 eras), `factions.md` (6 facções), `arco-principal.md`, `in-world-docs.md` (23 documentos descobríveis in-game), `foreshadowing.md`, `INCOHERENCES.md` (tracker de incoerências resolvidas), `farpas-linguagens.md`, `guia-dialogos.md`, `guia-narrativa-fluida.md`, `comic-reliefs.md`, `tradicoes-cultura.md`, `vozes-party.md`, `gus-apartes-c-arcane.md`, `bibliografia-rag.md`, `PERGUNTAS-CANON-GUS-AHSD-2026-08-01.md`.
- **`characters/`** (16 arquivos): 1 ficha por personagem de party/antagonista (`gus.md`, `bento-requiem.md`, `caua-volt.md`, `dante-grid.md`, `iara-lumen.md`, `jaci-proxy.md`, `linda-siren.md`, `sterling-locke.md`, `patch-zero.md`) + `party.md` (visão de grupo) + `prelore_vilao.md` + 4 "contos" de personagens secundários (Brunus, Pyotor, Yakov).
- **`deep/`** (74 arquivos): camada de deep-lore destinada a virar livro pós-release. Subpastas: `antologia/` (14 contos in-character, 1 por membro de party/antagonista/figura histórica), `characters/` (28 fichas profundas, incluindo 1 por "mestre" análogo do roster), `eras/` (5, cosmologia e as 3 eras), `factions/` (7), `settings/` (8, 1 por bioma/distrito), `magic/` (3, as linguagens mágicas), `ontologia/` (4, cosmologia formal e leitmotivs), `antagonists/` (3), `stinger/` (2, ganchos pós-créditos/sequência). `_INDEX.md` orienta a camada inteira.
- **`diary/`** (8 arquivos): sistema de "Diário do Gus" in-game: `entries-docs-descobriveis.md`, `entries-fichas-bestiary.md`, `entries-manuscrito-glossario.md`, `entries-mapas-timeline.md`, `foreshadow-links.md`, `knowledge-gates.md` (inclui o save schema, §10.1), `ui-spec.md`.
- **`environments/`** (9 arquivos): 1 por bioma jogável (`01-cidade-cyber-gotica.md` até `08-selve-profunda.md`) + `_INDEX.md`.
- **`lingua/`** (4 arquivos): conlang própria do jogo (`00-arquitetura.md`, `01-fonologia.md`, `02-lexico-semente.md`, `03-gramatica-nucleo.md`).
- **`propostas/`** (6 arquivos): cenas em proposta, não canônicas (ver Seção 2).

### `docs/book/` (11 arquivos, 3.783 linhas) - blueprints editoriais, não o livro em si

`BIBLE-V1-*` (Capa, Prefácio, Índice, Estrutura, Apêndice G): blueprint de TOC para o Volume 1 "Bíblia Worldbuilding". `BIBLE-V2-*` (Capa, Índice, Prefácio, Estrutura, Apêndice H): blueprint do Volume 2 "Antologia Narrativa", que consolida os 14 contos de `docs/narrative/deep/antologia/`. **FATO**: `docs/book/BIBLE-V1-STRUCTURE.md:3` e `docs/book/BIBLE-V2-STRUCTURE.md:3` dizem explicitamente "**Não é livro redigido; é blueprint de organização**" - ou seja, o "livro" citado em `inicial.md` como plano de licenciamento comercial ainda não tem texto final, só a estrutura editorial.

### `docs/specs/` (9 arquivos, 565 linhas) - specs técnicas de asset visual por personagem

`character-spec-{bento-requiem,caua-volt,dante-grid,gus,iara-lumen,jaci-proxy,linda-siren,sterling-locke}.md` + `_INDEX.md`. Ver Seção 3: todas as 8 specs de personagem trazem, na linha 1 do título, "Especificação Técnica de Asset **3D**", e um aviso logo na linha 3 dizendo que essa parte é histórica pós-pivô para 2D.

### `docs/art/` (5 `.md` + 5 PNGs de validação, 726 linhas)

`style-guide.md` (guia de estilo visual, com bloco extenso "Histórico" da era 3D), `sprites-inventory.md`, `props-inventory.md`, `vfx-combate-familias.md`, `characters/gus.md` (sheet de produção do protagonista). `_validacao/` tem 5 PNGs de teste de rotação de sprite do personagem Bento.

### `docs/qa/` (1 arquivo, 108 linhas)

Só `playtest_plan_vs.md` ("Proposta para ratificação", não canônico).

### `docs/_secret/` (2 arquivos, 46 linhas)

`cosmologia-origem-notas-secret.md` (notas de processo sobre a cosmologia de origem, explicitamente marcado "PRIVADO, gitignored, NUNCA publicar" em `docs/_secret/cosmologia-origem-notas-secret.md:1`) e `easter-eggs-manual.md`.

### `docs/audio/` e `docs/production/`

**FATO**: ambos os diretórios existem mas estão **vazios** (`find docs/audio -type f` e `find docs/production -type f` não retornam nenhum arquivo). Não há doc de áudio nem de produção fora do que está em `docs/design/producao/`.

### `resources/` (207 arquivos `.md`, 4.794 linhas, mais ativos binários)

- **`dialogues/`** (4 arquivos `.dlg.txt`): roteiro de diálogo em formato-texto próprio (ver Seção 6).
- **`translations/`** (3 arquivos): `pt_br.md` (locale primário, formato `## CHAVE_UPPER_SNAKE`), `en_intl.md`, `README.md`.
- **`prompts_images/`** (204 arquivos `.md`): prompts de geração de imagem por categoria (arquitetura, consumíveis, ambientes, fauna/flora, feitos, hardware_triad, tableaus históricos, logos/glifos, props, cartas rúnicas, texturas, tradições, ui_frames, veículos, VFX de glitch). Ver Seção 5: são PROMPTS, não ativos prontos.
- **`images/`** (79 PNGs em 11 subpastas + 1 solto): amostras de arte já geradas (arquitetura, consumíveis, fauna, flora, logos/glifos, props, cartas rúnicas, tradições, ui_frames, veículos) + `card-frame-tests/` (24 PNGs de teste de moldura de carta).
- **`sprites/`** (893 PNGs em 88 pastas de 1º nível): ver Seção 5.
- **`glb/`** (8 arquivos `.glb`, 402 MB): modelos 3D. Ver Seção 3 - contradiz a afirmação de que foram deletados.
- **`vfx/`** (281 arquivos): boot pixel, testes de transição.
- **`livros/`** (75 PDFs + 9 HTML + 2 bancos vetoriais Lance): material de referência de línguas élficas (Quenya/Sindarin) usado como insumo de RAG para a conlang do jogo (`docs/narrative/lingua/`). `rag/` e `rag_elvish/` são bancos vetoriais Lance (902 MB + 13 MB, formato `.lance`/`.txn`/`manifest`), não documentação legível diretamente.

### `assets/` (repositório de áudio final)

`assets/music/` (2 arquivos MP3: `Arena_GusWorld.mp3`, `cidade_tema_provisorio.mp3`) e `assets/sfx/` (6 arquivos WAV, todos com sufixo `_provisorio` exceto os dois de hit). Ver Seção 5.

---

## Seção 2 - Status de canonicidade

**Método**: `grep -rn "^\*\*Status" docs --include="*.md"` mais inspeção direta dos nomes de arquivo com `proposta`/`PROPOSTA`/`analise`/`análise`. O corpus usa cabeçalho `**Status:** ...` de forma consistente no início de cada documento de design/narrativa - é uma convenção real do projeto, não inferência minha.

### Documentos que se autodeclaram CANÔNICOS (lista, com citação)

- `docs/design/pillars.md:3` - "Revisão 1 [...] Validado ponto-a-ponto. **Canônico.**"
- `docs/design/mecanicas/combat.md:3` - "Canônico. Decisões ratificadas [...] 2026-05-26."
- `docs/design/mecanicas/puzzle-gambito.md:2` - "Canônico. Ratificado Sprint 2 W2 2026-06-03."
- `docs/design/mecanicas/knowledge-progression.md:2` - "Canônico. Ratificado Sprint 2 W2 2026-06-03."
- `docs/design/mecanicas/core-loop-exploracao.md:2` - "Canônico. Ratificado Sprint 2 W2 2026-06-03."
- `docs/design/mecanicas/onboarding-vs.md:2` - "Canônico. Ratificado Sprint 3 W2 2026-06-03."
- `docs/design/mecanicas/locomotion.md:3` - "Canônico - decisão do líder 2026-06-22. [...] NÃO reabrir."
- `docs/design/mecanicas/economia.md:3` - "canônico (decisões ratificadas [...] 2026-05-30)."
- `docs/design/mecanicas/encontros-aleatorios.md:3` - "Canônico. As 4 decisões estruturais [...] 2026-08-12."
- `docs/design/levels/blockout-distritos-inferiores.md:3` - "Canônico (design). Ratificado Sprint 5 W3 2026-06-03."
- `docs/design/narrativa/dialogue-tree-npc-intro.md:2` - "Canônico. Ratificado Sprint 4 W2 2026-06-03."
- `docs/design/producao/ci-build-plan.md:3`, `docs/design/producao/plano_vs.md:3`, `docs/design/producao/raid-log.md:3` - "Canônico" (mas ver a ressalva de stack obsoleto abaixo).
- `docs/art/vfx-combate-familias.md:3` - "Status: Canônico. Decisões ratificadas [...] 2026-06-24."
- `docs/narrative/lore-bible.md:3` - "Revisão 1 [...] **Canônico.**"
- `docs/narrative/timeline.md:3` - "Canônico (expansão Bloco G)."
- `docs/narrative/factions.md:3` - "Revisão 1 [...] **Canônico.**"
- `docs/narrative/in-world-docs.md:3` - "Canônico - expansão Bloco G + Bloco H viralizado."
- `docs/narrative/deep/_INDEX.md:3` - "Canônico (camada deep-lore)."
- `CHARS.md:2` (raiz do repo) - "Canônico. Imutável sem aprovação do criador supremo."

### Documentos explicitamente PROPOSTA / rascunho / não-canônico (lista, com citação)

- `docs/design/propostas/copy-stubs-combate.md:4` - "**PROPOSTA. NADA AQUI É CANON.**"
- `docs/design/mecanicas/modos-morte.md:3` - "PROPOSTA - spec fechada [...], aguardando canonização." **RESOLVIDO em 24/08/2026** (`TODO.md`, item `G3`): §1 a §5 do documento passaram a CANÔNICO; só o §6 (plano de implementação) segue morto, e agora como REVOGADO, não mais como proposta. Citação mantida porque descreve o estado real do documento no momento do levantamento (21/08/2026), e o levantamento inteiro é datado por natureza (linha 4 acima).
- `docs/design/mecanicas/cartas-spec-dados.md:3` - "PROPOSTA - [...] NÃO implementada."
- `docs/design/mecanicas/cartas-technomagik.md:3` - "PROPOSTA [...] aguarda revisão."
- `docs/design/mecanicas/cartas-spec-logica.md:3` - "PROPOSTA [...] Documento de design de LÓGICA [...] Não é código."
- `docs/design/mundo-topologia.md:3` - "PROPOSTA (decisões do criador, brainstorm interativo 2026-07-12)."
- `docs/qa/playtest_plan_vs.md:2` - "Proposta para ratificação."
- `docs/narrative/propostas/*-PROPOSTA.md` (6 arquivos, nome de arquivo já autodeclara).
- `docs/design/brainstorm-backlog.md:3` - "NADA aqui é canon fechado ainda: são sementes para brainstorm dirigido futuro."

### Zona intermediária (canon parcial / mistura de status dentro do mesmo arquivo)

Vários documentos misturam trechos canônicos com trechos em aberto, **dentro do mesmo arquivo**, com cabeçalho `**Status:**` repetido seção a seção:
- `docs/design/mecanicas/economia.md` tem 5 cabeçalhos `**Status:**` diferentes em linhas 3, 85, 117, 155, 223 - cada bloco de regra econômica com seu próprio grau de ratificação.
- `docs/design/mecanicas/cartas-numeros-proposta.md:3` - "os 4 pontos estruturais [...] foram FECHADOS [...] O restante [...] segue PROPOSTA `//PLAYTEST`."
- `docs/design/mecanicas/deck-mao-sistema.md:3` - "ARQUITETURA FECHADA [...] Números exatos [...] = `//PLAYTEST`."
- `docs/design/mecanicas/cartas-comuns-statlines.md:3` - "BASELINE aprovado [...] como ponto de partida do playtest N=3 [...] Nomes = PROVISÓRIOS."
- `docs/design/mecanicas/battle-screen.md:3` e `:246` e `:250` - decisões macro ratificadas, mas com pelo menos 3 camadas de atualização incremental datadas separadamente.
- `docs/narrative/propostas/disputas-linguagens-farpas-PROPOSTA.md:72` - "Cinco fechadas, **duas EM ABERTO**."

### Convenção de marcação de número não-fechado

O corpus usa consistentemente o marcador `//PLAYTEST` (comentário de código emprestado para prosa) para sinalizar "este número é afinável, a estrutura é que é canon" - visto em `cartas-numeros-proposta.md:3`, `deck-mao-sistema.md:3`, `cartas-comuns-statlines.md:2`. **[INFERÊNCIA]**: isso sugere disciplina real de separar "decisão estrutural" (canon) de "valor numérico" (ajustável por playtest), o que é uma prática de design saudável, mas também significa que **nenhuma tabela de números deste corpus pode ser tratada como dado final de balanceamento** sem checar se está sob esse marcador.

### Nota sobre `docs/design/producao/*` e a palavra "Canônico"

**FATO**: `ci-build-plan.md:3`, `plano_vs.md:3` e `raid-log.md:3` se autodeclaram "Canônico", mas os próprios arquivos trazem, poucas linhas abaixo, uma "NOTA DE STACK" dizendo que descrevem um pipeline **Godot 4 + C# .NET 8 AOT já aposentado** (`docs/design/producao/ci-build-plan.md:6`, `docs/design/producao/plano_vs.md:5`). Ou seja: "canônico" aqui qualifica a **filosofia/conteúdo de produção** (RAID, MVV, exit criteria), não os comandos técnicos citados no corpo do texto, que os próprios arquivos already marcam como obsoletos. Ver Seção 3 e Seção 7 para o alcance dessa obsolescência.

---

## Seção 3 - Contradições com "jogo 2D pixelado" (PRIORITÁRIO)

### 3.0 O fato mais importante desta seção: o corpus já registra um pivô 3D→2D, mas de forma incompleta e com pelo menos 1 contradição factual de ativo

**FATO**: pelo menos 15 arquivos do corpus (`docs/art/style-guide.md`, `docs/art/characters/gus.md`, `docs/design/producao/art-spike-protocol.md`, as 8 specs em `docs/specs/*.md`, `docs/design/mecanicas/battle-screen.md`, `docs/narrative/deep/characters/jaci-proxy.md`) contêm a frase quase idêntica **"Visual vigente: 2D pixel-art via PixelLab (pivô 3D→2D, ver ADR-008/ADR-010 e CLAUDE.md)"**, por exemplo em `docs/specs/character-spec-gus.md:3` e `docs/specs/_INDEX.md:3`. Ou seja, para uma fatia grande do corpus, a resposta a "é 3D ou 2D?" já está resolvida no texto: **2D pixel-art**, e o material 3D foi deliberadamente movido para blocos "Histórico" dentro dos próprios arquivos (ex.: `docs/art/style-guide.md:221` `<summary>Histórico - spec 3D (era Godot, superada)</summary>`).

**FATO GRAVE (achado prioritário)**: os arquivos ADR-008 e ADR-010, citados dezenas de vezes como a fonte de autoridade desse pivô, **não existem em lugar nenhum do repositório**. `find . -iname "ADR-*.md"` não retorna nenhum resultado, e `docs/tech/` (caminho citado literalmente em `docs/design/producao/art-spike-protocol.md:59` como `../../tech/adr/ADR-008-repivot-qt-to-__DEP_REMOVIDA__.md`) não existe (`ls docs/tech` = "Arquivo ou diretório inexistente"). Busquei todos os números de ADR citados no corpus e nenhum arquivo `ADR-XXX` existe: `ADR-001, 002, 003, 004, 005, 006, 008, 010, 014, 015, 016, 017, 019, 020, 021` são citados por nome mas nenhum tem arquivo correspondente. O `CLAUDE.md` atual do repo (13 linhas, lido por inteiro) também **não menciona** ADR-008, ADR-010 nem a seção "Decisões fechadas: Visual" que `docs/art/style-guide.md:5` cita como fonte. `ROADMAP.md`, citado dezenas de vezes (ex. `docs/design/mecanicas/dialogue-tree-npc-intro.md:115`, `docs/narrative/diary/ui-spec.md:411`) como onde ficam os detalhes de engine, também **não existe** no repositório (`find . -iname "ROADMAP.md"` vazio). **[INFERÊNCIA]**: isso sugere que o pivô 3D→2D e a decisão de engine (`C++20 + __DEP_REMOVIDA__`, provavelmente SDL, ver nota abaixo) aconteceram numa iteração anterior do projeto cujos artefatos de decisão (ADRs, ROADMAP, e o próprio código em `GusEngine/`) foram perdidos ou não migrados para este estado do repositório - coerente com o fato dado na tarefa de que o git não tem nenhum commit.

**FATO**: o código-fonte `GusEngine/` é citado 14 vezes no corpus (ex. `docs/design/mecanicas/cartas-spec-dados.md:12-13`, referenciando arquivos exatos como `GusEngine/domain/include/gus/domain/combat/combat_records.hpp`) como se existisse e fosse consultável, mas **não existe nenhum diretório `GusEngine` neste repositório** (`find . -iname "GusEngine*"` vazio) - condizente com o fato dado na tarefa ("não existe uma linha de código").

**FATO - artefato de redação a registrar**: 26 arquivos contêm a string literal `__DEP_REMOVIDA__` no meio de nomes técnicos (`__DEP_REMOVIDA__Renderer`, `__DEP_REMOVIDA__VIDEODRIVER=dummy` em `docs/design/producao/plano_vs.md:5`, `C++20 + __DEP_REMOVIDA__` em dezenas de lugares). Pelo padrão de uso (ex. `docs/design/producao/plano_vs.md:5` cita literalmente `__DEP_REMOVIDA__VIDEODRIVER=dummy`, que é a sintaxe real da variável de ambiente `SDL_VIDEODRIVER=dummy`), **[INFERÊNCIA]** essa string parece ser um placeholder que substituiu o nome de uma dependência técnica (aparentemente "SDL") em todo o corpus, possivelmente por uma ferramenta de sanitização. **FATO curioso que enfraquece essa inferência**: a mesma string `__DEP_REMOVIDA__` também aparece corrompendo palavras sem relação nenhuma com engines, como `Sto__DEP_REMOVIDA__ight Archive` em `docs/book/BIBLE-V1-APENDICE-G.md:152` (claramente deveria ser "*Stormlight* Archive", de Brandon Sanderson) e `www.__DEP_REMOVIDA__amer.com` em `docs/design/research/game-design-refs-2026-07.md:49` (claramente deveria ser "pcgamer.com") e `core::__DEP_REMOVIDA__` em `docs/narrative/gus-apartes-c-arcane.md:83` (no contexto Rust, muito provavelmente "core::fmt"). Não investiguei a causa da ferramenta de sanitização (fora do escopo deste inventário), mas **registro como achado de integridade do corpus**: em pelo menos 3 pontos a mesma marca de redação corrompeu texto que não tinha nenhuma relação com a dependência técnica supostamente redigida, o que é sintoma de um `sed`/regex amplo demais rodado sobre o corpus inteiro sem revisão dos falsos-positivos.

### 3.1 Contradição textual explícita: `docs/design/pillars.md` (canônico) nunca foi atualizado para o pivô

**FATO**: `docs/design/pillars.md:27` (documento canônico, "Revisão 1", 2026-05-15, sem nenhuma marca de revisão posterior) diz literalmente: **"Não é fotorrealista. Cel-shaded 3D low-poly, paleta restrita (style guide define)."** Esta é a frase citada na tarefa.

**FATO**: fiz `grep -n "pixel\|ADR\|pivot\|pivô\|PixelLab" docs/design/pillars.md` no arquivo inteiro e **não há nenhuma ocorrência** dessas palavras. Ou seja: ao contrário de `docs/art/style-guide.md` e das 8 specs de `docs/specs/`, que trazem no topo um aviso explícito de pivô 3D→2D, **`pillars.md` - que é o documento mais citado como fonte de verdade de todo o corpus (toda feature "responde: qual pillar serve?", linha 5) - não tem NENHUMA menção ao pivô e continua descrevendo o jogo como 3D**, num trecho que ele próprio marca como "Canônico" sem ressalva.

**FATO**: `docs/design/pillars.md:176` também descreve o hub central como tendo "Circuito impresso 3D" (isso é lore/estética de cenário, não necessariamente indicativo de pipeline de renderização - marco como caso ambíguo, não contradição forte).

### 3.2 Contradição de ativo real: os arquivos `.glb` que a doc diz terem sido deletados ainda existem no disco

**FATO**: `docs/design/mecanicas/battle-screen.md:138` diz: **"NAO ha bake 3D: o jogo e 2D-only (style-guide, ADR-008); o 3D era so ferramenta de bake e foi aposentado no pivot (**os .glb do art-spike foram deletados**)."**

**FATO**: `resources/glb/` contém **8 arquivos `.glb`, totalizando 402 MB**, com datas de modificação recentes (`brunus.glb`, `gus_com_antena_aparelho.glb`, `gus.glb`, `Gus.glb`, `Gus_movimento.glb`, `heliaco_vyr.glb`, `pyotor.glb`, `yakov.glb` - a mais recente datada de 25/07, ou seja depois de qualquer uma das datas de ratificação de pillars/style-guide citadas no corpo do texto). Isto é uma **contradição direta e verificável entre o que a documentação afirma (`.glb` deletados) e o que existe fisicamente no repositório (`.glb` presentes e pesando 402 MB)**.

### 3.3 Documentos canônicos que ainda descrevem câmera/navegação 3D, sem nota de pivô

- `docs/design/mecanicas/core-loop-exploracao.md:95` (documento marcado **Canônico**, ratificado Sprint 2, linha 2): tabela de decisão **DA-1** diz "Câmera e navegação no overworld: **3/4 orbital + controle direto** - WASD/stick move Gus; câmera orbita por botão separado. Coerente com **'3D real'** e combat.md." **FATO**: rodei `grep -n "pixel\|ADR\|pivot\|pivô\|PixelLab" docs/design/mecanicas/core-loop-exploracao.md` e **não há nenhuma ocorrência** - este documento canônico, assim como pillars.md, não tem nenhuma nota de atualização para o pivô 2D, apesar de descrever explicitamente câmera orbital 3D como decisão fechada.
- `docs/design/research/game-design-refs-2026-07.md:5` (documento de pesquisa, não canônico per se, mas usado para calibração): "câmera 3/4 orbital 3D (ref. Chrono Trigger), visual low-poly cel-shaded" - descreve a mesma câmera 3D como referência de calibração de mercado.
- `docs/narrative/characters/party.md:25` e `:120`: rotula os links para `docs/art/characters/<nome>.md` como **"Cross-ref produção 3D"** e **"Sheets produção 3D"** - nomenclatura desatualizada (esses arquivos hoje descrevem pipeline PixelLab 2D, ver Seção 3.0), sem nota de correção.
- `docs/design/roster-analogos/03-tesla.md:39` e outras 15 fichas do roster-analogos (`01-faraday`, `02-maxwell`, `04-volta`, `05-einstein`, `06-newton`, `07-planck`, `08-mandelbrot`, `09-euler`, `10-godel`, `11-ada-lovelace`, `12-turing`, `13-von-neumann`, `14-giordano-bruno`, `15-john-dee`, `16-pythagoras`, `17-hayek`, `18-mises`, `19-menger`, `20-bastiat`, `21-helion-tusk`): todos os prompts de imagem já dizem explicitamente **"retrato pixel-art cel-shaded"** - ou seja, o roster-analogos (o material mais recente entre os citados, várias entradas sem data mas geradas depois do pivô) **já nasceu escrito em termos 2D pixel-art**, sem menção a mesh/tris/vertex. Isto é consistente com o pivô, não uma contradição.

### 3.4 Specs técnicas de personagem: pivô parcial, com lacunas próprias assinaladas

**FATO**: as 8 specs em `docs/specs/character-spec-*.md` têm todas o mesmo padrão: título continua "Especificação Técnica de Asset **3D**" (ex. `docs/specs/character-spec-gus.md:1`, `docs/specs/character-spec-sterling-locke.md:1`, `docs/specs/character-spec-jaci-proxy.md:1`), aviso de pivô na linha 3, e a spec técnica 3D inteira (proporção de mesh, budget de tris, prompt Midjourney "Full body 3D game asset sprite [...] Anime 3D style, cel-shaded [...] Clean isometric view, orthographic projection") preservada dentro de um bloco `<summary>Histórico</summary>` (ex. `docs/specs/character-spec-gus.md:29`). **Cada uma dessas specs assinala explicitamente, na própria linha, o que ainda falta decidir para a versão 2D** - por exemplo `docs/specs/character-spec-linda-siren.md:23`: "Como o efeito diegético 'alto-falantes pulsando com o áudio' (antes mesh deformer 3D) vira animação de sprite 2D: **pendente**." e `docs/specs/character-spec-sterling-locke.md:27`: "Como manter a 'quebra deliberada do cômico SD' (que dependia de silhueta 3D angular) legível em pixel-art [...]: pendente - provavelmente o ponto mais sensível de tradução 3D→2D deste elenco."

**FATO**: `docs/specs/_INDEX.md:9` confirma que a "spec 2D detalhada (resolução de sprite, nº de frames/direções, paleta indexada, prompt PixelLab por personagem) é **decisão pendente do criador supremo**, ainda não definida."

### 3.5 Menções de 3D em contexto de VFX/apresentação - já resolvidas no próprio texto (não-contradição, registro por completude)

- `docs/art/vfx-combate-familias.md:43` e `docs/design/producao/art-spike-protocol.md:6`: ambos afirmam explicitamente que o 3D é usado **só como ferramenta de bake** para gerar sprite 2D final, não como pipeline de renderização em jogo - coerente com 2D.
- `docs/design/research/game-design-juice-intent-2026-07.md:71,99` e `docs/design/research/game-design-refs-2026-07.md:12,36,53,143,147,156,164`: usam "3D" repetidamente como **contraste negativo** ("substituir VFX 3D caro por camadas de juice baratas em 2D/UI"), reforçando a decisão 2D, não contradizendo-a.

### 3.6 Resumo da seção (síntese, não fato isolado)

Juntando 3.0-3.5: o corpus **não é uniformemente 3D nem uniformemente 2D** - ele registra uma transição real (3D/Godot/C# → 2D pixel-art/C++/PixelLab), mas essa transição está **incompleta e mal propagada**: (a) o documento mais alto na hierarquia de canon (`pillars.md`) não foi tocado; (b) um documento canônico de mecânica central (`core-loop-exploracao.md`) descreve câmera 3D orbital como decisão fechada sem nota; (c) existe um ativo físico (`resources/glb/`, 402 MB) que contradiz uma afirmação textual de que foi deletado; (d) toda a cadeia de evidência da decisão (ADR-008, ADR-010, ROADMAP.md, `GusEngine/`) está ausente do repositório, então não há como auditar o pivô a partir apenas do que existe aqui.

---

## Seção 4 - Regras de jogo que viram dado/POCO (entidades e campos)

**Nota de leitura**: separo documentos que já trazem **esquema de campos explícito** (struct/record/enum/tabela de propriedades nomeadas) de documentos que descrevem a entidade só em prosa de design (sem schema). Vários esquemas citados abaixo são de um protótipo **C#/Godot já decomissionado** segundo os próprios docs (ver Seção 3/7) - marco isso em cada caso.

### Cartas / itens

- **`docs/design/mecanicas/cartas-spec-dados.md`** (PROPOSTA, não implementada, `:3`) define a entidade **`CardPhysicalState`** com esquema de campos explícito em C++: struct `CardPhysicalState` (`:147-172`, campos incluindo estado de origem ROM/EPROM/pirata, ciclos, déficit de bateria, flags `is_infected`/`is_diagnosed`), enums de suporte (`:98`), extensão aditiva a `struct Card` (catálogo, `:53`) e a `struct CardInstance` (posse, `:76-77`). Cita caminhos exatos de código que **não existem neste repo** (`GusEngine/domain/include/gus/domain/combat/combat_records.hpp:12`, `GusEngine/domain/include/gus/domain/deck/deck_records.hpp:13`).
- **`docs/design/mecanicas/combat.md:813-852`**: traz 5 `record struct` em C# (`StatusEffect`, `Card`, `ComboRecipe`, `PipelineSlot`, `IntentPreview`) - **FATO**: `combat.md:727` avisa que essa seção descreve "o protótipo C# + Godot [...] que morre no M8", ou seja este schema é do código antigo, não do C++ atual.
- **`docs/design/mecanicas/deck-mao-sistema.md:93-142`** ("Catálogo das cartas comuns - ESTRUTURA", fechada 2026-07-16) define a estrutura de catálogo de cartas comuns (arquétipo × 5 famílias) e os números baseline de aquisição/descarte, mas em prosa/tabela de design, não como struct de código.
- **`docs/design/mecanicas/cartas-comuns-statlines.md`**: statlines (Power/mana/velocidade) das 30 cartas comuns, com `**Status:**` de "velocidade = canon pétreo" vs. resto `//PLAYTEST`.
- **`docs/design/card-frame-spec.md`**: define a moldura visual da carta (não é schema de dado de jogo, é spec de asset gráfico).
- **`docs/design/roster-analogos/_IDS-CARTAS.md`** e **`_EFEITOS-ESCOLHIDOS.md`**: catálogo de IDs e efeitos das cartas "especiais" dos 21 mestres análogos (não inspecionei campo-a-campo, mas o nome do arquivo indica schema de ID + efeito).

### Personagens

- **`CHARS.md`** (raiz, canônico): tabela única com **189 linhas de tabela** (`grep -c "^|" CHARS.md`), esquema de campos explícito e documentado em `CHARS.md:8-13`: colunas Nome, Apelido/codinome, Características, 1ª aparição, Status (com 5 valores possíveis de enum: `✅ canônico`, `🟡 secundário`, `⚪ ambient`, `🔴 antagonista`, `💀 morto pré-jogo`). Este é o inventário mestre de personagens de todo o corpus.
- **`docs/specs/character-spec-*.md`** (8 arquivos): schema de asset visual por personagem (proporção, paleta, budget de tris - histórico 3D; prompt PixelLab - pendente, ver Seção 3.4).
- **`docs/narrative/characters/*.md`** (16 arquivos) e **`docs/narrative/deep/characters/*.md`** (28 arquivos): fichas narrativas de personagem (papel, arco, voz), em prosa, sem schema de campos tabular.

### Inimigos

- **`docs/design/mecanicas/combat.md`**: menciona `enemy_difficulty_constants.hpp` (`docs/design/mecanicas/encontros-aleatorios.md:3`) e `EnemyTemplate → CombatActor` como spawn-path, mas não encontrei, na varredura, uma tabela dedicada e completa de campos de inimigo (stats base, resistências) fora do que está embutido em `combat.md` e no bestiário narrativo abaixo.
- **`docs/narrative/diary/entries-fichas-bestiary.md`**: fichas de bestiário com campo "Ataques" listado (`:656`), mas é conteúdo de diário in-game (lore), não schema de engine.

### Diálogos

- **`resources/dialogues/*.dlg.txt`** (4 arquivos): formato-texto próprio (comentário de cabeçalho cita "ADR-014", `resources/dialogues/npc_intro_bertoldo.dlg.txt:5`) consumido por `DialogueRuntime`/`parse_text_to_dialogue_graph` - **novamente um caminho de código que não existe no repo**. O arquivo também documenta, no próprio cabeçalho (`:6-10`), que existiu um formato irmão `.dialogue` (sintaxe Godot) já removido do disco.
- **`docs/design/narrativa/dialogue-tree-npc-intro.md`** (canônico): blueprint/árvore de diálogo com escolhas e flags, é o "molde" de onde os `.dlg.txt` derivam.
- **`docs/narrative/guia-dialogos.md`**: guia de estilo de diálogo (voz, não schema).

### Mapas / mundo

- **`docs/design/mundo-topologia.md`** (PROPOSTA): 13 áreas do mundo aberto, esqueleto estrutural em prosa/tabela, sem schema de campo de tile/mapa.
- **`docs/design/levels/blockout-distritos-inferiores.md`** (canônico): grafo de nós do primeiro distrito (traçado 90x60, 16 fachadas) - descreve estrutura de nível em prosa/tabela de design, não schema de dado.

### Missões

- **`docs/design/mecanicas/missoes-cronometradas.md`**: missões de tempo real ponto-A→ponto-B, decisão de design fechada, sem schema de campos de missão explícito na amostra lida.

### Economia / save

- **`docs/design/mecanicas/economia.md`**: tabelas de fontes/sinks de crédito com valores exatos (`:77-94`, ex. "Encontro vencido: 8 cr × fórmula", "Recompensa de missão-capstone: 610/377/233/144 cr por dificuldade") - é a entidade "economia" com campos numéricos explícitos, mas em tabela de design, não struct de código.
- **`docs/narrative/diary/knowledge-gates.md:380-426`**: **schema de save explícito**, "Save schema (JSON versionado `save_version: 1`)" com exemplo de JSON a partir da linha 384. **FATO**: o próprio doc (`:376`) avisa que essa spec foi "escrita sobre o stack antigo Godot 4 + GDScript" e será "re-derivada pelo squad técnico na engine atual" - ou seja, o schema de save também tem uma camada de obsolescência declarada.

---

## Seção 5 - Ativos: o que existe de fato

### Sprites de personagem (`resources/sprites/`)

**FATO**: 88 diretórios de 1º nível, 893 arquivos PNG no total. **FATO**: apenas **7 personagens têm conjunto de animação completo** (subpasta `anims/` com múltiplos estados: `attack_melee`, `battle_idle`, `breathing_idle` com 4 direções, `caindo`, `cast`, `defend`, `hurt_magic`, `hurt_physical`, `ko`, `revive`, `run` com 4 direções, `victory`, `walk` com 4 direções) - `bento_requiem`, `caua_volt`, `dante_grid`, `iara_lumen`, `jaci_proxy`, `linda_siren` e `personagens_inspirados/gus` (o protagonista, com pasta própria fora do padrão dos demais). Isso bate com os 7 membros de party mais o protagonista que têm spec técnica em `docs/specs/` (excluindo o antagonista Sterling Locke, que tem spec mas **não tem pasta de sprite animado**).
- Os demais ~80 diretórios de personagem (a maioria dos ~85 citados na tarefa) contêm **apenas imagens estáticas** (retrato/pose única), sem pasta `anims/` nem `walk/` - **[INFERÊNCIA]**: consistente com serem NPCs secundários/análogos do roster que ainda não passaram pela produção de animação, e não com um erro de varredura (confirmei contagem de `anims`/`walk` = 6 pastas cada, mais o caso especial do Gus).
- `resources/sprites/icons-m5/` (pasta à parte, ícones de UI de batalha, não personagem): 5 ícones de família, 4 de intent, 3 de modificador, 15 de retrato, 20 de status, 5 de app_icon - **FATO**: o `REVISAO.html` da mesma pasta (`resources/sprites/icons-m5/REVISAO.html:6`) diz que são "35 assets gerados autonomamente no PixelLab (a melhor de 4 variações de cada)" - ou seja, **estes já são ativos prontos para consumo**, gerados pelo pipeline 2D vigente.
- `resources/sprites/models_frente/`, `resources/sprites/world/` (14 arquivos, distritos inferiores), `resources/sprites/personagens_inspirados/` (Brunus, Pyotor, Yakov, além do Gus) são pastas de categoria diferente de "1 personagem = 1 pasta".

### Imagens de referência (`resources/images/`)

**FATO**: 79 PNGs em 11 subpastas temáticas (arquitetura 9, card-frame-tests 24, consumíveis 7, fauna 6, flora 6, logos/glifos 7, props 10, cartas rúnicas 5, tradições 3, ui_frames 2, veículos 5) + 1 arquivo solto na raiz de `images/`. Estes **são ativos de imagem já gerados** (formato PNG pronto para uso), em contraste com a pasta seguinte.

### Prompts de imagem (`resources/prompts_images/`)

**FATO**: 204 arquivos, **todos em formato `.md`** (nenhum PNG dentro desta árvore) - são **texto de prompt para geração futura**, não ativo pronto. Confirma o padrão visto no roster-analogos (Seção 3.3): a produção de imagem deste projeto documenta o prompt em Markdown antes/ao lado de gerar o PNG.

### VFX (`resources/vfx/`)

281 arquivos em `boot_pixel/`, `boot_pixel_test/`, `transition_test/` (com subpastas `boot`, `concept_a_iris_scan`, `concept_b_scanline`) - não abri arquivo a arquivo, mas os nomes indicam sequências de teste de transição/boot já renderizadas.

### Cartas / runas

`resources/images/runic_cards/` (5 PNG prontos) + `resources/prompts_images/runic_cards/` (prompts, contagem incluída nos 204 acima) + `resources/images/card-frame-tests/` (24 PNG de teste de moldura, incluindo subpastas `cards/`, `lumkey/`, `recolor/`).

### Áudio (`assets/`)

**FATO**: `assets/music/` tem só **2 arquivos MP3** (`Arena_GusWorld.mp3`, `cidade_tema_provisorio.mp3`) e `assets/sfx/` tem **6 arquivos WAV** (`hit_digital_alt_provisorio.wav`, `hit_digital_provisorio.wav`, `menu_blocked_provisorio.wav`, `menu_click_provisorio.wav`, `menu_hover_provisorio.wav`, `ui_confirm_provisorio.wav`). **FATO**: 4 dos 6 SFX e 1 das 2 músicas trazem o sufixo `_provisorio` no próprio nome de arquivo - ou seja, a maioria do áudio existente **se autodeclara placeholder**, não asset final. Não há trilha para batalha em VFX de família nem para exploração além da cidade.

### Traduções (`resources/translations/`)

3 arquivos: `pt_br.md` (locale primário, formato `## CHAVE_UPPER_SNAKE` documentado em `resources/translations/pt_br.md:5`), `en_intl.md`, `README.md`. **FATO**: `resources/translations/pt_br.md:5` cita `MdTranslationLoader.cs` como o loader - **outra referência a código C# que não existe no repo atual** (mesma classe de achado da Seção 3/7).

### Diálogos (`resources/dialogues/`)

4 arquivos `.dlg.txt` (ver Seção 4) - conteúdo pronto no formato-texto que o corpus considera "fonte editável de verdade" (`resources/dialogues/npc_intro_bertoldo.dlg.txt:10`), mas cobrindo só 1 cena de intro NPC (3 arquivos de uma mesma cena "cena15" + 1 de intro de NPC "Bertoldo").

### Livros/RAG (`resources/livros/`)

75 PDFs + 9 HTML de gramática élfica (Quenya/Sindarin, em inglês, material de terceiros usado como referência para a conlang do jogo) + 2 bancos vetoriais Lance (`rag/` 902 MB, `rag_elvish/` 13 MB) com 452 arquivos internos `.lance`/`.txn`/`manifest` - infraestrutura de busca semântica para apoiar a escrita da lore/conlang, não ativo de jogo em si.

### Modelos 3D (`resources/glb/`)

8 arquivos `.glb`, 402 MB (ver Seção 3.2 - contradição com a doc que diz terem sido deletados).

---

## Seção 6 - HTML/RML/CSS: o que existe de fato

**FATO (resultado negativo honesto)**: `find . -iname "*.rml" -o -iname "*.rcss" -o -iname "*.css"` retorna **zero arquivos** em todo o repositório. **Não existe nenhum arquivo `.rml`, `.rcss` ou `.css`** neste corpus.

**FATO**: `find . -iname "*.html"` retorna **9 arquivos**, e nenhum deles é um template de UI de jogo:
- **8 arquivos são material de gramática élfica** (Quenya/Sindarin), em inglês, sob `resources/livros/elvish/en/`: `grammar_cases.html`, `grammar_mutations.html`, `grammar_pron_rek.html`, `grammar_quenya_past_tense.html`, `grammar_quenya_perfect_tense.html`, `grammar_quenya_pronouns.html`, `grammar_sindarin_past_tense.html`, `grammar_verbs.html`. Conferi o conteúdo de `grammar_cases.html:1-13`: é uma página estática de referência linguística ("The Sindarin case system"), material de terceiros (linguística élfica de Tolkien), usada como insumo de pesquisa para a conlang do jogo (`docs/narrative/lingua/`) - **não tem nenhuma relação com interface de jogo**.
- **1 arquivo é uma página de QA/preview de assets**: `resources/sprites/icons-m5/REVISAO.html`, com CSS inline (não arquivo `.css` separado) e HTML estático que carrega 35 PNGs de ícone lado a lado para conferência visual (`resources/sprites/icons-m5/REVISAO.html:1-6`). É uma ferramenta de revisão pontual do artista, não um template de tela de jogo, e não usa RmlUi/RCSS.

**Conclusão desta seção (resultado negativo honesto)**: a afirmação do líder de que "já tem alguns [html/rml/css] prontos no cwd" **não se confirma**. O que existe sob essas extensões é (a) livros de gramática élfica de terceiros e (b) uma página HTML solta de conferência de ícones. **Zero arquivos de interface de jogo em RML/RCSS/CSS existem no corpus**, apesar de "usaremos html/rml/css para criar formatações" ser citado como decisão em `inicial.md`, e apesar de o corpus de design (`docs/design/mecanicas/battle-screen.md`, `docs/design/research/game-design-juice-intent-2026-07.md`) já pressupor RCSS/`glintfx` em várias passagens técnicas (ex. `docs/design/mecanicas/modos-morte.md:160` cita `difficulty_menu_____.hpp/.cpp` como par de arquivo RCSS+código, e `docs/design/research/game-design-juice-intent-2026-07.md:124` descreve `@keyframes`/`transform` como "glintfx-nativo") - ou seja, **o vocabulário RCSS/glintfx já é usado nos textos de design como se a integração existisse, mas nenhum arquivo `.rcss`/`.rml` real foi encontrado**.

---

## Seção 7 - Lacunas

Lacunas identificadas por comparação entre o que um jogo deste porte (RPG turn-based + puzzle + aventura, single-player, engine própria) tipicamente precisa e o que a varredura encontrou. Cada item indica se é ausência total (não encontrei nada) ou cobertura parcial (encontrei fragmentos).

1. **Toda a cadeia de decisão técnica (ADR) está ausente.** Ver Seção 3.0: 15 números de ADR são citados por nome (`ADR-001` a `ADR-021`), zero arquivos existem. Sem eles, não há como auditar POR QUE o projeto pivotou de Godot/C# para C++/PixelLab, nem reconstituir as decisões de licenciamento (`ASSETS-LICENSE.md` cita `ADR-005` e `ADR-021` como a fonte do regime de licença dupla código/assets, nenhum dos dois existe).
2. **`ROADMAP.md` não existe** apesar de citado dezenas de vezes como onde vivem os detalhes de arquitetura de engine atual.
3. **`GusEngine/` (o código) não existe**, confirmando o fato dado na tarefa - mas isso significa que **toda struct/campo citado nas specs de carta (Seção 4) é uma referência a um arquivo fantasma**; a reconstrução do código terá que **inferir** os campos a partir da prosa dos `.md`, já que não há um `.hpp` real para conferir.
4. **`docs/audio/` e `docs/production/` estão vazios** - não há documento de direção de áudio (estilo musical, orçamento de faixas, ferramenta de composição) nem plano de produção fora do que está em `docs/design/producao/` (que é sobre o pipeline antigo Godot/C#, ver Seção 2).
5. **Não há spec de UI/HUD em formato consumível (RML/RCSS)** - ver Seção 6. Existe spec de UI em prosa (`docs/narrative/diary/ui-spec.md`, `docs/design/mecanicas/battle-screen.md`), mas nenhuma implementação nem template de marcação.
6. **Faltam esquemas de dado completos para "inimigo" e "mapa/tile"** como entidades de 1ª classe - encontrei citações de arquivos que os definiriam (`enemy_difficulty_constants.hpp`, `EnemyTemplate`) mas nenhum documento dedicado com tabela de campos de inimigo comparável ao que existe para carta (`cartas-spec-dados.md`) ou personagem (`CHARS.md`).
7. **Áudio quase inteiramente placeholder**: 2 músicas (1 provisória) e 6 SFX (4 provisórios) para um jogo de 4-8h (`docs/narrative/lore-bible.md:5`) com "5 famílias" de combate e "13 áreas" de mundo (`docs/design/mundo-topologia.md:1`) - a cobertura de trilha por área/família de combate está, pelo que a varredura encontrou, **não coberta**.
8. **O "livro" com licença própria (citado em `inicial.md`) só existe como blueprint editorial**, não como texto redigido - ver Seção 1 (`docs/book/BIBLE-V1-STRUCTURE.md:3`, `BIBLE-V2-STRUCTURE.md:3`, ambos dizem explicitamente "Não é livro redigido").
9. **Save schema depende de stack aposentado**: o único schema de save encontrado (`docs/narrative/diary/knowledge-gates.md:380`) é explicitamente rotulado como escrito para o stack Godot/GDScript antigo e "a re-derivar" - não há um schema de save já escrito para C++20.
10. **Sprites animados cobrem só 7 de ~85 personagens** (Seção 5) - toda a produção de animação de NPCs/análogos além da party principal e do protagonista está pendente.
11. **Nenhum arquivo de teste, CI real ou build system foi encontrado** neste corpus documental (coerente com "zero linhas de código"), mas os documentos de produção (`ci-build-plan.md`, `plano_vs.md`) descrevem um pipeline de CI para uma stack (Godot/.NET) diferente da atual (C++/CMake) - o plano de CI vigente (CMake+Ninja+CTest, citado en passant em `docs/design/producao/plano_vs.md:5`) não tem documento dedicado próprio, só a menção corretiva dentro do doc antigo.
12. **A licença dividida código/assets está declarada mas as duas ADRs que a sustentam (ADR-005, ADR-021) não existem** - ver item 1. `ASSETS-LICENSE.md` no entanto está presente e legível na raiz do repo, então a REGRA em si (Apache 2.0 para código, split de assets por data de corte) está registrada, só falta o histórico de decisão.

---

## Fim do relatório

Este documento foi escrito integralmente pelo agente `technical-writer`, em modo somente-leitura, sem nenhuma alteração dentro do projeto GusWorld. Caminho do arquivo: `/var/tmp/builds/claude-1000/-home-petrus-IDrive-Documentos-projetos-claudebrain-Projects-GusWorld/ead63a0b-09c1-4057-9d86-648ceaf458c8/scratchpad/mapa-corpus-gusworld.md`.
