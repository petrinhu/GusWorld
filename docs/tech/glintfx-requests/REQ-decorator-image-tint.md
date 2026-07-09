# glintfx feature request: tint/recolor de imagem-decorator por cor (RCSS)

> **Pedido do consumidor (GusWorld) pro dev do glintfx.** GusWorld NUNCA mexe na lib glintfx; só consome e pede. Este doc registra a "dor" + a spec técnica pra o dev avaliar. Fallback documentado no fim (não bloqueia o GusWorld).

## A dor (use case real)

GusWorld usa glintfx (RmlUi 6.3 + backend GL3, embed mode) na UI. Estou compondo as cartas do "Codex de Conjuros" num modelo HÍBRIDO: uma **textura base de moldura** (pixel-art gerada no PixelLab: pedra gótica + **runas brancas neutras** + estrutura de ouro) entra como `decorator`/`background-image` de um elemento RML, e o glintfx sobrepõe **a cor do domínio + o glow**.

São **6 domínios** (ciano/violeta/verde/azul/carmesim/âmbar) + a carta lendária do Tusk. Quero, a partir de **UMA base neutra**, obter as 6 cores **trocando só uma cor no RCSS**, em runtime, data-driven e **animável** (pra casar com o glow pulsante). Hoje os decorators do glintfx (`image`, `tiled-*`, `gradient`, `drop-shadow`, `mask-image`, `polygon`) **não têm como modular/tingir a textura amostrada por uma cor** — então eu teria que shippar N texturas pré-coloridas e trocar o `decorator` por elemento (desperdício, não animável, não data-driven).

## O que peço

Um jeito, via RCSS, de **modular/tingir a imagem de um decorator por uma cor**. Modos desejados (do mais simples ao ideal):

1. **`multiply` / modulate** (essencial): `out.rgb = texel.rgb * tint.rgb`, `out.a = texel.a * tint.a`. Branco → cor; escuro fica escuro. Recolore as runas brancas mantendo a pedra escura.
2. **`luminance-key` (ideal)**: tingir SÓ os pixels claros/neutros (as runas), preservando os já coloridos (o **ouro** e a pedra escura ficam intactos). Via limiar de luminância + saturação, ou um canal-máscara. É o que deixa a base 100% reaproveitável (ouro imutável, runas coloridas por domínio).
3. **Dirigível por custom property / variável RCSS** (ex.: `--domain`), pra recolor por instância = setar 1 variável; e **interpolável** (transition/keyframes) pra animar junto do glow pulsante.

## API RCSS proposta (sugestão; o dev decide a forma)

Opção A — parâmetro no decorator:
```css
.card {
  decorator: image( frame_neutral.png ) tint( var(--domain), luminance-multiply );
}
```
Opção B — propriedades dedicadas:
```css
.card {
  decorator: image( frame_neutral.png );
  image-tint-color: var(--domain);          /* interpolável */
  image-tint-mode: luminance-multiply;        /* none | multiply | luminance-multiply | screen */
  image-tint-threshold: 0.55;                 /* opcional: corte de luminância p/ o keyed */
}
```
`--domain` setado por carta (ex.: `#22D3EE` eletromag). Default `none` = sem efeito (não quebra decorators existentes).

## Esboço de implementação (GL3 backend)

No fragment shader do decorator, após amostrar a textura:
```glsl
vec4 texel = texture(uTex, uv);
vec3 tinted = texel.rgb * uTint.rgb;                 // multiply
if (uMode == LUMINANCE_MULTIPLY) {
    float L = dot(texel.rgb, vec3(0.299, 0.587, 0.114));
    float sat = maxc(texel.rgb) - minc(texel.rgb);
    float w = smoothstep(uThreshold, 1.0, L) * (1.0 - sat);  // só claro+neutro
    tinted = mix(texel.rgb, texel.rgb * uTint.rgb, w);
}
frag = vec4((uMode==NONE ? texel.rgb : tinted), texel.a * uTint.a);
```
Uniforms novos: `uTint` (vec4), `uMode` (int), `uThreshold` (float). Plumbing: parse das propriedades RCSS → instância do decorator → uniforms. Poucas linhas por decorator.

## Requisitos / edge cases

- **Alpha correto** (premultiplied): não colorir a cor de pixels transparentes de forma errada.
- **Compõe com o `drop-shadow`** (glow) sobreposto: o glow é independente do tint.
- **Animável**: cor do tint como propriedade interpolável (transition/keyframes) — precisamos animar o glow pulsante do Tusk.
- **Correção sRGB/gamma**: tingir no mesmo espaço em que o backend renderiza.
- **No-op por padrão** (`mode: none`): decorators existentes intactos.

## Critério de aceite

Uma PNG de "runas brancas sobre pedra escura + ouro" como decorator + `--domain` em cada uma das 6 cores → renderiza 6 molduras recoloridas onde, no modo `luminance-multiply`, **o ouro e a pedra escura ficam visualmente inalterados** e só as runas mudam de cor; e animar `--domain` (ou a cor do tint) transiciona suave.

## Se já existir

Se o glintfx/RmlUi já tem `image-color` / color-modulate por decorator, me aponte que eu uso o que já tem — este pedido é só se não houver tint de textura por decorator hoje.

## Fallback (NÃO bloqueia o GusWorld)

Se a feat demorar, geramos **6 bases já coloridas** no PixelLab (uma por domínio, com o arco) e o glintfx só troca a imagem + aplica o glow/estado. Sem depender de nova feat. A feat é uma OTIMIZAÇÃO (1 textura, data-driven, animável), não um bloqueio.

Cross-ref: `docs/design/card-frame-spec.md` (trilha híbrida), `reference_glintfx_api`, `reference_pixellab_mcp`, ADR-010 (glintfx embed).
