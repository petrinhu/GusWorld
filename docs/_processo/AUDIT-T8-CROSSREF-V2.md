# F5-BK.AUDIT.FULL T8 v2 — CROSS-REF INTEGRITY

**Data:** 2026-05-22 (TEXTREVIEW v2)
**Issues totais:** 20 (5 CRÍTICOS + 7 MÉDIOS + 8 LEVES)
**Veredito:** APROVAR COM FIXES MÉDIOS, backbone cross-ref canon ROBUSTO

---

## STATUS DE CASCATA (revisão F1-DL.TRACKER-CLOSE 2026-05-30)

> Marcador de bookkeeping. NÃO altera o canon.

**Os 5 CRÍTICOS T8 v2 são técnicos (sintaxe wikilink), permanecem PENDENTES (não verificados).** São de integridade de link Obsidian, não incoerências factuais. CRIT-T8v2-05 (path INCOHERENCES em `~/.claude/agents/revisor-textual.md`) NÃO foi tocado (fora do repo do jogo; arquivo de agent).

**Validação da afirmação deste AUDIT sobre INCOHERENCES (linha "INCOHERENCES C1-C15: resolvidos"):** CONFIRMADA em 2026-05-30. Verificação ponto-a-ponto contra o canon central confirma que C1-C15 estão resolvidos. INCOHERENCES.md foi atualizado (F1-DL.TRACKER-CLOSE) com veredito RESOLVIDO em cada Cx + STATUS GLOBAL (15/15 resolvidas, 0 abertas). A matriz deste AUDIT estava correta.

## ESTATÍSTICAS

- 115 docs scaneados (88 narrative + 27 outros)
- 1.293 wikilinks `[[X]]` totais
- ~115 alvos únicos resolvíveis
- ~98% resolução correta

## CRÍTICOS (5)

### CRIT-T8v2-01. 115 wikilinks escape `\|` quebrado
Padrão `[[target\|alias]]` gera target literal `target\` no Obsidian. Distribuído:
- environments/_INDEX:23-30 (8)
- environments/07:56 + 08:54
- 107+ outros rodapés/tabelas
Fix: sed global `\\|` → `|` em `[[...]]` cross-doc

### CRIT-T8v2-02. 15+ wikilinks decorativos in-character era-2
era-2-boom-tecnico:1077-1465 usa `[[cripto-glifo canon em margem cerimonial preservada]]` como anotações marginais simulando notas cronista. Sintaxe wikilink incorreta. Fix: substituir `[margem: ...]` ou `◊ ... ◊`. Entra refator era-2 agendado.

### CRIT-T8v2-03. 4 wikilinks agents
environments/03+04 referenciam `[[audio-designer-composer]]`, `[[art-director]]`, `[[engine-graphics-programmer]]`, `[[lead-game-designer]]` (vivem ~/.claude/agents/ fora vault). Fix: degradar inline code `` `agent-name` ``.

### CRIT-T8v2-04. 1 JSON placeholder (degradado LEVE)
diary/knowledge-gates:403 JSON `["doc05", "comic11"]` placeholders sem ref canon. False positive grep wikilink. Fix opcional: substituir DD-005 + Cena 11.

### CRIT-T8v2-05. Skill revisor-textual.md path INCOHERENCES errado
~/.claude/agents/revisor-textual.md aponta `/INCOHERENCES.md` mas arquivo só existe em `docs/narrative/INCOHERENCES.md`. Fix: atualizar path skill OU mover INCOHERENCES.md pra raiz por consistência inventários.

## MÉDIOS (7)

- MD-T8v2-01 Extensão `.md` em 10 wikilinks (padronizar sem)
- MD-T8v2-02 Refs `..` prefixo vs sem (`deep/_INDEX` vs era-2 direto)
- MD-T8v2-03 `[[deep/factions]]` vs `[[../../factions]]` vs `[[factions]]` 3 sintaxes
- MD-T8v2-04 `[[characters]]` ambíguo (não-doc, pasta)
- MD-T8v2-05 `[[doc05]]` placeholder JSON
- MD-T8v2-06 CLAUDE.md self-ref `[[CLAUDE.md]]`
- MD-T8v2-07 `[[setting 08]]` placeholder literal

## LEVES (8)

- LV-T8v2-01 # Última revisão formato inconsistente
- LV-T8v2-02 §X.Y.Z nível 3 profundo raros (aceitável)
- LV-T8v2-03 `[[foreshadow-links]]` vs `[[diary/foreshadow-links]]` (OK Obsidian shortest-path)
- LV-T8v2-04 F-NNN gaps esperados
- LV-T8v2-05 in-world-docs `Documento N` vs `DD-NNN` dual canon
- LV-T8v2-06 `[[prelore_vilao]]` underscore (vault canon)
- LV-T8v2-07 Memos refs longas `[[../../../../memory/...]]`
- LV-T8v2-08 `[[_INDEX]]` sublinhado prefix Obsidian convention

## MATRIZES

DD-001 a DD-023: ✅ íntegros dual-canon (in-world-docs + diary)
F-NNN F000-F133: ✅ gaps esperados, master list OK
EE-1 a EE-18 + Cena 1-14: ✅ íntegros comic-reliefs
INCOHERENCES C1-C15: ✅ resolvidos cronologicamente + cross-ref AUDIT-T1-NOMES:279

## STATUS

**APROVAR COM FIXES MÉDIOS** — backbone canon robusto ~98% wikilinks resolvem.

CRIT-01+05 aplicáveis imediato. CRIT-02 absorve refator era-2 agendado. CRIT-03+04 fix trivial.

**Quote canon TEXTREVIEW §1 v2:** "Trate cada lote de ingestão como evidência interconectada de macroestrutura, mapeando relações com precisão algorítmica. O canon é ABSOLUTO."
