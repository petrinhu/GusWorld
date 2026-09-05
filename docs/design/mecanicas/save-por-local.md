# Save por local

> **Status:** decisão de design canonizada, resgatada de documento morto. **Não é decisão nova.**
>
> **Origem:** esta regra foi decidida pelo líder no projeto anterior e vivia só numa célula de tabela de pendências (`TODO_ARCHIVE.md`, item `SAVE-LOAD-UI`, e `TODO.md` antigo, item `FARADAY-DUNGEON-ITENS`), num backup somente leitura, sem documento próprio no corpus vivo. Este arquivo resgata a decisão e lhe dá endereço permanente, sem inventar nada além do que já estava decidido.
>
> **Data original da decisão:** política de save por local, 2026-07-08. Arco da carta Gaiola de Faraday entrelaçado a ela, canonizado em 2026-07-18.
>
> **Relação com `GODS_LAWS.md`:** este documento descreve uma **regra de jogo e de narrativa**, não uma implementação. A implementação técnica do envelope de save (formato binário selado, validador semântico, criptografia) é regida pela **L-25** e nasce à parte. O código de save em si é do zero, sob a **L-01**: o motor de save do projeto anterior não é base nem referência. A camada que desenha a UI de save só nasce quando o GlintFx existir (**L-06**, **L-27**). Nenhuma decisão nova é tomada por agente neste documento (**L-11**): onde falta decisão, este documento diz que falta, e de quem é.

---

## 1. A regra, por tipo de lugar

O jogo tem duas categorias de lugar, e a regra de save muda por completo entre elas.

### 1.1 Cidade

**Save livre.** Dentro da cidade (o hub central, ver `docs/design/mundo-topologia.md`), o jogador salva quando quiser, onde quiser, sem restrição de mecânica ou de narrativa.

### 1.2 Dungeon (estilo Zelda)

Nas dungeons, espaço feito à mão descrito em `docs/design/mundo-topologia.md` §4, a regra é restritiva. **Todas as 13 dungeons têm o campo que restringe o save**, mas a intensidade não varia por dungeon no sentido de balanceamento entre tipos: a distribuição final é **12 dungeons em intensidade total e 1 em intensidade fraca**, e a fraca é exclusiva da dungeon de abertura, **Dutos — aparato (abertura)**, a primeira que o jogador visita — ela existe por razão de onboarding, o primeiro contato do jogador com a própria restrição de save, não por variação de balanceamento entre as demais (decisão do líder, 30/08/2026, por `AskUserQuestion`, que substitui a distribuição de 21/08/2026):

- **Intensidade total:** salva-se só na porta, de entrada ou de saída da dungeon. Carregar um save reinicia o jogador na porta, nunca no meio da dungeon. Save manual e autosave ficam desligados enquanto o jogador está dentro, fora da porta.
- **Intensidade fraca:** o autosave fica desligado dentro da dungeon; o save manual fica disponível em **qualquer ponto dentro dela**; e carregar um save devolve o jogador à **entrada** da dungeon, nunca ao ponto onde salvou (decisão do líder, 30/08/2026, verbatim: "Sem auto-save. Save manual dentro. Carregar devolve para a entrada da dungeon.").

Isto é deliberado: a dungeon é o trecho de risco do jogo, e a ausência de save no meio dela é o que sustenta a tensão de exploração e combate no estilo das referências que o líder nomeou (Zelda de SNES, Chrono Trigger, Stardew Valley, já citadas em `GODS_LAWS.md` L-26 para a câmera, e a mesma família de referência vale aqui para o ritmo de risco).

## 2. As duas exceções

A supressão de save dentro da dungeon tem exatamente duas portas de saída, e cada uma tem um custo diferente para o jogador.

### 2.1 O Emissor do Tesla

Cada dungeon esconde um dispositivo PEM (pulso eletromagnético) que, uma vez descoberto e desativado pelo jogador, **reativa save e autosave dentro daquela dungeon**. O item-chave que dá acesso a esse dispositivo é o **Emissor do Tesla**, e o líder já fixou que ele é de **custo alto**.

O que isto custa ao jogador: um investimento caro (em recursos do jogo, ainda não quantificado, ver Seção 6) para comprar de volta a liberdade de salvar dentro de uma dungeon específica. É a via "eu pago para não correr risco".

### 2.2 A carta passiva Gaiola de Faraday

Se o jogador possui a carta passiva **Gaiola de Faraday**, save e autosave ficam disponíveis dentro de qualquer dungeon onde essa exceção se aplique, sem precisar descobrir o dispositivo PEM local.

O que isto custa ao jogador: nada em recursos, mas a carta em si tem uma história de obtenção e de perda que é o núcleo da Seção 4 deste documento, e o custo real dela é dramático, não mecânico.

**As duas exceções não se excluem.** Ter a carta e também descobrir o Emissor do Tesla numa mesma dungeon não é contraditório; a carta cobre o jogo inteiro, o Emissor é por dungeon.

## 3. O sinal diegético

O jogo não usa texto de tutorial para explicar quando o save está disponível (a **L-29**, corte **C-16**, proíbe tutorial em parede de texto e exige onboarding orgânico).

**O sinal diegético no lugar (luz, som, estado do ambiente) e o menu de save somam, e nenhum dos dois é dispensável** (decisão do líder, 05/09/2026, por `AskUserQuestion`). O ambiente é o sinal **primário**: é olhando o lugar, jogando, que o jogador aprende que entrou no campo do PEM, sem uma linha de diálogo ou um aviso explicando a regra. O menu de save **confirma** esse aprendizado, mostrando a opção de salvar disponível ou bloqueada no momento em que o jogador de fato abre o menu. É o jogador quem aprende, primeiro pelo ambiente e depois vendo o menu confirmar, que "dentro da dungeon existe uma restrição" e que existe algo a fazer sobre isso.

**Em aberto:** só a forma exata de implementação na tela e no ambiente (ícone, texto do item desabilitado, o desenho concreto do sinal ambiental — que luz, que som, que estado do lugar) é o que falta, e nasce com a camada `present/`, quando o GlintFx tiver janela, texto na tela e tradutor de marcação (**L-06**, **L-27**). Este documento já fixa que o veículo do sinal é o par ambiente-mais-menu (acima), não que falta decidir se existe algo além do menu.

## 4. Entrelaçamento com o arco do Dante

> ⚠️ **SPOILER DE ENREDO A PARTIR DAQUI.** Esta seção descreve a traição do Dante e a perda da carta Faraday no clímax do jogo. Isolada deliberadamente do restante do documento para que qualquer citação da regra mecânica (Seções 1 a 3) possa circular sem revelar o enredo.

A Gaiola de Faraday não é só uma carta passiva com efeito de jogo. É, por decisão do líder canonizada em 2026-07-18, a **primeira carta especial do jogador**, obtida cedo e quase às escuras, e por isso a de **maior apego emocional** de toda a coleção. O jogador confia nela o jogo inteiro.

No **clímax**, ela **se perde**: um vírus-arma fabricado sob medida pelas indústrias Sterling especificamente para neutralizar a Gaiola de Faraday é instalado pelo **Dante**, um companion do jogador com acesso logístico à carta. Isto não é uma infecção comum e acidental — as cartas especiais são imunes a vírus comum por desenho; esta é a exceção que confirma a regra, uma arma dedicada contra uma carta específica, entregue por uma traição específica.

O destino da carta é híbrido: ela **"queima"** (`is_burned_out`, no vocabulário do corpus antigo, não uma infecção curável comum) e **só a redenção do Dante a recupera** — ele precisa, por ação sua, remover o vírus que ele mesmo instalou e restaurar a carta.

**Por que isto pesa tanto, e por que é deliberado:** a carta-companheira do jogador desde o início morre na hora mais crítica do jogo, pelas mãos de quem o jogador tinha razão de confiar. O peso dramático não é acidente de sistema, é a escolha central do arco: perdoar e recuperar o Dante salva **duas coisas ao mesmo tempo**, o traidor e a carta mais querida do jogador.

**Onde isto toca a regra de save:** a carta Gaiola de Faraday é, ao mesmo tempo, uma peça de mecânica (Seção 2.2) e um marco de enredo com data de validade dentro da campanha. Qualquer sistema que implemente a exceção 2.2 tem de saber que ela **deixa de valer** em algum ponto do clímax e **pode voltar a valer** depois da redenção do Dante. Isto é regra de jogo que muda por evento de história, não uma flag estática.

## 5. O que isto exige do resto do sistema

Esta seção descreve **exigência**, não implementação. Quem decide a forma técnica é o líder; o formato de dado do mapa é atribuição do GlintFx, e o motor de save em si é regido pela L-25 e nasce no seu próprio trabalho.

1. **A área carregada precisa expor mais que geometria.** O código que representa "onde o jogador está" não pode ser só malha e colisão: ele precisa carregar um dado que diga se aquele lugar é cidade ou dungeon, e, sendo dungeon, se o PEM local está ativo, desativado, ou nunca existiu ali, **e qual a intensidade do campo ali, fraca ou total (Seção 1.2)**. Essa informação é regra de jogo (camada `domain/`, por L-17), consultada pela UI de save, não decidida por ela. A intensidade em si é dado do mapa, não do save (`TODO.md`, item `G8`) — precisa estar nesta lista exatamente por isso, para o item de mapa ter onde expô-la sem o save precisar de campo próprio.
2. **Save e autosave precisam ser suprimíveis por contexto.** O sistema de save não pode assumir "sempre disponível". Ele precisa de um ponto de decisão que responda, antes de gravar, se o contexto atual permite salvar — e essa pergunta depende do estado do jogador (tem a carta? qual o estado do PEM local? qual o ponto da campanha, antes ou depois da perda da carta?), não só da posição geométrica.
3. **A porta de entrada/saída da dungeon precisa ser um lugar nomeado e revisitável.** Carregar um save feito na porta tem de devolver o jogador exatamente a ela, o que exige que a porta seja um ponto de save canônico da dungeon, não uma coordenada arbitrária.
4. **O estado "carta perdida" precisa ser uma condição consultável, não um item apagado do inventário.** Se a Gaiola de Faraday simplesmente desaparecer do inventário no momento da perda, o sistema de save por local perde o sinal de que a exceção 2.2 deixou de valer. A perda da carta é evento de enredo com efeito mecânico durável, e o dado que representa isso precisa sobreviver ao evento em si.

## 6. O que continua em aberto

Nenhum destes pontos é decidido por este documento. Cada um pertence ao líder (**L-11**).

1. **Como o Emissor do Tesla é obtido, e quanto custa exatamente.** O corpus antigo fixa só que o custo é "alto"; não há número, moeda ou requisito de progressão definidos. Decisão do líder.
2. ~~Qual das 13 dungeons recebe qual intensidade (fraca ou total), e com que critério.~~ **Resolvido em 30/08/2026 pelo líder, por `AskUserQuestion`:** 12 dungeons em intensidade total, 1 em intensidade fraca — a de abertura, "Dutos — aparato (abertura)" — por decisão direta, não pelo critério de quatro regras apresentado em `docs/design/mecanicas/proposta-intensidade-campo-save.md` (que propunha 5 fracas e 8 totais). Ver Seção 1.2.
3. **O que acontece com um save feito na porta quando o jogador perde a carta no clímax.** Se existir um save gravado na porta de uma dungeon enquanto a Gaiola de Faraday ainda estava ativa (exceção 2.2 valendo) e o jogador depois carregar esse save já tendo perdido a carta no enredo principal, não há regra definida sobre qual estado prevalece, nem se isso é sequer alcançável dado o formato de save único e a ordem do enredo. Decisão do líder, com insumo técnico de quem desenhar o validador semântico da L-25.
4. **A forma exata de implementação do sinal, na tela e no ambiente, incluindo como comunicar fraca versus total.** Este documento já fixa que o sinal soma ambiente (primário) e menu (que confirma), decisão do líder de 05/09/2026 (Seção 3); o que falta é só a forma técnica concreta de cada um — ícone, texto do item desabilitado, e o desenho do sinal ambiental em si (que luz, que som, que estado do lugar) — e ela nasce com a camada `present/`, quando o GlintFx tiver janela, texto na tela e tradutor de marcação (L-06, L-27). A intensidade variável (Seção 1.2) acrescenta uma exigência a essa forma ainda não desenhada: o sinal precisa comunicar não só "pode salvar" ou "não pode salvar", mas também **qual das duas restrições está em vigor** ali, sem recorrer a tutorial em parede de texto (corte C-16 da L-29). Como a distribuição fechou em 12 dungeons total e 1 fraca (Seção 1.2), essa exigência se restringe na prática à dungeon de abertura, já que as outras 12 são sempre total.
5. **A UI de save em si.** Fora do escopo deste documento: layout de tela, slots, autosave por gatilho de evento e os demais detalhes de interface pertencem à L-27 e nascem só quando o GlintFx traduzir marcação.
