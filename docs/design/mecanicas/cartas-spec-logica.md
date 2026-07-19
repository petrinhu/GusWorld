# Cartas: hardware/pirataria/energia — SPEC DE LÓGICA DE GAMEPLAY

**Status:** PROPOSTA (`gameplay_engineer`, 2026-07-18). Documento de design de LÓGICA — pseudocódigo, state machines, fluxos, pontos de integração. **Não é código**, não cria testes, não toca `GusEngine/`. Aguarda: (1) revisão do líder nos pontos marcados **AMB**, (2) o doc irmão de dados `cartas-spec-dados.md` (a criar pelo `backend-engineer` — este doc referencia os campos que a lógica abaixo PRECISA existir no domínio, mas não os define/nomeia como record final), (3) implementação real como onda futura (`CARDS-HARDWARE-ENGINE`, ainda não aberta no `TODO.md`).

**Cross-ref (fonte, leia antes):**
- [`cartas-hardware-pirataria-energia.md`](cartas-hardware-pirataria-energia.md) — o SISTEMA (14 seções), fonte de todo comportamento abaixo.
- [`cartas-numeros-proposta.md`](cartas-numeros-proposta.md) — os NÚMEROS (drain=ManaCost, capacidades por dificuldade, %contaminação, pesos urandom, timing adware, split Turing). Este doc **não redefine nenhum número**, só referencia por seção.
- [`cartas-technomagik.md`](cartas-technomagik.md) — taxonomia de carta (COMUM/ESPECIAL/SUPER), ManaCost, `EffectKind`/`TriggerHook` do executor `techMagic`.
- [`combat.md`](combat.md) §5 (AP/Mana), §9 (status framework), §10 (pipeline de cast + erros de compilação), §16 (event bus).
- [`deck-mao-sistema.md`](deck-mao-sistema.md) — deck/mão, invariantes anti-exploit, classe protegida (ESPECIAL/SUPER nunca infecta/nunca vai pro deck morto).
- Memória `reference_techmagic_engine_impl` — estado real de implementação do executor (`techmagic.cpp`, `combat_state_machine.cpp`), padrão pra adicionar `EffectKind` novo.
- Memória `feedback_todo_efeito_loga_terminal` — regra dura: todo efeito, bom ou ruim, loga no terminal.

**Fronteira de responsabilidade (não-sobreposição):** este doc especifica COMPORTAMENTO/FLUXO (o que acontece, quando, sob qual condição). Ele **não** define o `struct`/record final, não escolhe tipo de storage, não decide serialização/save. Onde a lógica exige um campo, listo como **"contrato pedido ao domínio"** — o `backend-engineer` decide o formato exato em `cartas-spec-dados.md`.

---

## 1. Onde isto pluga no pipeline de cast existente

O sistema de hardware/vírus **não substitui** nada de `combat.md` §10 (pipeline de 3 slots) nem §11 (fórmula de dano) — ele intercala **duas camadas novas** ao redor da resolução de carta já existente, uma ANTES (gates de pré-condição, mesma família do "ERRO DE COMPILAÇÃO") e uma DEPOIS (efeitos pós-cast, mesma família dos hooks `TriggerHook::OnCast`).

```
ActionSelect: jogador escolhe "jogar carta" (combat.md §3/§10)
        │
        ▼
┌─────────────────────────────────────────────────────────────┐
│ GATES DE PRÉ-CONDIÇÃO (ordem fixa; cada falha é um "erro de  │
│ compilação" ou um estado visível na UI — nunca silencioso)   │
│                                                                │
│  1. AP suficiente?                    (combat.md §10, já existe)
│  2. Mana suficiente?                  (combat.md §10, já existe)
│  3. Alvo válido / Null requer Scan?   (combat.md §10, já existe)
│  4. NOVO — Bateria da carta >= custo? (§3 deste doc)          │
│     → falha: carta aparece INERTE na mão, nem entra no menu   │
│       de ação (mesma UX do botão desabilitado do Null-sem-Scan)│
└─────────────────────────────────────────────────────────────┘
        │ todos os gates passam
        ▼
┌─────────────────────────────────────────────────────────────┐
│ NOVO — SEQUÊNCIA DE ADWARE (§9 deste doc)                     │
│  Se card.HasAdware → intercepta ANTES do débito de recurso    │
│  (propaganda 5s + X). Não roda se a carta não tem adware.      │
└─────────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────┐
│ DÉBITO DE RECURSO                                              │
│  Mana -= ManaCost                     (já existe)              │
│  NOVO: BateriaCarta -= ManaCost       (§3 deste doc, mesmo nº) │
└─────────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────┐
│ NOVO — VIRUS PRE-CAST (§4 deste doc)                           │
│  Se card.IsInfected: consulta o payload.                       │
│  LogicBomb com condição satisfeita AGORA → desvia o fluxo      │
│  (substitui/inverte o efeito abaixo). Outros payloads deixam   │
│  a resolução normal seguir e agem DEPOIS (pós-cast) ou são     │
│  passivos contínuos (Backdoor, fora deste ponto — §4.2).       │
└─────────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────┐
│ RESOLUÇÃO NORMAL DO EFEITO (inalterada)                        │
│  COMUM → resolvedor record-base (StatusApplied/SynergyStatuses)│
│  ESPECIAL/SUPER → techMagic::execute(OnCast, card, ctx)        │
└─────────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────┐
│ NOVO — VIRUS POST-CAST (§4 deste doc)                          │
│  Worm: aplica Slow na própria carta-fonte (mesmo se ROM/original)
│  + rola propagação 13% (3 direções, §5.2).                     │
│  Zip-bomb: dispara aqui (após o efeito nominal "pequeno" já ter │
│  resolvido, ela "incha").                                       │
└─────────────────────────────────────────────────────────────┘
        │
        ▼
   Log de terminal (TODOS os efeitos acima, sucesso/bloqueio/dissipação)
```

**Por que dois pontos de interceptação (pré e pós), não um `EffectKind` novo:** o executor `techMagic` (ADR-016) só roda pra ESPECIAL/SUPER; COMUM resolve pelo caminho record-base (`reference_techmagic_engine_impl`: "SynergyStatus não é EffectKind... exclusivo de ESPECIAL/SUPER"). Vírus/bateria têm que agir em **qualquer** tier de carta (uma comum pirata infecta igual a uma especial-falsa). Por isso a lógica deste doc embrulha o `resolve_use_card` inteiro (ponto único, agnóstico de tier), em vez de virar mais um `EffectKind` dentro do executor. Nomeio esse embrulho conceitual **`CardHardwareLayer`** (nome de trabalho, não union com nomenclatura final do backend).

---

## 2. Estado de uma carta-instância (contrato pedido ao domínio, não definido aqui)

A lógica abaixo assume que cada **instância** de carta (lembrando: "carta = instância única com ID", `deck-mao-sistema.md` §7 inv.1) carrega, além do que já existe (`Id`, `Tier`, etc.), os seguintes fatos consultáveis. **Isto é um pedido de campos ao `backend-engineer`, não uma definição de record:**

| Fato precisado pela lógica | Por quê | Onde é lido/escrito |
|---|---|---|
| Origem da carta: original (ROM) / homebrew (EPROM) / pirata (clone) | decide capacidade de bateria e % de contaminação (`cartas-numeros` §1a/§3) | leitura no gate de bateria + no roll de contaminação |
| Carga atual da bateria (unidades, mesma escala do `ManaCost`) | gate de pré-condição (§3) | decrementada a cada cast; incrementada na troca |
| Capacidade máxima da bateria | referência do gate + UI (barra de carga) | fixa por origem+dificuldade (`cartas-numeros` §1a), não muda em runtime |
| SoH (state-of-health) da **bateria física** (0-100%) | degradação por recarga (`cartas-numeros` §1b) | pertence à BATERIA como item (ver nota abaixo), não à carta |
| `IsInfected` (oculto ao jogador até diagnóstico) | gate de vírus (§4) | setado 1x na aquisição (fora do combate), nunca pelo combate |
| `VirusPayloadKind?` (nulo se não infectada) | qual handler de §4.1 roda | setado junto com `IsInfected` |
| `VirusDiagnosed` (bool) | controla o que a UI mostra ao jogador (§6) | setado pelo diagnóstico do Turing |
| Parâmetro de gatilho da Logic Bomb (se o payload for esse) | qual condição observar (§4.1.1) | sorteado 1x na infecção, dentre um pool pequeno |
| Espécie da carta (`CardSpeciesId`, para RunaDex — distinto de instância) | RunaDex é por espécie, não por cópia (§8) | consultado por eventos de bus, nunca mutado pelo combate |

**Nota — bateria como item separado:** a carta tem uma bateria **instalada**, mas a troca (§3.2) implica que baterias existem como **itens de inventário independentes** (spares carregadas, spares descarregadas que "ocupam espaço e não podem ser jogadas fora", `cartas-hardware-pirataria-energia.md` §5). Ou seja: `CardInstance.BatteriaInstalada → BatteryItem { charge, capacity, soh }`, e o inventário guarda `BatteryItem[]` soltos. A troca é uma operação de **swap de referência** entre a bateria instalada e uma bateria do inventário — não uma transferência de "carga" abstrata. Modelagem exata = decisão do `backend-engineer`.

---

## 3. Energia: bateria CR2032

### 3.1 Consumo por cast (drain = ManaCost, já fechado)

Sem eixo novo: `BateriaCarta.charge -= ManaCost_da_ação`, mesmo valor que já sai da mana do ator (`cartas-numeros` §1a, AMB-02 já resolvida: "recurso Y" = `ManaCost`, não um multiplicador extra). Regra de Híbridas (Faraday/Maxwell/Newton/von Neumann/John Dee, `cartas-technomagik.md` §2.3): a face **passiva** (`ManaCost=0`, sempre ligada) nunca drena; só a face **ativa**, quando de fato disparada (`OnCast` do lado castável), drena as ~6 unidades.

```
on_card_selected_for_play(card_instance, ap_cost, mana_cost):
    if card_instance.battery.charge < mana_cost:
        # gate de pré-condição — a carta NUNCA aparece selecionável
        ui.mark_inert(card_instance)          # mesma UX do botão Null-sem-Scan
        return REJECTED                        # não consome AP nem entra no log de combate
    # ... segue pros demais gates existentes (AP, mana do ator, alvo)
```

`REJECTED` aqui **não é** um "ERRO DE COMPILAÇÃO" em runtime (não é uma tentativa que falhou) — é a carta simplesmente não aparecer disponível, igual uma carta cujo `ManaCost` do ator já não cobre. Mantém a UX consistente com `combat.md` §10 (a UI nunca deixa o jogador tentar uma ação impossível de ver na tela; só as pré-condições *dinâmicas* como mana do ator entram no fluxo de erro visível).

### 3.2 Estados da bateria (state machine)

```
                 cast (charge -= ManaCost)
        ┌──────────────────────────────────┐
        │                                   ▼
   ┌─────────┐                        ┌─────────┐
   │ CHARGED │ ─── charge chega a 0 ─▶│ DEPLETED │
   │(charge>0)│                       │(inerte)  │
   └────┬────┘                        └────┬────┘
        │                                    │
        │        troca de bateria (§3.3)     │
        └──────────────┬─────────────────────┘
                        ▼
                 ┌─────────────┐
                 │  CHARGED    │ (nova bateria instalada;
                 │  (renovado) │  a antiga vai pro inventário
                 └─────────────┘  como item DEPLETED, não descartável)
```

- `DEPLETED` (bateria zerada) = carta inerte: gate §3.1 sempre `REJECTED` até troca.
- Bateria descarregada retirada da carta **não desaparece**: vira item de inventário (`BatteryItem{charge:0}`), ocupa espaço, não pode ser jogada fora (regra de "crime ambiental" — só vender/reciclar no ferro-velho; fora do escopo de combate, é regra de inventário/mundo, não deste doc).

### 3.3 Troca de bateria — três contextos, três fluxos

| Contexto | Custo | Pré-condição | Efeito |
|---|---|---|---|
| **Cidade** (fora de combate) | grátis | ter ≥1 `BatteryItem` carregada no inventário compatível | swap direto: bateria instalada ↔ bateria do inventário |
| **Estação de recarga** | crédito (`cartas-numeros` §1b, fórmula por SoH) | ter crédito | entrega bateria NOVA carregada (SoH 100%) em troca da velha (que sofre -13pp de SoH e é rearmazenada ou vendida); é uma transação econômica, não lógica de combate — apenas consumida por este doc como origem de `BatteryItem` novo |
| **In-battle (emergência)** | **2 AP** (fixo, não escala por dificuldade, `cartas-numeros` §1c) | ter ≥1 `BatteryItem` carregada compatível no inventário acessível em combate | nova ação de combate (§3.4) |

Cidade e estação são fluxos de **fora de combate** (menu de inventário/loja) — não são deste doc de lógica de combate; ficam aqui só para o fluxo estar completo e a fronteira ficar clara (implementação real = UI de inventário + serviço econômico, `backend-engineer`/`economy-designer`).

### 3.4 Nova ação de combate: `SwapBattery`

Precisa entrar na tabela de ações de `combat.md` §5 (proposta de linha nova, a reconciliar quando implementado — este doc não edita `combat.md`):

| Ação | Custo AP | Custo Mana |
|---|---|---|
| **Trocar bateria (emergência)** | **2** | 0 |

```
action SwapBattery(actor, card_instance_id, spare_battery_item_id):
    pré-condições:
      - actor.ap >= 2                              → falha: "ERRO DE COMPILAÇÃO: AP insuficiente"
      - inventory.has(spare_battery_item_id)
        AND spare.charge > 0                        → falha: "sem bateria de reposição carregada"
      - card_instance está no deck ativo do actor    → falha: alvo inválido
    efeito:
      old_battery = card_instance.battery
      card_instance.battery = spare_battery_item(consumido do inventário)
      inventory.add(old_battery)                    # a usada volta pro inventário, DEPLETED ou parcial
      actor.ap -= 2
      log("> <actor> troca a bateria de <carta> em campo. Carga: <nova>/<capacidade>.")
```

Não gasta Mana (é uma ação de hardware, não um cast). Como qualquer efeito, **loga sempre** — sucesso e cada motivo de falha.

---

## 4. Vírus: payloads como efeitos ocultos

Todo payload é **oculto** até (a) o Turing diagnosticar (§6) OU (b) o próprio payload se manifestar em jogo (alguns disparam mesmo sem diagnóstico — é o "susto"). Cada payload tem: **trigger** (quando avalia), **condição** (quando de fato dispara), **efeito**, **log obrigatório** (mesmo quando não dispara, se for relevante pro jogador entender o "quase").

### 4.1 Tabela de payloads

| Payload | Trigger | Condição de disparo | Efeito | Visível ao jogador antes do diagnóstico? |
|---|---|---|---|---|
| **Logic Bomb** (sabotador) | Pré-cast, todo cast da carta infectada | condição sorteada na infecção (§4.1.1) | substitui/inverte o efeito nominal da carta (vira contra o caster, ou simplesmente falha) | NÃO — só se manifesta ao disparar |
| **Backdoor/spyware** | contínuo, enquanto a carta está na mão/equipada em combate | sempre ativo (sem roll) | IA inimiga ganha viés informacional sobre a party (§4.2) | NÃO — nunca há um "momento" visível; é um vazamento constante |
| **Worm de deck** | pós-cast, toda vez que a carta É jogada | sempre (mas com dois efeitos de força diferente, ver abaixo) | (a) SEMPRE aplica Slow na própria carta-fonte (permanente enquanto infectada); (b) rola 13% de propagação (§5.2) | PARCIALMENTE — o jogador percebe a carta "mais lenta" mesmo sendo original, mas não sabe a causa até diagnóstico |
| **Falso-benigno** (isca Bastiat) | pré-cast (finge buff) | — | **BLOQUEADO POR DESIGN** — ver §4.1.2 | — |
| **Adware Sterling** | pré-cast, todo cast | sempre (não é infecção "hostil", é opt-in consciente, §9) | insere sequência de propaganda (§9); não é dano/sabotagem | SIM — o jogador sabe que a carta tem adware (foi avisado na aquisição, `cartas-hardware...` §10) |
| **Zip-bomb** | pós-cast, toda vez que a carta é jogada | sempre | ver §4.1.3 (efeito exato AMB) | NÃO até disparar |

### 4.1.1 Logic Bomb — pool de condições (proposta, AMB)

O doc-fonte diz "dispara numa condição (turno crítico, chefe, HP baixo)" sem fixar qual. Proponho: **na infecção** (aquisição), sorteia-se 1 condição de um pool pequeno, fixada na instância (não muda depois):

| Condição candidata | Definição operável |
|---|---|
| `HpBelowPercent(30)` | dispara no 1º cast feito com o ator dono da carta com HP < 30% |
| `IsBossOrMiniBoss` | dispara no 1º cast contra/durante um encontro elite/mini-boss/boss |
| `RoundIndexAtLeast(5)` | dispara a partir da 5ª rodada do combate (metáfora: "bug de longa execução") |

```
on_logic_bomb_precast(card_instance, ctx):
    if condition_met(card_instance.trigger_condition, ctx):
        log("> ALERTA: <carta> falha catastroficamente. (payload oculto disparado)")
        invert_or_fail_effect(ctx)   # ver AMB-04: inverter (vira status ruim no caster)
                                     # ou simplesmente FALHAR (efeito nulo, mana/AP/bateria já gastos)
        return INTERCEPTED           # a resolução normal do §1 NÃO roda
    return PASSTHROUGH               # segue pro efeito nominal normalmente
```

**AMB-04:** o doc-fonte não especifica se a logic bomb *inverte* o efeito (vira dano no próprio caster) ou *anula* (carta simplesmente falha, recursos perdidos). Proponho inversão (mais dramático, ecoa "o golpe do Dante generalizado") como default, mas é decisão de sensação de jogo — líder confirma.

### 4.1.2 Falso-benigno — bloqueado (não especificado aqui)

Já sinalizado em `reference_techmagic_engine_impl`: **"Bastiat/RevealHiddenCost — BLOQUEADO por design: precisa da feat EFEITOS-ADIADOS-OCULTOS (scheduler de efeitos futuros + gramática da pegadinha vs telegraph-honesto §13 — decisão do líder, não autônoma)."** Este payload por definição cobra um custo **retardado** (não no momento do cast) — exige um scheduler genérico de "efeito que dispara N turnos/eventos depois", que hoje não existe no motor. Não proponho pseudocódigo aqui: seria inventar a primitiva sem decisão do líder. **Fica como item de handoff, não de spec.**
> **DECISÃO DO LÍDER (2026-07-18): ADIAR.** O falso-benigno fica **PLANEJADO (não cortado)**, dependente da feat `EFEITOS-ADIADOS-OCULTOS` numa onda de motor futura. Por ora o sistema segue com os outros **5 payloads** ativos; a rolagem de contaminação (AMB-07) usa os **4 tipos disparáveis hoje** (LogicBomb / Backdoor / Worm / ZipBomb — Adware é opt-in, fora da rolagem) até a feat existir. **Não priorizar a feat agora.**

### 4.1.3 Zip-bomb — efeito exato (AMB-05)

O doc-fonte lista três possíveis desfechos sem escolher: "trava/corrompe o deck, entope a memória (impede usar outras cartas no turno) ou consome a bateria de uma vez." Proponho como default implementável (reaproveita mecanismo já existente, `Silence`-like) — **mas mantenho como AMB, não decido sozinho**:

```
on_zip_bomb_postcast(card_instance, actor, ctx):
    log("> <carta> incha além do esperado — memória sobrecarregada!")
    apply_actor_flag(actor, MemoryJammed, duration = "resto do turno")
    # efeito de MemoryJammed: bloqueia jogar QUALQUER outra carta pelo resto
    # deste turno (mais amplo que Silence, que só bloqueia carta-com-efeito;
    # aqui bloqueia literalmente qualquer carta, reaproveitando o gate de
    # pré-condição de combat.md §10, mensagem "ERRO DE COMPILAÇÃO: memória
    # cheia (zip-bomb)")
```

**AMB-05:** proposta acima escolhe "entope a memória" (mais fácil de implementar reaproveitando o padrão de `Silence`, sem inventar dano-de-inventário nem falha de save). As outras duas leituras ("corrompe o deck" = a carta se torna permanentemente `IsInfected`/pior; "consome a bateria de uma vez" = `battery.charge = 0` instantâneo) são alternativas mais severas. Líder escolhe qual (ou se as três coexistem por severidade/raridade).

### 4.2 Backdoor/spyware — integração com IA inimiga

Não é um "efeito de carta" no sentido de dano/status — é um viés contínuo na decisão da IA enquanto a carta infectada está na mão ativa em combate. Consome o contrato `IEnemyBrain` (`combat.md` §13) sem alterá-lo: a IA já expõe `DecideAction(CombatState state)`; o `CombatState` passa a incluir um sinal de "vazamento" que o `UtilityBrain` pode ponderar na pontuação de utilidade (ex.: prioriza focar o dono da carta infectada, ou o membro de HP mais baixo com mais confiança).

```
CombatState.leaked_intel:
    for each ally with an equipped/mão card where card.IsInfected AND card.VirusPayloadKind == Backdoor:
        leaked_intel.add(ally.id)   # a IA "sabe" mais sobre este alvo

UtilityBrain.score_action(candidate_action, state):
    base_score = ...(já existente)...
    if candidate_action.target.id in state.leaked_intel:
        base_score *= BACKDOOR_TARGETING_BIAS   # //PLAYTEST, valor a fechar
    return base_score
```

`ScriptedBrain` (Trash) é determinístico e não pontua utilidade — o Backdoor **não tem efeito prático contra Trash** (consistente com o doc-fonte: o vetor dramático é "o Sterling sempre sabe seus movimentos", que é papel de mini-boss/boss com `UtilityBrain`, não de trash aleatório). **AMB-06:** magnitude do viés (`BACKDOOR_TARGETING_BIAS`) e se ele deveria também revelar Gambito-Prever do jogador ao inimigo (efeito espelho do que o jogador faz ao inimigo) ficam para o `economy-designer`/balance, não decididos aqui.

---

## 5. Contaminação

### 5.1 Na aquisição (fora do combate — este doc só descreve a fórmula, não a executa)

Per `cartas-numeros-proposta.md` §3: rola **1 vez**, no momento de aquisição (loot/compra/craft/upload homebrew), usando a tabela de probabilidade por origem (Especial 0% / Comum 1% / Pirata especial 8% / Pirata comum 21% / Homebrew 55% fixo). **Isto roda no serviço de aquisição** (backend/economy — fora da árvore de combate), não no gameplay_engineer. Este doc só documenta a fórmula pra a lógica de combate saber o que consumir depois:

```
on_card_acquired(card_instance):   # chamado pelo serviço de aquisição, NÃO pelo combate
    risk = contamination_table[card_instance.origin_kind]   # cartas-numeros §3
    if roll(risk):
        card_instance.is_infected = true
        card_instance.virus_payload_kind = pick_random_payload()   # AMB-07: distribuição entre os 6 tipos
        if card_instance.virus_payload_kind == LogicBomb:
            card_instance.trigger_condition = pick_random(condition_pool)   # §4.1.1
    # card_instance.is_infected fica OCULTO na UI até diagnóstico (§6)
```

**AMB-07:** qual a distribuição de probabilidade ENTRE os 6 tipos de payload dado que a carta infectou? (uniforme? Zip-bomb mais rara por ser mais destrutiva? Adware não deveria nem entrar aqui pois é opt-in — ver nota abaixo). Não especificado em nenhum doc-fonte; proponho excluir Adware desta rolagem (adware não é "infecção", é feature opt-in de cartas-bônus, §9) e distribuir os 5 restantes (LogicBomb/Backdoor/Worm/FalsoBenigno-bloqueado/ZipBomb) — mas como Falso-benigno está bloqueado por design (§4.1.2), a distribuição efetiva hoje seria só 4 tipos até a feat de efeitos adiados existir. Confirmar com líder.

### 5.2 Propagação secundária (worm, dentro do combate)

Já coberta em §4.1 (Worm de deck): toda vez que uma carta **já sabidamente infectada** é jogada, rola 13% (`cartas-numeros` §3) de contágio em 3 direções:

```
on_worm_postcast(card_instance, ctx):
    apply_status(card_instance.owner, Slow, magnitude=SLOW_WORM, duration=PERMANENT_WHILE_INFECTED)
    log("> <carta> arrasta a execução — desempenho degradado (worm ativo).")
    if roll(0.13):
        direction = pick_random([EnemyDeck, WorldEcosystem, OwnDeck])
        match direction:
            case EnemyDeck:
                target = ctx.random_enemy_card_in_encounter()
                if target: infect(target)
                log("> o vírus salta pro sistema inimigo!")
            case OwnDeck:
                target = ctx.owner.random_other_card_in_active_deck(exclude=card_instance)
                if target: infect(target)
                log("> o vírus se espalha por outra carta do seu deck.")
            case WorldEcosystem:
                emit_world_event(CardInfectionSpread, source=card_instance)   # fora do combate;
                # consumido por sistema de mundo/economia (reputação de vendedor, estoque
                # contaminado) — NÃO implementado neste doc, só o gatilho é definido aqui
                log("> o vírus escapa pra rede — em algum lugar, um vendedor não vai notar.")
```

Nota: `EnemyDeck` e `OwnDeck` são resolvíveis dentro do combate (dados já em memória: a fila de inimigos do encontro, o deck ativo do dono). `WorldEcosystem` sai do escopo de combate — é só um evento emitido; a implementação de "que vendedor fica marcado" pertence ao sistema de mercado negro (fora do escopo deste doc, fora do gameplay de combate).

**Guard de classe protegida:** `infect(target)` DEVE recusar alvo ESPECIAL/SUPER (mesmo guard de `deck-mao-sistema.md` §7 inv.9 — especiais originais têm risco de contaminação **0% sempre**, mesmo por worm; a exceção única é o vírus-arma scriptado da Sterling contra a Gaiola de Faraday, que é um evento narrativo pontual, **não** este sistema geral).

---

## 6. Cura / diagnóstico do Turing

State machine da carta infectada (por instância):

```
        [aquisição infecta, §5.1]
                  │
                  ▼
         ┌──────────────────┐
         │  InfectedHidden   │  ◀── payload pode disparar SEM diagnóstico
         │ (IsInfected=true, │      (Logic Bomb, Backdoor, Worm — todos
         │  Diagnosed=false) │       funcionam mesmo oculto, §4)
         └─────────┬─────────┘
                    │ diagnóstico do Turing (ação fora de combate,
                    │ ou item/gadget de campo — fora do escopo de
                    │ combate deste doc)
                    ▼
         ┌──────────────────┐
         │InfectedDiagnosed  │  UI agora mostra "INFECTADA: <payload>"
         │ (Diagnosed=true)  │  ao jogador; payload continua funcional
         └─────────┬─────────┘  igual (diagnóstico não desativa nada)
                    │
        ┌───────────┴────────────┐
        │ AttemptCure()           │ decline (deixa como está)
        ▼                         │
┌───────────────┐                 │
│  roll 62/38%   │                ▼
│ (cartas-numeros│         permanece InfectedDiagnosed
│    §6)         │         (jogável, risco conhecido, ou
└───┬───────┬────┘          descartada no ferro-velho — fora
    │62%    │38%             do escopo de combate)
    ▼       ▼
┌───────┐ ┌──────────────┐
│ Clean │ │ ChipsetBurned │
│(volta │ │ (carta        │
│ ao uso│ │ PERMANENTEMENTE│
│ normal│ │ destruída/     │
│)      │ │ perdida)       │
└───────┘ └──────────────┘
```

```
action AttemptCure(card_instance):
    pré-condição: card_instance.virus_diagnosed == true    # não pode tentar cura no escuro
    pré-condição: card_instance.tier != Especial AND card_instance.tier != Super
        # guard: especiais/super têm 0% de risco de infecção geral (§5.1), então
        # nunca deveriam chegar aqui por este caminho; guard defensivo, não deveria
        # disparar em jogo normal (a exceção Sterling/Faraday é evento narrativo à parte)
    if roll(0.62):
        card_instance.is_infected = false
        card_instance.virus_payload_kind = null
        card_instance.virus_diagnosed = false
        log("> Turing: remoção bem-sucedida. <carta> limpa.")
        return CURED
    else:
        destroy_card_instance(card_instance)   # perda permanente — fora do deck
                                                 # ativo E do deck morto (não é
                                                 # "descarte", é destruição real)
        log("> Turing: a tentativa falhou — o chipset queimou. <carta> perdida.")
        return BURNED
```

`AttemptCure` é ação **fora de combate** (bancada/oficina do Turing) — listada aqui só para fechar a state machine da carta; não é uma ação da FSM de combate (`combat.md` §3).

---

## 7. `urandom` (pirata × original)

### 7.1 Algoritmo de sorteio

Duas tabelas de pesos já fechadas (`cartas-numeros` §4): original (fraco 21/médio 34/forte 21/jackpot 8) e pirata (fraco 7/médio 2/forte 1/jackpot 0/**backfire 5**, total 15, backfire=1/3 exato).

```
on_urandom_cast(card_instance, caster, ctx):
    is_pirata = (card_instance.origin_kind == PirataClone)
    weights = is_pirata ? URANDOM_PIRATA_WEIGHTS : URANDOM_ORIGINAL_WEIGHTS   # cartas-numeros §4
    faixa = weighted_pick(weights, ctx.rng)      # 1 draw de RNG, contado (determinismo canônico §11)

    if faixa == Backfire:                         # só existe na versão pirata
        bad_effect = pick_from_bad_effect_pool(ctx.rng)   # AMB-08: pool não definido
        apply_effect(bad_effect, target=caster)    # backfire é SEMPRE no próprio caster (fixo,
                                                     # não randomizado — cartas-numeros §4 é explícito)
        log("> urandom: PIRATA FALHA — o efeito se volta contra <caster>!")
        return

    pool = classify_owned_cards_by_faixa(caster.owner_full_collection, faixa)
        # fraco   → ManaCost 1 (comuns "Jab")
        # médio   → ManaCost 2 (comuns "Golpe+status")
        # forte   → ManaCost 3 (comuns "Assinatura")
        # jackpot → qualquer das 20 ESPECIAIS já possuídas pelo jogador
    if pool.empty():
        log("> urandom: nenhum efeito compatível na coleção — sorteio dissipa.")
        return    # AMB-09: fallback pra faixa vizinha, ou dissipa mesmo (sem custo devolvido)?

    chosen_card = uniform_pick(pool, ctx.rng)      # 2º draw de RNG
    target_side = pick_target_side(ctx.rng)        # AMB-10: 50/50 self/inimigo, independente
                                                     # do alvo "natural" da carta sorteada?
    log("> urandom: sorteou <chosen_card.id> (<faixa>) → aplicado em <target_side>.")
    resolve_card_effect(chosen_card, target=target_side, ctx)
        # reusa o resolvedor normal (record-base OU techMagic::execute conforme
        # o tier da carta sorteada) — urandom não reimplementa efeito nenhum,
        # só REDIRECIONA pra um efeito já existente no catálogo
```

### 7.2 Ambiguidades específicas do urandom

- **AMB-08 (pool de "efeito ruim" do backfire):** o que exatamente é aplicado no caster quando dá backfire? Um status negativo genérico (Poison/Stun aleatório de baixa magnitude)? Perda de mana/AP? O doc-fonte não especifica o CONTEÚDO do backfire, só a probabilidade (1/3). Proponho: reusar um pool pequeno de status negativos leves já existentes (Stun 1 turno, ou Poison magnitude baixa) — sorteado uniformemente — em vez de inventar um efeito novo. Confirmar com líder.
- **AMB-09 (pool vazio):** se o jogador não possui nenhuma carta na faixa sorteada (ex.: sorteou "jackpot" mas não tem nenhuma especial ainda), o que acontece? Proponho: dissipa (log + nada acontece, recursos já gastos ficam gastos) em vez de "cair pra faixa de baixo", pra não inflar silenciosamente a chance efetiva das faixas baixas. Mas é decisão de sensação de jogo.
- **AMB-10 (lado do alvo):** "podendo ser bom ou ruim, no caster ou no inimigo — totalmente imprevisível" (doc-fonte) não deixa claro se o lado é sorteado INDEPENDENTE do efeito escolhido (ex.: pode sortear uma cura e aplicá-la no inimigo, ou um dano e aplicá-lo no caster) ou se segue o alvo natural da carta sorteada (cura sempre em aliado, dano sempre em inimigo — só o EFEITO é aleatório, não o alvo). A primeira leitura é mais caótica/fiel ao "totalmente imprevisível"; a segunda é mais segura/legível. **One-way door de sensação — líder decide.**
> **RESOLVIDO (líder, 2026-07-18): INDEPENDENTE do efeito (caótico).** O lado (self/inimigo) é sorteado junto, separado do efeito — pode cair uma cura no inimigo ou um dano no próprio caster. Fiel ao "totalmente imprevisível" e ao espírito caótico da carta do Gus (+ a pirata azarada com backfire 1/3).

---

## 8. RunaDex

Puramente **observador de eventos já existentes** — não introduz nenhuma ação de combate nova, só consome o `CombatBus`/`PlayerBus` (`combat.md` §16).

### 8.1 State machine (por espécie de carta, não por instância)

```
Unknown ──(evento: inimigo joga a carta OU Scan revela)──▶ Vista (silhueta)
Vista   ──(evento: jogador adquire, qualquer canal)───────▶ Tida (face revelada)
Tida    ──(evento: carta entra na mão/loadout ativo)──────▶ NoDeck (moldura neon azul)
NoDeck  ──(evento: carta sai da mão/loadout)───────────────▶ Tida (nunca regride a Vista)
```

### 8.2 Hooks (reuso de eventos existentes, nenhum sistema novo)

| Transição | Evento consumido | Fonte |
|---|---|---|
| `Unknown → Vista` | `CombatBus.ActionResolved(actor=inimigo, action=UseCard(cardSpeciesId))` | já existe (`combat.md` §16) — RunaDex só assina o evento pra cartas ainda não vistas |
| `Vista → Tida` | evento de aquisição (loot/compra/craft/upload) | serviço de aquisição, fora do combate — RunaDex assina via `PlayerBus` |
| `Tida → NoDeck` | mudança de loadout (mão ativa) | sistema de deck/mão (`deck-mao-sistema.md`) — RunaDex assina evento de loadout, não implementado neste doc |

```
on_runa_dex_event(event, species_id):
    entry = runa_dex[species_id]
    match event:
        case EnemyCardObserved | ScanRevealed:
            if entry.state == Unknown: entry.state = Vista
        case CardAcquired:
            if entry.state in {Unknown, Vista}: entry.state = Tida
        case EnteredLoadout:
            entry.state = NoDeck
        case LeftLoadout:
            if entry.state == NoDeck: entry.state = Tida
    check_completion()

check_completion():
    discovered = count(species where state in {Tida, NoDeck})
    total = total_known_species_count()
    if discovered == total AND not urandom_original_already_granted:
        grant_card(player, urandom_original_instance)   # prêmio, 1x, evento de progresso
        log("> RunaDex completa. Prêmio: urandom (original) adicionado à coleção.")
```

Sem AMB relevante aqui — a mecânica é toda observacional, sem número de balance em jogo.

---

## 9. Adware Sterling: sequência de cast

Não é dano/sabotagem — é interrupção de UX, opt-in consciente (o jogador foi avisado ao aceitar a carta grátis, `cartas-hardware...` §10). Timing fechado: **5 segundos** até o X aparecer (`cartas-numeros` §5).

```
on_cast_precondition_passed(card_instance, ctx):
    if card_instance.has_adware:
        ui.show_ad_overlay(card_instance.ad_content)   # bloqueia input do jogador
        wait(5.0)   # segundos reais — gate de apresentação, não consome AP/mana/turno
        ui.show_close_button(position=TOP_RIGHT_SMALL)
        wait_for_player_click(close_button)
        ui.dismiss_ad_overlay()
        log("> propaganda Sterling dispensada. Prosseguindo com a compilação.")
    # segue pro débito de recurso normal (§1)
```

**Refinamento opcional registrado em `cartas-numeros` §5 (não obrigatório):** após a 3ª exposição na MESMA sessão de jogo, o X aparece imediatamente (skip). Se adotado, precisa de um contador por-sessão (`AdwareExposureCount`), que é estado de UI/sessão, não de combate — marco como **AMB-11** (adotar ou não o skip; se sim, onde mora o contador — sessão de jogo vs por-carta).

**Nota de posição no pipeline (§1):** a sequência de adware roda **antes** do débito de recurso — o jogador só "paga" (mana/bateria/AP) depois de fechar o anúncio, então cancelar a ação nesse ponto (se o motor permitir cancelar uma ação em progresso) não deveria cobrar nada. **AMB-12:** o jogador pode cancelar durante a propaganda (ex.: apertar um botão de "voltar")? O doc-fonte não cobre isso; proponho que NÃO (uma vez que o jogador confirmou "jogar carta", a sequência de adware é obrigatória até o fim — reforça a "sátira do adware real", que não deixa escapar) — mas é decisão de UX a confirmar.

---

## 10. Resumo de pontos de integração com sistemas já existentes

| Sistema já existente | Como este doc se conecta |
|---|---|
| `combat.md` §10 (pipeline de cast + erros de compilação) | novo gate de bateria (§3.1) segue o MESMO padrão visual de erro (`ERRO DE COMPILAÇÃO: ...`) e o mesmo padrão de "botão desabilitado" (Null-sem-Scan) pra carta inerte |
| `combat.md` §9 (status framework) | Worm reusa `Slow` já existente; Zip-bomb proposto reusa o padrão de `Silence` (bloqueio de jogar carta); backfire do urandom reusa status negativos já catalogados |
| `combat.md` §13 (`IEnemyBrain`) | Backdoor pluga como sinal adicional em `CombatState` consumido por `UtilityBrain.score_action`, sem mudar a interface `IEnemyBrain` |
| `combat.md` §16 (`CombatBus`/`PlayerBus`) | RunaDex é 100% observador de eventos já emitidos + eventos de aquisição/loadout (fora do combate); Worm emite `CardInfectionSpread` pro sistema de mundo (fora do combate) |
| `reference_techmagic_engine_impl` (executor `techMagic`) | vírus/bateria NÃO viram `EffectKind` novo — envolvem o `resolve_use_card` inteiro (`CardHardwareLayer`, §1), agnóstico de COMUM vs ESPECIAL/SUPER |
| `deck-mao-sistema.md` §7 (invariantes anti-exploit, classe protegida) | guard reaplicado em `infect()` (§5.2) e `AttemptCure()` (§6): ESPECIAL/SUPER nunca infecta (exceto o evento narrativo único Sterling/Faraday, fora deste sistema geral) |
| regra "todo efeito loga" (`feedback_todo_efeito_loga_terminal`) | cada pseudocódigo acima tem uma linha `log(...)` em CADA desfecho (sucesso, bloqueio, dissipação, falha) — nenhum branch fica silencioso |

---

## 11. Lista consolidada de ambiguidades (AMB)

| ID | Resumo | Onde |
|---|---|---|
| AMB-04 | Logic Bomb: inverte o efeito (dano no caster) ou anula (carta falha)? | §4.1.1 |
| AMB-05 | Zip-bomb: qual dos 3 desfechos do doc-fonte é o real (memória/deck/bateria)? Proposta = memória (Silence-like) | §4.1.3 |
| AMB-06 | Magnitude do viés de targeting do Backdoor na IA; se também vaza o Gambito-Prever do jogador pro inimigo | §4.2 |
| AMB-07 | Distribuição de probabilidade ENTRE os tipos de payload dado que infectou (uniforme? Adware fica de fora da rolagem?) | §5.1 |
| AMB-08 | Conteúdo exato do "efeito ruim" do backfire do urandom pirata | §7.2 |
| AMB-09 | Pool vazio no urandom (sorteou faixa sem carta correspondente na coleção): dissipa ou cai pra faixa vizinha? | §7.2 |
| AMB-10 | urandom: o lado do alvo (self/inimigo) é sorteado independente do efeito, ou segue o alvo natural da carta sorteada? **One-way door de sensação.** | §7.2 |
| AMB-11 | Skip do adware após 3ª exposição na sessão: adotar? Onde mora o contador? | §9 |
| AMB-12 | Jogador pode cancelar durante a sequência de adware? | §9 |
| — | Falso-benigno (isca Bastiat) inteiro: bloqueado até a feat `EFEITOS-ADIADOS-OCULTOS` existir (decisão do líder, não autônoma) | §4.1.2 |

---

## 12. Handoff / próximos passos

- **`backend-engineer`:** criar `cartas-spec-dados.md` definindo os records reais (`CardInstance` extensions, `BatteryItem`, enums `VirusPayloadKind`/`OriginKind`/`RunaDexEntryState`) a partir do contrato pedido em §2 deste doc.
- **Líder:** resolver AMB-04 a AMB-12 (todas são decisões de sensação/balance, nenhuma é puramente técnica) + a feat bloqueante `EFEITOS-ADIADOS-OCULTOS` (Falso-benigno).
- **`economy-designer`:** números finos que este doc deixou como `//PLAYTEST` implícito (magnitude do Slow do worm, `BACKDOOR_TARGETING_BIAS`, magnitude do backfire do urandom se AMB-08 resolver por status parametrizado).
- **`gameplay_engineer` (implementação futura, quando a onda abrir):** este doc É o ponto de partida; TDD red→green por payload, um de cada vez, seguindo o padrão de verificação adversarial já usado no motor `techMagic` (`reference_techmagic_engine_impl`: implementer ≠ reviewer, mutation testing).
- **`TODO.md`:** este doc não abre item novo na tabela de pendências (fora do escopo desta tarefa); recomendo ao orquestrador abrir `CARDS-HARDWARE-ENGINE` como item novo referenciando este spec quando a implementação for priorizada.
