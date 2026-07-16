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
- **Baseline numérica (framework fechado pelo criador 2026-07-12, via economy-designer; ancorada em Trash 55 HP / Def 8, Atk médio de companion ~10, fórmula combat.md §11, TTK-alvo trash 3-5 turnos):**

  | Arquétipo | ManaCost | Power / Magnitude | Nota |
  |---|---|---|---|
  | Dano single-target barato | 1 | Power **3** | ~5 hits neutros pra matar Trash (teto do range) |
  | Dano single-target médio | 2 | Power **5** | ~4 hits neutros |
  | Dano single-target forte | 3 | Power **8** | 4 neutros / **3 explorando fraqueza** (chão do range) |
  | DoT (Bioquímico, achatado) | 2 | Magnitude **5**/tick × 3 = 15 total | comparável ao médio, pago em paciência |
  | Control (Stun/Disrupt/Break/Knockback) | 2-3 | Stun=pula turno; Disrupt=-30/50% Power; Break/Corrode=-3 Def | 0 dano direto, trade tático |
  | Buff/Utilitário (Shield/Regen/Haste) | 1-2 | Shield=Def do lançador; Regen=**3**/turno; Haste=+1 SPD | reusa mecanismo existente |

  O range TTK 3-5 emerge sozinho variando tier × fraqueza (nada forçado). Os Powers 3/5/8 caem na sequência de Fibonacci (mesmo easter egg velado do HP 34/55/89/144). Modificadores continuam somando por cima (Object +1 / Stream +2 / Null +1, combat.md §8). **FRAMEWORK:** as 3 faixas de ManaCost + a derivação de Power a partir do HP-Trash. **AJUSTE FINO (playtest N=3):** os valores 3/5/8 podem mexer ±1-2; a proporção entre tiers é o que se mantém.

### 2.3 ESPECIAL (as 20 cartas dos mestres)

- **O que é:** o conjuro-assinatura de cada um dos 20 mestres do roster de análogos (Faraday, Tesla, Euler, Gödel, Einstein, Turing, von Neumann, Mises etc., ver `_IDS-CARTAS.md`). Cada uma é **lendária e única**: existe exatamente 1 cópia no jogo inteiro.
- **Pré-compilada, travada:** ao contrário da COMUM, a ESPECIAL **não recebe modificador**. Ela já vem pronta, sem Object/Stream/Null anexável. Isso a diferencia mecanicamente da COMUM além da raridade narrativa: é uma peça fixa, não um sistema aberto a runtime.
- **Ocupa slot em campo:** consome 1 dos 15 slots do deck em campo (gdd.md §6.2: "deck de 40-60 cartas-token totais, 15 em campo"). Não é bônus fora do deck; o jogador escolhe se vale o slot. **EXCETO as sub-categoria FORA-DE-COMBATE (§2.3, Euler/Menger), que NÃO ocupam slot de deck de combate.** (Faraday saiu desta lista em 2026-07-15 — virou Híbrida, ver nota abaixo.)
- **Custo de Mana premium (arcabouço fechado pelo criador 2026-07-12, via economy-designer):** **`ManaCost` fixo ≈ 6** (faixa aceitável 5-7; número exato dentro da faixa fica pra playtest). Racional: `manaMax = 2 + turnoIndex` (cap 8), então 6 destrava por volta da **rodada 4**, gastando quase todo o pool no turno (tensão real: jogar a especial = não sobra mana), alcançável em elite/mini-boss/boss onde ela deve brilhar, sem virar troféu que nunca dispara (o risco do custo-cap 8). Anti-abuso já coberto por 3 travas independentes: 1×/batalha + sem modificador + mana sem carry-over (impossível bankar pra abrir com ela no turno 1). Trunfos-fora-da-roda (Gödel etc.) NÃO recebem taxa de mana extra por padrão; só reavaliar (+1) se o playtest mostrar que a versão fora-da-roda supera sistematicamente a de-dentro contra Fraco.
- **Quatro sub-categorias (economy-designer + criador 2026-07-12; 4ª adicionada 2026-07-14 por PS-Y1):** ao statear as 20, elas se dividem, e o custo depende disso:
  - **ATIVA (paga o mana premium ~6, 1×/batalha):** as que o jogador JOGA num turno (burst, invocação, control forte). Ocupam 1 dos 15 slots do deck de combate.
  - **PASSIVA (ManaCost 0, equip-only):** sempre ligadas enquanto ocupam 1 slot do Codex; o "custo" delas é o SLOT ocupado, não mana. NÃO competem com o "1×/batalha". Fiel ao fenômeno real (a Gaiola de Faraday é proteção estrutural, não uma ação disparada). Ex.: Ada, Pythagoras, Hayek, Mises, Bastiat, Planck, Turing (lista atualizada 2026-07-14, achado PS-G3; ver `_EFEITOS-ESCOLHIDOS.md` pra a classificação final das 20). **Gödel saiu desta lista em 2026-07-15** — virou ATIVA (ManaCost 0, mas castável, não equip-only), ver nota abaixo.
  - **FORA-DE-COMBATE (utilitário de exploração/economia, NÃO ocupam slot de deck de combate):** guardadas à parte, agem no overworld/dungeon. **Euler** (revela o grafo/mapa da sala), **Menger** (revela valor real de loot + converte item em Crédito, 1×/encontro). Uma categoria leve, sem penalizar o deck de batalha. (**Faraday** tinha a face anti-PEM aqui até 2026-07-14; essa face segue existindo — habilita save onde há PEM, cross-ref `project_save_dungeon_pem_faraday` + FARADAY-DUNGEON-ITENS, ainda sem programa/wiring — mas a carta em si migrou pra Híbrida, ver linha abaixo.)
  - **HÍBRIDA (decisão do criador 2026-07-14, achado PS-Y1):** cartas com um efeito PASSIVO sempre-ligado + um efeito ATIVO no MESMO card. Ocupam **1 slot** do deck de combate. O **passivo fica sempre ligado** enquanto equipada (NÃO sofre o limite de 1×/batalha); a **ativa** paga o mana premium ~6 e obedece o **1×/batalha** normal. Modela as cartas de efeito duplo: **Maxwell, Newton, von Neumann, John Dee** (ver `_EFEITOS-ESCOLHIDOS.md`). No `Card` record isso é um flag/composição (passivo + ativo coexistem), não uma carta a mais.
    - **Faraday (ADR-016 Balde B, decisão do líder 2026-07-15, F-3) entrou nesta categoria com uma assimetria a registrar:** ao contrário das 4 acima (passivo DE COMBATE sempre-ligado + ativo), a Faraday tem só a face ATIVA implementada em combate (`OnCast` → BlindagemEM, §5.5); a face "sempre-ligada" dela é a fora-de-combate ORIGINAL (anti-PEM, overworld/dungeon), que não é um passivo de combate no sentido do `TriggerHook::Always` das outras 4 — é um contexto totalmente diferente (exploração, não turno). `CardCategory::Hibrida` aqui sinaliza "carta com duas faces mecânicas" de forma mais ampla que a definição original (dupla-face-EM-COMBATE); não é um erro de categorização, é a categoria se provando reusável pra um caso vizinho.
  - **Gödel (ADR-016 Balde B, decisão do líder 2026-07-15, PR3, G-1) saiu de PASSIVA e virou ATIVA:** o trunfo "Sentença Indecidível" deixou de ser só a flag fixa `IgnoresWeaknessWheel` (§2.3 abaixo) e ganhou um segundo componente castável — mana **0** (não paga o premium ~6 das outras Ativas), `OnCast` concede um status novo (`NullProof`, §9) ao PORTADOR (a própria Gödel ou um aliado). 1×/batalha, mesmo gate das Ativas. A flag original continua intacta (item 11 genérico, ver §9); o status é o trunfo NOVO, guardado até o próximo hit relevante.
- **Uso limitado por batalha:** 1× por batalha (vale pras ATIVAS). Reusa o mesmo flag mecânico da Análise Preditiva (combat.md §2.1: "não se acumula, não recarrega durante a batalha").
- **Relação com a roda de fraqueza (combat.md §6):**
  - **Regra geral:** a base da carta ESPECIAL fica DENTRO da roda de fraqueza (tem família, tem contra). O jogador ainda pode explorar fraqueza/resistência normalmente contra ela e com ela.
  - **Família das não-elementais (decisão do criador 2026-07-14, achado PS-R1):** só as ~7 cartas do domínio eletromagnético (Faraday/Maxwell/Tesla/Volta/Euler...) mapeiam limpo em Elétrico; as ~13 restantes (matemáticos/computação/economistas/ocultistas) recebem a família **`Universal`** (valor novo do enum `CardFamily`, combat.md §17): FORA da roda de fraqueza, `multFraqueza` sempre 1.0, sem Fraco/Resistente/Imune. Resolve o campo obrigatório `Card.Family` que antes não tinha valor pra elas.
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
- **Efeito:** DoT front-loaded (dano concentrado nos primeiros ticks, decaindo) **mais** throttle leve de **SPD apenas** (Slow) enquanto o alvo está superaquecido.
- **Diegese:** o sistema do alvo está rodando quente demais; ele fica mais LENTO (throttle) enquanto dissipa o calor, igual um processador real sob carga.
- **Arcabouço fechado (criador 2026-07-12, via economy-designer):**
  - **Throttle = SÓ SPD** (reusa 1:1 o mecanismo Slow: `SPD ±= Magnitude`, aditivo, clamp 0, recomputa fila). **NÃO mexe em Power** — reduzir Power já é o papel canônico do Disrupt (Sônico); dar isso ao Elétrico furaria a regra "sem sobreposição de papéis" (combat.md §6). Slow sugerido -1 SPD.
  - **Forma do DoT = 3 turnos, front-load 50/30/20** (do total da magnitude: metade no 1º tick, depois decai). Dá ritmo de leitura (o Scan mostra quanto falta) e fica distinto do Poison (achatado/sustentado), reforçando o burst do Elétrico.
  - **StackRule = Refresh** (reaplicar renova a Duration e a curva, NÃO empilha magnitude nem soma um 2º DoT concorrente — trava anti-degeneração, combat.md §15).
  - **Magnitude (ancorada na baseline COMUM, §2.2, Power-médio 5):** DoT total **~15** (equivale a uma carta média espalhada em 3 turnos), front-load 50/30/20 = **8 / 5 / 2**. FRAMEWORK; o valor exato segue AJUSTE FINO no playtest (pode mexer ±1-2 junto com a baseline).

### 5.2 Resfriamento ("gelo/frio")

- **Categoria:** BUFF utilitário (não é debuff, distinto do padrão "status ruim" que domina a lista de combat.md §9).
- **Efeito:** `+SPD` **mais** a 1ª carta jogada no turno custa menos mana (efetivamente um overclock defensivo: "computador funciona melhor no frio").
- **Diegese:** reduzir a temperatura do sistema aumenta a margem de operação seguro, permitindo rodar acima do normal por um turno: o inverso lógico da Sobrecarga térmica (§5.1), lá o sistema sofre por rodar quente, aqui o sistema ganha por rodar frio.
- **Arcabouço fechado (criador 2026-07-12, via economy-designer):**
  - **`+1 SPD`** (reusa o mecanismo aditivo de Haste; +1 é a granularidade que o jogo já usa pra SPD em ambiente, combat.md §18 T8).
  - **Desconto de mana: -1 na 1ª carta do turno**, com **clamp de custo mínimo 1** (uma carta nunca fica a 0 mana).
  - **Duration = 3 turnos** (mesma janela dos gadgets Drone-Vigia/Torre-Autômata, §7.1).
  - **StackRule = Refresh** (recastar renova Duration, não empilha SPD nem desconto).
  - **Trava anti-loop (framework, combat.md §15):** `ManaCost(carta de Resfriamento) ≥ desconto × Duration`, ou seja, o card custa no mínimo o que economiza ao longo da própria vida (senão recastar em loop viraria lucro líquido de mana). Com -1 × 3 = 3, **o card custa 4** (não o breakeven exato 3: +1 garante que reforjar em loop nunca é neutro nem lucro). Mana sem carry-over já impede bankar.
  - **Nota de consistência (não bloqueia):** o terreno Gelo (combat.md §18) hoje PENALIZA +1 mana na 1ª carta; o status Resfriamento PREMIA -1. Polaridades opostas do mesmo elemento, defensável (ambiente frio hostil emperra vs resfriamento ATIVO do próprio sistema turbina). Registrado, não é conflito.
  - **Kit-Morenh** (gadget, §7.1) aplica Resfriamento de graça 1×/batalha: aceitável, já gated pelo próprio limite do gadget.
  - **Ajuste fino (playtest):** se +1 SPD / -1 mana é sentido como impactante o bastante, e o `ManaCost` exato do card (3 vs 4).

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

### 5.5 BlindagemEM (EM-Shield, carta Faraday — ADR-016 Balde B, decisão do líder 2026-07-15)

- **Categoria:** BUFF de imunidade (não é debuff; entra na mesma classe utilitária de Shield/Regen/Haste/Resfriamento, §5.3).
- **Família origem:** Elétrico (é o **3º status do Elétrico**, junto de Stun e Sobrecarga térmica).
- **Efeito:** enquanto ativa, bloqueia a aplicação de qualquer **debuff** de `family_origin` Elétrico no portador (ex.: Stun elétrico, Sobrecarga térmica). Buffs elétricos (ex.: um Shield concedido por carta Elétrica) e debuffs de qualquer OUTRA família passam direto — o portão só trava a combinação exata "debuff + Elétrico" (decisão F-1, `_EFEITOS-ESCOLHIDOS.md` AMB-03).
- **"Previne + limpa" (F-2):** ao ser conjurada, além de prevenir os próximos, remove imediatamente os debuffs elétricos JÁ ativos no alvo (ex.: uma Sobrecarga térmica em andamento é limpa no cast).
- **Duração:** 3 turnos do portador, `StackRule = Refresh` (recastar renova a duração, não empilha).
- **Carta-fonte:** Faraday (§8, lendária, 1 cópia). A carta passou de ForaDeCombate (posse-only) para **Híbrida** (§2.3): ganhou uma face de combate castável (`OnCast` aplica BlindagemEM, mana ~6, alvo só ALIADO/self — cast em inimigo dissipa, F-4) além da face fora-de-combate original (anti-PEM, posse-only), que segue como feat futura sem programa.
- **Diegese:** a gaiola de Faraday, além de anular pulsos eletromagnéticos externos (a face fora-de-combate), agora também pode ser "vestida" num aliado em combate — um escudo estrutural contra descarga elétrica hostil, coerente com o fenômeno real (proteção passiva, não uma arma).

### 5.6 Mapa família → status (atualizado, pós-BlindagemEM)

| Família | Status existentes | Status novo |
|---|---|---|
| Elétrico | Stun, Sobrecarga térmica | **BlindagemEM** (3º status do Elétrico, buff de imunidade) |

---

## 6. Dano: fórmula não muda

A fórmula de dano canônica (combat.md §11, cadeia divisiva UseCard + sorteio de canal FALHA/CRIT/COMUM) **não é alterada** por este documento. Os 2 status novos (§5) plugam nela pelos mecanismos já existentes:

- **Sobrecarga térmica:** o componente DoT resolve como tick de status padrão (combat.md §9, "tick processado no TurnStart do ator afetado"), fora da cadeia divisiva de UseCard. O componente de throttle mexe em `SPD` (recomputa fila, mesmo mecanismo de Haste/Slow, combat.md §9) e/ou em `Power` (entra como fator na cadeia divisiva da próxima carta ofensiva do alvo afetado, análogo ao Disrupt).
- **Resfriamento:** o `+SPD` usa o mesmo mecanismo aditivo de Haste (combat.md §9: `SPD ±= Magnitude`, recomputa fila). O desconto de mana na 1ª carta do turno é um efeito novo de categoria (redução de `ManaCost`), não uma alteração da fórmula de dano em si.

Nenhum fator novo entra em `multFraqueza`, `multMod`, `multCombo`, `multExpose` ou `multAmbiente` (combat.md §11) por causa deste documento — **exceção pontual:** o status `NullProof` (Gödel/Null-Proof, §9, ADR-016 Balde B PR3) INTERCEPTA `multFraqueza` no início da cadeia, ANTES do curto-circuito de imunidade: se o atacante porta o status e o multiplicador calculado é `< 1.0` (Resistente OU Imune), o resolvedor força `multFraqueza = 1.0` e consome o status. Não é um fator multiplicativo a mais na cadeia — é uma reescrita condicional do próprio `multFraqueza` antes dele entrar na fórmula, mesma família mecânica do trunfo `IgnoresWeaknessWheel` já existente (que também reescreve `multFraqueza` para 1.0, incondicionalmente).

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
| Carta ESPECIAL (ATIVA ou PASSIVA) | ocupa 1 dos 15 slots em campo do Codex. **EXCEÇÃO:** as ESPECIAIS FORA-DE-COMBATE (Faraday/Euler/Menger) **NÃO ocupam slot de deck de combate** (§2.3, categoria leve, guardadas à parte). Correção 2026-07-14 (achado PS-R2) |
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

#### 8.1.1 Três camadas de nome por carta — regra de convivência (decisão do criador 2026-07-14, achado PS-Y13)

Cada carta ESPECIAL tem 3 nomes; a regra de qual serve pra quê:

| Camada | Papel | i18n? | Exemplo |
|---|---|---|---|
| **Título** | Nome descritivo que o jogador lê na prosa dos mestres e na UI | **SIM — chave `tr()` desde o começo** (PT agora, traduzido pós-1.0, coerente com o i18n canônico) | "Dilatação Temporal" |
| **Alcunha EN** | Codinome techno curto, mostrado como flavor de combate | **NÃO se traduz** — fica fixa em inglês em todo locale (tech é inglês universalmente; trata-se como nome próprio de produto/tech) | "Time-Dilate" |
| **ID de código** | Identificador interno | **irrelevante à língua** (é só código) | `cardExec-einstein` |

- O **título** é a única camada traduzível; a **alcunha EN** é constante entre idiomas (não vira chave de tradução); o **ID** nunca aparece ao jogador.
- Consequência pros 20 diálogos de encontro (Tavus-Eco/Morlhin): o mestre nomeia o **título** (traduzível) e pode cravar a **alcunha EN** como o "nome de guerra" da carta; o `narrative-writer` segue esse molde pra as 20 cenas não divergirem.

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
- **Custo de Mana premium das cartas ESPECIAIS** (§2.3): **ARCABOUÇO FECHADO 2026-07-12** (via economy-designer) = `ManaCost` fixo ≈ 6 (faixa 5-7, destrava rodada 4). Só o número exato dentro da faixa + a taxa dos trunfos-fora-da-roda ficam pra playtest.
- **Magnitude/duração da Sobrecarga térmica e do Resfriamento** (§5): **FECHADO 2026-07-12** (§5.1: throttle SÓ SPD, DoT 3 turnos 50/30/20 = **8/5/2** total ~15; §5.2: +1 SPD, -1 mana 1ª carta clamp min 1, Duration 3, **card custa 4**, trava anti-loop). Números destravados pela baseline COMUM (§2.2); AJUSTE FINO ±1-2 no playtest.
- **Baseline numérica das cartas COMUNS** (§2.2): **FECHADA 2026-07-12** (via economy-designer) = ManaCost 1/2/3 → Power 3/5/8 + arquétipos DoT/control/buff, ancorada em Trash 55 HP. É a régua que destrava tudo. AJUSTE FINO no playtest.
- **Statlines das 20 cartas ESPECIAIS:** o ARCABOUÇO está fechado (§2.3: 3 sub-categorias — ativa mana~6 / passiva mana-0 equip-only / fora-de-combate Faraday+Euler+Menger). Mas o **statline final por-carta DEPENDE de escolher o efeito A/B/C de cada mestre** (hoje os docs `roster-analogos/01..20` só RECOMENDAM uma opção; não é decisão do criador ainda). Logo o statline por-carta é **DIFERIDO até a feat de escolher os 20 efeitos** (feat do roster + criador). Existe uma tabela-rascunho provisória do economy-designer (2026-07-12, assumindo as opções recomendadas) como ponto de partida, NÃO canônica.
  - **Volta (regra de recurso) — RESOLVIDO** (efeito escolhido em `_EFEITOS-ESCOLHIDOS.md`, 2026-07-14 achado PS-Y4): o Volta virou **leech termodinâmico** (dreno que absorve energia do ALVO e devolve à party como mana+HP, só x% do absoluto drenado, o resto "perdido como calor", 2ª lei). NÃO fura o "sem carry-over" (é dreno do inimigo, não retenção de mana própria). O exemplo antigo "mana não-gasta vira Shield" está DESCARTADO. Falta só o x% (brainstorm VOLTA-LEECH-%).
  - **Gap Bioquímico:** nenhuma das 20 cai na família da Jaci (os mestres skewam pra info/lógica); aceito como honesto (a Jaci é servida pelo kit próprio + comuns DoT). Reavaliar só se o playtest pedir.
- **Dados faltantes (bloqueiam número fino, não a forma):** Atk/SPD do Elite (Daemon-Guard) e demais inimigos (TBD combat.md §17); tag de inimigo "comando central" (efeito do Mises, trabalho de `gameplay_engineer`).
  - **Como a "quantização" do Planck pluga na variância `v` de §11 — RESOLVIDO** (ADR-016 addendum "manifesto item 5", decisões do líder 2026-07-15, A1/A2/A5/A6): 3 degraus da PRÓPRIA faixa de `v` (piso/centro/teto), chances fixas 25%/50%/25%, zero mudança de balance (média = base). Ver `combat.md` §11 ("Quantum-Lock (Planck)") e `_EFEITOS-ESCOLHIDOS.md` (AMB-06).
- **Primitivas de engine que os efeitos escolhidos EXIGEM (achado PS-Y2, decisões do criador 2026-07-14):** os efeitos de `_EFEITOS-ESCOLHIDOS.md` pedem 3 primitivas ainda ausentes do motor; entram no backlog de `gameplay_engineer`:
  - **Status `Reflect` (Newton / Force-Law) — ENTREGUE (ADR-016 addendum, Balde B PR2, decisões do líder 2026-07-15, N-1..N-4):** o Poço Gravitacional virou **AoE** — `OnCast`, `side_filter EnemyOnly`: imobiliza (Stun, 1 turno) **TODOS os inimigos vivos** do grupo, e o dano-de-ATK do conjurador acompanha o mesmo grupo (o `power` da carta segue 0). Mirar um **ALIADO** vira o modo-benefício (N-3, mesmo padrão do Einstein/Faraday): a carta não ataca ninguém, concede **Reflect 30%/3 turnos** ao aliado alvo (`side_filter AllyOnly`, `ApplyStatus` sobre o `StatusId::Reflect` ordinal 15 — já existia como marcador da passiva `OnDamageReceived`, agora também aplicável como STATUS comum). Em inimigo, o Reflect-status dissipa; em aliado, o Stun dissipa (mesmo racional F-4 do Faraday). Se o dono da passiva-própria (`OnDamageReceived → Reflect`, equipada, intocada) **também** tiver o Reflect-status ativo, as duas fontes **SOMAM** (N-4, ex. 30%+30% = 60% — cada fonte dispara num ponto separado do código, sem dedup). Todo reflexo (equipado OU por status) é `take_damage` **PURO** (anti-recursão) e **nunca** entra no ledger de combos (`round_hits_`) nem no `RepeatLastAction` (`last_action_`).
  - **Regra geral "fogo amigo desligado" — ENTREGUE junto do PR2 (achado de auditoria, não scope creep):** nenhuma carta causa dano-base num alvo do **PRÓPRIO time** do conjurador (o dano zera nesse alvo, 0% de RNG consumido, sem aplicar status ofensivo); só o efeito declarativo da carta (`OnCast`, status) roda normalmente. Corrige retroativamente um bug pré-existente que fazia Einstein e Faraday em modo-aliado também causarem dano-de-ATK no beneficiário — os dois nunca deveriam ter machucado quem recebia o benefício. Achado-dominó corrigido junto: `resolve_targets` de uma carta **Grupo** tinha o lado-alvo fixo no "sempre os inimigos da party"; agora o alvo é sempre o lado **OPOSTO ao ator que joga a carta** (então um INIMIGO castando uma carta Grupo acerta a party, não o próprio time dele).
  - **Combo CROSS-ATOR (Pythagoras):** `√(a²+b²)` quando **2 aliados** batem no mesmo alvo no mesmo round. O pipeline de combos de `combat.md` §10 hoje só resolve combo de **1 ator**. Precisa de um combo que agregue os hits de atores diferentes no mesmo alvo/round.
  - **RepeatLastAction (Mandelbrot/Fractal-Echo + Ada/Re-Run) — ENTREGUE 2026-07-14 (ADR-016 addendum, MVP step 5):** eco do RESULTADO de uma ação de dano (reaplica o dano já causado, na % da carta, via `take_damage` puro — sem novo sorteio/crítico/mana/status). Mandelbrot (`OnCast`, sempre dispara, 0% consumo de RNG) e Ada (`OnAllyTurnEnd`, chance 34%, eco 100%) compartilham o handler. Exigiu ligar `OnAllyTurnEnd` a um ponto de disparo real (estava só declarado desde o step 2). A janela de memória é "última ação de dano de QUALQUER aliado NESTA RODADA" (zerada na fronteira de rodada) — ver AMB-01 em `_EFEITOS-ESCOLHIDOS.md` (a redação original dizia "neste turno").
  - **ChainDamage (Tesla / Chain-Arc) — ENTREGUE (ADR-016 addendum, MVP step 6):** descarga em cadeia. `OnCast`: depois que o dano-base atinge o alvo primário, a corrente **salta** pros próximos inimigos VIVOS na ordem da fila (lado oposto ao caster, excluindo o primário), **2 saltos = 3 alvos no total**. Cada salto **retém 62%** do anterior (decaimento multiplicativo: salto _k_ = `lround(danoPrimário × 0,62^k)`; ex.: dano 10 → 2º alvo 6, 3º alvo 4). Para quando faltam alvos vivos ou o salto arredonda pra 0; primário imune (dano 0) = a cadeia não propaga. Dano dos saltos é PURO (`take_damage`, não redispara hooks nem entra no ledger de combos), 0% de RNG. **Exceção de catálogo:** a Tesla é a única especial com **dano-base próprio** (`power`) — a cadeia escala dele. Esse dano-base é **turbinável por item futuro** — ver o **capacitor** (trade-off série/paralelo: mais tensão de pico por menos saltos, ou vice-versa), feat de item SEPARADA, ainda não implementada.
  - **NullProof (Gödel / Null-Proof) — ENTREGUE (ADR-016 addendum, Balde B PR3, decisões do líder 2026-07-15, G-1..G-3):** Gödel virou **castável mana-0**: `OnCast` concede o status `NullProof` ao PORTADOR (`side_filter AllyOnly`, inclui self — a própria Gödel ou um aliado). No PRÓXIMO hit ofensivo do portador, se o alvo é **Resistente OU Imune** (`multFraqueza < 1,0` — os dois tiers, sem distinção, G-3), o resolvedor força `multFraqueza = 1,0` e **CONSOME** (remove) o status; contra Neutro/Fraco o status fica **intacto** (G-2: só consome quando há algo a furar, não é um "consome sempre que ativo"). Duração-sentinela alta (99 turnos, `//PLAYTEST`) porque a saída real é por consumo, não por tick. O trunfo ORIGINAL (`IgnoresWeaknessWheel`, sempre-1.0 incondicional na própria carta Gödel) ganhou wiring genérico ao mesmo tempo — qualquer carta futura com a flag herda o mesmo comportamento, via totalmente INDEPENDENTE do status `NullProof`. `estimate_card_damage` (preview da UI) espelha a MESMA regra sem consumir (contrato puro do preview).
  - **DelayAction (Einstein / Time-Dilate) — ENTREGUE (ADR-016 addendum, MVP step 7, decisão do líder 2026-07-15 + addendum modo-aliado assimétrico 2026-07-15):** dilatação temporal, com efeito ASSIMÉTRICO por lado do alvo. `OnCast`, alvo **INIMIGO** (lado oposto ao caster): empurra a **ação do alvo** para o **FIM da fila da rodada corrente**, via a MESMA primitiva do Gambito-Reordenar (`InitiativeQueue::reorder_actor`) — a leitura original "empurra a próxima ação dele pra depois do próximo turno da party" foi traduzida no motor como "fim da fila da rodada" (ver AMB-02 em `_EFEITOS-ESCOLHIDOS.md`). `OnCast`, alvo **ALIADO** (mesmo lado do caster): **espelho benéfico** — o aliado AVANÇA, indo agir no **primeiro slot ainda-não-agido logo após o ator atual** ("dilata o tempo A FAVOR dele"), clampado pra nunca ultrapassar esse limite (invariante anti turno-duplo: o alvo nunca pode ocupar o slot de quem está em resolução agora). Regra de dissipação IDÊNTICA nos dois lados: um alvo que **JÁ AGIU nesta rodada** faz a carta se **dissipar** (no-op, não banca pra próxima rodada); alvo morto, fora da fila, ou sendo o `current()` também dissipa. Sem dano, 0% de RNG. A reordenação é **apagável por recomputação de SPD** (`recompute_by_speed`) — igual ao Gambito: se um status que muda SPD expira antes do alvo chegar no destino, a reordenação pode sumir (comportamento intencional, não bug). Família **Cinético** (não Universal — mesma família mecânica das cartas COMUNS de reordenar/knockback).
  - **RevealIntent (John Dee / Black-Mirror, "Scrying") — ENTREGUE (ADR-016 addendum, MVP step 8, manifesto item 6, decisões do líder 2026-07-15, D1-D4; ver AMB-07 em `_EFEITOS-ESCOLHIDOS.md`):** carta **Híbrida/Universal**, `TargetShape::Self`, mana `kActiveManaCost` (D4). `OnCast` aplica um buff (`StatusId::Scrying`, dur 3, `Refresh`) no PRÓPRIO conjurador e faz um **dump read-only** do `IntentPreview` de cada inimigo vivo (1 linha de log por inimigo — regra "todo efeito loga"). Enquanto QUALQUER aliado vivo portar Scrying, a FSM **RE-DUMPA na fronteira de rodada** (D3) — status-based, NÃO carta equipada (`execute_equipped` nunca despacha isto); expira normal por tick de duração, sem código dedicado de expiração. **D2 (não fura o caos do Patch-Zero):** se o brain do alvo é o CAÓTICO, a Scrying retorna RUÍDO — nunca revela os campos previstos, preservando a one-way door do boss. **D1, as duas faces do design original entram:** (i) a face de MUNDO ("Espelho Negro" — revela ocultos/passagens na exploração) fica STUB posse-only (zero código de combate, sem sistema de ocultos no overworld ainda); (ii) o **Scan de combate melhorado** — com a carta equipada, `resolve_scan` revela também status ativos + posição na fila + intent previsto do alvo, usando SÓ dados que já existem no modelo (nenhum atributo oculto novo). **Fail-soft assimétrico** no brain ausente (loga "sem sinal", nunca lança) — diferente do Gambito-Prever manual, que continua fail-fast: a Scrying roda em contextos automáticos/bônus opcional, não pode derrubar o combate. **TESTE-REI:** dois combates com a mesma seed, com e sem Scrying ativo, produzem HP/dano/ordem-de-fila/contagem-de-RNG byte-idênticos — prova 0% de RNG e read-only total.
  - **Maxwell / Spectra-Wave ("Onda Unificada") — ENTREGUE (ADR-016 addendum, manifesto item 12, decisão AUTÔNOMA do orquestrador 2026-07-16; ver AMB-08 em `_EFEITOS-ESCOLHIDOS.md`):** dano elétrico/luminoso em ÁREA — **NENHUM `EffectKind` novo**, a carta reusa PURO o caminho `TargetShape::Grupo` já entregue pelo Newton (mesmo `resolve_targets`/`resolve_use_card`): todos os inimigos vivos do lado oposto ao conjurador tomam dano-base (`power=5`/`kMaxwellPower`, `//PLAYTEST`, + ATK do conjurador, cadeia divisiva de sempre). **`effects` VAZIO** — nenhum `EffectSpec`, o dano-base sozinho é o efeito inteiro. Modo-aliado é **no-op PURO** (regra geral "fogo amigo desligado" zera o dano; sem `EffectSpec` não há nada além do log de dissipação — diferente do Newton, que ainda concede Reflect-status no modo-aliado). A face irmã fora-de-combate ("iluminar áreas escuras") fica **STUB posse-only, ZERO código** (mesmo padrão do Faraday/Euler/Menger) — feat MAXWELL-AREAS-ESCURAS segue como design pendente do criador, fora deste executor.
  - **Clone (von Neumann / Bruno) = entidade-Objeto, COM representação visual (decisão do criador 2026-07-14):** o clone **NÃO entra na fila de turnos** como 4º ator (o Party=3 fixo, §2, fica intacto); é uma **entidade-Objeto** que reusa o sistema de modificadores/invocação (dá hit extra, absorve golpe, etc.), mais barato pra dev solo. **PORÉM precisa aparecer visualmente** (ex.: um sprite-eco/fantasma translúcido num slot de batalha) pra o jogador PERCEBER que a mecânica está funcionando, mesmo sem turno próprio. O visual é obrigatório; a fila de turnos não muda.
- **Sincronização do marcador de raridade visual do frame** (comum/rara/lendária) com os 3 tiers desta taxonomia (§2.5): depende de `CARTAS-BALANCEAMENTO`.
- **Reconciliação da grafia "techMagic" → "TechnoMagik"** dentro de `techmagic.md` (já sinalizada naquele doc, item `TECHMAGIC-CANON`).
- **Reconciliação da raiz Sylvarin de "Morenh"** (§8.2, nota) com o léxico-semente formal.

---

**Última revisão:** 2026-07-12, rodada 3 (via economy-designer + criador): baseline COMUM fechada (§2.2, ManaCost 1/2/3 → Power 3/5/8); 3 sub-categorias das ESPECIAIS (§2.3, ativa/passiva/fora-de-combate); números dos 2 status destravados (§5, Sobrecarga 8/5/2, Resfriamento card 4). Statlines por-carta das 20 = rascunho provisório (`cartas-statlines-rascunho.md`), DIFERIDO até a escolha dos efeitos A/B/C do roster.
