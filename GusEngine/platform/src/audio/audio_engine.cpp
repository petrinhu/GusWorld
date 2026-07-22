// gus/platform/src/audio/audio_engine.cpp
//
// Implementacao do AudioEngine (M6 F1, ADR-011). Ver header. Travado por
// platform/tests/audio_engine_test.cpp (TEST-FIRST, caminho null-device + WAV
// sintetico) + smoke opcional de device real (GUSWORLD_AUDIO_SMOKE=1, app/).
//
// GLINTFX-INTEGRACAO F2 (2026-07-22): a implementacao trocou de miniaudio DIRETO
// (third_party/miniaudio, vendorizado) para o modulo glintfx::Audio (glintfx
// v0.18.0, GLINTFX_MODULE_AUDIO=ON). A fachada gus::platform::audio::AudioEngine
// (header) NAO mudou - nenhuma assinatura, nenhum consumidor de app/ tocado. Este
// .cpp NAO inclui mais miniaudio.h nem define MINIAUDIO_IMPLEMENTATION: o
// glintfx compila o proprio src/miniaudio_impl.c dele (glintfx_audio, OBJECT
// library dobrada no alvo glintfx STATIC) - dois sets fortes de simbolos ma_*
// no mesmo binario dariam duplicate symbol no link, entao esta TU so pode falar
// com a API pimpl do glintfx::Audio (glintfx/audio.hpp), nunca com tipo ma_*.
//
// ESPACOS DE ID: gus::platform::audio::SoundId continua 1-based e com DOIS
// espacos INDEPENDENTES (sfx x music, ver header) - exatamente como na F1. O
// glintfx::Audio::SoundId por baixo e um UNICO espaco global (load_sound
// devolve o proximo id monotonico, sem separar sfx/musica). Por isso o Impl
// guarda DOIS vetores de traducao (sfx_ids/music_ids: indice = SoundId-1 da
// fachada, valor = glintfx::Audio::SoundId real) - a independencia dos dois
// espacos e uma invariante DESTA fachada, nao do glintfx.
//
// MIXER MINIMO (ADR-011 item 3, "musica/sfx separados"): o glintfx::Audio nao
// tem ma_sound_group (nem noção de bus) - so volume por-SoundId e volume
// master. Emulamos os 2 grupos (music_group/sfx_group da F1) aplicando
// set_volume() em TODO SoundId carregado daquele tipo sempre que (a) um som
// novo e carregado (herda o volume do grupo NA HORA do load) ou (b)
// set_music_volume/set_sfx_volume muda o volume do grupo (reaplica em todos os
// ja carregados). Nao e um group node de verdade (nao ha 1 multiplicador
// intermediario no grafo miniaudio) mas o efeito observavel bate: todo som do
// mesmo "grupo" acaba no mesmo volume.

#include "gus/platform/audio/audio_engine.hpp"

#include <algorithm>  // std::clamp
#include <cmath>      // std::isfinite
#include <cstdint>    // std::uint32_t (glintfx::Audio::SoundId)
#include <iostream>
#include <vector>

#include <glintfx/audio.hpp>

namespace gus::platform::audio {

struct AudioEngine::Impl {
    glintfx::Audio audio;

    // Traducao SoundId (fachada, 1-based) -> glintfx::Audio::SoundId (espaco
    // global do glintfx). indice [id-1]. Enderecos NAO precisam ser estaveis
    // aqui (sao so numeros, nao nos de grafo intrusivo como o ma_sound da F1).
    std::vector<glintfx::Audio::SoundId> sfx_ids;
    std::vector<glintfx::Audio::SoundId> music_ids;

    // Musica corrente (SoundId da FACHADA, 1-based; kInvalidSound = nenhuma
    // musica tocou ainda). Guardado (nao resetado por stop_music) pelo mesmo
    // motivo da F1: music_is_playing() precisa continuar refletindo o estado
    // do node (inclusive "ainda tocando" durante um fade-out agendado).
    SoundId current_music_id = kInvalidSound;
};

AudioEngine::AudioEngine(bool device_active) noexcept : impl_(std::make_unique<Impl>()) {
    glintfx::AudioConfig cfg;
    // Null-device (ADR-011 item 4, contrato preservado): false = forca
    // AudioConfig::null_backend=true (o glintfx::Audio continua funcional -
    // decodifica e gerencia sons de verdade - so nao abre device de hardware),
    // usado em testes/CI/smoke sem garantia de placa de som.
    cfg.null_backend = !device_active;

    if (!impl_->audio.init(cfg)) {
        std::cerr << "AudioEngine: falha ao inicializar (glintfx::Audio::init) - "
                     "jogo roda sem audio (degradacao graciosa)\n";
        // impl_->audio.is_initialized() fica false; API inteira vira no-op
        // (available() abaixo checa isso).
    }
}

// Deterministico: ~Audio() do glintfx (membro de Impl) chama shutdown() sozinho
// (idempotente, ordem son->engine garantida pelo proprio modulo) - nao ha nada
// a fazer aqui alem de destruir o Impl. Definido no .cpp (nao "= default" no
// header) porque Impl e tipo incompleto la (PImpl).
AudioEngine::~AudioEngine() = default;

bool AudioEngine::available() const noexcept { return impl_->audio.is_initialized(); }

// NO-OP pos GLINTFX-INTEGRACAO F2: glintfx::Audio::play_oneshot() faz o reap
// preguicoso das copias terminadas INTERNAMENTE (Impl::voices dele, ver
// glintfx/audio.hpp - "no timer, no background thread... first from inside
// play_oneshot() itself"). Nao ha mais pool de instancias proprio aqui pra
// podar. Metodo mantido (o header NAO muda por contrato desta onda) mas nunca
// mais chamado por play_sfx.
void AudioEngine::reap_finished_sfx_instances() noexcept {}

SoundId AudioEngine::load_sfx(const char* path) {
    if (!available() || path == nullptr) {
        return kInvalidSound;
    }

    const glintfx::Audio::SoundId gid = impl_->audio.load_sound(path);
    if (gid == 0) {
        std::cerr << "AudioEngine::load_sfx: falha ao carregar '" << path << "'\n";
        return kInvalidSound;
    }

    // Herda o volume do "grupo" sfx NA HORA do load (emulacao do
    // ma_sound_group - ver comentario de topo de arquivo).
    impl_->audio.set_volume(gid, sfx_volume_);

    impl_->sfx_ids.push_back(gid);
    return static_cast<SoundId>(impl_->sfx_ids.size());  // 1-based
}

SoundId AudioEngine::load_music(const char* path) {
    if (!available() || path == nullptr) {
        return kInvalidSound;
    }

    // NOTA (surpresa desta onda, ver retorno): glintfx::Audio::load_sound() faz
    // SEMPRE um decode SINCRONO COMPLETO (mesmo contrato pra sfx e musica - nao
    // ha flag de streaming exposta). A F1 usava MA_SOUND_FLAG_STREAM pra
    // musica (so validava no load, decode real comecava em play_music) - essa
    // lazyness se perde aqui. Sem regressao observavel nos 21 testes (WAV
    // sinteticos curtos), mas musica longa/grande vira decode antecipado no
    // load em vez de streaming.
    const glintfx::Audio::SoundId gid = impl_->audio.load_sound(path);
    if (gid == 0) {
        std::cerr << "AudioEngine::load_music: falha ao carregar '" << path << "'\n";
        return kInvalidSound;
    }

    // Herda o volume do "grupo" music NA HORA do load (mesma emulacao do sfx).
    impl_->audio.set_volume(gid, music_volume_);

    impl_->music_ids.push_back(gid);
    return static_cast<SoundId>(impl_->music_ids.size());  // 1-based
}

void AudioEngine::play_sfx(SoundId id) {
    if (!available() || id == kInvalidSound || id > impl_->sfx_ids.size()) {
        return;
    }

    const glintfx::Audio::SoundId gid = impl_->sfx_ids[id - 1];
    // play_oneshot() e a polifonia fire-and-forget (ma_sound_init_copy do
    // lado deles, cap de 32 vozes por id com voice-stealing da mais antiga) -
    // equivalente direto do ma_sound_init_copy manual que a F1 fazia aqui.
    if (!impl_->audio.play_oneshot(gid)) {
        std::cerr << "AudioEngine::play_sfx: falha ao instanciar (id=" << id << ")\n";
        return;
    }

    ++sfx_play_count_;  // hook de teste (M6 F3) - so conta o caminho que TOCOU de fato
    last_sfx_id_ = id;  // hook de teste (item 1, 2026-07-10/11) - ver last_sfx_id()
}

void AudioEngine::play_music(SoundId id, bool loop, float fade_in_seconds) {
    if (!available() || id == kInvalidSound || id > impl_->music_ids.size()) {
        return;
    }

    // Troca de faixa: se uma musica DIFERENTE da pedida estiver marcada como
    // corrente, para ela IMEDIATAMENTE (fade_out=0) antes de iniciar a nova -
    // a ANTERIOR nunca recebe fade-out aqui (so a NOVA pode entrar com
    // fade-in), mesmo contrato da F1/F3. Chamar play_music de novo com o
    // MESMO id (troca de loop, por exemplo) nao entra aqui - play() abaixo ja
    // reinicia do frame 0 sozinho (contrato do glintfx::Audio::play()).
    if (impl_->current_music_id != kInvalidSound && impl_->current_music_id != id) {
        const glintfx::Audio::SoundId prev_gid = impl_->music_ids[impl_->current_music_id - 1];
        impl_->audio.stop(prev_gid, 0.0f);
    }

    // FADE-IN (M6 F4, ADR-011): clampa localmente pra preservar o contrato da
    // F1 - fade_in_seconds nao-finito/negativo vira "sem fade" (volume cheio
    // imediato) em vez de REJEITAR a chamada inteira (o proprio play() do
    // glintfx rejeitaria um fade invalido devolvendo false sem tocar nada, o
    // que quebraria o "toca de qualquer jeito" da F1 para esse caso hostil).
    const float fade_in_s =
        (std::isfinite(fade_in_seconds) && fade_in_seconds > 0.0f) ? fade_in_seconds : 0.0f;

    const glintfx::Audio::SoundId gid = impl_->music_ids[id - 1];
    if (!impl_->audio.play(gid, loop, fade_in_s)) {
        std::cerr << "AudioEngine::play_music: falha ao (re)iniciar (id=" << id << ")\n";
        return;
    }

    impl_->current_music_id = id;
    ++music_play_count_;  // hook de teste (M6 F4) - so conta o caminho que TOCOU de fato
}

void AudioEngine::stop_music(float fade_seconds) {
    if (!available() || impl_->current_music_id == kInvalidSound) {
        return;
    }

    // Mesma clampagem local do play_music acima (preserva o contrato da F1:
    // fade_seconds nao-finito/negativo = parada imediata, nunca uma chamada
    // rejeitada por fade invalido).
    const float fade_out_s =
        (std::isfinite(fade_seconds) && fade_seconds > 0.0f) ? fade_seconds : 0.0f;

    const glintfx::Audio::SoundId gid = impl_->music_ids[impl_->current_music_id - 1];
    // Fader NATIVO do glintfx/miniaudio (ADR-011 item 3: "use o que a lib da
    // em vez de reinventar"). O node continua existindo ate o proximo
    // play_music trocar ou o destrutor limpar - nao ha polling nesta API
    // minima pra saber quando o fade termina (music_is_playing() reflete o
    // estado real durante o fade, ver comentario do glintfx::Audio::stop()).
    impl_->audio.stop(gid, fade_out_s);
}

void AudioEngine::set_master_volume(float volume) {
    master_volume_ = std::clamp(volume, 0.0f, 1.0f);
    if (available()) {
        impl_->audio.set_master_volume(master_volume_);
    }
}

float AudioEngine::master_volume() const noexcept { return master_volume_; }

void AudioEngine::set_music_volume(float volume) {
    music_volume_ = std::clamp(volume, 0.0f, 1.0f);
    if (!available()) {
        return;
    }
    // Reaplica em TODO SoundId de musica ja carregado (emulacao do
    // ma_sound_group_set_volume - ver comentario de topo de arquivo).
    for (const glintfx::Audio::SoundId gid : impl_->music_ids) {
        impl_->audio.set_volume(gid, music_volume_);
    }
}

float AudioEngine::music_volume() const noexcept { return music_volume_; }

void AudioEngine::set_sfx_volume(float volume) {
    sfx_volume_ = std::clamp(volume, 0.0f, 1.0f);
    if (!available()) {
        return;
    }
    // Reaplica em TODO SoundId de sfx ja carregado (mesma emulacao acima).
    for (const glintfx::Audio::SoundId gid : impl_->sfx_ids) {
        impl_->audio.set_volume(gid, sfx_volume_);
    }
}

float AudioEngine::sfx_volume() const noexcept { return sfx_volume_; }

unsigned int AudioEngine::sfx_play_count() const noexcept { return sfx_play_count_; }

SoundId AudioEngine::last_sfx_id() const noexcept { return last_sfx_id_; }

unsigned int AudioEngine::music_play_count() const noexcept { return music_play_count_; }

bool AudioEngine::music_is_playing() const noexcept {
    if (!available() || impl_->current_music_id == kInvalidSound) {
        return false;
    }
    const glintfx::Audio::SoundId gid = impl_->music_ids[impl_->current_music_id - 1];
    return impl_->audio.is_playing(gid);
}

}  // namespace gus::platform::audio
