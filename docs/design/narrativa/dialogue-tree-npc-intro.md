# Dialogue Tree — NPC Introdutório do VS — Blueprint
**Status:** Canônico. Ratificado Sprint 4 W2 2026-06-03. F2-N.1.
**Cross-ref:** core-loop-exploracao.md §2 (nó NPC), §3 (Scan-overworld), §8 (Knowledge); onboarding-vs.md §2 (degrau 1), §3 (gating), §7 (orçamento ≤5min); knowledge-progression.md §6 (Diário/estado); pillars.md (P1 info-não-poder, P4 prodígio 11 anos, anti-pillar wall-of-text); GDD §6.1 (Scan), §7; CHARS.md §7 (NPCs ambientais).

**Convenção:** pt-br. Termos game-dev no original (node, flag, jump, reconverge, gate, first-visit). Sem em-dash (blueprint estrutural, stubs entre colchetes, ZERO travessão de fala). Falas = STUB de intent (`[o que comunica]`), prose final = `narrative-writer`.

> **Nota de canon (não-passiva):** este NPC NÃO é o tutor de mecânica. onboarding-vs.md §1 é taxativo: "ensina por fazer, nunca por ler". Se este NPC explicasse Scan/combate em fala, violaria o anti-pillar. Papel correto: contextualizar o 1º nó de lore (porta de entrada diegética), apontar o caminho, dar cor ao mundo. A MECÂNICA é ensinada pelo espaço, não por ele.

---

## 1. Papel e identidade (canônico)

**Função dramática:** "âncora de chegada". Estabelece tom (cidade ciber-gótica habitada, não vazia), valida a curiosidade do Gus (P4), e dá o gancho diegético para o 1º nó de lore SEM entregar a solução (P1: info, não poder). NÃO dá quest, NÃO dá item, NÃO ensina botão.

**Tom de voz (stub):** velho técnico de praça, seco-afetuoso, fala em meia-frase, trata Gus como adulto-pequeno (respeita a inteligência, não infantiliza). Contraponto geracional: ele lembra a Era 2, Gus lê o presente. Subtexto > texto: ele sabe mais do que diz.

**Identidade canônica (DA-1, ratificada 2026-06-03):** **Seu Bertoldo Caím** (CHARS.md §7, ambient: 62 anos, técnico aposentado Era 2, lê jornal de papel na Praça da Compilação 7-9h). Encaixe natural: já vive no overworld da cidade, persona-mentor de praça pronta, lastro técnico para apontar lore Era 2 sem ser expositor. Fixado como guia do VS (NÃO Gargi Vance, NÃO companion).

## 2. Grafo de nós

```
                 [ENTER]
                    |
              n0_greet  (first-visit?)
              /        \
        (sim)/          \(não = revisit, ver §7)
            v            v
      n1_hook        n7_revisit_hub (dispatch por estado)
       (aponta lore)
            |
      +--- escolha de resposta (3 ramos) ---+
      |            |             |           |
  n2a_curioso  n2b_pragmatico  n2c_seco   (skip/sair)
      |            |             |           |
      +-----------> n3_reconverge <----------+
                    |  (tronco comum)
                    v
                  [EXIT -> jogador segue ao 1o no de lore]
```

Tabela de nós (ID + intent stub):

| Node | Intent stub (o que comunica, NÃO a fala) | Saída |
|---|---|---|
| `n0_greet` | nota o garoto, comentário ambiente (cidade respira, dia comum sombrio); checa first-visit | -> `n1_hook` ou `n7_*` |
| `n1_hook` | aponta o 1o no de lore ("aquilo ali ninguem mais para pra ler") + valida a curiosidade do Gus; NÃO diz o que é nem como decifrar | abre 3 escolhas |
| `n2a_curioso` | resposta a "[Perguntar o que e aquilo]"; ele devolve enigma, nao resposta (P1) | -> `n3` |
| `n2b_pragmatico` | resposta a "[Perguntar se e perigoso / por onde sigo]"; aponta direcao, tom util | -> `n3` |
| `n2c_seco` | resposta a "[Encerrar / so acenar]"; respeita o silencio, fecha curto | -> `n3` |
| `n3_reconverge` | despedida comum + 1 frase de cor (Era 2 echo, foreshadow leve); seta flags de saida | -> `[EXIT]` |
| `n7_revisit_hub` | dispatcher de revisita por estado (§7) | -> n8a/n8b/n8c |

**Voz do Gus nas escolhas (P4):** as 3 opções de jogador são deduzidas/curiosas, nunca power-fantasy. Ex. de intent: `[Curioso: o padrao naquilo nao fecha]`, `[Pragmatico: por onde se anda aqui?]`, `[Seco: acenar e seguir]`. Prose final = narrative-writer.

## 3. Condições de ramificação (gates de leitura)

| Gate | Lê flag/estado | Efeito |
|---|---|---|
| first-visit vs revisit | `npc_intro.met` | `false` -> `n0_greet`; `true` -> `n7_revisit_hub` |
| estado de progresso (revisit) | `lore_node_1.deciphered`, `combat_sentinela.cleared`, `puzzle_patrol.cleared` | seleciona n8a/n8b/n8c (§7) |
| comentário Knowledge-aware (opcional) | `KnowledgeKills[sentinela] >= 1` (via PlayerBus, read-only) | desbloqueia 1 linha alternativa em `n8b` (NPC nota que Gus "ja pegou o jeito") |

Escolhas de resposta (`n2a/b/c`) NÃO têm pré-condição: sempre as 3 disponíveis no first-visit (acessibilidade, sem gate punitivo). A ramificação aqui é de EXPRESSÃO (P-expression), não de gate.

## 4. Flags (set/check) e contrato de persistência

| Flag | Tipo | Quem escreve | Quem lê | Persistir em SaveDataV1? |
|---|---|---|---|---|
| `npc_intro.met` | bool | `n0_greet` (set true ao entrar 1a vez) | gate first-visit | **SIM** -> `QuestProgress`/flags |
| `npc_intro.choice` | enum {curioso, pragmatico, seco} | `n2a/b/c` | telemetria + callback narrativo pós-VS (DA-4 canon: reservado) | **SIM** (barato, habilita reatividade companion/ending depois) |
| `lore_node_1.deciphered` | bool | sistema de lore (core-loop §3), NÃO este diálogo | `n7` dispatcher | SIM (já previsto knowledge) |
| `combat_sentinela.cleared` | bool | FSM combate (combat.md §16 PlayerBus), NÃO aqui | `n7` dispatcher | SIM (QuestProgress VS) |
| `puzzle_patrol.cleared` | bool | sistema puzzle (puzzle-gambito), NÃO aqui | `n7` dispatcher | SIM |

**Contrato:** este diálogo só ESCREVE `npc_intro.met` e `npc_intro.choice`. Todas as outras flags ele apenas LÊ (read-only), produzidas por combate/puzzle/lore via PlayerBus. Evita acoplamento: o NPC reage ao mundo, não o controla. Flags de leitura JÁ devem existir no contrato do SaveSystem (F2-B); este doc só declara a dependência.

## 5. Reconvergência

Os 3 ramos de escolha (`n2a/b/c`) convergem todos para `n3_reconverge` (tronco comum único). Justificativa de design (anti explosão combinatória, pattern "branching with reconverge", onboarding ≤5min): a escolha muda o SABOR e seta `npc_intro.choice`, mas NÃO ramifica o estado do jogo nem o caminho físico. Nenhum ramo vira beco; nenhum exige conteúdo extra de redação além de 1 linha curta por ramo + 1 linha comum. Custo de redação para narrative-writer: ~6-7 falas curtas first-visit + 3 stubs revisit. Cabe no budget.

## 6. Integração com Knowledge Progression + onboarding

- **First-visit (degrau 1, onboarding §2):** o diálogo acontece ANTES do 1o Scan, na janela 0:45-1:30 (onboarding §7). Ele só APONTA o nó de lore; quem ensina Scan é a affordance + barra diegética do Gus (onboarding §3 gate de scan), não o NPC. O NPC não pode dizer "use o Scan" (violaria §1).
- **Knowledge-aware (opcional, revisita):** se o jogador voltar após ter scaneado/derrotado o Sentinela (`KnowledgeKills[sentinela] >= 1`, read-only via PlayerBus), `n8b` ganha 1 linha alternativa de reconhecimento ("o garoto ja le o bicho"). Reativo (core-loop §8), não obrigatório. Stub apenas; sem prose.
- **Diário (DA-3 canon SIM, 2026-06-03):** este NPC NÃO cria entry de bestiary (knowledge-progression §6: entry nasce do 1o combate, não de fala), MAS dispara **1 nota de cor no Diário do Gus** (seção não-bestiary) no first-visit. Tom analítico-afetuoso de 11 anos (P4), 1-2 linhas, registra a 1ª impressão do Gus sobre o velho/a praça. Escrita = narrative-writer no handoff.

## 7. Revisita (estado pós-combate / pós-puzzle)

`n7_revisit_hub` despacha por estado, em prioridade decrescente (sempre 1 fala curta, nunca re-tutorial):

| Estado lido | Node | Intent stub |
|---|---|---|
| nada limpo ainda (só `met`) | `n8a` | reconhecimento leve, reforço suave do gancho ("ainda nao foi ver?") |
| `combat_sentinela.cleared` true | `n8b` | nota que o garoto "se virou la dentro"; +linha alt se Knowledge>=1 | 
| `puzzle_patrol.cleared` true (fim do VS) | `n8c` | fecho do slice: 1 frase de foreshadow (cidade x Selve, P5 contraste), gancho para além do VS |

Regra: revisita NUNCA repete a fala first-visit inteira (anti-tédio); sempre curta (1 linha). Sem novas flags escritas na revisita (exceto telemetria opcional).

## 8. Orçamento de tempo (cabe no ≤5min, onboarding §7)

First-visit alvo: **≤30-45s de leitura** (janela 0:45-1:30 menos a caminhada). Limites de design para o narrative-writer:
- `n0_greet` + `n1_hook`: ≤2 linhas curtas somadas (1 ambiente + 1 gancho).
- cada `n2a/b/c`: 1 linha.
- `n3_reconverge`: 1 linha + meia-linha de cor.
- Total first-visit: ~5-6 linhas curtas. Skippable a qualquer momento (botão avançar; P-acessibilidade, sem trava temporal).
- Anti wall-of-text: nenhum nó com bloco de lore expositiva. A lore mora no nó de lore (decifrável), não na boca do NPC. Se a fala precisar explicar o mundo, está errada (mover para in-world-doc).

---

## Decisões Canonizadas (Sprint 4 W2 2026-06-03)

| # | Decisão | Escolha |
|---|---|---|
| **DA-1** | Identidade do NPC introdutório | **Seu Bertoldo Caím** (existente, CHARS.md §7). Fixado como guia do VS. NÃO Gargi (queima beat família), NÃO companion. |
| **DA-2** | Lib/sistema de diálogo | **OBSOLETO (pós-ADR-008).** A escolha original era DialogueManager (addon Godot 4, MIT), registrada em ADR-003 e decidida 2026-05-30 (TODO F2-E.6-DECISION). O re-pivot para a engine própria C++20 + SDL3 (ADR-008) aposentou o addon Godot. O **blueprint de diálogo deste doc segue válido** (é dado de design agnóstico de engine); o runtime de diálogo será re-derivado em C++20 pelo squad técnico. Ver ROADMAP.md. |
| **DA-3** | NPC dispara nota de cor no Diário no first-visit? | **SIM**, 1 nota de cor não-bestiary, voz analítica de Gus (P4). Escrita = narrative-writer. |
| **DA-4** | `npc_intro.choice` tem callback futuro? | **Reservar para callback pós-VS** (companion/ending reconhece o tom do Gus). Persistir desde já (custo trivial). |
