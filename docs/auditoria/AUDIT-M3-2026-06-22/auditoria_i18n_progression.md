# Auditoria: i18n + Progression (knowledge/xp)

- Subsistemas: `GusEngine/domain/src/i18n`, `GusEngine/domain/src/progression`
- Testes: `md_translation_loader_test.cpp`, `translation_parity_validator_test.cpp`, `xp_differential_test.cpp`, `enemy_knowledge_tracker_test.cpp`
- Criterio: 2 (paridade semantica com o C#)

## Contexto e metodo

Quatro POCOs portados de `engine/foundation/localization/*` e `engine/foundation/knowledge/*`. Paridade conferida lado a lado no subsistema de maior risco (XpDifferential: tem formula e arredondamento, onde divergencias C#/C++ costumam morar). Correspondencia 1:1 de arquivos C# -> C++ verificada para os 4.

## Achados

| ID | Sev | Descricao | Evidencia | Remediacao | Estado |
|---|---|---|---|---|---|
| PRG-1 | (OK) | XpDifferential: formula identica ao C# (`clamp(1 - max(0, player-enemy)*0.15, 0, 1)`), `kPenaltyPerZone`=0.15, sem bonus reverso (anti catch-up) | C#: `engine/foundation/knowledge/XpDifferential.cs:48-58`; C++: `domain/src/progression/xp_differential.cpp:14-27` | n/a | ✓ |
| PRG-2 | (OK) | Arredondamento determinista equivalente: C# `Math.Round(..., MidpointRounding.AwayFromZero)` -> C++ `std::llround` (round-half-away-from-zero). Validacao de baseXp/zonas negativas em ambos | C#: `XpDifferential.cs:75-79`; C++: `xp_differential.cpp:29-39` | n/a | ✓ |
| PRG-3 | (OK) | EnemyKnowledgeTracker portado, correspondencia 1:1 com `engine/foundation/knowledge/EnemyKnowledgeTracker.cs`; cobertura em suite verde | `domain/tests/enemy_knowledge_tracker_test.cpp` (8KB de casos); ctest verde | n/a | ✓ |
| I18N-1 | (OK) | MdTranslationLoader portado de `MdTranslationLoader.cs` (1:1); parser de chaves UPPER_SNAKE_CASE, multi-linha, last-wins; suite verde | `domain/tests/md_translation_loader_test.cpp`; ref C# `engine/foundation/localization/MdTranslationLoader.cs` | n/a | ✓ |
| I18N-2 | (OK) | TranslationParityValidator portado de `TranslationParityValidator.cs` (1:1): validador de paridade de chaves entre locales; suite verde | `domain/tests/translation_parity_validator_test.cpp`; ref C# `engine/foundation/localization/TranslationParityValidator.cs` | n/a | ✓ |

## Conclusao

Paridade semantica com o C# de referencia confirmada. O ponto de maior risco (arredondamento de XP) foi conferido linha a linha e bate. EnemyKnowledgeTracker, MdTranslationLoader e TranslationParityValidator tem correspondencia direta de arquivo e suites verdes. Nenhum achado de severidade. Aprofundamento de paridade caso-a-caso dos demais nao foi necessario para o porte (todos POCO puros, suites verdes, fonte C# pequena e espelhada).
