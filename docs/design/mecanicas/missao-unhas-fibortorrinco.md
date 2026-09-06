<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Missão: as 144 unhas de Fibortorrinco-Zeckendorf

**Status:** Decisão de design fechada pelo líder e pelo Gus Dragon, em conjunto, em 06/09/2026. Números, regra central e NPC fechados; local exato de venda e implementação seguem abertos (ver §4 e §5).

Cross-ref: `docs/narrative/diary/entries-fichas-bestiary.md` §5.5 (Fibortorrinco-Zeckendorf, o inimigo que larga a matéria-prima), `docs/design/mecanicas/unha-veneno-fibortorrinco-item.md` (a unha e a gota, com a reação química completa), `docs/design/mecanicas/estado-envenenado-fibortorrinco.md` (o estado que a gota permite aplicar), `docs/design/mecanicas/missoes-cronometradas.md` (outro tipo de missão do projeto, não confundir: esta não é cronometrada em tempo real), `CHARS.md` §7 e `docs/narrative/deep/factions/pelicano-branco.md` §4 (ficha canônica de Helena Sirinhaém, usada sem contradizer).

## 1. Conceito

**Helena Sirinhaém** pede ao jogador **144 unhas frescas de Fibortorrinco-Zeckendorf** em troca de ensinar a receita da gota de veneno e entregar 1 gota já pronta. Não é uma missão de tempo real (`missoes-cronometradas.md` é outro sistema, não se aplica aqui): é uma missão de coleta e entrega, cujo desafio não é o relógio, é reunir o lote inteiro.

Helena já é canon fechado (`CHARS.md` §7, `pelicano-branco.md` §4): cinquenta anos, herborista júnior do Pelicano Branco pelos critérios locais, especialista em antídotos respiratórios desde que os surtos passaram a ter componente aerotransportado em -2, aliada operacional de Jaci e treina a menina em formulação de pó inalável e **titulação por gota de óleo**. Esta missão usa exatamente essa competência (titular por gota é o mesmo verbo técnico que a reação de 144 unhas para 2 gotas pede) e não descreve a personagem de novo: quem quiser o retrato completo lê as duas fontes acima.

## 2. Por que ela pede, e por que não vende ela mesma

Helena **domina a fórmula e o método** de produzir a gota (`unha-veneno-fibortorrinco-item.md` §3), mas está **sem matéria-prima**. A razão disso não é capricho de missão: está ancorada em dois fatos já canônicos do Pelicano Branco, não inventados aqui.

1. **Divisão de trabalho do vilarejo** (`pelicano-branco.md` §3, §4): Helena é **formuladora**, não caçadora. A ficha dela descreve trabalho de bancada (formulação, titulação), nunca coleta em campo. Quem caça e reconhece fauna da fronteira, incluindo fauna perigosa, é **Tatauín Branca** ("caçadora-coletora... conhece a Selve Profunda em raio que ninguém mais do vilarejo atinge"). Helena depender de terceiros para trazer matéria-prima animal já é o desenho estrutural do vilarejo, não uma lacuna preenchida por esta missão.
2. **Escassez real e já registrada** (`pelicano-branco.md` §5, evento "-2 a -1"): "aceleração de surtos menores, padrões de fauna corrompida em rotas antes limpas, recolhimento de sementes em quantidades cinco a oito por cento abaixo da média da década anterior". A mesma tendência de escassez que já é canon para as sementes-relíquia se estende, por leitura direta e não por invenção nova, a qualquer insumo de origem animal coletado nas mesmas rotas afetadas, incluindo a unha de Fibortorrinco-Zeckendorf.

As duas âncoras juntas bastam para a cena: Helena sempre dependeu de terceiros para a matéria-prima, e a rota que normalmente supria esse insumo está com fauna corrompida há dois anos canônicos. Não é preciso inventar um motivo novo nem usar o evento maior de Ano 0 (a operação Sterling contra o vilarejo, `pelicano-branco.md` §5) para justificar a escassez: aquele evento é sobre sementes-relíquia especificamente, e usá-lo aqui superdimensionaria a missão.

**Consequência de exclusividade, registrada para não se furar por atalho óbvio:**

- **Não se pode comprar o veneno de outro NPC.** Só Helena domina a fórmula e o método (`unha-veneno-fibortorrinco-item.md` §3); nenhum outro vendedor do jogo tem a gota em catálogo, antes ou depois desta missão.
- **Não se pode comprar unhas no mercado para completar a missão.** A unha vendida no mercado é a variante **ressecada**, que só serve como item de sabor ("palito de dentes mentolado", resquício de veneno sem potência mecânica); a reação exige **unhas frescas**, que só vêm de combate contra o Fibortorrinco-Zeckendorf (`unha-veneno-fibortorrinco-item.md` §4). O mercado vende a paródia do insumo, não o insumo.

Estas duas travas fecham os dois atalhos óbvios (comprar o produto pronto; comprar o ingrediente pronto) e são o que torna a missão genuinamente sobre caçar o Fibortorrinco-Zeckendorf, não sobre economia de crédito.

## 3. A entrega e o prêmio

- **Custo:** exatamente 144 unhas frescas de Fibortorrinco-Zeckendorf, entregues de uma vez. Não há entrega parcial nem prêmio proporcional a uma entrega menor: a reação em si já não existe para quantidades menores (`unha-veneno-fibortorrinco-item.md` §2), e a missão herda essa mesma régua de não linearidade. Um jogador que chega com 72 unhas não recebe metade do prêmio, não recebe nada, e a mensagem de Helena deveria deixar isso claro antes de ela aceitar as unhas erradas por engano (detalhe de diálogo, não decidido aqui).
- **Prêmio:** **aprender a receita** (destrava, para o jogador, a capacidade de repetir a reação de 144 unhas sozinho no futuro, `unha-veneno-fibortorrinco-item.md` §3) **mais 1 gota de veneno já pronta**, entregue de imediato.

## 4. O que falta decidir

- **Local exato de venda.** Helena mora no Pelicano Branco (`pelicano-branco.md` §4), não na cidade: ela é formuladora de bancada, e quem leva mercadoria do vilarejo até as cidades-fronteira para vender é **Bem-Te-Vi Caldeira** ("carrega cestas de bambu com sementes secundárias, ervas secas, antídotos básicos para venda nas cidades-fronteira", `pelicano-branco.md` §4). Onde exatamente a gota chega ao "mercado" que o líder e o Gus Dragon mencionaram (ponto de venda dentro do próprio vilarejo, ou via a rota de Bem-Te-Vi até uma cidade-fronteira) **não foi decidido**; fica como candidato de leitura, não como fato.
- **Onde o Fibortorrinco-Zeckendorf aparece de fato** (qual caverna, se é a Caverna dos Perdidos já canônica ou um arquétipo novo de dungeon): ver `entries-fichas-bestiary.md` §5.5, nota operacional. Se a caverna ficar dentro do território da Selve/Selve Profunda (onde Tatauín já caça), a distância até o Pelicano Branco fecha naturalmente com a missão; isso é leitura favorável, não decisão.
- **Trigger da missão** (quando ela fica disponível: progresso narrativo, Knowledge, ou disponível desde o início) e se é repetível ou única.
- **Texto de diálogo** (o pedido de Helena, a explicação de por que ela está sem matéria-prima, a recusa às unhas ressecadas) é trabalho de `narrative-writer`, não deste documento (L-76 do projeto: designer projeta o esqueleto, writer escreve a prosa final).

## 5. Ponteiro de implementação

Sistema de missões/quests do jogo ainda não existe em código (não há inventário, não há sistema de diálogo implementado). Esta missão depende dos mesmos alicerces que `unha-veneno-fibortorrinco-item.md` §6 já lista como pendência.
