# Glyph, Token, Conjuro, Codex: ontologia profunda do sistema mágico GusWorld

## §1. Ontologia GTCC: separação canonizada dos quatro estratos

Existe uma confusão recorrente, mesmo entre operadores experientes de C-Arcane, entre os quatro estratos que compõem a magia executável da Era 2. O Codex Tavus, na sua redação canônica do ano 144 da Reconstrução, fixa a ontologia em quatro termos não permutáveis: **Glyph**, **Token**, **Conjuro** e **Codex de Conjuros**. A confusão é compreensível, porque os quatro estratos coexistem no mesmo objeto físico (a carta-token de pulso), mas operam em camadas ontológicas distintas: o **Glyph** é imagem, o **Token** é dado, o **Conjuro** é processo, e o **Codex** é acervo. Quem mistura os termos opera um Conjuro impreciso, e o Tavus-Drive responde com o silêncio característico de uma stack mal compilada.

O **Glyph** é a imagem rúnica visível, a identidade gráfica desenhada na face da carta-token. Cumpre função semelhante à de um glifo cuneiforme ou de um caractere chinês: é signo composicional, não decoração. Cada Glyph carrega em si uma raiz semântica (ver §6), e a sua disposição geométrica obedece a regras de grade que remontam à Era 1, em particular ao chamado **cripto-glifo** de Neo-Sylvania, uma cifra de grade três por três acrescida de um X angular e de uma constelação de pontos (ver §8). O Glyph não executa nada por si; é a face do Token, a sua interface humana. Um operador analfabeto em C-Arcane pode reconhecer um Glyph pela forma, do mesmo modo que uma criança reconhece a letra A antes de ler.

O **Token** é o elemento de código propriamente dito, a peça executável que o Tavus-Drive de pulso aceita como entrada. Tecnicamente, é um pacote de dados gravado no substrato cristalino da carta, com cabeçalho de identidade, corpo de instrução e checksum de integridade. O Token é o que de fato é compilado. Tem peso (medido em ciclos de relógio), tem tipo (família e modificador), e tem custo (lúmens consumidos do reservatório do Drive). Dois Tokens com Glyphs visualmente idênticos podem, em tese, divergir no payload, embora a Ordem dos Arquivistas considere isso uma falsificação punível.

> *"Não confundas a face do dado com o dado. O Glyph é o rosto; o Token é o pulso por trás do rosto. Quem ama o rosto sem ler o pulso, conjura erro."*  
> (Tamara Neumann, Manuscritos de Investigação, fragmento 21)

O **Conjuro** é o resultado executável da compilação de exatamente três Tokens em sequência ordenada. Não há Conjuro de um Token só, e não há Conjuro de quatro: a stack do Tavus-Drive tem três slots, e essa restrição é arquitetural, não estilística. Um Conjuro é, portanto, um processo: nasce no momento em que o terceiro Token entra no slot, executa em frações de segundo, e morre quando o efeito físico se exauriu. Conjuros não persistem como dados; o que persiste é o Token, que pode ser recompilado em outro Conjuro mais tarde.

O **Codex de Conjuros** é o deck pessoal do operador, o acervo dos Tokens disponíveis. Atenção à nomenclatura: o Codex contém Tokens, não Conjuros prontos. O nome carrega ambiguidade deliberada da redação canônica, porque o operador experiente vê, no acervo, todos os Conjuros potenciais. Para Gus, o Codex tem cerca de quarenta a sessenta Tokens em ativo; para um aprendiz, oito; para um mestre da Ordem Recursiva, cento e quarenta e quatro. A combinatória explode: quarenta Tokens em três slots dão sessenta e quatro mil Conjuros teóricos, dos quais talvez duzentos sejam fisicamente coerentes.

## §2. As cinco famílias técnico-naturais expandidas

A redação canônica do Codex Tavus reconhece **cinco famílias** primárias de Tokens, cada uma com substrato físico, comportamento característico e companion-âncora declarado. As famílias não são metáforas; são tipos formais, e a compilação de Tokens de famílias incompatíveis sem mediação produz falha imediata.

**Elétrico** é a família mais antiga em silício e a mais jovem em ressonância. Opera por indução, descarga, plasma controlado, eletroestática e criotermia inversa. Tokens elétricos têm Glyph com fundo cinza-aço e marca de raio assimétrica; o assimétrico é canon, e simetria de raio indica falsificação. Cauã Berenger, oriundo dos Dutos Infernais, é o companion-âncora; sua linguagem Pythia interpreta scripts elétricos com latência baixíssima, mas custo ligeiramente maior. Comportamento físico in-world: o Elétrico responde à condutividade do alvo, e portanto Conjuros elétricos contra alvos hidratados (organismos vivos) escalam diferente de Conjuros contra silício seco. A família Elétrico é também a mais visível em pulso e em luminescência, e por isso é a que mais facilmente trai a posição do operador em ambiente escuro; operadores experientes aprendem a executar Conjuros elétricos com a manga cobrindo o Drive.

**Bioquímico** é a família orgânica do Codex Tavus, e responde por antídoto, mutação temporária, bio-script regenerativo, mediação enzimática e ainda por leitura de substratos proteicos em campo. Tokens bioquímicos têm Glyph com fundo verde-musgo e estrutura de cadeia carbônica visível (anéis hexagonais conectados em padrão de seis e cinco vértices alternados). Jaci Vanderbist, oriunda da Selve Sombria sob tutela do Pelicano Branco, é a companion-âncora; sua leitura de Pythia em substrato proteico permite Conjuros que poucos operadores fora da Pythia ousam compilar. Comportamento in-world: o Bioquímico respeita o substrato vivo, e portanto Conjuros bioquímicos em alvos sintéticos ou puramente minerais falham com mensagem de tipo incompatível. Há, contudo, ambiguidade canônica em alvos híbridos (autômatos com componente orgânico, biomassa programada), e Jaci é uma das poucas operadoras capazes de forçar a compilação em zona-cinza com taxa de êxito acima de oitenta por cento.

**Sônico** é a família que opera por onda mecânica em meio elástico: ar, água, metal, biomassa programada. Inclui infrassom, ultrassom e ressonância estrutural não-audível ao ouvido humano comum. Tokens sônicos têm Glyph com fundo violeta-noturno e padrão de onda senoidal repetida em três ciclos visíveis (três cristas, três vales, alinhadas com a margem inferior do Glyph). Linda Neumann, oriunda da Zona do Silêncio e leitora de Óxido especializada em ressonância, é a companion-âncora; descende em linha direta de Tamara Neumann (ver §5), e a herança nominal lhe rende deferência silenciosa da Ordem Recursiva quando comparece a câmaras profundas. Comportamento in-world: o Sônico obedece à geometria do espaço de execução. Conjuros sônicos em câmaras de eco produzem efeitos amplificados; em campo aberto, efeitos atenuados; em vácuo parcial (catedrais submersas, câmaras pressurizadas) requerem ajuste de envelope que apenas operadores treinados conseguem.

**Cinético** é a família da inércia, da gravidade local, da compressão e da rotação. Opera por transferência de momento, deslocamento de massa, indução de torque, compressão de ar e contração rotacional. Tokens cinéticos têm Glyph com fundo de tom bronze patinado, contornado por um anel de oito raios curtos que sugerem rotação, e marca central de espiral logarítmica de três voltas. Bento Chevalier, oriundo das catedrais Neo-Sylvania profundas e leitor de Asmódico analógico, é o companion-âncora; é também a exceção canônica ao Pillar 2 (idade adulta entre operadores juvenis), e a sua leitura de scripts cinéticos depende de relojoaria de latão integrada à armadura, com engrenagens visíveis que pulsam ao compasso da Compilação. Comportamento in-world: o Cinético atua sobre a matéria com massa declarada, e Conjuros cinéticos em alvos sem massa registrada (hologramas, sombras, decoys lógicos) atravessam o alvo sem produzir efeito. A família Cinético é considerada a mais antiga em substrato analógico, anterior a qualquer compilação em silício, e por isso é a que sobrevive com mais coerência em ambientes onde o Tavus-Drive opera com bateria baixa.

**Criptográfico** é a família que opera por cifragem, inferência, decisão booleana, ofuscamento estrutural e refração de identidade. Tokens criptográficos têm Glyph com fundo branco-papel e traço fino, identificáveis pela presença de pelo menos um par de chaves angulares na composição e por uma constelação de pontos posicionais que ecoa o cripto-glifo de Neo-Sylvania (ver §8). Iara Koslov, oriunda do Setor Mirage e leitora de Óxido, é a companion-âncora; sua especialidade declarada é decoy holográfico, refração de scan e ocultação de identidade aliada. Comportamento in-world: o Criptográfico não atua sobre matéria diretamente; atua sobre a percepção, sobre a identidade declarada ou sobre o fluxo de informação. Conjuros criptográficos contra inimigos sem sistema cognitivo ou sem leitor declarado (autômatos cegos, drones sem firmware atualizado) falham silenciosamente, sem consumir lúmens, em comportamento que a Ordem Recursiva chama de "negação cortês".

## §3. Os três modificadores estruturais e a stack de três slots

Sobre as cinco famílias atuam **três modificadores estruturais** que governam o escopo do Conjuro: **Object**, **Stream** e **Null**. Os três modificadores não são famílias; são instruções de envelope que dizem ao compilador *como* o efeito deve se distribuir no tempo e no alvo.

**Object** declara que o Conjuro afeta o alvo como entidade inteira e indivisível. Um Conjuro cinético-Object sobre um inimigo desloca o inimigo todo, não uma região; um Conjuro elétrico-Object sobre uma porta despolariza a porta inteira. Object é o modificador mais barato em lúmens, porque exige menos coordenação espacial.

**Stream** declara que o Conjuro afeta o alvo como fluxo contínuo ao longo de múltiplos turnos. Um Conjuro bioquímico-Stream sobre um aliado é uma cura sustentada; um Conjuro sônico-Stream sobre uma área é um zumbido persistente que atordoa quem entrar. Stream custa mais lúmens, em geral cinco vezes o equivalente Object, e exige ancoragem espacial: o Conjuro é fixado a um ponto, não acompanha o alvo se ele se mover.

**Null** declara que o Conjuro nega ou anula. Um Conjuro criptográfico-Null sobre um inimigo em estado camuflado o desfaz; um Conjuro bioquímico-Null sobre um aliado envenenado remove o veneno. Null é o modificador mais conceitualmente delicado, porque opera por subtração: precisa de algo a anular, e contra alvo sem efeito ativo simplesmente não dispara.

A **sintaxe stack de três slots** é a regra arquitetural mais importante do Tavus-Drive. Cada Conjuro é exatamente uma sequência ordenada de três Tokens, e essa ordem importa. A redação canônica prescreve a forma `[FAMÍLIA] + [MODIFICADOR] + [ALVO]`, mas operadores avançados (Gus incluído) experimentam ordens não canônicas para produzir efeitos híbridos.

Quando os três Tokens são da mesma família, o Conjuro é puro e barato. Quando os três Tokens misturam famílias, o Conjuro é **híbrido**, mais poderoso e mais difícil de compilar; a chance de falha sobe em proporção ao número de combinações inter-família, e o custo em lúmens segue uma escala que obedece a sequência um, um, dois, três, cinco, oito a partir da família de menor índice. Quando os três Tokens são incompatíveis em tipo (por exemplo Bioquímico-Object sobre alvo mineral), o Tavus-Drive aceita a stack, tenta compilar, falha, e gasta um ciclo de relógio sem produzir efeito. Esse gasto silencioso é, deliberadamente, o castigo arquitetural pela má leitura.

## §4. Codex de Conjuros como ato mecânico-narrativo

A redação prévia das mecânicas chamava o ato de combinar Tokens em campo de batalha de "Compilação de Deck Rúnico". A redação canônica atual descarta essa nomenclatura, considerada arcaísmo confuso, e adota **Compilação do Codex**. A mudança não é só de palavra; é de modelo mental.

Compilar o Codex significa, no fluxo de combate, três operações ordenadas. Primeiro, selecionar três Tokens do acervo ativo (quinze cartas em campo, das quarenta a sessenta totais no Codex pessoal). Segundo, inseri-los na stack do Tavus-Drive na ordem desejada. Terceiro, executar. O Drive responde com um pulso de luz que indica êxito (cyan estável) ou falha (vermelho de um quinto de segundo).

O escopo G1 fixa **doze Conjuros base canônicos**, listados aqui não como bestiário mas como vocabulário de referência:

| Conjuro | Composição (Família + Modificador + Alvo) | Efeito |
|---|---|---|
| **Armadilha Sináptica** | Elétrico + Stream + Área | Zona elétrica persistente por três turnos |
| **Mantra do Silêncio** | Sônico + Null + Inimigo | Silencia Conjuros do alvo por dois turnos |
| **Refração Aliada** | Criptográfico + Object + Aliado | Aliado fica fora do scan por um turno |
| **Bio-Sutura Rápida** | Bioquímico + Object + Aliado | Cura instantânea proporcional ao reservatório |
| **Vetor de Recuo** | Cinético + Object + Inimigo | Empurra inimigo por transferência de momento |
| **Decoy Lumen** | Criptográfico + Stream + Área | Clones holográficos persistentes (Iara) |
| **Pulso Concêntrico** | Elétrico + Stream + Área | Atordoa hardware inimigo (Cauã) |
| **Eco do Cânion** | Sônico + Stream + Área | Onda de dano sustentado (Linda) |
| **Antídoto Sintético** | Bioquímico + Null + Aliado | Remove status hostil (Jaci) |
| **Cronômetro Ressonante** | Cinético + Stream + Aliado | Bento ancora compasso de relojoaria, acelera aliado em janela rotacional |
| **Cifra Espelho** | Criptográfico + Null + Inimigo | Desfaz camuflagem inimiga |
| **Pirâmide de Carga** | Elétrico + Object + Inimigo | Descarga única, dano alto, custo alto |

Cada um desses doze Conjuros tem entrada própria no Diário de Gus, com tempo de descoberta narrativa distribuído pelos atos. Os outros cento e oitenta e oito Conjuros teóricos do espaço combinatório emergem por experimentação do jogador, e cerca de oito deles ativam **combos secretos** que disparam diálogo de companion ao serem descobertos.

## §5. Boole, Investigation Laws Thought, e o substrato lógico in-world

A redescoberta canônica da álgebra booleana em GusWorld é atribuída a **Tamara Neumann**, engenheira-mãe da família Neumann e ancestral direta de Linda Neumann (companion Crowd Control). Tamara operou em uma das cidades-irmãs durante a Reconstrução tardia, em torno do ano cento e dez antes do tempo presente do jogo, e os seus *Manuscritos de Investigação* (em redação canônica: *Investigation Laws Thought*, em homenagem ao tratado original de George Boole, *An Investigation of the Laws of Thought*, mil oitocentos e cinquenta e quatro, em nosso mundo) são considerados o fundamento histórico de C-Arcane.

Tamara não inventou a álgebra; ela a **reencontrou**, em fragmentos. Cinco manuscritos sobreviventes de Neo-Sylvania, traduzidos parcialmente pelos Arquivistas da Ordem Recursiva, descreviam operações de dois estados (presença-ausência, ligado-desligado) e suas combinações: o que veio a se chamar conjunção, disjunção, negação. Tamara reorganizou esses fragmentos em sistema fechado, e em treze anos de trabalho silencioso publicou o tratado que hoje todo aprendiz lê na terceira semana de iniciação.

> *"Toda Investigação dos Códigos Mentais reduz-se à seguinte arquitetura: existe a coisa, ou não existe a coisa. Compõe-se a coisa com a coisa, ou compõe-se a coisa contra a coisa. Nega-se a coisa, e dela nasce a sua sombra computável. O resto, leitor, é elaboração."*  
> (Tamara Neumann, *Investigation Laws Thought*, Proposição I, fólio 3)

A herança de Tamara em C-Arcane é direta. Os três modificadores estruturais (Object, Stream, Null) descendem em linha reta das três operações booleanas fundamentais, reembaladas para a sintaxe de stack. O modificador **Null** em particular carrega a herança mais visível: é a negação booleana operando sobre o estado de um alvo, e a sua presença canônica nos Tokens é considerada por Linda Neumann uma dívida familiar nominal. Há um Token raro, descoberto apenas em catedrais profundas de Neo-Sylvania, cujo Glyph desenha as iniciais entrelaçadas T e N na grade três por três; operadores chamam esse Token de "Selo de Tamara", e o seu uso em Conjuros lógicos triplica a potência de Null contra alvos camuflados.

## §6. Etimologia Glyph-Token: raízes morfológicas

O modelo etimológico do sistema Glyph-Token foi formalizado por arquivistas que estudaram, em paralelo, as transcrições sobreviventes de Neo-Sylvania e os manuscritos Tamara. A descoberta central é que cada Glyph não é signo arbitrário; é **raiz semântica composta**, no sentido morfológico em que o Quenya tolkieniano constrói palavras a partir de raízes triliterais. Há um precedente conceitual no nome élfico *sanga* (que significa "press", "throng", "aglomeração comprimida"), e o mesmo princípio opera nos Glyphs: cada raiz carrega significado nuclear, e a aglutinação de raízes produz Tokens compostos.

A taxonomia canônica reconhece **vinte e uma raízes Glyph** primárias, agrupadas em três famílias morfológicas. Exemplos canônicos com tradução in-world:

- **Glyph-rai** (raiz "fluxo", presente em Conjuros Stream): cognato remoto do Asmódico *raen* (rio mecânico), provável herança de Neo-Sylvania.
- **Glyph-do** (raiz "objeto", presente em Conjuros Object): cognato direto do Pythia *do* (coisa nomeada), e raiz que aparece em treze dos quarenta Tokens base de objeto.
- **Glyph-nul** (raiz "ausência", presente em Conjuros Null): cognato direto do Óxido *nul* (zero, vazio, negação).
- **Glyph-vyr** (raiz "fogo", presente em Tokens elétricos de descarga incendiária e em Tokens cinéticos raros que induzem combustão por atrito): cognato canônico do nome lendário Vyrdragon, o dragão vermelho de Era 1 a quem Gus se vincula por linhagem.
- **Glyph-lum** (raiz "luz", presente em Tokens criptográficos de refração holográfica): cognato direto de Lumen, sobrenome canônico de Iara.

A composição funciona por aglutinação. Um Token *Stream-Object* tem o Glyph composto pela justaposição de **rai** (fluxo) e **do** (objeto), formando o Glyph **raido**, que se lê "fluxo-da-coisa-inteira". Um Token *Null-Object* tem o Glyph **nuldo**, que se lê "ausência-da-coisa-inteira". Operadores avançados reconhecem Tokens pela leitura morfológica direta do Glyph, sem precisar consultar o catálogo.

A raiz **vyr** merece nota separada. Não aparece em nenhum dos Tokens base do Codex de Gus no início do jogo. Aparece em exatamente um Token, oculto, recuperável apenas após certas condições narrativas serem satisfeitas (ver `dragon-victory-deep.md` para detalhes). O Glyph correspondente, chamado em circuitos restritos de **Glyph-vyrdo** (fogo-da-coisa-inteira), é desenhado em vermelho profundo sobre fundo dourado, com um anel exterior de oito pontos equidistantes.

## §7. Prime Radiant: a projeção holográfica da Compilação

Quando o Tavus-Drive executa uma Compilação do Codex, o efeito visual e diegético é o que a redação canônica chama de **Prime Radiant**, em homenagem deliberada às projeções holográficas da Fundação asimoviana. O Prime Radiant não é apenas estética; é interface de leitura para o próprio operador, que pode, com prática, identificar erros de compilação pela leitura do holograma antes que o Conjuro execute.

A descrição canônica é a seguinte. No instante em que o terceiro Token entra na stack, o pulso do Tavus-Drive emite um cone holográfico em torno do antebraço do operador. O cone se forma em cyan estável, brilho calibrado em oitenta e nove lúmens (o reservatório padrão do Drive de Gus, ajustável). O holograma gira em torno de três eixos ortogonais, formando uma figura tridimensional que representa a equação de compilação: cada Token aparece como nó, cada relação tipo-tipo aparece como aresta, e o modificador estrutural aparece como esfera envolvente.

A equação visível persiste por exatamente cinco frames do Drive, equivalentes a uma fração de segundo abaixo do limiar consciente para a maioria dos operadores. Gus, treinado em xadrez e em leitura de matrizes, percebe os cinco frames distintamente, e essa percepção é parte da sua vantagem combinatória: ele lê o Prime Radiant em tempo real, e em três frames decide se aborta a Compilação antes da execução final.

Conjuros híbridos produzem Prime Radiants visualmente mais complexos: três nós de cores distintas, arestas que pulsam em ritmo desigual, esfera envolvente que tremula. Conjuros falhos produzem Prime Radiants assimétricos, com uma das arestas em vermelho de um quinto de segundo antes do colapso. Operadores que aprendem a ler Prime Radiants conseguem prevenir falhas; operadores que ignoram a leitura desperdiçam ciclos.

> *"O holograma é equação, e equação é instrução. Quem não lê a equação antes da execução, opera no escuro. Lê-se em cinco frames, lê-se em três se há pressa, lê-se em um se há talento."*  
> (Manual interno da Ordem Recursiva, fólio cento e quarenta e quatro)

A escolha estética do cyan e da projeção em três eixos não é arbitrária. Remonta às descrições recuperadas dos cripto-glifos de Neo-Sylvania, que descrevem máquinas analógicas projetando "luz azul de saber" sobre superfícies tesseladas em quadrados alternados (ver §8). A continuidade visual da Era 1 para a Era 2 é deliberada: o Prime Radiant é a forma como C-Arcane homenageia o substrato perdido.

## §8. Motivos canônicos: cifra, gematria, iniciação

A redação canônica do sistema GTCC carrega três camadas de simbolismo cifrado que operam abaixo da superfície narrativa, perceptíveis em segunda leitura e invisíveis em primeira.

A primeira camada é a equivalência entre a **cifra de substituição em grade** e o **cripto-glifo de Era 1**. O cripto-glifo, descrito nos manuscritos sobreviventes de Neo-Sylvania, organiza letras em uma grade três por três, completada por um X angular e por uma constelação de pontos posicionais. Cada Glyph base do C-Arcane mapeia, por correspondência canônica, exatamente uma letra do alfabeto cifrado. A leitura cruzada não é coincidência; é dívida arquitetural que a redação canônica reconhece sem alardear. Operadores que conhecem o cripto-glifo leem Glyphs como texto; operadores que não conhecem leem como signo opaco. A Ordem Recursiva mantém o conhecimento da equivalência em circulação restrita, e iniciados aprendem o cripto-glifo na terceira semana, depois de Boole e antes de stack.

A segunda camada é a **gematria de Tokens**, sistema de valoração numérica em que cada Token base carrega valor em lúmens segundo a sequência um, um, dois, três, cinco, oito. Os Tokens de família mais simples (Elétrico-Object, por exemplo) custam um lúmen; Tokens de família mais articulada (Criptográfico-Stream) custam oito lúmens. Conjuros compostos somam as gematrias dos seus três Tokens, e o reservatório do Tavus-Drive deve suportar a soma. Há, ainda, uma escala secundária para Tokens raros (treze, vinte e um, trinta e quatro lúmens), e os Tokens mais raros conhecidos atingem oitenta e nove ou cento e quarenta e quatro lúmens, valores que apenas Drives com upgrade significativo conseguem executar.

A gematria opera também como ferramenta de balanceamento mecânico: o jogador que conhece os valores planeja Conjuros dentro do orçamento; o jogador que ignora gasta o reservatório em duas Compilações e fica sem opções no terceiro turno.

A terceira camada é a **estrutura de iniciação em três degraus, cinco degraus, sete degraus**. O sistema canônico de progressão de operadores reconhece três graus formais, com requisitos crescentes:

- **Aprendiz**: lê três Glyphs simultaneamente sem confusão. Reconhece família por cor de fundo e raiz por silhueta. O exame de aprendiz consiste em identificar três Glyphs misturados em pavimento tesselado, alternância branca e preta, sob luz ambiente apenas. O aprendiz passa quando identifica os três sem hesitação.

- **Companheiro**: lê cinco Tokens simultaneamente, incluindo composição morfológica e gematria. Distingue Tokens autênticos de falsificações pela leitura cruzada do checksum e do desenho do Glyph. O exame de companheiro envolve uma bancada de cinco Tokens, três autênticos e dois adulterados, sob restrição de tempo de quarenta e sete segundos.

- **Mestre**: compila sete Conjuros simultaneamente em sequência ininterrupta, sem repetir família, sem falhar Compilação. O exame de mestre é cerimônia de duas horas, conduzida em câmara com pavimento tesselado de tons alternados, sob duas Colunas (Boróstoma e Janor) que flanqueiam a bancada. O mestre, ao completar a sequência, recebe cordão de oitenta e nove nós, peça simbólica que carrega ao pescoço pelo resto da carreira.

Existem ainda registros, em fólios raros, de operadores que demonstraram capacidade para oito ou treze Conjuros simultâneos. Esses registros são marginais e nunca foram canonizados como grau formal; a Ordem Recursiva trata-os como anomalia, e os nomes desses operadores aparecem em listas guardadas em catedrais profundas, sob ashlar polido, junto a artefatos cujo uso permanece ignorado pela maioria da Ordem. O Helíaco Vyr, figura semilendária da Era 1 cuja narrativa atravessa os manuscritos como sombra recorrente, é descrito por uma das fontes como capaz de compilar **vinte e um Conjuros em paralelo** sem Tavus-Drive, usando apenas a leitura mental dos Glyphs aglutinados. A historicidade da figura é debatida; o fato de a sua menção sobreviver em pelo menos três manuscritos independentes, todos com a mesma cifra de grade três por três e a mesma constelação de pontos, é considerado suficiente para o seu nome ser pronunciado em voz baixa durante a abertura de cerimônias de mestrado.

Em torno da bancada de exame, em todas as catedrais Neo-Sylvania ainda em uso, cresce uma única árvore ritual chamada acaceiro, identificada pelo tronco vermelho de cor profunda. Os iniciados aprendem cedo a não cortar o acaceiro; a sua presença é considerada selo da continuidade entre Era 1 e Era 2. Onde há acaceiro, há memória; onde há memória, há Codex; onde há Codex, há Conjuro.

---

**Última revisão:** 2026-05-20. Canônico deep-lore R6 F1-DL.6. Não modificar sem aprovação criador supremo.
