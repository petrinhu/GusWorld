> **LEI DAS LEIS, ANTERIOR ATÉ À LEI ZERO: só o líder pode quebrar uma lei deste arquivo** — agente nenhum quebra, flexibiliza, reinterpreta ou "adapta ao caso" por conta própria — **e nem a ordem direta dele dispensa a confirmação**: antes de executar, nomeie a lei que está sendo quebrada, cite o texto dela, diga o que ela protege e o que se perde ao quebrá-la, e pergunte por `AskUserQuestion` se é isso mesmo que ele quer; **quando o pedido for ALTERAR ou REVOGAR uma lei, argumente CONTRA primeiro, sempre e sem exceção**, com razões concretas, o problema que a lei existe para impedir, os trade-offs da mudança e o que fica desprotegido depois dela, e só então leve a escolha por `AskUserQuestion` entre **confirmar** a alteração e **cancelá-la**; pressa, obviedade aparente, "ele já mandou uma vez" e aprovação dada em outro contexto **nunca** substituem essa confirmação, e silêncio jamais vale como aval. (Ordem do líder, 22/08/2026.)

> **LEI ZERO, ACIMA DE TODAS: O JOGO ASSENTA EM GLINTFX, E QUEM É DONO DA JANELA E DO LAÇO É ELE.** O GusWorld liga em exatamente duas coisas: o **GlintFx** e o **sistema operacional**. Ordem do líder em 21/08/2026, verbatim: *"aceita apenas link com framework GlintFx em ../Glintfx e [github]/petrinhu/GlintFx e com o SO. [...] Você NUNCA cria workarounds nem passa por cima da camada de GlintFx. Se ainda não existir a funcao em GlintFx, registre a necessidade no bus e espere parado a resposta dele."* Qualquer análise, atalho ou desenho que contorne o GlintFx está **errado por construção**. Exceção a esta lei só existe se o líder a conceder nominalmente, caso a caso. — **REFORMA DE 28/08/2026, decisão do líder após argumentação contra (lei das leis). Verbatim:** *"o que entra no produto entregue é dependência e está proibido; o que só ajuda a construir e nunca é ligado ao binário, não é. Logo, se você precisar de python como apoio, além de bash script, pode."* **A palavra que sempre esteve na lei é `liga`**, e a reforma diz o que ela significa: **dependência é o que embarca no que o jogador recebe.** Ferramenta que constrói, verifica ou gera, e **nunca** é ligada ao binário, **não é dependência** — CMake, Ninja, `ctest`, `clang-tidy`, `cppcheck`, `gitleaks`, `git`, `reuse`, **Python** e script de shell entram por aqui. ⚠️ **A régua operacional, que o líder mandou entrar junto para a linha não ficar fofa: _se o arquivo sumisse da máquina do JOGADOR, o jogo deixaria de rodar?_** **Sim** → é dependência, e está proibida. **Não** → é ferramenta, e é permitida. A pergunta é sobre a máquina **dele**, não sobre a nossa. ⚠️ **O caso do GERADOR, que a reforma resolve nominalmente porque era o furo:** um gerador (de catálogo `.gw.*`, de código, de tabela) **não linka**, logo é **ferramenta** — mas **o que ele produz EMBARCA**, e por isso o **output obedece integralmente às leis do produto** (L-18: nada em formato de texto; L-25: envelope selado). *"Não linka"* nunca foi o mesmo que *"não afeta o que o jogador recebe"*. ⚠️ **O que a reforma NÃO faz, e foi dito ao líder antes de ele decidir:** ela não elimina a superfície de **supply chain**. Pacote comprometido no caminho de build injeta no artefato sem jamais aparecer como dependência do binário. Ferramenta de build é zona **permitida**, não zona **neutra** — e quanto menos ferramenta, menor a superfície. **Motivo de fundo, medido em 28/08/2026:** sob a leitura estrita anterior, o próprio projeto era ilegal — quatro arquivos Python rastreados, incluindo o guard **da própria LEI ZERO**, mais o CMake e o `ctest` que o `C1` vai usar. Lei que o próprio build viola é lei que todos aprendem a ignorar, e isso corrói as outras. — **A divisão de responsabilidade é a que o próprio GlintFx declara**, ordem do líder em 21/08/2026, verbatim: *"do modo que glintfx disse"*, apontando para a **L-02** do `GODS_LAWS.md` dele, que diz: *"framework 2D completo (janela, loop, render2d, input, gamepad, áudio, fonte, asset, math2d); **o consumidor escreve só a lógica dele**"*. Portanto: **janela, laço principal, entrada, áudio e desenho são do GlintFx. O GusWorld escreve a lógica do jogo, e mais nada.** O jogo **não** cria janela, **não** possui contexto gráfico e **não** roda laço próprio de quadro. — **Registro de decisão revogada, para ninguém ressuscitar:** o `ADR-010` do projeto anterior adotava o modelo inverso (o jogo dono da janela, o framework embutido como camada de interface). Aquele modelo só fazia sentido porque existia uma biblioteca externa criando a janela, e ela foi removida do ecossistema em 20/08/2026. O ADR-010 foi **apagado** do repositório em 21/08/2026 por ordem do líder, sob a L-24.

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
| [L-05](#l-05) | precisar de algo que o GlintFx ainda não tem, ou de ferramenta de apoio | Proibido dublê: ou liga no GlintFx, ou não existe. ⚠️ **Ferramenta que só constrói e nunca embarca NÃO é dependência** (LEI ZERO, reforma de 28/08/2026) |
| [L-06](#l-06) | decidir o que construir primeiro | Núcleo puro agora; a camada que desenha só nasce com o GlintFx |
| [L-07](#l-07) | precisar de função do framework, ou receber ideia do Gus | Bus: pedido só quando o jogo esbarrar de verdade na falta |
| [L-08](#l-08) | criar `LICENSE`, cabeçalho de arquivo, versionar asset ou publicar | AGPL-3.0-or-later no código; assets e lore com todos os direitos reservados; livros à parte |
| [L-09](#l-09) | escrever CI ou declarar suporte de plataforma | Cinco alvos; Fedora 44 primário; CachyOS é próprio, não Arch |
| [L-10](#l-10) | ir executar qualquer trabalho de produto | Main só orquestra; auditoria/arquitetura são de C-level fable da constelação; implementação é de agente operacional sonnet da constelação, nunca genérico |
| [L-11](#l-11) | fazer **qualquer** pergunta ao líder, inclusive de esclarecimento | `AskUserQuestion` SEMPRE, com opções clicáveis; nunca pergunta em prosa |
| [L-12](#l-12) | escrever qualquer mensagem ao líder | Timestamp real `[DD/MM/YY - HH:MM:SS]` obtido do `date` |
| [L-13](#l-13) | tocar em qualquer trabalho que dependa de canon | Canon desatualizado bloqueia: atualiza primeiro, trabalha depois |
| [L-14](#l-14) | concluir que algo está morto, obsoleto ou descartável | Nada é declarado morto por agente; a decisão é do líder |
| [L-15](#l-15) | versionar binário, asset pesado ou configurar o repositório | `livros/` e `glb/` fora do git; LFS no resto; regras nascem antes do primeiro commit |
| [L-16](#l-16) | homenagear pessoa real, ou citar o filho do líder | Homenagem só com aceite; o filho aparece só como "Gus Dragon" ou Dragon-Drv; caso do roster fechado, não se reabre |
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
| [L-29](#l-29) | propor recurso, sistema ou escopo novo | Os 14 cortes: a cerca do que o jogo NÃO é, incluindo o escopo fechado sem duração fixa da campanha (C-15) |
| [L-30](#l-30) | escrever, reordenar ou acrescentar item na tabela, ou escrever/editar checklist em qualquer outro documento do projeto | Tabela aponta para o documento que a especifica; TODO.md é a única fonte de checklist, o resto aponta de volta pelo ID, sem caixa de estado própria |
| [L-31](#l-31) | o líder aprovar, rejeitar ou mudar algo, ou fechar item de alta prioridade | Avisar o Gus Dragon sem ele perguntar |
| [L-32](#l-32) | fechar uma fatia, fechar uma onda, ou pensar em `git push` | Commit por fatia; push só ao fim da onda, aceito só quando a verificação automática daquele push sai verde e os testes estão verdes |
| [L-33](#l-33) | criar unidade nova, escrever documento, teste, commit, item ou asset, **ou revisar fatia** | Atomizar fora do código; monolito é acoplamento, não tamanho; cinco perguntas na revisão |
| [L-34](#l-34) | ver qualquer coisa vinda do Gus Dragon, em qualquer dos cinco canais | Pedido dele é prioridade e é SEMPRE respondido; ack imediato interrompe a fatia, conteúdo espera a onda fechar |
| [L-35](#l-35) | rodar qualquer teste que EXECUTE código: suíte, fuzzing, sanitizer, mutação, sonda de janela ou entrada | Nenhum teste dinâmico toca a sessão do líder; sempre em container. Sem container, o resultado é "não executado" |
| [L-36](#l-36) | decidir se lore ainda não escrita atrasa uma fatia de engenharia, ou sentir a tentação de esperar completar canon antes de codar | Lore que falta não bloqueia código, puxa-se sob demanda em micro-sessão pontual; distinta da L-13, que trava por canon que contradiz, não por canon que ainda falta |
| [L-37](#l-37) | editar, revisar ou propor mudança em fala, aparte, revide, cena ou traço de caracterização já aprovado que define o Gus original | Mudança em conteúdo já aprovado do Gus original exige nova autorização explícita do líder; parente da L-16, sem fundir nela |

---

## L-01

Nenhuma linha de código, arquivo ou protótipo do projeto anterior serve de base, referência ou canon para o código do GusWorld, que nasce do zero. Nenhuma referência a arquivo de código do corpus é tratada como existente, e nenhum campo derivado da prosa se fecha sem o líder. A documentação e a lore em `docs/` e `resources/` não são atingidas por esta lei.

## L-02

Nenhum modelo, mesh, câmera 3D ou bake entra no jogo em runtime: o jogo é 2D pixel-art, e o pipeline 3D é exclusivo do líder, para livro e impressão 3D. Nenhum agente gera, converte, organiza ou opina sobre esse pipeline sem pedido explícito dele.

## L-03

Nenhuma linguagem além de C++23 entra no código do jogo ou no seu padrão sem ordem explícita do líder; ferramenta de apoio ao build que não embarca no binário do jogador foge desta proibição (LEI ZERO).

## L-04

Nenhuma função, arquivo, classe ou módulo do jogo reúne mais de um assunto ou responsabilidade: monolito é proibido em todo nível. Nenhum elemento do jogo (carta, item, inimigo, efeito, diálogo, missão) vive como campo dentro de um agregado maior; cada um é átomo com POCO próprio.

## L-05

Nenhum stub, mock, simulacro, camada própria embrulhando o GlintFx, seleção condicional entre implementações ou modo headless simulando o framework é permitido: o que não liga no GlintFx não existe. Regra de jogo pura, que não precisaria do GlintFx para funcionar, escapa desta proibição e pode ser escrita antes de o framework existir.

## L-06

Nenhuma camada que desenha nasce antes de o GlintFx ter janela, contexto gráfico, entrada e texto; só o núcleo de regra de jogo, em TDD estrito, é escrito agora. Nenhum relatório apresenta tela, demo ou progresso visual que não existe.

## L-07

Nenhum pedido de função ao GlintFx sai por antecipação: só quando o jogo esbarrar de verdade na falta, com o uso real descrito. Nenhum pedido sai fora do bus, clone `<vault>/gusworld_ia_autocomm/`, sob o slug `gusworld`. Nenhum pedido enviado pelo bus carrega classificação de prioridade de quem recebe, e nenhuma ideia do Gus Dragon fica sem confirmação imediata de recebimento, sem discussão com o líder, sem resposta honesta e sem entrada imediata na tabela de pendências logo após a decisão, citando a autoria dele.

## L-08

Nenhum código do jogo sai sob licença diferente de AGPL-3.0-or-later, e a ressalva sobre o caráter offline nunca entra dentro do texto da licença. Nenhum cabeçalho de copyright usa titular diferente de Petrus Alves da Silva Costa, o titular designado pelo líder e registrado em `REUSE.toml`. Nenhum asset, texto de sabor ou prosa in-game sai de um regime único de todos os direitos reservados a esse titular, ressalvados apenas marca (nome, logotipo, trade dress, nomes de personagem) e conteúdo de fã não comercial. Nenhum dos dois livros-companheiros herda a licença do jogo. Dentro de `docs/`, nada em `narrative/` ou `specs/` sai do regime reservado; na raiz, `CHARS.md`, `PLACES.md` e `sinopse.md` também não saem dele. A distribuição de cada caminho do repositório entre a licença do código e o regime reservado segue `REUSE.toml`, o documento autorizado por esta lei. Nenhum arquivo nasce sem identificação padronizada de copyright e licença: fonte de código leva cabeçalho próprio, binário e asset levam arquivo companheiro, e diretório inteiro pode ser coberto por um arquivo de regras.

## L-09

Nenhum CI cobre menos que cinco alvos distintos (Fedora 44 pinado, Ubuntu, Arch, CachyOS e Windows), e nenhuma declaração de suporte a uma plataforma se apoia no job verde de outra, especialmente CachyOS sobre Arch.

## L-10

Nenhum trabalho de produto é executado pela thread principal: ela só orquestra, delega e reverifica. Nenhuma auditoria e nenhuma criação de arquitetura saem de agente que não seja C-level da constelação bigtech, no modelo fable. Nenhuma implementação sai de agente que não seja agente operacional da constelação bigtech, no modelo sonnet. Nenhum agente genérico, anônimo ou improvisado executa auditoria, arquitetura ou implementação. Nenhum C-level fica dormente por presunção de agente, e quem implementa nunca é quem revisa nem quem reverifica.

## L-11

Nenhuma pergunta ao líder, inclusive de esclarecimento sobre o que ele quis dizer, sai em prosa solta: toda pergunta usa `AskUserQuestion`, sem o campo `preview`, com 2 a 3 alternativas, prós, contras, impacto e esforço, a recomendada primeiro. Nenhum agente decide sozinho o que muda o próximo passo, exceto decisão trivial e reversível com default óbvio, que segue o default e é apenas informada; esta exceção governa só a forma da pergunta que já ia acontecer, nunca cria pergunta nova. Relatar resultado e oferecer o próximo passo continuam sendo prosa, não pergunta. Nenhuma decisão do líder que seja destrutiva, viole princípio do projeto ou inviabilize marco é executada sem o agente antes nomear o problema, o risco concreto e uma alternativa; reafirmada a ordem, o agente executa por inteiro.

## L-12

Nenhuma mensagem ao líder começa sem o timestamp real no formato `[DD/MM/YY - HH:MM:SS]`, obtido do relógio do sistema no momento da mensagem; nunca estimado ou reaproveitado do timestamp anterior.

## L-13

Nenhum trabalho que dependa de canon contraditório avança antes de o canon ser atualizado com o líder. Nenhuma implementação se apoia em canon que se sabe errado, e nenhuma contradição encontrada fica fora do registro. Bloqueios vivos, abertos e do líder: a re-derivação técnica do esquema de save em `docs/narrative/diary/knowledge-gates.md` segue bloqueada; os documentos de `docs/design/producao/` que descrevem pipeline aposentado seguem bloqueados; as sinalizações do parágrafo 5 de `docs/design/mecanicas/modos-morte.md` seguem abertas.

## L-14

Nenhum agente declara algo morto, obsoleto, descartável ou irrelevante por conta própria: documento, decisão, referência órfã, asset ou ideia perdida vai ao líder com a evidência, e a decisão é dele.

## L-15

Nenhum arquivo de `resources/livros/` ou `resources/glb/` entra no controle de versão. Nenhum binário pesado do restante do projeto entra fora do Git LFS. Nenhuma regra de exclusão ou de atributo do repositório nasce depois do primeiro commit sem autorização do líder: elas nascem antes dele, e só mudam depois com decisão dele registrada.

## L-16

Nenhuma homenagem a pessoa real existe sem aceite prévio confirmado pelo líder, nunca presumido pelo agente. Nenhum nome de batismo de menor é versionado: o filho do líder aparece só como Gus Dragon ou como Dragon-Drv. Nenhum agente reabre o caso já fechado do análogo do capstone do roster; a decisão do líder ali é definitiva, o histórico do git não se reescreve, e a seção de fontes consultadas daquele arquivo fica como está.

## L-17

Nenhuma camada da espinha (`present/`, `app/`, `domain/`, `content/`, `core/`) depende da de cima, e nenhuma inclusão do GlintFx sobrevive ao gate de CI fora de `present/`, que só nasce quando o GlintFx existir. Nenhuma regra de jogo deixa de ser transição determinística que recebe estado e comando e devolve estado novo e eventos, com o sorteio dentro do próprio estado. Nenhuma unidade vira dona de tudo, nenhum barramento genérico substitui os tipados, nenhum condicional de dificuldade substitui tabela de parâmetros, nenhum detalhe de C++ quebra o replay determinístico, e nenhuma regra de domínio vaza para a interface. Nenhum replay determinístico fica sem o teste automatizado permanente que o verifica (semente mais lista de comandos reproduz o mesmo estado final). Nenhum número entra literal no corpo da regra: ele nasce nomeado em tabela de parâmetros de configuração. Nenhum POCO de domínio é escondido atrás de superfície opaca: ao contrário do GlintFx, aqui eles são visíveis por desenho, porque são o contrato interno e o formato de save.

## L-18

Nenhum dado do jogo (mapa, save, configuração ou catálogo de conteúdo) chega à máquina do jogador em formato de texto (a proibição é sobre o que se distribui, não sobre a fonte de conteúdo mantida em texto estruturado no repositório), e a régua é a máxima proteção contra edição, não o mínimo suficiente. Nenhum custo real desta lei é contornado ou silenciado: é nomeado ao líder, com mitigação proposta.

## L-19

Nenhuma linha de comportamento entra sem um teste que falhava antes dela existir, e nenhuma ordem de serviço se fecha sem a saída real do teste vermelho e depois verde; ao escrever a asserção, pergunta-se o que o defeito produziria, nunca só o que o acerto produz. Nenhum commit de módulo de código com comportamento passa sem os cinco portões, valendo a partir do primeiro módulo desse tipo (commit de canon e de documentação fica fora deles): zero aviso de compilação tratado como erro, sanitizadores de memória e de comportamento indefinido a cada fatia fechada, análise estática automatizada, varredura de segredo cobrindo também o histórico completo do controle de versão, e o portão de mensagem de commit, que barra termo da lista cifrada, caminho de área cifrada, dado pessoal e credencial, e reprova o commit quando não consegue ler a chave, nunca aprova em silêncio. Nenhum push sai sem rodar antes, local, o mesmo script que a verificação remota roda. Nenhuma meta numérica de cobertura é perseguida: cobertura é consequência do TDD, nunca alvo. Nenhuma release 1.0 sai sem o dossiê formal de auditoria.

## L-20

Nenhuma plataforma da matriz de cinco entra depois do primeiro commit: a fundação de build e o CI são a primeira fatia do projeto. Nenhum módulo de comportamento depois dessa fundação escapa do TDD sob a alegação de ainda estar montando a base.

## L-21

Nenhum apagamento do documento fundador do projeto acontece antes de cada um dos 28 itens estar resolvido, decidido em lei, executado de fato no repositório e memorizado no lugar certo. Nenhum apagamento acontece sem o checklist item a item apresentado ao líder e confirmado por ele.

## L-22

Nenhum identificador ou comentário de código sai do inglês em `snake_case`, sem prefixo de membro; esta lei substitui o `CONTRACT.md` §5.1 nesse ponto. Nenhuma mensagem de commit sai do português, e nenhum termo de lore sem tradução honesta (nome próprio, unidade inventada, língua construída) é traduzido para inglês.

## L-23

Nenhuma versão ou tag do GusWorld sai fora do formato de quatro componentes: o primeiro para quebra de compatibilidade sentida pelo jogador, o segundo para recurso novo compatível, o terceiro para correção e o quarto para build sem mudança de código. Nenhuma tag ou release sai sem aval explícito do líder no contexto, mesmo com o formato fixado.

## L-24

Nenhum texto revogado pelo líder permanece legível em bloco de histórico, nota de status superado ou seção recolhida: revogado se apaga por inteiro, inclusive do índice de gatilhos. Nenhuma exceção existe além de texto com efeito jurídico ou direito adquirido de terceiro, que não se apaga e vai ao líder. Nenhuma citação órfã para documento inexistente é apagada por agente sozinho: primeiro o líder decide, pela L-14, que o documento citado está morto; só depois se apaga o ponteiro, preservando a afirmação.

## L-25

- Nenhuma comunicação pública promete impossibilidade de editar, recuperar ou trapacear em save, configuração, mapa ou catálogo: só detecção confiável de adulteração.
- Nenhum desses quatro dados sai de fora do envelope binário único com selo criptográfico, serialização binária versionada, gravação atômica com cadeia de backups e validação em dois níveis (selo e validador semântico) no carregamento.
- Nenhuma fase da proteção (envelope selado; catálogo compilado com save híbrido verificável por reexecução; âncora anti-rollback por chip de segurança do hardware, opcional) avança fora de ordem, e nenhuma delas trava o jogo de rodar sem esse chip.
- Nenhum texto de conteúdo chega à distribuição: a fonte fica em texto estruturado dentro do repositório, lida só pelo gerador em tempo de build.
- Nenhuma amarra de máquina existe fora do slot Hardcore.
- Nenhuma primitiva criptográfica vem de biblioteca de terceiro, implementação própria ou API do sistema operacional: vem só do GlintFx.
- Nenhum caminho além de `docs/_secret/`, `docs/narrative/deep/stinger/`, `docs/book/` e `tools/security/termos_proibidos.txt` entra na cifra do repositório sem ordem nova do líder, e nenhuma marcação de cifra acontece depois do primeiro registro do conteúdo no controle de versão.
- Nenhuma configuração com selo inválido recusa abrir o jogo: cai em padrão de fábrica com aviso.
- Nenhum formato de save, mapa ou pacote nasce sem a ferramenta de inspeção e reselo junto.
- Nenhum número ou regra de carta fica fora do executável: são código sob a licença do código e vivem dentro dele. Nenhum texto de sabor ou prosa de carta entra no executável: ficam no pacote binário selado como asset.

## L-26

Nenhuma perspectiva do jogo sai de 3/4 top-down fixa com mapa em grade quadrada, e nenhuma câmera gira ou orbita. Nenhuma direção além das quatro cardeais existe, e nenhuma delas é espelhamento horizontal de outra: todas são desenhadas à mão. Nenhuma proposta de diagonal, rotação de câmera ou projeção isométrica avança sem decisão do líder.

## L-27

Nenhuma interface, arquivo de marcação, folha de estilo, tela piloto ou prova de conceito é escrita antes de o GlintFx traduzir marcação de tela.

## L-28

Nenhum pacote, biblioteca, ferramenta, extensão, modelo ou arquivo de rede é baixado, instalado, removido ou atualizado sem perguntar ao líder antes. Nenhuma tarefa que falte algo falha calada: o agente para e diz o que falta, para quê e o custo. Nenhuma elevação de privilégio roda sem `sudo -A`, pedindo a senha por diálogo gráfico, e nenhuma autorização de instalação vale para a instalação seguinte.

## L-29

Nenhuma proposta de recurso, sistema ou escopo atravessa a cerca abaixo sem virar pedido de exceção explícito ao líder, citado pelo número do corte. Nenhum número de corte revogado (`C-03`, `C-13`) é reaproveitado ou renumerado: a lacuna fica na tabela.

| # | O corte |
|---|---|
| **C-01** | Nenhum multiplayer e nenhum cooperativo: só single-player. |
| **C-02** | Nenhum mundo aberto e nenhum mundo persistente; a geografia é a do documento de topologia do mundo (treze áreas em grafo), não uma forma ditada por este corte, e nenhum gating é trava dura: o jogador pode tentar chegar a qualquer área desde o início. |
| **C-04** | Nenhuma voz no jogo, inclusive leitura assistiva do Diário; dublagem só como conteúdo adicional pós-lançamento, fora desta cerca. |
| **C-05** | Nenhuma captura de movimento. |
| **C-06** | Nenhuma dificuldade dinâmica adaptativa: só dificuldade escolhida e fixa por save. |
| **C-07** | Nenhum suporte a mod e nenhum editor dentro do jogo. |
| **C-08** | Nenhum placar e nenhum ranking entre jogadores; conquistas internas não são atingidas. |
| **C-09** | Nenhum lançamento sai sem conteúdo completo em português e em inglês. |
| **C-10** | Nenhuma certificação de console agora, inclusive verificação para portáteis específicos. |
| **C-11** | Nenhum romance, nenhum sistema de moralidade e nenhum final além de um único com variantes mínimas; ressalvados um afeto platônico infantil nunca correspondido em texto e o placar de comedimento, que é economia de conduta. |
| **C-12** | Nenhum simulador de namoro e nenhum medidor de afeto romântico; afinidade de personagem não jogável como reação social não é atingida. |
| **C-14** | Nenhum conteúdo pago adicional, nenhum passe de temporada e nenhum serviço contínuo; expansão gratuita pós-lançamento não é atingida. |
| **C-15** | Nenhum número de horas fixa a duração da campanha antes de existir cena jogável para medir, e nenhum escopo de campanha fica aberto: ela é fechada e definida por conteúdo, e mecânica nova só entra se couber num jogo com fim. |
| **C-16** | Nenhum tutorial em parede de texto e nenhuma abertura cinematográfica acima de noventa segundos. |

## L-30

Nenhum item da tabela de pendências fica sem apontar, na coluna de descrição técnica, para a lei, documento de design, ADR ou manual que o especifica; na ausência de um, o item diz que não há documento. Nenhum documento do projeto além da tabela de pendências mantém checklist com estado próprio, duplica a contagem, a soma ou o percentual dela, ou cria arquivo ou seção paralela de fila de trabalho: caixa de marcar fora da tabela vira marcador simples, e citar o status de um item só é legítimo nomeando o identificador dele. Checklist-gabarito de processo, reutilizável e sem estado por item, não é fila de trabalho e fica fora desta proibição.

## L-31

Nenhuma aprovação, rejeição ou mudança de ideia do Gus Dragon, e nenhum fechamento de item de alta prioridade, fica sem aviso a ele sem que precise perguntar. Nenhum aviso promete instantaneidade além do que a sessão aberta permite, e nenhuma resposta na discussion 7, o catálogo de bugs dele, sai sem timestamp e uma das três classificações que ele fixou: Bug Consertado, Bug Funcional, Bug Possível.

## L-32

Nenhuma fatia fecha sem commit local citando o identificador do item e tocando o status dele no mesmo commit. Nenhum push sai antes de a onda inteira fechar, com todos os testes verdes; nenhum push se considera aceito antes de a verificação automática remota daquele push sair verde, e reprovação nela bloqueia o que vem depois. Nenhum merge na branch principal e nenhuma tag saem sem aval explícito no contexto, e nenhum push se dá por confirmado sem checar o estado publicado no remoto, nunca só a mensagem do comando.

## L-33

Nenhum documento, teste, commit, item de tabela ou asset de origem escapa da regra que proíbe monolito em código: monolito não é definido por tamanho, é acoplamento sem fronteira, medido por nome próprio possível, mudança que exige tocar outras partes, e consumidor forçado a levar tudo. Nenhuma fatiação acontece nos quatro casos em que agregar é certo: partes que só têm sentido juntas, ordem de leitura que é o próprio conteúdo, artefato de compilação, e coesão exigida por contrato de terceiro verificável. Nenhuma unidade acumula razões de mudar vindas de leis diferentes e não relacionadas; isso é monolito em formação. Nenhuma unidade criada ou crescida numa fatia fecha revisão sem as cinco perguntas do revisor respondidas e registradas (quais leis a obrigam mudar, se a descrição dela precisa de duas ações ligadas, se o teste monta o mundo inteiro para exercitá-la, se a lista de inclusões mistura grupos que não conversam, e quem paga a próxima funcionalidade no histórico de mudanças); silêncio conta como não revisada, e divergência entre implementador e revisor vai ao líder. Nenhum átomo é menor que a menor unidade com significado: fatiar sem sentido e abstração especulativa são proibidos tanto quanto o monolito. Nenhum portão automático por número (linhas, métodos) substitui as cinco perguntas: o teto de linhas do `CONTRACT.md` é recomendação, nunca portão. Nenhum teste verifica mais de um comportamento, nenhum commit faz mais de uma coisa, nenhum item de tabela é mais de uma entrega, e folha que reúne vários assets é artefato de compilação, nunca de fonte. Nenhum auditor aceita relatório de revisão sem conferir ele mesmo: responde as cinco perguntas sobre o maior arquivo de cada camada. Nenhum formato ou esquema de terceiro entra nesta lei: ela vale só para o que este projeto autora e possui.

## L-34

Nenhuma mensagem do Gus Dragon, em qualquer dos cinco canais dele (issue, comentário em issue, discussion, comentário em discussion e arquivo de texto na caixa de entrada do bus), fica sem leitura em primeiro lugar na sessão nem sem confirmação imediata de recebimento, que interrompe a fatia em andamento na hora; o conteúdo da resposta espera a onda fechar. Nenhuma resposta automática falta após a decisão do líder. Nenhuma ideia dele é aprovada por um agente sozinho, e nenhuma resposta promete instantaneidade que a sessão não garante ou deixa de se adequar a uma criança.

## L-35

Nenhum teste dinâmico (suíte, integração, fuzzing, sanitizador, teste de mutação, benchmark, sonda de janela, teclado, mouse ou tela) roda na sessão viva do líder: roda sempre em container, e sem container disponível o resultado é que não foi executado. Nenhuma proteção se apoia em desligar variável de ambiente de exibição: `XDG_RUNTIME_DIR` do processo sob teste é trocado por diretório próprio, com permissão restrita a `chmod 700`, e a troca é provada em vigor antes de interagir, nunca só assumida por ter subido o container. O compositor gráfico aninhado dentro do container é piso, nunca alternativa a ele.

## L-36

Nenhuma fatia de engenharia espera lore ainda não escrita: lore que falta se puxa sob demanda em micro-sessão pontual, nunca em ciclo completo de aprofundamento de lore. Esta lei nunca compete com a lei do canon contraditório: canon que contradiz bloqueia, canon que ainda não existe não bloqueia.

## L-37

Nenhuma fala, aparte, revide, cena ou traço de caracterização do Gus original já aprovado pelo líder linha a linha é editado, reescrito, cortado, comprimido ou acrescentado sem nova autorização explícita dele, nem quando a mudança parece compressão honesta. Esta lei não se confunde com a lei de homenagem a pessoa real: aquela autoriza a homenagem existir; esta autoriza mudar o que já foi aprovado.
