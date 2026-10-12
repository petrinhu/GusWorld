# Cronovisor e a placa Lemon-Gama

> **Status:** FECHADO em 11/10/2026, com as decisões do líder aplicadas: o Cronovisor é a metade que vê do rádio e é **opcional**; a fabricação é do Joaquim Bartolomeu, com a cadeia de encomenda que segue; a visita acontece **depois** do rádio montado; a interface é uma placa de Lemon-Gama, que pertence a uma família; e o espelhamento da imagem é **defeito do aparelho**. Seis pontas continuam abertas e estão em §12. Documento novo, público (a partição está em §11).
>
> **Cross-ref:** `docs/design/mecanicas/radio-padre-landell.md` (peça irmã, mesma regra de acesso e mesma trava), `docs/design/technomagik.md` (seção Tavus-Eco, cena de encontro), `docs/design/mundo-topologia.md` §2 (grafo de 13 áreas e dificuldade), `CHARS.md` (Padrinho Tiago, linha 84; Joaquim Bartolomeu, linha 88; Gargi Vance e Pyotor na §4), `PLACES.md` (Edifício Vance, linha 83; Subsolo do Edifício Vance, linha 84; Zona do Silêncio), `GODS_LAWS.md` L-38 (o jogador resolve o enigma), L-29 (cortes C-02, C-04 e C-15), `docs/narrative/comic-reliefs/easter-eggs-educacionais.md` (EE-25).

---

## 1. O que ele é, e por que existe

O Cronovisor é a metade que vê do rádio do padre Landell. O rádio dá ao Gus acesso ao eco compilado de um mestre do Codex (o Tavus-Eco, ver `technomagik.md`). O Cronovisor recebe a mesma transmissão e a decodifica em imagem: o que o eco carrega visualmente aparece no próprio aparelho, dentro da ficção, em vez de ficar só no texto da conversa.

Efeito que o jogador observa: depois que ele acerta o puzzle de um mestre, com o rádio já ligado, o Cronovisor mostra uma cena que aquele mestre deixou no eco. A cena vem de uma lista fechada e aprovada para cada mestre (§3).

**O Cronovisor é opcional, e recompensa quem busca** (decisão do líder, 11/10/2026). Sem ele, a carta e a conversa do eco acontecem do mesmo jeito, porque o rádio basta para isso. O que se perde é a camada de imagem, o registro visual que o eco deixou. Consequência: **a narrativa não pode contar com o que o Cronovisor mostra.** Nada que o jogador precise entender para seguir pode existir só ali.

## 2. Por que é a metade que vê, e acopla no Tavus-Drive

O Pillar 3 fixa que toda habilidade do Gus passa por um dos três vértices do triângulo de hardware (`docs/design/pillars.md` §3). O Cronovisor é um upgrade plugado no Tavus-Drive, como o rádio, e segue a mesma lógica causal: o Tavus-Drive é o executor, e o Cronovisor é o ramo que **só recebe**. Ele não dispara conjuro, não resolve efeito e não altera o mundo. Recebe a imagem de um programa alheio, que é a mesma operação que o rádio faz com o som.

A diferença para o rádio é de sentido, não de regra: o rádio sintoniza, o Cronovisor olha. Por isso o Cronovisor não pode ser o que confere acerto, nem o que aponta onde ir (§3).

## 3. A regra: o que o Cronovisor NÃO mostra

O Cronovisor **mostra o que o eco já tem para mostrar depois do acerto**. Ele **não mostra** nada que ajude a resolver o puzzle. Em lista:

- **Nenhum estímulo do puzzle.** Antes do acerto, o Cronovisor não exibe glifo, padrão, sequência, forma do artefato a ligar nem posição de nada. Antes do acerto ele exibe só o estado do sinal, presente ou ausente, e esse estado depende apenas de o rádio estar ligado e de o eco estar presente. Ele não varia com a proximidade do artefato certo, com a configuração parcial nem com o número de tentativas. Se variasse, seria indicador de quente e frio, que é a dica que a L-38 proíbe.
- **Nenhum destaque sobre o mundo.** Não há contorno, scan, marcação, seta, brilho ou prompt sobre o artefato, a parede ou a câmara. Isso é o que a L-38 proíbe de forma expressa.
- **Nenhuma cena com o passo-chave.** Cada mestre tem uma lista fechada de cenas, aprovada pelo designer do puzzle daquele mestre. Nenhuma cena da lista mostra o passo que resolve o puzzle, e nenhuma mostra o mestre resolvendo o próprio puzzle.
- **Nenhuma informação de lado.** Nenhuma cena e nenhum elemento de puzzle depende de esquerda ou direita lida no Cronovisor. Um puzzle que precise de lateralidade não pode usar o Cronovisor como fonte. Isso torna o defeito de espelhamento (§4.1) inofensivo para o enigma.
- **Nenhum juízo de acerto.** O Cronovisor não diz se o jogador está perto ou longe. Quem confere o acerto é o jogo, e o aparelho só recebe.
- **Nenhum vocabulário técnico do puzzle.** O Cronovisor não nomeia o que o jogador deve fazer.

A regra vale para este documento e para qualquer documento futuro de puzzle que use o Cronovisor. A frase-guarda da L-38, repetida aqui como manda a lei: **nenhum enigma do jogo, em nenhuma dungeon, câmara ou área secreta, cede por ferramenta do jogo em vez do jogador que segura o controle.** E a frase final da mesma lei: **ferramenta que mostra estado observável do mundo é legítima; o que esta lei proíbe é a ferramenta que faz o raciocínio no lugar do jogador.** Qualquer solução em que o Cronovisor aponte a resposta contradiz esta decisão.

Por que imagem exige mais cuidado que som: uma cena visual vaza solução com mais facilidade que uma fala, porque o olho acha o padrão sem interpretação. Por isso a proteção tem três camadas, e as três são obrigatórias: a lista fechada de cenas (primeira bullet acima), o estado do sinal antes do acerto (sem conteúdo do puzzle) e o jogo conferindo o acerto por conta própria.

Separação de condições, como no rádio (`radio-padre-landell.md` §9): a **condição de execução** é o puzzle, resolvido pelo jogador; a **condição de recepção** é o aparelho, que só entrega o que o eco já tem. Nenhuma das duas faz o trabalho da outra.

## 4. A placa Lemon-Gama

A interface entre os dois aparelhos analógicos e o Tavus-Drive é uma placa de Lemon-Gama. É uma placa de computador de placa única, pequena e montada por um técnico só, sobre a qual o Joaquim fabrica a interface do Cronovisor e do rádio.

Por que uma placa de computador faz sentido aqui: o rádio e o Cronovisor são aparelhos analógicos (bobina, diafragma, tubo, cristal), e o Tavus-Drive é o executor digital, que roda o eco compilado. Alguma coisa precisa ler o sinal analógico, convertê-lo e entregar pulso ao executor, e fazer o caminho inverso para a imagem. Um computador de placa única é exatamente esse meio-termo, e é o que um técnico analógico consegue manter com as próprias mãos.

Ligações da Lemon-Gama:

1. **Ao rádio:** lê o estado do circuito primário, que é o que o diafragma abre e fecha, e o transforma em pulso para o executor.
2. **Ao Cronovisor:** envia a imagem decodificada do eco para o aparelho de visão.
3. **Ao Tavus-Drive:** entrega o pulso já convertido na porta do executor, que é o vértice do triângulo de hardware.

Nome: **Lemon-Gama** segue o padrão de nome da casa, `Palavra-Palavra` com hífen e maiúsculas (`Tavus-Drive`, `Tavus-Eco`, `Patch-Zero`).

**Nome diegético.** `Lemon-Gama` é o nome verdadeiro da placa dentro do mundo, fabricada e vendida por gente daquele mundo. Não é paródia, homenagem nem "versão do mundo de" outra coisa. No jogo, o nome aparece só como nome de ferramenta, dito como um técnico diria, e nenhum texto que o jogador lê compara a placa a qualquer outra.

**Correspondência de design (não é ficção).** A placa de mercado que corresponde à Lemon-Gama é a Orange Pi, e o nome vem dela por troca da letra pi por gama. Essa correspondência existe só neste documento, para quem projeta, e não vaza para o jogo.

### 4.1 Família de placas

O mundo tem uma linha inteira de placas, e a Lemon-Gama é uma delas (decisão do líder, 11/10/2026: a família existe). A razão é de verossimilhança: peça que pertence a uma linha parece real, e peça solta parece feita para a ocasião. Os nomes abaixo são **proposta, não canon**, e cada um depende de aprovação do líder.

Critério proposto: a letra grega indica a geração, pela ordem do alfabeto grego (Alfa é a primeira). O nome de fruta é só o nome da linha, e os três nomes do mesmo padrão ficam com o mesmo peso de nome próprio.

| Proposta | Geração |
|---|---|
| Banana-Alfa | 1 |
| Amora-Beta | 2 |
| Lemon-Gama | 3 (já no briefing) |
| Figo-Delta | 4 |
| Caju-Épsilon | 5 |

Alternativas que o líder pode escolher no lugar do critério de geração: a letra indicando porte (da menor à maior), ou o fabricante do mundo. A escolha não muda o resto do documento.

### 4.2 O defeito de espelhamento (canon, decisão do líder de 11/10/2026)

O Cronovisor, na primeira ligação, devolve a cena **espelhada**, invertida da esquerda para a direita. A fonte histórica confirma que a foto de Cristo atribuída a Ernetti era espelhada em relação ao original (§9). No mundo do jogo, o mesmo fenômeno é um defeito do aparelho, e não uma fraude.

Regra do defeito:

1. **Aparece uma vez, na primeira ligação do aparelho.** A primeira ligação acontece na visita do Joaquim (§5, beat 4). Não há outra condição no jogo que reintroduza a inversão. O jogador não gerencia nenhum estado.
2. **É declarado pelo próprio aparelho.** A inversão se anuncia como estado do Cronovisor, não como parte da cena. O jogador sabe que a imagem está invertida sem precisar deduzir isso da cena.
3. **É consertado na visita.** O ajuste do Joaquim corrige a inversão de uma vez. A partir daí a cena sai na orientação certa.
4. **Não carrega informação de enigma.** A inversão só acontece na visita, que é depois do rádio montado, e ocorre numa cena de ajuste, não numa cena de puzzle. Além disso, nenhum puzzle lê lateralidade do Cronovisor (§3). Assim a inversão nunca indica caminho nem passo-chave.

Justificativa da escolha: defeito que se anuncia e se conserta é cor local. Defeito que o jogador tem de deduzir é enigma disfarçado, e a L-38 proíbe as duas coisas. A frase-guarda da L-38 vale aqui também: **nenhum enigma do jogo cede por ferramenta do jogo em vez do jogador que segura o controle.** Qualquer solução em que a inversão do Cronovisor aponte a resposta contradiz esta decisão.

Por que o defeito ecoa o motivo do espelho no canon: o Espelho Negro do John Dee (`docs/narrative/deep/characters/mestre-ocu-02-john-dee.md`), o Espelho de Obsidiana (`docs/design/dungeons/espelho-obsidiana-artefato.md`) e o Espelho Acústico Sterling (`sinopse.md`) já são motivos do canon. O defeito não cria símbolo novo.

## 5. A cadeia da encomenda, em seis beats

A cadeia segue a ordem que o líder ditou. Ela faz o Joaquim entregar a placa só depois da missão de pagamento, e não antes.

Decisão do líder (11/10/2026): **a visita acontece depois de o rádio estar montado e ligado.** O jogador já tem o rádio funcionando e já ouviu um mestre quando o Joaquim chega. A razão é que a cena ganha peso porque o jogador sabe o que está sendo ampliado, e o circuito faz sentido físico, já que liga duas coisas que existem.

1. **Referência.** O Padrinho Tiago, que atravessa entre o Núcleo Metropolitano e a Zona do Silêncio, diz ao Gus que o Joaquim aceita encomendas. **Onde:** no Núcleo Metropolitano, numa das travessias de Tiago (`CHARS.md`, linha 84).
2. **Esquema.** O Gus desenha o esquema completo do rádio e do Cronovisor, com as cinco peças do rádio nomeadas como no blueprint (`radio-padre-landell.md` §6) e a Lemon-Gama ligada aos dois. **Onde:** no apartamento Vance, sexto andar, lado leste do Núcleo (`PLACES.md`, linha 83).
3. **Envio por email com fotos.** O Gus manda ao Joaquim o esquema e as fotos das peças que já tiver. **Onde:** no apartamento Vance.
4. **Visita e ajuste ao vivo.** O Joaquim vai à casa do Gus, com o rádio já montado e ligado. Ele tenta conectar o Cronovisor pela primeira vez. A imagem sai espelhada (§4.2), e o ajuste ao vivo corrige a inversão. A conexão funciona em parte, mas a placa final não sai dessa visita. **Onde:** no apartamento Vance, na bancada ativa da Gargi (§8).
5. **Recusa do dinheiro.** O Joaquim não cobra em dinheiro, nem do Gus nem da party. Ele pede a conclusão de uma missão. **Onde:** na mesma visita, logo depois do ajuste.
6. **Missão de pagamento.** O Gus e a party encontram os suprimentos eletrônicos que o Joaquim pede (§6) dentro do dungeon que ele teme. Só quando os itens chegam às mãos dele, o Joaquim entrega a interface pronta. **Onde:** a missão acontece na Periferia, em vielas (§6); a entrega acontece no apartamento Vance (§12, ponta 4).

Efeito que o jogador observa: o Joaquim não é comprado com moeda. Ele entrega o que o jogador conquistou, e a placa chega como resultado de uma missão, não de uma compra.

## 6. O dungeon que o Joaquim teme: Periferia, vielas

**Escolhido:** o dungeon das **vielas da Periferia residencial**, do tipo **labirinto** (`mundo-topologia.md` §2, linhas 56 e 91).

**Dificuldade no grafo:** tier 2, média (`mundo-topologia.md` §2, linha 22). Alcançável cedo, sem depender de nenhuma área difícil.

**Razão da escolha:** o Joaquim é um técnico que trabalha pelo som, e um labirinto de vielas apertadas tira a referência de rota de quem caminha sozinho. O medo dele é esse: entrar sozinho em um lugar onde perder a saída é fácil e a ajuda só vem de quem estiver junto. Um labirinto é o tipo de dungeon que se teme sozinho, e é o tipo que a party atravessa com facilidade.

**Por que não os outros:**

- **Zona do Silêncio:** é a casa dele, tier 3, e o puzzle dela é acústico. Pôr o medo do Joaquim na própria casa seria contraditório.
- **Dutos Infernais:** é o dungeon de abertura, já conhecido pela party desde o começo. Não cabe como medo.
- **Ferrovelhos:** é o reduto do Dante, o traidor. Misturar a missão do Joaquim com o reduto dele amarraria duas tramas sem necessidade.
- **Setor Mirage:** é o Festival, de puzzle de ilusão, um tipo de desafio que não combina com o medo de um técnico de som.
- **Catedrais Neo-Sylvania e Selve Sombria:** são tier 4, o que empurraria o Cronovisor para o fim do jogo. Como o rádio já é gate cedo, o Cronovisor não deve depender de área tardia.

**Coexistência a registrar:** a Periferia também guarda a peça 4 do rádio, as esferas de faísca (`radio-padre-landell.md` §6, item 4). Ela pode ser achada na mesma área das vielas. A coexistência é aceitável porque a peça 4 é uma peça de rádio e os suprimentos pedidos são outros itens, mas a escolha não é neutra (§12, ponta 3).

**Suprimentos eletrônicos pedidos:** os três abaixo são **proposta pendente de aprovação do líder** (§12, ponta 1). Nenhum deles é uma das cinco peças do rádio.

1. Um tubo de vácuo de estágio de alta tensão, do tipo que a sucata da Periferia ainda guarda de rádios antigos.
2. Um capacitor de alta tensão, para o estágio de conversão da Lemon-Gama.
3. Um cristal de quartzo de frequência fixa, para o oscilador que sincroniza o pulso enviado ao executor.

**Por que a missão não é gate:** o Cronovisor é opcional (§1 e §10). A missão é o pagamento de uma encomenda, e não o único acesso a algo essencial.

## 7. Quem referencia o Joaquim: Padrinho Tiago

Quem diz ao Gus que o Joaquim aceita encomendas é o **Padrinho Tiago**, personagem existente e canônico (`CHARS.md`, linha 84): coordenador Underground do Silêncio, que atravessa o Núcleo e a Zona, e que tem vínculo com a Linda e com a família Bento. Ele já é o elo natural entre a cidade e a Zona, então a referência não cria personagem novo.

O Joaquim já tem uma relação prévia com o jogador, pelo easter egg público EE-25 (`docs/narrative/comic-reliefs/easter-eggs-educacionais.md`, linhas 57 a 72). Ali ele dá um nome e não entrega o caminho de bandeja, e recusa repetir: "Não vou repetir de outro jeito só porque você quer mais fácil." A postura que o Cronovisor herda é a mesma. O aparelho recebe, e quem decide é o jogador.

Nenhuma personagem nova entra neste documento. Não há proposta pendente de pessoa.

## 8. Por que a visita é no apartamento Vance, e a bancada é a da Gargi

**O encontro é no apartamento Vance, e não na Zona do Silêncio.** A razão é de desenho: a Zona do Silêncio é tier 3, área de dificuldade média-alta, e o ajuste do rádio não pode depender dela (`radio-padre-landell.md` §3). Pôr o encontro lá faria o Cronovisor depender de área tardia. O Joaquim trabalha na Zona no canon (`environments/07-zona-do-silencio.md`, linha 57), e o encontro na casa do Gus não nega isso: é a escolha do líder, e este documento só a justifica pelo gate.

**A bancada é a ativa da Gargi, e não a do subsolo do Edifício Vance.** A família tem duas bancadas, e o canon as fixa assim:

- **Bancada ativa da Gargi:** na sala do apartamento Vance. A Gargi é técnica de bancada eletrônica e conserta placa-mãe à noite nessa sala (`sinopse.md`, seção de família; `CHARS.md`, §4).
- **Bancada herdada do Pyotor:** no subsolo do Edifício Vance, sobre um piso coberto de carpete, com um alçapão sob o carpete cujo conteúdo o canon não define (`PLACES.md`, linha 84).

A escolha da bancada da Gargi tem três razões:

1. Ela é a bancada em uso. Um ajuste ao vivo pede ferramenta espalhada, energia e alguém trabalhando ao lado, e a sala da Gargi é o lugar onde isso acontece todas as noites.
2. Ela fica no apartamento, que é a safe base do Gus. A cena de ajuste fica no lugar que o jogador já conhece.
3. A bancada do subsolo deixa em aberto o que há sob o alçapão. Usá-la para o ajuste traria uma pergunta de canon que este documento não pode decidir.

## 9. Quem foi Pellegrino Ernetti

O Cronovisor é uma lenda real, atribuída ao padre **Pellegrino Ernetti**, monge beneditino italiano, da Abadia de San Giorgio Maggiore, em Veneza, onde entrou em 1941. Nasceu em 1925 em Rocca Santo Stefano, perto de Roma, e morreu em Veneza em 1994.

**O que ele alegou:** em 1972 afirmou ter criado uma máquina que observava e filmava eventos do passado. O visor era mudo: a alegação era de imagem sem som. Disse ter visto a Crucificação e uma apresentação da peça perdida *Thyestes*, de Ênio. Ele apresentou uma foto como sendo de Cristo.

**A foto:** a imagem foi identificada depois como uma escultura de madeira de Lorenzo Coullaut Valera. Uma fonte descreve a imagem como invertida (espelhada) de um cartão-postal do Santuário do Amor Misericordioso, em Collevalenza. São a mesma história vista de dois ângulos, e a escultura é a identificação principal. Nas duas versões, a foto foi desmascarada.

**Gemelli:** Agostino Gemelli aparece ligado ao Ernetti numa história de gravador. Segundo o relato de Peter Krassa, em 15 de setembro de 1952 Ernetti trabalhava com Gemelli num gravador. Gemelli não consta entre os colaboradores da máquina.

**O que a lenda não tem:** a alegação não tem verificação independente, e os estudiosos a tratam como pseudocientífica.

**Há duas versões do fim da história, e elas não concordam.** Nenhuma é escolhida aqui:

- Uma diz que Ernetti confessou, no leito de morte, que o texto atribuído à Antiguidade fora montado a partir de fragmentos de autores antigos, e que a imagem de Cristo fora fabricada. Nessa confissão ele também disse que esperava que o Cronovisor funcionasse. Essa versão está na Wikipedia.
- Outra diz que Ernetti sustentou a alegação até morrer, em 1994. Essa versão vem de uma fonte do líder, e as duas consultas feitas para este documento não a encontraram. Ela fica aqui como versão em disputa, sem confirmação.

A página do Cronovisor trata a confissão como suposta, sem detalhar o conteúdo.

**Não confirmado por nenhuma das consultas:** o livro de François Brune, a revista e a data de publicação da alegação (La Domenica del Corriere, 2 de maio de 1972), a explicação de Ernetti envolvendo uma freira espanhola, a atribuição da instrução do escultor a Madre Speranza, a loja do santuário como origem da foto, o nome de Fermi entre os colaboradores, a objeção física sobre captar todos os comprimentos de onda e a afirmação de que a máquina nunca foi mostrada a ninguém. Não os trate como fato até confirmação.

**Ponte com o jogo:** a lenda descreve um visor que registra o passado. O Cronovisor do jogo recebe a transmissão que o rádio já recebe, do eco, e a decodifica em imagem, sem captar o espectro inteiro. O jogo resolve, in-fiction, a parte que a lenda não explica: a transmissão é uma só, a do eco, e não qualquer onda do passado.

## 10. Cortes que o Cronovisor respeita

- **C-04 (sem voz):** o Cronovisor mostra imagem, como o visor da própria lenda, que era mudo (§9). Qualquer fala de mestre ou do Joaquim continua em texto. Nenhuma dublagem entra por este caminho.
- **C-02 (sem gating por barreira dura):** o Cronovisor é opcional, por decisão do líder de 11/10/2026. Ele não barra o caminho principal, e a carta do eco não depende dele (§1).
- **C-15 (duração e orçamento):** o Cronovisor é mecânica nova, e só entra se couber no orçamento de conteúdo da área onde vive. A missão de pagamento vive na Periferia, e sua execução precisa caber no orçamento dessa área.

## 11. Partição público e cifrado

Este documento é **público**. Ele descreve o que o jogador observa (o Cronovisor existe, acopla no Tavus-Drive, a Lemon-Gama faz a interface, a encomenda tem seis beats, o dungeon do pagamento é o das vielas da Periferia), a lore pública do Joaquim e a lenda do Ernetti, que é anterior ao jogo e não é segredo.

Nenhum conteúdo aqui é revelação escondida: as cenas do Cronovisor, quando forem escritas, são listas aprovadas pelo designer de puzzle, e não estão neste documento. Por isso não há gêmeo cifrado em `docs/_secret/mecanicas/`. Se uma cena passar a depender de um segredo cuja resposta não deva ficar pública, o gêmeo nasce naquele momento, nomeado `cronovisor-solucao.md`, e não antes.

A relação prévia do Joaquim com o jogador vem do easter egg público EE-25 (§7). Há também um desdobramento desse easter egg em área cifrada (`docs/_secret/`). Este documento não lê nem cita o conteúdo desse arquivo, e não cria nenhuma relação nova que dependa dele.

## 12. Pontas abertas para o líder

Estas seis questões não foram decididas por agente. Não se resolvem sem o líder, e cada uma precisa de resposta antes de virar canon.

1. **Os três suprimentos (§6):** tubo de vácuo, capacitor de alta tensão e cristal de quartzo. São proposta, não canon.
2. **Dungeon do pagamento:** a escolha de vielas da Periferia é recomendação deste documento, com as razões de §6. O líder decide.
3. **Coexistência com a peça 4 do rádio:** a Periferia tem a peça 4 na mesma área das vielas. O líder confirma que a coexistência é aceitável.
4. **Lugar da entrega da placa:** proposta de que o Joaquim espere no apartamento Vance, o mesmo lugar do ajuste, para fechar a cena. O líder decide.
5. **Lista de cenas do Cronovisor:** ainda não existe. Quando existir, precisa de aprovação do líder e de revisão do designer do puzzle daquele mestre.
6. **Nomes da família de placas (§4.1):** a tabela é proposta. O líder escolhe os nomes, ou o critério (geração, porte ou fabricante).

**Decisões do líder de 11/10/2026, já aplicadas no documento:**

- O Cronovisor é opcional e não é gate: §1, §10 e §6.
- A visita acontece depois do rádio montado: §5.
- A família de placas existe: §4.1.
- O espelhamento é defeito do aparelho, com a regra de não virar pista: §4.2 e §3.

---

**Grounding de pesquisa** (verbetes em inglês `Pellegrino_Ernetti` e `Chronovisor`, consultados em 11/10/2026, sem o verbete em português disponível): nascimento em 13/10/1925, Rocca Santo Stefano; morte em 08/04/1994, Veneza; entrada na Abadia de San Giorgio Maggiore em 1941; alegação de 1972 sobre um visor mudo do passado; foto da Crucificação identificada como escultura de Lorenzo Coullaut Valera, espelhada em relação ao original, segundo a página do Cronovisor; Gemelli num gravador em 15/09/1952, segundo o relato de Peter Krassa; confissão de leito de morte, citada na Wikipedia. A versão de que Ernetti sustentou a alegação até morrer vem de uma fonte do líder e não foi encontrada nas consultas. Os itens não confirmados estão em §9.
