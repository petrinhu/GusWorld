# AUDIT-M3 Seguranca: crypto propria do GusEngine + uso (anti-tamper save/templates)

**Data:** 2026-06-22
**Persona:** Security Engineer / AppSec (defensive-only)
**Escopo:** revisao read-only de `GusEngine/core/crypto/` (SHA-256 + HMAC-SHA256 proprios) e do uso em `GusEngine/domain/src/save/save_serializer.cpp` e `GusEngine/domain/src/templates/template_serializer.cpp`. Reforco nao-bloqueante sobre o M3 ja aprovado pelo internal-auditor (0 criticos).
**Modelo de ameaca (ADR-006):** integridade anti-tamper CASUAL de save/template LOCAL single-player. Chave fixa embutida no binario, extraivel por decompile. NAO ha sigilo, NAO ha rede, NAO ha multiplayer. A chave embarcada e by-design, NAO e achado.
**Referencias:** FIPS 180-4 (SHA-256), RFC 2104 / RFC 4231 (HMAC-SHA256), CWE-789 (alocacao sem limite), CWE-130 (length destrutivo), CWE-20 (validacao de entrada), CWE-208 (timing).

---

## Sumario executivo

A implementacao propria esta **correta e disciplinada** para o modelo declarado. SHA-256 e HMAC-SHA256 seguem FIPS/RFC fielmente, a comparacao de tag ja e em tempo constante (`fixed_time_equals`), e a ordem de verificacao do decoder e segura (integridade ANTES de migrar, fail-secure por excecao). O decoder valida magic, total declarado e bounds de leitura.

O unico vetor real no modelo de ameaca e **DoS por pre-alocacao** (`reserve(count)` / `require(len)` com `count`/`len` u32 atacante-controlados ANTES de checar contra o buffer restante): um save/template forjado de poucos bytes pode pedir alocacao de ate ~4 GiB e derrubar o jogo por `bad_alloc`. Impacto local (o atacante so corrompe o proprio save), mas e um crash trivial de blindar. Classificado IMPORTANTE 🟠 (1 achado). Demais: 4 COSMETICOS 🟢. Zero CRITICOS 🔴.

**Precisa de decisao do lider:** SIM, 1 item. O fix do IMPORTANTE (IMP-01) toca codigo de producao (2 decoders). Patch descrito abaixo, NAO aplicado.

| Sev | ID | Titulo | Estado Auditado |
|---|---|---|---|
| 🟠 IMPORTANTE | IMP-01 | DoS por pre-alocacao no decoder (reserve/require com u32 nao-confiavel) | ⚠ (fix recomendado, decisao do lider) |
| 🟢 COSMETICO | COS-01 | `fixed_time_equals` pode ser otimizada/curto-circuitada pelo compilador | ⚠ (mitigacao opcional) |
| 🟢 COSMETICO | COS-02 | Eixo 1 (timing) tem relevancia ~nula no modelo local; documentar | ✓ |
| 🟢 COSMETICO | COS-03 | `read_f64` aceita NaN/Inf nos bits do save (defesa em camada de validacao, nao no codec) | ✓ |
| 🟢 COSMETICO | COS-04 | Duplicacao do envelope/Reader entre save e template (DRY/manutencao do contrato de seguranca) | ✓ |

---

## Eixo 1: comparacao de tag HMAC em tempo constante

**Estado Auditado: ✓ (ja implementado corretamente)**

`core/src/crypto/hmac_sha256.cpp:69-77` implementa `fixed_time_equals` por XOR acumulado sobre os 32 bytes, sem early-exit:

```
diff |= a[i] ^ b[i];   // varre os 32 bytes sempre
return diff == 0;
```

Ambos os decoders usam essa funcao no ponto de verificacao, NAO `==`/`memcmp`:

- `domain/src/save/save_serializer.cpp:351` — `if (!crypto::fixed_time_equals(expected, actual))`
- `domain/src/templates/template_serializer.cpp:189` — idem.

Nao ha `memcmp`, `std::equal` nem `operator==` de `std::array` na verificacao de tag em nenhum dos dois arquivos. Correto.

**Relevancia real no modelo (save local):** BAIXA, beirando nula. Um timing-oracle de HMAC permite forjar uma tag valida byte-a-byte medindo o tempo de rejeicao, MAS exige: (a) um oraculo remoto/repetido de verificacao, (b) ausencia de outro caminho mais barato. Aqui o atacante tem a chave (esta no binario que ele controla): ele computa o HMAC correto diretamente, sem precisar de timing. O `fixed_time_equals` aqui e **higiene defensiva correta e barata**, espelha `CryptographicOperations.FixedTimeEquals` do C# de referencia, e protege contra o cenario futuro de o mesmo codec ser reusado num contexto com chave secreta (ex.: save em nuvem, validacao server-side). **Recomendacao: MANTER.** Nenhuma mudanca necessaria. Ver COS-01/COS-02 para o residual cosmetico.

---

## Eixo 2: correcao/robustez da implementacao propria

**Estado Auditado: ✓ (correto contra os vetores; revisao de logica confirma)**

### SHA-256 (`core/src/crypto/sha256.cpp`)

| Aspecto | Linha | Avaliacao |
|---|---|---|
| Constantes K[0..63] (raizes cubicas) | 17-30 | Conferem com FIPS 180-4 4.2.2. |
| Estado inicial H[0..7] (raizes quadradas) | 90-91 | Conferem com FIPS 180-4 5.3.3. |
| Round function (Sigma0/1, ch, maj, t1, t2) | 56-72 | Correto, fiel a 6.2.2. |
| Message schedule W[16..63] | 45-51 | sigma0/sigma1 corretos. |
| Padding: byte `0x80` apos a mensagem | 105 | Correto. |
| Length em BITS, 64-bit, BIG-ENDIAN | 111-115 | `bit_len = size*8`, gravado big-endian no fim do tail. Correto. |
| Decisao 1 vs 2 blocos de tail (`rem >= 64-8`) | 108 | Correto: se nao cabe o `0x80` + 8 bytes de length no resto do bloco, usa 2 blocos. |
| Mensagem vazia | n/a | `size=0`: nenhum bloco completo, tail de 1 bloco com `0x80` em [0] e length=0. Produz o digest canonico de "" (validado pelo vetor FIPS). |
| Mensagem grande | n/a | Loop por blocos de 64; ver ressalva de length abaixo. |

**Ressalva de correcao teorica (NAO um achado pratico):** `bit_len = static_cast<std::uint64_t>(size) * 8u` (linha 111). Em plataforma 64-bit, `std::size_t` e 64 bits; se `size >= 2^61`, `size*8` estoura 64 bits e o length fica errado. Isso e um limite intrinseco do SHA-256 (campo de length e de 64 bits, mensagem maxima 2^64-1 bits). Nenhum save/template chega perto de 2 exabytes, entao **sem impacto no modelo**. Documentado por completude.

### HMAC-SHA256 (`core/src/crypto/hmac_sha256.cpp`)

| Aspecto | Linha | Avaliacao |
|---|---|---|
| ipad 0x36 / opad 0x5c | 30-33 | Correto (RFC 2104). |
| key > block (64): reduzida por SHA-256 | 20-22 | Correto (RFC 4231). Resultado de 32 bytes em key_block de 64, zero-pad implicito. |
| key < block: zero-pad ate 64 | 19,24 | `key_block{}` zero-inicializado, `memcpy(key, key_size)`. Correto. |
| key vazia (key_size=0) | 24 | `memcpy(dest, key, 0)`: no-op valido. key_block fica todo-zero. Resultado = HMAC com chave nula, valido e deterministico. |
| inner = SHA256(ipad \|\| msg) | 36-40 | Correto. |
| outer = SHA256(opad \|\| inner) | 43-47 | Correto. |

**Cuidado de ponteiro (NAO um achado no uso atual):** `memcpy(key_block, key, key_size)` com `key` potencialmente nullptr. No codigo de producao a chave sempre vem de `embedded_key().data()` sobre um vector nao-vazio, entao `key != nullptr`. So seria UB se alguem chamasse `hmac_sha256(nullptr, 0, ...)` diretamente. `memcpy(dst, nullptr, 0)` e tecnicamente UB pelo padrao C, embora benigno na pratica. Como a API publica aceita ponteiro cru, vale uma nota no header ou um guard `if (key_size)`. Cosmetico, fora do uso atual; nao listado como achado separado para nao inflar o relatorio.

**Validacao contra vetores:** ADR-006 e os headers afirmam gate TEST-FIRST contra FIPS 180-4 (SHA-256) e os 7 casos do RFC 4231 (HMAC). A revisao de logica acima e consistente com isso. O qa-engineer cobre a execucao dos vetores; aqui confirma-se que a logica que passa nos vetores nao tem desvio escondido (ex.: endianness do digest final em sha256.cpp:122-127 e big-endian, correto).

---

## Eixo 3: robustez do decoder a entrada hostil

O save/template e lido do disco do usuario: entrada NAO-confiavel (CWE-20). Ambos os decoders (`unpack_save` / `unpack`) seguem o mesmo desenho, avaliado em conjunto.

### O que esta CORRETO (✓)

1. **Magic validado ANTES de qualquer leitura de campo** — save:327-330, template:160-165. Rejeita arquivo de tipo errado (chaves distintas por dominio: GDS2 vs GDT1, save:42-46 / template:33-38, impede cross-aceitacao). ✓
2. **Tamanho minimo checado antes de indexar header+hmac** — save:324, template:154 (`data.size() < kHeaderLen + kHmacLen`). Evita out-of-bounds na leitura do header e do magic. ✓
3. **Length declarado conferido contra o tamanho real, em u64, ANTES de fatiar** — save:338-342, template:174-179. `expected_total = kHeaderLen + payload_len + kHmacLen` em `std::uint64_t`, comparado com `data.size()`. **Sem integer overflow:** a soma e em 64 bits e `payload_len` e u32 (max ~4.29e9), entao `expected_total` cabe folgado em u64; nao ha wrap. Se nao bate, rejeita. Isso ja neutraliza length gigante e truncamento no nivel do ENVELOPE. ✓
4. **Integridade (HMAC) ANTES de migrar/desserializar** — save:372-375 + comentario "nunca migra bytes adulterados, CONTRACT §7"; template valida no `unpack` antes de qualquer `Reader`. Fail-secure: byte-flip lanca `SaveIntegrityError`/`TemplateIntegrityError`. ✓
5. **Bounds-check em CADA leitura do payload** — `Reader::require(n)` (save:178-182, template:120-125) checa `pos_ + n > buf_.size()` antes de ler u8/u32/u64/f64/string. `pos_ + n` em `std::size_t` (64-bit): com `pos_ <= buf_.size()` (payload <= ~4 GiB) e `n` pequeno, sem overflow de `size_t`. ✓
6. **`expect_end()` rejeita cauda extra** — save:171-175, template:112-117. Payload bem-formado mas com bytes sobrando = corrupcao. ✓
7. **Defesa em profundidade pos-HMAC** — `validate()` apos desserializar/migrar (save:403, template:232/271). Mesmo com HMAC valido (chave vazada / schema drift), invariantes de dominio sao rechecadas. Excelente: nao confia so no selo. ✓
8. **Payload vazio** — `payload_len = 0` e total = 8+0+32 = 40 bytes: aceito pelo envelope, HMAC confere, e ao desserializar o primeiro `read_u32()` (version) dispara `require(4)` sobre buffer de 0 bytes -> excecao limpa. Sem crash. ✓
9. **Versao futura / invalida (save)** — save:382-387: `version > current` -> `SaveVersionTooNewError`; `version < 1` -> `SaveCorruptError`. Forward-only correto. ✓

### IMP-01 🟠 — DoS por pre-alocacao (reserve/require com contagem u32 nao-confiavel)

**Arquivos:**
- `GusEngine/domain/src/save/save_serializer.cpp:147-149` (`read_string`: `require(len)` aloca string de `len`), `:155-160` (`read_string_list`: `reserve(count)`), `:159` (loop).
- `GusEngine/domain/src/templates/template_serializer.cpp:92-99` (`read_string`), `:101-109` (`read_deck`: `reserve(count)`).

**Categoria:** CWE-789 (Memory Allocation with Excessive Size Value) / CWE-130 (Improper Handling of Length Parameter Inconsistency). DoS local.

**Descricao:** o envelope ja garante que o PAYLOAD TOTAL bate com `data.size()` (item 3 acima). Mas DENTRO do payload, cada `count`/`len` e um u32 lido do payload e usado para alocar ANTES de saber se ha bytes suficientes:

- `read_string`: `len` (u32, ate ~4.29e9) -> `require(len)` so falha DEPOIS, mas a construcao `std::string s(begin+pos, begin+pos+len)` so roda apos o `require`. O `require` protege a string em si. **Porem** `read_string_list`/`read_deck` fazem `list.reserve(count)` com `count` u32 ANTES do loop, e cada `count` pode ser ~4.29e9 mesmo num arquivo de 40 bytes.

**Repro conceitual (sem payload completo):** um GDS2/GDT1 valido com HMAC correto (o atacante TEM a chave: pode gerar um arquivo selado) cujo `party_roster`/`base_deck` declara `count = 0xFFFFFFFF`. O decoder chama `reserve(4294967295)` (string list -> ~4.29e9 * sizeof(std::string), dezenas de GiB) ANTES de ler qualquer elemento. Resultado: `std::bad_alloc` / `std::length_error` nao tratada -> crash do processo, ou thrashing de memoria. Tambem aplica a payload truncado: `count` grande num payload curto so e barrado pelo `require` DENTRO do loop, mas o `reserve` ja rodou.

**Impacto no modelo:** o usuario so corrompe/derruba o PROPRIO jogo (save local single-player). Sem escalonamento, sem RCE (alocacao, nao escrita OOB; nao ha `new[count]` cru com escrita). Severidade real = robustez/UX (jogo crasha em vez de dizer "save corrompido"), nao comprometimento. Por isso **IMPORTANTE, nao CRITICO**. Mas e o tipo de crash que o fuzzer do qa-engineer vai achar em segundos e e trivial de blindar.

**Remediacao recomendada (patch descrito, NAO aplicado):** antes de cada `reserve`/alocacao por contagem, validar `count`/`len` contra os bytes restantes do buffer. Como CADA elemento consome no minimo 4 bytes (o u32 de length de cada string), o numero de elementos nunca pode exceder `bytes_restantes / 4`. Adicionar um helper no `Reader` e usa-lo antes de todo `reserve`:

```cpp
// Reader: bytes ainda nao lidos.
std::size_t remaining() const { return buf_.size() - pos_; }

// Maximo de elementos que CABEM, dado o tamanho minimo de cada um.
// Rejeita count absurdo ANTES de reserve (CWE-789).
std::uint32_t bounded_count(std::uint32_t count, std::size_t min_elem_bytes,
                            const char* what) const {
    if (min_elem_bytes != 0 &&
        static_cast<std::uint64_t>(count) * min_elem_bytes > remaining())
        throw SaveCorruptError(std::string("Contagem implausivel em ") + what + ".");
    return count;
}
```

Uso em `read_string_list`/`read_deck` (min_elem_bytes = 4, o u32 de length por string), nos int-maps e char-states (min_elem_bytes = bytes minimos por entrada), e em `read_string` (clamp de `len` contra `remaining()` antes da construcao; o `require` ja cobre, mas falhar cedo evita mensagem confusa). Alternativa minima e suficiente: trocar `reserve(count)` por nao chamar `reserve` (deixar o vector crescer) ou `reserve(std::min(count, remaining()/4))`. A versao com `bounded_count` e a defesa mais clara e barata.

**Verificacao:** teste `deserialize_rejects_implausible_count` que sela (com a chave de teste) um payload com `count = 0xFFFFFFFF` e exige `SaveCorruptError`/`TemplateCorruptError` em vez de `bad_alloc`/crash. Entra na bateria de fuzzing do qa-engineer (ver "Pontos a fuzzar").

**Defesa em profundidade:** mesmo sem o fix, o impacto e local; com o fix, o decoder degrada graciosamente ("save corrompido") em vez de derrubar o jogo. Recomenda-se tambem rodar o decoder sob ASan/UBSan no fuzzing (ja previsto no kit do projeto).

> **Decisao do lider necessaria:** aplicar IMP-01 toca 2 arquivos de producao (save_serializer.cpp, template_serializer.cpp). Patch acima descrito, NAO aplicado. Sinalizar a thread principal.

---

## Achados cosmeticos

### COS-01 🟢 — `fixed_time_equals` pode ser enfraquecida pelo otimizador

**Estado Auditado: ⚠ (mitigacao opcional, sem impacto no modelo atual)**
`hmac_sha256.cpp:69-77`. A garantia de tempo constante depende do compilador NAO transformar o loop XOR-acumulado num early-exit equivalente (improvavel aqui, mas nao contratual em C++). Em crypto real usa-se barreira (ex.: `volatile`, ou rotinas como `CRYPTO_memcmp`/`sodium_memcmp`). **No modelo local (chave conhecida pelo atacante) isso e irrelevante.** Mitigacao opcional, so se o codec migrar para um contexto com chave secreta: marcar `diff` como `volatile std::uint8_t` ou rotear por uma rotina de comparacao endurecida. Nenhuma acao requerida agora.

### COS-02 🟢 — Relevancia do eixo timing no modelo local: documentar

**Estado Auditado: ✓**
O `fixed_time_equals` esta certo e deve ficar, mas vale uma linha no header/ADR explicitando que a protecao de timing e **higiene/portabilidade futura**, nao uma defesa ativa no modelo single-player com chave embarcada (onde o atacante computa o HMAC diretamente). Evita que um leitor futuro superestime a garantia de seguranca do selo. Doc-only.

### COS-03 🟢 — `read_f64` aceita NaN/Inf

**Estado Auditado: ✓ (tratado na camada certa)**
`save_serializer.cpp:139-144` reconstroi `double` por `memcpy` dos bits: um save selado pode conter NaN/Inf em `playtime_seconds`/posicao. Isso NAO e bug do codec (preservar bits e o correto para roundtrip), e a defesa pertence a `SaveData::validate()` (save:403). Confirmar que `validate()` rejeita NaN/Inf onde for invariante de dominio (ex.: playtime negativo/NaN). Fora do escopo crypto; sinalizado ao backend-engineer/qa como ponto de validacao de dominio, nao como achado de codec.

### COS-04 🟢 — Envelope + Reader duplicados entre save e template

**Estado Auditado: ✓ (debito de manutencao, nao vulnerabilidade)**
`pack/unpack` + classe `Reader` + `put_*` sao quase identicos nos dois arquivos (save_serializer.cpp:105-186 / template_serializer.cpp:71-129, e os envelopes). O contrato de seguranca (ordem magic->length->hmac->bounds) vive em dois lugares: um fix de seguranca (ex.: IMP-01) precisa ser aplicado em DOBRO, e ha risco de divergencia futura. Recomenda-se, em refactor posterior, extrair o envelope HMAC + Reader bounded para `core/` (ou um `domain/codec/` compartilhado), parametrizando magic + chave. Nao bloqueia o M3; melhora a manutenibilidade do controle de seguranca. DRY do contrato anti-tamper.

---

## Pontos a fuzzar (handoff ao qa-engineer)

Foco em `deserialize_save` e `deserialize_character`/`deserialize_enemy` como funcoes-alvo do fuzzer (libFuzzer/AFL++ com ASan+UBSan, ja no kit do projeto):

1. **Corpus seed:** um save/template valido selado com a chave de teste -> mutar bytes.
2. **count/len gigante** (IMP-01): forcar `0xFFFFFFFF` em cada campo de contagem (party_roster, party_active, base_deck, flags, inventory, quest_progress, relations, character_states, enemy_knowledge, deck). Esperado pos-fix: excecao de corrupcao, nunca `bad_alloc`/crash.
3. **Truncamento progressivo:** cortar 1..N bytes do fim (header, payload, hmac). Esperado: `*CorruptError`/`*IntegrityError`, nunca OOB.
4. **Length declarado != real:** mexer no campo length do envelope (maior e menor). Esperado: rejeicao no envelope (item 3 do eixo 3).
5. **Magic invalido / cross-tipo:** GDS2 num decoder de template e vice-versa. Esperado: rejeicao.
6. **Byte-flip pos-selo valido:** flipar 1 bit em qualquer posicao de um arquivo valido. Esperado: `*IntegrityError` (HMAC).
7. **version fora de faixa (save):** 0, negativo (via u32 alto), `current+1`. Esperado: `SaveCorruptError`/`SaveVersionTooNewError`.
8. **Strings com bytes nao-UTF8 / nulos embutidos:** garantir que `std::string` com `\0` no meio nao quebra a logica downstream (relevante se IDs forem usados como chave/caminho).
9. **NaN/Inf nos f64** (COS-03): confirmar que `validate()` barra onde for invariante.
10. **Rodar sob ASan/UBSan e checar 0 findings** em todo o corpus mutado.

---

## Conclusao

A crypto propria e o uso anti-tamper estao **solidos para o modelo de ameaca declarado** (integridade casual local). Eixo 1 (timing) ja resolvido e correto; eixo 2 (correcao SHA/HMAC) confirmado fiel a FIPS/RFC; eixo 3 (decoder hostil) tem o envelope bem blindado e UM ponto a endurecer: pre-alocacao por contagem nao-confiavel (IMP-01), DoS local trivial de corrigir.

**Veredito:** aprovado como reforco nao-bloqueante. 0 CRITICOS. 1 IMPORTANTE (IMP-01) com patch descrito que requer decisao do lider por tocar producao. 4 COSMETICOS (1 doc-only, 1 mitigacao opcional, 1 validacao de dominio, 1 debito DRY).

**Nada foi alterado no codigo. Nada commitado.**
