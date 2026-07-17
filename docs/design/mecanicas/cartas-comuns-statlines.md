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

## NOMES + frases pedagógicas (aprovado pelo líder 2026-07-16 — SUPERA os provisórios das tabelas acima)

Cada carta = **sintaxe real da linguagem-âncora do dono** + uma frase que ensina um conceito de programação de leve (público 11+). Voz consistente por família. (`narrative-writer`; sem latim bíblico no Bento — compliance religião real.)

**⚡ Elétrico / Cauã (Pythia):** `zap()` · `except Choque:` · `import fulgor` · `sleep(1)` · `yield mais()` · `finally: fulmina()`
**🧪 Bioquímico / Jaci (Pythia):** `self.picar()` · `for tick in praga:` · `class Epidemia:` · `self.def -= 4` · `self.hp += cura` · `if infectado: dano *= 1.4`
**🔊 Sônico / Linda (Óxido):** `&eco` · `&mut ruido` · `panic!("eco")` · `borrow_conflitante()` · `mutex.lock()` · `unwrap()`
**🔐 Criptográfico / Iara (Óxido):** `Some(golpe)` · `dbg!(alvo)` · `#[derive(Debug)]` · `pub alvo` · `drop(alvo)` · `Ok(dano)`
**💥 Cinético / Bento (Asmódico):** `MOV.golpe` · `PUSH.impacto` · `PUSH.multiplex` · `SUB.custodia` · `CALL.tutela` · `HLT.ultima`

(Frases pedagógicas completas no relatório do `narrative-writer` — a canonizar 1:1 quando produzir o doc de conteúdo final.) Flags: 3 nomes longos (`#[derive(Debug)]`, `borrow_conflitante()`, `mutex.lock()`) podem pedir fonte menor/2 linhas na moldura (art-director); nome exibido vs slug interno = decisão de impl.

## VELOCIDADE (compilada/rápida × interpretada/lenta) — decisão do líder 2026-07-16

Amarração FECHADA (ver `combat-flavor.md §2`): **velocidade = propriedade da CARTA, com AFINIDADE (não trava) à linguagem do dono.** Interpretada (Pythia) = lenta (resolve casas à frente na fila CTB, mais potente, vulnerável a interrupção → `RUNTIME ERROR`); compilada (C-Arcane/Óxido/Asmódico) = rápida (resolve no turno, efeito fechado → `ERRO DE COMPILAÇÃO`). Afinidade: Cauã/Jaci tendem lento (Pythia), Iara/Linda/Bento tendem rápido; MAS por role/balance uma carta pode fugir (ex.: a assinatura-burst do Cauã pode ser rápida, aí ganha voz compilada). **ATRIBUÍDO** (parecer do lead-game-designer, **decisão autônoma 2026-07-17 — confirmar retroativamente**; tudo `//PLAYTEST`, o N=3 inverte qualquer linha sem custo = só o campo de velocidade no record, plugado no `cardSpeedMult` do ActionClock `ADR-017`).

**Régua-mestre (acima da afinidade):** o efeito perde sentido se resolver DEPOIS da próxima ação do alvo/aliado? **SIM → RÁPIDA sempre** (CC/proteção/heal de emergência que chegam tarde = carta desperdiçada); **NÃO → segue a afinidade da família** e pode ir LENTA pra ganhar potência/tensão (DoT que "se espalha", trava temporizável, execute que aposta na sinergia seguir de pé).

**Defaults por arquétipo:** Jab = RÁPIDA sempre (filler 1 mana, punir com interrupção é ruim p/ todas); Golpe+status = segue afinidade pura; Assinatura = tende RÁPIDA (confiabilidade do burst; exceção se for DoT); Status-puro = MISTO (família decide: Disrupt/Silence/Expose-setup = rápida por janela curta; Def/Break = lento por potência); Utilidade = tende RÁPIDA (landing garantido); Finalizador = LENTA sempre (o risk/reward É apostar que o SynergyStatus segue de pé).

**As 30 (R=rápida/compilada · L=lenta/interpretada):**
- **Elétrico/Cauã** (afim.Pythia-L): Pulso R*, Choque L, Arco R*, Trava L, Overclock L, Fulminante L → 4L/2R.
- **Bioquímico/Jaci** (afim.Pythia-L): Espinho R*, Infecção L, Toxina L, Ferrugem L, Religação(cura) R*, Epidemia L → 4L/2R.
- **Sônico/Linda** (afim.Óxido-R): Estalo R, Ruído R, Onda R, Estático R, Silêncio R, Ressonância L* → 5R/1L.
- **Cinético/Bento** (afim.Asmódico-R): Impacto R, Empurrão R, Terremoto R, Fratura L*, Blindagem R, Colapso L* → 4R/2L.
- **Criptográfico/Iara** (afim.Óxido-R): Sonda R, Brecha R, Backdoor R, Vulnerabilidade R, Decrypt R, Exploit L* → 5R/1L.

**8 exceções (`*`, velocidade contraria a afinidade do dono → re-voz do narrative-writer, EM CURSO):** Pulso/Arco/Espinho/Religação (Pythia-lenta mas RÁPIDA → hook JIT real Numba/PyPy ou reflexo bio); Ressonância/Exploit (Óxido-rápida mas LENTA → Rust `async`/`.await` real); Fratura/Colapso (Asmódico-rápida mas LENTA → Assembly DMA/deferred interrupt). Os hooks são autênticos à linguagem-âncora (não trocam de linguagem, só de registro) — resolve "exceção quebra a voz da família". + **frase cômica do Gus** ocasional no cast (defende C-Arcane, comic-reliefs C.4; densidade baixa; narrative-writer).

**Watchlist N=3:** (1) Golpe+status (5/5 afinidade pura, arquétipo com menos "tensão") pode acusar monotonia; (2) finalizadores de janela curta (Sônico 1-2t, Cripto 2-3t) podem "whiffar" se a sinergia expira antes do resolve — se punir demais, ampliar a duração do status-gatilho (não a velocidade).

## Riscos a vigiar no N=3 (do parecer do lead-game-designer)
1. **"Golpe+status" (mana2/power5+status) pode dominar** sobre "dano puro" e "status-puro" no mesmo custo — a defesa é a economia de AP (1 carta=1 AP vs 2 cartas=2 AP de um pool de 3). Medir taxa de escolha; se dominar, baixar o Power do combo (5→3).
2. **Bioquímico pode parecer fraco em combate curto** (DoT só entrega em 3 turnos). Comparar DPS-efetivo vs as 4.
3. **Criptográfico depende de sinergia de time** (Expose amplifica o dano de todos). Verificar se o jogador solo/IA explora.

## Pendências
- **Naming** das 30 (narrative-writer; voz do mundo; líder lê antes de aprovar) — cruza com a frente LINGUAGENS-COMICAS-DISPUTAS (Pythia/Óxido/Asmódico/C-Arcane).
- **Frases pedagógicas** por carta (didática) + VFX — dentro de CARTAS-PRODUCAO.

## Engine (CARTAS-COMUNS-ENGINE — FEITO 2026-07-16, `backend-engineer`, TDD + gêmeo preview<->real)

- **SynergyStatus (Finalizador Opção A):** `Card::synergy_statuses`/`synergy_percent` generaliza o `multExpose` (`combat.md` §9/§11) pra qualquer `StatusId`; +40% fixo (`//PLAYTEST`, data-driven, sem hardcode) quando ≥1 status da lista está presente no alvo, sem stack por-status. Wired em `resolve_use_card`/`estimate_card_damage` (`GusEngine/domain/src/combat/combat_state_machine.cpp`), campos em `Card` (`GusEngine/domain/include/gus/domain/combat/combat_records.hpp`). Log diegético `[SINERGIA: alvo vulnerável, dano +N%]`. Paridade verificada com Quantum-Lock/Planck (degraus sobre a base já amplificada).
- **Recarga de AP/mana (Elétrico-utilidade "Tavus-Overclock"):** `Card::restore_ap`/`restore_mana`; trava 1×/turno (`CombatActor::overclock_used()`, resetada no refresh de `TurnStart`, `GusEngine/domain/src/combat/combat_actor.cpp`); custo sempre pago antes; Silence bloqueia; log de sucesso e de bloqueio.
- Testes: `GusEngine/domain/tests/cartas_comuns_engine_test.cpp` (16 casos, 51 assertions) — sinergia (dispara/ausente/não-stack/gêmeo extremos/gêmeo crítico/contagem RNG/interação Planck/log), recarga (persistência do bônus de AP/clamp de mana/reset da trava/1ª e 2ª jogada/reset no turno seguinte/custo pago antes/Silence).
- Status: 🔍 Pendente verificação (item CARTAS-COMUNS-ENGINE do TODO.md). Pré-requisito da impl das 30 comuns (CARTAS-PRODUCAO) cumprido.

---

## Re-voz das 8 exceções de velocidade (PROPOSTA 2026-07-17, aguarda leitura do líder)

**Status: PROPOSTA do `narrative-writer`, NÃO aprovada.** As tabelas principais acima (arquétipos por família e a seção "NOMES + frases pedagógicas") continuam valendo como estão até o líder ler e aprovar esta seção; nenhum nome foi trocado nas tabelas. Escopo: reconciliar a voz (nome + frase de cast) das 8 comuns cuja VELOCIDADE contraria a afinidade de linguagem do dono (`combat-flavor.md` §2, `VELOCIDADE` acima), usando só hooks técnicos autênticos da própria linguagem-âncora, sem trocar de linguagem em nenhum caso.

Convenção: `frase de cast` é o texto técnico que aparece na tela de conjuração (registro dev completo, `combat-flavor.md` §1); o "hook" é o conceito real da linguagem que justifica a exceção. Nome mantido nas 8: em todos os casos a conotação do nome atual já sustenta a exceção sem precisar trocar (justificado carta a carta abaixo).

### 1. Tavusa-Pulso (Cauã, Elétrico, Jab): nome mantido

- Frase de cast: `>>> zap()` (prompt REPL, Enter, eco na hora; sem `def`, sem `import` por cima)
- Hook: o REPL real do Python avalia uma linha solta assim que aperta Enter, sem montar um arquivo/script inteiro primeiro. Pulso é um "one-liner" digitado direto no prompt: nunca entra na fila de parse completo que o resto da família Pythia usa, então resolve no mesmo turno.
- Por que o nome cabe: "Pulso" já sugere um tiro curto e seco, coerente com uma linha solta no REPL em vez de um script.

### 2. Tavusa-Arco (Cauã, Elétrico, Assinatura): nome mantido

- Frase de cast: `@jit` seguido de `def arco(): fulgor.disparar()` (decorador que troca bytecode por código de máquina depois de "aquecer")
- Hook: JIT real (Numba/PyPy) recompila pra máquina uma função "quente", chamada muitas vezes. A assinatura de um personagem é, por definição, o golpe mais treinado dele: narrativamente é A função mais quente do repertório de Cauã, e por isso já roda compilada quando o jogador a usa.
- Por que o nome cabe: "Arco" (de arco voltaico, carga que se acumula e descarrega) combina com a ideia de algo que esquentou até virar caminho direto.
- Aparte cômico opcional (densidade baixa, comic-reliefs C.4): Gus resmunga "Cara, você precisa de JIT pra ficar rápido? Em C-Arcane eu já nasço compilado." (leve implicância carinhosa Gus x Cauã, tom de rivalidade fraterna, não debochado)

### 3. Erynin-Espinho (Jaci, Bioquímico, Jab): nome mantido

- Frase de cast: `self.picar()`, com nota visual de "builtin" (mono verde, ícone C) em vez de "def" (mono branco, ícone Pythia) na moldura
- Hook: no CPython real, os builtins são implementados em C e rodam mais rápido que a função Python equivalente, porque pulam o laço de despacho do interpretador. A picada de Erynin é reflexo, não "pensamento" (ela nem processa o alvo, só reage); narrativamente é a única ação do kit da Jaci que roda "por baixo" do Pythia, direto no substrato C, coerente com reflexo bio-instintivo.
- Por que o nome cabe: "Espinho" já é reflexo puro na natureza (planta que fere sem decidir); não precisa mudar.

### 4. Sylvesse-Religação (Jaci, Bioquímico, Utilidade/CURA): nome mantido

- Frase de cast: `@jit(nogil=True)` seguido de `def religacao(): self.hp += cura` (libera a trava do interpretador pra rodar sem fila)
- Hook: `nogil` é opção real (Numba/Cython) pra liberar a trava global do interpretador numa chamada crítica, prioridade máxima. Religação é a ÚNICA cura do jogo: narrativamente é tratada como "código de emergência" pré-compilado, uma ligação que não pode esperar a fila normal de interpretação. Isso justifica ser a exceção rápida numa família lenta: emergência sempre fura fila.
- Por que o nome cabe: "Religação" já é o ato de reconectar algo às pressas; a urgência está no próprio nome.
- Aparte cômico opcional (densidade baixa, comic-reliefs C.4, Gus x Jaci = "aliança emocional mais profunda" em `party.md`): Gus, baixinho, enquanto ela cura alguém: "Ta, isso é rápido rápido. Quase C-Arcane." Jaci revira os olhos sem tirar a concentração da cura.

### 5. Lhinin-Ressonância (Linda, Sônico, Finalizador): nome mantido

- Frase de cast: `async fn ressonancia(eco: Onda) -> Dano { eco.amplificar().await }`
- Hook: Rust real tem `async`/`.await` de verdade (não é gambiarra, não quebra a voz Óxido). Fisicamente, ressonância exige o som ir e voltar pra amplificar; o `.await` É o tempo de viagem do eco, não um truque narrativo. Coerente com ser a única carta lenta de uma família rápida: essa carta especificamente depende de um processo assíncrono real, as outras do Sônico não.
- Por que o nome cabe: "Ressonância" já implica acúmulo/eco ao longo do tempo, não um golpe instantâneo.

### 6. Rimin-Exploit (Iara, Criptográfico, Finalizador): nome mantido

- Frase de cast: `let payload = exploit(alvo).await;`
- Hook: mesmo recurso real do Óxido (`async`/`.await`). Narrativamente, Exploit aposta que a falha (o status Expose no alvo) continua aberta até o payload assíncrono terminar de rodar; se a janela fechar antes do `.await` resolver, o exploit falha (risk/reward de finalizador). Consistente com Exploit já ser "o único finalizador zero-engine" da tabela (depende do `multExpose` global, não do `SynergyStatus`): a espera assíncrona É a dependência de sinergia, só que expressa em tempo de fila em vez de campo de dados.
- Por que o nome cabe: "Exploit" já é um termo real de segurança que descreve uma janela de oportunidade explorada, não um golpe seco.

### 7. Ondha-Fratura (Bento, Cinético, Status-puro): nome mantido

- Frase de cast: `DMA.custodia -> INT 0x1F` (a transferência começa agora, roda em segundo plano, o interrupt dispara quando termina de chegar)
- Hook: DMA (Direct Memory Access) real transfere dados em paralelo ao processador principal e avisa por interrupção quando termina, exatamente o padrão "inicia agora, completa depois" que Assembly usa de verdade (não é invenção). A Fratura vaza aos poucos (Def cai por 4 turnos), coerente com uma transferência ainda em curso, não um golpe fechado.
- Por que o nome cabe: "Fratura" já é dano que se espalha com o tempo (rachadura que cresce), não um impacto único.

### 8. Ondha-Colapso (Bento, Cinético, Finalizador): nome mantido

- Frase de cast: `CALL vetor_colapso -> IRET` (a chamada entra no vetor de interrupção, só retorna quando a fila chega nela)
- Hook: vetor de interrupção real é uma tabela de endereços que o processador consulta quando um evento assíncrono acontece; `IRET` (interrupt return) só executa quando a interrupção é finalmente atendida. Colapso é o finalizador que aposta que o alvo continua desestabilizado (Knockback/Break) quando a "interrupção" finalmente retorna, coerente com risk/reward de um finalizador lento.
- Por que o nome cabe: "Colapso" já é um evento que se prepara e desaba depois, não instantâneo.
- Aparte cômico opcional (densidade baixa, comic-reliefs C.4, Gus x Bento = "Linguagem (C-Arcane vs Asmódico). Modernidade vs tradição." em `party.md`): Gus comenta, meio impressionado meio implicante: "Vetor de interrupção, hein? Isso é literalmente 'esperar o sistema acordar'. C-Arcane nem dorme." Bento só grunhe, sem se abalar (ele venera o ancestral, não precisa de aprovação).

### Notas de escopo

- Nenhuma das 8 muda de linguagem-âncora nem de família; todos os hooks (REPL one-liner, builtin em C, JIT/`nogil`, `async`/`.await`, DMA/vetor de interrupção) são conceitos técnicos reais das respectivas linguagens (Pythia = Python, Óxido = Rust, Asmódico = Assembly), coerente com Pillar 1 (magia = software, analogia real).
- Os 2 apartes cômicos do Gus (Tavusa-Arco, Ondha-Colapso) respeitam a densidade baixa pedida ("nem todo cast") e a dualidade de voz do Gus (`project_gus_voz_personalidade`): fora de combate ele é mais cauteloso, mas o rage/implicância com hardware-e-linguagem é sempre afetuoso, nunca amargo. Ficam como OPCIONAIS, marcados pra decisão do líder incluir ou não.
- Frases de cast completas (com todas as fases visuais de `combat-flavor.md` §1, ex. `> compilando... > linkando... -> EXEC` pras rápidas e `> parse: montando AST... > interpretando... -> EVAL` pras lentas) ficam pro doc de conteúdo final quando o naming das 30 for aprovado; aqui só o núcleo que resolve a reconciliação de voz x velocidade.
- Pendência cruzada: esta proposta cobre só as 8 exceções; o naming das 22 comuns restantes (sem exceção de velocidade) segue a seção "NOMES + frases pedagógicas" já existente acima, sem mudança.
