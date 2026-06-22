# Auditoria: Save (serializer GDS2, migrators, slots, backup)

- Subsistema: `GusEngine/domain/src/save`
- Testes: `save_serializer_test.cpp`, `save_migrators_test.cpp`, `save_slots_test.cpp`, `save_backup_test.cpp`
- Criterios: 2 (paridade C#), 3 (oraculo semantico), 4 (HMAC antes de migrar), 5 (determinismo)

## Contexto e metodo

Portado de `engine/foundation/save_system/*` (C#: JSON + HMAC) com formato proprio binario compacto GDS2 e oraculo de equivalencia SEMANTICA (ADR-006 decisao 3): NAO byte-a-byte, mas roundtrip identico + tamper rejeitado + determinismo + validate-no-load. `kSaveSchemaVersion` deve ser 3.

## Achados

| ID | Sev | Descricao | Evidencia | Remediacao | Estado |
|---|---|---|---|---|---|
| SAV-1 | (OK) | `kSaveSchemaVersion == 3` (ancora correto, paridade total com o C#) | `domain/include/gus/domain/domain_info.hpp:14`; teste `save_migrators_test.cpp:70` | n/a | ✓ |
| SAV-2 | (OK) | Oraculo (a) roundtrip: rich + minimal preservam TODOS os campos, inclusive enemy_knowledge V3 e timestamp injetado | `save_serializer_test.cpp:87-133` | n/a | ✓ |
| SAV-3 | (OK) | Oraculo (b) tamper: flip rejeitado nas 4 regioes (payload e hmac -> IntegrityError; magic e length -> CorruptError); truncado e vazio -> CorruptError | `save_serializer_test.cpp:155-210` | n/a | ✓ |
| SAV-4 | (OK) | Oraculo (c) determinismo: mesmo objeto+chave gera selo identico; delta de 1 campo muda o selo | `save_serializer_test.cpp:185-196,127-133` | n/a | ✓ |
| SAV-5 | (OK) | Oraculo (d) validate-no-load: payload forjado com HMAC valido mas HP/Xp/kills negativos ou id vazio -> rejeitado por validate() no load | `save_serializer_test.cpp:221-253` (usa `serialize_save_unchecked`) | n/a | ✓ |
| SAV-6 | (OK) | Migrators forward-only V1->V2->V3: save V1 sobe os dois saltos, campos novos vazios (semantica honesta); save futuro (v99) rejeitado | `save_migrators_test.cpp:75-139`; `domain/src/save/save_migrators.cpp:23-73` | n/a | ✓ |
| SAV-7 | (OK) | HMAC checado ANTES de versao/migracao (nunca migra bytes adulterados) | `save_migrators_test.cpp:143-150`; `save_serializer.cpp:375` antes de `:379`/`:399` | n/a | ✓ |
| SAV-8 | (OK) | Persiste EnemyKnowledge V3 reusando `progression::KnowledgeStore` (std::map -> serializacao determinista) | `save_data.hpp:106-109`; `save_serializer_test.cpp:112-125` | n/a | ✓ |
| COS-1 | 🟢 | Comentario stale no header de migrators: "Por que para em V2: o ancora kSaveSchemaVersion = 2" contradiz o codigo (ancora ja e 3, chain ja vai a V3). Herdado de quando o marco parava em V2 | `domain/include/gus/domain/save/save_migrators.hpp:12-15` | Atualizar o comentario para refletir V3 (o codigo abaixo ja esta correto). Sem urgencia. NAO tocado nesta auditoria (read-only sobre o codigo) | ⚠ |
| COS-2 | 🟢 | Comentario stale: MAGIC "GDS2 ... schema atual V2" no serializer; o schema atual e V3 | `domain/src/save/save_serializer.cpp:32` | Trocar "V2" por "V3" no comentario. Sem impacto (MAGIC e a string "GDS2", inalterada) | ⚠ |
| COS-3 | 🟢 | Comentario de cabecalho do teste diz "u32 schema_version (V2 atual...)"; o fixture ja usa `kSaveSchemaVersion`=3 | `domain/tests/save_serializer_test.cpp:17` | Atualizar texto do comentario para V3 | ⚠ |

Nota lateral (nao e achado, sem evidencia de defeito): a chave HMAC embarcada e a string literal `gusengine-cpp-2026-save-v2-hmac-sha256-key` (`save_serializer.cpp:43`). O rotulo "v2" no nome da chave e cosmetico e NAO deve mudar sem cuidado: alterar a string invalidaria todos os saves existentes (a chave entra no MAC). Recomenda-se NAO renomear; documentar que o "v2" no nome e historico. Por isso NAO foi listado como COS a corrigir.

## Conclusao

Save robusto e em paridade semantica total com o C#. Os 3 cosmeticos sao comentarios desatualizados sem efeito em comportamento, build ou seguranca. Slots (1 auto + 5 manuais) e backup chain via port injetavel cobertos por `save_slots_test.cpp` e `save_backup_test.cpp` (suite verde). Nenhum critico.
