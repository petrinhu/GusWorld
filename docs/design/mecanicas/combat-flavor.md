# Flavor de Combate: Cartas Lenta/Rapida + Acervo de Falhas

**Status:** Decisoes do criador supremo em 2026-06-23 (brainstorm colaborativo). Camada de SABOR e a mecanica de velocidade de carta do combate. O motor e as regras-base vivem em [`combat.md`](combat.md) (canonico); a apresentacao/tela em [`battle-screen.md`](battle-screen.md). A mecanica de cast-time descrita aqui (paragrafo 1) e uma EXTENSAO a ser canonizada no combat.md pelo lead-game-designer (ratificacao do criador); os parametros finos ainda serao fechados.

**Convencao:** pt-br na prosa. As mensagens de erro ficam no ORIGINAL tecnico (ingles do terminal real): a autenticidade e a piada (Pillar 1/2, magia = software). Sem em-dash; usa hifen, virgula, parenteses, dois-pontos.

**⚠ Atualizacao 2026-07-17 (lider):** o **§2 foi reescrito** e agora carrega a **regua-lei** de velocidade (**CANON PETREO**, acima dos pillars). A clausula "afinidade, nao trava" que vivia ali esta **removida**. Todo agente que for desenhar carta (inclusive **ESPECIAIS/SUPER**, que passam pelo executor techMagic, [ADR-016](../../tech/adr/ADR-016-techmagic-effect-engine-data-driven.md)) le o §2 **antes**.

**Cross-ref:** [`combat.md`](combat.md), [`battle-screen.md`](battle-screen.md), [`pillars.md`](../pillars.md), [`arco-principal.md`](../../narrative/arco-principal.md) (eixo Compilacao vs Interpretacao, Gus vs Sterling), [`cartas-comuns-statlines.md`](cartas-comuns-statlines.md) (§VELOCIDADE: o doc de CONTEUDO que espelha a regua-lei do §2), [`gus-apartes-c-arcane.md`](../../narrative/gus-apartes-c-arcane.md) (regua fato x juizo).

---

## 1. Cartas lenta (interpretada) vs rapida (compilada)

Decisao do criador 2026-06-23: cartas tem VELOCIDADE tematizada pela dicotomia real compilado vs interpretado (Pillar 1/2). Modelo escolhido: **tempo de conjuracao REAL na fila CTB** (nao cosmetico).

- **Carta RAPIDA (compilada):** resolve no mesmo turno. Efeito FECHADO (pre-determinado no "build"). Codigo de maquina dispara imediato.
- **Carta LENTA (interpretada):** iniciada agora (gasta AP), mas o efeito RESOLVE algumas posicoes a frente na fila CTB (um marcador "interpretando..." entra na fila). Resolve lendo o tabuleiro NAQUELE momento (flexivel, mais potente). Durante o cast fica VULNERAVEL a Stun/Disrupt/Silence/dano (interrompe ou enfraquece). O Gambito ja ve o marcador na fila e pode proteger ou reordenar.

**Decisao de jogo (risk vs reward, Sid Meier):** arrisco a interpretada poderosa que pode ser interrompida, ou jogo seguro com a compilada imediata?

### Falas das fases (aparecem durante a conjuracao)

As fases sao texto que aparece enquanto a magia processa. Na lenta, UMA fase por passo na fila (e o que comunica visualmente que esta demorando e abre a janela de interrupcao). A rapida e curta de proposito (contraste). Registro: tecnico (sabor dev completo).

```
RAPIDA (compilada):
  > compilando...   > linkando...   -> EXEC

LENTA (interpretada), 1 fase por passo na fila:
  > abrindo env...
  > parse: montando AST...
  > interpretando...
  > abstraindo...
  -> EVAL   (executa, le o estado AGORA)
```

### Falha por canal (a sacada tematica)

- **Rapida (compilada) interrompida/barrada:** falha no BUILD -> `ERRO DE COMPILACAO` (compile-time; ja canon no combat.md paragrafo 10).
- **Lenta (interpretada) interrompida:** so quebra em RUNTIME (e justamente a que pode ser interrompida no meio do cast) -> `RUNTIME ERROR` / excecao. A natureza interpretada E a vulnerabilidade.

Ao falhar, sorteia-se aleatoriamente uma frase do acervo (paragrafo 3), do canal correspondente.

---

## 2. Linguagens runicas -> velocidade da carta

> ## ⚠ CANON PETREO (nao e `//PLAYTEST`, nao e decisao de agente)
>
> **A regua-lei deste paragrafo foi aprovada pelo lider em 2026-07-17, carta a carta**, junto com a velocidade das 30 comuns (ver [`cartas-comuns-statlines.md`](cartas-comuns-statlines.md) §VELOCIDADE, o doc de CONTEUDO que espelha esta lei).
>
> **Motivo:** o eixo compilado x interpretado e **definicao pessoal do Gus original** e uma **promessa do lider ao menino**. Isso o coloca **acima dos pillars** na hierarquia de canon: um pillar pode ser revisado em brainstorm, este eixo nao.
>
> **Regra de alteracao:** qualquer mudanca exige **autorizacao explicita do lider**, naquele contexto. Aprovacao anterior nao vale pra frente. **Nenhum agente inverte uma velocidade "por balance".**
>
> **Se o playtest doer:** o remedio e **Power / mana / duracao de status / casas de espera na fila**. **Nunca a velocidade.** A velocidade e premissa; o balance trabalha em volta dela.
>
> **Historico:** a clausula "AFINIDADE, nao exclusividade" que ocupava este paragrafo (2026-07-16) foi **SUPERADA e removida** em 2026-07-17. Ela era a **causa-raiz** de uma atribuicao de velocidade que INVERTIA o eixo. Ver §2.4.

As linguagens do canon parodiam linguagens reais; o nome (parecido com o original) TELEGRAFA o tipo da carta. Camada de legibilidade dupla: quem programa reconhece, o leigo aprende pela cor/icone. O eixo Compilacao (Gus, disciplina) vs Interpretacao (Sterling, controle) ja e canonico em [`arco-principal.md`](../../narrative/arco-principal.md).

### 2.1 A regua-lei: conjurar = compilar + executar

O jogo **ja mostra isso na tela**. A fase de cast da carta rapida e, verbatim (§1 acima):

```
> compilando...   > linkando...   -> EXEC
```

A velocidade **nao e uma regra imposta por cima da lore**. Ela e a **consequencia aritmetica** do que o jogo ja afirma que acontece: se conjurar e compilar e executar, entao **quanto custa compilar e quanto demora conjurar**. A velocidade de cada carta CAI DELA SOZINHA, sem inventar nada e sem mentir em nenhum ponto.

**A LINGUAGEM TRAVA.** Nao e afinidade, nao e tendencia, nao e sabor. E **lei**, e esta **acima de role, de balance e de arquetipo**.

| Linguagem canon | Eco real | O que acontece DE VERDADE ao conjurar | Velocidade | Falha | Ancora de personagem |
|---|---|---|---|---|---|
| **Asmodico** | Assembly | **Nao compila. MONTA.** Ja e codigo de maquina, so traduz simbolo pra opcode | **rapida (A MAIS)** | ERRO DE COMPILACAO | Bento; Dante (origem) |
| **C-Arcane** | **C** | **Compila rapido.** E a fama real do C, e e merecida | **rapida** (o ponto doce) | ERRO DE COMPILACAO | Gus; Dante (late game) |
| **C-Arcane Major** | **C++** | Compila. Mesma familia, mesmo pacto firme | **rapida** | ERRO DE COMPILACAO | Gus (so na frase rara) |
| **Oxido** | Rust | Compila **DEVAGAR**: borrow checker + monomorfizacao + LLVM. **`async` e o pior caso notorio** | **rapida** no simples, **LENTA** no `async` | ERRO DE COMPILACAO | Iara, Linda |
| **Pythia** | Python | **Nao compila. INTERPRETA em runtime**, linha a linha, toda vez | **lenta** (excecao: §2.2) | RUNTIME ERROR | Caua, Jaci |
| **HIBRIDO** | cast numa linguagem, **executor final em outra** | A velocidade e a do **elo mais lento**. Se o executor final e interpretado, a carta e lenta, **e a culpa e do executor** | **a do elo mais lento** | a do elo que quebrou | caso canon: `Ondha-Fratura` (cast Asmodico, executor Pythia) |
| **DRE / GRE** | runtime/VM (JS/Go/Lua) | **Nao compila. Interpreta tudo em runtime**, por projeto (anti-compilacao e a tese do vilao) | **lenta** | RUNTIME ERROR | Sterling (vilao) |

**Nota de canon (2026-07-17):** `C-Arcane` mapeia pra **C sozinho**. O C++ tem nome diegetico proprio, **C-Arcane Major**, criado pelo lider em 2026-07-17 (resolve AMB-01 de [`gus-apartes-c-arcane.md`](../../narrative/gus-apartes-c-arcane.md)). A linha antiga `C-Arcane | C / C++` tratava os dois como um so. **Nao muda velocidade nenhuma**: os dois compilam, os dois sao rapidos.

**Achado a registrar (o eixo novo nao quebra a narrativa, ele a ENDIREITA):** a linha `DRE / GRE (Sterling) | interpretada | lenta` **ja estava correta e REFORCA a regua-lei**. O runtime do vilao ser lento casa exatamente com a oposicao **Compilacao (Gus) x Interpretacao (Sterling)** de [`arco-principal.md`](../../narrative/arco-principal.md). Idem `Asmodico | montada | rapida (a mais)`: **ja estava correto** e ja antecipava esta regua. O framework tinha a verdade; a clausula "afinidade" a atropelou.

### 2.2 As duas excecoes honestas (que PROVAM a regra, nao furam)

**1. Pythia rapida quando GENUINAMENTE COMPILA.** Nao e "a carta e especial". E que **naquele caso especifico o Python compila de verdade**:

- **`@jit` (Numba/PyPy) compila mesmo.** Vira codigo de maquina. Nao e metafora, e o que a ferramenta faz.
- **Builtin do CPython E escrito em C.** Roda rapido porque **nao e Python rodando**, e C compilado embaixo do capo (fato dos apartes A1/A2: *"tem um C-Arcane ali embaixo fazendo o trabalho pesado"*).

Repare no que isso faz: a Pythia so fica rapida **quando para de ser interpretada**. Toda vez que a Pythia e rapida, e porque compilou. Isso **PROVA a tese do Gus** com mais forca do que uma excecao furada proveria.

**2. Hibrido: a lerdeza vem da parte interpretada.** Se o cast e Asmodico mas o executor final e Pythia, a carta e **LENTA**, e **a culpa e do Python, nao do Assembly**. O Asmodico fez a parte dele em nanossegundos e ficou esperando o interpretador. **Assembly continua o mais rapido**, e a lerdeza tem um culpado nomeado.

### 2.3 🚫 MENTIRA PROIBIDA (regra de redacao inegociavel)

> **"Nao minta JAMAIS para uma crianca."**

**NUNCA afirmar, em nome de carta nenhum, em frase de cast nenhuma, em aparte nenhum, em VFX nenhum:**

- ❌ que **interpretado e rapido**
- ❌ que **compilado e lento por ser compilado**

Ambas sao falsas, e este paragrafo existe porque foram ditas uma vez. Um agente que precisar de uma carta lenta numa familia compilada **nao inventa um hook**: ele acha o **motivo real** (o `async` do Oxido e lento de verdade; o executor Pythia do hibrido e lento de verdade) ou **nao faz a carta lenta**.

**Teste de deteccao (aplicar a todo hook novo):** nao pergunte *"esse recurso existe?"*. Pergunte: **"a frase que sobra afirma que interpretado e rapido, ou que compilado e lento?"** Se sim, o hook esta morto, **por mais real que o recurso seja**. Os 3 hooks ja mortos (REPL rapido, `async` como espera diegetica, vetor de interrupcao/`IRET`) eram todos tecnicamente verificaveis e **mesmo assim mentiam**, porque a **conclusao** que sustentavam era falsa. Registro completo em [`cartas-comuns-statlines.md`](cartas-comuns-statlines.md) §"Os 3 hooks DESCARTADOS".

Isto e irmao da regua dos apartes ([`gus-apartes-c-arcane.md`](../../narrative/gus-apartes-c-arcane.md)): **fato e proibido de ser falso, juizo e livre.** O Gus pode achar o Oxido feio. Ele nao pode dizer que interpretado e rapido.

### 2.4 O que esta clausula SUBSTITUI (pra ninguem ressuscitar)

**A clausula "AFINIDADE, nao exclusividade" (2026-07-16) esta MORTA.** Ela dizia, verbatim, que a linguagem era so afinidade e que o personagem *"PODE ter cartas de outra velocidade **quando o role/balance pede**"*.

**Por que produziu o erro:** a clausula rebaixava a linguagem a **tendencia estetica** e promovia **role/balance** a criterio que **vencia** a linguagem. Traduzindo o que ela autorizava: *"se o balance pedir, faca a carta interpretada ser rapida."* Foi obedecendo a isso que (1) o arquetipo virou lei ("Finalizador = LENTA sempre", "Jab = RAPIDA sempre") acima da linguagem; (2) quando arquetipo e linguagem colidiram, **a linguagem cedeu**; (3) pra vestir a colisao, inventaram-se hooks que **afirmavam fato falso**. O erro foi **sistematico, nao aleatorio**: seguiu a regra escrita, e a regra estava errada.

**Regra derivada (vale para as ESPECIAIS/SUPER tambem):** **nenhum arquetipo dita velocidade. A LINGUAGEM dita.** O arquetipo dita **custo, Power e condicao**. Se um arquetipo futuro "precisar" ser lento numa familia compilada, **o arquetipo esta errado, nao a regua**.

**A "regua-mestre" antiga tambem morreu:** *"CC/heal = rapida sempre, senao chega tarde e e carta desperdicada"*. Ela **colide com a lei** e perde. `Tavusa-Trava` (Stun, Pythia) e **LENTA**, e isso e **custo ACEITO e INTENCIONAL** pelo lider: um Stun que chega tarde e **exatamente a licao** de que interpretado te trai na hora H. Idem `Tavusa-Overclock` (recarga de recurso que chega tarde vale menos). **Watchlist do N=3, nao bug.** Remedio permitido: duracao do status ou casas de espera. Nunca a velocidade.

**Gus = compilador universal** (poliglota, usa qualquer linguagem). **Frase comica do Gus:** o Gus solta uma fala engracada de vez em quando durante o cast / nas disputas de linguagem (defende C-Arcane; ver comic-reliefs C.4) e os apartes atrelados a cartas especificas ([`gus-apartes-c-arcane.md`](../../narrative/gus-apartes-c-arcane.md)); conteudo do narrative-writer, densidade baixa (nem todo cast).

### 2.5 Ressalva: o `>>>` do prompt do Caua NAO e afirmacao de velocidade

O prompt `caua@pythia:~$ >>>` (§5 abaixo) e **caracterizacao**: o Caua "vive no REPL". **Nao e**, e **nao pode virar**, justificativa de rapidez.

O hook **"REPL one-liner e rapido" esta MORTO** e nao volta: o REPL do Python **continua interpretado**; digitar direto no prompt **nao compila nada**. O hook trocava "compilar" por "nao precisar salvar arquivo", que nao e a mesma coisa e nao afeta a velocidade de execucao. Era a mentira mais sutil das tres, e por isso a mais perigosa. Consequencia canon: a `Tavusa-Pulso` (o jab do Caua) e **LENTA**, e sem residuo visual de REPL na frase de cast dela. **Manter o `>>>` no prompt; nunca deriva-lo em rapidez.**

---

## 3. Acervo de 100 frases de falha (por linguagem)

Sorteadas aleatoriamente ao falhar, do canal correspondente. Marcador: `[C]` = COMPILE (falha de carta rapida), `[R]` = RUNTIME (falha de carta lenta). Todas autenticas da linguagem real equivalente.

### C-Arcane (C/C++)
```
1.  error: expected ')' before ';' token                         [C]
2.  error: expected ';' before '}' token                          [C]
3.  error: unexpected end of file while parsing                   [C]
4.  error: expected expression before '}' token                  [C]
5.  error LNK2019: unresolved external symbol                    [C]
6.  undefined reference to 'main'                                 [C]
7.  error: too few arguments to function 'cast'                   [C]
8.  error: no matching function for call to 'invoke()'           [C]
9.  error: 'i' was not declared in this scope                    [C]
10. error: lvalue required as left operand of assignment          [C]
11. warning: implicit declaration of function 'channel'          [C]
12. error: control reaches end of non-void function              [C]
13. error: expected initializer before 'token'                   [C]
14. error: conflicting types for 'main'                          [C]
15. error: redefinition of 'self'                                [C]
16. error: 'class Spell' has no member named 'cast'              [C]
17. error: invalid conversion from 'int' to 'char*' [-fpermissive][C]
18. error: cannot bind non-const lvalue reference to an rvalue   [C]
19. terminate called after throwing an instance of 'std::out_of_range' [R]
20. Segmentation fault (core dumped)                             [R]
```

### Oxido (Rust)
```
1.  error[E0382]: use of moved value: `mana`                                       [C]
2.  error[E0502]: cannot borrow `grid` as mutable because it is also borrowed as immutable [C]
3.  error[E0507]: cannot move out of borrowed content                              [C]
4.  error[E0308]: mismatched types: expected `bool`, found `()`                    [C]
5.  error[E0277]: the trait bound `Glyph: Display` is not satisfied                [C]
6.  error[E0106]: missing lifetime specifier                                       [C]
7.  error[E0499]: cannot borrow `caster` as mutable more than once at a time       [C]
8.  error[E0596]: cannot borrow `self.hp` as mutable, as it is behind a `&` reference [C]
9.  error[E0384]: cannot assign twice to immutable variable `rune`                 [C]
10. error[E0425]: cannot find value `i` in this scope                              [C]
11. error[E0061]: this function takes 2 arguments but 1 argument was supplied      [C]
12. error[E0072]: recursive type `Node` has infinite size                          [C]
13. error[E0599]: no method named `cast` found for struct `Spell`                  [C]
14. error[E0515]: cannot return value referencing local variable `buffer`          [C]
15. error: expected one of `,`, `:`, or `}`, found `token`                         [C]
16. error[E0432]: unresolved import `crate::sigil`                                 [C]
17. error[E0004]: non-exhaustive patterns: `None` not covered                      [C]
18. thread 'main' panicked at 'index out of bounds: the len is 3 but the index is 7' [R]
19. thread 'main' panicked at 'called `Option::unwrap()` on a `None` value'        [R]
20. thread 'main' panicked at 'attempt to divide by zero'                          [R]
```

### Asmodico (Assembly)
```
1.  Segmentation fault (core dumped)                             [R]
2.  Bus error (core dumped)                                      [R]
3.  *** stack smashing detected ***: terminated                  [R]
4.  double free or corruption (out)                              [R]
5.  Illegal instruction (core dumped)                            [R]
6.  General protection fault                                     [R]
7.  free(): invalid pointer                                      [R]
8.  malloc(): corrupted top size                                 [R]
9.  munmap_chunk(): invalid pointer                              [R]
10. Trace/breakpoint trap (core dumped)                          [R]
11. EXC_BAD_ACCESS (code=1, address=0x0)                         [R]
12. Killed (out of memory)                                       [R]
13. kernel: traps: divide error                                  [R]
14. corrupted size vs. prev_size                                 [R]
15. Aborted (core dumped)                                        [R]
16. realloc(): invalid old size                                  [R]
17. page fault at rip, error 4 in libc                           [R]
18. Error: invalid combination of opcode and operands            [C]
19. Error: junk at end of line, first unrecognized character is `%' [C]
20. Error: symbol `_start' is already defined                    [C]
```

### Pythia (Python)
```
1.  IndentationError: unexpected indent                          [C]
2.  IndentationError: expected an indented block                 [C]
3.  SyntaxError: invalid syntax                                  [C]
4.  SyntaxError: unexpected EOF while parsing                    [C]
5.  SyntaxError: '(' was never closed                            [C]
6.  TabError: inconsistent use of tabs and spaces in indentation [C]
7.  SyntaxError: cannot assign to literal                        [C]
8.  RecursionError: maximum recursion depth exceeded             [R]
9.  ZeroDivisionError: division by zero                          [R]
10. TypeError: 'NoneType' object is not iterable                 [R]
11. TypeError: 'NoneType' object is not subscriptable            [R]
12. AttributeError: 'NoneType' object has no attribute 'cast'    [R]
13. KeyError: 'self'                                             [R]
14. IndexError: list index out of range                         [R]
15. NameError: name 'mana' is not defined                       [R]
16. TypeError: unsupported operand type(s) for +: 'int' and 'str'[R]
17. ValueError: invalid literal for int() with base 10: 'glyph'  [R]
18. UnboundLocalError: local variable 'hp' referenced before assignment [R]
19. ModuleNotFoundError: No module named 'sigil'                 [R]
20. MemoryError                                                  [R]
```

### DRE/GRE (Sterling) (runtime/VM: JS + Go + Lua)
```
1.  TypeError: undefined is not a function                       [R]
2.  TypeError: Cannot read property 'x' of null                  [R]
3.  TypeError: Cannot read properties of undefined (reading 'cast') [R]
4.  RangeError: Maximum call stack size exceeded                 [R]
5.  ReferenceError: mana is not defined                          [R]
6.  TypeError: Cannot set properties of null (setting 'hp')      [R]
7.  TypeError: spell.cast is not a function                      [R]
8.  Uncaught SyntaxError: Unexpected token '}'                   [R]
9.  panic: runtime error: invalid memory address or nil pointer dereference [R]
10. panic: runtime error: index out of range [7] with length 3  [R]
11. fatal error: all goroutines are asleep - deadlock!          [R]
12. runtime: goroutine stack exceeds 1000000000-byte limit      [R]
13. panic: runtime error: integer divide by zero                [R]
14. panic: close of closed channel                              [R]
15. panic: assignment to entry in nil map                       [R]
16. attempt to index a nil value (global 'caster')              [R]
17. attempt to call a nil value (method 'cast')                 [R]
18. attempt to perform arithmetic on a nil value (field 'hp')   [R]
19. stack overflow                                              [R]
20. attempt to compare nil with number                          [R]
```

**TOTAL: 100.**

---

## 3b. Acervo de fim de combate (vitoria / derrota / performance)

Para a tela de resultado (log de build ao vivo; ver [`battle-screen.md`](battle-screen.md) paragrafo 3.1). Distintos das 100 frases de falha-de-carta (par. 3): aquelas sao falha de UMA carta no meio da luta; estas sao o FIM do combate. Originais tecnicos, autenticos.

### Vitoria (party venceu)
```
1.  BUILD SUCCEEDED
2.  Process finished with exit code 0
3.  Compilation finished successfully
4.  0 errors, 0 warnings
5.  All tests passed (8/8)
6.  Test suite passed: 13 passed, 0 failed
7.  LeakSanitizer: no leaks detected
8.  Linking succeeded
9.  Clippy: no warnings emitted
10. Coverage: 100% of statements
11. Merged branch 'combat' into main
12. Fast-forward merge, working tree clean
13. Deploy succeeded, 0 rollbacks
14. All checks have passed
15. Done. Build completed without errors.
```

### Derrota (party wipe) -- processo/servico morto (sistemico, nao sintaxe)
```
1.  Killed (signal 9: SIGKILL)
2.  Process terminated (core dumped)
3.  Process finished with exit code 137
4.  Out of memory: Killed process (party)
5.  Kernel panic - not syncing: Attempted to kill init
6.  fatal error: all goroutines are asleep - deadlock!
7.  Connection to host lost
8.  Process finished with exit code 139 (interrupted by signal 11: SIGSEGV)
9.  terminate called recursively
10. Service exited, status=143/n/a
11. Aborted (core dumped)
12. No more processes left to schedule. System halted.
```
(exit 137 = 128+9 SIGKILL; exit 139 = 128+11 SIGSEGV; status 143 = 128+15 SIGTERM: codigos reais de morte por sinal.)

### Rotulos de performance (metrica "compile time" = turnos)
```
[RAPIDO] (elogio + bonus de eficiencia):
1. Blazing fast: built in 0.4s
2. Clean build, no warnings
3. Zero-cost abstraction
4. Optimized build (-O3)
5. Cache hit, nothing to rebuild
6. Sub-millisecond execution
7. Hot path inlined
8. Compiled with link-time optimization

[NEUTRO] (build lento; NUNCA xinga, so constata):
9.  Build complete
10. Compiled
11. Done
12. Finished
```

---

## 4. Regra de i18n (sugestao da ux-writer, canonica)

- O **codigo de erro** (ex: `Segmentation fault (core dumped)`) e string LITERAL nao-traduzivel: faz parte do gag tecnico e da autenticidade. O dev ri do erro real em qualquer locale.
- O **subtitulo do leigo** (ex: "a magia falhou") vai pro CSV `tr()` com chave propria (ex: `combat.spell_fail.subtitle`), traduzido normalmente.

Coerente com o i18n canonico do GusWorld (dev em pt-br, estrutura i18n-ready desde dia 1; tradicao en-intl pos-1.0).

---

## 5. Apresentacao das falas: terminal (software) vs caixa quente (humano)

Decisao do criador 2026-06-23 (hibrido com significado). Os baloes de fala viram telas de TERMINAL, mas so quando faz sentido. O CONTRASTE vira linguagem:

- **TERMINAL** quando o SOFTWARE/maquina fala: combate (falas de carta, COMPILADO, frases de falha do paragrafo 3), auto-kill (kill -9...), magia, HUD, mensagens de sistema, e o sarcasmo tecno dos personagens (banter de linguagem). Frio, monospace.
- **CAIXA QUENTE (com retrato)** quando o CORACAO fala: cena emocional, lore intima, momentos de party, drama (ex: a traicao do Dante, Gus e Jaci). Cor suave, fonte legivel, retrato grande. Protege o calor das cenas humanas.

### Prompt-de-linguagem por personagem (caracterizacao no modo terminal)

Cada personagem "fala" no prompt da sua linguagem-ancora (paragrafo 2), de graca pela lore:

| Personagem | Prompt de terminal | Detalhe |
|---|---|---|
| Gus | `gus@c-arcane:~$` | compilado, legivel (a sintese final do heroi) |
| Caua | `caua@pythia:~$ >>>` | o REPL do Python literal ("voce compila tudo, eu so rodo") |
| Jaci | `jaci@pythia:~$ >>>` | Pythia bio (mesma familia do Caua, sabor organico) |
| Iara | `iara@oxido:~$ cargo run` | Rust, elegancia opaca |
| Linda | `linda@oxido:~$` | Oxido (crowd control) |
| Bento | `bento@asmodico:~$` | asm, baixo nivel (registradores/hex, tom de monitor antigo) |
| Dante | `dante@asmodico:~$` -> `dante@c-arcane:~$` | **FORESHADOW:** o prompt muda de Asmodico pra C-Arcane no late game (~75%), sinal sutil da corrupcao Sterling (ja canon em `characters/dante-grid.md`; player atento nota) |
| Sterling | `sterling@DRE:/#` | root (`#`), o runtime do vilao (DRE/GRE); o `#` ja grita "privilegio demais" |

### Nota de escopo

Decisao macro tomada. O design FINO dos dois registros (estilo do terminal cru vs quente, fonte, cores, integracao com o retrato, animacao de digitacao tipo type-on) vai pra ux-ui-designer + ux-writer (propostas -> AskUserQuestion ao criador), no design do M5 e do sistema de dialogo. Afeta a [`battle-screen.md`](battle-screen.md) e o runtime de dialogo (a re-derivar em C++; ver [`dialogue-tree-npc-intro.md`](../narrativa/dialogue-tree-npc-intro.md)).

---

## 6. Pendencias (a fechar no design do M5)

- Canonizar a mecanica de cast-time (paragrafo 1) como extensao do combat.md, via lead-game-designer (ratificacao do criador). Parametros finos a definir (todos `//PLAYTEST`, afinaveis livremente): quantas posicoes a frente a lenta resolve (por carta?), o que a interrupcao faz (cancela e perde AP/mana? atrasa? reduz potencia proporcional ao dano?), o que "le o tabuleiro tarde" significa (re-mira? escala com estado?).
- ~~A amarracao linguagem<->carta<->personagem~~ **RESOLVIDA (lider, 2026-07-17): a linguagem TRAVA a velocidade.** Ver §2 (regua-lei) e [`cartas-comuns-statlines.md`](cartas-comuns-statlines.md) §VELOCIDADE (as 30 comuns, aprovadas carta a carta). **Nao reabrir sem autorizacao explicita do lider.**
- Estilo visual das fases (spinner/log) e da mensagem de falha na tela: ver [`battle-screen.md`](battle-screen.md).
