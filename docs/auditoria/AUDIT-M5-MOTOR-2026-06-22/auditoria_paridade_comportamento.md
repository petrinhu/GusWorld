# Auditoria: Paridade de comportamento com o C# (chunks 1-3 + nucleo)

- Subsistemas: InitiativeQueue, WeaknessWheel, CombatActor, ambiente (Modifier/Clock/Transitions/Catalog), ComboTable, ScriptedBrain
- Referencia C#: `engine/foundation/turn_combat/*.cs`
- Criterio: 2 (porte FIEL onde NAO ha divergencia intencional; a §11 esta no capitulo proprio)

## Contexto e metodo

Comparacao lado a lado C# x C++ dos algoritmos portados, com foco nos pontos de maior risco de divergencia (ordenacoes, stacking numerico, regras de absorcao). Os enums foram conferidos por ordinal.

## Achados

| ID | Sev | Subsistema | Conferencia de paridade | Evidencia (C++ / C#) | Estado |
|---|---|---|---|---|---|
| PAR-1 | (OK) | Enum `CardFamily` | ordinais identicos `Eletrico=0, Bioquimico=1, Sonico=2, Cinetico=3, Criptografico=4` | `combat_enums.hpp:33-38` / `CombatEnums.cs:16-22` | ✓ |
| PAR-2 | (OK) | InitiativeQueue (construcao) | sort por SPD desc ESTAVEL (empate mantem ordem de entrada): C++ `std::stable_sort` espelha `OrderByDescending` do LINQ (estavel) | `initiative_queue.cpp:30-34` / `InitiativeQueue.cs:38` | ✓ |
| PAR-3 | (OK) | InitiativeQueue (reorder/cursor) | `reorder_actor` move e re-clampa cursor identico a `ReorderActor`+`Math.Clamp`; `sync_cursor_to` espelha o re-sync do C# | `initiative_queue.cpp:43-70` / `InitiativeQueue.cs:60-101` | ✓ |
| PAR-4 | (OK) | WeaknessWheel (ciclo) | mesmo ciclo `Eletrico->Cinetico->Criptografico->Sonico->Bioquimico->Eletrico` | `weakness_wheel.hpp:8-9,31-44` / `WeaknessWheel.cs:7,22-23` | ✓ |
| PAR-5 | (OK) | WeaknessWheel (multiplicadores) | 1.5 (fraco) / 1.0 (neutro) / 0.66 (resistente); imune 0.0 fora da roda base (incremento futuro, IGUAL ao C#) | `weakness_wheel.hpp:53-65` / `WeaknessWheel.cs` | ✓ |
| PAR-6 | (OK) | CombatActor (Shield pool) | `absorb_with_shield`: `absorbed=min(dano,Mag); Mag-=absorbed; Hp-=(dano-absorbed)`; remove ao zerar; log Absorbed/Expired | `combat_actor.cpp:98-131` / `CombatActor.cs:197-245` | ✓ |
| PAR-7 | (OK) | CombatActor (recursos/turno) | `refresh_resources_for_turn`: AP=maxAP; maxMana=min(cap 8, base 2 + round); recarrega mana sem banking | `combat_actor.cpp:185-189` + `combat_constants.hpp:26,29` / `CombatActor.cs:327-335` | ✓ |
| PAR-8 | (OK) | EnvironmentClock (ciclo) | `Dia(5)->Crepusculo(2)->Noite(5)->Aurora(2)`; telegraph default 2; index_of espelha Array.IndexOf | `environment_clock.cpp:21-55` / `EnvironmentClock.cs:22-46` | ✓ |
| PAR-9 | (OK) | EnvironmentCatalog (stacking + cap) | `mult_ambiente = produto das camadas ativas`, ignora `EnvironmentId::None`, clamp final `[0.44, 2.25]` | `environment_catalog.cpp:334-342` + `combat_constants.hpp:57,62` (kMultAmbienteCapMin/Max) / `EnvironmentModifier.cs` §11 | ✓ |
| PAR-10 | (OK) | ComboTable (Match) | resolucao por assinatura exata (`signature_matches`), deterministica, sem RNG; receitas `pulso_stream` (mult 2.0) e `raiz_null` (mult 1.5 + Regen 5/2 Refresh) | `combo_table.cpp:16-55` / `ComboTable.cs:26-74` | ✓ |
| PAR-11 | (OK) | ScriptedBrain | `preview_intent`/`decide_action`: alvo primeiro player vivo, senao Pass; dano previsto subtrativo `max(kMinDamage, atk-def)` | `scripted_brain.cpp:17-41` / `ScriptedBrain.cs:20-41` | ✓ |

## Observacao de fidelidade (cosmetico, sem impacto)

- COS-3 (`recompute_by_speed` mais estavel que o C#): na construcao da fila ambos usam sort estavel. Mas em `RecomputeBySpeed` o C# usa `_order.Sort((a,b)=>b.Spd.CompareTo(a.Spd))`, que e `List<T>.Sort` (introsort, NAO estavel); o C++ usa `std::stable_sort` (`initiative_queue.cpp:60-64`). Em empate de SPD durante um recompute (ex.: Haste/Slow igualando velocidades), o C++ preserva a ordem relativa anterior enquanto o C# poderia embaralhar. Isto so AUMENTA o determinismo do C++ e e mais previsivel taticamente (alinhado ao espirito do Pillar 2). Como o C# de prototipo morre no M8 e a §4 do design pede previsibilidade, a divergencia e benigna e ate desejavel. Severidade: COSMETICO. Recomendacao: nenhuma acao obrigatoria; se quiser paridade exata 1:1 historica, trocar por sort nao-estavel seria um RETROCESSO de determinismo, entao manter `stable_sort`.

## Conclusao

Onde o porte e fiel (tudo exceto a §11), a paridade de comportamento com o C# bate 1:1 em todos os 11 pontos conferidos, incluindo os de maior risco (ordenacoes, stacking ambiental com cap, absorcao de Shield). Nenhuma divergencia NAO-intencional. A unica diferenca (COS-3) e benigna e favoravel ao determinismo. Nenhum achado critico nem importante.
