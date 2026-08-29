<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

> ⚠️ **DOCUMENTO HISTÓRICO, recuperado em 25/08/2026 do `gusworld_legacy`. O formato descrito aqui NÃO é o formato deste projeto.**
>
> **O que morreu:** o `.gmap` era formato **próprio do projeto anterior**. Hoje o **formato de arquivo de mapa é do GlintFx** (L-30 do `GODS_LAWS.md` deles), e nós somos **consumidores, não coautores** — a nossa é só a extensão `.gw.map` (ver `docs/tech/convencao-formatos-gw.md`). O encanamento descrito abaixo (`gusworld_app --compile-map`, `domain/map/map_serializer.hpp`) não existe e não vai existir.
>
> **Por que ele foi trazido mesmo assim:** ele registra **decisões do líder de 23/06/2026** que sobreviveram à troca de dono do formato, e que valem como insumo do pedido ao GlintFx — o mapa selado contra adulteração ("o jogador não edita o mapa para atravessar parede"), a fonte editável separada do artefato compilado, e o **identificador fixo de mapa** como amarra contra troca de mapa. Esse último conceito **existe hoje no contrato do GlintFx** com outro nome, o que prova que a decisão era boa e não morreu com o formato.
>
> **A fonte que ele descreve foi trazida junto** e está viva: `resources/maps/source/distritos_inferiores.csv`, o traçado 90×60 dos Distritos Inferiores. O `.gmap` compilado **não** foi trazido — é derivado, e o derivador não existe mais.

---

# Mapas do GusWorld (formato e pipeline)

Decisao do lider (2026-06-23): o mapa do overworld e uma **matriz de tiles**
guardada num **binario proprio `.gmap` SELADO** (anti-tamper: o
jogador nao edita o mapa pra atravessar parede), **editavel no dev via uma fonte
CSV + um compilador**.

## Lar dos arquivos

```
assets/maps/
  source/    *.csv  fonte editavel, VERSIONAVEL em git (le-se a mao)
  compiled/  *.gmap binario proprio selado, gerado pelo compilador
```

- `source/distritos_inferiores.csv`  fonte do 1o mapa (cidade ato 1, graybox).
- `compiled/distritos_inferiores.gmap`  compilado selado (regeneravel a qualquer hora).

O `.csv` e a fonte de verdade. O `.gmap` e derivado: pode ser regerado pelo comando
abaixo. (Se o build versionar so a fonte, gere o `.gmap` no passo de build/empacote.)

## Pipeline (3 partes)

1. **`TileMap`** (POCO, `domain/map/tile_map.hpp`): matriz row-major de tile-ids
   `uint16_t` (`enum TileKind`: `0=Chao 1=Parede 2=Marco 3=Entrada 4=Saida`,
   extensivel) + `tile_size` + metadados (spawn do player, portais nomeados).
   `to_tile_grid()` gera o `core::spatial::TileGrid` (livre/bloqueado) que a colisao
   do overworld JA consome (so `Parede` bloqueia).
2. **`.gmap`** (`domain/map/map_serializer.hpp`): envelope
   `MAGIC "GMAP" | LENGTH | PAYLOAD | SELO(32)`. `load_map` valida o selo ANTES da versao, rejeita futuro (forward-only)
   e adulteracao, sinalizando por valor (`MapLoadResult` Ok/SeloInvalid/Corrupt/
   VersionTooNew/Invalid). Migrators forward-only desde a v1.
3. **Compilador CSV** (`domain/map/map_csv.hpp`): `parse_csv_to_tilemap` (POCO) +
   o modo de ferramenta do app que faz o I/O de arquivo na fronteira.

## Compilar um mapa

```bash
# do diretorio GusEngine/ (ajuste o caminho do binario conforme o preset):
build/linux-release/app/gusworld_app \
  --compile-map assets/maps/source/distritos_inferiores.csv \
                assets/maps/compiled/distritos_inferiores.gmap
```

O I/O de arquivo (ler `.csv`, escrever `.gmap`) vive SO no `app/` (fronteira). A
logica de transformacao e POCO puro do `domain/` (sem dependencia de plataforma, sem fstream).

## Formato CSV

- Cada **linha de grade** = uma linha do mapa (eixo Y para baixo). Os valores sao
  tile-ids separados por virgula; cada numero = uma celula (X cresce da esquerda
  para a direita). Todas as linhas de grade tem a MESMA largura.
- **Diretivas** de metadado (prefixo `#`):
  - `#tile_size <float>`  lado da celula em unidades de mundo (default 1.0)
  - `#spawn <x> <y>`      celula de spawn do player
  - `#portal <id> <x> <y>` portal/saida nomeada (pode repetir)
- Linhas em branco e comentarios `//` (linha inteira) sao ignorados.

## Ver o mapa com a legenda de blockout

O jogo tem DUAS leituras do mesmo mapa (fatia E do DEMO-CIDADE-VESTIDA):

- **producao** (default): `Marco`, `Entrada` e `Saida` saem na cor do `Chao`, e a
  celula de `Parede` que uma peca de cenario VESTE tambem nao pinta. E o que o
  jogador ve: so chao e parede, com a ARTE marcando o lugar.
- **blockout**: a legenda inteira de volta (Marco ambar, Entrada verde, Saida azul,
  parede vestida pintando parede). Ligada por env var:

```bash
GUSWORLD_TILE_PALETTE=blockout build/linux-release/app/gusworld_app
```

Qualquer outro valor (ou a ausencia dela) cai em producao, de proposito: a tela
limpa e o estado de repouso, e mostrar a legenda e um ato deliberado. Ao conferir
tracado depois de mexer neste CSV, ligue a env var - senao as ancoras estao
invisiveis, que e exatamente o objetivo delas em producao.

## 1o mapa: Distritos Inferiores (graybox)

Derivado de `docs/design/levels/blockout-distritos-inferiores.md`, que e a
descricao VIVA do tracado - o paragrafo abaixo descreve a topologia, nao o
tamanho.

Grade **90x60 celulas x tile_size 2.0m = caixa 180x120m** (retracado na fatia A
do DEMO-CIDADE-VESTIDA, 2026-08). O texto abaixo dizia **30x20** e ficou
desatualizado desde entao: corrigido de passagem ao documentar a env var da
paleta, porque numero de grade errado num README de pipeline e do tipo que
alguem copia. Topologia hub+radiais
fiel: corredor de entrada norte -> hub (Praca da Compilacao, fonte como Marco
central; Bertoldo e a placa Era 2 como Marcos) -> ramo leste opcional com o
Terminal (Marco) -> choke (vao central) -> arena rebaixada (spawn do encontro
como Marco, cover boxes como Parede, flanco leste) -> corredor-puzzle -> saida
sul. Paredes de borda + internas. Os 5 nos de design entram como `Marco` (id 2),
a entrada como `Entrada` (3) e a saida como `Saida` (4). Reachability do gold path
(entrada -> 5 nos -> saida) verificada por flood-fill.

PLACEHOLDER: posicao/colisao, sem arte final (Placeholder-first, F2-PROD.2). A
geometria fina (rampa do choke, alturas) e validacao de travessia cronometrada
exigem o runtime nas maos do criador (blockout doc, passo 7).
