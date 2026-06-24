# Mapas do GusWorld (formato e pipeline)

Decisao do lider (2026-06-23): o mapa do overworld e uma **matriz de tiles**
guardada num **binario proprio `.gmap` SELADO com HMAC-SHA256** (anti-tamper: o
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
   `MAGIC "GMAP" | LENGTH | PAYLOAD | HMAC-SHA256(32)`, mesma disciplina do save
   (ADR-006). `load_map` valida o HMAC ANTES da versao, rejeita futuro (forward-only)
   e adulteracao, sinalizando por valor (`MapLoadResult` Ok/HmacInvalid/Corrupt/
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
logica de transformacao e POCO puro do `domain/` (sem SDL, sem fstream).

## Formato CSV

- Cada **linha de grade** = uma linha do mapa (eixo Y para baixo). Os valores sao
  tile-ids separados por virgula; cada numero = uma celula (X cresce da esquerda
  para a direita). Todas as linhas de grade tem a MESMA largura.
- **Diretivas** de metadado (prefixo `#`):
  - `#tile_size <float>`  lado da celula em unidades de mundo (default 1.0)
  - `#spawn <x> <y>`      celula de spawn do player
  - `#portal <id> <x> <y>` portal/saida nomeada (pode repetir)
- Linhas em branco e comentarios `//` (linha inteira) sao ignorados.

## 1o mapa: Distritos Inferiores (graybox)

Derivado de `docs/design/levels/blockout-distritos-inferiores.md` (F2-G.1).
Grade **30x20 celulas x tile_size 2.0m = caixa ~60x40m**. Topologia hub+radiais
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
