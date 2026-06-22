# Auditoria: Formula de dano §11 NOVA (item M5-DMG)

- Arquivos: `domain/src/combat/combat_state_machine.cpp` (impl), `domain/tests/combat_formula_test.cpp` (17 testes)
- Canon de verdade: `docs/design/mecanicas/combat.md` §11 (canonizada 2026-05-26; evoluida 2026-06-22, M5-DMG) + §7 (record Card, CritChance)
- Criterio: 3 (a §11 DIVERGE de proposito do C#; confirmar fidelidade ao canon, NAO ao C#)

## Contexto e metodo

A §11 e a unica divergencia INTENCIONAL do C# de referencia neste marco (o prototipo C# morre no M8; o motor C++ passa a seguir a §11 como contrato de producao). A auditoria confere a implementacao linha a linha contra a §11 e re-roda os 17 testes dedicados de formula. A funcao auditada e `CombatStateMachine::resolve_use_card` (loop por alvo em `combat_state_machine.cpp:572-663`).

## Achados (fidelidade a cada clausula da §11)

| ID | Sev | Clausula §11 | Implementacao | Evidencia | Estado |
|---|---|---|---|---|---|
| S11-1 | (OK) | Cadeia divisiva `(Power+Atk) * (100/(100+Def)) * multFraqueza * multMod * multCombo * multExpose * multAmbiente` | identica; ordem dos fatores preservada (inclui `mult_disrupt` da §9 como penalidade de Power, fator legitimo) | `combat_state_machine.cpp:594-597` | ✓ |
| S11-2 | (OK) | Curto-circuito de imunidade `multFraqueza==0 => dano 0` ANTES de qualquer RNG (0 consumos) | `if (mult_fraqueza == 0.0f) { take_damage(0); ...; continue; }` antes de tocar `rng_` | `combat_state_machine.cpp:601-611` | ✓ |
| S11-3 | (OK) | Variancia `v = max(0.05, 0.30*exp(-kills*0.10))` preservada intacta | `std::max(0.05f, 0.30f * std::exp(-kills * 0.10f))` | `combat_state_machine.cpp:615-616` | ✓ |
| S11-4 | (OK) | `fumbleChance = round(5*exp(-kills*0.50))` | `std::lround(5.0 * std::exp(-kills * 0.50))` (double, depois cast int) | `combat_state_machine.cpp:621-622` | ✓ |
| S11-5 | (OK) | `critChance = max(5, card.CritChance)` (piso global 5%, carta eleva) | `std::max(5, card.crit_chance)` | `combat_state_machine.cpp:623` | ✓ |
| S11-6 | (OK) | UM sorteio de canal `roll = rng.Next(0..99)`; `roll<fumble`->FALHA; `roll<fumble+crit`->CRIT; senao COMUM | `rng_->next(100)`; cadeia `if/else if/else` identica e mutuamente exclusiva | `combat_state_machine.cpp:629-649` | ✓ |
| S11-7 | (OK) | FALHA: `danoFinal=0`, log estetica erro de compilacao | `damage=0; channel_suffix=" FALHA DE COMPILACAO";` | `combat_state_machine.cpp:633-636` | ✓ |
| S11-8 | (OK) | CRIT: `round(danoBase*(1+v)*1.5)`, sufixo `[CRITICO]` | `std::lround(base_damage * (1.0f+v) * 1.5f)`; `channel_suffix=" [CRITICO]"` | `combat_state_machine.cpp:637-641` | ✓ |
| S11-9 | (OK) | COMUM: 2o roll `r=rng.NextDouble()`, `round(danoBase*(1+(v*2*r-v)))` | `r=rng_->next_double()`; `std::lround(base_damage*(1+(v*2*r-v)))` | `combat_state_machine.cpp:643-648` | ✓ |
| S11-10 | (OK) | Ordem/contagem de consumo do RNG: imune=0, falha=1, crit=1, comum=2 | imune retorna antes do `next(100)`; FALHA/CRIT consomem so o `next(100)`; COMUM consome `next(100)`+`next_double()` | testado em `combat_formula_test.cpp:359-385` (CountingRandom: falha 1/0, crit 1/0, comum 1/1) | ✓ |
| S11-11 | (OK) | `multExpose` ultimo fator antes do sorteio; so UseCard | `mult_expose = 1 + Mag/100` na cadeia divisiva; ataque basico subtrativo nao usa | `combat_state_machine.cpp:580-588` | ✓ |
| S11-12 | (OK) | Um unico `round` no final por canal | `std::lround` aplicado uma vez por canal (CRIT e COMUM); FALHA e 0 fechado | `combat_state_machine.cpp:640,648` | ✓ |

## Exemplo numerico canonico da §11 (verificado)

Dado da §11: `Power=20, Atk=10, Def=0, multFraqueza=1.5, demais=1.0` => `danoBase = 30 * 1.0 * 1.5 = 45`. `kills=0` => `v=0.30`, `maxArma=58.5`, `fumble=5%`, `crit=max(5,0)=5%`.

| Canal | Esperado §11 | Verificado | Evidencia (teste) |
|---|---|---|---|
| CRIT | `round(58.5*1.5)=88` | dano de crit usa `round(danoBase*(1+v)*1.5)`; teste analogo (danoBase=10,v=0.30) confere `round(10*1.30*1.5)=20` | `combat_formula_test.cpp:215-228, 281` |
| COMUM | faixa 32..58 (r in [0,1)) | `danoBase*(1+(v*2r-v))`; r=0 -> 45*0.70=31.5->32; r->1 -> 45*1.30=58.5->58 (limite); testes de fundo/topo confirmam | `combat_formula_test.cpp:161-177` |
| FALHA | 0 + log "FALHA DE COMPILACAO" | dano 0, log presente | `combat_formula_test.cpp:289-300` |

A escada de decaimento da falha (`5,3,2,1,1,0,0` para kills 0..6) e validada explicitamente em `combat_formula_test.cpp:324-334`, batendo com a tabela da §11.

## Observacao de fidelidade (cosmetico, sem impacto)

- COS-2 (imunidade 0/0 sem cobertura end-to-end): a §11 manda imunidade (`multFraqueza==0`) curto-circuitar antes do RNG (0 consumos). O codigo faz isso corretamente (`combat_state_machine.cpp:601`). Porem a `WeaknessWheel` ainda nao produz `0.0` (imunidade e "flag de inimigo/lore, incremento futuro", `weakness_wheel.hpp:54`), entao o caminho 0/0 nao e exercitavel via FSM ponta-a-ponta hoje. Os proprios testes documentam isso (`combat_formula_test.cpp:352-356`). Quando a flag de imunidade for plugada, recomenda-se um teste end-to-end que assegure 0 consumos de RNG. Severidade: COSMETICO (o codigo do curto-circuito esta correto e e fiel ao C#, que tambem nao expoe imunidade na roda base).

## Nota ao lider (arranjo CRIT contiguo vs topo)

A nota do M5-DMG no TODO.md cita ambiguidade entre o briefing ("CRIT no topo", `roll >= 100 - crit`) e a §11 (CRIT contiguo a FALHA, `roll < fumble + crit`). O backend-engineer seguiu a §11. A auditoria CONFIRMA fidelidade a §11 (`combat.md:368`). Os dois arranjos produzem `P(CRIT)=critChance/100` identico (canais mutuamente exclusivos por ordem de teste, enquanto `fumble+crit <= 100`): e diferenca de legibilidade, nao de balance. Sem achado. Registrado so para o lider ratificar a §11 como arranjo canonico.

## Conclusao

A formula de dano §11 NOVA esta implementada FIELMENTE ao canon de `combat.md` §11 em todas as 12 clausulas verificadas, com o exemplo numerico canonico batendo e a contagem de consumo de RNG por canal testada. Nenhum achado critico nem importante. Um cosmetico de cobertura futura (COS-2).
