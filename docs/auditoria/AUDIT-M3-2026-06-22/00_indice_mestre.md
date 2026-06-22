# Dossie de Auditoria Interna: Marco M3 (Logica pura portada C# -> C++20)

- Projeto: GusWorld (jogo indie, Godot 4 + C# .NET 8; engine portada para C++20 + Qt6)
- Marco auditado: M3 (porta de save + i18n + progression/knowledge + templates + crypto)
- Data: 2026-06-22
- Auditor interno: internal-auditor (dono do livro)
- Porte: jogo indie solo, escopo de auditoria ENXUTO e proporcional
- Referencias: ADR-006 (crypto/HMAC/formato proprio, oraculo SEMANTICO), [[AUDITORIAS]], [[CONTRACT]], [[TESTES]]

## 1. Escopo

Capitulos do livro (proporcional ao porte M3, logica pura POCO sem Qt nem I/O):

| Capitulo (subsistema) | Arquivos auditados | Detalhamento |
|---|---|---|
| Invariante das 4 camadas | `GusEngine/core`, `GusEngine/domain` (grep) | secao 4 deste indice + auditoria_crypto |
| Crypto propria (SHA-256, HMAC) | `core/src/crypto`, `tests/{sha256,hmac_sha256}_test.cpp` | `auditoria_crypto.md` |
| Save (serializer, migrators, slots, backup) | `domain/src/save`, `domain/tests/save_*` | `auditoria_save.md` |
| Templates (GDT1, serializer) | `domain/src/templates`, `domain/tests/template_*` | `auditoria_templates.md` |
| i18n (loader + parity) | `domain/src/i18n`, `domain/tests/*translation*` | `auditoria_i18n_progression.md` |
| Progression (knowledge + xp) | `domain/src/progression`, `domain/tests/{xp_,enemy_}*` | `auditoria_i18n_progression.md` |
| Qualidade C++20 | headers e src de core/domain | `auditoria_qualidade_cpp.md` |

Metodo: leitura de codigo + leitura do submodulo C# de referencia (`engine/foundation/`) + reproducao do build/teste. Achado sem evidencia (arquivo:linha ou saida de comando) nao entra.

## 2. Reproducao do build (evidencia primaria)

Comando executado na raiz de `GusEngine/`:

```
cmake --preset linux-release
cmake --build --preset linux-release
ctest --preset linux-release
```

Resultado (exit 0):

```
100% tests passed, 0 tests failed out of 174
Total Test time (real) = 0.42 sec
```

Esperado pelo criterio: 174/174. ATINGIDO. Build verde, suite verde. Submodulo C# de referencia presente e populado (`git submodule status` -> `engine (heads/main)`).

## 3. Sumario executivo

O marco M3 esta solido. A invariante arquitetural das 4 camadas (zero Qt e zero I/O real em `core/` e `domain/`) e respeitada de forma comprovavel. A crypto propria casa com os vetores oficiais (FIPS 180-4 para SHA-256, RFC 4231 para HMAC-SHA256), e o oraculo de equivalencia semantica do ADR-006 esta implementado nos serializers de save e de templates (roundtrip + tamper nas 4 regioes + determinismo + validate-no-load). A ordem de seguranca critica (verificar HMAC ANTES de ler versao e migrar) esta correta e tem teste dedicado. O `kSaveSchemaVersion` e 3, com chain forward-only V1->V2->V3 e guarda anti-downgrade. A paridade semantica com o C# de referencia foi conferida no subsistema com maior risco de divergencia (XpDifferential) e bate.

Nao ha achados criticos (zero 🔴). Os achados sao cosmeticos (comentarios e rotulos herdados de quando o ancora de schema era V2, agora desatualizados em relacao ao codigo, que ja esta em V3). Nenhum afeta comportamento, build ou seguranca.

Parecer: o marco esta APTO. Recomenda-se a promocao do item M3 no TODO de `🔍 Pendente verificacao` para `✅`.

## 4. Invariante das 4 camadas (chave)

| ID | Sev | Descricao | Evidencia | Remediacao | Estado |
|---|---|---|---|---|---|
| INV-1 | (OK) | `grep -rl '#include <Q' GusEngine/core GusEngine/domain` retorna VAZIO | saida de comando (secao 2 da auditoria; vazio) | n/a | ✓ |
| INV-2 | (OK) | `grep -rlE '<fstream>\|<filesystem>\|QFile' GusEngine/core GusEngine/domain` retorna VAZIO | saida de comando (vazio) | n/a | ✓ |

Conclusao: `core/` e `domain/` sao POCO puro, headless, sem Qt e sem I/O direto. O I/O fica nas camadas de fora (`platform/`, `app/`) via ports injetaveis (ex.: backup chain). Invariante CUMPRIDA.

## 5. Contagem de achados por severidade

| Severidade | Quantidade |
|---|---|
| 🔴 CRITICO | 0 |
| 🟠 IMPORTANTE | 0 |
| 🟢 COSMETICO | 3 |
| Total | 3 |

Detalhe dos cosmeticos: COS-1, COS-2, COS-3 (ver `auditoria_save.md`). Sao comentarios/rotulos stale "V2" num codigo que ja opera em V3; sem impacto funcional.

## 6. Parecer de prontidao para auditor externo

APTO com ressalva cosmetica. O dossie e honesto: lista os 3 stale comments. Nenhum 🔴, nenhum 🟠. A suite de 174 testes cobre os criterios de aceite do ADR-006 e os vetores oficiais de crypto. Para um auditor externo de seguranca aprofundado da crypto propria (analise de timing-safety da comparacao de HMAC, hardening da chave embarcada), ver recomendacao de especialista na secao 7.

## 7. Recomendacao de especialistas (para a thread principal disparar, NAO bloqueia o dossie)

- `security-engineer`: auditoria aprofundada da crypto propria fora do escopo de vetores (comparacao de tag em tempo constante; a chave HMAC e uma string embarcada no binario, o que e aceitavel para integridade anti-tamper de save local, mas NAO e segredo; convem documentar esse modelo de ameaca explicitamente). Severidade potencial: a confirmar pelo especialista; nada bloqueia M3.
- `qa-engineer` (opcional): revisao da piramide de testes e de paths nao cobertos (ex.: fuzzing de payload do decoder binario). A cobertura atual de roundtrip+tamper+truncado ja e forte para o porte.

## 8. Estado dos arquivos do livro

- `00_indice_mestre.md` (este)
- `auditoria_crypto.md`
- `auditoria_save.md`
- `auditoria_templates.md`
- `auditoria_i18n_progression.md`
- `auditoria_qualidade_cpp.md`
