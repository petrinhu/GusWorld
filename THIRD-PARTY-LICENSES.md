# Licenças de terceiros

Este arquivo lista as dependências de terceiros do GusWorld e suas licenças.

| Dependência | Licença | Titular | Repositório |
|---|---|---|---|
| GlintFx | AGPL-3.0-or-later | petrinhu | https://github.com/petrinhu/GlintFx |
| Pixel Operator Mono (fonte) | CC0 1.0 (domínio público) | Jayvee Enaguas (HarvettFox96) | https://creativecommons.org/licenses/zero/1.0/ |
| 8values (fork/port em `docs/design/roster-analogos/8values-engine/`) | MIT | 8values | https://github.com/8values/8values.github.io |

O GusWorld assenta sobre o framework GlintFx (ver `GODS_LAWS.md`, Lei Zero). O executável final do GusWorld, ao linkar com o GlintFx, é obra combinada sob os termos da AGPL-3.0 (ver `GODS_LAWS.md` L-08 e `ASSETS-LICENSE.md`).

## Nota sobre a fonte

`resources/fonts/PixelOperatorMono.ttf` e `PixelOperatorMono-Bold.ttf` foram recuperados do projeto anterior em 25/08/2026. A licença **não foi presumida**: foi lida da tabela `name` do próprio arquivo da fonte, que declara, verbatim, *"Released by Jayvee Enaguas (HarvettFox96), licensed under a Creative Commons Zero (CC0) 1.0"*.

**CC0 é dedicação ao domínio público**, então a fonte pode ser distribuída junto com o jogo sem conflito com a AGPL-3.0-or-later do código nem com o regime de asset reservado. **A atribuição não é exigida pela CC0** — está aqui porque creditar quem fez é o certo, não porque a licença obrigue.

Ela **já era canon deste projeto antes de existir no repositório**: `docs/design/mecanicas/terminal-estetica.md` decide quais glifos usar com base no que a `PixelOperatorMono` tem, e descarta a substituição por outra face porque *"o glifo viria com o desenho de OUTRA fonte, e para box-drawing isso quebra a grade pixel"*.

## Nota sobre o 8values

`docs/design/roster-analogos/8values-engine/` guarda um **fork/port** do quiz **8values** (espectro político de 8 valores / 4 eixos, projeto original em https://github.com/8values/8values.github.io, copyright *"Copyright (c) 2020 8values. http://8values.github.io."*, MIT License), usado como **ferramenta de produção/design** para classificar o espectro político das figuras históricas do roster em `docs/design/roster-analogos/`.

**Esta ferramenta NÃO é dependência de produto.** Ela nunca roda em runtime e nunca é linkada ao binário do jogo, então cai do lado permitido da régua da LEI ZERO/`GODS_LAWS.md` L-05: *"se o arquivo sumisse da máquina do JOGADOR, o jogo deixaria de rodar? Sim → é dependência, e está proibida. Não → é ferramenta, e é permitida."* Ela entra nesta tabela por completude de atribuição, não porque compõe a obra combinada do executável.

A atribuição completa (autor e projeto original, o que foi transcrito verbatim do 8values, o que o GusWorld acrescentou, e a licença de cada arquivo da pasta) vive em `docs/design/roster-analogos/8values-engine/ATTRIBUTION.md`; o texto integral da MIT License original está em `docs/design/roster-analogos/8values-engine/LICENSE_8VALUES_ORIGINAL.txt`. Este arquivo aponta para lá em vez de duplicar.
