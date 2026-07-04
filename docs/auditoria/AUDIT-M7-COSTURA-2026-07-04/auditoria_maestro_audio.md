# Auditoria: Maestro + ownership do AudioEngine

- Subsistema: `app/maestro.{hpp,cpp}`, `app/maestro_logic.{hpp,cpp}`
- Criterio: AUD-ARCH + AUD-QUALITY (orquestracao leve, ownership claro, logica pura testavel, degradacao segura)

## Contexto e metodo

O M7-COSTURA (ADR-012 decisao 4/5) introduz um **maestro leve** acima das 2 telas (cidade/batalha) - explicitamente NAO um gerenciador de cenas generico (anti over-engineering). O Inc 2 sobe a posse do `AudioEngine` da `battle_preview` para o `Maestro` (paga a divida consciente do ADR-011): 1 instancia viva pro loop inteiro, device nao reaberto a cada entrada de batalha. Metodo: leitura do header/impl + do POCO `maestro_logic` + reproducao dos testes `app/tests/maestro_logic_test.cpp` sob ASan.

## Achados

| ID | Sev | Descricao | Evidencia | Estado |
|---|---|---|---|---|
| MAE-1 | (OK) | O `Maestro` e DONO de 1 unica instancia de `AudioEngine` (`audio_` membro, `device_active=true`), viva pro loop inteiro; a `battle_preview` recebe PONTEIRO NAO-DONO (`&audio_`) e so o usa pro SFX do hit + fade visual proprio, nunca toca musica | `maestro.hpp:137` (`AudioEngine audio_{true}`); `maestro.cpp:313-315` (passa `&audio_` a `run_battle_preview_embedded`) | ✓ |
| MAE-2 | (OK) | Crossfade cidade<->arena cronometrado no PICO da opacidade (tela 100% preta), via POCO testavel `crossfade_music`/`battle_crossfade_target`; degrada seguro (`kInvalidSound` -> no-op, cai pro tema da cidade) se a faixa da arena falhar ao carregar | `maestro.cpp:299-300, 348`; testes #1093-1098 (`crossfade_music` nullptr/kInvalidSound no-op; `battle_crossfade_target` prefere batalha, cai pra cidade) | ✓ |
| MAE-3 | (OK) | Volume carregado do `settings.json` e aplicado no AudioEngine ANTES de tocar qualquer musica no boot (evita 1 frame em volume cheio) | `maestro.cpp:103-107` (`load_system_settings` -> `set_music_volume`/`set_sfx_volume` antes do `play_music`) | ✓ |
| MAE-4 | (OK) | Edge-trigger da batalha (BUG-6): a batalha so dispara na TRANSICAO nao-overlap->overlap (rising edge), impedindo re-disparo em loop apos fuga/derrota (o jogador volta AINDA sobre o inimigo) | `maestro.cpp:228-250` (`should_trigger_battle_on_edge`); testes de `maestro_logic` (#1076 `outcome_marks_enemy_defeated` so Victory; #1090 `should_stop_running_after_battle`) | ✓ |
| MAE-5 | (OK) | Contrato de quit distinto de outcome (BUG-3): fechar a janela DURANTE a batalha propaga como quit (retorno `true` distinto de qualquer `CombatOutcome`), pulando `reacquire_renderer` de proposito; nao reabre a cidade em loop | `maestro.cpp:101, 322-332`; `run()` encerra o `while` (`maestro.cpp:238-240`) | ✓ |
| MAE-6 | (OK) | Reconstrucao INCONDICIONAL do renderer da cidade apos batalha/menu (mesmo se o passo falhou), com degradacao logada (segue rodando sem desenhar, sem crash) - evita cidade "morta" pro resto da sessao | `maestro.cpp:192-196` (menu), `336-340` (batalha) | ✓ |
| MAE-7 | (OK) | Logica de decisao e POCO puro testavel headless (`maestro_logic`): posicao do inimigo, footprint AABB, trigger, roteamento de outcome, alvo do crossfade - o orquestrador `Maestro` so consome | `maestro_logic.hpp/.cpp`; `maestro_logic_test.cpp` (reproducao fiel do mapa real); ASan app suite 387 cases exit 0 | ✓ |
| MAE-8 | (nota) | O `Maestro` "save" desta onda e so instancia em MEMORIA (`save_.flags`), sem I/O em disco - coerente com ADR-012 (persistencia real = M2-SAVE-IO, onda seguinte). Nao e achado; e escopo declarado | `maestro.hpp:169`, comentario `on_battle_result` | ✓ |

## Observacao de ownership (memory-safety da costura)

O `Maestro` usa `std::unique_ptr<SdlWindow> city_` (RAII) e um `AudioEngine audio_` por VALOR (membro, destruido no dtor). A `SDL_Window* window_` e o unico recurso cru, destruido explicitamente no dtor (`SDL_DestroyWindow`) na ordem correta (`city_.reset()` ANTES, pra o renderer soltar a janela). Regra dos 5 respeitada: copia/atribuicao `= delete` (`maestro.hpp:76-77`). A suite de app roda LIMPA sob ASan+UBSan (387 test cases, 3172 assertions, exit 0), sem use-after-free na troca de renderer nem no ciclo de vida da janela.

## Conclusao

O Maestro cumpre o contrato do ADR-012: orquestracao leve, ownership do AudioEngine subido corretamente (divida do ADR-011 paga), logica de decisao isolada em POCO testavel, degradacao segura em toda falha (audio ausente, renderer, janela fechada no meio). Memory-safety confirmada sob ASan no codigo proprio. Nenhum achado de severidade neste capitulo (o UAF de audio e da lib vendorizada miniaudio, capitulo `auditoria_qualidade_cpp_asan.md`).
