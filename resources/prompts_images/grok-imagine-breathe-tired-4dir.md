# Grok Imagine — respiração cansada no lugar (N/S/E/W)

Use a **mesma âncora sul** (ou o frame idle sul aprovado) como referência em todas as gerações.

São **4 strips 1×8**, um por cardinal. Sem diagonais. Sem deslocamento dos pés. Sem walk.

Fluxo: S → E → N → W. Um prompt por geração.

---

## IDENTIDADE

Substitua só a linha `Character lock` nos 4 prompts.

```text
Use the attached south-facing anchor as the only identity source.
Preserve exact face, haircut, hair length, hair color, outfit, colors, outline weight, pixel size, silhouette width, weapon/props, and boot shape.
Do not restyle. Do not add detail. Do not change proportions. Do not crop. Do not zoom.
Same character scale in every cell. Head-to-toe visible. Feet planted on one shared baseline.
Solid flat magenta background #FF00FF, no gradient, no ground plane art, no drop shadow, no text, no labels, no frames, no gutters.
Pixel art game sprite, crisp pixels, limited palette matching the anchor, thick clean outlines, no anti-alias, no painterly shading.
```

---

## CICLO (já embutido)

8 frames, no lugar, respiração **cansada / ofegante leve**:

1. end of exhale — chest lowest, shoulders dropped, head a hair down, mouth slightly open
2. start inhale — ribs begin to lift, shoulders start up
3. mid inhale — chest expanding, shoulders rising, head lifting toward neutral
4. peak inhale — chest highest, shoulders highest of the cycle, torso tallest
5. start exhale — chest starts falling, air leaving, head begins to ease down
6. mid exhale — shoulders dropping, hair settles with a 1-frame lag
7. late exhale — chest almost empty, slight slump
8. recover toward next inhale — tiny lift so the loop closes clean

Pés **não saem do chão**. Sem passada. Sem sliding. Bob vertical só do tórax/ombros/cabeça, **menor que o walk** (1 px no sprite final, no máximo 2 no peito).

Braços pendurados, acompanham o torso alguns pixels; não balançam como walk. Cabelo sobe atrasado na inspiração e cai atrasado na expiração.

---

## 1 — SUL (S)

```text
Pixel art tired-breathing idle sprite strip, exact 1x8 grid, eight equal cells, landscape sheet.
Direction: south, facing the camera, standing in place. No locomotion. Feet planted.
Keep the same south face and body construction as the attached anchor.

Use the attached south-facing anchor as the only identity source.
Preserve exact face, haircut, hair length, hair color, outfit, colors, outline weight, pixel size, silhouette width, weapon/props, and boot shape.
Do not restyle. Do not add detail. Do not change proportions. Do not crop. Do not zoom.
Same character scale in every cell. Head-to-toe visible. Feet planted on one shared baseline.
Solid flat magenta background #FF00FF, no gradient, no ground plane art, no drop shadow, no text, no labels, no frames, no gutters.
Pixel art game sprite, crisp pixels, limited palette matching the anchor, thick clean outlines, no anti-alias, no painterly shading.
Character lock: [DESCREVA AQUI: cabelo, roupa, capa, arma, botas, paleta].

8-frame tired breathing cycle, even timing, in-place, seamless loop, no camera move.
The character is winded: mouth slightly open, heavier chest and shoulder motion than a calm idle, small head dip on exhale. Not collapsing. Not sitting. Not talking.
Frame order left to right:
1 end of exhale, chest lowest, shoulders dropped, head a hair down, mouth slightly open
2 start inhale, ribs begin to lift, shoulders start up
3 mid inhale, chest expanding, shoulders rising, head lifting toward neutral
4 peak inhale, chest highest, shoulders highest, torso tallest of the cycle
5 start exhale, chest starts falling, head eases down
6 mid exhale, shoulders dropping, hair settles with 1-frame lag
7 late exhale, chest almost empty, slight slump
8 recover toward next inhale, tiny lift so the loop closes
Arms hang at the sides and ride with the torso only a few pixels. No walk pendulum. Hands stay near the hips.
Hair secondary motion only: lag on inhale up, lag on exhale down. Same haircut as the anchor.
Vertical change is chest/shoulder/head only. Feet and boots do not leave the baseline. Hips almost locked. No bounce, no walk, no weight-shift step.
Same bounding box in every cell. Character centered. No cell overflow.
```

---

## 2 — LESTE (E)

```text
Pixel art tired-breathing idle sprite strip, exact 1x8 grid, eight equal cells, landscape sheet.
Direction: east, true side view, facing right. Profile obtained by yawing the south head 90 degrees. Do not add bulk. Do not invent a new costume silhouette.
Standing in place. No locomotion. Feet planted.

Use the attached south-facing anchor as the only identity source.
Preserve exact face, haircut, hair length, hair color, outfit, colors, outline weight, pixel size, silhouette width, weapon/props, and boot shape.
Do not restyle. Do not add detail. Do not change proportions. Do not crop. Do not zoom.
Same character scale in every cell. Head-to-toe visible. Feet planted on one shared baseline.
Solid flat magenta background #FF00FF, no gradient, no ground plane art, no drop shadow, no text, no labels, no frames, no gutters.
Pixel art game sprite, crisp pixels, limited palette matching the anchor, thick clean outlines, no anti-alias, no painterly shading.
Character lock: [DESCREVA AQUI: cabelo, roupa, capa, arma, botas, paleta].

8-frame tired breathing cycle, even timing, in-place, seamless loop, no camera move.
The character is winded: mouth slightly open, heavier chest and shoulder motion than a calm idle, small head dip on exhale. Not collapsing. Not sitting. Not talking.
In side view the chest expansion must read as ribcage volume moving forward/up on inhale and back/down on exhale. Shoulders rise and fall. The near arm hangs and rides the torso.
Frame order left to right:
1 end of exhale, chest lowest, shoulders dropped, head a hair down, mouth slightly open
2 start inhale, ribs begin to lift, shoulders start up
3 mid inhale, chest expanding, shoulders rising, head lifting toward neutral
4 peak inhale, chest highest, shoulders highest, torso tallest of the cycle
5 start exhale, chest starts falling, head eases down
6 mid exhale, shoulders dropping, hair settles with 1-frame lag
7 late exhale, chest almost empty, slight slump
8 recover toward next inhale, tiny lift so the loop closes
Arms hang. No walk pendulum. Hands stay near the hips.
Hair secondary motion only. Side view shows hair mass behind the head with lag. Same cut as the anchor.
Vertical change is chest/shoulder/head only. Feet and boots do not leave the baseline. Hips almost locked. No bounce, no walk, no weight-shift step.
Same bounding box in every cell. Character centered. No cell overflow.
```

---

## 3 — NORTE (N)

```text
Pixel art tired-breathing idle sprite strip, exact 1x8 grid, eight equal cells, landscape sheet.
Direction: north, back view, standing in place facing away from the camera. No face. Back of the haircut and back of the outfit/cloak must match the south anchor. Do not invent a different haircut on the back of the head.
No locomotion. Feet planted.

Use the attached south-facing anchor as the only identity source.
Preserve exact face, haircut, hair length, hair color, outfit, colors, outline weight, pixel size, silhouette width, weapon/props, and boot shape.
Do not restyle. Do not add detail. Do not change proportions. Do not crop. Do not zoom.
Same character scale in every cell. Head-to-toe visible. Feet planted on one shared baseline.
Solid flat magenta background #FF00FF, no gradient, no ground plane art, no drop shadow, no text, no labels, no frames, no gutters.
Pixel art game sprite, crisp pixels, limited palette matching the anchor, thick clean outlines, no anti-alias, no painterly shading.
Character lock: [DESCREVA AQUI: cabelo, roupa, capa, arma, botas, paleta].

8-frame tired breathing cycle, even timing, in-place, seamless loop, no camera move.
The character is winded: heavier shoulder-blade and upper-back rise on inhale, drop on exhale, small head dip on exhale. Not collapsing. Not sitting.
From the back, breathing reads as shoulders spreading/lifting and the cloak/torso widening a pixel on inhale, then settling.
Frame order left to right:
1 end of exhale, chest/back lowest, shoulders dropped, head a hair down
2 start inhale, shoulders start up and slightly out
3 mid inhale, upper back expanding, shoulders rising, head lifting toward neutral
4 peak inhale, shoulders highest, torso tallest of the cycle
5 start exhale, shoulders start falling, head eases down
6 mid exhale, shoulders dropping, hair settles with 1-frame lag
7 late exhale, almost empty, slight slump
8 recover toward next inhale, tiny lift so the loop closes
Arms hang at the sides and ride with the torso only a few pixels. No walk pendulum.
Hair secondary motion only. Back view keeps the same hair length and cut as the anchor.
Vertical change is chest/shoulder/head only. Feet and boots do not leave the baseline. Hips almost locked. No bounce, no walk, no weight-shift step.
Same bounding box in every cell. Character centered. No cell overflow.
```

---

## 4 — OESTE (W)

```text
Pixel art tired-breathing idle sprite strip, exact 1x8 grid, eight equal cells, landscape sheet.
Direction: west, true side view, facing left. Exact profile counterpart of the east strip. Do not redesign. Clothing and weapon stay on the same anatomical side as the south anchor (do not flip a hip item onto the wrong hip).
Standing in place. No locomotion. Feet planted.

Use the attached south-facing anchor as the only identity source.
Preserve exact face, haircut, hair length, hair color, outfit, colors, outline weight, pixel size, silhouette width, weapon/props, and boot shape.
Do not restyle. Do not add detail. Do not change proportions. Do not crop. Do not zoom.
Same character scale in every cell. Head-to-toe visible. Feet planted on one shared baseline.
Solid flat magenta background #FF00FF, no gradient, no ground plane art, no drop shadow, no text, no labels, no frames, no gutters.
Pixel art game sprite, crisp pixels, limited palette matching the anchor, thick clean outlines, no anti-alias, no painterly shading.
Character lock: [DESCREVA AQUI: cabelo, roupa, capa, arma, botas, paleta].

8-frame tired breathing cycle, even timing, in-place, seamless loop, no camera move.
The character is winded: mouth slightly open, heavier chest and shoulder motion than a calm idle, small head dip on exhale. Not collapsing. Not sitting. Not talking.
In side view the chest expansion must read as ribcage volume moving forward/up on inhale and back/down on exhale. Shoulders rise and fall. The near arm hangs and rides the torso.
Frame order left to right:
1 end of exhale, chest lowest, shoulders dropped, head a hair down, mouth slightly open
2 start inhale, ribs begin to lift, shoulders start up
3 mid inhale, chest expanding, shoulders rising, head lifting toward neutral
4 peak inhale, chest highest, shoulders highest, torso tallest of the cycle
5 start exhale, chest starts falling, head eases down
6 mid exhale, shoulders dropping, hair settles with 1-frame lag
7 late exhale, chest almost empty, slight slump
8 recover toward next inhale, tiny lift so the loop closes
Arms hang. No walk pendulum. Hands stay near the hips.
Hair secondary motion only. Side view shows hair mass behind the head with lag. Same cut as the anchor.
Vertical change is chest/shoulder/head only. Feet and boots do not leave the baseline. Hips almost locked. No bounce, no walk, no weight-shift step.
Same bounding box in every cell. Character centered. No cell overflow.
```

---

## Correção

Anexe a âncora sul + o strip errado.

```text
Edit this 1x8 tired-breathing strip to match the attached south-facing anchor identity.
Keep the existing breathing poses, frame order, facing direction and magenta #FF00FF background.
Feet stay planted. No walk. No step.
Fix only: [rosto / cabelo / escala / peito / ombros / direção / overflow].
Do not restyle. Do not change the grid. Do not add text.
Same bounding box and foot baseline in every cell.
Keep mouth slightly open, chest/shoulder rise on inhale, 1-frame hair lag, minimal bob.
```

---

## Vídeo curto (opcional)

Anexe a âncora daquela direção.

```text
Animate this exact pixel-art character breathing tired in place, about 1 second, seamless loop.
Camera locked. No zoom. No pan.
Keep flat magenta #FF00FF background.
Direction: [south / east / north / west].
Feet planted. No walk. No step.
Winded idle: mouth slightly open, chest and shoulders rise on inhale and fall on exhale, small head dip on exhale.
Arms hang and ride the torso only. Hair lags one frame.
Do not restyle. Keep identity, outfit, outline and pixel style identical to the source.
```

---

## Atlas sugerido

| Linha | Direção |
|------|---------|
| 0 | S |
| 1 | E |
| 2 | N |
| 3 | W |

Colunas 0–7 = frames 1–8 do ciclo de respiração.

Monte o atlas depois do corte e do chroma key. Não peça 4×8 no Imagine.
