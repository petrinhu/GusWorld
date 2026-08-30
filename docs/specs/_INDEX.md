# GusWorld — Resources

**Visual vigente: 2D pixel-art via PixelLab** (pivô 3D→2D, ver CLAUDE.md).

Specs técnicas canônicas reaproveitáveis do projeto GusWorld (`Projects/gusworld/`). **Imutáveis sem aprovação do criador supremo.**

## Character Specs

**Padrão visual vigente:** 2D pixel-art estilizado via pipeline PixelLab. Cada spec mantém os TRAÇOS DE IDENTIDADE canônicos (nome, idade, cor-marca, acessório-assinatura, silhueta, role) — agnósticos de dimensão. As specs continuam a fonte canônica de aparência (proporção, paleta, traços-assinatura) e alimentam os prompts de geração de sprite. **Duas peças da spec 2D detalhada foram decididas pelo líder em 30/08/2026:** resolução de sprite (180×180 canônico para todo o elenco, sem regerar o que já está em 256×256 — ajuste em tempo de execução pelo GlintFx) e profundidade de paleta por personagem (Gus em paleta rica, resto do elenco em paleta enxuta). O restante da spec 2D (nº de frames/direções, prompt PixelLab por personagem) é **decisão pendente do criador supremo**, ainda não definida (ver `Projects/gusworld/docs/art/style-guide.md`, §8, §9 e seção "Pendências consolidadas").

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

## Status

| Personagem | Spec | Sheet projeto |
|---|---|---|
| Gus | ✅ | ✅ `docs/art/characters/gus.md` |
| Cauã | ✅ | ⬜ pendente |
| Iara | ✅ | ⬜ pendente |
| Bento | ✅ | ⬜ pendente |
| Linda | ✅ | ⬜ pendente |
| Dante | ✅ | ⬜ pendente |
| Jaci | ✅ | ⬜ pendente |
| Sterling | ✅ | ⬜ pendente |
