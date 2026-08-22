# ADR-004: Contrato do EnvironmentModifier (§18 Ambientes de Combate)

| | |
|---|---|
| **Status** | Accepted (contrato ratificado pelo criador supremo 2026-05-30, TODO F2-E.11-CONTRACT; IMPLEMENTADO no Sprint 12, 2026-06-03, F2-E.11) |
| **Date** | 2026-05-30 (contrato) / 2026-06-03 (implementação) |
| **Decisor** | petrinhu (criador supremo) |
| **Reversibilidade** | Semi-irreversível. `multAmbiente` toca o núcleo da fórmula de dano (§11), que já tinha ampla cobertura de teste. Contrato fixado ANTES da impl justamente para evitar retrofit. |
| **Substitui** | nenhum (formaliza o contrato do record antes da implementação F2-E.11) |

## Contexto

O **Sistema de Ambientes de Combate** (combat.md §18) materializa o setting bipartido (Pillar 5: cidade ciber-gótica × Selve Sombria) e a natureza-matemática (Pillar 2) dentro da arena de combate. São três camadas simultâneas (terreno × clima × período) cujos efeitos agem por até quatro canais: `multAmbiente` por família, status facilitado, efeitos de fila/SPD, e interação com hardware (Pillar 3: Scan/Matriz/Tavus).

O canal mais delicado é o **`multAmbiente`**: ele entra na **fórmula de dano §11**, que já tinha ampla cobertura de teste (133 testes no momento da decisão). Mexer no núcleo de combate sem um contrato fixo arrisca **retrofit caro** em testes e em código já verde. Por isso o criador supremo ratificou em 2026-05-30 (F2-E.11-CONTRACT) o contrato do `EnvironmentModifier` **ANTES** de qualquer implementação, classificando a mudança como semi-irreversível.

## Decisão

Contrato canônico do `EnvironmentModifier` e da integração com a fórmula §11, ratificado e implementado:

1. **`multAmbiente` é o ÚLTIMO fator da fórmula §11.** Ordem da cadeia divisiva: `(Power + Atk) × (100/(100+Def)) × multFraqueza × multMod × multCombo × multExpose × multDisrupt × multAmbiente`, seguida de variância Knowledge → crit → `round` único.
2. **Default = 1.0 (retrocompat).** Encontro sem ambiente marcado produz `multAmbiente = 1.0`; o combate sem ambiente fica **inalterado** byte-a-byte. Só `UseCard` consome; o ataque básico subtrativo não.
3. **Stacking de 3 camadas MULTIPLICA.** Os multiplicadores das camadas ATIVAS (terreno × clima × período) que afetam a mesma família se **multiplicam** entre si, com **cap final `multAmbiente ∈ [0.44, 2.25]`** (piso 0.66 × 0.66 ≈ 0.44; teto 1.5 × 1.5 = 2.25). O cap é trava de segurança numérica; a curadoria de transições (§18.6) impede que 2 fontes ×1.5 da mesma família coexistam organicamente.
4. **NUNCA toca `multFraqueza`.** O ambiente é fator próprio na cadeia divisiva; a roda de fraqueza (§6) é independente e nunca é alterada por ambiente.
5. **Hooks de hardware (Pillar 3).** O record declara deltas de hardware (`HardwareHook`): custo de AP do Scan, Scan grátis, antecipação do Gambito-Prever, delta de SPD da party. O cap anti-Scan (degradações de Scan não somam além de -2 AP num encontro, §18.1) é aplicado pela curadoria que monta o conjunto ativo; o record só declara o delta da camada.
6. **Ciclo de PERÍODO por rodada de fila.** A roda Dia(5) → Crepúsculo(2) → Noite(5) → Aurora(2) avança por **rodada completa de fila** (1 tique quando todos os atores agiram uma vez; casa com `RoundIndex`/ramp de mana, independente do tamanho da party). **Telegraph = 2 turnos** (as fases curtas Crep/Aurora são a própria janela-ponte). Determinístico, sem RNG.
7. **T6 Anomalia Perlin = caso especial Patch-Zero.** Vetor anti-padrão exclusivo do boss: **`multAmbiente = 1.0`** (NÃO mexe no dano), mas degrada Scan/Gambito (ruído: Gambito-Prever retorna `IsChaotic`; Scan exige 2 leituras para a fraqueza). O caos do Patch-Zero é informacional, não numérico.

## Alternativas consideradas

### A. `multAmbiente` como fator intermediário (não-último)

**Pros:**
- Posição arbitrária na cadeia funcionaria matematicamente (multiplicação é comutativa).

**Cons:**
- Ordem importa para legibilidade e para o `round` único final; fixar "último" dá um ponto de inserção único e estável.
- Stacking e cap ficam mais difíceis de raciocinar se o fator estiver embaralhado entre `multMod`/`multCombo`/`multExpose`.

**Decidida:** REJEITADA. "Último fator antes da variância/crit" é o ponto de inserção mais legível e testável.

### B. Stacking aditivo (somar os deltas das camadas)

**Pros:**
- Cap linear seria trivial.

**Cons:**
- Quebra a semântica de "multiplicador por família" (×1.3, ×0.66) já canonizada nas tabelas §18.2/§18.3/§18.4.
- Mistura mal com o resto da cadeia, que é toda multiplicativa.

**Decidida:** REJEITADA. Multiplicativo é coerente com a fórmula §11 e com os valores tabelados.

### C. Default neutro + stacking multiplicativo com cap [0.44, 2.25] (escolhida)

**Pros:**
- `multAmbiente = 1.0` garante retrocompat total (combate sem ambiente = baseline).
- Multiplicativo casa com a cadeia divisiva inteira e com os valores canon das tabelas.
- Cap [0.44, 2.25] = limites naturais do sistema (0.66² e 1.5²), trava só a segurança numérica.

**Decidida:** ACEITA e IMPLEMENTADA.

## Consequências

### Positivas

- **Regressão sem-ambiente = baseline garantida por teste.** Conjunto vazio (ou só `None`) produz 1.0; o combate pré-existente não muda.
- **Contrato fixo evitou retrofit.** Os 133 testes de combate que já existiam não precisaram ser reescritos; `multAmbiente` plugou como fator novo no fim da cadeia.
- **Catálogo completo §18 no VS** (decisão F2-PROD.4): 8 climas + 4 períodos + 12 terrenos visíveis + 3 codex, todos os números transcritos direto das tabelas §18 (gameplay_engineer não inventa balance).
- **Hardware nunca vira inútil** (anti-pillar): cap anti-Scan e curadoria determinística preservam a leitura de sistema (Pillar 3).

### Negativas (custos aceitos)

- **Canais 3 e 4 parcialmente diferidos.** Efeitos de canal-3 (fila/SPD-global por ambiente) e parte dos hooks de hardware estão marcados **"jogo posterior"**: os campos do record já existem e estão preparados, mas a aplicação plena fica para fase futura (cartas-clima e inimigos que invocam ambiente também são fase posterior).
- **Semi-irreversibilidade aceita.** O `multAmbiente` está soldado ao fim da fórmula §11; mover o fator depois exigiria revisitar os testes de dano.

## Estado de implementação (Sprint 12, 2026-06-03)

**IMPLEMENTADO e verde.** 577 testes passando.

| Arquivo | Papel |
|---|---|
| `engine/foundation/turn_combat/EnvironmentEnums.cs` | `EnvironmentLayer` (Terreno/Clima/Periodo), `EnvironmentTier` (Visivel/Codex), `EnvironmentId` (catálogo completo + `None`). |
| `engine/foundation/turn_combat/EnvironmentModifier.cs` | Record POCO: `FamilyMults`, `FacilitatedStatus` (reusa enum §9, nenhum status novo; "Root" = `Slow` extremo), `HardwareHook` (ScanApDelta/ScanFree/PreverTurnDelta/PartySpdDelta), `PeriodDuration`, `MultFor(family)`. Imutável; sem dependência Godot. |
| `engine/foundation/turn_combat/EnvironmentCatalog.cs` | Fonte única de instâncias canônicas (números transcritos das tabelas §18); `MultAmbiente(family, active)` = produto das camadas ativas com `Math.Clamp(produto, 0.44, 2.25)`. |
| `engine/foundation/turn_combat/EnvironmentClock.cs` | Roda determinística de período Dia(5)→Crep(2)→Noite(5)→Aurora(2); `Advance()` por rodada de fila; `TurnsRemaining`/`TransitionTelegraphed` (telegraph 2); `Project()`; `IsChaoticVector` (T6 Perlin). |
| `engine/foundation/turn_combat/EnvironmentTransitions.cs` | Tabela de mutabilidade curada e determinística (§18.6). |
| `engine/foundation/turn_combat/CombatConstants.cs` | `MultAmbienteCapMin = 0.44f`, `MultAmbienteCapMax = 2.25f`, neutro 1.0. |
| `engine/foundation/turn_combat/CombatStateMachine.cs` | `multAmbiente = MultAmbienteFor(card.Family)` como ÚLTIMO fator da cadeia divisiva §11; conjunto de ambientes ativos vazio por default → 1.0. |

Pontos confirmados na impl: ciclo 5/2/5/2 por **rodada**; telegraph **2 turnos**; verb **Scan-ambiente** (1 AP, tier Codex); evento `CombatBus.EnvironmentSet`; **T6 Perlin `multAmbiente = 1.0`** (degrada Scan/Gambito, não o dano); **regressão sem-ambiente = baseline** confirmada por teste.

## Cross-refs

- `docs/design/mecanicas/combat.md` §18 (catálogo completo de ambientes) + §11 (fórmula de dano, `multAmbiente`, stacking, cap).
- `ADR-002-csharp-aot-over-gdscript.md` (stack C# POCO + camadas Foundation/Back).
- `TODO.md` F2-E.11-CONTRACT (ratificação do contrato), F2-E.11 (impl), F2-QA.3 (testes TDD), F2-PROD.4 (escopo do catálogo no VS).

---

**Fim do ADR-004.**
