# Missões cronometradas (tempo real)

> **Status:** Decisão de design fechada via AskUserQuestion (brainstorm líder, 2026-07-18). **Ideia do Gus Dragon** (playtester), referência: as missões cronometradas de *Hollow Knight: Silksong*. Números (minutos, margem) → `economy-designer`/balance. Spec de impl → `gameplay_engineer`.
>
> **Cross-ref:** `docs/design/pillars.md` (Pillar 4 — prodígio analítico), memória `project_morte_dificuldade_canon` (escada de dificuldade), `project_save_dungeon_pem_faraday` (save por local).

## Conceito

Uma missão de **aventura/exploração** (fora do core de combate): o player tem **X minutos de tempo REAL** para ir do **ponto A ao ponto B** do mapa e **fazer/levar algo** antes de o relógio zerar.

## Decisões fechadas

- **O relógio corre durante a batalha só a partir do Difícil** (decisão do líder, 24/08/2026, item `G5` — resolve a tensão com o Pilar 1, *"Active turn-based sem timer no turno do jogador"*):

| Dificuldade | Relógio na batalha | Por quê |
|---|---|---|
| **Fácil** | **congela** | nenhuma pressão de tempo sobre o turno |
| **Médio** *(default)* | **congela** | o jogador default nunca sente cronômetro no combate; o Pilar 1 é respeitado onde alcança mais gente |
| **Difícil** | corre, visível | a pressão passa a ser parte do desafio, e quem escolheu Difícil escolheu isso |
| **Hardcore** | corre, visível, prazo mais apertado | como esta spec já previa |

  Fora da batalha o relógio corre em **todos** os modos — a missão é cronometrada nos quatro. O que muda é só o que acontece quando o combate começa.

  **Custo assumido, nomeado em vez de silenciado:** no Fácil e no Médio, "encarar um inimigo custa tempo real" deixa de valer, e com ele o incentivo de desviar das lutas. A intenção original do Gus Dragon sobrevive nos dois modos superiores. Foi troca consciente: o Pilar 1 protege o jogador default, e o desafio de rota continua existindo pela distância e pelos atalhos, que não dependem do relógio correr na batalha.
- **Desafio central = planejar a ROTA** (não reflexo). Há atalhos e caminhos a descobrir; o mérito é **achar a rota eficiente** (e talvez usar itens/cartas de mobilidade), não ter só dedo rápido. **Alinha o Pillar 4** (o Gus vence pensando). O relógio-corre-sempre reforça isso: a rota ótima desvia das lutas.
- **Consequência de falhar (tempo esgotado) por dificuldade:**
  - **Fácil:** igual ao Médio (só perde o bônus, completa).
  - **Médio:** só perde o **bônus** — a missão ainda se completa (nunca trava progresso).
  - **Difícil:** **falha + retry** — estourar é falha, mas dá pra tentar de novo.
  - **Hardcore:** igual ao Difícil (falha + retry), mas com **timer mais apertado**.
  - **Nota:** a falha **nunca é permanente** (sempre retry ou só perde bônus) — coerente com acessibilidade e o Pillar 4 (não punir reflexo). Deliberadamente **mais suave** que a escada de morte (que tem permadeath no Hardcore, `project_morte_dificuldade_canon`).

## Pontos abertos
- Números: X minutos, margem de folga, tamanho da rota → `economy-designer`/balance.
- Se há missões cronometradas recorrentes ou é um tipo pontual; onde a primeira aparece.

Ponteiro (L-30): `TODO.md` item `F4` (o item que governa este documento inteiro; aceite final ainda não fechado).
