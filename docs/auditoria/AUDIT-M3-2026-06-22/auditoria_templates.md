# Auditoria: Templates (envelope GDT1, serializer)

- Subsistema: `GusEngine/domain/src/templates`
- Testes: `template_serializer_test.cpp`, `template_source_test.cpp`, `canonical_templates_test.cpp`, `character_template_test.cpp`
- Criterios: 2 (paridade C#), 3 (oraculo semantico), 5 (qualidade)

## Contexto e metodo

Portado de `engine/foundation/data/TemplateSerializer.cs` (C#: JSON + HMAC) para envelope proprio binario GDT1. Oraculo de equivalencia SEMANTICA do ADR-006. Cobre CharacterTemplate, EnemyTemplate, TemplateSource, CanonicalTemplates.

## Achados

| ID | Sev | Descricao | Evidencia | Remediacao | Estado |
|---|---|---|---|---|---|
| TPL-1 | (OK) | Oraculo (a) roundtrip: character e enemy preservam todos os campos por valor; deck vazio roundtrippa como vazio (nao some) | `template_serializer_test.cpp:72-99` | n/a | ✓ |
| TPL-2 | (OK) | Header valido: MAGIC "GDT1" + length(u32 LE) = total - 8 - 32; distingue de save ("GDS2") | `template_serializer_test.cpp:103-120` | n/a | ✓ |
| TPL-3 | (OK) | Oraculo (b) tamper: flip rejeitado nas 4 regioes (payload/hmac -> IntegrityError; magic/length -> CorruptError); truncado, vazio, so-header -> CorruptError | `template_serializer_test.cpp:124-183` | n/a | ✓ |
| TPL-4 | (OK) | Oraculo (c) determinismo: mesmo objeto+chave -> selo identico; objetos diferentes -> envelopes diferentes | `template_serializer_test.cpp:154-163` | n/a | ✓ |
| TPL-5 | (OK) | Validate-no-load: payload forjado a mao com HMAC valido mas max_hp=0 -> rejeitado (std::invalid_argument); fail-fast no serialize de template invalido | `template_serializer_test.cpp:196-234` | n/a | ✓ |
| TPL-6 | (OK) | pack/unpack do envelope cru roundtrippa o payload identico | `template_serializer_test.cpp:238-242` | n/a | ✓ |

## Conclusao

Templates em paridade semantica com o C#, oraculo do ADR-006 integralmente coberto (roundtrip + tamper 4 regioes + determinismo + validate-no-load). Nenhum achado de severidade. CanonicalTemplates e TemplateSource (precedencia de fonte) cobertos pelos respectivos testes na suite verde.
