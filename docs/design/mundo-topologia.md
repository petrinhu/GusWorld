# MUNDO-TOPOLOGIA — as 13 áreas do mundo aberto

> **Status:** PROPOSTA (decisões do criador, brainstorm interativo 2026-07-12). Esqueleto estrutural; ecossistemas finos, tipos de dungeon e ordem de companions a brainstormar/detalhar depois.
>
> **Filtro de produção (dev solo, `feedback_solo_baixa_infra_escopo`):** todo número foi calibrado pra caber numa produção solo. O caro (dungeons feitas à mão) fica capado; o barato (lugares secretos pequenos, interiores) escala.
>
> **Item TODO.md:** `MUNDO-TOPOLOGIA-AREAS` (seed #5/#6/#7 do brainstorm-backlog). Cross-ref `gdd.md` §7.1 (sem gate-hard), `PLACES.md` (settings canônicos), `gus-abertura.md` §13 (a 1ª aresta concreta: Distritos → Dutos), `project_fibonacci_easter_egg`.

---

## 1. Princípio de acesso (canon, gdd §7.1)

Mundo **aberto por design**: todas as áreas principais são alcançáveis desde o começo. Gating **nunca** por barreira dura — só por **dificuldade** (mais longe da origem = ecossistema + inimigos mais duros) e por **áreas especiais contornáveis** (destravam com item/carta/habilidade e viram atalhos, nunca o único caminho). A origem é os **Distritos Inferiores** (casa do Gus).

## 2. As 13 áreas (híbrido ancorado no canon) + gradiente de dificuldade

9 saem direto dos 8 settings canônicos + sub-locais; 4 são conectivas canônicas (Ferrovelhos, Orla Recursiva, Montadora Confluência, 1 área "faraday" especial). Dificuldade cresce com a distância da origem:

| Tier (dist. origem) | Áreas |
|---|---|
| 1 — fácil (origem) | **Distritos Inferiores** (start) · **Núcleo Metropolitano** (hub) |
| 2 — média | **Dutos Infernais** · **Setor Mirage** · **Periferia** · **Ferrovelhos** |
| 3 — média-alta | **Zona do Silêncio** · **Orla Recursiva** (charneira Cidade↔Selve) |
| 4 — difícil (Selve) | **Selve Sombria** · **Catedrais Neo-Sylvania** · **Montadora Confluência** |
| 5 — clímax | **Selve Profunda** |
| especial | **Área "faraday" secreta** (contornável, vira atalho; ela mesma é dungeon-segredo) |

> **Nota de canon (Periferia × Ferrovelhos, reconciliado 2026-07-12):** os dois são **duas zonas jogáveis do MESMO setting canônico "Periferia"** — **Periferia** (residencial oeste; Praça do Compilador, refugiados Tesla/Einstein) + **Ferrovelhos = "Periferia Industrial", distrito 2** (lore-bible §5.1: ferro-velho, oficinas, **lar do Dante**, atuação da FIR, economistas austríacos). Contíguos no mundo, divididos em dois mapas por gameplay.

## 3. Densidade (5 cheias + 8 enxutas = 13, Fibonacci)

Eixo de **população/quests** (independente da contagem de dungeons do §4):

- **Cheias (5)** — densas de NPCs, quests, vida: **Distritos Inferiores · Núcleo Metropolitano · Setor Mirage · Catedrais Neo-Sylvania · Selve Profunda**.
- **Enxutas (8)** — poucos NPCs, mais travessia/segredo: **Dutos Infernais · Periferia · Ferrovelhos · Zona do Silêncio · Orla Recursiva · Selve Sombria · Montadora Confluência · Área faraday**.

Nota: os **Dutos** são "enxuta" de população mas já **densos de enredo** (missão de abertura + Cauã + retorno do Ato 2) — uma "enxuta densa de história".

## 4. Dungeons — distribuição IRREGULAR (total 13, Fibonacci velado)

**Princípio (decisão do criador):** 1 dungeon por área seria previsível (o jogador vira checklist). Então o número **varia por área conforme a ficção** (0 a 2), e o **Fibonacci fica escondido no TOTAL (13)**, nunca visível por área (respeita o easter egg velado — o jogador não decodifica o padrão). "Dungeon" = espaço feito à mão (level design + puzzle/encontros); é o item CARO, por isso capado em ~13.

| Área | Dungeons | Ficção |
|---|---|---|
| Distritos Inferiores | 0 | rua/tutorial + NPCs (a dungeon da abertura fica nos Dutos) |
| Núcleo Metropolitano | 0 | hub cívico: lojas, quests, gente |
| Dutos Infernais | 2 | aparato (abertura) + laboratório FIR (Ato 2) |
| Setor Mirage | 1 | o Festival / Cult |
| Periferia (residencial) | 1 | vielas apertadas da Praça do Compilador |
| Ferrovelhos (Periferia Industrial) | 1 | reduto do Dante / ferro-velho |
| Zona do Silêncio | 2 | zona morta, cheia de ruína |
| Orla Recursiva | 1 | anomalia de fronteira |
| Selve Sombria | 1 | entrada da floresta |
| Catedrais Neo-Sylvania | 2 | várias catedrais |
| Montadora Confluência | 0 | lugar de lore/quest, não-dungeon (+1 **repositório leve** do mini-mapa M3, fora do cômputo das 13 — ver `mini-mapa.md` §0) |
| Selve Profunda | 1 | a dungeon final |
| Área faraday especial | 1 | ela mesma é a dungeon-segredo |
| **Total** | **13** | Fibonacci velado no agregado |

Dungeons chamam-se "**faraday**" no idioma do local (motivo Gaiola de Faraday; ver seed #2 `FARADAY-DUNGEON-ITENS` + `project_save_dungeon_pem_faraday`): algumas revestidas por gaiola que contém o PEM só dentro delas.

**Tipos de dungeon (seed #6; variedade, peso em puzzle pela Pillar 4):** cada uma das 13 tem um tipo, distribuído pra nenhuma vizinha repetir. Contagem: tutorial 1 · puzzle puro 4 · só batalhas 3 · labirinto 2 · mista 3.

| Dungeon | Tipo |
|---|---|
| Dutos — aparato (abertura) | tutorial (+ puzzle leve) |
| Dutos — laboratório FIR | só batalhas |
| Setor Mirage — Festival | puzzle puro (ilusão/decifrar) |
| Periferia (residencial) — vielas | labirinto |
| Ferrovelhos — reduto do Dante | só batalhas |
| Zona do Silêncio #1 | puzzle puro (acústica/silêncio) |
| Zona do Silêncio #2 | mista |
| Orla Recursiva | labirinto (fractal recursivo) |
| Selve Sombria | mista |
| Catedrais #1 | puzzle puro (engrenagem litúrgica) |
| Catedrais #2 | só batalhas |
| Selve Profunda (final) | mista (clímax) |
| Área faraday | puzzle puro (EM/Faraday) |

**Conceitos de dungeon (só conceito — tema + gimmick + forma; level design fino fica pra produção com a engine + `level-designer`; criador aprovou "só conceito" 2026-07-12):**

1. **Dutos — aparato (abertura):** linear-curta; gimmick = roteamento de energia cooperativo (Gus lê o circuito, Cauã é a corrente), cadeia de 3 sub-circuitos; ensina o loop e apresenta a party. O 1º grande momento de gameplay.
2. **Dutos — laboratório FIR (Ato 2):** hub com celas; gimmick = ondas de guardas FIR + terminais que o Gus hackeia (desliga câmeras/torres) pra abrir caminho; clímax = payoff da Subestação 7.
3. **Setor Mirage — Festival:** labirinto-de-ilusões curto; gimmick = o scan distingue real × holográfico (parede/inimigo falso vs real); decifrar o padrão do Festival pra desligar a extração sensorial. Arco Iara.
4. **Periferia (residencial) — labirinto de vielas:** o bairro apertado da Praça do Compilador; gimmick = becos/portões/escadas que reconfiguram a rota; refugiados Tesla/Einstein nos cantos.
5. **Ferrovelhos (Periferia Industrial) — reduto do Dante (só batalhas):** arena em camadas de sucata (cobertura/armadilha), ondas até o núcleo; lar do Dante (tensão velada), FIR presente, e o mercado espontâneo dos economistas nos cantos.
6. **Zona do Silêncio #1 — puzzle acústico:** câmaras ressonantes; gimmick = som não se propaga (silêncio mecânico); o Gus usa vibração/UHF (Matriz Ortodôntica) pra "ver" o que não ouve; puzzle de ressonância. Lar da Linda.
7. **Zona do Silêncio #2 — mista:** linear com bolsões; gimmick = combate abafado (inimigos que só reagem a vibração) + pequenos puzzles acústicos. Respiro de variedade após a #1.
8. **Orla Recursiva — labirinto fractal:** forma auto-similar (salas que se repetem em escala); gimmick = ler a regra recursiva (a "semente" do padrão) pra não se perder; a flora fractal invade o concreto. A dobradiça cidade↔Selve.
9. **Selve Sombria — mista:** trilha orgânica ramificada; gimmick = flora matemática (plantas em sequência — abrir caminho lendo o padrão de crescimento) + fauna-bug. Entrada da Selve, lar da Jaci.
10. **Catedrais #1 — puzzle litúrgico:** vertical (nave + campanário); gimmick = engrenagens/carrilhões que o Gus sincroniza (a liturgia é um programa mecânico); a sequência abre a cripta. Lar do Bento, Ordem Recursiva.
11. **Catedrais #2 — só batalhas:** claustro-arena; gimmick = guardiões-autômatos da catedral; o pavimento tesselado dá buffs posicionais; onda até o altar.
12. **Selve Profunda (final) — mista (clímax):** culminante longa; gimmick = tudo junto (puzzle + labirinto + batalha), paletas fundidas (recompensa ato 3), rumo ao confronto final. O ápice.
13. **Área faraday — puzzle EM (contornável/atalho):** câmara blindada; gimmick = dentro da gaiola o scan/HUD "cega" (sinal morto) — o Gus resolve às escuras, por dedução pura; guarda carta/item-chave.

## 5. Lugares secretos — gradiente Fibonacci por distância (barato)

Camada por cima das dungeons: **lugares secretos pequenos** (1-2 salas, 1 puzzle/recompensa, tiles reusados, **opcionais**), mais numerosos quanto **mais longe da origem** (gradiente velado 0→1→2→3→5). São o BARATO que enche os ermos de recompensa de exploração sem custar como dungeon. Distribuição também **irregular** por área (não um número limpo por tier — o Fibonacci fica no agregado, não exposto).

**Distribuição concreta (criador, 2026-07-12; total 21 = Fibonacci velado):**

| Área | Lugares secretos |
|---|---|
| Distritos Inferiores | 0 |
| Núcleo Metropolitano | 1 |
| Dutos Infernais | 1 |
| Setor Mirage | 1 |
| Periferia | 1 |
| Ferrovelhos | 2 |
| Zona do Silêncio | 3 |
| Orla Recursiva | 2 |
| Selve Sombria | 1 |
| Catedrais Neo-Sylvania | 1 |
| Montadora Confluência | 2 |
| Selve Profunda | 5 |
| Área faraday | 1 |
| **Total** | **21** |

Hubs populados (cheias) têm poucos/zero; os ermos concentram (Zona do Silêncio 3, Selve Profunda 5). Números irregulares de propósito (Fibonacci só no agregado). São opcionais e baratos; alguns podem ser procedurais/reusados.

## 6. Interiores dos 20 mestres do Codex (superfície, não-bloqueantes)

**Decisão do criador (2026-07-12):** cada um dos **20 mestres do Codex** (as 20 figuras históricas; Tusk é o capstone à parte) tem sua **área especial própria** — mas na **SUPERFÍCIE**, como **interior de casa / prédio / construção**, **NÃO bloqueante de passagem** (coerente com gdd §7.1). O jogador entra, resolve a missão/puzzle do mestre, encontra o **Tavus-Eco** (o self compilado do mestre) e **ganha a carta** dele (cross-ref `techmagic.md` Tavus-Eco + AMB-24 + `MESTRES-TAVUS-ECO-ENCONTRO` + cartas especiais `cartas-technomagik.md`).

- Camada **distinta** das 13 dungeons e dos lugares secretos: são espaços-interiores de missão, não dungeons de exploração.
- **Custo:** MÉDIO no agregado (20 interiores), mas cada um é barato (interior + 1 puzzle + diálogo do Tavus-Eco); reusa moldura de puzzle e o mecanismo Tavus-Eco. Escala com o roster já desenhado.

**Mapeamento TEMÁTICO POR CAMPO (criador, 2026-07-12):** cada área vira um "campus" de um campo — coerente, memorável e diegeticamente didático (o jogador aprende o campo pelo lugar). Os 10 mestres com lugar canônico seguem o canon; os 10 sem-lugar foram agrupados por campo. (Fonte da pesquisa de lugar: leitura do roster, 2026-07-12.)

| Área (campus) | Mestres | Âncora |
|---|---|---|
| **Dutos Infernais** — eletromagnetismo & energia | Faraday · Maxwell · Volta · Euler | Euler = dutos técnicos (canon); resto por campo |
| **Núcleo Metropolitano** — computação & matemática | von Neumann · Turing · Gödel · Ada Lovelace · Planck | von Neumann = Universidade Pública (canon) |
| **Ferrovelhos** — economia austríaca | Menger · Hayek · Mises · Bastiat | Menger = Mercado da Sucata Honesta (canon; nota PS-G5 2026-07-14: o Mercado é geograficamente a charneira Núcleo↔Periferia, `PLACES.md`, mas é tratado como parte do "campus Ferrovelhos" por tema; o ícone do interior do Menger fica na borda Ferrovelhos↔charneira); ordem espontânea emerge do mercado de sucata |
| **Periferia** — cientistas-refugiados cooperativos | Tesla · Einstein | ambos canon (Praça do Compilador, F067/F068); expulsos por mecenas coletivistas (axiologia) |
| **Catedrais Neo-Sylvania** — ciência sagrada da Era 1 | Giordano Bruno · John Dee · Pythagoras · Newton | Bruno/Dee canon Era 1; Pythagoras Era 1; Newton = eco Catedral Atelaiá |
| **Orla Recursiva** — geometria fractal | Mandelbrot | canon (borda da Orla, Rio Verdor; eco Núcleo Mandelbrot na Selve Profunda) |

Notas: (a) 6 das 13 áreas hospedam os interiores (as demais não têm mestre — concentração temática, ok); (b) **flag de canon a resolver:** o roster trata "Periferia Industrial" = "Ferrovelhos" (glossário lore-bible), mas o mapa atual os separa — decidir depois se Ferrovelhos é sub-distrito da Periferia ou área própria; (c) o Núcleo (5) e as Catedrais/Dutos/Ferrovelhos (4) são densos de interiores, coerente com serem campi.

## 7. Ecossistemas por área + contraste Pillar 5 (criador, 2026-07-12)

Cada área tem **UMA paleta clara** (barato: uma paleta por área, sem híbridos espalhados). O eixo Pillar 5 (megacidade ciber-gótica × Selve) usa **contraste DURO**: a mistura de paletas é reservada como recompensa.

| Área | Ecossistema (look) |
|---|---|
| Distritos Inferiores | ciber-gótico baixo/sujo, neon frio, concreto |
| Núcleo Metropolitano | megacidade ciber-gótica, vertical, corporativa |
| Dutos Infernais | subterrâneo industrial-arcano: canos, calor, brilho |
| Setor Mirage | ilusão holográfica, neon-glitch, ofuscamento |
| Periferia | favela-improviso, sucata habitada, quente |
| Ferrovelhos | ferro-velho: montanhas de sucata, ferrugem, óxido |
| Zona do Silêncio | distrito morto acusticamente, abafado, decaído |
| Orla Recursiva | **a dobradiça:** onde o concreto encontra a flora fractal — vegetação recursiva invadindo a cidade, glitch (única zona de mistura fora do clímax) |
| Selve Sombria | floresta-fronteira, bioluminescência, flora matemática |
| Catedrais Neo-Sylvania | catedrais góticas na mata, pedra + vitral-tech |
| Montadora Confluência | fábrica: carros elétricos flutuantes, linha alimentada por sucata |
| Selve Profunda | núcleo denso da Selve, fractal — **fusão PLENA de paletas (recompensa do ato 3)** |
| Área faraday | malha metálica, sinal morto, "cego" ao scan (blindagem EM) |

**Regra de contraste (Pillar 5 canon):** cidade = frio/neon/gótico; Selve = orgânico/biolum/quente. **Única transição = Orla Recursiva**; fusão plena só na **Selve Profunda** (ato 3). Custo BARATO (uma paleta por área).

**Detalhe de UX/UI (decisão do criador):** os **menus/HUD mudam de cor conforme a paleta da área atual** — imersão diegética. Custo BARATO na stack: é trocar as variáveis de cor do **RCSS** por área (UI servida pelo RmlUi via glintfx; cross-ref `project_rmlui_ui_stack` + `reference_glintfx_api`). Um tema RCSS por paleta de área, aplicado ao entrar na área.

## 8. Companions: lares e ordem de recrutamento (criador, 2026-07-12)

Lares canônicos, mapeados nos tiers de dificuldade:

| Companion | Lar (canon) | Tier |
|---|---|---|
| Cauã "Volt" | Dutos Infernais | T2 — **#1 (recrutado na abertura)** |
| Iara "Lumen" | Setor Mirage | T2 |
| Dante "Grid" (**traidor**) | Ferrovelhos (Periferia Industrial) | T2 |
| Linda "Siren" | Zona do Silêncio | T3 |
| Bento "Requiem" | Catedrais Neo-Sylvania | T4 |
| Jaci "Proxy" | Selve Sombria | T4 |

- **Ordem ABERTA, só sugerida por dificuldade** (gdd §7.1): sem ordem forçada; a distância-dificuldade nudga naturalmente Iara/Dante (T2) → Linda (T3) → Bento/Jaci (T4). Cada arco tem de funcionar em qualquer ordem (já é o design canon do Ato 2, "ordem livre").
- **Dante recrutável CEDO (simmer longo):** o Ferrovelhos (Periferia Industrial, lar dele) é T2/perto da origem, então o Dante naturalmente entra entre os primeiros — quanto mais tempo na party antes da revelação, mais dolorosa a traição. Nudge por proximidade, **sem gate duro**; um jogador que o deixe pra depois aceita um simmer mais curto (edge case do mundo aberto). Casa com o foreshadow que dispara "após 3 slots de Ato 2".

## 9. Fios abertos (continuar brainstorm)

Resolvidos em 2026-07-12: mapeamento dos 20 interiores (§6, temático por campo); distribuição dos lugares secretos (§5, total 21); conceitos das 13 dungeons (§4, "só conceito"); **reconciliação Periferia × Ferrovelhos** (§2 nota: duas zonas jogáveis do mesmo setting Periferia; Ferrovelhos = Periferia Industrial = lar do Dante).

Restam:

1. **Layout FINO de cada dungeon** (level design sala-a-sala) — fase de PRODUÇÃO, com o `level-designer` + a engine de mapa. NÃO fazer antes da engine (retrabalho).
2. **`ENGINE-MAPA-ONDA`** — onda de implementação do loader/compilador `.gmap` ([[reference_formato_mapa_gmap]]), pré-req de tudo isto virar jogável.
3. Detalhe: qual carta/item-chave cada lugar secreto e a área faraday guardam (liga a `cartas-technomagik.md` + FARADAY-DUNGEON-ITENS).
