# Capacitor (item de hardware) — turbina de cartas elétricas

**Status:** DESIGN fechado pelo criador (2026-07-15, via AskUserQuestion). **CÓDIGO = feat separada**, ainda não implementada: depende do sistema de inventário/slots de bolsa, que não existe em código hoje. Este doc registra o desenho; a implementação entra quando o inventário for codado.

Cross-ref: `docs/design/mecanicas/cartas-technomagik.md` (carta Tesla / ChainDamage), `docs/tech/adr/ADR-016-techmagic-effect-engine-data-driven.md`, `project_repositorios_perdidos_canon` (loot = hardware), `project_economia_canon`.

## 1. Conceito

O **capacitor** é um item de hardware que o jogador acha ao longo do jogo (nos repositórios perdidos das dungeons) e que **aumenta o dano das cartas elétricas**. Tematicamente é perfeito: um capacitor real *armazena carga e a descarrega em rajada* — exatamente o que uma descarga elétrica de combate faz.

Encaixa na axiologia do loot (`project_repositorios_perdidos_canon`): não se acha dinheiro, se acha **hardware**. O capacitor é uma das poucas peças de loot **usáveis** (a maioria é só vendável no SSD), o que o torna um achado deliberadamente especial, não um drop genérico.

## 2. Decisões fechadas

| Eixo | Decisão |
| :--- | :--- |
| **Progressão** | **Tiers Mk.I / Mk.II / Mk.III.** O jogador acha capacitores de qualidade crescente; cada tier dá um bônus maior. Progressão de loot legível (upgrade de hardware). |
| **Alcance do bônus** | **Todas as cartas elétricas** (Tesla + qualquer carta elétrica futura), não só o Tesla. Dá valor de longo prazo ao item sem inflar cada carta individualmente. |
| **Slots** | **3 slots na bolsa, mas só 2 podem ser ligados no circuito ao mesmo tempo.** O jogador carrega até 3 e escolhe quais 2 usar. |
| **Configuração** | Os 2 capacitores ligados formam um **circuito**: **série** OU **paralelo**. O jogador escolhe a ligação conforme a vantagem que quer. |

## 3. Série vs. paralelo — a física real vira trade-off de jogo

O núcleo do item é uma escolha de **eletrônica real** (Pillar 2: natureza é matemática rígida). Dois capacitores iguais (capacitância `C`, tensão nominal `V`) num circuito simples:

| Ligação | Capacitância equivalente | Tensão | Efeito físico |
| :--- | :--- | :--- | :--- |
| **Paralelo** | soma: `C_eq = 2C` (sobe) | igual: `V` (não sobe) | reservatório maior, guarda **mais carga total** (`Q = CV`) |
| **Série** | cai: `C_eq = C/2` (desce) | soma: `V_eq = 2V` (sobe) | descarrega com **mais "pressão"** (voltagem), reservatório menor |

### Mapeamento pro Tesla (cadeia: *dano por alvo* × *nº de saltos*)

- **Voltagem** (a pressão da descarga) ↔ **dano por alvo** (o punch de cada acerto).
- **Capacitância / carga** (o tamanho do reservatório) ↔ **nº de saltos** (quantos inimigos a cadeia alcança).

Daí o trade-off cai direto da física:

- **SÉRIE** → **+ dano por alvo** (voltagem dobra), **− nº de saltos** (capacitância cai pela metade). A *"agulha"*: concentra a descarga num soco forte, mas alcança menos alvos.
- **PARALELO** → **+ nº de saltos** (capacitância dobra, reservatório maior), **− dano por alvo** (voltagem travada, cada acerto bate menos). O *"chuveiro"*: espalha a corrente por mais inimigos, cada um levando menos.

É a escolha clássica e fisicamente honesta: **rajada concentrada** vs. **cadeia longa**. O jogador que entende de circuito reconhece; o que não entende ainda sente o trade-off na pele.

## 4. Arquitetura (por que não trava o ChainDamage)

O bônus do capacitor é um **modificador upstream**: ele altera o **dano-base** e a **magnitude (saltos)** da carta elétrica **ANTES** de o handler `ChainDamage` rodar. O handler só propaga e decai o `ctx.damage` e o `spec.magnitude` que recebe — já turbinados.

Consequência: o capacitor **não toca** o executor techMagic. Quando o sistema de inventário existir, ele computa o `(power_efetivo, magnitude_efetiva)` da carta elétrica a partir dos capacitores ligados + a configuração (série/paralelo) e alimenta o disparo normal. Zero retrabalho no motor de cartas.

- **SÉRIE:** `power_efetivo = power_base × (1 + bônus_voltagem)`, `magnitude_efetiva = magnitude_base − Δsaltos`.
- **PARALELO:** `magnitude_efetiva = magnitude_base + Δsaltos`, `power_efetivo = power_base × (1 − penalidade_voltagem)`.

Todos os números (`bônus_voltagem`, `Δsaltos`, penalidade, escala por tier Mk.I/II/III) são **//PLAYTEST** — calibrados no playtest N=3.

## 5. Fora de combate

A **religação** série↔paralelo e a troca de quais 2 dos 3 capacitores estão ligados acontece **fora de combate** (tela de bolsa/loadout), não no meio da luta. Provisório; confirmar no playtest se vale permitir religar mid-combate como ação cara.

## 6. Pendências de implementação (quando o inventário existir)

- Item `capacitor` com tier (Mk.I/II/III) no sistema de itens.
- Slot dedicado de capacitor na bolsa (3 posições, 2 ativas).
- Modelo de circuito (série/paralelo) + UI de religação fora de combate.
- Cálculo do `(power_efetivo, magnitude_efetiva)` das cartas elétricas a partir dos capacitores ligados, injetado upstream no disparo.
- Números finos (bônus por tier, Δsaltos, penalidade) — playtest.
