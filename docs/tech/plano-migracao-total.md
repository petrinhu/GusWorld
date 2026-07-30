# Plano - migração total: glad, miniaudio "e todas as outras possíveis"

**Autor:** Caetano (CTO). **Status:** pesquisa + plano executável; nenhuma linha de código tocada para produzir este documento.
**Medido em:** HEAD `a105e60` (2026-07-30 00:41, gate `log-clock-zero`). Working tree com edições de OUTROS agentes no momento da medição (`player_sprites_loader.*`, `app/tools/CMakeLists.txt`, `app/tools/appmode_spike/`, `app/tools/idle_facing_probe.cpp`) - nenhuma fatia deste plano toca esses arquivos.
**Complementa:** `plano-camadas-sdl.md` (fatias SDL), `plano-fim-dos-workarounds.md` (cascatas C1-C5), `glintfx-boundary.md` (a régua). Este doc responde à ordem do líder de 2026-07-30: "inicie migração total de glad, miniaudio e todas as outras possíveis" - **ampliada na mesma noite** com: *"pesquisar também workarounds para acabar com eles usando framework glintfx"* e *"mande todas as necessidades para glintfx, NUNCA resolva sozinho"*. O censo de workarounds está na **seção 12** (adendo), e TODO item deste plano carrega uma das três classes: **(a)** executável por nós agora (API deles existe, é só consumir); **(b)** necessidade a encaminhar pelo bus (o plano descreve o pedido, nunca a solução caseira); **(c)** fica conosco por natureza (regra do jogo ou std C++, com justificativa).

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

**Adendos de 2026-07-30 (convergência independente + refinamento):** (1) o agente que executava a task #12 chegou às MESMAS duas conclusões (miniaudio feito, glad no `#error`) por caminho separado, antes de ler este plano, e parou sem contornar - duas medições independentes concordando é o melhor selo que um achado pode ter. (2) Achado DELE que este plano não tinha: mesmo com o `#error` relaxado, `gl_proc_address` devolve **um ponteiro por nome**, enquanto o glad fornece **~3.582 linhas de vocabulário** (tipos, enums, protótipos) - relaxar o gate destrava o carregamento, não substitui o vocabulário. Já está no bus. (3) Consequência levantada pelo orquestrador ao enviar a pergunta A3: **a resposta dela decide se o glad pode morrer de verdade** - se o preâmbulo de quadro for "contrato do host", manteremos vocabulário GL mínimo para sempre (de alguma fonte: um subset de protótipos é inevitável), e é melhor saber isso agora do que no meio do cutover.

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

## 12. Censo de workarounds v2 (adendo da mesma noite, por ordem do líder)

Ponto de partida: o censo do `plano-fim-dos-workarounds.md` §3 e o `glintfx-boundary.md` §Lacunas - usados como insumo, **não como lista fechada**. Método da varredura nova: (1) grep de sinais explícitos com filtro manual de contexto - ⚠️ armadilha linguística medida: "contorno" em pt-br é quase sempre *outline* visual (24 arquivos casaram, ~4 eram contorno-de-verdade); (2) cruzamento do CHANGELOG glintfx v0.23-v0.26 (5 versões em 2 dias) contra o nosso código, atrás de contorno cuja API já chegou e ninguém voltou para remover; (3) leitura dos blocos de comentário-dossiê das telas RML.

### 12.1 Mortos hoje, confirmados no código (nenhuma ação além de registro)

| workaround | matou | prova |
|---|---|---|
| W1 injeção de `@font-face` por string | `load_font_face` (v0.24) | grep: só comentários históricos restam (`difficulty_menu_loop.cpp:98-112`) |
| W2 bracket `end()`/`begin()` em volta do texto | `Draw2d::draw_text` (D2D-TEXT, `8ad8ee0`) | `render2d_glintfx.cpp:245`: "nao ha mais bracket" |
| W3 pipeline GL de texto duplicada | idem | GL cru do arquivo: 65 -> 7 (medido) |

### 12.2 Vivos COM pedido/entrega no bus (nada novo a encaminhar)

`DOC-GLCOHAB` (o (b)-loader esvaziou com a C1; restam as seções de doc); ícone de janela (pedido S1-ícone enviado 2026-07-29); `FRAMEGRAB-TEX` e `IMG-ENCODE` (viraram itens W21 deles, o primeiro BLOQUEANTE); `SDL_Delay` 5 usos e a travessia de path de asset (encaminhados pelo orquestrador em 2026-07-30 - o `SDL_Delay` é o gabarito da régua: pede-se mesmo esperando "usem a std", porque a fronteira é declarada por ELES). Entregues e já consumidos: decode (v0.25 - resto vira Fatia S1), clock e log (v0.26, `FATIA1-LOG-CLOCK`).

### 12.3 Achados NOVOS desta varredura - vivos e SEM pedido

**A1 - Ciclo por disco do fundo congelado. Classe (a) no transporte, condicionada a UMA decisão do líder; (já-pedido) na captura.** O fluxo hoje: `glReadPixels` (readback nosso) -> `stbi_write_png` em DISCO -> `IRenderer::load_texture(path)` relê o arquivo -> RAII apaga o PNG (`6e29cca`). O disco é transporte puro - e o TODO do glintfx chama exatamente este ciclo de "que nunca precisou existir". Desde a v0.25 existem `decode_image_memory` + `create_texture(pixels)`; o lado nosso que falta é um método aditivo pixels->textura no NOSSO `IRenderer` (trivial nos 3 backends: glintfx `create_texture`, gl3 `glTexImage2D`, sdl `SDL_CreateTexture` - zero lib nova, zero wrapper de terceiros, só consumo). Isso mata os `stbi_write_png` do fundo congelado SEM esperar a `IMG-ENCODE`, e **é a ponta nossa da ponte que o `FRAMEGRAB-TEX` vai plugar depois do cutover** (eles entregarão pixels em memória; o método já estará lá) - trabalho que sobrevive à reforma, não que ela joga fora. **Por que não é fatia desta rodada:** muda o contrato `IRenderer` (aditivo, mas contrato entre camadas = decisão de arquitetura do líder) e a família é grande (`i_renderer.hpp` + 3 impls + mocks + 5-6 telas + `sdl_window`/`maestro`). Fica DESENHADA e PARADA: quando o líder acordar, um "sim" destrava uma fatia por família com ~15 arquivos. ⚠️ Se aprovada, conferir na hora quais dos 8 `stbi_write_png` são fundo congelado e quais são captura-para-humano (`--battle`) - as duas metades têm destinos diferentes (memória vs `IMG-ENCODE`).

**A2 - Três contornos de BUG DE LAYOUT do RmlUi em `system_menu_rml.cpp`. Classe (b) - encaminhar.** A família que "ninguém chamou de contorno" porque parece CSS de design, e é compensação (o perfil exato do `cell_px=16`):
  1. **BUG-A flex colapsado** (`:480-495`): filho `display:flex` com `width:auto`/`100%` dentro de `overflow-y:auto` colapsa para a largura do padding (12dp) e quebra o hit-test; só largura ABSOLUTA (558dp) contornou. O comentário-dossiê já aponta o código do vendor.
  2. **Corredor da scrollbar** (`GLINTFX-SCROLL`, `:496-520`): a reserva de `GetScrollbarSize` do RmlUi (`LayoutDetails.cpp:178-188`) só vale para filhos com largura auto/percentual - que o BUG-A proíbe. Consequência: `width:546dp` mágico + `padding-right` compensatório no cabeçalho irmão. ⚠️ O comentário diz "bug reportado ao vivo pelo líder" - **conferir no bus se o report virou item deles**; se não virou, o sinal se perdeu e é exatamente o caso que a lei quer evitar.
  3. **Anel inset M2** (`:527+`): `box-shadow` externo é cortado pelo clip de overflow (clip_area=Padding, zero folga em 3 lados); contornado trocando para anel `inset`.
  - **O pedido (b), um só, em lote:** relatório de bug de layout ao glintfx (donos do RmlUi vendorizado + patches), com os três casos e os pointers de código que os dossiês já anotaram. **Custo do contorno atual, declarado:** duas larguras absolutas acopladas (546dp/558dp) que quebram em silêncio se o painel mudar de tamanho, e um cabeçalho que precisa imitar o corredor da lista para as colunas alinharem. Não propomos conserto nosso; a decisão patch-local vs upstream é deles.

**A3 - Os 7 GL cru do begin-frame embed** (`render2d_glintfx.cpp:119-125`; era a seção 4.4). **Classe (b) formal - ENVIADA ao bus em 2026-07-30** como pergunta (não pedido), com as duas leituras e sem preferência nossa: o embed deve expor prepara-alvo/clear, ou o estado GL de entrada é contrato do host (a DOC-GLCOHAB deles já tangencia)? Sete linhas de custo, zero urgência - mas o peso dela cresceu ao enviar: **a resposta decide o destino final do glad** (ver adendo da seção 3): "contrato do host" implica vocabulário GL mínimo permanente do nosso lado.

**Nota medida para a S1 (alpha):** `decode_image_file/memory` devolve **straight alpha RGBA8, idêntico ao `stbi_load` cru** - divergência DELIBERADA e documentada do caminho premultiplicado interno deles (`image.hpp:40-52`: "straight alpha is the honest, unsurprising default"). A troca da S1 é semanticamente 1:1; o risco de halo em borda semitransparente só existiria se alguém plugasse o caminho premultiplicado privado, que nem é público.

**A4 - Headers stb órfãos** (`stb_rect_pack.h`, `stb_image_resize2.h`): classe **(a)**, já é a Fatia S2.

**Verificados e NÃO são contornos** (para ninguém "consertar"): o glow por `drop-shadow` (receita documentada do `docs/effects.md` deles); `measure_text_width` do `Render2dGlintfx` (sobrescreve corretamente com `Draw2d::measure_text`, `render2d_glintfx.cpp:254-263`); a lacuna de modifier no capture de tecla (`key_translation_glintfx.hpp:133-137` - lacuna NOSSA pré-existente no caminho SDL, já fechada no caminho glintfx que a F4-3 vai consumir); vsync manual `SDL_GL_SetSwapInterval` (peça da casca, morre na F4-3 - `WM-VSYNC` v0.24 já cobre o App mode); `text_metrics`/`font_atlas`/`stb_truetype` (metade legada condenada pela F4-4, não compensação); conversão float->Sint16 da deadzone de gamepad (tradução de fronteira legítima, testada).

### 12.4 Classificação (a)/(b)/(c) consolidada do plano inteiro

| classe | itens |
|---|---|
| **(a) executável agora** | S1 (decode nas pipelines legadas), S2 (órfãos stb), S3 (gates) |
| **(a) desenhada, PARADA por decisão de contrato do líder** | A1 (pixels->textura no `IRenderer`, mata o disco do fundo congelado) |
| **(b) encaminhar pelo bus** | A2 (3 bugs de layout RmlUi, em lote), A3 (begin-frame embed); conferir se o `GLINTFX-SCROLL` do líder virou item deles |
| **(b) já encaminhados/na fila deles** | FRAMEGRAB-TEX (bloqueante), IMG-ENCODE, S1-ícone, SDL_Delay, travessia de path |
| **(c) fica conosco** | monocypher (trava por path), translator i18n (ratificado), std C++ (arbitragem §8 do plano-fim), conteúdo/regra de jogo |

Nota de fronteira para o líder (não é desta rodada, e não é workaround): `core/spatial`/`core/anim`/`core/math_util` são módulos genéricos que PRECEDEM o glintfx - pela diretriz "todo o genérico é deles", são candidatos de longo prazo a virar PROMPT em lote. É decisão de roadmap one-way (a F4 do freeze), não pedido desta noite; registro para o assunto não sumir.
