# Evidência regenerada da F3 (GLINTFX-INTEGRACAO), 2026-08-07

A auditoria de 2026-08-06 (grupo B) marcou a F3 como **NÃO-AUDITÁVEL**: os
diretórios de evidência do spike original (`/var/tmp/spike_evidencia_live/`,
`/var/tmp/gusworld_spike_appmode/<ts>/`) tinham sumido do disco, moravam em
`/var/tmp`, nunca foram versionados. O `RUNBOOK.md` deste diretório já
registrava a causa (linha 3-8): o próprio spike só entrou pro git em
`58d16ff3` (2026-08-06, onda `CI-RODA-OS-GATES`), depois de dias untracked.

Esta pasta é a evidência regenerada da fatia **headless** (build + 60s sob
Xvfb + métricas + snapshot), que é a parte reproduzível sem o líder. O
**roteiro live de F1/F2/F3 de troca de modo de janela** (`RUNBOOK.md`, linhas
52-79) **não foi refeito aqui**: ele exige o "pode ir" explícito do líder no
momento (regra `feedback_combinar_antes_experimento_arriscado`), e essa parte
já tem registro histórico na célula do `TODO.md` (sessão live de 2026-07-23,
10 transições, zero travamento). Reautorizar aquele roteiro é decisão do
líder, não deste agente.

## O que a F3 se propõe a provar (RUNBOOK.md + cabeçalho do probe)

Que a cidade real (`Render2dGl3` + `OverworldSim`, os MESMOS tipos que a
produção usa) desenha DENTRO do `frame_callback` de um `glintfx::App`, com um
`FixedTimestep` próprio alimentado pelo `dt` que o App entrega por frame, sem
o loop espiralar: o suficiente para provar o PADRÃO de integração antes de
decidir a F4 (adoção real do App mode, one-way-door, pós-M8).

## O que esta rodada mede (headless, sem o roteiro live)

- O binário compila e linka contra o glintfx `v0.30.0` real (não mockado),
  compilado FROM-SOURCE com `GLINTFX_BACKEND_GLFW=ON` (exigido por
  `glintfx::App`, `app.hpp` linha 7).
- O `frame_callback` executa 60s sob Xvfb sem crash, sem hang, `exit=0`.
- `Render2dGl3` + `OverworldSim` carregam e desenham a cidade real
  (`distritos_inferiores.gmap`) e o sprite do Gus: ver `snapshot.png`
  (capturado aos 5s via `glintfx::App::snapshot()`, `SNAPSHOT-M5` no log).
- `metrics.json`: 18021 frames em 60.0027s (~300 fps sob Xvfb, não é
  representativo de vsync real, é headless); `ticks_histogram` mostra só
  **1** frame em 18021 com 3+ ticks de `FixedTimestep` (`ticks_ge3_count: 1`,
  0,006%), o loop não espirala, mesma conclusão qualitativa da sessão live
  de 2026-07-23 (lá: 17 em 46330, 0,04%; ambas as rodadas ficam bem abaixo de
  qualquer teto de alarme).
- `dt_callback_s` vs `dt_own_s`: `abs_delta_dt_callback_vs_proprio_s.p95 =
  7,3e-05s`, o `dt` que o glintfx entrega e o relógio monotônico próprio do
  probe concordam de perto.

## Como foi rodado (reprodutível)

Pré-requisito: `build/linux-release/` já configurado e compilado (o probe
reusa `_deps/rmlui-src` e `_deps/glintfx-src` de lá via `SOURCE_DIR`, offline,
e os `.a` de produção via `IMPORTED`).

```bash
cd GusEngine
export TMPDIR=/var/tmp   # build pesado, NUNCA tmpfs (/tmp)
cmake -S app/tools/appmode_spike -B build/appmode_spike_scratch -G Ninja
cmake --build build/appmode_spike_scratch
```

Xvfb isolado (receita canônica de `tools/check.sh`, seção SUITE: display
livre + `XDG_RUNTIME_DIR` próprio `chmod 700`, nunca a sessão viva):

```bash
Xvfb :102 -screen 0 1024x768x24 &
mkdir -p /var/tmp/<algum-dir>/xdg_runtime && chmod 700 /var/tmp/<algum-dir>/xdg_runtime

env -u WAYLAND_DISPLAY DISPLAY=:102 SDL_VIDEODRIVER=x11 \
    XDG_RUNTIME_DIR=/var/tmp/<algum-dir>/xdg_runtime XDG_SESSION_TYPE=x11 \
    DBUS_SESSION_BUS_ADDRESS="unix:path=/var/tmp/<algum-dir>/xdg_runtime/no-dbus" \
    GUSWORLD_SPIKE_SECONDS=60 \
    timeout 90 build/appmode_spike_scratch/appmode_spike_probe
```

`GUSWORLD_SPIKE_WINMODES` **não foi setado** (fica `DESATIVADO` por default,
ver `probe.log` linha 4): o caminho F1/F2/F3 de troca de modo de janela é
exatamente o que o RUNBOOK reserva pro roteiro live, não pra esta rodada.

O probe grava em `/var/tmp/gusworld_spike_appmode/<timestamp>/` (caminho fixo
no código, `appmode_spike_probe.cpp` linha 532), os 3 arquivos desta pasta
(`probe.log` renomeado pra `probe_output.txt`, porque `*.log` está no
`.gitignore` raiz; `metrics.json`; `snapshot.ppm` convertido pra
`snapshot.png`) são a cópia trazida de lá pra dentro do repo, exatamente pra
não repetir o sumiço que motivou a auditoria a marcar F3 como não-auditável.

## Ambiente da rodada

- Commit do `GusEngine` no momento: `96058209e334de829bef9a034b04a9a69abc7a9c`
- Pin do glintfx consumido: **v0.30.0** (`38a301c7`, mesmo `_deps/glintfx-src`
  do preset `linux-release`; ver `GusEngine/CMakeLists.txt:743`)
- `GLINTFX_MODULE_DRAW2D`: `ON` em produção (`GusEngine/CMakeLists.txt:399`),
  mas este probe compila o glintfx com módulos próprios, independentes
  (`GLINTFX_MODULE_TEXT/FX` ON, `AUDIO/GAMEPAD/I18N/DRAW2D` OFF, ver
  `CMakeLists.txt` deste diretório, linhas 102-108); o estado do flag em
  produção não afeta este probe.
- Compilador: GCC 16.1.1, `Release`, Xvfb `1024x768x24`, `SDL_VIDEODRIVER=x11`
