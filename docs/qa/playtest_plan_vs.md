# Plano de Playtest — Vertical Slice
**Status:** Proposta para ratificação. Sprint solo 2026-06-04. F2-QA.4/F2-D.7.
**Cross-ref:** gdd.md §8, onboarding-vs.md §7/§8, plano_vs.md M.3.

**Convenção:** pt-br. Termos de game-dev no original (time-to-fun, telemetria, facilitador, gargalo, gate, telegraph). Sem em-dash.

---

## 1. Charter

**Objetivo:** validar a métrica primária do GDD §8: **time-to-fun ≤ 5min**. Provar que um jogador novato fecha o core loop (scan -> compilar -> prever -> combo -> resolve o Sentinela-Bit) sem briefing e sem nenhum modal de texto (anti-pillar wall-of-text).

**Escopo (o VS):** 1 área-cidade em blockout + 1 combate-tutorial (só Sentinela-Bit, DA-1) + 1ª patrulha do puzzle Gambito. Build de release exportado (M.4), não editor.

**O que NÃO é:** não é teste de bug nem QA funcional (zero crash já é exit-criterion do M.1, fora daqui). Não é teste de balance numérico (D.1) nem de perf (M.2). Este é teste de **fun e onboarding**: o objeto medido é o tempo e o ponto de fricção do aprendizado, não a corretude do código.

## 2. Perfil dos testadores (N=3 familiar, GDD §8)

Time fixo canônico (memória project_inspiracoes_pessoais). **Sem briefing prévio**: cada um joga frio, como jogador novato. O facilitador NÃO ensina.

| Testador | Papel | Por que importa |
|---|---|---|
| **Gus Dragon** (filho, 11 anos) | jogador-alvo | idade canônica do Gus; referência de tom e de skill-floor real (Pillar 4) |
| **Iago** (irmão, eng. sênior) | jogador analítico | testa se o sistema "lê" para quem decompõe rápido; risco oposto (otimiza cedo demais) |
| **Petrus** (criador) | jogador + facilitador | conhece o design; roda por último para não enviesar; útil como baseline de teto |

Ordem sugerida: Gus Dragon e Iago primeiro (frios de verdade), Petrus por último. Cada um joga isolado (sem assistir os outros antes da própria run).

## 3. Métrica primária

**time-to-fun** = tempo do `SESSION_START` até `SENTINELA_DEFEATED`, **tendo executado os 4 verbs ao menos 1x cada** (scan-combate, compilar, prever, combo). Cronômetro de **gameplay real** (exclui pausas, menus, leitura fora de fluxo).

- **Alvo primário:** ≤ **5:00** (GDD §8, onboarding §7 timeline 0:00–5:00).
- **Alvo secundário:** cruzar a **1ª patrulha** (`FIRST_PATROL_CROSSED`) ≤ **6:00** (onboarding §8).

Secundárias não bloqueantes (GDD §8): "would play again" ≥ 70% (2/3 já satisfaz).

## 4. Instrumentação (contrato de medição para G.5/G.8)

A medição NÃO pode depender de cronômetro manual no relógio: o facilitador observa comportamento, mas o **tempo é instrumentado**. O jogo emite eventos com `timestamp` (ms de gameplay, exclui pausa) para um log da sessão. **Isto é um contrato:** quando o `CombatManager` (G.5) e o `ExploreController`/save (G.8) forem construídos, eles JÁ devem emitir estes eventos. Ancorados nos buses que o combat.md §16 já define (`CombatBus`/`PlayerBus`), portanto custo de implementação baixo.

### 4.1 Eventos de telemetria automática (o entregável)

Formato: **1 CSV por sessão** (`docs/qa/runs/<testador>_<data>.csv`), 1 linha por evento, append-only. Colunas: `timestamp_ms_gameplay, event, payload`. Emitir via um `TelemetrySink` simples plugado nos buses (debug-build / flag de playtest; nunca em release final do jogador).

| Evento | Quando dispara | Fonte (bus/sistema) | Marca |
|---|---|---|---|
| `SESSION_START` | 1º input do jogador no overworld | ExploreController (G.EXPLORE) | t=0 do cronômetro |
| `FIRST_SCAN_OVERWORLD` | 1º scan no nó de lore (degrau 1) | ExploreController / Scan-overworld | verb-raiz aprendido |
| `COMBAT_STARTED` | entra no combate Sentinela (wipe) | `CombatBus.CombatStarted` (já existe) | fim da fase exploração |
| `FIRST_CARD_COMPILED` | 1ª carta jogada com sucesso (degrau 2) | `CombatBus.ActionResolved` (action=UseCard) | verb compilar |
| `FIRST_SCAN_COMBAT` | 1º Scan em combate, revela fraqueza (degrau 3) | `CombatBus.ActionResolved` (action=Scan) | verb scan + trade-off AP |
| `FIRST_GAMBIT_PREVIEW` | 1º Prever, lê IntentPreview (degrau 4) | `CombatBus.ActionResolved` (action=Predict) | verb prever |
| `FIRST_COMBO` | 1ª receita casada `COMPILADO: X` (degrau 5) | `CombatBus.ActionResolved` (comboId != null) | verb combo (composição) |
| `COMPILE_ERROR` | cada `ERRO DE COMPILAÇÃO` (combat.md §10) | `CombatBus.ActionResolved` (rejeitada) | payload = tipo do erro; conta repetição |
| `SENTINELA_DEFEATED` | Sentinela-Bit a HP0; `CombatEnded` vitória | `CombatBus.CombatEnded` (outcome=Victory) | **stop do time-to-fun primário** |
| `FIRST_PATROL_CROSSED` | 1ª travessia da patrulha (degrau 6) | ExploreController / puzzle Gambito (G.6) | stop do alvo secundário ≤6:00 |
| `SESSION_END` | save manual ou fim da sessão | save/load (G.8) / `PlayerBus` | encerra log |

**Regras do contrato:** (a) todo evento carrega `timestamp_ms_gameplay` que NÃO avança durante pausa/menu; (b) eventos `FIRST_*` disparam **uma única vez** por sessão (o 1º contato é o que mede aprendizado); (c) `COMPILE_ERROR` é o único repetível e seu payload distingue o tipo (mana/AP/alvo/Null-sem-Scan/pipeline-cheia, §10) para detectar "regra não ensinada pelo erro" (onboarding §8); (d) os nomes de `action` no payload (`UseCard`/`Scan`/`Predict`/`Reorder`) seguem a §12/§16 do combat.md, não inventar.

### 4.2 Observação manual do facilitador (complementar, não substitui)

A telemetria dá **quando**; o facilitador registra **por quê** travou (§5). Os dois juntos fecham o diagnóstico. Sem a telemetria, "achei que demorou" não é falseável; sem a observação, um CSV não diz onde o jogador olhou perdido.

## 5. Roteiro de observação (observar, NÃO perguntar durante)

Regra de ouro (GDD §8 / onboarding §8): durante a run, **não interferir, não ensinar, não perguntar**. Intervenção verbal do facilitador = run falha (plano_vs.md M.3). Registrar por degrau (folha de observação):

- **Trava (>30s parado sem ação significativa)** em qualquer degrau -> marcar o degrau. Esse é o gargalo do onboarding.
- **Pede ajuda verbal** ("e agora?", "o que eu faço?") -> marcar degrau + transcrever a frase.
- **Ataca neutro a batalha inteira sem nunca scanear** (nenhum `FIRST_SCAN_COMBAT` antes de `SENTINELA_DEFEATED`) -> o gate borrado (onboarding §3) falhou.
- **Trata o puzzle-Gambito como mecânica nova** (hesita, não reconhece que é "o mesmo prever", onboarding §6) -> carga cognitiva mal-encadeada.
- **Spam de `COMPILE_ERROR` do mesmo tipo** -> a regra não está sendo ensinada pelo erro, precisa de affordance melhor (não de texto).

## 6. Hipótese falseável + critério de REFAZER onboarding (GDD §8, onboarding §8)

**Hipótese primária (falseável):** "Um jogador novato (Gus Dragon / Iago / Petrus, sem briefing) executa o core loop completo (scan -> compilar -> prever -> combo -> resolve o Sentinela-Bit) em ≤ 5:00 de gameplay real, cruza a 1ª patrulha em ≤ 6:00, sem que nenhum modal de texto seja necessário."

**Dispara REFAZER o onboarding (qualquer um dos três):**
1. time-to-fun **> 5:00 em ≥ 2 dos 3** testadores, OU
2. algum degrau é **gargalo (>30s travado) para ≥ 2 dos 3**, OU
3. **≥ 1 testador vence o combate sem nunca ter scaneado** (gate borrado falhou).

**Resposta ao fail:** revisar a **ordem das mecânicas** (onboarding §2) e/ou os **gates diegéticos** (§3). **NUNCA adicionar tutorial-texto / pop-up / modal** (anti-pillar wall-of-text, Pillar 4). O fix é de design de espaço, não de instrução.

## 7. Pós-sessão (não bloqueante)

Só DEPOIS de encerrada a run (nunca durante):
- 1 pergunta aberta: **"O que você achou que precisava fazer e não conseguiu?"** (revela o modelo mental do jogador vs o pretendido).
- **"Would play again?"** (sim/não) -> alimenta a secundária ≥70%.

**Consentimento:** Gus Dragon é menor de idade (11) -> consentimento do responsável, que é o próprio Petrus (criador e pai). Registrar uma linha de consentimento no log da sessão de Gus Dragon. Sem gravação de rosto; captura de tela do gameplay (OBS) e CSV bastam.

## 8. Cadência e registro

- **Gate de disparo:** rodar quando **F2-M.3** estiver pronto para teste, ou seja, M.1 fechado (VS coeso, save/load) + onboarding diegético D.6 implementado + build M.4 distribuível. NÃO exige arte final (placeholder-first, plano_vs.md §2).
- **Quantas sessões:** 1 rodada de 3 sessões (1 por testador) por iteração de onboarding. Se a hipótese falhar (§6) e o onboarding for refeito, **nova rodada de 3** após o ajuste. Repetir até 3/3 dentro do alvo.
- **Registro:** CSV por sessão em `docs/qa/runs/`; 1 nota-resumo por rodada em `docs/qa/playtest_results_<data>.md` (time-to-fun de cada um, degrau-gargalo se houver, veredito pass/refazer, decisão de ajuste). Vídeo OBS opcional anexado/linkado, não obrigatório.

---

## Decisões Abertas

| ID | Questão | Recomendação |
|---|---|---|
| **PT-1** | `timestamp_ms_gameplay` deve excluir pausa via clock próprio do jogo, ou registra wall-clock e o facilitador anota as pausas? | **Clock de gameplay no engine** (pausa não avança). Mais barato medir e impossível de enviesar; o `TelemetrySink` lê o mesmo relógio do loop. Confirmar com G.8. |
| **PT-2** | O `TelemetrySink` (flag de playtest) entra já no M.1 ou só num build dedicado de M.3? | **Plugar cedo (M.1)** atrás de flag desligada por default: o contrato §4.1 guia G.5/G.8 desde a construção e evita retrofit. Decisão de engenharia (flag opt-in), levar ao criador se houver custo de escopo. |
