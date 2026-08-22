# Lente: grafo de dependência técnica e sinalização de fundação (GusWorld)

Autor: software-architect (lente 1 de 4 para o Cosmo consolidar). Não decide onda nem WSJF, não escreve a tabela.

Convenção: FATO = citação de lei ou de trecho do arquivo bruto. INFERÊNCIA = dedução minha, marcada como tal.

## Item a item

`A1 | pré-requisitos: nenhum | flags: ONE-WAY | FATO (L-15): resources/livros e resources/glb ficam fora do git; depois do primeiro push corrigir isto exige reescrever histórico e forçar o remoto público. Não é FUNDACAO no sentido de "propaga erro para outros módulos", mas é porta de mão única pela natureza do git.`

`A2 | pré-requisitos: A3 (parcial) | flags: nenhuma | INFERÊNCIA: a parte de LFS não depende de nada; a marcação git-crypt para docs/_secret/** só protege de verdade se o filtro já estiver registrado (A3) antes de qualquer arquivo secreto ser staged, senão o primeiro add desses arquivos vai em texto puro no objeto git (ainda que depois "corrigido", o blob antigo continua no histórico se já tiver sido commitado).`

`A3 | pré-requisitos: nenhum | flags: ONE-WAY | INFERÊNCIA: configurar git-crypt e exportar a chave para fora da árvore é decisão cujo erro (perda da chave, chave dentro da árvore) é irreversível: conteúdo cifrado sem a chave é dado perdido para sempre. FUNDACAO também, no sentido de que todo docs/_secret futuro depende deste setup estar correto desde o início.`

`A4 | pré-requisitos: A2, A3 | flags: nenhuma | INFERÊNCIA: a frase errada só pode ser corrigida para descrever o mecanismo real depois que o mecanismo real (git-crypt configurado e .gitattributes marcando o filtro) existir; corrigir antes produziria uma segunda frase igualmente falsa.`

`A5 | pré-requisitos: nenhum | flags: ONE-WAY | FATO (L-08): texto verbatim e imutável da AGPL-3.0; a escolha da licença do código, uma vez combinada com o GlintFx (também AGPL) e distribuída, não é revertida sem consentimento de todo contribuidor futuro (relicenciamento).`

`A6 | pré-requisitos: A5 | flags: nenhuma | INFERÊNCIA: o diretório LICENSES/ precisa do texto integral da AGPL e do texto da reserva de asset já definidos para copiá-los verbatim.`

`A7 | pré-requisitos: A6 | flags: nenhuma | INFERÊNCIA: o REUSE.toml codifica regras por diretório (código AGPL, asset reservado, fronteira do catálogo da L-25); precisa dos identificadores de licença já resolvidos em A6 para referenciá-los.`

`A8 | pré-requisitos: A5, A9 | flags: nenhuma | INFERÊNCIA: o README precisa do texto de licença (A5) e do link para a nota offline (A9, já redigida pelo Cláudio) para montar a seção.`

`A9 | pré-requisitos: nenhum | flags: nenhuma | FATO: texto já redigido pelo Cláudio (CLO); é só gravar, sem dependência técnica de outro item.`

`A10 | pré-requisitos: nenhum | flags: nenhuma | FATO: hoje só lista o GlintFx (AGPL-3.0-or-later, já é lei conhecida); não depende de outro item desta lista.`

`A11 | pré-requisitos: A5, A6 | flags: FUNDACAO | FATO (L-08): SPDX desde o primeiro arquivo; precisa dos identificadores de licença corretos (A5/A6) para o cabeçalho não citar identificador errado. FUNDACAO porque, a partir do momento em que C-D-E-F começam a criar arquivo, cada um herda esta convenção; corrigir em massa depois de centenas de arquivos escritos é caro, embora não impossível.`

`A12 | pré-requisitos: A1 a A11, e ver nota sobre grupo B abaixo | flags: ONE-WAY | FATO (cabeçalho do grupo A: "tudo isto ANTES do primeiro commit", L-15, L-08): este é o portão de mão única mais crítico da lista inteira. O remoto público já existe e está vazio; uma vez pushado, qualquer erro de licença, vazamento de caminho ou segredo exige reescrita de histórico e force-push. INFERÊNCIA importante: o cabeçalho do grupo B diz "nada disto pode ir público como está" com o repositório já público; se o primeiro commit incluir de uma vez o corpus inteiro (docs/, resources/), então B1-B6 também são pré-requisito técnico de A12, não só de "ir público" em abstrato. Se o plano for um primeiro commit mínimo (só licença/gitignore/README) seguido de commits posteriores trazendo o corpus, B1-B6 gateiam esses commits seguintes, não A12 em si. Este ponto precisa de confirmação do líder sobre o que entra no primeiro commit, não é meu para decidir.`

`B1 | pré-requisitos: G1 (aprovação em bloco do líder) | flags: nenhuma | FATO (texto do item): "Lista completa vai ao líder para aprovação em bloco antes de qualquer edição." Nenhuma edição pode começar antes de G1.`

`B2 | pré-requisitos: nenhum | flags: nenhuma | FATO (L-16): homenagem só com aceite, nome de menor nunca versionado; é remoção de dado real, sem dependência técnica de outro item.`

`B3 | pré-requisitos: nenhum | flags: nenhuma | INFERÊNCIA: substituição textual mecânica (caminho absoluto por genérico), sem dependência de outro item.`

`B4 | pré-requisitos: nenhum entre si | flags: nenhuma | FATO (L-13): os itens de contradição de plataforma citados já têm a decisão de canon resolvida (L-02, L-26); os outros cinco pontos (vocabulário .NET, bloco histórico, títulos de spec, linha órfã, citações órfãs) são limpeza textual independente. Nenhum bloqueia o outro.`

`B5 | pré-requisitos: nenhum para a decisão em si | flags: nenhuma (é decisão, não trabalho) | FATO/INFERÊNCIA: gateia D9 (diálogo como dado) e E8 (gerador de conteúdo), porque a forma de serialização do diálogo depende de saber se .dlg.txt e traduções em Markdown são fonte de build (permitido em texto dentro do repo, por L-25) ou formato de runtime (proibido em texto, por L-18). L-25 já resolveu essa mesma tensão para o catálogo geral ("texto estruturado dentro do repositório, lido apenas pelo gerador em tempo de build"); B5 provavelmente confirma o mesmo padrão para diálogo, mas isso é inferência minha, não decisão já tomada.`

`B6 | pré-requisitos: nenhum | flags: nenhuma | FATO: "nenhum humano olhou" ainda; é conferência independente, sem dependência de código. Gateia, por prudência, a inclusão das 1266 imagens no commit que as tornar públicas (mesma lógica de A12/B).`

`C1 | pré-requisitos: nenhum técnico direto; sequenciamento por convenção do próprio documento (A antes, C "a primeira fatia" depois) | flags: FUNDACAO | FATO (L-17): as cinco camadas com dependência só para baixo são a espinha; todo D, E, F vive dentro delas. Erro na fronteira das camadas custa reestruturar diretório e alvo de build de tudo que já foi escrito em cima.`

`C2 | pré-requisitos: C1 | flags: nenhuma | INFERÊNCIA: o gate de CI que reprova #include <glintfx/ fora de present/ precisa que o caminho present/ exista como convenção de diretório (C1) para ter o que checar; roda dentro do pipeline definido em C4.`

`C3 | pré-requisitos: C1 | flags: FUNDACAO | FATO (L-19): "nenhuma linha de código de comportamento entra sem um teste que falhava antes"; não existe framework de terceiro (proibido), então o harness próprio é pré-requisito literal de D1 até D14 inteiros. Erro aqui (harness que não reporta falha corretamente) contamina toda a suíte construída em cima.`

`C4 | pré-requisitos: C1 | flags: nenhuma | FATO (L-09, L-20): cinco entradas distintas na matriz; "divergência de plataforma aparece no dia em que nasce". Não é ONE-WAY (configuração de CI é refazível), mas atrasar custa dívida acumulada de portabilidade.`

`C5 | pré-requisitos: C1, C4 | flags: nenhuma | FATO (L-19): os quatro portões rodam dentro do pipeline que C4 define; Werror e análise estática não precisam de C3, mas ASan/UBSan fazem mais sentido com binários de teste (C3) rodando.`

`C6 | pré-requisitos: nenhum | flags: nenhuma | INFERÊNCIA: configuração de formatação é arquivo isolado, sem dependência de build ou CI.`

`C7 | pré-requisitos: C2, C4, C5, C6 | flags: nenhuma | FATO (L-19): "script local que espelha o CI" só pode espelhar o que já foi definido nos gates anteriores.`

`C8 | pré-requisitos: C3 | flags: nenhuma | INFERÊNCIA: o hook de TDD do projeto precisa saber contra qual harness de teste (C3) verificar vermelho/verde para funcionar.`

`D1 | pré-requisitos: C1, C3 | flags: FUNDACAO | INFERÊNCIA: tipos básicos, identificadores, Result/Error são a base de todo POCO de D3 em diante e de content/. É o item cujo erro mais se propaga: qualquer troca de forma do tipo de erro ou id depois de D3-D11 escritos por cima é reescrever assinatura em todo lugar.`

`D2 | pré-requisitos: D1 | flags: FUNDACAO | FATO (L-17): "o sorteio entra como parte do estado (semente mais contador), nunca como global." Usado por D5 (fórmula de dano), D13 (replay), D14 (auto-resolve), F2 (carta urandom). Se nascer global por engano, todo consumidor que já ligou nele precisa ser reescrito para virar estado, e o replay (D13) fica inválido retroativamente.`

`D3 | pré-requisitos: D1 | flags: nenhuma | INFERÊNCIA: este item mistura duas camadas do L-17 (catálogo é content/, instância e estado físico são domain/); a definição do catálogo (content/) precisa existir antes da instância (domain/) poder referenciá-la por id. Não é erro do brief, é uma nota de arquitetura para quem for implementar separar os dois módulos mesmo estando no mesmo item de tabela.`

`D4 | pré-requisitos: D3 | flags: FUNDACAO | FATO (canon citado): invariantes anti-exploit (instância única com id, mão como lista de ids, descarte só num sentido, venda atômica e idempotente, especial protegida) precisam da carta/instância (D3) já existir. Erro aqui é exatamente a classe de bug de exploit que é cara de corrigir depois que save (D11) e economia (D7) já assumiram o invariante errado.`

`D5 | pré-requisitos: D2, D3, D4 | flags: FUNDACAO | INFERÊNCIA: máquina de estados de combate depende do sorteio contado (D2, para a fórmula de dano) e das cartas em jogo (D3, D4). É central: D6, D13, D14, F3 e F4 dependem dela diretamente.`

`D6 | pré-requisitos: D5 | flags: nenhuma | INFERÊNCIA: efeitos de status se conectam à máquina de estados de combate; sem D5 não há onde o efeito de status atuar.`

`D7 | pré-requisitos: D1, D3, D4, D5 | flags: nenhuma | INFERÊNCIA: economia (fontes e sumidouros) toca venda de carta (D4, "venda atômica") e recompensa de combate (D5); a régua do comedimento como propriedade executável precisa desses números existirem primeiro.`

`D8 | pré-requisitos: D1, D3, D4 | flags: nenhuma | INFERÊNCIA: progressão de conhecimento e mestria por uso rastreia uso de carta/deck (D3, D4).`

`D9 | pré-requisitos: D1; decisão B5 pendente | flags: nenhuma (bloqueio por decisão, não por GlintFx) | INFERÊNCIA: diálogo como dado só pode fixar sua forma de armazenamento depois que B5 resolver a tensão .dlg.txt/Markdown vs L-18.`

`D10 | pré-requisitos: D1 | flags: nenhuma | FATO (L-26, já resolvida): grade quadrada, quatro direções cardeais, colisão por célula. A lei já fechou a decisão de perspectiva; este item é só a implementação. Gateia F1 (marcação de bloco atravessável é usada pela carta glitch).`

`D11 | pré-requisitos: D3, D4, D5, D6, D7, D8, D9, D10 | flags: FUNDACAO | INFERÊNCIA: o modelo de save precisa da forma final de cada sistema de domínio que contribui estado; construir o save antes de D3-D10 estabilizarem obriga a re-serializar tudo a cada mudança de forma. É a base direta de E2.`

`D12 | pré-requisitos: aplica-se DURANTE a construção de D3 a D11, não depois deles | flags: FUNDACAO | FATO (L-17): "Contrato comando/evento aplicado a cada sistema acima." Isto não é um módulo sequencial ao final; é uma propriedade que cada sistema de D3 a D11 já nasce tendo. Se for tratado como fase separada no fim, o custo é refatorar cada sistema de regra para expor aplica(estado, comando) depois de já ter sido escrito de outro jeito. Aviso direto ao Cosmo: não colocar D12 como onda isolada pós-D11; ele é critério de aceite de cada item D3-D11.`

`D13 | pré-requisitos: D2, D5, D12 | flags: nenhuma | FATO (regra de teste do brief): teste de replay é downstream do que ele cobre (RNG contado, máquina de combate, contrato comando/evento). Nunca antes deles. Ambiguidade minha: "byte a byte" pode significar comparação estrutural do estado em memória ou comparação de bytes serializados (o que dependeria de E2); não dá para saber pelo texto, sinalizo a dúvida sem decidir.`

`D14 | pré-requisitos: D2, D5, D7 | flags: nenhuma | INFERÊNCIA: auto-resolve para balanceamento em massa precisa de combate (D5), RNG contado (D2) e números de economia (D7) para produzir métrica útil de balanceamento.`

`E1 | pré-requisitos: nenhum técnico rígido para a forma dos campos; BLOQUEADO-GLINTFX para o selo criptográfico | flags: FUNDACAO, ONE-WAY, BLOQUEADO-GLINTFX (parcial) | FATO (L-25): envelope binário (magia, versão, tipo, nonce, dado, selo) é a espinha comum a save, config, mapa e pacote de conteúdo. FATO (L-25): "Primitivas criptográficas vêm do GlintFx"; hoje o GlintFx tem um header, sem essas primitivas. Por L-05 (proibido dublê), não se pode escrever um selo falso como placeholder. Logo o campo estrutural (magia/versão/tipo/nonce/dado) é redigível hoje, mas o item como um todo, com selo real, é IMPOSSÍVEL HOJE. ONE-WAY porque, uma vez que existirem saves de jogador no formato, mudar o envelope quebra compatibilidade (a própria L-23 reserva o componente A da versão para isso).`

`E2 | pré-requisitos: D3 a D11, E1 | flags: FUNDACAO | INFERÊNCIA: serialização binária versionada dos POCOs precisa que os POCOs (D3-D11) já tenham forma estável e do formato de envelope (E1) para embutir os dados. O campo de versão mitiga o risco de ONE-WAY total, mas migração ainda é cara.`

`E3 | pré-requisitos: D3 a D10, E2 | flags: nenhuma | FATO (L-25): "validador semântico... é o único nível que sobrevive à chave vazada." Não depende de criptografia, só das regras de domínio (para saber o que é "alcançável") e do estado já deserializado (E2).`

`E4 | pré-requisitos: E1, E2 | flags: nenhuma | INFERÊNCIA: gravação atômica e cadeia de backups escrevem o que o envelope (E1) e a serialização (E2) produzem; não depende do selo criptográfico em si.`

`E5 | pré-requisitos: E1 | flags: nenhuma | FATO (L-25): "configuração selada nunca pode impedir o jogo de abrir." É comportamento de fallback do caminho de leitura de E1; a lógica de fallback pode ser desenhada mesmo com o selo real ainda pendente (o caminho de "selo inválido" é testável com qualquer seleção que sinalize inválido).`

`E6 | pré-requisitos: E1 | flags: BLOQUEADO-GLINTFX (parcial) | FATO (L-25): "nasce junto com o formato, no mesmo trabalho, não depois." Não é sequencial-depois de E1, é a mesma fatia. Mas para inspecionar e re-selar de verdade, precisa do selo real, então herda o mesmo bloqueio parcial de E1.`

`E7 | pré-requisitos: nenhum | flags: nenhuma | FATO (L-07, L-25): "o pedido ao bus é registrado quando a fatia do envelope começar." Não é downstream de E1 terminar; é a ação que dispara no início da fatia de E1, para dar ao GlintFx o máximo de tempo de resposta antes que E1 precise do selo de verdade.`

`E8 | pré-requisitos: G2 (decisão pendente), D3, E1, E2 | flags: BLOQUEADO-GLINTFX (parcial) | FATO (G2, texto do item): a ferramenta/formato do gerador está "deixado em aberto de propósito"; sem essa decisão o item não pode começar. FATO (L-25): o artefato "tabelas compiladas" (número e regra) não usa criptografia e é redigível hoje; o artefato "pacote binário selado" (texto de sabor) herda o bloqueio de E1/E6 para o selo real.`

`E9 | pré-requisitos: E1 a E6 (fase 1 completa), D12, D13 | flags: ONE-WAY, BLOQUEADO-GLINTFX | FATO (L-25): "Fase 2", explicitamente sequencial depois da fase 1. Depende do contrato comando/evento (D12) e da confiabilidade do replay (D13) para o registro de comandos encadeado ser verificável por re-execução. Mesma família de risco ONE-WAY que E1 (save de jogador em formato específico).`

`E10 | pré-requisitos: E9 | flags: BLOQUEADO-GLINTFX | FATO (L-25): "Fase 3, opcional... Nunca requisito para jogar." Âncora TPM depende de primitivas criptográficas do GlintFx (inexistentes hoje) e da fase 2 (E9) já existir para ter o que ancorar.`

`E11 | pré-requisitos: E1 | flags: BLOQUEADO-GLINTFX | INFERÊNCIA: amarra de máquina só no slot Hardcore provavelmente usa hash/derivação de chave (primitiva do GlintFx) para identificar a máquina sem armazenar dado bruto; herda o bloqueio de E1. Gateado também pela decisão G3 (modos de morte / Hardcore).`

`F1 | pré-requisitos: D3, D10; decisão G4 pendente (dois pontos em aberto) | flags: BLOQUEADO-GLINTFX (parcial, só a manifestação visual) | FATO (texto do item): usa a marcação de bloco atravessável de D10 e o tipo de carta/bateria/carga de D3. A regra de travessia em si é regra de jogo pura (permitida por L-05); só o desenho da travessia na tela depende de present/, que não existe.`

`F2 | pré-requisitos: D2, D3; D8 para o prêmio da RunaDex | flags: nenhuma | FATO: D3 já lista "origem ROM/EPROM/pirata" como campo do estado físico da carta, então o dado que F2 precisa já está previsto ali. A chance 1 em 3 usa o sorteio contado (D2).`

`F3 | pré-requisitos: D5, D6 | flags: BLOQUEADO-GLINTFX (parcial, só o VFX de explosão) | FATO (texto do item): "a ficha técnica existe no design; falta o efeito de estourar na batalha", ou seja, a regra de dano/efeito depende de combate (D5) e de status/evento (D6); a explosão visual depende de present/.`

`F4 | pré-requisitos: D5, D10 | flags: BLOQUEADO-GLINTFX (parcial, só o relógio visível na tela) | INFERÊNCIA: o relógio como contador de estado (tick) é regra pura de domínio, ligado a combate (D5, "inclusive durante as batalhas") e a rota (D10, mapa); a exibição do relógio ao jogador precisa de present/. Note-se também que este item pressupõe uma camada app/ de fluxo de cena (L-17) que não tem ID próprio nesta lista.`

`G1 | pré-requisitos: lista completa de reparo compilada (parte do trabalho de B1, sem ID próprio) | flags: nenhuma | FATO (texto de B1): a aprovação em bloco precisa da lista pronta para ser aprovada.`

`G2 | pré-requisitos: nenhum | flags: nenhuma | FATO: "deixado em aberto de propósito pelo CTO e pelo CISO." Gateia E8 diretamente.`

`G3 | pré-requisitos: nenhum | flags: nenhuma | INFERÊNCIA: gateia a parte de "morte" de D5 (combate) e E11 (Hardcore, "a morte apaga o save sem recuperação" por L-25), porque referencia infraestrutura hoje inexistente.`

`G4 | pré-requisitos: nenhum | flags: nenhuma | FATO: gateia F1 diretamente ("carta glitch em batalha" é um dos dois pontos citados em F1 e G4). A outra ideia ("bateria acabando dentro da parede") não tem item F correspondente nesta lista, ver sugestão ao final.`

`H1 | pré-requisitos: corpus público estável (pós B1-B6, pós A12) | flags: nenhuma | INFERÊNCIA: a Wiki deriva de docs/, então precisa que docs/ já esteja limpo e público. O item não cita "tag de versão" como H2 cita; friso que é inferência minha, não fato do texto do item.`

`H2 | pré-requisitos: tag de versão (portanto, praticamente todo o resto da tabela até um release taggeado) | flags: nenhuma | FATO (texto do item): "Pré-requisito: tag de versão." É o item mais a jusante de toda a lista.`

## Grupo I (TST-*/AUD-*)

Não recebi IDs específicos de TST-*/AUD-* no arquivo bruto (o item diz que "a skill injeta os IDs canônicos" a partir do catálogo, podados para este stack). Não posso dar linha por item. Regra geral que deve valer quando esses IDs forem gerados, e que já está no meu mandato de atenção:

- Teste unitário não é item de tabela: nasce junto com o código pelo TDD estrito da L-19 (vermelho, verde, refatorar), então não tem ID próprio nem entra em onda separada.
- Teste não unitário (ASan/UBSan, replay D13, auto-resolve D14, matriz de CI C4, análise estática C5) é sempre downstream do código que ele cobre.
- Auditoria (dossiê `AUDITORIAS.md`, portão de release antes da 1.0 por L-19) é downstream de código mais teste, e é o item mais a jusante depois de H2 em termos de "tudo precisa existir primeiro".
- Nenhum destes deve aparecer numa onda antes do código que verificam.

## Fundação, em ordem de criticidade

1. **C1** (esqueleto de cinco camadas) — tudo (D, E, F) vive dentro dele; erro na fronteira custa reestruturar diretório e alvo de build inteiros.
2. **C3** (harness de teste próprio) — sem ele não existe TDD possível (L-19), e D1-D14 inteiros dependem dele.
3. **D1** (tipos básicos, id, Result/Error) — base de todo POCO seguinte; troca de forma aqui reabre D3 em diante.
4. **D2** (sorteio determinístico contado) — se nascer global por engano, todo consumidor (D5, D13, D14, F2) precisa ser reescrito, e o replay fica inválido retroativamente.
5. **D5** (máquina de estados de combate) — dependido diretamente por D6, D13, D14, F3, F4.
6. **D12** (contrato comando/evento) — não é fase separada, é propriedade que D3-D11 já nascem tendo; tratá-lo como fase à parte no fim custa refatorar cada sistema já escrito.
7. **D11** (modelo de save) — agrega a forma de D3-D10; mudar tarde obriga re-serializar tudo.
8. **E1** (envelope binário) — espinha comum a save, config, mapa e catálogo; e é também ONE-WAY (ver abaixo).
9. **A3** (git-crypt configurado) e **A1** (.gitignore dos diretórios pesados) — erro aqui é vazamento ou reescrita de histórico do repositório público.
10. **A11** (cabeçalhos SPDX desde o primeiro arquivo) — retrofitting em escala é caro, ainda que não impossível.

## Portas de mão única (ONE-WAY) e custo de errar

- **A12 (primeiro commit/push)**: a maior porta de mão única da lista. O remoto já é público; qualquer conteúdo indevido (segredo, caminho de máquina, licença errada, imagem vazando tela pessoal) exige reescrita de histórico e force-push, com o risco adicional de o próprio commit de "limpeza" expor o problema no diff.
- **E1 (envelope binário) e por extensão E9 (save híbrido fase 2)**: uma vez que exista um save de jogador no formato, mudar magia/versão/tipo quebra compatibilidade; é exatamente o que a L-23 reserva ao componente A da versão ("quebra de compatibilidade sentida pelo jogador, save antigo incluído"). Custo de errar: forçar todo jogador com save existente a perder progresso, ou construir migração cara.
- **A5/A8 (licença AGPL-3.0 do código)**: uma vez distribuído o binário combinado com GlintFx sob AGPL, relicenciar depois exige consentimento de todo contribuidor futuro. Custo de errar: impossível de fato reverter num projeto com mais de um contribuidor.
- **A3 (chave do git-crypt)**: perder a chave depois de cifrar e commitar é perda permanente do conteúdo de docs/_secret/**. Custo de errar: dado irrecuperável, não apenas caro.
- **D10/L-26 (perspectiva 3/4 top-down, grade quadrada, direções cardeais)**: já decidida por lei, não é decisão em aberto nesta lista, mas registro o custo citado na própria L-26 para contexto: qualquer diagonal, rotação de câmera ou isométrico futuro custa refazer a locomoção dos sete personagens já com sprite pronto.

## Ciclos

Não encontrei ciclo verdadeiro (dois itens dependendo um do outro) entre os IDs da lista bruta. Um ponto merece registro por parecer cíclico à primeira vista, mas não é:

- **E1 depende de primitivas do GlintFx, que dependem de E7 ter sido registrado no bus, que por lei (L-07) deve disparar "quando a fatia do envelope começar"**, ou seja, no início do próprio trabalho de E1. Isto é uma dependência sequencial externa (E7 dispara → GlintFx responde em prazo desconhecido → E1 completa o selo), não um ciclo. Não proponho corte porque o próprio protocolo do bus (L-07) já resolve a ordem: registrar cedo e seguir com a parte não bloqueada de E1 (o layout estrutural) enquanto se espera.

## Itens impossíveis hoje (não colocar em onda inicial)

- **E1** com selo criptográfico real, **E6** (ferramenta de re-selo real), **E9** (fase 2), **E10** (âncora TPM), **E11** (amarra de máquina): todos dependem de primitivas criptográficas que o GlintFx ainda não tem (hoje: um header, nada mais). Por L-05, não se pode fingir essas primitivas.
- **E8**, na parte do "pacote binário selado" (texto de sabor): mesmo bloqueio de E1/E6. A parte de "tabelas compiladas" (número e regra) não é bloqueada.
- **F1, F3, F4**, na parte de manifestação visual (travessia de bloco na tela, explosão de VFX, relógio visível ao jogador): bloqueadas porque dependem de present/, que só nasce com o GlintFx (L-06, L-27). A regra pura por trás de cada uma (F1: travessia como dado; F3: efeito de dano/status; F4: contador de tick) não é bloqueada.
- De forma geral, **qualquer coisa que exigiria a camada present/** (janela, sprite na tela, entrada, texto na tela, marcação de interface) é impossível hoje, por L-06 e L-27; nenhum item desta lista tem ID dedicado só a present/, mas F1/F3/F4/E-crypto tocam a fronteira dela.

## Sugestões fora da lente (não são item novo na tabela, são gap identificado)

- Não existe item com ID próprio para o módulo **content/** de catálogo separado da instância de domínio; D3, como está escrito, mistura as duas camadas do L-17 (catálogo é content/, instância é domain/). Sinalizo para quem for implementar D3 separar os dois módulos fisicamente.
- Não existe item com ID próprio para a camada **app/** (fluxo de cena, avanço de turno, per L-17), embora F4 dependa dela implicitamente.
- A segunda ideia do Gus citada em G4 ("bateria acabando dentro da parede") não tem item F correspondente nesta lista; só a carta glitch (F1) está itemizada.
- Os IDs `TST-*`/`AUD-*` não vieram no arquivo bruto; esta lente não pôde analisá-los individualmente, só registrar a regra geral de ordem (seção "Grupo I" acima).
