# Auditoria: Qualidade C++20

- Subsistemas: `GusEngine/core`, `GusEngine/domain`
- Criterio: 5 (RAII, sem new/delete cru, optional/string_view, const-correctness, determinismo)

## Contexto e metodo

Checagem de idiomas C++20 modernos por grep e leitura amostral nos headers e fontes de core/domain.

## Achados

| ID | Sev | Descricao | Evidencia | Remediacao | Estado |
|---|---|---|---|---|---|
| CPP-1 | (OK) | Sem `new`/`delete` cru em `core/src` e `domain/src` (gerencia por valor / containers / RAII) | `grep -rnE '\b(new\|delete)\b' core/src domain/src` -> nenhuma alocacao manual | n/a | ✓ |
| CPP-2 | (OK) | Determinismo de serializacao garantido por `std::map` (ordenado) em todas as estruturas serializadas (flags, inventory, quest_progress, relations, character_states, enemy_knowledge) | `domain/include/gus/domain/save/save_data.hpp:95-109` | n/a | ✓ |
| CPP-3 | (OK) | Uso de `std::optional`/`std::string_view` presente no domain (10 unidades) | `grep -rl 'std::optional\|std::string_view' domain/include domain/src` -> 10 | n/a | ✓ |
| CPP-4 | (OK) | Const-correctness: parametros e locais marcados `const` nos fontes (ex.: `const int gap`, `const double factor`); metodos const em headers de structs | `domain/src/progression/xp_differential.cpp:24-25`; `save_data.hpp`, `character_template.hpp`, `enemy_template.hpp` (metodos `const`) | n/a | ✓ |
| CPP-5 | (OK) | Erros por excecoes tipadas e fail-fast (`std::invalid_argument`, `SaveCorruptError`, `SaveIntegrityError`, `SaveVersionTooNewError`, `TemplateCorruptError`, `TemplateIntegrityError`) em vez de codigos de retorno | usados nos testes de save e templates; ver `auditoria_save.md` e `auditoria_templates.md` | n/a | ✓ |

## Conclusao

Qualidade C++20 adequada ao porte: RAII por containers/valor, zero gerencia manual de memoria, determinismo via `std::map`, erros tipados com fail-fast, uso de tipos modernos. Nenhum achado de severidade. Nao foram executados linters estaticos (semgrep/clang-tidy) nesta passada por ser auditoria enxuta read-only; ver recomendacao opcional `qa-engineer` no indice mestre caso se queira cobertura estatica adicional antes do auditor externo.
