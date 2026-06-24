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

## Proximos passos (apos validacao do criador)

1. Criador valida o ESTILO nestes 5 (coesao do conjunto, espessura, tamanho, leitura).
2. Ajustes finos no dialeto, se houver (ex: 32 vs 48 px, espessura de outline).
3. So entao escrever os 25 prompts restantes do P0 (13 status, 4 intent, 3 modificador, 4 retratos CTB, 1 moldura de carta), todos herdando o dialeto travado aqui.
4. Gerados/aprovados os icones, seguir para P1 (VFX de ataque das 5 familias, poses de batalha).
```
