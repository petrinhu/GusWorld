# Sistema de Encontros Aleatórios (GusWorld)

**Status:** Canônico. As 4 decisões estruturais foram tomadas pelo criador supremo em 2026-08-12 via AskUserQuestion (modelo da taxa; estreia; interação com Faraday; composição do encontro). Materializa o item `ENCONTRO-ALEATORIO-SISTEMA` (decisão de CONSTRUIR de 2026-08-03) e destrava o item filho `ENCONTRO-FREQ-DIFICULDADE`. Arquitetura técnica da onda desenhada pelo CTO: módulo `domain/encounter/` POCO, `EncounterProfile` como DADO por área, `EncounterDirector` com contador de graça e rolagem por tile cruzado, tabela de multiplicador por dificuldade no molde de `enemy_difficulty_constants.hpp`, e o spawn-path `EnemyTemplate → CombatActor` (a lacuna de wiring já registrada em `DIFICULDADE-TABELA-DADO` nasce fechada aqui).

**Cross-ref:** `combat.md` (§2 parâmetros macro, §17 stats de referência, §19 eixo de domínio auto-kill/auto-resolve, §20 cross-ref de origem do combate), `docs/tech/adr/ADR-020` (lei do átomo: lugar novo é DADO, nunca método par-a-par), item `INIMIGO-SOME-POS-FUGA-DERROTA` (encontro fixo × genérico), memória `project_save_dungeon_pem_faraday` (PEM × autosave).

**Convenção de escrita:** pt-br. Termos de game-dev no original (tile, spawn, clamp, pity). Sem em-dash; usa ponto, vírgula, parênteses, dois-pontos.

---

## 1. Visão e pillars

O encontro aleatório é a ponte entre exploração e combate: transforma andar pela dungeon em decisão com custo (cada tile cruzado carrega risco crescente e legível). O sistema serve:

- **Pillar 1 (Lógica vence força):** a taxa é uma função determinística e publicável do movimento. Um jogador analítico consegue LER o sistema (percebe a janela de graça, planeja micro-rotas, decide quando vale atravessar). Nada de "encontro do nada" sem estrutura.
- **Pillar 2 (Magia é sistema formal):** a regra é uma fórmula fechada, com fronteiras exatas e testáveis, não uma vibração de designer.
- **Anti-frustração (Pillar 4):** a graça garante respiro pós-batalha (curar, reorganizar, salvar); o teto de 8% garante que exploração longa nunca vira marcha da morte.

Hoje o combate só dispara por gatilho fixo (`to_battle(EncounterId)`). Este sistema acrescenta o disparo por frequência, sem remover o gatilho fixo (os dois convivem: encontro fixo é scriptado, encontro aleatório é genérico).

---

## 2. Modelo da taxa: rampa com graça (decisão 1 do líder, 2026-08-12)

Modelo escolhido: **RAMPA estilo Final Fantasy**. Três regimes, em função de `t` = número de tiles cruzados desde o último reset de graça (§3).

### 2.1 Fórmula canônica

```
t = tiles cruzados desde o último reset (t = 0 no reset; incrementa a cada
    cruzamento de tile completado; a rolagem usa o valor já incrementado)

chanceBase(t):
  t <= 8          →  0                              (GRAÇA: 8 tiles inteiros sem risco)
  9 <= t <= 42    →  1% + (t − 9) × (7/33)%          (RAMPA: 34 tiles, linear de 1% a 8%)
  t >= 43         →  8%                              (PLATÔ: teto, constante)

Forma fechada equivalente:
  chanceBase(t) = (t <= 8) ? 0% : min(1% + (t − 9) × (7/33)%, 8%)
```

### 2.2 Pontos de fronteira (verificados)

| t (tiles desde o reset) | chanceBase | Regime |
|---|---|---|
| 1 a 8 | **0%** | graça (chance literalmente zero, nenhum RNG consumido) |
| 9 | **1,000%** | primeiro tile rolável (a rampa começa exatamente em 1%) |
| 10 | 1,212% | rampa |
| 13 | 1,848% | rampa |
| 21 | 3,545% | rampa |
| 34 | 6,303% | rampa |
| 42 | **8,000%** | último tile da rampa (34º tile rolável: 42 − 9 + 1 = 34) |
| 43+ | **8%** | platô (constante; NUNCA volta a crescer, NUNCA vira encontro forçado) |

O passo da rampa é `7/33` de ponto percentual por tile (≈ 0,2121 pp). Não existe pity de encontro: mesmo no platô, o jogador azarado nunca é punido com chance 100%; o teto é 8% para sempre.

### 2.3 Forma inteira exata (para implementação e testes)

Para eliminar float da regra (mesmo espírito determinístico da fórmula de dano, `combat.md` §11), a chance se expressa como threshold inteiro sobre base 6600:

```
thresholdBase(t):
  t <= 8   →  0
  t >= 9   →  min(66 + (t − 9) × 14, 528)

Rolagem por tile: roll = rng.Next(0..6599); encontro ⇔ roll < thresholdFinal
```

Verificação: `66/6600 = 1%` (t=9); passo `14/6600 = 7/33 %`; `528/6600 = 8%` (t=42, pois 66 + 33×14 = 528); t=43 dá 66 + 34×14 = 542, clampado em 528. A base 6600 foi escolhida porque TODOS os valores da rampa e TODOS os valores sob Faraday 0,5× (§4.2) saem inteiros exatos, sem arredondamento em nenhum tile.

### 2.4 Consumo de RNG (contrato de teste)

- Durante a graça (t ≤ 8): **zero** consumo de RNG.
- Após a graça: **exatamente 1** consumo por tile cruzado (o roll 0..6599).
- Testes com seed fixa assertam essa contagem, no mesmo padrão da ordem de consumo da §11 do `combat.md`.

### 2.5 O que conta como "cruzar um tile"

O contador incrementa quando o tile do grid sob o gatilho de ativação do jogador muda (hitbox de ativação nos pés, padrão já canônico do overworld). Andar dentro do mesmo tile não conta; ficar parado não conta; re-cruzar o mesmo tile indo e voltando conta a cada cruzamento (padrão FF clássico: o contador mede movimento, não território novo).

---

## 3. Graça: os 3 gatilhos de reset

A graça (t volta a 0) reseta em exatamente três situações:

1. **Fim de batalha, qualquer desfecho:** vitória, fuga ou derrota (`CombatBus.CombatEnded`). Inclui o **auto-kill silencioso** do selo Ouro (`combat.md` §19.2): auto-kill é uma vitória resolvida no overworld, então também reseta (sem isso, um jogador com Ouro andaria no platô de 8% encadeando auto-kills a cada poucos tiles, virando ruído).
2. **Entrada em área nova:** ao carregar um `AreaDescriptor` diferente (porta de dungeon, transição de mapa).
3. **Load de save:** carregar um save começa com graça cheia.

Consequências deliberadas:

- **Distância mínima dura entre dois encontros aleatórios: 9 tiles.** Nunca existe encontro back-to-back (a maior fonte de frustração do modelo Pokémon de chance fixa).
- **O contador NÃO persiste no save.** O gatilho 3 torna a persistência desnecessária por construção: nada deste sistema entra no save (mesmo princípio do soft enrage, que deriva tudo de `round_index`).
- **Exploit conhecido e ACEITO:** cruzar a borda de área ida-e-volta renova a graça indefinidamente (a borda vira zona segura de fato). É o comportamento FF clássico; quem faz isso só está escolhendo não lutar, e o custo dele é não ganhar loot nem Knowledge. Registrado por transparência (catálogo de exploits do projeto), não é bug.

---

## 4. Multiplicadores sobre a chance

A chance final é a chance base da rampa vezes multiplicadores independentes:

```
chanceFinal(t) = chanceBase(t) × multDificuldade × multFaraday
```

Aplicados na forma inteira: `thresholdFinal = thresholdBase(t) × multDificuldade × multFaraday`, com a base 6600 garantindo exatidão para o 0,5× do Faraday em todos os tiles.

### 4.1 `multDificuldade` (slot reservado; item `ENCONTRO-FREQ-DIFICULDADE`)

- **Nesta onda: 1,0 fixo** (neutro). O slot existe desde o D1 para a tabela não nascer como retrofit.
- A tabela real entra pelo item `ENCONTRO-FREQ-DIFICULDADE`, como TABELA DE DADO cruzando `DifficultyLevel`, no MESMO molde de `enemy_difficulty_constants.hpp::difficulty_multiplier_for` (Médio = 1,0 definitivo, demais níveis a calibrar), NUNCA como `if` de dificuldade espalhado (lei do átomo, ADR-020).
- **Tabela implementada 2026-08-12:** `gus/domain/encounter/encounter_difficulty_constants.hpp::encounter_rate_multiplier_for(DifficultyLevel)` — eixo ÚNICO (`DifficultyLevel`, sem `EnemyTier`, porque a taxa de encontro não varia por tier de inimigo), Médio=1,0 definitivo, Facil/Dificil/Hardcore=1,0 `//PLAYTEST`. O **wiring** no `EncounterDirector` (multiplicar `chanceFinal(t)` por este fator) ainda NÃO existe — é peça do `ENCONTRO-ALEATORIO-SISTEMA`, que consome a função quando o Director estiver pronto.
- Teto final por dificuldade (se um nível usar mult > 1,0 e ultrapassar 8%): decisão pré-registrada como PENDENTE do próprio `ENCONTRO-FREQ-DIFICULDADE`. Este doc não fixa esse número; fixa apenas que para o Médio o teto final é 8% (e 4% sob Faraday).

### 4.2 `multFaraday` = 0,5× (decisão 3 do líder, 2026-08-12: REDUZ, não suprime)

Quando a carta **Gaiola de Faraday** está ativa numa dungeon:

- `multFaraday = 0,5` (metade da chance, em TODOS os regimes: rampa e platô).
- A graça não muda (8 tiles); só a chance rolável cai. Teto efetivo sob Faraday: **4%**.
- Na forma inteira: threshold vai de 66..528 para 33..264, todos inteiros exatos.

**Decisão definitiva:** a opção de supressão total (0×, dungeon silenciosa enquanto a carta está ativa) foi apresentada e o líder escolheu a redução de 0,5×. Consequência de design registrada: a Faraday é MITIGAÇÃO tática contínua, não imunidade; a dungeon nunca fica 100% silenciosa por causa de uma carta só, o risco residual mantém a tensão de exploração viva, e o custo de manter a carta ativa (bateria, slot) segue tendo contrapartida honesta em vez de virar botão de desligar o sistema.

**Ortogonalidade com o PEM:** PEM descoberto continua afetando SOMENTE o autosave (memória `project_save_dungeon_pem_faraday`); não toca a taxa de encontro. São dois sistemas separados que a mesma carta atravessa por caminhos distintos.

### 4.3 Ordem e teto

Os multiplicadores aplicam sobre a chance JÁ clampada da rampa (isto é, sobre o valor com teto 8%). Não há re-clamp em 8% depois do `multDificuldade` (um nível difícil PODE ultrapassar 8%; o teto desse caso pertence ao `ENCONTRO-FREQ-DIFICULDADE`, ver §4.1). O `multFaraday` só reduz, então nunca cria problema de teto.

---

## 5. Composição do encontro: 1 inimigo (decisão 4 do líder, 2026-08-12)

- **Nesta onda, todo encontro aleatório instancia exatamente 1 inimigo.**
- O schema do `EncounterProfile` já nasce com `group_size_min` / `group_size_max` como DADO por entrada (fixados em 1/1 agora), para a calibração futura de grupos não exigir migração de schema (mesmo cuidado append-only dos serializers `.gdt`). Grupos NÃO são implementados nesta onda: o motor lê o dado, e o dado diz 1.
- O motor de combate já suporta 1 a 4 inimigos (`combat.md` §2); a limitação é da onda de encontros, não do combate. Quando grupos entrarem, é mudança de DADO + calibração, não de código do director.

### Schema conceitual do `EncounterProfile` (DADO por área)

| Campo | Significado |
|---|---|
| `entries[]` | lista de entradas de encontro da área |
| `entries[].template_id` | qual `EnemyTemplate` spawna (ex.: Sentinela-Bit) |
| `entries[].weight` | peso relativo do sorteio de QUAL inimigo (sorteio 2, só após o sorteio 1 dizer "tem encontro") |
| `entries[].group_size_min/max` | tamanho do grupo (1/1 nesta onda; dado de calibração futura) |
| parâmetros da rampa | graça, comprimento da rampa, chance inicial e teto entram como dado do perfil, com os defaults canônicos 8 / 34 / 1% / 8% (área excepcional pode ter números próprios SEM código novo) |

Área SEM `EncounterProfile` (ou com perfil vazio) = zero encontros, por construção. Nenhuma flag `is_cidade` hardcoded: a segurança emerge do dado ausente (ADR-020, lugar é dado).

O spawn instancia via o caminho `EnemyTemplate → CombatActor`, aplicando o `difficulty_multiplier_for(EnemyTier, DifficultyLevel)` de HP/Atk já existente (`DIFICULDADE-TABELA-DADO`): este sistema é o primeiro call site de produção daquela tabela.

---

## 6. Estreia: cidade 100% segura (decisão 2 do líder, 2026-08-12)

- O sistema nasce **completo e testado nesta onda**, mas fica **invisível para o jogador** até a 1ª dungeon existir.
- A cidade continua sem NENHUM encontro aleatório: o `AreaDescriptor` da cidade simplesmente não carrega `EncounterProfile`. Não é toggle, não é chance 0 configurada: é ausência de dado.
- A estreia jogável do sistema é a **1ª dungeon**, que nasce já com seu perfil (inimigos, pesos, rampa default).
- Coerência com o guarda-chuva Faraday das dungeons (`FARADAY-DUNGEON-ITENS`): dungeon é onde encontro, PEM e a carta Faraday se cruzam; a cidade é o contraste seguro do setting bipartido.

---

## 7. Por que RAMPA, e não os modelos concorrentes

Alternativas consideradas na decisão 1 (registradas para a escolha não se re-litigar):

| Modelo | Como funciona | Por que NÃO |
|---|---|---|
| **Chance fixa por passo (Pokémon)** | p constante por tile em grama/zona | Memoryless: variância alta demais nas duas pontas. Encontro back-to-back logo após a batalha (a frustração clássica do gênero) E secas longas que matam a tensão. Ilegível: o jogador não extrai estrutura nenhuma (fere Pillar 1). |
| **Step counter uniforme (DQ/FF por zona, sorteio de N passos)** | sorteia "próximo encontro em N passos", N uniforme | Melhor variância que Pokémon, mas o jogador não tem NENHUMA janela garantida pós-batalha, e o modelo esconde o estado (o N sorteado é invisível, então não há leitura possível, só superstição). |
| **RAMPA com graça (escolhido)** | 8 tiles a 0%, depois linear 1% → 8% em 34 tiles, platô | Graça = respiro garantido pós-batalha (janela dura para curar/reorganizar, anti-frustração). Variância contida nas duas pontas (nunca back-to-back; o platô impede seca infinita de tensão). E é LEGÍVEL: risco crescente que um jogador analítico percebe e usa (Pillar 1, e casa com o público: o Gus resolve por leitura de sistema). O contador de graça É o mecanismo anti-repique que o levantamento do item pedia. |

O teto de 8% (em vez de crescer até forçar encontro) é deliberado: explorar com calma, fazer backtracking e ler o cenário continuam viáveis para sempre; o sistema pressiona, não pune.

---

## 8. Interações com sistemas canônicos

| Sistema | Interação |
|---|---|
| **Eixo de domínio (`combat.md` §19)** | O director só decide QUE há encontro e QUAL inimigo. O desfecho segue o eixo: selo Ouro resolve como auto-kill silencioso (conforme o toggle de 3 estados §19.5), Bronze/Prata monta arena com [Resolver sem encarar] disponível, sem selo encara. Auto-kill conta como desfecho e reseta a graça (§3). |
| **`INIMIGO-SOME-POS-FUGA-DERROTA`** | Encontro aleatório é GENÉRICO por definição: não existe entidade persistente no overworld para "sumir". A regra de sumiço vale para encontros fixos; aqui o reset de graça pós-fuga cumpre o papel de respiro. |
| **Fuga (`combat.md` §14)** | Fugir reseta a graça (desfecho de batalha): o jogador que foge ganha os 8 tiles para escapar de verdade. O custo da fuga continua sendo o do próprio combate (sem loot, sem Knowledge). |
| **PEM / autosave** | Ortogonal (§4.2): PEM afeta autosave, nunca a taxa de encontro. |
| **`DIFICULDADE-TABELA-DADO`** | O spawn-path deste sistema é o primeiro consumidor de produção do `difficulty_multiplier_for` (HP/Atk por tier × dificuldade). |
| **`ENCONTRO-FREQ-DIFICULDADE`** | Consome o slot `multDificuldade` (§4.1). Só é escrevível agora que este sistema existe. |
| **Save** | Nada persiste (§3). |
| **Determinismo / speedrun** | Com seed fixa, a sequência de encontros é função da rota (contagem de tiles + rolls). Encounter manipulation por TAS/speedrun é possível e ACEITA (single-player; é a categoria clássica do catálogo de exploits, registrada, não combatida). |

---

## 9. Sensação esperada (derivações da fórmula, calibrar em playtest)

Aproximações analíticas para ancorar a 1ª calibração (`//PLAYTEST`: validar quando a 1ª dungeon existir; duas casas, derivadas da fórmula, não medidas em harness ainda):

| Métrica (sem Faraday, Médio) | Valor aproximado |
|---|---|
| Chance de andar 16 tiles sem encontro | ≈ 87% |
| Mediana de tiles até o encontro | ≈ 29 a 30 tiles |
| Chance de chegar ao platô (42+ tiles) sem encontro | ≈ 21% |
| Média de tiles até o encontro | ≈ 32 tiles |
| No platô, espaçamento médio adicional | 12,5 tiles (1/0,08) |

| Métrica (com Faraday 0,5×) | Valor aproximado |
|---|---|
| Mediana de tiles até o encontro | ≈ 40 tiles |
| Chance de chegar ao platô sem encontro | ≈ 46% |
| Média de tiles até o encontro | ≈ 46 tiles |

Leitura de design: sem Faraday, um corredor de dungeon típico rende um encontro a cada ~30 tiles de movimento; com Faraday, ~46 (a carta compra ~50% mais chão andado por encontro, sem zerar a tensão). Se o playtest disser que a dungeon está sufocante ou vazia, os knobs são DADO do perfil (§5): graça, comprimento da rampa, chance inicial, teto.

---

## 10. Escopo de implementação (ponte para a onda de código)

- **`domain/encounter/` POCO** (módulo estreito novo, ADR-020): `EncounterProfile` (dado, §5), `EncounterDirector` (contador t + rolagem inteira §2.3 + reset §3 + multiplicadores §4). Zero dependência de framework; testável headless.
- **Rolagem** consome `IRandomSource` injetado (mesma porta do combate, ADR-006), com o contrato de contagem da §2.4.
- **Spawn-path `EnemyTemplate → CombatActor`**: novo, aplica `difficulty_multiplier_for` no HP/Atk do template (fecha a lacuna de wiring registrada em `DIFICULDADE-TABELA-DADO` e `MIRA-PONDERADA-PROD`).
- **Sorteio de QUAL inimigo** (entre `entries[]` por `weight`): 1 consumo de RNG adicional, SÓ quando o sorteio 1 deu encontro (documentar na contagem de teste).
- **Todo efeito loga** (regra da casa): tile rolado com encontro loga diegético; a graça e os rolls silenciosos não poluem o log de jogo (ficam atrás de flag de debug, como o harness faz).
- TDD vermelho→verde + mutation testing adversarial por agente independente, padrão das ondas de motor.

---

**Última revisão:** 2026-08-12 (criação do doc, item `ENCONTRO-ALEATORIO-SISTEMA`: 4 decisões do líder via AskUserQuestion em 2026-08-12. Decisão 1: rampa estilo FF, graça de 8 tiles a 0%, rampa linear de 1% a 8% ao longo de 34 tiles (tiles 9 a 42), platô de 8% do tile 43 em diante; a fórmula de trabalho do brief foi corrigida na canonização porque dava 1% dentro da graça e 1,206% no primeiro tile rolável, contrariando os próprios termos da decisão ("8 tiles com chance zero" e "cresce de 1%"); a forma canônica é a da §2.1, com a forma inteira exata de base 6600 na §2.3. Decisão 2: cidade 100% segura por ausência de dado, estreia na 1ª dungeon. Decisão 3: Faraday reduz a taxa pela metade (0,5× multiplicativo), não suprime; definitivo. Decisão 4: 1 inimigo por encontro nesta onda, `group_size` no schema como dado para o futuro.)
