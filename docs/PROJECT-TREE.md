# Mapa da árvore do repositório

Referência canônica e exaustiva de "onde fica X" no GusWorld. Se você chegou aqui
sem saber por onde começar, é o lugar certo. **Não é tutorial nem explicação de
arquitetura** (isso vive em [`docs/tech/pivot/engine-design.md`](tech/pivot/engine-design.md)
e nos [ADRs](tech/adr/)), é só o mapa das pastas e o papel de cada uma.

> Tipo Diátaxis: Reference. Audience: qualquer pessoa (dev interno, colaborador
> futuro, o próprio criador em 6 meses) que precisa localizar um arquivo/pasta.
> Última varredura da árvore real: 2026-07-23. Owner: technical-writer (a pedido
> do criador). Gerado por varredura de disco (`find`/`git ls-files`), não por
> memória, se a árvore mudar, este doc pode ficar desatualizado; reconferir antes
> de confiar cegamente numa pasta antiga.

## Como ler este doc

- `[tracked]` = versionado no git (existe no clone de qualquer um).
- `[gitignored]` = só existe no disco local do criador (+ backup IDrive); o
  clone limpo do repo NÃO traz o conteúdo, só a pasta pode nem existir.
- Pastas de **arte de personagem** (`resources/sprites/`) e de **props/lore**
  (`resources/images/`) têm mapas dedicados e mais detalhados, ver
  [`docs/art/sprites-inventory.md`](art/sprites-inventory.md) e
  [`docs/art/props-inventory.md`](art/props-inventory.md). Aqui só a visão de
  1000 pés, para não duplicar.

## Regra de ouro dos caminhos de asset

Fonte única da verdade em código: `GusEngine/core/include/gus/core/asset_paths.hpp`
(comentado, com as razões de cada escolha). Resumo:

| Tipo de asset | Vive em | Motivo |
|---|---|---|
| Sprites de personagem (direcionais, animações) | `resources/sprites/<slug>/` | asset de jogo, gerado via PixelLab, gitignored (só disco/IDrive) |
| Props, lore, cartas, símbolos, retratos formais | `resources/images/` | idem, taxonomia espelha `resources/prompts_images/` |
| Áudio (SFX + música) | `assets/` (raiz do repo, **não** `resources/`) | kit provisório pequeno, **tracked** no git (ver nota abaixo) |
| Fontes + mapas compilados (`.gmap`) | `GusEngine/assets/` | asset de **engine**, versionado junto do código que o consome |
| Diálogos (`.dlg.txt`) | `resources/dialogues/` | conteúdo de jogo editável por narrative-designer/writer |
| Traduções i18n (`pt_br.md`/`en_intl.md`) | `resources/translations/` | catálogo consumido pelo translator C++ próprio |
| VFX pré-renderizado (frames de boot) | `resources/vfx/` | curadoria caso a caso, alguns tracked (boot_pixel), resto gitignored |

**Exceção anotada:** `resources/images/vance_dragon_glyph.png` fica na **raiz**
de `images/` (não numa subpasta) porque é o único arquivo dali que o runtime
lê de verdade (`kVanceDragonGlyphFile`, glifo do brasão no cockpit), por isso
é o único **tracked** dentro de `resources/images/`, o resto é gitignored.

**Distinção crítica:** `resources/sprites/personagens_inspirados/`: essa
subpasta é **só** para personagens baseados em **PESSOAS REAIS** (Gus, Yakov,
Pyotor Vance, Brunus Vetorial ← Bruno Vettore). NPCs comuns e figuras lendárias
do jogo **não** entram aqui, eles ficam direto na raiz de `resources/sprites/`
(ex.: `caua_volt/`, `seu_bertoldo_caim/`). Reorganização do criador em
2026-06-25; ver comentário no próprio header.

## Árvore da raiz

```
gusworld/
├── CLAUDE.md            # instruções de projeto pro Claude Code (stack vigente, pillars, regras)
├── TODO.md              # [tracked] backlog canônico vivo (skill tab_pendencias), board M0-M9 + INBOX
├── TODO_ARCHIVE.md       # [tracked] itens ✅ concluídos recortados do TODO.md (corte 2026-07-23, TODO.md > 400KB)
├── ROADMAP.md            # [tracked] marcos M0-M9+ (board de alto nível, complementa TODO.md)
├── CHARS.md              # [tracked] inventário canônico de TODOS personagens nomeados
├── PLACES.md             # [tracked] inventário canônico de TODOS lugares nomeados
├── CONTRACT.md           # [tracked] contrato de qualidade/processo do projeto
├── TESTES.md             # [tracked] plano de testes não-unitários
├── AUDITORIAS.md         # [tracked] rastreio de auditorias aplicáveis ao stack
├── TEXTREVIEW.md         # [tracked] protocolo de revisão literária/canônica (copydesk do deep-lore)
├── ORIENTACOES.md        # [tracked] dossiê de orientações técnicas pós-auditoria (Caetano/CTO, AC-* refs)
├── README.md             # [tracked] build/run atual (CMake + Ninja + CMakePresets)
├── CHANGELOG.md          # [tracked] changelog de release (humano, agrupado por tema)
├── sinopse.md            # [tracked] base canônica IMUTÁVEL (worldbuilding + protagonista)
├── ACKNOWLEDGMENTS.md    # [tracked] agradecimento nomeado ao stack open-source usado
├── AI-DISCLOSURE.md      # [tracked] divulgação de onde IA foi usada no projeto, por papel
├── ASSETS-LICENSE.md     # [tracked] licença dos assets (arte/música/lore), distinta da licença de código
├── THIRD-PARTY-LICENSES.md  # [tracked] atribuição de componentes de terceiros empacotados
├── LICENSE               # [tracked] GPLv3 (código)
├── resumo.pdf             # [gitignored] concept art/refs solto, local
├── .bigtech-porte         # [tracked] marcador de porte/ativação da constelação C-level (Cósimo)
├── .editorconfig          # [tracked] estilo de arquivo (indentação etc.)
├── .codeberg/FUNDING.yml  # [tracked] config de funding do Codeberg
├── .forgejo/workflows/ci.yml     # [tracked] CI canônico (Forgejo Actions, local-first)
├── .github/workflows/windows.yml # [tracked] espelho CI GitHub Actions (Windows/MSVC real)
├── .claude/               # [tracked, exceto .claude/local/] hooks (anti-em-dash, TDD guard), settings
├── docs/                  # ver seção "docs/" abaixo
├── GusEngine/              # ver seção "GusEngine/" abaixo, engine + jogo em C++20/SDL3
├── resources/             # ver seção "resources/" abaixo, dados vivos do jogo (arte, lore, i18n, save-adjacent)
├── assets/                # áudio VIVO do jogo (kit provisório SFX+música), ver tabela de assets acima
└── tools/                 # scripts de dev (ver seção "tools/" abaixo)
```

## `docs/`

Documentação de projeto (design, narrativa, técnico, auditoria). Não confundir
com `GusEngine/assets/` (assets de engine) nem `resources/` (dados de jogo).

```
docs/
├── design/               # GDD, pillars, mecânicas, mockups HTML, roster de cartas
│   ├── pillars.md         # 5 pillars criativos canônicos, imutáveis
│   ├── gdd.md              # Game Design Document (v0.2, pré-produção)
│   ├── brainstorm-backlog.md   # seeds de ideia, consultar ANTES de propor lore novo
│   ├── mecanicas/          # specs de sistema: combat, economia, deck/mão, cartas hardware, stamina, etc.
│   ├── mockups/            # protótipos HTML navegáveis (não PNG solto), UI kit, refs de estilo
│   │   ├── refs-*/          # [gitignored, exceto os .html] imagens de referência de pesquisa visual por domínio
│   │   ├── cockpit_barras_mana_ap/, menu_capturas/  # [tracked] screenshots headless de verificação de UI
│   ├── ui-kit/              # componentes RCSS/HTML do design system (tokens, card, slot, chip, button)
│   ├── roster-analogos/     # os 21 "cientistas-cartas" especiais (Faraday, Tesla, Newton, Gödel...) + metodologia
│   ├── levels/               # blockout de níveis (distritos inferiores etc.)
│   ├── narrativa/            # árvores de diálogo específicas de design (não confundir com docs/narrative/)
│   ├── producao/              # protocolo de spike de arte, plano de CI de build, plano do vertical slice
│   └── research/               # pesquisa de referência de game design (juice, refs externas)
├── narrative/            # TODO o deep-lore, personagens, facções, timeline, ambientes, língua conlang
│   ├── lore-bible.md, arco-principal.md, factions.md, timeline.md, in-world-docs.md, tradicoes-cultura.md, comic-reliefs.md   # REVISÃO 1, canônicos
│   ├── characters/          # fichas dos 6 companions + Gus + antagonistas (narrativo, distinto de docs/art/characters/)
│   ├── deep/                 # deep-lore paralelo por era/facção/setting/personagem (~365k palavras); ver deep/_INDEX.md
│   ├── environments/          # 8 settings (cidade cyber-gótica, Selve Sombria, catedrais, dutos, etc.)
│   ├── diary/                  # sistema de diário in-game (docs descobríveis, bestiário, knowledge gates)
│   ├── lingua/                  # conlang Sylvarin (fonologia, léxico, gramática)
│   ├── vozes-party.md, guia-dialogos.md, guia-narrativa-fluida.md   # guias de voz/estilo de escrita
│   ├── INCOHERENCES.md, foreshadowing.md   # rastreio de furos e plantios narrativos
│   └── bibliografia-rag.md      # índice do corpus RAG (livros de referência)
├── art/
│   ├── style-guide.md          # guia de estilo visual (SD 1:1:1, cel-shading)
│   ├── sprites-inventory.md     # MAPA CANÔNICO de resources/sprites/ (não duplicar aqui)
│   ├── props-inventory.md        # MAPA CANÔNICO de resources/images/ (não duplicar aqui)
│   ├── vfx-combate-familias.md    # famílias de VFX de combate por elemento/carta
│   ├── characters/gus.md           # spec de produção de arte do Gus (distinto do narrativo em narrative/characters/gus.md)
│   └── refs/                        # [PENDÊNCIA DE HIGIENE: vazia, ver seção "Órfãos"]
├── tech/
│   ├── adr/                     # Architecture Decision Records, ADR-001 a ADR-020, numerados e imutáveis após aceitos
│   ├── pivot/                    # documentos do pivot de engine (engine-design.md é a fonte viva pós-Godot)
│   ├── glintfx-requests/          # pedidos formais de feature pro glintfx (fronteira glintfx ⇄ GusWorld)
│   ├── glintfx-boundary.md         # a régua genérico→glintfx / específico→GusWorld
│   ├── architecture.md, engine-modules.md, build.md   # SUPERADOS (era Godot/C#), mantidos por registro histórico
│   ├── ai-assets-provenance.md      # proveniência de assets gerados por IA
│   ├── libs-vendoring-candidatas.md  # candidatas a lib vendorizada, antes de decidir FetchContent
│   └── runtime-hardening-plan.md      # plano de hardening de runtime (ASan/segurança)
├── auditoria/            # relatórios de auditoria formal (AUDIT-*, um dir por rodada, com índice mestre)
├── book/                 # material da "bíblia" impressa/e-book (2 volumes: capa, prefácio, glossário, apêndices)
├── qa/                   # plano de playtest do vertical slice
├── devlog/               # posts de devlog técnico (ex.: post-mortem de bug real, UAF em deck callback)
├── _secret/              # [gitignored inteiro] manual privado dos easter eggs, NUNCA publicar
├── audio/                # [PENDÊNCIA DE HIGIENE: vazia]
└── production/           # [PENDÊNCIA DE HIGIENE: vazia]
```

`docs/_secret/` merece nota: está listado no `.gitignore` (`docs/_secret/`),
ou seja **não é tracked**, existe só localmente, apesar de aparecer na
árvore do disco. Contém o mapa dos easter eggs Fibonacci/maçonaria; publicá-lo
mata o segredo pervasivo do jogo.

## `GusEngine/`

Engine + jogo em C++20/SDL3, arquitetura de 4 camadas (regra: cada camada só
depende das de baixo; gate de CI proíbe violação). Detalhe completo em
[`docs/tech/pivot/engine-design.md`](tech/pivot/engine-design.md).

```
GusEngine/
├── CMakeLists.txt, CMakePresets.json   # build raiz (linux-release / windows-release)
├── cmake/toolchains/mingw-w64.cmake     # toolchain de cross-compile pra Windows
├── compile_commands.json                 # [gerado, não versionar] symlink pro clangd
├── patches/                               # patches aplicados a libs de terceiro (ex.: rmlui teardown UB)
├── core/                 # CAMADA 1, POCO genérico, zero I/O, zero libs externas
│   ├── include/gus/core/    # headers públicos: anim/, crypto/, player/, spatial/, time/, math_util.hpp, version.hpp
│   │   └── asset_paths.hpp   # FONTE ÚNICA dos sub-caminhos de asset (ver tabela acima)
│   └── src/                  # implementação espelhando o include (anim/, crypto/, player/, spatial/, time/)
├── domain/               # CAMADA 2, POCO das regras do jogo (depende só de core/)
│   ├── include/gus/domain/    # cards/, combat/, deck/, dialogue/, hardware/, i18n/, infection/, input/, map/, progression/, save/, settings/, templates/
│   ├── src/                    # implementação espelhando o include
│   └── tests/                   # suíte Catch2 do domínio (a maior: combate, save, cartas, techMagic, i18n, map...)
├── platform/             # CAMADA 3, ÚNICA fronteira que toca libs externas (SDL3/GL/RmlUi/miniaudio)
│   ├── include/gus/platform/   # assets/, audio/, fs/, input/, render2d/, rmlui/, platform_info.hpp
│   ├── rmlui/                    # loader GL3 + header de include do RmlUi (vendorizado à mão, backend embed do glintfx)
│   ├── src/                       # implementação: assets/, audio/, fs/, input/, render2d/
│   └── tests/
├── app/                  # CAMADA 4, telas do jogo, gameplay, ferramentas internas
│   ├── include/gus/app/, src/    # maestro (orquestrador principal), screens/ (todas as telas: título, batalha, cidade, menus, diálogo), dialogue/, i18n/
│   ├── tests/                     # suíte Catch2 de UI/tela (battle_*, save_load_menu_*, npc_dialogue_*, title_menu_*, sysmenu_*)
│   ├── tools/                      # probes/spikes standalone (screenshot headless, diagnóstico, appmode_spike/ = spike do App mode do glintfx)
│   └── main.cpp
├── tests/                # suíte Catch2 de nível ENGINE (crypto AEAD/HMAC/SHA, física/colisão, timing, boot pixel)
├── third_party/          # libs vendorizadas manualmente (baixo volume): monocypher/ (AEAD/HMAC), stb/ (image/truetype/rect_pack)
├── assets/               # asset de ENGINE (não de jogo!), versionado junto do código
│   ├── fonts/              # PixelOperatorMono (CC0), única fonte da UI, sem fallback
│   └── maps/                # source/ (CSV editável) + compiled/ (.gmap binário selado HMAC+UUID) + README do pipeline
└── build/                # [gitignored] outputs de build local, não versionado
```

**Nota de terceiros faltantes:** o `.gitignore` só ignora `/build/` na raiz e
`GusEngine/compile_commands.json`; as demais dependências pesadas (SDL3,
RmlUi, glintfx, Catch2) **não vivem em `third_party/`**, vêm via CMake
`FetchContent` (baixadas e fixadas em build, pin no `CMakeLists.txt`). Só as
duas libs pequenas vendorizadas à mão (`monocypher`, `stb`) moram fisicamente
em `GusEngine/third_party/`.

## `resources/`

Dados de jogo versionados fora do código-fonte (arte, lore visual, i18n,
diálogo, mapas-fonte, referências 3D).

```
resources/
├── sprites/              # [maioria gitignored] sprites de personagem, ver docs/art/sprites-inventory.md
│   ├── personagens_inspirados/  # SÓ pessoas reais (gus, yakov, pyotor_vance, brunus_vetorial), ver nota acima
│   ├── icons-m5/                 # [tracked, exceção do .gitignore] ícones pixel-art pequenos: retratos, status, intent, app_icon
│   └── <slug>/                    # demais personagens (companions, NPCs, inimigos) direto na raiz
├── images/                # [maioria gitignored] props/lore/cartas, ver docs/art/props-inventory.md
│   └── vance_dragon_glyph.png     # [tracked] EXCEÇÃO, único arquivo lido pelo runtime (glifo do brasão)
├── translations/          # [tracked] catálogos i18n VIVOS (pt_br.md / en_intl.md), fonte do translator C++
├── dialogues/              # [tracked] grafos de diálogo VIVOS (.dlg.txt), consumidos pelo runtime POCO de diálogo
├── prompts_images/         # [tracked] prompts de geração de imagem (nano banana / PixelLab), prosa auto-contida
│   ├── feitos/               # os 170 prompts já executados (curados)
│   └── <categoria>/           # [PENDÊNCIA DE HIGIENE: várias vazias, estado "drenado pra feitos/", ver seção Órfãos]
├── glb/                    # [gitignored inteiro] modelos 3D (~50-60MB cada), arte conceitual, fora de escopo 2D atual
├── pers_3d/                 # [tracked] modelos 3D pequenos (Gus.glb, Gus_movimento.glb, Yakov.glb) para os livros futuros
├── vfx/                      # frames de VFX pré-renderizado; só boot_pixel/ é tracked (exceção do .gitignore), resto gitignored
└── livros/                    # [gitignored inteiro, ~943MB] corpus RAG (bibliografia de referência), indexado via `rag`
```

`glb/` vs `pers_3d/`: ambos guardam `.glb`, mas `glb/` é gitignored (peso alto,
concept art solta) enquanto `pers_3d/` é tracked (peso baixo, arte conceitual
3D destinada aos livros/book, ver `docs/book/`). Não confundir os dois ao
procurar um modelo.

## `assets/` (raiz, não confundir com `GusEngine/assets/`)

Áudio VIVO do jogo (SFX + música), carregado via env `GUSWORLD_SFX_DIR` /
`GUSWORLD_MUSIC_DIR` (fallback macro de compilação / CWD).

```
assets/
├── AUDIO_KIT_PROVISORIO.md   # nota do kit provisório CC0/IA, substituição futura por audio-designer-composer
├── music/                      # Arena_GusWorld.mp3 (batalha, Suno), cidade_tema_provisorio.mp3
└── sfx/                          # hits, blips de menu (hover/click/blocked), CONFIRM, todos WAV PCM16
```

Diferente de `resources/sprites/` e `resources/images/`, **este `assets/` é
totalmente `[tracked]`**, o kit é pequeno (12M) e provisório, então o líder
optou por versionar em vez de manter só em disco/IDrive.

## `tools/`

Scripts de dev/CI, não fazem parte do binário do jogo.

```
tools/
├── check.sh                  # gate local (build + smoke + arch-gate + i18n-parity + suite), o "CI da mesa"
├── asan_gate.sh                # gate de build com AddressSanitizer
├── crash_journal_check.sh       # checagem de journal de crash (systemd/coredump)
├── linuxci_container.sh          # runner containerizado do CI Linux (Forgejo Actions local-first)
├── winbuild_container.sh          # runner containerizado do CI Windows (cross MinGW ou validação)
├── i18n_parity.py                  # valida paridade de chaves entre pt_br.md e en_intl.md
├── gen_menu_ui_sfx.py               # síntese determinística (Pillow/wave, sem lib externa) dos blips de menu
└── lsan.supp                         # supressões do LeakSanitizer
```

## `.claude/`

```
.claude/
├── settings.json, settings.local.json   # config de sessão Claude Code (settings.local.json é local, não versionado no espírito, mas presente)
├── hooks/
│   ├── check_on_change.py    # hook de checagem (provável gate de qualidade em edição)
│   └── tdd_guard_cpp.py        # guard de TDD para C++ (red/green/refactor)
└── worktrees/                 # [vazio no momento da varredura] worktrees de agent isolado, quando usados
```

## Órfãos e pendências de higiene (registradas, não resolvidas por este doc)

Achados na varredura de 2026-07-23. Não foram removidos aqui, item de faxina
futura (candidato a entrar no `TODO.md` como item de higiene, se o líder
concordar):

- **`resources/sprites/prospero_vance/`**: pasta de personagem existe, mas
  está **vazia** (sem nenhum sprite dentro). Órfão: ou falta gerar o sprite,
  ou a pasta sobrou de um planejamento que não avançou.
- **`docs/audio/`, `docs/production/`, `docs/art/refs/`**: três pastas de doc
  totalmente vazias (só `.` e `..`). Provavelmente scaffolding original do
  projeto (fase 1) que nunca recebeu conteúdo.
- **`resources/prompts_images/<categoria>/` vazias**: `architecture_details/`,
  `consumables/`, `fauna/`, `flora/`, `hardware_triad/`, `logos_glyphs/`,
  `props/`, `runic_cards/`, `traditions/`, `ui_frames/`, `vehicles/` estão
  todas vazias. **Não é bug**: os prompts dessas categorias foram drenados
  para `resources/prompts_images/feitos/` (169-170 arquivos lá) após uso;
  esse é o estado normal de "prompt já consumido". As categorias vazias que
  sobram são só o esqueleto de onde novos prompts da mesma categoria
  entrariam, se necessário.
- **`.claude/worktrees/`**: vazia no momento da varredura (achado novo, não
  listado no pedido original). Normal se nenhum agent isolado em worktree
  está ativo agora; não é lixo per se, é infraestrutura latente do harness.

### Inconsistência achada vs. a descrição da tarefa

O pedido original presumia que **todo** o áudio em `assets/` (raiz) fosse
gitignored, igual sprites/images. **Não é o caso**: conferido via
`git ls-files assets/` e o `.gitignore`, e `assets/` **não tem regra de
ignore nenhuma**; o kit provisório inteiro (`AUDIO_KIT_PROVISORIO.md`,
`music/*.mp3`, `sfx/*.wav`) está `[tracked]` no git. Registrei o estado real
(tracked) na tabela de assets e na seção `assets/` acima. Se a intenção for
mudar isso (gitignorar áudio pesado no futuro, quando o kit provisório for
substituído pela versão final do audio-designer-composer), é decisão do
líder, não algo que este doc deveria silenciosamente corrigir.
