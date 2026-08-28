<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# Contrato de i18n do GusWorld

> **Status:** canônico. Item `D20` da tabela de pendências, sem pré-requisito, com `B9` e `D19`
> dependentes dele.
>
> **Ordem do líder, 25/08/2026, verbatim:** *"lembre de colocar as marcacoes i18n nos arquivos
> para deixar tudo pronto para outras linguas possiveis apos lancamento"*.
>
> **O critério pelo qual este documento se julga:** acrescentar um idioma depois do lançamento é
> **acrescentar DADO, nunca mudar FORMATO**. Toda regra abaixo existe para que essa frase seja
> verdade quando o terceiro idioma chegar, não só o segundo.
>
> **Contexto de canon que motivou este documento:** em 25/08/2026 o líder alterou o corte `C-09`
> da `GODS_LAWS.md` L-29. Antes: conteúdo só em pt-br, tradução depois do lançamento. Agora:
> **pt-br e inglês na 1.0**, estrutura pronta para os idiomas que vierem depois. Medido nesta
> data: `resources/translations/en_intl.md` tem 1.650 palavras contra 2.833 de `pt_br.md`; só o
> Diário do jogo (`docs/narrative/diary/`, 8 arquivos) são 54.772 palavras.
>
> **O que este documento NÃO é:** ele não desenha o formato de arquivo fonte nem o pacote binário
> selado — isso é trabalho do `B9` (`docs/tech/convencao-formatos-gw.md`, tipo `.gw.text`), que
> **depende** deste contrato e tem de satisfazer cada regra abaixo. Este documento fixa **o que**
> o formato precisa garantir; `B9` decide **como**, em texto fonte editável no repositório,
> compilado e selado no build (L-18, L-25, ADR-012 §7-8).

---

## 1. Princípio central

> **Acrescentar idioma é acrescentar dado. Mudar de dois idiomas para três não pode exigir tocar
> em código, em schema de arquivo, no envelope selado ou em qualquer chave existente.**

Todo o resto deste contrato é a decomposição dessa frase em regras verificáveis. Onde uma regra
abaixo obrigaria re-trabalho de formato ao entrar um terceiro locale, a regra está errada e deve
ser levada ao líder antes de `B9` implementar.

**Por que isto é mais que preferência de arquitetura:** a L-25 já fixa que save, configuração,
mapa e catálogo vivem num envelope binário único, versionado, selado. Mudar o formato do catálogo
de texto depois do lançamento colide de frente com compatibilidade de versão de formato — o mesmo
motivo por que `LOAD_MIGRATION_REQUIRED` existe para save. Locale como dimensão fechada de dois
elementos seria uma bomba-relógio de migração de formato; locale como dimensão aberta evita que
ela exista.

---

## 2. Locale como dimensão aberta

**A lista de locales nunca é um número fixo no código nem no schema do arquivo.** Hoje existem
dois: `pt_br` (referência) e `en_intl`. O contrato vale igual para dois ou para vinte.

### 2.1 O identificador de locale é uma string estável, não um enum fechado

O mecanismo (seja lá qual `B9` desenhar) lê a lista de locales existentes **de um registro de
dados**, nunca de um `switch`/`if locale == "en"` no código ou de um número de campos fixo no
envelope. Isto é o análogo direto do que a L-25 já faz para conquista (`.gw.achv`) e para item de
mundo (`.gw.box`): o catálogo tem N entradas, e N é dado, não constante de compilação.

**Recomendação, não decisão fechada:** manter `pt_br` e `en_intl` como identificadores já em uso
(são estáveis e citados em código, dado e nos dois catálogos vigentes) e adotar, para locales
**futuros**, identificador no padrão BCP 47 em minúsculas-com-underscore coerente com o que já
existe (`es_419`, `es_mx`, `ja`, `pt_pt`) em vez de inventar convenção nova a cada idioma. Isto é
recomendação de estilo, não arquitetura: renomear os dois identificadores existentes para caber
num padrão retroativo tocaria toda referência já escrita (código, dado, os dois `.md`) por ganho
zero. **Decisão pendente do líder, quando o terceiro locale for de fato despachado.**

### 2.2 Todo locale carrega um pequeno registro de metadado, não só o texto

Para o fallback (§3) e para plural/gênero (§6) funcionarem sem hardcode, cada locale precisa
declarar, como **dado** (dentro do que `B9` desenhar, não neste documento):

| Campo | Para quê |
|---|---|
| identificador estável | chave de lookup do locale |
| é a referência? (booleano) | só um locale tem isto verdadeiro; hoje é `pt_br` |
| cadeia de fallback | ver §3; hoje trivial (qualquer locale → referência), campo existe para quando deixar de ser |
| categorias de plural CLDR aplicáveis | ver §6; hoje `pt_br` e `en_intl` usam as mesmas duas (`one`, `other`) |

**Isto não é engenharia adiantada para um problema que não existe:** o campo nasce hoje, vazio ou
trivial, porque adicioná-lo depois é o próprio re-trabalho de formato que este contrato existe
para evitar. O que **não** nasce hoje é a lógica que trata categorias além de `one`/`other` — essa
espera o locale que precisar dela, como qualquer teto técnico faseado da L-25.

### 2.3 Locale é diferente de idioma

`pt-BR` ≠ `pt-PT`; `en-US` ≠ `en-GB`; um futuro `es` do jogo não pode presumir que serve para
`es-MX` e `es-ES` ao mesmo tempo sem revisão de vocabulário. O registro de locale (§2.2) é por
**locale**, não por idioma-macro, precisamente para que essa distinção seja representável quando
surgir. Não se cria dado para variantes regionais hoje (dimensão aberta não é dimensão povoada
sem necessidade); registra-se que o campo suporta a distinção quando ela for real.

---

## 3. Cadeia de fallback

Três situações são **diferentes entre si** e não podem cair no mesmo comportamento — confundir
uma "tradução ainda não feita" (normal, esperado, é o estado de 244 das 361 chaves de
`en_intl.md` hoje) com uma "chave que não existe em lugar nenhum" (sempre um bug) é o erro que
este contrato existe para impedir.

### 3.1 O locale inteiro está ausente

O jogador só pode escolher, na tela de idioma (`SETTINGS_LANGUAGE`), um locale que **existe** no
pacote instalado — a lista vem do registro (§2.2), nunca é digitável nem inferida livremente do
SO. Se um save mais novo referenciar um locale que este build não tem (por exemplo, o jogador
voltou a uma versão anterior depois de ter jogado numa que trazia um terceiro idioma), a mesma
doutrina da L-25 para configuração selada se aplica: **nunca recusar o boot**. Cai silenciosamente
para o locale de referência (`pt_br`), com aviso visível, exatamente como configuração corrompida
já cai para o padrão de fábrica com aviso, e não crash.

### 3.2 A chave existe no locale, mas o valor está vazio (tradução pendente)

Este é o caso comum durante o G1: `en_intl.md` tem hoje 244 chaves com valor vazio, esperando o
`D19`. **Não é erro, é estado de trabalho em andamento.** O comportamento é: percorrer a cadeia de
fallback do locale (§2.2) até achar um valor não vazio, terminando sempre no locale de
**referência**, que por contrato é o único que **DEVE estar sempre 100% completo** (regra já em
vigor em `resources/translations/pt_br.md`, mantida aqui). Hoje, com dois locales, a cadeia é
trivial: `en_intl` vazio cai direto em `pt_br`. Nada aparece em branco para o jogador nunca —
branco é indistinguível de "nada devia estar aqui", e é exatamente o oposto do que se quer numa
UI: melhor mostrar pt-br do que mostrar nada.

### 3.3 A chave não existe em locale NENHUM

Chave referenciada pelo código ou por dado de conteúdo (`.gw.card`, `.gw.foe` etc., a `DisplayName`
que `pt_br.md` §8 já documenta) mas ausente do catálogo inteiro — típico de erro de digitação, ou
de chave apagada enquanto algo ainda a referenciava. Isto **é sempre bug**, nunca estado de
tradução pendente, e o tratamento é deliberadamente diferente do §3.2:

- **Em runtime, o texto mostrado é a própria chave, crua**, por exemplo `DIALOGUE_CENA15_M1_N07_LEITURA`
  aparecendo na tela como se fosse a fala.
- **Um diagnóstico alto sai no log** (e, em build de debug, um assert), sempre — nunca falha
  silenciosa.

**Por que "aparecer a chave crua" é a escolha certa, e não vazio nem um texto genérico tipo
"[erro]":** vazio é invisível — o jogador vê um botão sem rótulo ou uma caixa de diálogo em
branco e não tem como nomear o problema num relato de bug, e o QA não consegue `grep` por "nada".
A chave crua, ao contrário, **é** o nome do bug: quem reporta cola o texto exato que apareceu na
tela, e isso já é o identificador que localiza a linha de código ou de dado errada, sem
investigação adicional. É a mesma lógica de honestidade que a L-25 já aplica a save corrompido —
declarar o defeito em vez de mascará-lo — aplicada ao texto em vez de ao arquivo.

### 3.4 Resumo do critério

| Situação | É bug? | Comportamento |
|---|---|---|
| Locale inteiro ausente (save/config aponta pra locale que este build não tem) | Não | Cai para o locale de referência, com aviso; nunca recusa abrir |
| Chave existe na referência, vazia no locale atual | Não (é trabalho em andamento) | Sobe a cadeia de fallback até achar valor; termina sempre na referência, que é 100% completa por contrato |
| Chave não existe em locale nenhum | **Sim** | Mostra a chave crua na tela + log de erro alto (assert em debug) |

---

## 4. Convenção de chave

### 4.1 Nomenclatura por domínio, já em uso e aqui endossada

`resources/translations/README.md` já fixa prefixos por domínio (`MENU_`, `SETTINGS_`, `A11Y_`,
`SAVE_`, `HUD_`, `COMBAT_`, `ERROR_`, `INFO_`, e o padrão `LORE_<DOC>_<KEY>` para o Diário) e a
convenção mecânica de diálogo fixada no piloto da Cena 15
(`docs/narrative/propostas/cena15-dlgtxt-PILOTO-PROPOSTA.md`, item `DLG-03`):

> `DIALOGUE_` + `<dialogue_id em MAIÚSCULAS>` + `_` + `<node_id em MAIÚSCULAS>`

Este contrato **endossa e generaliza** essas duas convenções para todo domínio futuro (`.gw.card`,
`.gw.foe`, `.gw.quest`, `.gw.achv` — a lista de treze tipos de `convencao-formatos-gw.md`): **um
prefixo por domínio de UI ou por elemento de jogo, nunca uma chave genérica reaproveitada entre
domínios.** A tabela de nomes de dado de combate em `pt_br.md` §8 (`FAMILY_<id>_NAME`,
`STATUS_<id>_NAME`, `CARD_<id>_NAME`) já segue exatamente este princípio e é o modelo a copiar
para os tipos ainda não criados.

### 4.2 Granularidade: uma chave por unidade de atenção do tradutor

A régua não é tamanho do texto, é a mesma da L-33 aplicada a documento: **uma chave cobre a menor
unidade que ainda faz sentido isolada para quem traduz.**

- **UI curta** (botão, rótulo, título): uma chave por string completa. Nunca concatenar duas
  chaves para formar uma frase (`"Sair" + " " + "do jogo?"` quebraria em qualquer língua com ordem
  de palavras diferente) — a chave inteira é a frase, como `MENU_QUIT_CONFIRM` já faz.
- **Diálogo**: uma chave por nó do grafo (`DIALOGUE_<id>_<node>`), nunca uma chave para o diálogo
  inteiro nem uma chave partida no meio de uma fala. O nó já é a unidade que o parser do motor
  consome (`ADR-014-dialogue-runtime-poco.md`), e é também a unidade que faz sentido pro tradutor
  ler isolada.
- **Prosa longa** (Diário, docs in-world): uma chave por **parágrafo**, não por documento inteiro
  nem por sentença. A convenção já citada em `README.md`
  (`LORE_PACTO_SENSORIAL_PARAGRAPH_03`) é o padrão certo, e a razão é dupla: (a) a mecânica de
  fragmento-antes-de-completo do Diário (`docs/narrative/diary/entries-docs-descobriveis.md`,
  "docs longos aparecem fragmentados no pickup") já exige granularidade de parágrafo para poder
  mostrar 1-3 trechos antes do documento inteiro; (b) uma correção de texto num parágrafo não pode
  forçar a retradução do documento inteiro nos outros locales.
- **Nome curto de dado de jogo** (carta, status, família, combo, ator): uma chave por entidade,
  como `pt_br.md` §8 já pratica.

### 4.3 Estabilidade: uma chave nunca muda de sentido

Duas operações são diferentes e o contrato as trata diferente:

1. **Reescrever o VALOR de uma chave existente** (corrigir texto, ajustar tom, alinhar copy a um
   mock aprovado) é seguro e já é prática corrente do projeto — o comentário em `pt_br.md` sobre
   `SAVE_SLOT_EMPTY`/`SAVE_SLOT_LABEL` documenta exatamente isso: o time confirmou, por grep, que
   nenhum call-site dependia do texto antigo, e trocou a palavra sem tocar a chave. **Reword não é
   rekey.**
2. **Repurposear uma chave** — usar a mesma identidade para um sentido diferente do que ela tinha
   quando qualquer locale (inclusive só a referência) já a traduziu — é proibido. Se o sentido
   mudou, **nasce uma chave nova**; a antiga fica marcada como não usada e só é removida depois de
   confirmado por grep que nada mais referencia (código nem dado), e com o líder informado — nunca
   apagada por presunção de agente (L-14).

**Por que isto é mais caro para chave do que para valor:** uma chave tem consumidor duplo — o
código/dado (`ACTION_<nome>` é lido por `gus/domain/input/action_registry.cpp`, conforme o
comentário já presente em `pt_br.md`) e cada locale que já a traduziu. Renomear ou repurposear
quebra os dois ao mesmo tempo: o código que busca a chave antiga e o trabalho de tradução já feito
em cada idioma vivo.

**A ordem de exibição nunca depende do nome da chave.** No formato de diálogo, o `id` do nó é só
identidade; a ordem real vem dos links `->` do grafo (`ADR-014`). Isso significa que inserir um
nó novo no meio de uma cena publicada **não exige renumerar** os nós vizinhos — anexa-se o
nó novo com um `id` que ainda não existe e emenda-se o grafo, sem tocar identificador nenhum já
traduzido. A numeração de dois dígitos do piloto da Cena 15 (`n01`...`n27`) existe para
ordenação alfabética de arquivo, não para ordenação de jogo — não confundir as duas.

---

## 5. Contexto para quem traduz

**A mesma palavra em botão e em prosa são chaves distintas — sempre.** O corpus já pratica isso:
"Cancelar" aparece em pelo menos oito chaves diferentes (`MENU_QUIT_CONFIRM_NO`,
`SAVE_OVERWRITE_CONFIRM_NO`, `CONTROLS_RESTORE_CONFIRM_NO` etc.), cada uma podendo variar
independente se um idioma futuro precisar de uma palavra diferente por contexto gramatical
(concordância de gênero, registro formal/informal). Isto não é redundância a limpar — é a proteção
que faz uma chave nunca precisar servir dois papéis.

**Mecanismo de contexto: formalizar o que já existe, não inventar sintaxe nova.** Os dois
catálogos já usam uma convenção informal — uma linha iniciada por `#` ou `###` imediatamente
acima de uma chave, ignorada pelo parser (regra 4 do `README.md`) — para documentar decisão de
copy, call-site, ou nota de tom (ver os comentários acima de `SAVE_SLOT_EMPTY`, de
`DIALOGUE_NPC_INTRO_*`, e o cabeçalho da Cena 15). **Este contrato torna obrigatório** o que já era
prática esporádica: toda chave cujo sentido dependa de contexto não óbvio pelo texto sozinho —
quem fala, o registro (`warm` × `terminal`, já em uso), gênero gramatical do falante, ambiguidade
lexical, ou limite de caracteres de um botão — carrega uma linha de comentário imediatamente acima
dizendo isso. Quem traduz lê o comentário antes da chave; quem não precisa de contexto (a maioria
das chaves de UI curtas, autoexplicativas pelo próprio texto) não é obrigado a escrever nada.

Ver também `docs/narrative/guia-dialogos.md` §11 (trocadilho e jargão) — regra irmã, tratada
à parte no §7 abaixo porque tem mecanismo próprio, não é caso geral de contexto.

---

## 6. Placeholder, plural e gênero

### 6.1 O que existe hoje, e o problema que carrega

Os dois catálogos usam interpolação posicional (`{0}`, `{1}`, ...), herdada do parser da engine
anterior e mantida como convenção mecânica do formato `## CHAVE` (regra 6 do `README.md`).
Pluralização hoje é feita **por escrita manual**, caso a caso:

- Evitando o problema por design: `SAVE_SCREEN_SUBTITLE_SAVE` diz *"{0} espaços (lista rola)"*
  sempre no plural, porque o número de espaços de save nunca é 1 na prática. Isto é uma escolha
  válida quando o singular genuinamente não ocorre.
- Escondendo o problema com uma marca de "(s)" hardcoded: `INFO_AUTOSAVE` diz *"Salvamento
  automático em {0} minuto(s)."* — isto **quebra** em qualquer idioma cuja regra de plural não
  seja "sufixo opcional", que é a maioria delas fora do inglês simplificado. Português já erra
  aqui de verdade: "em 1 minuto(s)" lido em voz alta soa errado, e um idioma com mais categorias
  de plural que o inglês (russo tem quatro; árabe tem seis) não tem como caber nesse padrão de
  jeito nenhum.

### 6.2 Recomendação: ICU MessageFormat, num subconjunto escrito em casa

**Isto é recomendação, não decisão fechada** — fica registrada aqui como a posição técnica deste
contrato, para o líder confirmar ou pedir alternativa.

Recomendo adotar **a sintaxe de plural e seleção do ICU MessageFormat** para toda chave nova que
precise de número variável ou de concordância de gênero, com placeholder **nomeado** em vez de
posicional:

```
{count, plural,
  =0   {Nenhum espaço}
  one  {# espaço}
  other {# espaços}
}
```

```
{gender, select,
  female {Ela salvou {count, plural, one {# jogo} other {# jogos}}}
  male   {Ele salvou {count, plural, one {# jogo} other {# jogos}}}
  other  {Salvou {count, plural, one {# jogo} other {# jogos}}}
}
```

**Por quê ICU e não uma sintaxe própria:** as categorias de plural (`zero`, `one`, `two`, `few`,
`many`, `other`) são as do CLDR, o mesmo vocabulário que qualquer ferramenta de tradução
profissional (e qualquer tradutor humano familiarizado com localização) já reconhece — não se
inventa uma taxonomia de plural própria quando a Unicode já mantém uma, testada contra centenas de
idiomas.

**Por que "num subconjunto escrito em casa" e não a biblioteca ICU4C:** o projeto é zero-dependência
por decisão já tomada (`ADR-006`, ratificada pela L-25 para a própria criptografia — "nenhuma
biblioteca de terceiro"), e ICU4C é uma dependência pesada para o que o jogo precisa. A
**sintaxe** ICU MessageFormat pode ser adotada sem a **biblioteca** ICU: um parser pequeno,
escrito em `domain/i18n/` junto do carregador de catálogo (`ADR-012` §7 já reserva esse espaço),
que entende `{arg}`, `{count, plural, ...}` com as categorias CLDR e `{..., select, ...}`, com a
tabela de qual categoria vale para qual locale sendo exatamente o "campo de plural CLDR" do
registro de locale do §2.2 — dado, não código condicional por idioma.

**Migração dos placeholders posicionais para nomeados:** recomendo que toda chave **nova** use
nome (`{player_name}`, `{count}`) em vez de posição (`{0}`, `{1}`), porque nome comunica ao
tradutor o que o valor **é**, e permite que um idioma com ordem de palavras diferente reordene os
argumentos livremente — coisa que posição não permite. **Isto não obriga a tocar nas chaves já
publicadas**: a sintaxe do placeholder vive dentro do **valor**, não na chave, então migrar uma
chave existente de `{0}` para `{count}` é uma edição de valor por locale, exatamente o tipo de
mudança que o §4.3 já classifica como segura (reword, não rekey) — e pode ser feita
progressivamente, sem pressa e sem quebrar nada, à medida que cada chave for revisitada.

### 6.3 O que fica pendente, para o líder ou para quem implementar `B9`

- Confirmar a adoção do subconjunto ICU acima (ou pedir alternativa).
- Decidir o momento da migração de `{0}`/`{1}` para nomeado nas chaves já publicadas — pode ser
  gradual, já que §4.3 mostra que isto não quebra nada.

---

## 7. Termo de lore sem tradução honesta

A L-22 já fixa a regra geral: **nome próprio do mundo do jogo, unidade inventada e palavra da
língua construída não se traduzem** — os exemplos citados na lei são `runa`, `tavus_drive`,
`selve`. O piloto da Cena 15 já encontrou um caso concreto e resolveu do jeito certo
(`cena15-dlgtxt-PILOTO-PROPOSTA.md`, item DLG-12): o par `compilar`/`compile` sobrevive traduzido
normalmente porque a piada atravessa a fronteira de idioma, enquanto o nome do lugar
(**Dutos Infernais**, **Neo-Sylvania**) é decisão de canon de nomenclatura, não de tradução — nome
próprio pode ganhar forma equivalente em outro idioma (`Infernal Ducts`) ou permanecer idêntico
(`Neo-Sylvania`), e quem decide isso é quem administra o canon do mundo, não quem traduz.

**Mecanismo, para não depender de memória de quem traduz:** um termo desta classe recebe, **no
valor de cada locale, o mesmo termo (ou a forma que o canon já fixou para aquele idioma)**, nunca
uma paráfrase nem uma tradução literal palavra-por-palavra. A chave carrega o comentário de
contexto do §5 explicando que aquele termo é intencionalmente preservado — sem esse comentário, um
tradutor futuro (ou um MT com revisão) não tem como distinguir "esqueceram de traduzir" de "é pra
ficar assim", e a segunda leitura errada é pior que a primeira porque silenciosamente destrói lore.

**Consequência para granularidade (§4.2):** um termo de lore sem tradução honesta **não abre chave
própria** — ele é parte do valor de uma chave normal (de UI, de diálogo, de prosa), com o
comentário de contexto ao lado. Não se cria uma classe de chave "glossário protegido"; o que
protege é o comentário, não o formato.

---

## 8. Orçamento de expansão

**Regra geral, sem medir texto específico ainda porque não há `present/` (L-06):** nenhum
contêiner de texto pode ser dimensionado no tamanho exato do texto em pt-br. A referência de
mercado (alemão e francês tipicamente +30-40% sobre o inglês; línguas com escrita mais larga por
caractere, ou línguas que precisam de transliteração, até +50%) vale como piso de folga a exigir
de qualquer caixa de diálogo, rótulo de botão ou HUD, quando a camada de apresentação nascer.
Truncar com reticências é falha de design, não solução — a caixa cresce, faz *word-wrap*, ou o
elemento usa largura mínima flexível; nunca corta o texto de um idioma porque coube no idioma de
desenvolvimento.

**Risco concreto já identificado no corpus, não hipotético:** o comentário em `pt_br.md` sobre
`SAVE_LOAD_WARN_DAMAGED` documenta que a fonte pixel do jogo (**PixelOperatorMono**,
`assets/fonts/`) **não cobre** `U+26A0` (sinal de alerta) — por isso o prefixo escolhido foi `!`
ASCII em vez do símbolo. Isto é uma evidência real, medida ao vivo, de que a fonte bitmap fixa do
jogo tem um repertório de glifos limitado. **Consequência que este contrato tem que declarar em
vez de esconder:** locale como dimensão aberta cobre a estrutura do catálogo (chave, valor,
fallback, plural). Ela **não** resolve sozinha um idioma cujo alfabeto a fonte pixel atual não
desenha — japonês, coreano, árabe, ou qualquer script fora do repertório latino acentuado que
`PixelOperatorMono` cobre hoje exigiria **fonte adicional por locale**, o que é dado (arquivo de
fonte por locale) se o renderizador de texto (ainda não escrito, é trabalho da camada `present/`
que a L-06 ainda bloqueia) já nascer preparado para trocar fonte por locale, e é retrabalho de
formato se o renderizador nascer hardcoded numa fonte só. **Isto fica registrado como pendência
para quando `present/` for desenhado — não é decisão deste documento, mas é o tipo de decisão que,
se tomada errado, transformaria "acrescentar idioma" de volta em "mudar formato" para qualquer
idioma fora do alfabeto latino.**

---

## 9. Relação com o que já existe

| Documento | O que ele é | Relação com este contrato |
|---|---|---|
| `resources/translations/README.md` | Manual operacional dos catálogos `.md` atuais (formato `## CHAVE`, workflow de adicionar chave, naming por prefixo) | Este contrato **endossa** a nomenclatura e a formaliza (§4, §5); tem autoridade de **regra**, o README continua como **manual de operação do dia a dia** enquanto o formato `.gw.text` de `B9` não substitui os `.md` atuais. Onde os dois divergirem, este contrato vence, porque `B9` depende dele. |
| `docs/narrative/propostas/cena15-dlgtxt-PILOTO-PROPOSTA.md` | Proposta (não canon) de conversão de uma cena para o formato `.gw.text` | A convenção de chave de diálogo dela (`DLG-03`) é generalizada aqui (§4.1); a triagem de trocadilho/jargão (`DLG-12`, e `guia-dialogos.md` §11) é referenciada no §7 como o precedente correto de tratamento caso a caso |
| `docs/narrative/guia-dialogos.md` §11 | Regra de trocadilho/jargão: caso a caso, versão própria por idioma quando o par não sobrevive | Complementar a este contrato, não sobreposta: §11 decide **quando reescrever o efeito em vez de traduzir literalmente**; este contrato (§7) decide **como o termo intraduzível fica marcado na chave** |
| `docs/tech/convencao-formatos-gw.md` | A convenção `.gw.<tipo>` e a lista de treze tipos | `B9` (dependente deste contrato) escreve a spec de `.gw.text` dentro dessa convenção |
| `GODS_LAWS.md` L-18, L-22, L-25, L-29 (`C-09`), L-30 | Leis vinculantes | Este contrato é a leitura operacional delas para o domínio de texto; não substitui nenhuma |

**O que este contrato NÃO decide, porque é decisão de outro item ou do líder:**

- O formato de arquivo fonte e o layout do envelope binário selado — `B9`.
- A biblioteca ou algoritmo exatos do parser ICU-lite — implementação de `B9`/`domain/i18n/`.
- Se e quando migrar `{0}`/`{1}` existentes para placeholder nomeado — pode ser gradual (§6.3).
- Esquema de identificador de locale para o terceiro idioma em diante — recomendação dada, decisão
  fica para quando o terceiro locale for despachado (§2.1).
- Fonte/glifo por locale na camada de apresentação — registrado como risco (§8), decisão de quando
  `present/` for desenhado, fora do escopo da L-06 hoje.

---

## 10. Checklist de aceite para `B9`

O formato que `B9` desenhar só cumpre este contrato se, para os itens abaixo, a resposta for sim:

- [ ] A lista de locales é lida de um registro de dados, não de um número fixo de campos ou de
      `if`/`switch` por identificador de idioma no código.
- [ ] Cada locale carrega os campos do §2.2 (identificador, é-referência, cadeia de fallback,
      categorias de plural), mesmo que hoje só dois locales existam e os campos sejam triviais.
- [ ] O comportamento dos três casos do §3.4 é implementado e é **auditável**: dá para provar, por
      teste, que locale ausente cai na referência, que chave vazia sobe a cadeia, e que chave
      inexistente aparece crua com log de erro.
- [ ] O locale de referência (`pt_br`) é a única entrada cuja completude é exigida em CI (gate de
      paridade estrutural, já citado em `README.md`, e a auditoria de conteúdo vazio é aceitável só
      para locales não-referência).
- [ ] A chave nunca é o campo que muda quando o idioma muda — só o valor. Nenhuma lógica no
      formato deriva comportamento do **nome** de uma chave (fora do prefixo de domínio do §4.1,
      que é convenção de organização, não de runtime).
- [ ] O parser aceita `{arg}` nomeado e o subconjunto ICU de `plural`/`select` do §6.2 (ou a
      alternativa que o líder aprovar no lugar).
- [ ] Nada do texto do jogador chega em texto puro na distribuição — a fonte fica no repositório,
      o artefato entregue é o pacote compilado e selado (L-18, L-25, ADR-012 §8), sem exceção para
      locale nenhum, inclusive os que ainda não existem.

---

## 11. Pendências registradas neste documento

1. Adoção do subconjunto ICU MessageFormat como sintaxe de placeholder/plural/gênero — recomendação
   do §6.2, decisão do líder.
2. Esquema de identificador para locales futuros (BCP 47 ou manter convenção livre) — §2.1.
3. Fonte por locale na camada de apresentação, quando `present/` nascer — risco declarado no §8,
   não decidido aqui.
4. Momento de migrar placeholder posicional para nomeado nas chaves já publicadas — §6.3, pode ser
   gradual.
