# TESTES.md, suíte de testes + auditorias canon (Godot adaptado)

> **Status:** Canônico. Escopo B aprovado pelo criador supremo: subset T+A sections do manual canon `Resources/Standards/TESTES.md` adaptado pra Godot 4 + GDScript.
>
> **Autoridade:** processual (T-sections = testes obrigatórios) + auditorial (A-sections = revisões periódicas).
>
> **Cross-ref:** `CONTRACT.md` §4 (DoD por tipo), `docs/tech/architecture.md`, `docs/tech/build.md`.
>
> **Última revisão:** 2026-05-19.

---

## §0. Pré-requisitos + ferramentas

### Stack GusWorld G1

- **Godot 4.4+ stable** (testado em 4.6.1)
- **GDScript** linguagem primária
- **C++ GDExtension** apenas sob pressão de perf (não G1)

### Ferramentas obrigatórias

| Ferramenta | Uso | Install |
|---|---|---|
| `gdtoolkit` (`gdformat` + `gdlint`) | Formatação + lint GDScript | `pipx install gdtoolkit` |
| `GUT` (Godot Unit Test) | Framework unit test GDScript | Addon Godot ou submodule |
| `bandit` + `safety` (opcional) | Secret scan + dep CVE | `pipx install bandit safety` |
| `git-secrets` | Pre-commit secret detection | Pacote distro |

### Não aplicáveis (G1 single-player puro)

- Testes de API REST/GraphQL (sem backend)
- Fuzzing de protocolos de rede (sem network)
- SQL injection (sem SQL)
- Pentesting web (sem web)

---

## T1. Testes Unitários (GDScript via GUT)

### Escopo obrigatório

Módulos `engine/*` **MUST** ter test suite. Módulos `/game/*` **SHOULD** ter cobertura quando lógica não-trivial.

### Suite mínima por módulo

```
engine/<module>/
├── <module>.gd                   (implementação)
├── test_<module>.gd              (suite GUT)
└── README.md                     (docs API pública)
```

### Módulos com cobertura obrigatória D1

| Módulo | Critério |
|---|---|
| `engine/save_system/` | Roundtrip save/load + cada migrator com input/output esperado |
| `engine/turn_based_combat/` | State machine (Round, Turn, ActionSelect, Resolve, TurnEnd) com casos boundary |
| `engine/card_engine/` | Deck operations (shuffle, draw, discard, reshuffle) + effects resolution |
| `engine/orbital_camera/` | Rotação, zoom min/max, follow target, collision-aware quando implementado |
| `engine/event_bus/` | Signal emit/connect/disconnect + edge cases (signal removido com listeners ativos) |

### Comandos

```bash
# Rodar todos os testes
godot --headless --path ./game -d -s addons/gut/gut_cmdln.gd

# Rodar suite específica
godot --headless --path ./game -d -s addons/gut/gut_cmdln.gd -gtest=res://engine/save_system/test_save_system.gd

# Com coverage (futuro, GUT 9+)
godot --headless --path ./game -d -s addons/gut/gut_cmdln.gd -gconfig=res://.gutconfig.json
```

### Critério "done"

- Verde local: zero failures.
- Verde CI: pipeline `tests` job retorna exit 0.
- Cobertura módulo crítico (engine/save, engine/combat, engine/card): ≥ 70% de lines.

---

## T2. Análise Estática (gdformat + gdlint)

### Disciplina

- `gdformat` **MUST** rodar pre-commit (hook configurado).
- `gdlint` **MUST** rodar em CI; erros bloqueiam merge.
- Warnings `gdlint` SHOULD ser justificados em comentário inline `# gdlint: ignore=<rule>` quando intencionais.

### Comandos

```bash
# Format check (não modifica)
gdformat --check engine/ game/ tests/

# Format apply
gdformat engine/ game/ tests/

# Lint
gdlint engine/ game/ tests/
```

### CI integration

```yaml
# .forgejo/workflows/lint.yml (resumo)
- name: gdformat check
  run: gdformat --check engine/ game/ tests/
- name: gdlint
  run: gdlint engine/ game/ tests/
```

### Critério "done"

- Zero erros `gdlint`.
- Zero diffs `gdformat`.
- Warnings justificados se presentes.

---

## T5. Scanning de Dependências (Godot + addons)

### Escopo

- Versão Godot fixada em `game/project.godot` (campo `config_version` + `config/features`).
- Addons internos (`engine/*` standalone-ready) versionados.
- Addons externos terceiros (GUT, DialogueManager, Ink, etc): MUST estar em `game/addons/<addon_name>/` com `plugin.cfg` + version locked.

### Disciplina

- Toda bump de Godot **MUST** gerar entry em `CHANGELOG.md` + commit `chore(deps): atualiza Godot X.Y.Z`.
- Addons externos **MUST** ser auditados antes de adicionar (compatibilidade Godot 4, license, mantenedor ativo nos últimos 12 meses).
- Dependências em `.tres` (Resources) que apontem pra recursos externos **MUST** estar versionadas.

### Comando inventário

```bash
# Inventariar addons instalados
find game/addons -name "plugin.cfg" -exec grep -l "name=" {} \; -exec cat {} \;

# Versão Godot em uso
godot --version
```

---

## T8. Verificação de Secrets

### Disciplina

- **Nenhum secret** deve estar versionado. GusWorld G1 single-player puro NÃO tem secrets de runtime, mas:
  - Tokens de CI (Forgejo, signing futuro): variables Forgejo Actions, NUNCA repo.
  - Future analytics opt-in keys: env vars, NUNCA repo.

### Comando pre-commit

```bash
git-secrets --scan
# OU
trufflehog filesystem ./ --no-update
```

### Hook canônico (já ativo no global)

`~/.claude/hooks/no_mdash.py` bloqueia em-dash + outros padrões anti-pattern.

### Critério "done"

- `git-secrets --scan` retorna exit 0 antes de cada push.
- CI roda secret scan job (futuro F2-CI.X).

---

## T12. Busca de CVEs nas Dependências

### Escopo

- Godot engine: monitorar [advisories Godot Engine](https://github.com/godotengine/godot/security/advisories).
- gdtoolkit: monitorar releases.
- Addons externos: revisar quando bumpar Godot major.

### Disciplina

- Verificação manual a cada milestone (F2-M.1, F2-M.2, F2-M.3, F2-M.4).
- Hotfix urgente se CVE crítico afetar engine + saves do jogador (raro).

### Comando (manual)

```bash
# Versão Godot atual + checagem manual GitHub advisories
godot --version
# Cross-ref: https://github.com/godotengine/godot/security/advisories
```

---

## A1. Descoberta e Modelagem

### Disciplina

Antes de implementar novo módulo `engine/*` ou feature game-side significativa:

1. **Descoberta canon:** revisar `docs/tech/architecture.md` + `engine-modules.md` + `pillars.md` pra contexto.
2. **Modelagem:** desenhar API pública (assinaturas + signals + Resources) em comentário no commit OU ADR se one-way door.
3. **Validação cross-pillar:** mudança respeita os 5 pillars canônicos?

### Critério "done"

- API pública documentada em docstring + README do módulo.
- ADR criado se decisão arquitetural one-way door (ver `CONTRACT.md` §8).

---

## A2. Auditoria de Arquitetura e Camadas

### Princípio

**Engine não depende de game.** Game **MAY** depender de engine. Asserts MUST quebrar em CI se `engine/` importa algo de `game/`.

### Estrutura canon

```
engine/      (reutilizável; standalone-ready)
  └── não importa de /game/
  └── não importa de /assets/
game/        (game-specific)
  └── importa engine/ via res://engine/...
  └── importa assets via res://assets/...
assets/      (sources arte/som)
  └── puro asset, sem código
docs/        (canon + design)
tests/       (test suites GUT)
```

### Audit comando

```bash
# Detecta import indevido engine -> game
grep -rE "extends.*game/|preload.*game/|load.*game/" engine/ && echo "VIOLATION" || echo "OK"
```

### Critério "done"

- Audit verde em CI job dedicado.
- Camadas respeitadas em todo merge.

---

## A3. UI/UX e Acessibilidade

### Escopo

Cobertura dos 4 gates D1 obrigatórios (`CONTRACT.md` §6):

1. Controles remappáveis (keyboard + gamepad).
2. Contraste WCAG 2.2 AA (4.5:1 corpo, 3:1 large/UI).
3. Reduce motion toggle.
4. Subtitles + closed captions.

### Audit por milestone

- **F2-M.1 (VS):** mínimo aceitável (rough toggles, contraste validado em menu principal).
- **F2-M.4 (build distribuível):** completo (todos 4 gates funcionais, validados em ferramenta + manual).
- **v1.0.0:** **MUST** estar 100% canon.

### Ferramentas

- Contraste: extensão browser (axe DevTools, Wave) aplicada a screenshots de UI.
- Manual: testar com gamepad + teclado físico; tab order; focus visíveis.

### Critério "done"

- Audit relatório em `docs/qa/a11y_audit_<milestone>.md` por milestone.

---

## A6. Cobertura de Testes (GUT coverage)

### Disciplina

- Cobertura **medida**, não estimada (GUT coverage report).
- Alvo por módulo:
  - `engine/save_system/`: ≥ 80% lines (críticos: migrators 100%).
  - `engine/turn_based_combat/`: ≥ 70% lines (state transitions cobertas).
  - `engine/card_engine/`: ≥ 70% lines.
  - `engine/orbital_camera/`: ≥ 50% lines (input handling testável).
  - Demais módulos engine: ≥ 50% lines.
  - `/game/scripts/`: ≥ 30% lines (lógica não-trivial coberta; cenas Godot inerentemente difíceis de testar).

### Comando

```bash
# Gerar report (GUT 9+, futuro)
godot --headless --path ./game -d -s addons/gut/gut_cmdln.gd -gcoverage_html=res://coverage/
```

### Critério "done"

- Cobertura atingida em todos módulos D1.
- Report HTML commitado em CI artifact (não no repo).

---

## A7. Dependências e Acoplamento

### Princípio

- Acoplamento engine → engine: minimizado via `event_bus` (signals globais).
- Acoplamento engine → game: **proibido** (ver A2).
- Acoplamento game → engine: explícito via `res://engine/...`.

### Audit

```bash
# Mapear deps cross-engine via grep
for module in engine/*/; do
  echo "=== $module ==="
  grep -rE "preload.*engine/|load.*engine/" "$module" | grep -v "^$module"
done
```

### Critério "done"

- Cada módulo engine importa no máximo `event_bus` + 1-2 outros módulos sibling.
- Zero ciclo de dependência (engine_a → engine_b → engine_a).

---

## A9. Análise Arquitetural Geral

### Escopo (milestone-bound)

A cada milestone (F2-M.1, F2-M.2, F2-M.3, F2-M.4), rodar análise completa:

1. **Camadas respeitadas** (A2).
2. **Acoplamento sob controle** (A7).
3. **Cobertura testes em alvo** (A6).
4. **A11y gates em status apropriado** (A3).
5. **Performance budget em alvo** (`CONTRACT.md` §5).
6. **Save format compat preservado** (`CONTRACT.md` §7).
7. **Lore canon preservado** (cross-ref pillars + lore-bible + characters).

### Output

Relatório em `docs/qa/architectural_audit_<milestone>.md` com:

- Status de cada item (✅ / 🟡 / ❌).
- Plano de remediação pra ❌ antes do próximo milestone.

---

## A10. Relatório Final de Auditoria (pré-release v1.0.0)

### Escopo

Pré-ship v1.0.0, **MUST** existir `docs/qa/final_audit_v1.0.0.md` cobrindo:

1. Todos T-sections aplicáveis (T1, T2, T5, T8, T12) verdes.
2. Todos A-sections aplicáveis (A1, A2, A3, A6, A7, A9) verdes.
3. Os 4 gates a11y D1 100% funcionais.
4. Performance budget validado em hardware target real (GTX 1050+, Steam Deck).
5. Build Linux + Windows reproduzível.
6. CHANGELOG.md `v1.0.0` entry com migration notes.
7. Save format `save_version: N` documentado + migrators all-up-to-N testados.

### Critério ship

- Zero ❌ em relatório final.
- 🟡 aceitáveis se documentados como débito conhecido em `docs/qa/known_issues_v1.0.0.md`.

---

## Classificação de Problemas

| Severidade | Definição | Resposta |
|---|---|---|
| **Crítico** | Crash, save corrupt, data loss, gate a11y v1.0.0 quebrado | Bloqueia ship. Hotfix imediato. |
| **Alto** | Bug reprodutível afetando gameplay core (combate, save, dialogue, locomoção) | Bloqueia milestone. Fix antes de canonizar. |
| **Médio** | Bug em sub-feature, sidequest, conteúdo opcional, polish | Backlog, fix antes da próxima minor. |
| **Baixo** | Cosmético, typo, texto desalinhado, sem impacto funcional | Backlog. Fix oportunístico. |

---

## Formato de Patch (commits de fix)

Seguindo `CONTRACT.md` §2 + §4:

```
fix(<scope>): <subject ≤ 70 chars>

Repro: <passos mínimos>
Root cause: <análise técnica>
Fix: <descrição da correção>
Test: <referência ao teste de regressão adicionado>

Refs: TODO.md <id>, ADR-NNN (se aplicável)
```

---

**Assinatura canônica:** este manual de testes vincula validação técnica de GusWorld G1 do D1 até ship v1.0.0. Revisão obrigatória em F2-M.1 + F2-M.4 + pré-v1.0.0.
