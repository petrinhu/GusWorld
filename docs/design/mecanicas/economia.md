# Economia mínima + sistema de craft — GusWorld G1/F3-Alpha

**Status:** canônico (decisões ratificadas pelo criador via AskUserQuestion 2026-05-30).  
**Escopo:** economia mínima ativa no VS (F2); sistema de craft documentado aqui, implementação em F3-Alpha.

---

## §0. Doutrina do comedimento (canon, decisão do líder, 2026-08-01)

**Decisão, verbatim do líder (2026-08-01):**

> "As batalhas, os itens, tudo que tem a ver com economia financeira ou de jogo, devem levar
> sempre ao mesmo fim: quem é comedido sofre menos no final. O esbanjador sofre mais no final.
> Isso deve ser canônico do jogo e colocado sempre ao agente de economia neste projeto."

E sobre como isso escala com a periculosidade do inimigo, no mesmo dia:

> "depende do grau de periculosidade do inimigo. Minions comuns usam opções mais burras a maioria
> das vezes, até pra deixar o jogador mais feliz com mais vitórias, mas sem perder o desafio. e
> isso se inverte proporcionalmente até chegar o mestre final, quando já se evoluiu,
> comprou/achou/adquiriu cartas boas e vai depender de uma economia inteligente do jogador pra
> chegar lá e ter uma vitória menos dolorosa: quem esbanja na juventude tem uma velhice pobre.
> Quem poupa e tem constância, não sofre na velhice."

### §0.1 Régua de validação (aplica a TODA mecânica nova de economia, craft, dano, bateria, carta,
hospital, mira inimiga)

Toda feature nova de economia responde, antes de fechar: **"o jogador comedido sai melhor no
final do que o esbanjador, nesta mecânica?"** Se a resposta for NÃO (ou empate), **o número está
errado, não a regra**: ajusta-se valor, curva ou peso até a resposta virar SIM. A doutrina nunca é
descartada pra acomodar um número confortável.

### §0.2 Generaliza o precedente do mesmo dia

A carta de provocação (honeypot, `proposta-mira-inimiga.md` §2.A) já recebeu, no mesmo dia, um
teto de bônus de defesa de **no máximo 10%**, e a justificativa do líder foi explícita:

> "é um treinamento didático para o mundo real: devo investir meu dinheiro para o futuro com
> prudência prevendo problemas maiores ou gastar logo e ter prazer imediato? a dúvida da
> humanidade: prudência × yolo."

A doutrina desta seção não é uma regra nova isolada: é o mesmo eixo (prudência × yolo) elevado de
UMA carta específica para **todo o sistema econômico do jogo** (crédito, bateria, craft, mira).

### §0.3 Escopo

Economia de crédito (faucets/sinks, §2 a §7 abaixo), bateria/energia de cartas
(`cartas-hardware-pirataria-energia.md`), pirataria × original (arco Bastiat, mesmo doc §12),
craft/upgrade de tier (§7.6), e a mira do inimigo escalonada por periculosidade
(`proposta-mira-inimiga.md`, mapeamento proposto em `proposta-economia-comedimento.md`).

### §0.4 Não é sofrimento sem fim

A doutrina NÃO revoga o anti-dark-pattern já canônico (§3.3.3 trava anti-bola-de-neve; Pillar 4
"ameaçado, nunca massacrado"). O esbanjador sofre MAIS, nunca sofre SEM SAÍDA: sofrimento
ilimitado é dark pattern, não ensino. O piso de recuperação (safe mode, cap de dívida) continua
existindo para os dois perfis; o que muda é o RESULTADO relativo entre eles, não a existência de
uma saída.

### §0.5 Auditoria e proposta derivadas

Análise de coerência da economia atual contra esta doutrina, tabela de escalonamento de mira por
tier de periculosidade, e seeds de entrega narrativa: ver `proposta-economia-comedimento.md`
(PROPOSTA, não canon; pendente de aprovação item a item do líder).

---

## §1. Moeda

**Moeda única no VS:** `crédito` (digital, rastreável, canon lore-bible §11.4).  
**Token-rádio** (analógico, underground): lore dormido no VS; promove a dual-currency em F3+ quando a Periferia/Setor Mirage abrir mecanicamente. Sem dual-currency prematura.

---

## §2. Sources (entrada de crédito)

| Source | Valor | Anti-grind |
|---|---|---|
| Encontro vencido | **8 cr** × `max(0, 1 − (player_zone − enemy_zone) × 0.15)` | herda XP differential já implementado — farm de zona baixa definha |
| Baú / terminal hackeado | **13–34 cr** (fixo por achado) | curatorado, não farmável |
| Marco narrativo (beat de capítulo) | **55 / 89 / 144 cr** | sequência recorrente; determinístico; não repetível |

### §2.1 Recompensa de missão-capstone (Helion Tusk)

**Status:** aprovado pelo criador (2026-07-12). Cross-ref: [`21-helion-tusk.md`](../roster-analogos/21-helion-tusk.md) §"Rewards".

A missão-capstone do Helion Tusk (a carta 21, "A Carta Perdida de Tusk") paga um crédito de conclusão que ESCALA por dificuldade, em escada φ/Fibonacci, mantendo o MESMO peso relativo entre modos:

| Dificuldade | Crédito |
|---|---|
| Fácil | **610 cr** |
| Médio | **377 cr** |
| Difícil | **233 cr** |
| Hardcore | **144 cr** |

- **Razão ~62% entre vizinhos** (sequência Fibonacci/φ): a economia geral é mais magra no Difícil/Hardcore, então um número absoluto menor pesa igual à economia mais farta do Fácil — peso relativo idêntico em todo modo.
- **Hardcore RESETA** (save isolado, canon `project_morte_dificuldade_canon` / `reference_save_crypto_v2`): os 144 cr não são rede de segurança, são troféu de uma corrida que pode terminar em permadeath.
- **Precedente restrito:** é o 1º valor de crédito do jogo que varia por MODO de dificuldade em vez de por zona (a tabela acima varia por `mult(zona)`). Escopo travado à recompensa desta missão-capstone — NÃO gera ripple pra outros faucets (encontro, baú, marco narrativo continuam variando por zona, não por modo).

---

## §3. Sinks: Hospital

### 3.1 Cura paga (instantânea)
- HP faltando: **1 cr por 3 HP** (arredonda pra cima). Quem apanhou pouco paga pouco.
- Reativar companion incapacitado: **valor da sequência recorrente imediatamente acima do HP-máximo do companion** (`seq_acima(HP_max+ε)`)  
  - HP-max 55 → custo **89 cr** · HP-max 34 → custo **55 cr**  
  - "Multa" proporcional à força do companion. Consequência real, reversível.

### 3.2 Cura grátis (tempo diegético — NUNCA N encontros)
- HP: recovery ao cruzar **1 beat narrativo** (nó de mapa / gate de área).
- Companion incapacitado: recovery ao cruzar **2 beats narrativos**.
- Regra dura: cura grátis NUNCA é amarrada a vencer N combates (incentivaria grind — anti-pillar).

### 3.3 Derrota (party wipe): safe mode e dívida

**Status:** parâmetros ratificados pelo criador supremo (AskUserQuestion, 2026-06-24). Item de tabela de pendências: `ECONOMIA-HOSPITAL-DOC` (canonização formal do brainstorm economy-designer + criador de 2026-06-23).
**Cross-ref:** [`battle-screen.md`](battle-screen.md) §3.1 (tela de resultado `BUILD FAILED` que corta pro Hospital), [`combat-flavor.md`](combat-flavor.md) §3b (acervo de frases de vitória/derrota/performance da tela de resultado), [`combat.md`](combat.md) §2.1 (HP de Gus = menor da party; Análise Preditiva absorve 1 golpe fatal por batalha) e §16 (`ActorIncapacitated` / `ActorDefeated` aciona este fluxo), e [`pillars.md`](../pillars.md) Pillar 4 (Prodígio de 11 anos: "Companions — imortais com incapacitação" — cura gratuita demora, cura paga é rápida — e "Game over": derrota de Gus na party nunca é reload puro no fluxo de wipe abaixo, é custo econômico via Hospital).

A derrota NÃO é game-over e NÃO perde progresso: é um **custo econômico** (Pillar 4). O wipe corta pro Hospital, que cura pelo custo proporcional da §3.1 (1 cr / 3 HP) e reativa incapacitados pela §3.1 (multa da sequência numérica recorrente `seq_acima(HP_max)`). Se o jogador não tem crédito pra cura completa, ele **ESCOLHE** (anti-softlock, nunca trava):

#### 3.3.1 Safe mode (grátis)

- Revive a party a **13% do HP-máximo por ator** (sequência numérica recorrente; arredonda pra baixo; **piso 1 HP**). Ex: HP-max 34 → 4 HP, HP-max 55 → 7 HP.
- Como o HP de Gus é o menor da party (combat.md §2.1), o % incide sobre statlines desiguais e **preserva a hierarquia de fragilidade** (companion tanky revive com mais HP absoluto; Gus segue o mais frágil).
- Sai capenga, mas SEM death-loop: a **Análise Preditiva** (combat.md §2.1, recarrega por batalha) cobre o próximo golpe fatal, garantindo um colchão.
- **Só ofertado em wipe REAL** (party toda incapacitada). Nunca acionável voluntariamente estando vivo.
- **Anti-abuso (net-negativo sempre):** perder rende ~6 cr de "HP de mercado" (13% da party, ~18 HP, ~6 cr a 1 cr / 3 HP), mas custa o crédito do encontro (8 cr × mult) mais XP, loot e Knowledge, que NÃO são recebidos na derrota. Perder de propósito pra curar é sempre pior que curar pagando.

#### 3.3.2 Cura completa a crédito (dívida)

Alternativa ao safe mode: sai com **cura completa**, devendo. Saldo fica NEGATIVO. Lida diegeticamente como **"taxa de recompilação do snapshot"** do Hospital: transparente, plafonada, finita (Pillar 2: sistema formal legível). É o **oposto da usura opaca, composta, de compadrio** (estilo Sterling): livre-troca mais responsabilidade conservadora é o lado bom da axiologia canon.

**Termos (exibidos no terminal ANTES de aceitar, sem letra miúda):**

| Componente | Regra | Teto |
|---|---|---|
| **Principal** | 1 cura completa da party atual (~48 cr nas statlines do VS; calculado da party, sem hardcode) | custo da cura completa |
| **Juros** | **5% SIMPLES** sobre o principal (NUNCA composto), cobrado **1× por NOVA ZONA cruzada** (não por encontro, não por tempo) | **21% do principal** (~11 cr); param de crescer no teto |
| **Multa (reincidência)** | só ao sofrer wipe **enquanto ainda devendo**. Escada curta: **8 cr** na 1ª reincidência, **13 cr** da 2ª em diante, **TRAVA em 13 cr**. Evento pontual, nunca relógio | 13 cr |
| **Dívida total** | principal mais encargos (tetos próprios) | **~72 cr fixo** |

- **Encargos são cobrança morta:** juros e multa aumentam o que se deve, mas NUNCA compram mais cura nem movem o cap. O Hospital só empresta até o cap do principal.
- **Quitação automática:** o jogador escolhe o **plano** ao sair do Hospital (reajustável nas próximas saídas): **agressivo (62%** do crédito recebido vai pra dívida) / **médio (50%)** / **suave (38%)**; o resto fica livre pra jogar. Ordem de abate: **multa, depois juros, depois principal** (estanca a fonte de juros primeiro). Pior caso (~68 cr) quita em ~17 encontros de mult baixo no plano médio: sempre quitável jogando normal.
- **Bloqueia SÓ compras voluntárias** (loja / craft / itens). **NUNCA bloqueia o Hospital** (curar sempre disponível = anti-softlock; ao bater o cap, resta o safe mode grátis, que nunca trava).

#### 3.3.3 Trava anti-bola-de-neve (regra dura)

Juros simples (jamais composto) mais tetos fixos (juros 21%, multa 13 cr, dívida total ~72 cr) mais multa plafonada por-evento mais abate automático = **é matematicamente impossível a dívida crescer mais rápido do que se paga jogando normalmente**. A dívida só anda numa direção (pra baixo) enquanto se joga; nunca afunda sozinha parada. Esta seção é canon anti-dark-pattern: se algum ajuste futuro permitir composição de juros, cap móvel ou multa recorrente, ele viola este contrato e fere o Pillar 4.

---

## §3.4 Auto-resolve ("Resolver sem encarar"): parâmetros econômicos

**Status:** parâmetros ratificados pelo criador supremo (AskUserQuestion, 2026-06-25).
**Cross-ref:** o DESIGN da mecânica (UX opt-in, resolução via motor headless, tematização do build sem otimização, toggle de 3 estados, restrição só-trash) é canon em [`combat.md`](combat.md) e [`battle-screen.md`](battle-screen.md). Esta seção canoniza APENAS a ECONOMIA (loot, dano, derrota). O eixo de modulação é o **selo de domínio do bestiário** (`KnowledgeKills`, combat.md §11), o MESMO que liga o auto-kill canon no selo Ouro (overworld, INBOX COMBATE-AUTOKILL).

A mecânica oferece, ao iniciar um encontro de **trash**, a escolha "Iniciar batalha? [S/N]". **SIM** entra na batalha normal. **NÃO** ("não encarar") resolve a luta por cálculo, com **penalidade**: loot REDUZIDO em `x%`, dano sofrido MAJORADO em `y%`, e chance de derrota que cai no fluxo §3.3. Penalidade pela ausência de engajamento (a "preguiça") somada à falta de conhecimento do inimigo. Uma **2ª confirmação** exibe a `P(derrota)` e o `x/y` do tier ANTES de aplicar (RNG transparente, Pillar 2: nunca opaco).

### 3.4.1 Escopo e gate de onboarding (regra dura)

- **Só trash mobs.** Nunca elites, mini-bosses ou bosses (idêntico à restrição do auto-kill canon).
- **Gate de onboarding:** o "Resolver sem encarar" só é OFERTADO a partir do selo **Bronze** naquele tipo de inimigo. Inimigo **sem selo** (`KnowledgeKills` baixo, ainda não dominado nem um pouco) cai **direto na batalha**, sem oferecer o pulo. Racional: você não pode pular o que nunca encarou; protege o aprendizado do combate (o coração do jogo). O selo **Ouro** já é resolvido de graça pelo auto-kill canon no overworld (nem chega a esta tela).
- **Faixa de atuação:** esta mecânica vive no intervalo **Bronze → Prata** (o NÃO-dominado-mas-conhecido). Abaixo disso, luta; acima (Ouro), auto-kill grátis.

### 3.4.2 Penalidade modulada pelo selo (x = loot cortado, y = dano majorado)

Quanto MENOR o conhecimento, PIOR o auto-resolve (mais dano, menos loot, mais risco), porque você conhece menos o inimigo. Quanto MAIOR (perto do Ouro), mais brando.

| Selo (Knowledge) | `x` (loot cortado) | `y` (dano majorado) | Racional |
|---|---|---|---|
| Sem selo | n/a | n/a | NÃO ofertado (gate §3.4.1): cai direto na batalha |
| **Bronze** | **40%** | **+40%** | conhece pouco: resolve quase às cegas, apanha e pilha pouco |
| **Prata** | **13%** | **+13%** (sequência numérica recorrente) | quase no Ouro: penalidade simbólica, recompensa o domínio |
| Ouro | n/a | n/a | auto-kill canon (grátis, overworld); fora desta mecânica |

> Nota sobre o tier "Sem selo": o modelo de simulação contemplava parâmetros para `Sem selo = 40%/+40%`, mas a decisão do gate de onboarding (§3.4.1) torna esse tier inacessível por esta mecânica (cai direto na luta). Os parâmetros `40%/+40%` ficam atribuídos ao **Bronze**, que é a primeira faixa de fato ofertada. A escala efetiva é **Bronze 40%/+40% → Prata 13%/+13%**.

- `x` reduz a entrada de crédito do encontro (8 cr × mult de zona, §2): `cr_auto = 8 × mult × (1 − x)`.
- `y` majora o dano sofrido pela party, que vira custo de cura no Hospital a 1 cr / 3 HP (§3.1): `cura_auto = ceil(HP_perdido × (1 + y) / 3)`.

### 3.4.3 Probabilidade de derrota (cai no §3.3)

Resolução estatística instantânea (não rola turno-a-turno). A chance de a party "perder" o auto-resolve é modulada pelo selo:

| Selo | `P(derrota)` | Racional |
|---|---|---|
| **Bronze** | **8%** (sequência numérica recorrente) | espelha o `y%=8%` do auto-kill canon ("o bug resistiu / mutou") |
| **Prata** | **3%** | quase seguro |

- **Forma canônica vigente (VS):** tabela fixa por selo (8% / 3%) como placeholder honesto. Fórmula transparente plugável depois (mesmo padrão do combat.md): `P_derrota = clamp(0.03, base_selo × (1 − margem_de_poder), 0.20)`, onde `margem_de_poder` deriva de Atk da party vs HP/Def do inimigo. A `P(derrota)` é SEMPRE exibida na 2ª confirmação (Pillar 2).
- **Se perde:** cai **direto no fluxo de derrota canon §3.3** (corta pro Hospital → safe mode grátis a 13% HP-máx OU dívida plafonada ~72 cr). **ZERO economia nova:** o auto-resolve não cria currency, sink, nem regra de Hospital própria; reusa 100% o §3.1 e o §3.3.

### 3.4.4 Saldo modelado: lutar vs auto-resolver (anti-preguiça)

Simulação sobre as statlines do VS (party HP-total 144; trash; dano real por tier de domínio; mult 1.0):

| Selo | Lutar (saldo líquido) | Auto-resolver (saldo de vitória) | Auto-resolver (E[saldo] com `P(derrota)`) |
|---|---|---|---|
| Bronze | **+2 cr** | **−1 cr** | **~−3 cr** |
| Prata | **+4 cr** | **+2 cr** | **~+1 cr** |

**Invariantes econômicos (canon anti-degeneração):**

1. **Auto-resolver é SEMPRE pior que lutar** (delta de saldo negativo em todo selo e toda zona, incl. mult alto). O combate nunca morre por arbitragem de skip. É a trava anti-preguiça.
2. **Não é free-money:** o auto nunca supera o saldo de lutar; no Bronze é net-negativo absoluto.
3. **Não é death-loop:** o pior caso é da ordem de um encontro; o §3.3 (safe mode + dívida plafonada) é o colchão anti-softlock. A multa por reincidência do §3.3 (trava em 13 cr) já desencoraja spammar auto-resolve com a party machucada.
4. **Anti-abuso herda o §3.3.1:** perder de propósito no auto pra "ganhar HP de mercado" continua net-negativo (perde o encontro inteiro: crédito + XP + loot + Knowledge).
5. **No Prata vira levemente positivo** (+2 cr de vitória, ~+1 cr esperado): cumpre o propósito de **agilizar trash chato já bem dominado**, sem virar rota de farm.

### 3.4.5 Bônus de "lutar de verdade" (o auto-resolve nunca supera)

Reforço do contrato de incentivo (canon em battle-screen.md §3.1):

- **Lutar** entrega **loot / XP / Knowledge CHEIOS** (o auto corta `x%`) MAIS o **bônus de eficiência por build rápido** (`blazing fast` / `clean build`), que o auto-resolve **nunca** recebe; MAIS mestria de carta (+1 por uso) e contagem `KnowledgeKills` subindo mais rápido.
- **Auto-resolver** dá loot / Knowledge **básicos** (consistente com o auto-kill canon: "auto dá o básico; batalha real dá bônus").
- Gradiente contínuo Bronze → Prata → Ouro: conforme você domina, lutar fica mais limpo (menos dano sofrido) E o auto fica mais brando, até o Ouro assumir de graça. O jogador que otimiza sempre luta; o auto-resolve é uma válvula de conforto pro trash dominado, não um caminho de progressão.

---

## §3.5 Vitória: a contraparte econômica (faucet) do wipe (sink)

**Status:** canonização formal (item `ECONOMIA-HOSPITAL-DOC`); nenhum número novo — consolida o que já é canon em [`battle-screen.md`](battle-screen.md) §3.1 e [`combat-flavor.md`](combat-flavor.md) §3b.

Todo faucet/sink deste documento tem sua contraparte: o Hospital (§3.3) é o **sink** da derrota; a vitória em combate é o **faucet** equivalente. A tela de resultado imprime `BUILD SUCCEEDED` / `exit 0` (frases do acervo em combat-flavor.md §3b) e concede:

- **Crédito do encontro** — já quantificado em §2 (`8 cr × mult(zona)`).
- **XP, loot e Knowledge** (o selo do bestiário sobe — ver [`knowledge-progression.md`](knowledge-progression.md)) mais **mestria de carta** (+1 por uso, Pillar 1). Sistema de progressão paralelo à economia de crédito; não duplicado aqui.
- **Métrica de build (turnos = "compile time"):** build **rápido** ganha rótulo de elogio (`blazing fast` / `clean build`, acervo em combat-flavor.md §3b) **mais bônus de eficiência de loot/XP**; build **lento** recebe rótulo **NEUTRO** (`Build complete` / `Compiled` / `Done` / `Finished`) — **nunca xinga o jogador**, só constata.
- Este bônus de eficiência é o mesmo referenciado em §3.4.5 como o que o auto-resolve **nunca** alcança (reforça: lutar de verdade > pular > perder).

A derrota (§3.3) NUNCA anula os ganhos de vitórias anteriores — o wipe é um evento pontual e reversível, não um confisco retroativo.

---

## §4. Bio-Ampola (consumível de combate)

- **Cura dinâmica:** `seq_inferior(floor((HP_max − HP_atual) / 2))`  
  Ex: faltam 35 HP → metade = 17.5 → valor inferior da sequência = **13 HP curados**.
- **NÃO reativa** companion incapacitado (só Hospital ou Life Ampola).
- **NÃO comprável.** Drop curado (1-2 no VS inteiro).
- 1 AP pra usar em combate.
- *Dilema tático:* cura mais quando mais danificado → decisão "uso agora ou guardo?" (Pillar 1).

---

## §5. Life Ampola

- **Reativa** companion incapacitado (HP = `seq_inferior(HP_max / 2)` ao reativar).
- **Craftável** (sistema de craft §7) **OU comprável** com crédito premium no Hospital.
- Não é drop curado — obtida por craft ou compra deliberada.

---

## §5.5. Lanches e ingredientes de craft (canon do Gus Dragon)

> **Canon pessoal (L-16 de `GODS_LAWS.md`).** Os itens abaixo são as comidas e os ingredientes de
> craft favoritos reais do Gus Dragon, filho do líder e colaborador humano do projeto,
> transformados em item de cura e em insumo de craft do jogo com aceite dele. Decisão original em
> 2026-07-17 (chocolate, suco de uva; suco de limão e água com gás ficaram com efeito aberto);
> fechada pelo líder em 2026-08-25 (efeito de suco de limão e água com gás) e **expandida pelo
> líder no mesmo dia, por ditado direto**, de quatro para **doze comidas** mais **oito ingredientes
> de craft** (ditado preservado em `DITADO-COMIDAS.md`, scratchpad de trabalho). **Não confundir
> com o PERSONAGEM Gus** (Gustaf VII Tavus Vance), canon público e distinto. Fora do apelido "Gus
> Dragon" (ou `Dragon-Drv`), o nome de batismo dele não é citado, aqui nem em nenhum outro lugar do
> projeto.

### 5.5.1 Comidas (12)

Doze consumíveis. Seis têm efeito FECHADO pelo líder; seis foram citadas por nome no ditado de
2026-08-25 mas ainda aguardam efeito — marcadas abaixo com ⚠️, sem nenhum efeito sugerido neste
documento (L-11: quem decide é o líder, não o agente).

| # | Item | Efeito | Craft / refino | Decisão |
|---|---|---|---|---|
| 1 | **Chocolate** | cura de HP, a MAIOR do catálogo (é o favorito real do Gus Dragon) | nenhum; consumível simples | líder, 2026-07-17 |
| 2 | **Suco de uva** | cura de HP, média | nenhum; consumível simples | líder, 2026-07-17 |
| 3 | **Suco de limão** | limpa efeito negativo (debuff) da party | refina em **suco puro de limão** (insumo de bateria de baixa qualidade, `cartas-hardware-pirataria-energia.md` §5) | líder, 2026-08-25 |
| 4 | **Água com gás** | restaura **fôlego do corpo** (`core::player::WindedTimer`, `stamina.md`) — **não** é energia de ação/AP; lacuna anterior FECHADA nesta rodada, ver §5.5.3 | refina em **água destilada** (mesmo papel de insumo de bateria) | líder, 2026-08-25 |
| 5 | **Sanduíche de queijo, sem queijo derretido** | fôlego do corpo **e** HP juntos, pouco de cada (mesmo recurso "fôlego" da linha 4, agora fechado — ver §5.5.3) | nenhum decidido | líder, 2026-08-25 |
| 6 | **Tirinhas de frango** | **buff temporário de ataque ou força** — a categoria "buff ofensivo temporário" está fechada; qual das duas estatísticas exatas (se forem distintas no jogo) não foi escolhida pelo líder | nenhum decidido | líder, 2026-08-25 |
| 7 | Churrasco | ⚠️ EFEITO A DECIDIR PELO LÍDER | nenhuma ligação óbvia a ingrediente do §5.5.2 | líder cita o item, 2026-08-25 |
| 8 | Pastilha de menta | ⚠️ EFEITO A DECIDIR PELO LÍDER | nenhuma ligação óbvia | líder cita o item, 2026-08-25 |
| 9 | Macarrão ao alho e óleo | ⚠️ EFEITO A DECIDIR PELO LÍDER | nome cita **Alho** (§5.5.2) | líder cita o item, 2026-08-25 |
| 10 | Pão de alho | ⚠️ EFEITO A DECIDIR PELO LÍDER | nome e composição óbvia citam **Alho** e **Manteiga** (§5.5.2) | líder cita o item, 2026-08-25 |
| 11 | Pizza de atum | ⚠️ EFEITO A DECIDIR PELO LÍDER | nome e composição óbvia citam **Atum em lata** e **Orégano** (§5.5.2) | líder cita o item, 2026-08-25 |
| 12 | Macarrão ao molho pesto | ⚠️ EFEITO A DECIDIR PELO LÍDER | nenhuma ligação óbvia (pesto tradicional leva parmesão, mas o NOME do item não cita o ingrediente — não presumido aqui) | líder cita o item, 2026-08-25 |

### 5.5.2 Ingredientes de craft (8)

Catálogo à parte, ditado no mesmo dia. São insumos de craft, não consumíveis de cura (não
confundir com os 18 ingredientes já canônicos do sistema de Poções/Implantes, §7.8 acima, que são
de outra família temática: hardware/Selve/cripto). **Origem (§5.5.2a) e papel (§5.5.2b) fechados
nesta rodada (2026-08-25, ditado direto do líder).** Quantidade por receita, se pertencem a uma
bancada nova ou já existente, e todo número (preço, taxa de sucesso) continuam **não decididos**:
ver §5.5.2c e "Não decidido" ao fim de cada subseção.

| Ingrediente | Papel | Origem | Comida onde a ligação é óbvia pelo nome | Decisão |
|---|---|---|---|---|
| Queijo parmesão ralado | **EM ABERTO** | loja/conhecimento/exploração (§5.5.2a) | nenhuma ligação óbvia | papel não decidido; origem líder, 2026-08-25 |
| Orégano | **Temperar** | loja/conhecimento/exploração (§5.5.2a) | Pizza de atum | líder, 2026-08-25 |
| Alho | **Cozinhar** | loja/conhecimento/exploração (§5.5.2a) | Macarrão ao alho e óleo; Pão de alho | líder, 2026-08-25 (papel cozinhar); ver nota abaixo sobre 2º papel |
| Manteiga | **Cozinhar** | loja/conhecimento/exploração (§5.5.2a) | Pão de alho | líder, 2026-08-25 |
| Páprica | **Temperar** | loja/conhecimento/exploração (§5.5.2a) | nenhuma ligação óbvia | líder, 2026-08-25 |
| Atum em lata | **Cozinhar** | loja/conhecimento/exploração (§5.5.2a) | Pizza de atum | líder, 2026-08-25 |
| Tempero composto seco (cebola, alho, salsa) | **Temperar** | loja/conhecimento/exploração (§5.5.2a) | nenhuma das doze comidas cita o tempero composto pelo nome (evidência vem do próprio nome do ingrediente) | líder, 2026-08-25 |
| Camarões sem casca | **EM ABERTO** | loja/conhecimento/exploração (§5.5.2a) | nenhuma das doze comidas cita camarão | papel não decidido; origem líder, 2026-08-25 |

#### 5.5.2a Origem dos oito (decisão do líder, 2026-08-25)

Perguntado entre "só loja", "loja mais drop travado por conhecimento" e "exploração e mundo", o
líder respondeu **"2+3"**: as duas opções combinadas, não uma escolha entre elas.

1. **Loja e negociação com personagens**, mesmo canal já usado pro suco puro de limão
   (`cartas-hardware-pirataria-energia.md` §5: *"O suco puro de limão também pode ser obtido em
   lojas e negociações com personagens"*), estendido aqui à família dos ingredientes de cozinha.
   Mais **drop travado por conhecimento** para o que for raro, precedente já fixado pros outros 18
   ingredientes do sistema de Poções/Implantes (§7.8 acima).
2. **Exploração do mundo**: armário, cozinha, baú, horta. Achado físico no cenário, não comprado
   nem dropado de inimigo.

**Exclusão explícita, escrita para não se reabrir sozinha:** nenhum dos oito vem de **drop de
inimigo comum (trash mob) repetido**. O corpus já fecha essa porta pros 18 ingredientes irmãos,
verbatim: *"não dá pra farmar trash mob pra conseguir componente-boss"* (§7.8 acima, nota
anti-grind). A mesma frase vale aqui palavra por palavra, porque a mesma porta furaria o mesmo
corte, **C-15** (campanha de 4 a 8 horas: farm de trash mob é tempo de jogo que a campanha não
tem), e o **Pilar 1** ("resolve por análise, nunca por grind"). Farm de inimigo comum não é fonte do
craft de cozinha; isto não é omissão a ler como brecha, é vetado pelo mesmo racional que já veta
pro resto do jogo.

**Não decidido:** qual ingrediente específico vem de qual canal específico (esta loja, aquele NPC,
aquele baú, aquela horta) é posicionamento de conteúdo/level design, não princípio; fica para
`economy-designer` e para quem desenhar os espaços de exploração. A tabela acima registra a mesma
política de origem pros oito porque foi isso que o líder decidiu: uma política uniforme pra família
inteira, não uma distribuição individual item a item.

#### 5.5.2b Papel de cada um (decisão do líder, 2026-08-25)

Perguntado se os oito craftam as doze comidas, se temperam comida já existente, se são insumo de
bateria, ou os três, o líder respondeu: **os três papéis, conforme o ingrediente.** Ele não disse
qual é qual; o mapeamento abaixo vem só de onde o próprio nome do ingrediente ou o nome do prato já
falam. Onde não é evidente, o papel fica **EM ABERTO**, não distribuído por simetria.

- **Cozinhar** (vira comida, base/fundação do prato, não aditivo de prato pronto): **Alho**
  (Macarrão ao alho e óleo: o prato É alho refogado no óleo. Pão de alho: alho+manteiga viram
  estrutura do pão assado), **Manteiga** (Pão de alho, funde e vira estrutura do pão), **Atum em
  lata** (Pizza de atum, é a proteína que dá identidade ao prato, não algo polvilhado por cima).
- **Temperar** (não cria prato, melhora um que já existe): **Orégano** e **Páprica**. Nenhuma das
  doze comidas cita a páprica pelo nome, e o orégano só aparece ligado à Pizza de atum; a evidência
  aqui não é nome de prato, é a natureza do próprio ingrediente: as duas são especiarias clássicas,
  sempre usadas em pequena quantidade pra realçar sabor de um prato já pronto, nunca como
  base/corpo dele (diferente do Alho, que É a base do prato nas duas citações que tem). **Tempero
  composto seco (cebola, alho, salsa)**: mesma lógica de evidência-pelo-nome-do-ingrediente, mais
  direta ainda. Nenhuma das doze comidas o cita pelo nome, mas o PRÓPRIO NOME do ingrediente já
  declara a natureza ("Tempero composto").
- **Insumo técnico** (vai para craft de bateria): **nenhum dos oito.** Essa família já está
  representada no catálogo por **suco puro de limão** e **água destilada**, refinados das comidas
  #3 e #4 (`cartas-hardware-pirataria-energia.md` §5), que não fazem parte deste catálogo de oito.
  Não forçado aqui por não haver evidência.
- **EM ABERTO** (nome não diz): **Queijo parmesão ralado**. Nenhuma das doze comidas o cita pelo
  nome (nem a #12, Macarrão ao molho pesto: pesto tradicional leva parmesão, mas o NOME do prato
  não cita o ingrediente, e o §5.5.1 já registrou "não presumido aqui"; a mesma cautela se aplica
  de novo, agora ao papel). Natureza (queijo ralado) não decide sozinha entre temperar e cozinhar.
  **Camarões sem casca**: nenhuma das doze comidas cita camarão pelo nome; natureza (proteína) não
  decide sozinha entre cozinhar e insumo técnico.

**Alho: papel duplo sinalizado, não confirmado.** Alho está confirmado como **Cozinhar** por
citação de nome em dois pratos, nos dois como base do prato, nunca como tempero de prato pronto. A
convenção culinária brasileira comum trata alho como base de "tempero" (refogado de alho e cebola),
mas essa é convenção cultural geral, não evidência textual deste corpus: nenhuma das doze comidas
usa alho no sentido de "tempera um prato já pronto". Fica sinalizado, não decidido: se o líder
quiser um segundo papel pro alho (temperar, além de cozinhar), é decisão dele, não inferência deste
documento.

**Não decidido:** quantidade por receita, se um ingrediente EM ABERTO acaba tendo papel único ou
duplo quando o líder decidir, e todo número de balanceamento (`economy-designer`, quando a onda
chegar lá).

#### 5.5.2c Bancada de cozinha: sistema novo, e a existência dela não está confirmada como decisão do líder

**Achado, não decisão:** não existe, em nenhum lugar do canon deste projeto, uma "bancada de
cozinha" ou sistema equivalente de craft de comida (busca dirigida no `docs/` inteiro por
`bancada de cozinha`, `estação de cozinha`, `kitchen`, `craft.*comida`, `cozinhar comida`: zero
ocorrências fora deste parágrafo). As três estações de craft já canônicas (§7.1: Bancada de
Compilação/Jaci, Forja de Firmware/Gus, Patch/Recompilação/Gus) são todas temáticas de
hardware/compilação (Pillar 1: "craft = compilação de componentes via hardware... nunca
caldeirão"), e a bancada RSB das oficinas do mercado negro/ferro-velho
(`cartas-hardware-pirataria-energia.md` §4/§14, usada pro refino de suco puro de limão/água
destilada e pela cópia pirata de carta) também é hardware/pirataria-themed. Nenhuma das quatro
cozinha comida. **Conclusão: se "bancada de cozinha" existir como sistema de jogo, ela é NOVA, não
é reuso de nenhuma bancada hoje canônica.**

**Ressalva sobre a origem da própria frase:** a ordem de serviço desta rodada cita "bancada de
cozinha" como o local onde o papel Cozinhar acontece, atribuído ao ditado do líder registrado em
`DITADO-COMIDAS.md`. Esse arquivo, porém, não existe neste checkout do repositório (busca por
`DITADO-COMIDAS.md` no projeto inteiro: nenhum resultado), então este documento **não pôde
verificar se "bancada de cozinha" é o termo literal do líder ou paráfrase de quem redigiu a ordem
de serviço.** Por cautela (memória de sessão: exemplo/paráfrase de quem explica uma decisão não é a
decisão em si), este documento registra o PAPEL "cozinhar" como fechado pelo líder, mas trata a
EXISTÊNCIA, o NOME e a LOCALIZAÇÃO de uma "bancada de cozinha" como **não confirmados**: pergunta
para o líder, não fato assentado aqui.

### 5.5.3 Fôlego do corpo × energia de ação (LACUNA FECHADA, 2026-08-25)

A versão anterior deste documento registrava como lacuna aberta se "água com gás" mirava o
**fôlego do corpo** (`core::player::WindedTimer`, `stamina.md`) ou a **energia de ação (AP)** de
combate. O líder fechou isso nesta rodada, por ditado direto: água com gás restaura **fôlego do
corpo**, não AP. A mesma leitura vale para a linha 5 (sanduíche de queijo sem queijo derretido),
que soma fôlego a HP.

Os dois recursos seguem distintos, como já eram:

1. **Fôlego do corpo** (`core::player::WindedTimer`, `stamina.md`) — a inércia da respiração
   ofegante do Gus no overworld, agora com fonte de recuperação por item (água com gás, sanduíche
   de queijo).
2. **Energia de ação (AP)** de combate — usada pela Bio-Ampola (§4) e pelo custo de troca de
   bateria em batalha (`cartas-hardware-pirataria-energia.md` §5, "In-battle (arena)"). **Nenhuma**
   das doze comidas restaura AP; se o líder quiser um item de cura que reponha AP, é decisão nova,
   não coberta por este ditado.

Números finais (magnitude de cura, quantidade de debuff limpo, valor de fôlego restaurado, preço,
fonte de drop/compra) continuam do `economy-designer`, na MESMA curva numérica canônica já usada
para Bio-Ampola (§4) e Life Ampola (§5): nenhum decidido aqui.

**O refino e o destino do que sai das comidas 3 e 4 são canon de carta, não de crédito, e vivem em
`cartas-hardware-pirataria-energia.md` §5** ("Bateria de baixa qualidade craftada"), porque o
consumo final é energia de carta, o mesmo domínio que já governa bateria, pirataria e RSB. Este
documento é o dono do CATÁLOGO das doze comidas, dos oito ingredientes de craft e da curva de
crédito deles; o outro é o dono do que acontece quando suco de limão e água com gás viram insumo
de hardware, inclusive o trade-off entre consumir agora e guardar para refinar, e agora também da
herança de risco da bateria craftada (mesma subseção, ver relatório desta rodada).

---

## §6. Companion KO

- `ActorIncapacitated` emitido (engine/foundation/turn_combat — já implementado).
- Companion fica **inativo** (fora da fila de iniciativa) até:
  - Cura paga no Hospital (custo §3.1), OU
  - 2 beats narrativos de descanso (grátis), OU
  - Life Ampola usada em campo (reativa com HP parcial).
- **Game-over** só se Gus (`ActorDefeated`): reload no Normal; companion KO ≠ game-over.

---

## §7. Sistema de Craft (implementação: F3-Alpha)

> CUT.3 revisado 2026-05-30: "craft complexo" cortado → substituído por "craft narrativo-gated enxuto" (anti-grind, sem durability, 13 receitas, 18 ingredientes). Reaberto conscientemente pelo criador como escopo de F3-Alpha.

### 7.1 Diegese
Craft = **compilação de componentes via hardware de Gus** (Pillar 2: magia=sistema formal). Nunca caldeirão; sempre determinístico, nunca RNG. Tem os componentes → compila.

**3 estações:**
1. **Bancada de Compilação** (Jaci, laboratório bio) → Poções
2. **Forja de Firmware** (Gus, terminal DRE hackeado) → Implantes + upgrades de stat ("armas")
3. **Patch/Recompilação** (Gus, save-point) → Consertos narrativos + melhorias de tier

### 7.2 Poções (5 receitas, Bancada de Compilação / Jaci)

| # | Nome | Tier | Ingredientes | Efeito mecânico |
|---|---|---|---|---|
| P1 | Ampola de Regen | 1 | 3 | status `Regen` mag 3 / dur 3 |
| P2 | Ampola de Antídoto | 1 | 3 | Dispel Poison/Corrode + imune DoT 2 turnos |
| P3 | Life Ampola | 2 | 5 (1 raro) | reativa companion incapacitado |
| P4 | Ampola de Sobrecarga | 2 | 5 | `Haste` mag 1 + +2 mana próximo turno |
| P5 | Ampola Recursiva | 3 | 8 (1 componente-boss) | cura recorrente por 5 turnos (1,1,2,3,5) + `Shield` |

### 7.3 "Armas" = Cartuchos Tavus-Drive + upgrades de stat (Forja de Firmware / Gus)

**Cartuchos passivos** (modulam resolução de cartas sem criar carta nova — GDD §9 proíbe craft de cartas):

| Nome | Efeito |
|---|---|
| Cartucho de Foco | +1 Power em cartas de 1 família escolhida |
| Cartucho de Cadência | 1ª carta do turno custa -1 mana |
| Cartucho de Eco | combos casados ganham +0.15 multCombo |

**Upgrades de stat** (calibração de firmware):

| Nome | Efeito |
|---|---|
| Calibração de Atk | +Atk flat (sigmoid, cap Tier 3) |
| Calibração de Def | +Def flat |
| Sincronização de SPD | +Spd (afeta fila §4) |

### 7.4 Implantes (5, ancorados no triângulo Pillar 3 — NUNCA cria 4º vértice)

| # | Implante | Vértice | Efeito |
|---|---|---|---|
| I1 | Lente de Varredura Profunda | Óculos | Scan revela buffs/debuffs + posição na fila |
| I2 | Antena de Longo Alcance | Matriz Ortodôntica | Gambito-Prever +1 turno à frente |
| I3 | Cache de Pulso | Tavus-Drive | cartas Elétrico -1 AP (mín 1) |
| I4 | Co-processador de Gambito | Óculos | Gambito-Reordenar custa 1 AP (era 2) |
| I5 | Amplificador Ortodôntico | Matriz Ortodôntica | Scan vira passivo (0 AP) contra 1 tipo extra |

### 7.5 Consertos (Patch/Recompilação — narrativo/scriptado, NUNCA durability)

Hardware **nunca quebra por uso normal**. Conserto = consequência narrativa pontual da sabotagem do Dante (foreshadow canônico):
- Boss danifica vértice em cutscene scriptada → Recompilação (custo crédito OU componente, 1× pontual).
- Pós-reveal traição Dante → puzzle de "limpeza de rootkit" (não craft, não consumo de recurso).
- Sabotagem progressiva → opt-in em save-points, nunca obrigatório.

**Regra dura: zero durability sistêmica.** Nenhum tax de engajamento. Pillar 4 inegociável.

### 7.6 Melhorias de Tier (T1→T2→T3)

Método: **recompilação over o item existente** (não receita nova separada).

```
T1 → T2:  item T1 + 1 componente-upgrade + ~150 cr
T2 → T3:  item T2 + 1 componente raro/boss + ~400 cr
```

Curva **sigmoid** (GDD §5.4): efeito ×1.6 por salto, platô em T3 — sem power-creep exponencial.

### 7.7 Custo de craft

**Componentes OU crédito-premium** (mesmo modelo da Life Ampola): player decide entre coletar ingredientes ou pagar mais caro. Sem hard-stuck; sink adicional de crédito.

### 7.8 Ingredientes (18, distribuídos por família/zona)

| # | Nome | Tipo | Fonte | Raridade |
|---|---|---|---|---|
| 1 | Seiva Recursiva | Selve | drop Bio/Selve | comum |
| 2 | Esporo Sintético | Selve | drop Selve (T5 Solo) | comum |
| 3 | Capacitor Sucateado | eletrônico | drop Periferia / compra | comum |
| 4 | Fio Condutor | eletrônico | drop Elétrico/Metal | comum |
| 5 | Fragmento de Dado | cripto | drop Criptográfico | comum |
| 6 | Cristal Bioluminescente | Selve | drop Selve (Biolum.) | incomum |
| 7 | Núcleo de Latão | eletrônico | drop Cinético/Catedrais | incomum |
| 8 | Ressonador Sônico | eletrônico | drop Sônico/Silêncio | incomum |
| 9 | Chave de Decriptação | cripto | drop Cripto/Mirage | incomum |
| 10 | Bio-gel Estável | Selve | compra (hospital/Jaci) | incomum |
| 11 | Lente Polida | eletrônico | compra / drop raro | raro |
| 12 | Antena UHF Rara | eletrônico | drop mini-boss | raro |
| 13 | Esporo-Mestre | Selve | drop inimigo-mestre (Knowledge-gated) | raro |
| 14 | Dado Íntegro | cripto | drop raro / puzzle | raro |
| 15 | Liga de Acaceiro | Selve | drop curado raro | raro |
| 16 | Núcleo de Recompilação | universal | compra cara (crédito) | incomum |
| 17 | Componente-Boss: Núcleo Quente | boss | drop boss único | épico |
| 18 | Componente-Boss: Fragmento Locke | boss | drop boss (Sterling) | épico |

Anti-grind: ingredientes raros/épicos vêm de Knowledge-gated drops (inimigos-mestre/boss únicos, poucas aparições). Herda o anti-grind do XP differential automaticamente — não dá pra farmar trash mob pra conseguir componente-boss.

### §7.9 Pedágio de bancada de terceiro e rede grátis do Tusk

**Status:** aprovado pelo criador (2026-07-12). Cross-ref: [`21-helion-tusk.md`](../roster-analogos/21-helion-tusk.md) §"Rewards".

**Regra geral:** craft numa bancada que NÃO é uma das 3 estações da party (§7.1: Bancada de Compilação/Jaci, Forja de Firmware/Gus, Patch/Recompilação/Gus) cobra um **pedágio de acesso de 13 cr fixo por uso** (Fibonacci), em cima do custo normal do craft (§7.7). As 3 estações da party seguem 100% grátis — sem mudança.

**Reward do Tusk:** ao concluir a missão-capstone (§2.1), a party ganha acesso grátis pra sempre à **rede de 5 bancadas do Tusk** (1 na Montadora Confluência + 4 nas frentes do consórcio dele, ver `21-helion-tusk.md` §"Fluxo da missão"): o pedágio de 13 cr zera nessas 5 estações especificamente, pelo resto do jogo. Economia poupada estimada em **~130-195 cr** no pós-jogo (10-15 usos × 13 cr).

- **Hardcore reseta a rede grátis também** — consistente com o save isolado (§2.1): a concessão está ligada ao save daquela corrida, não é permanente cross-save.
- **Plantio narrativo (2 menções cedo, antes da capstone):**
  1. **Ato 1:** um NPC artesão avisa a party que "bancada não é de graça, treze créditos" — primeira fricção com o pedágio, tom informativo, sem drama.
  2. **Meio de jogo:** numa das empresas do consórcio do Tusk (cadeia "Em Busca do Mega-Empresário", `21-helion-tusk.md`), a party PAGA o mesmo pedágio de 13 cr antes da capstone — pro "grátis pra sempre" do fim virar concessão GANHA (payoff), não um freebie que cai do nada.
- **Balanceamento:** é fricção de conveniência, NÃO sink essencial — Hospital (§3), ingrediente e melhoria de tier (§7.6) seguem cheios e obrigatórios. É reward de pós-endgame; não relaxa a escassez de crédito já estabelecida em §2/§3.

---

## §8. Estimativa de implementação (F3-Alpha)

| Etapa | Sessões |
|---|---|
| Doc canônico + planilha de simulação | 2.5 |
| Records POCO (Recipe, Ingredient, CraftStation, Implant) | 1 |
| CraftService (resolução determinística + testes) | 1.5 |
| Integração PlayerBus (crédito, inventário, drops) | 1 |
| Aplicação implantes/cartuchos no CombatActor | 1.5 |
| Melhoria de tier | 1 |
| UI de craft (Diário do Gus / estação) | 1.5 |
| Consertos narrativos (hooks scriptados) | 0.5 |
| QA adversarial (arbitragem, dominância, hard-stuck) | 1 |
| **Total F3-Alpha** | **~11.5 sessões** |

---

**Última revisão:** 2026-08-25, mesma data, mais uma rodada (§5.5.2a/b/c: fecha as duas lacunas que
a expansão anterior deixava nomeadas, a origem dos oito ingredientes de craft ("2+3": loja/negociação
com personagens mais drop travado por conhecimento, E exploração do mundo; exclusão explícita de
drop de inimigo comum repetido) e o papel de cada um (cozinhar/temperar/insumo técnico, atribuído só
onde nome de ingrediente ou de prato já diz: Alho/Manteiga/Atum em lata cozinham; Orégano/Páprica/
Tempero composto seco temperam; Queijo parmesão ralado e Camarões sem casca ficam EM ABERTO; nenhum
mostra evidência de insumo técnico). Sinaliza, sem confirmar, um possível segundo papel de temperar
pro Alho, e registra que "bancada de cozinha" não tem precedente em canon nenhum: se existir, é
sistema novo, e a própria existência/nome dela não pôde ser verificada contra o ditado original.
Revisão anterior, mesmo dia (expansão §5.5: o líder dita, direto, a ampliação de
quatro para **doze comidas** mais **oito ingredientes de craft**; fecha a lacuna fôlego-do-corpo ×
energia-de-ação para água com gás, extensiva ao sanduíche de queijo; liga ingrediente a comida só
onde o nome já diz — pão de alho/Alho+Manteiga, pizza de atum/Atum em lata+Orégano, macarrão ao
alho e óleo/Alho; seis comidas citadas sem efeito ficam marcadas ⚠️, nenhum efeito sugerido no
canon; `cartas-hardware-pirataria-energia.md` §5 fecha em paralelo se a bateria craftada herda os
riscos da pirata comprada). Revisão anterior, mesmo dia: 2026-08-25 (adição §5.5, catálogo dos
quatro lanches favoritos reais do Gus Dragon como itens de cura, canon pessoal sob L-16 de
`GODS_LAWS.md`; suco de limão e água com gás ganham efeito e viram matéria-prima de craft de
bateria de baixa qualidade, canonizado em `cartas-hardware-pirataria-energia.md` §5). Revisão
anterior: 2026-08-01 (adição §0, doutrina do
comedimento: "quem é comedido sofre menos
no final, o esbanjador sofre mais", canonizada pelo líder e escalonada por periculosidade do
inimigo; régua de validação aplicável a toda mecânica de economia nova; auditoria de coerência e
mapeamento de mira por tier derivados em `proposta-economia-comedimento.md`, ainda PROPOSTA).
Revisão anterior: 2026-07-12 (adição §2.1 recompensa de missão-capstone Helion Tusk escalada por
dificuldade + §7.9 pedágio de bancada de terceiro/rede grátis do Tusk; specs aprovadas pelo
criador, cross-ref `21-helion-tusk.md` §"Rewards"). Revisão anterior a essa: 2026-07-06
(canonização `ECONOMIA-HOSPITAL-DOC`: cross-refs explícitos §3.3 + §3.5 vitória, sem números
novos). Economia mínima = canon VS. Craft = spec F3-Alpha.
