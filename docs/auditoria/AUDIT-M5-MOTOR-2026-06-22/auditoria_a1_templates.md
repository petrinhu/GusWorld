# Auditoria: A1 fechado (CardFamily alias + validate de ordinal)

- Arquivos: `domain/include/gus/domain/templates/card_family.hpp`, `domain/src/templates/{character,enemy}_template.cpp`
- Origem: achado A1 ABERTO na auditoria do M3 (templates aceitavam ordinal de family/brain fora de dominio; CardFamily era copia local)
- Criterio: 4 (alias da fonte canonica sem duplicacao; ordinais 0..4 iguais = contrato binario .gdt intacto; validate rejeita ordinal fora de dominio; saves/templates NAO regridem)

## Contexto e metodo

O A1 pedia (a) religar `templates::CardFamily` a fonte canonica do combate (em vez de duplicar o enum) e (b) endurecer o `validate()` dos templates para rejeitar ordinais fora do dominio. Conferido por leitura do header de alias, do enum canonico e dos dois `validate()`.

## Achados

| ID | Sev | Descricao | Evidencia | Estado |
|---|---|---|---|---|
| A1-1 | (OK) | `templates::CardFamily` e ALIAS da fonte canonica `combat::CardFamily`, NAO copia local | `card_family.hpp:32` (`using CardFamily = gus::domain::combat::CardFamily;`) + include de `combat/combat_enums.hpp` (`card_family.hpp:26`) | ✓ |
| A1-2 | (OK) | Ordinais 0..4 identicos por construcao (e o MESMO enum) => contrato binario `.gdt` inalterado | `combat_enums.hpp:33-38` (Eletrico=0..Criptografico=4); alias nao redefine ordinal | ✓ |
| A1-3 | (OK) | `kCardFamilyCount = 5` exposto para a validacao de fronteira | `card_family.hpp:36` | ✓ |
| A1-4 | (OK) | `EnemyTemplate::validate()` rejeita ordinal de `family` fora de `[0, kCardFamilyCount)` | `enemy_template.cpp:49-51` (throw `std::invalid_argument`) | ✓ |
| A1-5 | (OK) | `EnemyTemplate::validate()` rejeita ordinal de `brain` fora de `[0, kBrainKindCount)` (brain permanece template-only) | `enemy_template.cpp:52-54` (throw `std::invalid_argument`) | ✓ |
| A1-6 | (OK) | `CharacterTemplate::validate()` rejeita ordinal de `family` fora de `[0, kCardFamilyCount)` | `character_template.cpp:49-51` (throw `std::invalid_argument`) | ✓ |
| A1-7 | (OK) | Saves/templates NAO regrediram: a suite completa (incl. template_* e save_*) segue 523/523 verde | `ctest --preset linux-release` -> 100% 523/523 (ver indice mestre secao 2) | ✓ |

## Conclusao

O achado A1 da auditoria do M3 esta FECHADO de forma comprovavel: uma unica definicao de `CardFamily` (a do combate), alias no namespace de templates sem duplicacao, ordinais 0..4 preservados (contrato binario `.gdt` intacto), e os dois `validate()` rejeitando ordinal fora de dominio para `family` e `brain` com excecao tipada. A suite de save/templates continua verde (sem regressao). Nenhum achado de severidade.
