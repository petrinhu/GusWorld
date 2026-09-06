<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Unha venenosa de Fibortorrinco-Zeckendorf e a gota de veneno (item)

**Status:** DESIGN fechado pelo líder e pelo Gus Dragon em 06/09/2026, em conjunto. **CÓDIGO = feat separada**, ainda não implementada: depende do sistema de inventário e do sistema de craft (`economia.md` §7), nenhum dos dois existe em código hoje. Este doc registra o desenho, no mesmo formato de `capacitor-item.md`.

Cross-ref: `docs/narrative/diary/entries-fichas-bestiary.md` §5.5 (Fibortorrinco-Zeckendorf, o inimigo que larga o item), `docs/design/mecanicas/estado-envenenado-fibortorrinco.md` (o estado que a gota aplica), `docs/design/mecanicas/missao-unhas-fibortorrinco.md` (a missão que ensina a receita, com a ficha de Helena Sirinhaém), `docs/design/mecanicas/economia.md` §7 (Sistema de Craft) e §7.8 (18 ingredientes já canônicos, família irmã), `docs/design/mecanicas/cartas/_vocabulario.md` §5 (status `Poison`, já existente).

## 1. Conceito

Dois objetos, um só documento, porque um não faz sentido sem o outro (L-33 do projeto: "partes que só têm sentido juntas" são o caso em que agregar é certo):

- **Unha venenosa de Fibortorrinco-Zeckendorf (fresca):** loot que o inimigo Fibortorrinco-Zeckendorf larga em combate (`entries-fichas-bestiary.md` §5.5). Insumo bruto, não usável sozinho.
- **Gota de veneno de Fibortorrinco-Zeckendorf:** o produto da reação química que consome unhas frescas em lote. É o item que o jogador de fato aplica em arma.

## 2. A reação: não linear, de propósito

**A ideia central do item inteiro é a não linearidade.** Não é um craft "N unhas → 1 gota" escalável: é uma proporção fixa, só válida no lote completo.

| Insumo | Produto | Válido? |
|---|---|---|
| 144 unhas frescas | 2 gotas | **sim, é a única receita que funciona no jogo** |
| 72 unhas frescas | 1 gota | **não.** Metade do insumo não dá metade do produto: dá **zero**. Não existe receita de 72 unhas para 1 gota. |
| 89 unhas frescas | 1 gota, isoladamente | só numa reação **sem receita definida**: desvantajosa e impraticável (89 unhas por 1 gota é uma taxa pior que 72 unhas por gota da receita de 144, e mesmo assim não existe como opção jogável, é a "conta que não fecha" que o líder citou para mostrar por que não compensa fracionar) |
| qualquer quantia abaixo de 144 | nada | a reação simplesmente não dispara |

**Leitura de design (a régua do time-lead, preservada aqui):** o jogador que reúne metade das unhas não leva metade do prêmio, leva nada. É essa "conta que não fecha" que dá identidade ao item, e é a razão de a missão (`missao-unhas-fibortorrinco.md`) pedir o lote inteiro de uma vez, não uma fração dele.

**Nota matemática, e a razão do nome do bicho (`entries-fichas-bestiary.md` §5.5):** 144 e 89 são dois membros consecutivos da sequência de Fibonacci (a mesma "sequência numérica recorrente" já canônica no projeto: safe mode 13% em `battle-screen.md`, pedágio de 13 cr em `economia.md` §7.9, a cura em degraus 1,1,2,3,5 da Ampola Recursiva em `economia.md` §7.2), e a razão entre eles (144/89) se aproxima da razão áurea, o que casa com o Pillar 2 do jogo ("magia é sistema formal", natureza é matemática rígida). Mas o que fecha a receita é o **teorema de Zeckendorf**: todo inteiro positivo se escreve de uma única maneira como soma de números de Fibonacci não consecutivos, sem solução parcial. É por isso que 144 unhas rendem 2 gotas (144 cai certo); 89 unhas renderiam, em teoria, 1 gota, mas essa reação **não tem receita definida no jogo** (desvantajosa e impraticável mesmo se existisse: 89 unhas por gota é uma taxa pior que os 72 por gota da receita de 144, §2); e 72 unhas (metade de 144) não rendem nada, porque metade do insumo da receita de 144 não é meia solução, é ausência de solução. O nome do inimigo, Fibortorrinco-Zeckendorf, carrega essa unicidade sem meio-termo no próprio nome.

## 3. Quem sabe fazer a reação

**Antes da missão (`missao-unhas-fibortorrinco.md`):** ninguém, exceto **Helena Sirinhaém**, a herborista do Pelicano Branco que a possui (`CHARS.md` §7, `docs/narrative/deep/factions/pelicano-branco.md` §4). A fórmula e o método são exclusivos dela; não é craft do jogador via bancada padrão (`economia.md` §7), é um processo fechado que só ela domina.

**Depois da missão:** o jogador **aprende a receita** (é parte do prêmio da missão, junto de 1 gota). A partir daí, presume-se que o jogador pode repetir a reação sozinho sempre que reunir 144 unhas frescas novas. Onde essa reação acontece fisicamente no jogo (bancada existente, local novo) **não está decidido**: nenhuma das 3 estações de `economia.md` §7.1 (Bancada de Compilação, Forja de Firmware, Patch/Recompilação) foi indicada pelo líder para isto, e não presumo uma sozinho.

## 4. Unha fresca × unha ressecada (por que não dá para comprar o atalho)

Existem duas variantes do mesmo nome de item, com papéis mecânicos opostos:

- **Unha fresca:** só vem de combate contra o Fibortorrinco-Zeckendorf (loot direto, `entries-fichas-bestiary.md` §5.5). É a única que entra na reação do §2.
- **Unha ressecada:** comprável no mercado. **Não serve para a reação.** Vira, no máximo, um item de sabor: "palito de dentes mentolado", resquício de veneno sem potência mecânica. Comprar unhas no mercado nunca substitui farmar o inimigo.

Esta distinção é o que fecha a porta de um atalho óbvio (comprar em vez de caçar); ela está detalhada como regra de missão em `missao-unhas-fibortorrinco.md`, e registrada aqui como propriedade do item.

## 5. Uso da gota: aplicação em arma

**Efeito:** derramar 1 gota de veneno numa arma de mão dá **21% de chance** de aplicar o estado envenenado (`estado-envenenado-fibortorrinco.md`) no inimigo atingido pelo próximo golpe dessa arma.

**Mecânica de "untar arma" ainda não existe em canon nenhum documento revisado para este item** (varredura feita, sem resultado): não há precedente de item consumível aplicado a uma arma como buff de próximo golpe, em vez de consumido diretamente pelo personagem. Fica sinalizado para o `gameplay_engineer` avaliar quando o sistema de armas/inventário existir: se é um modificador temporário de arma (dura N golpes ou N turnos), consumo único no próximo acerto, ou outro formato. **Não decidido aqui.**

## 6. Pendências de implementação (quando inventário e craft existirem)

- Item `fibolatypus_claw_fresh` (loot, exibição "Unha venenosa de Fibortorrinco-Zeckendorf") e `fibolatypus_claw_dried` (compra, sabor, exibição "Unha ressecada de Fibortorrinco-Zeckendorf") como duas entradas distintas de catálogo. Identificador em inglês `snake_case` (L-22 do projeto), derivado do nome em inglês do bicho (`fibolatypus`, decisão do líder mantida mesmo os seis bichos existentes usando português para o slug de asset); nome de exibição em pt-br usa "Fibortorrinco-Zeckendorf".
- Item `fibolatypus_venom_drop` (exibição "Gota de veneno de Fibortorrinco-Zeckendorf"), produto da reação de 144 unhas frescas.
- A reação em si: onde ela roda (bancada existente ou local novo), se tem UI própria ou é resolvida via diálogo com Helena pós-missão.
- A mecânica de "aplicar item em arma" (§5), hoje sem precedente em canon.
- Preço de venda/compra da gota no mercado, se e quando ela entra em catálogo comercial normal, e se Helena continua sendo fonte exclusiva depois da missão (isto é regra de `missao-unhas-fibortorrinco.md`, não deste documento).
