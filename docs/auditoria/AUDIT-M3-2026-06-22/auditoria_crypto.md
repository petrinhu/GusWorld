# Auditoria: Crypto propria (SHA-256 + HMAC-SHA256)

- Subsistema: `GusEngine/core/src/crypto`
- Testes: `GusEngine/tests/sha256_test.cpp`, `GusEngine/tests/hmac_sha256_test.cpp`
- Criterio 4 do escopo: vetores oficiais + HMAC checado ANTES de migrar

## Contexto e metodo

ADR-006 ratifica crypto propria (sem dependencia externa, sem Qt). A prova de correcao e o casamento com os vetores publicados. Auditado por leitura dos testes e dos hashes esperados contra os valores canonicos NIST/IETF, mais a execucao da suite.

## Achados

| ID | Sev | Descricao | Evidencia | Remediacao | Estado |
|---|---|---|---|---|---|
| CRY-1 | (OK) | SHA-256 testado contra os 5 vetores oficiais FIPS 180-4 / NIST: "abc", string vazia, 448 bits, 896 bits (2 blocos), 1.000.000 de 'a'. Hashes esperados batem com os valores canonicos | `tests/sha256_test.cpp:38-70` (hashes `ba7816bf...`, `e3b0c442...`, `248d6a61...`, `cf5b16a7...`, `cdc76e5c...`) | n/a | ✓ |
| CRY-2 | (OK) | HMAC-SHA256 testado contra os 7 test cases do RFC 4231 (incluindo key > block 131B e data grande); tags batem | `tests/hmac_sha256_test.cpp:54-117` | n/a | ✓ |
| CRY-3 | (OK) | Propriedades basicas: digest 32B, determinismo, sensibilidade a chave/entrada | `tests/sha256_test.cpp:74-95`, `tests/hmac_sha256_test.cpp:121-137` | n/a | ✓ |
| CRY-4 | (OK) | HMAC verificado ANTES de ler versao/migrar no load do save (anti-tamper na ordem correta) | `domain/src/save/save_serializer.cpp:346` (verify hmac em `unpack_save`) chamado em `:375` ANTES de `read_u32()` versao `:379` e `migrate_to_current` `:399`; teste dedicado `domain/tests/save_migrators_test.cpp:143-150` | n/a | ✓ |

## Conclusao

Crypto propria CORRETA contra os vetores oficiais. A ordem de seguranca critica (integridade antes de interpretar o conteudo) e respeitada e testada. Para hardening avancado (comparacao de tag em tempo constante, modelo de ameaca da chave embarcada), ver recomendacao `security-engineer` no indice mestre secao 7; nao e bloqueio de M3 (a chave embarcada de integridade anti-tamper local e um modelo aceitavel para save de jogo single-player).
