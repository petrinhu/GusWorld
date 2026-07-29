# Dossiê: `draw_text` do Draw2D vs. o texto atual do GusWorld — decisão do líder

**Autor:** qa-engineer (dossiê de evidência, sem poder de decisão).
**Item:** `GLINTFX-FONTFLIP-VALIDATE` (TODO.md).
**Escopo tocado:** só `docs/`. Nenhum arquivo de `platform/`, `CMakeLists.txt` ou `TODO.md` foi
alterado — o teste visual foi feito por um probe standalone fora da árvore do jogo (ver seção 2).

---

## 0. Resumo executivo (leia isto primeiro)

1. **O gate está exatamente como o briefing descreveu** — `GLINTFX_OWN_FONT_ENGINE OR
   GLINTFX_MODULE_DRAW2D` — mas **hoje, no GusWorld, as duas flags estão OFF**
   (`CMakeLists.txt:200` e `CMakeLists.txt:372`), então o núcleo de fonte do glintfx (`glx_sfnt`/
   `glx_raster`/`glx_hint`) **não compila no binário atual**. Confirmei por `nm` no binário real:
   zero símbolos `glx_sfnt_*`/`glx_raster_*`/`glx_hint_*`. (Há 346 símbolos `glx_gl*` — isso é o
   **carregador de funções GL** do glintfx, coincidência de prefixo, nada a ver com fonte.)
2. **Correção ao briefing:** "hoje o nosso texto é FreeType" é verdade só para as **5 telas de
   menu/diálogo via RmlUi** (title/system/save-load/difficulty/npc-dialogue, `.rml`+FreeType). O
   texto do **HUD/mundo de jogo** — os 19 sítios de `draw_text` em `battle_scene_render.cpp` +
   `sdl_window.cpp`, que são exatamente o alvo da migração Draw2D — **já usa um TERCEIRO
   rasterizador hoje: `stb_truetype`** (`platform/src/render2d/font_atlas.cpp`), não FreeType.
   Adotar `Draw2d::draw_text` troca **stb_truetype → núcleo próprio do glintfx**, não
   **FreeType → núcleo próprio**.
3. **Achado mais importante (comparação visual, seção 3):** no tamanho mais miúdo que o jogo
   realmente usa hoje (`kCockpitTextPx = 8px`, o número de HP/max sobre a barra), o núcleo
   próprio do glintfx renderiza os acentos pt-br (Á É Í Ó Ú Ç) **colapsados numa faixa
   ilegível de tiques** — um defeito real, não artefato do meu harness (testei com margem extra
   pra descartar clipping). Só que **o pipeline REALMENTE em produção hoje é PIOR AINDA**: o
   `font_atlas.cpp` baka UMA VEZ em 16px fixo e a GPU faz *downscale* NEAREST pra 8px no
   `draw_quad` — reproduzi esse downscale e o resultado é **quase ilegível**, pior que os dois
   bakes diretos (stb OU glx) no mesmo 8px. Ou seja: **adotar o Draw2D provavelmente MELHORA** a
   qualidade visual do HUD miúdo em relação ao que já está em produção, apesar do próprio defeito
   do núcleo glx nos acentos.
4. **A metodologia "3 DPIs" do item original não se aplica LITERALMENTE ao Draw2D** — ela foi
   desenhada pro `dp_ratio` da ui (RmlUi). O Draw2D escala texto por PROJEÇÃO de câmera (mundo
   960x540 fixo → pixel real), o mesmo esquema de todo sprite: o glifo é bakeado UMA vez no
   `size` lógico pedido e a bitmap resultante é esticada pela câmera — a resolução física do
   monitor **não rebakea nada, só estica o mesmo bitmap**. Traduzi "3 DPIs" pro conceito real do
   Draw2D (ver seção 3.2) e o resultado confirma: a diferença de qualidade que existe em 8px
   lógico **aparece igual em 1080p, Steam Deck ou 4K** — a resolução física não "salva" nem
   "piora" nada, porque não há re-bake por resolução.
5. **Nenhuma das limitações declaradas do núcleo (sem GSUB/GPOS, sem BiDi, sem multi-face, sem
   COLR emoji) nos afeta** — somos pt-br + en-intl, Latin script, uma fonte só
   (PixelOperatorMono), sem emoji. Isso não é um risco pro GusWorld.
6. **Achado adicional, fora do que foi pedido, mas relevante pro custo:** adotar `draw_text` do
   Draw2D também troca o **MODELO DE LAYOUT** — hoje é grade monospace fixa (cada glifo ocupa
   exatamente `px_size × px_size`, via `kMonoAdvanceRatio`); o Draw2D usa avanço PROPORCIONAL +
   kerning da tabela `kern` da fonte. Mesma fonte, resultado de posicionamento não
   necessariamente pixel-idêntico.

**Minha recomendação técnica (seção 5): não é um "não" — é "sim, MAS com o vsnap/hinting do
menor tamanho investigado antes de trocar, e sabendo que o ganho maior está nos tamanhos ≥9px
onde os dois motores já empatam".** A decisão final é do líder.

---

## 1. O fato, provado por arquivo:linha

### 1.1 O gate

`GusEngine/build/linux-release/_deps/glintfx-src/glintfx/CMakeLists.txt:688`:

```cmake
if(GLINTFX_OWN_FONT_ENGINE OR GLINTFX_MODULE_DRAW2D)
  set(_glx_core_src_dir ${CMAKE_CURRENT_SOURCE_DIR}/vendor/core/src)
  set(_glx_core_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/vendor/core/include)
endif()
```

O mesmo `if` se repete em `CMakeLists.txt:1077` (variáveis de path pro consumidor) e
`CMakeLists.txt:1382`. O consumidor real dessas variáveis é a biblioteca-objeto dedicada
`glintfx_fontcore` (`CMakeLists.txt:1077-1088`), que compila `vendor/core/src/{sfnt,raster,hint}.c`
**uma única vez** e é linkada no agregado `glintfx` STATIC — resolvendo o "multiple definition"
que existiria se `glintfx_text` (motor da ui) e `glintfx_draw2d` (texto do Draw2D) compilassem
cada um sua própria cópia dos mesmos símbolos `glx_sfnt_*`. Isso está **implementado e provado**
(o comentário do CMake cita "provado: as duas flags ON agora compilam e linkam sem símbolo
duplicado") — o ADR-0018 (e) já foi executado, não é mais um item em aberto.

### 1.2 O que `draw_text`/`load_font`/`measure_text` usam como rasterizador

`glintfx/include/glintfx/draw2d.hpp:931-1030` (assinaturas públicas) +
`glintfx/src/text_raster.hpp` (a costura interna, TX2 (a)):

- `Draw2d::load_font(path)` → `TextFace::open()` → `glx_sfnt_open()` (parser SFNT do núcleo C).
- `Draw2d::draw_text(...)` → decodifica UTF-8 (`decode_utf8_codepoint`, política U+FFFD) →
  `TextFace::glyph_id()` (`glx_sfnt_glyph_id`) → `plan_glyph_bake`/`bake_glyph_into`, que chamam
  `glx_hint_outline` (Y-snap opcional) e `glx_rasterize_outline` (rasterizador analítico AA) →
  glifo vira textura num atlas próprio do Draw2D (`text_layout.hpp`/`draw2d.cpp`, T1b/T2).
- **Zero FreeType, zero HarfBuzz, zero stb_truetype** neste caminho. É 100% o núcleo clean-room
  do próprio glintfx (`glx_sfnt`/`glx_raster`/`glx_hint`, vendorizado em `glintfx/vendor/core/`,
  o mesmo núcleo que a ui usa quando `GLINTFX_OWN_FONT_ENGINE=ON`, ADR-0011/L1.20-FONTFLIP).
- Layout: `glyph_advance_px` (hmtx, PROPORCIONAL) + `kern_px` (tabela `kern`, formato 0) — **não**
  é grade monospace. Word-wrap/alinhamento (`TextOptions`) consomem a mesma matemática
  (`GlyphAdvanceSource`, fonte única com o bake).

### 1.3 O teto declarado do motor próprio, e o que realmente nos toca

Do ADR-0018 (seção "Negative/accepted as cost" + OUT list) e do cabeçalho de `draw2d.hpp`:

| Limitação declarada | Nos afeta? | Por quê |
|---|---|---|
| Sem GSUB/GPOS (shaping complexo, ligaduras) | **Não** | PixelOperatorMono é bitmap-style mono, sem ligaduras; não usamos fontes com features OpenType avançadas |
| Sem BiDi/RTL | **Não** | pt-br + en-intl são LTR (`project_i18n_canonico`) |
| Sem fallback multi-face | **Não** | Uma fonte só, `PixelOperatorMono.ttf`/`-Bold.ttf`, sempre presente (asset embarcado) |
| Sem COLR v0 (emoji colorido) | **Não** | Não usamos emoji no HUD/diálogo hoje |
| Kerning só via tabela `kern` (não GPOS) | **Verificar** | Precisa confirmar se PixelOperatorMono kerneia via `kern` clássico ou só GPOS — não testado neste dossiê (ver seção 6, pendência) |
| Sem hifenização | **Não** | Não hifenamos texto hoje (grade monospace corta por glifo) |

A única linha realmente incerta é o kerning — **não medi** se `PixelOperatorMono.ttf` tem tabela
`kern` clássica populada (o núcleo do glintfx só lê essa, não GPOS). Isso é uma pendência real,
não um "não afeta" — ver seção 6.

---

## 2. O probe da comparação visual (o que fiz, e por que fora do build do jogo)

Não toquei `platform/render2d/` nem o `CMakeLists.txt` do jogo (outro agente trabalha lá agora).
Em vez disso, compilei um programa standalone em
`/var/tmp/builds/.../scratchpad/fontflip_probe/` que:

1. Linka **diretamente** os `.c`/`.cpp` já baixados pelo FetchContent existente
   (`glintfx/vendor/core/src/{sfnt,raster,hint}.c` + `glintfx/src/text_raster.cpp`) — zero GL,
   zero RmlUi, zero SDL. Compile leve (5 arquivos), sem risco de OOM.
2. Também compila `third_party/stb/stb_truetype.h` do próprio GusEngine (o mesmo header que
   `font_atlas.cpp` usa hoje).
3. Renderiza a MESMA string de teste — `"HP 144/144 Gustaf Vance acao magica nao e forca
   bruta! AEIOUC ÁÉÍÓÚÇ 0123456789 AP/MANA %"` (maiúscula/minúscula/dígito/acento pt-br/símbolo,
   não só o caso mais fácil) — por **três** motores:
   - **`glx`**: núcleo soberano do glintfx, layout proporcional+kerning (o que `Draw2d::draw_text`
     faria).
   - **`stb`**: `stb_truetype` bakeado FRESCO no tamanho exato pedido, layout monospace fixo
     (replicando fielmente o que `font_atlas.cpp` faria SE fosse chamado com aquele `cell_px`
     exato).
   - **`shp`** ("as-shipped"): reproduz o pipeline REAL que está em produção — bake ÚNICO em
     `cell_px=16` (hardcoded em `render2d_sdl.cpp:229` e `render2d_gl3.cpp:331`) e depois
     amostragem NEAREST desse atlas de 16px pro quad de destino de `px_size` (exatamente o que a
     GPU faz em `draw_quad(uv, tex)` com `GL_NEAREST`). **Isto é o que o jogador vê hoje em tela**,
     diferente do `stb` acima (que é um bake hipotético "se rebakeasse no tamanho exato").
4. Enumerei os **11 tamanhos lógicos reais** que o jogo usa (não escolhi "os mais importantes"):
   `kCockpitTextPx=8` (o menor, "HP/max"), `kCockpitLabelPx=9`, `kLogTextPx=11`,
   `kVerbTextPx/kMiraLabelPx/kDefeatNotePx=12`, `kPanelTextPx=13`, `kDefeatBarkPx=14`,
   `kCockpitNamePx=15`, `kDefeatRebootPx=20`, `kFloaterTextPx/kBannerTextPx=24`,
   `kBannerTextPx+6=30` (banner de intro).
5. Gerei as imagens finais em `docs/tech/fontflip-visuals/` (16 comparações 2-linhas cobrindo
   TODOS os 10 tamanhos + as 6 combinações de "3 DPIs" traduzidas, mais 5 comparações 3-linhas
   `triplet_*` nos tamanhos mais reveladores, incluindo o `shp` "como-realmente-está-em-tela").

Comando de build (reproduzível, não deixou nada no repo):

```sh
gcc -O2 -c vendor/core/src/{sfnt,raster,hint}.c -I vendor/core/include
g++ -O2 -std=c++17 -c src/text_raster.cpp -I vendor/core/include
g++ -O2 -std=c++17 -c probe_main.cpp -I vendor/core/include -I src -I third_party/stb
g++ -O2 *.o -o probe -lm
./probe assets/fonts/PixelOperatorMono.ttf out/
```

---

## 3. Resultado da comparação (a prova é a imagem, não a extração)

### 3.1 Nos tamanhos reais do jogo (enumeração completa, 1x)

Arquivos: `docs/tech/fontflip-visuals/cmp_1x_*.png` (10 arquivos, um por tamanho).

- **≥9px (`kCockpitLabelPx` pra cima): GLX e STB-fresco são visualmente equivalentes.** Conferi
  9, 11, 15 e 24px lado a lado — mesma legibilidade, mesmos acentos pt-br legíveis nos dois.
  Ver `cmp_1x_09_cockpitLabel.png`, `cmp_1x_11_logText.png`.
- **8px (`kCockpitTextPx`, o único caso "HUD miúdo" que o jogo realmente usa): GLX degrada nos
  acentos.** `docs/tech/fontflip-visuals/cmp_1x_08_ZOOM_acentos.png` (recorte ampliado 12x, com
  margem vertical extra pra eliminar a hipótese de clipping do meu harness — refiz o teste com o
  dobro de margem e o defeito persiste) mostra os acentos de "ÁÉÍÓÚÇ" colapsados numa faixa de
  tiques praticamente ilegível no GLX, enquanto o STB-fresco no MESMO tamanho lê limpo.

### 3.2 O comparativo de 3 vias no tamanho crítico (o achado principal)

Arquivo: `docs/tech/fontflip-visuals/triplet_1x_08_cockpitText_menor.png` (3 linhas: GLX / STB-
fresco / SHP-como-em-produção-hoje).

**A linha `shp` (o que está em produção, HOJE) é a PIOR das três** — quase ilegível, claramente
pior que o GLX (que já tinha o problema dos acentos) e muitíssimo pior que o STB-fresco. Nos
tamanhos 15px e 24px (`triplet_1x_15_cockpitName.png`, `triplet_1x_24_floaterText_banner.png`) as
três vias já ficam equivalentes — a diferença desaparece.

**Leitura honesta disto:** o jogo hoje paga um preço de qualidade no HUD miúdo que **nenhuma das
duas opções do dossiê está pedindo pra pagar** — é um efeito colateral do bake fixo em 16px +
downscale de GPU que já está em produção, independente da decisão sobre o Draw2D. Adotar
`draw_text` do Draw2D (que bakeia CADA tamanho sob demanda, preguiçosamente, por
`Font2d`/atlas próprio) **elimina esse downscale** — mesmo com o defeito de acentos do GLX em
8px, o resultado tende a ficar **melhor** que o que já está em tela, não pior.

### 3.3 "3 DPIs" traduzido pro modelo do Draw2D

O item original (`GLINTFX-FONTFLIP-VALIDATE`) foi escrito pro `dp_ratio` da **ui** (RmlUi),
onde `set_dp_ratio()` escala a unidade `dp` — um mecanismo que o Draw2D **não tem**: texto no
Draw2D escala por PROJEÇÃO de câmera (`ADR-0018` item 6/D12), o mesmo esquema de todo sprite
(mundo 960x540 fixo, projetado pro pixel real da janela). **Confirmei isso no código**:
`render2d_gl3.cpp:502-516`/`render2d_sdl.cpp` desenham o texto num `RectF` de MUNDO
(`px_size` fixo, ex.: 8), que passa por `build_quad_screen(camera, ...)` — ou seja, o glifo é
bakeado UMA VEZ no tamanho lógico pedido, e a MESMA bitmap pequena é esticada pra tela real.

Traduzi "3 DPIs" pros fatores de escala que a câmera 960x540 realmente produziria nas 3
resoluções físicas do item original: **Steam Deck nativo (1280x800) ≈ 1.333x** (por LARGURA,
`camera_from_world_rect`), **1080p (1920x1080) = 2x exato**, **4K (3840x2160) = 4x exato**.
Testei nos dois tamanhos-base 8px e 15px (`docs/tech/fontflip-visuals/cmp_dpi_base08_*.png` e
`cmp_dpi_base15_*.png`).

**Resultado:** já em 10.7px (Steam Deck × 8px base) os acentos do GLX voltam a ficar legíveis —
o defeito é específico do valor **exato** 8px, não um patamar amplo. Isso quer dizer que, SE o
motor rebakeasse por resolução física (o que ele **não faz** — bakeia uma vez no `size` lógico),
o problema sumiria em qualquer uma das 3 resoluções-alvo. Mas como o bake é único e a tela só
estica a bitmap, **a degradação de 8px aparece IGUALMENTE em 1080p, Steam Deck e 4K** — a
resolução física do monitor não muda nada, porque não há re-bake por DPI. Isto é uma correção
importante à premissa original do item: "comparar nos 3 DPIs" só faz sentido pro
**dp_ratio da ui**; pro Draw2D, o eixo relevante é o **tamanho lógico** (`px_size`), não a
resolução física de saída.

---

## 4. As opções para o líder (custo e reversibilidade)

### Opção A — manter stb_truetype pro texto de jogo, migrar só as outras 5 primitivas

Estado atual da fatia (`draw_sprite`/`draw_filled_rect`/`draw_line`/`draw_rect_outline`/câmera):
o outro agente já está fazendo isso. `font_atlas.cpp` continua vivo, sem mudança.

- **O que quebra:** nada. Zero regressão de rasterização.
- **O que precisa revalidar:** nada novo — o `GLINTFX-FONTFLIP-VALIDATE` original (que era sobre a
  ui, não sobre o Draw2D) segue em aberto, mas sem urgência nova.
- **Reversível:** trivialmente — é o "não fazer nada" quanto a texto.
- **Custo:** perde-se o ganho de qualidade que a seção 3.2 mostrou (o downscale de 16px→8px
  continua em produção, com a degradação medida). Também perde o UTF-8 completo/kerning/
  word-wrap nativo do Draw2D (hoje o `text_metrics.hpp` trata UTF-8 mas com avanço monospace
  fixo, sem kerning real).

### Opção B — adotar o núcleo próprio para o texto de jogo (`Draw2d::draw_text`)

- **O que quebra:** o MODELO DE LAYOUT muda (monospace fixo → proporcional+kerning, seção 0.6) —
  todo código que hoje assume `glyph_advance(px_size) == kMonoAdvanceRatio*px_size` pra medir
  largura de texto (`text_metrics.hpp::text_width`, usado pra centralizar rótulos/números no
  HUD) **precisa ser substituído** por `Draw2d::measure_text()` (fonte única com o próprio
  `draw_text`, ADR-0018 (a)/(f)) — senão widths ficam incoerentes entre medir e desenhar.
- **O que precisa revalidar:** hinting/vsnap no tamanho 8px especificamente (o defeito medido na
  seção 3.2) — considerar reportar ao dev do glintfx como achado, ou aceitar o teto "não pior que
  hoje, mas com ressalva nos acentos miúdos" antes de adotar em produção. Confirmar se
  `PixelOperatorMono` tem tabela `kern` clássica populada (pendência da seção 6).
  **Perde-se** a Latin-1 supplement cobertura DIRETA do stb (nosso `font_atlas.cpp` já cobre
  160-255 hoje) — mas o núcleo do glintfx é UTF-8 completo (qualquer codepoint no `cmap` da
  face), então isso não é uma perda, é uma AMPLIAÇÃO de cobertura (só limitada pelo que o
  `.ttf` contém).
- **Reversível:** SIM tecnicamente (a API `Font2d`/`draw_text`/`measure_text` não vaza tipo de
  terceiro, pImpl trocável), mas ADR-0018 marca Q-A como ONE-WAY-DOOR NA PRÁTICA — vira contrato
  de pixel observado (goldens calibrados por tamanho/DPI precisam recalibrar), e compromete o
  núcleo soberano como caminho de produção por anos.
- **Custo de compilação:** ~3 TUs C a mais (`sfnt.c`/`raster.c`/`hint.c`) — desprezível.

### Opção C — caminho misto (UI em FreeType, texto de jogo no motor próprio)

**Já é, em parte, o que temos hoje** (RmlUi/FreeType pras 5 telas de menu, algo-a-decidir pro
HUD/mundo) — a pergunta é só qual rasterizador entra no segundo grupo. **É tecnicamente possível
ter os dois rasterizadores ao mesmo tempo?** Sim — ADR-0018 (f) é explícito: "o core é
compartilhado, nada mais é" — cascas hospedadas separadas (`text_raster`/`text_layout` do
Draw2D vs. `FontEngineOwn` da ui), atlases separados, texturas GL separadas, zero estado mutável
compartilhado. Isso já é o desenho ATUAL proposto pelo dev (RmlUi segue em FreeType via
`GLINTFX_OWN_FONT_ENGINE=OFF`, Draw2D usa o núcleo próprio incondicionalmente quando
`GLINTFX_MODULE_DRAW2D=ON`, porque o núcleo do Draw2D **não depende** da flag `OWN_FONT_ENGINE`,
só do `MODULE_DRAW2D`). **O custo de ter dois rasterizadores diferentes rodando ao mesmo tempo
na mesma tela** (RmlUi/FreeType nos menus, Draw2D/glx no HUD de batalha) é: (1) duas fontes de
verdade de rasterização — um fix de hinting/AA numa não se aplica automaticamente à outra
(aceito no ADR como custo, não resolvido); (2) `PixelOperatorMono` renderizada ligeiramente
diferente conforme a tela (menu vs. HUD de batalha) — visualmente pode não incomodar, já que
raramente as duas aparecem juntas na mesma tela (menus e HUD de batalha são telas distintas), mas
é um fato a declarar, não a esconder.

---

## 5. Recomendação técnica (decisão é do líder)

Given a evidência: **recomendo Opção B (adotar o núcleo próprio pro texto de jogo), com uma
condição**: antes de considerar isso "pronto pra produção", pedir ao dev do glintfx (via
prompt, não mexendo na lib) uma checada específica no `bake_glyph_into`/`glx_hint_outline` pro
caso de glifos ACENTUADOS compostos (á, é, ó etc.) em tamanhos ≤8px — o defeito medido na seção
3.2/3.3 é concreto e reprodutível, e é exatamente o tipo de achado que o item
`GLINTFX-FONTFLIP-VALIDATE` existia pra capturar ANTES de adotar.

Isso não é "não adote" — é "adote sabendo exatamente onde está o único ponto fraco medido, e
com o fato a favor de que o baseline atual em produção é pior ainda nesse mesmo ponto". A
decisão de queimar esse ponto fraco (aceitar, reportar upstream, ou não adotar até corrigir) é do
líder.

---

## 6. Pendências / o que este dossiê NÃO cobriu

- **Kerning real de `PixelOperatorMono`:** não confirmei se a fonte tem tabela `kern` clássica
  populada (o núcleo do glintfx só lê essa, não GPOS). Se não tiver, `kern_px()` sempre retorna
  `0.f` (contrato documentado, não crash) — o layout proporcional ainda funciona, só sem ajuste
  fino de kerning.
- **Não testei o caminho GL de verdade** (glyph atlas GPU, blending, tint) — o probe é 100% CPU
  (bake), reproduz a MATEMÁTICA de rasterização mas não o pipeline GL completo do Draw2D (que
  não pude compilar sem tocar `GLINTFX_MODULE_DRAW2D=ON` no build do jogo, reservado ao outro
  agente). Os PNGs mostram fielmente o que `bake_glyph_into` produziria — o upload/blend GL
  seguinte é aditivo simples (D8), não deveria alterar esta conclusão, mas não foi provado
  ponta-a-ponta.
- **Bold** (`PixelOperatorMono-Bold.ttf`) não foi comparado — só a face regular.

---

## 7. Commit

SHA a preencher após o commit local deste arquivo (Conventional Commit citando
`GLINTFX-FONTFLIP-VALIDATE`, sem push).
