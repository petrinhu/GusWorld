# Licença dos assets (arte, música, lore in-game)

> Fronteira explícita entre **código** e **assets**. Regime decidido pelo líder supremo em 2026-08-21 (GODS_LAWS.md, L-08 e L-25).

GusWorld tem partes sob regimes diferentes. Este arquivo define qual regime cobre qual asset.

---

## Resumo

| Parte | Licença / regime | Onde |
|---|---|---|
| Código-fonte | AGPL-3.0-or-later | ver [LICENSE](LICENSE) |
| Número e regra de carta (catálogo de conteúdo) | AGPL-3.0-or-later (código) | ver "Fronteira do catálogo" abaixo |
| Arte, música, som, sprite, texto de sabor e prosa in-game | Todos os direitos reservados | ver "Assets" abaixo |
| Livros-companheiros (Vol 1 + Vol 2) | Direitos reservados (obra à parte) | ver "Livros-companheiros" abaixo |
| Marca (nome, logotipo, trade dress, nomes de personagem) | Fora de qualquer concessão de asset ou de código | ver "Marca" abaixo |

---

## Código = AGPL-3.0-or-later

Cobre todo o código-fonte vigente e os scripts de build:

- Fontes C++ (`.cpp`, `.h`, `.hpp`)
- Scripts de tooling/editor
- Scripts de build (CI, shell, empacotamento)

A licença completa está em [LICENSE](LICENSE) (texto AGPL-3.0-or-later verbatim, imutável). Titular: Petrus Alves da Silva Costa, 2026.

Motivo da escolha: o GusWorld assenta sobre o GlintFx, que é AGPL-3.0-or-later. O executável do GusWorld linkado a ele é obra combinada, e distribuir esse binário obriga a obra inteira a sair sob AGPL-3.0.

---

## Assets: todos os direitos reservados

Todo asset do jogo fica sob **todos os direitos reservados** ao titular (Petrus Alves da Silva Costa, 2026). Não é copyleft, e não concede licença automática de uso, cópia ou redistribuição a terceiros.

Cobre:

- Sprites, modelos 3D, texturas, atlas, materiais visuais
- Música, SFX, áudio
- Texto de sabor, descrição e qualquer prosa de carta
- Textos in-game (diálogos, lore exibida no jogo, descrições, UI)

Parte destes assets nasce de ferramentas de IA generativa de terceiro (PixelLab, Tripo3D, Suno, Gemini/Grok); nesses casos, os direitos reservados acima valem na medida em que os Termos de Serviço daquela ferramenta cedem a titularidade do conteúdo gerado ao criador.

---

## Fronteira do catálogo (número e regra vs. texto de sabor)

O catálogo de conteúdo (cartas, inimigos, receitas, statlines) é fatiado por natureza jurídica (GODS_LAWS.md, L-25):

- **Número e regra** (identificador, custo, poder, velocidade, efeito) são **código sob AGPL-3.0-or-later**, compilados dentro do executável. A Lei 9.610/98, artigo 8º, inciso II, exclui da proteção autoral as regras de jogo, então não há direito autoral ali para reservar.
- **Texto de sabor, descrição e qualquer prosa de carta** são **asset**, sob o regime de direitos reservados acima, e ficam fora do executável, no pacote binário selado.

Motivo: compilar conteúdo reservado dentro de um executável AGPL produziria um binário que se contradiz, anunciado como redistribuível e contendo parte que ninguém pode redistribuir. Arquivo de dado ao lado do executável é agregação consagrada; dentro do mesmo executável, a leitura da FSF é que os módulos formam um programa só.

Consequência de engenharia: o gerador de build produz **dois** artefatos, e uma carta existe em dois lugares. O selo criptográfico protege os dois igualmente.

---

## Livros-companheiros = direitos reservados

Os **dois livros-companheiros** são obra literária à parte, com **todos os direitos reservados** ao titular (Petrus Alves da Silva Costa, 2026):

- **Vol 1**: bíblia de worldbuilding
- **Vol 2**: antologia de contos

Reprodução, distribuição ou obra derivada exigem autorização escrita do titular. O fato de a lore aparecer no jogo (sob direitos reservados, ver "Assets" acima) NÃO estende licença nenhuma ao texto dos livros: são suportes distintos, com regimes distintos.

---

## Marca (carve-out)

Identidade de marca (nome do jogo, logotipo, trade dress, nomes de personagem) fica **fora de qualquer concessão**, seja de código, seja de asset. Nenhuma licença acima concede o direito de usar o nome "GusWorld", o logotipo do jogo, os nomes dos personagens, ou apresentar um fork ou derivado como se fosse o produto oficial.

Ver também [NOTICE](NOTICE), que registra este carve-out para o código.

---

## Permissão para conteúdo de fã (vídeos, streams, fan art)

O regime de direitos reservados não tira da comunidade o direito de jogar e compartilhar. Independente da parte do jogo envolvida, o titular concede permissão explícita para:

- Gravar e publicar vídeos de gameplay (Let's Plays, reviews, tutoriais, compilações)
- Fazer streaming ao vivo do jogo
- Publicar capturas de tela e clipes curtos
- Criar e compartilhar fan art, fan fiction e outro conteúdo derivado **não-comercial**, inspirado nos personagens e no mundo do jogo

Monetização padrão de plataforma (anúncios do YouTube/Twitch no vídeo do criador de conteúdo) é permitida. Isso não cobre: vender o asset original como se fosse próprio, republicar o jogo, ou uso comercial dos assets fora do contexto de conteúdo de fã acima descrito. Fan art e fan fiction seguem sujeitas ao carve-out de marca acima (não usar o nome/logo do jogo como se o conteúdo fosse produto oficial).

---

## Atribuições de terceiros

Bibliotecas e fontes de terceiros têm licenças próprias, listadas em [THIRD-PARTY-LICENSES.md](THIRD-PARTY-LICENSES.md).

---

*Recomendação técnica de fronteira de licença; a validação jurídica formal cabe ao titular, inclusive: classes de registro de marca a proteger e se o carve-out precisa de menção nominal aos nomes de personagem.*
