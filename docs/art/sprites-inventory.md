# Inventário de sprites dos personagens

Mapa de onde vivem os sprites de cada personagem, o que cada pasta já tem, e como
gerar o que falta. **Fonte da verdade sobre a árvore `resources/sprites/`.**

> Atualizado 2026-08-06 (ASSETS-VERSIONAR-SPRITES). Contagem via `identify` + `find`
> sobre o disco.

## Onde ficam

Todos em **`resources/sprites/<slug>/`**, um diretório por personagem. Os PNGs são
**gitignored por padrão** (`.gitignore`: `resources/sprites/*`), vivem só no disco +
backup IDrive — este doc, sim, é versionado, então o mapa sobrevive a um clone limpo
mesmo que a maior parte dos binários não.

**Exceção (ASSETS-VERSIONAR-SPRITES, 2026-08-06):** um punhado de arquivos ESPECÍFICOS
(não a pasta inteira) é versionado no git normal porque **código de produção e testes
automatizados exigem que existam num clone limpo** — sem eles a suíte fica vermelha,
já que o CI não tem o disco local do líder. Padrão do `.gitignore`: un-ignore o
diretório → re-ignore o conteúdo → un-ignore só o(s) arquivo(s)/subpasta(s) citados por
código/teste (por diretório, nunca por nome exato solto — nome exato deixa de casar em
silêncio se o arquivo for renomeado). Hoje são 3 exceções:
- `resources/sprites/vanda_do_cafe/south.png` — NPC fixo do overworld (`city_actors.hpp`).
- `resources/sprites/caua_volt_cyan_v2/walk/` (24 arquivos) — o walk do Cauã ativo.
- `resources/sprites/caua_volt/south.png` — fixture da família GENÉRICA da cascata de
  assets (`platform/tests/asset_source_test.cpp`); ver seção do Cauã abaixo.

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

### `personagens_inspirados/` = os HOMENAGEADOS (inspirados em pessoas reais)

Pasta canônica dos personagens **inspirados em pessoas reais** (homenagens):
- `gus` — o protagonista (o filho, "Gus Dragon")
- `pyotor_vance` — o pai · `yakov` — o tio
- `brunus_vetorial` — ← amigo real Bruno Vettore (só estes 4 são pessoas reais)

NÃO confundir com os NPCs comuns em `sprites/<slug>/`. A doc do tributo dos
homenageados-mestres (Faraday, Turing, Gödel, von Neumann...) vive em
`docs/design/roster-analogos/` (21 figuras).

### Gus PROTAGONISTA — `resources/sprites/personagens_inspirados/gus/` ⭐

**ATENÇÃO: o Gus NÃO fica em `sprites/gus/`.** A pasta canônica é
`sprites/personagens_inspirados/gus/`, apontada pelo header
`GusEngine/core/include/gus/core/asset_paths.hpp` → `kGusSpritesDir =
"sprites/personagens_inspirados/gus"`. (O `gustaf_i_tavus_vance/` é o **ancestral
Gustaf I**, não o protagonista — não confundir.) Tudo 256×256.

```
personagens_inspirados/gus/
├── rotations/            # 8 direções estáticas: 0_south 1_south-west 2_west
│                         #   3_north-west 4_north 5_north-east 6_east 7_south-east
├── walk/{south,north,east,west}/   # 7 frames cada (locomoção 4-dir)
├── anims/                # estados de combate + idle (frames por estado):
│   ├── breathing_idle/ (5)   ← a "RESPIRAÇÃO"/cansado, virada pro Sul.
│   ├── _breathing_idle_dirs_STAGING_2026-07-23/{north,east,west}/ (5 cada)
│   │        ← GERADO 2026-07-23 via API HTTP direta (/animate-with-text-v3, first_frame
│   │          = a rotação real de cada dir, animação "winded/ofegante", antena preservada
│   │          por construção). STAGING: aguarda aval do líder + wiring do loader
│   │          (ARTE-RESP-4DIR: montar breathing direcional + desligar
│   │          idle_animated_only_one_facing). NÃO sobrescreveu o Sul nem o código.
│   ├── battle_idle/ (7)  cast/ (7)  attack_melee/ (7)  attack_melee_east/ (9)
│   ├── defend/ (5)  hurt_magic/ (5)  hurt_physical/ (5)  ko/ (7)  revive/ (7)
│   ├── run/ (7)  run_east/ (9)  run_west/ (9)  victory/ (7)  dragon_victory/ (9)
│   └── attack_melee_east_OLD_leftfacing/ (9)  ← LEGADO, não usar
└── (raiz) gus_conceito.png (HD 1844×2304, NÃO-sprite) · gus_front_pixel.png ·
          gus_front_pixel_quant.png · retrato_gus_3q.png
```

**ANTENA DE UM LADO SÓ = ZERO FLIP (Pillar 3).** O aparato do Gus é assimétrico
(antena de um lado). NUNCA espelhar east↔west nem derivar uma direção por flip —
cada direção tem arte própria com a antena no lado certo. Vale pra todo o elenco,
mas é crítico no Gus.

### Party jogável — locomoção COMPLETA (4 dir + walk)

Estes 7 têm ciclo de caminhada pronto (o alvo de completude):

| slug | dim | walk frames | nota |
|---|---|---|---|
| `caua_volt` | 68×68 | 16 | **APOSENTADA** — leia a nota abaixo antes de mexer |
| `caua_volt_cyan_v2` | 180×180 | 24 | **ATIVA — o Cauã do jogo** (ciano canônico reforçado) |
| `iara_lumen` | 180×180 | 24 | Infiltradora |
| `bento_requiem` | 180×180 | 24 | Tanque |
| `linda_siren` | 180×180 | 24 | Crowd Control |
| `dante_grid` | 180×180 | 24 | TRAIDOR |
| `jaci_proxy` | 180×180 | 24 | Healer |

### Cauã Volt — migração 2026-08-06 (ASSETS-VERSIONAR-SPRITES)

Um agente moveu por engano arquivos de personagens **diferentes com o MESMO nome**
(`east.png`/`north.png`/`west.png`) para um destino plano, e a movida sobrescreveu em
silêncio os 3 arquivos de idle congelado (68×68) da pasta antiga `caua_volt/`. Nunca
estiveram no git (a política era "sprite = disco local", ver seção "Onde ficam" acima),
então não havia como recuperá-los — decisão do líder: **não tentar reconstruir; apontar
o Cauã para a arte boa que já existia**, em vez de regenerar a antiga.

- **`kCauaSpritesDir`** (`core/asset_paths.hpp`) passou de `sprites/caua_volt` pra
  `sprites/caua_volt_cyan_v2` — o Cauã do jogo usa **essa** pasta agora.
- **`caua_layout()`** (`player_sprites_loader.cpp`) ganhou o mesmo comportamento
  gracioso do Gus: sem `anims/breathing_idle/` no disco, o idle cai no **walk f0**
  congelado de cada direção (em vez de exigir `south.png`/`north.png`/`east.png`/
  `west.png` soltos na raiz — o ramo que causou a perda). Isso tornou os 3 arquivos
  perdidos **desnecessários**: o personagem não depende mais deles pra existir.
  `walk/` do `caua_volt_cyan_v2/` é **plano** (`walk_<dir>_<f>.png`, sem subpasta por
  direção — export direto do gerador), suportado pelo campo de dado novo
  `SpriteLayout::walk_dir_subfolder` (não um `if` por personagem — lei do átomo,
  ADR-020).
- **`caua_volt/`** (a pasta antiga) fica **APOSENTADA**: nenhum código de produção a lê
  mais. Só `south.png` (a única sobrevivente) segue versionada, como fixture de um
  teste de infraestrutura genérica (família GENÉRICA da cascata de assets,
  `platform/tests/asset_source_test.cpp`) que só precisa de ALGUM arquivo real — não é
  mais "a arte do Cauã". Não editar nem regenerar essa pasta esperando que afete o jogo.

**GERAÇÃO 2026-07-23 (party completa + Gus breathing):** os 6 companions ganharam o
conjunto de anims do Gus via API HTTP direta (`animate-with-text-v3`, `no_background`,
referência = o sprite direcional real de cada um). **14 tipos de anim × 6, 1086 frames,
0 opaco** (conferido). Em STAGING (`anims/_<anim>_STAGING_2026-07-23/`), aguardando
wiring do loader: breathing_idle/walk/run/caindo/caido_desacordado (direcionais 4-dir) +
battle_idle/cast/attack_melee/defend/hurt_magic/hurt_physical/ko/revive/victory
(front). `dragon_victory` NÃO (lore só do Gus). Gus breathing_idle N/L/O regenerado
transparente (o 1º lote saiu opaco, bug de `no_background` esquecido).

O **Gus protagonista** tem a árvore mais completa de todas (ver bloco ⭐ acima:
8 rotações + walk 4-dir + 16 estados de anim). O que FALTA nele é pontual: a
**respiração/cansado (`breathing_idle`) para Norte, Leste e Oeste** — hoje só o
Sul tem, e os outros lados caem no walk-f0 congelado (item `ARTE-RESP-4DIR`).

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

**RITUAL OBRIGATÓRIO de 5 passos (líder 2026-07-23, memória `feedback_ritual_geracao_sprite`):**
1. **Buscar NESTE mapa** (nunca sair procurando no disco). 2. **Buscar referência** aprovada do personagem (é o insumo; trava identidade+antena, ZERO flip). 3. **Gerar** via PixelLab, pasta nova para variante/regen. 4. **Baixar** os PNGs pra `resources/sprites/<slug>/` (sem isso ficam invisíveis pro jogo). 5. **Atualizar ESTE mapa** com o que foi gerado. O mapa alimenta a geração (passo 1) e a geração alimenta o mapa (passo 5) — nunca envelhece.

Detalhe da ferramenta em `reference_pixellab_mcp` (memória). Resumo:

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

## Props e lore → outro mapa

Props, cartas, símbolos e arte de lore vivem em `resources/images/` e têm mapa
próprio: [`props-inventory.md`](props-inventory.md). Aqui é só personagem-sprite.

## Conceitos de personagem aguardando geração (2026-07-23)

6 personagens com **pasta criada 2026-07-23** e o conceito HD dentro como
`sprites/<slug>/_concept_front.png` (a referência de geração). Falta GERAR os
sprites de jogo a partir dele (`generate-8-rotations-v3` + `no_background` → 4
direções; anims depois conforme o papel). Fila atrás da geração da party (limite de
taxa):

`anaximandro_vyrcatrix`, `anhuera_vanderbist`, `cassiano_vorto`, `yara_ducourt`,
`anselmo_boroshova_vance`, `mariana_vanderbist` — cada um com `_concept_front.png`, zero
sprite ainda.

Órfão sem canon: `aleatorio.png` (fica em `images/` raiz; não é o Sterling; aguarda
atribuição antes de virar pasta).
