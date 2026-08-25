# Licenças de terceiros

Este arquivo lista as dependências de terceiros do GusWorld e suas licenças.

| Dependência | Licença | Titular | Repositório |
|---|---|---|---|
| GlintFx | AGPL-3.0-or-later | petrinhu | https://github.com/petrinhu/GlintFx |
| Pixel Operator Mono (fonte) | CC0 1.0 (domínio público) | Jayvee Enaguas (HarvettFox96) | https://creativecommons.org/licenses/zero/1.0/ |

O GusWorld assenta sobre o framework GlintFx (ver `GODS_LAWS.md`, Lei Zero). O executável final do GusWorld, ao linkar com o GlintFx, é obra combinada sob os termos da AGPL-3.0 (ver `GODS_LAWS.md` L-08 e `ASSETS-LICENSE.md`).

## Nota sobre a fonte

`resources/fonts/PixelOperatorMono.ttf` e `PixelOperatorMono-Bold.ttf` foram recuperados do projeto anterior em 25/08/2026. A licença **não foi presumida**: foi lida da tabela `name` do próprio arquivo da fonte, que declara, verbatim, *"Released by Jayvee Enaguas (HarvettFox96), licensed under a Creative Commons Zero (CC0) 1.0"*.

**CC0 é dedicação ao domínio público**, então a fonte pode ser distribuída junto com o jogo sem conflito com a AGPL-3.0-or-later do código nem com o regime de asset reservado. **A atribuição não é exigida pela CC0** — está aqui porque creditar quem fez é o certo, não porque a licença obrigue.

Ela **já era canon deste projeto antes de existir no repositório**: `docs/design/mecanicas/terminal-estetica.md` decide quais glifos usar com base no que a `PixelOperatorMono` tem, e descarta a substituição por outra face porque *"o glifo viria com o desenho de OUTRA fonte, e para box-drawing isso quebra a grade pixel"*.
