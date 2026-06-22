# Auditoria: Qualidade / idiomatismo C++20 (motor de combate)

- Subsistema: `domain/{src,include}/.../combat`
- Criterio: 6 (ponteiros nao-donos sem use-after-free obvio; exceptions mapeadas; sem UB de float; sem leak; ASan/UBSan)

## Contexto e metodo

Checagem de idiomas C++20 por grep + leitura amostral, mais um build extra sob AddressSanitizer + UndefinedBehaviorSanitizer rodando a suite de combate. O briefing informa que o porte ja rodou limpo sob sanitizers; reconfirmado neste ambiente.

## Achados

| ID | Sev | Descricao | Evidencia | Estado |
|---|---|---|---|---|
| CPP-1 | (OK) | ASan + UBSan LIMPOS na suite de combate/dominio | build Debug `-fsanitize=address,undefined`; `./gusengine_domain_tests "[combat],[domain]"` -> exit 0, "All tests passed (9834 assertions in 501 test cases)"; zero relatorio de ASan/UBSan | ✓ |
| CPP-2 | (OK) | Sem `new`/`delete` cru em combat (gerencia por valor / containers / RAII) | `grep -rnE '\b(new\|delete)\b' domain/src/combat domain/include/gus/domain/combat` (excluindo `new_magnitude`) -> nenhuma alocacao manual | ✓ |
| CPP-3 | (OK) | Ponteiros nao-donos `CombatActor*` documentados como OBSERVADORES: os atores vivem no escopo do dono (chamador); FSM/fila/CombatState so observam | `combat_state_machine.hpp:17` ("ponteiros NAO-DONOS ... os atores vivem no escopo do dono"); ASan nao reportou use-after-free na suite | ✓ |
| CPP-4 | (OK) | Sem divisao por zero no divisor `100/(100+Def)`: `def() >= 0` garantido por `validate()` (lanca `std::out_of_range`), logo denominador `>= 100` | `combat_actor.cpp:89` (validate def>=0); `combat_state_machine.cpp:596` (divisor) | ✓ |
| CPP-5 | (OK) | Comparacao exata de float `mult_fraqueza == 0.0f` e SEGURA: o valor vem de literais exatos da WeaknessWheel (1.5/1.0/0.66) ou `1.0f` (universal compiler), nunca de aritmetica; a imunidade futura atribuiria 0.0f literal | `combat_state_machine.cpp:601`; `weakness_wheel.hpp:63-65` | ✓ |
| CPP-6 | (OK) | Erros por excecoes tipadas e fail-fast (`std::invalid_argument`, `std::out_of_range`) nos contratos de construcao/dano/cura/fila | `combat_actor.cpp:83-136`; `initiative_queue.cpp:29,46` | ✓ |
| CPP-7 | (OK) | Determinismo de serializacao/iteracao preservado: catalogo de ambiente em `std::map<EnvironmentId, EnvironmentModifier>` (ordenado), Meyers singleton const read-only | `environment_catalog.cpp:327-330` | ✓ |

## Conclusao

Qualidade C++20 adequada e acima do exigido para o porte: RAII por containers/valor, zero gerencia manual de memoria, ponteiros nao-donos com ownership claro e documentado (sem use-after-free detectado), erros tipados com fail-fast, sem UB de float (divisor protegido por invariante de validate; comparacao exata de float segura por construcao), determinismo por `std::map`. O destaque e a reconfirmacao LIMPA sob ASan + UBSan (501 test cases, 9834 assertions, zero diagnostico). Nenhum achado de severidade.
