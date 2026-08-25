<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Comparativos visuais de fonte (`fontflip`)

> ⚠️ **REGISTRO HISTÓRICO, recuperado em 25/08/2026 do `gusworld_legacy`.** São **22 capturas de tela** de uma investigação de renderização de fonte feita no **motor anterior**, que não existe mais. O encanamento que as produziu morreu; **o que elas mostram, não**.
>
> **Por que foram trazidas mesmo assim** (ordem do líder: *"arquivos de imagem e som traga todos de volta"*): elas são **medição visual**, e medição não envelhece com a troca de motor. Registram como a `PixelOperatorMono` se comporta em tamanho pequeno, com acento, em três resoluções-alvo, e o quanto o texto muda entre dois tamanhos de base. Quando a camada de tela nascer, isso é ponto de partida em vez de investigação a refazer.

## O que há aqui

- **11 comparativos `cmp_1x_*`** — o mesmo elemento de interface renderizado para comparação: texto de cockpit, rótulo, registro, texto de verbo e de mira, nota de derrota, texto de painel, bark de derrota, nome no cockpit, reinício, texto flutuante e banner de introdução.
- **1 comparativo de acento ampliado** (`cmp_1x_08_ZOOM_acentos`) — o caso que mais morde em pt-br, onde o acento some ou colide na grade de pixel.
- **6 comparativos de densidade** (`cmp_dpi_*`) — a mesma tela em **1080p, 4K e Steam Deck**, com duas bases de tamanho (08 e 15).
- **5 tripletos** (`triplet_1x_*`) — três variantes lado a lado do mesmo elemento.

## O que NÃO se pode concluir daqui

⚠️ Estas imagens saíram de um renderizador que **não é o nosso**. O desenho da fonte é o mesmo (a `PixelOperatorMono` está em `resources/fonts/`, CC0), mas **posicionamento, hinting e escala dependem de quem desenha** — e quem vai desenhar é o **GlintFx**. Trate como **hipótese a reconfirmar**, nunca como resultado válido para a camada de tela que ainda não existe (L-06, L-27).

**Decisão de design que consumiu esta investigação:** `docs/design/mecanicas/terminal-estetica.md`.
