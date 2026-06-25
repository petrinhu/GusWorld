# World Assets - Distritos Inferiores (overworld) - PixelLab prompts

**Gerador:** PixelLab. Usa duas ferramentas distintas: `create_topdown_tileset` (chao/parede repetiveis com auto-tiling) e `create_map_object` (casas, props, landmarks, decorativos pontuais). Este doc distingue explicitamente o que e TILESET do que e MAP OBJECT.

**Escopo:** assets de CENARIO do overworld top-down da 1a area do jogo (Distritos Inferiores, Praca da Compilacao), para o Vertical Slice. NAO inclui personagem/inimigo (esses sao character-state, outro balde) nem o combate (BattleScreen, doc separado).

**Cross-ref canon:** `docs/design/levels/blockout-distritos-inferiores.md` (topologia, 5 nos, gold path, landmarks), `docs/art/style-guide.md` (paleta City, ciber-gotico, cel-shading, leitura pixel-art), `PLACES.md` (Distritos Inferiores), `GusEngine/assets/maps/source/distritos_inferiores.csv` (o mapa real: grade 30x20, tile_size 2.0m, ids 0=Chao 1=Parede 2=Marco 3=Entrada 4=Saida), `resources/prompts_images/BATTLE_ICONS_M5_PIXELLAB.md` (dialeto de cor/outline ja aprovado, herdado aqui).

**Convencao de escrita:** prompts em ingles (lingua do gerador). Notas em pt-br. ZERO em-dash e en-dash neste arquivo: usa hifen, virgula, parenteses, dois-pontos.

---

## 0. VEREDITO DE VIABILIDADE (o mais importante)

**Veredito: VIAVEL para o cenario-base, com 1 lacuna declarada (ver abaixo). Da pra gerar AGORA os tiles (chao/parede/marco/entrada/saida) e a maioria dos map objects, porque o design e o mapa real ja existem e travam forma, escala e funcao. Falta apenas referencia visual ILUSTRADA da cidade (concept/mood paint), que NAO bloqueia o P0 mas deve ser apontada ao criador.**

### O que JA EXISTE e habilita a geracao (base suficiente)

1. **Paleta City fechada e canonica** (style-guide secao 4.1): BG deep `#0A0E1A`, BG mid `#1B2238`, BG light `#3A4566`, neutral `#6B6F7A`, accent cyan `#22D3EE`, magenta `#E11D74`, violet `#7C3AED`, hot signal `#FACC15`. Tom: noite perpetua, neon ciano/magenta, concreto azulado. Isso e suficiente para travar a cor de TODO o cenario.
2. **Render style fechado** (style-guide secao 1, 3, 10): low-poly cel-shaded com LEITURA pixel-art, silhueta chapada, sem PBR. PixelLab entrega exatamente essa leitura.
3. **Shape language do cenario definida** (style-guide secao 7): ambiente humano/corporativo = angulos agudos, simetria quebrada, uncanny, triangular. Pavimento tesselado de pedra Era 1, postes de neon, fonte de latao Era 2.
4. **O mapa REAL ja existe** (CSV): grade 30x20, tile 2.0m, com Chao/Parede/Marco/Entrada/Saida concretos. Os tiles tem funcao e contagem definidas. Nao e especulacao: e o nivel que vai rodar.
5. **Os landmarks estao nomeados e descritos** (blockout secao 1, 5): fonte de latao central (8m alt, landmark-ancora), holograma de Sterling piscando "continue" ao norte, portao sul iluminado, terminal-placa Era 2, postes de neon ciano, pavimento tesselado rachado.
6. **Easter eggs espaciais canon** (blockout secao 6): vaos de janela em proporcao 1:1:2:3:5, escada 3-5-7 degraus, fachadas em sequencia numerica. Aplico com densidade ~10-15%, velado.

### A LACUNA (declarar ao criador, nao bloqueia P0)

- **Nao existe concept art / mood paint ILUSTRADO da Praca da Compilacao** (uma pintura de referencia da cidade vista de cima ou em perspectiva). O que existe e DESCRICAO textual rica + paleta + o mapa funcional. Para o P0 (tiles e props funcionais do overworld jogavel) isso BASTA: PixelLab gera a partir de prompt textual ancorado na paleta. Para garantir COESAO ATMOSFERICA fina (que a praca inteira "pareca um lugar so" e nao um ajuntamento de tiles), o ideal seria depois gerar 1 a 2 mood-paints de referencia (key art da praca) ANTES de finalizar o P1/P2, e ajustar os tiles contra ela. Recomendacao: gerar o P0 ja, e em paralelo pedir ao criador se quer um mood-paint-ancora da praca (pode sair do proprio PixelLab ou do gerador de personagem nano-banana) para travar a atmosfera antes do polimento.
- **Decisao de tile_size visual pendente (fina, nao bloqueia):** o mapa e 2.0m por celula; em pixel art top-down isso vira tipicamente 16x16, 24x24 ou 32x32 px por tile. Proponho 32x32 px/tile (casa com o dialeto de 32 dos icones e da resolucao confortavel ao pavimento tesselado + rachaduras). O criador valida o tamanho final na tela; abaixo uso 32x32 como sugestao.

**Conclusao:** seguir com os prompts do P0 agora. A unica coisa que recomendo NAO gerar como final antes de mood-paint e o conjunto atmosferico de polimento (P2). Os tiles e props funcionais (P0/P1) podem ir.

---

## 1. Guia de estilo de cenario (herda os icones, especializa pra mundo)

Coerente com o style-guide e com o dialeto ja aprovado dos icones de batalha. O cenario tem regras proprias por ser top-down repetivel, mas COMPARTILHA a familia visual.

### Heranca do dialeto aprovado
- **Outline:** 1 px escuro `#0A0E1A` em map objects (casas, props, landmarks), igual aos icones. EXCECAO: tiles de chao NAO levam outline fechado por celula (senao vira grade quadriculada feia ao tesselar); o chao usa variacao de valor e juntas de pedra, com outline apenas onde encontra parede.
- **Cel-shading:** 2 a 3 valores duros por cor, bandas duras, sem gradiente suave. Igual aos icones.
- **Paleta restrita City** (secao 0): cada asset usa um sub-conjunto pequeno da paleta City. Neon (cyan/magenta) so como ACCENT pontual (style-guide: practical lights), nunca cobrindo area grande. O cenario e majoritariamente concreto azulado frio (BG mid/light + neutral); o brilho e raro e proposital.
- **Leitura pixel-art** ja e a leitura-alvo do jogo: tiles e props pixel REFORCAM a estetica.

### Regras especificas de top-down
- **Tile_size sugerido: 32x32 px** por celula de 2.0m (validar com o criador).
- **Perspectiva top-down levemente angulada (3/4 de cima):** coerente com a camera orbital 3/4 do jogo (style-guide); o chao e quase reto de cima, mas casas e props mostram um pouco da face frontal (altura legivel). Nao e top-down puro ortografico nem isometrico estrito.
- **Auto-tiling:** o tileset de chao e de parede deve casar nas 4 bordas (seamless) para o `create_topdown_tileset` montar transicoes. Pedir variantes de borda/canto.
- **Direcao de luz consistente:** style-guide City = luz cinza-azulada fraca vinda de cima-leste, sem sol. Sombra curta e fria para baixo-oeste em todos os props (coerencia de iluminacao entre assets).
- **Silhueta primeiro (Pillar 1, style-guide secao 7):** Parede tem que ler INSTANTANEAMENTE como bloqueio vs Chao navegavel. Marco (landmark) tem que saltar do chao comum. Entrada e Saida tem que ler como portais.
- **Shape language:** ambiente corporativo/humano = angulos agudos, simetria quebrada, uncanny (style-guide). Pedra Era 1 = tesselado geometrico rachado. Latao Era 2 = curvas mecanicas mais quentes (a fonte, ornamentos). Contraste Era 1 (pedra fria) vs Era 2 (latao) e proposital.
- **Sem gore, sem open-world clutter:** cidade densa mas curada, nao caotica.

### Easter eggs (velados, ~10-15%, nao rotular)
- Vaos de janela das fachadas em proporcao 1:1:2:3:5 (blockout secao 6).
- Pavimento tesselado: padrao xadrez/tesselado de pedra (eco maconico canon, velado: pavimento mosaico).
- Escada do choke = 3-5-7 degraus.
- Numeros Fibonacci em contagens de ornamento quando couber. NUNCA narrar; leitor atento nota.

### Checklist de aprovacao do conjunto de cenario
1. Parede le como bloqueio e Chao como navegavel a relance (silhueta/valor).
2. A paleta e a City canonica; neon so como accent pontual.
3. Os tiles tesselam sem costura visivel (auto-tiling).
4. Os map objects compartilham a direcao de luz e o outline 1px `#0A0E1A`.
5. A fonte de latao (landmark central) e inconfundivelmente o ponto-ancora da praca.
6. Coerente lado a lado com os icones de batalha ja aprovados (mesma familia visual).

---

## 2. Inventario + prompts

Legenda de prioridade:
- **P0** = minimo para o overworld jogavel (os 5 TileKinds do CSV + a fonte-landmark + portao). Sem isso a praca nao renderiza.
- **P1** = casas ciber-goticas e props de rua que dao identidade a praca.
- **P2** = polimento atmosferico e decorativos (idealmente apos um mood-paint-ancora).

### 2.1 TILESET (chao/parede repetiveis, `create_topdown_tileset`)

#### T0 - Chao: pavimento tesselado de pedra Era 1 (TileKind Chao=0) [P0]
**Tipo:** topdown tileset (lower/floor layer).
**Prompt:**
```
A seamless top-down pixel-art floor tileset for a cyber-gothic city plaza:
cracked tessellated Era-1 stone pavement, cool blue-grey concrete tones (base
#3A4566, shadow #1B2238, lighter joints #6B6F7A), a geometric tessellated /
mosaic paving pattern with thin dark joint lines and subtle cracks. Flat even
cold lighting, no sun. Cel-shaded, 2 to 3 hard value bands, no soft gradient,
no per-tile black outline (tiles must tile seamlessly without a grid look).
Muted, walkable, reads clearly as ground. Include edge and corner variants for
auto-tiling.
```
**Parametros PixelLab:** tool `create_topdown_tileset`; tile 32x32 px; layer floor; palette limited (`#3A4566`, `#1B2238`, `#6B6F7A`); seamless/auto-tile variants; cel/flat shading.
**Reforcar:** pavimento TESSELADO rachado (landmark textural da praca + easter egg mosaico velado). Concreto azulado frio. Chao navegavel le como navegavel.

#### T1 - Parede: bloco ciber-gotico (TileKind Parede=1) [P0]
**Tipo:** topdown tileset (wall layer).
**Prompt:**
```
A seamless top-down pixel-art wall tileset for a cyber-gothic city: dark angular
concrete-and-steel barrier blocks, 3m tall reading clearly as IMPASSABLE,
sharp-angled uncanny corporate geometry, base dark blue-grey (#1B2238) with near-
black shadow (#0A0E1A) and a thin cool edge highlight (#3A4566). Optional faint
cyan (#22D3EE) seam light as a rare accent. Strong solid silhouette that contrasts
against the lighter floor. Cel-shaded, 2 to 3 hard value bands, no soft gradient,
1px dark outline (#0A0E1A) where the wall meets the floor. Include straight,
corner, T-junction and end-cap variants for auto-tiling.
```
**Parametros PixelLab:** tool `create_topdown_tileset`; tile 32x32 px; layer wall; palette limited (`#1B2238`, `#0A0E1A`, `#3A4566`, rare cyan `#22D3EE`); auto-tile variants (straight/corner/T/end); cel/flat.
**Reforcar:** BLOQUEIO inconfundivel (silhueta solida, 2+ valores acima/abaixo do chao). Angulos agudos = corporativo uncanny (shape language). Neon so como costura rara.

#### T2 - Marco / landmark-tile generico (TileKind Marco=2) [P0]
**Tipo:** map object pontual (o Marco no CSV e celula-no, melhor como objeto colocado sobre o chao do que tile repetivel).
**Nota:** o TileKind Marco=2 marca celulas-no (landmarks/pontos de interesse: posicao da fonte, terminal, etc). Visualmente NAO e um tile repetivel: e onde um MAP OBJECT especifico assenta. Trato cada Marco concreto como objeto (fonte = O1, terminal-placa = O2, holograma Sterling = O3). Para Marcos sem objeto dedicado, um decalque-de-chao marca o ponto:
**Prompt (decalque de marco generico):**
```
A single top-down pixel-art floor decal marking a point of interest in a cyber-
gothic plaza: a subtle inlaid metal ring or glyph set into the stone pavement,
burnished brass (#E8A33D) with a faint cyan (#22D3EE) inlay, low-contrast so it
reads as "something here" without shouting. Sits flat on the ground, no height.
Cel-shaded, hard bands, 1px dark outline (#0A0E1A). Transparent background
outside the decal.
```
**Parametros PixelLab:** tool `create_map_object`; size 32x32 px; background transparent; palette (`#E8A33D`, `#22D3EE`, `#0A0E1A`); cel/flat.
**Reforcar:** marca de no/landmark no chao (Scan-overworld pinga estes). Latao + cyan velado. Os Marcos importantes viram objetos dedicados (O1-O3).

#### T3 - Entrada (TileKind Entrada=3, rampa norte do Edificio Vance) [P0]
**Tipo:** map object (portal de chao) sobre o tileset.
**Prompt:**
```
A top-down pixel-art entrance tile for a cyber-gothic plaza: a ramp/threshold
leading in from the north (from the Vance building), cool concrete with a soft
inviting cyan (#22D3EE) light strip framing the threshold, an arrow-like or open-
gate read suggesting "way in / safe". Base blue-grey (#3A4566) with cyan accent.
Cel-shaded, hard bands, 1px dark outline (#0A0E1A). Reads as a friendly entry
portal distinct from the hostile south exit. Transparent background outside the
tile.
```
**Parametros PixelLab:** tool `create_map_object`; size 32x32 px (ou 64x32 se ocupar 2 celulas, ver CSV: entrada ocupa 2 celulas no topo); background transparent; palette (`#3A4566`, `#22D3EE`, `#0A0E1A`); cel/flat.
**Reforcar:** portal de ENTRADA seguro (cyan acolhedor, "de onde vim"). Distinto da saida sul. No CSV a entrada ocupa 2 celulas (cols 14-15, linha 0).

#### T4 - Saida (TileKind Saida=4, portao sul p/ Periferia) [P0]
**Tipo:** map object (portal de chao) sobre o tileset; ver tambem O4 (o portao 3D-ish).
**Prompt:**
```
A top-down pixel-art exit tile for a cyber-gothic plaza: a southern gate
threshold leading down to the Periferia, heavier and more industrial than the
entrance, with a magenta (#E11D74) hazard-light strip framing it (signaling "edge
/ leaving the safe hub"). Base dark blue-grey (#1B2238) with magenta accent and
worn metal. Cel-shaded, hard bands, 1px dark outline (#0A0E1A). Reads as a gated
exit, more ominous than the cyan entrance. Transparent background outside the
tile.
```
**Parametros PixelLab:** tool `create_map_object`; size 32x32 px (saida no CSV: col 15, linha 19); background transparent; palette (`#1B2238`, `#E11D74`, `#0A0E1A`); cel/flat.
**Reforcar:** portal de SAIDA (magenta = borda/perigo, "para onde vou", charneira da Periferia). Bloqueado ate o puzzle resolver (o portao fechado = objeto O4).

### 2.2 MAP OBJECTS (landmarks, casas, props, `create_map_object`)

#### O1 - Fonte de latao Era 2 (LANDMARK central) [P0]
**Prompt:**
```
A top-down pixel-art map object: an ornate Era-2 brass fountain, the central
landmark of a cyber-gothic plaza, roughly 8m tall in world but shown as a tall
top-down hero prop. Burnished brass-amber (#E8A33D) mechanical curves with
titanium-grey (#9AA6B2) and a slow rotating mechanism, water glinting with a
faint cyan (#22D3EE) reflection. Warm brass contrasts with the cold blue-grey
plaza around it. Decorative, slightly uncanny clockwork ornament. Cel-shaded, 2
to 3 hard value bands, no soft gradient, 1px dark outline (#0A0E1A). Casts a
short cool shadow to the lower-left. Transparent background.
```
**Parametros PixelLab:** tool `create_map_object`; size ~96x96 px (hero prop, ocupa varias celulas); background transparent; palette (`#E8A33D`, `#9AA6B2`, `#22D3EE`, `#0A0E1A`); cel/flat.
**Reforcar:** o LANDMARK-ANCORA do hub (blockout: fonte de latao central girando). Latao Era 2 quente contra a praca fria. Easter egg: contagem de ornamentos/jatos pode seguir Fibonacci, velado.

#### O2 - Terminal-placa Era 2 (no de Lore) [P0]
**Prompt:**
```
A top-down pixel-art map object: an Era-2 lore terminal-plaque, a waist-high
angular pedestal with an engraved metal plate and a small dim screen, brass
(#E8A33D) frame on dark steel (#1B2238), a faint cyan (#22D3EE) glow when
scannable. Reads as "readable / interactive lore point". Cel-shaded, hard bands,
1px dark outline (#0A0E1A), short cool shadow lower-left. Transparent background.
```
**Parametros PixelLab:** tool `create_map_object`; size ~32x48 px; background transparent; palette (`#E8A33D`, `#1B2238`, `#22D3EE`, `#0A0E1A`); cel/flat.
**Reforcar:** no de Lore (L) do blockout (placa Era 2, Scan-overworld pinga como decifravel; ensina o verb scanear). Cyan = scannable.

#### O3 - Holograma Sterling piscando "continue" (landmark norte) [P0]
**Prompt:**
```
A top-down pixel-art map object: a large flickering corporate hologram billboard
looming at the north edge of the plaza, projecting a cold cyan-and-magenta
Sterling corporate sigil with a blinking "continue" prompt, an uncanny watching
presence. Sharp angular corporate shapes, cyan (#22D3EE) and hostile magenta
(#E11D74) light on a dark (#0A0E1A) projector base. Slight scanline flicker feel
(static frame, no animation needed). Cel-shaded, hard bands, 1px dark outline
(#0A0E1A). Transparent background.
```
**Parametros PixelLab:** tool `create_map_object`; size ~80x96 px; background transparent; palette (`#22D3EE`, `#E11D74`, `#0A0E1A`); cel/flat.
**Reforcar:** landmark de orientacao norte ("de onde vim", blockout secao 5) + presenca de Sterling vigiando (tom, Pillar 5). Cyan+magenta corporativo hostil. NAO usar glitch/RGB-split (reservado anomalo/Patch-Zero); aqui e flicker de scanline.

#### O4 - Portao sul fechado (bloqueio do puzzle) [P0]
**Prompt:**
```
A top-down pixel-art map object: a heavy closed industrial gate sealing the
southern exit of the plaza, worn metal and dark steel (#1B2238) with magenta
(#E11D74) warning lights and angular reinforcement bars, clearly LOCKED. Reads as
"blocked until solved". Heavier and grimmer than any other prop. Cel-shaded, hard
bands, 1px dark outline (#0A0E1A), short cool shadow. Transparent background.
```
**Parametros PixelLab:** tool `create_map_object`; size ~64x48 px; background transparent; palette (`#1B2238`, `#E11D74`, `#0A0E1A`); cel/flat.
**Reforcar:** portao fechado que so abre apos o puzzle-patrulha (blockout DA, gold path). Le como bloqueio. Par com o tile de saida T4 (T4 = limiar de chao; O4 = o portao que o cobre quando fechado).

#### O5 - Casa ciber-gotica A (fachada corporativa baixa) [P1]
**Prompt:**
```
A top-down pixel-art map object: a low cyber-gothic city building, angular
corporate facade with sharp uncanny geometry and broken symmetry, dark blue-grey
concrete (#1B2238 / #3A4566) with thin cyan (#22D3EE) neon seam lights as a rare
accent. Narrow tall windows arranged in a subtle 1-1-2-3-5 spacing rhythm. Slight
3/4 top-down angle showing a bit of the front face and the roof. Cel-shaded, 2 to
3 hard value bands, no soft gradient, 1px dark outline (#0A0E1A), short cool
shadow lower-left. Transparent background.
```
**Parametros PixelLab:** tool `create_map_object`; size ~96x96 px; background transparent; palette (`#1B2238`, `#3A4566`, `#22D3EE`, `#0A0E1A`); cel/flat.
**Reforcar:** casa-bloco da praca. Angulos agudos + simetria quebrada (corporativo uncanny, style-guide secao 7). Janelas em 1:1:2:3:5 (easter egg velado, blockout secao 6). Neon cyan so como costura.

#### O6 - Casa ciber-gotica B (variante mais alta, accent magenta) [P1]
**Prompt:**
```
A top-down pixel-art map object: a taller cyber-gothic city building variant,
gothic-industrial silhouette with angular buttress-like reinforcements, dark
steel and concrete (#0A0E1A / #1B2238) with a hostile magenta (#E11D74) neon sign
as the single bright accent. Sharp, slightly oppressive, corporate. Slight 3/4
top-down angle. Distinct silhouette from building A (taller, magenta not cyan).
Cel-shaded, hard bands, 1px dark outline (#0A0E1A), short cool shadow. Transparent
background.
```
**Parametros PixelLab:** tool `create_map_object`; size ~96x128 px; background transparent; palette (`#0A0E1A`, `#1B2238`, `#E11D74`, `#0A0E1A`); cel/flat.
**Reforcar:** variante de casa pra dar ritmo a praca (silhueta distinta da A: mais alta, accent magenta). Magenta = corporativo hostil (style-guide). 2 a 3 variantes de casa ja quebram a repeticao.

#### O7 - Poste de neon ciano (prop de rua) [P1]
**Prompt:**
```
A top-down pixel-art map object: a tall cyber-gothic street lamp post, dark metal
(#1B2238) with a bright cyan (#22D3EE) neon tube glowing at the top, a small
practical light pool on the ground below it. Thin angular silhouette. Cel-shaded,
hard bands, 1px dark outline (#0A0E1A), short cool shadow. Transparent background.
```
**Parametros PixelLab:** tool `create_map_object`; size ~32x80 px; background transparent; palette (`#1B2238`, `#22D3EE`, `#0A0E1A`); cel/flat.
**Reforcar:** poste de neon ciano (blockout: "postes de neon ciano"). Practical light pontual (style-guide: practical lights cyan/magenta alternados). Repetivel ao longo da praca.

#### O8 - Terminal de hack ambiental (no Terminal, ramo leste) [P1]
**Prompt:**
```
A top-down pixel-art map object: a small wall-mounted or standing hack terminal,
an inert dark panel (#1B2238) with dormant ports and a faint amber (#FACC15)
standby light, that reads as "interactive once scanned". Angular techwear look,
brass (#E8A33D) trim. Cel-shaded, hard bands, 1px dark outline (#0A0E1A), short
shadow. Transparent background.
```
**Parametros PixelLab:** tool `create_map_object`; size ~32x40 px; background transparent; palette (`#1B2238`, `#FACC15`, `#E8A33D`, `#0A0E1A`); cel/flat.
**Reforcar:** no Terminal (T) do blockout (painel inerte que Scan revela interativo; abre atalho pro flanco da arena). Amber standby = "tem algo aqui".

#### O9 - Cover box da arena (caixa de cobertura, 1m) [P1]
**Prompt:**
```
A top-down pixel-art map object: a low industrial cover crate / barrier (~1m
tall), worn metal and concrete (#3A4566 / #1B2238) with a thin cyan (#22D3EE)
edge light, clearly something a character ducks behind for cover. Solid blocky
silhouette. Cel-shaded, hard bands, 1px dark outline (#0A0E1A), short cool shadow.
Transparent background.
```
**Parametros PixelLab:** tool `create_map_object`; size ~48x48 px; background transparent; palette (`#3A4566`, `#1B2238`, `#22D3EE`, `#0A0E1A`); cel/flat.
**Reforcar:** cover box da arena rebaixada (blockout secao 7: 2 caixas 1m). Le como cobertura. Funcional, nao decorativo.

#### O10 - Decalque do board do puzzle (grid 7x5 holografico) [P1]
**Prompt:**
```
A top-down pixel-art floor object: a holographic puzzle board projected onto the
plaza floor, a faint 7x5 grid of cyan (#22D3EE) light lines with subtle violet
(#7C3AED) nodes, snap-to-grid geometric look (rune/holographic family). Flat on
the ground, low height. The grid suggests patrol-vector logic. Cel-shaded, hard
bands. Transparent background. Designed to be tinted/animated by code.
```
**Parametros PixelLab:** tool `create_map_object`; size ~112x80 px (7x5 celulas de 2m, escala 32/celula ~ ajustar); background transparent; palette (`#22D3EE`, `#7C3AED`, `#0A0E1A`); cel/flat unshaded-ish.
**Reforcar:** board holografico do puzzle-Gambito (blockout (P); grid 7x5). Familia VFX Rúnico (cyan+violeta, snap-to-grid, style-guide secao 11.1). O codigo anima as patrulhas por cima.

### 2.3 P2 (polimento atmosferico, idealmente apos mood-paint-ancora)

| ID | Asset | Tipo PixelLab | Nota |
|---|---|---|---|
| O11 | Variantes de rachadura/sujeira do chao (decals) | map object (decal) | quebra repeticao do tileset T0; passar so apos validar T0 |
| O12 | Tubulacao/duto exposto ciber-gotico (prop de parede) | map object | textura de fundo das fachadas |
| O13 | Cartaz/holo-anuncio menor (props de parede) | map object | densidade urbana; cyan/magenta accent |
| O14 | Lixeira / sucata / container (props de rua) | map object | charneira da Periferia ao sul (mais sucata descendo) |
| O15 | Vegetacao urbana morta / fios pendurados | map object | atmosfera; baixa prioridade |
| O16 | Variante de poste magenta (par do O7) | map object | alternancia cyan/magenta a cada quadra (style-guide secao 5.1) |
| O17 | Decalque de cone de deteccao do Sentinela (chao da arena) | map object (decal) | Scan pinta o cone no chao (blockout secao 6); pode ser so codigo |
| O18 | Mood-paint-ancora da Praca da Compilacao | (referencia, nao asset de jogo) | LACUNA da secao 0: gerar 1 a 2 key-arts da praca pra travar atmosfera antes de finalizar P2 |

---

## 3. Contagem e recomendacao

- **P0: 9 assets** (T0 chao, T1 parede, T2 marco-decal, T3 entrada, T4 saida, O1 fonte, O2 placa-lore, O3 holo-Sterling, O4 portao-fechado). E o minimo para a praca renderizar e o overworld ser jogavel: os 5 TileKinds do CSV + os landmarks que o gold path EXIGE (fonte, placa, holo, portao).
- **P1: 6 assets** (O5/O6 casas, O7 poste, O8 terminal-hack, O9 cover, O10 board-puzzle). Dao identidade e cobrem os nos restantes do blockout.
- **P2: ~8 assets** (O11-O18), polimento atmosferico, idealmente apos mood-paint.

**Recomendacao de ordem:**
1. **Comecar pelo par T0 (chao) + T1 (parede)**, que sao a base tesselavel: aprovados, a praca inteira ja tem piso e bloqueio jogaveis. Validar auto-tiling cedo (e o maior risco tecnico do tileset).
2. **Depois T3/T4 (entrada/saida) + O1 (fonte)**: fecham a leitura de navegacao por landmark (entrada cyan, saida magenta, fonte central). Com isso o gold path ja e legivel no overworld.
3. **O2/O3/O4** completam o P0 (os nos de lore, o landmark norte, o bloqueio sul).
4. So entao P1 (casas e props) e, antes do P2, **levantar com o criador o mood-paint-ancora** (a lacuna declarada): gerar 1 a 2 key-arts da praca para travar a atmosfera, e ajustar tiles/props contra ela no polimento.

**Pendencia para o criador (nao bloqueia P0):**
- Confirmar tile_size visual (sugiro 32x32 px/celula de 2.0m).
- Decidir se quer o mood-paint-ancora da praca antes do P2 (recomendo que sim, para coesao atmosferica).
- A decisao 2D-vs-3D da arena de batalha (pendente em outro doc) NAO afeta o overworld top-down: estes assets de cenario valem independente disso.

---

## 4. LOG DE GERACAO (gerados no PixelLab)

**2026-06-25 (modo autonomo, lider autorizou "continue mais algumas pixel arts pra ganhar tempo").** 10 MAP OBJECTS gerados via `create_map_object` (basic mode, high top-down, cel/basic shading, fundo transparente, ~20 fichas cada). Salvos em `resources/sprites/world/distritos_inferiores/` (local; versionar se o lider quiser, igual icons-m5). Revisao: `_REVISAO_mosaico.png` na mesma pasta.

| Asset | Arquivo | object_id | Tamanho |
|---|---|---|---|
| O1 Fonte de latao (landmark central) | fonte_latao.png | 6fa0f0e1 | 96x96 |
| O2 Placa-lore (no de Lore) | placa_lore.png | 6ee501cc | 32x48 |
| O3 Holograma Sterling (landmark norte) | holo_sterling.png | 468c2993 | 80x96 |
| O4 Portao sul fechado (bloqueio puzzle) | portao_sul_fechado.png | e2b4981d | 64x48 |
| O5 Casa ciber-gotica A (cyan) | casa_cibergotica_a.png | a9462183 | 96x96 |
| O6 Casa ciber-gotica B (magenta, alta) | casa_cibergotica_b.png | 2ff7dce9 | 96x128 |
| O7 Poste de neon ciano | poste_neon_ciano.png | b69b97c3 | 32x80 |
| O8 Terminal de hack | terminal_hack.png | 42fa97f7 | 32x40 |
| O9 Cover box da arena | cover_box.png | ee22053e | 48x48 |
| O10 Board do puzzle (grid 7x5) | board_puzzle.png | 96425eec | 112x112 |

**PENDENTE (nao gerado autonomo):** os 5 TILESETS (T0 chao, T1 parede, T2 marco, T3 entrada, T4 saida) via `create_topdown_tileset` (mais caro ~100s, async; o art-director apontou como o maior risco tecnico/auto-tiling). Deixados pro lider validar o 1o tileset acordado. + P2 (polimento) idealmente apos mood-paint.
