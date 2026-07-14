# Sistema de mapa / mini-mapa (GusWorld)

> **Status:** PROPOSTA (brainstorm interativo do criador, 2026-07-13/14). Design; implementação é feat seguinte (consome o `.gmap`/tile_map já existente em `GusEngine/domain/map/`). **O criador pediu VÁRIOS brainstorms de detalhe** para os pontos abaixo; este doc CAPTURA as decisões e marca o que falta.
>
> **Filtro de produção (dev solo, [[feedback_solo_baixa_infra_escopo]]):** faseável se o escopo apertar.
>
> **Âncora canônica:** o mapa real é o `.gmap` selado (`reference_formato_mapa_gmap`); o mini-mapa é uma REPRODUÇÃO em escala menor do mesmo tilemap. Cross-ref topologia (`mundo-topologia.md`), gdd §7.1 (sem gate-hard), sistema de dificuldade ([[project_morte_dificuldade_canon]]), Pillar 2 (magia = software, anomalia = bug), Pillar 3 (loop de hardware: óculos/matriz/Tavus-Drive), save por PEM/Faraday ([[project_save_dungeon_pem_faraday]]).

## 0. Princípio central: o mini-mapa é DIEGÉTICO e CONQUISTADO (decisão do criador 2026-07-14)

**O mini-mapa NÃO existe no início do jogo — mesmo com a config LIGADA.** Ele é montado peça por peça via hardware/software que a party encontra, cada camada com uma causa diegética real (Pillar 3 levado ao extremo). São **3 missões paralelas**, e cada uma destrava uma camada:

| Missão | O que encontra | O que destrava | Distância da origem | Dificuldade |
|---|---|---|---|---|
| **1** | **O implante** (implante removível de visão) | o **mini-mapa em si** aparece | menor | menor |
| **2** | Um **repositório perdido** (ver §0.1) com código compilável | a **skill de marcar pontos** no mapa (os marcadores/POIs) | média | média |
| **3** | **VRAM extra** (chip minúsculo, montado a partir do repositório) | acaba o **OOM** da GPU que renderiza o mapa → **desliga o glitch/fog** de área distante | **média** (filial da Montadora) | maior |

- **Todas já destravadas (mundo aberto, gdd §7.1):** a ordem 1→2→3 tem distância+dificuldade crescentes, mas NÃO é obrigatório seguir a ordem.
- **Lotação nos repositórios existentes (decisão do criador 2026-07-14):** as 3 são a recompensa especial de 3 pontos do mapa (reuso, barato):
  - **M1 (implante) → Ferrovelhos** (T2): ferro-velho onde se cata hardware usado; um implante de visão salvado. **É um LUGAR SECRETO do Ferrovelhos (puzzle de reparo + diálogo), NÃO a dungeon de batalha "reduto do Dante" (achado PS-Y10):** a dungeon principal do Ferrovelhos é só-batalhas; o M1 fica à parte, coerente com "repositório leve = lugar secreto".
  - **M2 (código dos marcadores) → Zona do Silêncio** (T3): zona morta/abandonada = repositório de código perdido, silencioso e corrompido.
  - **M3 (VRAM) → FILIAL da Montadora (decisão do criador 2026-07-14, achado PS-Y9):** NÃO na Montadora principal (Tier 4, quase-fim), mas numa **filial de distância MÉDIA** cuja função é **fabricar chips para a fábrica principal**. Assim o conserto do fog chega mais cedo (o Médio+ não navega o jogo todo com névoa). Para **montar o chip de VRAM**, a party precisa de UMA de duas vias (gate dado pelo **gerente da filial**):
    - **(a) Pagar** pela fabricação: **CARO** para o momento do jogo (valor exato = economy-designer; ver `LOOT-REPOSITORIOS` / TODO), OU
    - **(b) Mini-missão de sucata:** buscar **materiais de sucata com sobra** que justifiquem o gasto da empresa (a filial fabrica de graça se a party traz o insumo excedente). Casa com o Pillar econômico ("faz-se dinheiro/valor com trabalho").
- **Reconciliação 13-vs-14 dungeons — RESOLVIDA (decisão do criador 2026-07-14, achados PS-R4+PS-R5):** as 3 missões do mini-mapa são **repositórios LEVES** (tier "lugar secreto": 1-2 salas + 1 puzzle), **NÃO dungeons principais**. Consequências:
  - As **13 dungeons de `mundo-topologia.md` §4 permanecem 13** (o Fibonacci-13 velado fica intacto).
  - A **Zona do Silêncio mantém suas 2 dungeons** (#1 puzzle acústico / #2 mista); o **M2** é um **repositório leve** ao lado delas, não um 3º slot de dungeon.
  - A **filial da Montadora** ganha **1 repositório leve visitável** (o M3), sem virar 14ª dungeon; a **Montadora principal** segue "lore/quest, não-dungeon" (encontro do Tusk, endgame).
- **Fog por marco NÃO — a VRAM segue sendo o conserto de vez (PS-Y9):** o criador escolheu ADIANTAR a VRAM (filial média) em vez de fatiar o raio da névoa por marco. O fog continua binário (liga/desliga), só que a chave chega mais cedo.
- **Mapa-TAB da ÁREA ATUAL por dificuldade (decisão do criador 2026-07-14, achado PS-Y11):**
  - **Fácil:** o mapa-TAB da área ONDE o jogador está fica **liberado desde o início** (só o tilemap cru, sem marcadores, sem mundo-múndi). Resolve a janela inicial sem navegação.
  - **Médio+ (Médio/Difícil/Hardcore):** **zero mapa até achar o implante (M1)** — a conquista diegética vale integral. Consistente com o resto da escala de dificuldade (ex.: névoa só no Médio+).
- **O implante e o Gus (Pillar 3):** os membros da party usam o implante real; o **Gus faz reverse-engineering (RE) no hardware dos implantes da party e cria um app que EMULA a função do implante nos óculos dele.** (O Gus não põe implante; ele hackeia/emula, coerente com os óculos táticos + o triângulo de hardware.)

### 0.1. "Não existem tesouros; existem REPOSITÓRIOS PERDIDOS" (insight canônico, expandido 2026-07-14)

Baús/tesouros clássicos não existem. Detalhamento canônico ([[project_repositorios_perdidos_canon]]):
- **TODAS as dungeons SÃO repositórios perdidos / corrompidos** (as 13 de `mundo-topologia.md`).
- **O loot nunca é dinheiro/joia.** É **peça de hardware**, **código compilável** (mais barato) ou **algoritmo** (mais caro), **gravado no SSD da party** até ser **vendido** (vira Crédito) ou **usado**.
- **Dois pools distintos de drop (decisão do criador 2026-07-14, achado PS-R3):** não confundir os dois sistemas canônicos:
  - **Pool A — loot de repositório/dungeon:** hardware/código/algoritmo grande, **só-pra-vender** (Pillar "faz-se dinheiro com trabalho"), exceto os 3 do mini-mapa. É o que este doc descreve.
  - **Pool B — insumos de craft:** os **18 ingredientes** dropáveis de `economia.md` (§ craft F3-Alpha) que **entram em receita** e são **usáveis via compilação**. Não são "loot de repositório vendável"; são matéria-prima de craft.
  - Regra prática: se um drop **entra numa receita**, é Pool B (craft); senão, é Pool A (vender). Os 3 do mini-mapa são a exceção nomeada do Pool A (usáveis, não vendáveis por padrão).
- **Quase tudo do Pool A é pra vender.** Os **ÚNICOS itens do Pool A usáveis pela party** são os **3 componentes do mini-mapa: o implante, a VRAM e a skill de marcadores**. Todo o resto do Pool A (demais hardware/código/algoritmo) é ativo só-pra-vender.
- **Vender/recomprar vale pra implantes E skills.**
- **Axiologia econômica canon:** *"não se acha dinheiro, se faz dinheiro com esforço e trabalho duro"* — o jogador acha ATIVOS e os converte em Crédito por trabalho (explorar/decifrar/vender). Reforça [[project_axiologia_canonica]].

Casa com Pillar 2 (magia = software) e a compilação-no-cast das cartas.

### 0.2. Conteúdo das 3 missões (coração de cada, decidido 2026-07-14; detalhe fino depois)

- **M1 (implante, Ferrovelhos): reparar / RE um implante quebrado.** A party acha um implante de visão DANIFICADO no ferro-velho; o Gus faz o **reverse-engineering** dele (mini-puzzle de reparo/decifração) e então **emula a função nos óculos** (Pillar 3, RE, software>hardware, a cara do Gus). Barato: 1 puzzle + diálogo.
- **M2 (código dos marcadores, Zona do Silêncio): missão em 3 camadas.** (1) **atravessar** as ruínas da zona morta (labirinto, mecânica de silêncio/som-não-propaga) até o servidor; (2) **religar o sinal** (a zona é sinal-morto); (3) **decifrar e compilar** o repositório corrompido → a skill de marcadores. Casa com "repositório = código a compilar" + as cartas-lente (Turing).
- **M3 (VRAM, Montadora Confluência): FORJAR o chip.** A VRAM não é achada pronta: a party opera a forja/linha do Tusk pra **fabricar** o chip, com **sucata + materiais de ponta cedidos pelo Tusk + equipamento emprestado**. Amarra na axiologia "não se acha, se FAZ com trabalho" + conecta Ferrovelhos→Montadora.
  - **Clarificação canônica (criador):** as fábricas do Tusk usam sucata pra **baratear a produção E ser eco-friendly**, MAS **não usam só sucata** (são indústrias de tecnologia de PONTA; combinam sucata reciclada + materiais de ponta).
  - **Beat cômico (criador):** em algum ponto aqui, o **Gus se queima de leve na solda elétrica** (perde só **1 HP**, diebegético/humor) e solta uma **rage-line** de hardware (ex.: *"Ai! Viu? É por isso que eu CODO. Software não queima o dedo de ninguém."*; cross-ref o banco de rage em `vozes-party.md`).

## 1. Forma + comportamento visual (decidido 2026-07-14)

**Ambos**, com config no menu (mas ver §0: nada aparece antes da missão 1):
- **Mapa cheio no TAB — DOIS NÍVEIS:** TAB abre o mapa da **ÁREA atual** (tilemap real em escala menor); um botão/scroll afasta pro **mapa-múndi das 13 áreas** (nós conectados + distância-dificuldade visível). O mapa-múndi é barato (nós+linhas, não tiles).
- **Mini-mapa de canto: NORTE-FIXO**, raio pequeno ao redor da party (party = ponto/seta no centro). Norte sempre pra cima (mais legível + mais barato que rotacionar). Ligável/desligável nas opções.
- **Faseável:** se o escopo apertar, mapa-TAB primeiro, mini-mapa de canto depois.

## 2. Revelação + fog (com causa diegética)

- **Fog of war (revela ao andar):** área não-visitada = escondida; revela conforme a party caminha.
- **Fog/névoa de área distante = OOM de VRAM:** a GPU dos implantes/óculos tem **pouca VRAM** e sofre **OOM** ao renderizar o mapa longe → o distante aparece **corrompido** (ver §3). **A missão 3 (VRAM extra) resolve o OOM e desliga esse fog distante.**
- **Gate por dificuldade:** no **Fácil**, a **VRAM extra vem JUNTO com o implante na missão 1** (sem animação extra — é só citado em diálogo, algo como *"Veja, tem uns chips de vram aqui, podemos usar nos nossos implantes e você no seu óculos, Gus."*). Logo, no Fácil não há glitch/fog distante desde cedo. Do **Médio pra cima**, a VRAM é a missão 3 separada, então o fog distante persiste até achá-la. (A MESMA fala de diálogo aparece nos outros níveis quando a party enfim encontra a VRAM extra.)

## 3. Estética do fog: DADO CORROMPIDO por OOM (não nuvem) — decisão do criador

O fog distante **não é nuvem**: é **ruído de imagem corrompida** — pixels mal-posicionados/mal-gerados, como um **bitmap/arquivo corrompido** (glitch art / datamoshing / pixel-sorting), **porque é literalmente um OOM de VRAM** na GPU do implante. Tematiza Pillar 2 (magia = software; anomalia = bug) com causa mecânica real.
- **Transição ao descobrir (decidido 2026-07-14):** ao revelar/achar a VRAM, o glitch faz um **de-glitch rápido tipo "compilar/carregar"** — os pixels corrompidos se assentam no lugar em ~0.5s, como um arquivo terminando de descomprimir (o dado "compila" e vira a imagem real, Pillar 2). Barato: um lerp curto de shader / poucos frames.
- **Custo:** BARATO-MÉDIO — shader de corrupção ou overlay de pixels embaralhados sobre os tiles distantes (glintfx pode ajudar no efeito de tela). Sem partícula/animação pesada.

## 4. Marcadores (skill destravada pela missão 2)

Depois de compilar o repositório (missão 2), aparecem no mapa:
- **Sempre:** party; entradas/saídas entre áreas; objetivo atual; **save points / zonas PEM-Faraday**.
- **Pontos de interesse JÁ descobertos:** dungeons, os 20 interiores de mestre, atalhos (pontes do Euler).
- **Segredos "?" — EARNED:** um "?" só aparece num local quando **uma carta/poder/NPC avisa** que há algo ali (ex.: cartas-lente Turing/Dee/Bastiat). NÃO há "?" de todos os segredos por padrão. Ao descobrir, o "?" some. Sinergia com as cartas-lente.
- **Missões (principal/paralela):** **glow neon** ao redor da **área próxima** do objetivo, **sem** o local exato.
- **Config:** o jogador escolhe o que aparece no mini-mapa, mas **NUNCA** há opção de ligar TODOS os segredos (sem cheat "revelar tudo").
- **Estilo dos ícones (decidido 2026-07-14): glífos tech/software minimalistas** (monocromáticos, estilo símbolo de terminal/app; ex.: dungeon = colchetes, save = disquete, mestre = glífo de código, "?" = query). Legíveis por FORMA (não só cor → acessível a daltônicos), baratos (sprites pequenos/vetoriais), coerentes com magia=software e a estética do cockpit.
- **Hard — assinatura:** no Difícil/Hardcore, a skill de marcar pontos funciona por **assinatura**: se ficar **ligada o tempo todo, gasta Créditos do personagem mais rápido** (incentiva ligar/desligar).

## 5. VRAM: item, slot e peso (decisão do criador)

- **Slot específico no inventário pra a VRAM**, com **arte de trilhas de cobre se interligando** no slot (estética PCB).
- **Peso da VRAM extra = 0** (chip minúsculo, tecnologia avançadíssima).

## 6. PEM / Faraday (decisão do criador)

O **efeito PEM das dungeons** (que já bloqueia save, [[project_save_dungeon_pem_faraday]]) **TAMBÉM desativa o mini-mapa / VRAM / skill de marcadores** (a GPU do implante apanha do pulso). A **carta Faraday (EM-Shield)** evita esse efeito ruim — reforça o valor da carta (não protege só o save, protege o mini-mapa também). Cross-ref `_EFEITOS-ESCOLHIDOS.md` (Faraday).

## 7. Economia (handoff economy-designer)

- **Vender os implantes:** a party PODE vender os implantes, com **trade-off de perder a feature do mini-mapa**.
- **Recompra:** depois dá pra comprar de volta no **comércio de um bairro específico**, por **preço alto o bastante pra ser impossível comprar no início do jogo**. **Handoff `economy-designer`** pra achar esse preço (ancorado na curva de Crédito).
- **Assinatura Hard** (§4): custo por tempo-ligado da skill de marcadores — também calibrar com o economy-designer.

## 7.5. Persistência no save (requisito FORTE do criador 2026-07-14: "o engine de save deve ser muito robusto!")

**TODO o progresso do mini-mapa/loot fica no save do jogador — nada se perde** (exceto a penalidade intencional de morte, ex.: Hardcore permadeath). Campos que o schema do save passa a guardar:
- **Hardware/skills:** implante por membro da party + o app emulado do Gus; VRAM; skill de marcadores (compilada); implantes/skills vendidos ou recomprados.
- **Exploração:** fog-of-war por área (bitmap de tiles descobertos); "?" de segredos já resolvidos; POIs achados.
- **Inventário:** conteúdo do SSD (loot de hardware/código/algoritmo em posse); Créditos.
- **Config:** mini-mapa on/off, o que os marcadores mostram, nível de zoom preferido.
- **Estado:** modo de dificuldade; assinatura Hard dos marcadores (ligada/desligada + saldo gasto).

**Fundação robusta que já existe (herdada, não reinventar):** envelope **AEAD** (XChaCha20/Monocypher, ADR-015, [[reference_save_crypto_v2]]) cifra+autentica; `save_version` versionado com **migrators** desde o D1 (campo novo = bump + migrator); **recuperação por backup** + aviso de save danificado/versão-incompatível (commit `db9a185`); **fail-secure** (save duvidoso nunca carrega silenciosamente). Feat do `backend-engineer`: estender o schema do save-domain com estes campos (bump de `save_version` + migrator + teste de roundtrip/tamper).

## 8. Fios abertos — VÁRIOS brainstorms pedidos pelo criador

Fios VISUAIS resolvidos 2026-07-14: mapa-TAB dois níveis (§1); mini-mapa de canto norte-fixo (§1); transição do glitch = de-glitch "compilar" ~0.5s (§3); ícones = glífos tech minimalistas (§4); persistência no save (§7.5).

Fios das 3 MISSÕES resolvidos 2026-07-14: lotação (M1 Ferrovelhos / M2 Zona do Silêncio / M3 Montadora, §0) + coração de cada (§0.2) + **reconciliação 13-vs-14 dungeons** (RESOLVIDA — missões do mini-mapa = repositórios leves fora das 13 principais, ver §0). Resta: puzzle fino de cada, NPCs envolvidos, blocking.

Restam:
- O **repositório perdido** + **loot** como mecânica geral (o que é, como se compila; tipos+valores de hardware; inventário de códigos/algoritmos: quantos, quais, nomes, efeitos).
- Números (economy-designer): preço de recompra do implante/skills + custo da assinatura Hard + preços de venda do loot.
- Arte concreta: os glífos de cada marcador + o slot de VRAM (trilhas de cobre); paleta acessível (daltonismo).
- Diálogo canônico da fala da VRAM ("uns chips de vram aqui…") — redação final via `narrative-writer`.
- Interação com a venda: se vender o implante, o que acontece com a VRAM/skill já compiladas (perde tudo? só o mapa?).
- Feat de código (`backend-engineer`): estender o schema do save com os campos de §7.5 (bump `save_version` + migrator + teste roundtrip/tamper).
