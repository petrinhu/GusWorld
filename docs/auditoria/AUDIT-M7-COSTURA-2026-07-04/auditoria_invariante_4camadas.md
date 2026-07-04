# Auditoria: Invariante das 4 camadas (GATE de arquitetura)

- Subsistemas: `GusEngine/core`, `GusEngine/domain` (foco no novo `domain/settings`)
- Criterio: AUD-ARCH (`core/`+`domain/` sao POCO puro: sem Qt/SDL/RmlUi/glintfx, sem I/O direto)

## Contexto e metodo

A regra arquitetural do GusEngine (`engine-design.md`, `CONTRACT.md` §4/§8, `TESTES.md` A2) e que `core/` e `domain/` sao POCO C++ puro headless: nenhuma dependencia de framework de UI (Qt/SDL/RmlUi/glintfx) nem de I/O real (essas vivem em `platform/` e `app/` via ports). O M7-COSTURA introduziu um subsistema NOVO em `domain/settings` (serializacao de `settings.json`) - o ponto de maior risco de vazamento arquitetural desta rodada, porque "settings" cheira a I/O. Verificado que a serializacao opera sobre STRING (POCO) e o I/O real esta isolado em `platform/fs/settings_file_store`. Metodo: os MESMOS greps do GATE do `tools/check.sh` + do `.forgejo/workflows/ci.yml` (sincronia de proposito), reproduzidos a mao.

## Achados

| ID | Sev | Descricao | Evidencia | Estado |
|---|---|---|---|---|
| INV-1 | (OK) | Nenhum include de Qt/SDL/RmlUi/glintfx | `grep -rnE '#[[:space:]]*include[[:space:]]*<(Q[A-Za-z]\|SDL\|RmlUi\|glintfx)' core domain` -> VAZIO | ✓ |
| INV-2 | (OK) | Nenhum uso de namespace `Rml::` nem `glintfx::` | `grep -rnE '\b(Rml\|glintfx)::' core domain` -> VAZIO | ✓ |
| INV-3 | (OK) | Nenhum I/O direto de disco | `grep -rlnE '<fstream>\|<filesystem>\|std::filesystem\|std::ifstream\|std::ofstream' core domain` -> VAZIO | ✓ |
| INV-4 | (OK) | `domain/settings` (novo) e POCO: zero SDL, so `<algorithm>/<string>/<vector>` + o header irmao | `grep -rn SDL domain/include/gus/domain/settings domain/src/settings` -> VAZIO; includes = `<algorithm>` (std::clamp) `<string>` `<vector>` + `system_settings_json.hpp` | ✓ |

## Onde o SDL/I/O legitimamente vive (contraprova da separacao)

- `app/screens/system_menu.hpp` inclui `<SDL3/SDL.h>` **apenas** para `SDL_Keycode` - e camada `app/` (SDL-aware por design, mesmo padrao documentado de `battle_key_down` em `battle_preview.hpp`), FORA do escopo do GATE (que so cobre core/domain). A logica ali e pura no sentido de "nao abre janela, nao chama `SDL_Init`, so compara o valor do enum".
- O I/O real de `settings.json` vive em `platform/fs/settings_file_store.cpp` (`<fstream>`, `<filesystem>`), consumindo `domain::settings::parse/serialize` (POCO). Fronteira respeitada.

## Conclusao

`core/` e `domain/`, incluindo o subsistema novo `domain/settings`, sao POCO puro headless: zero Qt, SDL, RmlUi, glintfx; zero I/O direto; zero fonte de nao-determinismo embarcada. O `SDL_Keycode` da logica do menu esta corretamente na camada `app/`. Invariante das 4 camadas **CUMPRIDA**. Nenhum achado de severidade.
