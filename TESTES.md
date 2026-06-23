# TESTES.md, suíte de testes + auditorias canon (C++20 + SDL3)

> **Status:** Canônico. Escopo B aprovado pelo criador supremo: subset T+A sections do manual canon `Resources/Standards/TESTES.md` adaptado pro stack C++20 + SDL3 (engine própria).
>
> **Autoridade:** processual (T-sections = testes obrigatórios) + auditorial (A-sections = revisões periódicas).
>
> **Cross-ref:** `CONTRACT.md` §4 (DoD por tipo), `docs/tech/pivot/engine-design.md` (design da engine, plataforma SDL3 pós-ADR-008).
>
> **Última revisão:** 2026-06-23 (higienização de stack pós-ADR-008: testes em Catch2/ctest no lugar de dotnet/xUnit). Conteúdo anterior 2026-05-19.

---

## §0. Pré-requisitos + ferramentas

### Stack GusWorld G1 (pós-ADR-008)

- **C++20** linguagem primária canon (engine própria, AOT por natureza)
- **SDL3** na camada de plataforma (janela, render2d, input, gamepad, eventos); `core/`+`domain/` são POCO C++ puro
- **RmlUi** (UI do jogador, Fase 3 do re-pivot) + **miniaudio** (áudio)
- **CMake + Ninja + CMakePresets** para build reprodutível Linux + Windows

### Ferramentas obrigatórias

| Ferramenta | Uso | Install Fedora 44 |
|---|---|---|
| `cmake` (3.21+) | Configuração de build + presets | `sudo dnf install cmake` |
| `ninja` | Backend de build rápido | `sudo dnf install ninja-build` |
| GCC/Clang (C++20) | Compilador | `sudo dnf install gcc-c++ clang` |
| `ctest` | Runner de testes (Catch2) | incluído com CMake |
| Catch2 | Framework de unit test C++ | via FetchContent no CMake (pin de versão) |
| `clang-format` | Formatação de code style C++ | `sudo dnf install clang-tools-extra` |
| `clang-tidy` | Análise estática C++ (warnings, code style) | `sudo dnf install clang-tools-extra` |
| `gitleaks` (ou `git-secrets`) | Pre-commit secret detection | binário oficial / pacote distro |

### Não aplicáveis (G1 single-player puro)

- Testes de API REST/GraphQL (sem backend)
- Fuzzing de protocolos de rede (sem network)
- SQL injection (sem SQL)
- Pentesting web (sem web)

---

## T1. Testes Unitários (C++ via Catch2, pós-ADR-008)

### Escopo obrigatório

Módulos `core/*` e `domain/*` **MUST** ter test suite (lógica pura, roda headless). Código `app/*` **SHOULD** ter cobertura quando a lógica não é trivial; a fronteira `platform/*` roda smoke headless (`SDL_VIDEODRIVER=dummy`).

### Suite mínima por módulo

```
GusEngine/<layer>/<module>/
├── <module>.hpp / <module>.cpp   (implementação C++)
GusEngine/tests/
└── <module>_test.cpp             (suite Catch2)
```

Os testes vivem em `GusEngine/tests/` e são registrados no CMake; `ctest` os descobre. Catch2 entra via FetchContent com pin de versão (build reprodutível).

### Módulos com cobertura obrigatória D1

| Módulo | Critério |
|---|---|
| `domain/save/` | Roundtrip save/load + HMAC reject + cada migrator com input/output esperado |
| `domain/combat/` | State machine (TurnStart, ActionSelect, ActionResolve, TurnEnd, CheckEnd) com casos boundary + fórmula de dano §11 (canais FALHA/CRIT/COMUM) |
| `domain/i18n/` | Loader `.md` + fallback + interpolação + chave faltante + paridade de locale |
| `domain/progression/` | EnemyKnowledgeTracker + XpDifferential |
| `core/rng/` | PRNG determinístico seedável (mesma semente = mesma sequência) |
| `core/events/` | Sinal/slot interno: emit/connect/disconnect + edge cases (slot removido com emissores ativos) |

### Comandos

```bash
# Configurar (primeira vez)
cmake --preset linux-release

# Compilar
cmake --build --preset linux-release

# Rodar todos os testes
ctest --preset linux-release

# Rodar suite específica (filtro por nome de teste Catch2)
ctest --preset linux-release -R Save

# Saída detalhada em falha
ctest --preset linux-release --output-on-failure
```

### Critério "done"

- Verde local: zero failures (`ctest` exit 0).
- Verde CI: pipeline `tests` job retorna exit 0.
- Cobertura módulo crítico (`domain/save`, `domain/combat`): ≥ 70% de linhas.

---

## T2. Análise Estática (clang-format + clang-tidy, pós-ADR-008)

### Disciplina

- `clang-format --dry-run --Werror` **MUST** rodar pre-commit (hook configurado).
- `clang-tidy` warnings + code style **MUST** rodar em CI; warnings acima do nível configurado bloqueiam merge.
- Warnings de analisador SHOULD ser justificados via `// NOLINT(<check>)` + comentário quando intencionais.

### Comandos

```bash
# Format check (não modifica)
find GusEngine/{core,domain,platform,app,tests} -name '*.cpp' -o -name '*.hpp' \
  | xargs clang-format --dry-run --Werror

# Format apply
find GusEngine/{core,domain,platform,app,tests} -name '*.cpp' -o -name '*.hpp' \
  | xargs clang-format -i

# Análise estática (usa o compile_commands.json gerado pelo CMake)
clang-tidy -p build/linux-release GusEngine/**/*.cpp

# Build com warnings as errors (flag de compilação no preset)
cmake --build --preset linux-release   # presets ativam -Werror
```

### CI integration

```yaml
# .forgejo/workflows/lint.yml (resumo, pós-ADR-008)
- uses: actions/checkout@v4
  with:
    submodules: recursive
- name: clang-format check
  run: |
    find GusEngine/{core,domain,platform,app,tests} -name '*.cpp' -o -name '*.hpp' \
      | xargs clang-format --dry-run --Werror
- name: configure (gera compile_commands.json)
  run: cmake --preset linux-release
- name: clang-tidy
  run: clang-tidy -p build/linux-release $(git diff --name-only --diff-filter=ACM | grep -E '\.(cpp|hpp)$')
- name: build (warnings as errors)
  run: cmake --build --preset linux-release
```

### Critério "done"

- `clang-format --dry-run --Werror` exit 0.
- Build com `-Werror` exit 0.
- `clang-tidy` warnings justificados via `// NOLINT` se intencionais.

---

## T5. Scanning de Dependências (SDL3 + RmlUi + vendorizadas)

### Escopo

- Versões de SDL3 e RmlUi fixadas no `CMakeLists.txt`/`CMakePresets.json` via FetchContent (pin de tag/commit, build reprodutível).
- Libs C++ header-only vendorizadas em `GusEngine/third_party/` (filosofia zero-dep): version locked + licença registrada em `THIRD-PARTY-LICENSES.md`.
- Módulos próprios (`core/`, `domain/`) são standalone-ready, sem dependência externa.

### Disciplina

- Toda bump de SDL3, RmlUi ou lib vendorizada **MUST** gerar entry em `CHANGELOG.md` + commit `chore(deps): atualiza <lib> X.Y.Z`.
- Dependências externas **MUST** ser auditadas antes de adicionar (licença permissiva compatível com GPLv3, mantenedor ativo nos últimos 12 meses).
- O pin de versão **MUST** estar versionado no CMake (nunca "latest" flutuante).

### Comando inventário

```bash
# Versões pinadas no build (FetchContent)
grep -rEn "GIT_TAG|URL_HASH|FetchContent_Declare" GusEngine/CMakeLists.txt

# Inventariar libs vendorizadas
ls GusEngine/third_party/
```

---

## T8. Verificação de Secrets

### Disciplina

- **Nenhum secret** deve estar versionado. GusWorld G1 single-player puro NÃO tem secrets de runtime, mas:
  - Tokens de CI (Forgejo, signing futuro): variables Forgejo Actions, NUNCA repo.
  - Future analytics opt-in keys: env vars, NUNCA repo.

### Comando pre-commit

```bash
gitleaks detect --no-banner
# OU
git-secrets --scan
```

### Hook canônico (já ativo no global)

`~/.claude/hooks/no_mdash.py` bloqueia em-dash + outros padrões anti-pattern.

### Critério "done"

- Scan de secrets retorna exit 0 antes de cada push.
- CI roda secret scan job (futuro F2-CI.X).

---

## T12. Busca de CVEs nas Dependências

### Escopo

- SDL3: monitorar [advisories SDL](https://github.com/libsdl-org/SDL/security/advisories).
- RmlUi: monitorar [advisories RmlUi](https://github.com/mikke89/RmlUi/security/advisories).
- Libs vendorizadas (`GusEngine/third_party/`): revisar releases quando bumpar.

### Disciplina

- Verificação manual a cada milestone (F2-M.1, F2-M.2, F2-M.3, F2-M.4).
- Hotfix urgente se CVE crítico afetar a engine + saves do jogador (raro).

### Comando (manual)

```bash
# Versões pinadas atuais + checagem manual de advisories
grep -rEn "GIT_TAG|FetchContent_Declare" GusEngine/CMakeLists.txt
# Cross-ref: advisories de SDL3, RmlUi e das libs de third_party/
```

---

## A1. Descoberta e Modelagem

### Disciplina

Antes de implementar novo módulo `engine/*` ou feature game-side significativa:

1. **Descoberta canon:** revisar `docs/tech/pivot/engine-design.md` + `pillars.md` pra contexto.
2. **Modelagem:** desenhar API pública (assinaturas dos headers + eventos do barramento interno) em comentário no commit OU ADR se one-way door.
3. **Validação cross-pillar:** mudança respeita os 5 pillars canônicos?

### Critério "done"

- API pública documentada em docstring + README do módulo.
- ADR criado se decisão arquitetural one-way door (ver `CONTRACT.md` §8).

---

## A2. Auditoria de Arquitetura e Camadas

### Princípio

**Lógica pura não depende de plataforma nem da casca.** A invariante das 4 camadas: `core/`+`domain/` NÃO incluem `<SDL...>` nem nenhum header de plataforma; só `platform/` e `app/` linkam SDL3. Asserts MUST quebrar em CI se a invariante for violada.

### Estrutura canon

```
GusEngine/
├── core/        POCO C++ puro, ZERO SDL, 100% testável headless
├── domain/      POCO C++ puro, ZERO SDL (save, i18n, progression, templates, combat)
├── platform/    ÚNICA fronteira SDL3 (window, render2d, input, audio, fs)
├── app/         GusWorld-specific (screens, main)
└── tests/       suites Catch2 (registradas no CMake, descobertas por ctest)
assets/          (sources arte/som; puro asset, sem código)
docs/            (canon + design)
```

### Audit comando

```bash
# Detecta inclusão indevida de SDL/plataforma na lógica pura
grep -rE '#include[[:space:]]*[<"]SDL' GusEngine/core GusEngine/domain \
  && echo "VIOLATION" || echo "OK"
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

## A6. Cobertura de Testes (gcov/llvm-cov + lcov)

### Disciplina

- Cobertura **medida**, não estimada (relatório de cobertura do compilador).
- Alvo por módulo:
  - `domain/save/`: ≥ 80% linhas (críticos: migrators 100%).
  - `domain/combat/`: ≥ 70% linhas (transições de estado + fórmula de dano cobertas).
  - `core/rng/`, `core/events/`: ≥ 70% linhas.
  - Demais módulos `core/`/`domain/`: ≥ 50% linhas.
  - `app/`: ≥ 30% linhas (lógica não-trivial coberta; a fronteira de render/input é difícil de cobrir headless).

### Comando

```bash
# Build instrumentado (preset com --coverage / -fprofile-instr-generate)
cmake --preset linux-coverage
cmake --build --preset linux-coverage
ctest --preset linux-coverage

# Gerar report (GCC/gcov + lcov)
lcov --capture --directory build/linux-coverage --output-file coverage.info
genhtml coverage.info --output-directory coverage/
# Clang: usar llvm-profdata merge + llvm-cov show/report
```

### Critério "done"

- Cobertura atingida em todos os módulos D1.
- Report HTML como CI artifact (não no repo).

---

## A7. Dependências e Acoplamento

### Princípio

- Acoplamento intra-camada: minimizado via o barramento de eventos interno (`core/events/`, sinal/slot desacoplado de plataforma).
- Acoplamento lógica pura → plataforma: **proibido** (ver A2; `core/`+`domain/` nunca incluem SDL).
- Acoplamento `app/` → engine: explícito via include dos headers públicos.

### Audit

```bash
# Mapear inclusões entre módulos de domínio/core via grep
for module in GusEngine/core/*/ GusEngine/domain/*/; do
  echo "=== $module ==="
  grep -rhoE '#include[[:space:]]*[<"][^>"]+' "$module" | sort -u
done
```

### Critério "done"

- Cada módulo importa no máximo `core/events/` + 1-2 outros módulos sibling.
- Zero ciclo de dependência (módulo_a → módulo_b → módulo_a).

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
