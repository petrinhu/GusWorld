# Sistema de Deck e Mão (cartas comuns × especiais)

**Status:** DEFINIÇÕES-BASE do criador (2026-07-15); RESTO = brainstorm pendente (frente própria, não bloqueia o motor de efeitos das cartas). Registrado a partir da resposta do líder na decisão de escopo do Planck.

Cross-ref: [[project_sistema_cartas_technomagik]], [[project_repositorios_perdidos_canon]] (loot), `docs/design/mecanicas/cartas-technomagik.md`, `reference_techmagic_engine_impl` (motor de efeitos).

## 1. Duas classes de carta (decisão do criador)

- **Cartas COMUNS — para TODOS os personagens.** São **já compiladas** (equivalem a *apps prontos que qualquer pessoa usa*, sem precisar compilar). Cada personagem (Gus + os 6 companions) tem seu próprio conjunto de comuns.
- **Cartas ESPECIAIS — SÓ do Gus.** Abrange as **históricas (os 20 mestres do Codex)** + a **carta de Helon Tusk**. São cartas **"para compilar"**, e **compilar só o Gus faz** (canon: Gus = compilador universal, [[project_nome_gus_canon]]). Mantém a identidade dele intacta.
  - _(Nota: isso reconcilia a decisão anterior "Planck/Codex = Gus" — o MOTOR é agnóstico por-ator, a exclusividade é trava de CONTEÚDO: as especiais só entram no deck do Gus.)_

## 2. Definições canônicas (criador)

- **DECK** = o **total de cartas** que um personagem pode ter na **bolsa**. Ocupa **1 slot** (de inventário/bolsa).
- **MÃO** = o subconjunto do deck que pode ser **usado NA batalha**. A mão é **escolhida/fixada no momento da primeira ação** (a definir: primeira ação do personagem na cena, ou início da batalha). Possível: **gastar uma ação para "montar a mão"** (a confirmar).

## 3. Perguntas em aberto (BRAINSTORM pendente — item DECK-MAO-SISTEMA)

1. **Quais são as cartas comuns?** (brainstorm do catálogo de comuns — o que qualquer personagem pode usar; equivalem a "apps"). Quantas, quais efeitos, como se relacionam com as 5 famílias.
2. **Quantas cartas ESPECIAIS o Gus pode ter na mão?** As especiais são **EXTRAS** que somam ao total de comuns que todos têm (Gus = comuns + especiais na mão), ou competem pelo mesmo espaço?
3. **Tamanho máximo da mão por personagem = função das STATS?** (ex.: personagem com mais [INT? foco?] segura mais cartas na mão). Definir a fórmula.
4. **Montar a mão custa uma AÇÃO?** (gastar 1 turno/AP pra reconfigurar a mão no meio da batalha?).
5. **Tamanho do DECK** (bolsa) por personagem — fixo? cresce? limitado por slot?
6. Interação com o loot (repositórios perdidos = hardware/código vendável): cartas comuns caem de loot? como se adquire carta nova?
7. Interação com o **capacitor** (item que turbina elétricas) e outros itens de bolsa (slots).

## 4. Como conduzir

Frente de design (não código imediato). Brainstorm DIRETO com o criador ([[feedback_brainstorm_direto_sem_agentes]] — eu sugiro interativo; agentes só na execução pós-decisão). Cada decisão via AskUserQuestion. Casa com CARTAS-BALANCEAMENTO e o motor de efeitos já entregue (o efeito de cada carta é ortogonal ao sistema de deck/mão — o deck/mão é o meta-layer de quais cartas você traz e usa).
