# Missões cronometradas (tempo real)

> **Status:** Decisão de design fechada via AskUserQuestion (brainstorm líder, 2026-07-18). **Ideia do Gus Dragon** (playtester), referência: as missões cronometradas de *Hollow Knight: Silksong*. Números (minutos, margem) → `economy-designer`/balance. Spec de impl → `gameplay_engineer`.
>
> **Cross-ref:** `docs/design/pillars.md` (Pillar 4 — prodígio analítico), memória `project_morte_dificuldade_canon` (escada de dificuldade), `project_save_dungeon_pem_faraday` (save por local).

## Conceito

Uma missão de **aventura/exploração** (fora do core de combate): o player tem **X minutos de tempo REAL** para ir do **ponto A ao ponto B** do mapa e **fazer/levar algo** antes de o relógio zerar.

## Decisões fechadas

- **O relógio corre SEMPRE** — inclusive durante batalhas no caminho. Encarar um inimigo custa tempo real. Consequência de design (intencional): incentiva **evitar/otimizar** os encontros, não farmá-los.
- **Desafio central = planejar a ROTA** (não reflexo). Há atalhos e caminhos a descobrir; o mérito é **achar a rota eficiente** (e talvez usar itens/cartas de mobilidade), não ter só dedo rápido. **Alinha o Pillar 4** (o Gus vence pensando). O relógio-corre-sempre reforça isso: a rota ótima desvia das lutas.
- **Consequência de falhar (tempo esgotado) por dificuldade:**
  - **Fácil:** igual ao Médio (só perde o bônus, completa).
  - **Médio:** só perde o **bônus** — a missão ainda se completa (nunca trava progresso).
  - **Difícil:** **falha + retry** — estourar é falha, mas dá pra tentar de novo.
  - **Hardcore:** igual ao Difícil (falha + retry), mas com **timer mais apertado**.
  - **Nota:** a falha **nunca é permanente** (sempre retry ou só perde bônus) — coerente com acessibilidade e o Pillar 4 (não punir reflexo). Deliberadamente **mais suave** que a escada de morte (que tem permadeath no Hardcore, `project_morte_dificuldade_canon`).

## Pontos abertos
- [ ] Números: X minutos, margem de folga, tamanho da rota → `economy-designer`/balance.
- [ ] Se há missões cronometradas recorrentes ou é um tipo pontual; onde a primeira aparece.
