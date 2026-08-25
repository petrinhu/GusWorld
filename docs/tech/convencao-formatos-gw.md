<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Convenção de formatos próprios: `.gw.<tipo>`

> **Status:** decisão do líder, 24/08/2026, itens `G2` e complemento sobre mapa no mesmo dia. Este documento é a fonte canônica da convenção; a tabela de pendências aponta para ele (L-30).

## A regra, verbatim do líder

> *"`.gw.text` e mantenha o padrão de `.gw.[tipo]` para nossos formatos próprios"*

Todo arquivo de formato **próprio do GusWorld** usa a extensão composta `.gw.<tipo>`. O prefixo `gw` identifica o projeto; o `<tipo>` diz o que é.

## O que a convenção NÃO decide

⚠️ **Extensão não é formato.** A convenção nomeia o arquivo; ela não afirma que o conteúdo foi desenhado por nós. Os dois casos abaixo mostram por que a distinção importa, e não é sutileza:

| Extensão | Quem desenhou o conteúdo | Observação |
|---|---|---|
| `.gw.text` | **nós** | fonte de diálogo e texto; formato próprio de verdade, especificação em `B9` |
| `.gw.map` | **o GlintFx** | ver abaixo |

## O caso do mapa, decidido em 24/08/2026

O líder determinou que os arquivos de mapa do GusWorld usem a extensão **`.gw.map`**, seguindo o padrão. Ao registrar isso surgiu um conflito aparente, levado a ele e resolvido no mesmo ato:

**A L-30 do `GODS_LAWS.md` do GlintFx** (decisão do líder, 21/08/2026) diz que **o GlintFx é dono do formato de arquivo de mapa** — a lib publica o formato e o carregador, e o editor e o jogo são consumidores. A nossa própria tabela já registrava, de 22/08: *"Ele é dono do formato; nós somos consumidores, não coautores."*

**A decisão, verbatim na escolha do líder:** *extensão nossa, formato do GlintFx.* Ou seja:

- O arquivo **se chama** `.gw.map`, coerente com a convenção da casa.
- O **conteúdo é o formato que o GlintFx publica**, sem alteração nossa.
- A L-30 do GlintFx **fica intacta**, e o contrato de mapa não é reaberto.

**Por que isto importa na prática, e não é formalidade:** a sessão do `gusworld_mapeditor` está construindo o editor **sobre o formato do GlintFx**, declarando-se *"implementador de referência do escritor"*. Um formato paralelo nosso criaria dois formatos incompatíveis — o que o editor escreve e o que o jogo lê — e o editor de mapas do projeto deixaria de servir ao jogo. Como o formato do GlintFx é tratado lá como **API pública e contrato de compatibilidade binária**, com revisão dedicada e versionamento, um formato próprio também jogaria fora esse trabalho e recriaria em casa o que a LEI ZERO manda pedir ao framework.

## Consequência para tipos futuros

Ao criar qualquer formato novo, o nome segue `.gw.<tipo>` — **e a pergunta de quem é dono do conteúdo é feita separadamente**, antes de escrever a primeira linha. Se o conteúdo for do GlintFx (LEI ZERO), somos consumidores e a extensão é só nossa etiqueta; se for nosso, o formato precisa de especificação escrita, como o `B9` está fazendo para o `.gw.text`.

**A armadilha que este documento existe para impedir:** ver `.gw.` no nome de um arquivo e concluir que o formato é nosso, e daí sentir-se livre para mudá-lo.

## Nada disso muda o que chega ao jogador

Todo formato acima é **fonte dentro do repositório**. A distribuição não leva texto: o jogador recebe tabelas compiladas e pacotes binários selados (L-18 e L-25, reafirmado pelo líder em 24/08 — *"quero tudo binário após compilar"*).
