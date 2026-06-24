# Battle Icons M5 (BattleScreen): PixelLab prompts

**Gerador:** PixelLab (pixel art nativo: characters, objects, animations, icons, tilesets). Diferente dos `_IMAGEPROMPT.md` de personagem (gerador Gemini/nano-banana, prosa auto-contida em ingles que pede "downscale to pixel art"): aqui o PixelLab JA gera pixel art, entao o prompt descreve o icone direto em pixel art mais um bloco de parametros PixelLab.

**Escopo:** icones do M5 BattleScreen. Esta leva = as 5 FAMILIAS (A1-A5 do inventario), como TESTE DE ESTILO. Os 25 icones restantes do P0 (13 status, 4 intent, 3 modificador, 4 retratos CTB, 1 moldura de carta) entram DEPOIS que o criador validar o estilo destes 5.

**Cross-ref canon:** `docs/art/vfx-combate-familias.md` (cores e assinaturas por familia, decisao do criador 2026-06-24), `docs/art/style-guide.md` (leitura pixel-art, cel-shading, paleta), `docs/design/mecanicas/battle-screen.md` (HUD de combate), `docs/design/mecanicas/combat.md` paragrafo 6 (as 5 familias).

**Nota de cor (importante):** estes icones seguem o canon NOVO. Os prompts antigos em `runic_cards/` usavam violeta para o Sonico; foi SUPERADO. Cores travadas: Eletrico cyan, Bioquimico verde, Sonico AZUL-COBALTO (nao violeta), Cinetico latao, Criptografico violeta.

**Convencao de escrita:** prompts em ingles (lingua do gerador). Notas em pt-br. ZERO em-dash e en-dash neste arquivo: usa hifen, virgula, parenteses, dois-pontos.

---

## PARTE 1 - Guia de estilo dos icones do M5 (dialeto comum)

Todos os 30 icones P0 herdam este dialeto. Trava o estilo antes de escrever os 25 restantes. Validar a coesao do conjunto nos 5 de familia primeiro.

### Tamanho e grid

- **Tamanho canvas: 32x32 px.** E o sweet-spot do PixelLab para icone de HUD: detalhe suficiente para uma assinatura legivel, pequeno o bastante para um conjunto coeso e barato de gerar/iterar. (Os retratos CTB, quando chegarem, vao a 48x48 ou 64x64; os icones de status/familia/intent/modificador ficam todos em 32x32 para uniformidade na barra de HUD.)
- **Grid de leitura:** o motivo central ocupa uma area util de ~24x24 px (margem de ~4 px em volta), para o icone respirar quando encostado a outro na barra de status e nunca tocar a borda.
- **1 direcao, sem rotacao.** Icone e front-facing, vista unica, plano. Nao usar o sistema de 4/8 direcoes do PixelLab (isso e para personagem). Cada icone e uma imagem so.

### Espessura de linha e detalhe

- **Outline: 1 px**, escuro mas NAO preto puro (usar um escuro da paleta, ex. `#0A0E1A` da City BG deep do style-guide, para casar com o mundo). Outline fechado em volta da silhueta inteira (leitura forte contra qualquer fundo de HUD).
- **Detalhe interno: minimo.** Cada icone e UMA forma-assinatura reconhecivel a relance, nao uma ilustracao. Regra: se nao da pra entender o icone em ~0.3s a 32 px, simplificar. 2 a 3 niveis de valor por cor (sombra/base/luz), estilo cel (bandas duras, sem gradiente suave) coerente com o cel-shading do style-guide.
- **Cantos e formas em grid:** respeitar o pixel grid (sem anti-aliasing borrado; o PixelLab ja entrega aliased limpo). Diagonais em escada limpa.

### Silhueta e contraste (Pillar 1, leitura tatica)

- **Silhouette test obrigatorio:** o icone tem que ser identificavel so pela silhueta preenchida de preto. As 5 familias precisam ter SILHUETAS distintas entre si (nao so cores distintas), porque o jogador le a familia a relance e pode haver daltonismo. Forma e o 1o canal, cor reforca.
- **Glow interno, nao halo externo grande:** a cor-marca da familia aparece como nucleo emissivo (self-lit) DENTRO da forma, com no maximo 1 px de bloom. Sem aura larga (come o canvas de 32 px e suja a leitura na barra).
- **Contraste minimo 2 valores** entre o motivo e o que estiver atras dele no HUD.

### Fundo

- **Fundo TRANSPARENTE (alpha).** Icone de HUD vai por cima de paineis variados (terminal escuro, barra de status); precisa de alpha, nao do `#FFFFFF` solido que os prompts de personagem pedem. Esta e a diferenca-chave do balde de personagem.

### Paleta por familia (cores canonizadas, vfx-combate-familias.md)

Paleta LIMITADA por icone: a cor-marca da familia (2 a 3 valores: sombra/base/luz) mais o outline escuro mais 1 cor de apoio. Nada fora disto (Pillar 1: cor da familia inconfundivel; style-guide: paleta restrita deliberada).

| Familia | Cor primaria | Secundaria/apoio | Silhueta-assinatura |
|---|---|---|---|
| Eletrico | cyan `#22D3EE` (+ core branco-azul `#E8FBFF`) | outline escuro | raio ramificado anguloso (fork de Lichtenberg) |
| Bioquimico | verde-esmeralda `#34D399` | violeta-esporo `#A78BFA` (borda/gota) | gota/molecula organica arredondada |
| Sonico | azul-cobalto `#3B82F6` | cristas de gelo `#DBEAFE` | aneis concentricos (sonar), forma vazada |
| Cinetico | latao/ambar `#E8A33D` | cinza-titanio `#9AA6B2` | engrenagem mais cunha, mecanico solido |
| Criptografico | violeta `#7C3AED` | fractal-cyan `#67E8F9` (linhas de scan) | cadeado mais grade/wireframe |

### Coerencia com style-guide e leitura pixel-art

- **Shape language do style-guide aplicada:** Eletrico/Cinetico usam angulos agudos (perigo/impacto); Bioquimico usa curvas organicas (vivo); Sonico usa aneis concentricos (onda/CC); Criptografico usa grade ortogonal mais cadeado (sistema/estrutura). A forma carrega o significado mecanico da familia.
- **Snap-to-grid, estetica rúnica:** quando houver glifo/codigo embutido (status, modificador), usar a familia VFX Rúnico do style-guide (formas geometricas, monospace ficticia), nunca curva suave interpolada. Nos icones de familia o codigo NAO aparece (o icone e o simbolo-mae, limpo); o codigo aparece nos VFX de ataque (P1) e em alguns status.
- **Pixel-art-like ja e a leitura-alvo do jogo** (style-guide secao 12): o jogo e 3D cel-shaded mas le como pixel-art; os icones, sendo pixel art real, REFORCAM essa leitura, sem conflito.
- **Densidade de easter egg (~10-15%):** NAO forcar Fibonacci/maconaria nestes 5 icones-mae (sao simbolos de sistema, devem ser limpos). Easter eggs entram em arte de cenario/personagem, nao em HUD funcional.

### Checklist de aprovacao do conjunto

1. Os 5 lado a lado parecem UM conjunto (mesmo canvas 32x32, mesma espessura de outline 1 px, mesmo nivel de detalhe, so muda cor e forma).
2. Silhueta de cada um distinta dos outros 4 (teste em preto).
3. Cada cor-marca bate exatamente com o hex canonico.
4. Legivel a 32 px e ainda legivel se reduzido a 24 px (margem de seguranca do HUD).
5. Fundo transparente, outline fechado.

---

## PARTE 2 - Os 5 prompts PixelLab (icones de FAMILIA, A1-A5)

Cada prompt e auto-contido. Os 5 compartilham a MESMA espinha de descricao (icone 32x32, outline 1 px escuro, cel 2 a 3 valores, fundo transparente, vista unica frontal, glow interno), mudando so a forma-assinatura e a cor. Isso garante o conjunto coeso.

---

### A1 - Eletrico (raio ramificado cyan)

**Prompt:**
```
A 32x32 pixel-art game HUD icon: a single sharp branching lightning bolt
(Lichtenberg fork) with 2 to 3 jagged angular branches, pointing diagonally,
self-lit electric cyan (#22D3EE) with a hot white-blue (#E8FBFF) glowing core
along the bolt. Clean cel-shading, 2 to 3 value bands per color, no soft
gradient. Closed 1px dark outline (near-black #0A0E1A) around the whole shape.
Bright readable silhouette, instantly recognizable as electricity. Minimal
internal detail, one strong angular motif. Front-facing flat icon, single
view, no rotation. Transparent background.
```
**Parametros PixelLab sugeridos:**
- size: 32 x 32 px
- view: single front-facing icon (NAO usar 4/8 direcoes)
- background: transparent
- outline: single dark outline
- detail: low (icone, motivo unico)
- palette: limited (cyan `#22D3EE`, core `#E8FBFF`, outline `#0A0E1A`)
- shading: cel / flat (hard bands)

**Assinatura a reforcar:** o raio ramificado anguloso (a familia mais rapida, burst). Angulos agudos = perigo/impacto (shape language). Cyan e a cor-marca do Caua.

---

### A2 - Bioquimico (gota/molecula verde)

**Prompt:**
```
A 32x32 pixel-art game HUD icon: a single rounded organic droplet or simple
molecule shape (a teardrop with one or two small bonded bubbles), self-lit
emerald green (#34D399) core with a thin violet (#A78BFA) rim and one small
violet spore-dot. Soft organic curves (not angular), reading as a living
toxin/bio-substance. Clean cel-shading, 2 to 3 value bands, no soft gradient.
Closed 1px dark outline (near-black #0A0E1A). Bright readable silhouette,
instantly recognizable as poison/biochemistry. Minimal internal detail. Front-
facing flat icon, single view, no rotation. Transparent background.
```
**Parametros PixelLab sugeridos:**
- size: 32 x 32 px
- view: single front-facing icon
- background: transparent
- outline: single dark outline
- detail: low
- palette: limited (green `#34D399`, violet `#A78BFA`, outline `#0A0E1A`)
- shading: cel / flat

**Assinatura a reforcar:** curva organica (vivo, DoT que se assenta). Verde-esmeralda das ampolas da Jaci mais violeta-esporo na borda. Curvas = organico (shape language).

---

### A3 - Sonico (aneis concentricos cobalto, leitura de sonar)

**Prompt:**
```
A 32x32 pixel-art game HUD icon: three concentric expanding rings radiating
from a central point (a sonar / sound-wave ping), the rings getting thinner
outward, self-lit cobalt blue (#3B82F6) with the crest highlights in pale ice
blue (#DBEAFE). Open, airy, ring-based silhouette (hollow center, parallel
arcs), reading clearly as a sound/sonar pulse and distinct from a solid shape.
Clean cel-shading, 2 to 3 value bands, no soft gradient. Closed 1px dark
outline (near-black #0A0E1A) on the rings. Instantly recognizable as sonic/
area pulse. Minimal internal detail. Front-facing flat icon, single view, no
rotation. Transparent background.
```
**Parametros PixelLab sugeridos:**
- size: 32 x 32 px
- view: single front-facing icon
- background: transparent
- outline: single dark outline
- detail: low
- palette: limited (cobalt `#3B82F6`, ice `#DBEAFE`, outline `#0A0E1A`)
- shading: cel / flat

**Assinatura a reforcar:** aneis concentricos = onda/area-CC; silhueta VAZADA (anel, nao solido) para separar do cyan eletrico mesmo em escala de cinza. Cobalto (cor nova canon, NAO violeta) mais cristas de gelo. Linda "Siren".

---

### A4 - Cinetico (engrenagem/cunha latao, mecanico-Asmodico)

**Prompt:**
```
A 32x32 pixel-art game HUD icon: a solid brass cog-wheel fused with a heavy
angular wedge / arrow pointing right (impact and knockback), self-lit burnished
brass-amber (#E8A33D) with cooler titanium-grey (#9AA6B2) shadow planes giving
it weight. Solid, mechanical, heraldic, heavier and chunkier than the other
family icons (analog clockwork feel, not glowing code). A few visible gear teeth
read at this size. Clean cel-shading, 2 to 3 value bands, no soft gradient.
Closed 1px dark outline (near-black #0A0E1A). Instantly recognizable as a
mechanical impact/gear. Minimal internal detail. Front-facing flat icon, single
view, no rotation. Transparent background.
```
**Parametros PixelLab sugeridos:**
- size: 32 x 32 px
- view: single front-facing icon
- background: transparent
- outline: single dark outline
- detail: low-medium (poucos dentes de engrenagem legiveis)
- palette: limited (brass `#E8A33D`, titanium `#9AA6B2`, outline `#0A0E1A`)
- shading: cel / flat

**Assinatura a reforcar:** engrenagem mais cunha = impacto/deslocamento; a familia MECANICA (excecao Pillar 2 do Bento, Asmodico/relojoaria), por isso a unica solida-metalica e a unica quente. Latao envelhecido (cronometro do Bento). Quadrado/cunha = solido que vira agressivo (shape language).

---

### A5 - Criptografico (cadeado/grade violeta)

**Prompt:**
```
A 32x32 pixel-art game HUD icon: a small angular padlock overlaid on a thin
orthogonal grid / wireframe, self-lit violet (#7C3AED) with the grid scan-lines
in bright fractal cyan (#67E8F9). The padlock shackle reads clearly; the grid
suggests decryption / structure being revealed. Geometric, ortho, system-like
(not organic, not glowing-soft), reading as cryptography / lock. Clean cel-
shading, 2 to 3 value bands, no soft gradient. Closed 1px dark outline (near-
black #0A0E1A). Instantly recognizable as a lock/decrypt symbol. Minimal
internal detail. Front-facing flat icon, single view, no rotation. Transparent
background.
```
**Parametros PixelLab sugeridos:**
- size: 32 x 32 px
- view: single front-facing icon
- background: transparent
- outline: single dark outline
- detail: low-medium (cadeado mais poucas linhas de grade)
- palette: limited (violet `#7C3AED`, fractal-cyan `#67E8F9`, outline `#0A0E1A`)
- shading: cel / flat

**Assinatura a reforcar:** cadeado mais grade = utility/anti-buff/exposicao (decifrar/decrypt); grade ortogonal = sistema/estrutura (shape language), distinta das curvas e dos aneis. Violeta da fibra optica da Iara mais fractal-cyan do "modo decifrar". Iara "Lumen".

---

## Status de validacao

ESTILO dos 5 icones de familia APROVADO pelo criador (rodados no PixelLab, escolhidos: Eletrico v3, Bioquimico v1, Sonico v0, Cinetico v2, Cripto v3). O dialeto da PARTE 1 (32x32, outline 1px `#0A0E1A`, cel 2-3 valores duros, fundo transparente, silhueta distinta, glow interno, paleta canon) esta TRAVADO. A PARTE 3 abaixo herda esse dialeto exato.

---

## PARTE 3 - Demais icones P0 (25 prompts)

Os 25 restantes do P0, herdando o dialeto travado na PARTE 1. Tres sub-grupos seguem o MESMO dialeto dos 5 de familia (32x32, outline 1px `#0A0E1A`, cel 2-3 valores, fundo transparente, vista unica frontal, glow interno): STATUS, INTENT, MODIFICADOR. O 4o sub-grupo (RETRATOS CTB) usa um dialeto-irmao com ajustes declarados (tamanho maior, fundo solido).

**Regra de cor transversal (status):** a cor do icone de status = cor da FAMILIA-ORIGEM do status (vfx-combate-familias.md). Status utilitarios sem familia (Shield/Haste/Slow) usam uma cor NEUTRA de HUD; Regen usa um verde-cura suave distinto do verde-toxina do Bioquimico (ver nota em cada um).

**Neutro de HUD (definicao):** para Shield/Haste/Slow uso o cinza-azulado frio do style-guide `#6B6F7A` (Neutral warm da City) com luz em `#9AA6B2` (titanio). Distinto das 5 cores-marca de familia (nao confunde status utilitario com status elemental).

### 13 STATUS

#### S1 - Stun (Eletrico)
**Prompt:**
```
A 32x32 pixel-art game HUD status icon: a small gear locked by a lightning bolt
jammed between its teeth (a stuck/frozen cog), self-lit electric cyan (#22D3EE)
with a hot white-blue (#E8FBFF) spark accent. Reads as "stunned / skip turn /
locked". Clean cel-shading, 2 to 3 hard value bands, no soft gradient. Closed
1px dark outline (#0A0E1A). Strong distinct silhouette. Minimal internal detail.
Front-facing flat icon, single view, no rotation. Transparent background.
```
**Parametros PixelLab:** size 32x32; view single front-facing icon; background transparent; outline single dark; detail low; palette limited (cyan `#22D3EE`, spark `#E8FBFF`, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** engrenagem-relampago TRAVADA (o "loop infinito" do vfx). Cyan = Eletrico (Caua). Perde o proximo turno.

#### S2 - Poison (Bioquimico)
**Prompt:**
```
A 32x32 pixel-art game HUD status icon: a small vial or flask tipping with a
single emerald-green (#34D399) droplet falling from it, self-lit green core with
a thin violet (#A78BFA) rim. Organic rounded shapes. Reads as "poison / damage
over time". Clean cel-shading, 2 to 3 hard value bands, no soft gradient. Closed
1px dark outline (#0A0E1A). Distinct silhouette. Minimal internal detail. Front-
facing flat icon, single view, no rotation. Transparent background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low; palette limited (green `#34D399`, violet `#A78BFA`, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** frasco/gota pingando (DoT que pinga do alvo no vfx). Verde-esmeralda das ampolas da Jaci.

#### S3 - Corrode (Bioquimico)
**Prompt:**
```
A 32x32 pixel-art game HUD status icon: a small shield or armor plate being
eaten away, with a jagged violet (#A78BFA) crack splitting it and emerald-green
(#34D399) corrosion eating the edges. Reads as "armor corroded / defense down".
Distinct from the poison flask icon: this one is a cracking plate, not a vial.
Clean cel-shading, 2 to 3 hard value bands, no soft gradient. Closed 1px dark
outline (#0A0E1A). Distinct silhouette. Minimal internal detail. Front-facing
flat icon, single view, no rotation. Transparent background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low-medium; palette limited (green `#34D399`, violet `#A78BFA`, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** fissura violeta na armadura (Corrode reduz Def alem do dano; "armadura comida" no vfx). Mesma familia do Poison mas silhueta de PLACA RACHADA, nao de frasco (diferenciar).

#### S4 - Disrupt (Sonico)
**Prompt:**
```
A 32x32 pixel-art game HUD status icon: a broken / interrupted sound wave (a
sonar arc cut in the middle with a gap, weak-signal bars), self-lit cobalt blue
(#3B82F6) with pale ice-blue (#DBEAFE) crest highlights. Reads as "disrupted /
weakened next action". Open ring/arc silhouette, hollow. Clean cel-shading, 2 to
3 hard value bands, no soft gradient. Closed 1px dark outline (#0A0E1A). Distinct
silhouette. Minimal internal detail. Front-facing flat icon, single view, no
rotation. Transparent background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low; palette limited (cobalt `#3B82F6`, ice `#DBEAFE`, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** onda QUEBRADA / sinal fraco (a proxima acao sai enfraquecida). Cobalto = Sonico (Linda). Silhueta vazada, irma do icone de familia Sonico.

#### S5 - Silence (Sonico)
**Prompt:**
```
A 32x32 pixel-art game HUD status icon: a sound wave with a diagonal slash
through it (a muted/struck-out wave, like a "no sound" symbol), self-lit cobalt
blue (#3B82F6) with the slash in pale ice-blue (#DBEAFE). Reads as "silenced /
cannot cast cards". Distinct from Disrupt: Disrupt is a broken/weak wave, Silence
is a wave struck out by a clear slash. Clean cel-shading, 2 to 3 hard value
bands, no soft gradient. Closed 1px dark outline (#0A0E1A). Distinct silhouette.
Minimal internal detail. Front-facing flat icon, single view, no rotation.
Transparent background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low; palette limited (cobalt `#3B82F6`, ice `#DBEAFE`, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** mute-glyph (onda RISCADA). Bloqueia jogar cartas. Diferenciar de Disrupt pela barra diagonal clara.

#### S6 - Knockback (Cinetico)
**Prompt:**
```
A 32x32 pixel-art game HUD status icon: a heavy angular wedge / chevron arrow
shoving rightward (a push-back marker), self-lit burnished brass-amber (#E8A33D)
with titanium-grey (#9AA6B2) shadow planes for weight. Solid mechanical look.
Reads as "knocked back / pushed in the turn queue". Clean cel-shading, 2 to 3
hard value bands, no soft gradient. Closed 1px dark outline (#0A0E1A). Distinct
silhouette. Minimal internal detail. Front-facing flat icon, single view, no
rotation. Transparent background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low; palette limited (brass `#E8A33D`, titanium `#9AA6B2`, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** seta-cunha de recuo (empurra na fila CTB). Latao = Cinetico (Bento), familia mecanica/quente.

#### S7 - Break (Cinetico)
**Prompt:**
```
A 32x32 pixel-art game HUD status icon: a cracked / split shield broken in two,
self-lit titanium-grey (#9AA6B2) with the fracture line glowing brass-amber
(#E8A33D). Reads as "armor broken / defense down". Distinct from Knockback (an
arrow) and from Corrode (a green-eaten plate): Break is a clean split shield with
a brass crack. Clean cel-shading, 2 to 3 hard value bands, no soft gradient.
Closed 1px dark outline (#0A0E1A). Distinct silhouette. Minimal internal detail.
Front-facing flat icon, single view, no rotation. Transparent background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low-medium; palette limited (titanium `#9AA6B2`, brass `#E8A33D`, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** escudo PARTIDO (reduz Def por Duration turnos). Familia Cinetico, mas latao na fissura e titanio no corpo (inverte enfase do Knockback pra diferenciar). Diferenciar de Corrode (que e verde-comido).

#### S8 - Expose (Criptografico)
**Prompt:**
```
A 32x32 pixel-art game HUD status icon: a targeting crosshair / reticle overlaid
on a small wireframe box, self-lit violet (#7C3AED) with the reticle lines in
bright fractal cyan (#67E8F9). Reads as "exposed / takes more card damage,
weak points revealed". Geometric, ortho. Clean cel-shading, 2 to 3 hard value
bands, no soft gradient. Closed 1px dark outline (#0A0E1A). Distinct silhouette.
Minimal internal detail. Front-facing flat icon, single view, no rotation.
Transparent background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low-medium; palette limited (violet `#7C3AED`, fractal-cyan `#67E8F9`, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** crosshair sobre wireframe (alvo marcado, recebe mais dano de carta). Violeta = Cripto (Iara). Irmao do icone de familia Cripto.

#### S9 - Decrypt (Criptografico)
**Prompt:**
```
A 32x32 pixel-art game HUD status icon: a buff aura / shield glyph shattering
into falling hex fragments (small breaking pieces flying off), self-lit violet
(#7C3AED) with the shatter sparks in bright fractal cyan (#67E8F9). Reads as
"buffs stripped / decrypted". Distinct from Expose (a crosshair): Decrypt is a
shattering symbol with flying fragments. Clean cel-shading, 2 to 3 hard value
bands, no soft gradient. Closed 1px dark outline (#0A0E1A). Distinct silhouette.
Minimal internal detail. Front-facing flat icon, single view, no rotation.
Transparent background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low-medium; palette limited (violet `#7C3AED`, fractal-cyan `#67E8F9`, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** buff ESTILHACANDO em hex (remove todos os buffs do alvo, sem lockout). Familia Cripto. Diferenciar de Expose pela acao de quebrar/cair fragmentos.

#### S10 - Shield (utilitario, neutro)
**Prompt:**
```
A 32x32 pixel-art game HUD status icon: a solid intact shield with a faint inner
fill bar suggesting an absorption pool, self-lit cool neutral steel-grey
(#6B6F7A) with titanium (#9AA6B2) highlight, a thin cool-cyan inner glow hinting
at an energy barrier. Reads as "shield / absorb pool". Deliberately NEUTRAL color
(not tied to any of the 5 families). Clean cel-shading, 2 to 3 hard value bands,
no soft gradient. Closed 1px dark outline (#0A0E1A). Distinct silhouette (intact
shield, unlike the broken Break shield). Minimal internal detail. Front-facing
flat icon, single view, no rotation. Transparent background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low; palette limited (steel `#6B6F7A`, titanium `#9AA6B2`, faint cyan inner, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** escudo INTEIRO com barra de pool interna (absorve dano antes do HP). Cor NEUTRA (utilitario, sem familia). Diferenciar de Break (escudo partido).

#### S11 - Regen (utilitario, verde-cura)
**Prompt:**
```
A 32x32 pixel-art game HUD status icon: a soft pulsing healing cross or a small
leaf with an upward sparkle, self-lit a soft mint/teal healing green (#67E8C8,
distinct and lighter than the toxic emerald of Poison) with a pale glow. Reads as
"regeneration / heal over time". Warm, friendly, rounded. Clean cel-shading, 2 to
3 hard value bands, no soft gradient. Closed 1px dark outline (#0A0E1A). Distinct
silhouette. Minimal internal detail. Front-facing flat icon, single view, no
rotation. Transparent background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low; palette limited (heal-green `#67E8C8`, pale glow, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** cruz/folha pulsando (cura por tick). Verde-CURA suave e mais claro (`#67E8C8`), DELIBERADAMENTE distinto do verde-toxina `#34D399` do Poison, pra nunca confundir cura com veneno na barra de status (Pillar 1, leitura tatica).

#### S12 - Haste (utilitario, neutro)
**Prompt:**
```
A 32x32 pixel-art game HUD status icon: an upward-pointing speed chevron (double
arrow up) with small motion lines, self-lit cool neutral steel-grey (#6B6F7A)
with a bright titanium (#9AA6B2) edge and a faint cool-cyan speed glow. Reads as
"haste / speed up". NEUTRAL color (utility, not a family). Clean cel-shading, 2
to 3 hard value bands, no soft gradient. Closed 1px dark outline (#0A0E1A).
Distinct silhouette. Minimal internal detail. Front-facing flat icon, single
view, no rotation. Transparent background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low; palette limited (steel `#6B6F7A`, titanium `#9AA6B2`, faint cyan, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** seta-velocidade pra CIMA (SPD+, recomputa fila). Neutro. Par visual com Slow (mesma forma, direcao oposta).

#### S13 - Slow (utilitario, neutro)
**Prompt:**
```
A 32x32 pixel-art game HUD status icon: a downward-pointing speed chevron (double
arrow down) with small heavy/drag lines, self-lit cool neutral steel-grey
(#6B6F7A) with titanium (#9AA6B2) edge, dimmer and heavier than the Haste icon.
Reads as "slow / speed down". NEUTRAL color (utility, not a family). Clean cel-
shading, 2 to 3 hard value bands, no soft gradient. Closed 1px dark outline
(#0A0E1A). Distinct silhouette. Minimal internal detail. Front-facing flat icon,
single view, no rotation. Transparent background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low; palette limited (steel `#6B6F7A`, titanium `#9AA6B2`, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** seta-velocidade pra BAIXO (SPD-). Neutro. Par com Haste (forma identica, direcao invertida e tom mais escuro/pesado para leitura instantanea de "ruim").

### 4 INTENT

Telegraph sobre o inimigo (estilo Slay the Spire, battle-screen.md decisao 5). Devem ler como "plano do inimigo neste turno". Mesmo dialeto 32x32. Cor: usar um AMBAR-ALERTA neutro de telegraph (`#FACC15`, o Hot signal do style-guide, ja e a cor de "alerta/UI critica" do jogo) como base dos intents, para que o jogador associe "intent" a uma cor unica de HUD, distinta das cores de familia. Excecao: o Ruido/Patch-Zero usa o vermelho-anomalia.

#### I1 - Atacar
**Prompt:**
```
A 32x32 pixel-art game HUD intent icon: a sword or angular attack arrow pointing
down-forward, self-lit alert amber-yellow (#FACC15) with a darker amber shadow,
leaving clear empty space beside or below it where a predicted damage number will
be drawn in code (do not draw any number, just the symbol with breathing room).
Reads as "this enemy will attack". Clean cel-shading, 2 to 3 hard value bands, no
soft gradient. Closed 1px dark outline (#0A0E1A). Distinct silhouette. Minimal
internal detail. Front-facing flat icon, single view, no rotation. Transparent
background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low; palette limited (alert-amber `#FACC15`, dark amber, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** espada/seta de ataque + ESPACO VAZIO pro numero previsto (o codigo desenha o numero ao lado; o icone so reserva o lugar). Ambar-alerta = "intent" como cor unica de HUD.

#### I2 - Defender
**Prompt:**
```
A 32x32 pixel-art game HUD intent icon: a raised shield (guard stance), self-lit
alert amber-yellow (#FACC15) with darker amber shadow. Reads as "this enemy will
defend / brace". Distinct from the Shield status (which is steel-grey): this
intent shield is amber, signaling a PLANNED action, not an active buff. Clean
cel-shading, 2 to 3 hard value bands, no soft gradient. Closed 1px dark outline
(#0A0E1A). Distinct silhouette. Minimal internal detail. Front-facing flat icon,
single view, no rotation. Transparent background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low; palette limited (alert-amber `#FACC15`, dark amber, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** escudo erguido (vai defender). Ambar = intent (diferencia do Shield status, que e cinza-aco): cor comunica "plano" vs "estado ativo".

#### I3 - Aplicar-status
**Prompt:**
```
A 32x32 pixel-art game HUD intent icon: a generic incoming hostile debuff symbol
(a small downward spiral aura with a down-arrow, or a hex glyph with a downward
mark), self-lit alert amber-yellow (#FACC15) with darker amber shadow. Reads as
"this enemy will apply a debuff". Neutral/generic (not tied to one family, since
it covers any status). Clean cel-shading, 2 to 3 hard value bands, no soft
gradient. Closed 1px dark outline (#0A0E1A). Distinct silhouette. Minimal
internal detail. Front-facing flat icon, single view, no rotation. Transparent
background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low; palette limited (alert-amber `#FACC15`, dark amber, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** simbolo GENERICO de debuff hostil chegando (cobre qualquer status, por isso neutro-ambar e nao a cor de uma familia). Down-arrow = piora chegando.

#### I4 - Ruido / Patch-Zero (intent caotico)
**Prompt:**
```
A 32x32 pixel-art game HUD intent icon: a scrambled, unreadable glitch glyph (a
corrupted symbol made of jumbled broken pixel fragments and random noise),
self-lit anomaly red (#F43F5E) with hot-yellow (#FACC15) noise flecks. Reads as
"unreadable / chaotic intent". Deliberately illegible and unsettling, distinct
from all clean intent icons. Clean cel-shading where possible, hard pixel noise.
Closed 1px dark outline (#0A0E1A) around the scrambled mass. Distinct silhouette
(broken/irregular). Front-facing flat icon, single view, no rotation. Transparent
background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail medium (ruido controlado); palette limited (anomaly-red `#F43F5E`, hot-yellow `#FACC15`, outline `#0A0E1A`); shading cel/flat com ruido.
**Reforcar:** simbolo EMBARALHADO/ilegivel (intent caotico, EXCLUSIVO de Patch-Zero, one-way door canon; Gambito-Prever retorna ruido). Vermelho-anomalia = a unica familia visual de glitch do jogo (style-guide 11.4). Quebra deliberada do ambar-alerta dos outros intents: o jogador SENTE que este e diferente.

### 3 MODIFICADOR

Modificadores da pipeline (combat.md paragrafo 8). Mesmo dialeto 32x32. Cor: cinza-aco neutro de UI (`#6B6F7A` + titanio `#9AA6B2`), pois modificador transforma carta de QUALQUER familia (nao pertence a uma cor de familia). Null tem um detalhe de alerta.

#### M1 - Object
**Prompt:**
```
A 32x32 pixel-art game HUD modifier icon: a small persistent totem / deployable
trap-node on the ground (a compact tech obelisk or turret-stub with a glowing
core), self-lit cool steel-grey (#6B6F7A) with titanium (#9AA6B2) edges and a
small neutral cyan core light. Reads as "Object: persistent entity on the field".
Clean cel-shading, 2 to 3 hard value bands, no soft gradient. Closed 1px dark
outline (#0A0E1A). Distinct silhouette. Minimal internal detail. Front-facing
flat icon, single view, no rotation. Transparent background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low-medium; palette limited (steel `#6B6F7A`, titanium `#9AA6B2`, faint cyan core, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** totem/armadilha PERSISTENTE no campo (+1 mana). Cinza-aco neutro (modificador aplica a qualquer familia).

#### M2 - Stream
**Prompt:**
```
A 32x32 pixel-art game HUD modifier icon: three parallel forward-flowing arrows /
a fanning multi-hit splash (suggesting repetition or area spread), self-lit cool
steel-grey (#6B6F7A) with titanium (#9AA6B2) highlights. Reads as "Stream:
single-target becomes area or multi-hit". Clean cel-shading, 2 to 3 hard value
bands, no soft gradient. Closed 1px dark outline (#0A0E1A). Distinct silhouette.
Minimal internal detail. Front-facing flat icon, single view, no rotation.
Transparent background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low-medium; palette limited (steel `#6B6F7A`, titanium `#9AA6B2`, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** fluxo multi-hit/area (+2 mana). Setas paralelas = repeticao/espalhamento. Neutro.

#### M3 - Null
**Prompt:**
```
A 32x32 pixel-art game HUD modifier icon: a cancellation glyph (a circle with a
diagonal bar, a "no/invert" symbol) over a tiny inverted-rune mark, self-lit cool
steel-grey (#6B6F7A) with titanium (#9AA6B2) and a small alert-amber (#FACC15)
accent on the bar to signal it is special / conditional. Reads as "Null: invert
or cancel an effect (requires prior Scan)". Clean cel-shading, 2 to 3 hard value
bands, no soft gradient. Closed 1px dark outline (#0A0E1A). Distinct silhouette.
Minimal internal detail. Front-facing flat icon, single view, no rotation.
Transparent background.
```
**Parametros PixelLab:** size 32x32; view single; background transparent; outline single dark; detail low-medium; palette limited (steel `#6B6F7A`, titanium `#9AA6B2`, alert-amber `#FACC15`, outline `#0A0E1A`); shading cel/flat.
**Reforcar:** inversao/cancelamento "barrado" (+1 mana, EXIGE Scan previo; o mais forte). Acento ambar = sinaliza "especial/condicional" (o jogo desabilita sem Scan, com a mensagem de erro do paragrafo 10). Neutro com toque de alerta.

### 4 RETRATOS CTB pequenos

**Decisao de dialeto (justificada):** os retratos da fila CTB usam um dialeto-IRMAO, com 2 desvios deliberados do dos icones:

1. **Tamanho 48x48 px** (nao 32x32): retrato precisa de rosto reconhecivel (aparelho do Gus, heterocromia da Jaci); a 32 px o rosto nao le. 48 e o minimo que carrega identidade facial em pixel art mantendo a faixa CTB compacta. (64 seria opcao se a faixa for grande; recomendo 48 e validar na tela.)
2. **Fundo SOLIDO escuro, NAO transparente.** Justificativa: (a) o retrato e um "selo" emoldurado na faixa CTB, nao um simbolo que flutua sobre paineis variados como os icones de status; um fundo solido consistente da unidade visual a fila e separa cada retrato do vizinho; (b) o fundo escuro `#1B2238` (City BG mid do style-guide) faz o cabelo ruivo do Gus e os glows (cyan do Caua, ampolas da Jaci) saltarem (contraste, figure/ground do style-guide secao 7); (c) facilita o highlight de "proximo ator" (a moldura muda de cor sobre fundo estavel). O codigo ainda pode aplicar um leve vignette/moldura por cima.

Proporcao SD 1:1:1 e canon, mas em retrato (busto/rosto) o que importa e a CABECA grande e os tracos-assinatura; enquadrar cabeca e ombros.

#### P1 - Retrato CTB Gus
**Prompt:**
```
A 48x48 pixel-art portrait bust for a turn-order queue: head and shoulders of an
11-year-old boy with a big round chibi head (SD proportions), bright saturated
orange-red (#FF6B1A) asymmetric hair dominating the upper-RIGHT quadrant, fair
warm skin (#F5E6D8), serious analytical expression (not smiling). Signature
traits, must be visible: silver-blue dental braces (#C0D8E8) on his teeth, a thin
tactical antenna and active cyan (#22D3EE) glasses lenses, a hint of his arm
computer at the shoulder. Cel-shaded, clean flat colors, hard bands, 1px dark
outline (#0A0E1A). Centered bust, front-facing. Solid dark background (#1B2238).
```
**Parametros PixelLab:** size 48x48; view single front-facing bust; background solid dark `#1B2238`; outline single dark; detail medium (rosto + tracos-assinatura); palette: hair `#FF6B1A`, skin `#F5E6D8`, braces `#C0D8E8`, lens cyan `#22D3EE`, outfit `#2B3A55`, bg `#1B2238`, outline `#0A0E1A`; shading cel/flat.
**Reforcar:** aparelho ortodontico + antena + lentes cyan + cabelo ruivo `#FF6B1A` no quadrante superior DIREITO (canon, style-guide don'ts). Expressao seria/analitica, nao fofa.

#### P2 - Retrato CTB Caua
**Prompt:**
```
A 48x48 pixel-art portrait bust for a turn-order queue: head and shoulders of a
13-year-old mixed-race boy with a big round chibi head (SD), warm brown skin,
confident cocky grin. Signature traits, must be visible: jet-black asymmetric
UNDERCUT with thin glowing electric-cyan (#22D3EE) circuit-line tattoos on the
shaved scalp, bright reactive eyes, a hint of cyan-glowing forearm accumulator at
the shoulder. Cyan is the only bright accent against dark techwear. Cel-shaded,
clean flat colors, hard bands, 1px dark outline (#0A0E1A). Centered bust, front-
facing. Solid dark background (#1B2238).
```
**Parametros PixelLab:** size 48x48; view single front-facing bust; background solid dark `#1B2238`; outline single dark; detail medium; palette: skin warm brown, hair black, circuit cyan `#22D3EE`, bg `#1B2238`, outline `#0A0E1A`; shading cel/flat.
**Reforcar:** undercut com circuitos cyan glow no couro cabeludo (cor-marca cyan, unico brilho). Grin confiante. Coerente com `CAUA_VOLT_IMAGEPROMPT.md` (mesma identidade, agora em busto pixel).

#### P3 - Retrato CTB Jaci
**Prompt:**
```
A 48x48 pixel-art portrait bust for a turn-order queue: head and shoulders of an
11-year-old girl with a big round chibi head (SD), severe pale near-vampiric skin
(genetic, not goth), calm serious expression. Signature traits, must be visible:
HETEROCHROMIA (one cyan #22D3EE eye, one golden #FACC15 eye), a grey kevlar lab-
coat collar, and small glowing emerald-green (#34D399) bioluminescent vials at
her shoulder/chest. Cel-shaded, clean flat colors, hard bands, 1px dark outline
(#0A0E1A). Centered bust, front-facing. Solid dark background (#1B2238).
```
**Parametros PixelLab:** size 48x48; view single front-facing bust; background solid dark `#1B2238`; outline single dark; detail medium; palette: pale skin, eye cyan `#22D3EE`, eye gold `#FACC15`, vials green `#34D399`, coat grey, bg `#1B2238`, outline `#0A0E1A`; shading cel/flat.
**Reforcar:** heterocromia (olho cyan + olho dourado) + ampolas verde-esmeralda bioluminescentes + jaleco kevlar cinza. Expressao calma/seria.

#### P4 - Retrato CTB inimigo generico (trash mob do slice)
**Prompt:**
```
A 48x48 pixel-art portrait bust for a turn-order queue: head and shoulders of a
generic low-tier cyber-gothic enemy (a corporate FIR drone-soldier or a small
glitch-construct), angular asymmetric silhouette (uncanny corporate shape
language), faceless or visored, dark steel-blue body (#3A4566) with a single
hostile magenta (#E11D74) accent light. Reads instantly as "enemy / not party".
Cel-shaded, clean flat colors, hard bands, 1px dark outline (#0A0E1A). Centered
bust, front-facing. Solid dark background (#1B2238).
```
**Parametros PixelLab:** size 48x48; view single front-facing bust; background solid dark `#1B2238`; outline single dark; detail medium; palette: body steel-blue `#3A4566`, hostile magenta `#E11D74`, bg `#1B2238`, outline `#0A0E1A`; shading cel/flat.
**Reforcar:** placeholder de trash mob: silhueta ANGULAR/uncanny (inimigo cidade = triangular, style-guide secao 7) + UM neon hostil magenta (inimigo pode ter UM neon; party tem mais). Le na hora como "inimigo". Substituir por retratos especificos quando o bestiario do slice fechar.

### 1 MOLDURA de carta

#### F1 - Moldura/frame de carta (object)
**Prompt:**
```
A pixel-art trading-card frame for a hand-of-cards UI: a vertical portrait
rectangular card frame (empty inner area for the card art/info, drawn by code),
with an ornate-but-clean angular rune border in burnished brass (#E8A33D) on a
dark charcoal (#1B2238) card face, small decorative corner runes, cyber-gothic
arcane-tech look. The inner field is empty/flat (a placeholder fill) so code can
composite the card name, mana cost and family color on top. Clean cel-shading,
hard bands, 1px dark outline (#0A0E1A). Straight-on flat view, single, no
rotation. Transparent background outside the card silhouette.
```
**Parametros PixelLab:** size 64x96 px (proporcao retrato de carta ~2:3; ajustar ao leque na tela); view single straight-on; background transparent (fora da carta); outline single dark; detail medium (borda runica + cantos); palette: brass border `#E8A33D`, charcoal face `#1B2238`, outline `#0A0E1A`; shading cel/flat.
**Variantes por familia (tint da borda):** gerar a moldura-base UMA vez; o codigo (ou 5 variantes geradas) re-tinta a BORDA pela cor da familia da carta: Eletrico cyan `#22D3EE`, Bioquimico verde `#34D399`, Sonico cobalto `#3B82F6`, Cinetico latao `#E8A33D` (ja e a base), Criptografico violeta `#7C3AED`. Recomendo gerar a base em latao neutro e tintar em runtime (1 asset, 5 leituras; barato pra solo G1, coerente com a estrategia de atlas tintado do vfx-combate-familias.md).
**Reforcar:** frame do slot do leque (battle-screen.md 3.2: carta compacta mostra cor da familia + nome + mana + marcador rapida/lenta). Coerente com `runic_cards/` (borda rúnica de latao, face charcoal), mas aqui e a MOLDURA VAZIA reutilizavel, nao a arte de uma carta especifica.

---

## Proximos passos (apos esta leva)

1. Rodar os 25 no PixelLab e o criador escolher as versoes (como nos 5 de familia).
2. Validar coesao do conjunto inteiro do P0 (30 icones lado a lado: mesmo dialeto, status legiveis na barra, intents distintos das familias, retratos legiveis na faixa CTB).
3. Fechado o P0, seguir para P1 (VFX de ataque das 5 familias como sprite-sheets/animation + poses de batalha cast/ataque/hit/vitoria; depende ainda da decisao 2D-vs-3D da arena).

---

## PARTE 4 - Retratos do elenco (5 personagens restantes)

Bustos CTB do restante do elenco, MESMO dialeto-irmao dos retratos ja aprovados (Gus/Caua/Jaci na PARTE 3): busto pixel-art 48x48, cel-shading 2-3 valores duros, 1px dark outline `#0A0E1A`, fundo SOLIDO dark slate `#1B2238` (nao transparente), cabeca grande SD, enquadrar cabeca e ombros, tracos-assinatura canonicos martelados.

Fontes canon: `docs/narrative/characters/{linda-siren,bento-requiem,iara-lumen,dante-grid,sterling-locke}.md`. Cada cor-marca segue a familia/lore do personagem.

**Nota de idade/proporcao:** os 4 companions (Linda 12, Bento 14, Iara 12, Dante 13) sao criancas/adolescentes SD 1:1:1 (cabeca grande). **Sterling Locke e ADULTO** (antagonista, anti-Pillar 4): proporcao ligeiramente alongada (style-guide secao 3, NPC adulto 1:1:1.2), cabeca menos inflada, postura aristocratica fria. E o unico do conjunto que NAO e chibi-crianca, de proposito (hierarquia visual: o predador adulto destoa do elenco infantil).

#### P5 - Retrato CTB Linda "Siren"
**Prompt:**
```
A 48x48 pixel-art portrait bust for a turn-order queue: head and shoulders of a
12-year-old girl with a big round chibi head (SD), agile lean post-punk build,
sharp lateral scanning glance (always half-listening to the environment). Signature
traits, must be visible: chaotic spiky hair, a black leather jacket with neon
fuchsia (#E11D74) accents, large industrial headphones over the ears, small
pulsing audio modules on the shoulder pads glowing cobalt blue (#3B82F6), and a
thin chain necklace holding a single phonograph needle as a pendant. Cobalt and
fuchsia are her accents against black. Cel-shaded, clean flat colors, hard bands,
1px dark outline (#0A0E1A). Centered bust, front-facing. Solid dark background
(#1B2238).
```
**Parametros PixelLab:** size 48x48; view single front-facing bust; background solid dark `#1B2238`; outline single dark; detail medium; palette: hair black, jacket black, fuchsia `#E11D74`, audio-module cobalt `#3B82F6`, needle pendant silver, bg `#1B2238`, outline `#0A0E1A`; shading cel/flat.
**Reforcar (canon):** fones industriais (tic: ajusta sempre) + spiky hair caotico + jaqueta couro preta com fucsia-neon + modulos de audio nas ombreiras + PINGENTE de agulha de toca-discos no pescoco (wound canon). Cor-marca cobalto = familia Sonico (vfx). Olhar lateral escaneando.

#### P6 - Retrato CTB Bento "Requiem"
**Prompt:**
```
A 48x48 pixel-art portrait bust for a turn-order queue: head and shoulders of a
14-year-old boy (the oldest of the party), big chibi head (SD) but sturdy robust
build, calm steady firm expression (an emotional anchor, never panics). Signature
traits, must be visible: heavy gothic armor in titanium-grey (#9AA6B2) and aged
burnished brass (#E8A33D), a mechanical clockwork chronometer mounted on his chest
with visible gears, the top edge of a cathedral-like mechanical shield at his
shoulder, brass clockwork ornament with a faint warm glow. Warm brass and cold
titanium, gothic-industrial. Cel-shaded, clean flat colors, hard bands, 1px dark
outline (#0A0E1A). Centered bust, front-facing. Solid dark background (#1B2238).
```
**Parametros PixelLab:** size 48x48; view single front-facing bust; background solid dark `#1B2238`; outline single dark; detail medium-high (armadura + cronometro); palette: titanium `#9AA6B2`, brass `#E8A33D`, bg `#1B2238`, outline `#0A0E1A`; shading cel/flat.
**Reforcar (canon):** armadura gotica titanio + latao envelhecido + CRONOMETRO mecanico no peito (engrenagens visiveis) + escudo-catedral na ombreira. Latao = familia Cinetico/Asmodico (exceção Pillar 2, relojoaria). O mais velho/robusto da party; expressao firme de ancora emocional.

#### P7 - Retrato CTB Iara "Lumen"
**Prompt:**
```
A 48x48 pixel-art portrait bust for a turn-order queue: head and shoulders of a
12-year-old girl with a big chibi head (SD), pale skin, slender elongated frame,
guarded hard-to-read elegant expression (defensive opacity). Signature traits,
must be visible: long hair braided with thin optical-fiber strands glowing violet
(#7C3AED) and emerald (#34D399), a matte-black outfit, and a holographic
refraction cape at her shoulders shimmering with a faint fractal-cyan (#67E8F9)
prismatic sheen. Violet is her color-brand. Cel-shaded, clean flat colors, hard
bands, 1px dark outline (#0A0E1A). Centered bust, front-facing. Solid dark
background (#1B2238).
```
**Parametros PixelLab:** size 48x48; view single front-facing bust; background solid dark `#1B2238`; outline single dark; detail medium; palette: pale skin, hair dark with fiber violet `#7C3AED` + emerald `#34D399`, matte black outfit, cape sheen fractal-cyan `#67E8F9`, bg `#1B2238`, outline `#0A0E1A`; shading cel/flat.
**Reforcar (canon):** cabelo longo trancado com FIBRA OPTICA violeta/esmeralda + traje preto matte + CAPA de refracao holografica. Cor-marca violeta = familia Criptografica/Oxido. Expressao fechada/elegante (opacidade defensiva, flaw canon).

#### P8 - Retrato CTB Dante "Grid" (o TRAIDOR)
**Prompt:**
```
A 48x48 pixel-art portrait bust for a turn-order queue: head and shoulders of a
13-year-old boy with a big chibi head (SD), a friendly approachable competent
look on the surface with a subtle guardedness in the eyes (he hides something).
Signature traits, must be visible: a practical scrapyard-tech mechanic look,
modular tool-fingers / segmented gloves on a visible hand at the shoulder, a
utility harness with salvaged hardware, and a faint amber (#FACC15) work-light.
ONE small detail is OFF: a single thin cold-red (#F43F5E) status LED hidden among
the amber tools (a quiet wrong note, foreshadowing the betrayal, never obvious).
Cel-shaded, clean flat colors, hard bands, 1px dark outline (#0A0E1A). Centered
bust, front-facing. Solid dark background (#1B2238).
```
**Parametros PixelLab:** size 48x48; view single front-facing bust; background solid dark `#1B2238`; outline single dark; detail medium; palette: skin, dark scrap-tech outfit, amber work-light `#FACC15`, ONE hidden red LED `#F43F5E`, bg `#1B2238`, outline `#0A0E1A`; shading cel/flat.
**Reforcar (canon):** estetica de mecanico da Periferia Industrial / Ferrovelhos (sucata, hardware salvado) + DEDOS MODULARES (upgrade canon) + harness de utilidade. Aparencia AMIGAVEL e competente (engana o player). Easter egg de traicao: UM detalhe frio errado (LED vermelho-anomalia escondido entre o ambar), velado, nunca obvio (foreshadow canon; player Knowledge alta nota). NAO rotular.

#### P9 - Retrato CTB Sterling Locke (antagonista adulto)
**Prompt:**
```
A 48x48 pixel-art portrait bust of an adult antagonist for a turn-order queue:
head and shoulders of a cold aristocratic corporate predator, NOT chibi (slightly
elongated adult proportions, smaller head than the child cast), gaunt severe
face, frozen disdain, an immobile eyebrow and a fixed stare. Signature traits,
must be visible: a sterile palette of polished black graphene, matte sterile white
and mirror-chrome, a circular cryptographic ocular implant over the LEFT eye
projecting a forward-pointing saturated red (#FF0000) laser line (his single
signature threat color, the only saturated red outside virus zones). Impeccable
corporate attire, mirror-chrome accents (the only metallic-reflective figure in
the game). Cel-shaded, clean flat colors, hard bands, 1px dark outline (#0A0E1A).
Centered bust, front-facing. Solid dark background (#1B2238).
```
**Parametros PixelLab:** size 48x48; view single front-facing bust (proporcao adulta, cabeca menor que o elenco infantil); background solid dark `#1B2238`; outline single dark; detail medium-high (monoculo + atire); palette: polished black, sterile white, mirror-chrome, ocular laser red `#FF0000`, bg `#1B2238`, outline `#0A0E1A`; shading cel/flat (com 1 ponto especular cromado, exceção autorizada).
**Reforcar (canon):** ADULTO predador (anti-Pillar 4, sem redencao) destoa do elenco infantil (proporcao alongada, cabeca menor). MONOCULO ocular criptografico no olho ESQUERDO com laser vermelho `#FF0000` (a unica assinatura vermelha saturada fora de zonas Patch-Zero, a ameaca corporativa). Paleta esteril: preto-graphene polido + branco-esteril + cromo espelhado (unica figura com refletividade metalica plena no jogo). Expressao: desprezo congelado, sobrancelha imovel, aristocratico frio.
