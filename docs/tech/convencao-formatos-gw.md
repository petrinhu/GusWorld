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

## Os treze tipos, fixados em 25/08/2026

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
| `.gw.achv` | conquista (só a **definição**: nome, dica, condição de destravar, se é oculta) | **nós** | a criar |
| `.gw.bag` | o que o personagem carrega | **nós** | a criar |
| `.gw.box` | recipiente que fica no mundo | **nós** | a criar |
| `.gw.table` | mesa que **transforma** item | **nós** | a criar |

**A lista veio do canon, não de invenção:** a **L-04** nomeia *"carta, item, inimigo, efeito de status, diálogo, missão"*, e a **L-18** acrescenta que *"'item' inclui o catálogo de conteúdo"*. O corte da **L-29** que ainda elimina um candidato é o **C-12**: **afinidade romântica de companion** (medidor de afeto e simulador de namoro) continua fora. Afinidade de NPC como estado de reação social é outra coisa, e está dentro (L-29, alcance corrigido em 25/08/2026); ela não é catálogo, é resultado de diálogo, e não abre tipo próprio aqui.

**Conquista deixou de ser candidato eliminado por corte** (correção 25/08/2026): a citação antiga — "fora pelo `C-08`" — caiu junto com a revogação desse corte na mesma data (`GODS_LAWS.md` L-29). O jogo **tem** conquistas, e a definição de cada uma vira o tipo `.gw.achv` (ver a tabela acima e a seção dedicada abaixo). O que ficou de fora foi só a integração com a Steam, por colisão de licença (SDK Steamworks e AGPL), e isso não é assunto de formato.

**Receita deixou de ser candidato eliminado por corte** (correção 25/08/2026): a razão original — "fora pelo C-03 e pelo C-13" — caiu junto com a revogação desses dois cortes na mesma data (`GODS_LAWS.md` L-29). Isso não abre um décimo-terceiro tipo: **receita não vira tipo próprio porque o dado de criação passou a morar dentro do próprio `.gw.table`**, no ramo de criação (ver abaixo), não porque esteja cortada.

**Fora desta lista de propósito:** save e configuração. Não são catálogo de conteúdo — são **estado do jogador**, e a **L-25** já os põe no envelope binário selado. Misturá-los aqui confundiria fonte de compilação com estado de tempo de execução.

### Guardar não é transformar

`.gw.bag` e `.gw.box` **guardam**; `.gw.table` **transforma**. São tipos separados porque mudam por razões diferentes: bolsa cresce com melhoria de equipamento, mesa muda quando a regra de transformação muda. **Essa distinção sobrevive intacta à revisão abaixo:** ela não é sobre o que a mesa faz por dentro, é sobre guardar contra agir — e continuar guardando não virou agir. `.gw.bag` e `.gw.box` não ganharam poder de transformar nada; só `.gw.table` transforma, como sempre.

⚠️ **A cerca da mesa, revista em 25/08/2026 (revogação de `C-03` e `C-13`, `GODS_LAWS.md` L-29):** a cerca antiga dizia que a mesa **só transforma o que já existe** e que **nada nasce nela**. Isso deixou de ser verdade: o líder revogou os dois cortes que sustentavam a frase, e **a mesa vira a bancada**. `.gw.table` passa a cobrir **duas** operações, não uma:

- **Reparo** (o que a cerca antiga descrevia por inteiro): recarrega, repara, troca bateria, limpa vírus. O item entra e sai o MESMO item, com outro estado.
- **Criação:** craft de cópia pirata de carta — comum, ESPECIAL ou SUPER (`cartas-hardware-pirataria-energia.md` §15). Aqui o item que sai é **novo**, e é **sempre inferior** ao original: qualidade pior, efeito trocado, ou só o nome sem função nenhuma. A original de ESPECIAL/SUPER **nunca** nasce numa bancada, continua só por progresso narrativo.

**`.gw.table` carrega um campo dizendo qual das duas operações está em curso.** Reparo e criação não são o mesmo evento tratado como se fossem — são dois ramos explícitos do mesmo formato, não dois formatos. Isso **não abre a lista de treze tipos**: continua havendo um tipo por elemento de jogo (L-04), e a mesa continua sendo o único tipo que transforma; só a NATUREZA do que ela pode produzir mudou. A fronteira segue escrita aqui porque continua fácil de erodir em silêncio: agora o risco não é "recarrega virar fabrica", é a criação virar fabricação **do original** — e isso a cerca proíbe com a mesma firmeza de antes.

### Recipiente tem duas naturezas, e elas não moram juntas

É o mesmo corte que a carta já tem (catálogo contra instância):

| | O que é | Onde mora |
|---|---|---|
| **o que o recipiente É** | *"este baú, nesta dungeon, guarda estes itens"* | dado autorado, arquivo `.gw.box` |
| **o que aconteceu com ele** | *"este baú já foi aberto"* | estado do jogador, envelope selado (L-25) |

**Por que isto não é formalidade:** o item `D17` (coleta idempotente, achado do Gus Dragon) exige que abrir o baú duas vezes não gere duas cartas. Isso **só funciona se o baú guardar o próprio estado de já-aberto**. Estado no arquivo de conteúdo seria igual para todos os saves, e a proteção não existiria.

### Conquista tem duas naturezas, e elas não moram juntas

**Decisão do líder, 25/08/2026, sobre a pergunta "conquista vira tipo `.gw` próprio ou mora dentro do save": "1+2", as duas coisas.** É o mesmo corte que a carta e o recipiente já têm, aplicado ao tipo que a revogação do `C-08` (L-29, 25/08/2026) trouxe de volta:

| | O que é | Onde mora |
|---|---|---|
| **o que a conquista É** | *"esta conquista se chama X, a dica é Y, a condição de destravar é Z, e ela é oculta (ou não) até lá"* | dado autorado, arquivo `.gw.achv` |
| **o que aconteceu com ela** | *"o jogador já destravou esta conquista"* | estado do jogador, envelope selado (L-25) |

**Por que a definição não pode morar no save:** ela é igual para todo save, autorada e revisada junto com o resto do conteúdo, e compilada dentro do pacote binário selado pelo gerador de build (`E8`/`G2`). Guardar nome e condição dentro de cada save duplicaria o mesmo dado em todo save do jogador e tornaria a conquista editável exatamente no arquivo que a L-18 protege contra edição — a mesma mistura de fonte de compilação com estado de tempo de execução que a seção "Fora desta lista de propósito" já recusa para save e configuração.

**Por que o destravamento não pode morar no catálogo:** é o mesmo motivo do baú do `D17`, citado acima. É fato de UM jogador, nasce jogando; se morasse no arquivo de conteúdo, seria igual para todo mundo, e "destravei" deixaria de significar alguma coisa.

**O que o save guarda, e nada além disso:** o conjunto dos identificadores (`id`) estáveis das conquistas já destravadas. Não o nome, não a dica, não a condição — esses três só existem no `.gw.achv`, e são lidos de lá toda vez que a conquista precisa ser mostrada. **O `id` é estável e não posicional**: a ordem das conquistas no catálogo pode mudar sem que o save saiba, porque o save nunca guardou posição, só identidade.

**Razão de mudar própria, pela régua da L-33:** conquista não muda pela mesma razão que carta, inimigo ou efeito de status (L-17, regra de combate e balanço), nem é recipiente que ocupa lugar no mundo (`.gw.box`). Ela muda por design de progressão e narrativa — o que conta como marco, como se anuncia ao jogador — e carrega a mesma dualidade jurídica que a L-25 já reconhece para carta (número e regra são código; texto de sabor é asset reservado): nome e dica são prosa, a condição é regra. Nenhuma unidade hoje no catálogo cobre as duas coisas juntas; forçar conquista para dentro de `.gw.quest` misturaria "o jogo progride" com "o jogador foi reconhecido por algo", que são razões de mudar diferentes.

#### O que acontece quando o catálogo muda e o save é velho

Três mudanças possíveis no `.gw.achv`, e o save reage diferente a cada uma, precisamente porque só guarda o `id`:

1. **Conquista renomeada, ou dica reescrita.** O save não muda nada, porque nunca guardou o texto. Na próxima leitura, o mesmo `id` já destravado aparece com o nome novo. **Isto é o ganho estrutural de separar definição de destravamento**, não um caso a tratar à parte: renomear é de graça.
2. **Conquista removida do catálogo.** O `id` continua no save como dado órfão. Ele não é apagado e não reprova o carregamento: é um fato histórico do jogador ("isto já aconteceu"), não uma alegação sobre o estado atual das regras — a mesma leitura que já vale para um baú do `D17` já aberto numa dungeon redesenhada depois. Deixar de existir definição para exibir é problema de apresentação (`present/`, que ainda não nasceu, L-06), não de dado, e este documento não decide o que a interface faz com um `id` órfão.
3. **Condição de destravar alterada.** Quem já destravou continua destravado; a condição nova só rege quem ainda não destravou. É a mesma postura da seção "A mochila registra, não age": o save é registro do que já ocorreu, não recomputação sob a regra de hoje.

⚠️ **A lacuna que esta seção NÃO fecha, porque não é decisão de formato:** a L-25 promete, na fase 2, verificar o save por **re-execução** do histórico de comandos — *"para forjar um save que passe, é preciso produzir uma história de comandos que legitimamente chegue àquele estado"*. Se o destravamento de conquista for modelado como regra de domínio de verdade (comando/evento, L-17, o que ele deveria ser pela mesma lei que rege toda transição de estado), a re-execução completa tem de rodar contra **algum** ruleset — e não está decidido se é o vigente no momento em que o save foi gravado (exige versionar o catálogo e o verificador saber escolher a versão certa) ou o vigente no momento da verificação (quebra todo save cuja conquista já destravada dependia de uma condição afrouxada ou apertada depois). **Isto não é exclusivo de conquista** — vale para qualquer regra sujeita a replay —, e a fase 2 inteira ainda não tem desenho: só a fase 1 está na tabela hoje (`E2`, `E3`, `E4`). Fica registrado aqui para não ser descoberto tarde, e a decisão é do líder, quando a fase 2 for desenhada.

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
