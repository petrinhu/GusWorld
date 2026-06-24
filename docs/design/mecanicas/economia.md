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
| Marco narrativo (beat de capítulo) | **55 / 89 / 144 cr** | sequência recorrente; determinístico; não repetível |

---

## §3. Sinks: Hospital

### 3.1 Cura paga (instantânea)
- HP faltando: **1 cr por 3 HP** (arredonda pra cima). Quem apanhou pouco paga pouco.
- Reativar companion incapacitado: **valor da sequência recorrente imediatamente acima do HP-máximo do companion** (`seq_acima(HP_max+ε)`)  
  - HP-max 55 → custo **89 cr** · HP-max 34 → custo **55 cr**  
  - "Multa" proporcional à força do companion. Consequência real, reversível.

### 3.2 Cura grátis (tempo diegético — NUNCA N encontros)
- HP: recovery ao cruzar **1 beat narrativo** (nó de mapa / gate de área).
- Companion incapacitado: recovery ao cruzar **2 beats narrativos**.
- Regra dura: cura grátis NUNCA é amarrada a vencer N combates (incentivaria grind — anti-pillar).

### 3.3 Derrota (party wipe): safe mode e dívida

**Status:** parâmetros ratificados pelo criador supremo (AskUserQuestion, 2026-06-24).
**Cross-ref:** [`battle-screen.md`](battle-screen.md) §3.1 (tela de resultado `BUILD FAILED` que corta pro Hospital), [`combat.md`](combat.md) §2.1 (HP de Gus = menor da party; Análise Preditiva absorve 1 golpe fatal por batalha) e §16 (`ActorIncapacitated` / `ActorDefeated` aciona este fluxo).

A derrota NÃO é game-over e NÃO perde progresso: é um **custo econômico** (Pillar 4). O wipe corta pro Hospital, que cura pelo custo proporcional da §3.1 (1 cr / 3 HP) e reativa incapacitados pela §3.1 (multa Fibonacci `seq_acima(HP_max)`). Se o jogador não tem crédito pra cura completa, ele **ESCOLHE** (anti-softlock, nunca trava):

#### 3.3.1 Safe mode (grátis)

- Revive a party a **13% do HP-máximo por ator** (Fibonacci; arredonda pra baixo; **piso 1 HP**). Ex: HP-max 34 → 4 HP, HP-max 55 → 7 HP.
- Como o HP de Gus é o menor da party (combat.md §2.1), o % incide sobre statlines desiguais e **preserva a hierarquia de fragilidade** (companion tanky revive com mais HP absoluto; Gus segue o mais frágil).
- Sai capenga, mas SEM death-loop: a **Análise Preditiva** (combat.md §2.1, recarrega por batalha) cobre o próximo golpe fatal, garantindo um colchão.
- **Só ofertado em wipe REAL** (party toda incapacitada). Nunca acionável voluntariamente estando vivo.
- **Anti-abuso (net-negativo sempre):** perder rende ~6 cr de "HP de mercado" (13% da party, ~18 HP, ~6 cr a 1 cr / 3 HP), mas custa o crédito do encontro (8 cr × mult) mais XP, loot e Knowledge, que NÃO são recebidos na derrota. Perder de propósito pra curar é sempre pior que curar pagando.

#### 3.3.2 Cura completa a crédito (dívida)

Alternativa ao safe mode: sai com **cura completa**, devendo. Saldo fica NEGATIVO. Lida diegeticamente como **"taxa de recompilação do snapshot"** do Hospital: transparente, plafonada, finita (Pillar 2: sistema formal legível). É o **oposto da usura opaca, composta, de compadrio** (estilo Sterling): livre-troca mais responsabilidade conservadora é o lado bom da axiologia canon.

**Termos (exibidos no terminal ANTES de aceitar, sem letra miúda):**

| Componente | Regra | Teto |
|---|---|---|
| **Principal** | 1 cura completa da party atual (~48 cr nas statlines do VS; calculado da party, sem hardcode) | custo da cura completa |
| **Juros** | **5% SIMPLES** sobre o principal (NUNCA composto), cobrado **1× por NOVA ZONA cruzada** (não por encontro, não por tempo) | **21% do principal** (~11 cr); param de crescer no teto |
| **Multa (reincidência)** | só ao sofrer wipe **enquanto ainda devendo**. Escada curta: **8 cr** na 1ª reincidência, **13 cr** da 2ª em diante, **TRAVA em 13 cr**. Evento pontual, nunca relógio | 13 cr |
| **Dívida total** | principal mais encargos (tetos próprios) | **~72 cr fixo** |

- **Encargos são cobrança morta:** juros e multa aumentam o que se deve, mas NUNCA compram mais cura nem movem o cap. O Hospital só empresta até o cap do principal.
- **Quitação automática:** o jogador escolhe o **plano** ao sair do Hospital (reajustável nas próximas saídas): **agressivo (62%** do crédito recebido vai pra dívida) / **médio (50%)** / **suave (38%)**; o resto fica livre pra jogar. Ordem de abate: **multa, depois juros, depois principal** (estanca a fonte de juros primeiro). Pior caso (~68 cr) quita em ~17 encontros de mult baixo no plano médio: sempre quitável jogando normal.
- **Bloqueia SÓ compras voluntárias** (loja / craft / itens). **NUNCA bloqueia o Hospital** (curar sempre disponível = anti-softlock; ao bater o cap, resta o safe mode grátis, que nunca trava).

#### 3.3.3 Trava anti-bola-de-neve (regra dura)

Juros simples (jamais composto) mais tetos fixos (juros 21%, multa 13 cr, dívida total ~72 cr) mais multa plafonada por-evento mais abate automático = **é matematicamente impossível a dívida crescer mais rápido do que se paga jogando normalmente**. A dívida só anda numa direção (pra baixo) enquanto se joga; nunca afunda sozinha parada. Esta seção é canon anti-dark-pattern: se algum ajuste futuro permitir composição de juros, cap móvel ou multa recorrente, ele viola este contrato e fere o Pillar 4.

---

## §4. Bio-Ampola (consumível de combate)

- **Cura dinâmica:** `seq_inferior(floor((HP_max − HP_atual) / 2))`  
  Ex: faltam 35 HP → metade = 17.5 → valor inferior da sequência = **13 HP curados**.
- **NÃO reativa** companion incapacitado (só Hospital ou Life Ampola).
- **NÃO comprável.** Drop curado (1-2 no VS inteiro).
- 1 AP pra usar em combate.
- *Dilema tático:* cura mais quando mais danificado → decisão "uso agora ou guardo?" (Pillar 1).

---

## §5. Life Ampola

- **Reativa** companion incapacitado (HP = `seq_inferior(HP_max / 2)` ao reativar).
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
| P5 | Ampola Recursiva | 3 | 8 (1 componente-boss) | cura recorrente por 5 turnos (1,1,2,3,5) + `Shield` |

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
| 15 | Liga de Acaceiro | Selve | drop curado raro | raro |
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
