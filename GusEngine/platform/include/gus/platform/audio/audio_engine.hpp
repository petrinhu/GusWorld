// gus/platform/audio/audio_engine.hpp
//
// AudioEngine: alicerce tecnico da camada de audio (M6 F1, ADR-011). Inicializa e
// finaliza o device de audio via glintfx::Audio (GLINTFX-INTEGRACAO F2, 2026-07-22 -
// miniaudio deixou de ser vendorizado direto neste repo; o glintfx compila e expoe o
// device por baixo, ver nota completa no .cpp) com
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
// musica/SFX"): 2 grupos logicos internos (music/sfx) - desde a GLINTFX-INTEGRACAO F2
// emulados por-SoundId sobre glintfx::Audio (nao ha ma_sound_group real, ver
// set_music_volume/set_sfx_volume abaixo e a nota no .cpp), mas o efeito observavel
// (fade de musica separado dos SFX fire-and-forget) e o mesmo. Prepara o terreno
// (aditivo, nao reescrita - ver Reversibilidade do ADR) pro sistema de 5 buses de uma
// onda futura. NAO e esse sistema completo.
//
// NULL-DEVICE (device_active bool no construtor): mesmo padrao do Render2dGl3
// (gl_active). true = tenta abrir o device real do SO (uso normal do jogo). false =
// forca o modo noDevice do glintfx::Audio (miniaudio por baixo, usado em testes/CI/
// smoke SEM hardware de audio garantido) - o engine continua funcional (carrega/
// gerencia sons de verdade), so nao toca hardware. Em AMBOS os casos, se o init falhar
// de verdade (nem o device real nem o modo null sobem), available() fica false e a API
// inteira vira no-op seguro (com log de aviso na construcao).
//
// HEADER LIMPO (sem <glintfx/audio.hpp>): PImpl (mesmo padrao de Render2dGl3::Impl) -
// o .cpp e a UNICA TU desta classe que inclui glintfx/audio.hpp e fala com o tipo
// glintfx::Audio; nenhum tipo ma_* aparece nesta fachada (o glintfx compila o proprio
// src/miniaudio_impl.c dele, confinado dentro da lib - ver nota GLINTFX-INTEGRACAO F2
// no .cpp).
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
    // jogo, app/). false = forca o modo null-device (miniaudio por baixo do
    // glintfx::Audio, sem hardware - testes/CI/smoke sem garantia de placa de som).
    // Falha de init do device REAL
    // (sem soundcard/driver) tambem degrada com seguranca: available()==false, log de
    // aviso, e a API inteira vira no-op - o jogo nunca depende de audio pra rodar.
    explicit AudioEngine(bool device_active) noexcept;
    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    // true = o engine de audio (glintfx::Audio, miniaudio por baixo) subiu (device
    // real OU null-device) e aceita chamadas.
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
    // sem fade - a ANTERIOR nunca recebe fade-out aqui, so a NOVA pode entrar com
    // fade-in). loop=true repete a faixa (ma_sound_set_looping nativo, MA_SOUND_FLAG_
    // STREAM ja garante loop sem gap - o miniaudio reinicia o stream internamente, sem
    // silencio entre o fim e o reinicio). No-op seguro se id invalido/de SFX ou engine
    // indisponivel.
    //
    // fade_in_seconds (M6 F4, ADR-011; extensao ADITIVA prevista no criterio de saida
    // "fade entre telas" - NAO quebra chamadas antigas de play_music(id, loop)): <= 0
    // (default 0.0f) = volume cheio IMEDIATO, byte-identico ao comportamento da F1/F3.
    // > 0 = fade-in nativo do miniaudio (ma_sound_set_fade_in_milliseconds, volume 0 ->
    // 1 ao longo de fade_in_seconds) - mesmo espirito de "usar o fader da lib, nao
    // reinventar" do stop_music.
    void play_music(SoundId id, bool loop, float fade_in_seconds = 0.0f);

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

    // Volume POR GRUPO (MENU-PAUSA-CONFIG-SOM, M7-COSTURA): musica e SFX
    // independentes, em [0,1] (clampado). O glintfx::Audio nao tem ma_sound_group (nem
    // nocao de bus): os 2 grupos (music_group/sfx_group, ADR-011 item 3) sao EMULADOS
    // nesta fachada reaplicando set_volume() por-SoundId em todos os sons daquele tipo
    // (ver nota GLINTFX-INTEGRACAO F2 no .cpp) - nao ha 1 multiplicador intermediario
    // real na cadeia do miniaudio, mas o efeito observavel bate. ADITIVO - NAO substitui
    // set_master_volume (os dois multiplicam o volume efetivo: volume_efetivo = master *
    // grupo). Guardado mesmo se available()==false (o
    // slider de config funciona sem hardware, mesmo padrao de master_volume acima).
    void set_music_volume(float volume);
    [[nodiscard]] float music_volume() const noexcept;
    void set_sfx_volume(float volume);
    [[nodiscard]] float sfx_volume() const noexcept;

    // HOOK DE TESTE (M6 F3, ADR-011): quantos play_sfx EFETIVAMENTE dispararam (id
    // valido + engine available() - nao conta chamadas no-op) desde a construcao. A
    // classe e concreta (sem virtual, PImpl) por design desta onda minima; este contador
    // e o jeito mais barato de provar HEADLESS (null-device, sem hardware) "o play
    // disparou no evento certo" nos consumidores (ex.: BattleScene::play_hit_sfx), sem
    // introduzir uma interface so pra permitir um spy/mock.
    [[nodiscard]] unsigned int sfx_play_count() const noexcept;

    // HOOK DE TESTE (item 1, retoque ao vivo SAVE-LOAD-UI/MODOS-MORTE-FASE0
    // 2026-07-10/11, "SFX bloqueado do card Hardcore"): o SoundId do ULTIMO
    // play_sfx EFETIVAMENTE disparado (mesmo criterio de sfx_play_count() acima -
    // id invalido/engine indisponivel NAO atualiza isto); kInvalidSound se nenhum
    // play efetivo aconteceu ainda. sfx_play_count() sozinho so prova QUANTOS
    // plays aconteceram, nao QUAL SoundId - insuficiente pra provar ROTEAMENTO
    // (ex.: "hover no item bloqueado tocou o SFX bloqueado, NAO o hover normal").
    // Como SoundId e 1-based na ordem de load_sfx (ver o .cpp), um
    // consumidor de teste que sabe a ordem de load_sfx do caminho de producao
    // (ex.: difficulty_menu_loop.cpp: hover, click, blocked) pode comparar
    // last_sfx_id() contra o id esperado apos uma interacao real (SDL_PushEvent).
    [[nodiscard]] SoundId last_sfx_id() const noexcept;

    // HOOK DE TESTE (M6 F4, ADR-011): quantos play_music EFETIVAMENTE tocaram (id
    // valido + engine available() - nao conta chamadas no-op) desde a construcao. Mesmo
    // espirito de sfx_play_count(), agora pro lado de musica - prova HEADLESS que
    // "entrar na batalha tocou musica exatamente 1 vez" nos consumidores.
    [[nodiscard]] unsigned int music_play_count() const noexcept;

    // HOOK DE TESTE (M6 F4, ADR-011): true se ha musica tocando AGORA (ma_sound_is_
    // playing no ma_sound corrente - reflete o node state, independe do device real
    // estar consumindo audio, entao funciona identico em null-device). Usado pra provar
    // headless que o loop nao para sozinho e que SFX/musica coexistem (tocar o hit nao
    // para a musica - grupos music/sfx independentes desde a F1).
    [[nodiscard]] bool music_is_playing() const noexcept;

private:
    void reap_finished_sfx_instances() noexcept;

    struct Impl;                  // glintfx::Audio + vetores de traducao de SoundId
                                   // sfx/music (PImpl - miniaudio fica confinado dentro
                                   // do glintfx, nunca toca esta TU nem o .cpp)
    std::unique_ptr<Impl> impl_;  // sempre nao-nulo (mesmo em modo indisponivel)
    float master_volume_ = 1.0f;
    float music_volume_ = 1.0f;  // MENU-PAUSA-CONFIG-SOM: volume do grupo music_group
    float sfx_volume_ = 1.0f;    // MENU-PAUSA-CONFIG-SOM: volume do grupo sfx_group
    unsigned int sfx_play_count_ = 0;    // hook de teste (M6 F3) - ver sfx_play_count()
    unsigned int music_play_count_ = 0;  // hook de teste (M6 F4) - ver music_play_count()
    SoundId last_sfx_id_ = kInvalidSound;  // hook de teste (item 1, 2026-07-10/11) - ver last_sfx_id()
};

}  // namespace gus::platform::audio

#endif  // GUS_PLATFORM_AUDIO_AUDIO_ENGINE_HPP
