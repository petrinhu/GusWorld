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
| Periferia | 1 | reduto do Dante |
| Ferrovelhos | 1 | ferro-velho labiríntico |
| Zona do Silêncio | 2 | zona morta, cheia de ruína |
| Orla Recursiva | 1 | anomalia de fronteira |
| Selve Sombria | 1 | entrada da floresta |
| Catedrais Neo-Sylvania | 2 | várias catedrais |
| Montadora Confluência | 0 | lugar de lore/quest, não-dungeon |
| Selve Profunda | 1 | a dungeon final |
| Área faraday especial | 1 | ela mesma é a dungeon-segredo |
| **Total** | **13** | Fibonacci velado no agregado |

Dungeons chamam-se "**faraday**" no idioma do local (motivo Gaiola de Faraday; ver seed #2 `FARADAY-DUNGEON-ITENS` + `project_save_dungeon_pem_faraday`): algumas revestidas por gaiola que contém o PEM só dentro delas.

## 5. Lugares secretos — gradiente Fibonacci por distância (barato)

Camada por cima das dungeons: **lugares secretos pequenos** (1-2 salas, 1 puzzle/recompensa, tiles reusados, **opcionais**), mais numerosos quanto **mais longe da origem** (gradiente velado 0→1→2→3→5). São o BARATO que enche os ermos de recompensa de exploração sem custar como dungeon. Distribuição também **irregular** por área (não um número limpo por tier — o Fibonacci fica no agregado, não exposto).

## 6. Interiores dos 20 mestres do Codex (superfície, não-bloqueantes)

**Decisão do criador (2026-07-12):** cada um dos **20 mestres do Codex** (as 20 figuras históricas; Tusk é o capstone à parte) tem sua **área especial própria** — mas na **SUPERFÍCIE**, como **interior de casa / prédio / construção**, **NÃO bloqueante de passagem** (coerente com gdd §7.1). O jogador entra, resolve a missão/puzzle do mestre, encontra o **Tavus-Eco** (o self compilado do mestre) e **ganha a carta** dele (cross-ref `techmagic.md` Tavus-Eco + AMB-24 + `MESTRES-TAVUS-ECO-ENCONTRO` + cartas especiais `cartas-technomagik.md`).

- Camada **distinta** das 13 dungeons e dos lugares secretos: são espaços-interiores de missão, não dungeons de exploração.
- **Distribuição:** espalhados pelas 13 áreas conforme a **era/lugar canônico de cada mestre** (já definido nos `docs/design/roster-analogos/*.md`); uma área cheia pode hospedar vários interiores de mestre. Mapeamento exato = detalhe posterior (segue o roster).
- **Custo:** MÉDIO no agregado (20 interiores), mas cada um é barato (interior + 1 puzzle + diálogo do Tavus-Eco); reusa moldura de puzzle e o mecanismo Tavus-Eco. Escala com o roster já desenhado.

## 7. Fios abertos (continuar brainstorm)

1. **Ecossistemas finos** por área (o "look"/bioma de cada uma, contraste Pillar 5) — seed #5.
2. **Tipos de dungeon** por área (tutorial / puzzle puro / só batalhas / labirinto / mista) — seed #6.
3. **Ordem/lares dos companions** restantes (#2-6: Iara, Bento, Linda, Dante, Jaci) nas áreas.
4. Mapeamento exato dos 20 interiores de mestre → áreas (segue o roster).
5. Distribuição fina (quais áreas ganham quais lugares secretos).
6. Onda de implementação da engine de mapa (`ENGINE-MAPA-ONDA`, pré-req de tudo isto virar jogável).
