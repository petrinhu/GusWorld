// gus/platform/audio/audio_engine.hpp
//
// AudioEngine: alicerce tecnico da camada de audio (M6 F1, ADR-011). Inicializa e
// finaliza o device de audio via miniaudio (third_party/miniaudio, vendorizado) com
// RAII + degradacao graciosa: falha de init NUNCA crasha o jogo - vira "sem audio" com
// log, e toda a API depois vira no-op (mesmo padrao de Render2dGl3(gl_active=false) e
// Render2dSdl(nullptr) em platform/render2d - o jogo nunca depende de audio pra rodar).
//
// API MINIMA desta onda (ADR-011 secao 4, deliberadamente pequena - os 5 buses
// completos/Master/Music/SFX/UI/Voice + ducking + snapshots ficam pra onda futura):
//   load_sfx(path) / load_music(path)  -> SoundId (handle opaco, mesmo padrao de
//                                          TextureId em platform/render2d/i_renderer.hpp)
//   play_sfx(id)
//   play_music(id, loop)
//   stop_music(fade_seconds)
//   set_master_volume(v)
//
// MIXER MINIMO (ADR-011 item 3, "estrutura minima... volume master + separacao
// musica/SFX"): 2 ma_sound_group internos (music/sfx), o suficiente pra separar o fade
// de musica dos SFX fire-and-forget e preparar o terreno (aditivo, nao reescrita - ver
// Reversibilidade do ADR) pro sistema de 5 buses de uma onda futura. NAO e esse sistema
// completo.
//
// NULL-DEVICE (device_active bool no construtor): mesmo padrao do Render2dGl3
// (gl_active). true = tenta abrir o device real do SO (uso normal do jogo). false =
// forca o modo noDevice do miniaudio (usado em testes/CI/smoke SEM hardware de audio
// garantido) - o ma_engine continua funcional (carrega/gerencia sons de verdade), so
// nao toca hardware. Em AMBOS os casos, se ma_engine_init falhar de verdade (nem o
// device real nem o modo null sobem), available() fica false e a API inteira vira
// no-op seguro (com log de aviso na construcao).
//
// HEADER LIMPO (sem <miniaudio.h>): PImpl (mesmo padrao de Render2dGl3::Impl) - o .cpp
// e a UNICA TU desta classe que inclui miniaudio.h; MINIAUDIO_IMPLEMENTATION e definido
// so ali (third_party/miniaudio/miniaudio.h e single-header).
//
// Cross-ref: docs/tech/adr/ADR-011-m6-audio-onda1-plano.md (spec aprovada desta onda);
//            platform/include/gus/platform/render2d/render2d_gl3.hpp (mesmo padrao de
//            degradacao headless via bool no construtor + PImpl).

#ifndef GUS_PLATFORM_AUDIO_AUDIO_ENGINE_HPP
#define GUS_PLATFORM_AUDIO_AUDIO_ENGINE_HPP

#include <memory>

namespace gus::platform::audio {

// Handle opaco de som pre-carregado (SFX ou musica). 0 = invalido/ausente. Devolvido
// por load_sfx/load_music, consumido por play_sfx/play_music. Mesmo padrao de
// TextureId (gus::platform::render2d::TextureId): o chamador so guarda o numero, o
// backend mapeia pro recurso real. SFX e musica usam ESPACOS DE ID INDEPENDENTES (um
// SoundId de musica nao e valido em play_sfx e vice-versa - a API nao mistura os dois).
using SoundId = unsigned int;
inline constexpr SoundId kInvalidSound = 0;

class AudioEngine {
public:
    // device_active: true = tenta abrir o device de audio real do SO (uso normal do
    // jogo, app/). false = forca o modo null-device do miniaudio (sem hardware -
    // testes/CI/smoke sem garantia de placa de som). Falha de init do device REAL
    // (sem soundcard/driver) tambem degrada com seguranca: available()==false, log de
    // aviso, e a API inteira vira no-op - o jogo nunca depende de audio pra rodar.
    explicit AudioEngine(bool device_active) noexcept;
    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    // true = o engine miniaudio subiu (device real OU null-device) e aceita chamadas.
    // false = falha de init total - toda a API abaixo vira no-op seguro.
    [[nodiscard]] bool available() const noexcept;

    // Carrega um efeito sonoro CURTO (decodificado por completo no load - MA_SOUND_FLAG_
    // DECODE - pra play_sfx ser barato/repetivel). Devolve um SoundId valido, ou
    // kInvalidSound se falhar (arquivo ausente, engine indisponivel). O chamador DEVE
    // tolerar kInvalidSound. Idempotente NAO garantido (cada chamada cria um novo
    // handle, mesmo com o mesmo path - ao contrario de TextureId; sem cache de path
    // nesta onda, anti over-engineering ADR-011).
    [[nodiscard]] SoundId load_sfx(const char* path);

    // Carrega uma faixa de MUSICA (streaming - MA_SOUND_FLAG_STREAM - musica pode ser
    // longa; o load so VALIDA que o arquivo abre/decodifica, o stream real comeca em
    // play_music). Devolve kInvalidSound se falhar.
    [[nodiscard]] SoundId load_music(const char* path);

    // Dispara um SFX fire-and-forget (instancia efemera; a vida e gerenciada
    // internamente - nao ha handle de instancia, nem stop individual nesta onda). No-op
    // seguro se id invalido/de musica ou engine indisponivel.
    void play_sfx(SoundId id);

    // Toca uma musica, SUBSTITUINDO a que estiver tocando (para a anterior imediata,
    // sem fade - o fade e so em stop_music, por decisao do ADR-011). loop=true repete a
    // faixa (ma_sound_set_looping nativo). No-op seguro se id invalido/de SFX ou engine
    // indisponivel.
    void play_music(SoundId id, bool loop);

    // Para a musica corrente com FADE-OUT nativo do miniaudio (ma_sound_stop_with_fade_
    // in_milliseconds - fader da propria lib, nao reinventado). fade_seconds <= 0 =
    // parada imediata. No-op seguro se nao ha musica tocando/engine indisponivel.
    void stop_music(float fade_seconds);

    // Volume MASTER em [0,1] (clampado). Afeta musica + SFX igualmente (ma_engine_
    // set_volume - um unico controle nesta onda; volume por bus fica pra onda futura
    // dos 5 buses). Guardado mesmo se available()==false (ex.: slider de opcoes
    // funciona sem hardware - so nao produz som).
    void set_master_volume(float volume);
    [[nodiscard]] float master_volume() const noexcept;

private:
    void reap_finished_sfx_instances() noexcept;

    struct Impl;                  // ma_engine + ma_sound_group + sons (PImpl - miniaudio
                                   // fica confinado ao .cpp)
    std::unique_ptr<Impl> impl_;  // sempre nao-nulo (mesmo em modo indisponivel)
    float master_volume_ = 1.0f;
};

}  // namespace gus::platform::audio

#endif  // GUS_PLATFORM_AUDIO_AUDIO_ENGINE_HPP
