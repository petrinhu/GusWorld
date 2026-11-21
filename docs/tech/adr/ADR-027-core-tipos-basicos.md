# ADR-027: Tipos básicos do core: identificador, erro e resultado

| | |
|---|---|
| **Status** | Proposed (decisões técnicas do CTO, a ratificar pelo líder) |
| **Data** | 2026-11-20 |
| **Decisor** | CTO (Caetano), com ratificação do líder pendente |
| **Reversibilidade** | Hybrid. O layout de pastas e o nome dos arquivos trocam barato enquanto houver poucos consumidores. O formato do identificador (`uint64`, zero inválido) e o tipo de resultado viram contrato de todas as camadas de cima e do save: trocá-los depois de `content/` e `domain/` existirem custa reescrever cada consumidor. |

## Contexto

O item `D1` do `TODO.md` é o primeiro código de produto do jogo, na camada
`core/`, a mais baixa da espinha. A descrição dele pede "tipos básicos,
identificadores, resultado e erro", cada conceito como átomo em arquivo próprio
(GW-04), com TDD estrito desde a primeira fatia (GW-19).

Hoje o `core/` tem só o `core/CMakeLists.txt` e o `core/layer.cpp`, que define
`gusworld::core::layer_name()`, um símbolo mínimo que prova que a camada compila
e linka. Não há cabeçalho público, diretório de include nem teste de camada.

As leis que pesam sobre esta decisão:

- **GW-04:** nenhum arquivo reúne mais de um assunto. Cada conceito do `D1` vive
  no seu próprio cabeçalho.
- **GW-17:** "nenhum detalhe de C++ quebra o replay determinístico". Um tipo de
  `core/` é usado por todas as camadas de cima, então qualquer fonte de
  não-determinismo que ele permita se espalha para o jogo inteiro.
- **GW-33:** "fatiar sem sentido e abstração especulativa são proibidos tanto
  quanto o monolito". O `core/` não pode nascer com tipos que nenhum consumidor
  pediu ainda.

## Opções consideradas

### 1. Escopo de "tipos básicos"

- **Biblioteca de utilidades completa** (string fixa, vetor pequeno, tempo,
  ponto e retângulo, etc.). Rejeitada: nenhum consumidor existe para esses
  tipos, e a GW-33 proíbe abstração especulativa.
- **Só identificador, erro e resultado (escolhida).** São os três conceitos que
  a descrição do `D1` nomeia, e os três que o `D3` (POCOs de carta) e o resto da
  onda de domínio consomem de imediato. Todo outro tipo básico nasce na fatia do
  primeiro consumidor real.

Este é um **corte de escopo declarado**: "tipos básicos", no `D1`, quer dizer
estes três e nada mais.

### 2. Resultado

- **Tipo próprio** (`result<T>` escrito em casa). Rejeitada: reimplementa o que
  a biblioteca padrão já entrega, e o projeto é C++23 (GW-03).
- **Exceções para erro de regra.** Rejeitada: uma regra de jogo é transição
  determinística que devolve estado e eventos (GW-17); o erro faz parte do
  retorno, não do fluxo de controle.
- **`std::expected` do C++23 (escolhida).** Dois apelidos, sem tipo novo:
  `result<T> = std::expected<T, error>` e `status = std::expected<void, error>`.

### 3. Erro

- **Código inteiro solto** ou **string de mensagem.** Rejeitadas: o inteiro não
  tem nome no ponto de uso, e a string não se compara de forma estável.
- **Enum com nome, mais um POCO com local de origem (escolhida).**
  `error_code` é uma enum que começa com um único valor, `invalid_id`, e cresce
  só quando um consumidor precisar de outro. `error_code_name()` devolve o nome
  legível de cada valor. `error` é um POCO com dois campos: `code` e um
  `std::source_location` de onde o erro nasceu. A igualdade entre dois `error`
  compara **só o código**: o local de origem serve para diagnóstico e muda a cada
  ponto de chamada, então entrar na igualdade tornaria os testes frágeis.

### 4. Identificador

- **`uint64` cru.** Rejeitada: o compilador não impede passar o id de uma carta
  onde se espera o id de um inimigo.
- **`strong_id<tag>` sobre `uint64` (escolhida).** Um tipo por etiqueta. Zero é
  o valor inválido. A construção é explícita, há `==` e `<=>`, e **não há
  aritmética**: um id não se soma nem se incrementa. `from_raw(0)` devolve o erro
  `invalid_id`, então `from_raw` devolve um `result`.
- **Sem hash, por decisão.** O `strong_id` não especializa `std::hash`. Iterar um
  contêiner `unordered_*` segue uma ordem que depende da implementação e do
  histórico de inserção, e isso quebra o replay determinístico (GW-17). Com `<=>`,
  o id serve de chave em `std::map` e `std::set`, cuja ordem é a do próprio id.

### 5. Layout

- **Cabeçalhos ao lado do `.cpp`, incluídos por caminho relativo.** Rejeitada:
  a inclusão não diz de qual camada o tipo vem.
- **Diretório de include por camada (escolhida).** Os cabeçalhos ficam em
  `core/include/gusworld/core/<x>.hpp` e se incluem como
  `<gusworld/core/x.hpp>`. O prefixo `gusworld/core/` torna a camada visível em
  cada linha de inclusão. O `.clang-tidy` já declara
  `HeaderFilterRegex: '(^|/)(core|content|domain|app|present)/include/'`, que casa
  com este layout.

## Decisão

1. **Escopo:** o `D1` entrega identificador, erro e resultado, e nada mais.
2. **Arquivos de produção**, um conceito por arquivo (GW-04):
   - `core/include/gusworld/core/error_code.hpp` e `core/error_code.cpp`: a enum
     `error_code` e `error_code_name()`. O `error_code.cpp` dá ao core uma unidade
     de compilação real.
   - `core/include/gusworld/core/error.hpp`: o POCO `error`.
   - `core/include/gusworld/core/result.hpp`: os apelidos `result<T>` e `status`.
   - `core/include/gusworld/core/strong_id.hpp`: o `strong_id<tag>`.
   - O `core/layer.cpp` que já existe **fica**. Agente não declara nada morto
     (GW-14).
3. **Testes:** em `tests/core/test_*.cpp`, num executável `gusworld_core_tests`
   ligado ao harness próprio (`ADR-025`) e registrado no CTest.
4. **Identificadores** em inglês e `snake_case` (GW-22), com cabeçalho SPDX
   AGPL-3.0-or-later em todo arquivo novo (GW-08).

## Ordem de TDD

São cinco fatias, nesta ordem, e cada uma começa por um vermelho real: o do
cabeçalho que ainda não existe ou o de uma asserção que falha.

1. **Esqueleto do executável de teste.** O `gusworld_core_tests` e o
   registro dele no `tests/CMakeLists.txt`.
2. **`error_code`.** O primeiro teste do `error_code`.

As fatias 1 e 2 entram juntas, porque um executável sem nenhum caso não é um
vermelho válido: o harness recusa o registro vazio por desenho (`ADR-025`).

3. **`error`.** Depende do `error_code`.
4. **`result`.** Depende do `error`.
5. **`strong_id`.** Usa o `invalid_id` e o `result`.

**O que conta como vermelho para o guard** (`ADR-026`, `tools/tdd/in_container.sh`):

- O guard só lê como vermelho (código 1) duas coisas: um erro de compilação com
  local de origem (`arquivo.cpp:linha:coluna: error:` ou `fatal error:`) ou um teste
  que falha no CTest.
- Um `CMake Error`, ou uma regeneração do `build.ninja` que falha, vira código 3
  no contêiner e 126 no guard. O 126 é **fail-open**: a edição de produção passa
  com um aviso, e não houve vermelho nenhum.
- Por isso, em cada fatia, o teste e o registro dele no CMake entram primeiro, e o
  que falta é só o cabeçalho. O vermelho esperado é o `fatal error` da inclusão
  `<gusworld/core/x.hpp>` inexistente.
- **Não listar um `.cpp` que ainda não existe** em `target_sources`. Isso dá erro de
  configure, não vermelho. O `core/error_code.cpp` entra no `core/CMakeLists.txt`
  na mesma edição que o cria, na fase verde da fatia 2.

## Consequências

**Positivas:**

- O `core/` nasce com três tipos que têm consumidor imediato e nenhum
  especulativo (GW-33).
- O erro viaja no retorno, como a GW-17 pede de uma transição de regra.
- O compilador impede misturar ids de etiquetas diferentes.
- Sem hash, nenhum consumidor consegue montar sem querer um `unordered_map` de
  ids cuja ordem de iteração quebre o replay.

**Negativas, aceitas como custo:**

- Quem precisar de busca por hash terá de pedir uma decisão nova. A ausência é
  deliberada, não esquecimento.
- `error_code` começa com um só valor. Cada valor novo é uma mudança em
  `core/`, a camada de que todas dependem.

**Riscos:**

- **`std::expected` fora do GCC.** O guard de TDD compila só com GCC
  (`ADR-026`). O Clang e o MSVC, este com `/std:c++latest`, só se medem no CI
  (`ADR-024`). Se algum dos dois não entregar `std::expected` como o GCC entrega,
  a falha aparece na matriz do CI, não no laço local do guard.

## Dependências registradas

- **Portão 3 da GW-19 (análise estática) precisa enxergar cabeçalhos antes do
  `D1`.** Os dois `enumerate.sh` de `tools/ci/clang_tidy/` e `tools/ci/cppcheck/`
  listam hoje só arquivos `.cpp`, e quatro dos cinco arquivos de produção do `D1` são
  cabeçalhos. O ajuste é feito à parte, fora deste ADR. Aqui fica só a
  dependência.

## Pontos em aberto para o CTO

O que esta especificação não fixou e a fatia de código vai precisar decidir:

- O construtor explícito de `strong_id` é público? Se for, o que ele faz quando
  recebe zero, já que `from_raw` é quem devolve o erro.
- O `strong_id` tem construção padrão? Se tiver, ela produz o zero inválido?
- O nome do acesso ao valor cru (por exemplo, `value()`).
- O tipo subjacente da enum `error_code`, e o que `error_code_name()` devolve
  para um valor fora da enum.

## Cross-refs

- [ADR-025](ADR-025-harness-teste-proprio-minimo.md) (harness de teste próprio)
- [ADR-026](ADR-026-tdd-guard-em-conteiner.md) (guard de TDD em contêiner, códigos 1 e 126)
- [ADR-024](ADR-024-ci-cinco-plataformas-cachyos-container.md) (matriz de CI, onde o MSVC e o Clang se medem)
- `../../../GODS_LAWS.md` GW-04 (átomo), GW-14 (nada é declarado morto), GW-17 (espinha e replay determinístico), GW-19 (TDD estrito e cinco portões), GW-22 (nomes), GW-33 (proibida a abstração especulativa)
- `TODO.md`, item `D1`
