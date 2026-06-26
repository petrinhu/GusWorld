# ADR-009: RmlUi como camada de UI/HUD do jogo (sobre SDL3, dentro das 4 camadas)

**Status:** Accepted (ratificado pelo criador supremo via AskUserQuestion, 2026-06-25)
**Data:** 2026-06-25
**Decisores:** criador supremo (petrus) + software-architect (proposta)
**Cross-ref:** [ADR-008](ADR-008-repivot-qt-to-sdl3.md) (repivot Qt->SDL3, ja previu RmlUi na "fase 3"; este ADR-009 CONCRETIZA essa fase 3), [`docs/tech/pivot/engine-design.md`](../pivot/engine-design.md) (4 camadas, GATE), [`docs/design/mecanicas/battle-screen.md`](../../design/mecanicas/battle-screen.md) (Tatico Cockpit, pacing 2-beats, font-free), `GusEngine/CMakeLists.txt` (RmlUi 6.2 ja declarado).

> **STATUS: ACCEPTED.** O criador supremo ratificou via AskUserQuestion (2026-06-25) as 5 decisoes abaixo (ver "DECISOES RATIFICADAS"). Decorrencia: a **F0+F1 (PoC do cockpit)** foi disparada ao `engine-graphics-programmer`. O plano manda **PARAR e AVALIAR apos o PoC** (F1 e o gate de risco) antes de comprometer F2-F6. O detalhe tecnico abaixo segue valido como projetado.

---

## ADENDO (2026-06-25): backend de render = GL3 (OpenGL), nao SDL_Renderer

**Mudanca de decisao, ratificada pelo criador via AskUserQuestion apos o achado empirico da F0.**

**Contexto (achado da F0):** o andaime F0 foi construido sobre o backend oficial `RenderInterface_SDL` (SDL_Renderer). A composicao arena+HUD funcionou (texto FreeType, border, border-radius, body transparente, present unico). POReM, ao renderizar os EFEITOS do mock (gradiente `vertical-gradient`, `box-shadow`, glow), apareceram retangulos espurios: o `RenderInterface_SDL` oficial **NAO implementa `CompileShader`/`CompileFilter`** (retornam handle 0 na base `Rml::RenderInterface`). Ou seja, o SDL_Renderer puro **nao faz gradiente, box-shadow, blur, mask, drop-shadow** - justamente os efeitos que MOTIVARAM adotar RmlUi. So os backends com shader (GL3, VK, SDL_GPU) os implementam.

**Decisao:** trocar o backend de render do RmlUi de `RenderInterface_SDL` para **`RenderInterface_GL3`** (OpenGL 3.3 core, ja incluso no pin 6.2; traz glad header-only e implementa todos os shaders: gradiente, box-shadow, blur, mask, clip-mask, layers, filtros). SDL3 continua dono de janela + input + contexto GL (`SDL_GL_CreateContext`/`SDL_GL_SwapWindow`).

**Coexistencia arena + HUD (escolha: opcao A):** com GL nao ha mais `SDL_Renderer` (ele brigaria com o contexto GL manual). A arena (`Render2dSdl`, hoje sobre `SDL_Renderer`) migra para um backend OpenGL 2D enxuto (`Render2dGl3`) ATRAS da MESMA interface publica `IRenderer` - a API nao muda, entao a arena, o `battle_scene` e os ~1013 testes seguem intactos (so o backend interno vira GL). Motivo de preferir A sobre B (arena em render-target + composicao): B exigiria um SDL_Renderer offscreen separado + readback/upload por frame (custo na iGPU fraca, R-perf) + sync de textura; A da um unico contexto GL, um present (`SDL_GL_SwapWindow`), e controle do filtro NEAREST (pixel-art crisp) que o GL3 do RmlUi nao expoe por padrao (ele usa GL_LINEAR, bom pro chrome, ruim pro sprite). Arena desenha primeiro (NEAREST), HUD GL3 compoe por cima (efeitos nativos), swap unico.

**Impacto no CMake:** o backend vendorizado passa de `RmlUi_Renderer_SDL.*` para `RmlUi_Renderer_GL3.*` (+ `RmlUi_Include_GL3.h` com o glad). Linka GL do sistema (`OpenGL::GL` via `find_package(OpenGL)`). `RmlUi::RmlUi` + FreeType seguem.

**Reversibilidade:** os view-models POCO e o GATE de 4 camadas nao mudam; a troca de backend e two-way (o `RenderInterface_SDL` continua no historico). O que vira one-way e o esforco de migrar a arena pra GL - mitigado por A manter a API `IRenderer` (a logica de cena/test nao reescreve).

---

## DECISOES RATIFICADAS (criador supremo, AskUserQuestion 2026-06-25)

| # | Decisao | Escolha fechada | Decorrencia |
|---|---|---|---|
| 1 | **Fonte do chrome** | **FreeType** (`RMLUI_FONT_ENGINE=freetype`) | stb_truetype/`font_atlas` CONTINUA na arena (Render2dSdl); FreeType so no chrome RmlUi. `font-effect` (glow/outline) de graca. Convivem |
| 2 | **Escopo** | **so HUD/UI em RmlUi** | arena, sprites de ator e floaters de dano SEGUEM no Render2dSdl; RmlUi desenha o HUD POR CIMA, mesmo SDL_Renderer, em sequencia |
| 3 | **Build** | **FetchContent** (ja declarado, pin `6.2`) | nada de copiar pra third_party/ (la e header-only; RmlUi e compilada). So trocar `RMLUI_FONT_ENGINE` none->freetype quando a UI ligar |
| 4 | **Ordem de migracao** | **cockpit-first** | F1 PoC = o cockpit do mock `scratchpad/cockpit_otimo/index.html` (moldura TCG + botoes pill rococo/neon 3 estados + brasao GusWorld animado) vira `cockpit.rml`+`cockpit.rcss` |
| 5 | **GATE** | **estender** o `check.sh` | proibir `RmlUi`/`Rml::` (alem de Qt/SDL) em `core/`+`domain/`; torna a invariante das 4 camadas explicita |

**Execucao:** F0 (andaime RmlUi: RenderInterface+SystemInterface SDL3 + FreeType) + F1 (PoC cockpit com data-binding ao `battle_hud_model` POCO) disparadas ao `engine-graphics-programmer`. **Gate de plano: PARAR e AVALIAR apos o PoC** (paridade com o mock + pacing/motor verdes) antes de F2-F6.

---

## Contexto

### O problema-raiz (dor real, nao imaginaria)

Os mocks de UI sao HTML/CSS: gradientes, glow, `border-radius`, `box-shadow`, animacoes - tudo barato e lindo. O engine desenha **primitivas SDL cruas** via `IRenderer` (`draw_filled_rect`, `draw_rect_outline`, `draw_textured_rect`, `draw_text` glifo-a-glifo de um atlas stb_truetype). Resultado medido em playtest: a UI no jogo fica SEMPRE aquem do mock. O criador nomeou: *"voce me ilude: mock lindo, jogo com qualidade menor"*. E uma divida de fidelidade visual real, recorrente, com custo de retrabalho a cada tela.

A causa nao e falta de talento de arte: e que **reproduzir efeitos CSS (degrade radial, blur, sombra, transicao, raio de canto) a mao em primitivas SDL e caro e nunca empata** com um motor de layout/estilo dedicado. RmlUi e exatamente isso: um motor HTML/CSS-like (RML + RCSS) embarcavel em C++.

### O que ja esta decidido (nao reabrir)

- **RmlUi adotada** (decisao do criador, fechada). Licenca MIT, C++17 (compat C++20), backend SDL3 oficial, RCSS cobre os efeitos dos mocks, data-binding MVC.
- **ADR-008 ja previu** RmlUi como "fase 3" do repivot. O `CMakeLists.txt` da raiz **ja declara RmlUi 6.2 via FetchContent** com `RMLUI_SAMPLES=OFF`, `RMLUI_FONT_ENGINE=none`, `BUILD_SHARED_LIBS=OFF` - configurada mas **nenhum target linka ainda**. O andaime de build ja existe.
- **GATE das 4 camadas** (`tools/check.sh`): `core/` e `domain/` NAO incluem `<Q...>` NEM `<SDL...>`. Invariante auditada no CI.

### Constraints que moldam a decisao AGORA

1. O **motor de combate** (`domain/combat/`) ja esta portado/auditado e e POCO puro. **Nao pode ser tocado.**
2. A **BattleScreen** (Tatico Cockpit, variante C) esta em desenvolvimento avancado: pacing 2-beats funcional, HOLD de abertura espera-input, motor drenado por eventos, `PacingDirector` POCO testado. **Isso funciona e nao pode regredir.**
3. Ha **887 TEST_CASE/SCENARIO** (Catch2, headless), dos quais ~uma dezena cobrem `battle_layout` / `battle_scene` / `battle_menu` / `battle_hud_model` / `battle_log_model` / `battle_floaters` / `battle_pacing`. Sao o ativo mais valioso. **Nao podem ser jogados fora num big-bang.**
4. A **arena 2D** (sprites de ator, fundo, floaters de dano) ja roda no `Render2dSdl` com pacing e ancoragem de pe. **Funciona.**
5. Ja existe **font_atlas via stb_truetype** (`platform/render2d/font_atlas.cpp`) rasterizando Pixel Operator Mono (ASCII + Latin-1 pt-br). **A engine ja resolveu fonte uma vez.**

---

## Decisao (proposta)

**RmlUi vira o motor da camada de UI/HUD/telas (chrome): cockpit, menu de verbos, fila CTB, banner, log/terminal, overlay de COMPILAR, brasao animado, telas de menu/resultado.** Vive **exclusivamente em `platform/` (RenderInterface/SystemInterface sobre SDL3) + `app/` (documentos RML/RCSS, data-binding, controllers de tela)**. `core/` e `domain/` permanecem POCO INTOCADOS; o GATE continua valido (RmlUi e dependencia de `platform/`+`app/`, nunca de `core/`/`domain/`).

**A ARENA 2D (sprites de ator, fundo, floaters) NAO migra: continua no `Render2dSdl` atual.** RmlUi compoe POR CIMA da arena (HUD overlay). Migracao **faseada** comecando pelo cockpit como prova de conceito, sem quebrar pacing/motor/abertura.

Isto e uma **adicao de fronteira**, nao um redesign: o `IRenderer`/`Render2dSdl` continua dono dos pixels do mundo; o RmlUi assume o chrome. Espelha exatamente a divisao que o `engine-design.md` ja propunha para Qt ("RHI = pixels do mundo, Quick = chrome/menus") - so que com SDL3 + RmlUi no lugar de Qt RHI + Qt Quick.

---

## 1. Onde RmlUi entra nas 4 camadas

```
core/      POCO puro            [INTOCADO]  zero RmlUi, zero SDL
domain/    POCO puro            [INTOCADO]  combat/save/i18n/...  zero RmlUi, zero SDL
   |  (estado do jogo: HP, AP, Mana, fila CTB, intents, log, mao de cartas)
   v   via view-models POCO (battle_hud_model / battle_log_model ja existem)
platform/  fronteira SDL3
   +-- render2d/        Render2dSdl  -> ARENA (sprites, fundo, floaters)   [fica]
   +-- rmlui/  [NOVO]   RmlUiSdlRenderInterface (RenderInterface sobre SDL_Renderer)
                        RmlUiSystemInterface     (clock/log/clipboard sobre SDL)
                        RmlUiFontInterface        (so se NAO usar FreeType - ver secao 3)
app/       GusWorld-specific
   +-- screens/         battle_scene  -> orquestra: arena (Render2dSdl) + HUD (RmlUi)
   +-- ui/  [NOVO]      *.rml + *.rcss (documentos de UI: cockpit, ctb, banner, log...)
                        controllers que ligam o view-model POCO ao RmlUi::DataModel
```

### Fluxo de dados domain -> app -> RML (sem violar camada)

A regra de ouro: **RmlUi le, nunca escreve no dominio, e o dominio nunca conhece RmlUi.**

1. `domain/combat/` produz o estado (ja faz: HP/AP/Mana/fila/intent/log/dano).
2. Os **view-models POCO que JA EXISTEM** (`battle_hud_model`, `battle_log_model`, `battle_floaters`, `battle_layout`) sao a fonte de verdade da apresentacao. Eles ja sao testaveis headless e ja traduzem "estado do motor -> dados de tela".
3. Em `app/ui/`, um controller faz `RmlUi::DataModel` espelhar esses view-models: cada campo do data-model (`hp`, `ap_pips`, `active_name`, `log_lines`, `intent_icon`...) le do view-model POCO. O RCSS/RML consome via `data-value`/`data-for`/`data-if`.
4. Input do jogador: RmlUi captura o evento de UI (clique no verbo, tap na carta) e dispara um **comando POCO** (o mesmo que o `battle_menu`/`battle_scene` ja produz hoje), que vai ao motor. RmlUi nao toca regra de jogo.

**GATE continua valido:** RmlUi e `#include <RmlUi/...>`, nao `<SDL...>` nem `<Q...>`. O grep do `check.sh` audita Qt e SDL em `core/`+`domain/`; RmlUi nunca aparece la (vive em `platform/rmlui/` e `app/ui/`). **Recomendacao: estender o GATE** para tambem proibir `<RmlUi` em `core/`+`domain/` (uma linha no grep), tornando a invariante explicita em vez de implicita.

---

## 2. Render backend + coexistencia (arena vs HUD)

### Recomendacao: HUD em RmlUi POR CIMA, arena fica no Render2dSdl

A composicao mais simples e barata (e a que o backend oficial RmlUi/SDL3 suporta direto):

```
por frame:
  1. Render2dSdl.begin_frame()  -> desenha a ARENA (fundo, sprites de ator, floaters de dano)
  2. RmlUi context.Render()     -> desenha o HUD/chrome POR CIMA (cockpit, ctb, banner, log, overlay)
  3. SDL_RenderPresent          -> swap
```

Ambos desenham no **mesmo `SDL_Renderer`**, em sequencia (painter's order: arena primeiro, HUD depois). **Nao precisa render-to-texture.** O `RenderInterface` do RmlUi emite `SDL_RenderGeometry` (triangulos com textura) no mesmo renderer - convive com os `draw_*` do `Render2dSdl` sem conflito de contexto.

**Por que NAO migrar a arena pra RmlUi:**
- A arena e **espaco de jogo** (sprites com ancoragem de pe, floaters com timing, futuras animacoes de ataque/windup no Beat 1). Isso e logica de game-feel, nao layout de documento. CSS nao ajuda; atrapalha (RmlUi nao foi feito pra arena de combate animada).
- O pacing 2-beats e a fila de eventos vivem no `battle_scene`/`PacingDirector` POCO. Mover a arena pra RmlUi forcaria reescrever isso. **Risco alto, beneficio zero** (a arena ja esta visualmente OK; a dor e o CHROME).
- Floaters de dano: poderiam ser RmlUi (sao texto animado), mas estao acoplados ao timing do pacing. **Ficam no Render2dSdl no M5; podem migrar depois se o criador quiser glow/shadow neles.**

### Implementar o `RmlUi::RenderInterface` sobre SDL3: usar o backend oficial

O RmlUi 6.x **traz um backend SDL3 pronto** (`RmlUi_Renderer_SDL` / `RmlUi_Platform_SDL` em `Backends/`). Recomendacao: **adaptar o backend oficial** em vez de escrever do zero - ele ja resolve `SDL_RenderGeometry`, scissor (clip), texturas, e o mapeamento de eventos SDL->RmlUi. Encapsular dentro de `platform/rmlui/` atras de uma interface fina nossa (para o GATE e para nao vazar SDL pro app). Esforco: integracao, nao invencao.

**Ponto de atencao (R-comp):** o backend oficial assume que ELE controla o frame. No nosso caso o frame e compartilhado (arena primeiro). E preciso garantir que o RmlUi nao limpe a tela (`SDL_RenderClear`) nem dê present sozinho - so emita geometria. Trivial de ajustar no backend adaptado, mas e o detalhe que faz a coexistencia funcionar. Mitigacao na Fase 1 (PoC do cockpit).

---

## 3. Fontes: FreeType vs stb_truetype (decisao com nuance importante)

O criador framou como "FreeType = dep nova vs stb ja vendorizado". **Contra-argumento de arquitetura (dever de discordar):** a premissa "stb evita dep nova" e mais fraca do que parece, e "FreeType e mais simples" e mais forte. Detalho:

| Opcao | Como | Pros | Contras |
|---|---|---|---|
| **(A) FreeType** (default do RmlUi) | `RMLUI_FONT_ENGINE=freetype`; FreeType entra via FetchContent/vendor | Caminho **suportado e testado** pelo RmlUi (zero codigo nosso de fonte); hinting/kerning/subpixel de qualidade; cobre acentos pt-br e qualquer fonte futura sem esforco | +1 dependencia (FreeType, licenca FTL/GPL-compat OK pra GPLv3); +tempo de build |
| **(B) Custom FontEngineInterface sobre stb_truetype** | `RMLUI_FONT_ENGINE=none` (ja esta assim no CMake!) + implementar `Rml::FontEngineInterface` usando o `font_atlas` que JA temos | Zero dep nova; reusa o stb ja vendorizado e o font_atlas ja escrito/testado; controle total | **Trabalho nosso real:** implementar a interface inteira (medir string, layout de linha, kerning, fallback de glifo, geracao de geometria de texto, efeitos de fonte do RCSS tipo `font-effect: glow/shadow/outline`). stb_truetype e raster baixo-nivel; o RmlUi espera um motor de fonte COMPLETO. Os `font-effect` do RCSS (glow/outline na fonte) sao parte do "mock lindo" e dao trabalho a mao |

**Recomendacao do arquiteto: (A) FreeType.** Razao de trade-off:
- O objetivo de adotar RmlUi e **parar de reimplementar fidelidade visual a mao**. Escrever um FontEngineInterface custom e voltar a fazer exatamente isso (reimplementar tipografia a mao), so que num lugar mais critico. Contradiz o motivo da decisao.
- FreeType e a unica dep nova relevante, e barata: licenca compativel com GPLv3, build estavel, e o caminho que 99% dos usuarios do RmlUi seguem (logo, o mais testado e documentado).
- O `font_atlas`/stb atual **nao e desperdicado**: continua servindo a arena/floaters no `Render2dSdl` (que NAO usa RmlUi). So o CHROME passa a usar FreeType. Convivem.
- `font-effect` do RCSS (glow/shadow/outline no texto do log/banner) sai de graca com FreeType; com stb custom, cada um vira codigo nosso.

**Quando (B) faria sentido:** se o criador quiser ZERO deps novas como principio inviolavel, ou se o build de FreeType der problema no cross-compile Windows. Nesse caso, (B) e viavel mas custa ~3-5 dias so de FontEngine + risco de qualidade tipografica inferior ao mock. **A decisao e do criador** (DECISAO 1 no fim).

---

## 4. Build: vendor em third_party/ vs FetchContent

**Ja esta decidido de fato:** RmlUi entra por **FetchContent** (`CMakeLists.txt` linhas 56-70, pin `6.2`), igual SDL3 e Catch2. As 32 libs de `third_party/` sao **header-only** (incluidas sob demanda, fora do CMake); RmlUi e uma lib COMPILADA com seu proprio CMake - o padrao correto pra ela e FetchContent (como SDL3), nao copiar a arvore pra `third_party/`.

**Recomendacao: manter FetchContent.** Coerente com o que ja existe; pin de tag garante reprodutibilidade; `build/_deps` cacheia. Mudancas necessarias no CMake quando a UI for ligada:
1. `platform/CMakeLists.txt`: novo modulo `rmlui/`, `target_link_libraries(gusengine_platform PRIVATE RmlUi::RmlUi)`.
2. Trocar `RMLUI_FONT_ENGINE` de `"none"` para `"freetype"` SE a decisao 1 for FreeType (RmlUi puxa FreeType sozinho, ou declara-se via FetchContent).
3. `app/CMakeLists.txt`: copiar os assets `*.rml`/`*.rcss` pro diretorio de runtime (como ja faz com fontes/mapas).
4. Garantir `gusengine_core`/`gusengine_domain` **continuam sem** `RmlUi::RmlUi` no link (o GATE de build).

**Se FreeType der atrito no cross-compile Windows** (R-build): cair pro caminho (B) custom-stb e o plano B, ou vendorizar FreeType tambem por FetchContent com pin. Decidir empiricamente na Fase 1.

---

## 5. Plano de migracao FASEADO (anti big-bang)

Principio: **a cada fase, o jogo continua jogavel; nenhuma fase quebra pacing/motor/abertura.** O `Render2dSdl` da arena nunca para. O RmlUi entra tela-a-tela.

| Fase | Entrega | Pronto quando | O que NAO muda |
|---|---|---|---|
| **F0 - Andaime RmlUi** | `platform/rmlui/` com RenderInterface+SystemInterface (backend SDL3 adaptado) + FontInterface (FreeType ou stb, conf. decisao 1). Um `Hello RML` desenha 1 retangulo estilizado por cima da arena no smoke headless | smoke verde com RmlUi inicializado; GATE verde (RmlUi so em platform/+app/); 1 doc RML carrega e renderiza offscreen | arena, motor, pacing, todos os 887 testes |
| **F1 - PoC COCKPIT** (prova de conceito) | o mock `scratchpad/cockpit_otimo/index.html` (moldura TCG + botoes pill rococo/neon 3 estados + brasao GusWorld animado) vira `cockpit.rml`+`cockpit.rcss`, renderizado NO JOGO com data-binding ligado ao `battle_hud_model` POCO (HP/AP/Mana/retrato/verbos) | o cockpit no jogo bate o mock (gradiente, border-radius, glow, 3 estados de botao, brasao animado); pacing 2-beats intacto; HOLD de abertura intacto; motor intacto; clique no verbo dispara o MESMO comando POCO de hoje | arena (sprites/floaters no Render2dSdl), motor, fila de eventos, abertura-espera-input |
| **F2 - Fila CTB + Banner** | a faixa CTB (5 proximos, retratos) e o banner "VEZ DE X" viram RML/RCSS lendo o view-model | CTB e banner no jogo batem o mock; "+N" overflow funciona; banner troca por turno no ritmo do pacing | arena, motor, cockpit (F1) |
| **F3 - Log/Terminal** | o terminal/log fino (narra combate, cor por categoria, bold em COMPILADO/ERRO, scroll cortando) vira RML com `data-for` nas linhas | log no jogo le `battle_log_model`; cores por categoria; `font-effect` de enfase; rola no ritmo (D12) | arena, motor, F1/F2 |
| **F4 - Overlay de COMPILAR** | o leque da mao + pipeline de 3 slots (tap-to-place, snap, pulse de receita) em RML/RCSS; arena com dim atras (D5) | montar combo no jogo bate o mock; snap <100ms; pulse de receita; erros no log | arena, motor, F1-F3 |
| **F5 - Telas de menu/resultado** | tela de resultado (BUILD SUCCEEDED log-de-build), menus, modo-mira como overlay RmlUi | telas batem mock; transicao entrada/saida coerente | arena, motor |
| **F6 - (opcional) floaters em RmlUi** | SE o criador quiser glow/shadow nos numeros de dano, migrar floaters do Render2dSdl pra RmlUi | floaters com efeito; timing preservado | so os floaters; arena de sprites fica |

**Gate anti-big-bang:** F1 (cockpit) e a prova. So avanca pra F2 depois que o cockpit no jogo provar paridade com o mock E os testes de pacing/motor continuarem verdes. Se F1 falhar (coexistencia arena+RmlUi nao fechar, fonte ruim, perf na iGPU), reavalia ANTES de migrar o resto - custo afundado minimo.

---

## 6. Impacto nos TESTES / GATE

Esta e a parte mais delicada. **Contra-argumento importante (dever de discordar):** "layout vira CSS, joga fora o `battle_layout` POCO" e uma armadilha. RCSS NAO e testavel headless do jeito que o POCO e. Recomendo **preservar a fronteira testavel**, nao dissolve-la em CSS.

### O que acontece com os ~887 testes

A grande maioria (save, i18n, combat, progression, knowledge, sprites, pacing, hud_model, log_model, floaters) **nao toca em RmlUi e fica intacta.** RmlUi e camada de APRESENTACAO; o motor e os view-models POCO seguem testados como hoje. Os testes de pacing (`battle_pacing_test`, `battle_scene_test`) sao o cinto de seguranca que garante que F1-F5 nao regridem o ritmo.

### O `battle_layout` POCO: ainda vale OU vira RCSS?

**Recomendacao: o `battle_layout` muda de papel, nao morre.** Hoje ele calcula retangulos pixel-exatos (cockpit em x=0 w=174, CTB em x=188...) porque o Render2dSdl precisa de coordenadas. Quando o RML/RCSS assume o cockpit, **o RCSS passa a ser dono do posicionamento daquela zona** (e ai mora o ganho: flexbox/grid do RCSS faz o que hoje e constante magica).

Duas estrategias possiveis para os testes de layout:

- **(i) RCSS dono do layout fino, POCO dono das ZONAS macro.** O `battle_layout` deixa de cravar cada pixel interno do cockpit e passa a definir so as 4-5 ZONAS macro (retangulo do cockpit, da arena, do log, do banner) - que ainda precisam ser POCO porque a ARENA (Render2dSdl) precisa saber ONDE desenhar os sprites em relacao ao HUD. O layout INTERNO de cada zona (onde fica o retrato dentro do cockpit, gap dos pips) vira RCSS. Os testes de `battle_layout` ENCOLHEM para testar as zonas macro (que ainda importam pra arena) e os testes do interno do cockpit saem (viram responsabilidade do RCSS).
- **(ii) Manter o `battle_layout` como esta e o RCSS so estiliza (cores/efeitos), nao posiciona.** Menos ganho (perde o flexbox), mais seguranca (zero teste muda). Caminho conservador.

**Recomendacao: (i)**, mas **so a partir da zona que esta sendo migrada** (faseado). Na F1, so o interno do cockpit vira RCSS; o `battle_layout` mantem a zona macro do cockpit (pra arena saber o limite). Os testes que cravavam pixels internos do cockpit (`cockpit_portrait_rect`, `cockpit_hp_bar_rect`, posicoes de pip) saem ou viram testes de "a zona macro do cockpit e X". Os testes de arena/CTB-macro/log-macro continuam. **Quantitativo:** estimo que dos testes de `battle_layout_test` + `battle_hud_model_test`, uma fracao (os de geometria FINA interna) e aposentada por fase migrada; os de ZONA MACRO e de DADOS (view-model) ficam.

### Como testar UI RmlUi (sem display)

RmlUi nao tem o teste headless gratis que o POCO tem. Estrategias (em ordem de ROI):

1. **Manter o teste no view-model POCO, nao no RML.** O que importa testar e "o data-model expoe HP=13, 3 pips de AP, log com N linhas, verbo Atacar selecionado". Isso e testavel no `battle_hud_model`/`battle_log_model` POCO **sem RmlUi**. O RML so consome. **Teste o que alimenta o RML, nao o RML.** (Isto e a defesa de manter os view-models POCO vivos - a razao de NAO dissolver tudo em CSS.)
2. **Smoke de carga RML:** um teste que carrega cada `.rml`/`.rcss` num `Rml::Context` headless e verifica que parseia sem erro (documento valido, elementos esperados existem, data-model liga). Pega RML quebrado/RCSS invalido no CI. RmlUi roda headless (sem janela) pra parse/data-model.
3. **Snapshot opcional (pos-VS):** render offscreen de um doc RML pra bitmap + golden-image diff. Caro de manter (muda a cada ajuste de arte); deixar pra depois, se o criador quiser regressao visual.

**GATE:** estender o grep do `check.sh` pra proibir `<RmlUi` em `core/`+`domain/` (alem de Qt/SDL). Uma linha. Torna a invariante explicita.

**Saldo:** os ~887 testes em sua quase totalidade ficam; uma fracao pequena dos testes de GEOMETRIA FINA de HUD e aposentada por zona migrada (substituida por: teste de view-model POCO + smoke de carga RML). O motor, save, i18n, combat, pacing - o nucleo - nao perde 1 teste.

---

## 7. Riscos + esforco + alternativas

### Riscos

| # | Risco | Severidade | Mitigacao |
|---|---|---|---|
| R-comp | Coexistencia arena (Render2dSdl) + RmlUi no mesmo SDL_Renderer: clear/present duplo, ordem de desenho, scissor | Media | Backend SDL3 adaptado pra so emitir geometria (nao clear/present); validar na F0/F1 antes de migrar resto |
| R-perf | RmlUi na iGPU fraca (publico-alvo "maquinas fracas") - layout reflow + muitos draws | Media | RmlUi e leve (geometria em batch); HUD e estatico-ish (so muda no evento); medir FPS na F1; cachear documentos |
| R-font | FreeType no cross-compile Windows (MinGW/MSVC) | Media | Pin via FetchContent; se travar, plano B = custom stb FontEngine (decisao 1 ja preve) |
| R-pacing | Migrar o cockpit quebra o pacing 2-beats / HOLD de abertura | **Alta** | F1 e PoC isolada; testes `battle_pacing`/`battle_scene` sao gate; input do verbo dispara o MESMO comando POCO (zero mudanca de fluxo) |
| R-testes | Perder cobertura ao mover layout pra RCSS | Media | Estrategia (i) faseada: so aposenta teste de geometria FINA da zona JA migrada; view-models POCO continuam testados |
| R-escopo | RmlUi vira desculpa pra refazer TUDO (big-bang disfarcado) | Media | Plano faseado com gate por fase; arena fora de escopo explicitamente; F6 (floaters) e opcional |
| R-data-bind | Data-binding RmlUi nao casar com a cadencia de eventos do pacing (HUD atualiza no tempo errado) | Media | O controller le o view-model que JA respeita o pacing; RmlUi so reflete; testar na F1/F2 |

### Esforco (honesto, em dias de trabalho focado)

- **F0 (andaime RmlUi + interfaces SDL3 + fonte):** 2-4 dias (FreeType) / 5-8 dias (custom stb FontEngine). A escolha de fonte e o maior swing de esforco.
- **F1 (PoC cockpit, a prova):** 3-5 dias. Inclui resolver a coexistencia (R-comp) e o data-binding (R-data-bind). E o investimento de risco - se fechar, o resto e repeticao.
- **F2 (CTB+banner):** 1-2 dias.
- **F3 (log/terminal):** 2-3 dias (data-for nas linhas + cor/efeito por categoria).
- **F4 (overlay COMPILAR):** 3-5 dias (tap-to-place, pipeline, pulse - a tela mais interativa).
- **F5 (menus/resultado/mira):** 2-4 dias.
- **F6 (floaters, opcional):** 1-2 dias.

**Total realista:** ~14-23 dias uteis com FreeType (ate F5), +5 dias se custom-stb. F0+F1 (o risco todo) = ~5-9 dias - **fazer F0+F1 e PARAR pra avaliar** antes de comprometer o resto.

### Alternativas consideradas (e por que nao)

1. **Continuar com primitivas SDL + investir no atlas/efeitos a mao.** Rejeitada: e o status quo que CAUSA a dor; nao empata o mock; cada efeito vira codigo.
2. **Dear ImGui.** Rejeitada: e UI de ferramenta/debug, estetica de editor, nao faz a fidelidade "mock lindo" (sem CSS, sem skinning rico). Otima pra debug, pessima pra HUD de jogo bonito.
3. **Nuklear / outra immediate-mode.** Mesmo problema do ImGui.
4. **Render-to-texture da UI num layer separado.** Rejeitada como default: complexidade extra (gerir RTs, sync) sem beneficio - a composicao em sequencia no mesmo renderer e mais simples. Fica como plano B se R-comp nao fechar.
5. **Migrar a arena TAMBEM pra RmlUi.** Rejeitada: arena e game-feel animado, nao documento; alto risco, reescreve pacing, beneficio zero (a dor e o chrome).

---

## Consequencias

**Positivas:**
- A UI/HUD para de ficar aquem do mock: RCSS entrega gradiente/glow/border-radius/shadow/transition de graca. O mock VIRA a implementacao.
- Iteracao de UI fica barata (editar RCSS, nao recompilar logica de desenho).
- A divisao "arena = pixels do mundo / RmlUi = chrome" e limpa e ja prevista no design (ADR-008).
- `core/`/`domain/` intocados; GATE preservado (e reforcado).
- Motor e pacing intocados; ~887 testes em sua quase totalidade ficam.

**Negativas / aceitas como custo:**
- +1 dependencia compilada (RmlUi, ja no build) +possivelmente FreeType.
- ~14-23 dias de trabalho de migracao faseada.
- Uma fracao dos testes de geometria FINA de HUD e aposentada (compensada por view-model POCO + smoke de carga RML).
- Dois caminhos de fonte no projeto se escolher FreeType (stb pra arena, FreeType pra chrome) - aceitavel, convivem.

**Riscos / pontos de atencao:**
- R-pacing (Alta) e R-comp (Media) sao os que matam o PoC se mal feitos - por isso F0+F1 isoladas com gate.
- Nao deixar RmlUi virar pretexto de big-bang: arena fora, fases com gate.

## Reversibilidade

**Hybrid.** Adotar RmlUi e **two-way-ish**: a UI vive atras de view-models POCO; se RmlUi nao servir, volta-se ao Render2dSdl pro chrome (os view-models POCO continuam validos - eles nao dependem de RmlUi). O que e **one-way** e o ESFORCO afundado de reescrever as telas em RML/RCSS. A escolha de FONTE (FreeType vs stb) e two-way (troca de `RMLUI_FONT_ENGINE`). Por isso a Fase 1 (PoC cockpit) existe: prova a reversibilidade barata antes de afundar custo.

---

## DECISOES PRO CRIADOR (FECHADAS via AskUserQuestion, 2026-06-25)

As 5 decisoes abaixo foram apresentadas ao criador supremo e RATIFICADAS (registro consolidado no topo, "DECISOES RATIFICADAS"). Mantidas aqui com o desfecho para rastreabilidade.

1. **Fonte do chrome: FreeType (recomendado) ou custom stb_truetype?**
   - FreeType: caminho suportado do RmlUi, `font-effect` (glow/outline) de graca, +1 dep barata. Recomendado pelo arquiteto.
   - Custom stb: zero dep nova, reusa o que ja temos, mas ~3-5 dias de FontEngine + risco de tipografia/efeitos inferiores ao mock (contradiz o motivo de adotar RmlUi).
   - **FECHADO: FreeType** (`RMLUI_FONT_ENGINE=freetype`; stb_truetype segue na arena).

2. **Escopo: so HUD/chrome em RmlUi (recomendado) ou arena tambem?**
   - So chrome (cockpit/menu/log/ctb/banner/overlay/telas); arena de sprites fica no Render2dSdl. Recomendado.
   - Arena tambem: alto risco, reescreve pacing, beneficio zero. Nao recomendado.
   - **FECHADO: so HUD/UI** (arena/sprites/floaters seguem no Render2dSdl; RmlUi por cima).

3. **Build: FetchContent (recomendado, ja e assim) ou vendorizar em third_party/?**
   - FetchContent: coerente com SDL3/Catch2/RmlUi ja declarados; pin de tag. Recomendado.
   - third_party/: as 32 libs de la sao header-only; RmlUi e compilada - nao e o lugar dela.
   - **FECHADO: FetchContent** (pin `6.2`, ja declarado).

4. **Ordem de migracao: confirmar F0 -> F1 (cockpit, gate) -> F2 (CTB+banner) -> F3 (log) -> F4 (overlay COMPILAR) -> F5 (menus/resultado) -> F6 (floaters, opcional)?**
   - Ou priorizar outra tela primeiro? (ex.: o criador pode querer o LOG/terminal antes do cockpit, ja que "magia=software" e centrado em terminal.)
   - **FECHADO: cockpit-first** (F1 PoC = o mock `cockpit_otimo`). F0+F1 disparadas ao `engine-graphics-programmer`; PARAR e AVALIAR apos o PoC antes de F2-F6.

5. **(menor) Estender o GATE do check.sh pra proibir `<RmlUi` em core/+domain/ tambem?** (1 linha; torna a invariante explicita.) Recomendado: sim.
   - **FECHADO: sim** (proibir `RmlUi`/`Rml::` em `core/`+`domain/`, alem de Qt/SDL).
