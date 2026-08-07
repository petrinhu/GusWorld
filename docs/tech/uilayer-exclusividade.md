# UILAYER-EXCLUSIVIDADE: auditoria de exclusividade do `glintfx::UiLayer`

Auditoria **read-only**, segundo agente da fatia (o primeiro commitou `287c29b`,
verificado abaixo). Método: enumerar um universo fechado (grep de `UiLayer` em
`*.cpp`/`*.hpp`) e exigir conservação (soma das classes = N), nunca buscar
dentro dele.

**CORREÇÃO 2026-08-07 (`UILAYER-EXCLUSIVIDADE`, achado da auditoria de
2026-08-06):** o **N=233** abaixo (seção 1) **não era reproduzível** — um
auditor independente rodando o mesmo comando mediu 523 ou 359, conforme o
escopo. **Causa raiz medida:** o comando original (`grep -rn "UiLayer"
GusEngine/ --include='*.cpp' --include='*.hpp' | grep -v '/build'`) foi
digitado num shell interativo do Claude Code cujo `grep` é, na verdade, uma
**função que embrulha `ugrep --ignore-files`** — ela já respeita
`.gitignore` e tira `GusEngine/build/` da varredura **antes** do `grep -v
'/build'` redundante agir; por isso bateu, por acidente, com a contagem
sobre arquivos rastreados. Um `grep`/script fora desse shell (GNU grep puro,
subprocesso Python, outra máquina) **não filtra `.gitignore`**, e
`GusEngine/build/` nesta máquina pode conter **até 8 árvores de
build/scratch diferentes** (`linux-release`, `asan-gate`,
`caua_probe_scratch` etc.), cada uma com sua própria cópia vendorizada do
glintfx via FetchContent — uma única dessas árvores somou **2186**
ocorrências sozinha. O número final dependia de quantas árvores de scratch
existiam no disco **naquele instante**, um estado efêmero de sessão, nunca
canônico — daí a divergência 523/359 entre auditores honestos.
**Conserto:** `tools/uilayer_census.py`, commitado, varre **somente
`git ls-files`** (nunca o disco cru) — `GusEngine/build/` nunca aparece em
`git ls-files` porque está no `.gitignore`, então o número independe de
`grep` instalado, alias de shell ou árvores de scratch no disco. Rodado hoje
(HEAD pós-`287c29b`, árvore avançou desde a medição original):
**N = 294** (não mais 233 — código novo desde então, ex. `battle_ui_sfx.hpp`,
adicionou ocorrências reais). **A classificação exaustiva em classes a-e
(seção 2) NÃO foi re-derivada para o delta 233→294** — isso é trabalho de
julgamento de arquitetura (fora do escopo desta correção de
reprodutibilidade); os 6 sítios-dono e o veredito da seção 7 continuam
válidos para o que foi auditado, mas o próximo agente que tocar este
documento deve rodar `python3 tools/uilayer_census.py --list` e reconferir a
soma antes de declarar N atualizado.

## Estado auditado

- **SHA no início do trabalho:** `287c29bae380e3aafa17ae1883e09a2039ecea7d` (HEAD).
- **`git status --porcelain` no início:** `?? GusEngine/app/tools/appmode_spike/`
  (diretório untracked, conteúdo listado abaixo, não faz parte do universo
  varrido, ver nota). Isto é **menos** do que as "3 frentes não commitadas do
  líder" mencionadas no brief; entre o brief ser escrito e este agente começar,
  outra atividade na sessão aparentemente consolidou (commitou) as demais. Não
  toquei em nada não commitado.
- **`git status --porcelain` no fim:** ver seção final deste documento.

### O diretório untracked (`appmode_spike/`), fora do veredito desta fatia

```
GusEngine/app/tools/appmode_spike/CMakeLists.txt
GusEngine/app/tools/appmode_spike/main.rml
GusEngine/app/tools/appmode_spike/main.rcss
GusEngine/app/tools/appmode_spike/monitor_input.sh
GusEngine/app/tools/appmode_spike/appmode_spike_probe.cpp
GusEngine/app/tools/appmode_spike/RUNBOOK.md
```

`appmode_spike_probe.cpp` (o único arquivo `.cpp` ali) **não menciona `UiLayer`
nenhuma vez**, confirmado por grep direto no arquivo, zero ocorrências. A
única menção a "UiLayer" no diretório inteiro é uma linha de **comentário**
dentro de `main.rml` (arquivo `.rml`, fora do universo `*.cpp`/`*.hpp` definido
pelo método). Logo este diretório **não contribui nenhuma ocorrência** ao N
abaixo, e não precisou ser marcado na tabela por não conter nenhum sítio de
`UiLayer` em código C++. Registrado aqui por transparência, não por impacto no
veredito.

## 1. Universo fechado

Comando original (não reproduzível fora do shell específico que o mediu —
ver a nota de correção no topo do documento):
`grep -rn "UiLayer" GusEngine/ --include='*.cpp' --include='*.hpp' | grep -v '/build'`

**Comando reproduzível, commitado (2026-08-07):** `python3
tools/uilayer_census.py --list` — escopo declarado dentro do script
(`git ls-files GusEngine/*.cpp GusEngine/*.hpp`, nunca o disco cru).

**N = 233 ocorrências** medidas no commit `287c29b` (arquivo:linha, gravadas
na época em
`/var/tmp/builds/claude-1000/-home-petrus-IDrive-Documentos-projetos-claudebrain-Projects-gusworld/8513691b-1636-4828-ab4f-09f0f2ee63ad/scratchpad/uilayer_grep.txt`,
arquivo de trabalho não versionado) antes de qualquer classificação.
**Reconferido nesta correção rodando `tools/uilayer_census.py` sobre a
árvore no commit `287c29b`: bate exatamente em 233** — o número em si
estava certo naquele instante; o que faltava era o script que o
reproduzisse independente de shell/máquina. **No HEAD atual, o mesmo script
devolve N = 294** (ver nota de correção no topo).

## 2. Classificação exaustiva: cheque de conservação

| Classe | Descrição | Contagem |
|---|---|---|
| (a) | construção (`emplace` / ctor) | 17 |
| (b) | declaração de storage próprio (`optional<>`) | 6 |
| (c) | uso via referência/ponteiro recebido (parâmetro, padrão helper) | 5 |
| (d) | comentário ou string | 199 |
| (e) | include, alias ou assinatura | 6 |
| **Soma** | | **233** |

**233 = N declarado. A varredura está completa (cheque de conservação passou).**

### (a) construção: 17 sítios

6 são `ui_.emplace(glintfx::UiLayer::Config{...})` dentro das 6 classes-dono
(ver seção 3); os outros 11 são `glintfx::UiLayer ui(glintfx::UiLayer::Config{...})`
em pilha, dentro de ferramentas standalone e testes:

```
GusEngine/app/tests/difficulty_menu_loop_interaction_test.cpp:113   (helper load_probe_ui)
GusEngine/app/tests/save_load_menu_interaction_test.cpp:142         (helper load_ui)
GusEngine/app/tests/save_load_menu_interaction_test.cpp:904         (TEST_CASE direto)
GusEngine/app/src/screens/difficulty_menu_loop.cpp:366              (DifficultyScreen::enter, .emplace)
GusEngine/app/tools/framegrab_ordem/framegrab_ordem_probe.cpp:262   (probe standalone)
GusEngine/app/tools/sysmenu_controls_screenshot_probe.cpp:150       (probe standalone)
GusEngine/app/tools/npcdlg_screenshot_probe.cpp:162                 (probe standalone)
GusEngine/app/src/screens/battle_preview.cpp:360                   (BattleScreen::enter, .emplace)
GusEngine/app/src/screens/title_menu_loop.cpp:466                  (TitleScreen::enter, .emplace)
GusEngine/app/tools/sysmenu_pause_screenshot_probe.cpp:140          (probe standalone)
GusEngine/app/src/screens/npc_dialogue_loop_gl.cpp:240              (NpcDialogueScreen::enter, .emplace)
GusEngine/app/src/screens/system_menu_loop.cpp:857                 (SystemMenuLoopScreen::enter, .emplace)
GusEngine/app/src/screens/save_load_menu_loop.cpp:516               (SaveLoadScreen::enter, .emplace)
GusEngine/app/tools/msaa_glintfx_probe.cpp:306                      (probe standalone)
GusEngine/app/tools/frozen_bg_probe.cpp:199                         (probe standalone, bloco "dialogo")
GusEngine/app/tools/frozen_bg_probe.cpp:281                         (probe standalone, bloco "pausa")
GusEngine/app/tools/save_load_screenshot_probe.cpp:164              (probe standalone)
```

### (b) declaração de storage próprio: 6 sítios (exatamente as 6 classes-dono)

```
GusEngine/app/src/screens/difficulty_menu_loop.cpp:641      std::optional<glintfx::UiLayer> ui_;
GusEngine/app/src/screens/battle_preview.cpp:233            std::optional<glintfx::UiLayer> ui_;
GusEngine/app/src/screens/title_menu_loop.cpp:754           std::optional<glintfx::UiLayer> ui_;
GusEngine/app/src/screens/npc_dialogue_loop_gl.cpp:833      std::optional<glintfx::UiLayer> ui_;
GusEngine/app/src/screens/system_menu_loop.cpp:1629         std::optional<glintfx::UiLayer> ui_;
GusEngine/app/src/screens/save_load_menu_loop.cpp:931       std::optional<glintfx::UiLayer> ui_;
```

### (c) uso via referência recebida: 5 sítios (todos o mesmo padrão helper)

```
GusEngine/app/src/screens/difficulty_menu_loop.cpp:118      void register_pixel_operator_mono_fonts(glintfx::UiLayer& ui)
GusEngine/app/src/screens/title_menu_loop.cpp:155           void register_pixel_operator_mono_fonts(glintfx::UiLayer& ui)
GusEngine/app/src/screens/npc_dialogue_loop_gl.cpp:193      void register_pixel_operator_mono_fonts(glintfx::UiLayer& ui)
GusEngine/app/src/screens/system_menu_loop.cpp:183          void register_pixel_operator_mono_fonts(glintfx::UiLayer& ui)
GusEngine/app/src/screens/save_load_menu_loop.cpp:202       void register_pixel_operator_mono_fonts(glintfx::UiLayer& ui)
```

Nenhum destes 5 constrói, guarda ou dá origem a uma segunda instância: recebem
a referência da UiLayer que a própria classe-dono já possui, registram fontes,
e devolvem.

### (d) comentário ou string: 199 sítios

A imensa maioria (175 detectados automaticamente por `stripped.startswith("//")`,
mais 24 adicionais verificados manualmente: strings de diagnóstico em
`std::cerr`/`std::cout`/`std::fprintf`, mensagens `INFO`/descrição de `TEST_CASE`,
e 3 blocos de comentário `/* ... */` sem prefixo `//` na continuação). Lista
completa disponível no arquivo de trabalho do scratchpad (não repetida aqui por
volume; nenhuma dessas 199 ocorrências constrói, guarda ou expõe um `UiLayer`).

### (e) include, alias ou assinatura: 6 sítios

```
GusEngine/app/tests/difficulty_menu_loop_interaction_test.cpp:111   std::optional<glintfx::UiLayer> load_probe_ui(...)   (assinatura do helper de (a))
GusEngine/app/tests/save_load_menu_interaction_test.cpp:140         std::optional<glintfx::UiLayer> load_ui(...)         (assinatura do helper de (a))
GusEngine/app/tools/framegrab_ordem/framegrab_ordem_probe.cpp:313   const glintfx::UiLayer::CapturedFrame before = ui.capture_frame();
GusEngine/app/tools/framegrab_ordem/framegrab_ordem_probe.cpp:322   const glintfx::UiLayer::CapturedFrame mid = ui.capture_frame();
GusEngine/app/tools/framegrab_ordem/framegrab_ordem_probe.cpp:328   const glintfx::UiLayer::CapturedFrame after = ui.capture_frame();
GusEngine/app/tools/framegrab_ordem/framegrab_ordem_probe.cpp:366   const glintfx::UiLayer::CapturedFrame stale = ui.capture_frame();
```

As 4 últimas são variáveis de um **tipo aninhado** (`UiLayer::CapturedFrame`),
não do próprio `UiLayer`; não instanciam nem guardam a UiLayer, por isso não
entram em (a)/(b); classificadas aqui por serem usos de um tipo qualificado
pelo nome de `UiLayer` sem ser construção/storage/parâmetro.

## 3. Veredito por sítio dono (classes a+b)

Há exatamente **6 classes-dono** no repositório inteiro, uma por tela modal.
Cada uma tem exatamente 1 membro `std::optional<glintfx::UiLayer> ui_` e
exatamente 1 `.emplace()`. Pergunta feita: *"este sítio herda a garantia
estrutural do driver?"*

| Classe | Arquivo | `.emplace()` dentro de `enter()`? | `.reset()` dentro de `exit()`? | Todas instanciações via `run_screen_state()`? | Veredito |
|---|---|---|---|---|---|
| `DifficultyScreen` | `difficulty_menu_loop.cpp` | Sim (:366, `enter()` 350–517) | Sim (:521, `exit()` 517–…) | Único ctor em `:668`, seguido por `run_screen_state(screen, ...)` em `:669` | **HERDA** |
| `BattleScreen` | `battle_preview.cpp` | Sim (:360, `enter()` 289–…) | Sim (:1647, `exit()` 1612–…) | Único ctor em `:1659`, seguido por `run_screen_state(screen, ...)` em `:1660` | **HERDA** |
| `TitleScreen` | `title_menu_loop.cpp` | Sim (:466, `enter()` 451–615) | Sim (:620, `exit()` 615–…) | Único ctor em `:807`, dentro de um `for(;;)` que chama `run_screen_state(title_screen, ...)` em `:815` a cada volta | **HERDA** |
| `NpcDialogueScreen` | `npc_dialogue_loop_gl.cpp` | Sim (:240, `enter()` 234–397) | Sim (:404, `exit()` 397–…) | Único ctor em `:884`, seguido por `run_screen_state(screen, ...)` em `:885` | **HERDA** |
| `SystemMenuLoopScreen` | `system_menu_loop.cpp` | Sim (:857, `enter()` 842–1039) | Sim (:1044, `exit()` 1039–…) | Único ctor em `:1694`, dentro de loop que chama `run_screen_state(system_screen, ...)` em `:1704` | **HERDA** |
| `SaveLoadScreen` | `save_load_menu_loop.cpp` | Sim (:516, `enter()` 506–687) | Sim (:690, `exit()` 687–…) | Único ctor em `:960`, seguido por `run_screen_state(screen, sync_hook)` em `:962` | **HERDA** |

**⚠️ CORREÇÃO (apontada pelo revisor após o commit inicial, verificada e
aceita):** a coluna acima descreve o par emplace/reset canônico das 6
classes, mas **`BattleScreen` tem um terceiro toque em `ui_`** que a tabela
por si só não captura: `battle_preview.cpp:444`, um `ui_.reset()` **dentro do
próprio `enter()`** (range 289–1612, o mesmo onde está o `.emplace()` de
`:360`), não em `exit()`. Contexto:

```cpp
} else {
    std::cerr << "BattlePreview: [glintfx] UiLayer::ok()=false (attach falhou) "
                 "- caindo SEM UI neste run\n";
    ui_.reset();
}
```

É o **ramo de falha do próprio `enter()`**: se o attach da `UiLayer` não deu
certo (`ok()==false`), libera na hora e a tela segue sem UI; não é um sítio
fora do contrato, é limpeza de falha, e reforça a garantia (um layer que
falhou não fica pendurado até o `exit()`). Verifiquei as outras 5 classes
(`grep -n "ui_\.emplace\|ui_\.reset"` em cada arquivo) e **nenhuma tem
equivalente**: cada uma tem exatamente 1 `emplace()` + 1 `reset()`, só
`BattleScreen` tem os 3 toques. A formulação anterior deste documento
("`.reset()` só em `exit()`", sem essa ressalva) estava imprecisa; corrigida
aqui e na seção 7.

Prova de (ii) para cada classe: `grep -rn "\bNomeDaClasse\b"` no repo inteiro
(fora do próprio arquivo de definição) não retorna **nenhum outro** ponto de
construção; apenas comentários/docs citando o nome, e (para `BattleScreen`,
`TitleScreen`, `SystemMenuLoopScreen`, `SaveLoadScreen`, `NpcDialogueScreen`,
`DifficultyScreen`) as definições fora-de-linha dos próprios métodos. Cada
classe é `final : public gus::app::ScreenState`.

### Aninhamento (title→difficulty, pause→save/load)

**Título → Dificuldade** (verificado por mim, `title_menu_loop.cpp:807-843`):
o `TitleScreen title_screen(...)` é construído uma vez fora do loop; dentro do
`for(;;)`, `run_screen_state(title_screen, sync_hook)` roda e só then, **depois
que a chamada retorna** (o que estruturalmente já rodou `title_screen.exit()`,
liberando a `UiLayer` do título), o driver invoca
`run_difficulty_menu_loop_gl_current(...)`, que por sua vez constrói o
`DifficultyScreen` e chama seu próprio `run_screen_state`. A chamada aninhada
está no **mesmo nível de aninhamento do driver**, nunca dentro de
`handle_event()`/`tick()` do `TitleScreen`; não há caminho de código em que as
duas `UiLayer` (título e dificuldade) coexistam.

**Pausa → Salvar/Carregar** (`system_menu_loop.cpp`, `ui_.reset()` em `:1044`
antes das chamadas aninhadas em `:1671+`): **já verificado pelo líder da fatia
(CTO) antes deste brief**: cito conforme instruído, sem re-derivar.

## 4. A hipótese dos quatro `_rml` (+ `battle_input`)

Hipótese: `system_menu_rml.{hpp,cpp}`, `save_load_menu_rml.{hpp,cpp}`,
`battle_cockpit_rml.{hpp,cpp}` e `battle_input.{hpp,cpp}` são helpers de
binding chamados **pelos** loops, não telas próprias.

**Medida pela classificação da seção 2**: nenhuma das ocorrências de `UiLayer`
nesses 8 arquivos cai em (a), (b) ou (c): **todas são (d) comentário/string**.
Nenhum deles constrói (`emplace`/ctor), guarda (`optional<UiLayer>`) ou recebe
por referência um `UiLayer`. Tabela por arquivo:

| Arquivo | Ocorrências (linhas) | Classes |
|---|---|---|
| `save_load_menu_rml.hpp` | 12, 42 | D, D |
| `save_load_menu_rml.cpp` | 77 | D |
| `battle_cockpit_rml.hpp` | 72, 79, 83 | D, D, D |
| `battle_cockpit_rml.cpp` | 20, 46, 327, 418, 427, 517, 530, 608 | D ×8 |
| `system_menu_rml.hpp` | 38, 56 | D, D |
| `system_menu_rml.cpp` | 156, 537 | D, D |
| `battle_input.hpp` | 85 | D |
| `battle_input.cpp` | 82 | D |

**A hipótese se confirma por medição**: são helpers puros (geradores de
RML/RCSS por string, mapeadores de `id`→verbo, roteamento de clique) que
apenas **documentam** que a `UiLayer` real vive na classe-dono correspondente
e é passada/consultada de fora. Nenhum deles é uma tela própria.

## 5. Sítios não-dono: verdicts adicionais

### Helpers `register_pixel_operator_mono_fonts` (classe c, 5 sítios)

Recebem `glintfx::UiLayer&` de uma `UiLayer` já viva (a da classe-dono
correspondente, chamados de dentro do próprio `enter()`). **MANUAL-SEGURO**
por construção: não há storage próprio, a referência não sobrevive à chamada.

### Construções locais em pilha fora das classes-dono (11 sítios de classe a)

Dividem-se em duas categorias:

**PROCESS-SEPARADO** (9 sítios): cada um é o único `add_executable` do seu
próprio binário de ferramenta, nunca linkado ao jogo:
`framegrab_ordem_probe.cpp:262`, `sysmenu_controls_screenshot_probe.cpp:150`,
`npcdlg_screenshot_probe.cpp:162`, `sysmenu_pause_screenshot_probe.cpp:140`,
`msaa_glintfx_probe.cpp:306`, `frozen_bg_probe.cpp:199` e `:281` (dois
construtores no MESMO binário, mas em blocos `{ }` sequenciais dentro do mesmo
`main()`: o primeiro (`"dialogo"`) sai de escopo e destrói antes do segundo
(`"pausa"`) ser construído; verifiquei o código-fonte linha a linha, sem
overlap), `save_load_screenshot_probe.cpp:164`, confirmado via
`GusEngine/app/tools/CMakeLists.txt` e `GusEngine/app/tools/framegrab_ordem/CMakeLists.txt`
(cada nome tem seu próprio `add_executable`).

**MANUAL-SEGURO** (2 sítios, mesmo binário de teste `gusengine_app_tests`):
`difficulty_menu_loop_interaction_test.cpp:113` (helper `load_probe_ui`) e
`save_load_menu_interaction_test.cpp:142`/`:904` (helper `load_ui` e um
`TEST_CASE` direto): todas construções são locais de bloco `{ }` com RAII:
verifiquei especificamente o padrão em `save_load_menu_interaction_test.cpp:359-369`
e `:379-386`, cada `auto probe_ui = load_ui(...)` sai de escopo (destruindo a
`UiLayer`) antes do próximo ser aberto, com o comentário do próprio autor
confirmando a disciplina ("FECHA a UiLayer de sondagem ANTES do loop real abrir
a SUA PROPRIA"). Nota adicional: `catch_discover_tests(gusengine_app_tests ...)`
(ver `app/tests/CMakeLists.txt:184`) registra **um CTest por `TEST_CASE`**, o
que na prática invoca o binário filtrado por nome de teste; cada `TEST_CASE`
roda em processo próprio quando disparado via `ctest`. Mesmo se alguém rodar o
binário inteiro manualmente (todos os `TEST_CASE`s em 1 processo), o Catch2
executa cada `TEST_CASE` até o fim, incluindo destrutores, antes do próximo
começar; não há caminho para dois `UiLayer` de `TEST_CASE`s diferentes
coexistirem. **Nenhum destes 11 sítios é um caso de dois `UiLayer` simultâneos.**

## 6. Verificação da edição do agente anterior (commit `287c29b`)

Conferido contra o blob commitado (`git show 287c29b -- .../screen_state.hpp`,
não a working tree): o comentário "EXCLUSIVIDADE DO UILAYER" em
`screen_state.hpp` trocou a referência morta
`gus/app/src/screens/battle_preview.cpp:1043-1059` pelas âncoras `commit 89fb380`
+ ID `BATTLE-ESC-PAUSE-ACTOR-LIST`, com meia-linha explicando o porquê
("arquivo:linha se move e o SHA não"). Confirmei que o commit `89fb380` existe
de fato e tem a mensagem
`fix(BATTLE-ESC-PAUSE-ACTOR-LIST): revert menu de pausa na arena de batalha (crash real ao vivo)`,
batendo exatamente com o texto inserido. **Zero mudança de código**, a troca é
puramente textual; a semântica da garantia estrutural descrita no comentário
(a que `run_screen_state()` implementa) não foi alterada. Edição do agente
anterior **verificada e correta**.

## 7. Veredito final

- **Nenhum sítio INSEGURO encontrado.** Todos os 6 sítios-dono (a+b) **HERDAM**
  a garantia estrutural de `run_screen_state()`/`ScreenState`: `.emplace()`
  só em `enter()`; `.reset()` no `exit()` em todas as 6, **mais um `reset()`
  de limpeza no ramo de falha do próprio `enter()` em `BattleScreen`
  (`battle_preview.cpp:444`, quando `ui_->ok()==false`, libera na hora e
  segue sem UI, não fica pendurado até o `exit()`; ver correção na seção 3)**;
  e cada classe tem exatamente um ponto de instanciação no repositório
  inteiro, sempre seguido imediatamente por `run_screen_state()`.
- Os 11 sítios de construção fora das classes-dono são **PROCESS-SEPARADO**
  (9, ferramentas standalone) ou **MANUAL-SEGURO** (2, helpers de teste
  RAII-escopados, comprovados sem overlap).
- A hipótese dos 4 `_rml` (+ `battle_input`) **se confirma por medição**: 100%
  das ocorrências nesses 8 arquivos são (d) comentário/string; nenhum
  constrói, guarda ou recebe por referência um `UiLayer`.
- A edição do commit `287c29b` foi **verificada e está correta**: troca de
  referência morta por âncoras estáveis, zero mudança de semântica.

Nada foi consertado (não era necessário) e nenhum código foi tocado ou
buildado nesta fatia.

## 8. Estado do repositório ao final

```
$ git rev-parse HEAD
287c29bae380e3aafa17ae1883e09a2039ecea7d
$ git status --porcelain
?? GusEngine/app/tools/appmode_spike/
```

Idêntico ao estado inicial; nenhuma mudança introduzida por esta auditoria
além deste próprio documento.
