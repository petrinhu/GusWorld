# Core Loop de Exploração — Spec de Design (Vertical Slice)
**Status:** Canônico. Ratificado Sprint 2 W2 2026-06-03. F2-D.5.
**Cross-ref:** GDD §4/§5/§6.1/§6.3, combat.md (§12 Scan/Gambito, §18 Scan-ambiente), puzzle-gambito.md (a criar, F2-D.2).

**Convenção:** pt-br. Termos de game-dev no original (core loop, overworld, trigger, telegraph, first-strike). Sem em-dash.

> **Nota de canon (não-passiva):** combat.md §12 já estabelece que Scan existe fora de combate e vira passivo após N kills. Esta spec NÃO reabre isso: Scan no overworld é gratuito (AP só existe em combate). A unica DECISÃO ABERTA sobre Scan é se há limite/cooldown anti-spam, não se custa recurso.

---

## 1. Visão e o verb fora de combate

Fora de combate, o verb dominante (**decifrar**) continua sendo o motor. O jogador não "anda até o objetivo": ele **lê o espaço como sistema**. Aesthetic alvo na exploração, em ordem: **Discovery** (a cidade é rede de segredos a hackear), **Challenge** (ler padrão antes de agir), **Expression** (escolher o que investigar). Pillar 4: Gus se move por curiosidade e lógica, nunca por poder. Nada de open-world (GDD §9): a área-cidade do VS é um grafo curto de nós conectados, navegável de ponta a ponta.

## 2. Gramática da área-cidade (tipos de nó)

A área não é level design (isso é F2-G.1 do level-designer); é a **gramática** que ele instancia. 5 tipos de nó:

| Nó | Função | Como o jogador descobre | Manifestação hardware |
|---|---|---|---|
| **Combate** | encontro turn-based | silhueta de inimigo visível no overworld + ping de Scan | Óculos: contorno hostil no HUD |
| **Puzzle (Gambito)** | puzzle de patrulha/trajetória | obstáculo de movimento (patrulha, projétil ambiental) bloqueando passagem | Óculos: tabuleiro holográfico sobreponível |
| **NPC** | diálogo / quest stub | NPC com indicador de fala flutuante (`...`) | nenhuma (social, não técnico) |
| **Lore** | fragmento decifrável (in-world-doc, terminal, glifo) | brilho sutil + ping de Scan ao alcance | Óculos + Matriz: decodifica texto/glifo |
| **Terminal** | hack ambiental (porta, ponte, energia) | painel inerte que Scan revela como interativo | Tavus-Drive: joga carta ambiental (GDD §6.2) |

Navegação entre nós: caminho curto e legível (hub + ramos, alinhado a Pillar 5 "hub + radiais"). O VS tem ~1 nó de cada tipo (MVV plano_vs §4).

## 3. Scan no overworld (GDD §6.1)

Scan é **passivo-ambiente contínuo** fora de combate (não consome AP, que só existe em combate). Materializa o triângulo: **Óculos** captam, **Matriz** amplifica alcance.

- **Alcance:** raio fixo ao redor de Gus (proposta: ~8m em escala de blockout; calibrar em F2-G.EXPLORE). Fora do raio, nós aparecem como silhueta neutra; dentro do raio, revelam tipo + 1 dado.
- **O que revela por tipo de nó:** combate = fraqueza-âncora prevista (se já scaneado antes em batalha, herda Knowledge); puzzle = vetor da patrulha/projétil; lore = "decifrável" + dica de método; terminal = "interativo via carta X".
- **Padrão matemático do bioma (GDD §6.1):** ao scanear flora/estrutura da cidade, Óculos expõem o padrão (sequência numérica recorrente nas fachadas, grade ortogonal do circuito). Decifrar o padrão abre passagem ou expõe um "bug" escondido (nó de combate oculto).
- **Onboarding (F2-D.6):** o 1º nó de lore ensina Scan-overworld antes de qualquer combate. "Aponte o olhar, leia o padrão" via affordance visual, sem wall-of-text (anti-pillar).

## 4. Gatilho de combate

Aproximação física ao nó de combate dentro de um raio de contato dispara a transição para a FSM de combate (combat.md §3, `SetupPhase`). Feedback de transição < 150ms (flash de HUD + congelamento + wipe para a arena). Regra de vantagem é DECISÃO ABERTA (ver §10).

- Inimigo tem **cone de detecção visível** (Óculos pintam o cone no chão). Entrar no cone = inimigo investe; contornar = combate evitável (stealth opcional, coerente com GDD §6.3 patrulhas).
- Inimigo já derrotado naquela sessão **não respawna** (anti-grind, §8).

## 5. Gatilho do puzzle Gambito

O puzzle está **integrado à exploração**, não em zona separada (decisão de coerência: o mesmo tabuleiro de combat.md §12 lê patrulhas no overworld). O nó-puzzle é um **bloqueio de passagem**: uma patrulha ou um projétil ambiental cíclico que impede o caminho. O jogador:

1. Aproxima e o tabuleiro holográfico fica disponível (verb prever, GDD §6.3).
2. Lê o ciclo da patrulha/trajetória (vetor telegrafado, mesma leitura de `IntentPreview`).
3. Resolve cruzando na janela segura, ou usando uma carta ambiental para alterar o vetor.

Sem combate obrigatório no puzzle: é teste de leitura, não de dano. Detalhamento de layout/regras = `puzzle-gambito.md` (F2-D.2); esta spec só define o **acoplamento à exploração**.

## 6. Integração hardware-triad fora de combate (Pillar 3)

Os 3 vértices têm papel ativo no overworld; sem isto a spec violaria Pillar 3.

| Vértice | Papel na exploração |
|---|---|
| **Óculos Táticos** | input/scan contínuo: revela tipo de nó, cone de detecção inimiga, tabuleiro de patrulha, padrão do bioma |
| **Matriz Ortodôntica** | amplifica alcance do Scan-overworld e filtra ruído da cidade (combat.md §12 "sonar maxilar"); destaca anomalias |
| **Tavus-Drive** | executa cartas ambientais em nós-terminal (GDD §6.2 duplo-uso: a mesma carta resolve combate e puzzle) |

## 7. Feedback visual ≤150ms (HUD diegético)

GDD §4 exige reação < 150ms. Na exploração:

- **Mapa/HUD holográfico:** minimapa diegético dos Óculos; nós aparecem como ícones por tipo ao entrar no alcance do Scan.
- **Ping de Scan:** ao revelar um nó, pulso visual + som curto (< 150ms do cruzamento do alcance ao ícone aparecer).
- **Indicador de presença:** cone de detecção inimiga pintado no chão, atualizado em tempo real.
- **Notificação de decifração:** ao decifrar lore/padrão, micro-toast diegético (`PADRÃO IDENTIFICADO`) + entrada no Diário do Gus.
- Multimodal (Pillar 4 a11y): ícone + cor + som, nunca só cor.

## 8. Conexão com Knowledge Progression

Quando o jogador scaneia um inimigo no overworld **antes** do combate:

- **Proposta (recomendada):** Scan-overworld concede **meia-revelação** (revela fraqueza-âncora se o tipo já foi morto antes em batalha; caso contrário, só silhueta + HP estimado). **NÃO** incrementa `KnowledgeKills` (kill-count só sobe matando, combat.md §11) e **NÃO** dá first-strike por si só (isso é §10). Apenas adianta informação que o jogador já teria direito por histórico.
- Racional: separa "saber" (Scan) de "dominar" (kills). Preserva a curva de variância de combat.md §11 sem atalho de farm.

## 9. Anti-grind no overworld

- **Encontros finitos e não-respawnáveis na sessão:** inimigo derrotado não reaparece (sem círculos de farm). No VS, número fixo de nós de combate (MVV).
- **Respawn só por condição narrativa** (beat de capítulo), nunca por tempo/circular, coerente com economia.md §3.2 (cura grátis = beat, nunca N encontros).
- **XP differential** (combat.md §11) já pune farmar zona baixa; a exploração herda isso sem regra nova.
- Curiosidade é recompensada por **conteúdo único** (lore, carta, fragmento de hardware), não por repetição (GDD §5: sem grind).

## 10. Decisões Canonizadas

Itens ratificados pelo criador supremo Sprint 2 W2 2026-06-03.

| # | Decisão | Canon (ratificado 2026-06-03) |
|---|---|---|
| **DA-1** | Câmera e navegação no overworld | **3/4 orbital + controle direto** — WASD/stick move Gus; câmera orbita por botão separado. Coerente com "3D real" e combat.md. |
| **DA-2** | First-strike por Scan prévio | **Sem vantagem mecânica** — Scan no overworld só adianta info. Nenhum bônus em combate. Preserva Pillar 1: "info, não poder". |
| **DA-3** | Limite/cooldown do Scan-overworld | **Contínuo, sem cooldown** — passivo permanente. AP só existe em combate. Coerente com combat.md §12. |

**Cross-docs confirmados (Sprint 2 W2):** `puzzle-gambito.md` §8 (caminho único VS, reset suave, sem custo) e `knowledge-progression.md` §8 (Scan-overworld = meia-revelação, zero kill-count) criados no mesmo sprint. Cross-refs casam.

---
**Última revisão:** 2026-06-03 (F2-D.5 Sprint 2 W2). Decisões DA-1/DA-2/DA-3 canonizadas pelo criador supremo 2026-06-03.
