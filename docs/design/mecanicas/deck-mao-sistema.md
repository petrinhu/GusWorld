# Sistema de Deck e Mão (cartas comuns × especiais)

**Status:** ARQUITETURA FECHADA no brainstorm colaborativo com o criador (2026-07-16). Números exatos (tamanhos, taxas, preços) = `//PLAYTEST`. Catálogo das comuns + tela de montagem = frentes próprias (ver §9). Substitui a versão de definições-base de 2026-07-15.

Cross-ref: [[project_sistema_cartas_technomagik]], [[project_repositorios_perdidos_canon]] (loot), [[project_economia_canon]] (crédito), [[reference_techmagic_engine_impl]] (motor de efeitos), [[reference_save_crypto_v2]] (save assinado = base anti-tamper), `docs/design/mecanicas/cartas-technomagik.md`, `docs/design/mecanicas/capacitor-item.md`.

O efeito de cada carta é ortogonal a este sistema: o motor techMagic (ADR-016) já executa os efeitos; este doc é o **meta-layer** de QUAIS cartas você traz e usa.

---

## 1. Duas classes de carta (decisão do criador)

- **Cartas COMUNS — para TODOS os personagens.** São **já compiladas** (equivalem a *apps prontos que qualquer um usa*). Cada personagem (Gus + os 6 companions) tem seu próprio conjunto.
- **Cartas ESPECIAIS — SÓ do Gus.** As **históricas (20 mestres do Codex)** + a de **Helon Tusk**. São cartas **"para compilar"**, e **compilar só o Gus faz** (canon: Gus = compilador universal, [[project_nome_gus_canon]]). O motor é agnóstico por-ator; a exclusividade é trava de CONTEÚDO (só entram no deck do Gus).

---

## 2. Modelo da mão — LOADOUT DETERMINÍSTICO (decisão-âncora)

**TCG pela CONSTRUÇÃO, não pelo azar.** Pesquisa (Magic/Pokémon vs Hearthstone/Marvel Snap): o gênero moveu-se de "variância de compra" (mana screw/flood, frustração) para **menos sorte, mais decisão**. O GusWorld leva isso ao limite, porque o pillar é *"vence por lógica/otimização, não por sorte"* + *"natureza é matemática rígida, não caos"* + motor determinístico (conta cada saque de RNG).

- **Sem compra/sorteio em combate.** A MÃO é um conjunto FIXO de cartas que você monta (loadout); todas ficam disponíveis o combate inteiro, limitadas só por **mana/AP** + **recarga/1×-batalha** (especiais seguem o gate `specials_cast_` já existente).
- A profundidade vem de **o que você traz** (loadout dado o limite de mão) + **em que ordem joga** (curva de mana, combos como Hipotenusa/diversidade do Hayek), NÃO da sorte do saque.
- **Não há tela de "mão inicial"/mulligan** — o mulligan dos TCGs só existe por causa do sorteio, que não temos.

---

## 3. Tamanho da mão (decisão: base-por-personagem + stat)

Limite de mão = **base por personagem** (identidade; ex.: Gus prodígio segura mais, um tanque menos) **que cresce com progressão/upgrade de hardware** (óculos, Tavus-Drive, matriz — casa com o pillar "loop acoplado ao hardware") **+ um componente de STAT mental/foco**. Piso por personagem, modulado por stat, subindo com o rig. Números = `//PLAYTEST`.

---

## 4. Estrutura: bolsa → mão (um nível)

- **DECK/BOLSA = a coleção do personagem** (tudo que ele possui), num **slot de inventário**, com **capacidade upgradável** (cresce com upgrade de bolsa/hardware).
- **MÃO = uma SELEÇÃO (loadout) do deck ativo** para a batalha. **A mão NÃO é um container — é uma lista de IDs que aponta pro deck ativo** (invariante anti-dup, §7).
- Sem nível intermediário de "deck curado": a única escolha significativa é a MÃO.
- **Gus:** mão comum normal (como todos) **+ 1 slot DEDICADO só pra especial** (decisão do líder 2026-07-16; garante a identidade de compilador sem competir com comuns nem torná-lo dominante). `//PLAYTEST`.

---

## 5. Montagem da mão — quando (decisão: bancada + swap de emergência)

- **Default: na BANCADA, fora do combate** (menu de deck em pontos seguros: cidade, save points, entre batalhas). O combate SEMPRE abre com a mão pronta → **zero tela no meio do fluxo** (preserva o ritmo). Como equipar antes de sair.
- **Válvula tática:** dá pra reconfigurar a mão **no meio da batalha gastando 1 ação/turno** (raro, para emergências).

---

## 6. Aquisição e descarte

### 6.1 Adquirir (decisão: multi-canal determinístico + achados híbridos)
- **Especiais (Gus):** vêm **SÓ in-game, por progresso NARRATIVO** (encontra/aprende com cada mestre → compila a carta; Tavus-Eco / forja). **Nunca** compradas/craftadas/dropadas, e **classe PROTEGIDA** (não vão pro deck morto — ver §7). Confirmado pelo líder 2026-07-16.
- **Comuns — espinha determinística (você sempre consegue o que precisa, sem grind):**
  - **Loja "app store" (crédito):** compra o que quer. Casa com "faz-se dinheiro com trabalho" + a metáfora de app. **Lojas compram E vendem** cartas.
  - **Loot de repositórios/missão/luta:** GARANTIDO (recompensa fixa de arco/beat) **E ALEATÓRIO** (drop de missão/combate — variância de COLEÇÃO, não de combate; ok com o pillar anti-sorte porque não decide batalha, só engorda a coleção). Decisão do líder 2026-07-16.
  - **Craft (compilar)** via F3-Alpha (material/código → carta).
- **Achados na exploração — camada BÔNUS (nunca gate de progressão):** modelo **híbrido**:
  - **Maioria VISÍVEL/colocada** (brilho no chão, baú, drop) — determinístico, você vê e pega, zero grind.
  - **Zonas de grama especiais com encontro-surpresa OCASIONAL** — probabilístico, mas com **pity/anti-streak** e **seed CONTÁVEL** (o motor conta o RNG). Diegético: *não é sorte, é a função recursiva da Selve produzindo a carta* (pillar "natureza é matemática rígida; anomalias = bugs"). Taxa baixa; caem **comuns/duplicatas** (pra craft/venda), nunca especiais.

### 6.2 Descartar — deck ativo × deck morto (decisão: pen persistente + saídas)
- **Deck MORTO = pilha PERSISTENTE, INERTE, ONE-WAY.** Carta movida pro deck morto **NÃO volta ao deck ativo — nunca** (regra dura do criador). Limite **por contagem** (não peso — peso é micromanagement chato pra 11+).
- **Saídas do deck morto:**
  - **Upload/reciclagem (base):** vira crédito à taxa-base (**1 crédito/carta**), com mensagem diegética: *"seu código foi enviado ao repositório-commons e será reusado por devs precisando de ideia — você recebeu N créditos por essa doação."* (Carta = código: não some do nada, faz **upload**.) Pode ser automático com o tempo/ao salvar.
  - **Vender a NPCs do caminho:** NPCs errantes **compram** (só compra, paga possivelmente mais que a base). Encostar no NPC → menu {1-Conversar, 2-Comerciar, ...}.
  - **Lojas:** compram E vendem cartas (mercado).

---

## 7. INVARIANTES ANTI-EXPLOIT (inegociável — engenharia, não design)

Fecham as duas fraudes que o criador antecipou (duplicar carta; usar deck morto como slot extra), **por construção**:

1. **Carta = instância única com ID, vive em EXATAMENTE UM container** (deck ativo XOR deck morto). 
2. **A MÃO é uma SELEÇÃO (lista de IDs → deck ativo), não um container.** Carregar na mão não move nem copia → **duplicação impossível**.
3. **Deck morto é INERTE:** suas cartas **não contam na capacidade do deck ativo, não podem ser selecionadas pra mão, não podem ser jogadas** → não servem de "slot extra".
4. **One-way por código:** existe API `ativo→morto`; **NÃO existe** API `morto→ativo`.
5. **Venda/upload é ATÔMICO** (remove-do-container-DEPOIS-credita numa transação) e **idempotente**: carta sem container não vende de novo.
6. **Mão só puxa do deck ATIVO.**
7. Base de confiança: o save é **AEAD/HMAC assinado** (ADR-015) — adulterar o arquivo quebra a assinatura. Os invariantes acima protegem contra bug de LÓGICA (o vetor real dentro do jogo).
8. O **deck morto persistente** é mais superfície → o `qa-engineer` DEVE ter testes dedicados de dup/slot-extra/one-way/atomicidade/round-trip no save.
9. **CLASSE PROTEGIDA — ESPECIAL/SUPER nunca vão pro deck morto** (Finding B do parecer de economia, 2026-07-16). São únicas (1 de 20 + a Super), só-narrativa, sem craft/drop/2ª chance: mandá-las pro morto (one-way) = **perda permanente de conteúdo único**. A lógica/UI do descarte **RECUSA** carta ESPECIAL/SUPER (guard por tier no service de descarte; custo ~1 `if`, risco altíssimo se faltar). Elas ocupam slot de deck normalmente, mas o único "descarte" possível delas é não-existente (ficam sempre). Teste dedicado do `qa-engineer`: tentar mandar especial pro morto → rejeitado.

---

## 8. Itens de bolsa (capacitor etc.) — sistema de slots SEPARADO

O **capacitor** (`capacitor-item.md`) e outros itens de bolsa **NÃO competem** com slots de carta — são um sistema de slots próprio, equipado à parte. O capacitor é modificador UPSTREAM das cartas elétricas (altera power/saltos antes do handler), não uma carta.

---

## 8b. Catálogo das cartas comuns — ESTRUTURA (fechada 2026-07-16)

Ancorado no canon (`combat.md §6`): as 5 famílias = especialidades dos companions; Gus = universal. Ataque básico é **ação inata (não-carta)** → as comuns são TODAS coloridas por família.

| Família | Companion | Identidade | Status |
|---|---|---|---|
| Elétrico | Cauã Volt | burst single-target | Stun |
| Bioquímico | Jaci Proxy | DoT/degradação | Poison/Corrode |
| Sônico | Linda Siren | área-CC/interrupção | Disrupt/Silence |
| Cinético | Bento Requiem | impacto/deslocamento | Knockback/Break |
| Criptográfico | Iara Lumen | utilidade/anti-buff | Expose/Decrypt |

- **Template de arquétipos por família** (decisão do líder): desenha o molde 1×, instancia nas 5 (anti-OE). **~6-7 arquétipos:**
  1. **Jab** (mana 1, dano baixo, sem status) — o spam controlado.
  2. **Golpe+status** (mana 2, dano médio + o status da família).
  3. **Assinatura** (mana 3, a mecânica-cara: Elétrico=burst, Bio=DoT forte, Sônico=área, Cinético=deslocamento, Cripto=expose/anti-buff).
  4. **Status-puro** (mana 1-2, controle, pouco/zero dano — aplica o status forte).
  5. **Utilidade de classe** (o não-dano da cor: Bio=cura leve/regen, Cripto=scan+, Sônico=silence, Cinético=escudo/reposição, Elétrico=recarga/overclock).
  6. **Finalizador-sinérgico** (mana 3, bônus se o alvo JÁ tem o status da família — sinergia intra-família).
  7. (opcional por família.) Total ≈ 5×7 = **~35 comuns**.
- **Gus (universal):** a coleção dele pode cruzar famílias (companions ficam travados na sua), adquirida como todos (compra/loot/acha, compete por crédito); a MÃO é do mesmo tamanho (5 comuns + 1-2 especiais) → **vantagem = versatilidade de coleção, não mais slots**. Flag `IsUniversalCompiler` (canon F2-E.10, `combat.md §6`).

RESTA: **números `//PLAYTEST`** (em parecer com o `economy-designer`, 2026-07-16) + o **conteúdo real** de cada arquétipo × família (statlines) + playtest N=3.

---

## 8c. Números baseline (`//PLAYTEST` — parecer do `economy-designer` + decisão do líder 2026-07-16)

Todos afináveis no playtest N=3. O parecer completo (faucets×sinks, riscos-mestres) está no relatório do agente.

| Número | Valor | Nota |
|---|---|---|
| Curva mana→power | 1→3, 2→5, 3→8 | canon §2.2 (combat.md), mantido |
| Mão comum (base) | **5** | 71% do toolkit ~7 = escolha real de loadout |
| Delta Bento (tanque) | **−1 (mão 4)** | trava de identidade; compensar em Def/HP (medir TTK com/sem Bento no N=3) |
| Stat mental | +1 por patamar de foco/INT, **COM TETO** (mão máx ≤ toolkit−1, ~**6**) | sem cap a mão iguala o toolkit e mata a escolha no late-game (anti-snowball) |
| Slots especiais do Gus | **1** dedicado | decisão do líder (conservador; não competir com comuns, não abrir com 2 especiais fortes) |
| Deck/bolsa (capacidade) | **34 → 55 → 89** (progressão explícita, upgradável) | comporta 20 especiais + comuns no late-game |
| Deck morto (limite) | **8** + **auto-upload FIFO** ao encher | one-way; ESPECIAL/SUPER nunca entram (§7 inv. 9) |
| Upload ao commons (base) | **2-3 cr/carta** | a 1cr virava "carta = lixo"; 2-3 preserva hierarquia |
| Compra de comum na loja | **12-18 cr** | ratio compra:venda ~3-4:1 (padrão do gênero) |
| Venda de comum | NPC **3-5** / loja **4-6 cr** | |
| Achado na grama (pity) | teto **13** encontros + **taxa-base ~10-15%/encontro** | pity = rede de segurança, não relógio |

**Supersessão:** o canon antigo "deck de **15 em campo**" (`cartas-technomagik` §2.1, modelo de camada única) fica SUPERADO pela estrutura bolsa→mão (§4): a **mão-loadout (~5)** é o que fica "em campo" por batalha; o "15" não é mais um número do sistema.

**Riscos-mestres a vigiar no N=3 (do parecer):** (1) perda permanente de especial por bolsa cheia — RESOLVIDO por §7 inv.9; (2) reconciliação comum/especial — RESOLVIDA pela evolução de pillar 2026-07-16; (3) mão que cresce sem teto — RESOLVIDA pelo cap do stat mental acima.

---

## 9. Faseamento + frentes abertas (execução)

**Contra-argumento anti-OE (dev solo):** o loadout funciona SEM o mercado de NPC. Fasear:
- **MVP:** deck ativo + mão (loadout) + bancada + aquisição por loja/loot/craft + achados visíveis + **descarte por upload-em-crédito**. Já torna as cartas jogáveis.
- **Onda 2:** deck morto persistente completo + **NPC-trade / menu de diálogo {Conversar, Comerciar}** + mercado de loja (compra/venda) + grama-surpresa com pity.

**Frentes abertas (decisão/execução futura):**
- **TELA de montar a mão (bancada)** — brainstorm próprio, via **mockup HTML** (regra de UI); reflete o cockpit "Tático" + estética terminal. (`docs/design/mockups/`)
- **Números** (`//PLAYTEST`): base de mão por personagem + curva por hardware + peso da stat; slots especiais do Gus (1-2); capacidade do deck ativo; limite do deck morto; taxa-base de upload (1cr) e preço de NPC/loja; taxa/pity dos achados de grama.
- **Catálogo das comuns** — o conjunto real de "apps" por personagem, como se relaciona com as 5 famílias (frente de design + `economy-designer`/`lead-game-designer`).
- **NPC-trade:** menu de diálogo extra + fluxo de venda de carta (encosta → {Conversar, Comerciar, ...}).

---

## 10. Como conduzir

Frente de design colaborativa (não código imediato). Brainstorm DIRETO com o criador ([[feedback_brainstorm_direto_sem_agentes]]); cada decisão via AskUserQuestion. Implementação (pós-decisão) via agentes: `backend-engineer` (POCO de deck/mão/containers + invariantes + save), `gameplay_engineer` (loadout em combate, swap de emergência), `ux-ui-designer`/mockup (tela da bancada). Casa com CARTAS-BALANCEAMENTO e o motor de efeitos já entregue.
