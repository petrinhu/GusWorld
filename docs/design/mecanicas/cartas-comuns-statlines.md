# Statlines das cartas COMUNS (baseline //PLAYTEST)

**Status:** BASELINE aprovado pelo líder 2026-07-16 como ponto de partida do playtest N=3 (proposta do `lead-game-designer`, ancorada no canon). Números `//PLAYTEST` afináveis. **Nomes = PROVISÓRIOS** (passada de naming pelo `narrative-writer` pendente; o líder lê antes de aprovar). Cross-ref: `deck-mao-sistema.md` (§8b catálogo, §8c números), `cartas-technomagik.md` §2.2, `combat.md` §6/§7/§9/§11.

> ## ⚠ VELOCIDADE = CANON PÉTREO (não é `//PLAYTEST`)
>
> **A velocidade das 30 comuns foi aprovada pelo líder CARTA A CARTA em 2026-07-17.** Não é baseline afinável, não é decisão autônoma de agente, não é número de balance.
>
> **Motivo:** o eixo compilado × interpretado é **definição pessoal do Gus original** e uma **promessa do líder ao menino**. Isso a coloca **acima dos pillars** na hierarquia de canon: um pillar pode ser revisado em brainstorm, este eixo não.
>
> **Regra de alteração:** qualquer mudança de velocidade de qualquer carta exige **autorização explícita do líder**, naquele contexto, carta a carta. Aprovação anterior não vale pra frente. Nenhum agente inverte uma linha "por balance".
>
> **Se o playtest N=3 acusar problema:** o remédio é mexer em **Power / mana / duração de status / quantas casas a lenta anda na fila**, nunca na velocidade. A velocidade é premissa, o resto se ajusta em volta dela.
>
> **Histórico:** a atribuição anterior (2026-07-17, "8 exceções") **INVERTIA o eixo** e foi SUSPENSA e refeita com o líder. Ver §"Correção do framework" no fim do doc.

## Fundamentos (do canon)
- **Comuns NÃO passam pelo executor techMagic (ADR-016)**, isso é exclusivo de ESPECIAL/SUPER. Comuns usam o record-base de carta (`combat.md §7`, `StatusApplied`) + a fórmula divisiva §11 + `StatusId` já existentes. **Zero EffectKind novo.**
- Curva **mana→power: 1→3, 2→5, 3→8** (canon §2.2/§11).
- Status por família só os 2 canônicos de `combat.md §6`. Identidade não-sobreposta (anti feature-creep).
- Template de 6 arquétipos por família (30 comuns).

## Decisões do líder 2026-07-16 (ajustes sobre a proposta)
- **Finalizador-sinérgico = OPÇÃO A (extensão de engine):** campo `SynergyStatus` no record da comum + checagem no `resolve_use_card` (generaliza o `multExpose` que já existe só pro Expose) → `+40%` dano se o alvo já tem o status da família. NÃO é EffectKind novo, mas é engine além de dados → **item CARTAS-COMUNS-ENGINE (SynergyStatus)**.
- **Elétrico-utilidade = RECARGA DE AP/MANA (engine novo):** o líder escolheu a versão forte (não o fallback Haste). Devolver AP ou mana via carta comum não existe hoje → **item CARTAS-COMUNS-ENGINE (recarga-recurso)**; mexe na economia de recurso do combate (vigiar balance).

---

## Elétrico (Cauã "Volt"): burst single-target, Stun

| Arquétipo | Nome provisório | Mana | Power | Efeito/status | Intenção |
|---|---|---|---|---|---|
| Jab | Tavusa-Pulso | 1 | 3 | (nenhum) | spam barato |
| Golpe+status | Tavusa-Choque | 2 | 5 | Stun 1 turno //PT | dano + trava (1 AP) |
| Assinatura | Tavusa-Arco | 3 | 8 | (nenhum) | burst puro, identidade |
| Status-puro | Tavusa-Trava | 1 //PT | 0 | Stun 1 turno | trava barata |
| Utilidade | Tavusa-Overclock | 2 //PT | 0 | **RECARGA de AP/mana (engine novo)** //PT | overclock de recurso (forte, o líder escolheu esta) |
| Finalizador | Tavusa-Fulminante | 3 | 8 | +40% se alvo tem Stun (SynergyStatus) //PT | execute em alvo travado |

## Bioquímico (Jaci "Proxy"): DoT/degradação, Poison/Corrode

| Arquétipo | Nome provisório | Mana | Power | Efeito/status | Intenção |
|---|---|---|---|---|---|
| Jab | Erynin-Espinho | 1 | 3 | (nenhum) | picada barata |
| Golpe+status | Erynin-Infecção | 2 | (n/a) | Poison DoT 5/tick×3 = 15 (CANON) | a DoT é o golpe |
| Assinatura | Erynin-Toxina | 3 | (n/a) | Poison DoT 8/tick×3 = 24 //PT | DoT forte |
| Status-puro | Erynin-Ferrugem | 2 //PT | 0 | Corrode, Def −4, 4t //PT | controle (Def down) |
| Utilidade | Sylvesse-Religação | 2 //PT | 0 | Regen +3/turno (CANON), 3t //PT self/aliado | única cura do jogo (exclusiva Bio) |
| Finalizador | Erynin-Epidemia | 3 | 8 | +40% se alvo tem Poison/Corrode (SynergyStatus) //PT | recompensa manter doente |

## Sônico (Linda "Siren"): área-CC/interrupção, Disrupt/Silence

| Arquétipo | Nome provisório | Mana | Power | Efeito/status | Intenção |
|---|---|---|---|---|---|
| Jab | Lhinin-Estalo | 1 | 3 | (nenhum) | hit barato |
| Golpe+status | Lhinin-Ruído | 2 | 5 | Disrupt −30% Power próxima ação, 1t //PT | dano + sabotagem |
| Assinatura | Lhinin-Onda | 3 | 5/alvo //PT | AoE (Grupo/Área), Disrupt em cada, 1t | área-CC de verdade |
| Status-puro | Lhinin-Estático | 2 //PT | 0 | Disrupt −50% (teto canon), 2t //PT | controle forte |
| Utilidade | Lhinin-Silêncio | 2 //PT | 0 | Silence (bloqueia cartas), 2t //PT | trava o kit do alvo |
| Finalizador | Lhinin-Ressonância | 3 | 8 | +40% se alvo tem Disrupt/Silence (SynergyStatus) //PT | pune quem foi calado |

## Cinético (Bento "Requiem"): impacto/deslocamento, Knockback/Break

| Arquétipo | Nome provisório | Mana | Power | Efeito/status | Intenção |
|---|---|---|---|---|---|
| Jab | Ondha-Impacto | 1 | 3 | (nenhum) | soco barato |
| Golpe+status | Ondha-Empurrão | 2 | 5 | Knockback (DelayCurrent 1, one-shot) | dano + adia o alvo na fila |
| Assinatura | Ondha-Terremoto | 3 | 5/alvo //PT | Linha (2-3 alvos), Knockback em cada | reposiciona múltiplos |
| Status-puro | Ondha-Fratura | 2 //PT | 0 | Break, Def −6, 4t //PT | debuff de Def puro |
| Utilidade | Ondhesse-Blindagem | 2 //PT | 0 | Shield (Mag = Def do lançador, canon §9) self/aliado | o tanque protege o time |
| Finalizador | Ondha-Colapso | 3 | 8 | +40% se alvo tem Knockback/Break (SynergyStatus) //PT | pune alvo desestabilizado |

## Criptográfico (Iara "Lumen"): utilidade/anti-buff, Expose/Decrypt

| Arquétipo | Nome provisório | Mana | Power | Efeito/status | Intenção |
|---|---|---|---|---|---|
| Jab | Rimin-Sonda | 1 | 3 | (nenhum) | hit barato |
| Golpe+status | Rimin-Brecha | 2 | 5 | Expose +20% dano-de-carta recebido, 2t //PT | abre vulnerabilidade + cobra |
| Assinatura | Rimin-Backdoor | 3 | 5 //PT | Expose +40% 3t + Decrypt (dispel buffs) | expose forte + anti-buff |
| Status-puro | Rimin-Vulnerabilidade | 2 //PT | 0 | Expose +40%, 3t | prepara o alvo pro time |
| Utilidade | Rimin-Decrypt | 2 //PT | 0 | Decrypt puro (dispel TODOS buffs) | anti-buff |
| Finalizador | Rimin-Exploit | 3 | 8 | dano puro (o `multExpose` §9 já multiplica de graça se há Expose) + **CONSOME o Expose do alvo ao resolver** //PT | RÁPIDA; o custo é queimar a janela |

_(Cripto usa o `multExpose` global que já existe, não precisa do SynergyStatus.)_

**Rimin-Exploit: efeito NOVO (decisão do líder 2026-07-17).** A carta agora **consome (remove) o status Expose do alvo** quando resolve. Diegese: você explorou a falha, e a falha **foi corrigida**. Nenhum outro exploit entra por aquela porta.

- **Por que mudou:** o risco antigo da carta era a **espera** (ela era LENTA via `async`). O `async` morreu (Iara é Óxido, Óxido compila, o cast é rápido). Um finalizador rápido que aproveita Expose de graça e **deixa o Expose de pé** não paga nada. O consumo é o **risco novo que substitui o risco da espera**: mesma tensão, honestidade técnica preservada.
- **Consequência de jogo:** o time tem que **escolher a hora**. Expose é buff de time (amplifica o dano de todos, §9). Queimar o Expose com o Exploit é ganho imediato × dano coletivo dos outros. Decisão real, não automática.
- **Interação:** Exploit continua **zero-engine** no lado do dano (usa o `multExpose` global), mas a remoção do status **é engine nova** (limpar um `StatusId` do alvo pós-resolve). Verificar se o `resolve_use_card` já tem primitiva de remoção (o `Decrypt` dispela buffs, mas Expose é debuff no alvo, canal diferente). Se não tiver: **item novo pro `backend-engineer`** (`consume_status` no record da comum), escopo pequeno, mesmo padrão data-driven do `SynergyStatus`. Vira pré-requisito de CARTAS-PRODUCAO.
- **Ordem obrigatória:** consumir **DEPOIS** de aplicar o multiplicador. Se consumir antes, a carta perde o próprio bônus (bug de ordem). Caso de teste explícito pro `qa-engineer`.

---

## NOMES + frases pedagógicas (aprovado pelo líder 2026-07-16; SUPERA os provisórios das tabelas acima)

Cada carta = **sintaxe real da linguagem-âncora do dono** + uma frase que ensina um conceito de programação de leve (público 11+). Voz consistente por família. (`narrative-writer`; sem latim bíblico no Bento, compliance religião real.)

**⚡ Elétrico / Cauã (Pythia):** `zap()` · `except Choque:` · `import fulgor` · `sleep(1)` · `yield mais()` · `finally: fulmina()`
**🧪 Bioquímico / Jaci (Pythia):** `self.picar()` · `for tick in praga:` · `class Epidemia:` · `self.def -= 4` · `self.hp += cura` · `if infectado: dano *= 1.4`
**🔊 Sônico / Linda (Óxido):** `&eco` · `&mut ruido` · `panic!("eco")` · `borrow_conflitante()` · `mutex.lock()` · `unwrap()`
**🔐 Criptográfico / Iara (Óxido):** `Some(golpe)` · `dbg!(alvo)` · `#[derive(Debug)]` · `pub alvo` · `drop(alvo)` · `Ok(dano)`
**💥 Cinético / Bento (Asmódico):** `MOV.golpe` · `PUSH.impacto` · `PUSH.multiplex` · `SUB.custodia` · `CALL.tutela` · `HLT.ultima`

(Frases pedagógicas completas no relatório do `narrative-writer`, a canonizar 1:1 quando produzir o doc de conteúdo final.) Flags: 3 nomes longos (`#[derive(Debug)]`, `borrow_conflitante()`, `mutex.lock()`) podem pedir fonte menor/2 linhas na moldura (art-director); nome exibido vs slug interno = decisão de impl.

## VELOCIDADE (compilada/rápida × interpretada/lenta): CANON, líder, carta a carta, 2026-07-17

> **Esta seção SUBSTITUI INTEGRALMENTE a atribuição de 2026-07-16/17 ("afinidade não trava" + "8 exceções"), que está ERRADA e SUSPENSA.** A versão velha invertia o eixo compilado × interpretado. Ver §"Correção do framework".

### A régua-lei: conjurar = compilar + executar

O jogo **já mostra isso na tela**. A fase de cast da carta rápida é, verbatim (`combat-flavor.md` §1):

```
> compilando...   > linkando...   -> EXEC
```

A velocidade **não é uma regra imposta por cima da lore**. Ela é a **consequência aritmética** do que o jogo já afirma que acontece: se conjurar é compilar e executar, então **quanto custa compilar é quanto demora conjurar**. A velocidade de cada carta CAI DELA SOZINHA, sem inventar nada e **sem mentir em nenhum ponto**.

| Linguagem | O que acontece de verdade ao conjurar | Velocidade |
|---|---|---|
| **Asmódico** (Assembly) | **Não compila. Monta.** Já é código de máquina, só traduz símbolo pra opcode | **O MAIS RÁPIDO** |
| **C-Arcane** (Gus) | **Compila rápido.** É a fama real do C, e é merecida | **RÁPIDO** (o ponto doce) |
| **Óxido** (Rust) | Compila **devagar**: borrow checker + monomorfização + LLVM. E **`async` é o pior caso notório** | **RÁPIDO** no simples, **LENTO** no `async` |
| **Pythia** (Python) | **Não compila. Interpreta em runtime**, linha a linha, toda vez | **LENTO** |

### As duas exceções honestas (que PROVAM a regra, não furam)

**1. Pythia rápida quando GENUINAMENTE COMPILA.** Não é "a carta é especial". É que **naquele caso específico o Python compila de verdade**:

- **`@jit` (Numba/PyPy) compila mesmo.** Vira código de máquina. Não é metáfora, é o que a ferramenta faz.
- **Builtin do CPython É escrito em C.** Roda rápido porque **não é Python rodando**, é C compilado embaixo do capô (é o fato da A1/A2 dos apartes do Gus: *"tem um C-Arcane ali embaixo fazendo o trabalho pesado"*).

Repare no que isso faz: a Pythia só fica rápida **quando para de ser interpretada**. Isso **PROVA a tese do Gus** com muito mais força do que uma exceção furada proveria. Toda vez que a Pythia é rápida, é porque compilou.

**2. Híbrido: a lerdeza vem da parte interpretada.** Se o cast é Asmódico mas o **executor final é Pythia**, a carta é **LENTA**, e **a culpa é do Python, não do Assembly**. O Assembly fez a parte dele em nanossegundos e ficou esperando o interpretador. É o caso único da `Ondha-Fratura`.

### 🚫 MENTIRA PROIBIDA (regra inegociável)

> **"Não minta JAMAIS para uma criança."**

**NUNCA afirmar, em nome nenhum, em frase de cast nenhuma, em aparte nenhum:**

- ❌ que interpretado é rápido
- ❌ que compilado é lento **por ser compilado**

Ambas são falsas e este doc existe porque foram ditas uma vez. Um agente que precisar de uma carta lenta numa família compilada **não inventa um hook**: ele acha o **motivo real** (o `async` do Rust é lento de verdade; o executor Pythia do híbrido é lento de verdade) ou **não faz a carta lenta**.

Isto é irmão da régua dos apartes (`gus-apartes-c-arcane.md`): **fato é proibido de ser falso, juízo é livre.** O Gus pode achar o Óxido feio. Ele não pode dizer que interpretado é rápido.

### As 30 cartas: velocidade FINAL (CANON, aprovada carta a carta)

`R` = rápida/compilada (`> compilando... > linkando... -> EXEC`, falha em `ERRO DE COMPILAÇÃO`)
`L` = lenta/interpretada (`> parse: montando AST... > interpretando... -> EVAL`, falha em `RUNTIME ERROR`)

#### ⚡ Elétrico / Cauã (Pythia): **5L / 1R**

| Carta | Arquétipo | Vel | Motivo REAL |
|---|---|---|---|
| Tavusa-Pulso | Jab | **L** | Pythia interpreta. **O REPL não salva** (REPL continua interpretado) |
| Tavusa-Choque | Golpe+status | **L** | Pythia interpreta |
| Tavusa-Arco | Assinatura | **R** | **`@jit` compila de verdade** (função mais quente do Cauã) |
| Tavusa-Trava | Status-puro | **L** | Pythia interpreta |
| Tavusa-Overclock | Utilidade | **L** | Pythia interpreta |
| Tavusa-Fulminante | Finalizador | **L** | Pythia interpreta |

#### 🧪 Bioquímico / Jaci (Pythia): **4L / 2R**

| Carta | Arquétipo | Vel | Motivo REAL |
|---|---|---|---|
| Erynin-Espinho | Jab | **R** | **Builtin do CPython é C compilado** (não é Python rodando) |
| Erynin-Infecção | Golpe+status | **L** | Pythia interpreta |
| Erynin-Toxina | Assinatura | **L** | Pythia interpreta |
| Erynin-Ferrugem | Status-puro | **L** | Pythia interpreta |
| Sylvesse-Religação | Utilidade (CURA) | **R** | **`@jit(nogil=True)` compila de verdade** |
| Erynin-Epidemia | Finalizador | **L** | Pythia interpreta |

#### 🔊 Sônico / Linda (Óxido): **5R / 1L**

| Carta | Arquétipo | Vel | Motivo REAL |
|---|---|---|---|
| Lhinin-Estalo | Jab | **R** | Óxido compila. Rust simples é rápido |
| Lhinin-Ruído | Golpe+status | **R** | Óxido compila |
| Lhinin-Onda | Assinatura | **R** | Óxido compila |
| Lhinin-Estático | Status-puro | **R** | Óxido compila |
| Lhinin-Silêncio | Utilidade | **R** | Óxido compila |
| Lhinin-Ressonância | Finalizador | **L** | **`async` em Rust é o pior caso real de tempo de compilação** |

#### 💥 Cinético / Bento (Asmódico): **5R / 1L**

| Carta | Arquétipo | Vel | Motivo REAL |
|---|---|---|---|
| Ondha-Impacto | Jab | **R** | Asmódico **monta**, não compila. O mais rápido |
| Ondha-Empurrão | Golpe+status | **R** | Asmódico monta |
| Ondha-Terremoto | Assinatura | **R** | Asmódico monta |
| Ondha-Fratura | Status-puro | **L** | **HÍBRIDO: cast Asmódico, executor final Pythia.** A culpa é do Python |
| Ondhesse-Blindagem | Utilidade | **R** | Asmódico monta |
| Ondha-Colapso | Finalizador | **R** | Asmódico monta. **O risco é o SETUP, não a espera** |

#### 🔐 Criptográfico / Iara (Óxido): **6R / 0L**

| Carta | Arquétipo | Vel | Motivo REAL |
|---|---|---|---|
| Rimin-Sonda | Jab | **R** | Óxido compila |
| Rimin-Brecha | Golpe+status | **R** | Óxido compila |
| Rimin-Backdoor | Assinatura | **R** | Óxido compila |
| Rimin-Vulnerabilidade | Status-puro | **R** | Óxido compila |
| Rimin-Decrypt | Utilidade | **R** | Óxido compila |
| Rimin-Exploit | Finalizador | **R** | Óxido compila (nada de `async`). **Risco = consome o Expose** |

### PLACAR FINAL: **19 rápidas / 11 lentas**

| Família | Dono | Linguagem | R | L |
|---|---|---|---|---|
| Elétrico | Cauã | Pythia | 1 | **5** |
| Bioquímico | Jaci | Pythia | 2 | **4** |
| Sônico | Linda | Óxido | **5** | 1 |
| Cinético | Bento | Asmódico | **5** | 1 |
| Criptográfico | Iara | Óxido | **6** | 0 |
| **TOTAL** | | | **19** | **11** |

Conferido: 1+2+5+5+6 = **19 R**. 5+4+1+1+0 = **11 L**. Soma = **30**. ✅

**Leitura de design:** o eixo agora é **legível de olho**. As duas famílias Pythia carregam quase toda a lerdeza (9 das 11 lentas); as três compiladas somam 2 lentas, e cada uma tem um **motivo real e nomeável** (`async` do Rust; executor Pythia do híbrido). O jogador aprende o eixo **jogando**, sem ninguém explicar, e o que ele aprende é **verdade**.

### O que é `//PLAYTEST` aqui (e o que NÃO é)

**NÃO é `//PLAYTEST` (canon pétreo, só o líder muda):**
- ❌ A velocidade de qualquer uma das 30
- ❌ A régua-lei (conjurar = compilar + executar)
- ❌ As duas exceções honestas e seu formato
- ❌ A proibição de mentir

**É `//PLAYTEST` (afinável livremente no N=3):**
- ✅ **Quantas casas** na fila CTB a lenta anda (por carta? fixo? `cardSpeedMult` do ActionClock, `ADR-017`)
- ✅ **Quanto** a lenta ganha de potência/flexibilidade em troca da espera
- ✅ O que a **interrupção** faz (cancela e perde AP/mana? atrasa? reduz potência?)
- ✅ **Power / mana / duração de status** de qualquer carta
- ✅ O `+40%` do `SynergyStatus`

**Se o N=3 disser "o Cauã é frustrante, tudo dele é lento":** o remédio é **compensar** (mais Power, mais flexibilidade, menos casas de espera), **nunca** inverter a velocidade. A lerdeza da Pythia é o conceito, não o bug. É exatamente o que o líder quer que o jogador SINTA.

### Watchlist N=3 (consequências reais a medir)

1. **Cauã 5L/1R = o mais afetado.** Um Striker cujo jab é lento é uma proposta de jogo incomum. **É intencional** (ver o aparte da Pulso). Medir frustração vs graça. Se doer: compensar em Power/casas de espera, **jamais** em velocidade.
2. **CC lenta (Tavusa-Trava, Stun):** a régua-mestre VELHA mandava "CC = rápida sempre, senão chega tarde e é carta desperdiçada". A régua-lei nova **derruba isso**, e o líder aceitou o custo: um Stun que chega tarde é **exatamente a lição** de que interpretado te trai na hora H. Vigiar se whiffa demais; remédio = duração do Stun ou casas de espera.
3. **Tavusa-Overclock lento (recarga de recurso):** recurso que chega tarde vale menos. Mesmo raciocínio, mesmo remédio.
4. **Cripto 6R/0L = a família 100% confiável.** Verificar se vira escolha automática. Se dominar, o remédio é o **custo** (mana/Power), não injetar uma lenta artificial.
5. **Rimin-Exploit consumindo Expose:** medir se a decisão "queimo ou guardo" acontece de verdade ou se o jogador queima sempre no automático.
6. **Golpe+status** (5/5 no custo 2) pode acusar monotonia. Herdado da watchlist anterior, segue válido.

### 💡 Insight de design a registrar: o Finalizador nunca precisou do risco da espera

A versão velha desenhou o arquétipo **Finalizador = LENTA sempre**, com a justificativa "o risk/reward É apostar que o SynergyStatus segue de pé".

**Isso está errado por dois motivos independentes:**

1. **É ilegal nas famílias compiladas.** Pra fazer o finalizador do Bento (Asmódico, o mais rápido que existe) ser lento, foi preciso **inventar um hook** ("vetor de interrupção") cujo efeito líquido era **fazer Assembly parecer lento**. O líder pegou isso e criticou com razão. O arquétipo estava **forçando a mentira**: a regra "finalizador é lento" colidiu com a régua-lei, e em vez de o arquétipo ceder, a verdade cedeu. Foi essa a mecânica exata do erro.

2. **O risco já existia, no lugar certo.** O custo real do finalizador **sempre foi o SETUP**: você gastou **turnos e cartas** aplicando o status-gatilho antes de poder jogá-lo. Esse é o preço, e ele é pago **antes**. A espera era um **segundo imposto** cobrado em cima de um investimento já feito. Redundante, não elegante.

**A conclusão vira design, não desculpa:**

> **Finalizador rápido = cobra a aposta COM confiabilidade = a tese do Gus.**
> Compilado é melhor porque **ENTREGA**. Você investiu o setup, e o compilado **honra o investimento**. O interpretado te faz investir E esperar E ainda pode falhar.

O `Ondha-Colapso` rápido não é o Bento ficando forte demais: é o jogo **ensinando o eixo** no momento mais emocionante do combate. E repare no fecho: o finalizador do Cauã (`Tavusa-Fulminante`) **continua lento**, porque Pythia. Mesmo arquétipo, mesma função, velocidades opostas, e a diferença é **só a linguagem**. Isso é o eixo se explicando sozinho, sem uma linha de exposição.

**Regra derivada (aplicar às ESPECIAIS/SUPER também):** nenhum arquétipo dita velocidade. **A linguagem dita.** O arquétipo dita **custo, Power e condição**. Se um arquétipo futuro "precisar" ser lento numa família compilada, o arquétipo está errado, não a régua.

## Riscos a vigiar no N=3 (do parecer do lead-game-designer)
1. **"Golpe+status" (mana2/power5+status) pode dominar** sobre "dano puro" e "status-puro" no mesmo custo. A defesa é a economia de AP (1 carta=1 AP vs 2 cartas=2 AP de um pool de 3). Medir taxa de escolha; se dominar, baixar o Power do combo (5→3).
2. **Bioquímico pode parecer fraco em combate curto** (DoT só entrega em 3 turnos). Comparar DPS-efetivo vs as 4.
3. **Criptográfico depende de sinergia de time** (Expose amplifica o dano de todos). Verificar se o jogador solo/IA explora.

## Pendências
- **Naming** das 30 (narrative-writer; voz do mundo; líder lê antes de aprovar), cruza com a frente LINGUAGENS-COMICAS-DISPUTAS (Pythia/Óxido/Asmódico/C-Arcane).
- **Frases pedagógicas** por carta (didática) + VFX, dentro de CARTAS-PRODUCAO.
- **`consume_status` pro Rimin-Exploit** (efeito novo do líder 2026-07-17): checar se o `resolve_use_card` já remove `StatusId` do alvo; se não, item pro `backend-engineer` (escopo pequeno, data-driven). Pré-req de CARTAS-PRODUCAO.
- **Decisão do líder pendente:** corrigir `combat-flavor.md` §2 (framework, "afinidade não trava" = raiz do erro) e o mapeamento `C-Arcane = C` vs `C-Arcane Major = C++`. Ver §"Correção do framework".

## Engine (CARTAS-COMUNS-ENGINE, FEITO 2026-07-16, `backend-engineer`, TDD + gêmeo preview<->real)

- **SynergyStatus (Finalizador Opção A):** `Card::synergy_statuses`/`synergy_percent` generaliza o `multExpose` (`combat.md` §9/§11) pra qualquer `StatusId`; +40% fixo (`//PLAYTEST`, data-driven, sem hardcode) quando ≥1 status da lista está presente no alvo, sem stack por-status. Wired em `resolve_use_card`/`estimate_card_damage` (`GusEngine/domain/src/combat/combat_state_machine.cpp`), campos em `Card` (`GusEngine/domain/include/gus/domain/combat/combat_records.hpp`). Log diegético `[SINERGIA: alvo vulnerável, dano +N%]`. Paridade verificada com Quantum-Lock/Planck (degraus sobre a base já amplificada).
- **Recarga de AP/mana (Elétrico-utilidade "Tavus-Overclock"):** `Card::restore_ap`/`restore_mana`; trava 1×/turno (`CombatActor::overclock_used()`, resetada no refresh de `TurnStart`, `GusEngine/domain/src/combat/combat_actor.cpp`); custo sempre pago antes; Silence bloqueia; log de sucesso e de bloqueio.
- Testes: `GusEngine/domain/tests/cartas_comuns_engine_test.cpp` (16 casos, 51 assertions): sinergia (dispara/ausente/não-stack/gêmeo extremos/gêmeo crítico/contagem RNG/interação Planck/log), recarga (persistência do bônus de AP/clamp de mana/reset da trava/1ª e 2ª jogada/reset no turno seguinte/custo pago antes/Silence).
- Status: 🔍 Pendente verificação (item CARTAS-COMUNS-ENGINE do TODO.md). Pré-requisito da impl das 30 comuns (CARTAS-PRODUCAO) cumprido.

---

## Re-voz das cartas de velocidade notável (REESCRITA 2026-07-17, pós-decisão do líder)

> **Esta seção SUBSTITUI a "Re-voz das 8 exceções" (PROPOSTA de 2026-07-17), que está MORTA.** Aquela proposta reconciliava a voz de 8 cartas contra uma atribuição de velocidade que estava **invertida**. Corrigido o eixo, **5 das 8 deixaram de precisar de reconciliação** (a velocidade delas virou a velocidade natural da linguagem do dono) e **3 hooks foram descartados por serem falsos**.

**De 8 exceções para 3.** Só sobrou "exceção" onde a Pythia é rápida, e **essas três não são exceção de verdade**: são os três casos em que **a Pythia genuinamente compila**. Elas não furam a regra, elas **provam** a regra.

### Placar da correção

| Carta | Antes | Agora | O que aconteceu com o hook |
|---|---|---|---|
| Tavusa-Arco | exceção R | **R, exceção honesta** | ✅ **SOBREVIVE.** `@jit` compila de verdade |
| Erynin-Espinho | exceção R | **R, exceção honesta** | ✅ **SOBREVIVE.** Builtin do CPython é C |
| Sylvesse-Religação | exceção R | **R, exceção honesta** | ✅ **SOBREVIVE.** `@jit(nogil=True)` compila de verdade |
| Tavusa-Pulso | exceção R | **L, regra pura** | ❌ **HOOK MORTO.** "REPL é rápido" era **FALSO**: o REPL continua interpretado |
| Rimin-Exploit | exceção L | **R, regra pura** | ❌ **HOOK MORTO.** O `async` sai. Risco novo = consome o Expose |
| Ondha-Colapso | exceção L | **R, regra pura** | ❌ **HOOK MORTO.** O vetor de interrupção fazia **Assembly parecer lento** |
| Lhinin-Ressonância | exceção L | **L, motivo NOVO** | 🔄 Continua lenta. Não é mais "o eco viaja": é `async` de Rust |
| Ondha-Fratura | exceção L | **L, motivo NOVO** | 🔄 Continua lenta. Agora é **HÍBRIDO** (executor Pythia) |

Convenção: `frase de cast` = o texto técnico da tela de conjuração (registro dev completo, `combat-flavor.md` §1). **Todos os nomes mantidos** (nenhum precisou mudar). Nenhuma carta muda de linguagem-âncora nem de família.

---

### ✅ As 3 exceções que SOBREVIVEM (Pythia rápida PORQUE compila)

#### 1. Tavusa-Arco (Cauã, Elétrico, Assinatura): **RÁPIDA**

- **Frase de cast:** `@jit` seguido de `def arco(): fulgor.disparar()`
- **Motivo real:** JIT (Numba/PyPy) **recompila pra código de máquina** uma função "quente", chamada muitas vezes. Não é metáfora: a ferramenta compila.
- **Por que fecha:** a assinatura de um personagem é, por definição, o golpe **mais treinado** dele. É A função mais quente do repertório do Cauã, então já está compilada quando o jogador a usa. E repare: ela é rápida **exatamente na medida em que deixou de ser Python**.
- **Nome:** "Arco" (arco voltaico, carga que acumula e descarrega) combina com algo que esquentou até virar caminho direto.

#### 2. Erynin-Espinho (Jaci, Bioquímico, Jab): **RÁPIDA**

- **Frase de cast:** `self.picar()`, com marca visual de **builtin** (mono verde, ícone C) em vez de `def` (mono branco, ícone Pythia) na moldura.
- **Motivo real:** builtins do CPython são **implementados em C** e pulam o laço de despacho do interpretador. Quando roda um builtin, **não é Python rodando**, é C compilado. É o fato exato dos apartes A1/A2 (*"tem um C-Arcane ali embaixo fazendo o trabalho pesado"*).
- **Por que fecha:** a picada da Erynin é **reflexo, não pensamento** (ela não processa o alvo, reage). É a única ação do kit da Jaci que roda por baixo do Pythia, direto no substrato C.
- **Nome:** "Espinho" já é reflexo puro na natureza (planta que fere sem decidir).
- **Nota de arte (importante):** a marca visual de builtin **carrega a explicação inteira sem uma linha de texto**. Vale o cuidado do `art-director`: é o único lugar do jogo em que o jogador VÊ o C aparecendo por baixo da Pythia.

#### 3. Sylvesse-Religação (Jaci, Bioquímico, Utilidade/CURA): **RÁPIDA**

- **Frase de cast:** `@jit(nogil=True)` seguido de `def religacao(): self.hp += cura`
- **Motivo real:** `nogil` é opção real (Numba/Cython) que **compila** e libera a trava global do interpretador numa chamada crítica. Compila de verdade, roda sem fila.
- **Por que fecha:** Religação é a **ÚNICA cura do jogo**. Código de emergência é pré-compilado justamente porque não pode esperar a fila de interpretação. Emergência fura fila, e fura fila **compilando**.
- **Nome:** "Religação" já é reconectar às pressas. A urgência está no nome.

---

### 🔄 As 2 que continuam LENTAS, com motivo NOVO e verdadeiro

#### 4. Lhinin-Ressonância (Linda, Sônico, Finalizador): **LENTA**

- **Frase de cast:** `async fn ressonancia(eco: Onda) -> Dano { eco.amplificar().await }`
- **Motivo real (SUBSTITUI o antigo):** **`async` em Rust é o pior caso notório de tempo de compilação.** Não é "o eco precisa viajar" (aquilo era justificativa **física**, decorativa, e por isso frágil: não explicava por que a linguagem demora). O motivo agora é o motivo **de verdade**: a carta demora porque **`async` de Óxido é caro de compilar**, e isso é fato documentado, não invenção.
- **Por que fecha:** é a única lenta de uma família rápida, e o motivo é **nomeável e específico dessa carta**. As outras 5 do Sônico não usam `async`, e por isso são rápidas. A regra se sustenta sozinha.
- **APARTE DO GUS (canon aprovado, verbatim):**

> "Eu aviso e vocês não me escutam... Reinventando a roda quando a linguagem perfeita já existe. E essa lerdeza toda é o compilador de vocês brigando pra decidir quem é dono de cada byte. Em C-Arcane eu simplesmente SEI de quem é."

**Nota de voz:** este aparte é o irmão do **A27** (*"brigar com o verificador"*) e do **A11** (*"tem gente que desistiu do Óxido só de esperar ele compilar"*). Fato verdadeiro: o borrow checker é parte real do custo de compilação do Rust, e "brigar com o borrow checker" é expressão consagrada. Juízo (livre): que isso é lerdeza. **Alvo:** Linda.

#### 5. Ondha-Fratura (Bento, Cinético, Status-puro): **LENTA** (HÍBRIDO)

- **Frase de cast:** cast em Asmódico puro, e o **executor final em Pythia** (a moldura mostra a virada: o registro Asmódico entrega pra um `def` interpretado. **A troca de registro NA TELA é a piada** e é o que ensina o híbrido sem explicar).
- **Motivo real (SUBSTITUI o DMA):** a carta é **HÍBRIDA**: o **cast é Asmódico, mas o executor final é Pythia**. A lerdeza vem **da parte interpretada**. **A culpa é do Python, não do Assembly.** O Asmódico fez a parte dele instantaneamente e ficou esperando o interpretador.
- **Por que o DMA morreu:** DMA é conceito real, mas usá-lo aqui produzia a leitura **"Assembly é lento"**, que é **falsa** e é a mentira que este doc existe pra matar. O híbrido resolve com a verdade: **Assembly continua o mais rápido**, e a lerdeza tem **um culpado nomeado**.
- **Stats:** mantém **Def -6 / 4 turnos** (inalterado).
- **APARTE DO GUS (canon aprovado, verbatim):**

> "Tu tinha a pureza do Asmódico no core... e enfiou um executor Pythia em cima. Até Óxido é melhor que essa gambiarra. Estragou teu próprio projeto, Bento."

**Nota de voz:** raríssimo e valioso. É o **único aparte em que o Gus DEFENDE o Asmódico** ("a pureza do Asmódico no core") e o **único em que ele elogia o Óxido**, mesmo que só como pau pra bater ("até Óxido é melhor"). Rima com o **A15** (*"esses cantinhos são MESMO seus, eu não saberia fazer"*), o aparte em que o Bento quase sorri. Aqui a implicância é **de engenheiro pra engenheiro**: ele não está zoando o Bento por usar Asmódico, está indignado porque o Bento **sujou** o Asmódico. Isso é respeito disfarçado de bronca, e é a relação Gus x Bento (`party.md`: "Modernidade vs tradição") no melhor momento dela. **Alvo:** Bento.

---

### ❌ Os 3 hooks DESCARTADOS (e por quê, pra ninguém ressuscitar)

Registro obrigatório: os três hooks abaixo **estão mortos** e **não devem voltar** em nenhum doc, frase de cast ou proposta futura. Todos morreram pelo mesmo motivo: **afirmavam fato falso.**

| Hook morto | Carta | Por que é FALSO |
|---|---|---|
| **REPL one-liner é rápido** | Tavusa-Pulso | ❌ O REPL do Python **continua interpretado**. Digitar direto no prompt **não compila nada**. O hook trocava "compilar" por "não precisar salvar arquivo", que **não é a mesma coisa** e não afeta a velocidade de execução. Era a mentira mais sutil das três, e por isso a mais perigosa |
| **`async`/`.await` como espera diegética** | Rimin-Exploit | ❌ O recurso é real, mas era usado como **"o efeito demora pra chegar"** (semântica de runtime), quando o custo real do `async` de Rust é **tempo de COMPILAÇÃO**. Usar o mesmo hook pra Ressonância (compilação, correto) e pra Exploit (espera diegética, incorreto) provava que o hook estava sendo usado como **curinga**, não como fato |
| **Vetor de interrupção / `IRET`** | Ondha-Colapso | ❌ Conceito real, conclusão falsa: fazia **Assembly parecer lento**, quando Assembly **nem compila, só monta**, e é **o mais rápido que existe**. O líder pegou este e criticou com razão. Foi o hook que expôs o erro inteiro |

**A lição, pra não repetir:** os três hooks eram **tecnicamente verificáveis** (REPL existe, `async` existe, vetor de interrupção existe) e **mesmo assim mentiam**, porque a **conclusão** que sustentavam era falsa. Citar um recurso real **não** é o mesmo que dizer a verdade sobre ele.

> **Teste que teria pegado os três:** não pergunte "esse recurso existe?". Pergunte: **"a frase que sobra afirma que interpretado é rápido, ou que compilado é lento?"** Se sim, o hook está morto, por mais real que o recurso seja.

### 🌟 A Tavusa-Pulso: a ironia que o líder pediu pra preservar

`Tavusa-Pulso` é o **jab do Striker**. Jab = a carta mais barata, mais spammada, a que o jogador aperta sem pensar. **Todo instinto de design diz que ela é rápida.** A versão velha caiu exatamente nisso (o default dizia "Jab = RÁPIDA sempre").

**Ela é LENTA.** E é a melhor carta do conjunto pra ensinar o eixo, porque o líder colocou a lição no lugar onde ela **dói**:

> Palavras do líder: *"algo que era ESPERADO ser rápido e eficiente é na verdade chato e lento, é o conceito do Gus pra Python."*

O jogador **espera** que o jab seja rápido. O Cauã **jura** que a Pythia dele é rápida, sólida e eficiente. E aí ele aperta, e **espera**. Não é punição arbitrária: é a **tese do jogo inteiro** entregue na carta que o jogador usa mais vezes do que qualquer outra. A frustração **é** o ensino, e ela é **honesta** (Python é lento mesmo).

- **Frase de cast:** Pythia interpretada padrão (`> abrindo env... > parse: montando AST... > interpretando... > abstraindo... -> EVAL`). **Sem prompt de REPL, sem `>>>`, sem atalho**: o hook morreu e não deixa resíduo visual.
- **APARTE DO GUS (canon aprovado, verbatim):**

> "Isso aí que tu jura que é 'rápido, sólido e eficiente'? Tá esperando até agora. Em C-Arcane já tinha acabado, tomado banho e voltado."

**Nota de voz:** o *"tu jura"* é o detalhe que faz a frase. O Gus não está atacando a Pythia em abstrato, está cobrando **a propaganda que o Cauã faz dela**. É a distinção da régua dos apartes funcionando perfeitamente: **fato** (Python interpretado é lento, verdade) + **juízo** (que isso é vexame, opinião) + **queixa real** (a fama de lentidão do Python existe de verdade no mundo). *"Tomado banho e voltado"* é exagero cômico de criança de 11 anos, **não** é afirmação factual de tempo, então não viola a régua. **Alvo:** Cauã.

---

### Notas de escopo

- **Todos os motivos são reais.** `@jit`/`nogil` (Numba/PyPy compilam), builtin do CPython (é C), `async` do Rust (custo real de compilação), híbrido com executor Pythia (a lerdeza vem do interpretado). Nenhuma carta troca de linguagem-âncora. Coerente com Pillar 1 (magia = software, analogia real).
- **Os 3 apartes acima são CANON aprovado**, verbatim, não opcionais (diferente da proposta velha, cujos apartes eram sugestões marcadas "opcional"). Entram no pool de disparo em combate (`gus-apartes-c-arcane.md`, "Regras de disparo": em combate o Gus solta mesmo com inimigo presente, é a face "Dragon").
- **Pendência herdada:** a frequência de aparte **em batalha** é *"até 1x a cada X minutos, X ainda a definir com o líder"* (`gus-apartes-c-arcane.md`, AMB-02). Com 3 apartes atrelados a **cartas específicas**, o gatilho pode não ser só tempo: pode ser **evento** (a carta foi jogada) com cooldown por cima. **Decisão do líder**, não fechar aqui.
- **Pendência de UI (herdada, aplica-se aos 3):** requisito do líder em `gus-apartes-c-arcane.md`: frases longas aparecem **sem clipar o contêiner e sem reduzir a fonte** a ponto de atrapalhar a leitura. Os 3 apartes acima são longos. Vale pro glintfx.
- **Frases de cast completas** (com todas as fases visuais de `combat-flavor.md` §1) ficam pro doc de conteúdo final quando o naming das 30 for aprovado. Aqui só o núcleo de voz × velocidade.
- **Naming das 25 restantes:** segue a seção "NOMES + frases pedagógicas" acima, sem mudança. **Atenção:** aquela seção lista `sleep(1)` e `yield mais()` pro Elétrico, hoje coerentes (a família é lenta). Mas o `>>>` no prompt do Cauã (`combat-flavor.md` §5, `caua@pythia:~$ >>>`) **não** ressuscita o hook do REPL: lá é **caracterização de prompt** (o Cauã "vive no REPL"), não afirmação de velocidade. Manter, sem virar justificativa de rapidez.

---

## Correção do framework: `combat-flavor.md` §2 está SUPERADO (a raiz do erro)

**Registro obrigatório.** A atribuição errada não foi descuido pontual: ela foi **autorizada pelo framework**. A causa-raiz tem endereço.

### A cláusula que abriu a porta

`combat-flavor.md` §2, "Amarração FECHADA (decisão do criador 2026-07-16)", diz **verbatim**:

> "a linguagem/velocidade e **propriedade da CARTA**, com **AFINIDADE (nao exclusividade)** a linguagem-ancora comica do personagem (...) mas ele PODE ter cartas de outra velocidade **quando o role/balance pede**"

**Por que isso produziu o erro:** a cláusula rebaixa a linguagem a **tendência estética** e promove **role/balance** a critério que **vence** a linguagem. Traduzindo o que ela autoriza: *"se o balance pedir, faça a carta interpretada ser rápida."* Foi obedecendo a isso que:

1. o arquétipo virou lei ("Finalizador = LENTA sempre", "Jab = RÁPIDA sempre"), acima da linguagem;
2. quando arquétipo e linguagem colidiram, **a linguagem cedeu** (era só "afinidade", afinal);
3. pra vestir a colisão, inventaram-se hooks que **afirmavam fato falso** (Assembly lento, REPL rápido).

O erro foi **sistemático, não aleatório**: seguiu a regra escrita, e a regra estava errada. Por isso corrigir só as cartas não basta.

### Status: SUPERADO

> A cláusula "afinidade, NÃO trava" de `combat-flavor.md` §2 está **SUPERADA** pela **régua-lei** desta seção VELOCIDADE (líder, 2026-07-17).
>
> **A linguagem TRAVA.** Não é afinidade, não é tendência, não é sabor. É **lei**, e está **acima de role, de balance e de arquétipo**, porque o eixo compilado × interpretado é definição do Gus original (acima dos pillars).
>
> Balance NUNCA inverte velocidade. Balance ajusta **Power, mana, duração e casas de espera**. A velocidade é premissa; o balance trabalha **em volta** dela.

### ❓ DECISÃO PENDENTE DO LÍDER: corrigir o `combat-flavor.md` também?

**Minha recomendação: SIM, e com prioridade.** Motivos:

1. **`combat-flavor.md` é o doc de FRAMEWORK; este aqui é de CONTEÚDO.** Anotar "superado" no doc de conteúdo conserta as 30 cartas de hoje, mas deixa a cláusula errada de pé **na fonte que os agentes vão ler amanhã**. O próximo agente que abrir o framework pra desenhar as **ESPECIAIS/SUPER** vai ler "afinidade, não trava" + "role/balance pode pedir" e **repetir o erro inteiro**, de boa-fé.
2. **A superfície de risco é maior que as comuns.** As especiais/SUPER passam pelo executor techMagic (ADR-016) e são **narrativa protegida** (`reference_deck_mao_sistema`). Errar o eixo lá é mais caro que nas comuns.
3. **Contradição ativa entre dois docs canônicos.** Hoje o §2 diz "não trava" e este diz "trava". Dois canônicos em oposição direta = o próximo leitor escolhe **o que der na telha**.

**Escopo cirúrgico que eu proporia (nada além disto):**

- Reescrever o parágrafo "Amarração FECHADA" do §2: "afinidade, não trava" → **régua-lei** (conjurar = compilar + executar; a linguagem trava; balance ajusta em volta).
- **Corrigir a tabela do §2**, que tem uma imprecisão factual própria: `Óxido | Rust | compilada | rapida` está **incompleto**. Óxido compila **devagar**, e **`async` é o pior caso**. A linha precisa refletir isso, senão contradiz a `Lhinin-Ressonância`.
- **Adicionar a linha do HÍBRIDO** na tabela (cast numa linguagem, executor em outra, e a lerdeza vem do executor interpretado). Hoje o caso da `Ondha-Fratura` **não existe** no framework.
- **Gravar a MENTIRA PROIBIDA** no §2, como regra de redação, com o teste de detecção ("a frase que sobra afirma que interpretado é rápido ou que compilado é lento?").
- **Manter** o §2 onde ele já acerta: `Asmodico | montada (baixo nivel) | rapida (a mais)` **já está correto** e já antecipava a régua nova. Bom sinal: o framework tinha a verdade e a cláusula "afinidade" a atropelou.

**Aguardando ordem do líder.** Não toquei em `combat-flavor.md` nesta passada (fora do escopo autorizado).

---

## Termos canônicos novos (2026-07-17)

| Termo diegético | Real | Nota |
|---|---|---|
| **C-Arcane Major** | **C++** | Criado pelo líder em 2026-07-17 (resolve AMB-01 de `gus-apartes-c-arcane.md`). Mantém a convenção de nunca citar nome real de linguagem em diálogo. Estreia na **frase rara** (o "bilhete dourado", a única em que o Gus critica o C-Arcane): *"Dá pra fazer classe em C-Arcane puro, sabia? Não é BEM classe... é struct com ponteiro de função. O C-Arcane Major só deixou fácil pra quem tem preguiça."* |

**⚠ Consequência a resolver (achado desta passada):** com `C-Arcane Major = C++` canonizado, **`C-Arcane` deveria mapear pra C sozinho**. Mas duas tabelas canônicas ainda dizem `C-Arcane | C / C++`:

- `combat-flavor.md` §2 (tabela de linguagens)
- `gus-apartes-c-arcane.md` (tabela de convenção diegética)

Não é urgente (não afeta velocidade: C e C++ compilam, ambos rápidos), mas é **inconsistência real** e vai confundir. **Sugestão:** `C-Arcane = C` e `C-Arcane Major = C++`, atualizando as duas tabelas. **Decisão do líder**, junto com a do `combat-flavor.md`.
