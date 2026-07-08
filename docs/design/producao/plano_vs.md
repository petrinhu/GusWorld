# Plano de Produção — Vertical Slice (GusWorld)

**Status:** Canônico. Gerado F2-PROD.1 Sprint 1 W2 2026-06-03.

> **NOTA DE STACK (2026-06-23, pós-ADR-008).** Este plano foi escrito sobre o stack **Godot 4 + C# .NET 8 AOT**, depois aposentado pelo [ADR-008](../../tech/adr/ADR-008-repivot-qt-to-sdl3.md) (engine própria C++20 + SDL3). O **conteúdo de produção segue válido e canônico** (exit criteria por milestone, critical path, RAID, MVV, buffer policy são agnósticos de stack). Apenas as **mecânicas de build/perf citadas estão obsoletas**: onde se lê `dotnet`/AOT/`export_presets.cfg`/`godot --headless`/`Performance.get_monitor`/`RenderingServer`/`gl_compatibility`, o equivalente atual é CMake + Ninja + CMakePresets, `ctest`/Catch2, smoke headless `SDL_VIDEODRIVER=dummy` e profiling nativo C++ (sem runtime gerenciado). O **floor de perf** (iGPU) e o **teto RTX 3050 4GB** seguem canônicos. O "Plano B AOT inflado" perde o motivo original (C++ é AOT por natureza, sem runtime .NET); o gatilho de tamanho de binário (>150MB) permanece como sinal, mas a mitigação muda (LTO, strip, redução de assets). Re-derivar este plano para o stack atual é decisão de produção do criador (AskUserQuestion), fora do escopo da higienização de termos.

Documento leve de delivery. Não é PMO. Autoridade de escopo permanece no GDD (`docs/design/gdd.md`) e nos pillars. Aqui ficam só: critério de pronto por milestone, sequência crítica, riscos e política de buffer.

**Meta VS:** 4-6 meses para um slice coeso jogável de 5-10min que prova os 3 pilares-âncora (Scan, Compilação do Codex, Vetor do Gambito) em uma área-cidade com 1 combate e 1 puzzle.
**Time:** solo (Petrus). **Floor de perf canônico (W0):** iGPU Intel HD / AMD integrada (teto RTX 3050 4GB).

---

## 1. Exit criteria por milestone

Milestone sem critério mensurável é teatral. Cada linha abaixo declara DONE de forma objetiva e verificável.

### F2-M.1 — VS end-to-end coeso (5-10min, com grayboxing)

| Critério (mensurável) | Método de verificação |
|---|---|
| Loop completo jogável sem reload: mover (3/4 orbital) → falar com 1 NPC → 1 combate turn-based → 1 puzzle Gambito → save/load, tudo numa sessão contínua | Run manual gravada (OBS); cronômetro mostra 5-10min do start ao save |
| Combate roda game-side (não headless) com dados reais do `CharacterRepository`, não atores ad-hoc | Verificar em runtime que `StartCombat` recebe atores do repo; stats batem com os canônicos (§4) |
| Save grava e recarrega estado per-character (HP/XP/KnowledgeKills/deck) sem perda | Salvar mid-slice, fechar app, reabrir, load: estado idêntico (diff manual dos campos) |
| Zero crash / zero exceção não tratada no caminho dourado | Console limpo na run completa; 3 runs consecutivas sem stack trace |
| **Ação de milestone obrigatória — revisão ADR-001 do deep-lore paralelo (F1-DL.4-9):** ao fechar o M.1, o criador decide explicitamente **retomar / manter pausado / cortar** o deep-lore restante, e a decisão é registrada | Nota de decisão datada e anexada ao M.1 (escolha entre as 3 opções + 1 linha de justificativa). Milestone NÃO fecha sem essa nota (F2-PROD.8) |

> **Por que isto é ação de milestone, não nota solta (F2-PROD.8).** O ADR-001 reclassificou F1-DL.4-9 + F1-DL.REFAC de gating para "paralelo orgânico" e nomeou o **risco médio-alto** explícito de "entrar em modo só código e nunca voltar ao lore, gerando frustração tardia" (ADR-001, Riscos #3; Reversibilidade; Consequências/Negativas — "Decisão depende de disciplina"). O próprio ADR cravou a **mitigação: revisão obrigatória de status em F2-M.1**. Aqui essa mitigação vira critério de saída testável do M.1, não promessa implícita. As 3 opções de decisão são as do ADR: **retomar** (dispatch de ciclo deep-lore conforme `feedback_deep_lore_colaborativo_rag_visivel`), **manter pausado** (segue paralelo orgânico sem deadline) ou **cortar** (ex: Antologia Vol 2, Opção D do ADR). Cross-ref: `docs/tech/adr/ADR-001-pivot-lore-to-engine.md` + `raid-log.md` **R-03** (Revisar = F2-M.1). Decisão é one-way criativa do criador (AskUserQuestion no fechamento do M.1), não do produtor.

### F2-M.2 — 60fps @ 1080p no floor canônico

**Gate de hardware (sem ambiguidade).** "Passar no M.2" = passar **no FLOOR**, não no teto. Floor canônico (W0, F2-S.2-HW): **iGPU integrada** (Intel HD / AMD integrada). Teto: RTX 3050 4GB. O pass/fail abaixo é medido **exclusivamente na iGPU de teste**, a 1920×1080, fullscreen, renderer `gl_compatibility`, vsync OFF, build de **release exportado** (não editor, não debug). Performance boa só no teto = **FAIL** do M.2. O teto serve apenas para sanity-check (se nem no RTX 3050 passar, há bug, não limite de hardware).

**Cena de referência única (congelada para o M.2):** blockout-cidade do F2-G.1 + 3 atores da party + 2 inimigos + shaders do slice ativos (toon mínimo + qualquer FX de carta usado no combate). É a MESMA cena do M.1, sem otimização pontual só para o benchmark.

**Protocolo de medição (reproduzível):** captura via `Performance.get_monitor` (TIME_PROCESS + TIME_RENDER) gravando frame-time por frame em CSV através do harness do **F2-M.2a**; cada teste roda do clean-start (sem warm-up manual além dos primeiros 5s descartados). Três corridas por cenário; vale a **pior** das três (não a média). Resultado anexado ao milestone como CSV + um print do gráfico de frame-time.

| Critério (mensurável, FALSEÁVEL no floor) | Método de verificação |
|---|---|
| Exploração: na cena de referência, com câmera orbital em movimento contínuo (pan + zoom), **p99 frame-time ≤ 16.6ms** numa janela de 3min consecutivos na iGPU | CSV do harness F2-M.2a; calcular percentil 99 da janela de 3min; qualquer p99 > 16.6ms na pior das 3 corridas = FAIL |
| Combate (pior caso): 3 party vs 2 inimigos, turno com combo + FX de área (o turno mais pesado do deck do slice), **1% low ≥ 55fps** (frame-time do pior 1% ≤ 18.2ms) | Captura só do pior turno; ordenar frame-times, pegar o p99 do tempo (= 1% low de fps); < 55fps em qualquer das 3 corridas = FAIL |
| **Zero stutter visível** (sem espinhos isolados): nenhum frame individual acima de **33ms** (= salto perceptível) em toda a janela de 3min | Max frame-time da série; 1 frame > 33ms já marca o ponto para investigar; padrão recorrente de spikes = FAIL |
| VRAM/RAM dentro do orçamento do floor: **sem swap/thrash** (texturas evictadas em loop) na iGPU compartilhando RAM do sistema | `RenderingServer` memory monitor + monitor de RAM do SO; qualquer eviction-loop ou paging visível = FAIL |
| Plano B do binário inflado documentado com **gatilho observável** e opções de mitigação (não decidido aqui) | Ver bloco "Plano B — binário/AOT inflado" abaixo (F2-PROD.6); decisão técnica de threshold e ordem de mitigação = AskUserQuestion ao criador / Caetano |

**Plano B — binário/AOT inflado (gatilho + opções, decisão one-way pendente).**

O `build.md §7.4` prevê Linux ~80-120MB / Windows ~70-100MB (trade-off aceito no ADR-002 pelo AOT). O risco vivo é **R-05** (export físico) e o componente de tamanho de **R-01/R-05** no `raid-log.md`. Este plano registra o **gatilho** e as **opções**; a escolha do limite de corte e da ordem de mitigação é **decisão técnica one-way do criador / Caetano (CTO)**, não deste plano.

- **Gatilho observável (canon 2026-06-03):** `ls -lh` do artefato exportado (medido no fechamento do M.4) ultrapassa **150MB** (~25% de folga sobre o teto §7.4; tamanho não é pillar do VS, trade-off AOT já aceito no ADR-002). Registrar o tamanho observado a cada export; só aciona plano B acima de 150MB, nunca slip silencioso.
- **Sinal secundário (perf, não só tamanho):** se o p99/1%-low acima falhar no floor E o profiling atribuir o custo ao **runtime AOT / startup / footprint de memória** (não a draw-calls de cena), o gatilho de perf e o de tamanho convergem para a mesma raiz.
- **Opções de mitigação (ordem ratificada 2026-06-03: trim/assets primeiro, AOT→JIT por último):**
  1. **Trim / IL-linker agressivo:** cortar assemblies não usados, `TrimMode=full`, revisar `[DynamicallyAccessedMembers]` (build.md §7.2). Preserva o pillar de perf do ADR-002.
  2. **Reduzir assets embarcados:** auditar `export_files` / `embed_build_outputs`; mover assets grandes para pack externo se viável.
  3. **Reavaliar AOT → JIT (one-way de stack):** se o footprint do AOT for a causa-raiz, reabrir **ADR-002** (AOT vs JIT). Mexe em decisão de stack canônica — exige ADR novo, não decisão de produção.
  4. **Aceitar o tamanho:** se perf no floor estiver OK e só o tamanho exceder, registrar como débito conhecido e seguir (tamanho não é pillar do VS).
- **Escalonamento:** gatilho disparado → AskUserQuestion ao criador com trade-off explícito (perseguir trim/assets / reabrir ADR-002 / aceitar débito), conforme buffer policy §5. Nunca slip silencioso. Cross-ref `raid-log.md` R-05 (atualizar Prob.×Impacto se materializar).

### F2-M.3 — Playtest interno N=3 (time-to-fun ≤ 5min)

| Critério (mensurável) | Método de verificação |
|---|---|
| Os 3 testadores (Petrus, Gus Dragon, Iago) executam o loop completo (scan → compilar → prever → resolver 1 encontro) em ≤ 5min de gameplay real cada | Timer real por sessão; medir do primeiro input ao fim do 1º combate; 3/3 dentro do alvo |
| Nenhum testador pede ajuda externa para passar do tutorial diegético | Observação registrada; intervenção do facilitador = fail daquela run |
| Secundária (não bloqueante): "would play again" ≥ 70% | Pergunta pós-sessão (sim/não); 2 de 3 já satisfaz |

### F2-M.4 — Build Linux .tar.gz + .rpm distribuível (AppImage adiado)

**Artefatos do gate (canon 2026-06-03):** `.rpm` (nativo Fedora, = distro do gate / dev box) + `.tar.gz` (universal, extrai-e-roda). **AppImage foi ADIADO** para pós-VS (não entra no gate do M.4). Distro do gate = Fedora (1 distro, D2 canon); Ubuntu LTS é confirmação não-bloqueante via `.tar.gz`.

| Critério (mensurável) | Método de verificação |
|---|---|
| `.rpm` instala e roda em VM clean Fedora sem dependência manual extra | Boot VM Fedora limpa, `sudo dnf install ./gusworld-*.rpm`, jogo abre no menu |
| `.tar.gz` extrai e roda o binário standalone na mesma VM | Extrair, rodar executável, chegar ao menu sem erro de lib faltante |
| Export reproduzível por wrapper local (`scripts/build_linux.sh`, `set -euo pipefail`), não passo manual no editor | Rodar o script do zero (clone limpo + templates instalados) gera artefatos idênticos |
| Tamanho do binário dentro do esperado (≤ 150MB, gatilho M.2) | `ls -lh` do artefato; se exceder 150MB, aciona plano B de M.2 |

### F2-M.5 — Revisão style guide com dados reais de perf

| Critério (mensurável) | Método de verificação |
|---|---|
| Orçamentos de arte do style-guide (tris/draw-call/VRAM) confirmados ou ajustados contra números medidos no floor (M.2) | Diff do style-guide §10/§13: cada budget tem nota "validado em hw X" ou valor corrigido |
| Decisão "toon via built-in vs shader custom" fechada com screenshot in-Godot no renderer gl_compatibility | F2-A.4a: screenshot comparativo anexado; nota canônica no style-guide |
| Teste de daltonismo (Coblis) das 2 paletas passou ou paleta foi ajustada | F2-ART.COLORBLIND: print do simulador para protanopia/deuteranopia; anomaly-red × biolumen-green legíveis |

---

## 2. Critical path (sequência crítica)

O fun-loop é o caminho crítico, não a arte. A decisão placeholder-first (F2-PROD.2) tira a cadeia de arte (W5) do caminho do M.3 — graybox fecha o loop antes de qualquer modelo final.

```
F2-G.1 (blockout cidade, W3)
   │
   ├─→ F2-E.10 (TemplateSerializer .gdt, W3) ─→ F2-E.10b (CharacterRepository, W3)
   │                                                   │
   ├─→ F2-G.EXPLORE (mover + interagir + triggers, W4) │
   │                                                   ▼
   └────────────────────────────────────────────→ F2-G.5/G.9 (combate jogável, W4)
                                                       │
                       F2-G.6 (puzzle Gambito, W4) ────┤
                       F2-G.7 (diálogo 1 NPC, W4) ──────┤
                       F2-G.8 (save/load, W4) ──────────┤
                                                        ▼
                                                   F2-M.1 (VS coeso)
```

**Bloqueadores explícitos por milestone:**

- **F2-M.1** bloqueado por: F2-G.1 → F2-E.10 → F2-E.10b → F2-G.5/G.9 (aresta confirmada em F2-PROD.5: G.5 depende de E.10+E.10b, **não** só de E.5), mais F2-G.EXPLORE, F2-G.6, F2-G.7, F2-G.8. Pré-design: F2-D.1 (balance prototype), F2-D.2 (spec puzzle Gambito), F2-D.5/D.6 (loop fora de combate + onboarding).
- **F2-M.2** bloqueado por: F2-M.1 (precisa da cena real) + F2-M.2a (harness de profiling que gera o CSV de frame-time) + decisão de floor F2-S.2-HW (já fechada W0). Gate de perf é exit-criterion duro **medido no FLOOR iGPU** (passar só no teto RTX 3050 = FAIL); thresholds e protocolo em §1 M.2 (F2-PROD.6). O plano B de binário inflado tem **gatilho observável** mas **threshold/ordem de mitigação não decididos** — pendente de AskUserQuestion (criador / Caetano).
- **F2-M.3** bloqueado por: F2-M.1 + F2-D.6 (onboarding diegético). Não exige arte final.
- **F2-M.4** bloqueado por: F2-CI.5 (export templates 4.6 mono, hoje ausentes — bloqueio físico silencioso), **F2-CI.7 (PoC AOT — precede o preset, D1 canon 2026-06-03)**, F2-CI.6 (export_presets.cfg), F2-CI.1 (wrappers). NÃO bloqueado por CI Forgejo completo (anti-OE, F2-PROD.7).
- **F2-M.5** bloqueado por: F2-M.2 (dados reais de perf) + cadeia de arte mínima (F2-A.2 outline, F2-A.4a toon, F2-ART.COLORBLIND).

**Lateral fora do critical path (rodar em paralelo já em W2/W3):** F2-ART.SPIKE (validação pipeline AI, time-box 1 semana) — é o early-warning do gargalo nº1 de arte, mas não trava o fun-loop graças ao placeholder-first.

---

## 3. Top 3 riscos (RAID resumido)

| # | Risco | Categoria | Impacto | Prob. | Mitigação | Owner |
|---|---|---|---|---|---|---|
| R-01 | Pipeline de arte AI (TripoSR no RTX 3050 4GB) não fechar mesh utilizável → arte final escorrega e ameaça M.5 | Technical / Schedule | Alto | Média | Placeholder-first (F2-PROD.2) já tira arte do critical path do M.1/M.3; F2-ART.SPIKE time-boxed 1 semana em W2 decide AI-base vs hand-model com dado real; fallback = hand-model low-poly direto | Petrus |
| R-02 | §18 Ambientes inflar F2-E.11 (catálogo completo 15 terrenos + 8 climas + 4 períodos foi mantido no VS) e consumir o buffer | Scope | Médio | Média | F2-E.11 está fora do critical path do M.1; se ameaçar prazo, cair para 2-3 arenas representativas (trade já desenhado em F2-PROD.4) e adiar catálogo completo para pós-VS. Pré-produção é o lugar mais barato de cortar | Petrus |
| R-03 | Solo + deep-lore paralelo (F1-DL.4-9): dispersão criativa, lore "nunca retomar" OU lore roubando tempo do código (risco nomeado no próprio ADR-001) | Operational / Health | Médio | Alta | Deep-lore é descanso criativo entre steps técnicos, NUNCA bloqueia código (ADR-001); revisão de status obrigatória amarrada ao fechamento do M.1 (F2-PROD.8); WIP-limit mental de 1 frente técnica por vez | Petrus |

RAID vivo completo (com cadência de revisão por onda) fica em `raid-log.md` (F2-PROD.3, doc standalone canonizado 2026-06-03).

---

## 4. Minimum Viable VS

Conjunto MÍNIMO que ainda permite medir time-to-fun ≤ 5min (M.3). Tudo fora desta lista é cortável sem inviabilizar o playtest.

**Obrigatório (sem isto não há fun-loop mensurável):**

- 1 área-cidade em blockout navegável (graybox; F2-G.1).
- Movimento 3/4 com câmera orbital + 1 interagível + trigger de combate e de diálogo (F2-G.EXPLORE).
- 1 combate jogável game-side com a party canônica e os 2 inimigos, deck mínimo 5-8 cartas, fila de iniciativa visível, RNG visível (variância 0 no slice), Scan + 1 combo (F2-G.5/G.9 + F2-D.1).
- 1 puzzle Vetor do Gambito jogável, 1 layout (F2-G.6 + F2-D.2).
- 1 NPC com diálogo curto (stub, não prosa final; F2-G.7).
- Save/load funcional `save_version: 1` (F2-G.8).
- Onboarding diegético just-in-time do 1º encontro (F2-D.6) — **crítico**: é o que faz ou quebra os 5min.
- Áudio mínimo: 5-6 SFX + 1 track ambiente + hook de combate (F2-AU.2/4/6/7). Imersão barata, alto retorno; sem isso o slice parece quebrado.

**Cortável do MVV sem matar M.3 (degrada polish/escopo, não o loop):**

- Modelos/rig/anims finais (F2-G.2..G.4, G.10): graybox cobre o playtest.
- Todos os shaders custom além do toon mínimo (F2-A.3 holograma, A.6 glitch, A.7 água/lúmen): usar material flat no slice.
- §18 Ambientes completo (F2-E.11): 0 arenas modificadas é aceitável no MVV.
- Loot/quest/inventário completos (F2-G.LOOT, G.QUEST, G.INVENT): 1 recompensa fixa e 1 objetivo linear bastam.
- Knowledge progression real (F2-E.9): valor fixo no slice (combat.md §17 já permite).
- Música adaptive multicamada: 2 estados (explore/combat) bastam.

---

## 5. Buffer policy

Reserva de 20-30% do prazo do VS para o desconhecido. 4-6 meses de meta = ~5-7 semanas de buffer implícito embutidas na faixa de 6 meses. Regras de slippage:

- **Buffer é do projeto, não da tarefa.** Nenhuma tarefa "ganha" buffer próprio; o colchão é consumido só quando o caminho crítico real desliza.
- **Gatilho de escalonamento (1 semana):** qualquer item do critical path (§2) que atrase mais de 1 semana além da onda dispara decisão imediata via AskUserQuestion ao criador, com trade-off explícito (cortar escopo do MVV §4 / estender prazo / aceitar débito), nunca slip silencioso.
- **Ordem de corte sob pressão** (do mais barato ao mais caro de remover, preservando M.3): (1) §18 Ambientes → 2-3 arenas ou zero; (2) shaders não-essenciais → material flat; (3) loot/quest/inventário → versão fixa; (4) arte final → manter graybox e empurrar para pós-VS. **Nunca cortar:** o fun-loop, o onboarding, o save/load.
- **Frente única.** Solo: no máximo 1 item técnico ativo no critical path por vez; arte e deep-lore correm como laterais/descanso, nunca competindo com o caminho crítico.
- **Sinal verde-amarelo-vermelho** revisado a cada fechamento de onda (W3→W4→W5→W6). Amarelo = buffer começando a ser tocado; vermelho = critical path atrasado >1 semana → AskUserQuestion.

---

## Notas de canon

- **Floor de perf:** o briefing menciona "GTX 1060"; o canon mais recente (W0, F2-S.2-HW / F2-M.2-FLOOR, 2026-05-30) rebaixou o floor para **iGPU integrada** (teto RTX 3050 4GB). Este plano segue o canon W0. Se o criador quiser reverter para GTX 1060, é decisão one-way de orçamento de arte (AskUserQuestion).
- **Stats do encontro (§4)** e **TTK 3-5 turnos** conferem com combat.md (determinismo variância-0 no slice, AP 3 fixo, party 3, Análise Preditiva 1×/batalha protegendo Gus). Balance numérico real entra via F2-D.1 antes de animar.
