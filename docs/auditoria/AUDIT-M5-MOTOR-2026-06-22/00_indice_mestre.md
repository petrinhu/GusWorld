# Dossie de Auditoria Interna: Marco M5 (Motor de combate portado C# -> C++20)

- Projeto: GusWorld (jogo indie, Godot 4 + C# .NET 8; engine portada para C++20 + Qt6)
- Marco auditado: M5 (porta do motor de combate, metade de logica pura) + M5-DMG (formula de dano §11 nova)
- Data: 2026-06-22
- Auditor interno: internal-auditor (dono do livro)
- Auditoria LOCAL (rodada na maquina; Bash/Read/Grep + build + ctest + ASan/UBSan)
- Porte: jogo indie solo, escopo de auditoria ENXUTO e proporcional, mesmo padrao do AUDIT-M3
- Commits auditados (origin/main, HEAD=b0d1055):
  - e3fdce2 fundacoes (enums/constants/records/state)
  - 11ad9c0 atores/filas (CombatActor/InitiativeQueue/WeaknessWheel/CombatState)
  - 246a4a3 ambiente (EnvironmentModifier/Clock/Transitions/Catalog)
  - f775b69 nucleo (CombatStateMachine/ComboTable/IEnemyBrain/ScriptedBrain/PlaceholderCards) + porta RNG (IRandomSource/FixedRandom) + A1 fechado
  - b0d1055 formula de dano §11 nova (canal crit/falha sobre variancia Knowledge)
- Referencias: `docs/design/mecanicas/combat.md` (§7 record Card, §11 formula de dano ATUALIZADA 2026-06-22), ADR-006 (RNG/crypto/formato), ADR-004 (EnvironmentModifier), [[AUDITORIAS]], [[CONTRACT]], [[TESTES]]

## 1. Escopo

Capitulos do livro (proporcional ao porte: motor de combate POCO puro sem Qt nem I/O):

| Capitulo (subsistema) | Arquivos auditados | Detalhamento |
|---|---|---|
| Invariante das 4 camadas | `GusEngine/core`, `GusEngine/domain` (grep) | secao 4 deste indice + `auditoria_invariante_4camadas.md` |
| Formula de dano §11 NOVA (M5-DMG) | `domain/src/combat/combat_state_machine.cpp`, `domain/tests/combat_formula_test.cpp` | `auditoria_formula_dano_s11.md` |
| Paridade de comportamento com o C# (chunks 1-3 + nucleo) | `domain/{src,include}/combat/*`, `engine/foundation/turn_combat/*` | `auditoria_paridade_comportamento.md` |
| A1 fechado (CardFamily alias + validate ordinal) | `domain/include/.../templates/card_family.hpp`, `domain/src/templates/{character,enemy}_template.cpp` | `auditoria_a1_templates.md` |
| Porta de RNG (injetavel, pura) | `domain/include/.../combat/random_source.hpp`, `domain/tests/fixed_random.hpp` | `auditoria_rng_port.md` |
| Qualidade C++20 (ASan/UBSan, ponteiros nao-donos, float) | headers e src de combat | `auditoria_qualidade_cpp.md` |

Metodo: leitura de codigo + leitura do submodulo C# de referencia (`engine/foundation/turn_combat/`) + reproducao do build/teste + build extra sob ASan/UBSan. Achado sem evidencia (arquivo:linha ou saida de comando) NAO entra.

## 2. Reproducao do build (evidencia primaria)

Comandos executados na raiz de `GusEngine/`:

```
cmake --preset linux-release          # exit 0
cmake --build --preset linux-release  # exit 0
ctest --preset linux-release --output-on-failure
```

Resultado do ctest (exit 0):

```
100% tests passed, 0 tests failed out of 523
Total Test time (real) = 1.66 sec
```

Esperado pelo criterio: 523/523. ATINGIDO. Build verde, suite verde. Submodulo C# de referencia presente e populado (`git submodule status` -> `engine (heads/main)`).

Build extra de hardening (ASan + UBSan, Debug, ad-hoc, fora dos presets):

```
cmake -S . -B /tmp/m5_san_build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -g" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"      # exit 0
cmake --build /tmp/m5_san_build                                # exit 0
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=print_stacktrace=1 \
  ./domain/tests/gusengine_domain_tests "[combat],[domain]"    # exit 0
```

Resultado (exit 0): `All tests passed (9834 assertions in 501 test cases)`. ZERO erro de AddressSanitizer (sem use-after-free, sem leak), ZERO erro de UndefinedBehaviorSanitizer (sem UB de float/inteiro). O porte reconfirma limpo sob sanitizers neste ambiente.

## 3. Sumario executivo

O marco M5 (motor de combate) esta solido. A invariante arquitetural das 4 camadas (zero Qt e zero I/O real em `core/` e `domain/`) e respeitada de forma comprovavel: `core/` e `domain/` sao POCO puro, headless. A formula de dano §11 NOVA (item M5-DMG, a unica divergencia INTENCIONAL do C#) foi implementada FIELMENTE ao canon de `combat.md` §11: variancia Knowledge `v = max(0.05, 0.30*exp(-kills*0.10))` preservada; canal unico mutuamente exclusivo FALHA/CRIT/COMUM; `fumbleChance = round(5*exp(-kills*0.50))`; `critChance = max(5, card.CritChance)`; CRIT = `round(danoBase*(1+v)*1.5)`; imunidade (`multFraqueza==0`) curto-circuita ANTES de tocar o RNG (0 consumos); contagem de consumo do RNG por canal (imune=0, falha=1, crit=1, comum=2) testada. O exemplo numerico canonico da §11 (danoBase=45, crit=88, comum 32..58) bate.

Nos demais subsistemas (chunks 1-3 + nucleo), onde o porte e FIEL ao C#, a paridade de comportamento foi conferida lado a lado e bate 1:1: InitiativeQueue (sort estavel por SPD, reorder/recompute/cursor), WeaknessWheel (ciclo e multiplicadores 1.5/1.0/0.66), CombatActor (Shield pool de absorcao, refresh de recursos com mana cap 8 ramp 2+turno), ambiente (EnvironmentModifier/Clock ciclo 5/2/5/2/Transitions/Catalog stacking por produto com cap final `[0.44, 2.25]`), ComboTable (Match por assinatura exata, mult 2.0/1.5), ScriptedBrain (PreviewIntent/DecideAction). O enum `CardFamily` tem ordinais identicos ao C# (`Eletrico=0, Bioquimico=1, Sonico=2, Cinetico=3, Criptografico=4`).

O achado A1 (aberto na auditoria do M3) esta FECHADO: `templates::CardFamily` e alias da fonte canonica `combat::CardFamily` (uma definicao so, sem duplicacao, ordinais 0..4 preservados = contrato binario `.gdt` intacto), e o `validate()` dos templates rejeita ordinal de `family`/`brain` fora de dominio (`>= kCardFamilyCount` / `>= kBrainKindCount` lancam `std::invalid_argument`). Saves/templates NAO regrediram.

A porta de RNG (`IRandomSource`) e injetavel e o dominio e PURO: nenhum RNG global (`std::rand`/`mt19937`/`random_device`), nenhuma leitura de relogio (`chrono`/`time`) dentro do dominio. A semente real (data+hora+ms) fica na fronteira `app/` por ADR-006. O default `NeutralRandom` e deterministico (sempre COMUM, variancia zero).

Nao ha achados criticos (zero 🔴) nem importantes (zero 🟠). Os achados sao cosmeticos: um comentario stale na porta de RNG que ainda descreve a forma antiga da formula de crit; e duas observacoes de fidelidade sem impacto (imunidade 0/0 ainda sem cobertura end-to-end, fiel ao C#; e `recompute_by_speed` usa `stable_sort` no C++ vs `List.Sort` nao-estavel no C#, divergencia benigna que so AUMENTA o determinismo).

Parecer: o marco M5 e o item M5-DMG estao APTOS. Como ha 0 criticos e 0 importantes, recomenda-se a promocao do item M5-DMG no TODO de `🔍 Pendente verificacao` para `✅`. O internal-auditor NAO promove sozinho (decisao de status sobe ao lider via thread principal); a recomendacao fica registrada.

## 4. Invariante das 4 camadas (chave)

| ID | Sev | Descricao | Evidencia | Remediacao | Estado |
|---|---|---|---|---|---|
| INV-1 | (OK) | `grep -rln '#include <Q' GusEngine/core GusEngine/domain` retorna VAZIO | saida de comando (cap. invariante; vazio) | n/a | ✓ |
| INV-2 | (OK) | `grep -rlnE '<fstream>\|<filesystem>\|QFile\|std::filesystem' core domain` retorna VAZIO | saida de comando (vazio) | n/a | ✓ |
| INV-3 | (OK) | Sem Qt real (`\bQt[A-Z]`, `QObject`, `QString`, `Q_OBJECT`) em core/domain; os matches de "Qt" eram substring em comentarios "ZERO Qt" | saida de comando (vazio); ver `auditoria_invariante_4camadas.md` | n/a | ✓ |
| INV-4 | (OK) | Sem RNG global nem leitura de relogio no dominio de combate (`std::rand`/`srand`/`mt19937`/`random_device`/`chrono`/`<ctime>`/`std::time`) | grep vazio; os matches de "clock(" eram o metodo `period_clock()` | n/a | ✓ |

Conclusao: `core/` e `domain/` (combate incluido) sao POCO puro, headless, sem Qt e sem I/O direto, e sem fonte de nao-determinismo embarcada. Invariante CUMPRIDA.

## 5. Contagem de achados por severidade

| Severidade | Quantidade |
|---|---|
| 🔴 CRITICO | 0 |
| 🟠 IMPORTANTE | 0 |
| 🟢 COSMETICO | 3 |
| Total | 3 |

Detalhe dos cosmeticos:
- COS-1: comentario stale em `random_source.hpp:37-38` descrevendo `is_crit = next(100) < crit_chance` (forma anterior a §11). Ver `auditoria_rng_port.md`.
- COS-2: imunidade (`multFraqueza==0`, contagem RNG 0/0) sem cobertura end-to-end via FSM, porque a `WeaknessWheel` ainda nao retorna 0.0 (flag de imunidade = incremento futuro, fiel ao C#). O curto-circuito de codigo existe e esta correto. Ver `auditoria_formula_dano_s11.md`.
- COS-3: `recompute_by_speed` usa `std::stable_sort` no C++ enquanto o C# usa `List.Sort` (nao-estavel). Divergencia benigna: o C++ e MAIS deterministico em empates de SPD; nenhum risco. Ver `auditoria_paridade_comportamento.md`.

Nenhum afeta comportamento observavel, build, paridade canonica ou seguranca.

## 6. Nota ao lider (decisao de design ja tomada, registrada para ciencia)

A nota do item M5-DMG no TODO.md sinaliza uma AMBIGUIDADE entre o briefing ("CRIT no topo", `roll >= 100 - crit`) e a §11 (CRIT contiguo a FALHA, `roll < fumble + crit`). O backend-engineer seguiu a §11 (fonte de verdade canonica). A auditoria CONFIRMA: a implementacao casa exatamente com a §11 (`combat.md:368`). A escolha entre "faixa de CRIT contigua a FALHA" vs "faixa de CRIT no topo do intervalo" NAO altera a probabilidade de cada canal (sao mutuamente exclusivos por ordem de teste, e `P(CRIT) = critChance/100` em ambos os arranjos enquanto `fumble + crit <= 100`). E uma questao de legibilidade do codigo, nao de balance. Sem achado; registrado apenas para que o lider ratifique a §11 como o arranjo canonico definitivo. Decisao de alto valor permanece do lider.

## 7. Parecer de prontidao para auditor externo

APTO com ressalva cosmetica. O dossie e honesto: lista o comentario stale (COS-1) e as duas observacoes de fidelidade (COS-2, COS-3). Nenhum 🔴, nenhum 🟠. A suite de 523 testes cobre os criterios de aceite do porte e a §11 nova (17 testes dedicados de formula, incluindo contagem de consumo de RNG por canal e o exemplo numerico canonico). O porte reconfirma limpo sob ASan/UBSan neste ambiente.

## 8. Recomendacao de especialistas (para a thread principal disparar, NAO bloqueia o dossie)

- `qa-engineer`: testes property-based / fuzz das invariantes de combate. Alvos de alto valor: (a) `P(FALHA) + P(CRIT) + P(COMUM) == 1` e contagem de RNG por canal sob seeds aleatorias; (b) monotonicidade da variancia e da falha conforme `kills` sobe; (c) cap ambiental `[0.44, 2.25]` sob qualquer combinacao de camadas; (d) Shield: invariante `Hp + absorvido` conservado; (e) InitiativeQueue: reorder/recompute nunca corrompe o cursor nem perde atores. A piramide atual (501 unit cases) e forte para o porte; property-based endurece as invariantes numericas da §11.
- `security-engineer`: superficie sensivel BAIXA neste marco (motor POCO puro, sem I/O, sem rede, sem parsing de entrada nao-confiavel ainda). Vale uma passada FUTURA quando a fronteira `app/` plugar (1) a impl concreta de RNG semeada por relogio (ADR-006: confirmar que a semente nao vaza estado nem permite replay de combate em contexto competitivo, se aplicavel) e (2) o consumo de templates `.gdt` vindos de disco/save (o decoder ja foi endurecido no M3). Nada bloqueia M5.

## 9. Estado dos arquivos do livro

- `00_indice_mestre.md` (este)
- `auditoria_invariante_4camadas.md`
- `auditoria_formula_dano_s11.md`
- `auditoria_paridade_comportamento.md`
- `auditoria_a1_templates.md`
- `auditoria_rng_port.md`
- `auditoria_qualidade_cpp.md`
