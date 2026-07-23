# Inventário de props e arte de lore (`resources/images/`)

Mapa de tudo que vive em `resources/images/` — **props, lore, cartas, símbolos,
conceito** — distinto dos sprites de jogo direcionais (esses estão em
`resources/sprites/`, ver [`sprites-inventory.md`](sprites-inventory.md)).

> Organizado 2026-07-23. Os PNGs são **gitignored** (`resources/images/*`), só no
> disco+IDrive; este mapa é versionado. Antes disso era um dump plano de ~69
> arquivos na raiz; reorganizado em subpastas espelhando a taxonomia de
> `resources/prompts_images/` (onde vivem os prompts de geração).

## Regra de ouro

- **`vance_dragon_glyph.png` FICA NA RAIZ e NÃO se move** — o jogo carrega ele de
  `resources/images/` (`asset_paths.hpp` → `kVanceDragonGlyphFile`), é o glifo de
  abertura do cockpit. É o único arquivo aqui que o runtime lê, e é git-tracked.
- Todo o resto é referência/lore, não é lido pelo jogo (ainda).

## Subpastas (por categoria)

| Pasta | Qtd | O que é |
|---|---|---|
| `flora/` | 6 | plantas fractais: carvalho recursão, eucalipto fractal, girassol sombrio, samambaia L-system, erva de pedra, acaceiro tronco vermelho |
| `fauna/` | 6 | criaturas matemáticas: coruja Mandelbrot, raposa fractal, salamandra Perlin, tartaruga Voronoi, símio Fibonacci, fungo relógio |
| `architecture/` | 9 | colunas Boróstoma/Janor/quebrada, ashlar stones, pavimento tesselado, mármore São Vargas, catedral mãe, boca da funda (+detail) |
| `consumables/` | 7 | comida/poção: bolo de bit, café de neurônio, feijão computacional, pão de engrenagem, sopa 7 raízes, semente relíquia, ampola |
| `runic_cards/` | 5 | as 5 cartas-família: electric, sonic, biochemical, cryptographic, kinetic |
| `logos_glyphs/` | 7 | cripto-glifo Pigpen, Pigpen grid, selo glyph Vyr, Sterling sigma logo, partitura última frequência, selo pergunta, QRCode |
| `traditions/` | 3 | avental de aprendiz, cordão 89 nós, esquadro+compasso do Joaquim |
| `vehicles/` | 5 | FIR truck, runner bike, Sterling van, toca-discos da Linda, underground wave |
| `props/` | 10 | agulha da Linda, medalha do Vorto, caderno do Salviano, Tavus-Drive, óculos táticos, matriz ortodôntica, tomo pilha, scanner do Yakov, drone do Sterling, recursive order gear |
| `ui_frames/` | 2 | runic UI frame, buymecoffe |
| `homenageados/` | 1 | RETRATOS de figuras homenageadas/lendárias (não são glifo nem prop). Hoje: `heliaco_vyr` (figura lendária Era-1, eco de Hiram Abiff). Os homenageados REAIS (Faraday, Turing, Gödel, von Neumann...) têm a doc do tributo em `docs/design/roster-analogos/` (21 figuras); a arte deles, quando houver, vem pra cá. |
| `card-frame-tests/` | vários | mockups de moldura de carta (alguns TRACKED no git) |

## Easter eggs embutidos (não rotular em doc público — velado)

- **`flora/` + `fauna/`** carregam o easter egg **Fibonacci** (recursão, fractais,
  L-system, Mandelbrot/Perlin/Voronoi). Ver `project_fibonacci_easter_egg`.
- **`architecture/` + `traditions/`** carregam o easter egg **maçônico** (ashlar
  bruto/polido, pavimento tesselado, colunas, esquadro+compasso, cordão de 89 nós,
  avental). Ver `project_eastereggs_maconaria_canonica`.

## Conceitos de personagem (na raiz, NÃO são prop)

**MOVIDOS 2026-07-23** pras pastas dos personagens (`sprites/<slug>/_concept_front.png`),
saíram da raiz de `images/`. São a referência de geração — ver
[`sprites-inventory.md`](sprites-inventory.md). Restou na raiz de `images/` só o
`vance_dragon_glyph.png` (game-loaded) e o `aleatorio.png` (órfão). Órfão: `aleatorio.png` (mercenário cyber-tático que NÃO bate com
o Sterling corporativo — aguarda atribuição de canon).

## Como eu deixei isso bagunçado (registro, pra não repetir)

Estes ~69 arquivos foram gerados por mim (PixelLab, sessões anteriores) e largados
soltos na raiz de `images/` sem espelhar a taxonomia que eu mesmo tinha em
`prompts_images/`. Reorganizado 2026-07-23 a pedido do líder. Regra que fica: **arte
gerada nasce já na subpasta certa** (categoria de prop ou pasta do personagem),
nunca num dump plano.
