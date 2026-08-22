# Gus "Dragon" Vector Tavus Vance - Image Generation Prompt

## Contexto de lore (para a IA entender o mundo)

**Personagem:** Gus "Dragon" Vector Tavus Vance (protagonista). Nome formal canon: Gustaf VII Tavus Vance.
**Idade:** 11 anos (CANONICO e inegociavel - protagonista crianca, NUNCA adulto, NUNCA adolescente mais velho).
**Classe:** Utility / Control. **Linguagem-ancora:** C-Arcane.
**Mundo:** ciber-gotico onde magia = software (feiticos sao scripts runicos compilados). Gus resolve TUDO por logica e otimizacao (xadrez, TCG, padroes), NUNCA por forca fisica. Prodigio analitico, nao power-fantasy de guerreiro.

**Tres amarras de hardware (Pillar 3 - sempre visiveis, definem a silhueta):** Oculos Taticos (varredura/HUD) <-> Matriz Ortodontica (antena UHF/VHF nos dentes) <-> Tavus-Drive de pulso (executor de cartoes). Os tres SAO o personagem - sem eles nao e o Gus.

---

## Production target (style and rendering pipeline)

- Estilo: anime 3D estilizado, **cel-shaded** (bandas duras de toon shading, SEM gradiente PBR), low-poly. Refs: Sea of Stars / Sable / Death's Door.
- Proporcao: **Super Deformed (SD) chibi rigoroso 1:1:1** - cabeca = torso+pelve = pernas, cada um exatamente 1/3 da altura total, tres cabecas de altura. Cute mas com olhar analitico afiado.
- Membros DELGADOS (ectomorfo), nunca inflados como chibi tradicional - preservar a silhueta esguia de crianca de 11 anos.
- **Fundo branco solido absoluto `#FFFFFF`, shader Unlit, ZERO sombra no plano de fundo.** Personagem isolado, pronto pra recorte e conversao em pixel art / sprite de jogo.
- Vista frontal de corpo inteiro, projecao ortografica, contato visual direto com a camera. Iluminacao de estudio com rim-light nitido.

---

## Technical Prompt (corpo do prompt para o gerador)

Full body 3D game-asset sprite of an 11-year-old boy named Gus, an analytical child prodigy in a cyber-gothic world. STRICT Super Deformed chibi proportion 1:1:1 - head equals torso equals legs, three heads tall total, but with thin ectomorphic limbs (slight, slender build of a real 11-year-old, NOT a puffy chibi). Anime 3D style, cel-shaded with hard toon bands, low-poly, no PBR.

ETHNICITY AND SKIN: fair very pale skin, matte and low-saturation, classic Stokerian pallor of someone deprived of urban sunlight; skin shadows use cool, faintly purplish undertones.

HAIR (SIGNATURE - color-brand): bright VIBRANT NEON ORANGE hair, messy spiky asymmetric post-punk cut, angular sharp locks falling mostly over the UPPER-RIGHT quadrant of the face; dark-brown roots with gothic ambient occlusion for high-frequency contrast against the pale skin.

GLASSES (Pillar 3 hardware vertex - MUST be present): low-profile polygonal TACTICAL glasses fitted to the SD head, semi-translucent lenses with active glowing CYAN/turquoise emission showing floating data scan-lines. These are technical interface equipment, NOT sunglasses, NOT fashion.

BRACES (Pillar 3 hardware vertex - MUST be visible, shown with PRIDE): the Matriz Ortodontica orthodontic apparatus on his teeth, visible in an ironic confident half-smile - tiny metallic micro-cube brackets with high specular highlights connected by a dark-silver graphene/tantalum tensor wire that catches the light. This is a functional UHF/VHF antenna amplifier, a source of TECHNICAL PRIDE, never a deformity, never hidden.

WRIST DEVICE (Pillar 3 hardware vertex - MUST be present): on his LEFT wrist, the Tavus-Drive, a chunky protruding burnished-brass card-executor module. It is a tool, NOT a weapon.

CLOTHING: long dark cyber-gothic synthetic TRENCH COAT in carbonized dark grey, fitted to the 1:1:1 proportion; coat folds and high-shaft boots use matte synthetic Kevlar and dark leather with normal-mapped relief.

COLOR PALETTE: cool cyan/teal accents (glasses, technical glow) + neon orange hair + dark grey, black and deep maroon gothic base.

EXPRESSION: intelligent, analytical, eyes that scan and process everything; calm calculating gaze, the look of a mind solving a puzzle. NOT a warrior, NOT aggressive - he wins by LOGIC, not force.

POSE: standing, full body visible, frontal view, orthographic projection, direct eye contact with the camera. Pure solid white `#FFFFFF` background, isolated studio lighting, crisp clean edges, no shadows on the background.

[RENDER_PARAMS]: --ar 1:1 --style raw --v 6.0

---

**Lore Notes for AI (reforco - tracos fracos a IA apaga, manter fortes):**
- Idade 11 e CANONICA - crianca, nunca adulto.
- As 3 amarras de hardware (oculos taticos cyan + Matriz Ortodontica nos dentes + Tavus-Drive no pulso ESQUERDO) sao OBRIGATORIAS e visiveis - elas definem o personagem (Pillar 3).
- Cor-marca: cabelo LARANJA NEON + emissao CYAN. Pele PALIDA Stokeriana.
- Cabelo cobre majoritariamente o quadrante superior DIREITO da face.
- Matriz Ortodontica = ORGULHO TECNICO, mostrar com sorriso ironico confiante, jamais como defeito.
- Tavus-Drive = executor de cartoes (ferramenta), NUNCA arma.
- Sobretudo gotico, NAO armadura militar. Resolve por LOGICA, nao forca - classe Utility/Control.
- Excecoes de material permitidas: Normal Mapping SO em hard-surface (jaqueta/botas); Specular alto SO nos braquetes metalicos; resto do char flat cel-shaded.
- SD 1:1:1 chibi real (cabeca 1/3), NAO realista-estilizado.
- Fundo `#FFFFFF` unlit vale para sprite/portrait/key-art (recorte + pixel art). In-game o char fica no ambiente.
