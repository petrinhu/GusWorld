# Kit de áudio provisório — M6 F2 (ADR-011)

**Este é um kit provisório da 1ª onda do M6 (Áudio), curado conforme `docs/tech/adr/ADR-011-m6-audio-onda1-plano.md` decisão 3.** Não é produção/composição nova — é curadoria de sons prontos CC0, filtrada pelo canon (Pillar "magia = software" para o hit; bíblia de leitmotivs `docs/narrative/deep/ontologia/leitmotivs-musicais-detalhados.md` R7 §"01. GusWorld City cyber-gótica" para a música). Serve para provar o cano técnico (`GusEngine/platform/audio/`) e dar sinal honesto de feel em playtest. **Será substituído/expandido pelo `audio-designer-composer` numa onda futura dedicada** (assinatura sonora do combate, leitmotiv de Gus, canal-4 Patch-Zero — zero disso está aqui).

Todas as fontes abaixo são **estritamente CC0 / domínio público** (nenhuma CC-BY foi usada, para evitar gestão de atribuição num kit descartável em repo público).

---

## SFX

### `assets/sfx/hit_digital_provisorio.wav` — SFX principal de hit de combate

- **Fonte:** Kenney "Sci-Fi Sounds" pack — https://kenney.nl/assets/sci-fi-sounds (arquivo original `Audio/laserSmall_000.ogg`, convertido para WAV PCM 16-bit sem alteração de conteúdo)
- **Autor:** Kenney Vleugels (kenney.nl)
- **Licença:** CC0 (Creative Commons Zero) — declarada em `License.txt` dentro do pack: *"License: (Creative Commons Zero, CC0) http://creativecommons.org/publicdomain/zero/1.0/ — This content is free to use in personal, educational and commercial projects."*
- **Duração:** 0.239s (dentro da faixa alvo 100-400ms, punch imediato sem intro longa)
- **Formato:** WAV PCM 16-bit, 44.1kHz, estéreo
- **Por que passou no filtro canônico:** zap sintético curto e seco — soa como pulso digital/processado, não como thwack orgânico de espada/soco. Coerente com Gus executando um "cartão de software" (Pillar "magia = software"), não um golpe de guerreiro de fantasia genérica.

### `assets/sfx/hit_digital_alt_provisorio.wav` — SFX de hit alternativo (opção A/B)

- **Fonte:** Kenney "Digital Audio" pack — https://kenney.nl/assets/digital-audio (arquivo original `Audio/phaserDown2.ogg`, convertido para WAV PCM 16-bit)
- **Autor:** Kenney Vleugels (kenney.nl)
- **Licença:** CC0 — mesma declaração do `License.txt` do pack (ver acima, texto idêntico).
- **Duração:** 0.313s (dentro da faixa alvo 100-400ms)
- **Formato:** WAV PCM 16-bit, 44.1kHz, estéreo
- **Por que passou no filtro canônico:** phaser descendente sintético, timbre distinto do hit principal (dá opção de A/B pro criador escolher qual "sente" melhor no playtest) mas mantém a mesma linguagem digital/processada — nenhum som orgânico/percussivo real.

### `assets/sfx/ui_confirm_provisorio.wav` — SFX de confirmação de UI

- **Fonte:** Kenney "Interface Sounds" pack — https://kenney.nl/assets/interface-sounds (arquivo original `Audio/confirmation_001.ogg`, convertido para WAV PCM 16-bit)
- **Autor:** Kenney Vleugels (kenney.nl)
- **Licença:** CC0 — mesma declaração do `License.txt` do pack (ver acima).
- **Duração:** 0.290s (curto, não fadigante em uso repetido)
- **Formato:** WAV PCM 16-bit, 44.1kHz, mono
- **Por que passou no filtro canônico:** chime digital limpo e curto, mesma linguagem sintética/processada do resto do kit; serve de UI 2D (nunca 3D-spatial, por princípio de mix).

---

## Música

### `assets/music/cidade_tema_provisorio.mp3` — tema provisório de GusWorld City (loop)

- **Fonte:** OpenGameArt.org — "Pondering the Cosmos" — https://opengameart.org/content/pondering-the-cosmos
- **Autor:** Ruskerdax
- **Licença:** CC0 — declarada na página do item: *"This music is released to the public domain, which means you are free to use, modify, and distribute it in any form."*
- **Duração:** 5:07 (307.7s)
- **Formato:** MP3, 44.1kHz, estéreo, **re-encodado de 320kbps → 128kbps** (arquivo original CC0 tinha 12.3MB, acima do orçamento de repo; a transcodificação de bitrate NÃO altera o conteúdo musical, é só empacotamento — ffmpeg `-b:a 128k`, sem produção criativa nova). Arquivo final: 4.9MB.
- **Por que passou no filtro canônico:** descrição da própria fonte já usa quase literalmente o vocabulário da bíblia de leitmotivs (`docs/narrative/deep/ontologia/leitmotivs-musicais-detalhados.md`, linha 118, tema "01. GusWorld City cyber-gótica" — Lá menor harmônico, 4/4, 55 BPM, sintetizador analógico, atmosfera ambient open-ended): "dark sci-fi composition employing **vintage synthesizer sounds**... contemplative, cinematic atmosphere... **slow, ambient quality designed for looping playback**" (refs citadas pelo autor: Mass Effect, Deus Ex, FTL — todos trilhas synth analógico atmosféricas). Metrópole noturna respirando devagar, melancólico-contemplativo, NÃO música de ação — bate com o pedido.
- **Checagem técnica de loop (ffmpeg `volumedetect`, sem ouvido humano):** volume médio dos primeiros 3s = **-22.7 dB**, dos últimos 3s = **-23.1 dB** (praticamente idêntico) — sinal de que início e fim têm energia sonora parecida, favorável a um loop sem salto abrupto perceptível de volume. **Não há fade-out no final** (ao contrário do candidato alternativo abaixo) — a faixa mantém presença sonora constante até o corte, então não sofre do problema "silêncio morto antes do loop reiniciar".
- **Limitação honesta:** BPM exato (~55) e a tonalidade (Lá menor harmônico) **não foram verificados por análise de áudio nem por ouvido humano** — a curadoria foi feita pela descrição textual da fonte + inspeção técnica de volume (ffmpeg), não por audição. O criador precisa validar o "feel" real no playtest; se o BPM/tom não bater bem o suficiente, é candidato a troca já mapeada abaixo.

---

## Candidato alternativo de música considerado e descartado (documentado para referência futura)

- **"Corrupt Data Stream"** — OpenGameArt.org, https://opengameart.org/content/corrupt-data-stream — autor Tsorthan Grove — **também CC0** ("CC0 / Public Domain. No Rights Reserved."). MP3, 192kbps, 3:10, 4.5MB (já dentro do orçamento sem precisar re-encode). Descrição: "dark ambient/cyberpunk composition... moody, atmospheric soundscape". Checagem de volume: primeiros 3s = -21.5dB, **últimos 3s = -91.0dB (fade-out gradual até quase silêncio total)** — não é um fade abrupto/cortado, mas gera alguns segundos de silêncio antes do loop reiniciar, o que é menos ideal para loop contínuo do que "Pondering the Cosmos". Mantido aqui como plano B caso o criador prefira o timbre dele no playtest (é menor em tamanho e mais "cyberpunk" explícito no texto da fonte, menos "vintage synth" explícito).
- Descartados por licença (CC-BY / CC-BY-SA, fora do critério estrito CC0 desta onda): "digital evening" (bluszcz, CC-BY 3.0), "Endgame: Singularity" (Maxstack, CC-BY-SA 3.0), "Haze" (symphony, CC BY 4.0), "Continuum" (Grossman/Tsorthan Grove/migfus20, CC-BY 4.0).
- **freepd.com** (fonte sugerida no plano original) está **offline permanentemente** ("hosting and maintenance ceased") — não foi possível usar; substituído por OpenGameArt.org CC0 conforme a lista de fontes recomendadas do plano.

---

## Notas técnicas gerais

- SFX candidatos vieram em Ogg Vorbis (formato original Kenney) e foram convertidos para WAV PCM 16-bit via `ffmpeg -c:a pcm_s16le` (conversão de container/codec, sem alteração de conteúdo) — atende à recomendação de evitar Ogg/Vorbis no miniaudio sem decoder extra.
- Música já veio em MP3 (decodificação nativa do miniaudio), só precisou de redução de bitrate por tamanho de repo.
- Nenhum arquivo foi ouvido por humano ou por este agente (sem capacidade de audição) — toda curadoria foi por **descrição textual da fonte + metadata técnica** (duração via `ffprobe`, perfil de volume via `ffmpeg volumedetect`/`silencedetect`). **O criador (petrus) precisa validar o feel real ouvindo no jogo** antes de considerar este kit definitivo — é exatamente o propósito desta onda (dar sinal honesto pro playtest, não composição final).
- Packs Kenney completos ficaram baixados em scratchpad de sessão (não commitados) — só os 3 arquivos WAV selecionados e convertidos foram copiados para `assets/sfx/`.
