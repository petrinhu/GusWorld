# AUDITORIAS.md, manual de auditorias canon (C++20 + SDL3 + jogo)

> **Status:** Canônico. Escopo B (mesmo espírito do `TESTES.md`): subset das auditorias
> do manual canônico do vault ([[AUDITORIAS]] em `Resources/Standards/`) **podado** para o
> stack real do GusWorld G1 - engine própria C++20 + SDL3, jogo single-player puro, sem
> rede, sem banco SQL, sem PII sensível além de um `settings.json`/`save` local do jogador.
>
> **Autoridade:** auditorial. Auditoria é **downstream** de código + teste: só audita o que
> já foi implementado e testado (cross-ref `TESTES.md` §A1-A10, `CONTRACT.md` §4 DoD).
>
> **Cross-ref:** `TESTES.md` (T/A sections), `CONTRACT.md` (§4 DoD, §6 a11y gates, §7 save
> compat), `docs/tech/pivot/engine-design.md` (4 camadas), catálogo genérico da skill
> `tab_pendencias` (`references/catalogo-testes-auditorias.md`).
>
> **Criado:** 2026-07-04 (item AUD-M7-COSTURA - o GATE de auditoria pediu o manual e ele
> não existia na raiz; criado aqui, podado ao stack, seguindo o catálogo da skill).

---

## Como usar

Cada auditoria vira um item `AUD-*` na `TODO.md`, nas ondas finais de cada milestone
(downstream de código + teste). O dossiê de cada rodada vive em `docs/auditoria/AUDIT-<marco>-<data>/`
(padrão dos dossiês existentes AUDIT-M3, AUDIT-M5-MOTOR). Cada dossiê tem:
`00_indice_mestre.md` (escopo, sumário executivo, contagem por severidade, parecer) +
1 arquivo por capítulo auditado (contexto, método, achados em tabela, conclusão).

Achado **sem evidência** (arquivo:linha ou saída de comando) **NÃO entra** no livro.

---

## Catálogo aplicável (podado ao stack C++20 + SDL3 + jogo)

Derivado do catálogo genérico `AUD-*` da skill `tab_pendencias`, removendo o que não tem
superfície neste perfil (ver "Fora de escopo" abaixo). IDs semânticos estáveis.

| ID | Tema | O que verifica | Pré-requisito | Ferramentas |
|---|---|---|---|---|
| AUD-ARCH | Arquitetura e camadas | Invariante das 4 camadas (`core/`+`domain/` são POCO puro: **zero** `<Qt…>`, `<SDL…>`, `<RmlUi…>`, `<glintfx…>` e zero `Rml::`/`glintfx::`); I/O real só em `platform/`+`app/` via ports; SOLID/DRY; sem ciclo de dependência | código + teste do subsistema | `grep` do GATE (`tools/check.sh` + `.forgejo/workflows/ci.yml`), `clang-tidy`, revisão de camadas |
| AUD-QUALITY | Qualidade de código | God classes, complexidade, dead code, duplicação; ponteiros não-donos com ownership claro; RAII (sem `new`/`delete` cru); erros por exceção tipada / fail-fast | código + teste | `clang-tidy` (modernize, cppcoreguidelines), `tokei`, revisão |
| AUD-MEM | Memory-safety dinâmica | Sem use-after-free / leak / double-free / UB de float ou inteiro em runtime; build extra sob **ASan + UBSan** rodando as suítes | binário + suíte | ASan + UBSan (build ad-hoc `-fsanitize=address,undefined`) |
| AUD-SEC | Segurança (superfície do jogo) | Robustez de parsing de **entrada não-confiável de disco** (save/template/`.gmap`/`settings.json` - arquivos que o jogador pode adulterar): validação, HMAC-antes-de-usar, degradação sem crash; permissões do arquivo de dados do jogador (LGPD leve: `0600`/`0700`) | código + teste do decoder | `semgrep`, ASan/fuzz sobre decoders, revisão do modelo de ameaça |
| AUD-I18N | Paridade i18n | Paridade **estrutural** entre locales (`pt_br` fonte × `en_intl`): zero chave faltando/órfã/duplicada; zero string user-facing hardcoded (tudo via `tr()`); interpolação de placeholders `{0}/{1}` de fato aplicada onde a string os usa | catálogos + loader testados | `tools/i18n_parity.py` (espelha o loader C++), `grep` de literais |
| AUD-UI | UI/UX e acessibilidade | 4 gates a11y D1 (`CONTRACT.md` §6): controles remappáveis, contraste WCAG 2.2 AA, reduce-motion, subtitles/CC - no status apropriado ao milestone | tela implementada | axe/contrast checker sobre screenshots, teste manual gamepad+teclado |
| AUD-COV | Cobertura de testes | Cobertura **medida** (não estimada) nos módulos críticos (`domain/save`, `domain/combat`); paths críticos cobertos | suíte | `lcov`/`gcov`, `llvm-cov` |
| AUD-DEPS | Dependências e supply-chain | Pins imutáveis no `FetchContent` (SDL3, RmlUi por SHA, glintfx por tag), libs vendorizadas em `third_party/` com licença registrada; CVE das deps; licenças compatíveis com GPLv3 | build reprodutível | `trivy fs`, `syft`, `scancode-toolkit`, revisão de advisories |
| AUD-REPORT | Relatório final | Consolida os achados por severidade, parecer de prontidão, plano de remediação rastreado | as auditorias acima | consolidação manual (o `00_indice_mestre.md` do dossiê) |

### Fora de escopo (descartados - sem superfície neste perfil)

- **AuthN/AuthZ, sessão, token** - jogo single-player, sem login.
- **API REST / OpenAPI / status codes** - sem backend HTTP (o `AUD-API` do catálogo genérico não se aplica).
- **SQL / injeção SQL / EXPLAIN / migrations de banco** - sem banco SQL (o save é binário próprio; `AUD-DB` não se aplica).
- **DAST (ZAP/nuclei) / rede / TLS / CORS / CSRF** - sem socket, sem servidor.
- **Rate-limit / anti-DoS de rede** - sem rede.
- **PII sensível / LGPD pesado** - o único dado do jogador é `settings.json` + save local (preferências + progresso); tratado em `AUD-SEC` (permissão de arquivo), não como pipeline de PII.

Cripto própria (SHA-256/HMAC do save/`.gmap`) é auditada sob `AUD-SEC` quando a rodada
toca esse subsistema (ver ADR-006); não é um capítulo permanente de todo dossiê.

---

## Severidade dos achados

Alinhada ao `TESTES.md` (Classificação de Problemas) e ao manual canônico do vault.

- **🔴 CRÍTICO** - corrompe memória/estado, crash, save corrompido, data loss, gate a11y de
  v1.0.0 quebrado, ou é explorável. **Bloqueia release / ship.** Um 🔴 aberto impede marcar
  o item como `✅` e impede `✓` no Estado Auditado.
- **🟠 IMPORTANTE** - bug latente, invariante frágil, falta de validação, defeito user-facing
  visível sem quebra de função. **Corrigir antes da tag / do próximo milestone.** Não bloqueia
  o `✅` do milestone interno, mas mantém o Estado Auditado em `⚠` (com ressalvas) até fechar.
- **🟢 COSMÉTICO** - estilo, clareza, comentário stale, doc. **Não bloqueia.**

## Regra de estado

Estado Auditado na `TODO.md` / na tabela do dossiê:

- `-` não auditado
- `✓` aprovado (auditado, **zero** achado aberto de severidade 🔴 **ou** 🟠)
- `⚠` auditado **com ressalvas** (há 🟠 aberto rastreado, ou 🔴 sob mitigação documentada)

Nenhum item vai para uma tag de release (`v0.x`/`v1.0.0`) com achado **🔴 CRÍTICO** aberto.
Auditoria sempre tem como pré-requisito os itens de código + teste que ela cobre (nunca
auditar antes de testar). Nenhum 🔴 fica sem plano de remediação + re-teste.

---

**Assinatura canônica:** este manual de auditorias vincula a validação auditorial de
GusWorld G1 do D1 até ship v1.0.0. Complementa o `TESTES.md` (§A-sections). Revisão
obrigatória em F2-M.1, F2-M.4 e pré-v1.0.0, ou quando o stack muda por ADR.
