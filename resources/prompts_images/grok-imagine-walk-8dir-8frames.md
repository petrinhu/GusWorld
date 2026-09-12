# Grok Imagine — walk 8 direções × 8 frames

Use a **imagem âncora olhando para o sul** como referência em **todas** as gerações.

Fluxo:

1. Preencha o bloco `IDENTIDADE` uma vez.
2. Gere na ordem: S → E → N → W → SE → NE → SW → NW.
3. Depois de aprovar um strip, pode anexá-lo junto com a âncora sul na próxima direção vizinha.
4. Um prompt por geração. Não junte as 8 direções na mesma imagem.

Formato de cada sheet: **1×8 horizontal**, fundo magenta `#FF00FF`.

---

## IDENTIDADE

Copie este bloco para dentro de cada prompt, já preenchido.

```text
Use the attached south-facing anchor as the only identity source.
Preserve exact face, haircut, hair length, hair color, outfit, colors, outline weight, pixel size, silhouette width, weapon/props, and boot shape.
Do not restyle. Do not add detail. Do not change proportions. Do not crop. Do not zoom.
Same character scale in every cell. Head-to-toe visible. Feet on one shared baseline.
Solid flat magenta background #FF00FF, no gradient, no ground plane art, no drop shadow, no text, no labels, no frames, no gutters.
Pixel art game sprite, crisp pixels, limited palette matching the anchor, thick clean outlines, no anti-alias, no painterly shading.
Character lock: [DESCREVA AQUI: cabelo, roupa, capa, arma, botas, paleta].
```

Substitua só a linha `Character lock`. O resto permanece igual nos 8 prompts abaixo — já está embutido com um placeholder curto para você trocar.

---

## WALK (já embutido em cada prompt)

Ordem dos 8 frames, esquerda → direita:

1. contact, pé direito à frente, braço esquerdo à frente, braço direito atrás
2. down / recoil, joelhos mais flexionados, cabelo atrasa para baixo
3. passing, pernas mais próximas, torso no ponto mais alto do ciclo
4. up / reach, pé esquerdo estendendo, braço direito começando a ir à frente
5. contact, pé esquerdo à frente, braço direito à frente, braço esquerdo atrás
6. down / recoil oposto, cabelo atrasa para baixo
7. passing oposto
8. up / reach oposto

Braços em pêndulo completo (não tic). Cabelo com delay de 1 frame. Bob vertical mínimo (1–2 px do sprite final). Sem bounce.

---

## 1 — SUL (S) — mesma vista da âncora

```text
Pixel art walk cycle sprite strip, exact 1x8 grid, eight equal cells, landscape sheet.
Direction: south, facing the camera, walking in place toward the viewer.
Keep the same south face and body construction as the attached anchor.

Use the attached south-facing anchor as the only identity source.
Preserve exact face, haircut, hair length, hair color, outfit, colors, outline weight, pixel size, silhouette width, weapon/props, and boot shape.
Do not restyle. Do not add detail. Do not change proportions. Do not crop. Do not zoom.
Same character scale in every cell. Head-to-toe visible. Feet on one shared baseline.
Solid flat magenta background #FF00FF, no gradient, no ground plane art, no drop shadow, no text, no labels, no frames, no gutters.
Pixel art game sprite, crisp pixels, limited palette matching the anchor, thick clean outlines, no anti-alias, no painterly shading.
Character lock: [DESCREVA AQUI: cabelo, roupa, capa, arma, botas, paleta].

8-frame walk cycle, even timing, in-place locomotion, no camera move, no sliding.
Frame order left to right:
1 contact right foot forward, left arm forward, right arm back
2 down / recoil, knees more bent, hair lags downward
3 passing, legs closest, torso highest of the cycle
4 up / reach, left foot extending, right arm starting forward
5 contact left foot forward, right arm forward, left arm back
6 down / recoil opposite, hair lags downward
7 passing opposite
8 up / reach opposite
Arm swing is a full opposing pendulum, not a tiny twitch. Hands travel from behind the hip to in front of the chest line.
Hair has secondary motion: 1-frame delay vs the head, drag on down frames, slight follow-through on up frames. Strands keep the same cut as the anchor.
Vertical body bob is minimal: at most 1-2 pixels of the final sprite height between lowest (frames 2 and 6) and highest (frames 3 and 7). Do not bounce. Hips stay almost level. Head does not pump.
Same bounding box in every cell. Character centered in each cell. No cell overflow.
```

---

## 2 — LESTE (E)

```text
Pixel art walk cycle sprite strip, exact 1x8 grid, eight equal cells, landscape sheet.
Direction: east, true side view, facing right. One eye, ear and nose in profile obtained by yawing the south head 90 degrees. Do not add bulk. Do not invent a new costume silhouette.
Walking in place to the right.

Use the attached south-facing anchor as the only identity source.
Preserve exact face, haircut, hair length, hair color, outfit, colors, outline weight, pixel size, silhouette width, weapon/props, and boot shape.
Do not restyle. Do not add detail. Do not change proportions. Do not crop. Do not zoom.
Same character scale in every cell. Head-to-toe visible. Feet on one shared baseline.
Solid flat magenta background #FF00FF, no gradient, no ground plane art, no drop shadow, no text, no labels, no frames, no gutters.
Pixel art game sprite, crisp pixels, limited palette matching the anchor, thick clean outlines, no anti-alias, no painterly shading.
Character lock: [DESCREVA AQUI: cabelo, roupa, capa, arma, botas, paleta].

8-frame walk cycle, even timing, in-place locomotion, no camera move, no sliding.
Frame order left to right:
1 contact right foot forward, left arm forward, right arm back
2 down / recoil, knees more bent, hair lags downward
3 passing, legs closest, torso highest of the cycle
4 up / reach, left foot extending, right arm starting forward
5 contact left foot forward, right arm forward, left arm back
6 down / recoil opposite, hair lags downward
7 passing opposite
8 up / reach opposite
Arm swing is a full opposing pendulum, not a tiny twitch. Hands travel from behind the hip to in front of the chest line. In side view the near arm reads clearly in front of the torso on forward swing and behind the torso on back swing.
Hair has secondary motion: 1-frame delay vs the head, drag on down frames, slight follow-through on up frames. Strands keep the same cut as the anchor. Side view shows hair mass behind the head with lag.
Vertical body bob is minimal: at most 1-2 pixels of the final sprite height between lowest (frames 2 and 6) and highest (frames 3 and 7). Do not bounce. Hips stay almost level. Head does not pump.
Same bounding box in every cell. Character centered in each cell. No cell overflow.
```

---

## 3 — NORTE (N) — de costas

```text
Pixel art walk cycle sprite strip, exact 1x8 grid, eight equal cells, landscape sheet.
Direction: north, back view, walking away from the camera. No face. Show the back of the haircut, back of the outfit/cloak and rear silhouette that belong to the south anchor. Do not invent a different haircut on the back of the head.
Walking in place away from the viewer.

Use the attached south-facing anchor as the only identity source.
Preserve exact face, haircut, hair length, hair color, outfit, colors, outline weight, pixel size, silhouette width, weapon/props, and boot shape.
Do not restyle. Do not add detail. Do not change proportions. Do not crop. Do not zoom.
Same character scale in every cell. Head-to-toe visible. Feet on one shared baseline.
Solid flat magenta background #FF00FF, no gradient, no ground plane art, no drop shadow, no text, no labels, no frames, no gutters.
Pixel art game sprite, crisp pixels, limited palette matching the anchor, thick clean outlines, no anti-alias, no painterly shading.
Character lock: [DESCREVA AQUI: cabelo, roupa, capa, arma, botas, paleta].

8-frame walk cycle, even timing, in-place locomotion, no camera move, no sliding.
Frame order left to right:
1 contact right foot forward, left arm forward, right arm back
2 down / recoil, knees more bent, hair lags downward
3 passing, legs closest, torso highest of the cycle
4 up / reach, left foot extending, right arm starting forward
5 contact left foot forward, right arm forward, left arm back
6 down / recoil opposite, hair lags downward
7 passing opposite
8 up / reach opposite
Arm swing is a full opposing pendulum, not a tiny twitch. From the back, both arms remain readable against the torso and cloak.
Hair has secondary motion: 1-frame delay vs the head, drag on down frames, slight follow-through on up frames. Back view must keep the same hair length and cut as the anchor.
Vertical body bob is minimal: at most 1-2 pixels of the final sprite height between lowest (frames 2 and 6) and highest (frames 3 and 7). Do not bounce. Hips stay almost level. Head does not pump.
Same bounding box in every cell. Character centered in each cell. No cell overflow.
```

---

## 4 — OESTE (W)

```text
Pixel art walk cycle sprite strip, exact 1x8 grid, eight equal cells, landscape sheet.
Direction: west, true side view, facing left. Exact profile counterpart of an east-facing version of the south anchor. Do not redesign. Do not mirror clothing details that should stay on the same body side (sword/hip item stays on the same anatomical side).
Walking in place to the left.

Use the attached south-facing anchor as the only identity source.
Preserve exact face, haircut, hair length, hair color, outfit, colors, outline weight, pixel size, silhouette width, weapon/props, and boot shape.
Do not restyle. Do not add detail. Do not change proportions. Do not crop. Do not zoom.
Same character scale in every cell. Head-to-toe visible. Feet on one shared baseline.
Solid flat magenta background #FF00FF, no gradient, no ground plane art, no drop shadow, no text, no labels, no frames, no gutters.
Pixel art game sprite, crisp pixels, limited palette matching the anchor, thick clean outlines, no anti-alias, no painterly shading.
Character lock: [DESCREVA AQUI: cabelo, roupa, capa, arma, botas, paleta].

8-frame walk cycle, even timing, in-place locomotion, no camera move, no sliding.
Frame order left to right:
1 contact right foot forward, left arm forward, right arm back
2 down / recoil, knees more bent, hair lags downward
3 passing, legs closest, torso highest of the cycle
4 up / reach, left foot extending, right arm starting forward
5 contact left foot forward, right arm forward, left arm back
6 down / recoil opposite, hair lags downward
7 passing opposite
8 up / reach opposite
Arm swing is a full opposing pendulum, not a tiny twitch. Hands travel from behind the hip to in front of the chest line. In side view the near arm reads clearly in front of the torso on forward swing and behind the torso on back swing.
Hair has secondary motion: 1-frame delay vs the head, drag on down frames, slight follow-through on up frames. Strands keep the same cut as the anchor. Side view shows hair mass behind the head with lag.
Vertical body bob is minimal: at most 1-2 pixels of the final sprite height between lowest (frames 2 and 6) and highest (frames 3 and 7). Do not bounce. Hips stay almost level. Head does not pump.
Same bounding box in every cell. Character centered in each cell. No cell overflow.
```

---

## 5 — SUDESTE (SE)

```text
Pixel art walk cycle sprite strip, exact 1x8 grid, eight equal cells, landscape sheet.
Direction: southeast, three-quarter view facing down-right. Both eyes readable. Right side of the face slightly dominant. Volume of head and shoulders matches the south anchor; only yaw changes. Do not flatten into a side view. Do not stay fully front-facing.
Walking in place along the southeast axis.

Use the attached south-facing anchor as the only identity source.
Preserve exact face, haircut, hair length, hair color, outfit, colors, outline weight, pixel size, silhouette width, weapon/props, and boot shape.
Do not restyle. Do not add detail. Do not change proportions. Do not crop. Do not zoom.
Same character scale in every cell. Head-to-toe visible. Feet on one shared baseline.
Solid flat magenta background #FF00FF, no gradient, no ground plane art, no drop shadow, no text, no labels, no frames, no gutters.
Pixel art game sprite, crisp pixels, limited palette matching the anchor, thick clean outlines, no anti-alias, no painterly shading.
Character lock: [DESCREVA AQUI: cabelo, roupa, capa, arma, botas, paleta].

8-frame walk cycle, even timing, in-place locomotion, no camera move, no sliding.
Frame order left to right:
1 contact right foot forward, left arm forward, right arm back
2 down / recoil, knees more bent, hair lags downward
3 passing, legs closest, torso highest of the cycle
4 up / reach, left foot extending, right arm starting forward
5 contact left foot forward, right arm forward, left arm back
6 down / recoil opposite, hair lags downward
7 passing opposite
8 up / reach opposite
Arm swing is a full opposing pendulum, not a tiny twitch. Hands travel from behind the hip to in front of the chest line. In three-quarter view both arms stay visible, with the far arm slightly smaller but still swinging.
Hair has secondary motion: 1-frame delay vs the head, drag on down frames, slight follow-through on up frames. Strands keep the same cut as the anchor.
Vertical body bob is minimal: at most 1-2 pixels of the final sprite height between lowest (frames 2 and 6) and highest (frames 3 and 7). Do not bounce. Hips stay almost level. Head does not pump.
Same bounding box in every cell. Character centered in each cell. No cell overflow.
```

---

## 6 — NORDESTE (NE)

```text
Pixel art walk cycle sprite strip, exact 1x8 grid, eight equal cells, landscape sheet.
Direction: northeast, three-quarter rear-right. Back of the head and right shoulder dominant. A sliver of the face may remain visible. This is not a full back view and not a side view.
Walking in place along the northeast axis.

Use the attached south-facing anchor as the only identity source.
Preserve exact face, haircut, hair length, hair color, outfit, colors, outline weight, pixel size, silhouette width, weapon/props, and boot shape.
Do not restyle. Do not add detail. Do not change proportions. Do not crop. Do not zoom.
Same character scale in every cell. Head-to-toe visible. Feet on one shared baseline.
Solid flat magenta background #FF00FF, no gradient, no ground plane art, no drop shadow, no text, no labels, no frames, no gutters.
Pixel art game sprite, crisp pixels, limited palette matching the anchor, thick clean outlines, no anti-alias, no painterly shading.
Character lock: [DESCREVA AQUI: cabelo, roupa, capa, arma, botas, paleta].

8-frame walk cycle, even timing, in-place locomotion, no camera move, no sliding.
Frame order left to right:
1 contact right foot forward, left arm forward, right arm back
2 down / recoil, knees more bent, hair lags downward
3 passing, legs closest, torso highest of the cycle
4 up / reach, left foot extending, right arm starting forward
5 contact left foot forward, right arm forward, left arm back
6 down / recoil opposite, hair lags downward
7 passing opposite
8 up / reach opposite
Arm swing is a full opposing pendulum, not a tiny twitch. Hands travel from behind the hip to in front of the chest line. Rear three-quarter keeps both arms readable against cloak/torso.
Hair has secondary motion: 1-frame delay vs the head, drag on down frames, slight follow-through on up frames. Back-right hair mass matches the south haircut.
Vertical body bob is minimal: at most 1-2 pixels of the final sprite height between lowest (frames 2 and 6) and highest (frames 3 and 7). Do not bounce. Hips stay almost level. Head does not pump.
Same bounding box in every cell. Character centered in each cell. No cell overflow.
```

---

## 7 — SUDOESTE (SW)

```text
Pixel art walk cycle sprite strip, exact 1x8 grid, eight equal cells, landscape sheet.
Direction: southwest, three-quarter view facing down-left. Both eyes readable. Left side of the face slightly dominant. Construction is the counterpart of southeast, not a new design.
Walking in place along the southwest axis.

Use the attached south-facing anchor as the only identity source.
Preserve exact face, haircut, hair length, hair color, outfit, colors, outline weight, pixel size, silhouette width, weapon/props, and boot shape.
Do not restyle. Do not add detail. Do not change proportions. Do not crop. Do not zoom.
Same character scale in every cell. Head-to-toe visible. Feet on one shared baseline.
Solid flat magenta background #FF00FF, no gradient, no ground plane art, no drop shadow, no text, no labels, no frames, no gutters.
Pixel art game sprite, crisp pixels, limited palette matching the anchor, thick clean outlines, no anti-alias, no painterly shading.
Character lock: [DESCREVA AQUI: cabelo, roupa, capa, arma, botas, paleta].

8-frame walk cycle, even timing, in-place locomotion, no camera move, no sliding.
Frame order left to right:
1 contact right foot forward, left arm forward, right arm back
2 down / recoil, knees more bent, hair lags downward
3 passing, legs closest, torso highest of the cycle
4 up / reach, left foot extending, right arm starting forward
5 contact left foot forward, right arm forward, left arm back
6 down / recoil opposite, hair lags downward
7 passing opposite
8 up / reach opposite
Arm swing is a full opposing pendulum, not a tiny twitch. Hands travel from behind the hip to in front of the chest line. In three-quarter view both arms stay visible, with the far arm slightly smaller but still swinging.
Hair has secondary motion: 1-frame delay vs the head, drag on down frames, slight follow-through on up frames. Strands keep the same cut as the anchor.
Vertical body bob is minimal: at most 1-2 pixels of the final sprite height between lowest (frames 2 and 6) and highest (frames 3 and 7). Do not bounce. Hips stay almost level. Head does not pump.
Same bounding box in every cell. Character centered in each cell. No cell overflow.
```

---

## 8 — NOROESTE (NW)

```text
Pixel art walk cycle sprite strip, exact 1x8 grid, eight equal cells, landscape sheet.
Direction: northwest, three-quarter rear-left. Back of the head and left shoulder dominant. Construction is the counterpart of northeast, not a new design. This is not a full back view and not a side view.
Walking in place along the northwest axis.

Use the attached south-facing anchor as the only identity source.
Preserve exact face, haircut, hair length, hair color, outfit, colors, outline weight, pixel size, silhouette width, weapon/props, and boot shape.
Do not restyle. Do not add detail. Do not change proportions. Do not crop. Do not zoom.
Same character scale in every cell. Head-to-toe visible. Feet on one shared baseline.
Solid flat magenta background #FF00FF, no gradient, no ground plane art, no drop shadow, no text, no labels, no frames, no gutters.
Pixel art game sprite, crisp pixels, limited palette matching the anchor, thick clean outlines, no anti-alias, no painterly shading.
Character lock: [DESCREVA AQUI: cabelo, roupa, capa, arma, botas, paleta].

8-frame walk cycle, even timing, in-place locomotion, no camera move, no sliding.
Frame order left to right:
1 contact right foot forward, left arm forward, right arm back
2 down / recoil, knees more bent, hair lags downward
3 passing, legs closest, torso highest of the cycle
4 up / reach, left foot extending, right arm starting forward
5 contact left foot forward, right arm forward, left arm back
6 down / recoil opposite, hair lags downward
7 passing opposite
8 up / reach opposite
Arm swing is a full opposing pendulum, not a tiny twitch. Hands travel from behind the hip to in front of the chest line. Rear three-quarter keeps both arms readable against cloak/torso.
Hair has secondary motion: 1-frame delay vs the head, drag on down frames, slight follow-through on up frames. Back-left hair mass matches the south haircut.
Vertical body bob is minimal: at most 1-2 pixels of the final sprite height between lowest (frames 2 and 6) and highest (frames 3 and 7). Do not bounce. Hips stay almost level. Head does not pump.
Same bounding box in every cell. Character centered in each cell. No cell overflow.
```

---

## Correção (se um strip desviar da âncora)

Anexe a âncora sul + o strip errado.

```text
Edit this 1x8 walk strip to match the attached south-facing anchor identity.
Keep the existing walk poses, frame order, direction and magenta #FF00FF background.
Fix only: [rosto / cabelo / escala / braços / direção / overflow].
Do not restyle. Do not change the grid. Do not add text.
Same bounding box and foot baseline in every cell.
Full opposing arm swing and 1-frame hair lag must remain.
Minimal vertical bob only.
```

---

## Vídeo curto (opcional, uma direção por vez)

Anexe a âncora (ou o melhor still daquela direção).

```text
Animate this exact pixel-art character as an 8-frame walk in place, about 1 second, seamless loop.
Camera locked. No zoom. No pan. No camera orbit.
Keep flat magenta #FF00FF background.
Direction: [south / east / north / west / southeast / northeast / southwest / northwest].
Full opposing arm swing. Hair drag one frame behind the head.
Vertical bob tiny only. Do not bounce.
Do not restyle. Keep identity, outfit, outline and pixel style identical to the source.
```

---

## Ordem de atlas no engine (quando for montar)

Linhas sugeridas, cima → baixo:

| Linha | Direção |
|------|---------|
| 0 | S |
| 1 | SE |
| 2 | E |
| 3 | NE |
| 4 | N |
| 5 | NW |
| 6 | W |
| 7 | SW |

Colunas 0–7 = frames 1–8 do walk.

Não monte o atlas 8×8 no Imagine. Monte depois do corte e do chroma key.
