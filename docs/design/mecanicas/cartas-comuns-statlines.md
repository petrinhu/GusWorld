# Statlines das cartas COMUNS (baseline //PLAYTEST)

**Status:** BASELINE aprovado pelo líder 2026-07-16 como ponto de partida do playtest N=3 (proposta do `lead-game-designer`, ancorada no canon). Números `//PLAYTEST` afináveis. **Nomes = PROVISÓRIOS** (passada de naming pelo `narrative-writer` pendente; o líder lê antes de aprovar). Cross-ref: `deck-mao-sistema.md` (§8b catálogo, §8c números), `cartas-technomagik.md` §2.2, `combat.md` §6/§7/§9/§11.

## Fundamentos (do canon)
- **Comuns NÃO passam pelo executor techMagic (ADR-016)** — isso é exclusivo de ESPECIAL/SUPER. Comuns usam o record-base de carta (`combat.md §7`, `StatusApplied`) + a fórmula divisiva §11 + `StatusId` já existentes. **Zero EffectKind novo.**
- Curva **mana→power: 1→3, 2→5, 3→8** (canon §2.2/§11).
- Status por família só os 2 canônicos de `combat.md §6`. Identidade não-sobreposta (anti feature-creep).
- Template de 6 arquétipos por família (30 comuns).

## Decisões do líder 2026-07-16 (ajustes sobre a proposta)
- **Finalizador-sinérgico = OPÇÃO A (extensão de engine):** campo `SynergyStatus` no record da comum + checagem no `resolve_use_card` (generaliza o `multExpose` que já existe só pro Expose) → `+40%` dano se o alvo já tem o status da família. NÃO é EffectKind novo, mas é engine além de dados → **item CARTAS-COMUNS-ENGINE (SynergyStatus)**.
- **Elétrico-utilidade = RECARGA DE AP/MANA (engine novo):** o líder escolheu a versão forte (não o fallback Haste). Devolver AP ou mana via carta comum não existe hoje → **item CARTAS-COMUNS-ENGINE (recarga-recurso)**; mexe na economia de recurso do combate (vigiar balance).

---

## Elétrico (Cauã "Volt") — burst single-target, Stun

| Arquétipo | Nome provisório | Mana | Power | Efeito/status | Intenção |
|---|---|---|---|---|---|
| Jab | Tavusa-Pulso | 1 | 3 | — | spam barato |
| Golpe+status | Tavusa-Choque | 2 | 5 | Stun 1 turno //PT | dano + trava (1 AP) |
| Assinatura | Tavusa-Arco | 3 | 8 | — | burst puro, identidade |
| Status-puro | Tavusa-Trava | 1 //PT | 0 | Stun 1 turno | trava barata |
| Utilidade | Tavusa-Overclock | 2 //PT | 0 | **RECARGA de AP/mana (engine novo)** //PT | overclock de recurso (forte, o líder escolheu esta) |
| Finalizador | Tavusa-Fulminante | 3 | 8 | +40% se alvo tem Stun (SynergyStatus) //PT | execute em alvo travado |

## Bioquímico (Jaci "Proxy") — DoT/degradação, Poison/Corrode

| Arquétipo | Nome provisório | Mana | Power | Efeito/status | Intenção |
|---|---|---|---|---|---|
| Jab | Erynin-Espinho | 1 | 3 | — | picada barata |
| Golpe+status | Erynin-Infecção | 2 | — | Poison DoT 5/tick×3 = 15 (CANON) | a DoT é o golpe |
| Assinatura | Erynin-Toxina | 3 | — | Poison DoT 8/tick×3 = 24 //PT | DoT forte |
| Status-puro | Erynin-Ferrugem | 2 //PT | 0 | Corrode, Def −4, 4t //PT | controle (Def down) |
| Utilidade | Sylvesse-Religação | 2 //PT | 0 | Regen +3/turno (CANON), 3t //PT self/aliado | única cura do jogo (exclusiva Bio) |
| Finalizador | Erynin-Epidemia | 3 | 8 | +40% se alvo tem Poison/Corrode (SynergyStatus) //PT | recompensa manter doente |

## Sônico (Linda "Siren") — área-CC/interrupção, Disrupt/Silence

| Arquétipo | Nome provisório | Mana | Power | Efeito/status | Intenção |
|---|---|---|---|---|---|
| Jab | Lhinin-Estalo | 1 | 3 | — | hit barato |
| Golpe+status | Lhinin-Ruído | 2 | 5 | Disrupt −30% Power próxima ação, 1t //PT | dano + sabotagem |
| Assinatura | Lhinin-Onda | 3 | 5/alvo //PT | AoE (Grupo/Área), Disrupt em cada, 1t | área-CC de verdade |
| Status-puro | Lhinin-Estático | 2 //PT | 0 | Disrupt −50% (teto canon), 2t //PT | controle forte |
| Utilidade | Lhinin-Silêncio | 2 //PT | 0 | Silence (bloqueia cartas), 2t //PT | trava o kit do alvo |
| Finalizador | Lhinin-Ressonância | 3 | 8 | +40% se alvo tem Disrupt/Silence (SynergyStatus) //PT | pune quem foi calado |

## Cinético (Bento "Requiem") — impacto/deslocamento, Knockback/Break

| Arquétipo | Nome provisório | Mana | Power | Efeito/status | Intenção |
|---|---|---|---|---|---|
| Jab | Ondha-Impacto | 1 | 3 | — | soco barato |
| Golpe+status | Ondha-Empurrão | 2 | 5 | Knockback (DelayCurrent 1, one-shot) | dano + adia o alvo na fila |
| Assinatura | Ondha-Terremoto | 3 | 5/alvo //PT | Linha (2-3 alvos), Knockback em cada | reposiciona múltiplos |
| Status-puro | Ondha-Fratura | 2 //PT | 0 | Break, Def −6, 4t //PT | debuff de Def puro |
| Utilidade | Ondhesse-Blindagem | 2 //PT | 0 | Shield (Mag = Def do lançador, canon §9) self/aliado | o tanque protege o time |
| Finalizador | Ondha-Colapso | 3 | 8 | +40% se alvo tem Knockback/Break (SynergyStatus) //PT | pune alvo desestabilizado |

## Criptográfico (Iara "Lumen") — utilidade/anti-buff, Expose/Decrypt

| Arquétipo | Nome provisório | Mana | Power | Efeito/status | Intenção |
|---|---|---|---|---|---|
| Jab | Rimin-Sonda | 1 | 3 | — | hit barato |
| Golpe+status | Rimin-Brecha | 2 | 5 | Expose +20% dano-de-carta recebido, 2t //PT | abre vulnerabilidade + cobra |
| Assinatura | Rimin-Backdoor | 3 | 5 //PT | Expose +40% 3t + Decrypt (dispel buffs) | expose forte + anti-buff |
| Status-puro | Rimin-Vulnerabilidade | 2 //PT | 0 | Expose +40%, 3t | prepara o alvo pro time |
| Utilidade | Rimin-Decrypt | 2 //PT | 0 | Decrypt puro (dispel TODOS buffs) | anti-buff |
| Finalizador | Rimin-Exploit | 3 | 8 | dano puro — o `multExpose` §9 já multiplica de graça se há Expose | ÚNICO finalizador zero-engine |

_(Cripto usa o `multExpose` global que já existe — não precisa do SynergyStatus.)_

---

## Riscos a vigiar no N=3 (do parecer do lead-game-designer)
1. **"Golpe+status" (mana2/power5+status) pode dominar** sobre "dano puro" e "status-puro" no mesmo custo — a defesa é a economia de AP (1 carta=1 AP vs 2 cartas=2 AP de um pool de 3). Medir taxa de escolha; se dominar, baixar o Power do combo (5→3).
2. **Bioquímico pode parecer fraco em combate curto** (DoT só entrega em 3 turnos). Comparar DPS-efetivo vs as 4.
3. **Criptográfico depende de sinergia de time** (Expose amplifica o dano de todos). Verificar se o jogador solo/IA explora.

## Pendências
- **Naming** das 30 (narrative-writer; voz do mundo; líder lê antes de aprovar) — cruza com a frente LINGUAGENS-COMICAS-DISPUTAS (Pythia/Óxido/Asmódico/C-Arcane).
- **Engine:** CARTAS-COMUNS-ENGINE = SynergyStatus (Finalizador Opção A) + recarga-de-recurso (Elétrico-utilidade). Ambos pré-req da impl das comuns.
- **Frases pedagógicas** por carta (didática) + VFX — dentro de CARTAS-PRODUCAO.
