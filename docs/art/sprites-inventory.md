# Inventário de sprites dos personagens

Mapa de onde vivem os sprites de cada personagem, o que cada pasta já tem, e como
gerar o que falta. **Fonte da verdade sobre a árvore `resources/sprites/`.**

> Atualizado 2026-07-23. Contagem via `identify` + `find` sobre o disco.

## Onde ficam

Todos em **`resources/sprites/<slug>/`**, um diretório por personagem. Os PNGs são
**gitignored** (`.gitignore`: `resources/sprites/*`), vivem só no disco + backup
IDrive — este doc, sim, é versionado, então o mapa sobrevive a um clone limpo
mesmo que os binários não.

## Convenção de arquivos por personagem

```
resources/sprites/<slug>/
├── south.png            # frente (vira o jogador). É a REFERÊNCIA canônica do char.
├── north.png            # costas
├── east.png             # direita
├── west.png             # esquerda   (sem flip de east — Pillar 3, ver locomotion)
└── walk/
    ├── south/{0,1,2,3,...}.png   # ciclo de caminhada por direção
    ├── north/...
    ├── east/...
    └── west/...
```

Retratos/bustos de combate ficam à parte em `resources/sprites/icons-m5/retratos/`.
O jogo é **4-direcional sem flip** por design (Pillar 3); ver a memória
`project_locomotion_animacao`.

## Estado atual (81 pastas de personagem)

### Party jogável — locomoção COMPLETA (4 dir + walk)

Estes 7 têm ciclo de caminhada pronto (o alvo de completude):

| slug | dim | walk frames | nota |
|---|---|---|---|
| `caua_volt` | 68×68 | 16 | versão antiga/pequena (pipeline test 2026-06-22) |
| `caua_volt_cyan_v2` | 180×180 | 24 | **versão boa do Cauã** (ciano canônico reforçado) |
| `iara_lumen` | 180×180 | 24 | Infiltradora |
| `bento_requiem` | 180×180 | 24 | Tanque |
| `linda_siren` | 180×180 | 24 | Crowd Control |
| `dante_grid` | 180×180 | 24 | TRAIDOR |
| `jaci_proxy` | 180×180 | 24 | Healer |

Falta o **Gus protagonista** com walk: hoje só existe `gustaf_i_tavus_vance/`
(4 dir estáticas, sem walk) — mas esse é o **ancestral Gustaf I**, não o
protagonista (ver `project_nome_gus_canon`). O sprite de locomoção do
protagonista Gus ainda não está nesta árvore como pasta própria.

### Personagens com 4 direções estáticas, SEM walk (70)

A maioria do elenco de mundo/NPCs: 180×180, `south/north/east/west.png`, zero
walk. São NPCs que hoje não andam (parados no mundo). Ex.: `seu_bertoldo_caim`
(o NPC do M7), a família Chevalier, os Ferraz, `patch_zero` (+4 extras),
`sterling`/antagonistas, etc. Lista completa: `ls -d resources/sprites/*/`.

### Pastas especiais (não são um personagem 1:1)

- `icons-m5/` — retratos de combate + ícones (54 PNGs, inclui `retratos/retrato_gus_*`).
- `personagens_inspirados/` — arte-conceito HD de referência (161 arquivos; ex.
  `gus/gus_conceito.png` 1844×2304 — HD, NÃO é sprite de jogo).
- `world/` — cenário dos Distritos Inferiores.
- `seu_bertoldo_caim/` — NPC do M7 (retrato usado no diálogo).

## Como gerar o que falta (PixelLab, pipeline canônica)

Detalhe completo em `reference_pixellab_mcp` (memória). Resumo:

1. **Referência → 8 direções:** `create_character` **mode="v3"** +
   `reference_image_base64` = o `south.png` aprovado (≤256px, ~6KB de base64, cabe
   na tool call). Rotaciona AQUELE sprite exato, travando identidade + estilo (o
   ciano do Cauã, o aparato do Gus). v3 sempre entrega 8 direções — as 4 diagonais
   extras servem o item de backlog `ARTE-DIAGONAL-8DIR`.
2. **Walk:** `animate_character` (template `walk`/`walking-*`, 1 ger/dir, ou v3
   custom) sobre o char criado.
3. **Baixar SEMPRE os PNGs** pro `resources/sprites/<slug>/` (regra
   `feedback_pixellab_sempre_baixar_pngs`); conferir no disco antes de dar "feito".
4. **Nunca sobrescrever** arte aprovada — gerar em pasta nova (`<slug>_v3/`) e
   comparar antes de adotar.

Saldo/limites: Tier 1, 2000 gerações/ciclo, máx 8 jobs concorrentes; `get_balance`
e `list_*` não gastam.
