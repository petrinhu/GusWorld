# Plano Mínimo CI/Build — Gate do M.4

**Status:** Canônico. Ratificado Sprint 4 W2 2026-06-03. F2-PROD.7.
**Cross-ref:** build.md, plano_vs.md M.4, raid-log.md R-05.

Documento leve de delivery solo. Princípio reitor (anti-OE): o gate do M.4 é **"`.rpm` + `.tar.gz` rodam em VM clean Fedora sem libs extras, reproduzível via `scripts/build_linux.sh`"** — NÃO "pipeline CI verde". Export manual e smoke vêm PRIMEIRO; CI Forgejo vem DEPOIS e não bloqueia o Vertical Slice. Ordem dura: Fase 0 → 1 → 2 (= gate) → 3 (pós-gate).

**Decisões canônicas (2026-06-03):** artefatos do gate = `.tar.gz` + `.rpm` (Fedora-first); **AppImage adiado** pós-VS. Smoke em 1 distro (Fedora) como gate, Ubuntu não-bloqueante. Fase 0 reordenada: PoC AOT (F2-CI.7) ANTES do preset (F2-CI.6).

---

## Fase 0 — Pré-requisitos físicos (endereça R-05)

Sem isto, NENHUM export roda. Hoje a dir de templates está vazia e o `export_presets.cfg` não existe (R-05, bloqueio silencioso).

Sub-ordem canônica (D1 2026-06-03): **0.1 → 0.2 (PoC AOT) → 0.3 (preset)**. O PoC AOT precede a finalização do `export_presets.cfg` para evitar retrabalho se NativeAOT pleno reprovar no Godot 4.6. Dependência F2-CI.7 → F2-CI.6 registrada no TODO.

| Ordem | Item TODO | Ação | Critério falseável |
|---|---|---|---|
| 0.1 | **F2-CI.5** | Instalar export templates Godot 4.6 mono (`godot --headless --export-templates`) | `ls ~/.local/share/godot/export_templates/4.6.*.stable.mono/` lista arquivos; versão exata documentada no build.md |
| 0.2 | **F2-CI.7** | PoC AOT real: `dotnet publish -r linux-x64` mede e decide `use_aot=true` vs `EnableDynamicLoading=true` | Decisão registrada no build.md §7; se NativeAOT pleno inviável no Godot 4.6, o preset (0.3) já nasce coerente |
| 0.3 | **F2-CI.6 / F2-CI.1a** | Criar + versionar `game/export_presets.cfg` (Linux/X11 + Windows Desktop, renderer gl_compatibility, AOT conforme resultado de 0.2) | Arquivo commitado; `godot --headless --path ./game --export-release "Linux/X11" ...` não falha por preset ausente |

## Fase 1 — Build local 1-comando (reproduzível)

| Item TODO | Ação | Critério falseável |
|---|---|---|
| **F2-CI.1** | Materializar `scripts/build_linux.sh` (`set -euo pipefail`) conforme build.md §3.3: restore → format → build → test → import → export-release | Rodar o script de um clone limpo (templates instalados) gera `build/linux/gusworld.x86_64` sem passo manual no editor |
| (mesmo) | Empacotar artefatos distribuíveis: `.tar.gz` + `.rpm` (AppImage adiado, D3 canon) | `ls -lh build/linux/` mostra ambos; tamanho ≤ 150MB (gatilho M.2); se exceder, aciona plano B do M.2 |

Critério Fase 1 = **build reproduzível 1-comando**, não "build bonito". Sem `.tar.gz` e `.rpm` saindo do script, o gate não pode nem ser tentado. AppImage fica como item pós-VS (empacotamento adicional, não bloqueia o gate).

## Fase 2 — Smoke em VM clean (= O GATE REAL do M.4)

Espelha plano_vs.md M.4 exit criteria. Esta é a barra que define DONE.

| Critério (mensurável) | Método |
|---|---|
| `.rpm` instala e abre no menu em VM clean Fedora, sem dependência manual extra | Boot VM Fedora limpa → `sudo dnf install ./gusworld-*.rpm` → chega ao menu |
| `.tar.gz` extrai e roda standalone na mesma VM | Extrair → rodar binário → menu, sem erro de lib faltante |
| Reprodutível pelo wrapper, não passo manual | Re-rodar `scripts/build_linux.sh` do zero gera artefatos equivalentes |

**1 distro basta para o gate** (Fedora, dev box; D2 canon). Ubuntu LTS é confirmação extra via `.tar.gz`, recomendada mas não bloqueante do M.4. `.rpm` é nativo Fedora; usuários não-RPM usam `.tar.gz`.

## Fase 3 — CI Forgejo (PÓS-GATE, não bloqueia VS)

Só depois que a Fase 2 passar. Sequência: runner primeiro, workflow enxuto depois.

| Ordem | Item TODO | Escopo no VS |
|---|---|---|
| 3.1 | **F2-S.7** | Setup runner Forgejo Actions local (pré-req de tudo em CI) |
| 3.2 | **F2-CI.2 (impl)** | `.forgejo/workflows/ci.yml` enxuto: `dotnet format` + `dotnet build /warnaserror` + `dotnet test` (POCO xUnit, sem runtime Godot) em push/PR. Pipeline alvo **<10min**. Export Godot (templates pesados) = manual/tag, NÃO todo push |
| 3.3 | **F2-QA.5 / F2-SEC.1 / F2-SEC.4 / F2-S.12** | Test job exit-code real + audit camadas + gitleaks + `dotnet list package --vulnerable` + i18n lint. Defense-in-depth barato, plugado incrementalmente |

## Gate vs Deferido (critérios falseáveis)

| Item | M.4 gate? | Critério |
|---|---|---|
| F2-CI.5 templates instalados | **GATE** | dir de templates não-vazia, versão documentada |
| F2-CI.6 export_presets.cfg | **GATE** | arquivo versionado, export não falha por preset |
| F2-CI.7 PoC AOT | **GATE** | decisão AOT registrada; preset coerente com a realidade do 4.6 |
| F2-CI.1 build_linux.sh + `.tar.gz`/`.rpm` | **GATE** | 1-comando, clone limpo, ≤150MB |
| Smoke VM clean (1 distro, Fedora) | **GATE** | `.rpm` instala e abre no menu sem lib extra |
| Smoke 2ª distro (Ubuntu LTS, via `.tar.gz`) | Deferido (recomendado) | confirmação extra |
| **AppImage** | **Deferido pós-VS (D3 canon)** | empacotamento adicional; `.tar.gz`+`.rpm` cobrem o gate |
| Windows export (`build_windows.sh`, F2-CI.1) | **Deferido pós-v1** | template preparado, NÃO no gate; ship pós-v1 (CLAUDE.md) |
| CI Forgejo (F2-S.7 / F2-CI.2) | **Deferido pós-gate** | roda DEPOIS do export manual; não bloqueia M.1-M.4 |
| Test/secret/CVE/i18n jobs (F2-QA.5/SEC.1/SEC.4/S.12) | **Deferido pós-gate** | dependem de F2-CI.2 |
| Signing (Linux/Windows) | **Fora de G1** | sem signing em G1 (CLAUDE.md) |
| AOT validation gate (§7.3) / coverage upload / FsCheck / lockfile | **Deferido** | anti-OE, pós-VS (TODO W6) |

**Windows:** export template preparado mas FORA do gate v1. `scripts/build_windows.sh` existe no canon (build.md §3.3) mas seu artefato não entra no critério de M.4; ship pós-v1.

---

## Decisões Canonizadas (Sprint 4 W2 2026-06-03)

| # | Decisão | Escolha |
|---|---|---|
| D1 | Sub-ordem da Fase 0 | **0.1 → 0.2 (PoC AOT) → 0.3 (preset).** PoC AOT (F2-CI.7) precede o `export_presets.cfg` (F2-CI.6); dependência F2-CI.7 → F2-CI.6 adicionada ao TODO. Evita retrabalho do preset. F2-CI.7 também listado como bloqueador de M.4 em plano_vs.md §2. |
| D2 | Distros do smoke | **1 distro (Fedora) no gate; Ubuntu LTS confirmação não-bloqueante** (via `.tar.gz`). |
| D3 | Artefato distribuível | **`.tar.gz` + `.rpm` no gate** (Fedora-first). **AppImage adiado** pós-VS. M.4 do plano_vs.md atualizado. |
