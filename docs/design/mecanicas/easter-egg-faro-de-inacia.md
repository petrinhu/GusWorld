<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Easter egg: o faro de Inácia contra números forjados (EE-23/EE-24)

> **Status:** decisão do líder, 12/09/2026, por decisões verbatim (ver §3): canon do mecanismo
> fechado; o padrão matemático exato, o limiar de coleta e a fonte do nome formal são área cifrada.
> Prosa final (falas, texto do documento do vilão) fica com `narrative-writer`, por L-76 global:
> este documento é arquitetura, não prosa (beat, gatilho, dependência, estado mínimo).
>
> **Ponteiros (L-30):** cenas em `docs/narrative/comic-reliefs/easter-eggs-educacionais.md` (EE-23,
> EE-24); personagem em `CHARS.md` §5 (Inácia Berenger); geografia em
> `docs/design/mundo-topologia.md` §4 item 5 e §8 (campus Ferrovelhos, tema economia austríaca);
> mercado negro em `docs/design/mecanicas/cartas-hardware-pirataria-energia.md` §14.
>
> **O padrão matemático exato, o limiar de coleta e como o nome formal chega ao jogador são área
> cifrada:** `docs/_secret/mecanicas/faro-de-inacia-solucao.md`.

## 1. Ancoragem canônica

**A tese que este easter egg serve já é canon, não nova:** `docs/design/mecanicas/cartas-hardware-pirataria-energia.md:17` afirma: *"Mercado livre legítimo × a FRAUDE é que é o mal."* O mesmo documento, §14 ("Mercado negro", "Avaliar a fonte"), já descreve DUAS camadas de sinal contra fraude: **reputação** (macro, o vendedor) e **sinais/preço** (micro, o item, "o que não se vê mora no bom-demais-pra-ser-verdade"). Este easter egg adiciona uma **terceira camada de sinal**, sem substituir as duas: um padrão numérico das quantidades que um vendedor declara.

**Geografia:** `docs/design/mundo-topologia.md` §8 já liga o campus temático "Ferrovelhos, economia austríaca" (Menger, Hayek, Mises, Bastiat) ao **Mercado da Sucata Honesta**, com a nota explícita "Menger = Mercado da Sucata Honesta". O easter egg mora exatamente nesse cruzamento: o lugar onde o jogo já ensina, tematicamente, "avalie a fonte" (Bastiat, §1 de `cartas-hardware...md`) e "o valor emerge da troca, sem decreto" (Menger).

## 2. As decisões do líder (12/09/2026)

1. **Verbatim:** *"opção 3"*, o easter egg aparece nos DOIS momentos abaixo (mercado e documento do vilão), com a mesma lição, não em só um deles.
2. **Verbatim:** *"essa lei não é conhecida em geral, alguém teria de ensinar"*, não é descoberta espontânea do jogador; precisa de um personagem que ensine.
3. **Quem ensina é a Inácia Berenger**, sucateira do Mercado da Sucata Honesta, mãe do Cauã (`CHARS.md` §5). **Ela não dá aula de estatística.** Ela fareja número inventado por décadas de balcão: o faro vem primeiro, sem nome; o **nome formal** o jogador descobre depois, em outra fonte (área cifrada).
4. **Catálogo próprio confirmado:** os easter eggs de homenagem (`homenagens-diegeticas.md`) e os educacionais (`easter-eggs-educacionais.md`) ficam em arquivos distintos, com a mesma numeração `EE-N` corrida entre os dois.

## 3. Momento 1: o mercado (EE-23)

**Onde:** Mercado da Sucata Honesta, gate de relacionamento **Friendly Inácia alto** (reaproveita o padrão de gate já usado em `CHARS.md` linha 104 para a sub-quest da Penha Lírio: mesmo NPC, mesmo tipo de gate, sub-quest distinta).

**Loop de coleta:** ao interagir com vendedores do Mercado (Inácia incluída, e outros vendedores do mercado negro de `cartas-hardware...md` §14), o jogador pode **registrar** uma quantidade declarada (peças em estoque, unidades vendidas, lote recebido) no Diário: evento de domínio no mesmo espírito de `BestiaryEntryDocumented` (`docs/design/mecanicas/conquistas.md` §2.9), com o nome provisório `MarketNumberObserved(vendor_id, value)`. Nenhuma interface nova: o registro é uma linha de Diário, texto, não gráfico (§6).

Um vendedor específico (candidato natural: o "contato ligado ao Dante/FIR" que `cartas-hardware...md` §14 já cita como "fonte perigosa com peso narrativo") declara uma sequência de números que não bate com o que se espera de um estoque real; a razão matemática exata é área cifrada. Depois de coletar um número mínimo de declarações (área cifrada), o jogador pode levar a observação à Inácia.

**Como Inácia ensina (o núcleo do easter egg, o mais fácil de estragar):** ela não explica a lei. Ela reage por ofício. Linha ilustrativa, **não prosa final** (cabe a `narrative-writer` fechar o texto, L-76):

> INÁCIA: "Deixa eu ver esses números de novo... Isso aqui não bate. Trinta anos nesse balcão, ninguém guarda sucata assim. Sempre sobra pouco, começando em um, dois, três. Esse aí só sobra muito, começando em sete, oito, nove. Não sei o nome disso. Só sei que fede a papel forjado."

Ela nomeia o **sintoma** (o padrão que ela reconhece) e recusa nomear a **causa** (não conhece o termo formal), coerente com `docs/narrative/comic-reliefs.md`, corte `C-16`, e com a regra geral de onboarding orgânico do projeto (sem parede de texto, sem tutorial).

**Payoff mecânico:** a observação de Inácia devolve ao jogador um sinal de **reputação** sobre o vendedor suspeito, encaixando sem regra nova na camada de reputação que `cartas-hardware...md` §14 já define ("descoberta por experiência própria ou fofoca de NPCs"). Este easter egg é uma TERCEIRA via de descoberta de reputação, ao lado das duas já canônicas.

## 4. Dependência entre os dois momentos (obrigatória, ordem do líder)

**O Momento 2 só funciona se o Momento 1 já tiver sido encontrado.** Implementação sugerida (nome provisório, `snake_case`, L-22 global): uma flag de progresso, setada quando o jogador completa a cena com Inácia no Momento 1. Antes dela estar setada, o documento do Momento 2 é só flavor (números presentes, nenhum destaque, nenhuma opção de diálogo nova). Depois, ele ganha o "tell".

Isto é **knowledge-gate**, no mesmo padrão já canônico do projeto (ex.: "Knowledge alta" em sub-quests da Iara, `CHARS.md` §6): conteúdo que só se revela depois que o jogador tem a ferramenta certa, nunca por dificuldade adaptativa (o que violaria o corte `C-06`, §7). O mecanismo exato de como o nome formal do padrão chega ao Diário do jogador é área cifrada.

## 5. Momento 2: o documento do vilão (EE-24)

**Onde:** um documento in-world da Sterling Corp (relatório de produção/estoque/financeiro), no formato já canônico de `docs/narrative/in-world-docs.md` (numeração "doc N": o próximo número livre é trabalho de quem redigir o documento, não reservado aqui).

**O tell, ao nível de mecanismo (não de números exatos, que são prosa/conteúdo, L-76):** o relatório traz uma sequência de valores cujos primeiros dígitos **não seguem** o padrão que a cena com Inácia ensinou o jogador a esperar. A direção exata é decisão de quem escrever o documento. Só quando a flag do Momento 1 é verdadeira o jogo expõe uma opção de diálogo/observação nova sobre o documento ("os números não fecham"), puxando o mesmo faro que Inácia ensinou, sem repetir a explicação dela.

**Onde isto entra na história:** documento de missão contra a Sterling Corp (infiltração/heist, o tipo de conteúdo que já move o corpus, cf. `in-world-docs.md`); vira evidência/plot beat sobre números fabricados, reforçando o tema já canônico "Sterling = bug declarado feature" (`docs/narrative/comic-reliefs.md`).

## 6. Interface: nada novo, por desenho (L-27)

**Nenhuma tela nova é necessária ou proposta aqui.** Tanto o registro de números (Momento 1) quanto o "tell" do documento (Momento 2) são resolvidos inteiramente em **texto de diálogo e entrada de Diário**, seguindo o padrão já em uso no projeto para conteúdo do tipo `.dlg.txt` (`docs/narrative/comic-reliefs.md`, "Como estas cenas chegam ao jogador": direção cênica vira narração, não animação). **Não existe gráfico de barras, histograma nem qualquer visualização de distribuição de dígitos no v1 deste easter egg**: isso seria interface nova e cai direto na L-27 (nenhuma interface nasce antes do GlintFx). Se um dia alguém propuser um histograma visual como melhoria, ele nasce bloqueado até a camada `present/` existir, e não faz parte deste canon.

## 7. Conferência dos 14 cortes (L-29): nenhuma colisão encontrada

| Corte | Por que não colide |
|---|---|
| C-01 (multiplayer) | Não se aplica; conteúdo single-player. |
| C-02 (mundo aberto/persistente) | Usa áreas já existentes do grafo (Mercado da Sucata Honesta); nenhuma área nova. |
| C-04 (voz) | Tudo em texto/Diário; nenhuma dublagem. |
| C-05 (mocap) | Não se aplica. |
| **C-06 (dificuldade dinâmica adaptativa)** | O gate é **conhecimento** (flag de progresso), nunca desempenho do jogador; não é dificuldade adaptativa. Conferido com atenção por ser o corte mais próximo do risco aqui. |
| C-07 (mod/editor) | Não se aplica. |
| C-08 (placar/ranking) | A conquista oculta proposta (§8) é interna, não ranking entre jogadores; o corte proíbe ranking, não conquista interna (o próprio texto do corte abre essa exceção). |
| C-09 (PT+EN completo) | Sem impacto; texto novo entra nos dois idiomas quando escrito. |
| C-10 (certificação console) | Não se aplica. |
| C-11/C-12 (romance) | Não se aplica; Inácia é NPC adulto, sem enredo romântico. |
| C-14 (monetização) | Não se aplica; conteúdo gratuito de campanha. |
| C-15 (duração fixa) | Conteúdo opcional dentro do escopo fechado por conteúdo; não fixa hora nenhuma. |
| **C-16 (tutorial parede de texto)** | Verificado com atenção: a fala de Inácia (§3) é bark curto, não aula; a lei nunca é explicada em tela nem em pop-up. |

## 8. Conquista oculta (nome e gatilho provisórios, mesmo padrão de `conquistas.md`)

Nome de exibição e dica são prosa/asset (L-08), pendente de `narrative-writer`. Gatilho de domínio, provisório: destrava quando o jogador confirma corretamente, usando só o padrão numérico (sem outro sinal de reputação já revelado), qual vendedor do Mercado é o fraudador. `id` provisório (`snake_case`, L-22): `achv_nose_never_lies`. **Não fechado**, é sugestão; o líder decide o nome final junto com o texto (mesmo processo de `conquistas.md` §3).

## 9. Como Inácia guarda isto no CHARS.md

Aplicado nesta mesma fatia (canon do líder, não invenção de agente): `CHARS.md` §5, linha da Inácia Berenger, ganha a cláusula "ensina pelo faro de ofício, sem nomear, o padrão que separa estoque real de estoque inventado (easter egg, `docs/design/mecanicas/easter-egg-faro-de-inacia.md`)".

## 10. Pendências explícitas, para o líder e para o `economy-designer`

1. **Quantos números o jogador precisa coletar** no Mercado antes do padrão do vendedor suspeito ficar identificável: fechado, área cifrada.
2. **Margem/critério de "suspeito"** (quantos dígitos altos, de quantos, contam como anomalia): continua pendente de medição, por ordem explícita do líder. Proposta de fórmula/tabela paramétrica (nunca número literal no corpo da regra, L-17), não de valor único; fica com `economy-designer`.
3. **Nome de exibição da conquista e o texto exato da fala de Inácia e do documento do vilão:** prosa final, `narrative-writer` (L-76).
4. **De onde vem o NOME formal do padrão** para o jogador: fechado, área cifrada.
5. **Item novo de `TODO.md`** (não editado por mim, ordem explícita do team lead): sugestão de próximo ID livre medido em `TODO.md`, **D53** (maior D em uso hoje é D52), sem pré-requisito estrito, mas natural depender de nada além deste documento. Numeração de "doc N" em `in-world-docs.md` para o documento do vilão também não reservada aqui, pelo mesmo motivo.

---

**Última revisão:** 2026-09-12. Canon do mecanismo, decisão do líder; margem de suspeito e prosa final continuam pendentes (§10.2, §10.3).
