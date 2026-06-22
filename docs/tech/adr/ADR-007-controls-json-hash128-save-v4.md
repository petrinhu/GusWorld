# ADR-007: Persistencia de remapeamento de controles (controls.json) com deteccao de adulteracao (hash 128) e restauracao via backup no save (V4)

Status: Accepted
Data: 2026-06-22
Decisores: lider supremo (petrus), software-architect, backend-engineer

> ACEITO 2026-06-22. O lider cravou todos os forks abaixo e adicionou 3 camadas
> anti-tamper (T1.1 detect-and-respond, T1.2 slot-id selado, T2.2 KDF na origem da
> chave). A parte HEADLESS (POCO puro, ZERO Qt/IO) foi implementada em TDD pelo
> backend-engineer (ctest 658/658 verde; ASan/UBSan limpo; grep-invariante zero-Qt/
> zero-IO mantido). Ver "Decisao do lider (resolucao dos forks)" e "Camadas
> anti-tamper" abaixo. O I/O real (escrever/ler o .json e o save em disco, o caminho
> ~/.gusworld/saves, permissoes 0700/0600, "qual save e o mais recente", a janela de
> aviso com o diff) permanece PLATFORM/APP (TODO), com a fronteira ja pronta.

## Contexto

O subsistema `input_remap` ja vive em `GusEngine/domain/input/` como POCO puro (ADR-002 + F2-E.7): `ActionRegistry` (37 actions canonicas) + os records de binding (`KeyBinding`, `GamepadButtonBinding`, `MouseButtonBinding`, `GamepadAxisBinding`, `ActionBindings`, `InputRemapConfig`, com `config_version = 1`). O header `input_binding.hpp` registra explicitamente que NAO existe serializer portado: "quando a persistencia do remap virar necessaria, o serializer entra junto do backend (decisao de design para o lider)". Este ADR e essa decisao.

O lider desenhou a feature em detalhe (verbatim no fim deste ADR). O conceito esta FECHADO; este ADR crava o COMO, sem reabrir o conceito. Em uma frase: o remap do jogador mora num arquivo `*_controls.json` legivel/editavel; ao abrir o jogo, o jogo deteta se o arquivo foi editado a mao (via hash de 128 bits) e, se foi, mostra o diff e oferece restaurar a partir de um backup binario embutido no save mais recente.

Fatos que delimitam a decisao (estado atual, factual):

- `core/crypto/` ja tem SHA-256 e HMAC-SHA256 proprios, dep-free, validados contra vetores FIPS 180-4 / RFC 4231 (ADR-006). `sha256()` devolve `std::array<uint8_t, 32>`.
- `domain/save/` ja existe e foi auditado: envelope binario proprio `GDS2` (MAGIC `"GDS2"` || LENGTH u32 LE || PAYLOAD || HMAC-SHA256 de 32 bytes), migrators forward-only (chain V1 -> V2 -> V3), `SaveData` com `schema_version`, `timestamp_ms` (carimbo epoch ms injetado), 1 autosave (slot 0) + 5 manuais (slots 1..5). Ancora real `gus::domain::kSaveSchemaVersion = 3` em `domain/include/gus/domain/domain_info.hpp` (o comentario "V2" em `save_migrators.hpp` esta desatualizado; a fonte de verdade e o `domain_info.hpp`).
- ADR-006 RECUSOU JSON e dependencia externa no `domain/`. O save ficou binario proprio. O lider quer `controls.json` em JSON legivel. Esta e a tensao central deste ADR.
- A invariante de camadas (ZERO Qt, ZERO dep externa, ZERO I/O real em `core/`+`domain/`) e auditada por grep no CI. Vale para tudo que este ADR cravar como headless.
- O hash 128 NAO e seguranca. A chave/algoritmo sao publicos. E deteccao casual de edicao manual: dizer ao jogador "alguem mexeu nisso fora do jogo". O save ja tem seu proprio selo HMAC com chave embutida; o controls.json e deliberadamente reproduzivel por qualquer um (e pra ser editavel).

## Decisao

### 1. Formato do arquivo de controles: JSON, com serializer/parser PROPRIO minimo (dep-free)

O arquivo e JSON legivel (o lider quer editavel a mao). A saida soberana sobre a tensao com ADR-006: NAO entra lib JSON externa (nlohmann etc.). Entra um **serializer + parser JSON proprio minimo** em `domain/input/`, cobrindo SO o schema flat de controles abaixo (nao um JSON generico). Cobre: objetos, arrays, strings com escape basico (`\" \\ \n \t`), numeros inteiros e float, `true`/`false`. Sem suporte a unicode `\uXXXX`, sem notacao cientifica, sem comentarios. Escopo fechado = parser pequeno e auditavel.

Parsing ROBUSTO que NUNCA faz crash: o parser devolve um `ParseResult` (sucesso com `InputRemapConfig`, ou falha com `enum ParseError` + offset). JSON malformado (o jogador editou e quebrou a sintaxe, faltou virgula, etc.) e tratado como falha graciosa, igual a "arquivo ausente": o jogo cai no fluxo de restauracao/default, nunca em excecao nao tratada na borda headless. A funcao de parse NAO lanca; sinaliza por valor de retorno (estilo do `deserialize`, mas sem throw na borda do JSON, porque entrada de usuario malformada e ESPERADA, nao corrupcao de save selado).

Schema JSON (flat, fiel a `InputRemapConfig` -> `ActionBindings` -> os 4 tipos de binding):

```json
{
  "config_version": 1,
  "actions": [
    {
      "action_name": "move_up",
      "deadzone": 0.5,
      "keys": [
        { "keycode": 87, "ctrl": false, "shift": false, "alt": false }
      ],
      "gamepad_buttons": [ { "button_index": 11 } ],
      "mouse_buttons": [ { "button_index": 1 } ],
      "gamepad_axes": [ { "axis": 1, "axis_value": -1.0 } ]
    }
  ]
}
```

Regras de mapeamento (1:1 com os structs ja existentes em `input_binding.hpp`):
- `config_version` (int) <- `InputRemapConfig::config_version`.
- `actions` (array) <- `InputRemapConfig::actions`, **na ordem do `ActionRegistry`** (determinismo, ver item 2).
- `action_name` (string) <- `ActionBindings::action_name`. No parse, validado contra `ActionRegistry::get_by_name`: nome desconhecido = action ignorada (forward-compat, nao quebra).
- `deadzone` (float) <- `ActionBindings::deadzone` (default 0.5 se ausente).
- `keys[].keycode` (int 64-bit) <- `KeyBinding::keycode` (`long long`); `ctrl`/`shift`/`alt` (bool) <- os 3 modifiers.
- `gamepad_buttons[].button_index`, `mouse_buttons[].button_index` (int).
- `gamepad_axes[].axis` (int) + `axis_value` (float).
- Campos ausentes em arrays usam o default do struct (listas vazias sao validas). Chaves desconhecidas no JSON sao ignoradas (forward-compat).

### 2. Hash de 128 bits = SHA-256 truncado, sobre o JSON CANONICO normalizado

O hash 128 = primeiros 16 bytes (128 bits) do `sha256()` ja existente em `core/crypto`. Tipo: `std::array<uint8_t, 16>` (ou hex de 32 chars na borda JSON, ver item 3). Usa o SHA-256 cru, NAO o HMAC (nao e seguranca; chave publica seria teatro).

O hash e computado sobre o **JSON CANONICO re-serializado**, NAO sobre os bytes crus do arquivo. Justificativa: o serializer proprio produz uma forma canonica deterministica (ordem de actions = ordem do `ActionRegistry`; ordem de chaves fixa; sem espaco significativo). Hash do canonico => reformatar o arquivo a mao (adicionar espacos, reindentar, reordenar actions) NAO dispara falso-positivo; so uma mudanca SEMANTICA de binding dispara. Isso casa com o intento do lider ("avisar se foi modificado") sem punir cosmetica.

Pipeline determinista da deteccao:
1. Le bytes do arquivo (I/O = platform, fora daqui).
2. Parse -> `InputRemapConfig` (headless). Falha de parse = trata como "ausente/corrompido" -> fluxo de restauracao.
3. Re-serializa o config canonico -> bytes canonicos (headless).
4. `sha256(bytes_canonicos)` -> trunca 16 bytes -> hash-do-arquivo-atual (headless).
5. Compara com o hash-salvo-no-save (item 3).

Esta escolha (canonico vs cru) e o fork #1 que sobe pro lider, porque muda o comportamento do warning. O default cravado e o canonico (menos falso-positivo). Ver "Decisoes pro lider".

### 3. Save V4: bump 3 -> 4, embute backup de config + hash 128

`kSaveSchemaVersion` 3 -> 4 em `domain_info.hpp`. `SaveData` ganha dois campos aditivos (V4):

- `input_remap_backup` (`gus::domain::input::InputRemapConfig`): copia integral do esquema de controles vigente no momento do save. E o que se usa para restaurar.
- `controls_hash128` (`std::array<uint8_t, 16>`): o hash 128 do controls.json canonico vigente no momento do save. E o "valor esperado" contra o qual a deteccao compara.

Gravados em TODO save (auto + manuais), conforme o lider pediu.

Layout binario do PAYLOAD V4 (extensao aditiva do V3, mesmo envelope `GDS2`, little-endian, ordem deterministica preservada). Apos os campos V3 (`enemy_knowledge`), acrescenta:

```
... (campos V1+V2+V3, inalterados) ...
input_remap_backup:
  u32  config_version
  u32  actions_count
  repeat actions_count:
    str  action_name
    f32  deadzone
    u32  keys_count;          repeat: i64 keycode | u8 ctrl | u8 shift | u8 alt
    u32  gamepad_buttons_count; repeat: i32 button_index
    u32  mouse_buttons_count;   repeat: i32 button_index
    u32  gamepad_axes_count;    repeat: i32 axis | f32 axis_value
controls_hash128:
  16 bytes (raw)
```

Reusa as mesmas primitivas de write/read (str, list, mapas em ordem de chave) do serializer V3, exatamente como `serialize_save` ja faz; o `input_remap_backup` grava as `actions` na ordem do `ActionRegistry` (determinismo do selo). O HMAC do envelope cobre tudo, inclusive os campos novos: flip de qualquer byte do backup ou do hash quebra o selo (ja garantido pela estrutura do envelope).

Migrator forward-only V3 -> V4 (`migrate_v3_to_v4`): popula `input_remap_backup` com o **default canonico de bindings** (ver item 5) e `controls_hash128` com o hash 128 desse default canonico. Semantica honesta de um save V3: "nao havia backup de controles; assume-se o default". Mesma forma do `migrate_v1_to_v2` (popular campo novo com valor neutro honesto). Soma o passo na chain + bumpa a ancora; o test-guarda `current_schema_version() == kSaveSchemaVersion` trava o esquecimento.

Oraculo de equivalencia semantica (roundtrip + tamper) estende-se para os campos novos:
- Roundtrip: `SaveData` com `input_remap_backup` nao-trivial + `controls_hash128` arbitrario -> `serialize_save` -> `deserialize_save` -> igual byte-a-campo (o `operator== = default` de `InputRemapConfig` ja existe e cobre a comparacao profunda; `std::array<uint8_t,16>` compara trivialmente).
- Tamper: byte-flip dentro da regiao do backup OU do hash128 dentro do envelope -> `SaveIntegrityError`.
- Migracao: fixture V3 selada -> load -> backup = default canonico, hash128 = hash do default. Adiciona o passo a chain de fixtures por versao.

CONTRA-ARGUMENTO (registrado, decisao fica com o lider): embutir o backup de config em TODO save acopla um subsistema (input) ao formato de TODO save e infla cada save com a copia integral do esquema (37 actions). Em DEV o custo e trivial (kilobytes); a objecao e de design, nao de bytes. Alternativas mais limpas existem (sidecar do save mais recente, arquivo de backup unico) e estao no fork #2. O DEFAULT cravado e o que o lider pediu (embutido em todo save), porque (a) garante que QUALQUER save recente serve de fonte de restauro sem depender de um arquivo extra que pode sumir, e (b) o hash precisa mesmo viajar no save para a deteccao funcionar offline. Eu nao considero a alternativa CLARAMENTE melhor a ponto de sobrepor o pedido do lider; sobe como fork informativo, com minha recomendacao de manter o embutido.

### 4. Deteccao + diff (headless, domain)

`bool controls_were_modified(hash_atual, hash_salvo)`: compara as duas `std::array<uint8_t,16>`. Igual = nao modificado; diferente = modificado. Sem timing-safety (nao e seguranca). O "hash_salvo" vem do save mais recente (item 5).

Diff entre dois `InputRemapConfig` (puro, sem UI). Estrutura de retorno:

```
struct BindingChange {
  std::string action_name;          // "move_up" / chave i18n via ActionRegistry
  std::string label_i18n_key;       // do ActionRegistry, pra UI traduzir
  std::string was_human;            // representacao legivel do binding anterior  ("Espaco")
  std::string now_human;            // representacao legivel do binding atual     ("D")
};
struct ControlsDiff {
  std::vector<BindingChange> changes;   // so as actions que mudaram, ordem do ActionRegistry
  bool empty() const;
};
ControlsDiff diff_controls(const InputRemapConfig& was, const InputRemapConfig& now);
```

`was_human` / `now_human` sao strings ja formatadas pelo domain a partir dos bindings (ex.: keycode -> "Espaco"/"D", com modifier "Ctrl+A"). A TABELA keycode -> rotulo legivel e dado puro (um mapa estatico em domain ou em ActionRegistry); a traducao i18n final (se "Espaco" vira "Space") e da camada de apresentacao via `label_i18n_key`. O exemplo do lider ("magia: era Espaco, esta D") e exatamente um `BindingChange{action_name="cast_spell", was_human="Espaco", now_human="D"}` renderizado pela UI.

### 5. Restauracao (headless) + regra do "nao restaurar"

`InputRemapConfig restore_from_save(const SaveData& most_recent)`: devolve `most_recent.input_remap_backup`. Puro. A camada de apresentacao entao manda reescrever o arquivo (I/O = platform) com esse config (re-serializado canonico) + recomputa e regrava o hash no proximo save.

Default canonico de bindings (`InputRemapConfig default_controls()`): a fonte unica do esquema de fabrica (WASD + setas + stick etc.), em domain. Usado pelo migrator V3->V4 e como fallback quando nao ha save nenhum (jogo novo, arquivo ausente). O CONTEUDO concreto dos defaults (quais teclas) e design de input que ja vive/vivera no backend; este ADR so crava que existe UMA fonte canonica pura.

Regra "se nao restaurar, proximo save grava o esquema novo": e consequencia natural do item 3. Se o jogador escolhe "fui eu, continua", o jogo segue com o config-do-arquivo-atual; no proximo save, `input_remap_backup` = esse config e `controls_hash128` = o hash dele. A partir dai o arquivo atual vira o novo "esperado" e o warning some. Nenhuma logica especial: o save sempre fotografa o estado vigente.

### 6. Fronteira de camadas (o que e headless AGORA vs o que e app/platform depois)

HEADLESS (implementado AGORA, em `core/`+`domain/`, ZERO Qt/IO):
- Serializer + parser JSON proprio do schema de controles (parse robusto, sem crash).
- Hash 128 (SHA-256 truncado sobre o canonico).
- `controls_were_modified` (compara hashes).
- `diff_controls` + `BindingChange`/`ControlsDiff` + tabela keycode->rotulo legivel.
- `restore_from_save` + `default_controls`.
- Campos V4 no `SaveData` + serializacao/migrator/oraculo.

APP / PLATFORM (NAO agora; fica documentado pro app):
- A JANELA de aviso com o diff renderizado e os botoes "fui eu, continuar (ignora)" / "nao fui eu, restaurar".
- O I/O real: ler/escrever o arquivo `*_controls.json` em disco; ler o save mais recente do disco.
- A traducao i18n dos rotulos via `label_i18n_key`.
- A decisao de QUAL save e o "mais recente" no disco (a regra de ordenacao por `timestamp_ms` ja existe no domain via metadado; a leitura dos arquivos e platform).

Apenas a parte headless e implementada nesta onda. A UI entra quando o app/screens encostar em input settings.

### 7. Onde mora cada peca (paths coerentes com a arvore atual)

- Serializer/parser JSON de controles: `domain/input/controls_json.hpp` + `domain/src/input/controls_json.cpp` (perto de `input_binding.hpp`; e dado de input, nao de save).
- Hash 128 de controles: `domain/input/controls_hash.hpp` (`controls_hash128(const InputRemapConfig&)` -> usa `core/crypto/sha256`). Mora em domain (depende de `InputRemapConfig`), consumindo o core. NAO entra em `core/crypto` (core nao conhece `InputRemapConfig`).
- Diff + restore + defaults: `domain/input/controls_diff.hpp` (diff + `BindingChange`/`ControlsDiff`), `domain/input/controls_restore.hpp` (`restore_from_save`, `default_controls`, `controls_were_modified`). Podem ser 1 ou 2 headers; agrupamento fica a cargo do backend-engineer (convencao mecanica).
- Campos V4: dentro de `domain/save/save_data.hpp` (campos novos no `SaveData`), serializacao em `save_serializer.cpp`, migrator em `save_migrators.*`. `save_data.hpp` passa a incluir `gus/domain/input/input_binding.hpp` (domain->domain, dentro da mesma camada, permitido).

## Opcoes consideradas

Eixo formato do arquivo de controles: (A) binario proprio `GDS2`-like, como o save: coerente com ADR-006, zero parser novo, MAS o lider quer legivel/editavel e binario mata isso. (B) JSON via lib externa (nlohmann): rapido de escrever, mas viola a invariante zero-dep do ADR-006 por 1 arquivo flat. (C ESCOLHIDA): JSON via serializer/parser proprio minimo: preserva zero-dep, atende "legivel/editavel", custo = um parser pequeno de escopo fechado (so este schema), auditavel.

Eixo conteudo do hash 128: (1) sobre os bytes crus do arquivo: simples, mas reindentar/reordenar a mao dispara falso-positivo (o jogador "nao mexeu na semantica" e leva warning). (2 ESCOLHIDA): sobre o JSON canonico re-serializado: so mudanca semantica dispara; casa com o intento. Custo = parse+reserialize antes do hash. Sobe como fork #1 (o lider pode preferir o cru, mais "literal").

Eixo onde mora o backup de config: (i ESCOLHIDA, pedido do lider): embutido em TODO save. (ii) sidecar do save mais recente (1 arquivo `controls_backup.bin` ao lado). (iii) arquivo de backup unico global. Tradeoffs no item 3 + fork #2.

Eixo selo do hash 128: HMAC (chave secreta) vs SHA-256 cru. ESCOLHIDA SHA-256 cru: nao e seguranca, e deteccao; HMAC com chave-no-binario seria teatro de seguranca. O save em si ja tem HMAC com chave embutida pra integridade do envelope; o controls.json e proposital reproduzivel.

## Consequencias

Positivas: invariante zero-dep / zero-Qt do ADR-006 preservada (parser JSON proprio, nao lib); arquivo de controles legivel e editavel pelo jogador (intento do lider); deteccao de edicao manual robusta a reformatacao cosmetica (hash do canonico); restauracao funciona offline a partir de qualquer save recente (backup viaja no envelope ja selado por HMAC); oraculo semantico do save estende-se aos campos novos sem novo mecanismo; toda a logica e headless e testavel sem UI nem disco.

Negativas (aceitas como custo): mais um formato no projeto (JSON de controles) alem do binario do save, com parser proprio a manter; cada save cresce pela copia integral do esquema de controles (trivial em bytes, acoplamento de design registrado); o hash sobre o canonico exige um parse+reserialize correto e estavel (se o serializer canonico mudar de forma entre versoes, o hash de um mesmo config muda; mitigar congelando a forma canonica ou versionando-a junto com `config_version`).

Riscos / atencao: (a) o serializer canonico de controles e agora parte do contrato de hash; qualquer mudanca na forma canonica e breaking para a deteccao (mitigar: congelar via teste de fixture "config X -> bytes canonicos Y -> hash Z" e tratar mudanca como bump de `config_version`). (b) parser JSON proprio e codigo novo de superficie de bug em entrada de usuario: test-first com corpus de JSON malformado (truncado, virgula faltando, tipo errado, numero gigante, profundidade), provando que nenhum caso faz crash, todos caem em `ParseError` gracioso. (c) o migrator V3->V4 assume defaults canonicos para saves antigos: se os defaults mudarem no futuro, saves migrados carregam o default da epoca da migracao (aceitavel; e o save mais recente que manda).

## Reversibilidade

Two-way door no formato do controls.json e no parser (trocavel enquanto em DEV; o arquivo e do jogador local, sem base instalada real). One-way-ish no bump V3->V4 e no layout do payload do save: uma vez que saves V4 existam no disco do jogador, mudar o layout exige novo migrator (forward-only). Por isso o layout V4 e cravado AGORA, antes de haver saves reais. A escolha "hash sobre canonico vs cru" (fork #1) e two-way em DEV mas vira contrato de deteccao assim que houver controls.json no campo; decidir antes de release.

## Decisoes pro lider (forks que NAO decido sozinho)

1. **Sobre o que o hash 128 e computado: JSON canonico normalizado (recomendado) vs bytes crus do arquivo.** Implicacao direta no warning. Canonico = reformatar/reindentar/reordenar a mao NAO dispara aviso, so mudanca real de tecla dispara (menos falso-positivo, mais "inteligente"). Cru = QUALQUER byte diferente dispara, inclusive um espaco a mais (mais "literal/paranoico", mais simples de implementar, mas avisa por cosmetica). RECOMENDACAO: canonico. Casa melhor com "avisar se foi modificado" no sentido que importa (alguem trocou uma tecla), sem incomodar quem so reformatou.

2. **Backup de config: embutido em TODO save (pedido do lider, default) vs sidecar do save mais recente vs arquivo de backup unico.** RECOMENDACAO: manter o embutido em todo save, como pedido. A alternativa sidecar e marginalmente mais limpa (desacopla input do save), mas introduz um arquivo extra que pode sumir/dessincronizar e nao traz ganho real em DEV. Nao e claramente melhor a ponto de contrariar o pedido. Registro o tradeoff e mantenho o default do lider, salvo decisao em contrario.

3. **Nome e escopo do `[player]` no nome do arquivo `[player]_controls.json`.** Ambiguidade: existe multi-perfil de jogador no G1? O CLAUDE.md fala em single-player puro e o playtest e N=3 (Gus Dragon, Iago, etc.). Opcoes: (a) UM perfil unico, arquivo fixo `controls.json` (sem `[player]`), mais simples para G1; (b) multi-perfil real com nome de perfil sanitizado no nome do arquivo (`tester_controls.json`), preparando troca de jogador na mesma maquina (util pro playtest N=3). RECOMENDACAO: (b) com um perfil "default" quando nao ha selecao, porque o playtest com 3 pessoas na mesma maquina ja e um caso real e o custo de prever isso agora (so o nome logico carrega o perfil) e baixo. A camada de I/O (platform) e quem sanitiza e forma o nome; o headless so recebe o `InputRemapConfig`.

4. **Forma canonica do controls.json como contrato de hash versionado.** Sub-decisao tecnica derivada do fork #1 (so relevante se escolher "canonico"): congelar a forma canonica via teste de fixture e tratar mudanca de forma como bump de `config_version` do remap. RECOMENDACAO: sim, congelar. Sem isso, uma melhoria futura no serializer mudaria o hash de configs identicos e dispararia falso-positivo em massa. Decisao tecnica, mas registro porque amarra evolucao futura.

5. **Pretty-print vs compacto no arquivo escrito pelo jogo.** O jogo, ao escrever o controls.json, escreve indentado (legivel pra edicao manual, intento do lider) ou compacto? Como o hash e sobre o canonico (fork #1 recomendado), a indentacao do arquivo no disco NAO afeta a deteccao, entao da pra escrever bonito sem custo. RECOMENDACAO: pretty-print (indentado) no arquivo de disco, hash sobre o canonico compacto interno. Decisao menor, mas alinhada ao "editavel a mao".

## Design do lider (verbatim, requisito)

"Hibrido: arquivo json principal canonico: [player]_controls.json. Mas quero hash 128. Motivo: ao abrir o jogo no perfil do jogador, avisar se o arquivo foi modificado manualmente por causa do hash. Dai perguntar: se foi voce, pode continuar e ignore este arquivo. Se nao foi voce, quer restaurar? Para restaurar, devera existir um binario proprio dentro do savegame (todos os saves) com o esquema de configs salvo como backup. Se nao restaurar, proximo save o novo esquema sera ja salvo. O hash tambem fica salvo no save. Para restaurar, sera usado o save mais recente. O warning deve mostrar as mudancas. Exemplo: magia: era - espaco; esta: D"

## Decisao do lider (resolucao dos forks, 2026-06-22)

1. **Hash sobre o canonico (fork #1).** CRAVADO o canonico (opcao 1 recomendada): reformatar/reindentar/reordenar o arquivo a mao NAO dispara aviso; so a troca real de uma tecla dispara. Implementado: `controls_hash128` = primeiros 16 bytes do SHA-256 da forma CANONICA re-serializada.
2. **Backup embutido em TODO save (fork #2).** Mantido o pedido do lider (embutido em todo save). Implementado: `SaveData::input_remap_backup` gravado em todo save.
3. **Multi-perfil confirmado (fork #3 -> b).** Arquivo `[player]_controls.json`. Sanitize do nome do player (FUNCAO PURA headless): minusculas + espaco e caracteres problematicos de filesystem (`/ \ : * ? " < > |` e espaco) -> `_`. Ex.: "Jose Silva" -> `jose_silva` -> `jose_silva_controls.json`. Nome vazio/so-invalido -> perfil `default`. Implementado em `domain/input/controls_name.{hpp,cpp}`.
4. **Forma canonica congelada (fork #4).** Congelada via fixture de teste ("config X -> 16 primeiros bytes do SHA-256 do canonico"); mudanca de forma = bump de `config_version`.
5. **Pretty-print no disco (fork #5).** O arquivo de disco e pretty (legivel/editavel); o hash e sobre o canonico compacto. Implementado: `serialize_controls_pretty` (disco) vs `serialize_controls_canonical` (hash).

## Camadas anti-tamper (pacote recomendado, escolhido pelo lider)

Alem do hash 128 do controls.json, o lider adicionou 3 camadas ao SAVE V4:

- **T1.1 detect-and-respond.** O load passa a oferecer um caminho NAO-lancante `load_save(bytes, expected_slot) -> LoadOutcome{ LoadResult, SaveData }`, com `LoadResult` = `Ok | HmacInvalid | Corrupt | VersionTooNew | Invalid | WrongSlot`. A camada app decide avisar e oferecer o slot anterior. A logica e pura (sem UI). O `deserialize_save` lancante FOI PRESERVADO como primitiva (os testes legados de load continuam validos); `load_save` o envolve e converte excecao em valor. (Fork de design subido ao lider: ver nota no fim.)
- **T1.2 slot-id selado.** `SaveData::slot_id` (i32) viaja DENTRO do payload selado pelo HMAC. `load_save(bytes, expected_slot)` compara o slot_id selado com o slot de onde a camada de I/O leu: divergencia => `WrongSlot` (arquivo copiado entre slots no gerenciador de arquivos). `expected_slot < 0` = "nao verificar" (import avulso). Saves migrados de pre-V4 (slot_id = -1, nao-selado) adotam o slot lido como origem.
- **T2.2 KDF na origem da chave.** A chave de integridade do HMAC do save passa a ser DERIVADA (`core/crypto/key_derivation`: `derive_key(base, ctx) = SHA-256(base || 0x00 || ctx)`) em vez de embutida CRUA. Transparente ao layout (nao muda o envelope); MUDA o VALOR do HMAC. Como nao ha saves reais (DEV), o re-baseline foi indolor (o oraculo do save e roundtrip+tamper, nao vetores de HMAC congelados). ADR-006 ja registra "so a CHAVE e sensivel".

## Migrator V3 -> V4 (forward-only)

`migrate_v3_to_v4` popula `input_remap_backup` = `default_controls()` (canonico), `controls_hash128` = hash 128 desse default, `slot_id` = -1 (origem nao-selada; o `load_save` por slot atribui o slot lido). Soma o passo a chain V1->V2->V3->V4; `current_schema_version() == kSaveSchemaVersion == 4` (test-guarda).

## Pendencias PLATFORM/APP (fronteira pronta, NAO implementado nesta onda)

- I/O real: ler/escrever `~/.gusworld/saves/<perfil>_controls.json` e os saves em disco; permissoes do diretorio (`0700`) e dos arquivos (`0600`).
- Decidir QUAL save e o "mais recente" (ler arquivos + ordenar por `timestamp_ms`, ja metadado do save).
- A JANELA de aviso com o diff renderizado (`ControlsDiff`) e os botoes "fui eu, continuar (ignora)" / "nao fui eu, restaurar".
- A traducao i18n final dos rotulos via `label_i18n_key`.

## Fork subido ao lider (decisao pendente)

**Contrato do `load_save` vs `deserialize_save`.** O lider pediu "o load passa a retornar LoadResult em vez de so lancar". Para NAO quebrar os ~16 testes legados de load (REQUIRE_THROWS_AS) e proteger o oraculo do save ja auditado, a implementacao ADICIONOU `load_save` (nao-lancante, com `LoadResult` + slot-check) e PRESERVOU `deserialize_save` (lancante) como primitiva interna que `load_save` envolve. Recomendacao do backend-engineer: manter os dois (app usa `load_save`; testes e codigo interno usam a primitiva). Se o lider preferir que `deserialize_save` deixe de existir / passe a nao-lancar, e um segundo passo (migra os testes legados). DEFAULT entregue: os dois coexistem.

Cross-ref: ADR-006 (crypto + formato binario do save + KDF T2.2), `domain/input/input_binding.hpp`, `domain/input/action_registry.hpp`, `domain/input/controls_json.hpp`, `domain/input/controls_hash.hpp`, `domain/input/controls_diff.hpp`, `domain/input/controls_restore.hpp`, `domain/input/controls_name.hpp`, `domain/save/save_data.hpp`, `domain/save/save_serializer.hpp`, `domain/save/save_migrators.hpp`, `core/crypto/sha256.hpp`, `core/crypto/key_derivation.hpp`, `domain/include/gus/domain/domain_info.hpp` (kSaveSchemaVersion).
