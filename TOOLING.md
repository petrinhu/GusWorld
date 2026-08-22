# TOOLING.md - Ferramentas do GusWorld

> Catálogo restrito ao stack real deste projeto: **C++23, CMake/CTest, sem banco de dados, sem rede, sem web, sem Qt, sem Godot** (LEI ZERO do `GODS_LAWS.md`). O agent usa a ferramenta canônica do domínio sempre que aplicável, nunca reinventa em shell cru. Se faltar (⬇), instala com o comando indicado antes de usar.

## Legenda de status

- ✓ **temos** (instalado no sistema)
- ⬇ **baixar** (faltando, instalar sob demanda com o comando da coluna)

---

## 1. Build C++23

| Ferramenta | Status | Para quê | Instalar |
|---|---|---|---|
| cmake / ninja / make | ✓ | geração e build do projeto (L-03: C++23) | `sudo dnf install cmake ninja-build make` |
| clang-format | ✓ | formatação uniforme do código (L-22: `snake_case`, sem `m_`) | `sudo dnf install clang-tools-extra` |
| clang-tidy | ✓ | lint estático C++, portão 3 da L-19 | `sudo dnf install clang-tools-extra` |
| cppcheck | ✓ | análise estática C/C++, portão 3 da L-19 | `sudo dnf install cppcheck` |

---

## 2. Testes e sanitizers (portões da L-19)

| Ferramenta | Status | Para quê | Instalar |
|---|---|---|---|
| ctest | ✓ | roda a suíte de teste (vem com o CMake); TDD estrito (L-19) | (vem com cmake) |
| ASan (`-fsanitize=address`) | ✓ | detecção de memory error, portão 2 da L-19, build separado a cada fatia | (flag do compilador; vem com gcc/clang) |
| UBSan (`-fsanitize=undefined`) | ✓ | detecção de comportamento indefinido, portão 2 da L-19 | (flag do compilador; vem com gcc/clang) |

---

## 3. Segredo e licença

| Ferramenta | Status | Para quê | Instalar |
|---|---|---|---|
| gitleaks | ✓ | scan de segredo, portão 4 da L-19. Cobre a árvore por padrão; para dado sensível, `git log --all -p \| grep -ci <termo>`, sempre com `-i`, nunca `git grep` | `sudo dnf install gitleaks` |

REUSE/SPDX (L-08) é conferido por inspeção do cabeçalho de cada arquivo; ferramenta dedicada só se o líder autorizar instalação de `reuse` (`pipx install reuse`).

---

## 4. Versionamento de binário (L-15)

| Ferramenta | Status | Para quê | Instalar |
|---|---|---|---|
| git-lfs | ⬇ | binário pesado versionado (sprites, VFX, áudio). `resources/livros/` e `resources/glb/` ficam **fora** do git, LFS não se aplica a eles | `sudo dnf install git-lfs` |

---

## 5. Kit canônico por agent (usar SEMPRE que a tarefa pedir)

| Agent | Ferramentas canônicas |
|---|---|
| backend-engineer (C++23) | cmake, ninja, clang-format, clang-tidy, cppcheck, ASan, UBSan |
| tech-lead | clang-tidy, cppcheck, ctest |
| qa-engineer | ctest, ASan, UBSan |
| security-engineer / internal-auditor | gitleaks, `git log --all -p` |
| devops-sre | cmake/ninja/make (matriz de CI das 5 plataformas, L-09/L-20), git-lfs |
| compliance-legal | inspeção de cabeçalho REUSE/SPDX (L-08) |
