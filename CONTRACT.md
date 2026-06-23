# CONTRACT.md, disciplinas técnicas canônicas

> **Status:** Canônico. Autoridade sobre processo de desenvolvimento, commits, branching, qualidade.
>
> **Escopo:** vincula todo código, asset, documentação, e atividade técnica do projeto GusWorld G1 (solo indie, C++20 + SDL3, engine própria, vertical slice até release v1.0.0).
>
> **Última revisão:** 2026-06-23 (higienização de stack pós-ADR-008: Godot/C# para C++20 + SDL3). Revisão de conteúdo anterior 2026-05-19. Mudanças requerem ADR explícito.

---

## §1. Princípios + RFC 2119

Este documento usa as palavras-chave **MUST**, **MUST NOT**, **REQUIRED**, **SHALL**, **SHALL NOT**, **SHOULD**, **SHOULD NOT**, **RECOMMENDED**, **MAY**, **OPTIONAL** conforme definidas em [RFC 2119](https://www.rfc-editor.org/rfc/rfc2119) e [RFC 8174](https://www.rfc-editor.org/rfc/rfc8174).

**MUST** = obrigatório, viola = inválido / rejeitado / revertido.
**SHOULD** = recomendado, desvio precisa justificativa explícita (comentário, commit body, ADR).
**MAY** = permitido, decisão livre.

### Princípios fundamentais (Pillars técnicos)

1. **Single-dev sustainability.** Disciplina deve servir velocity solo, não simular team enterprise. Cooling-off, branching, audit servem pra capturar erros próprios, NÃO pra burocracia.
2. **Two-way doors sempre que possível.** Decisões reversíveis em < 1 dia preferíveis. One-way doors exigem ADR.
3. **Vertical slice first.** Toda mudança pré-VS responde "isto contribui pra VS coeso 5-10min jogável?" Se não, adiar.
4. **Validation real > documentation.** Prototype em código real > spec exaustiva.
5. **Anti over-engineering proativo.** Solo indie != AAA studio. YAGNI, KISS, DRY (nessa ordem de prioridade).
6. **Lore como serviço.** Deep-lore informa código quando útil. Não bloqueia. Não dirige decisões técnicas.

---

## §2. Conventional Commits + scopes game-dev

Todo commit MUST seguir [Conventional Commits 1.0](https://www.conventionalcommits.org/) com scopes game-dev específicos.

**Nota linguagem (pós-ADR-008 2026-06-23):** C++20 é a linguagem canon (engine própria; AOT por natureza). Naming snake_case pra arquivos, `PascalCase` pra tipos/classes, `snake_case` pra funções/variáveis, `m_` ou trailing `_` pra membros privados conforme `.clang-format`/`.clang-tidy`. A camada de plataforma usa SDL3; `core/`+`domain/` são POCO C++ puro (zero framework). (O stack anterior Godot 4 + C# .NET 8 AOT foi superado pelo ADR-002 e depois pelo ADR-008.)

### Formato

```
<type>(<scope>): <subject>

[body opcional]

[footer opcional]
```

### Types canônicos

| Type | Uso | Exemplo |
|---|---|---|
| `feat` | Nova feature jogável ou módulo | `feat(platform): render2d batcher de sprites sobre SDL_Renderer` |
| `fix` | Correção de bug | `fix(combat): turn order quando initiative empata` |
| `refactor` | Restruturação sem mudar comportamento | `refactor(save): extrair migrators pra módulo próprio` |
| `perf` | Otimização medida | `perf(render): batch reduz 40% draw calls` |
| `style` | Formatação, sem mudança lógica | `style(engine): aplica clang-format em todos os módulos` |
| `test` | Adiciona/corrige testes | `test(save): cobre migrator 1 para 2 com 5 casos` |
| `docs` | Documentação apenas | `docs(tech): atualiza engine-design.md §4.5` |
| `chore` | Manutenção, build, deps | `chore(deps): pin SDL3 3.2.x no FetchContent` |
| `build` | Sistema de build | `build(cmake): adiciona preset linux-release ao CMakePresets` |
| `ci` | Pipeline CI | `ci(forgejo): cacheia build do SDL3 entre runs` |

### Scopes game-dev (MUST usar)

| Scope | Domínio | Path típico |
|---|---|---|
| `engine` | Módulos reutilizáveis (lógica pura) | `GusEngine/core/*`, `GusEngine/domain/*` |
| `platform` | Fronteira SDL3 (janela, render2d, input, audio, fs) | `GusEngine/platform/*` |
| `game` | Código game-specific (casca da aplicação) | `GusEngine/app/*` |
| `art` | Arte 2D, sprites, atlas, shaders | `/assets/sprites/*`, `/assets/textures/*`, shaders |
| `narrative` | Lore canon, deep-lore, dialogue, in-world docs | `/docs/narrative/*` |
| `design` | GDD, pillars, balanceamento | `/docs/design/*` |
| `tech` | Arquitetura, ADRs, build, infra | `/docs/tech/*`, `/.forgejo/*` |
| `audio` | SFX, música, mix | `/assets/sfx/*`, `/assets/music/*` |
| `i18n` | Localização, strings, fontes | loader `.md` próprio + CSV de strings |
| `qa` | Testes, playtest plans, bugs | `GusEngine/tests/*`, `docs/qa/*` |
| `release` | Versionamento, CHANGELOG, packaging | `VERSION`, `CHANGELOG.md`, build outputs |

### Regras

- Subject **MUST** ser ≤ 70 caracteres.
- Subject **MUST** estar em imperativo presente ("adiciona", "corrige", NÃO "adicionei", "corrigi").
- Body **SHOULD** explicar *por quê*, não *o quê* (o quê está no diff).
- Body **MUST** mencionar `BREAKING CHANGE:` no footer se quebrar API ou save format.
- Body **SHOULD** referenciar TODO.md task (`F2-E.1`, `F1-DL.4`) quando aplicável.

### Anti-patterns proibidos

- ❌ `wip`, `fix`, `update`, `more`, `stuff`, `final` como subject standalone.
- ❌ Type `feat` em commit que é só refactor.
- ❌ Commit com 10+ arquivos sem ser refactor amplo declarado.
- ❌ Scope inventado (`feat(misc):`, `feat(general):`).

---

## §3. Branching + cooling-off

Strategy: **`main` + `feat/*` PRs solo** com cooling-off seletivo.

### Branches canônicos

| Branch | Uso | Política |
|---|---|---|
| `main` | Estado canonizado, sempre verde em CI | Direct push permitido pra commits pequenos (docs, chore, fix triviais ≤ 2 arquivos). Feat e refactor **MUST** vir de `feat/*` |
| `feat/<scope-curto>` | Feature em desenvolvimento | Criada de `main`, merge de volta com squash quando done |
| `fix/<id>` | Bugfix isolado urgente | Hotpath. Merge direto pós-validação |
| `chore/<id>` | Manutenção, deps, ci | Direct push em geral OK |

### Cooling-off (proteção contra impulso solo)

**Feature grande** = qualquer um dos critérios:
- ≥ 5 arquivos modificados
- Refactor cross-module
- One-way door arquitetural (decisão ADR)
- Toque em `GusEngine/domain/save/` ou em qualquer migrator

**Política:**
- Feature grande **MUST** dormir em `feat/*` por **mínimo 12 horas** antes do merge em `main`.
- Bugfix urgente, docs, chore: **MAY** ir direto sem cooling-off.
- Override emergencial (deploy quebrado, dados em risco): permitido, mas commit body **MUST** explicar override + criar follow-up task em TODO.md pra audit retroativo.

### Merge strategy

- Feature para main: **squash merge** (preserva histórico limpo).
- Bugfix para main: **fast-forward** quando único commit.
- Sem rebase de commits já pushed pra `origin/main`.

### Anti-patterns proibidos

- ❌ `--force` push em `main` (use revert commits).
- ❌ `--no-verify` (skipa hooks; reverter quando hook quebrado, NÃO bypassar).
- ❌ Amend de commit já pushed.
- ❌ Branch viva > 14 dias sem rebase ou merge (stale branch).

---

## §4. Definition of Done (DoD) por tipo

DoD MUST ser satisfeito antes de marcar task ✅ no TODO.md.

### DoD: feat(engine), feat(platform) ou feat(game) [pós-ADR-008]

- [ ] Código C++20 passa `clang-format` (sem diffs).
- [ ] Código C++20 passa `clang-tidy` (zero warnings, ou justificados em commit body / `// NOLINT(...)` com motivo).
- [ ] Teste Catch2 demonstra a feature; lógica pura roda headless (`core/`+`domain/` sem janela).
- [ ] CI verde (`cmake --preset` + `cmake --build` + `ctest`). Smoke da plataforma headless com `SDL_VIDEODRIVER=dummy`.
- [ ] Se módulo `core/` ou `domain/`: API pública documentada (comentário Doxygen-style nos headers públicos).
- [ ] Se toca `domain/save/`: migrator + teste Catch2 do migrator com input/output esperado.
- [ ] Strings user-facing via o loader de i18n próprio (chave de tradução, NUNCA hardcoded).
- [ ] Invariante das 4 camadas: `core/`+`domain/` NÃO incluem `<SDL...>` nem qualquer header de plataforma (auditável por grep no CI).
- [ ] Commit message segue §2.
- [ ] Cooling-off respeitado se feature grande (§3).

### DoD: feat(art)

- [ ] Asset commitado em formato source (`.blend`, `.kra`, `.aseprite`) + export final usado pela engine.
- [ ] Naming convention seguida (`<categoria>_<nome>_<variante>.ext`, snake_case).
- [ ] Style guide §SD 1:1:1 respeitado pra characters.
- [ ] Polycount budget respeitado (chars ≤ 5k tris, props ≤ 1k tris pra G1).
- [ ] Atlas gradient + vertex color quando aplicável (sem PBR, decisão fechada).

### DoD: feat(narrative)

- [ ] Não contradiz canon prévio (audit cross-canon manual via `git grep` ou narrative-designer).
- [ ] Zero em-dash horizontal em prosa autoral, headers, metadata (regra revisada 2026-05-16).
- [ ] Sem palavrão (censura canônica: porra para poxa, cacete para caçamba, caralho para caramba).
- [ ] Easter eggs Fibonacci ~10-20% + maçonaria ~10-15% quando aplicável (deep-lore).
- [ ] CHARS.md atualizado se novo NPC nomeado.
- [ ] PLACES.md atualizado se novo lugar nomeado.
- [ ] Cross-refs imutáveis preservadas (pillars + lore-bible + timeline).

### DoD: fix

- [ ] Repro mínimo documentado no commit body.
- [ ] Teste de regressão adicionado (se feasível) OU justificativa explícita de por que não.
- [ ] Root cause identificado (não só sintoma).

### DoD: docs

- [ ] Sintaxe Markdown válida (`mdformat` ou similar opcional).
- [ ] Wikilinks `[[X]]` resolvem se canônicos.
- [ ] Cross-refs absolutas (paths completos a partir da raiz do projeto).
- [ ] Tabela de conteúdo se doc > 200 linhas.

---

## §5. Performance budget

Target hardware canon: **GTX 1050 + 4GB VRAM + 8GB RAM dual-core 3.0GHz+** (cobre Steam Deck e laptops gaming 2017+).

### Métricas obrigatórias

| Métrica | Alvo | Trigger refac |
|---|---|---|
| Frame rate sustentado | 60fps @ 1080p | < 55fps por > 3s em cena típica |
| Frame time pico | < 16.6ms | > 25ms p99 em frame budget |
| Memory RAM | < 1.5GB residente | > 2GB em runtime de cena típica |
| Memory VRAM | < 1GB | > 2GB em cena pico |
| Load time scene | < 3s | > 5s sem progress UI |
| Save/load roundtrip | < 1s | > 2s |
| Cold start (splash até menu) | < 5s | > 8s |
| Turn resolve combat | < 100ms | > 250ms |

### Disciplinas

- Profile **SHOULD** rodar em milestone (M.1, M.2) antes de canonizar como done.
- Otimização **MUST** ser data-driven (profiler real, NÃO intuição).
- Premature optimization é anti-pattern: **SHOULD NOT** otimizar antes de medir.
- Otimização de baixo nível (SIMD, layout de dados, custom allocator) **MAY** ser introduzida apenas em ponto quente medido por profiler real.

---

## §6. Acessibilidade gates D1 (ship blockers v1.0.0)

Os seguintes gates **MUST** estar funcionais em ship de v1.0.0. Cortes em VS são aceitáveis; em v1.0.0 não.

### Gate 1: Controles remappáveis

- Keyboard + gamepad totalmente remappáveis pelo player (menu in-game).
- Schemes default sensatos (WASD + setas + gamepad standard).
- Persistência via save (`GusEngine/domain/input/`).
- Reset to defaults sempre disponível.

### Gate 2: Contraste WCAG 2.2 AA

- Texto corpo: contraste ≥ 4.5:1 contra fundo.
- Texto large (≥ 18pt ou 14pt bold): ≥ 3:1.
- Componentes UI interativos (botões, sliders): ≥ 3:1 contra adjacente.
- Validado por audit visual + ferramenta (axe ou colour-contrast checker).
- Aplicado em todas cenas de UI (menu, HUD, dialogue, save/load, inventory, settings).

### Gate 3: Reduce motion

- Toggle único em settings: `Reduzir movimento`.
- Quando ativo: screen shake **MUST** ser eliminado, camera bob reduzido a 0, motion blur OFF, parallax background reduzido a 0, transições de tela usam fade simples (sem deslocamento horizontal/vertical).
- Default OFF (motion ON), mas posição visível no menu de settings.
- Persistência via save.

### Gate 4: Subtitles + closed captions

- Dialogue text 100% subtitled (texto sempre acompanha voz).
- SFX nomeados em closed captions opcionais (`[passos atrás]`, `[porta range]`, `[música ascende]`).
- Toggle separado `Subtitles` + `Closed captions`.
- Tamanho de fonte ajustável (3 níveis: pequeno, médio, grande).
- Background opaco opcional pra subtitles (toggle).
- Locale-aware (pt-br dev + en-intl pós-v1.0.0).

### Disciplinas paralelas (SHOULD em v1.0.0, MUST em v1.x)

- Colorblind modes (protanopia, deuteranopia, tritanopia) com palette ajustável.
- Dyslexia-friendly font opcional (OpenDyslexic ou similar).
- Aim assist em combate turn-based (toggle assistência seleção de target).
- Pause on focus loss (window não-focada pausa o jogo).

---

## §7. Save format compat (forward-only)

Save format canon: **binário próprio versionado `save_version: N` (schema v4 atual, ADR-006/ADR-007) com criptografia própria (HMAC-SHA256) e migrators forward-only puros desde D1**.

### Princípios

- `save_version` é parte do header desde D1 da implementação `GusEngine/domain/save/`.
- Toda mudança de schema **MUST** bumpar `save_version`.
- Cada bump **MUST** ter um migrator dedicado em `GusEngine/domain/save/` (ex. `migrate_v3_to_v4`), implementando o contrato de migrator do domínio.
- Migrators **MUST** ser funções puras (input save N, output save N+1, sem efeito colateral, sem I/O).
- Migrators **MUST** ter teste Catch2 com input/output esperado.
- Chain de migrators automática: save em version K carrega via chain K para K+1 para ... para N (versão atual).

### Política forward-only

- Migration é **unidirecional**. Save antigo sobe pra versão atual; save novo NÃO pode descer pra versão antiga.
- Jogador NÃO downgradeia versão do jogo. Se quiser jogar com save velho, instala versão velha.
- Justificativa: complexidade bidirecional não compensa o caso raro de downgrade.

### Anti-patterns proibidos

- ❌ Quebrar save format sem bump de version + migrator.
- ❌ Migrator que altera dados de outro slot (efeito colateral).
- ❌ Save sem `save_version` no header.
- ❌ Carregar save sem validar `save_version <= current_version` (e sem checar o HMAC anti-tamper).

### Pre-v1.0.0 (G1 VS)

- Pre-1.0.0 versions **MAY** invalidar saves sem migrator (rebuild aceitável durante prototipagem).
- Documentar invalidações em CHANGELOG.md com migration notes.
- A partir de v1.0.0, migrators **MUST** ser obrigatórios.

---

## §8. Aplicação + conflito

### Hierarquia de autoridade técnica

1. **CLAUDE.md** (estado atual + decisões fechadas), autoridade situacional.
2. **CONTRACT.md** (este doc), autoridade processual.
3. **ADRs em `docs/tech/adr/*`**, autoridade pra decisões arquiteturais one-way door (ADR-008 = stack atual C++20 + SDL3).
4. **`docs/tech/pivot/engine-design.md`**, autoridade técnica detalhada da engine (4 camadas, plataforma SDL3 conforme ADR-008). Os antigos `architecture.md` / `engine-modules.md` / `build.md` descrevem o stack Godot/C# aposentado e estão marcados como SUPERADOS.
5. **`docs/design/pillars.md`**, autoridade criativa (5 pillars imutáveis).

Conflito entre níveis: nível mais alto vence. Conflito intra-nível: ADR explícito resolve.

### Override

Qualquer regra deste CONTRACT.md **MAY** ser overridden por commit específico se:
- Body do commit declara explicitamente `OVERRIDE CONTRACT §N` com justificativa.
- Follow-up task criada em TODO.md pra revisar override.
- Em sessão seguinte, decisão é: (a) revisar CONTRACT.md pra absorver mudança, ou (b) reverter override.

Override sem follow-up = débito técnico. CI **SHOULD** flagar overrides em audit periódico.

---

## §9. Revisão

Este CONTRACT.md é revisado:
- A cada milestone (M.1 VS, M.2 perf, M.3 playtest, M.4 release).
- Em qualquer ADR que toque processo.
- Quando regra produz fricção repetida (3+ sessões com mesmo workaround).

Mudanças requerem commit `docs(tech): atualiza CONTRACT.md §N, motivo` + ADR se mudança é one-way door.

---

**Assinatura canônica:** este contrato vincula desenvolvimento solo de petrinhu em GusWorld G1. Revisão obrigatória em F2-M.1 (vertical slice done).
