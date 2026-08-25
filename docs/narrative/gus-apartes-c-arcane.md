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
| C-Arcane | C | Gus |
| C-Arcane Major | C++ | Gus (só na frase rara) |
| Pythia | Python | Cauã, Jaci |
| Óxido | Rust | Iara, Linda |
| Asmódico | Assembly | Bento (e Dante) |

---

## Regras de disparo

Decisões do líder, 2026-07-17.

- **Gatilho social:** fora de combate o aparte só dispara **entre amigos** (só a party na cena; com estranhos o Gus fica quieto). **Exceção:** em combate ele solta mesmo com inimigo presente (é o "Dragon", a face de líder).
- **Frequência no mapa:** RARO, ~1 a cada **8-10 minutos** andando, chance baixa + trava de tempo mínimo. **Sorteio sem repetição (shuffle bag):** uma frase só volta a ser elegível quando TODAS já saíram.
- **Frequência em batalha:** até 1x a cada X minutos (**X ainda a definir com o líder**).
- **Reação do alvo:** **~80% = só a carinha 😑** (silêncio é a piada); **~20% = REVIDE** (o alvo responde). Os 13 revides são CANON (ver seção "Revides" abaixo, aprovada um a um pelo líder em 2026-07-17).
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

---

# Revides

> **APROVADO pelo líder, um a um, 2026-07-17. CANON.**
> Os 13 revides abaixo foram aprovados frase a frase, igual aos apartes. Qualquer mudança futura exige nova autorização explícita dele (regra: tudo que define o Gus original e a party passa por ele).

**O que é:** a resposta do alvo quando ele decide não engolir.

> ## ⚠ FREQUÊNCIA (CANON, vale para TODOS os 13 revides)
>
> O revide é **EVENTUAL**. Ele **dispara em ~20% das vezes** (1 a cada 5, decisão do líder). Nos outros **~80% o alvo NÃO revida: só faz a carinha 😑** e o silêncio é a piada. O revide é a exceção rara; a carinha é a regra. Um revide que aparecesse sempre mataria a piada do silêncio.

**Um revide SOB MEDIDA por frase** (decisão do líder): não são respostas genéricas sorteadas. Das 29 frases, **13 receberam revide** (as provocativas o bastante pra merecer resposta). As outras 16 o Gus solta no vazio: são as de tom "mais aula", as que já vêm com concessão embutida (A12, A15, A26, A27) ou as que ninguém no mundo se dá ao trabalho de responder.

**Régua de honestidade: a MESMA dos apartes.** Fato mentiroso não entra, nunca. Cada revide abaixo tem fonte fetchada nesta sessão. O que é juízo está marcado como juízo. Nenhum revide cita número que eu não tenha lido na fonte.

**TOM (ordem literal do líder): "mesmo tom do Gus".** Implicância afetuosa e verdadeira, registro caloroso e brincalhão. **NÃO é sermão, não é aula, não é ataque cruel.** O fato pode ser pesado; a entrega é leve. Revide que humilha quebra o tom e sai. Regra prática que usei: o revide sempre concede alguma coisa antes de cortar, e nunca sobra em cima do Gus depois que ele cala.

**Reação do Gus, por personagem** (dinâmicas de `party.md`, decisão do líder):

| Alvo | Dinâmica canônica | Reação do Gus ao revide |
|---|---|---|
| **Iara** | "Legibilidade vs elegância opaca. Convivência respeitosa; diferença permanente." | **Engole seco.** Não responde. Anda mais rápido. |
| **Cauã** | "Banter constante; rivalidade fraterna; aliança operacional sólida." | **Teima e revida de novo.** Perde o segundo round também. Não admite. |
| **Bento** | "Modernidade vs tradição. Aliança filosófica em ato 3." | **Respeita e cala.** É o único com quem ele não teima. |
| **Jaci** | "Pessoa vs sistema. Aliança emocional mais profunda." | **Quase pede desculpa.** Às vezes pede. |
| **Linda** | "Linguagem como arma. Aliança sólida; **Linda lê rápido**." | **Perde o CONTROLE da conversa** (decisão do líder, AMB-06 RESOLVIDA). Com a Iara ele perde o argumento; com a Linda ele perde o controle, que pra ele é pior. Cara disso: silêncio de três passos calculando, uma saída técnica meia-boca, e muda de assunto rápido demais. Ele guarda o golpe. |

---

## R-01 (responde a **A1**, o interpretador da Pythia é escrito em C-Arcane)

**Revida:** Cauã.

> "De nada mesmo, cara. Sério. Obrigado por fazer o chão."
>
> (pausa)
>
> "Aí eu subo no teu chão e faço em cinco linhas o que você faz em cinquenta. Você construiu a escada e ficou embaixo dela, segurando, pra ninguém subir errado."

**Reação do Gus (teima e revida de novo):**

> "Cinco linhas que rodam DEVAGAR."

> "Cinco linhas que já tão prontas enquanto você ainda tá compilando."

Gus abre a boca. Fecha. Passa o resto do corredor montando a resposta perfeita, que ele vai soltar em dois mapas de distância, quando ninguém mais lembrar do assunto.

**Fato (verdadeiro), os dois lados:** a documentação oficial do Python afirma que "Programs written in Python are typically much shorter than equivalent C, C++, or Java programs", e que "Python is an interpreted language, which can save you considerable time during program development because no compilation and linking is necessary", citando o ciclo "write/compile/test/re-compile" como lento. **Fonte:** Python Tutorial, "Whetting Your Appetite". **Do lado do Gus:** "rápido de escrever, devagar de rodar" é o trade honesto e já está ancorado na A2 (o núcleo do NumPy é C pré-compilado, "at near-C speeds"). **Por isso este revide não fura o eixo do Gus, reforça:** o Cauã ganha no relógio do programador, o Gus ganha no relógio da máquina. Os dois estão certos, e é isso que faz a briga durar o jogo inteiro.

## R-02 (responde a **A5**, "uma delas se vira sozinha")

**Revida:** Jaci.

> "Se vira sozinha."
>
> (pausa)
>
> "Ontem você pediu o décimo item de uma lista de nove, Gus."
>
> (pausa)
>
> "O meu teria parado na hora e falado 'não tem'. Com o nome do arquivo e a linha. O teu te entregou o que tava do lado. Não avisou nada. Você usou aquilo."

**Reação do Gus (quase pede desculpa):**

Ele abre a boca. Fecha.

> "...eu ia conferir depois."

(três passos)

> "Eu não conferi."

**Fato (verdadeiro), os dois lados:** em Python, índice fora do intervalo levanta `IndexError` ("Raised when a sequence subscript is out of range"), e exceção não tratada para a execução imprimindo um stack traceback com nome de arquivo e linha ("The file name... and line number are printed so you know where to look"). Em C, não existe checagem: "C provides no built-in protection against accessing or overwriting data in any part of memory and do not automatically check that data written to an array (the built-in buffer type) is within the boundaries of that array". **Fonte:** Python docs, "Built-in Exceptions" (`IndexError`); Python Tutorial, "Errors and Exceptions"; Wikipedia, "Buffer overflow". **Nota de tom:** é a Jaci, então o corte não é "você é burro", é "você se machucou e nem viu". É por isso que ele pede desculpa.

## R-03 (responde a **A6**, "ninguém precisa ME INSTALAR")

**Revida:** Cauã.

> "Ninguém precisa te instalar, é."
>
> (pausa)
>
> "Ninguém te avisa também."
>
> "Quando o meu quebra, ele escreve uma carta. Fala o arquivo, a linha, o que ele tentou fazer e por que não deu. Dá pra LER, Gus."

**Reação do Gus (teima e revida de novo):**

> "O meu também avisa!"

> "Duas palavras e o silêncio não é aviso, cara. É bilhete de despedida."

Gus fica ofendido pelo C-Arcane, não por ele. É diferente. Ele explica isso pra ninguém, em voz alta, por uns vinte segundos.

**Fato (verdadeiro), os dois lados:** Python imprime o contexto do erro "in the form of a stack traceback", com arquivo e linha. Em C, acesso inválido de memória vira um segmentation fault: o sistema envia `SIGSEGV` ao processo ("a signal called `SIGSEGV`... is sent to the offending process"), tipicamente encerrando o programa com a mensagem `Segmentation fault`, e a linguagem não dá diagnóstico nenhum (desreferenciar ponteiro nulo é comportamento indefinido em C). **Fonte:** Python Tutorial, "Errors and Exceptions"; Wikipedia, "Segmentation fault". **Nota:** "Falha de segmentação" é termo de máquina, não nome de linguagem, então passa na convenção diegética do doc.

## R-04 (responde a **A13**, "eu recompilo e vou tomar café")

**Revida:** Bento.

> "Vai."

(pausa longa. Uns oito passos. O Gus já tinha achado que acabou.)

> "Quando você voltar, quem escolheu cada passo do teu feitiço vai ter sido ele. Não você."

**Reação do Gus (respeita e cala):**

Ele não responde. Olha pro chão, anda mais três passos, e não solta mais nenhum aparte naquele mapa. É o único alvo com quem ele não teima.

**Fato (verdadeiro):** escrever em assembly (ou baixo nível) dá ao programador "greater visibility and control over processing details"; num compilador, essas escolhas são do otimizador. **Fonte:** Wikipedia, "Assembly language" (seção de uso atual). **Caveat honesto, e é por isso que a fala é essa e não outra:** a mesma fonte diz que "modern optimizing compilers are claimed to render high-level languages into code that can run as fast as hand-written assembly, despite some counter-examples". Ou seja: **o Bento NÃO pode dizer que é mais rápido**, seria mentira disfarçada de juízo. Ele diz que é ele quem DECIDE. Isso é verdade e é mais pesado. **Nota de tom:** o revide inteiro do Bento cabe em duas frases porque ele fala pouco. Se ficar maior, não é ele.

## R-05 (responde a **A16**, "todo C-Arcane meu vira Asmódico no fim")

**Revida:** Bento.

> "No fim."

(pausa)

> "E no começo. Antes do teu primeiro comando rodar, alguém já entrou, arrumou a casa e chamou o teu feitiço pelo nome."

(pausa)

> "E quando acaba, alguém apaga a luz."

**Reação do Gus (respeita e cala):**

> "...eu sabia disso."

(três passos)

> "Eu não tinha PENSADO nisso."

**Fato (verdadeiro):** o `crt0` "generally takes the form of an object file called `crt0.o`, often written in assembly language"; ele "performs any initialization work required before calling the program's main function"; e "after the main function completes the control returns to crt0, which calls the library function `exit(0)` to terminate the process". Ou seja: em C, antes do `main` e depois do `main`, quem está no comando é código de startup normalmente escrito em assembly. **Fonte:** Wikipedia, "crt0". **Nota:** este é o irmão exato da A16, e por isso é o revide mais barato de aprovar: o Gus abriu a porta ("no fim") e o Bento só apontou que o corredor tem duas pontas.

## R-06 (responde a **A17**, "se travou, fui eu que errei. Isso é liberdade!")

**Revida:** Iara.

> "É. Foi você que errou."

(ela não levanta os olhos do painel)

> "Sete de cada dez vezes, Gus. Não é chute meu, contaram. Quem fabrica as máquinas grandes passou ANOS contando os próprios buracos, e sete em cada dez era alguém mexendo em memória que não era dele."

(pausa)

> "Isso não é liberdade. Isso é estatística."

**Reação do Gus (engole seco):**

Ele não responde. Anda mais rápido que o normal por uns dez segundos, e ninguém comenta.

**Fato (verdadeiro):** "~70% of the vulnerabilities Microsoft assigns a CVE each year continue to be memory safety issues" (dado do MSRC, apresentado por Matt Miller em 2019); e, do lado do Chromium: "Around 70% of our high severity security bugs are memory unsafety problems (that is, mistakes with C/C++ pointers)". Duas organizações grandes, independentes, mesmo número. **Fonte:** Microsoft MSRC Blog, "A proactive approach to more secure code" (2019); Chromium Security, "Memory safety". **Caveat honesto (não vai pra fala):** o número é sobre bases de código C/C++ dessas organizações, não sobre "toda vulnerabilidade do mundo". A Iara diz exatamente isso ("quem fabrica as máquinas grandes... os PRÓPRIOS buracos") e nada além. **Este é o revide mais forte do lote e o mais perigoso pro tom:** ele só não vira sermão porque tem duas frases e um remate de piada seca. Não deixar crescer.

## R-07 (responde a **A19**, "C-Arcane escrito direito compila em quase qualquer máquina")

**Revida:** Linda.

> "Escrito direito."

(pausa)

> "Você ouviu o que você falou? A tua defesa começa com um 'se'."

(pausa)

> "Escreveram um documento, Gus. Gente que decide coisa grande. Diz que quem fabrica máquina consegue apagar CLASSES INTEIRAS de defeito trocando de linguagem. Pra uma que não deixa errar na memória."

(pausa)

> "Não citaram você pelo nome. Não precisou."

**Reação do Gus (perde o CONTROLE da conversa, AMB-06 canon):**

Três passos de silêncio, calculando.

> "...eles disseram que CONSEGUE. Não disseram que eu não consigo."

E ele muda de assunto rápido demais, o que entrega tudo.

**Fato (verdadeiro):** em 26 de fevereiro de 2024, o ONCD (Escritório do Diretor Nacional Cibernético da Casa Branca) publicou o relatório técnico "Back to the Building Blocks: A Path Toward Secure and Measurable Software", cujo comunicado "makes the case that technology manufacturers can prevent entire classes of vulnerabilities from entering the digital ecosystem by adopting memory safe programming languages". **Fonte:** ONCD, comunicado de imprensa do relatório técnico, 26/02/2024. **Caveat honesto (a fala respeita):** o trecho que eu verifiquei recomenda **adotar** linguagem memory-safe; ele **não proíbe nada** e **não cita C pelo nome** no trecho verificado. A Linda diz "não citaram você pelo nome" justamente porque é verdade.

**AMB-05 RESOLVIDA (decisão do líder, 2026-07-17).** A instituição real (ONCD/Casa Branca) vira, no mundo do jogo, um **órgão de padronização e segurança** (equivalente diegético de ISO/CERT): neutro, apolítico, que audita as linguagens rúnicas e publica "editais" de segurança que o jogador pode DESCOBRIR nos in-world-docs. O "documento que pede pro mundo parar de escrever coisa nova em C-Arcane" é um edital desse órgão. **Semente de canon a desenvolver (NÃO inventar a facção inteira agora):** existe um órgão assim; fica só o gancho, a ser detalhado depois em `factions.md` / `in-world-docs.md` com aprovação do líder. **A fala da Linda permanece com a instituição referida de forma vaga** ("gente que decide coisa grande"), agora **ancorada nessa semente** (o órgão existe, a vagueza é do ponto de vista dela, não buraco de canon).

## R-08 (responde a **A20**, "os quinhentos computadores mais potentes rodam C-Arcane")

**Revida:** Jaci.

> "Rodam."

(pausa)

> "Sabe a primeira imagem que fizeram de um buraco negro? Aquela coisa que ninguém no mundo tinha visto nunca, desde sempre?"

(pausa)

> "Montaram com a caixa de ferramentas da Pythia. Os números todos passaram por ela."

Gus abre a boca.

> "Eu sei. A parte de baixo da caixa é tua. Eu SEI, Gus."

(pausa)

> "Eu só queria que uma vez você contasse a história inteira. Não só a tua parte."

**Reação do Gus (quase pede desculpa, e desta vez pede):**

> "...desculpa."

(três passos)

> "A foto foi de vocês."

E ele fica quieto por muito tempo. É a maior pausa de aparte do jogo.

**Fato (verdadeiro), os dois lados:** o pacote `eht-imaging`, usado na reconstrução da primeira imagem de um buraco negro pelo Event Horizon Telescope (divulgada em 10/04/2019), tem NumPy no núcleo do processamento: "NumPy is at the core of array data processing used in this package"; "The efficient and adaptable n-dimensional array that is NumPy's central feature enabled researchers to manipulate large numerical datasets, providing a foundation for the first-ever image of a black hole". **Fonte:** numpy.org, case study "Black hole image". **A concessão da Jaci também é fato, e já está no doc:** o núcleo do NumPy é C pré-compilado (A2, fonte NumPy docs). Por isso ela pode dar o crédito ao Gus sem mentir. **Nota de tom:** este é o revide que define o lote. A Jaci não ganha da linguagem dele, ela ganha do JEITO dele. E o Gus não perde um argumento, ele percebe uma coisa. É o único revide que muda o personagem.

## R-09 (responde a **A21**, "C-Arcane é mais velho que os nossos pais")

**Revida:** Iara.

> "Velho é."

(pausa)

> "Sabe o que mais é velho? O buraco. O nada apontado."

(pausa)

> "O homem que inventou aquilo subiu num palco, já velho, e pediu desculpa pro mundo inteiro. Chamou de o erro de um bilhão."

(pausa)

> "O teu ainda tem o buraco, Gus. O meu não tem."

**Reação do Gus (engole seco):**

Ele não responde. Repete "um bilhão" baixinho, duas vezes, como quem confere uma conta.

A conta fecha.

**Fato (verdadeiro):** em 2009, Tony Hoare apresentou "Null References: The Billion Dollar Mistake", onde diz, verbatim: "I call it my billion-dollar mistake... I couldn't resist the temptation to put in a null reference, simply because it was so easy to implement. This has led to innumerable errors, vulnerabilities, and system crashes, which have probably caused a billion dollars of pain and damage in the last forty years." A citação está reproduzida **no livro oficial do Rust**, que apresenta Hoare como "the inventor of null" e afirma: "Rust does not have nulls, but it does have an enum that can encode the concept of a value being present or absent... `Option<T>`". Do lado do C: desreferenciar ponteiro nulo é comportamento indefinido. **Fonte:** The Rust Programming Language, cap. 6.1 "Defining an Enum"; Wikipedia, "Segmentation fault". **Caveat honesto (por isso a fala é redigida assim):** Hoare inventou a referência nula no ALGOL W, **não** no C. A Iara diz "o nada apontado" e "o teu ainda tem", e **nunca** diz que ele inventou o ponteiro nulo do C, o que seria falso. Segundo caveat: Rust só é livre de nulo no subconjunto **seguro** (ponteiro cru em `unsafe` pode ser nulo); a fala dela diz "o meu não tem" no registro de fanboy dela, que é o mesmo direito de juízo que o Gus tem. Se o líder quiser rigor absoluto aqui, troco por "o meu me obriga a olhar dentro antes de usar", que é fato puro e igualmente bom.

## R-10 (responde a **A22**, o turbofish, "eu não durmo direito desde que descobri isso")

**Revida:** Iara.

> "Enfiamos um peixe, sim. Porque o meu não sabia se aquele sinalzinho era 'menor que' ou não. A gente resolveu, deu nome, e ri da gente mesma. Tá tudo escrito."

(pausa)

> "O teu não sabe se uma PALAVRA é um nome ou um tipo, Gus. Ele tem que ir consultar uma tabela antes de conseguir LER a frase."

(pausa)

> "A gente pôs um peixe. Vocês puseram uma gambiarra. E não é apelido meu, não. É o nome que os TEUS deram."

**Reação do Gus (engole seco):**

> "...o peixe ainda é pior."

É a coisa mais fraca que ele já disse na vida. Ele sabe. Todo mundo sabe. Ninguém fala nada, e é pior ainda.

**Fato (verdadeiro), os dois lados:** o turbofish existe por necessidade técnica, pra desambiguar do operador "menor que" (já citado na A22: The Rust Reference, "Paths"). Do lado do C: a gramática de referência é context-sensitive, porque "classifying a sequence of characters as a variable name or a type name requires contextual information"; o exemplo clássico é `A * B;`, que "could either be multiplication or a declaration, depending on context"; e a solução consagrada, alimentar o parser de volta pro lexer, chama-se literalmente **"the lexer hack"**, ou seja, a gambiarra do analisador léxico, nome dado pela própria comunidade C. **Fonte:** Wikipedia, "Lexer hack". **Nota de tom:** funciona porque os DOIS lados perdem. A Iara não defende o peixe, ela admite o peixe e mostra que a casa dele também tem um. É o revide mais engraçado do lote e o menos cruel.

## R-11 (responde a **A25**, a bonequinha russa, "um número. Sozinho. Com medo.")

**Revida:** Linda.

> "Cada camada dessas é uma pergunta que o meu fez ANTES de rodar."

(pausa)

> "Dois feitiços teus mexendo no mesmo número na mesma hora compilam liso. Ficam bonitos. Rodam certo mil vezes. Na milésima primeira, não."

(pausa)

> "O meu nem compila. Ele para e me mostra o problema. Com o dedo."

(pausa)

> "O número não tá com medo, Gus. O número tá protegido. De você."

**Reação do Gus (perde o CONTROLE da conversa, AMB-06 canon):**

Silêncio.

(três passos)

> "...isso foi maldade."

E ele guarda. A Linda lê rápido: ela já sabia que ia ganhar essa antes de abrir a boca.

**Fato (verdadeiro):** "By leveraging ownership and type checking, many concurrency errors are compile-time errors in Rust rather than runtime errors"; e "rather than making you spend lots of time trying to reproduce the exact circumstances under which a runtime concurrency bug occurs, incorrect code will refuse to compile and present an error explaining the problem". O padrão `Arc<Mutex<T>>` que o Gus zoa na A25 é o do próprio livro oficial. **Fonte:** The Rust Programming Language, cap. 16 "Fearless Concurrency" e cap. 16.3 "Shared-State Concurrency". **Caveat honesto (a fala respeita, e é por isso que ela fala de COMPILAR e não de "bug nenhum"):** o mesmo livro admite que "Rust can't protect you from all kinds of logic errors when you use `Mutex<T>`" e que "`Mutex<T>` comes with the risk of creating deadlocks". A Linda não diz que o Óxido não tem bug de concorrência. Ela diz que aquele bug específico não compila, o que é verdade.

## R-12 (responde a **A28**, "pra que embrulhar se você ia abrir na hora mesmo?")

**A munição prometida no doc** ("fica de munição pra Iara num lote futuro"). **Revida:** Iara.

> "Pra você não conseguir fingir que não viu."

(pausa)

> "Abrir dá trabalho, dá. Mas eu TENHO que abrir. Tem que estar escrito na linha que eu abri e que eu aceitei o que ia acontecer se desse errado. Fica lá. Com o meu nome."

(pausa)

> "No teu, dá errado e a linha fica igualzinha. Ninguém abriu nada. Ninguém viu nada. O feitiço segue andando com o erro dentro, feliz da vida, até bater em alguém."

**Reação do Gus (engole seco):**

Ele não responde.

Mas no próximo conjuro, ele confere o retorno. Ninguém comenta.

**Fato (verdadeiro), os dois lados:** em Rust, operação falível devolve `Result<T, E>`, e o valor não sai de dentro sem `unwrap`/`expect`/`?`/`match` (já citado na A28: Rust Book cap. 9.2). Em C, a convenção é devolver código de erro, e **ignorar o retorno é legal e silencioso**: "All expression statements, such as function calls with an ignored value, are implicitly cast to `void`", e "failure to handle error codes or other values returned by functions can lead to incorrect program flow and violations of data integrity" (regra EXP12-C do CERT C, "Do not ignore values returned by functions"). A prova mais bonita de que o silêncio é o padrão é que o GCC precisou **inventar um atributo** só pra quebrar o silêncio: `warn_unused_result` "causes a warning to be emitted if a caller of the function with this attribute does not use its return value", útil "for functions where not checking the result is either a security problem or always a bug, such as `realloc`". **Fonte:** SEI CERT C Coding Standard, EXP12-C; GCC docs, "Common Function Attributes" (`warn_unused_result`). **Nota de tom:** o remate não é fala, é ação. O Gus conferir o retorno em silêncio no conjuro seguinte vale mais que qualquer réplica, e mantém o revide longe do sermão.

## R-13 (responde a **A29**, "vocês precisam baixar o sorteio de um estranho da internet")

**O contra-ataque previsto no doc** ("Se o líder quiser, isso vira contra-ataque da Linda depois"). **Revida:** Linda.

> "A minha caixa não sorteia, é verdade. Quem sorteia mora fora dela."

(pausa)

> "A tua sorteia."

(pausa)

> "E o manual da TUA sorte, Gus, o teu, escrito pelos teus, diz com todas as letras: não usa isso quando você precisar de sorte boa."

(pausa)

> "Tá lá. Vai ler. Você que gosta de manual."

**Reação do Gus (perde o CONTROLE da conversa, AMB-06 canon):**

Uns oito passos de silêncio.

> "...eu uso pra decidir quem vai na frente."

(pausa)

> "Ninguém morreu."

> "Ainda."

**Fato (verdadeiro):** o manual do `rand()` diz, literalmente: "Do not use this function in applications intended to be portable when good randomness is needed. (Use `random(3)` instead.)", e registra que em implementações antigas "the lower-order bits are much less random than the higher-order bits". **Fonte:** Linux man-pages, `rand(3)`. **Sem número:** eu **não** cravei nenhum valor de `RAND_MAX` porque não consegui confirmar o mínimo do padrão numa fonte fetchada nesta sessão (a cppreference devolveu 403). A fala não cita número nenhum. **Nota:** fecha o par com a A29 e paga a dívida que o doc registrou. O Gus não sabia disso, ou fingia não saber. Agora ele sabe, e o "ainda" da Linda é o melhor botão do lote.

---

## Log de ambiguidades do lote de revides

| ID | Ambiguidade | Opções | Status |
|---|---|---|---|
| **AMB-05** | O R-07 usa um relatório do mundo real (ONCD/Casa Branca, 2024). O doc transpõe **nomes de linguagem**, mas nunca precisou transpor uma **instituição**. Dizer "Casa Branca" quebra a diegese; inventar um órgão do mundo é mexer em canon (`factions.md`) | (a) manter vago como está ("gente que decide coisa grande"), (b) atribuir a uma facção canônica existente, (c) cortar o R-07 | **RESOLVIDA (líder, 2026-07-17): (a) + semente de canon.** A instituição vira um **órgão de padronização/segurança** (equivalente diegético de ISO/CERT: neutro, apolítico, audita linguagens e publica "editais" de segurança descobríveis nos in-world-docs). O documento do R-07 é um edital desse órgão. Fica só o **gancho** (não inventar a facção inteira agora; detalhar depois em `factions.md`/`in-world-docs.md` com aprovação do líder). A fala da Linda permanece vaga ("gente que decide coisa grande"), agora **ancorada na semente**. |
| **AMB-06** | O líder especificou a reação do Gus pra Iara, Cauã, Bento e Jaci. **Não especificou pra Linda** (R-07, R-11, R-13) | (a) silêncio calculando + saída técnica meia-boca + muda de assunto, (b) igual à da Iara (engole seco), (c) o líder define | **RESOLVIDA (líder, 2026-07-17): (a).** Com a Linda o Gus **perde o CONTROLE da conversa** (não o argumento, como com a Iara), e pra ele isso é pior. Deriva do `party.md` ("Linda lê rápido", "linguagem como arma"). Aplicada aos R-07, R-11, R-13. |
| **AMB-07** | O R-09 (Hoare) fala "o meu não tem [buraco]" pela Iara. Verdade no Óxido **seguro**; ponteiro cru em modo inseguro pode ser nulo | (a) manter (é juízo de fanboy, mesmo direito que o Gus tem), (b) trocar por "o meu me obriga a olhar dentro antes de usar" (fato puro, sem juízo) | **RESOLVIDA (líder, 2026-07-17): (a).** O líder aprovou a versão poética (juízo de fanboy da Iara), não a de fato-puro. O caveat de honestidade embutido no R-09 (Hoare = ALGOL W, não C; a Iara nunca diz que ele inventou o nulo do C) permanece. A versão (b) fica registrada no R-09 como alternativa não usada. |

## Frases descartadas do lote de revides (não passaram na verificação)

| Revide pretendido | Quem ia falar | Motivo do descarte |
|---|---|---|
| "O Cargo baixa tudo pra você e o teu obriga a caçar biblioteca na mão" | Iara, contra a A29 | Queixa real, mas eu ia precisar de fonte primária dizendo que C **não tem** gerenciador de pacotes oficial, e "não tem" é afirmação factual sobre um ecossistema inteiro (existem vcpkg, Conan, apt). Não achei fonte que cravasse. Fora. |
| "Sete em cada dez dos erros do MUNDO são culpa do teu jeito" | Iara, contra a A17 | **Extrapolação falsa.** O dado dos 70% é das bases de código C/C++ da Microsoft e do Chromium, não do mundo. O R-06 diz exatamente o que a fonte diz ("quem fabrica as máquinas grandes contou os PRÓPRIOS buracos") e nada além. |
| "O Asmódico é mais rápido que o teu C-Arcane" | Bento, contra a A13/A16 | **Não é defensável como fato.** A própria fonte de assembly diz que "modern optimizing compilers are claimed to render high-level languages into code that can run as fast as hand-written assembly". O Bento reivindicaria velocidade e estaria mentindo. Reescrito como R-04, que reivindica **controle** (verdadeiro e mais pesado). |
| "O teu compilador é escrito em C-Arcane, então os bugs dele são teus" | Cauã, contra a A5 | Verdadeiro que compiladores C são escritos em C, mas a conclusão ("os bugs são teus") é retórica vazia, e o revide virava aula. Autogol também: o argumento derruba a Pythia junto. Fora. |
| "Ninguém usa o teu pra fazer nada novo, só pra manter coisa velha" | Linda, contra a A21 | **Fato falso.** Kernel, bancos de dados, firmware e runtime de linguagem nova continuam sendo escritos em C hoje. Xingamento disfarçado de estatística. Fora. |
| "O `rand()` do teu tem viés e sempre dá o mesmo número" | Linda, contra a A29 | **Overclaim.** O man page fala de qualidade e dos bits baixos em implementações antigas; ele **não** diz que "sempre dá o mesmo número", e o glibc atual não é o gerador ruim clássico. O R-13 usa a frase literal do manual ("não usa quando precisar de sorte boa"), que é mais forte e é verdade. |
| "O teu não tem lista, não tem texto, não tem nada, você escreve tudo na mão" | Jaci, contra a A18 (43 palavras) | Impreciso: a biblioteca padrão do C tem string.h, e "não tem nada" é falso. A queixa real (std pequena) existe, mas é o espelho exato da A29, e o lote ia ficar repetindo o mesmo argumento com a boca trocada. Fora por redundância. |
| "A Pythia é usada em mais lugares que o C-Arcane" | Cauã, contra a A20 | Ranking de popularidade sem fonte primária citável (índices de popularidade são metodologicamente contestados). Não entra sem número que eu possa provar. |
| "Você nunca leu o padrão inteiro do teu" | Iara, contra a A18 | **Fato inventado sobre o Gus**, e ele provavelmente leu. Além disso, humilha sem informar: quebra a ordem de tom do líder. Fora. |
| "O teu deixa você somar letra com número e não reclama" | Cauã, contra a A17 | Verdadeiro em parte (conversões implícitas), mas eu não achei fonte primária fetchada nesta sessão pra redigir sem imprecisão, e o assunto é escorregadio (o compilador avisa com flag). Fora até ter citação. |
| Revide do Dante | Dante, em qualquer frase | **Cortado de propósito, decisão a reportar ao líder.** O Dante defende Asmódico "aparente" (`party.md`: "Asmódico autêntico vs Asmódico-fake") e migra pra C-Arcane no late game. Qualquer revide dele é ou pista de traição ou ruído. Se ele revida, isso é assunto de foreshadowing, não de piada. Precisa de decisão separada do líder. |

---

# A30 (diálogo: o gato e o peixe)

> **APROVADO pelo líder 2026-07-17, CANON.**
> Diferente de A1-A29 (fala solo do Gus, sem resposta obrigatória): este é um **aparte-DIÁLOGO**, uma troca de duas falas entre Linda e Gus. Conecta com o **A22** (o turbofish, "o peixe"): os dois operadores citados aqui têm apelido real de bicho, e o Gus fecha a piada com a lealdade tribal da família C (ver nota de canon abaixo).

**Linda** (Óxido):

> "Pode rir o quanto quiser do meu turbofish, Gus. É o operador mais elegante que existe. O peixe."

**Gus:**

> "Pois é... sabia que o irmão mais velho do meu C-Arcane acabou de ganhar um operador novo? Chamam de 'orelhas de gato'. O `^^`."
>
> (pausa)
>
> "Gato caça peixe, Linda."

**Reação da Linda:** perde o CONTROLE da conversa, o mesmo padrão dela nos revides (ver tabela "Reação do Gus, por personagem" acima, AMB-06). Aqui é ela quem perde: muda de assunto rápido demais.

**Fato (verdadeiro):** `^^` é o operador de reflexão votado pro **C++26** (proposta P2996, aprovada em junho/2025), sintaxe prefixa unária `^^T` que reflete um nome em `std::meta::info`. O apelido é real na comunidade C++, "cat-ear operator" (operador de orelhas de gato), porque `^^` lembra duas orelhas de gato. **Fonte:** isocpp.org, P2996R13; dev.classmethod.jp, "Cat-Ear Operator ^^". O turbofish (`::<>`, "o peixe") já é fato citado e fontado no A22 (The Rust Reference, "Paths").

**Nota de canon (C-Arcane Major = C++):** o "irmão mais velho do meu C-Arcane" é o **C-Arcane Major**, nome diegético do C++ já canonizado na "frase RARA (o bilhete dourado)" (AMB-01). No bilhete dourado, o Gus critica o C-Arcane Major por facilitar demais ("só deixou fácil pra quem tem preguiça"); aqui, contra o Óxido (linguagem forasteira à família C), a família C fecha fileira, lealdade tribal, e o Gus torce pelo gato do C++ mesmo sem gostar dele por dentro. Os dois momentos não se contradizem: são registros diferentes (crítica interna vs. defesa externa), decisão já resolvida com o líder.

