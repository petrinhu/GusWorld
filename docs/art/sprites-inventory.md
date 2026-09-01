# Inventário de sprites dos personagens

Mapa de onde vivem os sprites de cada personagem, o que cada pasta já tem, e como
gerar o que falta. **Fonte da verdade sobre a árvore `resources/sprites/`.**

> Reformado 31/08/2026 (decisão do líder de 30/08/2026, por `AskUserQuestion`): o
> documento passa a separar PARTE MEDIDA de PARTE DE JULGAMENTO, e corrige oito
> divergências achadas entre o texto anterior e o disco.

## Como ler este documento

Duas partes, deliberadamente separadas:

- **PARTE MEDIDA** — fatos verificáveis no disco e no git, num instante datado. Cada
  afirmação vem acompanhada do comando que a produz, para que qualquer pessoa
  reconfira num comando só. Ela descreve o disco em **31/08/2026, 21:11
  (America/Recife)** — divergência entre esta parte e o disco é erro DESTE
  documento, nunca do disco. **Rode o comando; não cite o número daqui de cabeça**
  depois que o disco tiver mudado.
- **PARTE DE JULGAMENTO** — o que não se mede: qual pasta é a ativa e por quê, o que
  foi decidido pelo líder e quando, o que aguarda decisão, papel de cada
  personagem. Escrita à mão, como antes.

---

## PARTE MEDIDA (disco em 31/08/2026, 21:11 — America/Recife)

### Versionamento — o mecanismo real

Os PNGs de `resources/sprites/` **não são ignorados pelo git**. São rastreados de
forma **incondicional** via Git LFS, pela regra do `.gitattributes` (linhas 14-23):

```
resources/**/*.png filter=lfs diff=lfs merge=lfs -text
```

(mesma regra para `.jpg`, `.jpeg`, `.gif`, `.wav`, `.mp3`, `.ogg`, `.flac`, `.zip`.)

Verificação:

- `/usr/bin/grep -i sprite .gitignore` → **zero linhas**. Não existe regra
  `resources/sprites/*` no `.gitignore`.
- `find resources/sprites -type f | wc -l` → **898**.
- `git ls-files resources/sprites | wc -l` → **898**.
- `comm -3` entre as duas listas ordenadas → **0 linhas de diferença**: os 898
  arquivos do disco são exatamente os 898 rastreados, sem arquivo fora do git nem
  entrada fantasma no git.

Não existe hoje nenhuma "exceção seletiva por arquivo" (nem `vanda_do_cafe/south.png`
isolado, nem tratamento especial de `caua_volt/walk/`) — o mecanismo é LFS
incondicional sobre todo PNG de `resources/`, sem lista de exceções.

### Estrutura de topo

- `find resources/sprites -mindepth 1 -maxdepth 1 -type d | wc -l` → **87 pastas de
  nível 1**.
- Pastas que **não** são personagem 1:1 (5): `icons-m5`, `personagens_inspirados`,
  `world`, `models_frente`, `prospero_vance`.
- **Personagens = 87 − 5 = 82.**
- Varredura pasta a pasta dos 82 (`find "resources/sprites/<slug>" -maxdepth 1
  -type d -name walk`): **6 têm `walk/`** — `caua_volt`, `iara_lumen`,
  `bento_requiem`, `linda_siren`, `dante_grid`, `jaci_proxy`. **76 não têm.**
- **Estáticos sem walk = 82 − 6 = 76.**
- Dos 76, contagem item a item de `find "resources/sprites/<slug>" -maxdepth 1
  -iname '*.png' | wc -l` mostra que **67 têm exatamente 4 arquivos**
  (`south/north/east/west.png`) e **9 divergem**: 7 têm só `_concept_front.png` (1
  arquivo — aguardando geração, ver parte de julgamento), `otelo_pancha` tem 5
  (`_concept_front.png` + as 4 direções) e `patch_zero` tem 8 (4 direções cardeais +
  4 diagonais).

### `models_frente/`

`find resources/sprites/models_frente -type f` → **9 arquivos**, todos rastreados
(`git ls-files resources/sprites/models_frente | wc -l` também dá 9):
`androide_inimigo.png`, `bento.png`, `bertoldo_caim.png`, `caua.png`, `dante.png`,
`iara.png`, `jaci.png`, `linda.png`, `sterling.png`.

### `personagens_inspirados/`

`find resources/sprites/personagens_inspirados -type f | wc -l` → **183 arquivos**.
Por subpasta:

| subpasta | arquivos |
|---|---|
| `gus` | 173 |
| `pyotor_vance` | 4 |
| `yakov` | 3 |
| `brunus_vetorial` | 3 |

173 + 4 + 3 + 3 = **183**.

### Gus — `anims/breathing_idle/`

`find .../gus/anims/breathing_idle -type f` → **20 arquivos**, 5 em cada uma das 4
subpastas (`south`, `north`, `east`, `west`). `md5sum` dos 20 arquivos: **hashes
todos distintos**, nenhuma repetição. Datas (`find -printf '%T+'`): `south/` em
23/06/2026; `north/`, `east/` e `west/` em 23/07/2026 (lote posterior).

A convenção `_breathing_idle_dirs_STAGING_2026-07-23/` **não existe**:
`find resources -iname '*STAGING*'` (buscado no projeto inteiro) → **0
resultados**. As 3 direções extras já estão na pasta final
`breathing_idle/<dir>/`, não em nenhuma pasta de staging.

Arquivo solto fora do padrão de 4 subpastas:
`personagens_inspirados/gus/walk/_south_strip.png`.

### `bento_requiem/` — pasta mista de tamanho

- Direções (`south/north/east/west.png`, 4 arquivos no topo da pasta): `identify`
  → **256×256** cada.
- `anims/` (14 subpastas; `find resources/sprites/bento_requiem/anims -type f |
  wc -l`): **181 arquivos**, todos **256×256** (amostrado por `identify`).
- `walk/` (`find resources/sprites/bento_requiem/walk -type f | wc -l`): **24
  arquivos**, todos **180×180** (`walk_<dir>_<n>.png`, plano, sem subpasta por
  direção).

### Anims dos companheiros (party)

`find resources/sprites/<slug>/anims -type f | wc -l` para cada um dos 6 com
`walk/`:

| slug | arquivos em `anims/` |
|---|---|
| `caua_volt` | 0 |
| `iara_lumen` | 0 |
| `linda_siren` | 0 |
| `dante_grid` | 0 |
| `jaci_proxy` | 0 |
| `bento_requiem` | 181 |

Só `bento_requiem` tem `anims/` preenchido; nos outros 5 a pasta existe (`find`
confirma o diretório) e está vazia.

### `aleatorio.png`

Não existe. `find . -iname 'aleatorio.png'` (projeto inteiro) → **0 resultados** no
disco. `git log --all --diff-filter=A --name-only | grep -i aleatorio` só acha
`docs/design/mecanicas/encontros-aleatorios.md` — falso-positivo do substring
"aleatorio" num nome de arquivo de design, nunca uma imagem. `resources/images/`
(raiz) tem **1 arquivo direto** (`find resources/images -maxdepth 1 -type f`):
`vance_dragon_glyph.png` (244.826 bytes), mais 12 subpastas.

### `icons-m5/`

`find resources/sprites/icons-m5 -type f | wc -l` (recursivo, com subpastas
`app_icon/familias/intent/modificador/retratos/status/`) → **55 arquivos**: **54
PNGs** (`-iname '*.png'`) + **1 `REVISAO.html`** — não mencionado em versões
anteriores deste documento.

### `prospero_vance/`

`find resources/sprites/prospero_vance -type f | wc -l` → **0**. A pasta **existe e
está vazia** (`ls -la` mostra só `.` e `..`). O personagem é **canônico**:
`CHARS.md` linha 179 o registra como "Comerciante-Itinerante Próspero Vance"
("Próspero"), **ancestral direto da família Vance** (Belinor → ... → Pyotor →
Gargi → Gus), histórico da Era 1 (💀) — morto há muito tempo, não é NPC que anda
pelo mundo, o que por si só explica a ausência de sprite de caminhada. O nome
aparece em **14 arquivos** do corpus rastreado, com **43 menções** só em
`docs/narrative/deep/eras/era-1-pre-codigo.md` §7.7
(`git ls-files -z | xargs -0 /usr/bin/grep -c "Pr[óo]spero"`), e `CHARS.md` linha
226 descreve ainda a linhagem institucional inteira derivada dele (11 títulos da
Casa Comercial Vance, transmitidos ao longo de ~720 anos, cada portador com
numeral romano e cognome).

O prompt de imagem dele já existe
(`resources/prompts_images/feitos/PROSPERO_VANCE_IMAGEPROMPT.md`), e ele é um dos
**87 prompts de `resources/prompts_images/feitos/` (168 arquivos
`*IMAGEPROMPT.md` no total) sem sprite de personagem gerado** — não é caso
isolado. Medido agora, slug a slug, comparando cada prompt à árvore
`resources/sprites/`: 86 desses 87 não têm pasta nenhuma em `resources/sprites/`;
`prospero_vance` é o único com pasta já criada, e vazia. Essa é a peculiaridade
real, registrada aqui como fato — sem conclusão sobre a causa.

### `seu_bertoldo_caim/`

`find resources/sprites/seu_bertoldo_caim -type f` → 4 arquivos:
`east.png`, `north.png`, `south.png`, `west.png` (as 4 direções estáticas,
sem `walk/`).

---

## PARTE DE JULGAMENTO

### Tamanho canônico vs. tamanho no disco (decisão do líder, 30/08/2026)

**O tamanho CANÔNICO e lógico do sprite de personagem é 180×180 para TODO o elenco**,
decisão do líder por `AskUserQuestion` em 30/08/2026 (conteúdo completo em
`docs/art/style-guide.md` §8). Isto **não é** o mesmo que o tamanho do arquivo no
disco hoje: a maior parte do elenco já nasceu em 180×180, mas dois personagens
estão em **256×256** — o **Gus** (protagonista, ver bloco ⭐ abaixo) e o
**`bento_requiem`** (a parte medida acima confirma: direções + `anims/` em
256×256, só `walk/` já em 180×180). **Esses arquivos NÃO são regerados** para
bater com o canônico; o ajuste de 256 para 180 é feito **em tempo de execução,
pelo motor gráfico do GlintFx**, na camada de apresentação — que ainda não existe
(L-06, L-27). Isto abre uma exigência nova ao framework, registrada no `TODO.md`
como item `P4`; o pedido só vai ao bus quando `present/` esbarrar de fato na falta
(L-07).

**Ao ler a coluna `dim` da tabela abaixo:** ela descreve o **arquivo real no
disco**, não o canônico. Onde os dois coincidem (180×180), nenhuma nota é
necessária. Onde divergem (o `bento_requiem`, e só na parte 256×256 dele), a
tabela sinaliza.

### Onde ficam

Todos em **`resources/sprites/<slug>/`**, um diretório por personagem. Os PNGs são
rastreados incondicionalmente via Git LFS (ver PARTE MEDIDA acima) — vivem no
git **e** no disco local, não só no disco.

### Convenção de arquivos por personagem

```
resources/sprites/<slug>/
├── south.png            # frente (vira o jogador). É a REFERÊNCIA canônica do char.
├── north.png             # costas
├── east.png              # direita
├── west.png               # esquerda   (sem flip de east — Pillar 3, ver locomotion)
└── walk/
    ├── south/{0,1,2,3,...}.png   # ciclo de caminhada por direção
    ├── north/...
    ├── east/...
    └── west/...
```

Retratos/bustos de combate ficam à parte em `resources/sprites/icons-m5/retratos/`.
O jogo é **4-direcional sem flip** por design (Pillar 3); ver a memória
`project_locomotion_animacao`.

## Estado atual (82 pastas de personagem)

**`seu_bertoldo_caim` É PERSONAGEM** (decisão do líder, 30/08/2026): sai da lista de
pastas especiais e conta no total. A contagem de personagens, antes 81, passa a
**82** (ver aritmética completa na PARTE MEDIDA, seção "Estrutura de topo").

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
`sprites/personagens_inspirados/gus/`, apontada pela constante `kGusSpritesDir =
"sprites/personagens_inspirados/gus"` (header `asset_paths.hpp`). (O
`gustaf_i_tavus_vance/` é o **ancestral Gustaf I**, não o protagonista — não
confundir.) Tudo 256×256 **no arquivo em disco**. ⚠️ **O tamanho canônico e lógico
é 180×180** (decisão do líder, 30/08/2026, ver seção "Tamanho canônico vs. tamanho
no disco" acima) — este arquivo **não é regerado** para bater com ele; o ajuste é
feito em tempo de execução pelo motor gráfico do GlintFx.

```
personagens_inspirados/gus/
├── rotations/            # 8 direções estáticas: 0_south 1_south-west 2_west
│                         #   3_north-west 4_north 5_north-east 6_east 7_south-east
├── walk/{south,north,east,west}/   # 7 frames cada (locomoção 4-dir), mais
│                                    #   _south_strip.png solto no topo de walk/
├── anims/                # estados de combate + idle (frames por estado):
│   ├── breathing_idle/{south,north,east,west}/ (5 cada, 20 total)
│   │        ← a "RESPIRAÇÃO"/cansado. Sul gerado 23/06/2026; Norte/Leste/Oeste
│   │          gerados 23/07/2026 via API HTTP direta (/animate-with-text-v3,
│   │          first_frame = a rotação real de cada dir, animação
│   │          "winded/ofegante", antena preservada por construção). ⚠️ Já estão
│   │          na pasta FINAL (não há mais staging — ver parte medida), o que
│   │          pode significar que o item de backlog `ARTE-RESP-4DIR` (montar
│   │          breathing direcional + desligar idle_animated_only_one_facing)
│   │          já está fechado. **Isto é achado, não decisão** — precisa de
│   │          revisão do líder antes de fechar o item.
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

Estes 6 têm ciclo de caminhada pronto no top-level de `resources/sprites/`
(o Gus, à parte, também tem — ver bloco ⭐ acima):

| slug | dim (arquivo no disco) | walk frames | nota |
|---|---|---|---|
| `caua_volt` | 180×180 | 24 | **ATIVA — o Cauã do jogo**, desde a reversão do líder em 08/08/2026 — leia a nota abaixo |
| `iara_lumen` | 180×180 | 24 | Infiltradora |
| `bento_requiem` | **256×256 nas direções e em `anims/`; 180×180 em `walk/`** | 24 | Tanque. ⚠️ **Pasta mista** (ver PARTE MEDIDA) — o `walk/` já está no canônico, o resto não. Não regerar: o ajuste do resto é do motor gráfico do GlintFx em tempo de execução. |
| `linda_siren` | 180×180 | 24 | Crowd Control |
| `dante_grid` | 180×180 | 24 | TRAIDOR |
| `jaci_proxy` | 180×180 | 24 | Healer |

Nenhum dos 6 tem `anims/` preenchido hoje, exceto `bento_requiem` (181 arquivos —
ver PARTE MEDIDA, "Anims dos companheiros").

### Cauã Volt — reversão de 08/08/2026 (decisão do líder)

Em 2026-08-06, um agente moveu por engano arquivos de personagens **diferentes com o
MESMO nome** (`east.png`/`north.png`/`west.png`) para um destino plano, e a movida
sobrescreveu em silêncio os 3 arquivos de idle congelado (68×68) da pasta antiga
`caua_volt/`. Nunca estiveram no git (a política era "sprite = disco local", texto
já corrigido acima), então não havia como recuperá-los ali. Como contorno
temporário, o projeto anterior passou a apontar `kCauaSpritesDir` para uma pasta
nova, `caua_volt_cyan_v2/` (180×180).

**Em 2026-08-08 o líder recuperou os 3 arquivos perdidos e o `walk/` completo (24
quadros) de uma cópia própria, e decidiu reverter `kCauaSpritesDir` + `caua_layout()`
de volta para `caua_volt/`** (commit `840479e` do projeto anterior,
`gusworld_legacy`, verbatim: *"O Caua Volt volta a usar a arte recuperada em
caua_volt/ [...] Os 24 quadros de walk/ de caua_volt_cyan_v2/ saem do git por
decisao dele"*). A pasta `caua_volt_cyan_v2/` foi descartada por essa decisão e
**não existe neste projeto** — nunca esteve no disco nem no histórico deste
repositório.

- **`kCauaSpritesDir`** (`core/asset_paths.hpp`) aponta para `sprites/caua_volt` —
  **essa** é a pasta que o Cauã do jogo usa.
- **`caua_layout()`** (`player_sprites_loader.cpp`) tem o mesmo comportamento
  gracioso do Gus: sem `anims/breathing_idle/` no disco, o idle cai no **walk f0**
  congelado de cada direção. `walk/` de `caua_volt/` é **plano**
  (`walk_<dir>_<f>.png`, sem subpasta por direção — export direto do gerador),
  suportado pelo campo de dado `SpriteLayout::walk_dir_subfolder` (não um `if` por
  personagem — lei do átomo, ADR-020).
- **`caua_volt/` é a pasta ATIVA**: os 28 arquivos no disco (4 direções soltas + 24
  quadros de `walk/`) são os recuperados pelo líder em 08/08/2026, medidos por
  `identify` em 180×180, `PaletteAlpha`, 34 a 53 cores conforme o quadro — já
  satisfazem o tamanho canônico de 180×180 (decisão do líder, 30/08/2026) **sem
  nenhuma regeração**.

### Geração 2026-07-23 (party completa + Gus breathing)

Os 6 companheiros **deveriam** ganhar o conjunto de anims do Gus via API HTTP
direta (`animate-with-text-v3`, `no_background`, referência = o sprite direcional
real de cada um) — mas **a parte medida confirma que só o `bento_requiem` recebeu
o lote** (181 arquivos); `caua_volt`, `iara_lumen`, `linda_siren`, `dante_grid` e
`jaci_proxy` têm `anims/` vazio. **Não existe** a pasta de staging
`_<anim>_STAGING_2026-07-23/` citada em versões anteriores deste documento — nem
para o Bento, nem para nenhum outro. O Gus recebeu, no mesmo período, o
`breathing_idle` de Norte/Leste/Oeste (ver bloco ⭐ acima), já na pasta final.

O **Gus protagonista** tem a árvore mais completa de todas (ver bloco ⭐ acima:
8 rotações + walk 4-dir + 16 estados de anim, incluindo agora `breathing_idle` nas
4 direções). O que falta nele hoje é o resto do elenco: os 6 companheiros ainda não
receberam o pacote de `anims/` que essa geração pretendia entregar, exceto o
Bento.

### `models_frente/`

Existe com 9 arquivos rastreados (ver PARTE MEDIDA) e não estava documentada em
nenhuma versão anterior deste mapa. **O papel dela não está decidido aqui** — só
o conteúdo é descrito.

### Personagens com 4 direções estáticas, SEM walk (76)

A maioria do elenco de mundo/NPCs: 180×180, `south/north/east/west.png`, zero
walk. 67 deles têm exatamente esses 4 arquivos. Os outros 9 divergem (ver PARTE
MEDIDA, "Estrutura de topo"): 7 aguardando geração (só `_concept_front.png`),
`otelo_pancha` (5 arquivos) e `patch_zero` (8 arquivos, com diagonais). Ex.:
`seu_bertoldo_caim` (o NPC do M7), a família Chevalier, os Ferraz, `patch_zero`,
`sterling`/antagonistas, etc. Lista completa: `ls -d resources/sprites/*/`.

### Pastas especiais (não são um personagem 1:1)

- `icons-m5/` — retratos de combate + ícones (55 arquivos: 54 PNGs, inclui
  `retratos/retrato_gus_*`, mais `REVISAO.html`).
- `personagens_inspirados/` — arte-conceito HD de referência (183 arquivos; ex.
  `gus/gus_conceito.png` 1844×2304 — HD, NÃO é sprite de jogo).
- `world/` — cenário dos Distritos Inferiores.
- `models_frente/` — 9 arquivos, papel ainda não documentado (ver acima).
- `prospero_vance/` — pasta vazia. Personagem canônico (`CHARS.md` linha 179,
  ancestral direto da família Vance, histórico da Era 1 — ver `### prospero_vance/`
  acima). Único, entre os 87 prompts de `resources/prompts_images/feitos/` ainda
  sem sprite gerado, com pasta já criada.

## Como gerar o que falta (PixelLab, pipeline canônica)

**RITUAL OBRIGATÓRIO de 5 passos (líder 2026-07-23, memória
`feedback_ritual_geracao_sprite`):**
1. **Buscar NESTE mapa** (nunca sair procurando no disco). 2. **Buscar referência**
aprovada do personagem (é o insumo; trava identidade+antena, ZERO flip).
3. **Gerar** via PixelLab, pasta nova para variante/regen. 4. **Baixar** os PNGs
pra `resources/sprites/<slug>/` (sem isso ficam invisíveis pro jogo).
5. **Atualizar ESTE mapa** com o que foi gerado. O mapa alimenta a geração
(passo 1) e a geração alimenta o mapa (passo 5) — nunca envelhece.

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

## Conceitos de personagem aguardando geração

**7 personagens** com pasta criada e só o conceito HD dentro, como
`sprites/<slug>/_concept_front.png` (a referência de geração), confirmado pela
varredura da PARTE MEDIDA (os 7 folders com exatamente 1 arquivo, esse arquivo).
Falta GERAR os sprites de jogo a partir dele (`generate-8-rotations-v3` +
`no_background` → 4 direções; anims depois conforme o papel). Fila atrás da
geração da party (limite de taxa):

`anaximandro_vyrcatrix`, `anhuera_vanderbist`, `cassiano_vorto`, `yara_ducourt`,
`anselmo_boroshova_vance`, `mariana_vanderbist`, **`heliaco_vyr`** — cada um com
`_concept_front.png`, zero sprite ainda. ⚠️ **`heliaco_vyr` é achado desta
reforma**: casa exatamente no mesmo padrão dos outros 6 (só `_concept_front.png`
no disco) mas não estava listado nesta seção em versões anteriores do documento.
