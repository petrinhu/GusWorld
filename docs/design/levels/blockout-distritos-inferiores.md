# Blockout — Distritos Inferiores (Vertical Slice)
**Status:** Canônico (design). Ratificado Sprint 5 W3 2026-06-03. F2-G.1. Build de editor (`.tscn` graybox) = follow-up do criador.
**Cross-ref:** core-loop-exploracao.md §2/§3/§4/§5 (5 tipos de nó, Scan raio ~8m, gatilhos); onboarding-vs.md §2/§7 (ordem dos degraus + timeline ≤5min); puzzle-gambito.md §8 (grid 7×5, P1/P2); dialogue-tree-npc-intro.md §1 (Bertoldo na Praça da Compilação); combat.md §17 (Sentinela-Bit HP55 + Daemon-Guard HP144); gdd.md §6/§7; pillars.md (P5 hub+radiais); PLACES.md (Praça da Compilação canon, Periferia canon); plano_vs.md §4 MVV / M.1.

**Convenção:** pt-br. Termos game-dev no original (blockout, graybox, gold path, sightline, choke point, CSG, trigger, capsule). Sem em-dash.

> **Nota de escopo (honesta):** este doc é o blockout DESIGN. Não constrói a `.tscn`. Toda dimensão e nó está concreta o bastante para que o graybox no editor (follow-up) seja trabalho mecânico. Placeholder-first (F2-PROD.2): cápsulas/CSG, zero arte final.

---

## 1. Visão da área

Praça civil na borda baixa do Núcleo Metropolitano, descendo para a charneira da Periferia. Tom P5 ciber-gótico: pavimento tesselado de pedra Era 1 rachado, postes de neon ciano, fonte de latão Era 2 girando, holograma de Sterling piscando "continue" ao fundo. Cidade que funciona apesar de si mesma (env 01-cidade §1). **Não open-world** (anti-pillar): grafo curto hub+radiais, atravessável de ponta a ponta em ~5min. Tamanho-alvo: caixa de ~60×40m em escala Godot. **Entrada:** rampa norte (vem do Edifício Vance, save base). **Saída:** portão sul para a Periferia, bloqueado pelo puzzle-patrulha (caminho único, canon).

## 2. Mapa blockout (topologia)

```
        N (entrada: rampa do Edifício Vance, save point S0)
        │  corredor curto, sightline reta para a fonte (landmark)
        ▼
   ┌─────────────────[ HUB: Praça da Compilação ]─────────────────┐
   │  (B) Bertoldo   ● fonte latão (LANDMARK central, 8m alt)       │
   │   NPC @entrada     │  pavimento tesselado, +0m                 │
   │   x=10,z=8         │                                           │
   │                (L) terminal-placa Era 2  ← 1o nó de LORE       │
   │                    x=22,z=14 (no caminho óbvio pós-Bertoldo)   │
   │                                                                │
   │   ┌── ramo leste (opcional, P-expression) ──┐                 │
   │   │  (T) Terminal hack ambiental             │                 │
   │   │  x=44,z=10  abre atalho lateral p/ arena │                 │
   │   └──────────────────────────────────────────┘                │
   └────────────────────────┬───────────────────────────────────────┘
                            │ choke: escada 3-5-7 degraus desce -2m
                            ▼
                  [ ARENA REBAIXADA: pátio de combate ]
                   (C) Sentinela-Bit (tutorial) ── depois Daemon-Guard
                   x=30,z=28  arena ~16×12m, -2m, 2 cover boxes (1m)
                            │ única saída sul
                            ▼
                  [ CORREDOR-PUZZLE: portão da Periferia ]
                   (P) grid 7×5 patrulha (P1 horiz, P2 vert)
                   x=30,z=40  alinhado às células de 2m
                            ▼
        S (saída: portão sul p/ Periferia — fim do VS)
```

ASCII alternativo do grafo de nós (ordem pedagógica = ordem espacial):

```
S0/entrada → (B)NPC → (L)Lore → [choke escada] → (C)Combate → (P)Puzzle → S(saída)
                 └──── (T)Terminal ramo opcional ──┘ (atalho p/ flanco da arena)
```

## 3. Posicionamento dos 5 nós (concreto)

| Nó | Tipo | Coord (x,z,y) | Footprint | Gatilho | Scan revela (raio ~8m) |
|---|---|---|---|---|---|
| **(B) Bertoldo Caím** | NPC | 10, 8, 0 | cápsula 0.5r ×1.7h | indicador `...` flutuante; raio interação 2m | nenhum (social, core-loop §2) |
| **(L) placa Era 2** | Lore | 22, 14, 0 | box 1.5×0.3×2 | brilho sutil + ping Scan ao alcance | "decifrável" + dica de método; ensina Scan-overworld (degrau 1) |
| **(C) arena combate** | Combate | 30, 28, -2 | arena 16×12, -2m | aproximação <3m do Sentinela dispara wipe <150ms p/ FSM | silhueta hostil; fraqueza Elétrica só após 1o kill (knowledge §8) |
| **(P) puzzle Gambito** | Puzzle | 30, 40, 0 | grid 7×5 ×2m/cel = 14×10m | aproximar liga board holográfico; bloqueio físico da saída | vetor das patrulhas P1/P2, cone 1 célula à frente |
| **(T) terminal hack** | Terminal | 44, 10, 0 | painel 1×0.2×1.5 | painel inerte que Scan revela interativo; carta ambiental (GDD §6.2) | "interativo via carta X"; abre atalho lateral p/ flanco da arena |

Notas: (C) usa o encontro canônico combat.md §17 (Sentinela-Bit HP55 ScriptedBrain como tutorial isolado; Daemon-Guard HP144 só DEPOIS, fora do cronômetro de 5min, onboarding §7 DA-1). (P) reusa o layout de referência puzzle-gambito §8 sem alteração de ciclos no VS (caminho único, D4 canon).

## 4. Gold path (materializa a timeline ≤5min)

A ordem espacial É a ordem pedagógica (onboarding §7). Rota dourada, passo a passo:

1. **0:00-0:45 mover/orbitar** — spawn em S0 (rampa norte). Corredor curto reto; sightline aponta direto para a fonte (landmark) ao fundo. Pratica WASD + orbitar câmera num espaço seguro, sem hazard (DA-1 core-loop §10).
2. **0:45-1:30 NPC + Lore** — chega ao hub; **(B) Bertoldo** está logo à esquerda da entrada (indicador `...`), contextualiza e aponta "aquilo ali" sem dar solução (dialogue-tree §1). A 2-3 passos no caminho óbvio está **(L) a placa Era 2**: Scan-overworld liga, micro-toast `PADRÃO IDENTIFICADO`, 1o verb (scanear) aprendido.
3. **1:30-3:30 combate** — seguindo o único caminho largo, o choke (escada 3-5-7, desce -2m) afunila para a arena rebaixada. Aproximar de **(C) Sentinela-Bit** dispara o wipe. Combate-tutorial ensina compilar → scan-combate → prever → combo (degraus 2-5).
4. **3:30-4:15 respiro/progress** — vitória; pátio fica calmo, micro-stub no Diário. Vale de pacing.
5. **4:15-5:00 puzzle** — a única saída sul é **(P)** o corredor-puzzle: 1ª patrulha (sandbox, ronda curta, sem fail real, puzzle §7), board liga sozinho, 1ª travessia reforça "prever" em 2º contexto (degrau 6). Cruza para **S** = fim do slice.

**Rotas alternativas (P-expression, gold path intacto):**
- **Ramo leste via (T) Terminal:** quem scaneia o painel inerte e joga a carta ambiental abre um atalho lateral que entra na arena pelo **flanco** (ângulo de aproximação alternativo ao combate, encounter-design). Recompensa = posição tática + 1 fragmento de lore no Diário (branch com retorno, nunca beco vazio).
- **Puzzle é caminho único** (canon D4): a saída sul não tem desvio. Rota alternativa de combate fica para o jogo final (puzzle §10).

## 5. Pacing espacial (tensão × respiro × sightlines)

```
Tensão
  ▲                    ●(C) combate (climax do slice)
  │                   ╱ ╲      ●(P) puzzle (tensão cognitiva, sem dano)
  │        ●(L) lore ╱   ╲    ╱
  │       ╱(curiosi-      ╲  ╱
  │ ●(B) ╱  dade)          ●╱ respiro pós-vitória
  │╱ NPC
  ●──────────────────────────────── tempo
  entrada                          saída
```

- **Sightline de objetivo:** da entrada, o jogador VÊ a fonte (landmark) e, atrás dela, o portão sul iluminado (meta no horizonte). Anti-"jogador se perde" (risco R2 onboarding §7).
- **Sightline de encontro:** o choke da escada esconde a arena rebaixada até o jogador descer; revelação controlada (tensão na curva, sightline §princípio). O Sentinela é visível ao topo da escada (silhueta), nunca emboscada por trás.
- **Landmarks de navegação (3):** (1) fonte de latão central = âncora do hub; (2) holograma Sterling piscante ao fundo norte = "de onde vim"; (3) portão sul iluminado = "para onde vou". Navegação por landmark, não por minimapa (princípio).
- **Choke com propósito:** a escada 3-5-7 força a transição hub→combate e dá perspectiva descendente da arena (momento de leitura antes da luta). Não é corredor decorativo.

## 6. Hardware-triad no espaço (Pillar 3) + easter egg

- **Óculos (Scan-overworld, raio ~8m):** pinga (L) como decifrável, (T) como interativo, e o cone de detecção do Sentinela pintado no chão da arena. Fora do raio, nós = silhueta neutra (core-loop §3).
- **Matriz Ortodôntica:** amplifica alcance/filtra ruído da praça; destaca a anomalia escondida no padrão das fachadas (gancho de combate oculto opcional, não no MVV).
- **Tavus-Drive:** executa a carta ambiental em (T) (duplo-uso GDD §6.2).
- **Padrão matemático do bioma (easter egg pervasivo, NÃO narrado):** as fachadas do hub e os degraus do choke seguem Fibonacci. Choke = **3, 5, 7** degraus (maçonaria canon, não-Fibonacci, deliberado). Vãos das janelas das fachadas norte em proporção 1:1:2:3:5. Rastro do board do puzzle numera 1,2,3,5 (puzzle §3). Densidade sutil ~10-15%; leitor familiar reconhece, leigo não nota.

## 7. Notas de geometria graybox (CSG, para o editor)

Escala canônica (core-loop-exploracao §3 calibra em F2-G.EXPLORE):
- Player width ~0.5m (cápsula 0.5r ×1.7h). Wall height 3m. Cover height 1m. Door/portão width ~2m. Célula de puzzle = 2×2m (grid 7×5 = 14×10m). Escada do choke = 5 degraus visuais agrupados em rampa de colisão (suave para navmesh).
- **Piso navegável:** CSGBox cinza médio, +0m no hub, -2m na arena (rampa de transição via escada). Colisão estática (StaticBody3D + CollisionShape box).
- **Bloqueios/paredes:** CSGBox 3m alt, material preto flat = sem-passagem. Bordas do mapa fecham a caixa 60×40.
- **Hazard:** nenhum hazard de dano no overworld do VS (puzzle é fail=reset suave, sem dano).
- **Nós interativos:** Area3D + trigger por tipo. (B) raio 2m; (C) raio 3m de aproximação; (L)/(T) raio ~8m de Scan; (P) Area3D que cobre a entrada do corredor-puzzle.
- **Cover na arena:** 2 CSGBox 1m alt (azul graybox) para 60-70% de tempo-em-cover possível (encounter checklist); 1 vantage point leve (+1m) acessível ao jogador pelo flanco via (T).

## 8. Cápsula de teste do loop (fecha M.1)

Esse blockout fecha o loop end-to-end de M.1 (plano_vs §1) numa sessão de 5-10min:
**mover** (corredor norte) → **NPC** (Bertoldo, hub) → **lore** (placa, Scan) → **combate** (Sentinela na arena rebaixada) → **puzzle** (patrulha no corredor sul) → atravessa para S e **(save)** no portão. Tudo contínuo, sem reload, navegável por landmark. Materializa a timeline de onboarding §7 e a sequência MVV de plano_vs §4. Daemon-Guard entra como 1º combate "real" pós-tutorial, fora do cronômetro de 5min.

---

## Decisões Canonizadas (Sprint 5 W3 2026-06-03)

| # | Decisão | Escolha |
|---|---|---|
| **DA-1** | Topônimo "Distritos Inferiores" | **Canonizado como sub-local novo** em PLACES.md (Sub-local Núcleo↔Periferia: borda baixa do Núcleo descendo à charneira da Periferia, engloba a Praça da Compilação + descida sul). Topônimo próprio, não apelido. |
| **DA-2** | Save points | **Entrada (S0) + portão sul.** Save na base + pós-puzzle; fecha o loop M.1 save/load testável sem save em zona tensa. |
| **DA-3** | Daemon-Guard (HP144) | **Spawn na arena, ativado pós-Sentinela** (1º combate "real"). Mantém o encontro canônico §17 íntegro sem inflar o tutorial de 5min (onboarding DA-1). |

## Follow-up de editor (mãos do criador, Godot — F2-G.1 build)

Trabalho mecânico, derivado direto das §3/§7 (nada de decisão de design pendente além das DAs acima):

1. Criar cena `game/levels/distritos_inferiores.tscn` (raiz Node3D), caixa do mapa 60×40m com CSGBox de borda (parede 3m, material preto flat).
2. Piso: CSGBox hub +0m + arena -2m + rampa-escada de transição (colisão suave para navmesh).
3. Instanciar os 5 nós como Area3D + marker nas coords da §3 (B/L/C/P/T), com os raios de trigger da tabela.
4. 2 cover boxes (1m) + 1 vantage (+1m) na arena; portão sul (door 2m) como bloqueio até `puzzle_patrol.cleared`.
5. Grid 7×5 do puzzle alinhado a células de 2m nas coords (P); reusar ciclos P1/P2 de puzzle-gambito §8 sem alteração.
6. 2 save points (S0 entrada + portão sul), conforme DA-2 ratificada.
7. Bake do NavMesh do graybox; smoke-test de travessia do gold path (§4) cronometrado contra os ≤5min.
8. Spawn-points de Sentinela-Bit + Daemon-Guard (DA-3) na arena, com dados do `CharacterRepository` (não atores ad-hoc, M.1).

**Nota:** geometria não é validável headless; o smoke-test de travessia (passo 7) exige o editor/runtime nas mãos do criador. Este doc deixa todo o resto pré-resolvido.
