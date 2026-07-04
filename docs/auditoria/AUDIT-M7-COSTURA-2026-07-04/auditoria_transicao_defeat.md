# Auditoria: Transicao (fade preto + boot pixelizado) + Flavor da derrota

- Subsistemas: `core/anim/fade_transition.{hpp,cpp}`, `app/sdl_window` (`step_with_fade`), `app/screens/battle_scene` (defeat flavor)
- Criterio: AUD-ARCH + AUD-QUALITY (matematica pura da curva; overlay isolado da camada de tempo/desenho; gating correto do flavor)

## Contexto e metodo

Inc 2 (ADR-012 decisao 5) entrega a transicao cidade<->batalha como fade preto curto com crossfade de musica no pico; Inc 2c troca o glitch procedural por uma sequencia de frames pre-renderizada (boot pixelizado), vetado ao vivo o glitch. Inc 3 entrega o flavor da derrota: overlay "reboot de sistema" nao-canonico que segura a tela antes do corte de volta. Metodo: leitura da curva pura + reproducao dos testes de `fade_transition` e de `defeat flavor` (core + app) sob ASan.

## Achados - curva de fade (POCO puro)

| ID | Sev | Descricao | Evidencia | Estado |
|---|---|---|---|---|
| FAD-1 | (OK) | `fade_overlay_alpha` e matematica PURA (zero SDL/GL/IO): converte elapsed->opacidade em [0,1]; quem mede tempo real e desenha o retangulo preto fica em app/ (`sdl_window::step_with_fade`) | `fade_transition.hpp:1-40` (doc da separacao); core suite exit 0 | ✓ |
| FAD-2 | (OK) | kOut (0->1) e kIn (1->0) espelhados; clampa elapsed negativo E alem-da-duracao (sem extrapolar); `duration<=0` devolve o estado FINAL sem dividir por zero | testes #1286-1295 (kOut/kIn extremos + meio=0.5 + clamp negativo/alem + espelhamento soma-1 + duration<=0) | ✓ |
| FAD-3 | (OK) | `direction` tambem escolhe a perna do boot pixelizado (Inc 2c) - o MESMO enum que ja escolhia kOut/kIn pro alpha | `maestro.cpp:265-268` (`step_with_fade(alpha, direction)`) | ✓ |
| FAD-4 | (OK) | `run_city_fade(duration<=0)` e no-op que devolve `true` na hora (simetrico ao core), sem rodar frame extra; propaga quit se a janela fechar durante o fade | `maestro.cpp:254-277` | ✓ |

## Achados - flavor da derrota (Inc 3)

| ID | Sev | Descricao | Evidencia | Estado |
|---|---|---|---|---|
| DEF-1 | (OK) | O overlay so ATIVA no `CombatOutcome::Defeat` (gate por outcome), nunca em Victory | testes #1035, #1037 (`Victory NUNCA ativa`) | ✓ |
| DEF-2 | (OK) | O timer `defeat_flavor_elapsed_` so envelhece com Defeat e TRAVA em `kDefeatFlavorSeconds` (nao ultrapassa); apos esgotar, `active` vira false e NAO religa | `battle_scene.hpp:672-675`; teste #1036 | ✓ |
| DEF-3 | (OK) | Render desenha o reboot (pool literal), a bark do companion VIVO (nome interpolado via `{0}` -> `display_name()`) e a nota-xadrez, SO enquanto `active`; sem companion vivo cai pra variante GENERIC (sem interpolacao) | `battle_scene.cpp:2068-2084`; testes #1038, #1039 | ✓ |
| DEF-4 | (OK) | A `Maestro` so ve o combate encerrado quando `run_battle_preview_embedded` devolve, o que agora ESPERA o overlay - a costura respeita o tempo do flavor (fix BUG-2 documentado) | `maestro.hpp:23-28` (comentario do contrato); fluxo de `to_battle` | ✓ |

## Nota de contraste (o bark do menu vs o bark da derrota)

O flavor da derrota interpola `{0}` corretamente a mao (`battle_scene.cpp:2071`, `bark.replace(pos, 3, speaker->display_name())`). Esse e o padrao de interpolacao do projeto (o `Translator` de app/ nao tem overload de format). O menu de pausa NAO segue esse padrao para `MENU_PAUSE_HINT` (`{0}/{1}` ficam literais) - ver achado ACH-2 em `auditoria_i18n_paridade.md`. Registrado aqui so como o CONTRASTE que evidencia o gap: a mesma equipe fez certo no bark e esqueceu na dica.

## Conclusao

A curva de fade e POCO puro robusto (clamps, sem divisao por zero, espelhamento provado). O flavor da derrota e gateado corretamente por outcome, com timer travado que nao religa, e a costura respeita o tempo do overlay. Nenhum achado de severidade neste capitulo. O unico ponto adjacente (`{0}/{1}` do menu) e do capitulo de i18n.
