# Auditoria: Invariante das 4 camadas (motor de combate POCO)

- Subsistemas: `GusEngine/core`, `GusEngine/domain` (foco em `domain/{src,include}/.../combat`)
- Criterio: 1 (core/ e domain/ SEM Qt, fstream, QFile, std::filesystem; sem RNG global; sem relogio)

## Contexto e metodo

A regra arquitetural do GusEngine (engine-design.md secao 2) e que `core/` e `domain/` sao POCO puro headless: nenhuma dependencia de Qt nem de I/O real (essas vivem em `platform/` e `app/`). Para o motor de combate acrescenta-se uma invariante de determinismo: nenhum RNG global nem leitura de relogio dentro do dominio (a aleatoriedade entra por porta injetavel `IRandomSource`, a semente real fica na fronteira `app/` por ADR-006). Verificado por grep exaustivo + inspecao dos poucos falsos positivos.

## Achados

| ID | Sev | Descricao | Evidencia | Remediacao | Estado |
|---|---|---|---|---|---|
| INV-1 | (OK) | Nenhum include de Qt | `grep -rln '#include <Q' core domain` -> VAZIO | n/a | ✓ |
| INV-2 | (OK) | Nenhum `<fstream>`, `<filesystem>`, `std::filesystem`, `QFile` | `grep -rlnE '<fstream>\|<filesystem>\|QFile\|std::filesystem' core domain` -> VAZIO | n/a | ✓ |
| INV-3 | (OK) | Nenhum simbolo Qt real | `grep -rnE '\bQt[A-Z]\|#include <Qt\|QObject\|QString\|QtCore\|QtGui\|Q_OBJECT' core domain` -> VAZIO | n/a | ✓ |
| INV-4 | (OK) | Nenhum RNG global nem relogio no dominio de combate | `grep -rnE '#include <chrono>\|#include <random>\|std::random_device\|std::mt19937\|std::chrono\|std::time\(\|<ctime>\|std::rand\(\|srand\(' domain/src/combat domain/include/gus/domain/combat` -> VAZIO | n/a | ✓ |

## Falsos positivos analisados (transparencia)

- Um grep amplo por `Qt` casou em `combat_state.hpp:5` (comentario "POCO puro, ZERO Qt") e similares: e a propria documentacao da invariante, nao uso de Qt. Confirmado vazio no grep estrito (INV-3).
- Um grep por `clock(` casou em `combat_state_machine.hpp:122` (`period_clock()`) e `combat_state_machine.cpp:183/290` (`advance_period_clock()`): sao nomes de metodo da roda temporal do PERIODO (logica deterministica, sem relogio de parede), nao a funcao `clock()` da libc. Confirmado vazio no grep estrito (INV-4).

## Conclusao

`core/` e `domain/`, incluindo todo o motor de combate, sao POCO puro: zero Qt, zero I/O direto, zero fonte de nao-determinismo embarcada. O I/O e a semente real ficam nas camadas de fora via ports injetaveis. Invariante das 4 camadas CUMPRIDA. Nenhum achado de severidade.
