> **LEI DAS LEIS, ANTERIOR ATÉ À LEI ZERO.** **Data:** ordem do líder, 22/08/2026.
>
> Não quebre, não flexibilize, não reinterprete nem "adapte ao caso" lei deste arquivo por conta própria: só o líder quebra lei, e nem a ordem direta dele dispensa a confirmação. Antes de executar a quebra, nomeie a lei, cite o texto dela, diga o que ela protege e o que se perde ao quebrá-la, e pergunte por `AskUserQuestion` se é isso mesmo que ele quer. **Diante de pedido de ALTERAR ou REVOGAR lei, argumente CONTRA primeiro, sempre e sem exceção**, com razões concretas, o problema que a lei existe para impedir, os trade-offs da mudança e o que fica desprotegido depois dela, e só então leve por `AskUserQuestion` a escolha entre **confirmar** a alteração e **cancelá-la**. Não aceite pressa, obviedade aparente, "ele já mandou uma vez" nem aprovação dada em outro contexto no lugar dessa confirmação, e nunca tome silêncio como aval.

> **LEI ZERO, ACIMA DE TODAS: O JOGO ASSENTA EM GLINTFX, E QUEM É DONO DA JANELA E DO LAÇO É ELE.** **Data:** 21/08/2026. **Verbatim:** *"aceita apenas link com framework GlintFx em ../Glintfx e [github]/petrinhu/GlintFx e com o SO. [...] Você NUNCA cria workarounds nem passa por cima da camada de GlintFx. Se ainda não existir a funcao em GlintFx, registre a necessidade no bus e espere parado a resposta dele."*; e *"do modo que glintfx disse"*, apontando para a **L-02** do `GODS_LAWS.md` do GlintFx, que diz: *"framework 2D completo (janela, loop, render2d, input, gamepad, áudio, fonte, asset, math2d); **o consumidor escreve só a lógica dele**"*. Apagamento do `ADR-010` por ordem do líder em 21/08/2026, sob a GW-24. **Reforma de 28/08/2026**, pela LEI DAS LEIS, depois da argumentação contra: *"o que entra no produto entregue é dependência e está proibido; o que só ajuda a construir e nunca é ligado ao binário, não é. Logo, se você precisar de python como apoio, além de bash script, pode."* A régua do jogador entrou na reforma por ordem do líder.
>
> Ligue o GusWorld em exatamente duas coisas: o **GlintFx** e o **sistema operacional**. Deixe com o GlintFx a **janela, o laço principal, a entrada, o áudio e o desenho**, e escreva no GusWorld a lógica do jogo, e mais nada: o jogo **não** cria janela, **não** possui contexto gráfico e **não** roda laço próprio de quadro. Não adote o modelo inverso, em que o jogo é dono da janela e o framework vira camada de interface embutida.
>
> Trate como **errado por construção** toda análise, atalho ou desenho que contorne o GlintFx. Se a função ainda não existe no GlintFx, registre a necessidade no bus e espere parado. Aceite exceção a esta lei só quando o líder a conceder nominalmente, caso a caso.
>
> Leia `liga` assim: **dependência é o que embarca no que o jogador recebe**, e está proibida; ferramenta que constrói, verifica ou gera, e **nunca** é ligada ao binário, **não é dependência** e é permitida (CMake, Ninja, `ctest`, `clang-tidy`, `cppcheck`, `gitleaks`, `git`, `reuse`, **Python** e script de shell). Decida pela régua: **_se o arquivo sumisse da máquina do JOGADOR, o jogo deixaria de rodar?_** **Sim**, é dependência e está proibida; **não**, é ferramenta e é permitida. Faça a pergunta sobre a máquina **dele**, não sobre a nossa.
>
> Trate o **gerador** (de catálogo `.gw.*`, de código, de tabela) como ferramenta, porque ele **não linka**, e o que ele produz como produto, porque **EMBARCA**: faça o output obedecer integralmente às leis do produto (GW-18: nada em formato de texto; GW-25: envelope selado).
>
> Trate a ferramenta de build como zona **permitida**, não **neutra**, porque pacote comprometido no caminho de build injeta no artefato sem jamais aparecer como dependência do binário, e quanto menos ferramenta, menor a superfície.

# GODS_LAWS.md

> Ordens expressas do líder (petrus). Execute este arquivo, não o declare: confira o **gatilho** de cada lei **no momento da ação**, não no fim.

Cite estas leis como GW-NN e as do GODS_LAWS global como L-NN. Leia como a GW-NN de mesmo número a L-NN de lei do projeto citada em commit, comentário ou verbatim antigo.

## Protocolo de uso (obrigatório)

1. **Antes de agir**, varra a coluna "Gatilho" do índice abaixo; se um gatilho casa com o que você vai fazer, leia a lei inteira antes do primeiro comando, não depois.
2. **Ao despachar subagent**, cole no prompt da task o texto completo das leis cujo gatilho casa com ela, mais o caminho absoluto deste arquivo, porque subagent **não herda** este contexto e não vai ler por conta própria.
3. **Ao relatar ao líder**, diga qual lei aplicou e como, se tocou área com lei, porque silêncio não prova conformidade.
4. **Registre aqui a lei nova no instante em que o líder a dá**, com data e o texto dele verbatim entre aspas, sem esperar "um momento melhor".
5. **Não revogue, não flexibilize nem reinterprete lei**: só o líder o faz. Na dúvida sobre o alcance de uma lei, pergunte por `AskUserQuestion` antes de agir.
6. Em conflito entre uma lei daqui e qualquer outro documento (manual, memória, hábito, preferência do agente), **siga a lei daqui**.

## Índice de gatilhos

| Lei | Gatilho: dispara quando você vai... |
|---|---|
| [GW-01](#gw-01) | procurar código anterior, "como era antes", ou base para reaproveitar |
| [GW-02](#gw-02) | tocar arte, câmera, sprite, modelo ou pipeline visual |
| [GW-03](#gw-03) | criar build, escolher padrão de linguagem |
| [GW-04](#gw-04) | escrever função, arquivo, classe, módulo, item ou carta |
| [GW-05](#gw-05) | precisar de algo que o GlintFx ainda não tem, ou de ferramenta de apoio |
| [GW-06](#gw-06) | decidir o que construir primeiro |
| [GW-07](#gw-07) | precisar de função do framework, ou receber ideia do Gus |
| [GW-08](#gw-08) | criar `LICENSE`, cabeçalho de arquivo, versionar asset ou publicar |
| [GW-09](#gw-09) | escrever CI ou declarar suporte de plataforma |
| [GW-10](#gw-10) | ir executar qualquer trabalho de produto |
| [GW-11](#gw-11) | fazer **qualquer** pergunta ao líder, inclusive de esclarecimento |
| [GW-12](#gw-12) | escrever qualquer mensagem ao líder |
| [GW-13](#gw-13) | tocar em qualquer trabalho que dependa de canon |
| [GW-14](#gw-14) | concluir que algo está morto, obsoleto ou descartável |
| [GW-15](#gw-15) | versionar binário, asset pesado ou configurar o repositório |
| [GW-16](#gw-16) | homenagear pessoa real, ou citar o filho do líder |
| [GW-17](#gw-17) | criar módulo, arquivo, ou desenhar a forma de um sistema |
| [GW-18](#gw-18) | escolher formato de arquivo de mapa, save, configuração ou item |
| [GW-19](#gw-19) | commitar, fechar fatia, pensar em cobertura e formatação, **ou escrever ordem de serviço que vai ser revisada** |
| [GW-20](#gw-20) | escrever CI, ou declarar que uma plataforma funciona |
| [GW-21](#gw-21) | achar que o `inicial.md` cumpriu o papel dele |
| [GW-22](#gw-22) | nomear qualquer coisa, ou escrever comentário e commit |
| [GW-23](#gw-23) | criar tag, publicar release, ou mexer na versão |
| [GW-24](#gw-24) | ver regra, seção ou documento que o líder revogou, ou escrever relatório/reconferência que narraria por que uma versão anterior dele reprovou |
| [GW-25](#gw-25) | tocar save, configuração, mapa ou catálogo de conteúdo |
| [GW-26](#gw-26) | tocar câmera, mapa, locomoção ou direção de sprite |
| [GW-27](#gw-27) | pensar em escrever tela, HUD ou marcação de interface |
| [GW-28](#gw-28) | precisar baixar, instalar ou atualizar qualquer coisa |
| [GW-29](#gw-29) | propor recurso, sistema ou escopo novo |
| [GW-30](#gw-30) | escrever, reordenar ou acrescentar item na tabela, ou escrever/editar checklist em qualquer outro documento do projeto |
| [GW-31](#gw-31) | o líder aprovar, rejeitar ou mudar algo, ou fechar item de alta prioridade |
| [GW-32](#gw-32) | fechar uma fatia, fechar uma onda, ou pensar em `git push` |
| [GW-33](#gw-33) | criar ou revisar unidade de código do jogo, ou pensar em atomizar teste, ferramenta, documento, commit, item ou asset |
| [GW-34](#gw-34) | ver qualquer coisa vinda do Gus Dragon, em qualquer dos cinco canais |
| [GW-35](#gw-35) | rodar qualquer teste que EXECUTE código: suíte, fuzzing, sanitizer, mutação, sonda de janela ou entrada |
| [GW-36](#gw-36) | decidir se lore ainda não escrita atrasa uma fatia de engenharia, ou sentir a tentação de esperar completar canon antes de codar |
| [GW-37](#gw-37) | editar, revisar ou propor mudança em fala, aparte, revide, cena ou traço de caracterização já aprovado que define o Gus original |
| [GW-38](#gw-38) | desenhar, revisar ou implementar qualquer enigma, puzzle ou câmara de lógica do jogo |

---

## GW-01

Escreva o código do GusWorld do zero, sem usar linha de código, arquivo ou protótipo do projeto anterior como base, referência ou canon. Trate como inexistente toda referência a arquivo de código do corpus, e feche com o líder todo campo derivado da prosa. Deixe fora desta lei a documentação e a lore em `docs/` e `resources/`.

## GW-02

Não ponha no jogo, em runtime, modelo, mesh, câmera 3D ou bake: o jogo é 2D pixel-art, e o pipeline 3D é exclusivo do líder, para livro e impressão 3D. Não gere, não converta, não organize nem opine sobre esse pipeline sem pedido explícito dele.

## GW-03

Use só C++23 no código do jogo e no padrão dele, salvo ordem explícita do líder. A ferramenta de apoio ao build que não embarca no binário do jogador fica fora desta proibição (LEI ZERO).

## GW-04

Dê a cada função, arquivo, classe ou módulo do jogo um só assunto ou responsabilidade: monolito é proibido em todo nível. Faça de cada elemento do jogo (carta, item, inimigo, efeito, diálogo, missão) um átomo com POCO próprio, nunca um campo dentro de um agregado maior.

## GW-05

Não escreva stub, mock, simulacro, camada própria embrulhando o GlintFx, seleção condicional entre implementações nem modo headless que simule o framework: o que não liga no GlintFx não existe. A regra de jogo pura, que não precisaria do GlintFx para funcionar, fica fora desta proibição, e você pode escrevê-la antes de o framework existir.

## GW-06

Não crie camada que desenha antes de o GlintFx ter janela, contexto gráfico, entrada e texto; até lá, escreva só o núcleo de regra de jogo, em TDD estrito. Não apresente em relatório tela, demo ou progresso visual que não existe.

## GW-07

Peça função ao GlintFx só quando o jogo esbarrar de verdade na falta, com o uso real descrito, nunca por antecipação. Mande todo pedido pelo bus, no clone `<vault>/gusworld_ia_autocomm/`, sob o slug `gusworld`, sem classificar a prioridade de quem recebe. Dê a toda ideia do Gus Dragon confirmação imediata de recebimento, discussão com o líder, resposta honesta e entrada imediata na tabela de pendências logo após a decisão, citando a autoria dele.

## GW-08

Licencie todo o código do jogo sob AGPL-3.0-or-later, e nunca ponha dentro do texto da licença a ressalva sobre o caráter offline. Use como titular de todo cabeçalho de copyright Petrus Alves da Silva Costa, o titular designado pelo líder e registrado em `REUSE.toml`. Mantenha todo asset, texto de sabor e prosa in-game num regime único de todos os direitos reservados a esse titular, ressalvados só marca (nome, logotipo, trade dress, nomes de personagem) e conteúdo de fã não comercial. Não estenda a licença do jogo aos dois livros-companheiros. Mantenha no regime reservado tudo o que está em `docs/narrative/` e `docs/specs/` e, na raiz, `CHARS.md`, `PLACES.md` e `sinopse.md`. Distribua cada caminho do repositório entre a licença do código e o regime reservado conforme o `REUSE.toml`, o documento autorizado por esta lei. Dê a todo arquivo, desde que nasce, identificação padronizada de copyright e licença: cabeçalho próprio na fonte de código, arquivo companheiro no binário e no asset; um arquivo de regras pode cobrir um diretório inteiro.

## GW-09

**Data:** decisão do líder de 11/10/2026, por `AskUserQuestion` e depois de contra-argumentação, com o chão de validação do GlintFx, que soltou o pino em 27/08/2026.

Cubra no CI cinco alvos distintos: Fedora, Ubuntu, Arch, CachyOS e Windows. Não declare suporte a uma plataforma apoiado no job verde de outra, em especial CachyOS sobre Arch. Faça todos os alvos da matriz acompanharem `:latest`, sem fixar versão de imagem, inclusive o Fedora, alvo primário por ser o sistema do líder, como no chão de validação do GlintFx. Aceite o custo de forma explícita: o CI pode ficar vermelho por atualização de distro, não só por mudança de código, e a GW-32 bloqueia push com CI vermelho.

## GW-10

**Data:** 20/11/2026, na sessão do claude_configs. **Verbatim:** *"Alinhar à LEI ZERO"*, a LEI ZERO do `GODS_LAWS.md` global, e *"Seguir o /modelos_sessao"*. Confirmação na sessão do projeto, pela LEI DAS LEIS, por `AskUserQuestion` em 20/11/2026: *"Confirmo as duas (Recomendado)"*. A redação é do agente; o texto do líder é só o das citações.

Não execute trabalho de produto na thread principal: ela só orquestra, delega e reverifica. Deixe auditoria e planejamento de arquitetura só a C-level da constelação bigtech, e criação e implementação só a agente operacional da constelação bigtech. Não ponha agente genérico, anônimo ou improvisado para executar trabalho. Não fixe modelo por esta lei: use o modelo e o effort que a skill `/modelos_sessao` mandar para cada agente, e, pela regra por tarefa dessa skill, mande prosa ou texto longo para leitor humano com `opus` e effort `max`. Não deixe C-level dormente por presunção de agente, e nunca deixe quem implementa revisar nem reverificar.

## GW-11

Pergunte ao líder sempre por `AskUserQuestion`, inclusive para esclarecer o que ele quis dizer, nunca em prosa solta: sem o campo `preview`, com 2 a 3 alternativas, prós, contras, impacto e esforço, e a recomendada primeiro. Não decida sozinho o que muda o próximo passo, salvo decisão trivial e reversível com default óbvio: nela, siga o default e apenas informe. Essa exceção governa só a forma da pergunta que já ia acontecer e nunca cria pergunta nova. Relate o resultado e ofereça o próximo passo em prosa, sem transformá-los em pergunta. Antes de executar decisão do líder que seja destrutiva, viole princípio do projeto ou inviabilize marco, nomeie o problema, o risco concreto e uma alternativa; reafirmada a ordem, execute por inteiro.

## GW-12

Comece toda mensagem ao líder pelo timestamp real no formato `[DD/MM/YY - HH:MM:SS]`, tirado do relógio do sistema no momento da mensagem, nunca estimado nem reaproveitado do timestamp anterior.

## GW-13

Não avance trabalho que dependa de canon contraditório antes de atualizar o canon com o líder. Não apoie implementação em canon que se sabe errado, e registre toda contradição encontrada. Mantenha vivos, abertos e com o líder estes bloqueios: a re-derivação técnica do esquema de save em `docs/narrative/diary/knowledge-gates.md`; os documentos de `docs/design/producao/` que descrevem pipeline aposentado; e as sinalizações do parágrafo 5 de `docs/design/mecanicas/modos-morte.md`.

## GW-14

Não declare por conta própria nada morto, obsoleto, descartável ou irrelevante: leve ao líder, com a evidência, o documento, a decisão, a referência órfã, o asset ou a ideia perdida, e deixe a decisão com ele.

## GW-15

Mantenha fora do controle de versão todo arquivo de `resources/livros/` e `resources/glb/`, e ponha no Git LFS todo binário pesado do resto do projeto. Crie as regras de exclusão e de atributo do repositório antes do primeiro commit, e mude-as depois só com decisão do líder registrada.

## GW-16

Crie homenagem a pessoa real só com aceite prévio confirmado pelo líder, nunca presumido pelo agente. Não versione nome de batismo de menor: escreva o filho do líder só como Gus Dragon ou como Dragon-Drv. Não reabra o caso já fechado do análogo do capstone do roster: a decisão do líder ali é definitiva, o histórico do git não se reescreve, e a seção de fontes consultadas daquele arquivo fica como está.

## GW-17

Faça cada camada da espinha (`present/`, `app/`, `domain/`, `content/`, `core/`) depender só das de baixo, e deixe o gate de CI barrar toda inclusão do GlintFx fora de `present/`, que só nasce quando o GlintFx existir. Escreva toda regra de jogo como transição determinística que recebe estado e comando e devolve estado novo e eventos, com o sorteio dentro do próprio estado. Não crie unidade dona de tudo, barramento genérico no lugar dos tipados, condicional de dificuldade no lugar de tabela de parâmetros, detalhe de C++ que quebre o replay determinístico, nem regra de domínio que vaze para a interface. Cubra o replay determinístico com teste automatizado permanente (semente mais lista de comandos reproduz o mesmo estado final). Não ponha número literal no corpo da regra: nomeie-o em tabela de parâmetros de configuração. Deixe os POCO de domínio visíveis por desenho, nunca atrás de superfície opaca, porque, ao contrário do GlintFx, aqui eles são o contrato interno e o formato de save.

## GW-18

Entregue à máquina do jogador todo dado do jogo (mapa, save, configuração ou catálogo de conteúdo) fora de formato de texto; a proibição é sobre o que se distribui, não sobre a fonte de conteúdo mantida em texto estruturado no repositório. Mire a máxima proteção contra edição, não o mínimo suficiente. Nomeie ao líder todo custo real desta lei, com mitigação proposta, sem contorná-lo nem silenciá-lo.

## GW-19

**Data:** emenda de 11/10/2026, adotada do projeto irmão GlintFx por ordem do líder: *"Pode fazer igual ao aviso de glintfx"*; lá, diante de dois dias sem código da biblioteca e cerca de 15 voltas de revisão, ele disse *"Já sao mais de DOIS DIAS de teste! Isso definitivamente não é normal. Otimize isso! Descubra como!"* e *"Está incompatível o rigor dos testes com o código. Ou os códigos estão PÉSSIMOS ou o rigor dos testes está com OE"*, e escolheu por `AskUserQuestion` **"Por camada"**. Emenda de 12/10/2026, na conversa com o líder: *"cuidado com OE nos testes!"*, *"Use KISS"*, *"e DRY"* e *"nao quero teste de teste de teste de teste..."* (OE: over-engineering, excesso de engenharia), confirmada por `AskUserQuestion` com as ressalvas abaixo. Exceção de 20/11/2026, quebra nomeada pela LEI DAS LEIS, por `AskUserQuestion`: *"Liberar até o C7 (Recomendado)"*. Esclarecimento de 21/11/2026, por `AskUserQuestion`, depois da revisão do `D1`: *"Só linhas de produto (Recomendado)"*.

Não escreva linha de comportamento sem um teste que falhava antes dela existir, e não feche ordem de serviço sem a saída real do teste vermelho e depois verde. Ao escrever a asserção, pergunte o que o defeito produziria, nunca só o que o acerto produz.

Passe todo commit de módulo de código com comportamento pelos cinco portões, a partir do primeiro módulo desse tipo; commit de canon e de documentação fica fora deles:

1. zero aviso de compilação, tratado como erro;
2. sanitizadores de memória e de comportamento indefinido a cada fatia fechada;
3. análise estática automatizada;
4. varredura de segredo cobrindo também o histórico completo do controle de versão;
5. portão de mensagem de commit, que barra termo da lista cifrada, caminho de área cifrada, dado pessoal e credencial, e reprova o commit quando não consegue ler a chave, nunca aprova em silêncio.

Rode local, antes de todo push, o mesmo script que a verificação remota roda. Não persiga meta numérica de cobertura: cobertura é consequência do TDD, nunca alvo. Não solte release 1.0 sem o dossiê formal de auditoria.

**Rigor da revisão por CAMADA.** Aplique a lei inteira ao código de produto e calibre a revisão pela camada:

- **N1, código de produto (as cinco camadas `core/`, `content/`, `domain/`, `app/`, `present/`):** faça revisão adversarial que executa e muta, até sair limpa. Mire a mutação **só nas linhas de PRODUTO que a fatia mudou**, no máximo 10 por revisão (padrão do Google: "State of Mutation Testing at Google", arxiv 2102.11378). Deixe linha de teste fora da mutação, e prove a força do teste pelo vermelho contra o defeito e pelos mutantes de produto que ele mata.
- **N2, portão que protege a sessão do líder** (o que a GW-35 exige: nenhum teste dinâmico roda fora de container): faça mutação mirada na exclusão que ele protege e **uma** rodada de conserto.
- **N3, ferramenta de teste comum** (o harness de `tests/harness/`, os autotestes, as travas de registro, os scripts de CI): faça revisão de leitura pelas cinco perguntas da GW-33 e confira **um** caso vermelho, sem o revisor procurar mutantes próprios. Na segunda reprovação da mesma família, mande o que sobra à INBOX ou declare-o como limite, e feche a fatia.
- **N4, texto** (documento, comentário, diário): dispense o espelho local do CI e o revisor.

Declare a camada no cabeçalho de toda ordem de serviço e tire dela o rigor e o orçamento de rodadas, nunca do julgamento do revisor. Como quem planeja, rode os comandos de cada ordem antes do despacho e anexe a saída à ordem.

Leia a emenda vinda do GlintFx com esta tradução de referência: onde o GlintFx diz `src/` e `include/`, aqui são as cinco camadas; as cinco perguntas de revisão são as da GW-33 deste projeto, não as da L-17; o portão de isolamento é o que a GW-35 exige; e o espelho local do CI é o item `C7`. Mantenha iguais aos do projeto irmão, de propósito, a substância, os números de rodadas e a fonte do limite de dez mutantes.

**Limite de níveis de verificação.**

- **Não crie nível novo de verificação sem ordem explícita do líder.** Nível de verificação é prova de que um portão reprova, autoteste de script, diagnóstico que mede ou controle que confirma outro controle.
- **Mantenha a prova de estreia vermelha**: veja cada portão novo reprovar contra aquilo que deve barrar antes de contá-lo como portão, porque ela já pegou portão falso, como um detector de comportamento indefinido que imprimia o erro e seguia em vez de reprovar. O limite corta a multiplicação de níveis, não o primeiro nível.
- **Antes de construir verificação nova, procure a que já existe**, porque a medição que se quer criar muitas vezes o portão já faz.
- **Enxugue o que inchou sem remover nada que reprova.**
- **Tire do arquivo do portão o diagnóstico que já respondeu o que media**, e registre a resposta na tabela de pendências do projeto.

**Exceção temporária: push do `D1` sem o espelho local.** Enquanto o item `C7` não fechar, mande as fatias do `D1` ao CI remoto sem rodar antes o espelho local que esta lei exige, com o buraco declarado nas linhas `D1` e `C7` do `TODO.md`. No dia em que o `C7` fechar, apague este parágrafo (GW-24).

## GW-20

Ponha as cinco plataformas da matriz desde o primeiro commit: a fundação de build e o CI são a primeira fatia do projeto. Não deixe módulo de comportamento, depois dessa fundação, escapar do TDD sob a alegação de ainda montar a base.

## GW-21

Não apague o documento fundador do projeto antes de cada um dos 28 itens estar resolvido, decidido em lei, executado de fato no repositório e memorizado no lugar certo, nem sem apresentar o checklist item a item ao líder e ter a confirmação dele.

## GW-22

Escreva identificador e comentário de código em inglês e em `snake_case`, sem prefixo de membro; nesse ponto, esta lei vence o `CONTRACT.md` §5.1. Escreva a mensagem de commit em português. Não traduza para inglês termo de lore sem tradução honesta (nome próprio, unidade inventada, língua construída).

## GW-23

Escreva versão e tag do GusWorld como `vA.B.C.D`, em quatro componentes, como no GlintFx: o primeiro para quebra de compatibilidade sentida pelo jogador, o segundo para recurso novo compatível, o terceiro para correção e o quarto para build sem mudança de código. Crie tag ou release só com aval explícito do líder no contexto, mesmo com o formato fixado.

## GW-24

**Data:** reforma de 06/09/2026, por `AskUserQuestion`: o líder escolheu, verbatim, a opção "Vale: apaga também". Fronteira do rodapé, mesma data, por `AskUserQuestion`: o líder escolheu, verbatim, a opção "Fica: é proveniência, não bilhete". Os argumentos que fundamentavam as duas opções foram escritos pelo orquestrador da sessão (main) na descrição da alternativa; o líder concordou com eles ao escolhê-las, não os formulou.

Apague por inteiro o texto que o líder revogar, inclusive do índice de gatilhos: não o guarde em bloco de histórico, nota de status superado nem seção recolhida. A única exceção é texto com efeito jurídico ou direito adquirido de terceiro: não o apague e leve-o ao líder. Não apague sozinho citação órfã para documento inexistente: espere o líder decidir, pela GW-14, que o documento citado está morto, e só então apague o ponteiro, preservando a afirmação.

Aplique a mesma proibição ao histórico de **PROCESSO**, não só ao canon do jogo: relatório de auditoria ou reconferência não narra dentro de si que um item já reprovou antes, quantas vezes nem por quê; apague, não reescreva em outra forma, porque uma régua só para tudo dispensa julgar caso a caso qual histórico é inocente, e critério que exige interpretar degrada na primeira exceção.

Mantenha o rodapé de "Última revisão" que fecha os documentos do projeto: ele registra a proveniência das decisões do líder com data e fica fora da proibição, e o corpo do documento diz só o que vale. Conte como rodapé só o BLOCO FINAL do arquivo, sem conteúdo normativo depois dele, aberto por uma linha que começa com "Última revisão" (com ou sem negrito, precedida ou não de separador `---`), que pode encadear "Revisão anterior" dentro do mesmo bloco, voltando várias datas. Aplique a regra geral, como a qualquer texto do corpo, às ocorrências que não são o bloco final: a nota de CABEÇALHO perto do topo de `resources/translations/pt_br.md` e `en_intl.md`, e o item de checklist de achados no meio de `docs/_processo/AUDIT-T8-CROSSREF-V2.md`.

Proteja o CONTEÚDO, nunca a POSIÇÃO: qualifique como rodapé protegido só a entrada que registra DATA e a DECISÃO ou MUDANÇA aplicada, o que vale e desde quando. Apague, mesmo dentro do bloco final, a entrada que renarra por que um estado anterior estava errado ou quantas vezes o item reprovou antes, e não mova esse texto do corpo para baixo de uma linha "Última revisão" para salvá-lo.

## GW-25

- Prometa em comunicação pública só detecção confiável de adulteração em save, configuração, mapa ou catálogo, nunca impossibilidade de editar, recuperar ou trapacear.
- Ponha esses quatro dados num envelope binário único com selo criptográfico, serialização binária versionada, gravação atômica com cadeia de backups e validação em dois níveis (selo e validador semântico) no carregamento.
- Avance as fases da proteção na ordem (envelope selado; catálogo compilado com save híbrido verificável por reexecução; âncora anti-rollback por chip de segurança do hardware, opcional), e não deixe nenhuma delas impedir o jogo de rodar sem esse chip.
- Não leve texto de conteúdo à distribuição: mantenha a fonte em texto estruturado dentro do repositório, lida só pelo gerador em tempo de build.
- Amarre à máquina só o slot Hardcore.
- Tire toda primitiva criptográfica só do GlintFx, nunca de biblioteca de terceiro, implementação própria ou API do sistema operacional.
- Ponha na cifra do repositório só `docs/_secret/`, `docs/narrative/deep/stinger/`, `docs/book/` e `tools/security/termos_proibidos.txt`, salvo ordem nova do líder, e marque a cifra antes do primeiro registro do conteúdo no controle de versão.
- Abra o jogo com configuração de selo inválido em padrão de fábrica, com aviso, sem recusar abrir.
- Crie todo formato de save, mapa ou pacote junto com a ferramenta de inspeção e reselo.
- Mantenha número e regra de carta dentro do executável, como código sob a licença do código, e o texto de sabor e a prosa de carta fora dele, no pacote binário selado como asset.

## GW-26

**Data:** decisão do líder, 12/09/2026, verbatim: "Opção um", escolhida entre confirmar oito direções desenhadas e cancelar a mudança.

Mantenha o jogo em perspectiva 3/4 top-down fixa, com mapa em grade quadrada, e não gire nem orbite a câmera. Desenhe à mão o sprite de toda criatura nas quatro direções cardeais, sem espelhar horizontalmente uma na outra. Aceite deslocamento diagonal na locomoção e exiba sempre o sprite cardeal mais próximo do vetor de movimento: move-se em oito, desenha-se em quatro. Não avance proposta de sprite diagonal, rotação de câmera ou projeção isométrica sem decisão do líder.

## GW-27

Não escreva interface, arquivo de marcação, folha de estilo, tela piloto nem prova de conceito antes de o GlintFx traduzir marcação de tela.

## GW-28

Pergunte ao líder antes de baixar, instalar, remover ou atualizar qualquer pacote, biblioteca, ferramenta, extensão, modelo ou arquivo de rede, e não use uma autorização de instalação para a seguinte. Não falhe calado: se falta algo à tarefa, pare e diga o que falta, para quê e o custo. Eleve privilégio só com `sudo -A`, com a senha pedida por diálogo gráfico.

## GW-29

**Data:** `C-15` decidido pelo líder em 11/10/2026, verbatim: *"Ele achou muito pouco 8h e concordo com ele. Esse jogo é pra ser longo, para meses de jogo, não horas"*; o "ele" é o Gus Dragon, que levantou a questão na issue 11 do barramento.

Não deixe proposta de recurso, sistema ou escopo atravessar a cerca abaixo sem virar pedido de exceção explícito ao líder, citado pelo número do corte. Não reaproveite nem renumere número de corte revogado (`C-03`, `C-13`): deixe a lacuna na tabela.

| # | O corte |
|---|---|
| **C-01** | Não faça multiplayer nem cooperativo: só single-player. |
| **C-02** | Não faça mundo aberto nem mundo persistente. Siga a geografia do documento de topologia do mundo (treze áreas em grafo), não uma forma ditada por este corte, e não faça de nenhum gating trava dura: o jogador pode tentar chegar a qualquer área desde o início. |
| **C-04** | Não ponha voz no jogo, nem na leitura assistiva do Diário; dublagem só como conteúdo adicional pós-lançamento, fora desta cerca. |
| **C-05** | Não use captura de movimento. |
| **C-06** | Não faça dificuldade dinâmica adaptativa: só dificuldade escolhida e fixa por save. |
| **C-07** | Não dê suporte a mod nem ponha editor dentro do jogo. |
| **C-08** | Não faça placar nem ranking entre jogadores; conquistas internas ficam fora deste corte. |
| **C-09** | Não lance sem conteúdo completo em português e em inglês. |
| **C-10** | Não busque, nesta fase do projeto, certificação de console, inclusive verificação para portáteis específicos. |
| **C-11** | Não faça romance, sistema de moralidade nem final além de um único com variantes mínimas; ficam ressalvados um afeto platônico infantil nunca correspondido em texto e o placar de comedimento, que é economia de conduta. |
| **C-12** | Não faça simulador de namoro nem medidor de afeto romântico; afinidade de personagem não jogável como reação social fica fora deste corte. |
| **C-14** | Não venda conteúdo adicional, passe de temporada nem serviço contínuo; expansão gratuita pós-lançamento fica fora deste corte. |
| **C-15** | Não fixe a duração da campanha em número de horas antes de existir cena jogável para medir, e não corte conteúdo por não caber num jogo curto: a campanha é **longa por desenho**, da ordem de meses de jogo e não de horas, e a duração só se mede quando houver cena jogável. Não deixe escopo de campanha aberto: ponha a cerca contra escopo infinito no orçamento próprio de conteúdo de cada uma das treze áreas e de cada dungeon, não na duração total, e aceite mecânica nova só se couber no orçamento da área onde ela vive. |
| **C-16** | Não faça tutorial em parede de texto nem abertura cinematográfica acima de noventa segundos. |

Fixe o piso de duração sem número, porque fixar "meses" em horas repetiria, na direção oposta, o erro de um número que ninguém mediu.

## GW-30

Aponte, na coluna de descrição técnica de todo item da tabela de pendências, a lei, o documento de design, o ADR ou o manual que o especifica; sem documento, diga no item que não há. Não mantenha, em documento do projeto fora da tabela de pendências, checklist com estado próprio, cópia da contagem, da soma ou do percentual dela, nem arquivo ou seção paralela de fila de trabalho: troque caixa de marcar fora da tabela por marcador simples, e cite o status de um item só nomeando o identificador dele. O checklist-gabarito de processo, reutilizável e sem estado por item, não é fila de trabalho e fica fora desta proibição.

## GW-31

Avise o Gus Dragon, sem ele precisar perguntar, de toda aprovação, rejeição ou mudança sobre ideia dele e de todo fechamento de item de alta prioridade. Não prometa instantaneidade além do que a sessão aberta permite. Responda na discussion 7, o catálogo de bugs dele, sempre com timestamp e uma das três classificações que ele fixou: Bug Consertado, Bug Funcional, Bug Possível.

## GW-32

**Data:** emenda de 20/11/2026. Verbatim do líder na primeira resposta: "Push para ramo de trabalho a qualquer hora, porque é ele que dispara o CI. Merge no main e tag só com a onda fechada e com o seu aval. A L-32 do projeto passa a dizer isso. O argumento contra é que o push cedo publica no repositório público trabalho ainda não revisado. O ramo de teste existe justamente para isso, e o portal de segredo roda em todo push." Confirmação, depois do argumento contra, por AskUserQuestion, verbatim do líder: "Confirmar o texto (Recomendado)".

Feche cada fatia com commit local citando o ID do item e tocando o status dele no mesmo commit. Faça push para o ramo de trabalho a qualquer hora, porque é ele que dispara o CI. Só considere aceito um push depois de a verificação remota dele sair verde, e trate a reprovação nela como bloqueio do que vem depois. Faça merge no main e tag só com a onda fechada e com aval explícito do líder no contexto. Confirme todo push conferindo o remoto.

## GW-33

**Data:** ordem do líder de 20/11/2026, verbatim: *"atomos devem ser apenas para o aplicativo do jogo, para os testes faça como for melhor"*, com o alcance confirmado por `AskUserQuestion`: *"Átomo só no código do jogo"*.

Aplique a regra de átomo só ao código do aplicativo do jogo: só as cinco camadas (`core/`, `content/`, `domain/`, `app/` e `present/`) seguem a GW-04 e esta lei. Organize testes, ferramentas, scripts de CI, documentos, commits, itens de tabela e assets como for melhor em cada caso, sem obrigação de atomizar.

No código do jogo, meça monolito por acoplamento sem fronteira, não por tamanho: nome próprio possível, mudança que exige tocar outras partes e consumidor forçado a levar tudo. Não fatie nos quatro casos em que agregar é certo: partes que só têm sentido juntas, ordem de leitura que é o próprio conteúdo, artefato de compilação e coesão exigida por contrato de terceiro verificável. Não deixe uma unidade acumular razões de mudar vindas de leis diferentes e não relacionadas: isso é monolito em formação.

Não feche a revisão de unidade de código do jogo criada ou crescida numa fatia sem as cinco perguntas do revisor respondidas e registradas: quais leis a obrigam a mudar; se a descrição dela precisa de duas ações ligadas; se o teste monta o mundo inteiro para exercitá-la; se a lista de inclusões mistura grupos que não conversam; e quem paga a próxima funcionalidade no histórico de mudanças. Conte silêncio como unidade não revisada, e leve ao líder a divergência entre implementador e revisor.

Não faça átomo menor que a menor unidade com significado: fatiar sem sentido e abstração especulativa são proibidos tanto quanto o monolito. Não ponha portão automático por número (linhas, métodos) no lugar das cinco perguntas: o teto de linhas do `CONTRACT.md` é recomendação, nunca portão. Como auditor, não aceite relatório de revisão sem conferir você mesmo: responda as cinco perguntas sobre o maior arquivo de cada camada do jogo. Aplique esta lei só ao que este projeto autora e possui, nunca a formato ou esquema de terceiro.

## GW-34

Leia primeiro na sessão toda mensagem do Gus Dragon, em qualquer dos cinco canais dele (issue, comentário em issue, discussion, comentário em discussion e arquivo de texto na caixa de entrada do bus), e confirme o recebimento na hora, interrompendo a fatia em andamento; deixe o conteúdo da resposta esperar a onda fechar. Mande sempre a resposta automática depois da decisão do líder. Não aprove ideia dele por agente sozinho, não prometa instantaneidade que a sessão não garante e adeque toda resposta a uma criança.

## GW-35

Rode todo teste dinâmico (suíte, integração, fuzzing, sanitizador, teste de mutação, benchmark, sonda de janela, teclado, mouse ou tela) sempre em container, nunca na sessão viva do líder; sem container disponível, o resultado é que não foi executado. Não apoie a proteção em desligar variável de ambiente de exibição: troque o `XDG_RUNTIME_DIR` do processo sob teste por diretório próprio, com permissão restrita a `chmod 700`, e prove a troca em vigor antes de interagir, sem assumi-la por ter subido o container. Use o compositor gráfico aninhado dentro do container como piso, nunca como alternativa a ele.

## GW-36

Não faça fatia de engenharia esperar lore ainda não escrita: puxe a lore que falta sob demanda, em micro-sessão pontual, nunca em ciclo completo de aprofundamento de lore. Não ponha esta lei em competição com a lei do canon contraditório (GW-13): canon que contradiz bloqueia, canon que ainda não existe não bloqueia.

## GW-37

Não edite, reescreva, corte, comprima nem acrescente fala, aparte, revide, cena ou traço de caracterização do Gus original já aprovado pelo líder linha a linha sem nova autorização explícita dele, nem quando a mudança parecer compressão honesta. Não confunda esta lei com a de homenagem a pessoa real (GW-16): aquela autoriza a homenagem existir; esta autoriza mudar o que já foi aprovado.

## GW-38

**Data:** decisão do líder, 12/09/2026, verbatim: "o jogador resolve, nao é pra ser automatico".

Faça todo enigma do jogo, em toda dungeon, câmara ou área secreta, ceder só ao jogador que segura o controle, nunca a ferramenta do jogo: não deixe personagem anunciar o padrão em fala ou aparte, marcação de interface (HUD, scan, contorno de destaque, prompt) apontar a resposta, nem passagem ceder por insistência ou tentativa exaustiva confortável. Aplique a regra a todo enigma do jogo, escrito antes ou depois desta lei, sem exceção por antiguidade, e repita no texto de cada documento de puzzle a frase-guarda: qualquer solução em que o jogo aponte a resposta contradiz esta decisão. Faça o jogo conferir só o acerto, nunca o vocabulário técnico por trás dele. Permita a ferramenta que mostra estado observável do mundo, e proíba a que faz o raciocínio no lugar do jogador.
