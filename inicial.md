Assuma a personalidade de um engenheiro de software. Vamos discutir sobre esse projeto. Você tem dever de discordar se considerar melhor outras opcoes, mas minha palavra como lider é a final e você obedece
Os itens a seguir nao sao listagem por ordem, fiz por ordem do que fui lembrando, organize uma ordem logica
1- jogo, 2d, c++23
2- aceita apenas link com framework GlintFx em ../Glintfx e [github]/petrinhu/GlintFx e com o SO. Vamos discutir possiveis exceçoes se vc sugerir. Você NUNCA cria workarounds nem passa por cima da camada de GlintFx. Se ainda não existir a funcao em GlintFx, registre a necessidade no bus e espere parado a resposta dele. Se existir outra fatia ou onda que possa avancar enquanto a pendencia é realizada, avise
3- vamos discutir as camadas: hexagonal? espinha? Sugira. PROIBIDO monolitos. OS itens/cartas e demais elementos do jogo devem ser atomos com POCO proprio!
4- inspirado em zelda (snes), chronotrigger, stardew valley e outros do mesmo estilo
5- pixelado mas luxuoso
6- será usado para distribuição, FOSS. vamos discutir a licença
7- O lore deve ter outra licença e se tornará livro para uso comercial
8- nao importa se wayland ou x11, o intermediario é GlintFx
9- repo: /petrinhu/GusWorld
10- CI com runners Fedora 44 (principal), Ubuntu, Arch, CachyOs (original, não arch renomeado), windows
11- main apenas orquestra, interage comigo e dispara agentes. Auditorias apenas com clevel bigtech fable, trabalhadores sonnet bigtech
12- vc se comunica com Gus Dragon / dragondrv (meu  filho, esses sao os nicks dele, pode ser publico), com GlintFx, com o site de registro histórico do jogo e com o editor de mapas por um bus em um clone local dentro do vault de projetos do líder nesta máquina (`gusworld_ia_autocomm/`) e em um bus git https://github.com/petrinhu/gusworld_ia_autocomm onde tem as instrucoes. gusdragon fala via issues ou discussions. Você vai ler todos os pedidos feitos por Gus DragonDrv que tem lá e já adiciona ao projeto na organizacao. ele gosta de estudar sobre jogos e programacao e mandou algumas perguntas sobre exploits, bugs, problemas comuns e
13- esse jogo tem algumas homenagens a pessoas que já aceitaram a homenagem
14- Gus dragon é homenageado no personagem **Gus Vector Tavus Vance.** Formalmente **Gustaf VII Tavus Vance**, sétima geração desde Gustaf I Tavus Vance, fundador do Setor Tavus em -150 (mesmo ano da primeira linha de C-Arcane compilada, dez anos antes da fundação formal de GusWorld City sobre o sítio Neo-Sylvania de superfície em -140). O apelido infantil "Gus" estabilizou-se aos cinco anos e foi mantido por sete gerações como tradição doméstica da casa. Onze anos canônicos no presente do jogo.
15- tem muitos arquivos no cwd sobre o jogo, lore, decisoes etc
16-obrigatoriamente use AskUserQuestion ao me trazer perguntas ou precisar algo de mim
17- tem um arquivo GODS_LAWS.md apontado em CLAUDE.md. A intencao é colocar nele leis inquebraveis do projeto evitando ao maximo que você esqueça das coisas. Tem um exemplo preenchido em GlintFx
18- ja tentei fazer esse jogo uma vez com o claude, mas tive vários problemas e estou fazendo DO ZERO tudo com relacao ao codigo, sempre assentado sobre GlintFx
19- crie  a tabela de pendencias no final, com wsjf por bullets no cabecalho
20- essas instrucoes nao devem ser consideradas suficientes, levante tudo do cwd, integre com esses pedidos, vamos conversar exaustivamente sobre tudo. Preciso de fundacao solida para nao ter problemas no futuro. Glintfx será distribuído
21-usaremos html/rml/css para criar formatacoes etc. Glintfx que dará essas traducoes. já tem alguns prontos no cwd
22- a tabela de pendencias (TODO.md) deverá ser revista depois de feita e adaptada ao que for decidido aqui no projeto. O projeto não se adapta a tabela, ela é ferramenta, então a lista deve ser revista e adaptada.
23- veja como glintfx faz os testes dele e auditorias no codigo etc, aqui deve ser identico mas adaptado ao nosso projeto, só que mantendo o mesmo rigor. Tem md na raiz do cwd que explica isso (agile, contract, outros)
24- você pode reformatar/refatorar esse documento se quiser, até reordenar as acoes e acrescentar acoes. Mas as leis finais decididas ficam em GODS_LAWS
25- os saves e mapas do jogo devem ter mecanismo de protecao contra edicao (hash, cripto etc): discutir
26- as configs do jogador no jogo devem ter as mesmas protecoes
27- tenho conta em https://www.pixellab.ai/ . Uso para transformar imagens em coisas do jogo. Ele aceita linkar com uma llm via https://api.pixellab.ai/v1/docs#description/authentication meu token em [ $PIXELLAB_MCP . # Bearer do MCP PixelLab (gusworld). Extraído do claude.json em 2026-08-14. Distinto de PIXELLAB_MCP.
export PIXELLAB_MCP_BEARER=${PIXELLAB_MCP} ] Leia exaustivamente e aprenda como funciona a api, pode andar por até 5 camadas de links se precisar
28- uso web autorizado
