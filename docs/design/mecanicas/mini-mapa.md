# Sistema de mapa / mini-mapa (GusWorld)

> **Status:** PROPOSTA (brainstorm interativo do criador, 2026-07-13/14). Design; implementação é feat seguinte (consome o `.gmap`/tile_map já existente em `GusEngine/domain/map/`).
>
> **Filtro de produção (dev solo, [[feedback_solo_baixa_infra_escopo]]):** faseável se o escopo apertar.
>
> **Âncora canônica:** o mapa real é o `.gmap` selado (`reference_formato_mapa_gmap`); o mini-mapa é uma REPRODUÇÃO em escala menor do mesmo tilemap. Cross-ref topologia (`mundo-topologia.md`), gdd §7.1 (sem gate-hard), sistema de dificuldade ([[project_morte_dificuldade_canon]]), Pillar 2 (magia = software, anomalia = bug).

## 1. Forma (decisão do criador)

**Ambos**, com config no menu:
- **Mapa cheio no TAB:** TAB abre o mapa grande da área (o tilemap real em escala menor) por cima da tela.
- **Mini-mapa de canto:** um mini-mapa pequeno num canto, **ligável/desligável nas opções**.
- **Faseável:** se o escopo apertar, entregar o mapa-TAB primeiro e o mini-mapa de canto depois.

## 2. Revelação + névoa (decisão do criador)

- **Fog of war (revela ao andar):** área não-visitada = escondida; revela conforme a party caminha.
- **Névoa de dificuldade:** áreas **distantes já descobertas** voltam a ficar sob névoa no mini-mapa. **Gated por modo de dificuldade:** no **Fácil, SEM névoa** (descoberto = sempre claro); do **Médio pra cima**, a névoa cobre as áreas distantes descobertas. (Casa com dificuldade-por-distância + gdd §7.1: aviso diegético de "longe/perigoso", nunca "nível X".)

## 3. Estética da névoa/fog: DADO CORROMPIDO (não nuvem) — decisão do criador

O fog e a névoa **não são nuvem**: são **ruído de imagem corrompida** — pixels mal-posicionados/mal-gerados, como um **bitmap/arquivo corrompido** (glitch art / datamoshing / pixel-sorting). Tematiza Pillar 2 (magia = software; o não-computado/não-descoberto aparece como **dado corrompido**, anomalia = bug). Ao descobrir, o glitch "resolve" e a imagem real aparece.
- **Custo:** BARATO-MÉDIO — um shader de corrupção ou overlay de pixels embaralhados sobre os tiles velados (o glintfx pode ajudar no efeito de tela). Sem partícula/animação pesada.

## 4. Marcadores (decisão do criador)

Aparecem no mapa:
- **Sempre:** party; entradas/saídas entre áreas; objetivo atual; **save points / zonas PEM-Faraday** (onde dá ou não pra salvar).
- **Pontos de interesse JÁ descobertos:** dungeons, os 20 interiores de mestre, atalhos (ex.: pontes do Euler).
- **Segredos "?" — EARNED, não automáticos:** um "?" só aparece num local específico quando **uma carta/poder/NPC avisa** que há algo ali (ex.: cartas-lente Turing/Dee/Bastiat, ou dica de NPC). **NÃO** existe "?" de todos os segredos por padrão. Quando a party descobre o que havia ali, **o "?" some**. Sinergia: achar segredo é papel das cartas-lente, não do mapa entregar tudo.
- **Missões (principal/paralela):** um **glow neon** envolve a **área próxima** do objetivo, **sem revelar o local exato** (orienta a região, não o ponto).

**Config:** o jogador escolhe nas opções o que aparece no mini-mapa, MAS **nunca** há opção de ligar TODOS os segredos no mini-mapa (sem cheat de "revelar tudo").

## 5. Fios abertos (continuar brainstorm)
- Níveis de zoom do mapa-TAB (a área toda? dá pra afastar e ver as 13 áreas / o mundo?).
- O mini-mapa de canto: raio/escala, rotaciona com a party ou norte-fixo.
- Como o glitch "resolve" visualmente ao descobrir (transição barata).
- Ícones concretos de cada marcador (arte barata).
- Interação com os 20 interiores de mestre (aparecem como ? até avisados, ou como ícone ao descobrir?).
- Legenda / acessibilidade (daltonismo nas cores dos marcadores).
