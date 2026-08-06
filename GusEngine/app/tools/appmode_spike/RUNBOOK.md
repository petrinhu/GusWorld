# RUNBOOK - F3-SPIKE-1 (roteiro live de winmodes)

**Scratch/descartavel** (`app/tools/appmode_spike/`) **mas VERSIONADO** desde
2026-08-06 (CI-RODA-OS-GATES): e a evidencia da F3 de GLINTFX-INTEGRACAO, e ela
so vale se existir no git. Cobre SO o
roteiro de troca de modo de janela (F1/F2/F3) ao vivo com o lider. O resto da
validacao do probe (build, 60s headless, metricas, snapshot) e feito pelo
implementador sob Xvfb, SEM este roteiro - ver o relatorio da onda.

---

## A SESSAO LIVE NAO COMECA SEM O AVAL EXPLICITO DO LIDER NO MOMENTO.

**Autorizacao de RISCO nao e autorizacao de TIMING.** Antes de rodar:

1. Avisar o que vai acontecer (troca de modo de janela repetida, F1/F2/F3, ~7
   transicoes).
2. Avisar o risco concreto: **o compositor pode parar de rotear input** (incidente
   real ja documentado pelo glintfx - uma rajada de trocas de modo matou o
   roteamento de teclado/mouse do KDE/Wayland ate reboot; a v0.18.0 tem settle
   mas imunidade NAO esta provada). Se acontecer, sera preciso **VT
   (Ctrl+Alt+F3)** pra recuperar um terminal, ou **reboot**.
3. Dar tempo pro lider **salvar trabalho aberto e fechar apps** (o incidente
   documentado exigiu reboot - qualquer coisa nao salva se perde).
4. Esperar o **"pode ir"** dele, explicito, nesse momento - nao vale autorizacao
   de uma sessao anterior nem "combinamos isso ontem".

Sem isso, PARAR aqui. Nao rodar nada abaixo.

---

## Pre-checklist (antes de comecar)

- [ ] **3 sessoes commitadas e pushadas** (Codeberg + GitHub, dual-remote) - se o
      compositor travar e for preciso reboot, nao pode haver trabalho preso.
- [ ] `monitor_input.sh <dir_de_log>` RODANDO em background (mesmo `<dir_de_log>`
      do probe da rodada - ver o proprio script, `app/tools/appmode_spike/
      monitor_input.sh`) - as 3 capturas (journal kwin, journal kernel,
      /proc/interrupts+devices) precisam estar gravando ANTES da 1a transicao.
- [ ] **Teclado reserva** (USB, se o principal for wireless/bluetooth e o
      travamento afetar o receptor) a mao, plugado, testado.
- [ ] `probe.log` do processo (o mesmo `<dir_de_log>`) sendo acompanhado (`tail
      -f`) por esta sessao, em paralelo ao monitor_input.sh.
- [ ] `GUSWORLD_SPIKE_WINMODES=1` setado no ambiente do probe (sem isso, F1/F2/F3
      so logam `WINMODES_DISABLED` - nada acontece, o roteiro nao tem efeito
      nenhum, seguro por default).
- [ ] `GUSWORLD_SPIKE_SECONDS=300` (ou maior) setado no ambiente do probe - o
      default e 60s e o roteiro abaixo (7 transicoes, dwell >=5s cada = 35s de
      dwell minimo, mais o tempo humano de ler/apertar/narrar entre uma e outra)
      NAO cabe em 60s; sem isso o probe se encerra sozinho no meio da rodada.

## Roteiro (7 transicoes, 2 rodadas)

Dwell (tempo parado em cada modo) **>= 5s** - da tempo do compositor assentar E
da tempo do monitor_input.sh capturar estado normal entre transicoes (o debounce
code-enforced do probe e so 2000ms; 5s de dwell e MARGEM humana, nao o piso
tecnico).

**Touchpad em movimento CONTINUO durante o roteiro inteiro** (nao so nas
transicoes) - e o sinal mais barato de "o compositor ainda esta vivo" (o
monitor_input.sh ve as IRQs do touchpad mudando; se pararem de mudar, e sinal de
alerta mesmo antes de qualquer transicao de janela).

- **Rodada 1** (W = Windowed, FD = FullscreenDesktop, Max = Maximized):
  1. W -> FD (F2), dwell >=5s
  2. FD -> W (F1), dwell >=5s
  3. W -> Max (F3), dwell >=5s
  4. Max -> W (F1), dwell >=5s

- **Rodada 2**:
  5. W -> FD (F2), dwell >=5s
  6. FD -> Max (F3), dwell >=5s (transicao Fullscreen->Maximized - a mais lenta,
     ate ~300ms internos por design, ver app.hpp::set_window_mode)
  7. Max -> W (F1), dwell >=5s

Isso ja bate no **hard cap de 10 transicoes** do probe? Nao (7 < 10) - sobra
margem pra 1 F1/F2/F3 extra se algo sair errado no meio e precisar repetir 1
passo isolado (o cap existe justamente pra impedir uma sequencia MUITO mais
longa que isso).

## Tabela de triagem ("travou")

Apos QUALQUER transicao, checar os 3 sinais (HB do `probe.log`, IN do
`probe.log`, IRQ do `monitor_interrupts_devices.log`) antes de prosseguir pra
proxima:

| HB (probe.log) | IN (probe.log, tecla/mouse novos) | IRQ (monitor_interrupts_devices.log, touchpad mudando) | Diagnostico |
|---|---|---|---|
| vivo (1x/s) | morto (parou de reportar tecla/mouse novo) | vivo (touchpad seguindo mudando) | **compositor parou de rotear input pro app** - o app roda mas nao recebe mais nada (o cenario documentado pelo glintfx) |
| morto (parou de reportar) | (irrelevante) | (irrelevante) | **app travou/crashou** (hang no processo, nao no compositor) |
| vivo | vivo | **morto** (IRQ parou de mudar) | **falha mais funda** que so o compositor - driver/kernel parou de ver o dispositivo |
| (qualquer) | (qualquer) | dispositivo **sumiu** de `/proc/bus/input/devices` (diff no monitor_interrupts_devices.log) | **registrar o timestamp exato** (o diff ja grava isso sozinho) - falha de driver, mais grave que os 2 acima |

## 
Nota (limite headless): sob Xvfb o probe NUNCA emite linha IN (nao ha input
fisico em headless). Por isso a assinatura da linha 1 (HB vivo + IN morto +
IRQ vivo) e indistinguivel de ambiente saudavel no teste headless e NAO deve
ser usada como criterio la; ela so e diagnosticavel na sessao AO VIVO, com
o roteiro desta pagina e o monitor_input.sh rodando.

Gate de abortar (qualquer um destes -> PARAR, nao seguir pro proximo passo)

- Qualquer linha da tabela de triagem acima disparar.
- `elapsed_ms` do `WM-DONE` no `probe.log` **> 1000ms** (10x o teto documentado
  de ~150-300ms - sinal de compositor ja hesitando).
- `live=` do `WM-DONE` **diferente** do modo pedido, **2 vezes seguidas** (o WM
  ja nao esta honrando os pedidos de forma confiavel).
- Crash ou restart do **kwin** aparecendo no `monitor_journal_kwin.log` (ou no
  journal geral).

Se qualquer gate disparar: **NENHUMA transicao a mais.** Apertar Q no probe se
ele ainda responder (aciona o WM-QUIT-RESTORE - 1 ultimo `set_window_mode
(Windowed)` antes de sair, ver `appmode_spike_probe.cpp`). Se o processo nao
responder mais a nada, a evidencia ja esta gravada em disco (`probe.log`,
`metrics.json` parcial nao sera escrito - o processo nao chegou no fim do
`main()` - mas os LOGS ja fluiram por-linha ate o ponto do travamento, isso e o
que importa pra diagnostico).

**Recuperacao:** `Ctrl+Alt+F3` pra um VT (console texto) se a interface grafica
parar de responder por completo; de la, `journalctl -b -1` (apos reboot, se for
preciso) recupera o journal da sessao anterior (a que travou) pra analise
posterior - o log do `monitor_input.sh` (`monitor_journal_kwin.log`/
`monitor_journal_kernel.log`) ja tera capturado a MESMA informacao em tempo
real, mas `journalctl -b -1` e o fallback caso o proprio monitor tambem tenha
sido interrompido (ex.: reboot no meio, antes do Ctrl+C/trap rodar).
