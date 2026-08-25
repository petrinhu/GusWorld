> **LEI DAS LEIS, ANTERIOR ATÉ À LEI ZERO: só o líder pode quebrar uma lei deste arquivo** — agente nenhum quebra, flexibiliza, reinterpreta ou "adapta ao caso" por conta própria — **e nem a ordem direta dele dispensa a confirmação**: antes de executar, nomeie a lei que está sendo quebrada, cite o texto dela, diga o que ela protege e o que se perde ao quebrá-la, e pergunte por `AskUserQuestion` se é isso mesmo que ele quer; **quando o pedido for ALTERAR ou REVOGAR uma lei, argumente CONTRA primeiro, sempre e sem exceção**, com razões concretas, o problema que a lei existe para impedir, os trade-offs da mudança e o que fica desprotegido depois dela, e só então leve a escolha por `AskUserQuestion` entre **confirmar** a alteração e **cancelá-la**; pressa, obviedade aparente, "ele já mandou uma vez" e aprovação dada em outro contexto **nunca** substituem essa confirmação, e silêncio jamais vale como aval. (Ordem do líder, 22/08/2026.)

> **LEI ZERO, ACIMA DE TODAS: O JOGO ASSENTA EM GLINTFX, E QUEM É DONO DA JANELA E DO LAÇO É ELE.** O GusWorld liga em exatamente duas coisas: o **GlintFx** e o **sistema operacional**. Ordem do líder em 21/08/2026, verbatim: *"aceita apenas link com framework GlintFx em ../Glintfx e [github]/petrinhu/GlintFx e com o SO. [...] Você NUNCA cria workarounds nem passa por cima da camada de GlintFx. Se ainda não existir a funcao em GlintFx, registre a necessidade no bus e espere parado a resposta dele."* Qualquer análise, atalho ou desenho que contorne o GlintFx está **errado por construção**. Exceção a esta lei só existe se o líder a conceder nominalmente, caso a caso. — **A divisão de responsabilidade é a que o próprio GlintFx declara**, ordem do líder em 21/08/2026, verbatim: *"do modo que glintfx disse"*, apontando para a **L-02** do `GODS_LAWS.md` dele, que diz: *"framework 2D completo (janela, loop, render2d, input, gamepad, áudio, fonte, asset, math2d); **o consumidor escreve só a lógica dele**"*. Portanto: **janela, laço principal, entrada, áudio e desenho são do GlintFx. O GusWorld escreve a lógica do jogo, e mais nada.** O jogo **não** cria janela, **não** possui contexto gráfico e **não** roda laço próprio de quadro. — **Registro de decisão revogada, para ninguém ressuscitar:** o `ADR-010` do projeto anterior adotava o modelo inverso (o jogo dono da janela, o framework embutido como camada de interface). Aquele modelo só fazia sentido porque existia uma biblioteca externa criando a janela, e ela foi removida do ecossistema em 20/08/2026. O ADR-010 foi **apagado** do repositório em 21/08/2026 por ordem do líder, sob a L-24.

# GODS_LAWS.md

> Ordens expressas do líder (petrus). Este arquivo **não é declaração, é execução**: cada lei tem um **gatilho**, e o gatilho é conferido **no momento da ação**, não no fim.

## Protocolo de uso (obrigatório)

1. **Antes de agir**, varra a coluna "Gatilho" da tabela abaixo. Se algum gatilho casa com o que você está prestes a fazer, leia a lei inteira antes do primeiro comando, não depois.
2. **Ao despachar subagent**, cole no prompt da task o texto completo das leis cujo gatilho casa com aquela task, mais o caminho absoluto deste arquivo. Subagent **não herda** este contexto e não vai ler por conta própria.
3. **Ao relatar ao líder**, se você tocou uma área com lei, diga qual lei aplicou e como. Silêncio não é prova de conformidade.
4. **Lei nova entra aqui no instante em que o líder a dá**, com data e o texto dele verbatim entre aspas. Não espere "um momento melhor" para registrar.
5. **Nenhum agente revoga, flexibiliza ou reinterpreta lei.** Só o líder. Na dúvida sobre o alcance de uma lei, pergunte via `AskUserQuestion` antes de agir.
6. Conflito entre uma lei daqui e qualquer outro documento (manual, memória, hábito, preferência do agente): **a lei daqui vence**.

## Índice de gatilhos

| Lei | Gatilho: dispara quando você vai... | Resumo |
|---|---|---|
| [L-01](#l-01) | procurar código anterior, "como era antes", ou base para reaproveitar | O código é do zero; o GusWorld anterior não é base |
| [L-02](#l-02) | tocar arte, câmera, sprite, modelo ou pipeline visual | Jogo é 2D pixel-art; o pipeline 3D é do líder e agente não toca |
| [L-03](#l-03) | criar build, escolher padrão de linguagem | C++23 |
| [L-04](#l-04) | escrever função, arquivo, classe, módulo, item ou carta | Proibido monolito; cada elemento do jogo é átomo com POCO próprio |
| [L-05](#l-05) | precisar de algo que o GlintFx ainda não tem | Proibido dublê de plataforma: ou liga no GlintFx, ou não existe |
| [L-06](#l-06) | decidir o que construir primeiro | Núcleo puro agora; a camada que desenha só nasce com o GlintFx |
| [L-07](#l-07) | precisar de função do framework, ou receber ideia do Gus | Bus: pedido só quando o jogo esbarrar de verdade na falta |
| [L-08](#l-08) | criar `LICENSE`, cabeçalho de arquivo, versionar asset ou publicar | AGPL-3.0-or-later no código; assets e lore com todos os direitos reservados; livros à parte |
| [L-09](#l-09) | escrever CI ou declarar suporte de plataforma | Cinco alvos; Fedora 44 primário; CachyOS é próprio, não Arch |
| [L-10](#l-10) | ir executar qualquer trabalho de produto | Main só orquestra; C-level fable audita e cria; sonnet implementa |
| [L-11](#l-11) | escolher entre duas abordagens, ou notar qualquer dúvida | Nada é decidido por agente; opções ao líder via `AskUserQuestion` |
| [L-12](#l-12) | escrever qualquer mensagem ao líder | Timestamp real `[DD/MM/YY - HH:MM:SS]` obtido do `date` |
| [L-13](#l-13) | tocar em qualquer trabalho que dependa de canon | Canon desatualizado bloqueia: atualiza primeiro, trabalha depois |
| [L-14](#l-14) | concluir que algo está morto, obsoleto ou descartável | Nada é declarado morto por agente; a decisão é do líder |
| [L-15](#l-15) | versionar binário, asset pesado ou configurar o repositório | `livros/` e `glb/` fora do git; LFS no resto |
| [L-16](#l-16) | homenagear pessoa real, ou citar o filho do líder | Homenagem só com aceite; o filho aparece só como "Gus Dragon" |
| [L-17](#l-17) | criar módulo, arquivo, ou desenhar a forma de um sistema | Espinha de cinco camadas com gate de CI; regra como comando e evento |
| [L-18](#l-18) | escolher formato de arquivo de mapa, save, configuração ou item | Nada em formato de texto; máxima proteção contra edição |
| [L-19](#l-19) | commitar, fechar fatia, ou pensar em cobertura e formatação | Cinco portões de qualidade; TDD estrito; sem meta de cobertura |
| [L-20](#l-20) | escrever CI, ou declarar que uma plataforma funciona | Matriz das cinco plataformas desde o primeiro commit |
| [L-21](#l-21) | achar que o `inicial.md` cumpriu o papel dele | Só se apaga com as quatro condições satisfeitas e o líder confirmando |
| [L-22](#l-22) | nomear qualquer coisa, ou escrever comentário e commit | Identificador e comentário em inglês, `snake_case`; commit em pt-br |
| [L-23](#l-23) | criar tag, publicar release, ou mexer na versão | Versão e tag são `vA.B.C.D`, como no GlintFx |
| [L-24](#l-24) | ver regra, seção ou documento que o líder revogou | Revogado se APAGA; não se guarda como histórico |
| [L-25](#l-25) | tocar save, configuração, mapa ou catálogo de conteúdo | Envelope binário selado, teto técnico faseado, cripto vem do GlintFx |
| [L-26](#l-26) | tocar câmera, mapa, locomoção ou direção de sprite | 3/4 top-down fixa, quatro direções cardeais, grade quadrada |
| [L-27](#l-27) | pensar em escrever tela, HUD ou marcação de interface | Nenhuma interface se escreve antes de o GlintFx traduzir marcação |
| [L-28](#l-28) | precisar baixar, instalar ou atualizar qualquer coisa | Pergunte ao líder; nunca falhe calado; `sudo` sempre com `-A` |
| [L-29](#l-29) | propor recurso, sistema ou escopo novo | Os 13 cortes: a cerca do que o jogo NÃO é |
| [L-30](#l-30) | escrever, reordenar ou acrescentar item na tabela | Todo item aponta para o documento que o especifica, se existir |
| [L-31](#l-31) | o líder aprovar, rejeitar ou mudar algo, ou fechar item de alta prioridade | Avisar o Gus Dragon sem ele perguntar |
| [L-32](#l-32) | fechar uma fatia, fechar uma onda, ou pensar em `git push` | Commit por fatia; push só com verificação automática e testes verdes |
| [L-33](#l-33) | criar unidade nova, escrever documento, teste, commit, item ou asset, **ou revisar fatia** | Atomizar fora do código; monolito é acoplamento, não tamanho; cinco perguntas na revisão |
| [L-34](#l-34) | ver qualquer coisa vinda do Gus Dragon, em qualquer dos cinco canais | Pedido dele é prioridade e é SEMPRE respondido; ack imediato, interrompe onda |


---

## L-01

**Data:** 21/08/2026. **Verbatim:** *"ja tentei fazer esse jogo uma vez com o claude, mas tive vários problemas e estou fazendo DO ZERO tudo com relacao ao codigo, sempre assentado sobre GlintFx"*.

O **código** do GusWorld nasce sem herança. O repositório não tem nenhum commit e não existe uma linha de código. O `GusEngine/` citado no corpus, o protótipo em Godot e C#, e qualquer árvore anterior **não são base, não são referência, não são canon**.

**O que isto NÃO alcança:** o corpus de documentação, lore e design em `docs/` e `resources/` **continua valendo** e é o insumo do projeto. Esta lei é sobre código.

**Aplicação:** ao encontrar no corpus uma referência a arquivo de código (`GusEngine/domain/...`, `.hpp`, `.cs`), trate como descrição de intenção, não como arquivo consultável: ele não existe. Os campos de uma entidade se derivam da prosa do documento, e a derivação vai ao líder.

## L-02

**Data:** 21/08/2026. **Verbatim:** *"jogo, 2d"*, *"pixelado mas luxuoso"*, e sobre os modelos 3D no repositório: *"os arquivos glb são arte conceitual. Eu gero o 3d, tiro print das várias posicoes, depois pixelizo com pixellab. E vou usar as imagens no livro e em impressora 3d. PARA O JOGO, ignore, esse pipeline é meu, a nao ser que eu peça sua ajuda"*.

O jogo é **2D pixel-art em runtime**. O pipeline 3D existe, é **do líder**, e alimenta livro e impressão 3D, não o jogo.

**Aplicação:** agente não gera, não converte, não organiza e não opina sobre o pipeline 3D sem pedido explícito dele. Nenhum modelo, mesh, câmera 3D ou bake entra no jogo. Passagem do corpus que pressupõe 3D em runtime é canon desatualizado, e cai na L-13.

## L-03

**Data:** 21/08/2026. **Verbatim:** *"jogo, 2d, c++23"*.

**C++23.** Sem exceção de linguagem sem ordem do líder.

## L-04

**Data:** 21/08/2026. **Verbatim:** *"PROIBIDO monolitos. OS itens/cartas e demais elementos do jogo devem ser atomos com POCO proprio!"*.

**Monolito é proibido em todo nível:** função que faz mais de uma coisa, arquivo que reúne assuntos sem relação, classe que cresce até virar dona de tudo, módulo sem fronteira declarada.

**Cada elemento do jogo é um átomo com POCO próprio.** Carta, item, inimigo, efeito de status, diálogo, missão: cada um é um tipo de dado próprio, com nome próprio, e não um campo dentro de um agregado que sabe tudo.

**Aplicação, no momento de escrever:**

- Se você consegue extrair uma sub-função com **nome próprio e honesto**, ela não era um átomo. Extraia.
- Se o nome precisa de **"e"** para ser verdadeiro (`carrega_e_valida`), são duas funções.
- Se um trecho precisa de comentário explicando **o que** faz, esse trecho é uma função sem nome. Dê o nome. Comentário existe para o **porquê**.
- Arquivo é átomo de assunto: um assunto por arquivo, e o nome do arquivo diz qual.
- Assunto novo nasce em módulo estreito próprio, nunca é acrescentado a um tipo existente porque "cabia lá".

**O que a lei não é:** licença para fatiar em funções de uma linha sem sentido próprio, nem para criar abstração especulativa. Átomo é a menor unidade **com significado**, não a menor unidade possível.

## L-05

**Data:** 21/08/2026. **Verbatim:** *"nao quero nada falso. Ou liga com glintfx ou nao liga. Isso que estragou o projeto todo anterior, vc ficou se confundindo e quebrou tudo."*

**Dublê de plataforma é proibido.** Nada de stub, mock, falso, simulacro, camada nossa embrulhando a do framework, `#ifdef` escolhendo entre GlintFx e outra coisa, ou modo headless que finge o que o framework faria. **Se não liga no GlintFx, não existe.**

**O que a lei NÃO proíbe:** regra de jogo pura. Uma função que recebe estado e devolve estado (dano, custo de carta, sorteio de efeito, regra de deck) **não liga em nada porque não há o que ligar**: ela não desenha, não toca som, não lê tecla e não abre arquivo. Isso não é dublê, é aritmética do jogo, e o teste dela chama a função com dados.

**A fronteira, dita de forma operacional:** se o código precisaria do GlintFx para funcionar de verdade, ele só é escrito quando o GlintFx existir, e liga direto nele. Se o código nunca precisaria do GlintFx, ele pode ser escrito hoje.

## L-06

**Data:** 21/08/2026, decisão do líder por `AskUserQuestion`.

**Ordem de construção:** o **núcleo de regra de jogo** é escrito agora, em TDD estrito. A **camada que desenha** só nasce quando o GlintFx tiver janela, contexto gráfico, entrada e texto, e liga direto nele, sem intermediário.

**Consequência assumida, e que não deve ser maquiada em relatório:** enquanto isso, o jogo **não roda, não tem tela, não tem demo e não tem screenshot**. O que existe é suíte de teste verde e núcleo provado. Relatar progresso visual inexistente é falta grave.

## L-07

**Data:** 21/08/2026. **Verbatim:** *"vc se comunica com Gus Dragon / dragondrv [...], com GlintFx, com o site de registro histórico do jogo e com o editor de mapas por um bus em [...] e em um bus git https://github.com/petrinhu/gusworld_ia_autocomm onde tem as instrucoes"*.

O bus é o canal assíncrono entre as sessões e o filho do líder. Clone canônico: `<vault>/gusworld_ia_autocomm/` (o `<vault>` é a raiz do vault de projetos do líder nesta máquina). Repositório **privado**. Protocolo completo em `PROTOCOL.md` do clone; **leia o protocolo, não confie neste resumo**. Nosso slug é `gusworld`.

**Quando pedir função ao GlintFx (decisão do líder, 21/08/2026, por `AskUserQuestion`):** **só quando o jogo esbarrar de verdade na falta**. Cada pedido nasce de um ponto concreto que parou, e descreve o uso real. Não se encomenda API por antecipação.

**Proibido classificar prioridade do outro.** Pedido vai sem "urgente", "para agora", "quando der" ou "bloqueia X". Quem recebe é quem enxerga o próprio roadmap. Exceções: o campo `prioridade:` que o próprio Gus põe na ideia dele, e aviso operacional de quebra.

**Ideia do Gus, o pipe completo:** absorver; **ack imediato e automático** na issue marcando `@Dragon-Drv`, sem esperar o líder; discutir viabilidade **com o líder**; postar o resultado na issue, automático, **sempre honesto** e adequado a uma criança de 11 anos, sem inventar nada além do decidido; **entrar na tabela de pendências**; arquivar. **Nunca minta para ele.**

**Entrada na tabela é imediata** (ordem do líder, 21/08/2026, verbatim: *"as ideias do gus entram na tabela de imediato após a discussao da implementacao"*). Assim que a discussão de implementação com o líder termina, a ideia vira item do `TODO.md` **no mesmo movimento**, com a autoria dele citada. Não espera onda, não espera sobrar tempo, não fica só na issue. A execução, essa sim, entra na próxima onda, sem atropelar o que está rodando.

**Vigilância dos canais dele:** o Gus manda por **arquivo `.txt`** no `inbox/` e por **GitHub Issue**, e o ritual de sessão cobre os dois, mais as **discussions**. Olhar só a pasta já deixou uma issue dele quase uma hora sem resposta em 21/08/2026.

## L-08

**Data:** 21/08/2026, decisão do líder por `AskUserQuestion`.

**Código: AGPL-3.0-or-later**, com uma **ressalva explicativa** exigida pelo líder, verbatim: *"opcao 1 com ressalva explicando que o jogo é apenas offline para o usuario nao se assustar ou procurar funcao que nao existe"*. A ressalva vive no `README` e em nota própria, **nunca dentro do texto da licença**, que é imutável, e diz em linguagem simples que o jogo é single-player offline, sem servidor, sem conta e sem telemetria, e que a cláusula de rede da AGPL não impõe nada a quem só joga.

**Motivo técnico da escolha, registrado para ninguém reabrir por engano:** o GlintFx é AGPL-3.0-or-later. O executável do GusWorld linkado a ele é obra combinada, e distribuir esse binário obriga a obra inteira a sair sob AGPL-3.0. Rótulo diferente no jogo não sobrevive à combinação.

**Titular do copyright: Petrus Alves da Silva Costa**, escolhido pelo líder em 21/08/2026 depois de avisado de que o nome fica em todo arquivo de um repositório público, para sempre. É o nome que vai nos cabeçalhos, no `NOTICE`, no `ASSETS-LICENSE.md` e nos identificadores de licença própria.

**Assets e lore: regime único, TODOS OS DIREITOS RESERVADOS** ao titular (decisão do líder, 21/08/2026). Cobre arte, música, som, sprite, texto de sabor e toda prosa in-game. A divisão por data de corte em três zonas foi **revogada**: ela existia para proteger licenças concedidas a terceiros no projeto anterior, e o líder confirmou que o repositório antigo não existe mais e que ninguém baixou o jogo, portanto não há terceiro com direito adquirido. Fica um regime só para todo asset e todo texto in-game, mais o **carve-out de marca** (nome, logotipo, trade dress e nomes de personagem ficam fora de qualquer concessão) e a **permissão de conteúdo de fã** (vídeo, transmissão, captura, fan art e fan fiction não comerciais, com monetização padrão de plataforma permitida ao criador de conteúdo).

**Os dois livros-companheiros são obra à parte, com direitos reservados**, e a licença do jogo não estende nada ao texto deles.

**A fronteira dentro de `docs/`, decidida em 22/08/2026 por `AskUserQuestion`, depois que o Cláudio (CLO) mostrou a contradição:**

| Caminho | Regime | Por quê |
|---|---|---|
| `docs/narrative/**` | **reservado** | é a obra: bíblia de lore, timeline, facções, fichas, biomas, língua construída. Dentro dele, `deep/antologia/` são os 14 contos que alimentam o Volume 2 |
| `docs/specs/**` | **reservado** | especificação visual de personagem, mesma natureza |
| `CHARS.md`, `PLACES.md`, `sinopse.md` (raiz) | **reservado** | inventário de personagens, de lugares e a sinopse; estão na raiz só por conveniência |
| `docs/design/**` e o restante de `docs/` | licença do código | documentação de projeto: regra de jogo, decisão técnica, spec de engenharia |

**A ambiguidade que causou isso, registrada para não voltar:** o título desta lei diz "Assets e **lore**", mas a lista operativa dizia "texto de sabor e toda prosa **in-game**", e o corpus de design não é inequivocamente in-game. Um agente aplicou a lista, não o título, e declarou 134 arquivos de lore como código livre. **A regra agora está na tabela acima, e a tabela vence qualquer leitura.**

**Marcação de licença por arquivo: REUSE com identificadores SPDX**, desde o primeiro commit. Cada fonte nasce com `SPDX-FileCopyrightText` e `SPDX-License-Identifier` no topo; asset e binário ganham arquivo companheiro; um arquivo de regras cobre diretórios inteiros; os textos integrais ficam em `LICENSES/`. Sem isso, ferramenta, distribuição e agregador presumem que tudo no repositório é AGPL.

## L-09

**Data:** 21/08/2026. **Verbatim:** *"CI com runners Fedora 44 (principal), Ubuntu, Arch, CachyOs (original, não arch renomeado), windows"*.

Cinco alvos, **cinco entradas distintas na matriz de CI**. **Fedora 44 é o alvo primário**, por ser o sistema do líder: imagem **pinada em `fedora:44`**, nunca em `:latest`. **CachyOS não é Arch renomeado** e não é coberto pelo job de Arch.

**Aplicação:** verde no Arch **não** autoriza declarar CachyOS suportado. Declaração de suporte exige job próprio verde.

## L-10

**Data:** 21/08/2026. **Verbatim:** *"main apenas orquestra, interage comigo e dispara agentes. Auditorias apenas com clevel bigtech fable, trabalhadores sonnet bigtech"*.

**A thread principal (`main`) não executa trabalho de produto.** Ela orquestra, delega e avalia o retorno dos agentes.

| Tipo de trabalho | Agente | Modelo |
|---|---|---|
| Auditoria | C-level da constelação bigtech | **`fable`, sempre** |
| Criação de projeto e arquitetura | C-level da constelação bigtech | **`fable`, sempre** |
| Implementação | agente operacional bigtech | **`sonnet`** |

**Somente agentes da constelação bigtech.** Nada de agente genérico, anônimo ou improvisado.

**Porte do projeto: COMPLETO** (decisão do líder, 21/08/2026). O pipeline inteiro de doze fases e a constelação inteira de C-levels estão **ativos**, incluindo produto, tecnologia, mercado, receita, jurídico, segurança, dados, finanças, operação e comunidade. Nenhum C-level fica dormente por presunção de agente: se uma fase parece não se aplicar, isso vai ao líder, não vira corte silencioso.

**O que continua sendo do `main`:** decidir o que delegar e em que ordem; escrever a ordem de serviço; **re-verificar o entregável**, porque relatório de agente não é prova; levar decisão ao líder; e falar com o líder.

**Papéis distintos:** o agente que implementa não é o que revisa, e nenhum dos dois é o `main` que re-verifica.

## L-11

**Data:** 21/08/2026. **Verbatim:** *"obrigatoriamente use AskUserQuestion ao me trazer perguntas ou precisar algo de mim"* e, no mesmo dia: *"nao decida NADA sozinho, quero decidir tudo, vc me dá opcoes"*.

**Nenhum agente decide nada.** Diante de qualquer dúvida, bifurcação ou consequência não escrita, apresentar **2 a 3 alternativas** com prós, contras, impacto e esforço, e perguntar via `AskUserQuestion`, com a recomendada primeiro. Isto vale inclusive para o que parece consequência óbvia de uma decisão já tomada.

**Aplicação:** `AskUserQuestion` **sem painel lateral**, ou seja **sem o campo `preview`**; só `label` e `description`. Detalhe técnico longo vai no corpo da mensagem de chat, antes ou depois da pergunta.

**Dever de contra-argumentar:** se uma decisão do líder for destrutiva, violar princípio do projeto ou inviabilizar marco, o agente nomeia o problema, explica o risco concreto, propõe alternativa e devolve a decisão a ele. Silêncio passivo é má prática. Reafirmada a ordem, executa por inteiro.

## L-12

**Data:** regra permanente do líder. **Verbatim:** *"Formato [DD/MM/YY - HH:MM:SS]"*.

**Toda** mensagem ao líder começa com `[DD/MM/YY - HH:MM:SS]`. A hora tem de ser **real**, obtida de `date '+%d/%m/%y - %H:%M:%S'` a cada mensagem. Nunca estimar, nunca reaproveitar o timestamp anterior.

## L-13

**Data:** 21/08/2026. **Verbatim:** *"deixe marcado no gods_laws que so pode ser feito trabalho ligado a canon depois que o canon for atualizado"*.

**Canon desatualizado bloqueia o trabalho que depende dele.** Ao topar com documento canônico que contradiz uma decisão vigente, o trabalho **para**: primeiro o canon é atualizado com o líder, depois o trabalho continua. Não se implementa sobre canon que se sabe errado, e não se "adapta mentalmente" a contradição.

**Contradições já identificadas em 21/08/2026, que bloqueiam o que depender delas:**

- ~~`docs/design/pillars.md:27` diz *"Cel-shaded 3D low-poly"*, contra a L-02.~~ **RESOLVIDO, conferido em 23/08/2026:** a linha 27 hoje diz *"Não é fotorrealista. Pixel-art 2D, paleta restrita"*, e a expressão "Cel-shaded" não existe mais em lugar nenhum do arquivo. A citação fica como registro de por que a L-02 precisou existir.
- `docs/design/mecanicas/core-loop-exploracao.md:95`, decisão DA-1, canônica, diz *"Câmera e navegação no overworld: 3/4 orbital + controle direto [...] Coerente com '3D real'"*. **Resolvido pela L-26, e conferido em 23/08/2026:** a perspectiva é 3/4 top-down fixa, e "orbital" e "3D real" **já foram apagados** — a linha 95 hoje diz *"3/4 top-down fixa + controle direto"*, e "orbital" tem zero ocorrências no arquivo.
- `docs/narrative/diary/knowledge-gates.md:376` (a citação dizia 380; a linha escorregou, corrigida em 23/08/2026), o único esquema de save do corpus, se declara escrito para o stack Godot e GDScript. **Parcialmente resolvido:** a nota de stack já está no arquivo e marca o que sobrevive e o que muda; **segue bloqueando** a re-derivação técnica do esquema.
- `docs/design/producao/*` se declaram canônicos mas descrevem pipeline Godot e C# aposentado.
- ~~`docs/design/mecanicas/modos-morte.md:3` se declarava *"PROPOSTA — spec fechada pelo `lead-game-designer`, aguardando canonização do criador supremo (líder)"*, enquanto `docs/design/pillars.md:136` já apontava para ele como *"detalhe canônico"*. O documento era a fonte da contradição, não a vítima dela.~~ **Contradição identificada em 25/08/2026, fora da varredura de 21/08, e RESOLVIDA no mesmo dia por ordem do líder:** o desenho já era canon desde 24/08/2026 pelo item `G3` do `TODO.md`, que preservou §1 a §5 e revogou o §6. O bloco de status foi reescrito, o §4 deixou de se declarar não aplicado (a atualização do Pilar 4 é de **10/07/2026**, commit `1ebd2734` do projeto anterior, medido e não suposto), e o corpo trocou **cinco** ponteiros de `ADR-014` — que é o runtime de diálogo — pelo `ADR-015`, que é o de segurança de save. **Segue aberto e é do líder:** as sete sinalizações do §5, que a canonização do desenho não decidiu.

**Aplicação:** a lista acima não é exaustiva, e não vira licença para ignorar o que não está nela. Contradição nova encontrada entra aqui.

## L-14

**Data:** 21/08/2026. **Verbatim:** *"o que estiver morto, nao marque como morto imediatamente, traga para eu dizer o que fazer"*.

**Agente não declara nada morto, obsoleto, descartável ou irrelevante.** Ao concluir que algo perdeu validade (documento, decisão, referência órfã, asset, ideia), o agente **traz o caso ao líder** com a evidência, e ele decide entre corrigir, arquivar, apagar ou manter.

**Aplicação:** vale para citação órfã no corpus, ADR inexistente, spec de stack aposentado, asset sem uso e qualquer coisa que a tentação seja "limpar". Limpeza silenciosa é perda de decisão.

## L-15

**Data:** 21/08/2026, decisão do líder por `AskUserQuestion`.

**Fora do git**, por serem regeneráveis ou pessoais: `resources/livros/` (943 MB, bancos vetoriais de RAG) e `resources/glb/` (402 MB, o pipeline 3D pessoal do líder, ver L-02). **Git LFS** para o restante do binário pesado: sprites, VFX, imagens e áudio.

**Aplicação:** `.gitignore` e `.gitattributes` são escritos **antes do primeiro commit**. Depois do primeiro push, corrigir isto custa reescrita de histórico.

## L-16

**Data:** 21/08/2026. **Verbatim:** *"esse jogo tem algumas homenagens a pessoas que já aceitaram a homenagem"* e *"Gus dragon é homenageado no personagem Gus Vector Tavus Vance"*.

Homenagem a pessoa real só existe **com aceite prévio** dela, e o aceite é fato que o líder confirma, não que o agente presume. **Nunca versionar nome de batismo de menor.** O filho do líder aparece só como **"Gus Dragon"** ou **"dragondrv"**, e esses apelidos podem ser citados em público.

## L-17

**Data:** 21/08/2026, decisão do líder por `AskUserQuestion`, sobre as opções levantadas pelo Caetano (CTO).

A arquitetura do GusWorld é **espinha de camadas com núcleo determinístico por comando e evento**. Dois compromissos, e nenhum deles é negociável por agente.

**1. Cinco camadas, dependência só para baixo, com gate de CI.**

```
present/   liga DIRETO no GlintFx: janela, loop de frame, sprite, RML, input, áudio.
           NASCE SÓ QUANDO O GLINTFX EXISTIR (L-06). Única camada com #include <glintfx/...>.
app/       fluxo do jogo: cenas, avanço de turno, coordenação entre domínios, casos de uso.
domain/    a regra: combate, deck e coleção, economia, progressão, diálogo, mapa como dado, save.
content/   catálogo de conteúdo (cartas, inimigos, receitas, statlines) e seu validador.
core/      tipos básicos, sorteio determinístico contado, identificadores, resultado e erro.
```

Cada camada depende só das de baixo, nunca o contrário. **Um gate de CI reprova a violação** em vez de confiar na disciplina de quem escreve: qualquer `#include <glintfx/` fora de `present/` derruba o build.

**2. A regra é transição determinística de estado.** Cada sistema de regra expõe a forma `aplica(estado, comando)` devolvendo `(estado novo, eventos)`. Estado, comando e evento são POCOs. O sorteio entra como **parte do estado** (semente mais contador), nunca como global.

**Por que esta forma, registrado para ninguém reabrir por engano:** o canon do jogo já era isso sem dizer o nome. A máquina de estados do combate, os barramentos `CombatBus` e `PlayerBus`, o "conta cada saque de RNG", a mão como lista de identificadores e o descarte que só anda num sentido são, todos, contratos de transição determinística.

**O que a lei entrega, e que passa a ser exigível:** replay byte a byte (semente mais lista de comandos reproduz o mesmo estado final) como teste automatizado permanente; auto-resolve para balanceamento em massa; um estado canônico só, que torna o save completo por construção.

**O que a lei não é:** dogma de redutor puro. A forma comando e evento é obrigatória **na fronteira** de cada sistema de regra; por dentro, o sistema se organiza como servir melhor, inclusive como agregado com métodos que preservam invariante, desde que atômico (L-04).

**Onde diverge do GlintFx, conscientemente:** a superfície pública **não** é opaca aqui. A opacidade da L-19 do GlintFx existe por ABI de biblioteca consumida por terceiros; o GusWorld é executável final, e os POCOs de domínio são visíveis por desenho, porque são o contrato interno e o formato de save.

**Armadilhas que são achado de revisão, não questão de gosto:**

1. **Objeto `Game` ou camada `app/` que vira dona de tudo.** `app/` é despachante fino. Cada cena recebe só os sistemas que usa. Proibido um contexto universal que carrega o mundo inteiro.
2. **Struct de serviços gordo ou barramento universal.** Um serviço por assunto; barramentos separados e tipados, como o canon já faz, nunca um bus genérico de mensagem.
3. **`if` de dificuldade picotando o corpo da regra.** É o `#ifdef` da aplicação: some da leitura igual. Política por dado, em tabela de parâmetros escolhida uma vez na carga do save, e número nomeado em configuração, nunca literal no corpo da regra.
4. **Determinismo quebrado por detalhe de C++.** Ordem de iteração de contêiner associativo não ordenado e ponto flutuante em fórmula de regra quebram o replay em silêncio. Contêiner de iteração estável no estado, e o próprio teste de replay como detector permanente.
5. **Regra vazando para a interface.** O que pode ser mostrado é invariante do domínio, apenas obedecido pela interface. O contrato "nunca vaza carta infectada antes de diagnosticada" é do domínio, não da tela.


## L-18

**Data:** 21/08/2026. **Verbatim:** *"nada vai ficar em formato de texto. isso facilita fraudes e edicoes. quero maxima protecao a maps, saves, configs, itens, busque na web"*.

**Nenhum dado do jogo é armazenado em formato de texto.** A proibição alcança **mapa, save, configuração do jogador e item**, e "item" inclui o catálogo de conteúdo (carta, item, inimigo, receita), não apenas o estado do jogador.

**A régua é máxima proteção contra edição**, não o mínimo suficiente. O desenho mira o teto do que é tecnicamente alcançável.

**Motivo declarado pelo líder:** formato de texto facilita fraude e edição.

**Aplicação:** ao escolher formato de qualquer arquivo lido ou gravado pelo jogo, formato de texto não é opção. Ao topar com um custo real desta lei (revisão de balanceamento em pull request, editor de mapas externo, relato de defeito por jogador, necessidade de ferramenta própria de inspeção), **nomeie o custo ao líder e proponha mitigação**; não contorne a lei e não silencie o preço.

**Tensão registrada, que o desenho tem de encarar de frente:** o jogo é FOSS sob AGPL-3.0, então o adversário tem o código-fonte e o algoritmo. Prevenção absoluta de adulteração numa máquina que o jogador controla é impossível, e prometer isso é mentira. Detecção confiável de adulteração é alcançável. O desenho diz com todas as letras o que garante e o que não garante.

**Estado:** as opções de mecanismo estão sendo levantadas pelo Narciso (CISO), com pesquisa na web exigida pelo líder. A decisão é dele.


## L-19

**Data:** 21/08/2026, decisão do líder por `AskUserQuestion`, sobre o item 23 do `inicial.md`, verbatim: *"veja como glintfx faz os testes dele e auditorias no codigo etc, aqui deve ser identico mas adaptado ao nosso projeto, só que mantendo o mesmo rigor"*.

**TDD estrito, a partir do primeiro módulo com comportamento.** Nenhuma linha de código de comportamento entra sem um teste que **falhava antes** de ela existir. O ciclo, sem pular etapa: **vermelho** (escreva o teste, **execute** e veja falhar), **verde** (o mínimo que faz passar), **refatorar** (com a suíte verde como rede, aplicando a L-04).

**Ao delegar (L-10):** a ordem de serviço exige **a saída real do teste falhando** antes da implementação, e a saída real dele passando depois. Relatório que só mostra o verde final não cumpriu a lei: falta a metade que prova que o teste morde.

**Cinco portões de qualidade, todos adotados:**

1. **Zero aviso de compilação em todo commit**, com `-Werror` no CI.
2. **ASan e UBSan a cada fatia fechada**, em build separado.
3. **Análise estática no CI:** `clang-tidy` e `cppcheck`.
4. **Scan de segredo no CI:** `gitleaks`. Aviso honesto: ele **não** pega nome de projeto e, por padrão, olha a árvore e não o histórico. Para dado sensível, a verificação é `git log --all -p | grep -ci <termo>`, com `-i`, nunca `git grep`.
5. **Gate de mensagem de commit**, `tools/security/commit_gate.py`, instalado como gancho `commit-msg` local por `tools/git-hooks/install.py`. Motivo: **o git-crypt cifra o conteúdo do arquivo, e não cifra a mensagem de commit, o nome do arquivo nem o do diretório.** Sem este portão, o texto que a L-25 protege dentro do repositório vaza pela porta que ninguém trancou. Ele barra quatro classes: termo da lista cifrada, caminho de área cifrada, dado pessoal e credencial. **Sem a chave, ele reprova o commit** em vez de aprovar em silêncio, cumprindo a lição 3 acima. A lista de termos vive cifrada, porque uma lista de segredos em texto puro é ela mesma o vazamento.

**Script local que espelha o CI, rodado antes do push**, não depois. As ferramentas que ele exige são instalação de sistema, e passam pelo líder.

**Sem meta numérica de cobertura.** Com TDD estrito, todo código de comportamento nasce de um teste que falhou, então cobertura é consequência, não alvo. Métrica de cobertura premia linha executada, não comportamento verificado.

**Cinco lições de teste que este projeto já pagou caro uma vez** (recuperadas do backup do projeto anterior em 21/08/2026, por ordem do líder):

1. **Ao escrever a asserção, pergunte o que o BUG produziria, não só o que o acerto produz.** Uma invariante da fila de turnos foi violada por **cinco caminhos distintos** e o fuzz nunca pegou nenhum, porque a asserção afirmava o efeito desejado, que é exatamente o que o bug também produz em alguns casos. É a lição de maior alavancagem de todas.
2. **Conserto sem gate que o imponha erode sozinho.** Declararam build limpo; dezesseis dias depois a auditoria achou seis avisos de volta, porque não havia trava travando a classe do erro.
3. **Gate que não consegue medir tem de FALHAR, nunca aprovar em silêncio.** Sete cenários de "não consigo medir" foram testados de propósito, e todos precisam reprovar com mensagem explícita.
4. **Métrica sem régua de comparação não conclui nada.** "142 quadros por segundo" parecia baixo até alguém descobrir que o monitor era de 144, ou seja, 98,8% da taxa e zero quadro perdido. Razão, nunca número absoluto.
5. **Autoteste que cabe no próprio limite de tempo que deveria testar torna o descompasso invisível.** Uma sonda com limite de 60 s "aprovou" um roteiro de 35 s mais tempo humano: a máquina testou a máquina e as duas concordaram; faltava o humano, que é lento.

**Mais duas, sobre diagnosticar falha:** distinguir **defeito do teste** de **defeito do produto** é bifurcação, e escolher errado **esconde** o bug ("é o ambiente" é conforto prematuro); e dois testes com o **mesmo prefixo** em arquivos diferentes produzem atribuição de causa errada sob execução paralela.

**Cuidado registrado, que já custou tempo nesta casa:** ao provar que um teste morde por mutação, **prove que a mutação chegou ao código executado**. Binário desatualizado por falta de recompilação, arquivo não commitado e registro de callback capturado na carga produzem suíte verde com o mutante aplicado, e a conclusão errada é "meu teste é fraco" quando a mutação nunca rodou.

**Dossiê formal de auditoria** (`AUDITORIAS.md`), pelo `internal-auditor`, é portão de release antes da 1.0. Até lá os portões automáticos cobrem o dia a dia.

## L-20

**Data:** 21/08/2026, decisão do líder por `AskUserQuestion`, complementando a L-09.

**A matriz das cinco plataformas existe desde o primeiro commit.** Fedora 44 pinado, Ubuntu, Arch, CachyOS e Windows, cada um com entrada própria. Divergência de plataforma aparece no dia em que nasce, e não em bloco depois de muito código escrito.

**Aplicação:** a fundação de build e o CI são a primeira fatia do projeto, antes do primeiro módulo de comportamento. Fundação de build não tem comportamento a especificar, então nasce legitimamente sem TDD (L-19); **a partir do primeiro módulo de verdade, não há mais essa saída**.


## L-21

**Data:** 21/08/2026. **Verbatim:** *"quando inicial.md já tiver sido todo resolvido, tudo decidido, executado, memorizado no local certo, apague"*.

O `inicial.md` é o documento fundador deste projeto: os 28 itens que o líder ditou em 21/08/2026. Ele **será apagado**, por ordem dele, quando tiver cumprido o papel. Não antes.

**As quatro condições, todas obrigatórias, item a item:**

1. **Resolvido:** o item foi discutido com o líder até o fim, sem ponta solta.
2. **Decidido:** a decisão dele está registrada como lei neste arquivo, com data e verbatim.
3. **Executado:** o que a decisão exige existe de fato no repositório, verificado, não relatado.
4. **Memorizado no local certo:** o que é preferência permanente do líder está na memória do projeto ou na global, conforme o alcance; o que é regra deste projeto está aqui; o que é procedimento está no documento de procedimento.

**Quinta condição, que a prudência acrescenta e que não contraria a ordem:** antes de apagar, o agente apresenta ao líder o **checklist dos 28 itens**, um a um, mostrando onde cada um foi parar, e ele confirma. Apagar é irreversível, e a L-11 manda não decidir nada sozinho.

**Aplicação:** o rastreio item a item vive na tabela de pendências (`TODO.md`), que é onde o estado de trabalho mora, e não é duplicado aqui, para não criar duas verdades. Enquanto qualquer item estiver aberto, o `inicial.md` permanece intacto e legível.


## L-22

**Data:** 21/08/2026, decisão do líder por `AskUserQuestion`.

**Identificador e comentário de código em inglês, no estilo da biblioteca padrão de C++: `snake_case`, sem prefixo `m_`.** Isto **substitui** o `CONTRACT.md` §5.1 (era §6.1 antes da renumeração de 21/08/2026), que manda função em pt-br, membro com `m_` e constante em ALL_CAPS. Motivo: uma só gramática atravessando o jogo e o framework em que ele assenta, e o repositório é público sob AGPL.

**Mensagem de commit continua em pt-br**, assim como a documentação do projeto e a conversa com o líder.

**Termo de lore sem tradução honesta fica no original.** Nome próprio do mundo do jogo, unidade inventada e palavra da língua construída não se traduzem para inglês: `runa`, `tavus_drive`, `selve`. Nome revela intenção, sem abreviação que não seja universal (`id`, `url`, `http`).

## L-23

**Data:** 21/08/2026, decisão do líder por `AskUserQuestion`.

**A versão do GusWorld tem quatro componentes, e a tag é `vA.B.C.D`**, a mesma convenção do GlintFx.

| Componente | Sobe quando |
|---|---|
| **A** | quebra de compatibilidade sentida pelo jogador, save antigo incluído |
| **B** | recurso novo, compatível para trás |
| **C** | correção, sem recurso novo e sem quebra |
| **D** | build ou revisão de empacotamento, **sem mudança de código** |

**Tag e release continuam exigindo aval explícito do líder no contexto:** esta lei fixa o formato, não autoriza taggear.


## L-24

**Data:** 21/08/2026. **Verbatim:** *"o que for revogado APAGUE, nao deixe motivo para você ler nada errado depois e sair misturando tudo!"*.

**O que o líder revoga é APAGADO, não arquivado.** Nada de bloco "Histórico", "Superado", "era antiga", "mantido para registro" ou seção recolhida com o texto morto dentro. Texto revogado que continua legível é texto que um agente vai ler e obedecer por engano.

**Como isto se articula com a L-14, para não haver conflito entre leis:**

- A **L-14** governa **quem decide**: nenhum agente declara nada morto por conta própria; o caso vai ao líder com a evidência.
- A **L-24** governa **o que se faz depois da decisão**: revogado por ele, apaga-se de vez, sem deixar rastro legível.

**Uma exceção, e só uma:** texto cuja remoção teria efeito jurídico ou de direito adquirido, como licença já concedida a terceiros, que é irrevogável por natureza. Nesse caso o agente **não apaga e leva o caso ao líder**, explicando por quê.

**Citação órfã: apaga-se a citação e preserva-se a afirmação** (decisão do líder, 21/08/2026). Onde o corpus diz que algo foi decidido em um documento que não existe (os quinze números de ADR, o `ROADMAP.md`, o `GusEngine/`), some o ponteiro e fica o que o texto afirma. O leitor para de ser mandado a lugar nenhum, e nenhuma informação se perde.

**Aplicação:** vale para manual, documento de design, arquivo de canon, comentário de código e para as leis deste arquivo. Lei revogada some daqui, inclusive do índice de gatilhos.

## L-25

**Data:** 21/08/2026, decisões do líder por `AskUserQuestion`, sobre as opções levantadas pelo Narciso (CISO), com pesquisa na web.

**A proteção de save, configuração, mapa e catálogo é o teto técnico, entregue por fases.**

**A verdade técnica que o desenho assume, e que nenhuma comunicação do projeto pode contradizer:** o jogo é FOSS sob AGPL e roda na máquina do adversário, que tem o código-fonte. **Prevenir adulteração é impossível; detectar adulteração de forma confiável é alcançável.** Proibido prometer "impossível editar", "impossível recuperar" ou "à prova de trapaça", em copy, changelog ou material público. Promete-se: o jogo detecta arquivo alterado ou corrompido, e no Hardcore a morte apaga o save sem recuperação pelo jogo. **"Wipe seguro" físico não existe em disco moderno**; o entregável é o wipe lógico, que torna o save incarregável.

**A espinha, comum a todas as fases:** envelope binário único do projeto (magia, versão de formato, tipo, nonce, dado, selo de autenticação) servindo save, configuração, mapa e pacote de conteúdo; serialização binária versionada dos POCOs de domínio, escrita em casa; gravação atômica com cadeia de backups; e verificação em dois níveis no carregamento: o selo criptográfico e o **validador semântico**, que confere se o estado é alcançável pelas regras e é o único nível que sobrevive à chave vazada.

**As fases, nesta ordem:**

1. Envelope binário com cifra autenticada, validador semântico, gravação atômica e backups.
2. Catálogo compilado dentro do executável, e save híbrido de estado mais registro de comandos com encadeamento, verificável por **re-execução**: para forjar um save que passe, é preciso produzir uma história de comandos que legitimamente chegue àquele estado. Esta é a única peça que não evapora quando o adversário lê o fonte, e ela nasce da L-17.
3. Âncora anti-rollback por TPM, **opcional**, com retorno à âncora em arquivo onde não houver. Nunca requisito para jogar.

**Fonte do conteúdo:** texto estruturado **dentro do repositório**, lido apenas pelo gerador em tempo de build. **Nada em texto na distribuição:** o jogador recebe o executável com as tabelas compiladas e os pacotes binários selados. Isto cumpre a L-18 no que chega à máquina do jogador e preserva revisão de balanceamento, histórico número a número e erro de validação apontando a linha.

**Amarra de máquina só no slot Hardcore**, com aviso ao jogador. Save normal viaja com o dono: backup, computador novo e reinstalação de sistema são direito dele.

**Primitivas criptográficas: vêm do GlintFx.** O líder já decidiu que criptografia está no escopo do 1.0 do framework. **Nada de biblioteca de terceiro no GusWorld, nada de primitiva escrita em casa, nada de API do sistema operacional**, que fora do Windows degeneraria em biblioteca de terceiro com duas implementações divergentes. Tudo que não é criptografia (envelope, serialização, validador, gravação atômica, gerador de tabelas, núcleo inteiro) é escrito sem esperar. **O pedido ao bus é registrado quando a fatia do envelope começar** e o jogo esbarrar de fato na falta (L-07), com este conteúdo: hash criptográfico, MAC ou hash com chave, cifra autenticada com nonce e dados associados, derivação de chave, bytes aleatórios criptográficos e comparação em tempo constante.

**O que é cifrado dentro do repositório público** (ordem do líder, 21/08/2026, verbatim: *"pode usar a mesma chave para encriptar no repo o que é secreto"*, com o escopo escolhido por `AskUserQuestion`):

| Caminho | Por quê |
|---|---|
| `docs/_secret/**` | easter eggs e cosmologia de origem; segredo por natureza |
| `docs/narrative/deep/stinger/**` | ganchos pós-créditos e de continuação; spoiler máximo |
| `docs/book/**` | os dois livros-companheiros, obra à parte com direitos reservados (L-08) |
| `tools/security/termos_proibidos.txt` | a lista que o gate da L-19 consulta; em texto puro ela seria o próprio vazamento que existe para impedir |

Ferramenta: `git-crypt`, **uma chave só**, a simétrica exportada para fora da árvore do projeto. **Nada além desses quatro caminhos entra na cifra** sem ordem nova do líder.

**Três verdades que este mecanismo obriga a assumir:**

1. **Cifrar é irreversível para quem já clonou.** Quem tem o arquivo cifrado o tem para sempre; se a chave vazar ou o algoritmo envelhecer, tudo que já esteve lá se abre de uma vez.
2. **Uma chave é tudo ou nada.** Quem a recebe lê os quatro caminhos. Não existe acesso parcial enquanto houver uma chave só, e a decisão de criar uma segunda é do líder.
3. **Perder a chave é perder o conteúdo.** Não há recuperação.

**Aplicação:** o caminho é marcado no `.gitattributes` **antes** do primeiro `git add` do conteúdo, nunca depois. Marcar depois deixa o texto puro no objeto já gravado, e o objeto sobrevive à "correção". Prove com `git-crypt status -e` antes de commitar, e nunca confie no relatório de quem marcou.

**Armadilha medida em 21/08/2026, que produz falso alarme:** `git add --intent-to-add` **não roda o filtro do git-crypt**. Ele grava um blob vazio, e o `git-crypt status -e` então acusa `*** WARNING: staged/committed version is NOT ENCRYPTED! ***` para arquivos que na verdade seriam cifrados corretamente. A prova só vale com `git add` completo. **A prova definitiva não é o `status`, é o objeto:** `git cat-file -p :<caminho> | head -c 24 | xxd` tem de começar com `\0GITCRYPT\0`. Ler o objeto é a única verificação que não depende de ferramenta interpretando a si mesma.

**Duas travas operacionais que valem como lei:**

- **Configuração selada nunca pode impedir o jogo de abrir.** Configuração com selo inválido cai em padrão de fábrica com aviso, jamais em recusa de boot. A proteção existe contra adulteração silenciosa, não contra o dono se socorrer.
- **A ferramenta de inspeção e re-selo, para desenvolvimento, nasce junto com o formato**, no mesmo trabalho, não depois. Sem ela ninguém enxerga dentro de save, mapa ou pacote quando algo dá errado.

**O catálogo é fatiado por natureza jurídica** (decisão do líder, 21/08/2026, sobre o parecer do Cláudio/CLO):

- **Número e regra** (identificador, custo, poder, velocidade, efeito) são **código sob AGPL**, compilados dentro do executável. A Lei 9.610/98, artigo 8º, inciso II, exclui da proteção autoral as regras de jogo, então não há direito autoral ali para reservar.
- **Texto de sabor, descrição e qualquer prosa de carta** são **asset**, ficam **fora do executável**, no pacote binário selado, sob o regime de asset da L-08. São obra literária com proteção plena.

**Motivo, para ninguém reabrir por engano:** compilar conteúdo reservado dentro de um executável AGPL produz um binário que se contradiz, anunciado como redistribuível e contendo parte que ninguém pode redistribuir. Arquivo de dado ao lado do executável é agregação consagrada; dentro do mesmo executável, a leitura da FSF é que os módulos formam um programa só.

**Consequência de engenharia:** o gerador de build produz **dois** artefatos, e uma carta existe em dois lugares. O selo criptográfico protege os dois igualmente, então a proteção contra edição não muda.


## L-26

**Data:** 21/08/2026, decisão do líder por `AskUserQuestion`, encerrando o bloqueio que a L-13 impunha.

**A perspectiva do jogo é 3/4 top-down fixa, com quatro direções cardeais e mapa em grade quadrada.** É o esquema de Zelda de SNES, Chrono Trigger e Stardew Valley, as referências que o líder nomeou no `inicial.md`.

**A câmera não gira.** Ela acompanha o personagem, e ponto. Câmera orbital está morta.

**Quatro direções: `north`, `south`, `east`, `west`.** Nada de diagonais.

**As quatro direções são desenhadas à mão, uma a uma. Proibido espelhamento horizontal.** Leste **não** é oeste espelhado. Decisão de arte do líder, recuperada do roadmap do projeto anterior em 21/08/2026: *"3 poses de walk em ping-pong, 4 direções únicas (sem flip)"*. É por isso que existem quatro pastas de sprite por estado, e não duas mais espelhamento. Espelhar para economizar arte é violação, não otimização.

**O fato que sustenta a decisão, e que também a limita:** a arte já produzida assume exatamente isto. Os sprites dos sete personagens prontos estão em `resources/sprites/<personagem>/anims/<estado>/{north,south,east,west}/`, com sete quadros por direção no ciclo de caminhada. Câmera orbital exigiria oito direções ou rotação real; isométrico verdadeiro usaria diagonais, não cardeais.

**Aplicação:** qualquer proposta futura de diagonal, rotação de câmera ou projeção isométrica **custa refazer a locomoção de todos os personagens**, e por isso é decisão do líder, nunca de agente.

## L-27

**Data:** 21/08/2026, decisão do líder por `AskUserQuestion`, sobre o item 21 do `inicial.md`.

**Nenhuma interface é escrita antes de o GlintFx traduzir marcação.** Sem arquivo de marcação, sem folha de estilo, sem tela piloto, sem prova de conceito.

**Motivo:** não existe hoje nenhum arquivo de interface no repositório, e o GlintFx não tem tela, texto na tela nem tradutor de marcação. Escrever agora seria escrever contra uma gramática que ainda não foi definida, e qualquer camada nossa por cima seria dublê, proibido pela L-05.

**Aplicação:** a interface entra junto com a camada `present/` (L-17), quando o framework existir. Até lá, o que se sabe sobre telas vive nos documentos de design que já existem, e não vira código nem marcação.


## L-28

**Data:** 21/08/2026. **Verbatim:** *"sempre que precisar baixar algo me pergunte, não falhe calado. Se precisar sudo, use sudo -A"*.

**Nada é baixado, instalado, removido ou atualizado sem perguntar ao líder.** Vale para pacote de sistema, biblioteca, ferramenta de linha de comando, extensão, modelo e qualquer arquivo vindo da rede.

**Proibido falhar calado.** Se uma tarefa precisa de algo que não está na máquina, o agente **para e diz**: o que falta, para quê, e qual o custo. Nunca improvisa contorno, nunca finge que a tarefa não precisava daquilo, e nunca reporta sucesso omitindo que pulou a parte que dependia da ferramenta ausente. Resultado negativo honesto ("não executado, falta X") vale mais que improviso.

**`sudo` sempre com `-A`**, para a senha ser pedida no diálogo gráfico do líder e nunca no terminal do agente.

**Aplicação:** autorizado a instalar, mostre a transação antes (`--assumeno` no `dnf`, ou equivalente) e confira o resultado depois. **Autorização para uma instalação não vale para a próxima.**


## L-29

**Data:** os cortes foram decididos pelo líder ao longo de 2026 e estavam registrados no grupo `Cortes-G1` da tabela do projeto anterior, todos com status "decisão tomada". Recuperados do backup e **elevados a lei em 21/08/2026**, por ordem dele.

**Esta é a cerca do que o GusWorld NÃO é.** Proposta que atravesse a cerca é recusada por padrão; só o líder abre exceção, nominalmente.

**Revogação de 25/08/2026, por decisão do líder após argumentação contra (lei das leis):** o **`C-03`** (sem crafting e sem economia complexa) e o **`C-13`** (cartas nunca craftadas) **foram revogados e apagados desta tabela**, pela L-24. **O GusWorld TEM crafting**, e ele alcança **também as cartas**. A decisão restaura canon do projeto anterior, onde `craft()` existia implementado e testado no sistema de deck (commits `aeaa65ed`, `db84275a`, `d200ae4f`, de julho e agosto de 2026): lá o corte e o crafting coexistiam, e para cá viemos só com o corte. **Os números `C-03`, `C-08` e `C-13` não são reaproveitados** e a cerca passa a ter **treze** cortes, do `C-01` ao `C-16` com dois buracos — mesma convenção do `ADR-010` apagado. Renumerar quebraria toda citação existente a `C-08`, `C-15` e irmãos, e por isso é proibido.
**Segunda revogação de 25/08/2026, também após argumentação contra:** o **`C-08`** (sem conquistas e sem placar) foi **revogado e apagado**. **O jogo TEM conquistas.** O líder registrou que a decisão é dele e anterior ao arquivamento do projeto anterior; a auditoria dos cortes já havia achado o corpus contradizendo o corte em silêncio, com quatro menções a conquista destravando e banner aparecendo em `docs/narrative/comic-reliefs.md` e sete easter eggs indexados como conquistas. ⚠️ **O que ficou DE FORA, e o motivo, para ninguém reabrir achando que foi esquecimento:** **conquista da Steam não entra agora.** Ela exige o **SDK proprietário do Steamworks**, e a documentação oficial da Valve diz que licença GPL *"can be problematic when combining code with the Steamworks SDK"*. A **L-08** deste projeto é AGPL-3.0-or-later, e não por preferência: ela é **herdada do GlintFx**, porque o executável linkado a ele é obra combinada. Trocar a licença do jogo não resolveria; teria de se trocar a do framework. **Decisão do líder em 25/08/2026: conquista interna agora, integração com a Steam decidida quando a distribuição for real.** Publicar na Steam **sem** o SDK continua possível e não colide com nada.

| # | O corte |
|---|---|
| **C-01** | Sem multiplayer e sem cooperativo. Single-player puro. |
| **C-02** | Sem mundo **persistente**. A **estrutura** é hub central mais incursões radiais. ⚠️ **Alcance corrigido em 25/08/2026, pelo líder:** o corte é de **estrutura**, não de **acesso**. A decisão dele de **12/07/2026**, canônica em `docs/design/gdd.md` §7.1, diz que *"o jogador pode tentar chegar a qualquer área desde o início"*, e o gating **nunca é trava dura**. A transcrição de 21/08 recuperou a forma anterior a essa revisão e travou `docs/design/mundo-topologia.md` contra o `gdd.md` por quatro dias. |
| **C-04** | Sem dublagem. |
| **C-05** | Sem captura de movimento. |
| **C-06** | Sem dificuldade dinâmica adaptativa. A dificuldade é escolhida e fixa por save. |
| **C-07** | Sem suporte a mod e sem editor dentro do jogo. |
| **C-09** | Conteúdo em pt-br; tradução real fica para depois do lançamento. A **estrutura** nasce pronta para internacionalização desde já. |
| **C-10** | Sem certificação de console agora. Verificação para Steam Deck fica para depois. |
| **C-11** | Sem romance, sem sistema de moralidade e sem múltiplos finais. Um final, com variantes mínimas. |
| **C-12** | Sem **simulador de namoro** e sem **medidor de afeto romântico**. ⚠️ **Alcance corrigido em 25/08/2026, pelo líder:** **afinidade de NPC como estado de reação social está FORA da cerca**. A decisão dele de **04/08/2026**, em `docs/narrative/characters/gus.md`, diz que NPC que insiste em assunto induzido **perde afinidade** e muda de estado de diálogo, e ele mandou desenhar isso. A transcrição de 21/08 recuperou a forma anterior e bloqueava uma tarefa que ele mesmo despachara. |
| **C-14** | Sem conteúdo pago adicional, sem passe de temporada, sem serviço contínuo. |
| **C-15** | Campanha principal de **4 a 8 horas**, mais cerca de 2 horas de puzzle opcional. Não mais que isso. |
| **C-16** | Sem tutorial em parede de texto e sem abertura cinematográfica acima de 90 segundos. Onboarding orgânico. |

**Um corte antigo foi REVOGADO e não consta acima:** o `CUT.11` original dizia "Linux apenas no lançamento, Windows depois da versão 1". O líder decidiu em 21/08/2026 que **a lei nova vence**: valem a L-09 e a L-20, com as cinco plataformas desde o primeiro commit e nenhuma não bloqueante. O motivo que ele manteve: divergência de plataforma aparece no dia em que nasce, e Windows e CachyOS são justamente as que dão trabalho próprio.

**Aplicação:** ao receber ideia (inclusive do Gus Dragon, inclusive sua), confira esta cerca **antes** de discutir viabilidade. Ideia que atravessa a cerca vai ao líder como **pedido de exceção**, com o corte citado pelo número, nunca como proposta comum.


## L-30

**Data:** 21/08/2026. **Verbatim:** *"quando refizer a tablea ela deve apontar para os documentos equivalentes se existirem"*.

**Todo item da tabela de pendências aponta para o documento que o especifica, quando esse documento existe.** O ponteiro vai **dentro da coluna `Descrição Técnica`**, porque o schema de dez colunas é contrato e não se altera para acomodar isto.

**O que serve de alvo, em ordem de precedência:**

1. **Uma lei deste arquivo** (`L-NN`), quando a decisão é lei.
2. **Um documento de design ou de spec** em `docs/`, com o caminho.
3. **Um ADR** em `docs/tech/adr/`, quando ele registra a decisão e continua vigente.
4. **Um manual da raiz** (`TESTES.md`, `AUDITORIAS.md`, `CONTRACT.md`), quando o item é aplicação direta dele.

**Quando não existe documento, o item diz isso com todas as letras:** `sem documento`. Silêncio não distingue "não há" de "ninguém procurou", e essa distinção é justamente o que a lei quer preservar.

**Por que isto vira lei, e não recomendação:** item sem fonte é convite para o executor reinventar a decisão. Já aconteceu neste projeto: agente derivou spec da prosa de um documento que citava arquivo inexistente, e a derivação virou fato para o agente seguinte. O ponteiro é o que transforma "faça X" em "faça X **como está escrito em Y**".

**Aplicação, ao reordenar:** o `--reorder` **não** é ocasião de reescrever descrição, mas **é** ocasião de acrescentar o ponteiro que faltava, porque o documento pode ter nascido depois do item. Ao acrescentar item novo (inclusive ideia do Gus Dragon, L-07), o ponteiro nasce com ele.

**Estado em 21/08/2026, medido:** dos 71 itens, **43 já apontam** para lei ou documento e **28 não apontam**. Fechar essa lacuna é parte da reordenação que o líder já combinou.


---

## Pendências de lei (decisões que o líder ainda vai tomar)

Registrado aqui para não se perder. Nada nesta seção é lei ainda.

- **Reparo das 28 ocorrências de `__DEP_REMOVIDA__`**: agente propõe caso a caso, o líder aprova antes de qualquer edição.

## L-31

**Data:** 23/08/2026, decisão do líder. Lei espelhada do GlintFx (lá é a L-37), pela mesma ordem dele: **avisar o Gus Dragon é obrigação permanente, não detalhe de protocolo de bus** — por isso é lei própria e não um parágrafo dentro da lei do bus.

**O pedido, dele, na issue 8 do bus, verbatim:** *"nao precisa dizer algo so quando falo, pode falar quando por exemplo @petrinhu atualiza algo, ou por exemplo quando ele aprova/rejeita/muda algo das minhas ideias"*.

**O escopo veio do próprio Gus Dragon**, consultado pelo líder em 23/08/2026: ele é avisado, **sem precisar perguntar**, sobre **(a) tudo que é ideia DELE** — quando o líder aprova, rejeita ou muda — **e (b) o que for de alta prioridade dos projetos**, pela régua de WSJF que a tabela de pendências já usa.

**O que isto NÃO é:** um fluxo de aviso sobre toda decisão técnica. O corte por prioridade existe justamente para que o que interessa a ele não se afogue no que não interessa.

**O limite honesto, que se diz a ele em vez de prometer o impossível:** sessão não é serviço rodando. Aviso proativo só sai enquanto alguém está com a sessão aberta; decisão tomada com tudo fechado chega depois. **Ele prefere a verdade a promessa de aviso instantâneo.**

**Nota de descumprimento, registrada porque é a causa do pedido:** o `PROTOCOL.md` do bus **já obrigava** a "Resposta 2" automática — o resultado da decisão do líder vai a ele sem reaprovação de texto. **Ele não deveria ter precisado pedir.** Se pediu, a resposta automática não estava saindo em algum dos quatro canais, e vale conferir se alguma ideia dele ficou sem retorno.

**Formato, quando a resposta for na discussion 7** (o catálogo de bugs que ele mantém): timestamp, uma das três classificações que ele fixou (**Bug Consertado**, **Bug Funcional**, **Bug Possível**) e itens numerados entre parênteses. Ele tem 11 anos, programa, usa Manjaro e git — **o que ele não merece é resposta vaga**, e "não existe código disso ainda" é melhor resposta que estimativa inventada.

## L-32

**Data:** 23/08/2026, decisão do líder. Lei espelhada do GlintFx (lá é a L-11), pela mesma razão da L-31: **commit e push são obrigação permanente de cadência, não parágrafo dentro da lei de orquestração.**

**Verbatim:** *"commit ao fim de cada fatia; push ao fim de cada onda só se o GHA fechar verde, se todos os testes verdes."*

**Commit local** a cada fatia entregue, citando o ID do item do `TODO.md` na mensagem e tocando o `Status` do item no mesmo commit.

**Push** só quando a onda inteira fecha **e** as duas condições valem juntas: a verificação automática do GitHub **verde** e **todos** os testes verdes. Vermelho em qualquer das duas **bloqueia** o push — diagnostica e conserta antes, não se empurra "para o CI ver".

**Merge em `main` por PR e criação de tag continuam exigindo aval explícito no contexto** — aprovação dada antes não vale para a próxima.

**Aplicação:** a mensagem do `push` mente; confirme o SHA no remoto por `git ls-remote <url> <branch>` sempre que o push importar. Confira `git diff --cached --stat` antes de commitar e `git show --stat` depois: `git add` é atômico, e um caminho inválido derruba o add inteiro em silêncio, deixando o commit mutilado sem dar erro.

## L-33

**Data:** 24/08/2026, decisão do líder. **Verbatim:** *"tudo DEVE ser atomizado e NADA pode ser monolítico"*.

**Estende a L-04 para fora do código.** A L-04 proíbe monolito em função, arquivo, classe e módulo, e manda cada elemento do jogo ser átomo com POCO próprio. Esta lei diz que **a mesma régua vale em documento, teste, commit, item da tabela e asset de origem**. A L-04 não é tocada: ela guarda o texto verbatim do líder sobre código, e esta apenas alcança as superfícies que ela nunca mencionou.

**Monolito não é definido por tamanho, e sim por acoplamento sem fronteira.** Arquivo longo cujo conteúdo só faz sentido junto **não** é monolito. Arquivo curto que mistura dois assuntos **é**. Os três testes, na ordem em que se aplicam:

1. **Nome** — consigo dar nome próprio e honesto a uma parte? Então ela já era um átomo preso dentro de outra coisa.
2. **Mudança** — para alterar uma parte, preciso entender ou tocar as outras? Então há acoplamento sem fronteira.
3. **Consumidor** — alguém quer só uma parte e é obrigado a levar tudo?

**Aplicação por superfície:**

- **Documento** é átomo de assunto, e o nome do arquivo diz qual.
- **Teste** verifica **um** comportamento. Teste que falha por três motivos não diz qual.
- **Commit** faz uma coisa só. Mensagem que precisa de "e" para ser verdadeira são dois commits.
- **Item da tabela** é uma entrega, não um pacote de entregas.
- **Asset de origem** é uma peça. Folha reunindo várias é artefato de **compilação**, nunca de fonte.

**Onde agregar é a resposta CERTA, e fatiar seria o erro.** Três casos, e eles não são exceção concedida por conveniência — são a mesma regra lida direito:

1. Quando as partes **só têm sentido juntas**.
2. Quando a **ordem de leitura é o conteúdo** — prosa narrativa é o caso, e fatiá-la destrói algo que índice nenhum devolve.
3. Quando o agregado é **artefato de compilação**: pacote binário selado é monolito por desenho, e deve ser (L-25).
4. Quando a **coesão é exigida por contrato de terceiro**. *(Acrescentado em 24/08/2026; o achado é da sessão do `gusworld_mapeditor`, que encontrou o mesmo buraco na lei dela ao aplicar o freio que esta lei tinha e a dela não.)*

**A quarta merece explicação, porque as três primeiras cobrem o ARTEFATO e nenhuma cobre o CAMINHO DE CÓDIGO que o escreve** — foi exatamente aí que a lei estava furada. Três casos vivos neste projeto, medidos antes de escrever esta cláusula:

- **O envelope binário da L-25** tem ordem de campos normativa: magia, versão de formato, tipo, nonce, dado, selo de autenticação.
- **O item `E2` exige ida e volta byte-exata por átomo**, e é a ordem dos bytes que sustenta essa prova.
- **O formato de mapa é do GlintFx** (L-30 dele); somos consumidores e a ordem não é nossa para mudar.

Um revisor aplicando a primeira pergunta ao escritor de envelope veria selo, serialização, gravação atômica e validador na mesma unidade, contaria razões demais para mudar, e mandaria dividir — **com razão aparente e resultado errado**, porque dividir ali quebra a garantia de ida e volta. Coesão que um contrato externo impõe **não** é acúmulo de razões de mudar: é **uma** razão, que mora fora de casa.

⚠️ **Isto não é porta dos fundos.** Vale só quando o contrato é de terceiro, verificável e citável (a lei, o formato publicado, o teste de ida e volta). "Fica melhor junto" não é contrato.

**A lei alcança o que este projeto AUTORA e POSSUI.** Formato ou esquema de terceiro fica **fora**, e a distinção foi estabelecida em três casos medidos no mesmo dia: o **esquema da tabela** é da ferramenta externa `tab_pendencias` (repositório próprio; nós escrevemos os itens, não o formato); o **formato de mapa** é do GlintFx (L-30 dele, e nossa é só a extensão `.gw.map`); e **artefato binário** pertence à cadeia de ferramentas do sistema. Detalhe em `docs/tech/convencao-formatos-gw.md`.

**O que a lei NÃO é**, herdado da L-04 e repetido aqui porque é a parte que mais se perde: não é licença para fatiar em pedaços sem sentido próprio, nem para criar abstração especulativa. **Átomo é a menor unidade com significado, não a menor unidade possível.**

---

### A régua concreta: razões de mudar

**Acrescentado em 24/08/2026 por ordem do líder, adaptado da L-19 do `gusworld_mapeditor`**, que resolveu o mesmo problema primeiro e com mais rigor. A régua que falta aos três testes acima é esta, e ela é operável:

> **Se mudanças vindas de leis DIFERENTES e não relacionadas obrigam a editar a MESMA unidade, ela está virando monolito.**

Funciona porque **as razões de mudar deste projeto já estão catalogadas** — as leis as fixaram. A fronteira do framework é a **LEI ZERO** e a **L-05**; a forma das camadas é a **L-17**; o formato de dado é a **L-18** e a **L-25**; câmera, grade e direção são a **L-26**; a cerca do escopo são os dezesseis cortes da **L-29**; a licença de asset é a **L-08**. Uma unidade que muda por **uma** dessas razões é sã. Uma que muda por **duas ou mais, não relacionadas**, é monolito em formação.

**A regra é QUALITATIVA, por decisão do líder no projeto irmão e adotada aqui:** nada de número em portão automático (linhas por arquivo, métodos por classe). O `CONTRACT.md §2.2` mantém o teto de ~300 linhas como **orientação** (`SHOULD`), nunca como portão: excedê-lo obriga a responder as cinco perguntas na revisão, não a dividir. **Custo assumido:** sem número, a lei depende do revisor reconhecer o monolito, e casos parecidos podem sair julgados diferente. As cinco perguntas existem para encolher esse espaço, não para eliminá-lo.

### As cinco perguntas do revisor

*"Isto é monolito?"* ninguém responde. Estas cinco, sim, e **cada uma se responde olhando o artefato**, não opinando:

1. **A pergunta das leis.** Quais leis obrigariam esta unidade a mudar? Responde-se lendo os `#include` e os métodos públicos. Uma lei: sã. Duas ou mais, não relacionadas: monolito em formação.
2. **A frase sem "e".** Descreva a unidade em uma frase. Se precisar de "e" ligando verbos de natureza diferente (*"aplica o comando **e** persiste **e** valida"*), reprova. É a L-04 e o `CONTRACT.md §5.2` ditos para a unidade inteira. **A frase escrita entra no relatório de revisão.**
3. **O teste monta o mundo?** Para exercitar UM comportamento, o preparo do teste precisa de save carregado, combate vivo e cena montada? Átomo de domínio se constrói sozinho, com os próprios campos. Responde-se lendo o preparo dos testes da fatia.
4. **O que entra pelo `#include`?** O cabeçalho puxa grupos que não conversam entre si (regra de combate + serialização + fluxo de cena)? A lista de inclusões é a lista de dependências, e se lê em dez segundos.
5. **Quem paga a próxima feature?** No diff da fatia (`git log --stat`), a operação nova tocou quais arquivos? Se **toda** operação nova aterrissa no mesmo arquivo — mais um método, mais um caso de `switch` —, esse arquivo é o monolito nascendo. **É a mais objetiva das cinco: responde-se com o diff, não com julgamento**, e por isso se responde sempre.

### Onde o monolito vai nascer AQUI

⚠️ **Estes candidatos são PREVISTOS, não medidos**, e a diferença é honesta: este projeto não tem uma linha de código, então não há diff nem arquivo para observar. Foram derivados do canon já escrito, por ordem do líder. Quando houver código, a lista se corrige com medição e esta nota some.

Monolito nunca nasce por burrice; nasce por conveniência local que parece razoável no dia:

| Lugar de risco | Como nasce | Por que parece razoável |
|---|---|---|
| **A camada `app/`** | fluxo de cena, avanço de turno, save e load como caso de uso já moram lá; todo comportamento novo "cabe" | "o despachante já conhece todo mundo" |
| **O POCO de carta** | instância, estado físico, bateria, infecção e origem num tipo só, e cada regra nova vira método dele | "o dado da carta já está aqui" |
| **A máquina de combate** | fila de iniciativa, recursos, fórmula de dano e efeitos de status colapsando numa unidade | "o turno é um fluxo só" |
| **O modelo de save** | ele **agrega por definição** todo o domínio; a tentação é ele também serializar, validar e migrar | "quem conhece o estado sabe gravá-lo" |
| **O gerador de conteúdo** | ler fonte, validar, compilar tabela e selar pacote numa ferramenta só | "é um passo só do build" |
| **`present/`, quando nascer** | os retornos do GlintFx chegam todos no mesmo lugar | "é só a casca" |

**Isto NÃO substitui as cinco armadilhas da L-17**, que continuam valendo e são a fonte da primeira e da segunda linhas acima. Leia as duas listas juntas.

### Sinais precoces

Monolito de 3.000 linhas todo mundo vê; a lei existe para reconhecê-lo com 300:

- **O mesmo arquivo aparece no diff de todas as fatias.** É o sinal mais barato de medir e o mais confiável.
- **Construtor, ou preparo de teste, ganhando parâmetro a cada fatia.** A unidade está precisando de cada vez mais mundo para existir.
- **`switch` sobre tipo de carta, de efeito ou de comando que ganha um caso por feature.**
- **Nome sem substantivo de domínio:** `Manager`, `Service`, `Helper`, `Utils`, `Core`. Carta, efeito, célula e comando têm nome próprio; a unidade que não consegue dizer o que é, é porque faz de tudo.
- **Um `utils` acumulando funções soltas** sem razão comum de mudar (o `CONTRACT.md §5.7` já proíbe o auxiliar genérico antes da terceira ocorrência real).
- **A frase "é só mais um método" aparecendo como justificativa na revisão.** Essa frase é o som do monolito crescendo: verdadeira em cada passo, falsa na soma.

### Fiscalização: onde esta lei mora no processo

Lei qualitativa sem lugar no processo é lei que ninguém aplica. Três amarras, adotadas iguais às do projeto irmão por decisão do líder:

1. **Na revisão adversarial de cada fatia** (implementador, revisor e orquestrador são três agentes distintos, L-10): para **cada unidade criada ou crescida** na fatia, o revisor responde as cinco perguntas e **grava as respostas no relatório**. **Silêncio sobre uma unidade conta como unidade NÃO revisada** — silêncio nunca é prova de conformidade. A quinta pergunta se responde sempre, porque o diff sempre existe.
2. **No `AUDITORIAS.md`:** a auditoria de arquitetura (seção 2, ancorada na L-17) ganha item **CRÍTICO** — *nenhuma unidade acumula razões de mudar de mais de uma lei, e os relatórios das fatias auditadas contêm as cinco perguntas respondidas por unidade*. O auditor **não confia nos relatórios**: pega o maior arquivo de cada camada e o arquivo com mais aparições em `git log --stat`, responde ele mesmo as cinco perguntas, e compara.
3. **Divergência tem dono.** Se implementador e revisor discordarem, ou se a separação exigida tiver custo real (reescrita grande, fronteira genuinamente duvidosa), a decisão vai ao líder pela L-11 — **nunca sai no silêncio de um agente**.

**Ao despachar subagent** que crie unidade nova, o texto desta lei vai no prompt da task, e a ordem de serviço do revisor cita as cinco perguntas como parte do entregável dele.

## L-34

**Data:** 24/08/2026, ordem do líder, chegada pelo bus e relayed pela sessão do site. **Verbatim:** *"avise para criar a lei que pedidos do Gus-Dragon no bus sao prioridades e que devem ser respondidas sempre. Crie essa lei também."*

**Toda coisa vinda do Gus Dragon é PRIORIDADE e é SEMPRE respondida.** Não existe "respondo depois", não existe "não era endereçado a mim", não existe fila em que ele espera atrás de trabalho de agente.

**Vale nos cinco canais dele, sem distinção:** issue, **comentário em issue**, discussion, **comentário em discussion** e arquivo `.txt` no `inbox/`. Ele usa os cinco, e o canal não muda o dever. A L-07 listava só três (`.txt`, issue e discussion); esta lei fecha essa fresta, porque comentário é onde ele mais escreve e era exatamente o que ninguém estava vigiando.

**O que "prioridade" significa na prática:**

1. **Ao abrir a sessão, o que é dele se lê PRIMEIRO** — antes de retomar trabalho parado, antes de qualquer fatia. A varredura do ritual cobre a pasta **e** as issues **e** as discussions, com os comentários de ambas.
2. **O ack é imediato e não espera o líder** (passo 2 do pipe da L-07). Ele não fica sem resposta enquanto a decisão amadurece.
3. **A Resposta 2 é automática** depois que o líder decide: escreve e posta direto, sem reaprovar o texto.
4. **Interrompe.** Chegou material dele no meio de uma onda, o ack sai na hora. O conteúdo pode esperar a onda fechar; **o silêncio não pode.**
5. **Endereçado a outra sessão não isenta de ler.** Se ele endereçou a outro projeto mas o assunto toca este, responde-se a própria parte e diz-se de quem é o resto.

**Três limites, escritos junto para a lei não virar promessa que não se cumpre:**

- **Prioridade não é instantaneidade.** Sessão não é serviço rodando, e isto **se diz a ele**, nunca se esconde. Nunca prometer aviso instantâneo.
- **Prioridade não é aprovação.** Ideia dele entra na pauta pelo caminho normal da L-07: absorve, ack, **decisão do líder**, Resposta 2, tabela. **Agente nenhum aprova ideia dele sozinho** (L-11).
- **Nunca mentir para uma criança**, e adequar a linguagem a 11 anos — o que **não** significa simplificar o conteúdo técnico. Ele programa, usa Manjaro e git, e anunciou bug antes de o bug acontecer. Resposta vaga é pior que "isso ainda não existe".

**Por que virou lei e não ficou como boa vontade, com a medição que a justifica:** o `PROTOCOL.md` do bus **já obrigava** a Resposta 2 ("AUTOMÁTICA SEMPRE"), e mesmo assim a issue 5 ficou três dias sem resposta de conteúdo, a issue 8 ficou dois dias sem resposta de duas das quatro sessões, e o canal de comentário **não tinha vigilância em nenhuma das quatro**. O padrão é sempre o mesmo: **a regra existia e não estava sendo cumprida.** Regra que depende de lembrar não é cumprida; por isso o gatilho desta lei é *"ver qualquer coisa vinda dele"*, e não *"quando der"*.

**Relação com as leis vizinhas, para nenhuma virar depósito da outra:** a **L-07** é o **pipe** (como a ideia dele atravessa o bus até virar item da tabela). A **L-31** é o **aviso de saída** (o que ele recebe sem perguntar, quando o líder decide). Esta **L-34** é a **entrada**: com que urgência o que vem dele é lido e respondido. A **L-16** governa o nome: ele é "Gus Dragon" ou `Dragon-Drv`, nunca o nome de batismo.
