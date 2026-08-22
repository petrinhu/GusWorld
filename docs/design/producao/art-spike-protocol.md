# Protocolo do Spike de Arte AI (F2-ART.SPIKE)

**Status:** Protocolo aprovado; **EXECUÇÃO DEFERIDA** (decisão criador 2026-06-03: arte fora do caminho crítico via placeholder-first; rodar quando arte virar prioridade, pós loop graybox-jogável). Sprint 6 W3. F2-ART.SPIKE.
**Cross-ref:** raid-log.md R-01/A-01, style-guide.md, TODO F2-ART.SPIKE.

> Eu produzo o ROTEIRO. O criador executa nas mãos dele (time-box rígido de 1 semana). O objetivo único do spike: validar ou refutar **A-01** (pipeline AI gera topologia retopo-ável dentro de 3k-4.5k tris, sem PBR) com DADO REAL. Char de teste: **Gus**. Hardware confirmado: RTX 3050 Laptop **4096 MiB** (ceiling); floor = iGPU.

## 1. Premissa sob teste e o que falseia

A-01 diz que `nano banana (frontal) → TripoSR (só shape)` entrega mesh que vira low-poly retopo-ável no budget. O spike é falseável: se qualquer critério de FAIL (§4) dispara dentro da semana, A-01 cai e R-01 sobe pra Alta×Alto → fallback hand-model (§5).

## 2. Setup de toolchain (Dia 0, ~½ dia)

| Item | O que | Checagem de hardware |
|---|---|---|
| Blender | 4.2 LTS (`flatpak` ou tar.xz oficial). **Ausente hoje** — instalar é pré-req. | abre + viewport Eevee roda |
| TripoSR | repo MIT (stability-ai/TripoSR) + venv Python 3.11/3.12 (3.14 do sistema é novo demais p/ torch — usar venv dedicado). torch CUDA. | `python run.py --help` sem erro |
| Godot | 4.6.1 (já instalado, `~/.local/bin/godot`). | já validado |
| nvidia-smi / radeontop | já tem nvidia-smi. `radeontop` p/ medir iGPU floor. | `nvidia-smi` lista a 3050 |

**Gate de hardware (Dia 0, antes de qualquer arte):** rodar TripoSR no exemplo do repo com `nvidia-smi -l 1` aberto. Se pico de VRAM ≤ ~3.5 GB → segue. Se OOM no 4 GB mesmo no exemplo → tentar `--chunk-size` menor / `mc-resolution` baixa; se ainda OOM → **abortar AI-base, ir pro fallback (§5)**. Não queimar a semana lutando contra os 4 GB.

## 3. Passos executáveis (Dias 1-4)

| # | Etapa | Entrada → Saída | Tool | Tempo est. |
|---|---|---|---|---|
| P1 | Imagem frontal do Gus | prompt do char-spec §4 → 1 PNG frontal limpo, fundo `#FFFFFF`, T-pose-ish | nano banana | 30-60 min |
| P2 | TripoSR só-shape | PNG → mesh bruto `.obj`/`.glb` (sem textura PBR) | TripoSR | 5-15 min/run + iterações |
| P3 | Import bruto | mesh → cena Blender, escala/orientação corrigidas (Y-up p/ Godot, aplicar transform) | Blender | 15 min |
| P4 | Retopo | mesh denso → low-poly **3k-4.5k tris**, quad-dominante, edge loops em ombro/cotovelo/joelho/boca se for riggar | Blender (Quad Remesher manual / retopo à mão) | 2-6 h (o gargalo real) |
| P5 | UV + normais | low-poly → UV sem stretch crítico, normais flat-export (style-guide §10), gradient atlas placeholder 256² | Blender | 1-2 h |
| P6 | Export | → `gus_spike.gltf` (glTF 2.0, scale/rotation aplicados, sem embed) | Blender | 10 min |
| P7 | Import Godot + toon | glTF → cena Godot, material toon (`diffuse_toon`) + outline inverted-hull se F2-A.2 pronto | Godot | 30 min |
| P8 | Render de juízo | screenshot in-engine, luz neutra + rim do bioma; silhouette test rápido | Godot | 30 min |

Nota: rig/skinning **fora do spike** (é F2-G.x). Mas no P4 avaliar SE a topologia *permitiria* rig (edge flow), porque isso pesa no gate.

## 4. Budget mensurável + critérios PASS/FAIL (falseáveis)

| Métrica | Onde medir | PASS | FAIL |
|---|---|---|---|
| Tris low-poly | Blender Statistics overlay | 3.000-4.500 | > 4.500 sem caber, ou silhueta SD 1:1:1 perdida abaixo de 3.000 |
| VRAM TripoSR | `nvidia-smi -l 1` no pico (P2) | ≤ ~3.5 GB | OOM no 4 GB irrecuperável |
| Topologia | viewport wireframe (P4) | quad-dominante, edge flow utilizável, sem n-gon na face/articulações | tri-soup não-retopo-ável / força refazer do zero à mão |
| Draw calls | Godot debug monitor (P7) | ≤ 2 (mesh + outline) | — (não é o risco aqui) |
| VRAM in-Godot floor | radeontop na iGPU OU estimativa | cena com Gus + atlas 256² cabe folgado | — |
| Tempo total do spike | relógio | mesh import-ável + retopo no budget em **≤ ~12 h úteis** dentro da semana | estoura o time-box de 1 semana |
| Silhouette | screenshot preto P8 | Gus reconhecível em 3 s | irreconhecível (mas isso é de imagem/retopo, não veredito sozinho) |

**Veredito AI-base PASSA** se: TripoSR roda no 4 GB **E** P4 entrega retopo 3k-4.5k quad-dominante utilizável **E** tudo dentro do time-box. **FALHA** se qualquer um destes quebra de forma irrecuperável.

## 5. Gate de decisão (fim da semana) — AI-base vs hand-model

Decisão binária objetiva, aplicada pelo criador:

- **TODOS os PASS de §4 batem** → adotar **AI-base-mesh** como pipeline de char. Documentar settings vencedores (resolução TripoSR, fluxo de retopo) como receita reprodutível pros demais 7 chars.
- **Qualquer FAIL irrecuperável** → adotar **hand-model low-poly direto** (fallback já assumido em R-01). O tempo do spike NÃO é perdido: vira baseline de "quanto custa um char à mão" pro budget de cronograma.
- **Zona cinza** (roda mas retopo do AI demora ~igual ou mais que modelar à mão) → AskUserQuestion ao criador com os números medidos. NÃO decidir sozinho (one-way-ish: define pipeline dos 8 chars).

Em qualquer dos casos, atualizar R-01/A-01 no raid-log (W3 = "fecha spike").

## 6. Instrumentação (o que registrar — 1 tabela no fim do spike)

Por etapa P1-P8: tempo real gasto, VRAM pico (P2), tris (P4), screenshot (P8), 1 linha de observação. Sem PMO: é um log de 8 linhas que alimenta o gate §5 e a revisão do raid-log. Salvar em `assets/models/_spike/` junto dos artefatos.

## 7. Riscos do próprio spike + abortar cedo

- **Abortar no Dia 0** se TripoSR nem instala/roda no 4 GB → pular direto pro fallback (§5), não gastar a semana.
- **WIP-limit:** 1 char (Gus) só. Não generalizar nem polir textura — é shape + topologia, nada mais.
- **Não cair no buraco do retopo perfeito:** o spike mede *viabilidade*, não entrega o asset final do Gus (isso é F2-G.2). Retopo "bom o bastante pra julgar" basta.
- **Time-box é lei:** bateu 1 semana sem veredito claro → conta como FAIL parcial → fallback + AskUserQuestion. Slip silencioso é proibido (política do raid-log §39-42).
- **Placeholder-first protege:** mesmo FAIL total não trava M.1/M.3 (graybox segue), só confirma que arte final do char é hand-model.

## Decisões (estado 2026-06-03)

- **Execução do spike = DEFERIDA** (decisão criador). Arte está fora do caminho crítico (placeholder-first cobre M.1/M.3); foco atual = combate jogável (código). Este protocolo fica pronto e guardado; roda quando a arte virar prioridade.
- **Pipeline de char (AI-base vs hand-model):** a decidir QUANDO o spike rodar (ou pular direto pra hand-model). Leitura técnica do 3d-artist: A-01 frágil, fallback hand-model provável (tri-soup + cel-shading exige normais limpas + char SD barato à mão). Não pré-comprometido.
- **Defaults a confirmar na execução:** Blender 4.2 LTS (addons de retopo maduros); retopo FOSS (RetopoFlow / Quad Remesher manual), sem ZBrush. Confirmar orçamento de ferramenta paga só se/quando rodar.
