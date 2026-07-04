# ADR-013: Porteiro único de assets (`AssetSource`) — Fase 1 (indireção sobre filesystem)

**Status:** Accepted (4 decisões cravadas pelo líder supremo via AskUserQuestion, 2026-07-04)
**Data:** 2026-07-04
**Decisores:** líder supremo (petrus) + software-architect. Item `ASSETS-VFS-F1` do `TODO.md` (PI8).
**Cross-ref:** [ADR-006](ADR-006-crypto-hmac-formato-domain.md) (crypto SHA-256/HMAC própria, dep-free, reusada na Fase 2 pra selar o pacote), [ADR-007](ADR-007-controls-json-hash128-save-v4.md) (fronteira headless×I/O, oráculo de equivalência, envelope selado), [ADR-008](ADR-008-repivot-qt-to-sdl3.md) (SDL3), [ADR-009](ADR-009-rmlui.md)/[ADR-010](ADR-010-adopt-glintfx-embed-mode.md) (glintfx/RmlUi — origem do risco do staging de fontes), [ADR-011](ADR-011-m6-audio-onda1-plano.md) (áudio: 3º grupo de call-sites), memória `reference_formato_mapa_gmap` (o `.gmap` já usa o pipeline **fonte editável no repo → build empacota/sela → runtime lê do artefato**; este ADR estende o MESMO padrão pro resto dos assets). Item filho: `ASSETS-SELO-F2` (Fase 2, pré-release).

## Contexto

O jogo abre arquivos de asset (PNG, TTF, WAV/MP3, Markdown de i18n) em **~15 call-sites espalhados** por `platform/` e `app/`, cada um com seu próprio `stbi_load` / `fopen` / `ifstream` / `ma_sound_init_from_file` **direto sobre um caminho de filesystem**. O `core/asset_paths.hpp` já centraliza os **caminhos** (strings `constexpr`), mas NÃO o **ato de abrir** — cada consumidor ainda faz o I/O e, pior, cada um reimplementa a mesma lógica de resolução.

O líder decidiu o **timing** dos "assets embutidos/selados" em 2 fases (decisão de 2026-07-04):
- **Fase 1 (ESTE ADR, `ASSETS-VFS-F1`):** só a **indireção** — criar um "porteiro" único (`AssetSource`) que todos os call-sites passam a chamar, com implementação = filesystem (comportamento idêntico ao de hoje). Nenhuma proteção ainda.
- **Fase 2 (`ASSETS-SELO-F2`, pré-release):** trocar o **backend** do porteiro (em 1 ponto) de filesystem-solto para pacote **selado/cifrado**, reusando a crypto do `.gmap`/save (ADR-006). Anti-tamper + proteção de spoilers/lore + integridade.

O ganho arquitetural da Fase 1 é isolar a Fase 2 num único ponto de troca: hoje selar significaria mexer em ~15 lugares; depois deste ADR, significa trocar 1 implementação de interface.

### Achado da investigação (mapa factual dos ~15 call-sites, 2026-07-04)

**Imagens (7 arquivos, todos via `IRenderer::load_texture(path)` → `stbi_load`):**
- `platform/src/render2d/render2d_gl3.cpp:439` e `render2d_sdl.cpp:133` — o I/O real (`stbi_load`) vive aqui, nos 2 backends.
- `app/src/boot_pixel_overlay.cpp:60`, `app/src/sdl_window.cpp:144`, `app/src/screens/player_sprites_loader.cpp:88,104,132`, `app/src/screens/anim_preview.cpp:38`, `app/src/screens/battle_preview.cpp:169,1491,1521,1530,1532,1533,1535`.

**Fontes (4 arquivos):**
- `platform/src/render2d/font_atlas.cpp:44-63` — o I/O real (`fopen`/`fread` em `read_file`), consumido por `render2d_gl3.cpp:330` e `render2d_sdl.cpp:228` via `bake_font_atlas(resolve_font_path(...))`.
- `app/src/screens/system_menu_loop.cpp:106-110` e `battle_preview.cpp:629-631,743-745` — `fs::copy_file` que faz **staging** dos `.ttf` para o glintfx/RmlUi (ver "Risco do staging de fontes" abaixo).

**i18n (3 arquivos):**
- `app/src/i18n/translator.cpp:41-54` — o I/O real (`std::ifstream`), acionado por `maestro.cpp:115-116` (boot) e `battle_preview.cpp:1543-1545` (overlay de dev).

**Áudio (1 módulo):**
- `platform/src/audio/audio_engine.cpp:155,176,225` — `ma_sound_init_from_file` ×3 (SFX one-shot + música streaming).

**Duplicação de lógica de resolução (motivador central do porteiro):** o padrão **`env override > macro embutida pelo CMake > fallback relativo ao CWD`** está **replicado em pelo menos 6 lugares**, com variações sutis de nome de env e de raiz:
- `font_atlas.cpp::resolve_font_path` (env `GUSWORLD_ASSETS` + `GUSWORLD_FONTS_DIR` + CWD, com checagem de `exists` pra env não "sequestrar" a fonte);
- `translator.cpp::resolve_translations_path` (env `GUSWORLD_TRANSLATIONS` + `GUSWORLD_TRANSLATIONS_DIR` + CWD);
- `player_sprites_loader.cpp:154`, `sdl_window.cpp:49`, `anim_catalog.cpp:133` (cada um com `GUSWORLD_ASSETS`);
- **4 resolvers distintos só dentro de `battle_preview.cpp`** — imagem (env `GUSWORLD_ASSETS`, l.91), SFX (env `GUSWORLD_SFX` + `GUSWORLD_SFX_DIR`, l.116), música (env `GUSWORLD_MUSIC` + `GUSWORLD_MUSIC_DIR`, l.525), fonte.

Isso corta nos dois sentidos: é **argumento a favor** do porteiro (elimina 6+ cópias de uma lógica sutil e propensa a divergir) e é **risco de retrofit** (unificar mal pode quebrar um fallback já em uso — cada resolver tem uma raiz e um nome de env próprios que precisam ser preservados; ver "Riscos").

## Decisão

Criar uma abstração única de leitura de asset — `AssetSource` — que substitui os ~15 pontos de I/O direto. A Fase 1 tem **uma** implementação, `FilesystemAssetSource`, com o mesmo comportamento de hoje (inclusive a cadeia de resolução `env > macro > CWD`, agora consolidada num lugar só). Os 4 pontos abaixo estão **cravados pelo líder** (AskUserQuestion, 2026-07-04).

### 1. Onde a Fase 1 mora fisicamente: **arquivos soltos no disco, só a interface** (opção D)

Nada é empacotado na Fase 1. Os PNG/TTF/WAV/MP3/MD continuam soltos no repo/disco exatamente como hoje. A Fase 1 é **100% indireção**: `AssetSource` (interface) + `FilesystemAssetSource` (lê do filesystem) por trás dos call-sites. O empacotamento **e** a cifra acontecem juntos na Fase 2 (`ASSETS-SELO-F2`), num único movimento.

Motivo (registrado): empacotar agora sem cifra (opção B) ou embutir no binário agora (opção A `incbin`) gastaria esforço de empacotamento que a Fase 2 possivelmente refaz — a escolha final de formato do pacote selado (`.pak` cifrado vs embed no binário vs híbrido) é da Fase 2. Fazer o pacote duas vezes (uma sem selo agora, outra com selo depois) é trabalho jogado fora. A opção D entrega 100% do valor arquitetural da Fase 1 (o ponto único de troca) com custo mínimo e zero antecipação de decisão da Fase 2.

Isso espelha o `.gmap` (memória `reference_formato_mapa_gmap`): a **fonte editável** vive no repo; o **build** empacota/sela; o **runtime** lê do artefato. Na Fase 1, "empacotar/selar" ainda é identidade (lê o arquivo solto); a Fase 2 liga o empacotamento+selo trocando só o backend.

### 2. Superfície da abstração: **leitura + metadados (`size` + `mtime`)** — escolha consciente do líder

> **Nota de transparência (registrada a pedido do coordenador):** a recomendação original do software-architect foi a superfície **mínima** (só `read(id) -> bytes`, sem metadados), por anti-over-engineering — nenhum call-site atual consome tamanho ou mtime. O líder decidiu, por conta própria, **incluir metadados** (tamanho + mtime), com vistas a uso futuro (hot-reload de assets em dev, barra de progresso de carga). A decisão não é destrutiva, não quebra pillar nem a arquitetura de camadas, e é uma escolha de engenharia legítima ainda que além do estritamente necessário hoje. Acatada; documentada aqui como decisão do líder, não como a recomendação original.

Superfície (contrato conceitual; nomes exatos ficam a cargo do backend-engineer/engine-graphics-programmer na implementação, convenção mecânica):

```cpp
namespace gus::platform::assets {

// Metadados de um asset (Fase 1: derivados do filesystem).
struct AssetInfo {
    std::uint64_t size = 0;      // bytes
    std::int64_t  mtime = 0;     // epoch (s ou ms; cravar na impl), 0 = desconhecido
};

class AssetSource {
public:
    virtual ~AssetSource() = default;

    // Lê o asset inteiro. nullopt = ausente/ilegível (degrada, como hoje).
    [[nodiscard]] virtual std::optional<std::vector<std::byte>>
        read(std::string_view id) const = 0;

    // Metadados sem ler o conteúdo. nullopt = ausente.
    [[nodiscard]] virtual std::optional<AssetInfo>
        stat(std::string_view id) const = 0;
};

}  // namespace gus::platform::assets
```

Notas de contrato:
- `read` devolve `nullopt` em ausência/erro — **preserva** a semântica de "degrada graciosamente" que já existe (`font_atlas.cpp::read_file` devolve vetor vazio; `translator.cpp::load_from_file` devolve `false`). Nenhum call-site atual lança em asset ausente; o porteiro mantém isso.
- `stat` é o método complementar dos metadados (separado de `read` pra permitir consultar tamanho/mtime sem carregar o conteúdo — é isso que habilita hot-reload por comparação de mtime no futuro).
- `mtime` na Fase 2 (pacote selado) provavelmente vira o timestamp do pacote ou um campo do índice; o contrato aceita `0 = desconhecido` pra não amarrar a Fase 2 a um mtime por-arquivo que um blob selado pode não ter.
- Interface só-leitura. Escrita de asset (staging pro glintfx) NÃO passa por aqui (ver "Risco do staging").

### 3. Camada: **`platform/`** (confirmado — precedente do `IRenderer`)

`AssetSource` (interface) e `FilesystemAssetSource` (impl) moram em `platform/` (ex.: `platform/include/gus/platform/assets/asset_source.hpp`). Mesmo precedente do `IRenderer`: interface de header limpo, mas que **representa** uma capacidade de I/O, portanto vive em `platform/`, não em `core/`.

`core/` **não pode** ter I/O real (invariante das 4 camadas, auditada por grep no CI): o `asset_paths.hpp` já declara explicitamente no cabeçalho que fica em `core/` **por serem só strings** ("zero I/O, zero SDL/Qt; a LÓGICA de resolução permanece em quem usa"). Colocar `AssetSource` em `core/` contradiria esse comentário e a invariante — recusado. Divisão de responsabilidade que fica **mais limpa** depois deste ADR:
- `core/asset_paths.hpp` — **quais** são os caminhos lógicos (strings `constexpr`). Permanece.
- `platform/assets/AssetSource` — **como** abrir (o I/O + a cadeia de resolução consolidada). Novo.

### 4. Convenção do `id`: **a mesma string relativa já centralizada em `core/asset_paths.hpp`**

O `id` passado pra `read`/`stat` **é** o caminho relativo lógico que já vive em `asset_paths.hpp` (ex.: `"sprites/personagens_inspirados/gus/anims/battle_idle/f0.png"`, `"assets/fonts/PixelOperatorMono.ttf"`, `"game/translations/pt_br.md"`). **Zero tabela de remapeamento nova.** O `asset_paths.hpp` já É a camada lógica de nomes; o `AssetSource` recebe esse nome lógico e resolve a raiz física internamente (a cadeia `env > macro > CWD` que hoje está duplicada passa pra dentro da impl `FilesystemAssetSource`).

Consequência pra Fase 2: o pacote selado indexa os assets **por esse mesmo `id` relativo** — o índice do `.pak` (ou o nome do símbolo embutido) é a string de `asset_paths.hpp`. Continuidade natural com o `.gmap`, que já é endereçado por um caminho compilado sob `assets/maps/compiled/`.

Descartada: `id` namespaced/opaco desacoplado do layout (`"sprite:gus:idle:f0"`). Seria mais "à prova de reorg", mas o `asset_paths.hpp` **já promete** ser o único lugar a editar numa reorg (é literalmente o motivo de ele existir — decisão do criador 2026-06-25); um segundo esquema de nomes seria indireção redundante.

## Opções consideradas

**Eixo 1 — onde a Fase 1 mora:** (A) embutir no binário agora (`incbin`/arrays C): zero I/O externo já, mas incha o executável e antecipa/possivelmente refaz trabalho da Fase 2. (B) `.pak` agrupado sem cifra agora: Fase 2 só põe o selo em cima, mas gasta empacotamento já sem nenhum benefício de proteção. (C) híbrido texto-embutido/mídia-em-pak: soma a complexidade dos dois sem necessidade comprovada. **(D ESCOLHIDA)** solto no disco, só a interface: 100% do valor arquitetural (ponto único de troca) com custo mínimo; a decisão de formato do pacote fica coesa na Fase 2.

**Eixo 2 — superfície:** (mínima) só `read(id) -> bytes`: recomendação do arquiteto (anti-OE; nenhum consumidor usa metadados hoje). **(com metadados — ESCOLHIDA pelo líder)** `read` + `stat(size, mtime)`: além do necessário hoje, habilita hot-reload/progress no futuro; custo trivial na impl filesystem (`std::filesystem::file_size`/`last_write_time`).

**Eixo 3 — camada:** `core/`: recusado (viola invariante zero-I/O + contradiz o header do `asset_paths.hpp`). **(`platform/` — ESCOLHIDA)**: precedente do `IRenderer`, respeita a direção de dependência (`app → platform → core`).

**Eixo 4 — id:** namespaced opaco (`"sprite:gus:..."`): indireção redundante ao `asset_paths.hpp`. **(string relativa do `asset_paths.hpp` — ESCOLHIDA)**: zero tabela nova, o header já é a camada lógica; índice da Fase 2 reusa o mesmo id.

## Consequências

**Positivas:**
- A Fase 2 (selo/cifra) vira troca de **1** implementação de interface, em vez de mexer em ~15 call-sites. É exatamente o desacoplamento que justifica a peça.
- Elimina 6+ cópias divergentes da cadeia de resolução `env > macro > CWD`, consolidando-as em `FilesystemAssetSource` (menos superfície de bug, um lugar só pra corrigir/testar a resolução).
- Divisão de camadas fica mais nítida: `core/asset_paths.hpp` = **quais** caminhos; `platform/assets` = **como** abrir. Invariante zero-I/O do `core/` preservada.
- `id` = caminho relativo do `asset_paths.hpp` dá continuidade direta ao pipeline do `.gmap` (fonte→build→runtime) e reusa o índice na Fase 2.
- Testável headless: `AssetSource` é interface → um `FakeAssetSource`/in-memory nos testes prova os consumidores sem tocar disco (mesmo padrão do `IRenderer` headless).

**Negativas / aceitas como custo:**
- Metadados (`stat`) são superfície além do necessário hoje (decisão consciente do líder, não a recomendação do arquiteto): mais um método a implementar/testar e a manter coerente na Fase 2, sem consumidor atual.
- Uma camada de indireção a mais entre cada consumidor e o `stbi_load`/`fopen` — trivial em custo, mas é uma peça nova no caminho quente de carga (mitigado: `read` devolve o buffer inteiro uma vez; não há I/O por-frame).
- O retrofit dos ~15 call-sites (próximo dispatch) precisa preservar **exatamente** cada raiz/env por-consumidor (fontes usam `GUSWORLD_ASSETS`+`GUSWORLD_FONTS_DIR` com checagem de `exists`; áudio usa `GUSWORLD_SFX`/`GUSWORLD_MUSIC`; i18n usa `GUSWORLD_TRANSLATIONS`). Unificar sem regressão é o trabalho fino da próxima onda.

**Riscos / pontos de atenção:**
- **Regressão de resolução no retrofit:** as 6+ cadeias não são idênticas (nomes de env e raízes diferentes; a fonte ainda faz `exists`-check pra env não sequestrar a pasta errada). A impl `FilesystemAssetSource` precisa de uma resolução por **família de asset** (sprite/fonte/i18n/sfx/música) ou de um mapa `id-prefix → (env, macro, raiz)`. Test-first: um teste por família provando que o mesmo caminho resolve pro mesmo lugar de antes, com e sem cada env setado.
- **Risco do staging de fontes pro glintfx (escape hatch da Fase 2):** os `fs::copy_file` em `battle_preview.cpp`/`system_menu_loop.cpp` **não são leitura nossa** — são materialização de `.ttf` reais em disco pra o **glintfx/RmlUi ler sozinho** (a `UiLayer` não expõe `Rml::LoadFontFace`; o `@font-face` do doc referencia um arquivo por caminho relativo achatado). Na Fase 1 isso é indiferente (arquivos já estão soltos). Na **Fase 2** (tudo selado), o glintfx não saberá ler de dentro de um pacote cifrado: esse caminho vai precisar de um **escape hatch de materialização** — `AssetSource::read("assets/fonts/...ttf")` → decifra em memória → grava num arquivo temporário efêmero (tmpdir com limpeza) → passa o caminho do tmp pro glintfx. Registrado aqui pra a Fase 2 não ser pega de surpresa; é a única leitura de asset que **não** fica encapsulada dentro do porteiro (o consumidor final é uma lib de terceiros). Não resolver na Fase 1.
- **Escrita fora do escopo:** `AssetSource` é só-leitura por decisão. O staging acima (escrita de tmp) e qualquer save (que tem seu próprio envelope `GDS2`, ADR-006/007) **não** passam pelo porteiro. Manter o porteiro só-leitura evita transformá-lo num VFS genérico prematuro.

## Reversibilidade

- **Superfície da interface: two-way door em DEV.** Adicionar/remover método de `AssetSource` enquanto só há um punhado de consumidores internos é barato. Fica mais caro conforme os ~15 call-sites forem retrofitados, mas segue interno (sem base instalada). Decidido agora o suficiente pra a Fase 2 apoiar.
- **Convenção do `id` = caminho do `asset_paths.hpp`: one-way-ish rumo à Fase 2.** Assim que o índice do pacote selado (Fase 2) for endereçado por essa string, mudar o esquema de id vira breaking do índice. Por isso está cravado agora, antes de existir pacote.
- **Camada `platform/`: two-way door**, mas mover pra `core/` está barrado pela invariante (não é reversibilidade real, é restrição fixa).
- **Fase 1 solta no disco (opção D): two-way door total** — é literalmente o estado de hoje mais uma interface; a Fase 2 é quem toma a decisão irreversível-em-release (formato do pacote selado), fora deste ADR.

## Próximos passos (NÃO neste dispatch)

1. **Implementar** `AssetSource` + `FilesystemAssetSource` em `platform/assets/` (TDD, com `FakeAssetSource` in-memory pros testes) — backend-engineer / engine-graphics-programmer.
2. **Retrofitar** os ~15 call-sites pra chamar o porteiro em vez do I/O direto, preservando cada cadeia de resolução (test-first por família de asset).
3. Fase 2 (`ASSETS-SELO-F2`, pré-release, ADR próprio): trocar o backend por pacote selado/cifrado (crypto ADR-006), resolver o escape hatch de materialização de fonte pro glintfx, e cravar o formato do pacote (embed vs `.pak` vs híbrido).
