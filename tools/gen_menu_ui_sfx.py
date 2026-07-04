#!/usr/bin/env python3
"""Gera os 2 SFX de UI do menu de sistema (hover + clique) por sintese pura.

SEM dependencia externa (so stdlib `wave` + `math` + `struct`) - offline,
determinístico, reproduzivel. Estilo aprovado pelo lider: "blip digital
limpo" (2026-07-04, tarefa MENU-PAUSA-CONFIG-SOM).

FORMATO: alinhado ao SFX de hit ja existente no jogo
(assets/sfx/hit_digital_provisorio.wav) para consistencia de pipeline -
PCM 16-bit, 44100 Hz, estereo (miniaudio decodifica nativo, sem lib extra).
Hit de referencia: peak -5.65 dBFS / RMS -21.57 dBFS (transiente percussivo).
Os blips de UI sao TONAIS (nao percussivos), entao o alvo de comparacao e o
PICO (peak), nao o RMS: hover e click miram picos na mesma faixa de -5 a -9
dBFS do hit, pra soarem "do mesmo kit" ao ouvido, sem estourar.

EASTER EGG (canon do projeto, ver memoria project_fibonacci_easter_egg):
as DURACOES dos 2 SFX sao numeros de Fibonacci consecutivos (55ms / 89ms) -
padrao mecanico pervasivo (~10-15% densidade) ja usado em HP/dano/loot/BPM
em outros lugares do jogo. Ninguem que nao conhece a serie percebe;
quem conhece reconhece a assinatura.

Uso:
    python3 tools/gen_menu_ui_sfx.py
Gera (sobrescreve) assets/sfx/menu_hover_provisorio.wav e
assets/sfx/menu_click_provisorio.wav.
"""

import math
import struct
import wave
import os

SAMPLE_RATE = 44100  # Hz - igual ao hit_digital_provisorio.wav (miniaudio nativo).
CHANNELS = 2          # estereo - igual ao hit (duplicamos o mesmo mono nos 2 canais).
SAMPLE_WIDTH = 2      # bytes = 16-bit PCM - igual ao hit.

OUT_DIR = os.path.join(os.path.dirname(__file__), "..", "assets", "sfx")


def _clamp(x, lo, hi):
    return max(lo, min(hi, x))


def gen_hover(path):
    """Hover: bip curto/discreto, tom agudo, nao cansa em navegacao rapida.

    - Duracao: 55 ms (Fibonacci) - curto o bastante pra nao empilhar quando
      o mouse passa rapido por varias opcoes seguidas.
    - Tom: seno puro em 1318.51 Hz (E6). Seno puro = zero harmonicos extras,
      o mais "limpo"/discreto possivel (evita fadiga auditiva em uso
      repetitivo, que e o risco #1 de som de hover mal desenhado).
    - Envelope: attack linear 3ms (0 -> pico) evita pop de start (comeca
      exatamente em amplitude zero, sem descontinuidade); decay EXPONENCIAL
      no restante (~49ms) simulando release de synth analogico (Moog-like,
      ancorado na bíblia de leitmotivs do projeto: decaimento suave, nao
      corte abrupto); ultimos 3ms recebem um fade-out linear adicional de
      SEGURANCA forcando a amostra final a 0.0 exato - garante zero-crossing
      no fim do buffer (sem click/pop de corte).
    - Pico: 0.32 (~-9.9 dBFS) - deliberadamente mais baixo que o click
      (feedback passivo/secundario, nao deve competir com o click nem com
      o hit de combate).
    """
    duration_s = 0.055  # 55ms - Fibonacci.
    freq = 1318.51  # E6 - tom agudo, discreto.
    peak = 0.32

    attack_s = 0.003
    safety_fade_s = 0.003
    decay_tau = 0.016  # constante de tempo do decaimento exponencial.

    n_samples = int(SAMPLE_RATE * duration_s)
    samples = []
    for i in range(n_samples):
        t = i / SAMPLE_RATE
        # envelope
        if t < attack_s:
            env = t / attack_s
        else:
            env = math.exp(-(t - attack_s) / decay_tau)
        # fade-out de seguranca nos ultimos safety_fade_s segundos.
        t_from_end = duration_s - t
        if t_from_end < safety_fade_s:
            env *= _clamp(t_from_end / safety_fade_s, 0.0, 1.0)

        val = peak * env * math.sin(2 * math.pi * freq * t)
        samples.append(val)

    _write_wav(path, samples)


def gen_click(path):
    """Click: confirmacao mais presente, 2 tons subindo (600Hz -> 1200Hz).

    - Duracao: 89 ms (Fibonacci, proximo de 55 - continuidade do easter egg
      com o hover) - mais longo que o hover, presenca de "comando aceito".
    - Tons: 600 Hz por ~35ms, depois 1200 Hz (EXATAMENTE uma oitava acima -
      intervalo mais consonante possivel, reforca "sistema aceitou", sem
      dissonancia) pelo restante. Transicao com crossfade curto de 4ms pra
      eliminar a descontinuidade de fase que um corte abrupto de frequencia
      causaria (isso SIM soaria como pop/click).
    - Envelope: attack linear 5ms (mais forte que o hover -> mais presenca,
      conforme spec) 0 -> pico; pico sustentado durante o 1o tom; decay
      exponencial comeca na transicao de tom e cobre o 2o tom; ultimos 3ms
      com fade-out linear de seguranca (mesmo raciocinio do hover: garante
      amostra final em 0.0 exato).
    - Pico: 0.55 (~-5.2 dBFS) - na MESMA faixa do pico do hit de combate
      (-5.65 dBFS), perceptualmente equivalente em volume/presenca; mais
      alto que o hover por design (feedback primario de confirmacao).
    """
    duration_s = 0.089  # 89ms - Fibonacci.
    freq_a = 600.0
    freq_b = 1200.0  # oitava acima de freq_a.
    peak = 0.55

    attack_s = 0.005
    switch_t = 0.035          # inicio da transicao de tom.
    crossfade_s = 0.004        # duracao do crossfade entre os 2 tons.
    safety_fade_s = 0.003
    decay_tau = 0.028

    n_samples = int(SAMPLE_RATE * duration_s)
    samples = []
    # fase continua por tom (evita salto de fase abrupto dentro de cada tom).
    for i in range(n_samples):
        t = i / SAMPLE_RATE

        # envelope de amplitude
        if t < attack_s:
            env = t / attack_s
        elif t < switch_t:
            env = 1.0
        else:
            env = math.exp(-(t - switch_t) / decay_tau)
        t_from_end = duration_s - t
        if t_from_end < safety_fade_s:
            env *= _clamp(t_from_end / safety_fade_s, 0.0, 1.0)

        # mistura dos 2 tons: peso do tom A cai / tom B sobe dentro da
        # janela [switch_t, switch_t + crossfade_s].
        if t < switch_t:
            w_b = 0.0
        elif t < switch_t + crossfade_s:
            w_b = (t - switch_t) / crossfade_s
        else:
            w_b = 1.0
        w_a = 1.0 - w_b

        val = peak * env * (
            w_a * math.sin(2 * math.pi * freq_a * t)
            + w_b * math.sin(2 * math.pi * freq_b * t)
        )
        samples.append(val)

    _write_wav(path, samples)


def _write_wav(path, mono_samples_float):
    """Escreve WAV PCM16 estereo (mesmo canal duplicado L/R) a partir de
    amostras float em [-1, 1]."""
    frames = bytearray()
    for s in mono_samples_float:
        s = _clamp(s, -1.0, 1.0)
        i16 = int(round(s * 32767))
        packed = struct.pack("<h", i16)
        frames += packed  # canal L
        frames += packed  # canal R (duplicado - mono "in stereo shell")

    with wave.open(path, "wb") as w:
        w.setnchannels(CHANNELS)
        w.setsampwidth(SAMPLE_WIDTH)
        w.setframerate(SAMPLE_RATE)
        w.writeframes(bytes(frames))


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    hover_path = os.path.join(OUT_DIR, "menu_hover_provisorio.wav")
    click_path = os.path.join(OUT_DIR, "menu_click_provisorio.wav")
    gen_hover(hover_path)
    gen_click(click_path)
    print(f"gerado: {hover_path}")
    print(f"gerado: {click_path}")


if __name__ == "__main__":
    main()
