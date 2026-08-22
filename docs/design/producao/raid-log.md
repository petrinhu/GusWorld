# RAID Log — Vertical Slice (GusWorld)

**Status:** Vivo e canônico. Criado F2-PROD.3 em 2026-06-03; formato standalone (Opção A) ratificado pelo criador 2026-06-03.
**Cross-ref:** `plano_vs.md` (Top-3 §3 é o resumo; este doc é a versão viva e expandida). IDs R-01/R-02/R-03 são os mesmos do plano. Critical path e MVV: ver `plano_vs.md` §2/§4 (não re-derivado aqui).
**Cadência de revisão:** a cada fechamento de onda (W3 → W4 → W5 → W6) e em F2-M.1. Solo dev: ler em 30s, saber o que está pegando fogo.
**Owner de tudo:** Criador (solo). "Mitigação" aponta o item de TODO que a executa quando existe.

> Formato: doc standalone (Opção A), ratificado pelo criador 2026-06-03. RAID vivo com revisão recorrente não cabe numa linha de backlog; ficar dentro do `plano_vs.md` misturaria estado-vivo com plano quase-estável.

---

## R — Risks

| # | Risco (curto) | Cat. | Prob.×Impacto | Gatilho / early-warning observável | Mitigação (TODO) | Revisar |
|---|---|---|---|---|---|---|
| R-01 | Pipeline arte AI (TripoSR no RTX 3050 4GB) não fechar mesh retopo-ável → arte final escorrega, ameaça M.5 | Tech/Sched | Média × Alto | Spike estoura time-box de 1 semana sem 1 mesh import-ável; OOM no 4GB; retopo > orçamento de tris (3k-4.5k) | Placeholder-first (F2-PROD.2) tira arte do critical path do M.1/M.3; **F2-ART.SPIKE** decide AI-base vs hand-model com dado real; fallback = hand-model low-poly direto | W3 (fecha spike) |
| R-02 | §18 Ambientes inflar F2-E.11 (catálogo completo mantido no VS por F2-PROD.4) e consumir buffer | Scope | Média × Médio | F2-E.11 começa a roubar dias do critical path; W4/W5 deslizando por causa de variações de arena | Catálogo está FORA do critical path do M.1; **trade já desenhado (F2-PROD.4)**: cair p/ 2-3 arenas representativas ou zero (MVV §4 aceita 0); adiar catálogo p/ pós-VS | W4 (já materializado parcial) |
| R-03 | Solo + deep-lore paralelo (F1-DL.4-9): dispersão, lore "nunca retomar" OU lore roubando tempo de código (risco do próprio ADR-001) | Op/Health | Alta × Médio | 3+ sessões seguidas sem avanço de código E sem avanço de lore; sensação de troca de contexto constante | Deep-lore = descanso, NUNCA bloqueia código (ADR-001); WIP-limit mental de 1 frente técnica por vez; **revisão obrigatória amarrada a F2-PROD.8 no M.1** | F2-M.1 |
| R-04 | Cadeia de arte serial vira caminho crítico se placeholder-first não segurar (modelo→rig→anim em sequência, 1 pessoa) | Schedule | Baixa × Alto | Playtest M.3 sendo adiado "até ter modelo"; tarefas G.2..G.4/G.10 entrando antes do loop fechar | Disciplina placeholder-first (F2-PROD.2 + plano §2): graybox fecha M.1/M.3 antes de qualquer arte final; ordem de corte do buffer (plano §5) preserva o loop | W5 |
| R-05 | Export físico bloqueado: templates 4.6 mono ausentes + `export_presets.cfg` ausente (bloqueio silencioso de M.4) | Tech/Op | Média × Médio | Primeiro `--export-release` falha por template faltante; descoberta tardia na semana do M.4 | **F2-CI.5** (templates), **F2-CI.6** (presets), **F2-CI.1** (wrappers) antes do M.4; smoke em VM clean (plano §1 M.4) | W6 |

## A — Assumptions (premissa falsa = vira risco)

| # | Premissa | Se falsa, vira | Como validar |
|---|---|---|---|
| A-01 | O pipeline AI (nano banana → TripoSR só-shape) produz topologia retopo-ável dentro do budget de tris (3k-4.5k) sem PBR | R-01 sobe para Alta×Alto; arte final só por hand-model | F2-ART.SPIKE em W3 com 1 char (Gus): se reprovar, fallback hand-model já assumido |
| A-02 | Placeholder-first segura o fun-loop: graybox + material flat bastam p/ medir time-to-fun ≤ 5min no M.3 sem arte final | R-04 sobe; M.3 passa a depender de arte | Playtest N=3 em graybox no M.3; se testadores travarem por falta de leitura visual, reavaliar |

## D — Dependencies (caminho crítico — cross-ref, não re-derivado)

| # | Dependência | Tipo | Onde está mapeada |
|---|---|---|---|
| D-01 | F2-G.1 → F2-E.10 → F2-E.10b → F2-G.5/G.9 (combate jogável com dados reais) | Interna / critical path | `plano_vs.md` §2 + F2-PROD.5 (aresta E.10b confirmada) |
| D-02 | F2-E.11 depende de F2-E.5b (status inertes precisam EXISTIR antes de ambientes modificá-los) | Interna | `plano_vs.md` §2; TODO notas §264 |
| D-03 | F2-M.4 (build distribuível) depende de export templates + presets + wrappers (não de CI Forgejo completo) | Externa / tooling | `plano_vs.md` §2 M.4; ligada a R-05 |

---

## Política de revisão (mínima)

- Tocar este doc a cada fechamento de onda: mudou Prob.×Impacto? Gatilho disparou? Mitigação ainda válida?
- Risco materializado ou gatilho disparado → AskUserQuestion ao criador com trade-off explícito (cortar MVV / estender prazo / aceitar débito), conforme buffer policy `plano_vs.md` §5. Nunca slip silencioso.
- Risco morto → mover p/ "Encerrados" com 1 linha de fechamento (não apagar, preserva histórico p/ post-mortem).

### Encerrados

(vazio — nenhum risco fechado ainda)
