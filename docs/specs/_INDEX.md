# GusWorld — Resources

**Visual vigente: 2D pixel-art via PixelLab** (pivô 3D→2D, ver CLAUDE.md). A descrição de padrão 3D abaixo é histórica.

Specs técnicas canônicas reaproveitáveis do projeto GusWorld (`Projects/gusworld/`). **Imutáveis sem aprovação do criador supremo.**

## Character Specs

**Padrão visual vigente:** 2D pixel-art estilizado via pipeline PixelLab. Cada spec mantém os TRAÇOS DE IDENTIDADE canônicos (nome, idade, cor-marca, acessório-assinatura, silhueta, role) — agnósticos de dimensão — e preserva a antiga spec de mesh/shader 3D num bloco de histórico interno. A spec 2D detalhada (resolução de sprite, nº de frames/direções, paleta indexada, prompt PixelLab por personagem) é **decisão pendente do criador supremo**, ainda não definida (ver `Projects/gusworld/docs/art/style-guide.md`, seção "Pendências consolidadas").

### Protagonista
- [`character-spec-gus.md`](character-spec-gus.md) — **Gus "Dragon" Vector Tavus Vance** (11 anos, ruivo asimétrico, óculos cyan, aparelho ortodôntico, sobretudo gótico, Tavus-Drive pulso esquerdo)

### Party (6 companions, idades 11-14)
- [`character-spec-caua-volt.md`](character-spec-caua-volt.md) — **Cauã "Volt" Berenger** (13, Striker EMP cinético, Dutos Infernais)
- [`character-spec-iara-lumen.md`](character-spec-iara-lumen.md) — **Iara "Lumen" Koslov** (12, Infiltradora ofuscamento, Setor Mirage)
- [`character-spec-bento-requiem.md`](character-spec-bento-requiem.md) — **Bento "Requiem" Chevalier** (14, Tanque gravitacional, Catedrais Neo-Sylvania; exceção Pillar 2 — magia analógica)
- [`character-spec-linda-siren.md`](character-spec-linda-siren.md) — **Linda "Siren" Neumann** (12, Crowd Control sônico, Zona do Silêncio)
- [`character-spec-dante-grid.md`](character-spec-dante-grid.md) — **Dante "Grid" Alencar** (13, **TRAIDOR canônico**, Suporte mecânico, Periferia Industrial)
- [`character-spec-jaci-proxy.md`](character-spec-jaci-proxy.md) — **Jaci "Proxy" Vanderbist** (11, Healer biológica, Selve Sombria)

### Antagonista
- [`character-spec-sterling-locke.md`](character-spec-sterling-locke.md) — **Sterling Locke** (adulto, antagonista corporativo monolítico, Cúpula Sterling; quebra deliberada do "SD cômico" via geometria angular ameaçadora)

## Convenções de spec

Toda spec segue 4 seções:
1. Parâmetros de Proporção e Geometria (Mesh)
2. Texturização, Paleta de Cores e Iluminação
3. Detalhamento de Hard-Surface e Acessórios
4. Prompt para Motores de Geração de Ativos

+ **Notas de aplicação** ao final (cross-ref com pillars, settings, mecânicas).

## Pipeline canônico de aprovação

> **Nota 2026-07.** O pipeline abaixo e a tabela Status são da era 3D, superada pelo pivot para C++23 com arte 2D pixel art (PixelLab). As SPECS continuam a fonte canônica de aparência (proporção, paleta, traços-assinatura) e alimentam os prompts de geração de sprite. Mantido como registro histórico.

1. Spec aprovada (este arquivo)
2. Blender blockout 1:1:1
3. Silhouette test 8 ângulos (turntable)
4. High-poly sculpt
5. Retopo low-poly (atinge budget de `Projects/gusworld/docs/art/style-guide.md`)
6. UV unwrap + atlas painting (cel-shading)
7. Rig (Rigify modificado pra SD)
8. Test render in-engine com câmera 3/4 rotacional
9. **Aprovação criador supremo antes de animar**

## Status

| Personagem | Spec | Sheet projeto | Blockout | Approved render |
|---|---|---|---|---|
| Gus | ✅ | ✅ `docs/art/characters/gus.md` | ⬜ | ⬜ |
| Cauã | ✅ | ⬜ pendente | ⬜ | ⬜ |
| Iara | ✅ | ⬜ pendente | ⬜ | ⬜ |
| Bento | ✅ | ⬜ pendente | ⬜ | ⬜ |
| Linda | ✅ | ⬜ pendente | ⬜ | ⬜ |
| Dante | ✅ | ⬜ pendente | ⬜ | ⬜ |
| Jaci | ✅ | ⬜ pendente | ⬜ | ⬜ |
| Sterling | ✅ | ⬜ pendente | ⬜ | ⬜ |
