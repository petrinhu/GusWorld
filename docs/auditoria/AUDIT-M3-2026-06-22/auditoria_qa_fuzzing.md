# Auditoria QA: analise estatica + fuzzing do decoder (M3)

**Tipo:** reforco de QA NAO-BLOQUEANTE (o M3 ja foi aprovado pelo internal-auditor com 0 criticos; ctest 174/174 verde no inicio).
**Escopo:** `GusEngine/core` + `GusEngine/domain`, com foco no DECODER binario (deserialize) de save (GDS2) e templates (GDT1), que recebem bytes NAO-CONFIAVEIS do disco do usuario.
**Data:** 2026-06-22
**Autor:** qa-engineer
**Regra:** somente testes adicionados; ZERO alteracao de codigo de producao. Achados que exijam mudanca de produto sao sinalizados, nao corrigidos aqui.

---

## 1. Analise estatica

| Ferramenta | Disponivel | Rodou | Versao |
|---|---|---|---|
| clang-tidy | sim (`/usr/bin/clang-tidy`) | sim | LLVM 22.1.7 |
| cppcheck | sim (`/usr/bin/cppcheck`) | sim | 2.21.0 |
| semgrep | NAO instalado | nao (nao instalei, por instrucao) | - |

### Comandos

```
cppcheck --enable=warning,style,performance,portability --inline-suppr \
  --std=c++20 -I core/include -I domain/include core/src domain/src

clang-tidy -p . --checks='-*,bugprone-*,cert-*,clang-analyzer-*,misc-*,\
  performance-*,portability-*,-misc-include-cleaner,\
  -bugprone-easily-swappable-parameters' <todos os .cpp de core/src + domain/src>
```

### Achados estaticos (filtrados; ruido descartado)

Nenhum bug real, UB ou leak. Os avisos sao de estilo/cosmetica ou falso-positivo pratico:

| # | Ferramenta | Local | Aviso | Veredito |
|---|---|---|---|---|
| E1 | clang-tidy | `core/src/crypto/sha256.cpp:40,123` | `bugprone-implicit-widening-of-multiplication-result` em `block[i*4]` / `digest[i*4]` | **Falso-positivo pratico.** `i` e indice de laco limitado (0..15 e 0..7); `i*4` <= 60, sem overflow de `int`. Sem risco. Opcional: `static_cast<std::size_t>(i)*4` para silenciar. |
| E2 | clang-tidy | varios headers de `domain` | `misc-non-private-member-variables-in-classes` | **Por design.** Sao POCO/struct de dado puro (`SaveData`, `Vec3`, templates). Encapsular contraria o estilo de dado puro do dominio. Ignorar. |
| E3 | clang-tidy | enums (`CardFamily`, `BrainKind`, `CombatOutcome`, `TemplateOrigin`) | `performance-enum-size` (sugere base menor) | **NAO ACATAR.** O tamanho/base do enum (`std::uint32_t`) e CONTRATO BINARIO do serializer (ordinal lido como u32). Trocar para u8 quebraria o formato. Manter. |
| E4 | cppcheck | `enemy_knowledge_tracker.cpp:40` | `shadowFunction` (parametro sombreia funcao livre `defeated_enemy_types`) | **Cosmetico.** Sombreamento de nome de funcao por parametro; sem efeito funcional. Renomear o parametro melhoraria legibilidade. Baixissimo. |
| E5 | cppcheck | `template_source.hpp:40` | `uninitMemberVarNoCtor` em `TemplateSelection::origin` | **Baixo.** `origin` (enum) sem inicializador default no struct. Os call-sites preenchem, mas um default explicito (`= TemplateOrigin::...`) eliminaria leitura de valor indeterminado se alguem construir agregado parcial. Recomendacao leve (nao no decoder; fora do caminho de bytes externos). |
| E6 | cppcheck | varios | `useStlAlgorithm` (laco cru vs `std::any_of`) | Estilo. Ignorar. |

**Conclusao estatica:** nenhuma correcao obrigatoria. E5 e a unica sugestao com algum valor defensivo (init default de enum-membro), e fica FORA do decoder de bytes nao-confiaveis. Registrada como recomendacao opcional, nao bloqueia.

---

## 2. Fuzzing / property-based do decoder (foco principal)

Adicionados 2 arquivos de teste Catch2, ligados em `domain/tests/CMakeLists.txt`:

- `domain/tests/save_serializer_fuzz_test.cpp` (decoder GDS2)
- `domain/tests/template_serializer_fuzz_test.cpp` (decoder GDT1)

**Determinismo:** todo gerador aleatorio usa SEED FIXA embutida no teste (`kSeed`), garantindo input identico em toda run (sem flakiness por aleatoriedade).

**Contrato exigido:** toda entrada malformada deve ser REJEITADA com erro tipado
(`SaveCorruptError` / `SaveIntegrityError` / `SaveVersionTooNewError` /
`TemplateCorruptError` / `TemplateIntegrityError` / `std::invalid_argument`),
**nunca crash, UB, leitura fora-de-limite ou aceitacao-de-lixo**. Onde o tipo exato
nao e o ponto (lixo aleatorio), o predicado exige terminacao por excecao capturavel
(o ASan/UBSan do CI pegaria qualquer OOB que escapasse).

### Cobertura por tecnica

| Tecnica | save (GDS2) | templates (GDT1) |
|---|---|---|
| Buffer vazio | sim | sim (char + enemy) |
| Curto demais (< header+hmac), todo n em 1..39 | sim | sim |
| **Truncamento em CADA offset** de um artefato valido (boundary, varre 0..size-1) | sim | sim (char + enemy) |
| Magic corrompido byte-a-byte | sim | sim |
| **Magic do dominio cruzado** (GDT1 num save / GDS2 num template) | sim | sim |
| Length gigante `0xFFFFFFFF` (anti-alocacao-absurda) | sim | sim |
| Length zero com payload real | sim | - |
| Length off-by-one (real-1, real+1) | sim | sim |
| **Payload HMAC-VALIDO com string-len interno gigante** (`0xFFFFFFF0`) | sim | sim (id-len) |
| **Payload HMAC-VALIDO com list/deck-count gigante** (`0xFFFFFFFF`) | sim | sim |
| Version futura (forward-only) | sim (`SaveVersionTooNewError`) | n/a |
| Version zero / `0xFFFFFFFF` (negativa como int) | sim | n/a |
| HMAC-valido bem-formado mas invariante violado (HP/atk neg, id vazio) -> `validate()` no load | sim (ja no suite base) | sim (max_hp=0, atk=-1, id vazio) |
| Cauda extra apos campos (`expect_end`) | sim (suite base) | sim |
| **Bytes 100% aleatorios** (2000 iter, seed fixa) | sim | sim (char + enemy = 4000) |
| **Payloads aleatorios DENTRO de envelope HMAC-valido** (2000 iter) | sim | sim |

**Total adicionado: 33 casos de teste** (`ctest -N | grep -c fuzz` = 33).

### Por que o ponto critico foi coberto: o `reserve(count)` guiado por count externo

`read_string_list` / `read_deck` fazem `reserve(count)` com `count` u32 lido do payload.
Hipotese de risco: count gigante (`0xFFFFFFFF`) causaria alocacao de ~4 bilhoes de
elementos antes de qualquer `require()`. **Resultado do fuzzing:** rejeitado de forma
limpa. O `reserve` no maximo lanca `std::length_error`/`std::bad_alloc` (capturavel,
nao crash) e, no caminho real, o `require()` da 1a string barra antes por falta de
bytes. Nenhum crash/OOB observado. Coberto por:
- `save/fuzz: payload HMAC-valido com list-count gigante rejeita`
- `tpl/fuzz: payload HMAC-valido com deck-count gigante rejeita`

### Robustez de overflow do length

`unpack` calcula `expected_total` em `std::uint64_t` e `require()` usa `pos_ + n` em
`std::size_t` (64-bit) contra `buf_.size()`. Como `len`/`count` sao u32 e `pos_` e
64-bit, **nao ha overflow de inteiro** no bounds-check. Confirmado pelos casos de
length gigante e off-by-one: todos rejeitam, nenhum aceita por wrap-around.

---

## 3. Achados (severidade)

### A1 (INFO / decisao de produto) — enum `family`/`brain` fora de range e ACEITO no load

`CharacterTemplate::validate()` / `EnemyTemplate::validate()` checam id, stats e deck,
mas **NAO validam o ordinal de `family`/`brain`**. Um payload HMAC-valido com
`family = 9999` desserializa para um `CardFamily` fora do conjunto `{0..4}` e e
**aceito** (nao lanca).

- **NAO e crash/UB/OOB.** O enum e `std::uint32_t` reinterpretado; ler/comparar/serializar
  o valor 9999 e bem-definido. Sem risco de memoria.
- **E "aceitacao de valor fora do dominio".** Um consumidor downstream (ex.: roda de
  fraqueza, fabrica de brain) que indexe por `family`/`brain` PODE assumir o conjunto
  fechado e ler fora de uma tabela. Esse consumidor ainda nao foi portado (turn_combat
  fora do M3), entao hoje nao ha exploração; e divida de robustez para quando entrar.
- **Severidade:** Baixa / Informativa. Nao bloqueia o M3.
- **Decisao para a thread principal levar ao lider:** validar o ordinal de enum no
  `validate()` dos templates (rejeitar `family > 4` / `brain > 1` com
  `std::invalid_argument`) **vs** aceitar e responsabilizar o consumidor. Recomendacao
  QA: validar no load (defesa em profundidade, coerente com o resto do `validate()`),
  mas e decisao de produto/arquitetura — NAO alterei o produto.
- **Teste que fixa o comportamento ATUAL (nao o ideal):**
  `tpl/fuzz: family fora de range e ACEITO hoje (achado documentado)`
  (tag `[achado]`). Se o produto passar a validar o ordinal, o teste deve virar
  `REQUIRE_THROWS_AS` — sera melhoria, nao regressao.

### A2 (recomendacao opcional, fora do decoder) — `TemplateSelection::origin` sem default

Ver E5. Init default explicito de enum-membro. Fora do caminho de bytes externos.
Severidade: muito baixa. Opcional.

### Sem achados de severidade Media/Alta/Critica

Nenhum crash, UB, leitura fora-de-limite, leak, ou aceitacao-de-lixo (alem do A1, que
e valor-fora-de-dominio sem consequencia de memoria) foi encontrado. O decoder e
solido: bounds-check consistente, integridade (HMAC) antes de materializar, `validate()`
de defesa em profundidade no load, `expect_end()` contra cauda extra.

---

## 4. Resultado da suite

```
ctest --preset linux-release
100% tests passed, 0 tests failed out of 207
```

- Base anterior: 174.
- Novos de fuzzing: 33.
- Total: **207/207 verde.**
- Tempo total: ~0.5 s (os lacos de 2000+ iteracoes sao deterministicos e rapidos).

---

## 5. Veredito QA

Reforco concluido. O decoder de save (GDS2) e templates (GDT1) resiste a entrada
malformada com rejeicao tipada e sem crash/UB/OOB em todas as classes testadas
(truncamento, magic, length, counts internos gigantes, version ilegal, bytes
aleatorios selados e nao-selados). Um unico achado de robustez (A1: enum fora de range
aceito, Baixa/Info) fica registrado para decisao do lider; NAO foi corrigido aqui
(somente teste). Build e suite seguem verdes (207/207). Zero alteracao de producao.
NAO comitado (a thread principal valida e commita).
