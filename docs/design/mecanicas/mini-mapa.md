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
| **3** | **VRAM extra** (chip minúsculo) | acaba o **OOM** da GPU que renderiza o mapa → **desliga o glitch/fog** de área distante | maior | maior |

- **Todas já destravadas (mundo aberto, gdd §7.1):** a ordem 1→2→3 tem distância+dificuldade crescentes, mas NÃO é obrigatório seguir a ordem.
- **O implante e o Gus (Pillar 3):** os membros da party usam o implante real; o **Gus faz reverse-engineering (RE) no hardware dos implantes da party e cria um app que EMULA a função do implante nos óculos dele.** (O Gus não põe implante; ele hackeia/emula, coerente com os óculos táticos + o triângulo de hardware.)

### 0.1. "Não existem tesouros; existem REPOSITÓRIOS PERDIDOS" (insight canônico, expandido 2026-07-14)

Baús/tesouros clássicos não existem. Detalhamento canônico ([[project_repositorios_perdidos_canon]]):
- **TODAS as dungeons SÃO repositórios perdidos / corrompidos** (as 13 de `mundo-topologia.md`).
- **O loot nunca é dinheiro/joia.** É **peça de hardware**, **código compilável** (mais barato) ou **algoritmo** (mais caro), **gravado no SSD da party** até ser **vendido** (vira Crédito) ou **usado**.
- **Quase tudo é pra vender.** Os **ÚNICOS itens usáveis pela party** são os **3 componentes do mini-mapa: o implante, a VRAM e a skill de marcadores**. Todo o resto (demais hardware/código/algoritmo) é ativo só-pra-vender.
- **Vender/recomprar vale pra implantes E skills.**
- **Axiologia econômica canon:** *"não se acha dinheiro, se faz dinheiro com esforço e trabalho duro"* — o jogador acha ATIVOS e os converte em Crédito por trabalho (explorar/decifrar/vender). Reforça [[project_axiologia_canonica]].

Casa com Pillar 2 (magia = software) e a compilação-no-cast das cartas.

## 1. Forma

**Ambos**, com config no menu (mas ver §0: nada aparece antes da missão 1):
- **Mapa cheio no TAB:** TAB abre o mapa grande da área (o tilemap real em escala menor).
- **Mini-mapa de canto:** ligável/desligável nas opções.
- **Faseável:** se o escopo apertar, mapa-TAB primeiro, mini-mapa de canto depois.

## 2. Revelação + fog (com causa diegética)

- **Fog of war (revela ao andar):** área não-visitada = escondida; revela conforme a party caminha.
- **Fog/névoa de área distante = OOM de VRAM:** a GPU dos implantes/óculos tem **pouca VRAM** e sofre **OOM** ao renderizar o mapa longe → o distante aparece **corrompido** (ver §3). **A missão 3 (VRAM extra) resolve o OOM e desliga esse fog distante.**
- **Gate por dificuldade:** no **Fácil**, a **VRAM extra vem JUNTO com o implante na missão 1** (sem animação extra — é só citado em diálogo, algo como *"Veja, tem uns chips de vram aqui, podemos usar nos nossos implantes e você no seu óculos, Gus."*). Logo, no Fácil não há glitch/fog distante desde cedo. Do **Médio pra cima**, a VRAM é a missão 3 separada, então o fog distante persiste até achá-la. (A MESMA fala de diálogo aparece nos outros níveis quando a party enfim encontra a VRAM extra.)

## 3. Estética do fog: DADO CORROMPIDO por OOM (não nuvem) — decisão do criador

O fog distante **não é nuvem**: é **ruído de imagem corrompida** — pixels mal-posicionados/mal-gerados, como um **bitmap/arquivo corrompido** (glitch art / datamoshing / pixel-sorting), **porque é literalmente um OOM de VRAM** na GPU do implante. Tematiza Pillar 2 (magia = software; anomalia = bug) com causa mecânica real. Ao achar a VRAM (missão 3), o glitch "resolve" e a imagem real aparece.
- **Custo:** BARATO-MÉDIO — shader de corrupção ou overlay de pixels embaralhados sobre os tiles distantes (glintfx pode ajudar no efeito de tela). Sem partícula/animação pesada.

## 4. Marcadores (skill destravada pela missão 2)

Depois de compilar o repositório (missão 2), aparecem no mapa:
- **Sempre:** party; entradas/saídas entre áreas; objetivo atual; **save points / zonas PEM-Faraday**.
- **Pontos de interesse JÁ descobertos:** dungeons, os 20 interiores de mestre, atalhos (pontes do Euler).
- **Segredos "?" — EARNED:** um "?" só aparece num local quando **uma carta/poder/NPC avisa** que há algo ali (ex.: cartas-lente Turing/Dee/Bastiat). NÃO há "?" de todos os segredos por padrão. Ao descobrir, o "?" some. Sinergia com as cartas-lente.
- **Missões (principal/paralela):** **glow neon** ao redor da **área próxima** do objetivo, **sem** o local exato.
- **Config:** o jogador escolhe o que aparece no mini-mapa, mas **NUNCA** há opção de ligar TODOS os segredos (sem cheat "revelar tudo").
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

## 8. Fios abertos — VÁRIOS brainstorms pedidos pelo criador
- Conteúdo de cada uma das **3 missões paralelas** (o mistério/puzzle de cada, onde ficam nas 13 áreas, quais NPCs).
- O **repositório perdido** como mecânica geral (o que é, como se compila, quantos há no jogo, o que mais destravam além dos marcadores).
- Números: preço de recompra do implante + custo da assinatura Hard (economy-designer).
- Zoom do mapa-TAB (ver o mundo todo / as 13 áreas?); mini-mapa de canto (raio/rotação/norte-fixo).
- Transição visual do glitch "resolvendo" ao achar a VRAM (barata).
- Ícones concretos dos marcadores + arte do slot de VRAM (trilhas de cobre) + acessibilidade (daltonismo).
- Diálogo canônico da fala da VRAM ("uns chips de vram aqui…") — redação final via `narrative-writer`.
- Interação com a venda: se vender o implante, o que acontece com a VRAM/skill já compiladas (perde tudo? só o mapa?).
