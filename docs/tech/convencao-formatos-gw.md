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

## Os doze tipos, fixados em 25/08/2026

O líder fechou a lista. **Um tipo por elemento de jogo**, e não um tipo genérico — é a **L-04** aplicada ao nome do arquivo: *"cada elemento do jogo é um átomo com POCO próprio"*.

| Extensão | Elemento | Dono do conteúdo | Situação |
|---|---|---|---|
| `.gw.text` | diálogo e texto | **nós** | **existe**, 4 arquivos |
| `.gw.map` | mapa | **o GlintFx** (L-30 dele) | extensão nossa, formato dele |
| `.gw.card` | carta | **nós** | em extração do legacy |
| `.gw.batt` | bateria | **nós** | a criar |
| `.gw.virus` | vírus de carta | **nós** | a criar |
| `.gw.item` | item que não é carta nem bateria | **nós** | a criar |
| `.gw.foe` | inimigo | **nós** | a criar |
| `.gw.stat` | efeito de status | **nós** | a criar |
| `.gw.quest` | missão | **nós** | a criar |
| `.gw.bag` | o que o personagem carrega | **nós** | a criar |
| `.gw.box` | recipiente que fica no mundo | **nós** | a criar |
| `.gw.table` | mesa que **transforma** item | **nós** | a criar |

**A lista veio do canon, não de invenção:** a **L-04** nomeia *"carta, item, inimigo, efeito de status, diálogo, missão"*, e a **L-18** acrescenta que *"'item' inclui o catálogo de conteúdo"*. Os cortes da **L-29** eliminam dois candidatos que continuam fora: **conquista** pelo **C-08**, **afinidade de companion** pelo **C-12**.

**Receita deixou de ser candidato eliminado por corte** (correção 25/08/2026): a razão original — "fora pelo C-03 e pelo C-13" — caiu junto com a revogação desses dois cortes na mesma data (`GODS_LAWS.md` L-29). Isso não abre um décimo-terceiro tipo: **receita não vira tipo próprio porque o dado de criação passou a morar dentro do próprio `.gw.table`**, no ramo de criação (ver abaixo), não porque esteja cortada.

**Fora desta lista de propósito:** save e configuração. Não são catálogo de conteúdo — são **estado do jogador**, e a **L-25** já os põe no envelope binário selado. Misturá-los aqui confundiria fonte de compilação com estado de tempo de execução.

### Guardar não é transformar

`.gw.bag` e `.gw.box` **guardam**; `.gw.table` **transforma**. São tipos separados porque mudam por razões diferentes: bolsa cresce com melhoria de equipamento, mesa muda quando a regra de transformação muda. **Essa distinção sobrevive intacta à revisão abaixo:** ela não é sobre o que a mesa faz por dentro, é sobre guardar contra agir — e continuar guardando não virou agir. `.gw.bag` e `.gw.box` não ganharam poder de transformar nada; só `.gw.table` transforma, como sempre.

⚠️ **A cerca da mesa, revista em 25/08/2026 (revogação de `C-03` e `C-13`, `GODS_LAWS.md` L-29):** a cerca antiga dizia que a mesa **só transforma o que já existe** e que **nada nasce nela**. Isso deixou de ser verdade: o líder revogou os dois cortes que sustentavam a frase, e **a mesa vira a bancada**. `.gw.table` passa a cobrir **duas** operações, não uma:

- **Reparo** (o que a cerca antiga descrevia por inteiro): recarrega, repara, troca bateria, limpa vírus. O item entra e sai o MESMO item, com outro estado.
- **Criação:** craft de cópia pirata de carta — comum, ESPECIAL ou SUPER (`cartas-hardware-pirataria-energia.md` §15). Aqui o item que sai é **novo**, e é **sempre inferior** ao original: qualidade pior, efeito trocado, ou só o nome sem função nenhuma. A original de ESPECIAL/SUPER **nunca** nasce numa bancada, continua só por progresso narrativo.

**`.gw.table` carrega um campo dizendo qual das duas operações está em curso.** Reparo e criação não são o mesmo evento tratado como se fossem — são dois ramos explícitos do mesmo formato, não dois formatos. Isso **não abre a lista de doze tipos**: continua havendo um tipo por elemento de jogo (L-04), e a mesa continua sendo o único tipo que transforma; só a NATUREZA do que ela pode produzir mudou. A fronteira segue escrita aqui porque continua fácil de erodir em silêncio: agora o risco não é "recarrega virar fabrica", é a criação virar fabricação **do original** — e isso a cerca proíbe com a mesma firmeza de antes.

### Recipiente tem duas naturezas, e elas não moram juntas

É o mesmo corte que a carta já tem (catálogo contra instância):

| | O que é | Onde mora |
|---|---|---|
| **o que o recipiente É** | *"este baú, nesta dungeon, guarda estes itens"* | dado autorado, arquivo `.gw.box` |
| **o que aconteceu com ele** | *"este baú já foi aberto"* | estado do jogador, envelope selado (L-25) |

**Por que isto não é formalidade:** o item `D17` (coleta idempotente, achado do Gus Dragon) exige que abrir o baú duas vezes não gere duas cartas. Isso **só funciona se o baú guardar o próprio estado de já-aberto**. Estado no arquivo de conteúdo seria igual para todos os saves, e a proteção não existiria.

### Nem tudo que parece recipiente é recipiente

O canon já fixou o caso mais importante, e ele é contraintuitivo — `docs/design/mecanicas/deck-mao-sistema.md` §7:

> *"**A MÃO é uma SELEÇÃO (lista de IDs), NÃO um container.** Carregar na mão não move nem copia → **duplicação impossível**."*

A régua é o invariante 1: **container é onde a coisa MORA, com exclusividade** (*"vive em exatamente um container"*). Mochila, baú e armário são; a mão não é, de propósito.

**A floresta é os dois, e o canon distingue** (decisão do líder, 25/08/2026): **ponto autorado** — o brilho no chão que o canon já cita, determinístico, você vê e pega — é **recipiente**; **colheita repetível** é **fonte de aparecimento**, e aí o item **nasce** ali em vez de morar ali. Escolher errado entre os dois é como o defeito de duplicação volta.

### A mochila registra, não age

**Decisão do líder, 25/08/2026:** a bateria degrada **por uso e por ciclo de carga, nunca por tempo de relógio** — que é o que o canon de energia já dizia.

**A mochila é armazenamento passivo.** O que parece auto-modificação é sempre um comando que passou por ali: usar a carta, trocar a bateria, avançar o turno. **Nunca existe "a mochila fez"**.

⚠️ **O motivo é técnico e é decisivo:** a L-17 exige a regra como `aplica(estado, comando) → (estado novo, eventos)`, e o `D13` exige que *"mesma semente mais mesma lista de comandos reproduz o estado final byte a byte"*. **Se a bateria descarregasse por tempo de parede, o replay nunca fecharia** — recarregar o save num dia diferente daria estado diferente, sem bug nenhum. O que o jogador percebe sobrevive intacto: ele abre a mochila e vê a bateria mais fraca. A causa é o uso dele, não o calendário — e isso é auditável.

## Consequência para tipos futuros

Ao criar qualquer formato novo, o nome segue `.gw.<tipo>` — **e a pergunta de quem é dono do conteúdo é feita separadamente**, antes de escrever a primeira linha. Se o conteúdo for do GlintFx (LEI ZERO), somos consumidores e a extensão é só nossa etiqueta; se for nosso, o formato precisa de especificação escrita, como o `B9` está fazendo para o `.gw.text`.

**A armadilha que este documento existe para impedir:** ver `.gw.` no nome de um arquivo e concluir que o formato é nosso, e daí sentir-se livre para mudá-lo.

## Nada disso muda o que chega ao jogador

Todo formato acima é **fonte dentro do repositório**. A distribuição não leva texto: o jogador recebe tabelas compiladas e pacotes binários selados (L-18 e L-25, reafirmado pelo líder em 24/08 — *"quero tudo binário após compilar"*).
