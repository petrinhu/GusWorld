# Parecer do CLO (Cláudio): statline de carta, AGPL e o catálogo compilado

> **Status: DECIDIDO em 01/09/2026, por `AskUserQuestion` (L-11).** O líder escolheu a **Opção A** (§4): o catálogo é fatiado por natureza jurídica, e o texto de sabor mora fora do executável, em pacote binário selado. Ver §11 para o registro completo da decisão.
>
> **Natureza deste documento:** orientação jurídico-técnica que fundamentou a decisão do líder. Não é aconselhamento jurídico vinculante e não substitui advogado humano. A seção 10 diz exatamente onde um advogado de carne e osso é necessário.
>
> **Método:** cada afirmação vem marcada como **FATO** (com citação de arquivo, linha ou texto de licença com URL) ou **INFERÊNCIA** (leitura minha, defensável mas não garantida).

---

## 0. Fatos de partida verificados

1. **FATO.** L-08 (`GODS_LAWS.md:120-128`): código do GusWorld sob **AGPL-3.0-or-later**, com ressalva explicativa de jogo offline exigida verbatim pelo líder; assets no regime herdado do `ASSETS-LICENSE.md` (Zona 1 CC-BY-SA 4.0 irrevogável até 31/07/2026; Zona 2 direitos reservados a partir de 01/08/2026; Zona 3 livros reservados; marca fora de qualquer concessão).
2. **FATO.** L-25 (`GODS_LAWS.md:355-382`): fase 2 prevê **catálogo compilado dentro do executável**; a fonte do conteúdo fica em **texto estruturado dentro do repositório**, lida só pelo gerador de build; a pendência deste parecer está registrada em `GODS_LAWS.md:382` e `:391`.
3. **FATO.** O repositório atual **não tem nenhum commit** (git status: só arquivos untracked). Nenhum asset e nenhuma linha de código foram publicados a partir deste repositório. Nada foi licenciado a ninguém **a partir daqui**.
4. **FATO.** `NOTICE:4-5` declara **Apache License 2.0**, contrariando a L-08. `ASSETS-LICENSE.md:13` idem ("Código-fonte | Apache License 2.0"), e cita arquivos que não existem neste repositório: `LICENSE`, `README.md`, `THIRD-PARTY-LICENSES.md`, `docs/tech/adr/ADR-005...`, `ADR-021...` (linhas 3, 13, 37, 116, 159). `AI-DISCLOSURE.md` cita `README.md` e `ACKNOWLEDGMENTS.md` inexistentes (linhas 3, 30) e contém o placeholder `__DEP_REMOVIDA__`.
5. **FATO (texto da licença).** AGPL-3.0, seção 0: *"A 'covered work' means either the unmodified Program or a work based on the Program."* Seção 5c: *"You must license the entire work, as a whole, under this License to anyone who comes into possession of a copy."* Seção 5, parágrafo final (definição de agregado): *"A compilation of a covered work with other separate and independent works, which are not by their nature extensions of the covered work, and which are not combined with it such as to form a larger program, in or on a volume of a storage or distribution medium, is called an 'aggregate' if the compilation and its resulting copyright are not used to limit the access or legal rights of the compilation's users beyond what the individual works permit."* Seção 13 (a cláusula de rede): *"If you modify the Program, your modified version must prominently offer all users interacting with it remotely through a computer network (if your version supports such interaction) an opportunity to receive the Corresponding Source..."*. Fonte: <https://www.gnu.org/licenses/agpl-3.0.en.html>.
6. **FATO (FAQ da FSF).** <https://www.gnu.org/licenses/gpl-faq.html#MereAggregation>: *"If the modules are included in the same executable file, they are definitely combined in one program."* E <https://www.gnu.org/licenses/gpl-faq.html#WhatCaseIsOutputGPL> reconhece a prática de jogos com dados sob licença própria: *"some programs, particularly video games, can have artwork/audio that is licensed separately from the underlying GPLed game."*
7. **FATO (lei brasileira).** Lei 9.610/98, art. 8º: **não** são objeto de proteção autoral, entre outros, *"I - as ideias, procedimentos normativos, sistemas, métodos..."*, *"II - os esquemas, planos ou regras para realizar atos mentais, jogos ou negócios"* e *"VI - os nomes e títulos isolados"*. Art. 7º, XIII: bases de dados e compilações **são** protegidas quando, *"por sua seleção, organização ou disposição de seu conteúdo, constituam uma criação intelectual"*. Fonte: <https://www.planalto.gov.br/ccivil_03/leis/l9610.htm>. No direito americano, no mesmo sentido: Feist v. Rural, 499 U.S. 340 (1991) (fatos não são protegíveis) e a orientação do US Copyright Office sobre jogos (*"Copyright does not protect the idea for a game... or the method or methods for playing it"*, <https://www.copyright.gov/registration/other-digital-content/>, circular de Games).
8. **FATO (fato relevante de titularidade).** O GlintFx é do próprio líder (`github.com/petrinhu/GlintFx`, Lei Zero). Hoje **não existe titular externo de copyright** em nenhuma das duas pontas da combinação. Isso muda o perfil de risco (seção 3.4).

---

## 1. Questão 1: statline é dado ou obra?

**Resposta curta: as duas coisas, por campo. A pergunta "código ou asset" não tem resposta única por carta; tem resposta por camada.**

Uma statline (nome, custo, poder, velocidade, efeito, texto de sabor) é um sanduíche de naturezas jurídicas distintas:

| Campo | Natureza | Proteção autoral |
|---|---|---|
| custo, poder, velocidade e demais números | regra de jogo / fato de balanceamento | **Nenhuma isolada.** Art. 8º, II da Lei 9.610 exclui expressamente *"regras para realizar... jogos"*; Feist exclui fatos. Um `3` não é expressão. |
| nome da carta | título isolado | **Improvável isolada** (art. 8º, VI). Mas nomes de lore tocam **marca** e concorrência desleal, que são outro regime (carve-out já existente). |
| efeito (a mecânica) | regra de jogo | A **regra em si** não é protegível (art. 8º, II). A **redação** do texto de regra tem proteção fina; a **implementação em C++** (os átomos da L-04) é código, protegível, e será AGPL de qualquer jeito. |
| texto de sabor | obra literária | **Proteção plena** (art. 7º, I). É o campo que carrega de verdade o valor da Zona 2. |
| o catálogo como conjunto | base de dados | **Protegível como compilação** pela seleção e organização (art. 7º, XIII), independente da proteção de cada item. |

**INFERÊNCIA central:** o que a Zona 2 (direitos reservados) consegue proteger de fato numa statline é a **camada expressiva**: texto de sabor, descrições, nomes no contexto da lore, e o catálogo como compilação. Os números e as regras já nasceriam sem proteção autoral; reservá-los tem efeito jurídico quase nulo. Isso importa para as saídas da seção 4: ceder a camada de regra à AGPL custa pouco, porque o direito autoral não a protegia; segurar a camada literária custa pouco à AGPL, porque ela não é software.

---

## 2. Questão 2: obra combinada ou mera agregação?

### 2.1 O que a licença diz

A AGPL cobre a *covered work* (seção 0) e, ao distribuir obra baseada no Programa, exige licenciar *"the entire work, as a whole"* (seção 5c). A válvula de escape é o **agregado** (seção 5, parágrafo final): obras separadas e independentes, que **não são por natureza extensões da covered work** e que **não são combinadas de modo a formar um programa maior**, distribuídas juntas *"in or on a volume of a storage or distribution medium"*, não são arrastadas pelo copyleft.

### 2.2 Aplicação aos três cenários perguntados

**(a) Dado em arquivo separado ao lado do executável.** É o cenário clássico de agregação, e é a prática consolidada da indústria: id Software liberou os engines de Doom e Quake sob GPL mantendo os arquivos de dados (WAD/PAK) proprietários, e o próprio FAQ da FSF reconhece jogos com *"artwork/audio that is licensed separately from the underlying GPLed game"* (#WhatCaseIsOutputGPL). Dado num arquivo lido pelo programa não é extensão do programa nem forma um programa maior. **INFERÊNCIA: risco baixo, posição confortável.** É exatamente o desenho da fase 1 da L-25 (pacotes binários selados).

**(b) Dado embutido no binário (tabelas C++ geradas e linkadas).** Aqui as duas leituras colidem:

- **Leitura da FSF (contra nós):** *"If the modules are included in the same executable file, they are definitely combined in one program"* (#MereAggregation). O executável é um arquivo só; a exceção de agregado fala em "volume of a storage or distribution medium", e um executável não é um meio de armazenamento, é um programa. Por essa leitura, tudo que está dentro do executável integra a obra que a seção 5c manda licenciar por inteiro.
- **Leitura de direito autoral estrito (a nosso favor):** copyleft só alcança o que o direito autoral alcança. Dado não é módulo de software; justaposição de bytes num arquivo não transforma texto literário em obra derivada de código, do mesmo jeito que imprimir um poema na contracapa de um manual não faz o poema derivar do manual. Um tribunal poderia tratar o binário como compilação (coletânea) de duas obras, não como obra única.

**INFERÊNCIA: cenário juridicamente cinzento, sem jurisprudência que o resolva, e a leitura da própria autora da licença pesa contra embutir conteúdo reservado num binário AGPL.** E o array C++ gerado é o **pior subcaso**: a fonte gerada é sintaticamente indistinguível de código, vai compilada e linkada, e a fronteira obra/dado fica invisível para quem recebe o repositório e o binário.

**(c) O problema prático que existe mesmo sem tribunal.** Se o binário contém conteúdo "todos os direitos reservados", o destinatário **não pode redistribuir o binário** (falta licença para a parte reservada). Só que a AGPL promete a ele exatamente essa liberdade sobre a obra coberta. Ou o conteúdo embutido está sob AGPL (e a reserva da Zona 2 morre para ele), ou o binário anunciado como AGPL não é redistribuível como um todo (e a promessa da licença fica falsa na prática). **Não dá para ter os dois no mesmo arquivo sem uma exceção explícita e bem redigida.** Distros, empacotadores e o F-Droid da vida tratam esse tipo de binário como "non-free" ou simplesmente recusam.

### 2.3 A nuance de quem pode reclamar

**FATO:** hoje o líder é titular único do código do GusWorld **e** do GlintFx. Ninguém tem legitimidade para acusá-lo de violar o próprio copyleft; titular não infringe a própria licença. O risco atual não é processo: é **incoerência do regime** (2.2.c) e **bomba-relógio**: no dia em que o GlintFx aceitar a primeira contribuição externa sob AGPL, nasce um cotitular terceiro com legitimidade para exigir que o binário combinado seja integralmente AGPL. **INFERÊNCIA: desenhar hoje contando com a titularidade única é construir sobre uma condição que o próprio roadmap FOSS do GlintFx tende a desfazer.**

---

## 3. Questão 3: a fonte do conteúdo no repositório público

1. **Estar no repositório não licencia.** Licença é declaração do titular, não osmose de diretório. Publicar no GitHub concede a terceiros apenas o que o Termo de Serviço do GitHub concede: ver, e forkar **dentro do GitHub** (GitHub ToS, seção D.5, *License Grant to Other Users*: <https://docs.github.com/en/site-policy/github-terms/github-terms-of-service>). Portanto a fonte do conteúdo em texto no repo (L-25) **não vira AGPL automaticamente**.
2. **Mas a ambiguidade trabalha contra o titular.** Um repo com `LICENSE` AGPL na raiz e sem marcação por caminho cria a presunção prática (detectores do GitHub, distros, agregadores) de que **tudo** ali é AGPL. Quem quer dois regimes num repo só precisa marcar a fronteira de forma legível por máquina.
3. **Prática consolidada, a que recomendo:** a especificação **REUSE** (<https://reuse.software/spec/>, spec 3.3) com identificadores **SPDX** (<https://spdx.org/licenses/>): cabeçalho `SPDX-License-Identifier:` + `SPDX-FileCopyrightText:` em cada arquivo; arquivo companheiro `.license` para binários e assets; `REUSE.toml` para regras por diretório inteiro; textos integrais em `LICENSES/`. Para a Zona 2 usa-se um identificador próprio, por exemplo `LicenseRef-GusWorld-Assets-Reserved`, com o texto da reserva em `LICENSES/`. Projetos reais com código e conteúdo em regimes distintos no mesmo repo: **0 A.D.** (código GPL-2.0+, arte CC-BY-SA 3.0), **Battle for Wesnoth** (código GPL, arte CC-BY-SA 4.0), e o modelo Doom/Quake (engine GPL no repo, dados proprietários fora).
4. **Aviso honesto, coerente com a L-25:** conteúdo reservado num repo público fica **copiável de fato**; a defesa é jurídica, não técnica. É a mesma verdade que a L-25 já assume para o executável (prevenir é impossível, detectar é alcançável).

---

## 4. Se há conflito, e as saídas

**Há conflito, sim, mas só no ponto onde a fase 2 da L-25 (catálogo compilado) encontra a Zona 2 (conteúdo novo reservado): o texto de sabor e a camada literária das cartas criadas depois de 01/08/2026 seriam embutidos num executável AGPL.** Os números e regras não geram conflito real (não são protegíveis); a arte e a música não geram conflito (não são compiladas no executável); os livros não geram conflito (nem entram no repo do jogo do jeito atual).

### Opção A (a que recomendo): fatiar o catálogo por natureza jurídica

- **Camada de mecânica** (identificadores, custo, poder, velocidade, efeito como parâmetro/regra): tratada como **código**, AGPL, compilada no executável. A fase 2 da L-25 fica **intacta** para o que ela mais protege (o balanceamento, o replay, a verificação por re-execução).
- **Camada literária** (texto de sabor, descrições de lore): tratada como **asset Zona 2**, fora do executável, servida pelo **pacote binário selado** da fase 1 da L-25, com uma **permissão expressa e estreita** de redistribuir o pacote **inalterado, junto do jogo, gratuitamente** (para não quebrar mirrors e empacotadores).
- **Custo:** o gerador de build produz dois artefatos em vez de um; o conteúdo de uma carta vive em dois regimes; a permissão de redistribuição precisa de redação cuidadosa (advogado, seção 10). Complexidade real, mas localizada no gerador.
- **Ganho:** elimina a contradição do binário (2.2.c), preserva a Zona 2 exatamente na camada que o direito autoral protege de verdade, e mantém a leitura FSF do nosso lado (o que está no executável é AGPL sem asterisco; o que é reservado está agregado ao lado, cenário 2.2.a).

### Opção B: catálogo inteiro sob AGPL, tudo compilado

- Statline completa, sabor incluído, licenciada AGPL como parte do programa. Zona 2 passa a valer só para arte, música e livros.
- **Custo:** texto de carta novo vira copyleft; qualquer fork pode reutilizar a escrita das cartas com atribuição e share-alike. É abrir mão declarado de um pedaço da Zona 2, decisão que só o líder pode tomar.
- **Ganho:** o regime mais simples e mais blindado juridicamente que existe; um executável, uma licença, zero exceção; a L-25 fase 2 intacta por inteiro.

### Opção C: catálogo inteiro fora do executável, em pacote selado reservado

- Nada de catálogo compilado; tudo (mecânica e sabor) vai no pacote binário selado, reservado, com a permissão estreita de redistribuição.
- **Custo:** **revisa a fase 2 da L-25** (o "catálogo compilado dentro do executável" deixa de existir), o que exige decisão expressa do líder alterando a lei (L-24: o texto antigo se apaga). Tecnicamente, a proteção anti-edição não piora tanto quanto parece (o selo criptográfico é o que protege, não o lugar dos bytes), mas o desenho aprovado muda.
- **Ganho:** reserva máxima do conteúdo; fronteira jurídica limpíssima (cenário 2.2.a).

### Opção D (não recomendo): manter tudo compilado e declarar exceção

- Embutir tudo e publicar uma declaração de que o conteúdo embutido é obra separada agregada dentro do binário, com permissão de redistribuir o binário oficial inalterado.
- **Custo:** é a posição mais frágil (contraria frontalmente o #MereAggregation da FSF), só se sustenta enquanto o líder for titular único das duas pontas (2.3), confunde distros e empacotadores, e vira passivo no dia em que o GlintFx tiver contribuidor externo.

**Comum a todas:** a escolha exige atualizar `ASSETS-LICENSE.md` e, conforme o caso, a L-25 (canon primeiro, L-13), e a marcação REUSE/SPDX da seção 3 vale para qualquer uma.

---

## 5. O NOTICE corrigido

**FATO:** `NOTICE` é um mecanismo da Apache 2.0 (seção 4(d) daquela licença); a AGPL não tem mecanismo NOTICE. Sob AGPL o arquivo pode continuar existindo como aviso de copyright, de licenciamento e de marca (as *"Appropriate Legal Notices"* das seções 0 e 5 da AGPL), mas o texto atual está errado (diz Apache 2.0). Texto proposto, em inglês como o atual:

```
GusWorld
Copyright (C) 2026 petrinhu

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version. See the LICENSE file for the full text.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
General Public License for details.

Game content, artwork, music and the companion books are covered by
separate terms. See ASSETS-LICENSE.md.

Trademark notice: the license above covers copyright only. It does not
grant any right to the GusWorld name, its logo, its trade dress, or its
character names as identifiers of origin. Forks and redistributions must
present themselves under a different name. See ASSETS-LICENSE.md.

GusWorld is a single-player, offline game. See OFFLINE-NOTICE.md for a
plain-language note about what the AGPL's network clause does and does
not mean for players. That note explains the license; it does not modify
it.
```

**INFERÊNCIA de redação:** o carve-out de marca como **aviso** (dizendo o que a licença não concede) é seguro; a AGPL não concede direitos de marca mesmo. A frase "must present themselves under a different name" é defensável como exercício do direito de marca, mas é o tipo de frase que um advogado de marcas deve validar (seção 10), porque não pode ser lida como restrição adicional ao copyright licenciado (AGPL seção 10 proíbe *further restrictions* sobre a obra coberta; marca é outro direito, e é exatamente por isso que a frase se sustenta).

---

## 6. A ressalva de jogo offline (L-08)

**Princípio de redação:** a ressalva **explica** e **não modifica**. Ela não pode dizer "a seção 13 não se aplica a este software" em absoluto, porque se aplica sim a quem modificar o jogo e o oferecer por rede. Ela diz que, no uso normal do jogo como distribuído, a cláusula não tem o que alcançar.

### Onde mora

- **README**, na seção de licença (versão curta ou integral), e
- **arquivo próprio** `OFFLINE-NOTICE.md` (a "nota própria" que a L-08 exige), bilíngue.

### Onde NÃO pode morar

- Dentro do `LICENSE`/`COPYING` (o texto da AGPL é imutável; a própria L-08 já proíbe).
- Como "termo adicional" ou cláusula com linguagem normativa anexada à licença (viraria tentativa de modificar a AGPL ou *further restriction*).
- Em cabeçalho SPDX ou em qualquer lugar que a faça parecer parte dos termos.

### Texto em pt-br

> **Nota sobre a licença: este jogo é offline**
>
> O GusWorld é um jogo single-player e offline. Ele não tem servidor, não tem conta, não tem loja embutida e não envia nem recebe nada pela internet enquanto você joga (sem telemetria). Se você procurar um modo online, ele não existe; não é defeito, é o desenho do jogo.
>
> O código do jogo usa a licença GNU AGPL, versão 3 ou posterior. A AGPL tem uma cláusula (a seção 13) que só fala de programas oferecidos a usuários através de uma rede. Como o GusWorld não funciona por rede, essa cláusula não muda nada para quem simplesmente joga: você não precisa fazer nada, aceitar nada nem configurar nada. Ela só passa a valer para alguém que modifique o código e ofereça a versão modificada como um serviço pela internet; essa pessoa, aí sim, precisa oferecer o código-fonte dela aos usuários do serviço.
>
> Esta nota existe para explicar a licença em linguagem simples. Ela não altera, não limita e não substitui o texto da licença, que está no arquivo `LICENSE` e prevalece sempre.

### Text in English

> **A note about the license: this game is offline**
>
> GusWorld is a single-player, offline game. It has no server, no account, no built-in store, and it does not send or receive anything over the internet while you play (no telemetry). If you are looking for an online mode, there is none; that is by design, not a bug.
>
> The game's code is licensed under the GNU AGPL, version 3 or later. The AGPL has one clause (section 13) that only concerns programs offered to users over a network. Since GusWorld does not operate over a network, that clause changes nothing for someone who simply plays the game: you do not have to do, accept, or configure anything. It only becomes relevant to someone who modifies the code and offers the modified version as a service over the internet; that person must then offer their source code to the users of that service.
>
> This note exists to explain the license in plain language. It does not alter, limit, or replace the license text, which lives in the `LICENSE` file and always prevails.

---

## 7. A L-24 aplicada aos documentos de licença

A L-24 manda apagar o revogado, com uma única exceção: *"texto cuja remoção teria efeito jurídico ou de direito adquirido"* (`GODS_LAWS.md:351`). Aplicação, item a item do `ASSETS-LICENSE.md` e do `NOTICE`:

### NÃO apagar (exceção da L-24; levar ao líder qualquer mudança)

1. **Declaração da Zona 1 (CC-BY-SA irrevogável), linhas 41-63.** É o registro do grant já recebido por terceiros no projeto anterior e será o **aviso operativo** dos assets Zona 1 que entrarem neste repositório (`assets/` e `resources/` já estão na árvore). Apagar não revogaria nada (a licença acompanha quem já recebeu, com ou sem o documento), mas criaria aparência de revogação e apagaria o aviso que a CC-BY-SA exige manter. Fica.
2. **Zona 2, Zona 3 e o carve-out de marca (linhas 67-116).** São o regime vigente confirmado pela L-08. Ficam (atualizados nas referências mortas).
3. **A permissão de conteúdo de fã (linhas 120-136).** É uma **licença já concedida ao público**. Pode ser retirada para o futuro, nunca para usos já feitos. Se o líder quiser mantê-la (recomendo, é boa vontade de graça), permanece; se quiser revogá-la, o caminho não é deleção silenciosa, é nota de revogação prospectiva com data, e isso passa por advogado. Enquanto o líder não decidir, **não apagar**.
4. **O parágrafo de retroatividade das licenças antigas do código (linhas 29-35), condicionado.** Os direitos que ele descreve (releases antigas sob GPLv3/AGPL/Apache) são de **outra obra** (L-01: o código daqui nasce do zero) e não podem ser desfeitos por ninguém, com ou sem o parágrafo. **Se o repositório antigo (ou suas tags) permanece acessível com a própria documentação de licença, este parágrafo pode ser removido daqui ou reduzido a uma linha**, porque o registro sobrevive onde os direitos nasceram. Se o repo antigo for apagado ou fechado, o registro deve sobreviver em algum lugar, e aí este parágrafo é o candidato. A condição é fato que só o líder conhece (o destino do repo antigo), então o caso vai a ele.

### PODE apagar (história morta sem efeito, mediante ordem do líder, L-14)

1. **A regra de fronteira dos arquivos Godot `.tscn`/`.tres` (linhas 140-153).** Ela mesma se declara histórica e diz não alcançar nenhum arquivo do repositório atual. A licença dos arquivos antigos vive no repo/tag antigos, não nesta prosa. Apagável sem efeito jurídico.
2. **A seção "Código = Apache License 2.0" (linhas 21-37) e o `NOTICE` atual inteiro.** Estão **errados** para este repositório (L-08 manda AGPL) e nada foi publicado daqui sob Apache: substituí-los não toca direito de ninguém. Não é nem "apagar história", é corrigir um documento que descreve um regime que nunca vigorou nesta obra.
3. **As referências a arquivos inexistentes** (`ADR-005`, `ADR-021`, `LICENSE`, `THIRD-PARTY-LICENSES.md`, `docs/tech/ai-assets-provenance.md`): trocar por menção datada sem link, ou remover, conforme o líder decidir o destino das citações órfãs (pendência já registrada em `GODS_LAWS.md:396`).

---

## 8. Arquivos de licença que precisam existir antes do primeiro commit

Mesmo gate pré-commit da L-15 (`.gitignore`/`.gitattributes`): depois do primeiro push, corrigir custa reescrita de histórico.

| # | Arquivo | Conteúdo |
|---|---|---|
| 1 | `LICENSE` | Texto **verbatim e imutável** da GNU AGPL-3.0 (<https://www.gnu.org/licenses/agpl-3.0.txt>). O "or-later" não se escreve no texto da licença; declara-se nos cabeçalhos SPDX e no NOTICE ("either version 3... or any later version"). |
| 2 | `LICENSES/` (REUSE) | `AGPL-3.0-or-later.txt`; `CC-BY-SA-4.0.txt` (se assets Zona 1 entram no repo); `LicenseRef-GusWorld-Assets-Reserved.txt` com o texto curto da reserva da Zona 2/3. |
| 3 | `REUSE.toml` | Regras por diretório: código sob `AGPL-3.0-or-later`; assets Zona 1 sob `CC-BY-SA-4.0`; assets Zona 2 sob `LicenseRef-GusWorld-Assets-Reserved`; o diretório da fonte do catálogo conforme a opção que o líder escolher na seção 4. |
| 4 | `ASSETS-LICENSE.md` | Reescrito: código passa a AGPL-3.0-or-later; links mortos corrigidos; a fronteira do catálogo registrada conforme a decisão da seção 4; itens da seção 7 tratados conforme o líder mandar. |
| 5 | `NOTICE` | O texto proposto na seção 5. |
| 6 | `README.md` | Seção de licença: código AGPL (link ao LICENSE), assets (link ao ASSETS-LICENSE.md), marca, e a ressalva offline (curta, com link ao OFFLINE-NOTICE.md). |
| 7 | `OFFLINE-NOTICE.md` | A nota bilíngue da seção 6. |
| 8 | `THIRD-PARTY-LICENSES.md` | Mínimo hoje: GlintFx, AGPL-3.0-or-later, titular petrinhu, link do repo. Cresce com cada dependência futura. |
| 9 | `AI-DISCLOSURE.md` | ⚠️ **NÃO CUMPRIDO na data em que este parecer declarou correção; conferido em 24/08/2026.** Os dois ponteiros continuavam mortos: `ACKNOWLEDGMENTS.md` nunca foi criado e o `README.md` não tinha seção de créditos, e mesmo assim a frase linkava os dois, em inglês e em pt-br. **Resolvido em 24/08/2026 por decisão do líder:** os dois ponteiros foram removidos e a frase passou a apresentar direto a lista que já vinha logo abaixo. O destino das ocorrências de termo removido segue pendência do líder. |
| 10 | Cabeçalhos por arquivo-fonte | `// SPDX-FileCopyrightText: 2026 petrinhu` + `// SPDX-License-Identifier: AGPL-3.0-or-later` em cada fonte novo, desde o primeiro. |

---

## 9. Resumo executivo para o líder

1. **Statline não é uma coisa só:** números e regras não têm proteção autoral (a lei brasileira exclui regras de jogo expressamente); texto de sabor é obra literária plena. A Zona 2 só protege de verdade a camada literária.
2. **Embutir conteúdo reservado no executável AGPL é a única combinação que conflita.** Arquivo selado ao lado do executável é agregação tranquila (modelo Doom/Quake, reconhecido pela própria FSF). Dentro do executável, a leitura da FSF diz "um programa só", e o binário viraria uma autocontradição: anunciado AGPL, mas não redistribuível por inteiro.
3. **Saídas:** A) fatiar (mecânica compilada sob AGPL, texto de sabor em pacote selado reservado) — recomendo; B) catálogo todo AGPL — mais simples, cede o texto das cartas ao copyleft; C) catálogo todo fora do executável — reserva máxima, revisa a fase 2 da L-25; D) tudo dentro com exceção declarada — frágil, não recomendo.
4. **Hoje ninguém pode processar ninguém** (o líder é titular das duas pontas), mas a primeira contribuição externa ao GlintFx muda isso. O desenho deve nascer correto, não escorado na titularidade única.
5. **NOTICE e ASSETS-LICENSE estão errados** (dizem Apache) e o repo não tem os arquivos que eles citam. A seção 8 lista o kit completo pré-primeiro-commit.
6. **L-24:** Zona 1, Zonas 2/3, marca e permissão de fã não se apagam (direito concedido ou regime vigente); regra Godot e seção Apache são apagáveis/substituíveis mediante ordem do líder.

## 10. Onde o advogado humano é necessário de verdade

1. **Registro de marca no INPI** (nome, logotipo; classes 9 e 41 ao menos) e a validação da frase "forks must present themselves under a different name".
2. **Redação final da permissão estreita de redistribuição** do pacote de conteúdo reservado (opções A e C) e da permissão de conteúdo de fã.
3. **Se a opção D for escolhida** (contra minha recomendação), a exceção precisa ser redigida por advogado, porque é a posição que mais depende de nuance.
4. **Qualquer revogação prospectiva** de permissão já concedida ao público.
5. **Qualquer disputa real** envolvendo Zona 1/CC-BY-SA do projeto anterior.

## 11. Decisão do líder (01/09/2026)

**Decisão, por `AskUserQuestion` (L-11):** **Opção A** (§4) — o catálogo é fatiado por natureza jurídica. Verbatim: *"manter licença do projeto, cada carta um átomo separado, cada uma com seu POCO"*.

Perguntado onde mora o texto de sabor da carta, o líder escolheu **"Fora, em pacote selado"**, tendo lido esta descrição: *"O objeto da carta carrega número e regra, compilados sob a licença do código; o texto de sabor e a lore vão num pacote binário selado à parte, com direitos reservados."*

**O que fica fixado:**

- O regime de licença do projeto **não muda**: código sob AGPL, assets e lore com direitos reservados (L-08).
- **Cada carta continua sendo um átomo com objeto próprio** (L-04): o objeto carrega **número e regra**, e vai compilado sob AGPL.
- **Texto de sabor e lore ficam fora do objeto**, no pacote binário selado, sob direitos reservados.
- Fundamento jurídico (ver §1 e §7 acima, não recopiado aqui): número e regra de carta não têm proteção autoral no Brasil (Lei 9.610/98, art. 8º, II) nem nos Estados Unidos (Feist v. Rural; orientação do US Copyright Office sobre jogos).

Esta decisão confirma, sob a lente mais funda deste parecer, o mesmo desenho que a L-25 já registrava para o fatiamento do catálogo. As pendências da seção 8 (kit de arquivos de licença) e da seção 10 (onde advogado humano é necessário) seguem abertas.

*Cláudio, CLO. Orientação técnica; decisão do líder registrada em 01/09/2026 via `AskUserQuestion` (L-11).*
