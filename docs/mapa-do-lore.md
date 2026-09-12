# Mapa do lore de GusWorld

> **O que este documento é:** um mapa. Ele diz onde cada assunto do mundo de GusWorld mora no repositório, organizado pelo assunto que você procura (gente, lugar, língua, sistema...), não pela pasta onde o arquivo foi salvo. A partir daqui, qualquer tópico do mundo se acha em no máximo dois saltos.
>
> **O que este documento não é:** não é fila de trabalho. Não tem coluna de status, caixa de marcar, prioridade ou onda; isso vive só em `TODO.md`. Não é um segundo inventário: onde já existe um documento soberano (`CHARS.md`, `PLACES.md`, `docs/narrative/factions.md`...), este mapa aponta para ele e não repete o conteúdo. Também não é auditoria de canonicidade: um documento citado aqui pode estar `Canônico`, `PROPOSTA` ou em brainstorm; o `**Status:**` de cada arquivo é dele mesmo, leia-o lá.
>
> **Como usar:** ache seu assunto na lista de eixos abaixo, siga o caminho. Quando uma pasta já tem índice próprio (`_INDEX.md`), este mapa aponta para o índice em vez de repetir a lista de arquivos dele.

## Documentos-âncora (valem para o mundo inteiro, não só para um eixo)

- **`sinopse.md`** (raiz do repo): a síntese cerimonial de todos os eixos canônicos, o ponto de partida mais curto para entender o mundo inteiro.
- **`docs/narrative/lore-bible.md`**: a bíblia central da lore, o documento mais denso e mais citado como cross-ref por todo o resto do corpus.
- **`docs/design/pillars.md`**: os cinco pilares de design; toda feature de jogo (e boa parte da lore aplicada a mecânica) responde "qual pillar serve?".
- **`docs/design/gdd.md`**: o GDD de uma página, visão executiva do jogo.
- **`CHARS.md`** e **`PLACES.md`** (raiz do repo): os dois inventários soberanos, imutáveis sem aprovação do líder. Todo eixo de "gente" e "lugares" abaixo cede a eles.

---

## 1. Gente

Quem povoa o mundo: party, aliados, antagonistas, NPCs, e os 21 "análogos" de figuras históricas reais reimaginadas como personagens.

- **Soberano: `CHARS.md`** (raiz) - inventário canônico de todo personagem nomeado do canon, com tabela de status. Todo doc abaixo cede a ele.
- `docs/narrative/vozes-party.md` - guia de voz de cada membro da party (como cada um fala).
- `docs/narrative/characters/` (16 arquivos) - ficha narrativa por personagem de party/antagonista (papel, arco, voz); inclui 4 "contos" de personagens secundários (Brunus, Pyotor, Yakov).
- `docs/narrative/deep/characters/` (28 arquivos) - fichas profundas de deep-lore, incluindo 1 por "mestre" análogo do roster (Faraday, Turing, Ada Lovelace etc.).
- `docs/narrative/deep/antologia/` (14 arquivos) - um conto in-character por membro de party/antagonista/figura histórica; é a lore de personagem em prosa, não em ficha.
- `docs/narrative/deep/antagonists/` (3 arquivos) - dossiês profundos dos antagonistas (Patch Zero, Sterling Locke, e o NPC-antagonistas geral).
- `docs/design/roster-analogos/` (29 arquivos) - as 21 fichas dos "mestres" análogos (Faraday a Helion Tusk) mais os arquivos de metodologia: `OBRA-DE-FICCAO-E-METODOLOGIA.md` (o aviso de ficção sobre reimaginar pessoas reais, citado no `README.md`), `_IDS-CARTAS.md`, `_EFEITOS-ESCOLHIDOS.md`, `_RECONCILIACOES.md` (cross-ref com as cartas especiais desses personagens), e a subpasta `8values-engine/` (4 arquivos de atribuição/metodologia do teste político usado como referência para os análogos economistas).
- Ver também `docs/narrative/deep/_INDEX.md` para a camada deep-lore inteira, da qual `characters/`, `antologia/` e `antagonists/` são só uma parte.

## 2. Lugares

As treze áreas do mundo, cidades, dungeons, áreas secretas e interiores.

- **Soberano: `PLACES.md`** (raiz) - inventário canônico de todo lugar nomeado.
- `docs/design/mundo-topologia.md` - o esqueleto estrutural: o grafo das 13 áreas do mundo e como se conectam. É a fonte da CONTAGEM e da geografia macro (⚠️ em edição concorrente no momento deste mapa; confira o `**Status:**` do próprio arquivo antes de citar).
- `docs/narrative/environments/` (9 arquivos, índice em `_INDEX.md`) - 1 documento por bioma jogável (cidade, Selve Sombria, Catedrais, Dutos, Mirage, Periferia, Zona do Silêncio, Selve Profunda), traduzindo lore em espaço jogável.
- `docs/narrative/deep/settings/` (8 arquivos, dentro de `docs/narrative/deep/`) - a mesma lista de 8 biomas, em versão deep-lore expandida.
- `docs/design/dungeons/` (23 arquivos) - dungeons e seus enigmas centrais: 8 dungeons na raiz da pasta (ex.: `catedrais-cordas-emaranhadas.md`, `periferia-pontes-de-konigsberg.md`, `subsolo-vance-disco-de-ouro.md`) mais a subpasta `areas-secretas/` (15 arquivos, índice em `_INDEX.md`) com as áreas secretas numeradas 01 a 13.
- `docs/design/levels/blockout-distritos-inferiores.md` - o grafo de nós do primeiro distrito jogável (blockout).
- As **soluções** dos enigmas de dungeon vivem cifradas em `docs/_secret/dungeons/` (ver eixo 12, "A fronteira do cifrado").

## 3. Tempo

Eras, cronologia, linha do tempo.

- **Soberano: `docs/narrative/timeline.md`** - a linha do tempo canônica: pré-história (Era Lendária) mais três eras estruturais (Pré-Código / Boom Técnico ou "Compilador" / Sterling).
- `docs/narrative/deep/eras/` (25 arquivos) - expansão deep-lore de cada era: `era-1-pre-codigo.md` com sua própria subpasta de 18 capítulos (`era-1-pre-codigo/capitulo-01...10`), `era-2-boom-tecnico.md`, `era-3-sterling.md`, `transicoes-entre-eras.md`, `cosmologia-origem-deep.md`.
- Data exata do colapso da Era Lendária e o que a dungeon final da Selve Profunda revela sobre ela ficam cifrados - ver `docs/_secret/cosmologia-origem-notas-secret.md` no eixo 12.

## 4. Língua e escrita

A conlang do mundo e as linguagens diegéticas de magia.

- `docs/narrative/lingua/` (4 arquivos: `00-arquitetura.md`, `01-fonologia.md`, `02-lexico-semente.md`, `03-gramatica-nucleo.md`) - a língua-mãe **Sylvarin** ("a fala da Selve"), em desenvolvimento por etapas. `00-arquitetura.md` é o ponto de entrada.
- `docs/narrative/deep/magic/4-linguagens-deep.md` - a bíblia comparativa das quatro linguagens vivas da Era 3 usadas como sistema de magia formal (C-Arcane, Asmódico, Pythia, Óxido) mais a anti-linguagem corporativa DRE. É o documento mais próximo de um "soberano" para esse sub-tópico.
- `docs/narrative/deep/magic/glyph-token-conjuro-codex-deep.md` - o códex de glifos/tokens/conjuros.
- `docs/narrative/farpas-linguagens.md` - as rivalidades e disputas entre falantes das diferentes linguagens.
- `docs/narrative/deep/eras/era-1-pre-codigo/capitulo-04-linguagem-asmodico-cripto-glifos.md` - a origem histórica do Asmódico.
- A inscrição da porta que grava números na língua (decisão `D23`, ver `TODO.md`) fica cifrada em `docs/_secret/dungeons/d23-inscricao-porta.md`.

## 5. Fauna e flora

⚠️ **Eixo com cobertura fraca** - não há bestiário de mundo nem catálogo de flora dedicado; o que existe:

- `docs/narrative/diary/entries-fichas-bestiary.md` - o bestiário mecânico do Diário do Gus (fichas de inimigos turn-based, reveladas por Knowledge).
- `docs/design/mecanicas/estado-envenenado-fibortorrinco.md`, `docs/design/mecanicas/missao-unhas-fibortorrinco.md`, `docs/design/mecanicas/unha-veneno-fibortorrinco-item.md` - a única criatura nomeada com mecânica própria (o Fibortorrinco), sua missão e o item derivado dela.
- `docs/narrative/deep/eras/era-1-pre-codigo/capitulo-09-02-vestigios-biologicos-sementes-fungos-flora.md` - os vestígios biológicos (sementes, fungos, flora) da Era Pré-Código.
- A flora tecnorgânica da Selve Sombria (natureza que segue matemática rígida, não caos) aparece embutida nos docs de ambiente do eixo 2 (`docs/narrative/environments/02-selve-sombria.md` e o par deep-lore), não em documento próprio.

## 6. Facções e ordens

- **Soberano: `docs/narrative/factions.md`** - as facções canônicas do mundo.
- `docs/narrative/deep/factions/` (7 arquivos: `cult-mirage.md`, `facoes-menores.md`, `fir.md`, `ordem-recursiva.md`, `pelicano-branco.md`, `sterling-corp.md`, `underground-silencio.md`) - expansão deep-lore de cada facção.
- `docs/narrative/tradicoes-cultura.md` - festividades por facção, calendário in-world, costumes, rituais, comida, vestuário; como a cultura de cada facção se mostra em ambiente.

## 7. Objetos

Cartas, itens, artefatos. ⚠️ **Sem inventário soberano** equivalente a `CHARS.md`/`PLACES.md` - o catálogo está fragmentado:

- `docs/design/mecanicas/cartas/` (28 arquivos, índice em `_INDEX.md`) - as 25 fichas de carta individuais extraídas do projeto anterior, mais `_vocabulario.md` (glossário compartilhado) e `FORMATO.md` (o formato de ficha).
- `docs/design/card-frame-spec.md` - a moldura visual da carta.
- No nível de `docs/design/mecanicas/` (fora da subpasta `cartas/`): `cartas-comuns-statlines.md`, `cartas-hardware-pirataria-energia.md`, `cartas-numeros-proposta.md`, `cartas-spec-dados.md`, `cartas-spec-logica.md`, `cartas-statlines-rascunho.md`, `cartas-technomagik.md` (cartas, poções e itens do sistema TechnoMagik) - specs e propostas de carta que ainda não migraram para dentro da subpasta `cartas/`.
- Itens não-carta: `docs/design/mecanicas/capacitor-item.md`, `docs/design/mecanicas/comidas-ingredientes-craft.md`, `docs/design/mecanicas/unha-veneno-fibortorrinco-item.md` (cross-ref eixo 5).
- Cross-ref de IDs e efeitos das cartas especiais dos 21 mestres análogos: `docs/design/roster-analogos/_IDS-CARTAS.md` e `_EFEITOS-ESCOLHIDOS.md` (eixo 1).

## 8. Sistemas do mundo

Magia, tecnologia, economia, energia - as regras que fazem o mundo funcionar.

- `docs/design/pillars.md` (documento-âncora, ver topo) valida toda decisão de sistema.
- **Magia/tecnomagia:** `docs/design/technomagik.md` (registro de brainstorm, ainda não é canon final) e `docs/narrative/deep/magic/natureza-matematica-rigida-deep.md` (a natureza segue matemática rígida, pillar 2). Ver eixo 4 para as quatro linguagens que São o sistema de magia.
- **Combate:** `docs/design/mecanicas/combat.md` (canônico), `combat-flavor.md`, `battle-screen.md`, `redline-medidor-risco.md` (medidor de risco), `spec-provocar-soft-enrage-criterio-cap.md`.
- **Exploração:** `core-loop-exploracao.md`, `locomotion.md`, `mini-mapa.md`, `encontros-aleatorios.md`, `missoes-cronometradas.md`.
- **Progressão e cartas em jogo:** `knowledge-progression.md`, `deck-mao-sistema.md`, `conquistas.md`.
- **Morte e dificuldade:** `modos-morte.md` (fail-state escalonado por dificuldade).
- **Puzzle:** `puzzle-gambito.md`, `terminal-estetica.md`.
- **Economia:** `economia.md`, `proposta-economia-comedimento.md`.
- **Save e energia:** `save-por-local.md`, `proposta-intensidade-campo-save.md`, `stamina.md` (a mecânica de Estamina).
- (todos os arquivos acima sem caminho completo estão em `docs/design/mecanicas/`.)
- `docs/design/propostas/copy-stubs-combate.md` - stubs de texto de sabor para combate (não-canônico).

## 9. História e arcos

O arco principal, os arcos de cada personagem, o que veio antes e depois dele.

- **Soberano: `docs/narrative/arco-principal.md`.**
- `docs/narrative/foreshadowing.md` - os ganchos plantados e onde pagam.
- `docs/narrative/deep/antologia/` (eixo 1) - os arcos individuais em prosa, um conto por personagem.
- `docs/narrative/deep/ontologia/` (4 arquivos: `cosmologia-formal-deep.md`, `leitmotivs-deep.md`, `leitmotivs-musicais-detalhados.md`, `tech-3-eras-deep.md`) - a camada de sentido por trás da história: cosmologia formal e os leitmotivs musicais associados a cada agente narrativo.
- `docs/narrative/deep/stinger/` (2 arquivos: `post-credits-deep.md`, `sequel-hooks-deep.md`) - ganchos pós-créditos e de sequência. **Cifrados** (ver eixo 12).
- `docs/narrative/deep/dragon-victory-deep.md` - o desfecho de vitória.
- `docs/design/lore-delivery-model.md` - não é lore em si, é o MODELO de como a lore chega ao jogador (fonte única de verdade da entrega, não do conteúdo).
- `docs/design/gus-abertura.md` - o esqueleto da missão de abertura do jogo.
- `docs/design/narrativa/dialogue-tree-npc-intro.md` - a árvore de diálogo canônica da introdução.
- `docs/narrative/propostas/` (6 arquivos, todos com sufixo `-PROPOSTA` no nome) - cenas propostas, explicitamente não-canônicas até ratificação.

## 10. Textos de dentro do mundo

Diário, glossário, bestiário, documentos in-world que o jogador lê dentro do jogo.

- **Soberano: `docs/narrative/in-world-docs.md`** - os 23 documentos descobríveis pelo Gus durante o jogo.
- **Soberano da sub-camada "Diário do Gus": `docs/narrative/diary/_INDEX.md`** - o Diário do Gus (Bloco H), com 8 arquivos: `entries-docs-descobriveis.md`, `entries-fichas-bestiary.md` (eixo 5), `entries-manuscrito-glossario.md`, `entries-mapas-timeline.md`, `foreshadow-links.md`, `knowledge-gates.md` (inclui o schema de save), `ui-spec.md`.
- Trechos in-character de cronista (excertos de texto de dentro do mundo) aparecem também em `docs/narrative/deep/eras/era-1-pre-codigo/capitulo-09-08-...` e `capitulo-10-trecho-in-character-cronista-era-3.md` (eixo 3).
- Os dois volumes que consolidarão a lore em livro (worldbuilding + antologia) são só blueprint editorial, cifrados: `docs/book/` (ver eixo 12).
- Fora de `docs/`: `resources/dialogues/` (roteiro de diálogo em formato-texto próprio) e `resources/translations/` (locale pt-br/en) guardam texto in-game pronto para consumo; não foram contados na varredura deste mapa (escopo era `docs/`).

## 11. Humor e segredos

Alívios cômicos, homenagens, easter eggs.

- `docs/narrative/comic-reliefs.md` - o documento de princípios (timing, tom, referências) e ponto de entrada.
- `docs/narrative/comic-reliefs/` (22 arquivos) - as cenas em si: `cena-01` a `cena-19`, mais `convencoes-diegeticas-guerra-de-plataforma.md`, `easter-eggs-educacionais.md`, `homenagens-diegeticas.md`.
- `docs/narrative/gus-apartes-c-arcane.md` - os apartes de fanboy do Gus em C-Arcane, aprovados um a um pelo líder; qualquer mudança futura exige nova autorização ([[project_lei_do_revogado]], ver `GODS_LAWS.md` L-37 do projeto).
- `docs/design/mecanicas/easter-egg-faro-de-inacia.md` e `easter-egg-sidequest-procure-por-pauli.md` - dois easter eggs educacionais com mecânica própria.
- O manual completo de easter eggs e a proposta de balanceamento deles ficam cifrados: `docs/_secret/easter-eggs-manual.md`, `docs/_secret/proposta-balanceamento-easter-eggs.md` (ver eixo 12).
- O aviso de ficção sobre reimaginar figuras históricas reais (`docs/design/roster-analogos/OBRA-DE-FICCAO-E-METODOLOGIA.md`, eixo 1) é o parente direto de `homenagens-diegeticas.md` aqui.

## 12. A fronteira do cifrado

Três áreas do repositório são cifradas por `git-crypt` e **nunca aparecem em texto puro fora dele** (`.gitattributes`, cross-ref `GODS_LAWS.md` L-08/L-18/L-25 do projeto). Este mapa cita só o CAMINHO de cada arquivo, nunca o conteúdo:

- **`docs/_secret/`** (17 arquivos, índice em `_INDEX.md`) - a maior parte é solução de enigma de dungeon, em `docs/_secret/dungeons/` (11 arquivos: capturas brutas, soluções, conferências de canon, pesquisas de mecânica). Os demais 6 são: `auditoria-privacidade.md`, `AUDIT-T5-EASTEREGGS-V2.md`, `cosmologia-origem-notas-secret.md` (eixo 3), `easter-eggs-manual.md` e `proposta-balanceamento-easter-eggs.md` (eixo 11), e o próprio `_INDEX.md`.
- **`docs/narrative/deep/stinger/`** (2 arquivos) - os ganchos pós-créditos e de sequência (eixo 9).
- **`docs/book/`** (11 arquivos) - os blueprints editoriais dos dois volumes-livro (`BIBLE-V1-*` = bíblia de worldbuilding, `BIBLE-V2-*` = antologia narrativa que consolida os 14 contos do eixo 1). São estrutura de sumário, não o livro redigido.

Fora dessas três árvores, nada em `docs/` é cifrado.

---

## Achados da varredura (não é eixo, é relato de medição)

**Método:** cada arquivo `.md` de `docs/` (425 no total) foi classificado num dos 12 eixos acima, ou marcado fora de escopo (documentação de processo, produção de asset, QA, segurança, engenharia/ADR - matéria real do projeto, mas não é *lore de mundo*).

- **Classificados em eixo de lore (incluindo os cifrados dos eixos 3, 9, 11 e 12): 325 de 425.**
- **Fora do escopo deste mapa: 100 de 425** - `docs/art/` (5, specs de produção visual), `docs/_processo/` (32, relatórios de decisão/auditoria de processo), `docs/qa/` (2), `docs/security/` (2), `docs/specs/` (9, specs técnicas de asset 3D), `docs/tech/` (24, contratos de engenharia e ADRs), e `docs/licoes-aprendidas.md` (1) na raiz de `docs/`. Nenhum destes é lacuna: é matéria de engenharia/produção/processo, coberta pelos próprios índices de pasta (`docs/tech/_INDEX.md`, `docs/tech/adr/_INDEX.md`) e pelos manuais canônicos do `CLAUDE.md`.
- Dentro de `docs/design/` (145) e `docs/narrative/` (177), a contagem por arquivo bateu com `find` em cada subpasta: 125 de 145 em `docs/design/` foram para um eixo de lore (os outros 20 são análise de pacing/mira, propostas de protocolo de simulação, plano de animação, backlog de brainstorm, pesquisa de mercado e requisitos de UI - processo de produção, não lore); 172 de 177 em `docs/narrative/` foram para um eixo (os outros 5 são guia de estilo de diálogo, guia de narrativa fluida, bibliografia do RAG, o tracker de incoerências resolvidas e um documento de perguntas em aberto - processo editorial, não lore).

**Arquivos que não couberam em eixo nenhum:** nenhum. Todo `.md` de `docs/` ficou classificado como lore (num dos 12 eixos) ou como fora de escopo (processo/produção/engenharia), com o motivo explícito acima.

**Lacunas de lore que a varredura revelou** (assunto que o mundo cita mas nenhum documento cobre de forma dedicada):

1. **Fauna e flora** (eixo 5) é o eixo mais fino: fora do bestiário mecânico do Diário e da criatura Fibortorrinco, não há bestiário de mundo com ficha por espécie, nem documento de flora tecnorgânica dedicado - a flora da Selve Sombria só aparece embutida nos docs de ambiente.
2. **Objetos** (eixo 7) não tem inventário soberano equivalente a `CHARS.md`/`PLACES.md`: o catálogo de cartas está espalhado por ~10 documentos de mecânica fora da subpasta `cartas/`, e não há um catálogo único de item não-carta (poções, ingredientes, artefatos) além dos 3 arquivos avulsos citados.
3. `docs/design/mundo-topologia.md` (a fonte da geografia macro, eixo 2) estava em edição concorrente no momento desta varredura - o caminho é estável, o conteúdo pode ter mudado desde então.
4. `docs/_processo/mapa-corpus-gusworld.md` já é um levantamento de corpus anterior (21/08/2026), mas é uma **auditoria datada de estado do projeto** (contradições 3D/2D, ADRs ausentes etc.), não um mapa de lore por assunto; não foi usado como fonte de conteúdo aqui porque parte do que ele descreve (ex.: ADRs citados) já foi purgada do repositório desde então.

---

## Outros índices, para descer mais fundo

Quando este mapa aponta para uma pasta, ela pode já ter índice próprio com a lista completa dos arquivos dela:

- `docs/design/dungeons/areas-secretas/_INDEX.md`
- `docs/design/mecanicas/cartas/_INDEX.md`
- `docs/narrative/deep/_INDEX.md`
- `docs/narrative/diary/_INDEX.md`
- `docs/narrative/environments/_INDEX.md`
- `docs/_processo/_INDEX.md` (processo, fora de escopo deste mapa)
- `docs/_secret/_INDEX.md` (cifrado, ver eixo 12)
- `docs/specs/_INDEX.md` (produção de asset, fora de escopo)
- `docs/tech/adr/_INDEX.md` (decisões de arquitetura, fora de escopo)
- `docs/tech/fontflip-visuals/_INDEX.md` (fora de escopo)
- `docs/tech/_INDEX.md` (fora de escopo)
