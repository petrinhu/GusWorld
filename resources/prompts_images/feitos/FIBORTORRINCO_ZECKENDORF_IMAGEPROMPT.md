# Fibortorrinco-Zeckendorf (Fibolatypus-Zeckendorf): image prompt (nano banana)

> Inimigo de combate novo, proposto pelo líder e pelo Gus Dragon. Sorteado com mais frequência em cavernas, menos na cidade. Vida muito alta (saco de pancadas), um único golpe — o esporão, que pode envenenar — e larga uma unha venenosa que vira ingrediente de craft. Perspectiva do jogo: três quartos vista de cima, fixa, quatro direções cardeais desenhadas à mão, sem espelhamento (L-26) — por isso este documento pede **quatro vistas distintas**, cada uma seu próprio prompt autocontido, em vez do fluxo de referência única + rotação automática usado hoje para o elenco humanoide (`docs/art/sprites-inventory.md`, ritual PixelLab `create_character` mode="v3"). Mais uma vista de ataque, porque o esporão é a única ação de combate dele e precisa de pose própria.
>
> **Nome:** Fibortorrinco-Zeckendorf (pt-br) e Fibolatypus-Zeckendorf (inglês), cunhados pelo líder em 06/09/2026, no padrão Animal-Conceito da casa (Símio-Fibonacci, Tartaruga-Voronoi, Coruja-Mandelbrot). "Fibortorrinco" enxerta Fibonacci em ornitorrinco/platypus; "Zeckendorf" é a metade que carrega o conceito matemático exato. O teorema de Zeckendorf diz que todo inteiro positivo se escreve de uma única maneira como soma de números de Fibonacci não consecutivos, sem solução parcial: é a regra da unha (144 unhas frescas rendem 2 gotas de veneno, porque 144 cai certo na sequência; 89 unhas renderiam, em teoria, 1 gota, mas essa reação não tem receita definida no jogo, desvantajosa e impraticável mesmo se existisse; 72, metade de 144, não rende nada, porque metade do insumo não é meia solução). O nome não descreve o padrão de pele abaixo, descreve a mecânica do item de drop. Identificador de dado/código deriva de `fibolatypus` (inglês, `snake_case`, L-22).
>
> **Tocado pelo mundo, não bicho comum:** os seis exemplares de fauna já existentes no corpus (Coruja-Mandelbrot, Fungo-Relógio, Raposa-Fractal, Salamandra-Perlin, Símio-Fibonacci, Tartaruga-Voronoi) seguem, sem exceção, a mesma fórmula — pelagem/pele marcada por um padrão matemático glowing ciano-fraco, atrelado ao nome do bicho. `pillars.md` reforça isso na mecânica (ruído de Perlin como RNG de comportamento de fauna) e a Selve Sombria tem fauna explicitamente "biocibernética" com "padrões numéricos recorrentes visíveis" (`pillars.md` linha 182). O corpus decide isso por unanimidade, então este prompt segue a mesma convenção, sem inventar anatomia fantástica: só o padrão de pele + o glow, nada de membro ou olho extra — o ornitorrinco real já é estranho o bastante.

---

## Identidade compartilhada (colar em TODOS os prompts abaixo, para travar consistência entre as gerações)

A cute stylized low-poly 3D creature: a platypus, but built STOCKY and LOW-SLUNG rather than tiny and fragile — broad flat body close to the ground, thick short legs, a heavy dense torso that reads as tough and hard to knock down (this is a high-HP damage-sponge enemy, NOT a delicate critter; the silhouette must communicate durability, not cuteness-first). Naturalistic platypus anatomy only, no extra limbs or eyes: a wide flat rubbery bill, small dark eyes set high on the head, a broad flat beaver-like tail, webbed feet. Dense short fur in dark slate-grey and deep charcoal-brown, marked with a faint glowing cyan reaction-diffusion pattern — irregular blotchy spots that blur and merge at their edges like a Turing pattern, denser along the spine and tail, thinning toward the belly (the same "world-touched" glowing-cyan-pattern language as the rest of the game's fauna, kept minimal and naturalistic, not a new anatomical feature).

ESPORÃO (the venomous spur, its only attack, must read clearly even small): a curved, hooked, dark horn-like spine on the inner hind ankle, larger and more prominent than a real platypus spur, with a faint venom-green (`#34D399`, the game's canonical Bioquímico/poison family color) glow bleeding from its tip along a thin vein up the leg — this is the single loudest color accent on the whole creature, so the player learns to watch it.

UNHA VENENOSA (the drop, becomes a craft ingredient — must be recognizable as "the thing you loot"): one front claw noticeably larger, glossier and more defined than the others, same faint venom-green tint at the tip as the esporão, so the dropped item icon reads as unmistakably the same claw.

Art style: cel-shaded anime 3D, low-poly stylized game asset, clean flat colors with soft cel shading, somber cyber-gothic nature mood with the venom-green accent as the one hot color against cool slate/charcoal/faint-cyan. Enemy-tier asset: lean/reduced color palette (not the protagonist's rich palette — `docs/art/style-guide.md` §9, decisão do líder 30/08/2026). Flat even neutral studio lighting, no dramatic shadows, no cast shadow, no scenery (clean asset turnaround for sprite extraction; the plain background lets the game engine composite the creature over both cave and city tiles, per `docs/art/style-guide.md`'s PixelLab pipeline).

---

## As quatro vistas cardeais (cada bloco é um prompt autocontido — cole a identidade compartilhada acima antes de cada um)

Câmera fixa em três quartos vista de cima (3/4 top-down), a mesma usada no resto do elenco. Nenhuma das quatro é espelhamento de outra: gere cada uma como desenho independente, mesmo mantendo a mesma identidade (L-26). Referencie a vista anterior já aprovada ao gerar a próxima, para travar proporção e padrão de pele — mas não recorte/inverta pixel algum entre elas.

### Sul (south) — vista padrão, de frente para a câmera/jogador

[identidade compartilhada] + Full body, facing the camera (south-facing, toward the viewer/player), standing on all fours in a low wide stance, bill and both eyes visible, the esporão-bearing hind leg turned slightly outward so the curved spur reads clearly against the flank, tail resting flat behind. Centered, plain solid pure-white (#FFFFFF) background.

### Norte (north) — de costas

[identidade compartilhada] + Full body seen from directly behind (north-facing, back to the viewer), the broad flat tail and the dense cyan-marked fur along the spine as the dominant read, both hind legs visible with the esporão leg's curved spur silhouette still readable in profile from this angle. Centered, plain solid pure-white (#FFFFFF) background.

### Leste (east) — perfil direito

[identidade compartilhada] + Full body in right-side profile (east-facing), the bill pointing right, low stocky stance emphasized by the long horizontal silhouette, the esporão clearly silhouetted against the hind leg on this side, faint venom-green glow visible along its curve. Centered, plain solid pure-white (#FFFFFF) background.

### Oeste (west) — perfil esquerdo, NÃO é o leste espelhado

[identidade compartilhada] + Full body in left-side profile (west-facing), the bill pointing left, drawn independently from the east view (same stance and proportions, but its own linework and shading pass, not a mirrored copy), the esporão leg and its venom-green glow visible on this side. Centered, plain solid pure-white (#FFFFFF) background.

---

## Vista de ataque (esporão) — a única ação de combate dele

[identidade compartilhada] + Dynamic attack pose: the platypus lunging forward low to the ground, the esporão-bearing hind leg kicked back and up in a sharp striking arc, the curved spur leading the motion with its venom-green glow intensified and a thin trailing streak of green light along the arc to sell the strike, bill open in a hiss, weight low and braced on the front legs (reads as a heavy, committed strike from a tank, not a quick jab). Centered, plain solid pure-white (#FFFFFF) background, flat even lighting (no dramatic shadow, so the extraction stays clean).

---

## Nota técnica (tamanho, paleta, próximo passo)

- **Tamanho-alvo do sprite final:** 180×180 px, o canônico para todo o elenco sem diferenciação por tier (`docs/art/style-guide.md` §8, decisão do líder 30/08/2026) — as imagens geradas aqui em alta resolução servem de referência para o passo de pixel-art via PixelLab, igual ao resto do elenco (`docs/art/sprites-inventory.md`).
- **Paleta:** enxuta (regime de NPC/inimigo, não a paleta rica do Gus), acento único quente em verde-veneno `#34D399` (cor canônica da família Bioquímico, `docs/art/vfx-combate-familias.md` via `BATTLE_ICONS_M5_PIXELLAB.md`) contra o resto frio em ciano-fraco/ardósia/carvão.
- **Fundo:** branco sólido nas cinco gerações (convenção da casa); a remoção de fundo e o corte para pixel-art acontecem no passo seguinte do pipeline PixelLab, não aqui.
- **Onde isto entra no jogo:** as quatro vistas cardeais alimentam `resources/sprites/<slug>/{south,north,east,west}.png`; a vista de ataque alimenta retrato de combate/animação de ataque quando o M5 BattleScreen chegar a esse conteúdo — este documento só entrega o pedido de arte, não decide o wiring técnico.
- **Pendência que fica com o líder:** confirmar se cavernas específicas do grafo de 13 áreas (`docs/design/mundo-topologia.md`) recebem taxa de sorteio maior; este prompt não presume qual.
