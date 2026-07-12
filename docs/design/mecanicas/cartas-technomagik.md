# Cartas, Poções e Itens do TechnoMagik (GusWorld)

**Status:** PROPOSTA (decisões do criador 2026-07-12, aguarda revisão). Consolida decisões fechadas do criador supremo sobre a taxonomia de cartas, poções e itens do sistema TechnoMagik. Documento de design, não de implementação: specs de engine (records, services) ficam para os agentes de implementação depois da revisão.

**Atualização 2026-07-12 (rodada 2):** fecha os 5 gadgets, a contagem de slots (deck/Codex, implantes do triângulo, companions, gadget, injetor, especial, ampolas em combate) e a dupla Força/peso (atributo de logística + tabela de pesos por item). Os números finos de preço em crédito e as magnitudes exatas dos 2 status novos seguem diferidos, agora apontados explicitamente para as ondas que os fecham (ver §9).

**Cross-ref (leia antes, este doc não duplica):**

- [`combat.md`](combat.md) §6 (5 famílias e roda de fraqueza), §7-8 (modelo de carta e modificadores), §9 (status framework), §10 (pipeline de 3 slots e combos), §11 (fórmula de dano). Este doc referencia esses números, não os redefine.
- [`../techmagic.md`](../techmagic.md): o guarda-chuva "TechnoMagik" (grafia canônica desde 2026-07-12; "techMagic" no corpo daquele doc é grafia antiga pendente de reconciliação em `TECHMAGIC-CANON`), a metafísica de 3 camadas, e o Tavus-Eco.
- [`economia.md`](economia.md) §4-5 (Bio-Ampola e Life Ampola já canônicas), §7 (sistema de craft F3-Alpha), §7.9 (pedágio de bancada de terceiro e rede grátis do Tusk).
- [`../pillars.md`](../pillars.md) Pillar 2 (magia é sistema formal computável) e Pillar 3 (triângulo de hardware Óculos/Matriz/Tavus-Drive).
- [`../narrative/lingua/02-lexico-semente.md`](../../narrative/lingua/02-lexico-semente.md): as 13 raízes Sylvarin usadas no naming (§8 abaixo).
- [`../roster-analogos/_IDS-CARTAS.md`](../roster-analogos/_IDS-CARTAS.md): os 21 IDs de domínio (`DOM-NN-R_vM.m`) dos 20 mestres + Tusk, base do naming ESPECIAL/SUPER.
- [`../gdd.md`](../gdd.md) linha 145 (anti-pillar preservado: "cartas são obtidas, não craftadas").

---

## 1. Mapa-mestre: magia é tecnologia, cada peça tem sua tech

TechnoMagik é o guarda-chuva que cobre várias tecnologias distintas, não uma coisa só. Cada peça do sistema de cartas/itens/poções mapeia para uma tecnologia diegética própria:

| Peça | Tecnologia diegética | Onde roda |
|---|---|---|
| **Conjuro / carta** | script compilado, na linguagem C-Arcane | Tavus-Drive (o executor de cartões, Pillar 3) |
| **Poção** | nanobots, nanomedicina | corpo do alvo (Injetor administra, §3) |
| **Item** | hardware | triângulo Óculos / Matriz / Tavus-Drive + gadgets (§7) |

Isso reforça Pillar 2 (nada de "magia misteriosa"): cada categoria de recurso do jogador tem uma explicação técnica coerente e distinta das outras duas.

---

## 2. Taxonomia de cartas: 3 tiers

Três tiers de carta, com identidade mecânica e narrativa distinta. Nenhum tier se sobrepõe a outro.

### 2.1 Visão geral

| Tier | O que é | Como se obtém | Onde vive |
|---|---|---|---|
| **COMUM** | conjuro compilado de 3 Tokens (5 famílias + roda de fraqueza, combat.md §6; modelo carta-base + modificador, combat.md §7) | progresso narrativo (nunca craft, nunca compra; gdd.md linha 145 preservado) | os 15 slots em campo, junto com ESPECIAL |
| **ESPECIAL** | conjuro-assinatura lendário dos 20 mestres, pré-compilado e único (1 cópia no jogo inteiro) | entregue pelo Tavus-Eco do mestre, ao fim da missão dele | 1 dos 15 slots em campo |
| **SUPER** | "A Carta Perdida de Tusk", efeito Consórcio (passiva que potencia as 20 ESPECIAIS + ativa suprema) | forjada com as 20 ESPECIAIS na bancada do Tusk (missão-capstone) | slot dedicado, FORA dos 15 |

### 2.2 COMUM

- **Composição:** 3 Tokens compilados. Cada Token é uma das 5 famílias elementais (Elétrico / Bioquímico / Sônico / Cinético / Criptográfico, combat.md §6) combinada com o modelo carta-base + modificador (combat.md §7-8: Object / Stream / Null).
- **"Compilar" = anexar o modificador no cast.** A compilação acontece em runtime, no momento de jogar a carta (combat.md §7: "modificadores são anexados em runtime, não pré-bakeados na carta"). Não existe uma segunda etapa de craft fora disso: **a compilação-no-cast É o único "craft" que existe no jogo.** Isso é consistente com o anti-pillar do gdd.md ("crafting de cartas: cartas são obtidas, não craftadas"): o jogador nunca monta uma carta nova numa bancada; ele obtém a carta pronta e, ao jogá-la em combate, o Tavus-Drive resolve a compilação do modificador.
- **Obtenção:** exclusivamente por progresso narrativo (missão, beat de capítulo, recompensa de arco de companion). Nunca craftada, nunca comprada em loja. Reforça Pillar 2 (o deck é curado, não gerado) e o teto de 40-60 cartas totais do jogo (pillars.md, anti-pillar "não é Magic: The Gathering completo").
- **Naming:** `cardExec-[efeito em Sylvarin]` (ver §8).

### 2.3 ESPECIAL (as 20 cartas dos mestres)

- **O que é:** o conjuro-assinatura de cada um dos 20 mestres do roster de análogos (Faraday, Tesla, Euler, Gödel, Einstein, Turing, von Neumann, Mises etc., ver `_IDS-CARTAS.md`). Cada uma é **lendária e única**: existe exatamente 1 cópia no jogo inteiro.
- **Pré-compilada, travada:** ao contrário da COMUM, a ESPECIAL **não recebe modificador**. Ela já vem pronta, sem Object/Stream/Null anexável. Isso a diferencia mecanicamente da COMUM além da raridade narrativa: é uma peça fixa, não um sistema aberto a runtime.
- **Ocupa slot em campo:** consome 1 dos 15 slots do deck em campo (gdd.md §6.2: "deck de 40-60 cartas-token totais, 15 em campo"). Não é bônus fora do deck; o jogador escolhe se vale o slot.
- **Custo de Mana premium:** custa mais mana que uma COMUM equivalente (valor exato fica para balanceamento, ver §9 PENDENTE).
- **Uso limitado por batalha:** 1× por batalha. Reusa o mesmo flag mecânico da Análise Preditiva (combat.md §2.1: "não se acumula, não recarrega durante a batalha").
- **Relação com a roda de fraqueza (combat.md §6):**
  - **Regra geral:** a base da carta ESPECIAL fica DENTRO da roda de fraqueza (tem família, tem contra). O jogador ainda pode explorar fraqueza/resistência normalmente contra ela e com ela.
  - **Exceção (trunfo fora da roda):** cartas cuja assinatura NARRATIVA já quebra a regra do sistema (ex.: Gödel, cuja "Sentença Indecidível" ignora imunidade por design temático, um teorema de incompletude não se deixa resolver por classificação de família) ficam marcadas como **trunfo fora da roda**: não seguem `multFraqueza`, resolvem por regra própria. Esse comportamento vive como uma **flag no Card record** (ex.: `IgnoresWeaknessWheel: bool`), não como exceção hardcoded no resolvedor da roda.
- **Entrega:** o Tavus-Eco do mestre (ver `techmagic.md`, canon 2026-07-12) entrega a carta ao fim da missão, em pessoa (projeção compilada). Nunca é loot de baú, nunca é compra.
- **Naming:** `cardExec-[figura]`, nome real do mestre, NÃO traduzido (ex.: `cardExec-Tesla`, `cardExec-Einstein`, `cardExec-Gödel`).

### 2.4 SUPER (Tusk)

- **O que é:** "A Carta Perdida de Tusk" (`CAP-01-L`, `_IDS-CARTAS.md`), a carta-capstone do Codex. Efeito Consórcio: **passiva** que potencia as 20 cartas ESPECIAIS já obtidas, mais uma **ativa suprema** própria.
- **Forja, não Tavus-Eco:** ao contrário das 20 ESPECIAIS (entregues pelo eco do próprio mestre), a SUPER é **forjada** pela party na bancada do Tusk, usando as 20 ESPECIAIS como componente do ritual (ver `roster-analogos/21-helion-tusk.md`, "O clímax: ritual de forja da Carta 21"). Pré-requisito duro: as 20 já coletadas.
- **Slot dedicado, fora dos 15:** não compete pelo deck de campo. Existe numa posição própria, exclusiva, separada da pipeline normal de montagem de deck.
- **Naming:** herda o nome in-world já canônico, "A Carta Perdida de Tusk"; não segue o padrão `cardExec-` (é a única exceção; ver §8, nota).

### 2.5 Nota de reconciliação (raridade visual de frame vs tier de taxonomia)

O `card-frame-spec.md` e `_IDS-CARTAS.md` já usam um marcador de raridade visual no rodapé do frame (comum / rara / lendária), explicitamente **provisório** (pendente do item `CARTAS-BALANCEAMENTO`). Esse marcador é um **eixo independente** do tier COMUM/ESPECIAL/SUPER definido aqui: por exemplo, algumas das 20 cartas ESPECIAIS estão hoje marcadas `C` (comum) no frame provisório (Faraday, Volta, Ada Lovelace, Turing), mas são ESPECIAL nesta taxonomia (únicas, pré-compiladas, entregues por Tavus-Eco). A sincronização entre os dois eixos (o frame deveria refletir o tier real: COMUM→comum, ESPECIAL→rara/lendária, SUPER→lendária/prismático) é trabalho do item `CARTAS-BALANCEAMENTO` (INBOX), não decidido aqui.

---

## 3. Poções (nanobots): Injetor + Ampolas

### 3.1 Diegese

Poção = **nanobots**, nanomedicina. Um enxame programável entra no corpo (ou sistema) do alvo e executa uma rotina específica: religar um processo travado, recarregar um buffer de energia, neutralizar uma subrotina hostil, ou fazer overclock temporário. Anti-body-horror deliberado (Pillar 4: protagonista de 11 anos, tom nunca visceral): a forma visual e narrativa do enxame é benigna e limpa (ampola, brilho, dissolução suave), nunca inseto/víscera/invasão explícita.

### 3.2 Injetor (equipamento permanente, obrigatório)

- **O que é:** o hardware que administra as Ampolas. Sem Injetor equipado, Ampolas não podem ser usadas.
- **Permanente:** achado ou comprado; uma vez obtido, fica no stash.
- **Zero durability:** consistente com a regra dura de `economia.md` §7.5 ("hardware nunca quebra por uso normal"). O Injetor nunca desgasta, nunca precisa reparo por uso.
- **Como se perde:** só por dívida (economia.md §3.3.2, cura completa a crédito com plano de quitação), roubo (evento narrativo pontual) ou venda voluntária. Nunca por desgaste, nunca por falha aleatória.

### 3.3 Ampolas (consumíveis)

Nome canônico em pt-br: **"Ampola de X"**. Consumidas ao usar (diferente do Injetor, que é permanente). É o único tipo de consumível do jogo que efetivamente se gasta (reforça a regra dura de zero durability em equipamento: só a Ampola gasta).

| Ampola | Efeito mecânico | Diegese (por que funciona) |
|---|---|---|
| **Ampola de Religação** | cura (HP) | os nanobots RELIGAM o processo travado. HP é lido como integridade do sistema vivo, não ferida física: coerente com o framing "vencer = colapso de pilha, corpo intacto" já usado no combate (combat-flavor.md, BUILD FAILED / BUILD SUCCEEDED) |
| **Ampola de Recarga** | mana | os nanobots recarregam o buffer de energia do Tavus-Drive |
| **Ampola de Antídoto** | remove status hostil (dispel) | os nanobots matam a subrotina hostil, ou seja, o status em si (equivalente mecânico ao já-canônico P2 "Ampola de Antídoto", economia.md §7.2, que hoje dispela Poison/Corrode; este doc generaliza a diegese pra "qualquer subrotina hostil", cabendo os novos status do §5) |
| **Ampola de Overclock** | buff | os nanobots forçam o sistema a rodar acima do normal por um tempo limitado |

**Cross-ref economia.md:** a Bio-Ampola (§4) e a Life Ampola (§5) já canônicas em `economia.md` se encaixam nesta família como casos específicos de Ampola de Religação (a Bio-Ampola cura HP em campo; a Life Ampola reativa companion incapacitado, que é a forma mais forte de "religar o processo"). Este doc não redefine os números de `economia.md` §4-5, só situa a diegese das 4 famílias de Ampola dentro do TechnoMagik.

---

## 4. Inventário híbrido: Mochila × Força da party

Dois eixos distintos, propositalmente separados:

- **Mochila (espaços):** número de espaços de loot/consumível que a party consegue carregar. **Comprável**: Mochila maior = mais espaços.
- **Força da party (peso):** quanto peso total a party aguenta carregar, derivado do atributo Força somado/combinado da party ativa. O Bento "Requiem" (tanque) carrega mais; o Gus (frágil, Pillar 4) carrega pouco sozinho.

| Regra | Detalhe |
|---|---|
| Equipamento | fica em **slots próprios**, não conta peso nem espaço de Mochila |
| Cartas | **sem peso, sem espaço de Mochila** (são dados, não objetos físicos, coerente com Pillar 2: um conjuro compilado não é um item de inventário) |
| Loot bruto / consumível | é o ÚNICO tipo de item que pesa e ocupa espaço de Mochila |

Isso cria uma decisão tática dupla e não-redundante: **espaço** (quanto cabe, resolve comprando Mochila maior) é diferente de **peso** (quanto a party consegue levar, resolve com a composição da party em campo; trade-off tático real, Pillar 1: decisão interessante, não conveniência).

### 4.1 Zero durability (reforço)

Regra dura já canônica em `economia.md` §7.5 e reafirmada aqui: **equipamento é permanente, sem desgaste**. As Ampolas (§3.3) são o único tipo de consumível do sistema inteiro que efetivamente se gasta com o uso. Nenhum item de hardware (Injetor incluso) tem barra de durabilidade, taxa de manutenção por uso normal, ou degradação passiva. Consertos existem só como consequência narrativa pontual e scriptada (economia.md §7.5), nunca como sistema de desgaste.

### 4.2 Força e pesos (fechado, decisão do criador 2026-07-12)

**Força é atributo de logística, não stat de combate.** Não entra na fórmula de dano (combat.md §11), não escala Atk/Def/SPD: governa só quanto a party consegue carregar. Cada personagem tem um valor fixo de Força:

| Personagem | Força |
|---|---|
| Gus | 13 |
| Companion comum (Cauã, Iara, Linda, Dante, Jaci) | 21 |
| Bento "Requiem" | 34 |

A **capacidade de carga da party** é a soma da Força dos 3 personagens ativos em campo (§2.1 combat.md: party = Gus + 2 companions). Exemplos:

- Gus + 2 companions comuns: 13 + 21 + 21 = **55**.
- Gus + Bento + 1 companion comum: 13 + 34 + 21 = **68**.
- Levar o Bento na party ativa aumenta diretamente o quanto a party consegue saquear: trade-off tático real (Pillar 1), não conveniência de menu.

**Peso por item** (o que conta contra a capacidade de carga, §4 acima já isola loot/consumível como o único tipo que pesa):

| Item | Peso |
|---|---|
| Carta | 0 |
| Equipamento equipado | 0 |
| Ampola (qualquer das 4, §3.3) | 1 |
| Ingrediente comum | 1 |
| Ingrediente raro | 2 |
| Componente-boss / épico | 3 |
| Injetor de reserva (não equipado) | 5 |

**Estouro de peso = hard-stop.** Ao atingir a capacidade de carga, a party simplesmente não pega mais loot até soltar, usar ou vender algo. Sem penalty de stat, sem degradação: é um limite de logística, não uma punição de combate (consistente com a regra dura de zero durability, §4.1, e com "sem hardware inútil" dos anti-pillars).

---

## 5. Status framework: 2 novos status (camada 1, anti-body-horror)

O set canônico de `combat.md` §9 (Stun / Poison / Corrode / Disrupt / Silence / Knockback / Break / Expose / Decrypt / Shield / Regen / Haste / Slow) **fica intacto**, sem alteração. Dois status novos entram, ambos ligados à metáfora "magia é tecnologia" (Pillar 2, camada 1 da metafísica de `techmagic.md`), evitando body-horror.

### 5.1 Sobrecarga térmica ("fogo")

- **Família origem:** Elétrico. É o **2º status do Elétrico** (o 1º continua sendo Stun, combat.md §6/§9).
- **Efeito:** DoT front-loaded (dano concentrado nos primeiros ticks, decaindo) **mais** throttle leve enquanto o alvo está superaquecido: redução pequena de SPD e/ou Power durante a duração do status.
- **Diegese:** o sistema do alvo está rodando quente demais; ele perde desempenho (throttle) enquanto dissipa o calor, igual um processador real sob carga.
- **Forma de resolução (segue o padrão do combat.md §9):** magnitude e duração vêm sempre da carta/combo que aplica, nunca hardcoded. DoT front-loaded = magnitude maior nos primeiros ticks, decrescendo (forma exata fica para balanceamento, §9 PENDENTE).

### 5.2 Resfriamento ("gelo/frio")

- **Categoria:** BUFF utilitário (não é debuff, distinto do padrão "status ruim" que domina a lista de combat.md §9).
- **Efeito:** `+SPD` **mais** a 1ª carta jogada no turno custa menos mana (efetivamente um overclock defensivo: "computador funciona melhor no frio").
- **Diegese:** reduzir a temperatura do sistema aumenta a margem de operação seguro, permitindo rodar acima do normal por um turno: o inverso lógico da Sobrecarga térmica (§5.1), lá o sistema sofre por rodar quente, aqui o sistema ganha por rodar frio.

### 5.3 Mapa família → status (atualizado)

| Família | Status existentes (combat.md §9) | Status novo |
|---|---|---|
| Elétrico | Stun | **Sobrecarga térmica** (2º status do Elétrico) |
| Bioquímico | Poison / Corrode | (nenhum novo; Poison Bioquímico já é o "veneno" canônico) |
| Sônico | Disrupt, Silence | (nenhum novo) |
| Cinético | Knockback, Break | (nenhum novo) |
| Criptográfico | Expose, Decrypt | (nenhum novo) |
| Utilitário (qualquer família via carta própria) | Shield, Regen, Haste, Slow | **Resfriamento** (utilitário, buff) |

### 5.4 Nota de esclarecimento: "veneno"

O termo "veneno" não introduz um status novo: é o nome coloquial já usado para o **Poison** existente (Bioquímico, combat.md §9). Este doc não cria um status de veneno adicional.

---

## 6. Dano: fórmula não muda

A fórmula de dano canônica (combat.md §11, cadeia divisiva UseCard + sorteio de canal FALHA/CRIT/COMUM) **não é alterada** por este documento. Os 2 status novos (§5) plugam nela pelos mecanismos já existentes:

- **Sobrecarga térmica:** o componente DoT resolve como tick de status padrão (combat.md §9, "tick processado no TurnStart do ator afetado"), fora da cadeia divisiva de UseCard. O componente de throttle mexe em `SPD` (recomputa fila, mesmo mecanismo de Haste/Slow, combat.md §9) e/ou em `Power` (entra como fator na cadeia divisiva da próxima carta ofensiva do alvo afetado, análogo ao Disrupt).
- **Resfriamento:** o `+SPD` usa o mesmo mecanismo aditivo de Haste (combat.md §9: `SPD ±= Magnitude`, recomputa fila). O desconto de mana na 1ª carta do turno é um efeito novo de categoria (redução de `ManaCost`), não uma alteração da fórmula de dano em si.

Nenhum fator novo entra em `multFraqueza`, `multMod`, `multCombo`, `multExpose` ou `multAmbiente` (combat.md §11) por causa deste documento.

---

## 7. Itens = hardware (visão geral)

Item é hardware, mapeado no triângulo do Pillar 3 (Óculos Táticos / Matriz Ortodôntica / Tavus-Drive) mais gadgets acessórios que não são vértice do triângulo, mas ferramentas de campo.

| Categoria | Exemplos de forma (conceitual) | Vértice / natureza |
|---|---|---|
| Upgrade de triângulo | módulos que plugam em Óculos, Matriz ou Tavus-Drive | um dos 3 vértices canônicos (Pillar 3) |
| Gadget acessório | drones, torres, escudos | NÃO é vértice novo (Pillar 3 é fechado em 3); gadget é ferramenta de campo separada, equipável em slot próprio |

Preços e pesos do Injetor + Ampolas seguem em aberto: ver §9 PENDENTE. Gadgets concretos e a contagem de slots (deste §7 e do resto do inventário) fecham abaixo (§7.1, §7.2).

### 7.1 Gadgets (5 fechados, decisão do criador 2026-07-12)

Todos os gadgets custam **1 AP**, são **1x por batalha** (mesmo flag mecânico da Análise Preditiva e das cartas ESPECIAL, combat.md §2.1: "não se acumula, não recarrega durante a batalha") e **reusam mecanismos já canônicos de combat.md**, sem inventar sistema novo:

| Gadget | AP | Uso | Efeito | Mecanismo reusado |
|---|---|---|---|---|
| **Drone-Vigia** | 1 | 1x/batalha | orbita por 3 turnos, revelando a intenção do inimigo a cada turno | `IntentPreview` (combat.md §12/§13) + modificador Object (combat.md §7-8) |
| **Torre-Autômata** | 1 | 1x/batalha | planta uma torre que ataca sozinha 1x/turno, por 3 turnos | modificador Object (combat.md §7-8) |
| **Escudo de Emergência** | 1 | 1x/batalha | aplica Shield instantâneo em si ou num aliado ao lado | status Shield (combat.md §9) |
| **Baliza de Redirecionamento** | 1 | 1x/batalha | joga 1 inimigo pra trás na fila de iniciativa, sem gastar o Gambito | `ReorderActor` (combat.md §4/§12), sem consumir a ação de Gambito-Reordenar |
| **Kit-Morenh** | 1 | 1x/batalha | aplica Resfriamento (§5.2 deste doc) num aliado | status novo Resfriamento (§5.2); nome herda a palavra Sylvarin `Morenh` já cunhada em §8.2 |

Gadgets são **universais**: qualquer um dos 5 pode ser equipado por qualquer personagem. Não há gadget exclusivo de companion.

### 7.2 Slots (fechado, decisão do criador 2026-07-12)

Mapa completo de slots do jogador, consolidando cartas + implantes + gadgets + consumíveis num único lugar:

| Categoria | Regra de slot |
|---|---|
| Deck / Codex | 40-60 Tokens no deck total, **15 em campo** (canon já existente, gdd.md §6.2 / §2.2 deste doc) |
| Implantes do triângulo (Gus) | **1 por vértice, 3 total.** Gus escolhe 1 candidato de 3-4 por vértice (Óculos / Matriz / Tavus-Drive, Pillar 3) |
| Companions | **1 slot de hardware-análogo cada.** Peça própria, distinta do triângulo do Gus: assimetria proposital, o triângulo (Pillar 3) é regra exclusiva do Gus, companions não herdam os 3 vértices |
| Gadget | **1 por personagem ativo em campo** = 3 gadgets equipados simultaneamente na party (§7.1) |
| Injetor | **1** (equipado ou não; ver §3.2) |
| Carta ESPECIAL | ocupa 1 dos 15 slots em campo do Codex (já fechado em §2.3, reforçado aqui) |
| Carta SUPER | **slot dedicado, FORA dos 15** (já fechado em §2.4, reforçado aqui) |
| Ampolas em combate | **sem slot separado**: qualquer Ampola carregada na Mochila (§4) é usável em combate; o limite real é o peso da Mochila, não um slot de "quickbar" à parte |

---

## 8. Naming: prefixo `cardExec-` + vocabulário Sylvarin

### 8.1 Padrão de nome

Prefixo **`cardExec-`** (camelCase, `card` + `exec`, reforça a metáfora "conjuro = script executável", Pillar 2).

| Tier | Padrão de nome | Exemplo |
|---|---|---|
| COMUM | `cardExec-[efeito em Sylvarin]` | `cardExec-Tavusa` (carta Elétrico) |
| ESPECIAL | `cardExec-[figura]`, nome real do mestre, NÃO traduzido | `cardExec-Tesla`, `cardExec-Einstein`, `cardExec-Gödel` |
| SUPER | herda o nome in-world já canônico, sem prefixo `cardExec-` | "A Carta Perdida de Tusk" |

### 8.2 Léxico Sylvarin dos efeitos comuns

Cada efeito de carta COMUM (as 5 famílias + os conceitos utilitários relevantes) recebe uma palavra Sylvarin (raiz + sufixo, conforme `docs/narrative/lingua/02-lexico-semente.md`), usada no `Id` interno (`cardExec-[palavra]`):

| Efeito | Palavra Sylvarin | Raiz-base (lexico-semente) |
|---|---|---|
| Elétrico | **Tavusa** | `tavus-` (o pulso que executa) |
| Fogo / Sobrecarga térmica | **Vyrin** | `vyr-` (fogo draconico, poder sagrado) + `-in` (adjetivo) |
| Frio / Resfriamento | **Morenh** | variante de `mor-` (sombra/escuro), nota abaixo |
| Bioquímico / veneno | **Erynin** | `eryn-` (mata, crescimento) + `-in` (adjetivo) |
| Sônico | **Lhinin** | `lhin-` (canto, fala, som) + `-in` (adjetivo) |
| Cinético | **Ondha** | `ondh-` (pedra, construção, alicerce) + `-a` (substantivo concreto) |
| Criptográfico | **Rimin** | `rime-` (número, ordem, cômputo) + `-in` (adjetivo) |
| Luz / Scan | **Cala** | `cale-` (luz, claridade) |
| Tempo | **Anhin** | `anh-` (tempo, ciclo lunar) + `-in` (adjetivo) |
| Escudo | **Ondhesse** | `ondh-` (pedra, alicerce) + `-esse` (abstrato/coletivo) |
| Cura / Regen | **Sylvesse** | `sylva-` (a mata viva) + `-esse` (abstrato/coletivo) |

> **Nota sobre "Morenh":** a raiz-semente `mor-` (lexico-semente #7) é "sombra, escuro", não "frio" literal. O criador definiu "Morenh" diretamente como a palavra para Resfriamento/frio; a reconciliação formal (se `mor-` ganha um sentido secundário de "frio" via associação sombra↔frio, ou se "Morenh" na verdade deriva de raiz nova a formalizar) é trabalho do léxico Sylvarin, fora do escopo deste doc, sinalizado aqui só para não perder o fio. Não bloqueia o uso do nome `cardExec-Morenh` nas cartas.

O léxico Sylvarin completo (regras de derivação, tabela de mutação consonantal etc.) é mantido em `docs/narrative/lingua/`; este documento só consome as palavras já fornecidas pelo criador, não abre novo trabalho de linguística.

---

## 9. PENDENTE (diferido para ondas de balanceamento)

**Fechado na rodada 2026-07-12 (rodada 2), não é mais brainstorm aberto:** gadgets concretos (§7.1, os 5 estão nomeados e especificados), contagem de slots (§7.2, mapa completo), Força e peso por item (§4.2, tabela fechada). O que resta abaixo não é esquecido nem em aberto sem dono: é **diferido de propósito** para as ondas dedicadas de números finos, listadas explicitamente por item.

- **Preços em crédito (diferido para a onda de ECONOMIA, `economia.md`):**
  - Preço do Injetor (achado vs comprado: os dois caminhos existem, valor de compra não definido).
  - Preços das 4 Ampolas (Religação / Recarga / Antídoto / Overclock): hoje só a Bio-Ampola e a Life Ampola têm número em `economia.md` §4-5; Recarga e Overclock ainda não têm valor.
- **Custo de Mana premium exato das cartas ESPECIAIS** (§2.3): "premium" está qualificado, não quantificado. Diferido para `CARTAS-BALANCEAMENTO` / playtest.
- **Magnitude/duração numérica** da Sobrecarga térmica e do Resfriamento (§5): a forma (DoT front-loaded + throttle; +SPD + desconto de mana) está fechada, os números não. Diferido para `CARTAS-BALANCEAMENTO` / playtest.
- **Sincronização do marcador de raridade visual do frame** (comum/rara/lendária) com os 3 tiers desta taxonomia (§2.5): depende de `CARTAS-BALANCEAMENTO`.
- **Reconciliação da grafia "techMagic" → "TechnoMagik"** dentro de `techmagic.md` (já sinalizada naquele doc, item `TECHMAGIC-CANON`).
- **Reconciliação da raiz Sylvarin de "Morenh"** (§8.2, nota) com o léxico-semente formal.

---

**Última revisão:** 2026-07-12, rodada 2 (gadgets, slots, Força/peso fechados; preços e magnitudes diferidos para ECONOMIA e CARTAS-BALANCEAMENTO/playtest; aguarda revisão).
