# Design Pillars — GusWorld

**Status:** Revisão 1 — concluída em sessão colaborativa com criador supremo (2026-05-15). Validado ponto-a-ponto. **Canônico.**

Toda feature do jogo responde: "qual pillar serve?". Não serve = corta.

---

## Anti-pillars (boundaries — o que GusWorld NÃO é)

### Fundamentais (não-negociáveis)

- Não é action-RPG real-time. Combate é turn-based com prep livre.
- Não é open-world. Estrutura é hub central (GusWorld City) + radiais (missões de companion).
- Não é roguelike. Narrativa linear com escolhas curadas, sem run procedural.
- Não é multiplayer. Single-player puro, sem cooperativo ou competitivo.
- Não é dark gratuito. **Toda escuridão serve a propósito narrativo claro** — sem edgy pra ser edgy, sem grimdark sem esperança, sem body horror, sem exploração de trauma infantil pra impacto barato.
- Não é grind-fest com level cap 99. **Knowledge Progression** (ver §Sistemas-âncora) substitui grind — repetir inimigo reduz XP e aumenta conhecimento.
- Não é "Magic: The Gathering completo". TCG é mecânica, não produto: **40-60 cartas totais no jogo** (todas descobríveis), deck de **15 em campo**, **mestria por uso** (carta cresce com repetição).

### Boundaries de gênero/forma

- Não é metroidvania. Sem mapa interconectado físico-gated; companions destravam settings narrativamente.
- Não é souls-like. Sem permadeath default, sem stamina-management, sem morte-como-progressão.
- Não é visual novel. Diálogo serve narrativa; combate/exploração são core.
- Não é tactics-grid (XCOM / Fire Emblem). Combate turn-based de party fixa, sem posicionamento em grid.
- Não é fotorrealista. Cel-shaded 3D low-poly, paleta restrita (style guide define).
- Não é deck-builder roguelike (Slay the Spire). Deck é narrativo + cartas curadas, não randomized run-based.
- Não é tutorial wall-of-text. Onboarding orgânico via jogo, sem pop-ups explicativos.

### Boundaries comerciais/técnicas

- Não é IAP / microtransação. Single-player paid game, zero monetização in-game.
- Não é always-online. Sem requisito de conexão; save local.

---

## Pillar 1 — Lógica vence força

**Frase-âncora:** Combate e exploração são resolvidos por análise, predição e combinação — nunca por reflexo, dano bruto ou grind.

### Mecânicas que servem

- **Active turn-based** sem timer no turno do jogador; **timed inputs opcionais** dão bônus (skipável sem punição).
- **Wait-mode toggle** (acessibilidade): user pode ligar/desligar pressão temporal.
- **Compilação do Codex** com 40-60 cartas curadas, deck 15 em campo, mestria por uso.
- **Telegrafia condicional**: 1º encontro com inimigo = surpresa. 2º+ = óculos táticos revelam próximo ataque **consumindo recurso** (energia/AP/turno parcial). Vetor do Gambito complementa.
- **RNG calibrado**: hit/dodge/crit com % visível; Gambito permite forçar re-roll ou cancelar; RNG **decai com knowledge** (Knowledge Progression).
- **Puzzles ambientais** que ensinam o sistema.

### Anti-mecânicas

QTE; real-time twitch; HP-sponge boss; dano inflacionado por level; "spam habilidade vence"; aleatoriedade que pune skill sem mitigação.

---

## Pillar 2 — Magia é sistema formal computável, natureza é matemática

**Frase-âncora:** Toda magia é diegética como sistema formal computável (em silício OU em substrato mecânico/analógico); todo bioma da Selve obedece padrão matemático observável pelo jogador.

> **Reformulação canônica vs original:** versão prévia dizia "magia é software". Substituído por "sistema formal computável" para acomodar **Bento "Requiem"** (magia heráldica em relojoaria mecânica de latão) — relógio mecânico É state machine, só substrato diferente que silício.

### Mecânicas que servem

- **Sintaxe de cartas** legível e curada: tipo + modificador + alvo, gramática internamente consistente.
- **~200 combinações pré-planejadas** + **5-10 combos secretos** descobríveis por experimentação.
- **Inimigos especiais = bugs/vírus**; debug = puzzle.
- **Fractais e padrões numéricos recorrentes visíveis** na flora da Selve; scan revela fórmula do bioma.
- **Ruído Coerente (Perlin/Simplex) como standard de RNG natural** — determinístico, seedable, multi-octave, fractal real. Usado em geração de bioma, padrões de spawn, comportamento de fauna passiva, textura procedural.
- **Diegético:** óculos táticos do Gus **revelam o noise underlying** (modo scan = vê o gradient Perlin literal por trás do "caos aparente").
- **Caos calibrado em camadas:**
  - Maioria da Selve: **caos determinístico** (parece caos, é Mandelbrot).
  - **Fronteira final / Patch-Zero**: caos genuíno irredutível — limite do conhecimento, núcleo de terror narrativo.

### Antagonista coerente

**Sterling Locke** opera em arco A→B: começa como **exploit** (joga dentro do sistema buscando bugs) → evolui pra **reescritor de compilador** (reescreve regras). Libera **Patch-Zero** (caos genuíno) na rede internacional como arma corporativa vazada.

### Anti-mecânicas

"Magia misteriosa" sem regra; RNG opaco; fauna com comportamento arbitrário; lore místico sem analogia computacional; "fé" como mecânica.

---

## Pillar 3 — Triângulo de hardware é a interface

**Frase-âncora:** Toda habilidade do Gus passa por **Óculos Táticos** (input/scan), **Matriz Ortodôntica** (range/amplificação) ou **Tavus-Drive** (executor). Mecânica nova encaixa no triângulo ou justifica exceção explícita.

### Mecânicas que servem

- **Upgrades plugados em um dos três vértices**.
- **Sinergias entre vértices** (ex: Óculos + Matriz = scan de longo alcance).
- **UI diegética** que reflete o hardware ativo.
- **Companions B+C híbrido:**
  - Alguns sem hardware (habilidades inatas/treinadas — Cauã reflexos, Iara psicologia)
  - Outros com 1 vértice próprio / hardware análogo individual (Bento relojoaria, Jaci bio-ampolas)
  - **Dante exceção Pillar 3 invertida (meta-hardware)**: NÃO tem vértice próprio. Opera root nos vértices dos outros (instala componentes alheios + manipula hardware dos companions). Paralelo embrionário do 4º elemento Sterling (Locke Core + Rede). Foreshadow estrutural traição canon (D.1 rootkit). Cross-ref lore-bible:188.
- **7 árvores de skill paralelas** (Gus + 6 companions), cada uma alinhada à tech/perfil do dono:
  - **Auto-mode**: IA sobe upgrade conforme arquétipo do personagem
  - **Manual**: jogador escolhe ramo
  - **XP individual** por participação em combate + **boost narrativo offscreen** quando companion está fora da party (volta pra cidade dele e ganha XP automático — não defasa)

### Antagonista coerente

**Sterling Locke tem 4º elemento proprietário (Locke Core + Rede Distribuída)** que Gus NÃO TEM nem terá. Vantagem corporativa: implante neural cortical (Locke Core) + enxame de drones/proxies (Rede). Gus combate **decodificando**, não copiando — coerência etimológica "Locke = fechadura". Boss final 2 fases: derrota Rede → expõe Locke Core → decodifica via knowledge farming.

### Anti-mecânicas

Habilidade "vinda do nada"; magia sem origem técnica; upgrade genérico "+10% dano"; inventário desconectado do corpo do Gus.

---

## Pillar 4 — Prodígio de 11 anos, não herói adulto

**Frase-âncora:** Gus resolve por inteligência, curiosidade e otimização — tom analítico, não power-fantasy. Mundo é sombrio; protagonista é luminoso (luminosidade reforjada, não ingênua).

### Mecânicas que servem

- **Diálogo técnico-precoce**, humor de nerd.
- **Vulnerabilidade física**: HP baixo, sem combate físico bruto, depende de hardware + companions.
- **Mentores adultos derrotados** que Gus ajuda; **adultos = antagonistas ou ausentes**.
- **Vitórias por descoberta**, não por força.
- **Companions peers 11-14**: todos crianças/adolescentes. Sem adultos ofuscando.

### Arc do Gus — multifase A+B+C+D

- **Ato 1**: A (puro otimista, inocência intacta).
- **Ato 2 progressivo**: B+C (perde inocência gradual, amadurece mantendo essência).
- **Pós-traição Dante (ato 3)**: D (trauma agudo, endurece).
- **Final**: reconcilia — luminosidade **reforjada** pela experiência, não destruída.

Arc clássica de iniciação heroica: perde → recupera → integra.

### Game over

- **Normal default**: game over puro (HP=0 → reload save).
- **Hard mode (unlock pós-zerar)**: permadeath + **kernel panic puzzle** ao chegar HP=0 (sequência puzzle pra reboot; falhar = game over real).

### Companions — imortais com incapacitação

- HP=0 do companion = **incapacitado** (não morto).
- Gus **carrega/leva** companion pro **hospital**.
- **Cura gratuita demora** (tempo de jogo passa); **cura paga é rápida** (gasta moeda).
- Implica economia/moeda como sistema obrigatório (ver §Sistemas-âncora).

### Temas dark calibrados

**SIM:** morte de NPCs off-screen ou estilizada (compila erro + dissolve); vício digital não-glorificado; exploração corporativa de crianças órfãs (subtema Sterling); traição íntima (Dante); bullying / isolamento como backstory. **Sangue:** **pouco** em batalha (gota leve, não jato) + indicador "ferido" andando (postura, mancando).

**NÃO:** suicídio; abuso físico explícito; sexualidade entre crianças; drogas reais; gore; **linguagem profana zero** (nem "droga"/"merda" — limpa total); fan-service edgy.

### Acessibilidade (P4 estende WCAG)

Indicador de HP baixo é **multimodal**: sangue + animação de postura + ícone + barra HP. Sangue NÃO é único marker (daltônicos não veem vermelho contra cenário).

### Anti-mecânicas

Romance; gore explícito centrado no Gus; fan-service edgy; dilema moral cínico adulto; "kid soldier" treinado em armas; power-fantasy adulta.

---

## Pillar 5 — Contraste multipolar com 2 âncoras (cidade × Selve)

**Frase-âncora:** O mundo de GusWorld é organizado em **8 settings de identidade visual/sonora/mecânica distinta**, ancorados pelo contraste **megacidade ciber-gótica × Selve Sombria tecnorgânica**. Cada setting é arena dramática de um companion ou antagonista — nunca decoração genérica.

> **Reformulação canônica vs original:** versão prévia dizia "bipartido". Substituído por "multipolar com 2 âncoras" — o contraste cidade×Selve permanece **eixo temático fundador**, mas o mundo se expressa em 8 polos satélites que orbitam esse eixo.

### Os 8 settings

| # | Setting | Vínculo operacional | Identidade-síntese |
|---|---|---|---|
| 1 | **GusWorld City** (Núcleo Metropolitano) | Gus | Circuito impresso 3D, neon ciano, grid ortogonal pra xadrez holo. **Hub central** persistente. |
| 2 | **Catedrais de Neo-Sylvania** | Bento "Requiem" | Gótico arcaico, vitrais digitais, latão envelhecido, acústica reverberante solene. |
| 3 | **Dutos Infernais** (subterrâneo) | Cauã "Volt" | Catacumbas reconfiguradas, plasma luminescente, arcos voltaicos, navegação vertical. |
| 4 | **Periferia Industrial** (Ferrovelhos) | Dante "Grid" (traidor) | Sucata, fuligem carbono, hardware descartado, matéria-prima infinita. |
| 5 | **Setor Mirage** (entretenimento) | Iara "Lumen" | Holo-publicidade sobreposta, refração instável, desorientação espacial. |
| 6 | **Zona do Silêncio** (antenas mortas) | Linda "Siren" | Cânion tecnológico, silêncio gótico, ruído branco estático. |
| 7 | **Selve Sombria** (fronteira tecnorgânica) | Jaci "Proxy" | Biomassa programada, padrões numéricos recorrentes visíveis, esporos sintéticos, fauna biocibernética. |
| 8 | **Catedrais de Silício + Cúpula Sterling** (ato 3) (canon Pillar 5 setting 8 = climax ato 3 Sterling, NÃO confundir com environments/08-selve-profunda.md que é setting Selve Profunda Jaci) | Sterling Locke | **Mesmo setting, 2 zonas opostas**: exterior = híbrido corrompido (Sterling tentou fundir cidade+Selve, falhou); interior = estéril euclidiano perfeito (santuário negacionista). |

### Mecânicas que servem

- **Setting = arena mecânica do companion** ou antagonista. Cada cenário tem features que favorecem habilidade do dono.
- **Hub + radiais**: cidade GusWorld é hub navegável persistente; missões companion são incursões radiais (vai → cumpre → volta).
- **Hub evolui visualmente** conforme jogo progride: Sterling expande controle → neon vira frio, drones aumentam, NPCs somem.
- **Capítulos seguem arcos de recrutamento dos companions** (D de C.1 narrativo): cada companion tem cidade própria + missão pessoal pré-recrutamento + arc convergente com Gus.
- **Companion sai da party** quando arco dele finaliza (volta pra cidade dele); **todos reconvergem no ato 3**.

### Estrutura de capítulos

- **Ato 1**: cidade (Gus solo + introdução ao mundo + 1-2 recrutamentos).
- **Ato 2**: incursões radiais (recruta resto dos companions, explora 5-7 settings).
- **Ato 3**: Catedrais de Silício corrompidas → Cúpula Sterling estéril (todos reconvergem; Dante revela traição; combate final em 2 fases — Rede + Locke Core).

### Paletas e soundscape

Pendente — definição detalhada por setting será feita em **modo colaborativo com `art-director` + `audio-designer-composer`** durante Fase 2 de produção. Style guide atual cobre 2 paletas (cidade + Selve); precisa expansão pra 8 paletas + 8 ambientes sonoros.

### Anti-mecânicas

Zona híbrida genérica; reuso de assets entre settings sem propósito narrativo; bioma intermediário "para variar"; setting como decoração desconectada de mecânica.

---

## Sistemas-âncora derivados (referência cruzada)

Decisões colaterais que emergiram da revisão dos pillars e merecem doc próprio em `docs/design/mecanicas/`:

### Knowledge Progression System (anti-grind real)

Cada kill de inimigo rende **menos XP** mas **mais conhecimento**.

- **XP por kill**: decrescente, escalonado por N apariçoes do inimigo no jogo inteiro
  - Inimigo comum (~20 apariçoes designadas) → -5% XP por kill, +5% lore, +5% previsibilidade
  - Mestre raro (4 apariçoes) → -25% por kill, +25% lore, +25% previsibilidade
  - Boss único (1 apariçao) → 100% nos 3 contadores no kill único
- **Previsibilidade**: +X% por kill (até 100% = RNG zerado pra esse inimigo)
- **Wiki/Lore**: +X% por kill no **Diário do Gus** (max 1 pág comum / 4 pág mestre-chave)
- **Carta mestria**: ortogonal — cresce por **uso da carta**, não por kill

**Trade-off do jogador:** farmar = vira "Vector" analítico (RNG menor, mais opções táticas) MAS XP definha rápido. Quem não farma joga com mais mistério/risco.

Doc dedicado: `docs/design/mecanicas/knowledge-progression.md` (a criar).

### Hospital + Economia

Implicação direta de P4 (companions incapacitados).

- Companions HP=0 = incapacitado → Gus carrega pro hospital
- Cura gratuita (demora — tempo de jogo passa)
- Cura paga (rápida — gasta moeda)
- **Moeda do jogo necessária** → convoca `economy-designer` em modo colaborativo pra modelar sources/sinks/inflação

Doc dedicado: `docs/design/mecanicas/hospital-economia.md` (a criar).

### Diário do Gus (UI canônica)

Wiki in-game paginada que registra Knowledge Progression. Sub-seções:
- **Bestiário** (inimigos — 1-4 páginas por entrada, preenche por kill)
- **Cartas** (mestria, combos descobertos, combos secretos)
- **Lore** (mundo, facções, settings, eventos)
- **Companions** (perfis, arcs, mini-quests)
- **Diagnóstico de hardware** (status do triângulo do Gus — vai mostrar degradação sutil pela sabotagem de Dante; foreshadowing)

### Skill Trees (7 paralelas)

Uma por personagem da party (Gus + 6 companions). Auto-mode + manual. XP individual em combate + boost narrativo offscreen.

### Dificuldade Normal / Hard

- **Normal default**: game over reload (acessível).
- **Hard unlock pós-zerar**: permadeath + kernel panic puzzle.

### Foreshadow Dante traidor

D híbrido (P3 / narrative):
- Foreshadow narrativo: dialogue/comportamento estranho ao longo do jogo
- Foreshadow mecânico: stats do Gus degradam sutilmente (telemetria comprometida)
- Reveal climax explica retroativamente

---

## Como usar este documento

- **Toda feature proposta** responde explicitamente: "qual pillar serve?"
- **Decisões dos pillars são one-way doors** — só revisar com aprovação explícita do criador supremo.
- **Anti-pillars são checklist de rejeição** em design review.
- **Sistemas-âncora** ganham doc próprio assim que entrarem em produção; este aqui é o índice de referência.

**Última revisão:** 2026-05-15. Próxima revisão prevista: após vertical slice (5 playtests externos + dados de perf).
