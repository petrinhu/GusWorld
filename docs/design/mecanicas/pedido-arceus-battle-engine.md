# PEDIDO REGISTRADO — Engine de batalha dinâmica estilo Pokémon Legends: Arceus (Action Speed / ticks)

**Status:** REGISTRADO 2026-07-16, aguardando o líder voltar do reboot pra resolver. **NÃO iniciar design/impl** até o líder abrir a pauta. Origem: pedido/dúvida do próprio **Gus Dragon** (relay do líder), endereçado ao dev de gusworld (esta sessão).

**Flag de escopo:** isto é uma **decisão one-way-door de FEEL de combate** (muda a ordem de turnos de CTB-clássico pra action-clock dinâmico com 3 estilos). Tratar como brainstorm CUIDADOSO com o líder (não autônomo). Cruza com o combate CTB atual (`combat.md`, `combat-flavor.md` cast-time/interpretada-lenta, item `CARTAS-CAST-TIME`, `COMBATE-TEORIA-JOGOS`, `battle-screen.md` D4 fila) e com a proteção do Gus central (threat modeling). Reconciliar com o que já existe antes de virar canon.

## Texto integral do pedido (verbatim, preservado)

```
para o dev de gusworld, um pedido/dúvida do PRÓPRIO Gus:

Crie uma engine de batalha turn-based dinâmica (não estritamente alternada 1x1) inspirada no sistema de Pokémon Legends: Arceus (2022), adaptada para um RPG com foco em estratégia profunda, agency do jogador e fluidez. O sistema usa Action Speed (velocidade de ação) baseada em um "action clock" / sistema de ticks/timers internos por participante. Cada Pokémon/personagem tem um timer de ação que diminui com o tempo/passagem de "ticks"; quando chega a zero (ou abaixo), ele age e o timer é resetado com base na sua Speed efetiva + modificadores da ação escolhida.

Descrição exaustiva do sistema core:
- Base inicial: A Speed stat determina o action time/ticks iniciais de cada combatente (Speed baixa = mais ticks para agir; Speed alta = menos ticks). O primeiro a chegar a zero age (desempates por "quem não agiu há mais tempo", depois Speed mais alta, depois preferência do player em empate total).
- A cada ação: A ação escolhida subtrai um valor do action time do usuário (ou adiciona delay). Depois da ação, o timer é resetado/ajustado para o próximo ciclo.
- Estilos de ataque (3 opções por movimento) — o jogador (e idealmente a IA) escolhe no momento do turno:
  - Normal: Dano/poder base. Modificador neutro de action speed. Ordem padrão.
  - Ágil (Agile): Dano reduzido (~50% ou configurável), status enfraquecidos. Aumenta action speed (reduz action time) -> age mais cedo. Pode gerar double-turn (com diminishing returns em consecutivos, pra evitar infinito). Em batalhas múltiplas, Ágil contra um alvo geralmente não acelera você antes dos outros inimigos.
  - Forte (Strong): Dano aumentado (+50% ou mais), melhor acerto/efeitos. Diminui action speed (aumenta delay) -> age mais devagar, dando brecha pro oponente ter mais turnos.

Exemplos de sequências (referência para impl e testes):
- Caso 1 (ambos Normal): P1 -> P2 -> P1 -> P2 -> P1 -> P2.
- Caso 2 (P1 Forte): P1-Forte -> P2 -> P2 -> P1-Forte -> P2 -> P2.
- Caso 3 (P1 Ágil): P1-Ágil -> P1 -> P2 -> P1-Ágil -> P1 -> P2 (double-turn controlado, com limite).
- Caso 4 (ambos Forte): P1-Forte -> P2-Forte -> P1 -> P2 -> P1 -> P2.
- Caso 5 (P1 Forte x2): P1-Forte -> P2 -> P2 -> P1-Forte -> P2 -> P2 -> P2 -> P1.
- Caso 6 (P1 Forte + P2 Ágil): P1-Forte -> P2-Ágil -> P2 -> P2 -> P1 -> P2.

O sistema mostra visualmente a ordem de ações atual/futura (barra/lista no canto, atualizada dinamicamente). MAS o preview original é impreciso porque assume que o inimigo usa ataque genérico sem modifiers. Na realidade, buffs (Calm Mind, Sword Dance), moves com priority (Quick Attack, Mach Punch) e estilos do inimigo alteram tudo de última hora.

Trade-offs centrais (balanceáveis via parâmetros):
- Mais dano por hit (Forte) = menos ações = risco de ficar "preso" vendo o oponente agir.
- Mais frequência (Ágil) = menos dano = bom pra setup/chain/pressão/limpar fraquezas, ruim pra burst ou tanques.
- Normal = equilíbrio seguro sem vantagem clara.
- 1x1: boa tensão estratégica. Multi/selvagens: mais complexidade e risco de player sem agency.
- Agency vs dinâmica: recompensa gerenciar Speed + estilo + previsão; pune falta de info ou Speed baixa.

Problemas identificados (críticas da comunidade a endereçar/mitigar):
- Previsibilidade baixa -> "o jogo decide quando eu posso jogar".
- Preview nem sempre confiável (ignora estilos/buffs/moves especiais do inimigo).
- Multi/alfas: player travado sem turnos por muito tempo.
- Inconsistências (turnos extras após miss, KO, buff).
- Letalidade alta -> frustração.

Opções de resolução (priorizar as que mantêm o core dinâmico, não voltar pra turnos fixos):
1. Transparência drástica do preview + UI: mostrar ordem atual + variações ("se inimigo usar Ágil/Forte/buff -> ordem X"); ícones/tooltips de modifiers; indicar double-turn e quando o preview é "incerto". ALTA PRIORIDADE. Camada extra de simulação leve. Aumenta agency via informação.
2. Padronizar e tornar explícitos os modifiers de speed: buffs sempre contam como "+Action Speed" com indicador; moves priority com modifier fixo/claro; reduzir casos ocultos que quebram o preview.
3. Mais ferramentas de controle/agency: moves com prioridade intrínseca; itens/abilidades que manipulam action time; "focar"/threat pra priorizar alvos em multi; limitar/sinalizar turnos extras pós-KO. Game theory (payoff matrices), evitar overbuff.
4. Letalidade e multi: scaling de threat por level; reduzir dano de fracos; mecânicas de foco/área em multi; retirada tática ou preview de threat. Priorizar proteção do "Gus" central.
5. Parâmetros configuráveis: modos "clássico dinâmico" / "alta transparência" / "híbrido"; expor damage multipliers por estilo, speed modifiers, limite de Ágil consecutivo, diminishing returns, desempates, comportamento da IA. Começar em alta transparência + boa agency. IA via Expectiminimax/MCTS.
6. Edge cases: ticks robusto (múltiplos participantes, switches, KOs — próximo herda modifiers?); limite de Ágil consecutivo (ex.: máx 2); ties + "quem não agiu há mais tempo"; integração com buffs/status que afetam speed; performance (pré-calcular / atualizar só quando muda); testes 1x1 vs multi vs boss. Modularidade: cada estilo/move com parâmetros claros.

Requisitos: Action Speed/action clock nativo; UI clara e atualizável (atual + previsão); IA que escolhe estilos inteligentemente; fácil iterar balance (multipliers/regras expostos); preservar fluidez/estratégia do original mas mitigar frustração de perda de agency; objetivo: dinâmico, recompensador pra quem gerencia Speed+estilos, justo e transparente pro jogador nunca se sentir "sem jogar".

Entregar: arquitetura inicial (classes/estruturas), pseudocódigo de cálculo de ordem + aplicação de estilos, e quais opções priorizar pra Gusworld. Funcionar em 1x1 e escalar pra multi sem frustrar; profundidade tática (game theory, threat modeling protegendo Gus, variance controlada).

Gus Dragon (continue o bom trabalho)
```

## Notas pra quando o líder abrir a pauta (NÃO agir antes)
- Reconciliar com o combate atual: hoje o GusWorld já é CTB (fila de iniciativa, `initiative_queue`) + já tem cast-time compilada/interpretada (`combat-flavor.md §1-2`, o "interpretada = resolve casas à frente na fila" é PARENTE do action-clock). Ver se este pedido EVOLUI o CTB atual ou o SUBSTITUI (one-way-door de feel).
- Cruza com itens vivos: `CARTAS-CAST-TIME`, `COMBATE-TEORIA-JOGOS`, `MODOS-MORTE` (letalidade), `battle-screen.md` D4 (fila de 5), threat/proteção do Gus.
- Provável squad quando abrir: `lead-game-designer` (mecânica) + `software-architect` (ADR do action-clock) + `gameplay_engineer`/`backend-engineer` (impl) + `economy-designer`/game-theory (balance) — mas SÓ após decisão do líder.
