# Onboarding do Vertical Slice — Spec de Design
**Status:** Canônico. Ratificado Sprint 3 W2 2026-06-03. F2-D.6.
**Cross-ref:** GDD §4 (core loop), §6 (mecânicas-âncora), §8 (time-to-fun ≤5min N=3); combat.md §3/§5/§10/§12/§13/§17; core-loop-exploracao.md §3/§4/§5; puzzle-gambito.md §7/§8; knowledge-progression.md §8; pillars.md (P1, P4, anti-pillar wall-of-text).

**Convenção:** pt-br. Termos de game-dev no original (onboarding, just-in-time, time-to-fun, affordance, telegraph, gating, sandbox). Sem em-dash; usa vírgula, parênteses, dois-pontos.

---

## 1. Filosofia de ensino (just-in-time diegético)

O onboarding NÃO é um modo separado: é o início da campanha jogado uma única vez, onde cada mecânica entra **no instante em que o espaço a exige** (just-in-time). Três regras invioláveis (anti-pillar wall-of-text):

- **Ensina por fazer, nunca por ler.** Toda mecânica é introduzida por affordance visual (o sistema mostra que algo é interativo) + no máximo 1-2 linhas de fala do Gus em barra diegética (STUB de intent, prose final = narrative-writer). Zero pop-up de manual, zero modal explicativo bloqueante.
- **Diegese é o tutorial.** A interface JÁ é o triângulo de hardware (Pillar 3): "ensinar a usar os Óculos" = ensinar o jogo. Não há camada meta de "aperte X para Y" descolada do mundo.
- **Carga cognitiva crescente, um verb por vez.** O jogador aprende scanear, depois compilar, depois prever, depois combinar. Nunca dois verbs novos no mesmo momento.

**Por que serve Pillar 4:** Gus é um prodígio analítico de 11 anos que descobre padrões. O onboarding ENCENA isso: o jogador, como o Gus, deduz a regra observando o sistema reagir, não recebendo um briefing adulto. A descoberta é a fantasia. Pillar 1 (lógica vence força) exige que o 1º momento prazeroso seja uma dedução ("Elétrico derruba o Cinético"), não um acerto de reflexo.

## 2. Ordem de introdução das mecânicas (espinha dorsal)

Sequência por carga cognitiva crescente. Cada degrau só abre quando o anterior foi executado uma vez com sucesso.

| # | Verb novo | Onde | Carga | Por que aqui |
|---|---|---|---|---|
| 0 | mover + orbitar câmera | overworld cidade | motora | base motora antes de qualquer sistema (core-loop-exploracao §10 DA-1) |
| 1 | **scanear** (overworld) | 1º nó de lore | leitura passiva | scan grátis no overworld (sem AP): ensina o verb-raiz "decifrar" sem custo nem risco (core-loop-exploracao §3) |
| 2 | **compilar** 1 carta | combate-tutorial (só Sentinela-Bit) | ação | 1ª vez que jogar carta importa; mana=3 no turno 1 cobre 1 carta básica (combat.md §5) |
| 3 | **scanear** (combate, 1 AP) | mesmo combate | trade-off | revela fraqueza Elétrica; ensina o custo de AP (combat.md §12). O inimigo está "borrado" sem isto (§3) |
| 4 | **prever** (Gambito-combate, 1 AP) | mesmo combate | predição | lê IntentPreview do Sentinela-Bit (telegraph 100% legível, ScriptedBrain, combat.md §13) |
| 5 | **combo** (pipeline 2 slots) | mesmo combate ou 2º turno | composição | fecha o core loop: scan informou, prever confirmou, combo é a resposta composta (combat.md §10) |
| 6 | **prever** (Gambito-puzzle) | 1ª patrulha pós-combate | reforço em 2º contexto | mesmo verb, domínio espaço/tempo: barateia o aprendizado (puzzle-gambito §7) |

**Racional da ordem:** scanear primeiro porque é leitura passiva (menor carga, e é o verb-raiz do qual os outros dependem). Compilar antes de scanear-em-combate porque "jogar uma carta e ver dano" é o feedback mais imediato e gratificante (engata o jogador antes de pedir que ele gaste AP em informação). Scan-combate vem logo após para ensinar o trade-off de AP enquanto o engajamento está alto. Prever depois porque exige já entender que o inimigo tem intent. Combo por último no combate porque compõe os anteriores. Puzzle-Gambito fecha reforçando "prever" num 2º enquadramento, consolidando o verb dominante.

## 3. Gating diegético (corredor suave, não muro)

O jogo guia sem travar. Nunca um modal "você precisa fazer X"; sempre o espaço tornando a próxima ação a mais óbvia.

- **Gate de scan-combate (degrau 3):** o Sentinela-Bit aparece com a silhueta **visualmente borrada/glitchada** e a barra de fraqueza marcada `???`. Atacar funciona (dano subtrativo básico sempre disponível, combat.md §11), mas o feedback visual de "alvo ilegível" convida ao Scan. O jogador PODE atacar às cegas, só não otimiza. Corredor, não muro (Pillar 1: info, não obrigação).
- **Gate de combo (degrau 5):** a pipeline (combat.md §10) fica visível e vazia; ao soltar a 1ª carta num slot, a UI destaca o 2º slot disponível. Affordance, não instrução.
- **Gate de puzzle (degrau 6):** a patrulha bloqueia fisicamente o único caminho de saída da arena-cidade (puzzle-gambito §8 grid 7×5, caminho único no VS). A projeção dos Óculos **liga sozinha na 1ª vez** (puzzle-gambito §7). Impossível pular sem resolver, mas o fail é reset suave sem dano.
- **Anti-skip sem trava:** nenhum degrau exige um botão secreto. Todos são a ação mais natural dado o que o espaço apresenta. Acessibilidade: wait-mode default ligado (pillars.md P1), sem pressão temporal no onboarding.

## 4. O 1º encontro como tutorial (luva-de-pelica)

**Fronteira crítica (DA-1 canon 2026-06-03):** o combate-tutorial roteirizado é **só o Sentinela-Bit** (Trash, HP55). O Daemon-Guard (Elite, HP144) NÃO entra no time-to-fun: ele é o "degrau 2" logo após, o 1º combate de verdade. Encontro completo de 2 inimigos com TTK 3-5 turnos cada estoura os 5min (ver §7).

Roteiro do Sentinela-Bit como tutorial (ScriptedBrain, intent 100% legível, combat.md §13):

- **Telegrafia clara:** o Sentinela-Bit telegrafa um ataque de baixo dano (Cinético) com 1 turno de antecedência via IntentPreview. Postura + ícone + barra de mira (multimodal, Pillar 4 a11y).
- **Dano não-letal por design:** com Gus HP34, o ataque do Sentinela faz dano calibrado para NUNCA chegar a fatal no tutorial (e a Análise Preditiva 1×/batalha, combat.md §2.1, é a rede de segurança final). Espaço para errar sem game over.
- **Trade-off "AP em scan vs AP em ataque":** o jogador tem AP=3. A descoberta-chave: gastar 1 AP em Scan revela a fraqueza Elétrica do Sentinela, e Cauã (Elétrico, Atk14) fecha o combate em menos turnos. Sem Scan, o jogador atacaria neutro e levaria mais turnos. O jogo deixa ele SENTIR a diferença: a 1ª batalha sem otimizar é vencível; a dedução "Scan vale o AP" é a recompensa cognitiva (Pillar 1). KnowledgeKills=0 no VS (±30% variância, combat.md §17), Scan ainda custa AP (passivo só com 8 kills, knowledge-progression §8).
- **Composição ativa:** Gus + Cauã + Jaci. Jaci (Bioquímico/healer) existe para o jogador aprender que dano não é tudo; mas no tutorial o foco é Cauã exploitando a fraqueza.

## 5. Feedback de erro como ensino (combat.md §10)

O ERRO DE COMPILAÇÃO ensina as regras formais sem manual (Pillar 2: gramática com erros detectáveis). No onboarding, errar é didático, não punitivo:

- `ERRO DE COMPILAÇÃO: mana insuficiente (custa X, tem Y)` ensina o recurso mana sem nunca o ter explicado em texto.
- `ERRO DE COMPILAÇÃO: AP insuficiente` ensina o orçamento de ações.
- `ERRO DE COMPILAÇÃO: alvo inválido para <família>` ensina a relação família/alvo.
- `ERRO DE COMPILAÇÃO: Null requer Scan prévio` (se o jogador tentar Null cedo) ensina a pré-condição informacional, reforçando que Scan habilita ações.

O erro é texto curto + som curto (<150ms, GDD §4), nunca um modal bloqueante. Tentar e ver o erro É o tutorial da regra. Nenhuma regra de combate precisa de tooltip prévio porque o erro a ensina no momento do contato.

## 6. Encadeamento aos 3 sub-verbos e o reforço do "prever"

O core loop (GDD §4: scan → planejar → executar → feedback → progress) tem 3 sub-verbos (scanear / compilar / prever). O onboarding os apresenta sequencialmente (§2) e então o **puzzle-Gambito reforça "prever" num 2º contexto** (puzzle-gambito §7):

- No combate, **prever** = ler a fila/intent do adversário (domínio: turnos).
- No puzzle, **prever** = ler o ciclo da patrulha e planejar a própria rota (domínio: espaço+tempo).
- Mesmo verb, vocabulário de UI distinto ("ronda"/"rota"/"passos" vs "turno"/"fila", puzzle-gambito §6): evita confusão cognitiva, mas o jogador reconhece "é a mesma habilidade de decifrar". O 2º contexto barateia o aprendizado e prova que o verb dominante (decifrar) generaliza, fechando a sensação de "este jogo é sobre ler sistemas".

## 7. Orçamento de tempo (cabe em ≤5min)

Timeline-alvo do jogador novato (N=3 familiar). Tempos são limites superiores conservadores.

| Janela | Conteúdo | Verb introduzido | Acumulado |
|---|---|---|---|
| 0:00–0:45 | mover + orbitar câmera no 1º corredor | mover/câmera | 0:45 |
| 0:45–1:30 | chegar ao 1º nó de lore, scan-overworld, micro-toast `PADRÃO IDENTIFICADO` | scanear (overworld) | 1:30 |
| 1:30–2:00 | transição para combate Sentinela-Bit (<150ms wipe) + 1ª carta (compilar) | compilar | 2:00 |
| 2:00–2:45 | scan-combate revela fraqueza Elétrica; Cauã exploita | scanear (combate, trade-off AP) | 2:45 |
| 2:45–3:30 | Gambito-prever lê intent; combo 2 slots fecha o Sentinela | prever + combo | 3:30 |
| 3:30–4:15 | vitória, feedback de progress (stub no Diário, combat.md §16) | (consolidação) | 4:15 |
| 4:15–5:00 | 1ª patrulha (sandbox protegido), projeção liga sozinha, 1ª travessia | prever (puzzle) | **5:00** |

**Onde estão os riscos de estourar:**
- **R1 (alto):** se o combate-tutorial for o encontro completo (Sentinela + Daemon-Guard Elite HP144), estoura. Mitigação: só Sentinela no tutorial (DA-1).
- **R2 (médio):** jogador explora o overworld antes do 1º nó de lore. Mitigação: corredor curto, 1º nó de lore no caminho óbvio (level-designer F2-G.1).
- **R3 (médio):** combo de 2 slots confunde no 1º contato. Mitigação: degrau 5 aceita "1 carta simples" como sucesso mínimo; combo é demonstrado mas não obrigatório para sair do combate (DA-2).
- **R4 (baixo):** patrulha do puzzle com janela muito apertada. Mitigação: 1ª patrulha = 1 guarda, ronda curta, sem fail real (puzzle-gambito §7).

## 8. Hipótese falseável de playtest (N=3 familiar, GDD §8)

**Hipótese primária (falseável):** "Um jogador novato (Gus Dragon 11 / Iago / Petrus, sem briefing prévio) executa o core loop completo (scan → compilar → prever → combo → resolve o Sentinela-Bit) em ≤5:00 de gameplay real, e cruza a 1ª patrulha em ≤6:00, sem que nenhum modal de texto seja necessário."

**Métrica primária:** time-to-fun = tempo do start até o jogador ter executado os 4 verbs ao menos 1× cada e vencido o Sentinela. Cronômetro de gameplay (exclui pausas).

**O que OBSERVAR (não perguntar, GDD §8 / Pillar de playtest):**
- Em que degrau o jogador trava (>30s parado sem ação) ou pede ajuda verbal. Esse degrau é o gargalo do onboarding.
- Se o jogador descobre sozinho o trade-off "Scan vale o AP" (degrau 3) ou ataca neutro a batalha inteira. Se ataca neutro sem nunca scanear, o gate borrado (§3) falhou.
- Se o jogador entende que o puzzle-Gambito é "o mesmo prever" (verbaliza ou aplica sem hesitar) ou trata como mecânica nova (carga cognitiva mal-encadeada, §6).
- Erros de compilação disparados: quais e quantos. Muitos do mesmo tipo = a regra não está sendo ensinada pelo erro, precisa de affordance melhor.

**Pós-sessão (não bloqueante):** 1 pergunta aberta ("o que você achou que precisava fazer e não conseguiu?") + "would play again?".

**Critério de REFAZER o onboarding (GDD §8):**
- time-to-fun > 5:00 em ≥2 dos 3 testadores, OU
- algum degrau é gargalo (>30s travado) para ≥2 dos 3, OU
- ≥1 testador termina o combate sem nunca ter scaneado (gate borrado falhou).
Qualquer um destes dispara revisão da ordem (§2) e/ou dos gates (§3), não adição de tutorial-texto (anti-pillar).

---

## Decisões Canonizadas (Sprint 3 W2 2026-06-03)

One-way doors ratificados pelo criador supremo.

| ID | Decisão | Escolha |
|---|---|---|
| **DA-1** | Escopo do combate-tutorial (1 ou 2 inimigos no time-to-fun) | **Só Sentinela-Bit no tutorial.** Daemon-Guard é o 1º combate "real" logo após, fora do cronômetro de 5min. Encontro canônico §17 permanece intacto. Protege a métrica time-to-fun ≤5min (GDD §8). |
| **DA-2** | Combo obrigatório para vencer o tutorial? | **Demonstrado/incentivado, NÃO obrigatório.** Vencer com cartas simples basta. Skill-floor baixo (Pillar 4 prodígio acessível); combo é skill-ceiling. |
| **DA-3** | Ordem scan-combate vs compilar | **Compilar primeiro, scan-combate logo após.** Feedback de dano engata o jogador antes de pedir investimento de AP em informação. |
| **DA-4** | Fala do Gus: texto on-screen ou mudo? | **1-2 linhas curtas do Gus em barra diegética não-bloqueante** (STUB; prose final = narrative-writer). Disambigua sem virar wall-of-text; dá voz ao protagonista (Pillar 4). |
