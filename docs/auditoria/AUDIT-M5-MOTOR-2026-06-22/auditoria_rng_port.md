# Auditoria: Porta de RNG (injetavel, dominio puro)

- Arquivos: `domain/include/gus/domain/combat/random_source.hpp`, `domain/tests/fixed_random.hpp`, default `NeutralRandom` em `combat_state_machine.cpp`
- Referencia C#: `engine/foundation/turn_combat/CombatStateMachine.cs` (campo `_rng`), `engine/tests/turn_combat/FixedRandom.cs`
- Criterio: 5 (IRandomSource injetavel; dominio puro, NUNCA RNG global nem relogio; semente real fica em app/ por ADR-006; NeutralRandom deterministico)

## Contexto e metodo

A §11 e o ADR-006 exigem que a aleatoriedade do combate entre por porta injetavel, mantendo o dominio puro e deterministico. A semente real (data+hora+ms) e responsabilidade da fronteira `app/`, nunca do dominio. Conferido por leitura do contrato da porta, do default e por grep de fontes de nao-determinismo (cruzado com `auditoria_invariante_4camadas.md` INV-4).

## Achados

| ID | Sev | Descricao | Evidencia | Estado |
|---|---|---|---|---|
| RNG-1 | (OK) | Porta `IRandomSource` abstrata e injetavel; superficie minima `next_double()` (variancia) + `next(int)` (canal) espelha `System.Random.NextDouble()`/`Next(maxValue)` | `random_source.hpp:30-40` | ✓ |
| RNG-2 | (OK) | FSM recebe `IRandomSource&` por injecao e nunca chama RNG global dentro do dominio | `random_source.hpp:7-9`; INV-4 grep vazio (sem `mt19937`/`random_device`/`rand`) | ✓ |
| RNG-3 | (OK) | Semente real (data+hora+ms) deixada para a fronteira `app/`, NAO no dominio (ADR-006 item 5) | `random_source.hpp:14-16` (comentario de contrato); INV-4 confirma ausencia de relogio no dominio | ✓ |
| RNG-4 | (OK) | Default `NeutralRandom` deterministico: `next_double()->0.5` (variancia zero), `next(max)->max-1` (cai em COMUM, fora de FALHA/CRIT) | `combat_state_machine.cpp:45-49` | ✓ |
| RNG-5 | (OK) | Duplo de teste `FixedRandom` deterministico, convencao `next(100)=min(next_int,99)` para selecionar canal | `combat_formula_test.cpp:18-22`, `fixed_random.hpp`; testes `random_source_test.cpp` verdes | ✓ |
| RNG-6 | 🟢 COSMETICO | Comentario STALE na porta descreve a forma ANTIGA de crit (`is_crit = next(100) < crit_chance`), substituida pela §11 (sorteio de canal FALHA/CRIT/COMUM) | `random_source.hpp:37-38` | aberto (so comentario) |

## Detalhe do COS-1 / RNG-6

`random_source.hpp:37-38` ainda diz: "Usado no roll de critico por carta (is_crit = next(100) < crit_chance)". Essa e a forma anterior a §11 (single crit-roll). Na §11 nova, `next(100)` decide UM canal de tres (FALHA/CRIT/COMUM) por faixas contiguas, e `next_double()` so e consumido no COMUM. O comentario nao afeta comportamento (a logica correta vive na FSM `combat_state_machine.cpp:629-649`), mas pode confundir um leitor futuro. Recomendacao: atualizar o comentario para refletir o sorteio de canal da §11. Severidade: COSMETICO. Conserto vai para o especialista (backend-engineer) numa passada de limpeza; o internal-auditor nao altera codigo.

## Conclusao

A porta de RNG esta correta: injetavel, dominio puro (sem RNG global nem relogio), semente real delegada a `app/` por ADR-006, default `NeutralRandom` deterministico e duplo `FixedRandom` para testes. Um unico achado COSMETICO (comentario stale RNG-6), sem impacto funcional. Nenhum critico nem importante.
