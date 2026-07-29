# Plano — fim dos workarounds e 100% glintfx

**Autor:** Caetano (CTO). **Status:** proposto, aguardando aprovação do líder. Nenhuma linha de código foi tocada para produzir este documento.
**Medido em:** HEAD `dbed60a` (2026-07-29 14:36). A árvore está sob edição concorrente (remoção do `@font-face`, fatias 0/2 do plano de camadas); **reconferir as medições antes de executar qualquer item**.
**Complementa, não substitui:** `docs/tech/plano-camadas-sdl.md` (as 5 fatias SDL) e `docs/tech/glintfx-boundary.md` (a régua e o registro de lacunas). Este doc responde: ordem por cascata, workarounds sem pedido, gestão de fatia bloqueada, definição de PRONTO, e o que é irreversível.

## 0. A lei que este plano executa

Canonizada pelo líder em 2026-07-29 (`CLAUDE.md`, "GLINTFX É A ÚNICA DEPENDÊNCIA DE FRAMEWORK"): proibido contornar lacuna; proibido reimplementar em casa o que caberia no glintfx; obrigatório sinalizar TODAS as necessidades pelo bus; lacuna encontrada = fatia PARA, sinaliza, AGUARDA. Alvo: zero dependência de framework fora do glintfx; sobra a biblioteca padrão de C++ e o que é regra do jogo.

## 1. Metodologia (comandos rodados, escopo declarado)

Script `measure_deps.py` (scratchpad da sessão; proposto para versionar em `tools/` na execução): parser de caractere que **remove `//` e `/* */` antes de contar**, preservando strings; padrões por dependência (`\bSDL_\w+\b`, `\bstbi_\w+\b`, `\bgl[A-Z]\w*\s*\(`, `\bstbtt_\w+\b`, `\bcrypto_\w+\b`, `glad`, `glintfx`).

Escopos varridos, declarados um a um:

- **app produção** = `GusEngine/app/src` + `GusEngine/app/include` + `GusEngine/app/main.cpp`
- **platform produção** = `GusEngine/platform/src` + `GusEngine/platform/include` + **`GusEngine/platform/rmlui`** (⚠️ minha primeira rodada esqueceu este subdiretório — a mesma classe de erro das 4 medições que mentiram hoje; corrigido e declarado)
- **core+domain** = tudo
- **fora**: `app/tests`, `app/tools`, `platform/tests`, `third_party`, `build` (não entram no binário `gusworld_app`; trava na seção 7)

Dois erros meus nesta medição, corrigidos antes de qualquer conclusão (registro porque a mecânica importa):

1. Padrão `glad` casou em **literais de string** (mensagens de log dizendo "glad") — inflou app/ para 6 arquivos. O uso real de API glad em `app/` é **zero**; o app consome o wrapper `gus/platform/rmlui/gl3_loader.hpp`.
2. `grep -c 'set_window_icon'` no `app.hpp` do glintfx devolveu 1 — mas era **`set_window_iconify_callback`** (substring, armadilha do `-w` de novo). `set_window_icon` **não existe** no glintfx v0.24.0.

## 2. Inventário confirmado (produção, binário do jogo)

A tabela do brief **confere integralmente** após as correções de escopo acima:

| dependência | `app/` (arq/occ) | `platform/` (arq/occ) | onde exatamente |
|---|---|---|---|
| SDL3 | **32 / 457** | 6 / 95 | app: mapa completo no `plano-camadas-sdl.md` §9; platform: `render2d_sdl.cpp` 49, `sdl_input.cpp` 34, `platform_info.cpp` 5, +3 headers |
| stb_image (`stbi_*`) | **6 / 13** | 2 / 14 | app: `app_icon` 4, `battle_preview` 4, `sdl_window` 2, `title/difficulty/save_load_menu_loop` 1 cada; platform: `render2d_gl3.cpp` 8, `render2d_sdl.cpp` 6 |
| GL cru (`glX(...)`) | 0 | **3 / 162** | `render2d_gl3.cpp` 94, `render2d_glintfx.cpp` 65, `rmlui/gl3_loader.cpp` 3 |
| glad (loader) | 0 | 2 | `rmlui/gl3_loader.cpp` (API) + `rmlui/RmlUi_Include_GL3.h` (o header do glad vendorizado — as "1450 ocorrências" são ele declarando a si mesmo) |
| stb_truetype | 0 | 1 / 2 | `render2d/font_atlas.cpp` |
| monocypher | 0 | 0 | só `core/src/crypto/` (3 arquivos, 10 occ) — **fica, regra do jogo** |

Pin do glintfx: **v0.24.0** (`GusEngine/CMakeLists.txt:438`), bumpado hoje.

## 3. Workarounds vivos — o censo, com e sem pedido

### 3.1 Com pedido no bus (estado em 2026-07-29 ~14:40)

| # | workaround / lacuna | pedido | estado |
|---|---|---|---|
| W1 | `@font-face` injetado por manipulação de string em 7 telas | API de fonte | **ENTREGUE** (`load_font_face`, v0.24.0, W18); remoção em execução agora por outro agente |
| W2 | Bracket `end()`/`begin()` do Draw2d em volta do texto (`render2d_glintfx.cpp:460`/`:482` no HEAD atual — as linhas 398/426 do brief mudaram com a edição de hoje) | `D2D-FLUSH` | aceito, W19 deles |
| W3 | Pipeline GL de texto duplicada do `Render2dGl3` dentro do `Render2dGlintfx` (`render2d_glintfx.cpp:111-191`, shader+VAO+VBO próprios — os 65 usos de GL cru do arquivo) | `D2D-TEXPIXELS` (aceito: "separar e expor") + rota de fonte (seção 4) | aceito; **mas ver seção 4: parte disto NÃO está bloqueada neles** |
| W4 | Coabitação GL no App mode (contexto corrente, loader, estado na entrada/saída) | `DOC-GLCOHAB` | aceito; (a)+(c) = doc; **(b) loader = decisão de fronteira deles, o ponto duro** |
| W5 | `stbi_load` para decodificar imagem→pixels (8 arquivos) | decode-para-pixels (inventário de 14:40) | enviado, sem resposta |
| W6 | `SDL_GetTicksNS`/`SDL_Delay` (17 occ) | relógio monotônico (Pedido C, retificação 14:35) | enviado, sem resposta |
| W7 | `SDL_Log` (30 occ) | log (Pedido D, retificação 14:35) | enviado, sem resposta |
| — | acentos 8px do motor de fonte deles (`FONT-ACCENT-8PX`) | retirado | era **culpa nossa** (corpo fora do envelope 9-13dp); cockpit já subiu para 9px (`battle_scene_render.cpp:61`, `kCockpitTextPx = 9.0f`) |
| — | bake fixo `cell_px=16` + downscale NEAREST | nenhum — **defeito nosso**, consertado em casa (commits `b80da6a`, `38144a4`), como manda a régua ("o código problemático é nosso?") | fechado |

### 3.2 SEM pedido correspondente — o pior caso, encontrado na varredura

Estes são contornos onde **o sinal sumiu e o custo ficou**. Três achados:

**S1 — Ícone de janela (`app/src/app_icon.cpp`).** Hoje: `stbi_load` decodifica o PNG → `SDL_Surface` → `SDL_SetWindowIcon`. Verificado por grep no `app.hpp` da v0.24.0: **não existe `set_window_icon`** no App mode (só `set_window_iconify_callback`, que é outra coisa). Quando a casca migrar (F4-3), o jogo **perde o ícone** e não há API para pedir de volta. Nunca foi sinalizado. **Ação: pedido novo no bus** (contrato sugerido: `set_window_icon(pixels, w, h)` — casa com o `D2D-TEXPIXELS` na forma "pixels em memória").

**S2 — Captura de frame → PNG (`capture_frame_to_png`).** Produção real, em **5 telas + `sdl_window.cpp`** (`sdl_window.cpp:503-539`, `title_menu_loop.cpp:490-495`, e save_load/difficulty/battle_preview): `glReadPixels` via `gl3_read_backbuffer_rgba` (o wrapper do glad em `platform/rmlui/gl3_loader.cpp`) + `stbi_write_png`. Duas consequências:

- **A mensagem de 14:20 ao glintfx contém um erro factual nosso**: dissemos que `stbi_write_png` era "só das nossas sondas de verificação visual, fora do binário do jogo". **Falso** — está em `app/src`, que compila em `gusworld_app`. Corrigir no bus (é a segunda retificação do dia; melhor nós que eles).
- O glintfx **já tem** `App::snapshot(ppm_path)` (`app.hpp:865-874`, captura FBO 0 pós-composição) — cobre o caso em App mode, **mas em PPM, não PNG**, e só no App (não no embed atual). **Ação: decisão do líder** (seção 9, D2): aceitar PPM (+conversão fora do binário), pedir PNG, ou mover a captura inteira para `tools/` (fora do binário — aí deixa de ser dependência de produto e a frase do bus vira verdadeira).

**S3 — Resíduo do backend RmlUi aposentado (`platform/rmlui/`).** `gl3_loader.cpp` + `RmlUi_Include_GL3.h` (o glad inteiro vendorizado) sobreviveram ao ADR-010 porque as telas ainda carregam funções GL para a casca SDL própria e para o readback do S2. Não é lacuna deles — é cadáver nosso que **morre por cascata** (seção 5): o load de funções morre com o App mode (glintfx carrega o próprio gl3w), o readback morre com o S2 resolvido. Sem pedido a fazer; entra aqui para o censo ficar completo e ninguém "consertar" isso com outro contorno.

**Verificados e NÃO são workarounds:** o glow por `drop-shadow` do `system_menu_rml.cpp` (receita documentada do próprio `docs/effects.md` deles, zero API improvisada); o modo headless `impl_==nullptr` do `Render2dGlintfx` (contrato de teste, não contorno); o translator i18n próprio (exceção **ratificada pelo líder** em 2026-07-21, registrada no `glintfx-boundary.md`).

## 4. O achado que muda a ordem: o texto NÃO está bloqueado no glintfx

O raciocínio corrente ("quando `D2D-TEXPIXELS` sair, o atlas vira `Texture2d`, o texto vira sprite, e morre a pipeline GL + glad + stb_truetype") é verdadeiro **mas é a rota longa**. Existe rota mais curta, e ela **já está desbloqueada**:

`Draw2d::draw_text`/`load_font`/`measure_text` **existem desde a v0.23.0** e estão no nosso pin atual. Os dois motivos para não usá-los caíram **hoje**:

1. O defeito de acentos em 8px era **corpo nosso fora do envelope 9-13dp** — o cockpit já subiu para 9px, e o dossiê (`fontflip-draw2d-dossie.md` §3.1) mediu que **de 9px para cima os dois motores empatam visualmente**.
2. O bake fixo 16px (que fazia o baseline em produção ser pior que qualquer opção) já foi consertado.

Ou seja: adotar o texto deles é **decisão nossa + trabalho nosso** (trocar `text_width`/grade monospace por `measure_text`/layout proporcional, recalibrar goldens), não espera de fatia nenhuma. As consequências em cascata:

- some a pipeline GL duplicada (**65 usos de GL cru** em `render2d_glintfx.cpp`, W3);
- some o bracket `end()`/`begin()` (W2) — texto vira draw batchado normal, a ordem de pintura é preservada pelo próprio batcher; **o `D2D-FLUSH` fica sem consumidor nosso** (avisar o glintfx para a W19 repriorizar — cortesia de fila, eles pediram transparência);
- some `stb_truetype` + `font_atlas.cpp`;
- o `D2D-TEXPIXELS` **continua valendo** para o caso geral (imagem gerada em memória: S1-ícone, composição, procedural) — mas sai do caminho crítico do texto.

**Pendências reais da rota:** (a) confirmar se `PixelOperatorMono.ttf` tem tabela `kern` clássica (pendência declarada do dossiê §6 — se não tiver, layout proporcional funciona sem kerning, contrato documentado); (b) é **one-way door na prática** (ADR-0018 deles: contrato de pixel observado, goldens recalibram) — decisão do líder, seção 9 D1.

A alternativa (rota B: manter stb_truetype como rasterizador e só subir o atlas via `D2D-TEXPIXELS`) **viola a lei como default**: rasterizar fonte é função de framework 2D que o glintfx já tem; manter o nosso é "reimplementar em casa". Rota B só é legítima como **exceção declarada** pelo líder (mesmo estatuto do monocypher), não como inércia.

## 5. As cascatas, mapeadas — e a ordem que minimiza trabalho jogado fora

```
C1 TEXTO (rota A, desbloqueada HOJE, só nossa)
   mata: GL cru do render2d_glintfx (65) + bracket W2 + stb_truetype + font_atlas
   esvazia: a necessidade do D2D-FLUSH (avisar) e metade do motivo do DOC-GLCOHAB(b)

C2 CAPTURA (S2: decisão D2 + eventualmente pedido PNG)
   mata: stbi_write + gl3_read_backbuffer + o resto do motivo do DOC-GLCOHAB(b)

C3 DECODE→PIXELS (pedido W5, aguardando) [+ TEXPIXELS para o caso geral]
   mata: stb_image inteiro (app 6 + platform 2, considerando C5 abaixo)

C4 ÍCONE (S1: pedido novo)
   mata: app_icon.cpp como está; pré-requisito de F4-3 não perder o ícone

C5 CUTOVER APP MODE (F4-3) + DECOMMISSION (F4-4, one-way)
   mata: SDL3 app (32 arq) + sdl_input/key_translation/platform_info +
         render2d_sdl + render2d_gl3 (94 GL cru + 8 stb_image) + gl3_loader/glad +
         RmlUi_Include_GL3.h + FetchContent SDL3
   depende: relógio (W6) + log (W7) respondidos; C4; decisão handle_event
            (plano-camadas §5); e DOC-GLCOHAB REDUZIDO a (a)+(c) se C1+C2 vierem antes
```

**A inversão que paga o plano:** o ponto duro do `DOC-GLCOHAB` é o **(b) — expor o loader de função GL**, que exige do glintfx uma decisão de fronteira cara (o loader é peça da trilha de internalização deles, não utilitário). Nós só precisamos de loader **porque desenhamos GL cru** (texto + readback). Se **C1 e C2 fecham antes de C5, o (b) desaparece do nosso pedido** — o `frame_callback` do App mode passa a receber só chamadas Draw2d/UiLayer, e o DOC-GLCOHAB encolhe para duas seções de documentação baratas. Fazer o cutover primeiro obrigaria o fornecedor a decidir a fronteira mais cara dele **para atender uma necessidade que nós mesmos vamos extinguir**. Ordem errada = trabalho deles jogado fora + fatia nossa parada atrás dele.

**Ordem proposta (o que roda quando):**

| onda | o quê | bloqueio |
|---|---|---|
| **agora** (em curso, não mexer) | remoção do `@font-face` (W1); fatias 0 e 2 do plano-camadas (código morto, tipo de tecla) | nenhum |
| **1** | Decisão D1 (rota do texto) → executar C1; avisar glintfx sobre o FLUSH; enviar pedidos S1 (ícone) e a retificação S2 do bus; decisão D2 (captura) | só decisões do líder |
| **2** | C2 (captura conforme D2) e C3 quando o decode→pixels responder; limpeza "aproveite cada toque" nos arquivos que a onda 1 tocar (commits separados) | resposta do bus (C3) |
| **3** | F4-3 cutover App mode (com relógio/log respondidos, ícone entregue, DOC-GLCOHAB já reduzido) | respostas W6/W7, S1; decisão handle_event |
| **4** | F4-4 decommission (one-way) + gate final zero (seção 7) | aprovação explícita do líder |

## 6. Fatia bloqueada: como fica visível (e a válvula)

A lei manda parar e esperar — correto, e a fila vai existir. Proposta de gestão, três amarras para o mesmo fato (auditável por grep, sem estado escondido):

1. **Bus**: a mensagem do pedido é a origem; o ID (ex.: `D2D-TEXPIXELS`) é a chave.
2. **`glintfx-boundary.md` §Lacunas**: linha na tabela viva (já é regra de lá: "entrada nova exige mensagem no bus").
3. **`TODO.md`**: item da fatia parada vai para `⏳` com o texto padronizado **"BLOQUEADO-GLINTFX: <ID do pedido> (enviado AAAA-MM-DD) — destrava: <o quê>"**. Nunca um item parado sem explicação; o status conta a história sozinho.

**O que se faz enquanto espera** (nada disso é contorno): fatias sem dependência (0/2 hoje), limpeza de camadas em arquivo já aberto por outro motivo (commit separado, regra do "aproveite cada toque"), testes/goldens da rota aprovada, conteúdo (CARDS, lore), e QA.

**Válvula de reavaliação (proposta, decisão D4):** pedido sem resposta há **7 dias** volta ao líder para re-decisão daquele item específico (esperar mais / re-escopar a fatia / pedir de outro jeito) — **nunca** decisão do agente, **nunca** contorno. Motivo de não temer a fila hoje, com número: o ciclo medido do glintfx em 2026-07-29 foi **pedido às 08:20 → tag v0.24.0 às ~14:25** (load_font_face + WM-VSYNC no mesmo dia). O risco de cronograma existe, mas o dado atual não o sustenta como objeção à lei; a válvula existe para quando o dado mudar.

## 7. Definição de PRONTO ("100%") — mecânica, não prosa

**100% = o closure de produção do `gusworld_app` não contém nenhum token de framework de terceiro fora do glintfx e das exceções NOMEADAS abaixo, medido por script com filtro de comentário, travado em gate de CI.**

Escopo do gate (`GATE(framework-only)`, entra junto com a onda 4): `app/src`, `app/include`, `app/main.cpp`, `platform/src`, `platform/include`, `platform/rmlui` (ou o que dele sobrar), `core/`, `domain/`. Zero para: `SDL_*`, `stbi_*`, `stbtt_*`, `glad*`, `gl[A-Z]*(`, e includes `<SDL`, `<stb_`, `<glad`.

Exceções, cada uma com trava própria (exceção sem trava é brecha):

| exceção | por quê | trava |
|---|---|---|
| biblioteca padrão C++ | linguagem, não dependência (seção 8) | nenhuma necessária |
| `monocypher` | regra do jogo (save-crypto) | permitido **só** sob `core/*/crypto/`; qualquer `crypto_*` fora desse path reprova |
| glintfx | é o framework | livre em `platform/`+`app/`; **zero em `core/`/`domain/`** (GATE(arch) existente continua) |
| `app/tests/`, `app/tools/` | não entram no binário | `GATE(tools-isolation)` do plano-camadas §6 (proíbe `add_subdirectory(tools)`); testes acompanham o tipo do contrato de produção |
| translator i18n próprio | regra de jogo ratificada (tiers de censura) | já é POCO em `domain/` sem lib externa — o gate nem o vê |
| FetchContent | manifesto fechado | lista permitida: `glintfx`, `RmlUi` (pin-espelho exigido pelo patch UB, infra DO glintfx), `Catch2` (teste). `SDL3` **sai da lista no F4-4**; adição nova = mudança de CI visível em PR |

Até lá, valem os gates transitórios já propostos no plano-camadas §8 (ratchet 32→só-cai, zero-tolerância por categoria fechada). O script de medição sai do scratchpad e **versiona** junto com o gate (ferramenta de verificação não versionada = medição não reproduzível).

## 8. Arbitragem: `std::chrono` e a linha linguagem × dependência

Arbitragem pedida, e dou com todas as letras: **a leitura está certa. Biblioteca padrão de C++ é linguagem, não dependência de terceiro.** A linha objetiva, com três testes que não dependem de opinião:

1. **Vem do toolchain** — nenhum FetchContent, nenhum vendor, nenhum pin, nenhum bump. Não existe "fila do fornecedor" para `std::chrono`.
2. **Especificada por ISO** — o contrato é da linguagem que o projeto já escolheu (C++20), não de um projeto com roadmap próprio.
3. **Presente em toda plataforma alvo por definição** — se o compilador está lá, ela está lá.

A lei do líder proíbe depender de **framework** fora do glintfx; a std lib falha os três critérios de "framework". Consequências práticas: (a) **pedir o relógio mesmo assim foi correto** — não pelo mérito, mas pelo protocolo ("sinalizar TODAS deixa a fronteira ser declarada por ELES"); a resposta "usem `std::chrono`" fecha o assunto de forma limpa e vira limite declarado no `capabilities.md` deles. (b) **Log é o caso oposto e o pedido é de mérito**: não há log na std lib (até C++26 não temos nada além de `iostream` cru), então "façam em casa" seria reimplementação real — o pedido D está certo em ser pedido forte. (c) Proponho canonizar no `glintfx-boundary.md`: *"a biblioteca padrão de C++ nunca é lacuna do glintfx; na dúvida sobre uma peça da std, o pedido vai ao bus como pergunta de fronteira, e 'usem a std' é resposta que encerra"*.

## 9. Decisões do líder (nenhuma tomada aqui; via AskUserQuestion pelo main)

- **D1 — Rota do texto.** A: adotar `Draw2d::draw_text`/`measure_text` (recomendada; one-way na prática — goldens recalibram, layout vira proporcional). B: manter stb_truetype + subir atlas via `D2D-TEXPIXELS` — só como **exceção declarada** tipo monocypher, porque como default viola a lei.
- **D2 — Captura de frame.** (i) aceitar `App::snapshot` PPM + conversão fora do binário; (ii) pedir variante PNG ao glintfx; (iii) mover a captura para `tools/` (sai do binário; a dependência deixa de ser de produto). Qualquer uma mata o S2; a (iii) é a mais barata e a mais honesta com o que a captura é (ferramenta de QA).
- **D3 — Ícone.** Enviar pedido S1 (recomendado: sem ele, F4-3 perde o ícone da janela) ou aceitar ficar sem ícone temporariamente no App mode.
- **D4 — Válvula dos 7 dias** (seção 6): aprovar o prazo e o rito (re-decisão pelo líder, por item).
- **D5 — Gates da seção 7** (definição de PRONTO + manifesto FetchContent fechado).
- **Herdadas do plano-camadas (§10), continuam pendentes:** tipo de `ScreenState::handle_event` no cutover; destino de `anim_preview.cpp`; destino de `battle_input.cpp`.

## 10. One-way doors (exigem "sim" explícito antes)

1. **D1-rota A** — contrato de pixel observado muda (ADR-0018 deles marca Q-A como one-way na prática); goldens e qualquer medição de largura de texto recalibram.
2. **F4-4** — decommission da casca SDL + remoção do FetchContent SDL3. Já reconhecido como one-way no TODO; recomendo **tag de preservação** antes (`pre-sdl-decommission`, mesmo rito da `pre-m8-godot-legacy`).
3. **Gate final zero** (seção 7) — reversão é mudança de CI visível; é a trava que impede o 100% de regredir em silêncio.
4. **Deleções em massa** (`render2d_gl3`, `render2d_sdl`, `font_atlas`, `platform/rmlui/`) — recuperáveis pela tag do item 2, mas a decisão de apagar é uma porta só de ida no fluxo normal.

## 11. Discordâncias e avisos (dever de contra-argumentar)

1. **Não discordo da lei** ("parar e esperar"). O dado de hoje (ciclo de horas no glintfx) não sustenta a objeção de cronograma. Discordo de operá-la **sem válvula**: fila sem prazo de reavaliação transforma qualquer silêncio do fornecedor em parada aberta. D4 resolve sem furar a regra.
2. **A mensagem de 14:20 ao glintfx tem um erro factual nosso** (stbi_write "fora do binário" — está dentro, 5 telas + sdl_window). Retificar no bus antes que eles planejem em cima de informação errada. Terceira mensagem de correção do dia é desconfortável e mesmo assim é o certo.
3. **Se D1=A, avisar o glintfx que o `D2D-FLUSH` perdeu o único consumidor nosso** antes da W19 deles — deixar um fornecedor construir capacidade para um uso que vamos extinguir é o espelho do erro que a inversão da seção 5 evita no DOC-GLCOHAB(b).
4. **Rota B como inércia seria violação silenciosa da lei** — se o líder a escolher, que seja como exceção nomeada e travada (tabela da seção 7), nunca como "ficou assim".

## 12. Rastreabilidade

- HEAD da medição: `dbed60a05be4d652b9a6128a81b18dd0919a1e56` (2026-07-29 14:36).
- Fontes primárias: `docs/tech/glintfx-boundary.md` (L1/L2), `docs/tech/plano-camadas-sdl.md` (fatias/números SDL), `docs/tech/fontflip-draw2d-dossie.md` (evidência visual da rota de texto), bus `gusworld_ia_autocomm` (mensagens de 2026-07-29: 08:20, 12:55, 13:40, 14:10, 14:20, 14:25, 14:35, 14:40).
- Verificações pontuais citadas: `render2d_glintfx.cpp:460/:482` (bracket) e `:111-191` (pipeline GL de texto); `app.hpp:865` (`snapshot`) e ausência de `set_window_icon` na v0.24.0; `sdl_window.cpp:503-539` e `title_menu_loop.cpp:490-495` (captura em produção); `battle_scene_render.cpp:61` (9px); `CMakeLists.txt:438` (pin v0.24.0); commits `b80da6a`/`38144a4` (fix do bake).
