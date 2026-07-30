# Plano - migração total: glad, miniaudio "e todas as outras possíveis"

**Autor:** Caetano (CTO). **Status:** pesquisa + plano executável; nenhuma linha de código tocada para produzir este documento.
**Medido em:** HEAD `a105e60` (2026-07-30 00:41, gate `log-clock-zero`). Working tree com edições de OUTROS agentes no momento da medição (`player_sprites_loader.*`, `app/tools/CMakeLists.txt`, `app/tools/appmode_spike/`, `app/tools/idle_facing_probe.cpp`) - nenhuma fatia deste plano toca esses arquivos.
**Complementa:** `plano-camadas-sdl.md` (fatias SDL), `plano-fim-dos-workarounds.md` (cascatas C1-C5), `glintfx-boundary.md` (a régua). Este doc responde à ordem do líder de 2026-07-30: "inicie migração total de glad, miniaudio e todas as outras possíveis".

## 0. Resposta em uma linha (leia isto antes de lançar agente)

**Da tabela do brief, só UMA linha é executável hoje.** O miniaudio **já migrou em 2026-07-22** (commit `d79d880`, `audio_engine.cpp` consome `<glintfx/audio.hpp>`; `ma_*` em produção = **zero**, medido). O glad **não é migrável hoje**: `glintfx::gl_proc_address` tem `#error` de compilação sob `GLINTFX_BACKEND_GLFW=OFF` (`gl_proc.hpp:77-79` do pin), e nós fixamos `OFF FORCE` (`GusEngine/CMakeLists.txt:128`) - ele só existe no App mode, ou seja, **glad morre por cascata na F4-3/F4-4, não por fatia própria**. O `stbi_write` está bloqueado pela `IMG-ENCODE` (W21 deles) e o cutover pela `FRAMEGRAB-TEX` (promovida a BLOQUEANTE no TODO deles em 2026-07-30, verificado por leitura direta). O que sobra executável: **stbi_load nas duas pipelines legadas de platform, higiene de headers stb órfãos, e travas de gate**. Isso pede **3 agentes (2 paralelos + 1 sequencial), não 5** - detalhe na seção 5.

⚠️ **Consequência imediata para o orquestrador:** o agente `glad-e-miniaudio-para-glintfx` (tasks #12/#13, ambas `pending` na hora desta medição) precisa ser **parado ou reescopado**: a #13 já está satisfeita há 8 dias e a #12 bate num `#error` do fornecedor - se ele "resolver" isso, será por contorno, que é proibido.

## 1. Metodologia (comandos, filtro de comentário, escopo declarado)

Script `measure_deps.py` (scratchpad da sessão, evolução do usado nos dois planos anteriores): parser de caractere que remove `//` e `/* */` preservando strings; padrões `\bSDL_\w+\b`, `\bstbi_[a-z_]+\b`, `\bstbtt_\w+\b`, `\bma_[a-z0-9_]+\b`, `\bcrypto_\w+\b`, `glad`, `\bgl[A-Z]\w*\s*\(`, `\bRml::|<RmlUi`, `<miniaudio`.

Escopos declarados: **app produção** = `app/src` + `app/include` + `app/main.cpp`; **platform produção** = `platform/src` + `platform/include` + `platform/rmlui` (o subdiretório que a primeira medição de 2026-07-29 esqueceu); **core+domain** = tudo. Fora: `app/tests`, `app/tools`, `platform/tests`, `third_party`, `build`.

Armadilhas desta rodada, no espírito das oito de ontem:

1. O padrão `glad` devolveu "4 arquivos / 5 occ" em app - **todos são a palavra "glad" dentro de STRING de log/erro** (ex.: `"falha ao carregar funcoes OpenGL (glad)"`), não API. O parser preserva strings de propósito; a classificação exige olhar o contexto. O consumo REAL de glad pelo app é via o wrapper `gus/platform/rmlui/gl3_loader.hpp` (7 arquivos de produção o incluem).
2. `grep 'ma_\|miniaudio'` cru devolve dezenas de arquivos de `domain/` - substring dentro de palavra (`forma_`, `sistema_`...). Com `\bma_[a-z0-9_]+`: **zero**.
3. A pesquisa exaustiva de includes (seção 2.3) foi feita por enumeração fechada (todos os `#include` não-std, não-`gus/` da produção), não por busca dirigida - a lição do "enumere o espaço pequeno".

## 2. A pesquisa exaustiva: tudo que existe fora de glintfx + std

### 2.1 Tabela-mestra (produção, binário `gusworld_app`, medida em `a105e60`)

| dependência | app (arq/occ) | platform (arq/occ) | core+domain | estado / dono do destino |
|---|---|---|---|---|
| SDL3 | 24 / 347 | 6 / 95 | 0 | cutover F4-3/F4-4; ratchet teto 24 confere; **bloqueado** (ver 4.1) |
| `stbi_write_png` (encode) | 5 arq / 8 chamadas (`sdl_window` 1, `title/difficulty/save_load_menu_loop` 1 cada, `battle_preview` 4) | 0 | 0 | **bloqueado**: `IMG-ENCODE` W21 deles (7 formatos, disco+memória) |
| `stbi_load`/`stbi_image_free` (decode) | 0 (app_icon migrou em `46a12e3`, pin v0.25.0) | 2 arq / 14 occ (`render2d_sdl.cpp` 6, `render2d_gl3.cpp` 8) | 0 | **EXECUTÁVEL HOJE** via `glintfx::decode_image_file` - Fatia S1 |
| glad (loader GL) | 0 API direta (5 hits = strings de log) | `rmlui/gl3_loader.cpp` (3) + `rmlui/RmlUi_Include_GL3.h` (1450 = o vendor declarando a si mesmo) | 0 | **INEXECUTÁVEL HOJE** (seção 3); morre por cascata na F4-3/F4-4 |
| GL cru `gl*(...)` | 0 | 3 arq / 104 (`render2d_gl3.cpp` 94, `render2d_glintfx.cpp` 7, `gl3_loader.cpp` 3) | 0 | gl3 morre na F4-4; os 7 do glintfx são begin-frame (viewport/blend/clear) - ver 4.4 |
| stb_truetype | 0 | `font_atlas.cpp` (2) | 0 | vivo SÓ para as pipelines legadas gl3/sdl (o `Render2dGlintfx` migrou pro `Draw2d::draw_text` em `8ad8ee0`, 2026-07-29); morre na F4-4 |
| miniaudio (`ma_*`) | **0** | **0** | 0 | **JÁ MIGRADO** (2026-07-22, `d79d880`); mudou de dono: vive DENTRO do glintfx (`GLINTFX_MODULE_AUDIO=ON`) |
| monocypher | 0 | 0 | 3 arq / 10 (`core/src/crypto/`) | **fica para sempre** (regra do jogo, exceção nomeada) |
| RmlUi (`Rml::`) | 0 | 0 | 0 | nosso código não toca; o FetchContent nosso é pin-espelho exigido pelo patch UB (infra DO glintfx) |

### 2.2 Manifesto FetchContent (o que entra transitivamente, `GusEngine/CMakeLists.txt`)

| nome | tag | por quê | destino |
|---|---|---|---|
| SDL3 | `release-3.4.10` | casca atual | **sai na F4-4** |
| RmlUi | `2cd28864...` | pin-espelho do glintfx (patch UB) | sai quando o glintfx dispensar |
| glintfx | `v0.26.0` | o framework | fica |
| Catch2 | `v3.7.1` | teste, não shippa | fica (exceção) |

Sem FetchContent de miniaudio, glad ou stb: **o miniaudio entra hoje por DENTRO do glintfx** (a mudança de dono que o brief alertou - confirmada, e é o estado desejado, não um problema); glad e stb são vendorizados (`platform/rmlui/`, `third_party/stb/`).

### 2.3 O que a varredura exaustiva achou que NINGUÉM tinha mapeado

Enumeração completa dos `#include` não-std/não-`gus/` da produção devolveu, além dos já conhecidos: `<dlfcn.h>` e `<windows.h>/<winapifamily.h>` - **ambos DENTRO de `platform/rmlui/RmlUi_Include_GL3.h`** (o glad vendorizado; morrem com ele). Nenhuma dependência de terceiro oculta além das da tabela. E dois **headers órfãos** em `third_party/stb/`: `stb_rect_pack.h` e `stb_image_resize2.h` com **zero consumidores** no repo (grep em todo `.cpp/.hpp/.h/.txt/.cmake` fora de `third_party/stb` e `build`) - candidatos a purga imediata (Fatia S2), com o `ACKNOWLEDGMENTS.md` (linhas 59 EN / 124 PT) creditando "resizing, rectangle packing" que precisará encolher junto (a lição do M9: o crédito sobrevive à lib e vira mentira no README público).

## 3. O achado que muda o brief: glad NÃO é migrável hoje

O brief diz "glad: migrável HOJE via `glintfx::gl_proc_address`". **Medido, é falso**, por três camadas:

1. **`#error` de compilação:** `gl_proc.hpp:77-79` do glintfx: `#if !GLINTFX_BACKEND_GLFW` -> `#error "glintfx::gl_proc_address requires GLINTFX_BACKEND_GLFW=ON (App-mode GL cohabitation...)"`. Nós fixamos `set(GLINTFX_BACKEND_GLFW OFF CACHE BOOL "" FORCE)` (`GusEngine/CMakeLists.txt:128`, embed-only). Incluir o header hoje **não compila**.
2. **Semântica:** a função resolve símbolos "contra o contexto GL que o `glintfx::App` possui" e devolve nullptr antes de qualquer `App` ser construído (doc do próprio header, lido na íntegra). Em embed mode não existe `App` e o contexto é NOSSO (da casca SDL) - não há o que resolver.
3. **Arquitetura:** quem precisa de loader em produção são `render2d_gl3.cpp` (94 GL cru) e o readback do fundo congelado - exatamente as peças que a F4-3/F4-4 mata ou substitui. Migrar o loader delas antes do cutover é investir na peça condenada.

**Conclusão:** glad não tem fatia própria. Ele morre por cascata: F4-3 liga `BACKEND_GLFW=ON` e o `frame_callback` passa a poder usar `gl_proc_address` para o que sobrar; F4-4 apaga `gl3_loader.*` + `RmlUi_Include_GL3.h` inteiros. **Task #12: reescopar para "parte da F4-3/F4-4", não executar agora.** Se o agente em voo tentar, a única forma de "sucesso" é contorno (ligar o backend por conta, ou copiar loader) - proibido pela lei.

E a **task #13 (miniaudio) já está satisfeita desde 2026-07-22** (`d79d880`, `feat(audio): miniaudio direto -> modulo glintfx::Audio`). Zero `ma_` em produção, testes e tools (medido com `\b`). O que falta é só a TRAVA para não regredir (gate na Fatia S3) - o miniaudio já "mudou de dono" uma vez; o gate garante que não volta pelo outro lado.

## 4. O que está bloqueado, e por quem (não planejar execução)

### 4.1 Pela W21 do glintfx (verificado por leitura direta do TODO deles, HEAD `d9ca024`)

- **`FRAMEGRAB-TEX`** (⏳, promovido a **BLOQUEANTE do cutover** em 2026-07-30): o `frame_callback` deles roda entre `begin_frame`/`end_frame` com a layer offscreen do RmlUi corrente; nosso `glReadPixels` do fundo congelado capturaria a superfície errada pós-cutover. Eles confirmaram o caso e a saída (variante do `snapshot` devolvendo buffer + `create_texture(pixels)` da v0.25.0). **Bloqueia: F4-3, e com ela todo o SDL de app (24 arq/347) e platform (6/95), o glad, o GL cru do gl3, o stb_truetype e o FetchContent SDL3.** É o gargalo-mestre do plano inteiro.
- **`IMG-ENCODE`** (⏳, 7 formatos, disco+memória, decisão do líder deles de 2026-07-30): libera os 8 `stbi_write_png` de produção (5 arquivos). Nota de cascata: se a `FRAMEGRAB-TEX` sair na forma "textura direto, sem disco", o fundo congelado **deixa de precisar de PNG em disco** - os dois itens juntos podem matar o `stbi_write` de produção por dois caminhos; qual rota vence é decisão de design do líder quando ambos entregarem, não nossa.

**Registro obrigatório (padrão do plano-fim §6):** os itens correspondentes no `TODO.md` devem ficar `⏳ BLOQUEADO-GLINTFX: FRAMEGRAB-TEX / IMG-ENCODE (W21, promovidos 2026-07-30)` - execução disso é do orquestrador (este doc não toca `TODO.md`).

### 4.2 Por decisão do líder (dormindo = fatia parada, lista para quando acordar)

Herdadas e ainda abertas: tipo de `ScreenState::handle_event` no cutover (plano-camadas §5); destino de `anim_preview.cpp` e `battle_input.cpp` (§6); F4-3 (cutover) e F4-4 (one-way, tag `pre-sdl-decommission` antes); gate final zero (plano-fim §7). **Nenhuma fatia desta rodada depende delas** - foram desenhadas exatamente para não depender.

### 4.3 Para sempre (exceções nomeadas, com trava)

`monocypher` (só sob `core/*/crypto/`), translator i18n próprio, Catch2 (teste), biblioteca padrão C++ (arbitragem do plano-fim §8).

### 4.4 Registro novo: os 7 GL cru do `render2d_glintfx.cpp`

Sobraram 7 chamadas (viewport/disable/enable/blend/clear no começo do frame embed, `render2d_glintfx.cpp:119-125`). Não há hoje API de "prepara o alvo e limpa com cor X" no Draw2d embed. **Não é fatia nossa: é candidato a lacuna.** Pela lei: documentar no `glintfx-boundary.md` §Lacunas + mensagem no bus perguntando se o embed deve expor clear/viewport ou se o estado é responsabilidade do host por contrato (a DOC-GLCOHAB deles já toca nisso). Custo de manter: 7 linhas; risco zero de contorno novo. **Não bloqueia nada** - registro para o sinal não sumir.

## 5. As fatias executáveis AGORA - lista exata de arquivos e prova de disjunção

Regra de desenho: nenhuma fatia muda assinatura pública (todas são detalhe de implementação dentro de `.cpp` ou arquivos novos/removidos), logo a isolação por arquivo é suficiente - não há "família de chamadores" a arrastar.

### Fatia S1 - `stbi_load` -> `glintfx::decode_image_file` nas pipelines legadas

- **Arquivos (exatos, e só estes):** `GusEngine/platform/src/render2d/render2d_sdl.cpp`, `GusEngine/platform/src/render2d/render2d_gl3.cpp`.
- **O quê:** trocar `stbi_load(path)/stbi_image_free` (14 occ) pela `glintfx::decode_image_file` (v0.25.0+, já provada em produção pelo `app_icon`, commit `46a12e3` - copiar a receita de lá, inclusive o tratamento de erro). A criação de textura (SDL_Surface / glTexImage2D) a partir dos pixels não muda.
- **Por que vale mesmo a peça sendo condenada (F4-4):** custo ~1h, zera o `stbi_load` do binário inteiro (o que habilita o gate da S3), e a F4-4 não tem ETA (bloqueada pela FRAMEGRAB-TEX + playthrough do líder + teste Windows). Não é investir na peça: é tirar a dep dela sem mudar comportamento.
- **Verificação:** build + `ctest` preset linux-release (o `render2d_gl3_test` exercita `load_texture` por path); reviewer ADVERSARIAL separado sabota o caminho de erro (arquivo inexistente, PNG corrompido) e confirma paridade de comportamento.
- **Assinaturas públicas: nenhuma muda** (`load_texture(path)` mantém contrato).

### Fatia S2 - higiene: headers stb órfãos + crédito

- **Arquivos (exatos, e só estes):** `GusEngine/third_party/stb/stb_rect_pack.h` (deletar), `GusEngine/third_party/stb/stb_image_resize2.h` (deletar), `ACKNOWLEDGMENTS.md` (encolher o texto das linhas 59/124 para o que restou: image load/write + truetype), `GusEngine/third_party/stb/README-VENDOR.md` se listar os órfãos (conferir ao executar).
- **Pré-condição obrigatória:** re-rodar o grep de consumidores NA HORA (árvore concorrente): `grep -rn 'stb_rect_pack\|stbrp_\|stb_image_resize\|stbir_'` fora de `third_party/stb` e `build` deve dar vazio.
- **Reversibilidade:** two-way trivial (`git revert`). Nenhum CMake muda (o include-dir `third_party/stb` continua, pros 3 headers vivos).
- **Verificação:** build limpo do preset + grep pós-remoção.

### Fatia S3 - travas (sequencial, DEPOIS da S1)

- **Arquivos (exatos, e só estes):** `GusEngine/tools/check.sh` (+ `TESTES.md` se o padrão da casa pedir registro do gate; conferir ao executar).
- **Gates novos:**
  1. `GATE(audio-zero)`: zero-tolerância a `\bma_[a-z0-9_]+` e `<miniaudio` em app/platform/core/domain produção. Congela a migração de 22/07.
  2. `GATE(stbi-load-zero)`: zero-tolerância a `\bstbi_load|\bstbi_image_free` no mesmo escopo (habilitado pela S1; `stbi_write` fica FORA deste gate até a IMG-ENCODE).
  3. `GATE(fetchcontent-manifest)`: a lista de `FetchContent_Declare` no `GusEngine/CMakeLists.txt` é fechada em {SDL3, RmlUi, glintfx, Catch2}; nome novo reprova (o manifesto do plano-fim §7 vira trava mecânica; SDL3 sai da lista na F4-4 por mudança visível).
- **Por que sequencial:** o gate 2 só passa com a S1 mergeada; e `tools/check.sh` é o arquivo mais quente do repo (5 gates, 2 adicionados em 24h) - dois agentes nele é colisão certa.

### Prova de disjunção (interseção vazia, conferida arquivo a arquivo)

| | S1 | S2 | S3 |
|---|---|---|---|
| S1 | - | ∅ | ∅ |
| S2 | | - | ∅ |
| S3 | | | - |

S1 = {2 .cpp em `platform/src/render2d/`}; S2 = {2 headers em `third_party/stb/`, `ACKNOWLEDGMENTS.md`, `README-VENDOR.md`}; S3 = {`tools/check.sh`, `TESTES.md`}. Nenhum arquivo em duas fatias; nenhum toca os arquivos em edição na working tree de agora (`player_sprites_*`, `app/tools/*`), nem os alvos prováveis do agente glad-e-miniaudio (`platform/rmlui/*`, `platform/src/audio/*`) - que de toda forma deve ser parado (seção 3).

## 6. Quantos agentes de fato, e o que NÃO paralelizar

**Recomendo 3 agentes, não 5** (2 em paralelo + 1 sequencial):

| onda | agente | fatia | build? |
|---|---|---|---|
| A (paralelo) | 1 | S1 | sim, sob `flock /var/tmp/gusworld-build.lock` + `TMPDIR=/var/tmp` |
| A (paralelo) | 2 | S2 | sim (prova de build limpo), sob o mesmo flock |
| B (após S1) | 3 | S3 | roda `tools/check.sh` (leve) + 1 build de sanidade |

- **Edição em paralelo: 2.** **Build simultâneo: 1** - o flock já serializa; com 31 GB de RAM e swap em 8 GB, dois configures FetchContent + links simultâneos é OOM previsível, e o custo de serializar é minutos.
- **Por que não 5:** não existe trabalho para 5. As outras candidatas a fatia estão bloqueadas por fornecedor (W21) ou por decisão do líder (F4-3/F4-4, one-way). Inventar mais 2 fatias para ocupar agentes significaria tocar SDL/cutover - exatamente o que está proibido sem o líder. Melhor 3 seguras que 5 com uma parada no meio por decisão pendente.
- **Reviewer:** implementer ≠ reviewer; o review da S1 executa (sabotagem de caminho de erro), não só lê. O orquestrador re-verifica build + spot-check das claims antes de aceitar.
- **Commit:** por fatia, com ID citado (S1/S2/S3 ou os IDs que o orquestrador registrar no TODO), Conventional Commit, `git add` por arquivo nomeado (nunca pathspec amplo - a árvore tem untracked de outros), `git diff --cached --stat` antes e `git show --stat` depois. **Sem push** (modo autônomo do líder autoriza push por onda completa; a decisão de quando a onda fechou é do orquestrador).

## 7. Cascatas atualizadas (o mapa do plano-fim, com o estado de hoje)

```
C1 TEXTO ............... FEITA (8ad8ee0, 2026-07-29): Draw2d::draw_text no Render2dGlintfx;
                          restam font_atlas/stbtt SÓ pras pipelines legadas gl3/sdl
C2 CAPTURA .............. reclassificada (S2 do plano-fim = feature fundo congelado);
                          destino agora depende de FRAMEGRAB-TEX + IMG-ENCODE (W21)
C3 DECODE->PIXELS ....... METADE FEITA (app_icon, 46a12e3); o resto é a Fatia S1 deste plano
C4 ÍCONE ................ pedido S1-ícone no bus; pré-requisito da F4-3 (sem mudança)
C5 CUTOVER (F4-3/F4-4) .. BLOQUEADO por FRAMEGRAB-TEX (bloqueante declarado por eles)
                          + decisões do líder; quando sair, mata DE UMA VEZ:
                          SDL app 24 arq + SDL platform 6 arq + glad inteiro (gl3_loader +
                          RmlUi_Include_GL3.h + dlfcn/windows.h) + GL cru do gl3 (94) +
                          stb_truetype/font_atlas + render2d_sdl/render2d_gl3 +
                          FetchContent SDL3
```

A F4-3/F4-4 continua sendo o item que apaga categorias inteiras - e continua fora do alcance desta rodada. Tudo que esta rodada faz é (a) encolher o que o cutover terá de arrastar e (b) travar o que já foi conquistado.

## 8. Definição de PRONTO desta rodada

- **S1 pronta** = zero `\bstbi_load|\bstbi_image_free` na produção inteira (medido com filtro de comentário) + suíte verde + review adversarial executado.
- **S2 pronta** = os 2 headers fora do índice + `ACKNOWLEDGMENTS.md` dizendo a verdade + build limpo.
- **S3 pronta** = `tools/check.sh` reprovando sob sabotagem (plantar um `ma_engine e;` num .cpp de teste do gate, ver o gate ficar vermelho, remover - o gate que nunca reprovou não está provado) + verde no estado real.
- **Rodada pronta** = os 3 acima + TODO/tasks atualizados pelo orquestrador (reescopo da #12, fechamento da #13, itens BLOQUEADO-GLINTFX) + relatório ao líder com o que ficou bloqueado e por quem.

O "100%" global permanece o do plano-fim §7 (`GATE(framework-only)` pós-F4-4) - inalcançável nesta rodada por bloqueio de fornecedor, e é honesto dizer isso ao líder em vez de esticar fatias.

## 9. One-way doors (nada disto nesta rodada; "sim" explícito do líder antes)

1. F4-3 (cutover) e F4-4 (decommission + tag `pre-sdl-decommission`).
2. Apagar `render2d_gl3`/`render2d_sdl`/`font_atlas`/`platform/rmlui/` (vem com a F4-4).
3. Gate final zero / `GATE(framework-only)`.
4. Rota do stbi_write quando IMG-ENCODE e FRAMEGRAB-TEX entregarem (PNG em memória vs textura direta - design de feature).

As fatias S1-S3 são todas two-way doors (revert limpo), de propósito.

## 10. Discordâncias e avisos (dever de contra-argumentar)

1. **"Migração total de glad e miniaudio" não é trabalho executável** - miniaudio está feito há 8 dias e glad é `#error` sob nosso flag. A ordem do líder se cumpre HOJE assim: executar o que resta (S1-S3), travar o conquistado, e deixar registrado que o resto está na fila do fornecedor (W21) e nas decisões dele (F4-3/F4-4). Cumprir "total" à letra hoje exigiria contornar o `#error` ou antecipar o cutover sem as respostas - as duas coisas que a lei dele mesmo proíbe.
2. **5 agentes é excesso para o que sobrou** (seção 6). O gargalo não é mão de obra: é o fornecedor (FRAMEGRAB-TEX/IMG-ENCODE) e as decisões one-way que só ele pode tomar.
3. **O agente em voo (`glad-e-miniaudio-para-glintfx`) deve ser parado antes que gaste build ou invente contorno** - as duas tasks dele estão vazias de conteúdo executável (uma feita, uma bloqueada).
4. **Os 7 GL cru do embed** (4.4): resistir à tentação de "limpar" com abstração própria - é pergunta de fronteira pro bus, não fatia.

## 11. Rastreabilidade

- HEAD da medição: `a105e60` (2026-07-30 00:41). Pin glintfx: v0.26.0 (`GusEngine/CMakeLists.txt:497`).
- Verificações diretas citadas: `gl_proc.hpp:77-79` e doc integral da função (checkout canônico do glintfx, HEAD `d9ca024`); `CMakeLists.txt:128` (`BACKEND_GLFW OFF FORCE`); `image.hpp:184/199` (`decode_image_file/memory`); TODO do glintfx linhas 203-205 (GLLOADER-GENDRIFT, FRAMEGRAB-TEX, IMG-ENCODE); `d79d880` (miniaudio->glintfx, 2026-07-22); `8ad8ee0` (D2D-TEXT); `46a12e3` (app_icon decode); `render2d_glintfx.cpp:119-125` (7 GL cru); `ACKNOWLEDGMENTS.md:59/124`.
- Scripts: `measure_deps.py` no scratchpad da sessão (candidato a versionar junto com os gates da S3, para a medição do gate ser a mesma do plano).
