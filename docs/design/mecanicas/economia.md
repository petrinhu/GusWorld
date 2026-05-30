# Economia mínima + sistema de craft — GusWorld G1/F3-Alpha

**Status:** canônico (decisões ratificadas pelo criador via AskUserQuestion 2026-05-30).  
**Escopo:** economia mínima ativa no VS (F2); sistema de craft documentado aqui, implementação em F3-Alpha.

---

## §1. Moeda

**Moeda única no VS:** `crédito` (digital, rastreável, canon lore-bible §11.4).  
**Token-rádio** (analógico, underground): lore dormido no VS; promove a dual-currency em F3+ quando a Periferia/Setor Mirage abrir mecanicamente. Sem dual-currency prematura.

---

## §2. Sources (entrada de crédito)

| Source | Valor | Anti-grind |
|---|---|---|
| Encontro vencido | **8 cr** × `max(0, 1 − (player_zone − enemy_zone) × 0.15)` | herda XP differential já implementado — farm de zona baixa definha |
| Baú / terminal hackeado | **13–34 cr** (fixo por achado) | curatorado, não farmável |
| Marco narrativo (beat de capítulo) | **55 / 89 / 144 cr** | Fibonacci; determinístico; não repetível |

---

## §3. Sinks: Hospital

### 3.1 Cura paga (instantânea)
- HP faltando: **1 cr por 3 HP** (arredonda pra cima). Quem apanhou pouco paga pouco.
- Reativar companion incapacitado: **Fibonacci imediatamente acima do HP-máximo do companion** (`Fibonacci(HP_max+ε)`)  
  - HP-max 55 → custo **89 cr** · HP-max 34 → custo **55 cr**  
  - "Multa" proporcional à força do companion. Consequência real, reversível.

### 3.2 Cura grátis (tempo diegético — NUNCA N encontros)
- HP: recovery ao cruzar **1 beat narrativo** (nó de mapa / gate de área).
- Companion incapacitado: recovery ao cruzar **2 beats narrativos**.
- Regra dura: cura grátis NUNCA é amarrada a vencer N combates (incentivaria grind — anti-pillar).

---

## §4. Bio-Ampola (consumível de combate)

- **Cura dinâmica:** `Fibonacci_inferior(floor((HP_max − HP_atual) / 2))`  
  Ex: faltam 35 HP → metade = 17.5 → Fibonacci inferior = **13 HP curados**.
- **NÃO reativa** companion incapacitado (só Hospital ou Life Ampola).
- **NÃO comprável.** Drop curado (1-2 no VS inteiro).
- 1 AP pra usar em combate.
- *Dilema tático:* cura mais quando mais danificado → decisão "uso agora ou guardo?" (Pillar 1).

---

## §5. Life Ampola

- **Reativa** companion incapacitado (HP = `Fibonacci_inferior(HP_max / 2)` ao reativar).
- **Craftável** (sistema de craft §7) **OU comprável** com crédito premium no Hospital.
- Não é drop curado — obtida por craft ou compra deliberada.

---

## §6. Companion KO

- `ActorIncapacitated` emitido (engine/foundation/turn_combat — já implementado).
- Companion fica **inativo** (fora da fila de iniciativa) até:
  - Cura paga no Hospital (custo §3.1), OU
  - 2 beats narrativos de descanso (grátis), OU
  - Life Ampola usada em campo (reativa com HP parcial).
- **Game-over** só se Gus (`ActorDefeated`): reload no Normal; companion KO ≠ game-over.

---

## §7. Sistema de Craft (implementação: F3-Alpha)

> CUT.3 revisado 2026-05-30: "craft complexo" cortado → substituído por "craft narrativo-gated enxuto" (anti-grind, sem durability, 13 receitas, 18 ingredientes). Reaberto conscientemente pelo criador como escopo de F3-Alpha.

### 7.1 Diegese
Craft = **compilação de componentes via hardware de Gus** (Pillar 2: magia=sistema formal). Nunca caldeirão; sempre determinístico, nunca RNG. Tem os componentes → compila.

**3 estações:**
1. **Bancada de Compilação** (Jaci, laboratório bio) → Poções
2. **Forja de Firmware** (Gus, terminal DRE hackeado) → Implantes + upgrades de stat ("armas")
3. **Patch/Recompilação** (Gus, save-point) → Consertos narrativos + melhorias de tier

### 7.2 Poções (5 receitas, Bancada de Compilação / Jaci)

| # | Nome | Tier | Ingredientes | Efeito mecânico |
|---|---|---|---|---|
| P1 | Ampola de Regen | 1 | 3 | status `Regen` mag 3 / dur 3 |
| P2 | Ampola de Antídoto | 1 | 3 | Dispel Poison/Corrode + imune DoT 2 turnos |
| P3 | Life Ampola | 2 | 5 (1 raro) | reativa companion incapacitado |
| P4 | Ampola de Sobrecarga | 2 | 5 | `Haste` mag 1 + +2 mana próximo turno |
| P5 | Ampola Recursiva | 3 | 8 (1 componente-boss) | cura Fibonacci por 5 turnos (1,1,2,3,5) + `Shield` |

### 7.3 "Armas" = Cartuchos Tavus-Drive + upgrades de stat (Forja de Firmware / Gus)

**Cartuchos passivos** (modulam resolução de cartas sem criar carta nova — GDD §9 proíbe craft de cartas):

| Nome | Efeito |
|---|---|
| Cartucho de Foco | +1 Power em cartas de 1 família escolhida |
| Cartucho de Cadência | 1ª carta do turno custa -1 mana |
| Cartucho de Eco | combos casados ganham +0.15 multCombo |

**Upgrades de stat** (calibração de firmware):

| Nome | Efeito |
|---|---|
| Calibração de Atk | +Atk flat (sigmoid, cap Tier 3) |
| Calibração de Def | +Def flat |
| Sincronização de SPD | +Spd (afeta fila §4) |

### 7.4 Implantes (5, ancorados no triângulo Pillar 3 — NUNCA cria 4º vértice)

| # | Implante | Vértice | Efeito |
|---|---|---|---|
| I1 | Lente de Varredura Profunda | Óculos | Scan revela buffs/debuffs + posição na fila |
| I2 | Antena de Longo Alcance | Matriz Ortodôntica | Gambito-Prever +1 turno à frente |
| I3 | Cache de Pulso | Tavus-Drive | cartas Elétrico -1 AP (mín 1) |
| I4 | Co-processador de Gambito | Óculos | Gambito-Reordenar custa 1 AP (era 2) |
| I5 | Amplificador Ortodôntico | Matriz Ortodôntica | Scan vira passivo (0 AP) contra 1 tipo extra |

### 7.5 Consertos (Patch/Recompilação — narrativo/scriptado, NUNCA durability)

Hardware **nunca quebra por uso normal**. Conserto = consequência narrativa pontual da sabotagem do Dante (foreshadow canônico):
- Boss danifica vértice em cutscene scriptada → Recompilação (custo crédito OU componente, 1× pontual).
- Pós-reveal traição Dante → puzzle de "limpeza de rootkit" (não craft, não consumo de recurso).
- Sabotagem progressiva → opt-in em save-points, nunca obrigatório.

**Regra dura: zero durability sistêmica.** Nenhum tax de engajamento. Pillar 4 inegociável.

### 7.6 Melhorias de Tier (T1→T2→T3)

Método: **recompilação over o item existente** (não receita nova separada).

```
T1 → T2:  item T1 + 1 componente-upgrade + ~150 cr
T2 → T3:  item T2 + 1 componente raro/boss + ~400 cr
```

Curva **sigmoid** (GDD §5.4): efeito ×1.6 por salto, platô em T3 — sem power-creep exponencial.

### 7.7 Custo de craft

**Componentes OU crédito-premium** (mesmo modelo da Life Ampola): player decide entre coletar ingredientes ou pagar mais caro. Sem hard-stuck; sink adicional de crédito.

### 7.8 Ingredientes (18, distribuídos por família/zona)

| # | Nome | Tipo | Fonte | Raridade |
|---|---|---|---|---|
| 1 | Seiva Recursiva | Selve | drop Bio/Selve | comum |
| 2 | Esporo Sintético | Selve | drop Selve (T5 Solo) | comum |
| 3 | Capacitor Sucateado | eletrônico | drop Periferia / compra | comum |
| 4 | Fio Condutor | eletrônico | drop Elétrico/Metal | comum |
| 5 | Fragmento de Dado | cripto | drop Criptográfico | comum |
| 6 | Cristal Bioluminescente | Selve | drop Selve (Biolum.) | incomum |
| 7 | Núcleo de Latão | eletrônico | drop Cinético/Catedrais | incomum |
| 8 | Ressonador Sônico | eletrônico | drop Sônico/Silêncio | incomum |
| 9 | Chave de Decriptação | cripto | drop Cripto/Mirage | incomum |
| 10 | Bio-gel Estável | Selve | compra (hospital/Jaci) | incomum |
| 11 | Lente Polida | eletrônico | compra / drop raro | raro |
| 12 | Antena UHF Rara | eletrônico | drop mini-boss | raro |
| 13 | Esporo-Mestre | Selve | drop inimigo-mestre (Knowledge-gated) | raro |
| 14 | Dado Íntegro | cripto | drop raro / puzzle | raro |
| 15 | Liga de Acaceiro | Selve | drop curado raro (easter egg maçônico) | raro |
| 16 | Núcleo de Recompilação | universal | compra cara (crédito) | incomum |
| 17 | Componente-Boss: Núcleo Quente | boss | drop boss único | épico |
| 18 | Componente-Boss: Fragmento Locke | boss | drop boss (Sterling) | épico |

Anti-grind: ingredientes raros/épicos vêm de Knowledge-gated drops (inimigos-mestre/boss únicos, poucas aparições). Herda o anti-grind do XP differential automaticamente — não dá pra farmar trash mob pra conseguir componente-boss.

---

## §8. Estimativa de implementação (F3-Alpha)

| Etapa | Sessões |
|---|---|
| Doc canônico + planilha de simulação | 2.5 |
| Records POCO (Recipe, Ingredient, CraftStation, Implant) | 1 |
| CraftService (resolução determinística + testes) | 1.5 |
| Integração PlayerBus (crédito, inventário, drops) | 1 |
| Aplicação implantes/cartuchos no CombatActor | 1.5 |
| Melhoria de tier | 1 |
| UI de craft (Diário do Gus / estação) | 1.5 |
| Consertos narrativos (hooks scriptados) | 0.5 |
| QA adversarial (arbitragem, dominância, hard-stuck) | 1 |
| **Total F3-Alpha** | **~11.5 sessões** |

---

**Última revisão:** 2026-05-30. Economia mínima = canon VS. Craft = spec F3-Alpha.
