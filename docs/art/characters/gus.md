# Character Sheet — Gus Vector Tavus Vance

> **Spec mestre canônica:** `Resources/gusworld/character-spec-gus.md`
> Este arquivo é o **shortcut de produção** para o time. Toda alteração visual de Gus exige aprovação do user (criador supremo).

## TL;DR de produção

| Campo | Valor |
|---|---|
| **Proporção** | Super Deformed (SD) **1:1:1** — cabeça 1/3 + torso/pelve 1/3 + pernas 1/3 |
| **Morfologia** | SD comprimido + extremidades ectomorfas delgadas (NÃO chibi inflado) |
| **Render style** | Cel-shaded 3D (toon, bandas duras) |
| **Cabelo** | Laranja-neon `#FF6B1A`, assimétrico, mechas angulares, **quadrante superior direito** dominante, alta densidade de polígonos para sombreamento |
| **Pele** | Alva, fosca, saturação mínima, sombras com subtons frios levemente arroxeados |
| **Olhos / Óculos** | Lentes cyan emissivas `#22D3EE`, linhas de varredura animadas |
| **Aparelho ortodôntico** | Microcubos metálicos especulares + filamento prata-escuro (grafeno/tântalo). Visível em fala/sorriso |
| **Indumentária** | Sobretudo gótico cinza-escuro carbonizado, fibra sintética. Normal map em jaqueta + botas (Kevlar/couro sintético fosco) |
| **Tavus-Drive** | Pulso ESQUERDO. Módulo saliente. Glow violeta `#A78BFA` |
| **Cor única no jogo** | Laranja `#FF6B1A` do cabelo — nenhum outro asset usa |

## Poly budget (revisado para SD)

| Componente | Tris | Notas |
|---|---|---|
| Corpo SD inteiro | 2.000–3.000 | Reduzido (SD = menos detalhe de extremidade) |
| Cabelo | 1.500–2.500 | Hi-poly relativa pra suporte de cel-shading + silhueta caótica |
| Indumentária (sobretudo + botas) | 1.000–1.500 | Permitir normal map |
| Acessórios (óculos + aparelho + Tavus-Drive) | 500–800 | Hard-surface, geometria limpa |
| **Total Gus** | **~5.000–7.000 tris** | Excede budget original (3-4.5k) — aprovado por ser único hero |

Texture maps:
- Atlas principal 1024² (rosto + pele + roupa + acessórios). Pode subir de 512² original devido a cel-shading detail.
- Normal map 512² SÓ pra jaqueta + botas.
- Emission map separado 256² pra lentes dos óculos + Tavus-Drive glow.
- Specular highlight via material slot dedicado para braquetes (metallic = 1.0, roughness baixa).

## Shape language

- **Triângulo invertido** (cabelo + ombros do sobretudo) + **base estreita** (pernas finas).
- Silhueta SD 1:1:1 deve ser identificável em 8 ângulos (turntable test obrigatório).
- Cabeça asimétrica: lado direito DOMINA (mechas caem maioria à direita do rosto).
- Aparelho ortodôntico visível em silhueta lateral (mandíbula com linha extra).

## VFX associadas

- **Modo scan** (óculos ativos): scanline cyan emergindo das lentes, partículas hexágono.
- **Tavus-Drive ativando carta**: glow violeta no pulso esquerdo → carta holográfica projetada acima da mão.
- **Sintonização Ortodôntica**: ondas concêntricas cyan/violeta saindo da mandíbula durante uso (foreshadowing visual sutil).

## Don'ts (revisão)

- Proporção realista (cabeça 1/6 ou 1/7) → **REJEITADO**. SD 1:1:1 é canônico.
- Chibi inflado tradicional (bracinhos curtos gordinhos) → **REJEITADO**. Extremidades ectomorfas.
- Cabelo simétrico → **REJEITADO**. Sempre asimétrico, quadrante superior direito.
- Cabelo em hex ≠ `#FF6B1A` → **REJEITADO**.
- Aparelho ortodôntico ausente/coberto → **REJEITADO**. Visível em fala/sorriso.
- Tavus-Drive no pulso direito → **REJEITADO**. Pulso ESQUERDO é canônico.
- PBR metallic em qualquer coisa fora dos braquetes → **REJEITADO**.
- Normal map fora de jaqueta/botas → **REJEITADO**.
- Cute/fofo/kawaii expressão default → **REJEITADO**. Expressão analítica/sério/sorriso irônico ocasional.

## Pipeline de aprovação

1. Blender blockout (proporção 1:1:1 validada com guides)
2. Silhouette test 8 ângulos → screenshot apresentado ao user
3. High-poly sculpt (cabelo + indumentária)
4. Retopo low-poly (atinge budget acima)
5. UV unwrap + atlas painting (Krita ou Substance Painter pra cel-shading)
6. Rig (Rigify modificado pra SD)
7. Test render in-Godot com câmera 3/4 rotacional → screenshot apresentado ao user
8. Aprovação final do user antes de animar
