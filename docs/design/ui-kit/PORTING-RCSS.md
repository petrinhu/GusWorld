# Porte do UI Kit para RCSS (glintfx / RmlUi 6.3)

Notas do dev do glintfx (revisão dos 6 componentes contra RmlUi 6.3, 2026-07-16). **Veredito: nada inviável.** card/slot/chip/botão/tokens quase diretos; a bancada é a mais trabalhosa mas 100% viável. Os mockups em `docs/design/ui-kit/*.html` são o ALVO 1:1; estas são as adaptações mecânicas pro RCSS real.

## Roda direto (RmlUi 6.3 suporta)
- `var(--...)` (custom properties, inclusive as de família `--fam-*`).
- Flexbox completo: `display:flex`, `flex-direction`, `align-items`, `justify-content`, `flex:1`, `flex-wrap`, `gap`. Cavalo de batalha de layout.
- `border`, `border-radius` (em px), `text-transform:uppercase`, `letter-spacing`, `opacity`, `position:absolute/relative`, `box-shadow`, `float`, `vh`/`vw`.
- Gradientes linear e radial (via `decorator:`, ver abaixo).
- A curva de mana (divs com altura + gradiente): zero problema, sem canvas/SVG.

## Precisa adaptar (de -> para, tudo mecânico)
1. **`display:grid` -> flexbox.** RmlUi não tem CSS Grid. `.grid` (2 colunas), `.cards` (grid 2col/auto-rows) e `place-items:center` viram flex (`flex-wrap` + larguras, `align/justify:center`). Maior adaptação, mas mecânica.
2. **`::before`/`::after` com `content` -> elemento real no RML.** RmlUi trata `::before` como pseudo-CLASSE, não gera pseudo-elemento. As barras de cor de família (`.card::before`, `.slot.filled::before`) e os rótulos gerados (`.nm::after` "[na mão]") viram `<div class="fambar">`/`<span>` no markup.
3. **`background:linear-gradient(...)` -> `decorator:linear-gradient(...)`.** No RCSS gradiente/imagem de fundo vai por `decorator:`, não `background:`. Cor sólida segue em `background-color`. Só sintaxe.
4. **`repeating-linear-gradient` (scanlines do body) -> textura tileada** (`decorator: image(scanline.png)` repetida) ou omitir. RmlUi só tem linear/radial, não repeating. Estético.
5. **`text-shadow` (glow do brand) -> `filter: drop-shadow(...)`.** Text-shadow puro não existe; drop-shadow dá o mesmo halo.
6. **`border-radius:50%` (círculo de mana) -> px** (ex.: `10px` num elemento 20x20). O parser de radius é length, não %.

## Fonte (conecta com FONT-EXTEND-GLITCH)
As setas `▲▼` (U+25B2/25BC) e chips geométricos **NÃO estão no PixelOperatorMono** -> caem na fonte estendida (item [[reference_terminal_glitch_fonte]] / FONT-EXTEND-GLITCH). Sem ela, viram tofu no jogo (nos mockmups em browser aparecem porque o browser tem a fonte). Ao portar: ou estender a fonte com esses glifos, ou usar um sprite/ícone pra seta.

## Padrão de porte
O grosso do trabalho = `grid->flex` + `::before->elemento`; o resto é troca de sintaxe. A estética "Tático" (mono/cyan/cor-por-família) é 100% expressável. **O dev do glintfx ofereceu revisar o RCSS real de um componente-PILOTO (ex.: o card) pra fechar o padrão** antes de portar o resto. Recomendação: começar o porte pelo `10-card` como piloto, validar o RCSS com o dev, e replicar o padrão pros demais.
