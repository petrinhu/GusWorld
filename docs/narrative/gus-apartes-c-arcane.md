# Apartes de fanboy do Gus (C-Arcane)

> **APROVADO pelo líder, uma a uma, 2026-07-17. CANON.**
> Qualquer mudança futura exige nova autorização explícita dele (regra: tudo que define o Gus original passa por ele). Não misturar com o conteúdo já aprovado de `comic-reliefs.md`.

**Momento:** party andando pelo mapa, fora de combate, sem inimigo à vista. O Gus, entediado, solta "do nada". Só acontece **entre amigos** (com estranhos ele fica quieto, [[project_gus_voz_personalidade]]); **exceção: em combate ele solta mesmo com inimigo presente** (é o "Dragon", a face de líder). Detalhe completo em "Regras de disparo" abaixo.

**Molde:** "Você sabia que [característica real do C-Arcane], por isso obviamente é melhor que [outra linguagem]?" A forma varia, o espírito não.

**Regra inegociável de redação (revisada por ordem do líder, 2026-07-17):** o Gus é um fanboy de 11 anos, não um mentiroso. A régua separa **fato** de **juízo**:

- **Fato: proibido mentir.** Afirmação sobre como a linguagem funciona ("Óxido tem coletor de lixo") precisa ser verdadeira e ter fonte. Fato falso **não entra**, nunca.
- **Juízo: livre.** Achar a sintaxe feia, confusa, exagerada ou preguiçosa é **opinião**, e opinião de fanboy não é mentira. Um moleque de 11 anos surtando com o `::<>` é caracterização, não falsidade.
- **A ponte entre os dois:** a **queixa tem que existir de verdade no mundo real** (ser reclamação conhecida ou documentada). O Gus não inventa dor alheia, ele só julga com veneno a dor que já existe.

Cada aparte abaixo marca o que é **Fato** (com fonte) e o que é **OPINIÃO do Gus** (com a queixa real que a sustenta).

**Tom:** implicância com carinho fraterno. Nunca desprezo, nunca arrogância cruel. O alvo faz a carinha de "de novo isso", não sai magoado.

**Convenção diegética** (`comic-reliefs.md`): as linguagens do mundo são C-Arcane, Pythia, Óxido e Asmódico. Os fatos reais (C, Python, Rust, Assembly) estão transpostos, nunca citados por nome real no diálogo.

| Linguagem | Real | Quem defende |
|---|---|---|
| C-Arcane | C / C++ | Gus |
| Pythia | Python | Cauã, Jaci |
| Óxido | Rust | Iara, Linda |
| Asmódico | Assembly | Bento (e Dante) |

---

## Regras de disparo

Decisões do líder, 2026-07-17.

- **Gatilho social:** fora de combate o aparte só dispara **entre amigos** (só a party na cena; com estranhos o Gus fica quieto). **Exceção:** em combate ele solta mesmo com inimigo presente (é o "Dragon", a face de líder).
- **Frequência no mapa:** RARO, ~1 a cada **8-10 minutos** andando, chance baixa + trava de tempo mínimo. **Sorteio sem repetição (shuffle bag):** uma frase só volta a ser elegível quando TODAS já saíram.
- **Frequência em batalha:** até 1x a cada X minutos (**X ainda a definir com o líder**).
- **Reação do alvo:** **80% = só a carinha 😑** (silêncio é a piada); **20% = REVIDE** (o alvo responde). Lote de revides = **próxima tarefa** (munição já guardada: a honestidade da Iara na A28, "o embrulho impede ignorar erro, que é exatamente o que C-Arcane permite").
- Frases de tom "mais aula" (as que soam explicativas) o Gus pode soltar no vazio, sem exigir reação elaborada do alvo.

---

## Alvo: Pythia (Cauã, Jaci)

### A1
> "Você sabia que o interpretador da Pythia é um programa escrito em C-Arcane? Toda vez que você conjura, Cauã, tem um C-Arcane ali embaixo fazendo o trabalho pesado. De nada."

**Reação:** Cauã. **Fato:** o CPython (interpretador de referência do Python) é implementado em C; o núcleo só precisa de um compilador C pra ser construído. **Fonte:** Python Developer's Guide, "Setup and Building" ("The core CPython interpreter only needs a C compiler to be built").

### A2
> "As partes rápidas da Pythia, as de fazer conta pesada, já são C-Arcane compilado por baixo. A Pythia só grita o nome delas e recebe o crédito."

**Reação:** Jaci. **Fato:** o núcleo do NumPy é código C pré-compilado; a "vetorização" existe justamente porque o laço acontece em C otimizado, não em Python. **Fonte:** NumPy docs, "What is NumPy" ("speedily executed by pre-compiled C code", "at near-C speeds").

### A3
> "Você sabia que a Pythia recolhe o lixo dela quando ELA acha que é hora, não quando você precisa? C-Arcane nunca para no meio pra faxinar. Você limpa a sua sujeira e pronto."

**Reação:** Cauã. **Fato:** o CPython usa contagem de referências mais um coletor cíclico que dispara automaticamente por limiares de alocação (`gc.set_threshold`), podendo rodar em momento não escolhido pelo programador. C não tem coletor de lixo nem runtime que interrompa. **Fonte:** Python docs, módulo `gc`; Wikipedia "C (programming language)" ("minimal runtime support", coleta de lixo só via bibliotecas externas).

### A4
> "Cada passo do seu feitiço em Pythia é lido por outro programa antes de acontecer. O meu já nasceu pronto. É por isso que eu chego antes, Cauã, não é sorte."

**Reação:** Cauã. **Fato:** Python executa via interpretador (bytecode lido em tempo de execução); a própria documentação do NumPy recomenda evitar laços explícitos em Python porque o trabalho por elemento passa pelo interpretador, e delega a C compilado. **Fonte:** NumPy docs, "What is NumPy" (vetorização); Python Developer's Guide.

### A5
> "Pra Pythia nascer, ela precisa de um compilador de C-Arcane. Pra C-Arcane nascer, precisa de um compilador de C-Arcane. Vê a diferença? Uma delas se vira sozinha."

**Reação:** Jaci. **Fato:** o núcleo do CPython é construído com um compilador C, e nada além disso; C é autossuficiente (compiladores C são escritos em C). **Fonte:** Python Developer's Guide, "Setup and Building".

### A6
> "Sabia que pra rodar Pythia na máquina de alguém, a pessoa precisa ter a Pythia instalada? O meu vira um programa e acabou. Ninguém precisa ME INSTALAR."

**Reação:** Cauã. **Fato:** programas Python dependem do interpretador presente na máquina alvo; programas C compilam para binário nativo, com suporte de runtime mínimo. **Fonte:** Python Developer's Guide; Wikipedia "C (programming language)" ("minimal runtime support").

---

## Alvo: Óxido (Iara, Linda)

### A7
> "Você sabia que o Óxido leva a biblioteca inteira dentro de cada feitiço, mesmo o feitiço que só diz 'oi'? Pra ele emagrecer, você tem que arrancar pedaço dele. O meu já nasce magro."

**Reação:** Iara. **Fato:** a `libstd` do Rust é ligada estaticamente e otimizada para velocidade, não para tamanho; reduzir binário exige remover `core::fmt` e, no limite, ir de `#![no_std]`. **Fonte:** repositório `min-sized-rust` (johnthagen), seção sobre `libstd`, `core::fmt` e `no_std`.

### A8
> "O layout de uma struct em Óxido pode mudar a cada vez que ele compila. A cada vez, Iara. Quando o Óxido quer conversar com o resto do mundo sem se perder, ele tem que pedir licença e falar C-Arcane. Tem um modo especial só pra isso."

**Reação:** Iara. **Fato:** Rust não garante layout estável (`repr(Rust)`: "Type layout can be changed with each compilation"), e o `repr(C)` existe explicitamente para interoperar com a linguagem C. **Fonte:** The Rust Reference, "Type Layout".

### A9
> "O Óxido tem um modo especial pra falar C-Arcane. C-Arcane não tem modo especial pra falar Óxido. Pensa nisso com carinho, Linda."

**Reação:** Linda. **Fato:** `repr(C)` é a ponte de FFI do Rust; a ABI do C é a interface comum entre linguagens, e não existe recíproca em C. **Fonte:** The Rust Reference, "Type Layout" ("The `C` representation is designed... for creating types that are interoperable with the C Language").

### A10
> "Quando o seu programa em Óxido pede memória ao sistema, ele pede através da biblioteca do C-Arcane. Todo dia. Educadamente..."

**Reação:** Iara. **Fato:** o alvo tier 1 `x86_64-unknown-linux-gnu` depende de glibc (a biblioteca C do GNU), versão 2.17 ou superior. **Fonte:** Rust, "Platform Support" (`x86_64-unknown-linux-gnu`, "glibc 2.17+").

### A11
> "Você sabia que tem gente que desistiu do Óxido só de esperar ele compilar? Eu não inventei isso, Linda. Perguntaram, e eles responderam..."

**Reação:** Linda. **Fato:** na Rust Compiler Performance Survey 2025, 45% dos que pararam de usar Rust citaram tempo de compilação como um dos motivos; 55% esperam mais de 10 segundos por rebuild incremental. **Fonte:** Rust Blog, "Rust Compiler Performance Survey 2025 Results".

### A12
> "Justiça pro Óxido: ele não tem faxineiro de memória nenhum, resolve tudo na hora de compilar, e nisso ele tá certo. O preço é o compilador dele reclamar mais que o Bento num domingo. Eu prefiro que reclamem de mim depois."

**Reação:** Iara (quase um elogio, ela desconfia). **Fato:** Rust não usa coletor de lixo; a memória é gerida por ownership verificado em tempo de compilação, sem custo em runtime. **Fonte:** The Rust Programming Language (o livro), cap. 4.1 "What is Ownership?" ("Memory is managed through a system of ownership with a set of rules that the compiler checks"; "None of the features of ownership will slow down your program while it's running").

---

## Óxido, lote 2: sintaxe e ergonomia

> **APROVADO pelo líder, uma a uma, 2026-07-17. CANON.**
> Lote pedido pelo líder com a régua afrouxada: aqui o Gus implica com **sintaxe e ergonomia**, e implicar com sintaxe é **opinião**. Toda queixa abaixo existe de verdade no mundo real (é reclamação conhecida ou documentada); o veneno é do Gus.

### A22
> "Iara, sabia que aquela coisa tem nome OFICIAL? Turbofish. Peixe-turbo. Não é apelido de zoeira que inventaram pra zoar vocês, tá escrito na documentação da linguagem." (pausa) "Vocês precisaram enfiar um PEIXE no meio do feitiço pra ele entender que o número é um número. Os dois pontinhos são as bolhinhas que ele solta." (pausa maior) "Eu não durmo direito desde que descobri isso."

**Reação:** Iara. **Fato (verdadeiro):** a sintaxe `::<>` para argumentos genéricos em expressões é chamada oficialmente de "turbofish" na referência da linguagem, e o `::` existe pra desambiguar do operador "menor que". Exemplo real: `(0..10).collect::<Vec<_>>()`. **Fonte:** The Rust Reference, "Paths" ("The `::` token is required before the opening `<` for generic arguments to avoid ambiguity with the less-than operator. This is colloquially known as 'turbofish' syntax"). **Queixa real que sustenta:** o turbofish é a estranheza sintática mais famosa do Rust, a ponto de ter site próprio dedicado a ela. **OPINIÃO do Gus:** que peixe em código é sinal de que algo deu muito errado.

### A23
> "Linda, por que tem uma aspa solta no meio do tipo de vocês? Aspa aberta e nunca fechada. Alguém esqueceu de terminar a frase e o resto do mundo achou que era proposital." (pausa) "E antes que você fale: eu fui ler o manual de VOCÊS. Ele chama a própria sintaxe de 'meio incomum'. MEIO, Linda."

**Reação:** Linda. **Fato (verdadeiro):** anotações de lifetime começam com apóstrofo (`&'a str`), e o livro oficial descreve a sintaxe nesses termos, admitindo que o conceito é estranho pra quem vem de fora. **Fonte:** The Rust Programming Language, cap. 10.3 "Validating References with Lifetimes" ("Lifetime annotations have a slightly unusual syntax: The names of lifetime parameters must start with an apostrophe (`'`)"; "Annotating lifetimes is not even a concept most other programming languages have, so this is going to feel unfamiliar"). **Queixa real que sustenta:** lifetimes são a barreira de entrada mais citada do Rust. **OPINIÃO do Gus:** que aspa sem par é erro de digitação promovido a recurso.

### A24
> "Vocês têm DOIS tipos de texto, Iara. Dois. Um que é dono do texto e outro que só tá olhando o texto de longe. Aí você escreve o feitiço com um e ele exige o outro, e você fica ali, convertendo texto em texto."

Iara abre a boca.

> "Não. Não me explica. Eu SEI que tem motivo. Texto é texto. Escreve a palavra e vai viver."

**Reação:** Iara. **Fato (verdadeiro):** Rust tem `String` (dona, na heap) e `&str` (empréstimo, fatia); a conversão entre os dois é frequente e apoiada por deref coercion. **Fonte:** The Rust Programming Language, cap. 4.3 "The Slice Type" ("The type of `s` here is `&str`: It's a slice pointing to that specific point of the binary"; passagem sobre deref coercion permitir passar `&String` onde se espera `&str`). **Queixa real que sustenta:** `String` vs `&str` é a dor clássica número 1 de iniciante em Rust. **OPINIÃO do Gus:** que uma coisa só deveria ter um tipo só. (E sim, o Gus tem `char*`, `const char*` e array de char. Ele não vê problema nenhum nisso. Ninguém aponte.)

### A25
> "Linda, eu vi o teu feitiço por cima do teu ombro. É um tipo dentro de outro tipo dentro de outro tipo. Parece aquela bonequinha russa que abre e tem outra dentro." (pausa) "Eu abri três camadas e ainda não tinha chegado no número. Ele tava lá no fundo. Um número. Sozinho. Com medo."

**Reação:** Linda. **Fato (verdadeiro):** para compartilhar um valor mutável entre threads, o padrão do livro oficial é literalmente `Arc<Mutex<T>>`, com `Arc::new(Mutex::new(0))` e acesso via `counter.lock().unwrap()`. **Fonte:** The Rust Programming Language, cap. 16.3 "Shared-State Concurrency" (exemplo do contador com `Arc<Mutex<i32>>`). **Queixa real que sustenta:** o empilhamento de tipos aninhados é meme conhecido na comunidade. **OPINIÃO do Gus:** que embrulho demais é desconfiança do próprio código.

### A26
> "Por que IMPRIMIR precisa de exclamação, Iara? Escrever 'oi' na tela é motivo pra GRITAR? Eu peço com educação e o meu escreve. No de vocês você tem que berrar com a linguagem pra ela obedecer."

Iara, sem levantar os olhos do painel:

> "É macro. A exclamação existe porque ele confere o que você escreveu ANTES de compilar. O teu deixa você mentir o formato e só descobre na cara do usuário."

Silêncio de três passos.

> "...tá. Essa foi boa."

**Reação:** Iara (é dela o único aparte em que ela devolve e ganha). **Fato (verdadeiro), os dois lados:** `println!` é macro e usa a mesma sintaxe de `format!`, o que permite validar a string de formato e os argumentos em tempo de compilação (impossível numa função comum). Do outro lado, em C, se a especificação de conversão do `printf` não casa com o argumento, o comportamento é indefinido pelo padrão da linguagem; a checagem existe só como extensão do compilador (`-Wformat` no GCC/Clang), não como garantia da linguagem. **Fonte:** Rust std docs, `std::println!` ("This macro uses the same syntax as `format!`"); padrão C (comportamento indefinido em conversão incompatível no `printf`) e a flag `-Wformat` do GCC/Clang. **Nota de tom:** este é o irmão do A12 e do A15. O Gus dá o braço a torcer quando o argumento é bom, e é isso que impede o personagem de virar chato.

### A27
> "Linda, vocês têm uma EXPRESSÃO pronta pra isso. 'Brigar com o verificador'. Brigar. Com a ferramenta. A tua ferramenta é adversária, ela tem que ser CONVENCIDA a deixar você trabalhar." (pausa) "O meu nunca brigou comigo. O meu nem tá olhando. O meu confia em mim."

Três passos.

> "Às vezes confia demais. Mas isso é entre a gente."

**Reação:** Linda. **Fato (verdadeiro):** "fighting the borrow checker" é expressão consagrada na comunidade Rust, e o borrow checker aparece nas pesquisas oficiais como uma das áreas com que os usuários mais têm dificuldade. **Fonte:** Rust Blog, "2023 Annual Rust Survey Results" (borrow checker entre as áreas com que "Rustaceans seem to struggle with the most", ao lado de async e traits/generics; 43% preocupados com o Rust ficar complexo demais). **Queixa real que sustenta:** a expressão existe justamente porque a experiência é comum. **OPINIÃO do Gus:** que ferramenta que discute não é ferramenta. **Nota:** o remate ("às vezes confia demais") é o Gus admitindo segfault sem dizer a palavra. Não cortar, é o que salva a frase de ser arrogante.

### A28
> "Toda linha de vocês termina desembrulhando alguma coisa. Desembrulha, desembrulha, desembrulha. É aniversário todo dia na casa de vocês." (pausa) "E quando cansa de desembrulhar, vocês põem uma interrogação no fim e fingem que resolveram. Pra que embrulhar, Iara, se você ia abrir na hora mesmo?"

**Reação:** Iara. **Fato (verdadeiro):** operações falíveis em Rust devolvem `Result<T, E>`; `.unwrap()` e `.expect()` extraem o valor e entram em pânico no erro, e o `?` propaga o erro pra cima. **Fonte:** The Rust Programming Language, cap. 9.2 "Recoverable Errors with Result" (`unwrap`, `expect`, operador `?`). **Queixa real que sustenta:** o ritual de desembrulhar (e o `.unwrap()` espalhado) é reclamação corrente e alvo de piada na própria comunidade. **OPINIÃO do Gus:** que embrulho que sempre se abre é burocracia. (O contra-argumento honesto, que o Gus não vai dar, é que o embrulho impede o programador de ignorar o erro em silêncio, que é exatamente o que C permite. Fica de munição pra Iara num lote futuro.)

### A29
> "Sortear um número, Linda. UM número. A caixa de ferramentas que vem junto com o Óxido não sorteia número. Vocês precisam baixar o sorteio de um estranho da internet." (pausa) "E junto do sorteio vem um monte de outra coisa que ninguém aí leu. Eu tô só falando."

**Reação:** Linda. **Fato (verdadeiro):** a std do Rust não oferece geração de números aleatórios estável (o módulo `std::random` é experimental); a std é declaradamente mínima e aponta pro ecossistema em crates.io, onde vive o `rand`. **Fonte:** Rust std docs, página inicial ("a set of minimal and battle-tested shared abstractions for the broader Rust ecosystem", com link pra crates.io; `std::random` listado como Experimental). **Queixa real que sustenta:** "std pequena demais" e o volume de dependências transitivas de um projeto simples são reclamações correntes. **OPINIÃO do Gus:** que precisar da internet pra jogar um dado é vexame. **Caveat honesto (não vai pra fala):** o `rand()` do C existe, mas é de qualidade notoriamente ruim. O Gus não sabe disso ou finge que não. Se o líder quiser, isso vira contra-ataque da Linda depois. **Sem número:** a queixa de "monte de dependência" ficou qualitativa de propósito, não achei fonte primária pra cravar quantidade.

---

## Alvo: Asmódico (Bento)

### A13
> "Você sabia que o seu Asmódico só serve pra UMA máquina? Troca a máquina e você reescreve tudo, do zero, na mão. Eu recompilo e vou tomar café."

**Reação:** Bento (encara em silêncio). **Fato:** cada linguagem assembly é específica de uma arquitetura (x86, ARM), enquanto linguagens de alto nível são portáveis entre arquiteturas. **Fonte:** Wikipedia, "Assembly language" ("each assembly language is specific to a particular computer architecture such as x86 or ARM").

### A14
> "O sistema inteiro embaixo dos nossos pés foi reescrito em C-Arcane justamente pra poder trocar de máquina sem morrer. Não foi capricho, Bento. Foi sobrevivência."

**Reação:** Bento. **Fato:** o kernel do Unix foi extensivamente reimplementado em C na Version 4 Unix (nov/1973), o que viabilizou sua portabilidade. **Fonte:** Wikipedia, "C (programming language)" ("At Version 4 Unix, released in November 1973, the Unix kernel was extensively re-implemented in C").

### A15
> "O núcleo que roda tudo é C-Arcane, com Asmódico só nos cantinhos onde não tem outro jeito." (pausa) "E olha, Bento: esses cantinhos são MESMO seus. Eu não saberia fazer."

**Reação:** Bento (é o aparte em que ele quase sorri). **Fato:** o kernel Linux é escrito em C (dialeto GNU, migrado a C11 em 2022) mais assembly da arquitetura alvo em partes específicas. **Fonte:** Wikipedia, "Linux kernel" ("written in a special C programming language supported by GCC... and assembly language... of the target architecture").

### A16
> "Todo C-Arcane meu vira Asmódico no fim, sabia? A diferença é que eu não escrevo a versão final na mão. Eu descrevo, e ela aparece..."

**Reação:** Bento. **Fato:** compiladores C traduzem para código de máquina da arquitetura alvo (assembly/machine code); assembly é a representação simbólica direta desse código. **Fonte:** Wikipedia, "Assembly language" (relação assembly/código de máquina) e "C (programming language)" (compilação para múltiplas plataformas).

---

## Genéricas (sem alvo fixo, a party inteira sofre)

### A17
> "Você sabia que C-Arcane não tem faxineiro, não tem porteiro e não tem síndico? Não tem nada rodando atrás de você. Se travou, fui eu que errei. Isso é liberdade, gente!"

**Reação:** party inteira. **Fato:** C tem suporte de runtime mínimo e não fornece coleta de lixo nativa (só via bibliotecas externas), logo não há pausa de runtime imprevisível. **Fonte:** Wikipedia, "C (programming language)" ("all with minimal runtime support"; "object orientation and garbage collection are provided by external libraries").

### A18
> "O padrão inteiro do C-Arcane tem quarenta e três palavras reservadas. Quarenta e três! Dá pra decorar num dia chuvoso e saber a linguagem inteira. Vocês nunca vão saber a de vocês inteira."

**Reação:** Iara e Cauã, simultaneamente. **Fato:** o padrão atual de C define 43 palavras reservadas. **Fonte:** Wikipedia, "C syntax" ("The following words are reserved, not allowed as identifiers, of which there are 43").

### A19
> "C-Arcane escrito direito compila em quase qualquer máquina que exista. Não é que ele seja mágico. É que ele chegou primeiro em todas elas."

**Reação:** Linda. **Fato:** um programa C conforme o padrão e escrito com portabilidade em mente compila para uma ampla variedade de plataformas e sistemas operacionais com poucas mudanças. **Fonte:** Wikipedia, "C (programming language)".

### A20
> "Os quinhentos computadores mais potentes que existem rodam um sistema escrito em C-Arcane. Os quinhentos. Não é opinião minha, é contagem!"

**Reação:** Jaci. **Fato:** o kernel Linux é escrito em C, e todos os 500 supercomputadores da lista TOP500 rodam Linux. **Fonte:** Wikipedia, "Linux kernel" ("all of the world's 500 most powerful supercomputers run on Linux").

### A21
> "C-Arcane é mais velho que os nossos pais e ainda tá embaixo de tudo que vocês usam. Toda linguagem nova promete matar ele. Depois liga pedindo carona."

**Reação:** party. **Fato:** C foi desenvolvido entre 1972 e 1973 (mais de 50 anos) e segue como base de kernels, libcs e interpretadores de outras linguagens (CPython em C; Rust dependendo de glibc no alvo tier 1). **Fonte:** Wikipedia, "C (programming language)"; Python Developer's Guide; Rust "Platform Support".

---

## A frase RARA (o bilhete dourado)

**Raridade:** a mais rara do jogo. A ÚNICA em que o Gus critica o C-Arcane. Aprovada pelo líder na forma diegética abaixo.

> "Dá pra fazer classe em C-Arcane puro, sabia? Não é BEM classe... é struct com ponteiro de função. O C-Arcane Major só deixou fácil pra quem tem preguiça."

**Verificação técnica:** correta. Orientação a objetos em C puro se faz com struct mais ponteiro de função: é exatamente como o GObject (base do GTK) implementa classes, herança (por embutir a struct do pai como primeiro membro) e despacho dinâmico (tabela de métodos virtuais), sem qualquer suporte da linguagem. O kernel Linux usa o mesmo padrão nas structs de operações. **Fonte:** GObject docs, "Concepts" ("All class structures must contain as first member a `GTypeClass` structure"; "The interface structure is expected to contain the function pointers of the interface methods").

**Nota de canon (decisão do líder, 2026-07-17):** AMB-01 resolvida. O líder criou o nome diegético do C++: **C-Arcane Major**. A frase entra na versão diegética, mantendo a convenção do resto do doc (nenhum nome real de linguagem no diálogo). Termo canônico novo: **C-Arcane Major = C++**.

---

## Log de ambiguidades

| ID | Ambiguidade | Opções | Status |
|---|---|---|---|
| AMB-01 | Nome real ("C", "C++") na frase rara vs convenção diegética do doc | (a) manter verbatim do líder, (b) variante diegética genérica, (c) usar `C-Arcane Major`, nome diegético do C++ | **RESOLVIDA: (c).** O líder criou `C-Arcane Major` como nome diegético canônico do C++. Frase rara entra na forma diegética. |
| AMB-04 | O lote 2 usa "manual de vocês" e "documentação" na boca do Gus (A22, A23). Isso pressupõe que existe documentação escrita das linguagens no mundo diegético | (a) ok, é coerente com magia = software, (b) trocar por "os próprios magos do Óxido admitem" | **Mantida como (a):** coerente com Pillar 1 (magia = software). Linha registrada aqui como **confirmar com o líder** se ele quiser revisitar. |
| AMB-02 | Frequência de disparo dos apartes no mapa | (a) cooldown por tempo, (b) por transição de mapa, (c) pool aleatório sem repetição | **RESOLVIDA.** Ver "Regras de disparo": shuffle bag sem repetição + cooldown ~8-10 min no mapa. Frequência em batalha ainda com X a definir. |
| AMB-03 | A21 é reserva ou entra fixo (fecha em 21, número Fibonacci) | (a) reserva, (b) fixa | **RESOLVIDA: (b) fixa.** Pool fecha em 21 + lote 2 (A22-A29) = 29 frases totais. |

## Pendência de implementação (UI)

Requisito do líder (não sugestão): frases longas dos apartes precisam aparecer na tela **sem cortar/clipar o contêiner e sem reduzir a fonte** a ponto de dificultar a leitura. Repassar para implementação de UI/HUD (glintfx) quando o sistema de apartes for construído.

## Frases descartadas (não passaram na verificação)

| Frase pretendida | Motivo do descarte |
|---|---|
| "Pythia só descobre que o feitiço tá errado quando ele já tá no ar" (tipagem dinâmica) | Verdadeira no geral, mas não achei fonte primária fetchada nesta sessão; e é imprecisa (existem verificadores estáticos opcionais). Fora até ter citação. |
| "Óxido demora porque tem coletor de lixo" | **FALSO.** Rust não tem coletor de lixo, usa ownership verificado em compilação. É exatamente o erro do rascunho queimado. Substituída pela A12, que afirma o contrário e dá o crédito honesto. |
| "Binário do 'olá mundo' em Óxido tem 300 KB" | Número não confirmado na fonte (o `min-sized-rust` documenta as causas do tamanho, não crava o valor). Reescrita como A7, sem número. |
| "Nenhuma máquina do mundo roda sem C" | Exagero indemonstrável (existe firmware em assembly puro, e outras stacks). Reescrita como A19, "quase qualquer máquina". |
| "C é a linguagem mais rápida que existe" | Indefensável como absoluto (depende de compilador, alvo, código). Velocidade pura ficou de fora por ordem do líder de variar as características. |
| "Compilador do C-Arcane é instantâneo" | Relativo demais pra afirmar sem benchmark citável. Cortada. |

### Descartes do lote 2 (sintaxe e ergonomia)

| Frase pretendida | Motivo do descarte |
|---|---|
| "Cargo baixa duzentas dependências pra escrever 'oi' na tela" | Queixa **real** (dependency bloat é reclamação corrente), mas o **número é invenção minha**. Não achei fonte primária que cravasse quantidade. Reescrita como A29, qualitativa ("um monte de outra coisa"), ancorada num fato citável: a std do Rust não tem sorteio de número estável. |
| "O turbofish foi inventado como piada e ficou" | **Fato falso.** A sintaxe existe por necessidade técnica (desambiguar do operador `<`); o que é coloquial é o NOME. A A22 diz a verdade e é mais engraçada por isso: o absurdo é o nome ser oficial, não a sintaxe ser piada. |
| "Óxido é lento pra escrever porque a sintaxe é feia" | Juízo sem queixa real específica por baixo. Vago, é xingamento e não piada. Fora pela régua nova ("a queixa tem que existir de verdade"). |
| "`String` e `&str` existem porque quem fez o Óxido não pensou direito" | Vira **afirmação factual sobre intenção de projeto**, e falsa: a separação é deliberada (posse vs empréstimo). O Gus pode achar RUIM (A24), não pode dizer que foi descuido. |
| "Óxido não tem herança, então não dá pra fazer nada direito" | Verdadeiro que não há herança de struct, mas o aparte virava aula e o Gus perde a graça explicando. Além disso é frágil: o C dele também não tem. Autogol. Cortada. |
| "Ninguém entende lifetime, nem quem escreveu" | Segunda metade é fato inventado sobre pessoas reais. Cortada; a A23 usa a admissão REAL do manual oficial ("slightly unusual"), que é mais forte. |
| "`.unwrap()` é o jeito certo de programar em Óxido" | Falso e injusto (o livro oficial recomenda `expect`, e `?` pra propagar). A A28 implica com o RITUAL sem afirmar isso. |
