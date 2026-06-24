# Flavor de Combate: Cartas Lenta/Rapida + Acervo de Falhas

**Status:** Decisoes do criador supremo em 2026-06-23 (brainstorm colaborativo). Camada de SABOR e a mecanica de velocidade de carta do combate. O motor e as regras-base vivem em [`combat.md`](combat.md) (canonico); a apresentacao/tela em [`battle-screen.md`](battle-screen.md). A mecanica de cast-time descrita aqui (paragrafo 1) e uma EXTENSAO a ser canonizada no combat.md pelo lead-game-designer (ratificacao do criador); os parametros finos ainda serao fechados.

**Convencao:** pt-br na prosa. As mensagens de erro ficam no ORIGINAL tecnico (ingles do terminal real): a autenticidade e a piada (Pillar 1/2, magia = software). Sem em-dash; usa hifen, virgula, parenteses, dois-pontos.

**Cross-ref:** [`combat.md`](combat.md), [`battle-screen.md`](battle-screen.md), [`pillars.md`](../pillars.md), [`arco-principal.md`](../../narrative/arco-principal.md) (eixo Compilacao vs Interpretacao, Gus vs Sterling).

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

As linguagens do canon parodiam linguagens reais; o nome (parecido com o original) TELEGRAFA o tipo da carta. Camada de legibilidade dupla: quem programa reconhece, o leigo aprende pela cor/icone. O eixo Compilacao (Gus, disciplina) vs Interpretacao (Sterling, controle) ja e canonico em [`arco-principal.md`](../../narrative/arco-principal.md).

| Linguagem canon | Eco real | Tipo | Velocidade | Falha | Ancora de personagem |
|---|---|---|---|---|---|
| **C-Arcane** | C / C++ | compilada | rapida | ERRO DE COMPILACAO | Gus; Dante (late game) |
| **Oxido** | Rust | compilada | rapida | ERRO DE COMPILACAO | Iara |
| **Asmodico** | Assembly | montada (baixo nivel) | rapida (a mais) | ERRO DE COMPILACAO | Bento; Dante (origem) |
| **Pythia** | Python | interpretada | lenta | RUNTIME ERROR | Caua, Jaci |
| **DRE / GRE** | runtime/VM (JS/Go/Lua) | interpretada | lenta | RUNTIME ERROR | Sterling (vilao) |

**Amarracao pendente (a fechar com o criador no design do M5):** a linguagem e propriedade da CARTA (telegrafa o tipo) ou trava do PERSONAGEM? Indicio para "propriedade da carta": Caua e Pythia (interpretada/lenta) mas e Striker/burst (sugere rapido), entao travar a velocidade no personagem conflitaria com a familia. Provavel: linguagem = propriedade da carta, com afinidade (nao exclusividade) a linguagem-ancora comica do personagem. Gus = compilador universal (poliglota).

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

## 4. Regra de i18n (sugestao da ux-writer, canonica)

- O **codigo de erro** (ex: `Segmentation fault (core dumped)`) e string LITERAL nao-traduzivel: faz parte do gag tecnico e da autenticidade. O dev ri do erro real em qualquer locale.
- O **subtitulo do leigo** (ex: "a magia falhou") vai pro CSV `tr()` com chave propria (ex: `combat.spell_fail.subtitle`), traduzido normalmente.

Coerente com o i18n canonico do GusWorld (dev em pt-br, estrutura i18n-ready desde dia 1; tradicao en-intl pos-1.0).

---

## 5. Pendencias (a fechar no design do M5)

- Canonizar a mecanica de cast-time (paragrafo 1) como extensao do combat.md, via lead-game-designer (ratificacao do criador). Parametros finos a definir: quantas posicoes a frente a lenta resolve (por carta?), o que a interrupcao faz (cancela e perde AP/mana? atrasa? reduz potencia proporcional ao dano?), o que "le o tabuleiro tarde" significa (re-mira? escala com estado?), e a amarracao linguagem<->carta<->personagem (paragrafo 2).
- Estilo visual das fases (spinner/log) e da mensagem de falha na tela: ver [`battle-screen.md`](battle-screen.md).
