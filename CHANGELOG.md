# Changelog

Todas as mudanças notáveis deste projeto serão documentadas neste arquivo.

O formato é baseado em [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/),
e este projeto adere a [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Added
- `docs/tech/adr/ADR-002-csharp-aot-over-gdscript.md` canonizado (Accepted, one-way door massivo). Mudança GDScript → C# .NET 8 AOT como linguagem primária. Critério: máxima performance em máquinas fracas (GTX 1050 target, Steam Deck).
- `~/.claude/projects/.../memory/reference_godot_csharp.md` (síntese 3 páginas docs Godot C#).
- `README.md` hub-and-spoke (escopo B aprovado).
- `CHANGELOG.md` (Keep a Changelog format, escopo A aprovado).
- `TESTES.md` adaptado Godot/GDScript (escopo B aprovado, T+A sections subset).
- `CONTRACT.md` game-focused canon (319 linhas, 9 seções, 4 decisões canon: cooling-off 12h, perf target GTX 1050+4GB, save forward-only, 4 a11y gates D1).
- `docs/tech/adr/ADR-001-pivot-lore-to-engine.md` canonizado (Accepted, two-way door reversível).
- `game/project.godot` inicial (Godot 4.4 Forward+, locale fallback pt_BR, 1920x1080).
- `game/VERSION` (0.0.1, single source of truth).
- `.gitignore` expandido (`export_presets.cfg.user` + `user://`).
- Deep-lore R3 Settings: 8 docs canônicos `docs/narrative/deep/settings/*.md` (25.201 pal totais).
- Deep-lore R2 Facções: 7 docs canônicos `docs/narrative/deep/factions/*.md` (22.030 pal totais).
- Deep-lore Era 1 Pré-Código §§1-10: ~318k palavras canônicas em `docs/narrative/deep/eras/era-1-pre-codigo.md`.

### Changed
- **BREAKING (ADR-002):** Linguagem canon pivot GDScript → C# .NET 8 AOT. `CLAUDE.md` decisões fechadas atualizadas. `CONTRACT.md` §2 + §4 DoD atualizados (PascalCase, dotnet format, xUnit). `README.md` tech stack atualizado.
- `CLAUDE.md` atualizado: "Estado atual" reflete pivot Fase 1 para Fase 2 (ADR-001). Deep-lore canon ~365k pal entregue.
- `TODO.md` atualizado: F1-DL.4-9 + F1-DL.REFAC reclassificados de Alta gating para Média paralelo orgânico. F2-S.1 ✅ (Godot 4.6.1 stable já instalado). F2-S.2 🔄 em andamento. F2-S.4 ✅ (repo Codeberg confirmado). F2-S.8 ✅ (CONTRACT.md canon).
- Decisão canon pós-Era 1: Fibonacci descontinuado em contagem de palavras + RAG queries (20 queries fixas, alvos arredondados). Easter eggs Fibonacci + maçonaria MANTIDOS no texto/conteúdo.

### Deprecated
- (nada ainda)

### Removed
- (nada ainda)

### Fixed
- (nada ainda)

### Security
- (nada ainda)

---

## Notas de versionamento

- Versões pré-`v1.0.0` (`v0.x.x`) podem invalidar saves sem migrator (rebuild aceitável durante prototipagem). A partir de `v1.0.0`, migrators forward-only são **obrigatórios** (ver `CONTRACT.md` §7).
- Bumps de versão acompanham:
  - **MAJOR** (`v1.x.x` para `v2.0.0`): quebras de save format sem migrator, mudança de pillar, redesign de mecânica core.
  - **MINOR** (`v1.0.x` para `v1.1.0`): nova feature, nova mecânica, nova área, novo companion.
  - **PATCH** (`v1.0.0` para `v1.0.1`): bugfix, balance tweak, hotfix de localização.

---

## Marcos planejados (não-versionados ainda)

| Marco | Versão alvo | Descrição |
|---|---|---|
| F2-M.1 Vertical Slice | `v0.1.0` | 5-10min coeso jogável: cidade explora + 1 combate turn-based + 1 puzzle Gambito + save funcional |
| F2-M.2 Perf validada | `v0.1.1` | 60fps @ 1080p GTX 1050+ confirmado em profiler real |
| F2-M.3 Playtest externo | `v0.2.0` | Time-to-fun ≤ 5min validado com N ≥ 5 jogadores |
| F2-M.4 Build distribuível | `v0.3.0` | `.tar.gz` Linux + `.zip` Windows rodando em máquina limpa |
| F3 Production | `v0.5.x - v0.9.x` | Conteúdo completo + balance + a11y gates v1.0.0 |
| F4 Beta | `v0.9.x` | Beta fechada + bug bash + localization en-intl |
| F5 Release | `v1.0.0` | Ship Linux + Windows. Vol 1 livro lore-bible + Vol 2 antologia narrativa. |
