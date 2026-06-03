# Puzzle Vetor do Gambito — Spec de Design
**Status:** Canônico. Ratificado Sprint 2 W2 2026-06-03.
**Cross-ref:** GDD §6.3, combat.md §12, pillars.md.

Materializa GDD §6.3 ("mesmo tabuleiro decifra movimento de patrulhas... e prevê trajetória de projéteis ambientais") fora do combate. Serve Pillar 1 (lógica vence força), Pillar 2 (sistema determinístico legível), Pillar 3 (triângulo de hardware), Pillar 4 (prodígio de 11, vence por predição). Convenção: pt-br; sem em-dash.

---

## 1. Verb, vértice e diegese

- **Verb:** **prever** (idêntico ao Gambito-combate) aplicado ao ESPAÇO + ao TEMPO, não à fila de turnos. Sub-verbo: **planejar a própria rota** (o jogador move o Gus, não o adversário).
- **Vértice primário:** Óculos Táticos projetam o **mini-board holográfico** sobre o cenário real.
- **Manifestação diegética:** Gus para, ativa os Óculos, e o mundo ganha sobreposição em wireframe ciano (grid ortogonal, Pillar 5 cidade). As patrulhas viram peças com vetor de movimento previsto. Gus traça sua travessia como um problema de xadrez resolvido no papel antes de mexer a peça (Pillar 4: não age por reflexo, planeja).

## 2. Regras formais

- **Estado inicial:** grade de células discretas (cidade = grid ortogonal). Gus em célula de entrada; objetivo em célula de saída; N entidades patrulhando em **ciclo fixo conhecido** (loop determinístico, sem RNG: Pillar 2).
- **Transição:** o tempo do puzzle é **discreto por passos** ("ticks"). 1 passo do Gus = 1 passo de cada patrulha no seu ciclo. Tudo se move em lockstep e é totalmente previsível.
- **Win-condition:** Gus alcança a célula-saída sem ser detectado (entrar na **célula-de-detecção** de uma patrulha = fail). Detecção = célula ocupada pela patrulha + cone/raio de visão dela naquele tick.
- **Fail-condition:** ser detectado. Consequência mecânica = **reset suave** (D2 canon 2026-06-03): volta à entrada, patrulhas resetam, Gus sem dano. Puzzle 100% cognitivo.
- **Solução única ou múltiplas rotas:** projetado para **múltiplas rotas válidas** (≥2 caminhos), com 1 rota "elegante" mais curta. Solução única vira tentativa-e-erro; múltiplas rotas premiam leitura de sistema (Pillar 1).

## 3. Mini-board holográfico (lookahead)

- Ativar projeção mostra os próximos **K passos** de cada patrulha como rastro numerado (1, 2, 3, 5... Fibonacci sutil no estilo do rastro, easter egg canon).
- **K (D1 canon 2026-06-03): K=2 base; K=3 via upgrade da Matriz Ortodôntica** (Pillar 3). K=2 no VS — simples, legível, prodígio de 11 anos. K=3 como recompensa de upgrade de hardware mid-game.
- A projeção **não move o Gus**: é leitura. O movimento é input separado (verb "planejar a rota"). Isto separa cognitivamente leitura (Óculos) de ação (Tavus-Drive), Pillar 3.
- Sem timer no planejamento (Pillar 1: sem twitch). A pressão é espacial/lógica, não de reflexo.

## 4. Tipos de entidade previsível

| Tipo | Setting | Comportamento | No VS? |
|---|---|---|---|
| **Patrulha** (NPC guarda/drone) | cidade | ciclo de ronda fixo + cone de visão | **SIM (VS usa este)** |
| Projétil ambiental | Selve | trajetória balística periódica (esporo/pólen-vírus) | fase posterior |
| Hazard físico | cidade | feixe/porta/prensa em cadência fixa | fase posterior (camada de tempero) |

O VS entrega **só patrulha** (área-cidade canônica, plano_vs §1/§4). Os outros tipos reusam o mesmo motor de "entidade com trajetória determinística previsível"; dados já preparados, não implementados.

## 5. Dificuldade e escalabilidade

- **Eixo de complexidade:** (1) 1 patrulha → (2) 2 patrulhas independentes → (3) 2 patrulhas cujos ciclos se cruzam (janela de passagem curta) → (4) patrulha + hazard de cadência → (5) entidade que **reage** ao Gus (fase posterior; quebra determinismo puro com cuidado).
- **Curva:** crescer por **adição de entidades e estreitamento de janela**, nunca por aumentar K exigido nem por velocidade. Mantém skill-floor baixo, skill-ceiling alto.
- **No VS:** **nível 1-2** (1 patrulha de ensino → 1 puzzle real com 2 patrulhas e janela legível). 1 layout jogável (§8).

## 6. Distinção de combate (anti-sobreposição cognitiva)

| Eixo | Gambito-COMBATE (combat.md §12) | Gambito-PUZZLE (este doc) |
|---|---|---|
| Domínio | fila de iniciativa (ordem de turnos) | espaço (células) + tempo (passos) |
| Operações | Prever intent + Reordenar (`ReorderActor`) | Prever rota das patrulhas + mover o Gus |
| Recurso | AP (1 / 2) dentro do turno | sem custo (D3 canon: puzzle 100% cognitivo) |
| Quem se move | o adversário (manipulado) | o próprio Gus (planejado) |
| Falha | dano / KO | detecção → reset suave (D2 canon) |

Design anti-confusão: vocabulário de UI distinto ("rota" / "ronda" / "passos", nunca "turno"/"fila"); board ortogonal do puzzle x fila linear do combate; o puzzle nunca cobra AP nem mana. Mesmo verb (prever), contextos visualmente e mecanicamente separados.

## 7. Ensino diegético (time-to-fun ≤ 5min)

- **Just-in-time, zero wall-of-text** (anti-pillar). 1ª patrulha = sandbox protegido: 1 guarda, ronda curta, sem fail real (se detectado, NPC só resmunga e Gus recua 1 célula).
- A projeção dos Óculos **liga sozinha na primeira vez** (Gus narra 1 linha: "Padrão de ronda... previsível."). O jogador descobre que mover-no-tempo-certo evita o cone. Ensina por fazer, não por ler.
- Encadeado ao onboarding do VS (F2-D.6): scan → compilar → prever-combate → **prever-puzzle** reforça o verb "decifrar" num 2º contexto, barateando o aprendizado.

## 8. Layout jogável de referência (blockout)

Grid 7×5. `G`=Gus entrada, `X`=saída, `#`=parede, `.`=livre, `P1/P2`=patrulhas, `→/↓`=sentido inicial do ciclo. Ciclos: P1 vai-e-volta horizontal (cols 2-4, row 2); P2 vai-e-volta vertical (col 5, rows 1-3). Cone de visão = 1 célula à frente no sentido atual.

```
col→  0   1   2   3   4   5   6
row0  #   #   #   #   #   #   #
row1  G   .   .   .   .  P2↓  X
row2  #   .  P1→ .   .   .   #
row3  #   .   .   .   .   .   #
row4  #   #   #   #   #   #   #
```

Rota elegante: aguardar P1 afastar (passo onde P1 está em col 4), cruzar row2→row1 quando P2 está no extremo oposto do seu ciclo, entrar em X. ≥2 rotas válidas (alternativa por row3). Level-designer pode reusar a grade trocando ciclos/janela para escalar (§5).

## 9. Integração com o triângulo de hardware (Pillar 3)

- **Óculos Táticos:** projetam o board e o rastro de K passos (input/leitura). Sem Óculos = sem puzzle (coerência Pillar 3).
- **Matriz Ortodôntica:** amplifica **range** (quantas patrulhas o board mostra de uma vez) e **detalhe** (revela cone de visão exato; upgrade revela K+1). Espelha o papel da Matriz no combate (range/amplificação).
- **Tavus-Drive:** executa o **passo planejado** (confirma a rota traçada) e habilita interação com 1 célula-objeto (ex: terminal que pausa 1 patrulha por 1 passo) usando carta ambiental (GDD §6.2 duplo-uso). Sem custo de recurso (D3 canon 2026-06-03): puzzle 100% cognitivo, coerente com "sem AP fora de combate".
- T6 Anomalia Perlin (combat.md §18.4) é o **anti-puzzle** canônico: onde o board retorna ruído, a predição falha. Reservado pós-VS / Selve profunda.

## 10. Restrições honradas

Determinístico, sem RNG punitivo (Pillar 2). Sem twitch (Pillar 1: pressão é lógica). Hardware-triad presente (Pillar 3). Complexidade de 11 anos (K baixo, janelas legíveis, Pillar 4). Stealth aqui é **modo de puzzle pontual, não sistema obrigatório** (respeita corte GDD §9). No VS, caminho único (D4 canon 2026-06-03): todos N=3 testadores validam o Gambito-puzzle. Rota alternativa de combate no jogo final. Sem open-world, sem crafting profundo.

---

## Decisões Canonizadas (Sprint 2 W2 2026-06-03)

| ID | Decisão | Escolha |
|---|---|---|
| D1 | Lookahead K e custo | **K=2 base; K=3 via upgrade da Matriz Ortodôntica** (Pillar 3) |
| D2 | Consequência de falha (detecção) | **Reset suave** — volta à entrada, sem dano, patrulhas resetam |
| D3 | Custo do verb em exploração (Tavus-Drive) | **Sem custo** — puzzle 100% cognitivo |
| D4 | Stealth no caminho dourado | **Caminho único no VS** (jogo final: rota alternativa de combate) |
