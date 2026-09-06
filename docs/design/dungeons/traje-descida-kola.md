<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: NONE — regime reservado (L-08 de GODS_LAWS.md; docs/design/ fica fora do
código, sob o mesmo regime de `narrative/`, `specs/`, `CHARS.md`, `PLACES.md`, `sinopse.md`).
-->

# Especificação do campo de descida da Kola-SG3-12262, "o traje" (`D25`)

> **Documento inteiro PÚBLICO.** Nada aqui revela o que se descobre no fim da dungeon nem como o
> confronto final termina — o critério do líder que já rege o split do hub da dungeon (verbatim:
> *"um leitor do repositório público pode saber que a dungeon existe, onde fica, como se chega e o
> que a missão pede; não pode saber o que se descobre no fim nem como termina"*) classifica este
> item do lado público: o traje é sobre COMO A PARTY CHEGA ao fundo da caverna, não sobre o que ela
> encontra lá.
>
> **Fonte:** decisão do líder, verbatim, 06/09/2026 (ver seções 1-5) e
> `docs/_secret/dungeons/CAPTURA-BRUTA-kola-espelho-obsidiana.md` item 12 (as três impossibilidades
> físicas medidas — cifrada por estar no mesmo diretório de apoio à dungeon, só o caminho é
> citável, L-25).
>
> **Cross-refs:** `docs/design/dungeons/kola-sg3-12262-espelho-obsidiana.md` (hub da dungeon, beat
> 2) · `docs/design/roster-analogos/21-helion-tusk.md` (Helion Tusk) ·
> `docs/narrative/characters/brunus-vetorial.md` (Brunus Vetorial) ·
> `docs/design/mecanicas/cartas-hardware-pirataria-energia.md` §5 (mana e bateria são o mesmo
> recurso; receita de bateria caseira) · `docs/design/mecanicas/stamina.md` (Estamina —
> ver seção 4) · `CHARS.md` §4 e §8e.

---

## 1. O que o traje é

**Não é roupa: é um campo eletromagnético** que envolve a party inteira, filtrando o infravermelho
(calor) e os gases quentes de entrar. "Traje" é só o **nome do item** que gera e sustenta o campo —
o objeto físico que aparece na mochila é o gerador, não uma veste. Ele também **reverte o CO2
inspirado em carbono e oxigênio**: o oxigênio volta ao ar respirável dentro do campo, e o carbono
resultante **acumula num slot da mochila como grafite** (uso do grafite: seção 5).

**Resolve duas das três impossibilidades físicas medidas para descer a pé até 12.262 m** (a
terceira — o túnel que se fecha sozinho — é geografia da própria caverna, fora do escopo deste
item, `docs/design/dungeons/kola-sg3-12262-espelho-obsidiana.md` seção 1):

| # | Impossibilidade | Como o campo resolve |
|---|---|---|
| 1 | Túnel que se fecha sozinho | Não é o traje — geografia da dungeon |
| 2 | Temperatura extrema | Filtragem de infravermelho pelo campo |
| 3 | Atmosfera irrespirável | Ciclo fechado: CO2 exalado vira C (grafite) + O2 reinjetado, sem tanque externo |

## 2. Consumo: ativação barata, sustentação cara — a mesma carga das cartas

**Custo de ativação baixo, consumo alto enquanto ligado.** Isto é interação de sistema, não sabor:
mana e carga de bateria já são o mesmo recurso no jogo inteiro
(`cartas-hardware-pirataria-energia.md` §5, "mana e bateria são o mesmo recurso"), então **cada
turno com o campo ligado é carga que não vira carta** — a tensão entre manter o campo ativo e ter
bateria sobrando para conjurar É o desenho pretendido, não efeito colateral a corrigir depois.

**Bateria zerada com o campo ligado: dano por turno**, até a party sair da área extrema ou
recarregar — nunca morte súbita, nunca expulsão da área.

**O item não desgasta e não tem prazo de validade: dura para sempre.** O que se esgota, e se
recarrega como qualquer bateria do jogo, é a carga que ele consome enquanto ligado — o gerador em
si nunca precisa ser refeito ou substituído.

## 3. O powerbank de Helion Tusk: cena, não só mecânica

Helion Tusk empresta um powerbank que cobre **ao menos 55%** da carga necessária para a missão — os
outros ~45% são responsabilidade da party (levar bateria própria). A **devolução é obrigatória** ao
fim da missão, e a saída da missão tem **diálogo obrigatório com Helion**, checando se a party está
bem.

**A redação da fala não é deste documento** (`narrative-writer`, quando for a vez). O que a cena
precisa carregar: o risco real de emprestar equipamento próprio a quem vai a um lugar que quase
matou a party na primeira descida, e o cuidado genuíno de Helion ao perguntar se estão bem —
coerente com o mote "alquimista + tecnicista" já canônico para a dupla (`CHARS.md` §8e).

## 4. Correr com o campo ligado: consome Estamina e carga de bateria, dois recursos distintos

**Regra fixada pelo líder, hoje, com a reconciliação de nome fechada no mesmo dia (líder,
06/09/2026, por `AskUserQuestion`):** com o campo ligado, correr consome **Estamina e carga de
bateria ao mesmo tempo** — dois recursos, sem sobreposição de nome. A Estamina é o recurso já
canônico de `docs/design/mecanicas/stamina.md` (fechado desde 23/06/2026, números aprovados,
implementação stub existente, `core::player::Stamina`): não nasce recurso novo, é o mesmo que o
líder chamou de "barra de estamina" hoje. A palavra "carga", sozinha, fica reservada ao recurso de
bateria/mana das cartas (`cartas-hardware-pirataria-energia.md` §5); o recurso de correr do
personagem se chama Estamina em todo o corpus.

**Balanço de quanto o campo drena de cada recurso** fica para o `economy-designer` — `stamina.md`
já fixa os números de drain/regen de Estamina fora do contexto do traje; o consumo adicional do
campo ligado é número novo, não decidido aqui.

## 5. Grafite: terceiro material sólido da receita de bateria caseira

O grafite que o traje acumula é **insumo de craft de bateria** (líder, hoje, verbatim: "é o
terceiro item para fabricar bateria") — o **terceiro material sólido**, ao lado de zinco e cobre
(decisão do líder, 06/09/2026, por `AskUserQuestion`), na receita da bateria caseira de baixa
qualidade já fechada em `cartas-hardware-pirataria-energia.md` §5. A razão é a mesma física real que
já justifica zinco e cobre ali: a pilha comum de zinco-carbono usa exatamente um bastão de grafite
como eletrodo, ao lado do zinco — o grafite completa o par que a química cítrica do jogo já usa,
sem inaugurar receita paralela. Detalhe completo, incluindo a contagem final de materiais:
`cartas-hardware-pirataria-energia.md` §5, "Bateria de baixa qualidade (craftada)".

**Fonte do grafite: um lugar só, e isso é dependência de progressão real, registrada aqui como
fato.** Diferente de zinco e cobre (duas rotas cada: ferro-velho e loja/negociação), o grafite só
existe como subproduto do traje desta dungeon (seção 1: reverte o CO2 respirado em carbono) — sem
rota alternativa de compra ou drop conhecida. A Kola-SG3-12262 é dungeon de Ato 3
(`kola-sg3-12262-espelho-obsidiana.md`), então o grafite só entra no inventário da party depois de
alcançar e descer a essa dungeon ao menos uma vez com o traje ligado.

**Sinalizado para o líder, não decidido aqui:** se a receita da bateria caseira passa a EXIGIR
grafite sempre (travando essa variante de craft até o Ato 3) ou se continua funcionando com os
quatro materiais anteriores (sem grafite) para quem ainda não fez a dungeon, com o grafite entrando
como upgrade opcional de resultado quando disponível. A diferença importa porque a bateria caseira
é hoje framed como rede de segurança de INÍCIO de jogo (`cartas-hardware-pirataria-energia.md` §5,
"melhor que nada pra quem tá sem bateria nenhuma no inventário") — se o quinto material vira
obrigatório, essa rede de segurança deixa de existir antes do Ato 3.

## 6. Nenhuma altura, nenhuma verticalidade de câmera, nenhuma coordenada Z (L-26)

A "descida" a 12.262 m é inteiramente **travessia de mapa em grade 2D**, igual a qualquer outra
transição de área do jogo — nunca um sistema de profundidade/altitude simulado por número. As
"primeira descida", "segunda descida" e "terceira descida" do hub público são transições de ÁREA
dentro da topologia já existente, lidas na mesma perspectiva 3/4 top-down fixa e grade quadrada que
todo o resto do jogo usa (L-26). O traje não introduz:

- nenhuma coordenada de profundidade/altura por personagem ou por tile;
- nenhuma rotação de câmera, projeção isométrica ou eixo vertical de movimento;
- nenhum HUD ou medidor de "quão fundo" o jogador está, além do que a progressão de área/nível já
  comunica por level design (fora do escopo desta fatia).

O item só decide **se a party pode ocupar as áreas/tiles marcados como ambiente extremo** (calor,
atmosfera), exatamente como qualquer outro gate de progresso do jogo decide se a party pode ocupar
uma área — nunca como valor de profundidade simulada.

## 7. Gêmeos: o que já diverge, e por que não corrijo aqui

Busca literal ("traje", "roupa térmica", "traje térmico"): **encontrados 6 arquivos, analisados 6,
falharam 0** em achar o texto — mas **3 já divergem** do canon fixado hoje, porque descrevem o item
como vestimenta física:

- `CHARS.md:86` (entrada de Brunus Vetorial): "o traje térmico que resolve as impossibilidades de
  calor e respiração".
- `docs/design/dungeons/kola-sg3-12262-espelho-obsidiana.md` (linhas 111, 151, 286): "roupa térmica
  com tubos de oxigênio", "traje térmico".
- `docs/design/dungeons/areas-secretas/12-selve-profunda-kola.md` (linhas 33, 55): "o traje
  térmico", "com o traje".

**Não corrigido por esta fatia:** a propagação de consequências a `CHARS.md` e ao hub da dungeon já
tem item próprio na tabela de pendências, `D30` — editar os três agora seria pisar em outro átomo
(L-33). Sinalizado aqui para quem executar `D30` não perder nenhuma das três ocorrências.

## 8. Cortes da L-29 conferidos (nenhum atravessado)

- **C-02** (gating nunca é trava dura): o campo é gate de sobrevivência num ambiente, não trava de
  acesso à área — a Kola-SG3-12262 já é alcançável desde o início. **Cabe.**
- **C-06** (sem dificuldade dinâmica adaptativa): o consumo do campo é fixo por regra, não ajusta
  por performance do jogador. **Cabe.**
- **C-15** (mecânica nova só se couber num jogo com fim): o campo cria uma interação de recurso
  (carga que não vira carta) que já existia em espírito para o resto do jogo (mana=bateria); não é
  sistema novo, é aplicação da regra existente a mais um item. **Cabe.**
- Nenhum outro corte é tocado.

## 9. Handoff

- Balanço de quanto o campo ligado drena de Estamina vs. de carga de bateria (seção 4): número do
  `economy-designer`.
- Grafite obrigatório ou opcional na receita da bateria caseira, e a dependência de progressão que
  isso cria até o Ato 3 (seção 5): decisão do líder, depois número do `economy-designer`.
- Propagação a `CHARS.md`, ao hub da dungeon e à área secreta B (seção 7): escopo de `D30`.
- Redação em prosa da cena do powerbank (seção 3): `narrative-writer`, com a nota de tom já
  registrada.
- Nenhum número de balanceamento além dos 55% dados pelo líder — o resto (drain por turno, dano por
  turno de bateria zerada, curva de degradação) é número pendente de medição, com combate rodando.

---

**Última revisão:** 06/09/2026.
