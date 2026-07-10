# Auditoria: Integridade de dados de save (achado-chave CRIT-1)

- Subsistemas: `gus/app/screens/save_load_menu_loop.cpp` (`build_previews_and_cache`,
  `do_save`, `do_delete`), `gus/app/screens/save_load_menu.cpp`
  (`confirm_selected_slot`/`slot_selectable`), `gus/platform/fs/save_file_store.cpp`
  (`save_game`/`load_game`/`delete_save`), `gus/domain/save/save_backup.hpp`
  (rotação N=3).
- Critério: AUD-SEC (robustez de entrada não-confiável de disco — "arquivos que o
  jogador pode adulterar") + AUD-QUALITY (invariante de contrato: "gravar por cima pede
  confirmação").

## Contexto e método

O formato de save é selado (envelope + HMAC, ADR-006) e o `load_game` já degrada com
segurança um arquivo presente-mas-corrompido/adulterado, devolvendo um `LoadOutcome`
com `LoadResult` != `Ok` (`HmacInvalid`/`Corrupt`/`VersionTooNew`/`Invalid`/`WrongSlot`) —
isso é uma decisão de design já correta e testada (M2/ADR-007). O que esta auditoria
verificou é **o que a camada NOVA de UI faz com esse resultado**, porque
`build_previews_and_cache` (o único ponto que varre o disco pra montar a tela) tem uma
política explícita e documentada: um save presente-mas-não-Ok **degrada como slot
VAZIO** para fins de exibição (comentário em `save_load_menu_loop.cpp:163-167`: "os
avisos dedicados são etapa futura, fora do núcleo desta onda").

Isso por si só é um gap **já aceito** pelo líder (ver `auditoria_ux_pontos_lider.md` —
os 2 avisos de load ficaram fora de escopo). **O que esta auditoria encontrou é uma
consequência distinta e mais grave, no modo SAVE, que ninguém sinalizou nem aceitou
explicitamente**: como a tela de Salvar só pede confirmação de sobrescrita quando o
preview diz `occupied == true` (`confirm_selected_slot`, `save_load_menu.cpp:200-210`),
e o slot corrompido tem `occupied == false` (por causa do degrade acima), **o clique/
Enter nesse slot grava DIRETO por cima, sem NENHUM diálogo de confirmação** — mesmo
havendo dado recuperável (cadeia de backup) daquele slot em disco.

Método de verificação: como `save_load_menu_loop.cpp` não tem teste unitário (ver
COV-8), a auditoria escreveu um programa de verificação **efêmero, fora do repositório**
(`/tmp`, não commitado, nunca tocou código de produção) que consome só a API PÚBLICA já
existente (`save_load_menu.hpp` + `save_file_store.hpp`), ligado contra as bibliotecas
estáticas já compiladas do projeto (`libgusengine_{app,platform,domain,core}.a`), para
reproduzir exatamente a sequência que `build_previews_and_cache` + `confirm_selected_slot`
executam.

## Repro executado (evidência real, não teórica)

Passo a passo (diretório de saves 100% de scratch, `/tmp`, nunca o `$HOME` real):

1. `save_game(first, slot=1, dir)` → grava a 1ª versão (ok=1).
2. `save_game(second, slot=1, dir)` → grava a 2ª versão; a 1ª vira `backup1` pela rotação
   já existente (ok=1). `load_game(1, dir)` confirma `result=Ok` (0) antes de corromper.
3. **Corrompe 1 byte do arquivo PRIMÁRIO do slot 1** (simula adulteração/bit-rot — o
   arquivo continua existindo em disco, só o conteúdo quebra).
4. Saída real:
   ```
   has_save(1)=1 (arquivo AINDA existe)
   load_game(1) outcome.has_value()=1 result=1 (HmacInvalid)
   ```
5. Simulando **exatamente** a lógica de `build_previews_and_cache`
   (`save_load_menu_loop.cpp:168-194`: `has_save`→true, `load_game`→ não-Ok → degrada):
   ```
   previews[1].occupied=0  (a UI mostra "Vazio 1" mesmo com arquivo corrompido em disco)
   ```
6. Abrindo a tela em modo Save (`save_load_menu_open`) e clicando no slot 1
   (`save_load_menu_click_slot`, a MESMA função que o mouse real dispara em
   `save_load_menu_loop.cpp:498-501`):
   ```
   slot_selectable(1)=1
   action == SlotChosen (grava DIRETO, SEM pedir confirmação)? 1
   action == None (abriu confirming_overwrite)? 0  confirming_overwrite=0
   ```
7. Confirmado: **nenhum mini-diálogo de sobrescrita abriu**. Executando a 3ª gravação
   (o que `do_save` faria de fato): `save_game` grava a 3ª versão, a rotação empurra o
   PRIMÁRIO CORROMPIDO (dado inútil) para `backup1` — empurrando o `backup1` ANTIGO (a
   1ª gravação, dado **bom e recuperável**) para `backup2`. Ainda dentro da janela
   `kBackupChainDepth=3` nesta única iteração, mas **gravações repetidas "inocentes"**
   nesse slot que o jogador vê como "Vazio" continuam empurrando a cadeia — a 1ª
   gravação boa eventualmente cai fora da janela de 3 e é **perdida de vez**, sem o
   jogador nunca ter visto qualquer aviso de que ali NÃO estava vazio.

## Achados

| ID | Sev | Descrição | Evidência | Remediação proposta (NÃO aplicada) | Estado |
|---|---|---|---|---|---|
| CRIT-1 | 🔴 CRÍTICO | Slot com arquivo primário presente mas corrompido/adulterado (qualquer `LoadResult != Ok`) é exibido como "Vazio" na tela de Salvar e, por isso, **pula a confirmação de sobrescrita** — 1 clique/Enter regrava o slot sem aviso, erodindo silenciosamente a cadeia de backup em direção à perda definitiva de dado recuperável. Repro real executado (acima) com o código de produção real (biblioteca já compilada), sem alterar nada | `save_load_menu_loop.cpp:163-194` (`build_previews_and_cache` degrada não-Ok como `empty_slot_preview`) + `save_load_menu.cpp:200-210` (`confirm_selected_slot` só abre o diálogo se `slot.occupied`); execução real do programa de verificação (saída acima) | Distinguir, no `SaveSlotPreview`, "genuinamente vazio" (`has_save()==false`) de "presente mas ilegível" (arquivo existe, `LoadResult != Ok`) — ex.: campo booleano novo `present_but_unreadable` (ou reaproveitar `occupied=true` com uma exibição dedicada tipo "dado corrompido"); gatear a confirmação de sobrescrita em "existe arquivo primário" (`has_save`), não só no `occupied` de exibição. É um fix cirúrgico — NÃO exige implementar os 2 avisos completos de UI já deferidos pelo líder (aquele gap é sobre o LOAD; este é uma consequência distinta e mais grave no SAVE) | ⚠ aberto |
| CRIT-1-b | (nota, mesmo achado) | O mesmo problema se aplica em QUALQUER modo de falha (`HmacInvalid`/`Corrupt`/`VersionTooNew`/`Invalid`/`WrongSlot`) — `build_previews_and_cache` trata todos igualmente como "não-Ok → vazio"; o repro usou `HmacInvalid` (adulteração) por ser o cenário mais realista dentro do modelo de ameaça já definido em `AUDITORIAS.md`/AUD-SEC ("arquivos que o jogador pode adulterar"), mas a mesma cadeia de causa vale pra corrupção acidental (bit-rot/disco) | leitura de `save_load_menu_loop.cpp:180-191` — o `if` só checa `result == LoadResult::Ok`, sem distinguir qual dos 5 outros valores | (mesma remediação de CRIT-1) | ⚠ aberto |

## Por que isto NÃO é coberto pelo gap já aceito (§7 do brief)

O brief pediu explicitamente para registrar como **gap conhecido, não crítico** a
ausência dos 2 avisos de LOAD (versão-incompatível/corrompido; controles-diferentes).
Este achado é diferente em 3 pontos:

1. É sobre o modo **SALVAR**, não Carregar — o líder nunca foi consultado sobre a
   interação "corrompido-degrada-como-vazio" + "vazio-não-pede-confirmação" no fluxo de
   escrita.
2. Não depende de implementar nenhum aviso novo de UI — o fix é interno (gatear a
   condição de confirmação em `has_save` em vez de `occupied`), ortogonal ao trabalho
   futuro dos avisos.
3. Tem uma consequência concreta e demonstrada de **perda de dado** (não é só "falta um
   texto explicativo pro jogador") — bate na definição textual de 🔴 do próprio
   `AUDITORIAS.md` ("save corrompido, data loss").

## Conclusão

O motor de save (`save_backup`/`save_serializer`, M2/ADR-006) segue correto e já bem
testado — o backup rotaciona certo, o HMAC detecta adulteração certo, `delete_save`
apaga a cadeia inteira certo. O problema é uma interação nova, introduzida pela UI desta
onda, entre duas decisões que isoladamente fazem sentido (degradar corrompido como
"vazio" pra não crashar a tela; não pedir confirmação pra slot genuinamente vazio) mas
que juntas abrem uma janela real de perda de dado silenciosa. Recomenda-se remediar
antes de fechar `✅`/`✓` no `SAVE-LOAD-UI` (ver parecer no `00_indice_mestre.md`).
