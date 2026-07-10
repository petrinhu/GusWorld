# Auditoria: Invariante das 4 camadas (SAVE-LOAD-UI)

- Subsistemas: `gus/domain/save/save_policy.hpp` (novo, política de autosave), `gus/app/
  screens/{save_load_menu,title_menu}.{hpp,cpp}` (POCO de UI), `gus/app/screens/
  {save_load_menu,title_menu}_loop.cpp` (loop GL/I-O), `gus/platform/fs/
  save_file_store.{hpp,cpp}` (fronteira real de disco), `gus/app/maestro_logic.hpp`
  (`should_autosave_after_battle`).
- Critério: AUD-ARCH (`core/`+`domain/` = POCO puro; I/O real só em `platform/`+`app/`
  via ports; SOLID/DRY).

## Contexto e método

O SAVE-LOAD-UI introduz 1 arquivo novo de domínio (`save_policy.hpp`), reusa sem alterar
o domínio de save já existente (`save_data`/`save_slots`/`save_backup`/`save_serializer`,
M2/ADR-006), e adiciona 6 arquivos novos em `app/` (3 pares lógica-pura/render-RML para
as telas de título e save/load, mais os 2 loops GL que orquestram SDL+I/O real). Método:
leitura de código + os mesmos `grep` do GATE (`tools/check.sh`) rodados isoladamente
contra os diretórios tocados por esta feature, mais leitura linha-a-linha dos 4 arquivos
POCO (`save_load_menu.hpp/.cpp`, `title_menu.hpp/.cpp`) confirmando ausência de
`#include` de I/O ou GL.

## Achados

| ID | Sev | Descrição | Evidência | Estado |
|---|---|---|---|---|
| INV-1 | (OK) | `save_policy.hpp` (POCO, política "o autosave pode gravar agora?") é `constexpr`, zero I/O, zero SDL — só 1 enum (`LocationKind`) + 1 função pura | `save_policy.hpp:37-51`; único hit de "SDL"/"I/O" no arquivo é comentário (linha 4), confirmado via `grep -n "SDL\|fstream\|filesystem" save_policy.hpp` | ✓ |
| INV-2 | (OK) | GATE de arquitetura (grep de includes Qt/SDL/RmlUi/glintfx em `core/`+`domain/`) reproduzido isoladamente — vazio | `grep -rnE '#include <(Q[A-Za-z]\|SDL\|RmlUi\|glintfx)' core domain` → sem saída (rc=1, "nada encontrado") | ✓ |
| INV-3 | (OK) | Nenhum uso de namespace `Rml::`/`glintfx::` em `core/`+`domain/` | `grep -rnE '\b(Rml\|glintfx)::' core domain` → vazio | ✓ |
| INV-4 | (OK) | Nenhum I/O direto (`<fstream>`/`<filesystem>`/`ifstream`/`ofstream`) em `core/`+`domain/`, incluindo o `domain/save/` inteiro (política + slots + backup + serializer, este último já auditado em rodadas anteriores) | `grep -rlnE '<fstream>\|<filesystem>\|std::filesystem\|ifstream\|ofstream' core domain` → vazio | ✓ |
| INV-5 | (OK) | `save_load_menu.hpp`/`.cpp` e `title_menu.hpp`/`.cpp` (lógica de estado/navegação das 2 telas novas) são POCO — dependem só de `<SDL3/SDL.h>` pelo TIPO `SDL_Keycode` (parâmetro), nunca chamam `SDL_Init`/`SDL_PollEvent`/nenhuma função de runtime SDL. Mesmo padrão já estabelecido por `system_menu.hpp` (auditado no dossiê M7-COSTURA) | leitura completa dos 4 arquivos; zero chamada de função SDL, zero `glintfx::`, zero `#include <fstream>` | ✓ |
| INV-6 | (OK) | A separação em 3 sub-camadas dentro de `app/` (ESTADO/DECISÃO → RENDER RML → LOOP GL+I/O) documentada no cabeçalho de `save_load_menu.hpp`/`title_menu.hpp` é seguida à risca: `save_load_menu_rml.cpp`/`title_menu_rml.cpp` só LEEM o estado (nunca mutam), `*_loop.cpp` é o único lugar que chama `gus::platform::fs::{has_save,save_game,load_game,delete_save}` | leitura de `save_load_menu_loop.cpp`/`title_menu_loop.cpp` — todas as chamadas de I/O real concentradas nesses 2 arquivos | ✓ |
| INV-7 | (OK) | `FsSaveStore` (única classe que toca `std::filesystem`/`std::ifstream`/`std::ofstream` de verdade para o save) implementa o port `gus::domain::save::SaveStore` — inversão de dependência correta, o domínio não conhece a implementação de arquivo | `save_file_store.hpp:53` (`class FsSaveStore final : public gus::domain::save::SaveStore`); `save_file_store.cpp:1-20` (`#include <filesystem>`/`<fstream>` só aqui, dentro de `platform/`) | ✓ |
| INV-8 | (OK) | `should_autosave_after_battle`/`should_stop_running_after_battle`/`battle_crossfade_target` (lógica de decisão da Maestro) são `constexpr` puros em `maestro_logic.hpp`, testáveis sem `SDL_Init` — a classe `Maestro` (dona da janela/SDL) só CONSOME | `maestro_logic.hpp:72-75`; `maestro_logic_test.cpp:93-105` (testado nos 4 `CombatOutcome` + `static_assert` em tempo de compilação) | ✓ |

## Conclusão

Nenhuma regressão de camada entrou com o SAVE-LOAD-UI. A política de autosave por local
(a única peça nova de `domain/`) nasceu corretamente POCO e `constexpr`; a lógica das 2
telas novas segue rigorosamente o padrão já estabelecido por `system_menu.hpp` (ESTADO
puro / RENDER data-driven / LOOP GL+I-O isolado); a fronteira real de bytes↔disco
permanece 100% dentro de `platform/fs/`. GATE(arch) do `tools/check.sh` confirma isso
também de forma automatizada (`GATE=0` na reprodução do capítulo 2 do índice mestre).
Nenhum achado de severidade neste capítulo.
