<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Contrato do formato de mapa — lado consumidor

> **Status:** espelho do contrato que o **GlintFx** fechou e congelou em 22/08/2026 no bus. Item
> `B10` da tabela de pendências, sem pré-requisito, servindo `D10`, `D11`, `E1` e `E3`.
>
> **Postura deste documento, e ela é a coisa mais importante nele:** o GlintFx é **dono** do
> formato de mapa (`L-30` do `GODS_LAWS.md` deles). O GusWorld é **consumidor, não coautor**.
> Este documento **não propõe, não negocia e não "melhora"** nada do formato — ele **registra**
> o que foi decidido do outro lado do bus, e diz o que **nós** fazemos com isso. Onde o contrato
> deles deixa algo em aberto, este documento diz `em aberto no contrato do GlintFx` e para por
> aí; não preenche o vazio com suposição nossa.
>
> **Convenção de nome de arquivo:** `.gw.map` é **nossa** etiqueta (`docs/tech/convencao-formatos-gw.md`);
> o **conteúdo** é o formato publicado pelo GlintFx, sem alteração nossa.
>
> **Fonte primária, cadeia completa no bus** (`gusworld_ia_autocomm`, repositório privado):
> `inbox/glintfx/archive/20260821-2317-gusworld-necessidade-de-mapa.md` (nosso pedido) →
> `inbox/glintfx/archive/20260822-0030-mapeditor-necessidade-de-formato.md` (pedido do
> mapeditor) → `inbox/mapeditor/archive/20260822-0050-glintfx-ack-necessidade-de-formato.md`
> (ack) → **`inbox/mapeditor/archive/20260822-0100-glintfx-decisoes-de-mapa-20-itens.md`** (os
> 20 itens decididos — a fonte do contrato) → `inbox/mapeditor/archive/20260822-0120-glintfx-ordem-rcss-antes-do-mapa.md`
> (implementação adiada, contrato intacto) → `inbox/mapeditor/archive/20260822-0200-glintfx-selo-de-integridade-e-ordem-mantida.md`
> (selo aberto vira lei) → `inbox/gusworld/archive/20260825-1300-glintfx-resposta-cripto-envelope.md`
> (correção mapa canônico × save). ⚠️ O envio de 22/08/2026 foi endereçado **a nós e ao
> mapeditor no mesmo envio** ("as duas listas se cruzam em cinco pontos, e responder em separado
> criaria duas versões da mesma decisão"), mas a cópia física não está arquivada em
> `inbox/gusworld/archive/`; está em `inbox/mapeditor/archive/`. Registrado aqui para quem for
> conferir esta spec não perder tempo procurando no lugar errado.

---

## 1. A distinção que vem antes de tudo: mapa canônico ≠ save

Correção enviada pelo GlintFx em **25/08/2026** (`inbox/gusworld/archive/20260825-1300-glintfx-resposta-cripto-envelope.md`),
**posterior** ao congelamento do contrato e que muda a leitura de um dos sete pontos abaixo (o
selo, §2.7). Citação:

> *"Mapa canônico e save são problemas diferentes, e o mesmo envelope serve os dois em **modos**
> diferentes."*

| | Mapa canônico | Save |
|---|---|---|
| **Natureza** | conteúdo estático que **nós distribuímos** | escrito na máquina do jogador, em execução |
| **O que importa** | **detecção** de corrupção/adulteração | **autenticidade** (confidencialidade é bônus) |
| **Segredo envolvido** | **nenhum** — detecção pura sai de graça: o jogo guarda o resumo dos mapas canônicos e compara | envelope selado com cifra autenticada (chave fornecida por quem chama, `L-25` nossa) |
| **Papel da cifra** | acrescenta só **ilegibilidade**, por pedido do líder — não é o que garante a detecção | acrescenta autenticidade e confidencialidade de verdade |

**Por que isto muda o desenho:** tratar os dois como o mesmo problema produz um mecanismo mais
pesado do que o necessário num dos dois lados. O **selo aberto de integridade** do §2.7 é
exatamente a metade "mapa" desta distinção — sem chave, sem assinatura, sem segredo. O **envelope
selado do save** (`L-25` nossa, item `E1`) é a outra metade, e não é assunto deste documento.

**Consequência prática para `D10`/`D11`:** o verificador de mapa (`E3` na parte que toca mapa) não
precisa de chave nenhuma para detectar mapa corrompido ou trocado; o verificador de save precisa,
e essa necessidade é do envelope `L-25`, não deste contrato.

---

## 2. Os sete pontos do contrato, congelados em 22/08/2026

Convenção herdada da fonte: **`[LÍDER]`** = decisão do líder do GlintFx, tomada por
`AskUserQuestion` do lado deles. **`[PMU]`** = porta de mão única — o contrato congela ali, e não
volta atrás sem quebrar todo arquivo de mapa já gravado no mundo.

### 2.1 Volume de colisão vira LISTA por objeto `[LÍDER]` `[PMU]`

Cada objeto posicionado carrega uma **lista** de volumes de colisão (não um único volume
embutido). Cada volume é `{forma, offset, rotação, sensor, enabled}`.

- **`sensor`** (booleano): o volume **bloqueia fisicamente** ou apenas **notifica** sobreposição,
  sem bloquear.
- **`enabled`** (booleano): alternável **em runtime** pela lógica do jogo consumidor — a mudança
  reflete na colisão imediatamente, sem recarregar o mapa.

**O que isto resolve para nós:** porta com dois estados de colisão, quando a porta é **objeto**
— a troca de estado é o `enabled`. Porta que é **marcador de célula** (não objeto) já era
resolvida por outra decisão anterior deles (mapa aceita mudança permanente de célula em
runtime); as duas formas de porta convivem no formato, e a escolha entre elas é de quem desenha
o mapa, não deste documento.

**`[PMU]` porque:** o layout do registro de objeto é contrato de dado gravado; trocar de
"um-volume-embutido" para "lista contada" depois quebraria todo arquivo já existente.

### 2.2 `blocks_path` separado de `blocks_move`, desde a v1 `[LÍDER]`

Bloquear **movimento físico** e bloquear **busca de caminho automática** (pathfinding) são bits
distintos, desde a primeira versão do formato — não é campo que pode entrar depois sem quebra,
porque depois que a superfície pública de pathfinding existir, separar os dois vira quebra de
API.

**O caso que motivou:** célula **livre para andar** por onde a busca automática **não deve**
mandar ninguém (água rasa, zona de perigo). A máscara de travessia por marcação de autoria
(§2.4 nossa, na verdade §2.5 desta spec — ver a nota abaixo) não resolve isso, porque ela só
**desbloqueia**, nunca desencoraja.

### 2.3 UUID de mapa, com destino de teleporte por ele `[LÍDER]` `[PMU]`

Todo mapa nasce com **identidade própria gravada no cabeçalho**, independente do nome do arquivo
e do caminho no disco. O destino de porta/teleporte que sai do mapa atual é **endereço direto**:
`{uuid do mapa, posição}` — **não** um símbolo resolvido em runtime.

O UUID é **opaco para o GlintFx** (16 bytes que a lib não interpreta) e serve, do nosso lado,
como o identificador estável de **área** (no nosso desenho, área **é** arquivo de mapa) — chave
das nossas próprias tabelas externas (perfil de encontro, tema de cor por bioma).

**Correção que o GlintFx registrou sobre si mesmo, citada porque é honesta e vale saber:** o
default anterior do CTO deles era `map_ref` como string de nome de arquivo; foi corrigido porque
renomear um arquivo quebraria em silêncio todo teleporte que apontava para ele.

### 2.4 Canal de propriedades nomeadas (chave → bytes) `[LÍDER]`

Existe em **mapa, objeto e marcador**. **Nunca em célula** — não é preferência de design, é
custo: um mapa tem dezenas de milhares de células, e um bolso de tamanho variável em cada uma
explode arquivo e tempo de leitura.

A lib **guarda e devolve sem interpretar**. É o mecanismo que sustenta, do nosso lado, o payload
opaco por objeto (conteúdo de jogo anexado a um objeto do mapa — que aquele marcador guarda uma
carta-chave, que aquele ponto é o lar de um companion) e a referência de tipo de objeto (que tipo
de coisa está ali: carta, NPC, item de cenário — o "tipo" em si é conteúdo nosso, opaco para a
lib).

### 2.5 Camada única por célula na v1, com reserva aditiva `[LÍDER]`

**Uma marca de semântica de arte/terreno por célula, na v1.** O que se acrescenta é uma seção
**normativa** na especificação do GlintFx declarando que camadas adicionais entram depois como
bloco **aditivo**, sem quebrar arquivo nenhum já gravado — isso é possível porque o valor da
célula já é opaco para a lib: camada extra é passagem sem interpretação.

**A célula também carrega, separadamente, um campo de marcação opaca de autoria** (não é
"semântica de arte", é outra coisa): toda consulta de colisão aceita uma **máscara de
travessia**, e célula bloqueante cuja marca casa com a máscara conta como transponível **só
naquela consulta, por ator, sem estado**. Esse é o mecanismo genérico por trás da carta `glitch`
(ideia do Gus Dragon) e de qualquer mecanismo de travessia equivalente que inventarmos depois,
sem exigir tipo novo da lib a cada carta.

### 2.6 Preservação de bloco desconhecido ao regravar `[LÍDER]` `[PMU]`

Exigência **normativa do formato**, não recomendação: um escritor conforme **preserva** o que
não reconhece. Concretamente:

1. O leitor **retém** `{id, bytes crus}` de todo bloco auxiliar desconhecido, em vez de
   descartá-lo.
2. O escritor **reemite** esse bloco intacto.
3. A saída canônica ganha uma regra de **posição** para o bloco preservado.
4. A taxonomia de bloco ganha um bit **"seguro de copiar"** (técnica do PNG): distingue bloco que
   pode ser reemitido cegamente de bloco que fica *stale* depois de uma edição e deve ser
   **descartado com diagnóstico** em vez de devolvido corrompido ao arquivo.

**`[PMU]` — o maior do dossiê:** o bit mora no identificador do bloco, imutável depois que o
primeiro arquivo existir no mundo.

**Aplicabilidade ao GusWorld, e é uma nuance que a fonte não nomeia explicitamente para nós:**
esta exigência liga em quem **escreve** mapa. O GusWorld, hoje, **só lê** mapa em runtime — quem
escreve é o `gusworld_mapeditor`. Enquanto o GusWorld não tiver nenhuma ferramenta própria que
regrava `.gw.map`, este ponto do contrato não pede implementação nossa; ele documenta por que o
formato é seguro de evoluir **por quem quer que o escreva**, e é por isso que entra nesta spec —
`E3` (validador semântico) e qualquer inspetor nosso de mapa precisam, no mínimo, **pular** bloco
desconhecido na leitura (a metade mais fraca da exigência, que qualquer leitor já precisa
cumprir para não quebrar em versão futura do formato).

### 2.7 Selo aberto de integridade — detectar no mapa, proteger no save `[LÍDER]` `[PMU]`

Entra no formato um **selo aberto**, que **qualquer um verifica**, cobrindo **corrupção,
truncamento e edição por ferramenta errada** — **sem chave, sem assinatura, sem nenhum segredo
dentro do formato**. Virou **`L-30` do GlintFx**.

Citação verbatim, registrada porque sustenta a regra e não só a ilustra:

> *"editar mapa num jogo que distribui editor é uso legítimo; trapaça é editar o save"*

**Corolário gravado como fato pelo próprio GlintFx, não como opinião:** em projeto com fonte
publicado, **detectar alteração é alcançável e impedir não é** — a mesma verdade que a nossa
`L-25` e `L-18` já assumem, agora dita do lado do formato de mapa.

**Consequência que o GlintFx registrou de propósito, além do que foi pedido:** qualquer proposta
futura de assinatura, DRM, chave embarcada ou ofuscação **no formato de mapa** vira **achado de
revisão** do lado deles, não feature — a razão fica escrita para não precisar ser redescoberta.

**Nota histórica, sem efeito no contrato:** o raciocínio original ("editor é distribuído, editar
é uso legítimo") foi corrigido em 25/08/2026 — o líder do GusWorld informou que o
`gusworld_mapeditor` é ferramenta **interna**, não distribuída. A premissa mudou; **a decisão do
selo aberto não foi revista** e continua valendo como estava. Ver §1 para a distinção que ela
compõe com o envelope de save (que é o assunto que a premissa errada estava, na verdade, sobre —
o pedido de cripto entrar no escopo do GlintFx, `item E7` da nossa tabela).

**Algoritmo do selo: escolha do CTO do GlintFx**, dentro da dependência zero deles, implementado
em casa por eles. `em aberto no contrato do GlintFx`: qual algoritmo específico.

---

## 3. O que ainda não é código, do lado deles

**`inbox/mapeditor/archive/20260822-0120-glintfx-ordem-rcss-antes-do-mapa.md`**: em 22/08/2026 o
líder do GlintFx decidiu, verbatim, **"rcss primeiro"**. A trilha de mapa fica **desenhada,
pontuada e fatiada, mas não entra em execução ainda**. Citação direta:

> *"O que se adia é a implementação, não o contrato — e essa ordem é a barata. Adiar
> implementação de contrato fechado custa tempo; adiar o contrato e implementar antes custa
> retrabalho e quebra de arquivo alheio."*

**Nenhuma data foi dada, e nenhuma é estimada aqui.** O que muda para nós, concretamente:
`D10` e `D11` projetam o modelo de domínio **contra este contrato hoje**; o código que
efetivamente liga no leitor/escritor de mapa do GlintFx só nasce quando esse código existir do
lado deles (`LEI ZERO`, `L-05` — proibido dublê de plataforma).

---

## 4. Itens correlatos decididos, mas explicitamente revisáveis (`[CTO]`)

Diferente dos sete pontos do §2 (`[LÍDER]`, fechados), estes quatro foram decididos pelo CTO do
GlintFx **por consequência de lei já dada**, e a própria fonte os declara "abertos a
contra-argumento" de quem consome. Registrados aqui por completude, sem status de porta de mão
única:

- **Herança de tipo com exceção por instância** — o formato guarda **instâncias resolvidas**,
  mais uma referência de tipo opaca; herança em si e override por instância ficam no editor,
  chaveados pelo UUID de instância, e o override cabe no canal de propriedades (§2.4).
- **Redimensionar mapa** — o **recálculo é do editor**; a lib só fixa, na especificação, a
  convenção de coordenada para que o recálculo seja bem definido.
- **Agrupamento de objetos** — fica **fora do formato**. Grupo de sessão de edição é do editor;
  grupo que o jogo precisa em runtime cabe no canal de propriedades (§2.4).
- **Ordem de serialização determinística** — critério fixado como normativo (ordem da
  especificação para os blocos, ordem da lista do modelo para os elementos), escolhido para
  minimizar diff. A fonte diz literalmente: "se você tiver critério melhor para revisão de
  mudança, é hora de dizer" — mas isso é prerrogativa de quem escreve mapa (mapeditor), não
  nossa, porque o GusWorld não regrava mapa (ver §2.6).

Nenhum destes quatro afeta o modelo de leitura que `D10` precisa hoje; são relevantes
principalmente para quem **escreve** o formato.

---

## 5. Quem consome este contrato, e o que cada um precisa dele

| Item | O que precisa deste contrato |
|---|---|
| `D10` | mapa como dado: modela célula (bloqueio contínuo, `blocks_path`/`blocks_move`, marca de autoria e máscara de travessia), objeto (lista de volumes `sensor`/`enabled`, âncora, payload opaco), UUID de mapa e de área, destino de teleporte `{uuid, posição}` |
| `D11` | save como estado do jogador: não duplica nada que já vive no mapa (ex.: intensidade de campo de dungeon, `G8`); usa UUID de mapa/área como chave externa |
| `E1` | envelope selado do **save** — distinto do selo aberto do **mapa** (§1); não depende deste contrato para o mecanismo de cifra, mas depende dele para saber que o mapa **não** entra nesse envelope selado do mesmo jeito |
| `E3` | validador semântico: para a parte que toca mapa, confere o selo aberto (§2.7) e, no mínimo, pula bloco desconhecido na leitura (§2.6) |

---

## 6. O que este documento não decide

Não é assunto deste documento, e não deve ser confundido com ele: o desenho do envelope de
**save** (`L-25` nossa, `E1`), a criptografia que o GlintFx vai entregar para esse envelope
(`E7`, já respondido em `inbox/gusworld/archive/20260825-1300-glintfx-resposta-cripto-envelope.md`),
e qualquer decisão de conteúdo (o que vai dentro do payload opaco de cada objeto — isso é nosso,
mas é assunto de `D10`/`D17`/catálogo, não deste contrato de formato).
