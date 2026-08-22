# SPEC: Provocar, Soft Enrage e o critério do cap de rodadas

> **Status: SPEC DE DESIGN (números propostos, pronta para virar fatia), não canon.**
> Nasce das 3 direções aprovadas pelo líder em 2026-08-11 depois da Fase C do estudo de
> pacing (`analise-pacing-fase-c-20260811.md`, seções 6, 7 e 8), que reprovou 0 de 4
> candidatos de statline por dois motivos de DESENHO e um de CRITÉRIO. Autor:
> `lead-game-designer`, 2026-08-11. **Nenhum número aqui entra em `combat.md` nem no motor
> sem decisão explícita do líder item a item.** Não commitado por decisão de processo.
>
> Docs-fonte lidos para escrever esta spec: `analise-pacing-fase-a.md`,
> `analise-pacing-fase-a-bis-20260810.md`, `analise-pacing-fase-b-20260811.md`,
> `analise-pacing-fase-c-20260811.md`, `proposta-protocolo-simulacao-pacing.md`,
> `proposta-mira-inimiga.md` (decisões do líder de 2026-08-01), `combat.md` §5/§9/§11/§15.1/§17,
> `combat_constants.hpp`, `combat_state_machine.cpp` (`mira_target_weight`,
> `pick_weighted_enemy_target`, `resolve_defend`), `mira_sim_harness.hpp`,
> `pacing_sim_harness.hpp`.

---

## 0. Resumo dos números propostos (a página que decide)

| # | Peça | Número proposto | Origem do número |
|---|---|---|---|
| 1 | Ação **Provocar** | 1 AP, 0 mana, duração 1 rodada | Espelha o custo de Defender (`combat.md` §5); duração 1 força re-pagar todo turno |
| 1 | Peso de mira do provocador | **peso final x 12,0** (`kMiraProvokeMultiplier`) | Calibrado para 73-85% dos golpes irem ao provocador na party de referência (§2.5) |
| 1 | Preço do Provocar | **Def x 0,5** (arredondada para baixo) contra ataques enquanto ativo | O honeypot se anuncia vulnerável (Pillar 1); sem esse preço a mecânica trivializa o trash (§2.7) |
| 1 | Provocar + Defender no mesmo turno | **PERMITIDO** (2 dos 3 AP) | Precedente FFXIII (Provoke e Steelguard são ações separadas e encadeáveis); a penalidade de Def impede a parede invulnerável |
| 2 | **Soft enrage**: rodada de início | **rodada 8** no trash/elite (regra: teto da janela-alvo do tier + 3) | Janela-alvo 3-5 (`combat.md` §15.1); p90 medido = 5 em TODAS as células aprovadas (Fase B §3) |
| 2 | Soft enrage: incremento | **+15% do Atk BASE por rodada, linear, piso de +1/rodada** | Precedente WoW/Gruul (+15% por intervalo); piso de +1 garante escalada em inimigo de Atk baixo |
| 2 | Efeito do enrage no pior caso | luta-tartaruga termina **até a rodada 15** (prova em §3.4) | Metade do cap de 30: margem estrutural de 2x, não sorte |
| 3 | Critério do cap: teto de taxa | **1,0 x 10⁻⁴** com limite superior de 95% (Poisson exato) | Tolera até 20 lutas capadas em 300k e ainda reprova o 39/11 medido (1,46 x 10⁻⁴) |
| 3 | N correspondente | **300.000 por ponto** (hoje 240.000) | 3/N para k=0; custo medido: +25% de CPU, ~5 min para a grade inteira |
| 3 | Gate complementar (novo) | **p99 de duração <= 20 rodadas no P6** | Percentil é estável; não depende da sorte da cauda (lição "enumere o espaço") |

Três decisões de arquitetura que atravessam tudo:

1. **Provocar REUSA `mira_target_weight`** (um termo multiplicativo novo), não cria mecanismo
   paralelo de mira. Lei do átomo, ADR-019/020, e preserva a trava de paridade
   harness/motor (`mira_harness_production_parity_test.cpp`).
2. **Soft enrage é REGRA DE JOGO**, no motor real, com parâmetros vindos do descritor do
   ENCONTRO (data-driven por tier), não constante global hardcoded. Se ficasse só no
   harness, o problema continuaria em produção.
3. **O cap de 30 rodadas NÃO existe em produção hoje.** `kRoundCap = 30` mora em
   `mira_sim_harness.hpp` (test-only); o motor real só tem um teto de 10.000 TURNOS que
   LANÇA exceção. A proposta é manter assim: o enrage passa a ser a garantia de término
   (§3.4 traz a prova), e o cap segue sendo instrumento de MEDIÇÃO, não regra. Detalhe em §3.5.

---

## 1. O que a Fase C mediu, em uma tabela (a base de todo número abaixo)

Só os números que esta spec usa. Fonte: Fase B §3/§6 e Fase C §3/§4/§5.

| Grandeza medida | Valor | Onde |
|---|---|---|
| Duração mediana da luta saudável (P1/P2, células aprovadas) | **4 rodadas** (5 nas células Atk 11) | Fase B §3 |
| % de lutas saudáveis na janela 3-5 | **96,6% a 98,6%** | Fase B §3 |
| p90 de duração das células aprovadas | **5 rodadas** (nenhuma passa disso) | derivado da janela 3-5 acima |
| Vitória P2 (parede), mira titular | 92,00% a 92,56% | Fase C §4 |
| Vitória P2 (parede), mira vizinha (F4 suave) | **85,59% a 86,71%** (piso é 90%) | Fase C §4 |
| Quedas do Gus em P2, titular -> vizinha | 7,44% -> **13,29%** (teto é 12%) | Fase C §4 |
| Duração da tartaruga P6 (33/12 titular) | mediana 7, p10-p90 = 4-11 | Fase C §5 |
| Duração da tartaruga P6 (39/11 titular, esponja) | mediana 10, p10-p90 = 5-15 | Fase C §5 |
| Lutas no cap de 30 (33/12 e 34/12 titulares) | **1 em 240.000** (4,2 x 10⁻⁶) | Fase C §5.1 |
| Lutas no cap de 30 (39/11 titular) | **35 em 240.000** (1,46 x 10⁻⁴) | Fase C §5.1 |
| Vazão do harness | 41.000 a 75.000 lutas/s | Fase C, cabeçalho |
| Piso de relevância (MCID) | 5 pontos percentuais / 0,5 rodada | protocolo §5 |
| Faixa de vitória do trash (E1) | 90% a 97% | protocolo §4.1 |
| Teto de quedas do Gus (E4) | 12% | protocolo §4.1 |

**Party de referência usada em toda conta desta spec** (`mira_sim_harness.hpp`, espelha
`combat.md` §17): Gus HP 34 / Atk 8 / Def 5; Cauã HP 55 / Atk 14 / Def 8; Jaci HP 55 /
Atk 9 / Def 10. Pool total 144 HP. Trash Sentinela-Bit: HP 33 / Atk 12 / Def 8, 1 AP/turno.
Dano básico = `max(1, Atk - Def)`; Defender aplica Shield de pool = Def, duração 1.

**ATENÇÃO, viés declarado (RESOLVIDO na origem em 2026-08-11, mas as contas AQUI ainda são
as antigas):** quando esta spec foi escrita, o estudo NÃO tinha o Bento. A "parede" medida
em P2 era a **Jaci** (Def 10, HP 55), o membro de maior Def da party de referência, usada
como PROXY do tanque porque o Bento não tinha statline canônico em `combat.md` §17. Isso
mudou: o statline dele foi aprovado pelo líder em 2026-08-11 (item
`BENTO-STATLINE-COMBATE`), **HP 55 / Atk 10 / Def 13 / SPD 5, Cinético**, e o harness da
Fase C-bis já roda os cenários de parede/provocação com ele (`pacing_sim::third_member_spec`).

**O que continua pendente, e por isso a advertência não sai daqui:** todo número desta spec
foi derivado com **Def 10**, não 13. Onde §2.5 e §2.7 falam em pool de Shield 5 (metade de
10) e no dano líquido que decorre disso, o valor com o tanque real é 6 (metade de 13), e o
golpe do Sentinela-Bit no provocador passa de `max(1, 12-5) = 7` para `max(1, 12-6) = 6`.
A própria spec pré-declarou essa consequência ("quando o statline do Bento existir, as
faixas de §2.5 e §2.7 precisam ser reconferidas com a Def dele, não herdadas"), e a
reconferência **não foi feita nesta fatia**: ela mexe em faixa pré-registrada, logo é
decisão do `lead-game-designer` com aval do líder, não efeito colateral de uma
implementação.

**Reconferência FEITA em 2026-08-12, ANTES do run oficial da C-bis: ver §8. Resultado:
nenhuma faixa pré-registrada muda** (G1 segue 70-85%; o critério do G4 segue como escrito,
com o empate 0%×0% agora analiticamente esperado e coberto pelo plano B pré-declarado no
protocolo §9.3). As tabelas de §2.5/§2.7 acima permanecem como registro histórico da
derivação com Def 10; as contas vigentes com Def 13 são as de §8.

**Run oficial da C-bis EXECUTADO em 2026-08-12** (`analise-pacing-fase-c-bis-20260812.md`):
G1 e G3 reprovaram os 4 candidatos; o líder decidiu corrigir SÓ o multiplicador. A
proposta do valor novo, com a conta pré-run, está em **§9** (item `PROV-MULT-AJUSTE`).

**Run oficial da C-ter EXECUTADO em 2026-08-12** (`analise-pacing-fase-c-ter-20260812.md`):
G1 consertado nos 8 pontos (previsão do §9 confirmada com erro ≤ 0,3pp); G3 reprovou de
novo, insensível ao multiplicador, como o §9.5 previu. A resposta do líder (testar a
correção pré-declarada de §2.7/§2.8: exclusividade Provocar+Defender) está especificada em
**§10**, com o mecanismo, o destino do P7 e a previsão pré-run. **Aviso de leitura: a
previsão de §10 é que a exclusividade NÃO resolve o G3, e metade dessa previsão é dado já
medido, não modelo**; a recomendação transparente está em §10.6.

---

## 2. Item 1: a ação PROVOCAR

### 2.1. Achado que precede a spec: colisão com uma decisão JÁ APROVADA

`proposta-mira-inimiga.md` §1 e §2.A registram que **o líder já aprovou, em 2026-08-01, uma
CARTA de provocação** (conceito honeypot, fator **F5** da tabela de mira), com três decisões
fechadas na ocasião:

- **F5 aprovado**: o portador soma peso alto por N turnos.
- **Não é trava absoluta**: verbatim, *"o inimigo pode escolher outro alvo, só é bem menos
  provável; trava absoluta mata a leitura do telegrafar"*.
- **Contrapartida decidida**: o portador ganha bônus de defesa de **no máximo 10%**, e o
  teto baixo é deliberado, para manter tenso o eixo *"enfraquecer um tanker numa batalha
  estúpida ou poupar ele e a bateria das cartas para algo provavelmente difícil no futuro"*.

O briefing desta spec não citou o F5. **Se Provocar virar uma ação básica gratuita com o
mesmo efeito, a carta honeypot morre**: toda a tensão que o líder desenhou (posse da carta,
bateria, pirata x original, prudência x yolo) evapora se o mesmo efeito custa 1 AP e nada mais.
Isso é feature-creep por colisão, e é dever nomear antes de especificar.

**Reconciliação proposta (e é o desenho que esta spec adota):** as duas existem, em degraus
distintos do MESMO mecanismo, diferenciadas pelo PREÇO e não pela força bruta.

| | Ação **Provocar** (nova) | Carta **honeypot** (F5, já aprovada) |
|---|---|---|
| Custo | 1 AP | 1 AP + mana da carta + **dreno de bateria** + posse da carta |
| Peso de mira | x12,0 | x12,0 (o mesmo) |
| Duração | **1 rodada** (re-paga todo turno) | **2 rodadas** (número já usado no harness, `kHoneypotDurationRounds`) |
| Defesa | **Def x 0,5 contra ataques** enquanto ativo | **+10% de Def** (contrapartida decidida pelo líder) |
| Disponibilidade | sempre, para qualquer membro | só quem tem a carta e bateria |
| Silence bloqueia? | **não** (é ação básica, não cast) | **sim** (é carta) |

Leitura de design: a ação dá ao tanque uma FUNÇÃO permanente (que ele hoje não tem), pagando
com a própria pele; a carta é a versão premium, que faz a mesma coisa por mais tempo e
**sem** o preço de defesa, gastando um recurso escasso. Nenhuma canibaliza a outra: o eixo
de decisão do líder (poupar a bateria ou não) fica intacto, e ganha um degrau abaixo dele.

### 2.2. Verb, pillar e risk/reward

- **Verb:** *chamar o golpe para mim*.
- **Pillar servido:** Pillar 1 (magia é software) na diegese, e Pillar 4 (o jogo se resolve
  por lógica, não por força) na função: substitui um sorteio opaco por uma alavanca
  explícita e legível.
- **Diegese:** um honeypot é um serviço que se anuncia deliberadamente vulnerável para
  atrair o ataque e mantê-lo longe do que importa. É literalmente uma provocação escrita em
  software, e a penalidade de Def não é castigo arbitrário: é o próprio conceito
  (a isca precisa parecer fraca). Já é o vocabulário aprovado em `proposta-mira-inimiga.md` §2.A.
- **Risk:** você perde metade da Def, atrai ~3/4 dos golpes da rodada, e se cair a party
  perde o escudo humano no pior momento. Se quem provoca é o Gus, a queda dele encerra o
  combate (`check_end`, acoplamento derrota = queda do Gus medido desde a Fase A).
- **Reward:** os outros dois agem livres, e o membro cuja queda encerra a partida
  (o Gus) fica fora da mira enquanto durar.
- **Por que não é comando dominado** (crítica de Robert Boyd, "Adding Depth to Defend",
  citada na Fase C §6): Provocar custa 1 de 3 AP, então o provocador ainda ataca duas vezes
  no mesmo turno. Não é um turno queimado, é um turno redirecionado.

### 2.3. Regras formais

```
Mecânica: Provocar

Entrada (input do jogador): ação Provocar em ActionSelect, sem alvo (é auto-aplicada).
Custo: 1 AP, 0 mana. Nova entrada em CombatActionType (append-only) e na tabela de
       custos canônica de combat.md §5.

Estado consultado: nenhum (não depende de HP, posição, nem carta).

Estado modificado: aplica no PRÓPRIO ator o status Provocado
  StatusEffect{ id = StatusId::Provocado (append-only),
                magnitude = penalidade de Def já calculada (Def - floor(Def*0,5)),
                duration = 1,
                StackRule::Replace,
                family = Universal }
  Via add_status LEGADO (auto-aplicado, mesmo caminho de Scrying/Eco): NÃO passa pelo
  portão de imunidade ofensiva (BlindagemEM não bloqueia o próprio Provocar).

Saída (feedback ao jogador):
  - CombatLogEntry diegético obrigatório (regra "todo efeito loga"), com o número:
    "<ator> abre uma porta falsa: o escalonador inimigo passa a preferi-lo (Def <x> -> <y>)."
  - Marcador persistente no retrato do provocador na HUD enquanto o status durar
    (a UI é do glintfx; o domínio só expõe o StatusId).
  - O telegrafar de intent já aprovado (proposta-mira-inimiga.md §1) deve mostrar o
    inimigo mirando o provocador, senão a alavanca fica invisível e volta a queixa de
    origem do líder ("simplesmente vejo acontecer e parece bom").

Efeito na mira: enquanto Provocado estiver ativo, o peso final do candidato é
  multiplicado por kMiraProvokeMultiplier (§2.5).

Efeito na defesa: enquanto Provocado estiver ativo, a Def EFETIVA do ator vale
  floor(Def * kProvokeDefFactor), com piso 1. Vale para o dano recebido E para o pool de
  Shield gerado por Defender no mesmo turno (uma Def só, sem duas verdades).

Duração: 1 rodada. Como o tick de duração roda no TurnStart do PRÓPRIO ator
  (apply_status_tick), o status cobre exatamente a janela "até o meu próximo turno",
  que é a leitura que o jogador espera.
```

### 2.4. Parâmetros sintonizáveis

Todos nascem em `combat_constants.hpp`, nomeados, nunca literais no meio do código
(mesmo padrão dos `kMira*` que já existem).

| Parâmetro | Default | Faixa útil | Efeito |
|---|---|---|---|
| `kMiraProvokeMultiplier` | **12,0** | 6,0 a 20,0 | Fatia dos golpes que vai ao provocador (§2.5) |
| `kProvokeDefFactor` | **0,5** | 0,4 a 0,75 | Preço em pele: quanto o honeypot se expõe |
| `kProvokeDurationRounds` | **1** | 1 a 2 | Frequência com que se re-paga a ação |
| `kProvokeApCost` | **1** | 1 a 2 | Quanto do turno o Provocar consome |
| `kHoneypotCardMultiplier` | **12,0** | igual ao da ação | Carta F5; mesmo peso, outra duração |
| `kHoneypotCardDurationRounds` | **2** | 2 a 3 | Já é o número do harness |
| `kHoneypotCardDefBonusPct` | **10** | teto fixado pelo líder | Contrapartida aprovada em 2026-08-01 |

### 2.5. Onde entra na fórmula de mira: REUSO, não mecanismo paralelo

**Decisão: reusar `mira_target_weight` com um termo novo, multiplicativo, aplicado depois da
soma.** A fórmula passa de

```
peso = max(0, kMiraBaseWeight + F1(dano) + F2(ferido) + F3(suporte))
```

para

```
peso = max(0, kMiraBaseWeight + F1 + F2 + F3) * (Provocado ? kMiraProvokeMultiplier : 1.0)
```

Três motivos, na ordem em que pesam:

1. **Lei do átomo (ADR-019/020):** o vocabulário "peso de mira" já existe e é um só. Um
   segundo mecanismo de redirecionamento criaria duas fontes de verdade sobre quem o
   inimigo ataca, e a próxima carta de mira (Firewall/F6, também já aprovada) teria de
   escolher em qual das duas entrar.
2. **A trava de paridade sobrevive.** `mira_harness_production_parity_test.cpp` garante que
   estudo e motor calculam o MESMO peso. Somando um termo, a trava continua valendo; criando
   um mecanismo paralelo, o estudo passaria a simular um jogo que o motor não roda.
3. **Multiplicativo e não aditivo, porque aditivo não segura o efeito quando os aliados se
   ferem.** F2 dá até +100 por aliado ferido; um bônus fixo de +500 no provocador vira uma
   fatia cada vez menor conforme a party apanha, ou seja, o taunt enfraquece exatamente
   quando é mais necessário. Multiplicando, a fatia se mantém, e sobe quando o próprio
   provocador se fere (o F2 dele também é multiplicado), que é o comportamento correto:
   o tanque machucado continua sendo o alvo.

**Calibração do 12,0** (party de referência, cenário parede, 3 inimigos):

| Situação | Peso do provocador | Peso dos outros dois | Fatia do provocador |
|---|---|---|---|
| Party inteira sem ferimento | 100 x 12 = 1200 | Cauã 100+36 (F1), Gus 100+6 = **242** | **83,2%** |
| Aliados feridos pela metade | 1200 | 242 + ~100 = **342** | **77,8%** |
| Aliados muito feridos (F2 quase cheio) | 1200 | ~442 | **73,1%** |
| Provocador já ferido pela metade | 150 x 12 = 1800 | 442 | 80,3% |

F1 do Cauã estimado com Atk 14 contra Def 8 (raw 6 por golpe, 3 golpes por turno, janela de
2 rodadas = ~36); do Gus, Atk 8 contra Def 8 (raw 1, ~6).

**Faixa-alvo pré-registrada: 70% a 85% dos golpes no provocador.** É forte o bastante para o
jogador sentir a alavanca funcionando, e fraco o bastante para NÃO ser trava (1 em 4 a 1 em
7 golpes ainda escapa), respeitando a decisão do líder de 2026-08-01. O número 12,0 é o ponto
de partida; a Fase C-bis mede a fatia real e a calibração final sai do dado, não daqui.

**Consequência que fecha uma pendência antiga: o F4 (defendendo) fica fora da fórmula, para
sempre.** Ele estava com "sinal indefinido, depende do estudo estatístico"
(`proposta-mira-inimiga.md`, tabela de aprovações). Com uma alavanca explícita no jogo, a
mira não precisa inferir intenção lendo quem defende: quem quer ser alvo, avisa. O braço
`C_F4_suave` continua existindo **como teste de robustez** (um inimigo mais esperto que
evita defensores), não como candidato a produção. Isso também resolve o item pendente
`IA-ALVO-PRIMEIRO-DA-LISTA` na ordem certa: decide-se o desenho da defesa primeiro, liga-se
em produção a mira que sobreviver a ele (Fase C §9).

### 2.6. Furos e exceções DECLARADAS

Precedente: o Guard do Darkest Dungeon declara os furos dele em vez de fingir que não
existem. Mesmo padrão aqui.

| # | Situação | Regra | Motivo |
|---|---|---|---|
| E1 | **Efeito em área** (TargetShape de área) | **Ignora Provocado.** Área acerta quem estiver na área. | Sem isso, área vira dano single-target e a identidade da família Sônica (`combat.md` §6) some. |
| E2 | **Provocador cai** | Status morre com ele; a rodada seguinte volta ao sorteio normal. | Cadáver não é candidato; nenhuma regra especial é necessária. |
| E3 | **Provocador é atordoado (Stun)** | **Cancela Provocado imediatamente.** | O honeypot é um serviço rodando; processo congelado não anuncia nada. Dá ao inimigo uma contra-jogada real (uma carta de Stun tira o taunt), o que é bom design e ecoa o Guard do DD. |
| E4 | **Silence** | **Não bloqueia** a ação Provocar (não é cast). Bloqueia a CARTA honeypot. | Assimetria de propósito: a ação é a rede de segurança sempre disponível. |
| E5 | **Dois aliados provocam na mesma rodada** | Sem regra especial: os dois pesos x12 competem entre si e dividem a mira. | Comportamento emergente correto, e sem exceção nova para testar. |
| E6 | **Todos provocam** | Idem: o multiplicador é comum e some na normalização; equivale a ninguém provocar. | Auto-anulação natural, sem código extra e sem invulnerabilidade. |
| E7 | **Inimigo provocando a party** | **Fora de escopo desta spec.** | O alvo da party é escolhido pelo JOGADOR; forçar alvo de jogador é outro assunto (perda de agência) e merece decisão própria. |
| E8 | **Provocar com 1 único inimigo vivo** | Funciona igual (o sorteio ponderado tem 1 candidato do lado dele, mas 3 do nosso). | Nenhuma exceção. |
| E9 | **Fuga (Flee) enquanto Provocado** | Sem interação; o status simplesmente deixa de importar. | Nenhuma exceção. |

### 2.7. Risco de degeneração e as mitigações

**Degeneração nº 1 (a séria): a parede invulnerável.** Nota já medida em
`mira_sim_harness.hpp`: o Shield absorve 100% do dano básico enquanto `Atk <= 2 x Def`. Com
o trash em Atk 12 e o tanque-proxy em Def 10, o dano bruto é 2 por golpe contra um pool de
10: **zero dano**. Se Provocar mandasse ~78% dos golpes para um defensor nessa condição, a
party pararia de tomar dano e a vitória subiria para perto de 100%, estourando o teto E1 de
97% ("seguro demais") e esvaziando o combate. A mecânica passaria a resolver o problema
errado.

**Mitigação: a penalidade de Def (`kProvokeDefFactor = 0,5`), que é o preço do honeypot.**
Com Def efetiva 5, o dano bruto por golpe vira 12 - 5 = 7. Conferindo os dois modos:

| Modo do provocador (Def 10 base, HP 55) | Def efetiva | Golpes/rodada (~78% de 3) | Dano líquido/rodada | Rodadas até cair |
|---|---|---|---|---|
| Só Provocar | 5 | 2,34 | 2,34 x 7 = **16,4** | ~3,4 |
| Provocar + Defender (2 AP) | 5 (Shield pool 5) | 2,34 | 16,4 - 5 = **11,4** | ~4,8 |
| Só Defender (sem provocar) | 10 (Shield pool 10) | ~1,0 | max(0, 2 - 10) = **0** | não cai |

O quadro é exatamente a tensão que se quer: defender sozinho é seguro e inútil (não protege
ninguém); provocar protege a party e custa a pele do provocador na escala da própria luta
(3 a 5 rodadas). Não existe modo gratuito.

**Por que Provocar + Defender fica PERMITIDO:** precedente FFXIII (Provoke e Steelguard são
ações separadas, encadeáveis no mesmo ATB), 2 dos 3 AP consumidos, e a linha do meio da
tabela mostra que a combinação não produz invulnerabilidade (11,4 de dano líquido por
rodada, não zero). É o "turno completo de Sentinel", pago com o turno inteiro de ataque
menos um. **Esta é a decisão desta spec com maior risco de estar errada**, e por isso ela vem
com gate pré-registrado próprio em §2.8 (G3): se a combinação estourar o teto de vitória, a
correção pré-declarada é torná-las mutuamente exclusivas, e não mexer no multiplicador.

**Degeneração nº 2: Provocar vira jogada automática.** Se protege sempre e custa pouco, o
jogador aperta sem pensar e a decisão morre (o líder já registrou esse critério para a carta:
*"se o balanceamento fizer a carta virar escolha automática, o número está errado, não a
regra"*). Vigiado pelo gate G4 (§2.8): num bot que provoca SEMPRE, a taxa de vitória tem de
ficar dentro da mesma faixa 90-97%, e as quedas do provocador têm de subir de forma visível.
Provocar sempre deve custar o tanque.

**Degeneração nº 3: o Gus provoca.** Permitido de propósito e não precisa de trava: a queda
do Gus encerra o combate, então provocar com ele é apostar a partida. É uma decisão
interessante (Sid Meier), não um exploit, e o custo é máximo e óbvio.

### 2.8. Métricas de balance (pré-registro, a fixar ANTES de rodar)

| Gate | O que mede | Critério proposto |
|---|---|---|
| **G1** | Fatia dos golpes inimigos que vão ao provocador (cenário P7, §5) | **70% a 85%**, nos DOIS braços de mira |
| **G2** | Vitória no P2/P7 no braço vizinho (`C_F4_suave`), a falha que originou tudo | **>= 90%** e quedas do Gus **<= 12%** |
| **G3** | Vitória no P7 com o provocador também defendendo | **<= 97%** (teto E1: não pode trivializar) |
| **G4** | Quedas do PROVOCADOR no P7 | tem de ser **estritamente maior** que as quedas dele no P2 (senão provocar é grátis) |
| **G5** | Sensibilidade ao braço de mira: diferença de vitória entre titular e vizinha no P7 | **<= 5pp** (o piso de relevância). É o objetivo declarado: com alavanca explícita, o resultado deixa de depender da esperteza da IA |
| **G6** | Duração (E2) no P7 | continua na janela 3-5 |

G5 é o gate que responde a pergunta que a Fase C fez. Hoje a diferença entre braços é de
5,85 a 6,77 pontos percentuais (Fase C §4). Se, com Provocar, ela cair abaixo do piso de
relevância, o desenho ficou robusto e o assunto encerra.

---

## 3. Item 2: SOFT ENRAGE anti-tartaruga

### 3.1. Fórmula

```
k(r)     = max(0, r - R0 + 1)                    // rodadas de enrage já acumuladas
bonus(r) = 0                                se k = 0
bonus(r) = max( k, floor(atk_base * pct * k / 100) )   se k >= 1
atk_efetivo(r) = atk_base + bonus(r)
```

- `r` = índice de rodada (a mesma "volta completa da fila" de `combat.md` §4.1, que é a
  unidade de toda a janela 3-5; **nunca** turno de ator).
- `R0` = `kSoftEnrageStartRound`, por tier de encontro (§3.2).
- `pct` = `kSoftEnrageAtkPctPerRound` = **15**.
- **Linear sobre a base, não composto.** Composto acelera demais e fica ilegível na tela
  ("por que ele bate 40?"); linear é previsível, e o Gambito/Scan conseguem prever o valor
  da rodada seguinte, o que importa para o Pillar 4.
- **Piso de +1 por rodada** (`max(k, ...)`): sem ele, um inimigo de Atk baixo (4, 5) nunca
  escalaria, porque `floor(0,6 x k)` fica em 0 por várias rodadas, e a garantia de término
  de §3.4 cairia justamente no caso mais lento.
- Aplica-se **só ao lado inimigo**. A party não recebe bônus simétrico: quem tem a iniciativa
  de encerrar a luta é o jogador, e o enrage é a pressão para que ele a use.

### 3.2. Rodada de início: a regra, não o número solto

**Regra proposta: `R0 = teto da janela-alvo de duração do tier + 3 rodadas de folga`.**

| Tier | Janela-alvo (`combat.md` §15.1) | p90 medido | `R0` proposto | Status |
|---|---|---|---|---|
| Trash / Elite | 3-5 rodadas | **5** (Fase B §3, todas as células aprovadas) | **8** | Calibrado nesta spec |
| Boss | 12-18 rodadas | não medido | 21 | **PROPOSTA**, depende do `BOSSES-ZELDA-BRAINSTORM` |
| Boss final (Sterling, 2 fases) | ~30 rodadas somando fases | não medido | 33 | **PROPOSTA**, idem |

Por que a folga de 3 rodadas e por que 8 no trash:

1. **8 é o dobro da mediana medida (4).** O enrage só existe para a luta que já saiu do
   desenho, não para a luta normal.
2. **8 está 3 rodadas acima do p90 medido (5).** Em nenhuma célula aprovada da Fase B mais de
   10% das lutas passa de 5 rodadas. A luta saudável praticamente não enxerga o enrage.
3. **Metade das lutas-tartaruga também não enxerga** (mediana do P6 = 7 no 33/12). Isso é
   desejado: defender ainda compra tempo, o que o gate p10 >= 4 do P6 exige.
4. **Começar em 6 seria errado.** As células aprovadas de Atk 11 (39/11, 36/11) têm mediana
   **5**; um gatilho em 6 pegaria a cauda normal delas e puniria jogo legítimo.

**ATENÇÃO ao boss:** um `R0` global de 8 destruiria o design de boss, cuja janela-alvo é
12-18 rodadas. O `R0` tem de nascer no descritor do ENCONTRO (dado), não como constante
única. Bosses estilo Zelda são puzzle, e um enrage mal posto transforma puzzle em corrida
contra o relógio. Os valores de boss acima são proposta para o brainstorm dedicado, não
número desta spec.

### 3.3. A escalada em números (trash, `atk_base` = 12, `pct` = 15, `R0` = 8)

| Rodada | k | bonus | Atk efetivo | Dano bruto contra Def 5 (Gus) | contra Def 8 (Cauã) | contra Def 10 (proxy do tanque) |
|---|---|---|---|---|---|---|
| <= 7 | 0 | 0 | 12 | 7 | 4 | 2 |
| 8 | 1 | +1 | 13 | 8 | 5 | 3 |
| 9 | 2 | +3 | 15 | 10 | 7 | 5 |
| 10 | 3 | +5 | 17 | 12 | 9 | 7 |
| 11 | 4 | +7 | 19 | 14 | 11 | 9 |
| 12 | 5 | +9 | 21 | 16 | 13 | 11 |
| 13 | 6 | +10 | 22 | 17 | 14 | 12 |
| 14 | 7 | +12 | 24 | 19 | 16 | 14 |
| 15 | 8 | +14 | 26 | 21 | 18 | 16 |

### 3.4. Prova de término (é isto que substitui o cap)

Pior caso analítico construído para durar o máximo possível: os 3 inimigos concentram TODOS
os golpes no membro de maior Def (proxy do tanque, Def 10, HP 55), que redefende toda rodada
(pool 10, renovado). Dano líquido por rodada = `3 x max(1, atk - 10) - 10`, com piso 0.

| Rodada | Atk efetivo | Bruto x3 | Menos Shield 10 | HP acumulado no alvo (de 55) |
|---|---|---|---|---|
| <= 8 | 12-13 | 6-9 | 0 | 0 |
| 9 | 15 | 15 | 5 | 5 |
| 10 | 17 | 21 | 11 | 16 |
| 11 | 19 | 27 | 17 | 33 |
| 12 | 21 | 33 | 23 | **56 -> cai** |

Depois dele, o segundo mais duro (Def 8, HP 55, pool 8): na rodada 13, `3 x 14 - 8 = 34`; na
14, `3 x 16 - 8 = 40`. Cai na rodada 14. Sobrando o Gus (Def 5, HP 34, pool 5), na rodada 15:
`3 x 21 - 5 = 58`. Fim.

**Teto analítico de pior caso: rodada 15.** Metade do cap de 30 do harness. E é pior caso
adversarial: qualquer distribuição realista de golpes (que fere alvos de Def menor) termina
antes. A garantia é estrutural porque `bonus(r)` cresce sem limite e a mitigação da party é
**limitada** por construção: Shield tem pool = Def, a cura da Jaci é limitada pela bateria
(`CURANDEIRA-LIMITE-USOS`), e não existe regeneração ilimitada no motor. Portanto existe
sempre um `r` a partir do qual o dano por rodada excede qualquer mitigação.

**Condição de validade a monitorar:** se um dia entrar mecânica de mitigação que ESCALE sem
teto (Shield percentual, regeneração proporcional ao HP máximo, redução multiplicativa
empilhável), esta prova cai e o enrage precisa ser reavaliado. Fica registrado como
invariante a vigiar em toda carta nova de defesa.

### 3.5. Escopo: qualquer luta, e o que acontece com o cap

**Aplica a QUALQUER luta que passe de `R0`, não só à tartaruga.** Três motivos:

1. Detectar "o jogador está tartarugando" exigiria o motor julgar a estratégia do jogador.
   Isso é opaco, punitivo e burlável (bastaria atacar de leve uma vez por rodada). Um relógio
   de rodadas é objetivo, visível e impossível de discutir.
2. Uma luta normal que trava por azar também é falha de pacing. A Fase C mediu 3,08% de lutas
   fora da janela 3-5 no P1; a fatia dessas que passa de 8 rodadas merece o mesmo empurrão.
3. Um átomo só, uma regra só, um conjunto de constantes só (ADR-020).

**O cap de 30 rodadas:**

- Em produção **não existe** cap nenhum hoje. `kRoundCap = 30` vive em
  `mira_sim_harness.hpp`, test-only; o motor real só tem o teto de 10.000 turnos que
  **lança exceção** (isto é um guarda de bug, não uma regra de jogo).
- **Proposta: continuar sem cap em produção**, porque §3.4 mostra que o enrage já garante o
  término, e porque um cap de 30 colidiria de frente com o boss final, cuja duração-alvo é
  ~30 rodadas (`combat.md` §15.1).
- **O cap permanece no harness**, exatamente como está, como INSTRUMENTO DE MEDIÇÃO: ele é o
  sensor que detecta "esta luta não terminou quando deveria". Trocar o sensor no meio do
  estudo quebraria a comparabilidade com as Fases A, A-bis, B e C.
- Se o líder ainda assim quiser uma rede de segurança em produção, o precedente registrado na
  Fase C §7 é o do xadrez (regra dos 50 e dos 75 lances: uma camada reivindicável e uma
  automática). O que **não** se deve importar é o mecanismo do harness: lá o cap resolve por
  wipe determinístico da party, o que como regra de jogo seria uma morte sem causa visível.
  Isso vai como pergunta ao líder (§7, Q5).

### 3.6. Onde isto entra no motor real

- **Módulo novo e estreito** (ADR-020: assunto novo nasce em módulo próprio, nunca acretado
  num struct existente): `gus/domain/combat/soft_enrage.hpp` com função **pura**
  `int soft_enrage_atk_bonus(int atk_base, int round_index, const SoftEnrageParams&)` e
  `struct SoftEnrageParams { bool enabled; int start_round; int pct_per_round; }`.
  Pura significa testável sem FSM, e é onde o TDD e o mutation testing mordem.
- **Parâmetros vêm do descritor do ENCONTRO** (dado, por tier), não de constante global. Os
  defaults de trash (`enabled = true`, `start_round = 8`, `pct_per_round = 15`) vivem em
  `combat_constants.hpp` como valores nomeados de referência.
- **Ponto de aplicação:** a fronteira de rodada em `CombatStateMachine::advance_to_next_actor`,
  no mesmo bloco que já roda `process_round_end_hooks()` / `regroup_round_by_side()` /
  `advance_period_clock()` quando `queue_.round_index()` avança. É o único lugar do motor que
  hoje sabe que uma rodada virou.
- **Atk efetivo tem de valer em TODOS os canais derivados de Atk**: ataque básico
  (`resolve_basic_attack`), dano de carta (`resolve_use_card`) e, obrigatoriamente, os dois
  previews (`preview_basic_attack_damage` e `estimate_card_damage`). O header já exige em
  comentário que os previews espelhem a fórmula real. **Se o preview não enxergar o enrage, o
  jogo mente para o jogador na tela de mira**, e isso é pior que não ter enrage.
- **Serialização:** o bônus é derivado de `round_index`, que já é estado da luta. Nada novo
  precisa entrar no save (combate não persiste no meio). Registrar isso explicitamente evita
  que alguém adicione campo por precaução.

### 3.7. Telegrafar e log (não negociável)

Regra canônica da casa: todo efeito loga mensagem diegética, e a mira aprovada já tem
telegrafar. O enrage precisa dos três avisos:

1. **Uma rodada antes** (`r = R0 - 1`), aviso único: *"o watchdog do processo inimigo começa a
   contar: a próxima rodada eleva a prioridade dele."*
2. **A cada rodada de escalada**, uma linha com o número: *"escalonamento de prioridade: o
   Sentinela-Bit sobe para Atk 15 (+3)."*
3. **Marcador na HUD** enquanto ativo, com o valor corrente (a apresentação é do glintfx; o
   domínio só expõe o número).

Diegese (Pillar 1): não é raiva, é escalonamento de prioridade. Escalonador de sistema
operacional real eleva a prioridade de processo que espera demais, justamente para evitar
starvation e travamento. A luta empacada é um deadlock, e o watchdog dispara. O vocabulário
já está aprovado em `proposta-mira-inimiga.md` §1 ("a mira do inimigo não é raiva, é
escalonamento de processo"), então isto é continuidade, não invenção.

### 3.8. Degeneração, ganho colateral e métricas

- **Anti-exploit colateral, e é ganho real:** esticar a luta hoje é potencialmente lucrativo
  para farmar Mastery/Knowledge por AÇÃO (o mesmo vetor que obrigou a trava 1x/turno do
  Tavus-Overclock). O enrage fecha essa porta pelo lado do design, sem trava nova. Cruzar com
  o catálogo de exploits (`reference_catalogo_exploits_speedrun`, categorias softlock e
  farming) na revisão do Gus Dragon.
- **Alinhamento com a doutrina do comedimento:** quem estica a luta paga mais HP e portanto
  mais hospital (E10). O jogador comedido, que resolve a luta na janela, não sente o enrage
  existir. Briefar o `economy-designer` com o número, porque o custo por encontro na cauda
  sobe.
- **Risco declarado:** o enrage pune indiretamente o jogador LENTO por falta de recurso
  (mana/AP/carta ruim), não só o que tartaruga de propósito. Com `R0 = 8` contra mediana 4
  isso é raro por construção, mas é o efeito colateral honesto de qualquer pressão por
  relógio, e o gate H1 abaixo existe para medi-lo.

| Gate | O que mede | Critério proposto |
|---|---|---|
| **H1** | % de lutas P1/P2 que alcançam a rodada `R0` | **<= 1,0%** (se passar disso, o enrage está mordendo jogo normal e `R0` sobe) |
| **H2** | Delta de vitória e de quedas em P1/P2, com enrage x sem enrage | dentro do piso de relevância (**<= 5pp**): a luta saudável não pode sentir |
| **H3** | p99 de duração no P6 (tartaruga) | **<= 20 rodadas** |
| **H4** | Lutas que batem o cap de 30 no P6 | ver §4 (teto de taxa) |
| **H5** | Duração máxima observada em toda a grade | **< 20 rodadas**, coerente com o teto analítico de 15 de §3.4 |

---

## 4. Item 3: o critério do cap vira TETO DE TAXA

### 4.1. Números propostos

| Item | Valor proposto |
|---|---|
| Teto de taxa de lutas no cap (P6) | **1,0 x 10⁻⁴** (1 em 10.000 encontros) |
| Estatística de decisão | **limite superior de 95% de Poisson exato**, não a taxa pontual |
| N por ponto | **300.000** (hoje 240.000) |
| Gate complementar | **p99 de duração no P6 <= 20 rodadas** |
| Regra de leitura | reprova se `UB95(k)/N > teto`; aprova caso contrário. Imprimir SEMPRE k, a taxa e o UB, mesmo quando k = 0 (zero declarado, não presumido) |

### 4.2. A conta que decide entre 10⁻⁴ e 10⁻⁵ (e por que 10⁻⁵ não resolve a dor)

A dor concreta do líder foi: **uma única luta em 240.000 reprovou dois candidatos inteiros.**
Um critério novo que não conserte isso não serve para nada. Limite superior de 95% de Poisson
exato: `UB(k=0) = 3,00`; `UB(k=1) = 4,74`; `UB(k=2) = 6,30`.

| Teto | N | Quantas lutas capadas ainda APROVAM | Veredicto sobre a dor |
|---|---|---|---|
| 10⁻⁵ | 240.000 | **nenhuma**, e nem k=0 aprova (3/240.000 = 1,25 x 10⁻⁵ > 10⁻⁵) | Pior que o atual: indemonstrável |
| 10⁻⁵ | 300.000 | **apenas k = 0** (k=1 dá 1,58 x 10⁻⁵ > 10⁻⁵) | É o "zero estrito" com outro nome. **Não conserta a dor.** |
| 10⁻⁵ | 500.000 | k <= 1 (k=1 dá 9,5 x 10⁻⁶) | Conserta, ao custo de N maior |
| **10⁻⁴** | **300.000** | **k <= 20** (k=20 dá 9,7 x 10⁻⁵; k=21 dá 1,008 x 10⁻⁴) | **Conserta com folga larga** |

E o teste que qualquer critério novo precisa passar para não ser conserto interesseiro
(a mesma disciplina que a Fase C §8 se impôs): **ele não pode salvar quem merecia reprovar.**
O 39/11 esponja mediu 35 lutas capadas em 240.000 (1,46 x 10⁻⁴), o que em 300.000 equivale a
~44 eventos, muito acima do limite de 20. **Continua reprovado no teto de 10⁻⁴.** O critério
novo só absolve a flutuação de cauda, exatamente o que se queria.

**Custo de CPU: irrelevante, e isto foi medido, não estimado.** A 41.000 a 75.000 lutas/s, a
grade de 48 pontos a 300.000 lutas dá 14,4 milhões de lutas, ou **3 a 6 minutos**. Mesmo
500.000 por ponto (24 milhões) fica em 5 a 10 minutos. **O custo de CPU não deve ser o
argumento em nenhuma direção**: a escolha entre 10⁻⁴ e 10⁻⁵ é decisão de produto sobre
rigor, não de orçamento de máquina. Fica registrado para ninguém usar "sai caro" como
desempate.

**Significado para o jogador**, que é a régua honesta para decidir: um playthrough tem da
ordem de 300 a 600 encontros de trash. A 10⁻⁴, a chance de um playthrough inteiro ver UM
empate técnico é ~3% a 6%; a 10⁻⁵, ~0,3% a 0,6%. E as duas contas são pessimistas por
construção, porque o P6 é um cenário adversarial (a party inteira só defende, o tempo todo),
que nenhum jogador real executa por muitas rodadas seguidas. Somando o enrage de §3, a taxa
verdadeira esperada é **zero estrutural**, e o gate passa a ser piso de segurança, não alvo.

### 4.3. O gate complementar de percentil (e por que ele importa mais que o de taxa)

Gate de evento raro depende da sorte da cauda; gate de percentil, não. **Proposta:
`p99 de duração no P6 <= 20 rodadas`**, medido em paralelo ao teto de taxa, com os dois
tendo de aprovar.

Motivo, na lição da casa "enumere o espaço, não busque dentro dele": o teto de taxa só olha
o único evento que bate o cap, e por isso é frágil. O p99 mede a forma inteira da cauda e
teria pego o 39/11 (mediana 10, p90 15, logo p99 seguramente acima de 20) **sem depender de
nenhuma luta chegar a 30**. É o gate que enxerga o problema antes de ele virar evento.

Referência para calibrar o 20: com enrage ligado, o teto analítico de pior caso é 15 (§3.4);
20 dá 5 rodadas de folga para variância de RNG e para as configurações que a prova não cobre.

### 4.4. Generalização: princípio metodológico, não número solto para o P6

Proposta de acréscimo ao `proposta-protocolo-simulacao-pacing.md` (seção nova, ou anexo à
§4/§5), valendo para TODO gate futuro de evento raro:

> **Princípio do evento raro.** Nenhum critério do estudo pode exigir "zero eventos" de forma
> estrita. Todo gate de evento raro se declara como uma tripla, fixada JUNTA no pré-registro,
> antes de qualquer dado existir:
> **(teto de taxa `p`; confiança de 95%; `N >= 3/p`)**.
> A decisão usa o **limite superior de 95% de Poisson exato** para o `k` observado, nunca a
> taxa pontual. O relatório imprime sempre `k`, a taxa observada e o limite superior,
> inclusive quando `k = 0` (zero declarado distingue "não houve evento" de "ninguém contou").
> Quando existir uma métrica de PERCENTIL que meça o mesmo fenômeno, ela entra como gate
> irmão obrigatório, porque percentil não depende da sorte da cauda.
>
> **Justificativa (fontes já levantadas na Fase C §8):** regra de três de Hanley &
> Lippman-Hand (JAMA, 1983): zero em N demonstra no máximo taxa <= 3/N; law of truly large
> numbers (Diaconis & Mosteller, JASA, 1989): critério de "zero em N" fica mais difícil
> conforme N cresce, ou seja, mede o tamanho da amostra e não o jogo; c=0 acceptance sampling
> (ASQ/Squeglia; ISO 28594): mesmo o controle de qualidade que "exige zero defeitos na
> amostra" é formalizado como decisão sob risco quantificado por curva OC, nunca como
> demonstração de ausência.

Isto **não reinterpreta a Fase C**. O veredicto dela (0 de 4) foi dado pelo critério que
estava escrito e assim permanece. O princípio vale da próxima rodada em diante.

---

## 5. O que a Fase C-bis precisa medir (insumo para o pré-registro)

Cenários novos ou alterados, para o protocolo:

| Cenário | Composição | Para que existe |
|---|---|---|
| **P7 (novo): parede com provocação** | 3 inimigos; tanque-proxy **provoca + defende**; os outros dois atacam | O cenário que mede se a correção funcionou (gates G1-G6). É o P2 com a alavanca ligada |
| **P7b (novo): provocar sem defender** | idem, mas o provocador só provoca | Isola o efeito da penalidade de Def e alimenta a decisão Q2 (§7) |
| **P6 (alterado): tartaruga com enrage** | inalterado, com o enrage ligado | Gates H3, H4, H5 |
| **P1/P2 (inalterados) com enrage ligado** | inalterados | Gates H1, H2: prova de que a luta saudável não sente |
| Braços de mira | os mesmos dois (`F_sem_F4` titular, `C_F4_suave` vizinho) | G5 exige os dois; o vizinho segue sendo teste de robustez, não candidato |

Papéis novos no harness: `ProvokeThenAttack` e `ProvokeAndDefend` (espelhando os papéis
existentes `DefendThenAttack` / `DefendOnlyTurtle`), e o motor do harness continua chamando a
função de produção, nunca reimplementando a regra.

Candidatos: os **mesmos 4** da Fase C (33/12 controle, 34/12, 33/13, 39/11), porque nenhum
foi reprovado por ser um número ruim; foram derrubados por propriedades do sistema em volta
deles, que é justamente o que esta spec muda.

**Ordem obrigatória (Fase C §9):** decidir o desenho da defesa (esta spec) ANTES de ligar
qualquer mira em produção (`IA-ALVO-PRIMEIRO-DA-LISTA`). Ligar a mira ponderada e depois
trocá-la seria retrabalho garantido.

---

## 6. Fatias de implementação sugeridas (ordem, não escopo fechado)

Sem código nesta spec. A ordem abaixo existe porque cada fatia destrava a próxima.

1. **PROV-1 (domínio puro):** `StatusId::Provocado` (append-only), `CombatActionType::Provoke`
   (append-only), `resolve_provoke`, Def efetiva com `kProvokeDefFactor`, log diegético. TDD
   e mutation testing.
2. **PROV-2 (mira):** termo multiplicativo em `mira_target_weight` + espelho no harness +
   atualização da trava de paridade. **A trava de paridade tem de falhar antes de passar**
   (senão não está medindo nada).
3. **ENRAGE-1 (domínio puro):** módulo `soft_enrage`, função pura, `SoftEnrageParams`,
   testes de fronteira em `R0 - 1`, `R0`, `R0 + 1` **e um passo além do limite** (a lição de
   que testar só a fronteira exata não pega o alargamento do limite).
4. **ENRAGE-2 (FSM):** aplicação na fronteira de rodada, Atk efetivo em ataque, carta e
   **nos dois previews**, logs e telegrafar.
5. **SIM-1 (harness):** papéis `ProvokeThenAttack` / `ProvokeAndDefend`, cenários P7/P7b,
   enrage ligado, e a impressão dos gates novos (k, taxa, UB95, p99) **com o veredicto
   aplicado e visível**, não só os números crus.
6. **PROTO-1 (documento):** pré-registro novo em `proposta-protocolo-simulacao-pacing.md`
   (princípio do evento raro, teto, N, p99, gates G e H), assinado pelo líder ANTES de rodar.
7. **Fase C-bis:** só depois de 1-6, e só depois do pré-registro fechado.

A carta honeypot (F5) e a carta Firewall (F6) não entram nesta sequência: elas dependem do
motor de cartas e do nome em Sylvarin, que é do `narrative-writer`, e a fatia PROV-2 já deixa
o mecanismo de peso pronto para as duas.

---

## 7. Perguntas em aberto para o líder

Nenhuma delas é número calibrável: todas são identidade, escopo ou pré-registro, que por
regra da casa não se decide sem ele.

**Q1. Ação Provocar e carta honeypot convivem?** (§2.1) O líder aprovou em 2026-08-01 a
CARTA de provocação, com contrapartida de defesa de no máximo 10% e toda a tensão de bateria
e posse. A ação nova, se for gratuita, canibaliza a carta. Esta spec propõe conviverem em
degraus (ação: 1 rodada, custa Def; carta: 2 rodadas, dá +10% de Def), mas a decisão é dele.
Alternativas: só a carta (contraria a direção nova), ou a carta vira outro eixo (provocação
em área, ou provocação que não gasta o turno).

**Q2. Provocar e Defender podem ser feitos no mesmo turno?** (§2.7) Esta spec propõe SIM
(2 dos 3 AP, precedente FFXIII), com a penalidade de Def impedindo a parede invulnerável, e
com o gate G3 pré-registrado para reprovar caso trivialize. A alternativa é torná-las
mutuamente exclusivas desde já ("ou eu levo o golpe, ou eu aguento o golpe"), que é mais
seguro e menos rico. **É a decisão desta spec com maior risco de estar errada.**

**Q3. Quem pode provocar?** Esta spec propõe **universal** (qualquer membro), com a
identidade do tanque emergindo do statline dele (mais HP e mais Def para pagar o preço), e
sem sistema de classes no motor. A alternativa é restringir ao Bento, o que dá identidade
mais forte mas exige um campo de personagem novo e tira do Gus a jogada de se sacrificar por
um aliado (que é decisão interessante, e cara: a queda dele encerra o combate).

**Q4. Nome do comando em pt-br.** Proposta: **"Provocar"**, porque é imediatamente legível
para o público 11+ (a interface não pode exigir vocabulário de rede), com a diegese de
honeypot vivendo no LOG e não no rótulo do botão. Alternativas levantadas: "Isca" (mais
diegética, menos óbvia), "Desafiar" (mais épica, menos técnica), "Chamar Atenção" (mais
literal, longa demais para botão). O nome da CARTA continua pendente do `narrative-writer`
em Sylvarin, como já estava.

**Q5. O jogo deve ter um cap de rodadas de verdade?** (§3.5) Hoje não tem: o cap de 30 é do
harness. Esta spec propõe continuar sem, porque o enrage garante o término (prova em §3.4) e
porque 30 colidiria com o boss final (~30 rodadas de duração-alvo). A alternativa é um cap
em camadas, no precedente do xadrez (uma camada suave, outra automática), e nesse caso é
preciso decidir o que acontece ao bater: o harness resolve por wipe da party, que como regra
de jogo seria morte sem causa visível.

**Q6. Pré-registro do critério do cap:** teto **10⁻⁴ com N = 300.000** (proposta desta spec:
tolera até 20 lutas capadas, conserta a dor da luta única, e ainda reprova o 39/11), ou
**10⁻⁵ com N = 500.000** (mais rigoroso, também tolera a luta única, custa 5 a 10 minutos de
CPU em vez de 3 a 6). **10⁻⁵ com N = 300.000 não deve ser escolhido**: é o "zero estrito" com
outro nome, e não resolve nada. O gate irmão de percentil (p99 <= 20 rodadas) entra nos três
casos.

**Q7. Enrage em boss.** (§3.2) Esta spec calibra só trash/elite (`R0 = 8`) e propõe a REGRA
`R0 = teto da janela-alvo + 3`, que daria 21 no boss e 33 no boss final. Como bosses são
puzzle estilo Zelda e aguardam o `BOSSES-ZELDA-BRAINSTORM`, a decisão de o boss ter ou não
enrage (e com qual folga) deveria sair naquele brainstorm, não aqui. Confirmar se pode ficar
pendente.

**Q8. Statline do Bento. RESPONDIDA em 2026-08-11** (item `BENTO-STATLINE-COMBATE`): o
statline existe e foi aprovado pelo líder, **HP 55 / Atk 10 / Def 13 / SPD 5, Cinético**
(`combat.md` §17), e o harness da C-bis já o usa nos cenários de parede/provocação. A
pergunta original era: *toda conta de tanque nesta spec usa a Jaci (Def 10) como proxy,
porque o Bento não tem statline em `combat.md` §17 e não está na party de referência do
estudo; se a intenção é que o Bento seja o provocador canônico, o statline dele precisa
existir ANTES da Fase C-bis, senão a simulação valida um tanque que o jogo não tem.*
**Fica em aberto o desdobramento**, não a pergunta: as contas de §2.5/§2.7 desta spec ainda
são as de Def 10 e aguardam reconferência (ver a advertência do início do documento).

---

## 8. Reconferência com o Bento real (Def 13) — 2026-08-12

Registro exigido pela advertência de §1 e pela pré-condição 3 de
`proposta-protocolo-simulacao-pacing.md` §9.5. Feita **antes** do run oficial da C-bis,
sobre as MESMAS fórmulas de §2.5/§2.7, trocando apenas a proxy (Jaci, Def 10, Atk 9) pelo
Bento real (**HP 55 / Atk 10 / Def 13 / SPD 5**, `combat.md` §17, item
`BENTO-STATLINE-COMBATE`). Executor: Caetano (CTO), em modo autônomo delegado pelo líder
em 2026-08-12 — registrado como **decisão autônoma, a confirmar retroativamente**, e ela
só coube em modo autônomo porque **confirma** as faixas; se qualquer faixa tivesse de
MUDAR, a decisão voltaria ao líder antes do run. **Confirmada pelo líder em 2026-08-12**,
via `AskUserQuestion`, junto com o veredito da C-bis oficial.

**Resultado: nenhuma faixa pré-registrada muda.**

### 8.1 §2.5 reconferida (calibração do ×12 → gate G1)

Na fórmula da fatia, a troca da proxy pelo Bento só altera o F1 do PRÓPRIO provocador
(Atk 10 contra Def 8 = raw 2 por golpe, onde a proxy saía raw 1); Cauã e Gus ficam
idênticos. F1 do provocador: no P7 (provoca + defende, 1 ataque restante) ≈ 2×1×2 = 4;
no P7b (só provoca, 2 ataques) ≈ 2×2×2 = 8.

| Situação | Peso do provocador | Peso dos outros dois | Fatia (antes, com proxy) |
|---|---|---|---|
| Party inteira sem ferimento (P7) | (100+4)×12 = 1248 | 242 | **83,8%** (83,2%) |
| Idem, P7b | (100+8)×12 = 1296 | 242 | **84,3%** (—) |
| Aliados feridos pela metade | 1248 | 342 | **78,5%** (77,8%) |
| Aliados muito feridos | 1248 | 442 | **73,8%** (73,1%) |
| Provocador ferido pela metade | (150+4)×12 = 1848 | 442 | **80,7%** (80,3%) |

Tudo dentro de 70-85%. **Faixa do G1 mantida.** Ressalva honesta: o topo analítico
encosta mais na borda do que antes (84,3% no P7b, folga de 0,7pp). Se G1 reprovar por
cima no run oficial, a correção já está pré-declarada no protocolo §9.3: mexer em
`kMiraProvokeMultiplier`, e só nele.

### 8.2 §2.7 reconferida (preço do honeypot → gate G4)

Def efetiva provocando = floor(13 × 0,5) = **6**. Golpe do Sentinela-Bit no provocador:
max(1, 12 − 6) = **6** (era 7 com a proxy).

| Modo do provocador (Def 13 base, HP 55) | Def efetiva | Golpes/rodada (~78% de 3) | Dano líquido/rodada | Rodadas até cair (antes) |
|---|---|---|---|---|
| Só Provocar (P7b) | 6 | 2,34 | 2,34 × 6 = **14,0** | ~3,9 (~3,4) |
| Provocar + Defender (P7) | 6 (Shield pool 6) | 2,34 | 14,0 − 6 = **8,0** | ~6,9 (~4,8) |
| Só Defender (P2) | 13 (Shield pool 13) | ~1,0 | max(0, 1 − 13) = **0** | não cai (idem) |

Leitura, nas duas direções:

1. **A degeneração nº 1 (parede invulnerável) segue mitigada.** Provocar continua
   custando a pele (nenhuma linha dá dano líquido zero exceto a que não protege ninguém),
   e a trivialização segue vigiada por G3 (vitória <= 97%). Nenhum número precisa mudar.
2. **Mas o G4 perde poder de discriminação no trash, por construção.** A luta mediana
   dura 4 rodadas (Fase B §3) e o provocador do P7 precisa de ~7 rodadas de foco
   sustentado para cair — sem contar que os inimigos morrem durante a luta e o foco real
   cai junto. Quedas ~0% no P7 E no P2 são o resultado analiticamente ESPERADO com o
   Bento real. A prévia medida (n = 20.000/ponto, semente 90001+, candidato 33/12, P2 e
   P7, enrage ligado e desligado: provocador em 0,000% nos dois cenários) confirma a
   conta. É EXATAMENTE o cenário do plano B pré-declarado no protocolo §9.3 (decisão do
   líder de 2026-08-11, commit `b17006b0`): **se o run oficial confirmar o empate 0%×0%,
   G4 vira informativo** — segue medido e impresso, deixa de decidir candidato.
   **O critério do G4 fica como está no pré-registro; nada foi ajustado "para passar".**

Recomendação de leitura para o relatório da C-bis (informativa, NÃO é gate novo — gate
não nasce depois do dado): imprimir também as quedas do provocador no **P7b**, onde a
conta acima prevê queda dentro da própria janela 3-5 (~14 de dano líquido/rodada contra
55 HP). É a evidência de que provocar não é grátis quando não vem acompanhado de
Defender, e dá ao plano B do G4 um contraponto medido em vez de só a ausência.

### 8.3 Efeito colateral conferido: §3.3/§3.4 (enrage) não precisam de correção

A coluna "contra Def 10 (proxy do tanque)" de §3.3 e a prova de término de §3.4 usam o
tanque do cenário-tartaruga, e o **P6 roda com a Jaci por desenho declarado**
(`BENTO-STATLINE-COMBATE`: parede/provocação = Bento; sem-papel P1/P6 = Jaci). A âncora
analítica de 15 rodadas segue intacta para H3/H4/H5. Registro hipotético, para o caso de
uma grade futura mover o Bento para um cenário-tartaruga: com Def 13 (Shield pool 13), o
alvo só começa a tomar dano líquido na rodada 11 e cai na 15; Cauã cai na 17 e o Gus na
18 — pior caso analítico ~**rodada 18**, ainda abaixo do H5 (< 20), mas com folga de 2
rodadas em vez de 5. Vigiar essa premissa se o desenho da grade mudar.

---

## 9. Correção do G1 reprovado: proposta `kMiraProvokeMultiplier` 12,0 → **10,5** — 2026-08-12

Registro exigido pela reprovação do run oficial da C-bis
(`analise-pacing-fase-c-bis-20260812.md` §5/§6: **G1** estourou o teto por cima no braço
titular, 85,1-85,7% contra teto 85,0%; **G3** trivializou, 100,0%/99,9% contra teto 97%) e
pela decisão do líder de 2026-08-12 (item `PROV-MULT-AJUSTE`): a correção desta rodada é
**só o multiplicador** — a exclusividade Provocar+Defender NÃO entra agora, para testar
numa mudança única se a mesma causa física resolve os dois gates. Autor:
`lead-game-designer`, sobre o artefato congelado da C-bis (md5
`74f739173572af5c1a7564b7209a7a69`), escrita **ANTES** do run C-ter (pré-registro se fixa
antes do dado). **Proposta, não canon: o valor não entra em `combat_constants.hpp` sem
aprovação do líder via AskUserQuestion; o default no código segue 12,0 até lá, e a tabela
de §2.4 não foi alterada.**

### 9.1 Por que a conta do §8 errou 1,5pp para cima, e o que muda no método

O §8.1 previu 83,8% no P7 são (topo analítico 84,3% no P7b) com folga de 0,7pp; o medido
foi 85,1-85,7% no P7. A análise da C-bis (§9 dela, achado 1) identificou o erro de MODELO:
a tabela estática não continha o estado estacionário que o próprio taunt produz —
**provocador ferido, aliados sãos** (o provocador concentra os golpes, se fere, e o F1/F2
dele multiplica por 12 junto; os aliados, protegidos, nunca sobem o próprio peso). Esse
caso, na mesma fórmula, dá 1848/(1848+242) = 88,4% em M=12, e o medido (85,3%) é a mistura
entre o estado são (83,8%) e ele.

Consequência para esta seção, em duas regras:

1. **O estimador primário deixa de ser a tabela estática crua e passa a ser a INVERSÃO do
   dado medido.** O peso do provocador é `W_p × M` e o dos demais é `W_o`; a fatia medida
   determina a razão efetiva `r = W_p/W_o` que a dinâmica REALMENTE produziu, já com toda
   a realimentação de F1/F2 dentro:

   ```
   fatia_medida = M·r / (M·r + 1)   =>   r = fatia/(1 − fatia) / M      (com M = 12)
   fatia_estimada(M_novo) = M_novo·r / (M_novo·r + 1)
   ```

   Os 6 valores de `r` distintos, extraídos dos 8 pontos P7 medidos (33/12 e 34/12 são
   bit-idênticos, 4ª medição consecutiva):

   | Candidato | Braço titular (fatia medida → r) | Braço vizinho (fatia medida → r) |
   |---|---|---|
   | 33/12 = 34/12 | 85,3% → **0,4836** | 77,8% → **0,2920** |
   | 33/13 | 85,7% → **0,4994** | 78,2% → **0,2989** |
   | 39/11 | 85,1% → **0,4760** | 74,0% → **0,2372** |

2. **Viés do estimador, declarado com a direção:** `r` constante superestima levemente a
   fatia em M menor (com menos golpes, o provocador se fere menos e a realimentação do
   F1/F2 dele encolhe em direção à tabela estática). Superestimar **protege o teto**
   (o valor real fica abaixo da estimativa) e **come margem do piso** (idem). A escala do
   efeito é pequena — a fatia só se move ~1-4pp entre M=12 e M=10,5, então o ferimento do
   provocador quase não muda; estimado em ≤0,2pp — mas a assimetria pesa na escolha do
   valor em §9.3: a folga pedida no piso é maior que a do teto.

### 9.2 A restrição tem DOIS lados, e o §8 só olhou o teto

G1 pré-registrado: **70% a 85%, nos DOIS braços** (§2.8). Baixar o multiplicador afasta o
braço titular do teto e aproxima o braço vizinho do **piso**. Os dois piores casos da
grade, pelo dado medido:

- **Teto:** 33/13 titular (r = 0,4994; hoje 85,7%, o mais alto).
- **Piso:** 39/11 vizinho (r = 0,2372; hoje 74,0%, o mais baixo — a esponja alonga a luta,
  os aliados acumulam ferimento e o F2 deles dilui o taunt).

### 9.3 Varredura e escolha: 10,5 maximiza a menor folga

Fatia estimada nos dois piores casos, por M (fórmula de §9.1; folga = distância até a
borda mais próxima da faixa 70-85):

| M | Pior teto (33/13 titular) | Pior piso (39/11 vizinho) | Menor folga | Veredicto |
|---|---|---|---|---|
| 12,0 (atual) | 85,7% medido — **FORA** | 74,0% medido (folga 4,0) | — | reprovado no run oficial |
| 11,0 | 84,6% (folga 0,4) | 72,3% (folga 2,3) | **0,4pp** | crava o teto — repete o erro do §8 |
| **10,5** | **84,0% (folga 1,0)** | **71,4% (folga 1,4)** | **1,0pp** | **máximo da menor folga** |
| 10,0 | 83,3% (folga 1,7) | 70,3% (folga 0,3) | **0,3pp** | crava o PISO — o mesmo erro, na outra borda |
| 9,0 | 81,8% (folga 3,2) | 68,1% — **FORA** | — | reprova o piso |

**Valor proposto: `kMiraProvokeMultiplier` = 10,5.** É o ponto de equilíbrio da faixa com
o dado medido: nenhum M dá folga grande nos dois lados ao mesmo tempo, porque a banda
70-85 tem 15pp de largura e o espalhamento medido entre braços chega a 11,1pp (39/11:
85,1 × 74,0). A assimetria da escolha (1,4pp no piso contra 1,0pp no teto) é deliberada:
o viés declarado em §9.1 joga o valor real para BAIXO da estimativa nos dois braços, o
que adiciona segurança ao teto e consome piso — o lado com mais folga é o lado com viés
contra.

**Por que esta folga de 1,0pp é mais confiável que a folga de 0,7pp do §8 que inverteu:**
a do §8 saiu de tabela estática NÃO calibrada, com um erro de modelo de ~1,5pp fora dela
(a classe de erro era maior que a folga). Esta sai de inversão do próprio dado dinâmico a
N=300.000 (o efeito que derrubou o §8 já está DENTRO de `r`); o resíduo conhecido é o
viés de `r` constante (≤0,2pp, direção declarada) e o ruído amostral (centésimos de pp).
A folga é ~5× o erro residual conhecido, não menor que ele.

### 9.4 As contas com 10,5, no molde do §8

#### 9.4.1 Estimador primário (inversão), os 8 pontos da grade

| Candidato | Titular estimado (folga p/ teto 85) | Vizinho estimado (folga p/ piso 70) |
|---|---|---|
| 33/12 controle | **83,6%** (1,4) | **75,4%** (5,4) |
| 34/12 gêmeo | **83,6%** (1,4) | **75,4%** (5,4) |
| 33/13 quente | **84,0%** (1,0) | **75,8%** (5,8) |
| 39/11 molde | **83,3%** (1,7) | **71,4%** (1,4) |

Todos os 8 pontos dentro de 70-85, menor folga 1,0pp (33/13 titular) e 1,4pp (39/11
vizinho).

#### 9.4.2 Cross-check independente: a tabela estática do §8, agora com o 6º caso

Mesmas fórmulas de §2.5/§8.1 (Bento real, F1 do provocador = 4 no P7), M = 10,5 — e com o
caso que faltou ao §8, exigido pela lição da análise:

| Situação | Peso do provocador | Peso dos outros dois | Fatia |
|---|---|---|---|
| Party inteira sem ferimento (P7) | (100+4)×10,5 = 1092 | 242 | **81,9%** |
| Idem, P7b | (100+8)×10,5 = 1134 | 242 | **82,4%** |
| Aliados feridos pela metade | 1092 | 342 | **76,2%** |
| Aliados muito feridos | 1092 | 442 | **71,2%** |
| Provocador ferido pela metade, aliados muito feridos | (150+4)×10,5 = 1617 | 442 | **78,5%** |
| **Provocador ferido pela metade, aliados sãos (o 6º caso, estado estacionário do taunt)** | 1617 | 242 | **87,0%** |

Amarrando as duas rotas: em M=12, o medido (85,3%) fica a 32,6% do caminho entre o estado
são (83,8%) e o estacionário (88,4%). Aplicando a MESMA fração de mistura em M=10,5:
81,9 + 0,326×(87,0 − 81,9) = **83,6%** — idêntico ao estimador por inversão (83,6%). Duas
rotas independentes, o mesmo número.

**Registro honesto do P7b:** pela inversão (r = 0,5528, do medido 86,9%), o P7b em M=10,5
fica em ~**85,3%** — ainda acima de 85. O P7b é informativo por pré-registro (nenhum gate
reprova por ele); fica registrado aqui para ninguém se surpreender no relatório do C-ter.

### 9.5 G3: o que esta conta PREVÊ, com o limite dela declarado

A pergunta do líder embutida nesta rodada é se a mesma causa (multiplicador) resolve G1 E
G3. **A conta consegue prever G3, e a previsão é: NÃO resolve — G3 deve reprovar de novo
em M=10,5, e em qualquer M da faixa útil.** Modelo e calibração:

- No P7 a party só perde se o **Gus** cai (`check_end`); a vitória do G3 ≈ 1 − P(queda do
  Gus). A fatia de golpes que chega ao Gus é `(1 − fatia_do_provocador) × 106/242`
  (o peso relativo dele entre os não-provocados). A luta dura mediana 4 rodadas (G6) com
  ~9 ataques inimigos no total (3+3+2+1, morrendo pelo caminho); o Gus precisa de 5 golpes
  (7 de dano contra 34 HP) para cair. Aproximação de Poisson sobre o nº de golpes nele:
- **Calibração no medido (M=12):** fatia do Gus 6,4% → λ ≈ 0,58 → P(≥5 golpes) ≈ 0,03-0,06%.
  Medido no run oficial: quedas do Gus no P7 de 0,01-0,14%. O modelo acerta a ordem de
  grandeza no ponto que existe.
- **M=10,5:** fatia do Gus 7,2% → λ ≈ 0,65 → quedas ≈ 0,06% → **G3 ≈ 99,9%. FORA do teto
  de 97, de novo.**
- **M=6,0 (piso da faixa útil):** fatia do Gus ~11,2% → λ ≈ 1,01 → quedas ≈ 0,4% →
  **G3 ≈ 99,6%. Ainda FORA** — e nesse M o braço vizinho já reprovou o piso do G1 (63,7%).
- **O que seria preciso para G3 = 97%:** quedas do Gus ≈ 3% → λ ≈ 1,75 → fatia do Gus
  ~19,4% → fatia do provocador ≈ **56%** → M ≈ **2,6**. Fora da faixa útil (6,0-20,0) e
  ~14pp ABAIXO do piso do G1.

**Conclusão analítica, registrada PRÉ-RUN como previsão falseável: G1 e G3, como
pré-registrados, são incompatíveis no P7 de trash com o Bento real.** Não existe M que
satisfaça os dois: qualquer fatia dentro da banda 70-85% deixa tão poucos golpes no Gus
que a vitória fica ≥99%. A física é a que a análise da C-bis apontou (o tanque de Def
13/HP 55 não cai em luta de 4 rodadas — o P7b sem Defender também venceu ~100%); o
multiplicador muda ONDE os golpes caem, não o fato de que caem em quem aguenta.

**Limite da previsão, declarado:** é modelo (Poisson, golpes independentes, luta fixa em
~4 rodadas, ignora queda do Cauã alongando a luta), calibrado num único ponto medido.
Mas para G3 passar seria preciso um fator de 30-60× nas quedas do Gus em relação ao
medido, e nenhuma dessas imprecisões produz esse fator dentro da faixa. O run C-ter é
quem decide: **se medir G3 ≤ 97% com M=10,5, o modelo desta seção está errado e o
registro fica aqui para a autópsia.** Se confirmar a reprovação, a informação é limpa e
já paga o run — a causa não era a combinação com Defender nem o tamanho do multiplicador,
e a decisão seguinte (re-pré-registrar o teto do G3 para o trash-com-alavanca, ou G3
virar informativo no trash na mesma família da decisão do G4 — "o preço é AP, não pele" /
"a parede com alavanca vence mesmo") é **do líder**, não desta conta, e se fixa ANTES de
qualquer rodada que dependa dela.

### 9.6 G4, G5, G6 e H1-H5 com o valor novo

- **G5 (≤5pp entre braços) fica confortável, e a alavanca continua fazendo o trabalho da
  Fase C.** Os dois braços seguem vencendo ~99,9% (§9.5), diferença esperada ~0,1pp como
  no run oficial. E a pergunta de fundo — o Provocar ainda resolve a mira-esperta-no-
  cenário-parede? — segue respondida: no braço vizinho o provocador ainda leva **71-76%**
  dos golpes (peso são de 1092 contra 242 dos outros dois SOMADOS, 4,5×), enquanto o
  P2-vizinho sem alavanca segue com o Gus caindo 13-14%. Baixar de 12,0 para 10,5 tira
  ~2pp da fatia, não a natureza da ferramenta.
- **G6 (janela 3-5):** a duração é dirigida pelo DPS da party, que não muda (Provocar
  segue 1 AP). Mediana 4-5 inalterada.
- **G4 (informativo permanente no trash, decisão do líder 2026-08-12):** a previsão é o
  empate 0%×0% persistir — o Bento passa a levar MENOS golpes que antes. Coerente com a
  decisão: o preço do Provocar no trash é o AP, não a pele.
- **H1-H5:** intocados. O enrage não muda, o P6 não tem provocador, e H2 segue pareado
  por seed.

### 9.7 Seed do C-ter: recomendação (a decisão é do líder)

**Recomendo seed NOVA para o run oficial do C-ter (proposta: 20260812, a data), com um
braço informativo opcional na 20260801.** O motivo é de disciplina, não de número: o
valor 10,5 foi CALIBRADO sobre o dado da seed 20260801 (a inversão de §9.1 usa aqueles 8
pontos); re-rodar o veredito na mesma seed torna a confirmação in-sample — o run
confirmaria parcialmente a própria régua que o calibrou. Com N=300.000 o risco numérico é
desprezível (ruído de centésimos de pp contra folga de 1,0pp), mas o pré-registro é a
espinha do estudo inteiro, e o C-ter é exatamente o run que vai a veredito. O braço
opcional na 20260801 custa ~5 min de CPU (§4.2) e compra a comparação ponto a ponto com a
C-bis (o mesmo truque do pareamento do H2) — útil para atribuir qualquer surpresa ao
multiplicador e não à seed. O controle de regressão do empate bit a bit 33/12 × 34/12
funciona igual em qualquer seed (é interno ao run). Registrar a escolha e o motivo no
pré-registro do C-ter antes de disparar.

---

*§9 escrito pelo `lead-game-designer` em 2026-08-12 (item `PROV-MULT-AJUSTE`), sobre o
artefato congelado da C-bis e a análise datada. Proposta, não canon: o valor 10,5 aguarda
AskUserQuestion do líder; nenhuma linha de código foi alterada nesta fatia.*

---

## 10. Correção pré-declarada do G3 (exclusividade Provocar+Defender): mecanismo, destino do P7 e previsão pré-run — 2026-08-12

Registro exigido pela decisão do líder de 2026-08-12: testar a correção pré-declarada em
§2.7/§2.8 ("se a combinação estourar o teto de vitória, a correção pré-declarada é
torná-las mutuamente exclusivas, e não mexer no multiplicador"), depois de o run C-ter
medir G3 em 100,0%/99,8% contra teto 97% já COM o multiplicador corrigido
(`analise-pacing-fase-c-ter-20260812.md` §6). Autor: `lead-game-designer`, item
`PROV-MULT-AJUSTE`, escrito **ANTES** de qualquer implementação e de qualquer run
(pré-registro se fixa antes do dado). Proposta, não canon: nada aqui entra no motor sem
decisão do líder via AskUserQuestion.

### 10.0 O veredicto desta seção em cinco linhas (ler antes de gastar implementação)

1. **A previsão pré-run é que a exclusividade NÃO resolve o G3, e metade dela nem é
   previsão: é dado já medido.** O cenário-irmão P7b (só Provocar, sem Defender) é
   fisicamente INTOCADO pela trava, e o C-ter o mediu vencendo **99,98%**. Qualquer
   redefinição de P7 que colapse em P7b já tem o G3 medido: FORA do teto, sem rodar nada.
2. A única variante que sobra (P7 alternado, §10.2 opção b) tem previsão na banda
   **95-99%**, cavalgando o teto de 97: pela primeira vez na onda, a distância à borda é
   MENOR que a classe de erro demonstrada do modelo, e o veredito não é chamável (§10.4).
3. **Mesmo um PASS do alternado seria vácuo:** a estratégia mais simples (provocar todo
   turno) continua legal e continua vencendo ~100%. O propósito do teto E1 ("não pode
   trivializar") não é restaurável por NENHUMA trava entre Provocar e Defender, porque a
   trivialização nunca veio da combinação; veio da sobrevivência do tanque (medido duas
   vezes: C-bis e C-ter).
4. Efeito colateral pré-declarado: **a exclusividade REABRE o G1 que o C-ter acabou de
   fechar** (§10.3), nas duas rotas possíveis, e a rota P7b já está MEDIDA fora
   (85,3-85,6% contra teto 85), o que puxaria M ≈ 9,3 e mais um ciclo de calibração.
5. Recomendação (§10.6): apresentar ao líder a alternativa de custo zero (G3 informativo
   no trash-com-alavanca, opção 1 da análise do C-ter) ANTES de gastar implementação +
   run. Se ele mantiver a exclusividade, que seja como decisão de identidade do verbo
   ("ou eu chamo o golpe, ou eu aguento o golpe", Q2), não como conserto do G3, e nesse
   caso rodar a variante (b) com o pacote de §10.5.

### 10.1 Mecanismo exato da exclusividade (se aprovada)

```
Regra: exclusividade Provocar × Defender (nível de AÇÃO, memória de turno)

- Escopo: as duas AÇÕES BÁSICAS, no MESMO ator, no MESMO turno, nas DUAS direções:
  Provocar torna Defender ilegal até o fim do turno do ator; Defender torna Provocar
  ilegal até o fim do turno do ator.
- Fonte de verdade ÚNICA: um predicado de legalidade no domínio, no mesmo ponto em que o
  custo de AP já é validado. Três consumidores, a MESMA função:
    (1) o motor REJEITA a ação ilegal (fail-closed), nunca resolve em silêncio;
    (2) a UI consulta o predicado para desabilitar o botão, com tooltip do motivo;
    (3) qualquer brain (papéis do harness; ScriptedBrain de aliado, se um dia existir)
        passa pelo predicado. O harness chama produção, nunca reimplementa (o mesmo
        contrato que a paridade da mira já protege, §2.5).
- Estado: flag turn-local no contexto de turno do ator. NÃO é StatusEffect, NÃO entra no
  save (combate não persiste no meio), zera no fim do turno. A alternativa "bloquear por
  status" foi rejeitada: o Shield não rastreia origem (ação × carta), e as cartas têm de
  ficar isentas (E10).
- Log diegético (regra "todo efeito loga"), na rejeição e no tooltip:
  "postura já anunciada ao escalonador: o mesmo processo não pode ser isca e parede no
  mesmo ciclo."
```

Exceções declaradas (continuação da tabela de §2.6):

| # | Situação | Regra | Motivo |
|---|---|---|---|
| E10 | **Cartas de escudo e a carta honeypot (F5)** | **FORA da trava.** | A trava é entre as duas AÇÕES básicas. A carta paga bateria + posse (degraus de §2.1); estender a trava a cartas é decisão separada do líder, não default. |
| E11 | **Atores diferentes na mesma rodada** (um provoca, outro defende) | Livres. | A trava é por ator. |
| E12 | **Turnos diferentes do mesmo ator** (alternância) | Livre, por construção. | Provocado dura 1 rodada e expira no TurnStart do próprio ator, então não há sobreposição de status entre turnos. É o que define o P7-alternado de §10.2. |

Diegese (Pillar 1): um serviço não pode se anunciar deliberadamente vulnerável (honeypot)
e levantar o firewall no mesmo ciclo; o escalonador lê UMA postura por processo. A trava
tem leitura in-world limpa, o que conta a favor da opção "identidade de verbo" em §10.6.

### 10.2 O destino do P7: as três rotas, e por que a escolha é a alternância

Com a trava, o P7 como definido no protocolo §9.1 ("tanque provoca E defende") deixa de
ser jogável: ele É a combinação proibida. O protocolo §9.1 precisa de re-pré-registro
assinado antes de qualquer run.

| Rota | Definição nova do P7 | Prós | Contras | Veredicto |
|---|---|---|---|---|
| (a) | P7 := P7b (só provoca) | zero código de harness | a trava não toca a física do P7b: **G3 já está medido, 99,98%, FAIL** (mesma seed = bit-idêntico). O run não paga NADA; G3 e o informativo P7b colapsam no mesmo número | rejeitada |
| **(b)** | **P7-alternado**: rodada ímpar Provocar + 2 ataques; rodada par Defender + 2 ataques | é o uso tático mais defensivo que continua LEGAL; mede o custo real da trava (o buraco de cobertura nas rodadas de Defender); única rota em que o run compra número novo | G3 fica não-chamável de antemão (§10.4); G1 não sobrevive como está (§10.3) | **escolhida** |
| (c) | outra composição | — | nenhuma identificada que meça algo que (b) + P7b não meçam; quem propuser, que a pré-registre antes do dado | não proposta |

Consequências da rota (b) no harness e nos gates:

- Papel novo `ProvokeAlternateDefend` (espelha os existentes; chama produção). O papel
  antigo `ProvokeAndDefend` **não morre: vira teste negativo**: tentar Provocar+Defender
  no mesmo turno TEM de ser rejeitado pelo motor (é o teste que prova a trava viva); sai
  da grade oficial.
- **G3 não fica obsoleto e o teto NÃO muda:** teto 97 intocado (mexer no teto agora seria
  régua calibrada no dado que acabou de medir 99,8-100,0, o contra-argumento nº 2 da
  análise do C-ter); só o CENÁRIO é remapeado para o P7-alternado. Se a rota fosse (a),
  a resposta à pergunta "quais dos dois manter" seria: manter o G3 (gate decisivo, teto
  97) e aposentar a linha informativa duplicada do P7b; mas a rota (a) está rejeitada.
- P7b segue existindo e segue informativo, como pré-registrado. Nenhum gate novo nasce
  depois do dado.

### 10.3 O colateral que a decisão precisa enxergar: o G1 reabre

O G1 (fatia 70-85% dos golpes no provocador, §2.8) foi pré-registrado **sobre o P7**
(provoca + defende, 1 ataque restante) e foi calibrado em M = 10,5 para ESSE cenário
(§9.3). Com a trava, o cenário do G1 deixa de existir, e as três rotas de re-mapeamento
são todas ruins ou caras:

1. **G1 no P7b:** já está medido no C-ter: **85,3% / 85,6% / 85,4% > teto 85** (o §9.4
   registrou esse número "para ninguém se surpreender"; o P7b roda ~1,7-1,9pp mais quente
   que o P7 porque o provocador ataca 2× e a realimentação F1/F2 dele sobe). FAIL
   conhecido antes de rodar. Consertar exigiria M ≈ 9,3 (inversão com o pior ponto,
   33/13, r = 0,566: 5,25/0,566 = 9,28; a restrição de piso do §9.3 degenera, porque o
   P7b é bit-idêntico entre braços de mira) e MAIS uma rodada de calibração, o ciclo que
   a C-ter acabou de fechar.
2. **G1 no P7-alternado (fatia global):** dilui para ~**57-62%** (metade das rodadas sem
   taunt, com o Bento em ~30% de fatia natural), FORA da banda por baixo; e a banda
   70-85 perde o significado, porque passa a medir uma mistura de posturas.
3. **G1 condicional às rodadas com taunt ativo:** métrica nova nascendo depois do dado
   (proibido pela disciplina do estudo), e cairia de volta em ~85 (as rodadas de taunt do
   alternado se parecem com o P7b).

**Consequência pré-declarada: qualquer run da exclusividade exige decidir e re-assinar o
cenário do G1 ANTES do disparo**, junto com o protocolo §9.1. Sem isso, o run nasce sem
G1 legível, e o G1 é o gate que acabou de ser conquistado.

### 10.4 Previsão pré-run do G3 (a conta, com o limite dela declarado)

**Rota (a): não é previsão, é dado.** A trava não altera nenhuma ação que o P7b executa;
com a mesma seed, o resultado é bit-idêntico ao C-ter: vitória 99,98% titular, quedas do
Gus 0,015%. **FORA do teto de 97.** Zero informação nova num run.

**Rota (b), P7-alternado:** o modelo Poisson de §9.5, recalibrado para a alternância.

- **Premissa de ordem, ancorada no dado (não é chute):** a party age antes dos inimigos
  na rodada. Prova pelo próprio C-ter: se a rodada 1 do P7b fosse sem taunt, a fatia
  global teria de ficar vários pp abaixo da fatia por rodada; o medido (85,3% global)
  iguala a fatia por rodada, e a conta inversa (0,31×3 + f×6 = 0,853×9) exigiria f > 1,
  impossível. Logo a rodada 1 já é taunteada.
- Com a party agindo primeiro: rodadas COM taunt = 1 e 3 (5 dos ~9 ataques inimigos da
  luta mediana de 4 rodadas); rodadas SEM taunt (Bento defendeu, taunt expirou no
  TurnStart dele) = 2 e 4 (4 ataques).
- Fatia do Gus com taunt ativo: **7,2%** (§9.5, M = 10,5). Sem taunt: **~30%** no braço
  titular (peso 106 de 350) e **~36%** no vizinho (106 de 296; o F4 desconta o Bento
  defendendo). O Gus precisa de 5 golpes (dano 7 contra 34 HP) para cair.
- **Titular:** λ ≈ 5×0,072 + 4×0,303 ≈ **1,57** → P(≥5) ≈ **2,2%** → **G3 ≈ 97,8%.**
- **Vizinho:** λ ≈ 5×0,072 + 4×0,358 ≈ **1,79** → P(≥5) ≈ **3,5%** → **G3 ≈ 96,5%.**
- **Classe de erro demonstrada do modelo: fator de até ~3 nas quedas** (calibração do
  §9.5 contra o medido do C-ter: 0,06% modelado, 0,023%/0,19% medidos). Fator 3 sobre
  2,2% cobre 0,7-6,6%, ou seja G3 entre ~93 e ~99: **a borda de 97 está DENTRO da classe
  de erro.** Fontes de viés nas duas direções, declaradas: a realimentação F2 do Gus
  (golpes nele sobem o peso dele, clustering) empurra quedas para CIMA e G3 para baixo;
  o DPS maior do Bento alternado (2 ataques todo turno, contra 1 no P7 antigo) encurta a
  luta e empurra G3 para cima.

**Pré-registro honesto, falseável:**

| Ponto | Central | Banda declarada | Veredito chamável? |
|---|---|---|---|
| G3 P7-alternado, titular | **97,8%** | **95-99%** | **NÃO** (é a primeira previsão da onda em que a distância à borda é menor que o erro do método) |
| G3 P7-alternado, vizinho | **96,5%** | **94-98%** | NÃO; inclina a PASS. Veredito RACHADO entre braços é plausível |
| P7b (tudo) | bit-idêntico ao C-ter com a mesma seed | exato | SIM: 99,98% / fatia 85,3% |
| G1, se remapeado ao P7b | 85,3-85,6% | já medido | SIM: **FAIL por cima** |
| G1, se na fatia global do alternado | ~60% | 55-65% | SIM na prática: **FAIL por baixo** |
| G4 (informativo) | empate 0%×0% persiste | — | o Bento alternado sofre ~15/rodada só nas 2 rodadas de taunt (~30 de 55 HP): não cai |
| G6 | mediana 4, janela ≥99% | — | PASS (DPS igual ao do P7b) |
| H1-H5 | inalterados | — | PASS (o enrage e o P6 não são tocados) |

O que falsearia o MODELO desta seção: G3-alternado titular medido **< 94% ou > 99,5%**.
O que NÃO depende do número medido: **a vacuidade**. O P7b segue legal, mais simples e
já medido em 99,98%; se o G3-alternado passar, o gate fica verde com uma estratégia
trivial legal ao lado, que é exatamente o que o teto E1 existia para impedir.

### 10.5 O que um run compraria, e com qual seed

- **Rota (a): nada.** Re-medição de número conhecido.
- **Rota (b): UM número novo** (o G3-alternado, não-chamável de antemão) **mais o custo
  de cobertura da trava medido** (dano sofrido pela party no alternado; previsão: entre
  os 13,5% do pool do P7 e os 28,7% do P7b), que é o dado de design honesto que a trava
  produz: quanto a party paga pelas rodadas em que o taunt está em baixo.
- **Seed recomendada: a MESMA 20260812.** Aqui a preocupação in-sample de §9.7 NÃO se
  aplica: nada desta seção foi calibrado sobre o dado da 20260812 (a previsão vem de
  modelo, não de inversão desta amostra), e a mesma seed converte P7b/P1/P2/P6 em
  controles bit-idênticos de graça (o truque de pareamento do H2): qualquer movimento
  fora do P7 denuncia bug, não física.
- **Pacote mínimo antes do disparo, se o líder mandar rodar:** trava no domínio (§10.1) +
  papel `ProvokeAlternateDefend` + teste negativo do papel antigo + re-pré-registro
  ASSINADO do cenário do G1 e do protocolo §9.1 + esta seção como previsão registrada.

### 10.6 Recomendação transparente (dever de contra-argumentar; a decisão é do líder)

1. **O problema, nomeado:** a exclusividade remove a estratégia errada. O C-ter mediu
   (análise §6) que a combinação Provocar+Defender não é a causa da trivialização: tirar
   o Defender DOBRA o dano sofrido pela party e não move a vitória. A trava proíbe a
   jogada que só suaviza dano e mantém legal a jogada que trivializa.
2. **O risco concreto:** gastar trava + papel novo + re-pré-registro + rodada para, na
   previsão, terminar com G3 vermelho de novo, ou verde por um fio e vácuo; e com o G1,
   que o C-ter acabou de fechar, reaberto nas duas rotas (metade disso já está MEDIDA,
   não estimada: P7b 85,3-85,6% > 85; P7b vence 99,98%).
3. **Alternativas, em ordem de custo:**
   - (i) **G3 vira informativo no trash-com-alavanca** (opção 1 da análise do C-ter):
     custo zero, fecha o estudo com o dado existente (os 4 candidatos aprovam a bateria),
     mesma família da decisão já tomada para o G4 ("a parede com alavanca vence mesmo; o
     preço é AP"); G6 (duração 3-5) e E9 seguem vigiando o P7, então "vence sempre" não
     vira "vence sem jogar".
   - (ii) **Adotar a exclusividade como decisão de DESIGN** (identidade do verbo: "ou eu
     chamo o golpe, ou eu aguento o golpe"; a diegese de §10.1 é limpa), declarando que
     ela NÃO conserta o G3 e que reabre o G1; nesse caso, rodar a variante (b) com o
     pacote de §10.5. É legítima, mas é outra justificativa, e deve ser registrada como
     tal.
   - (iii) Se o objetivo for realmente mover o G3, a única alavanca que a física medida
     indica é a **sobrevivência do tanque contra trash** (opção 4 da análise: taunt com
     decay, trash que ameaça tanque), custo cheio de spec + pré-registro + rodada; com o
     registro honesto de que G6 verde (lutas de 4-5 rodadas) sugere que o "defeito" pode
     não ser sentível na mão do jogador.
4. **Decisão final: do líder, via AskUserQuestion** (o orquestrador leva as opções). Se a
   ordem for implementar e rodar mesmo assim, esta seção é o pré-registro, e a autópsia
   prometida caso o modelo erre.

---

*§10 escrito pelo `lead-game-designer` em 2026-08-12 (item `PROV-MULT-AJUSTE`), sobre a
análise congelada do C-ter e o dado já medido do P7b. Proposta, não canon: mecanismo,
redefinição de P7 e destino do G3 aguardam AskUserQuestion do líder; nenhuma linha de
código foi alterada nesta fatia.*

**VEREDITO DO LÍDER 2026-08-12: recomendação de §10.6 aceita. G3 vira informativo
PERMANENTE no trash** (mesma família da decisão do G4) — a exclusividade Provocar+Defender
NÃO é implementada; nenhuma linha de código desta seção entra no motor. Ciclo da onda
`PROV-MULT-AJUSTE` fechado: os 4 candidatos aprovam a bateria completa com o critério
vigente (G1+G5+G6+H1-H5 decisivos; G2/G3/G4 informativos).

---

*Spec escrita pelo `lead-game-designer` em 2026-08-11 sobre os dados congelados das Fases A,
A-bis, B e C e, segundo a redação original, sobre o código de produção lido em
`combat_constants.hpp` / `combat_state_machine.cpp`. Não é canon. Não commitada por
decisão de processo (commit e push só com ordem do líder).*

> ⚠️ **Não verificado contra código.** A afirmação de que esta spec foi escrita "sobre
> o código de produção lido em `combat_constants.hpp` / `combat_state_machine.cpp`"
> vem da análise de projeto, não de leitura de código executado. O código deste
> projeto nasce do zero (GODS_LAWS.md, L-01), e a fonte que a redação original citava
> não existe. O conteúdo de design desta spec continua válido como especificação;
> revalidar contra código quando houver implementação.
