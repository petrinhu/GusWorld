// gus/platform/src/audio/audio_engine.cpp
//
// Implementacao do AudioEngine (M6 F1, ADR-011). Ver header. Travado por
// platform/tests/audio_engine_test.cpp (TEST-FIRST, caminho null-device + WAV
// sintetico) + smoke opcional de device real (GUSWORLD_AUDIO_SMOKE=1, app/).
//
// UNICA TU do projeto que define MINIAUDIO_IMPLEMENTATION (miniaudio.h e single-
// header vendorizado em third_party/miniaudio/ - qualquer outro .cpp que precise dos
// TIPOS de miniaudio so pode INCLUIR o header, nunca redefinir a implementacao).
//
// ENDERECOS ESTAVEIS: ma_sound/ma_sound_group sao nos de um grafo intrusivo (guardam
// ponteiros pros vizinhos apos ma_*_init) - NAO podem ser realocados em memoria depois
// de inicializados. Por isso todo ma_sound de vida mais longa que uma chamada fica
// atras de um std::unique_ptr<ma_sound> dentro de um vector (o vector pode crescer/
// realocar o ARRAY DE PONTEIROS a vontade; os ma_sound em si, no heap, ficam parados).
//
// MIXER MINIMO (ADR-011 item 3): 2 ma_sound_group (music/sfx) - filhos diretos do
// endpoint do engine. NAO e o sistema de 5 buses (Master/Music/SFX/UI/Voice) + ducking
// + snapshots da onda futura (explicitamente adiado pelo ADR).

#include "gus/platform/audio/audio_engine.hpp"

#include <algorithm>  // std::clamp
#include <iostream>
#include <string>
#include <vector>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace gus::platform::audio {

struct AudioEngine::Impl {
    ma_engine engine{};
    bool engine_ready = false;

    ma_sound_group music_group{};
    ma_sound_group sfx_group{};
    bool groups_ready = false;

    // SFX: templates decodificados por completo no load (MA_SOUND_FLAG_DECODE),
    // indexados por SoundId (1-based; slot [id-1]). Enderecos ESTAVEIS (unique_ptr).
    std::vector<std::unique_ptr<ma_sound>> sfx_templates;

    // Instancias efemeras (fire-and-forget) em execucao - uma ma_sound_init_copy por
    // play_sfx. Podadas preguicosamente (reap_finished_sfx_instances) antes de cada
    // novo play e no destrutor.
    std::vector<std::unique_ptr<ma_sound>> sfx_instances;

    // Musica: so o CAMINHO e cacheado no load (o stream real comeca em play_music -
    // musica pode ser grande, decodificar tudo no load seria desperdicio). 1-based.
    std::vector<std::string> music_paths;

    // Musica corrente tocando (no maximo 1 - play_music substitui). nullptr = nenhuma.
    std::unique_ptr<ma_sound> current_music;
};

namespace {

// Flags comuns: NO_SPATIALIZATION porque o jogo e 2D sem audio posicional (evita
// atenuacao/pan surpresa vindos do modelo de spatializacao default do miniaudio,
// pensado pra jogos 3D - nao faz parte do escopo desta onda nem de nenhuma futura
// mapeada no ADR-011).
constexpr ma_uint32 kSfxLoadFlags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION;
constexpr ma_uint32 kMusicLoadFlags = MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION;

}  // namespace

AudioEngine::AudioEngine(bool device_active) noexcept : impl_(std::make_unique<Impl>()) {
    ma_engine_config cfg = ma_engine_config_init();
    if (!device_active) {
        // Null-device (ADR-011 item 4): o ma_engine continua funcional (decodifica e
        // gerencia sons de verdade), so nao abre nenhum device de hardware - usado em
        // testes/CI/smoke sem garantia de placa de som. channels/sampleRate precisam
        // ser explicitos quando noDevice=true (miniaudio nao tem device pra inferir).
        cfg.noDevice = MA_TRUE;
        cfg.channels = 2;
        cfg.sampleRate = 48000;
    }

    const ma_result engine_result = ma_engine_init(&cfg, &impl_->engine);
    if (engine_result != MA_SUCCESS) {
        std::cerr << "AudioEngine: falha ao inicializar ("
                  << ma_result_description(engine_result)
                  << ") - jogo roda sem audio (degradacao graciosa)\n";
        return;  // impl_->engine_ready fica false; API inteira vira no-op
    }
    impl_->engine_ready = true;

    const ma_result music_group_result =
        ma_sound_group_init(&impl_->engine, 0, nullptr, &impl_->music_group);
    const ma_result sfx_group_result =
        ma_sound_group_init(&impl_->engine, 0, nullptr, &impl_->sfx_group);
    if (music_group_result != MA_SUCCESS || sfx_group_result != MA_SUCCESS) {
        std::cerr << "AudioEngine: falha ao criar o mixer minimo (buses music/sfx) - "
                     "jogo roda sem audio (degradacao graciosa)\n";
        ma_engine_uninit(&impl_->engine);
        impl_->engine_ready = false;
        return;
    }
    impl_->groups_ready = true;
}

AudioEngine::~AudioEngine() {
    if (!impl_) {
        return;
    }
    // Ordem de destruicao do grafo miniaudio: sons -> grupos -> engine.
    for (auto& instance : impl_->sfx_instances) {
        ma_sound_uninit(instance.get());
    }
    impl_->sfx_instances.clear();

    for (auto& tmpl : impl_->sfx_templates) {
        ma_sound_uninit(tmpl.get());
    }
    impl_->sfx_templates.clear();

    if (impl_->current_music) {
        ma_sound_uninit(impl_->current_music.get());
        impl_->current_music.reset();
    }

    if (impl_->groups_ready) {
        ma_sound_group_uninit(&impl_->sfx_group);
        ma_sound_group_uninit(&impl_->music_group);
    }
    if (impl_->engine_ready) {
        ma_engine_uninit(&impl_->engine);
    }
}

bool AudioEngine::available() const noexcept {
    return impl_->engine_ready && impl_->groups_ready;
}

void AudioEngine::reap_finished_sfx_instances() noexcept {
    auto& instances = impl_->sfx_instances;
    for (auto it = instances.begin(); it != instances.end();) {
        if (ma_sound_at_end(it->get())) {
            ma_sound_uninit(it->get());
            it = instances.erase(it);
        } else {
            ++it;
        }
    }
}

SoundId AudioEngine::load_sfx(const char* path) {
    if (!available() || path == nullptr) {
        return kInvalidSound;
    }

    auto sound = std::make_unique<ma_sound>();
    const ma_result result = ma_sound_init_from_file(
        &impl_->engine, path, kSfxLoadFlags, &impl_->sfx_group, nullptr, sound.get());
    if (result != MA_SUCCESS) {
        std::cerr << "AudioEngine::load_sfx: falha ao carregar '" << path << "' ("
                  << ma_result_description(result) << ")\n";
        return kInvalidSound;
    }

    impl_->sfx_templates.push_back(std::move(sound));
    return static_cast<SoundId>(impl_->sfx_templates.size());  // 1-based
}

SoundId AudioEngine::load_music(const char* path) {
    if (!available() || path == nullptr) {
        return kInvalidSound;
    }

    // Probe: abre como STREAM so pra validar que o arquivo existe/decodifica, depois
    // fecha - o stream de verdade comeca em play_music (troca de faixa e frequente;
    // decodificar antecipado no load desperdicaria - musica pode ser longa).
    ma_sound probe;
    const ma_result result = ma_sound_init_from_file(&impl_->engine, path, kMusicLoadFlags,
                                                      &impl_->music_group, nullptr, &probe);
    if (result != MA_SUCCESS) {
        std::cerr << "AudioEngine::load_music: falha ao carregar '" << path << "' ("
                  << ma_result_description(result) << ")\n";
        return kInvalidSound;
    }
    ma_sound_uninit(&probe);

    impl_->music_paths.emplace_back(path);
    return static_cast<SoundId>(impl_->music_paths.size());  // 1-based
}

void AudioEngine::play_sfx(SoundId id) {
    if (!available() || id == kInvalidSound || id > impl_->sfx_templates.size()) {
        return;
    }

    reap_finished_sfx_instances();  // poda antes de crescer o pool

    auto instance = std::make_unique<ma_sound>();
    ma_sound* tmpl = impl_->sfx_templates[id - 1].get();
    const ma_result result =
        ma_sound_init_copy(&impl_->engine, tmpl, 0, &impl_->sfx_group, instance.get());
    if (result != MA_SUCCESS) {
        std::cerr << "AudioEngine::play_sfx: falha ao instanciar ("
                  << ma_result_description(result) << ")\n";
        return;
    }

    ma_sound_start(instance.get());
    impl_->sfx_instances.push_back(std::move(instance));
    ++sfx_play_count_;  // hook de teste (M6 F3) - so conta o caminho que TOCOU de fato
}

void AudioEngine::play_music(SoundId id, bool loop, float fade_in_seconds) {
    if (!available() || id == kInvalidSound || id > impl_->music_paths.size()) {
        return;
    }

    // Troca de faixa: para/descarta a musica corrente IMEDIATAMENTE, sem fade (a
    // ANTERIOR nunca recebe fade-out aqui - so a NOVA pode entrar com fade-in).
    if (impl_->current_music) {
        ma_sound_uninit(impl_->current_music.get());
        impl_->current_music.reset();
    }

    auto sound = std::make_unique<ma_sound>();
    const std::string& path = impl_->music_paths[id - 1];
    const ma_result result = ma_sound_init_from_file(&impl_->engine, path.c_str(),
                                                      kMusicLoadFlags, &impl_->music_group,
                                                      nullptr, sound.get());
    if (result != MA_SUCCESS) {
        std::cerr << "AudioEngine::play_music: falha ao (re)abrir stream '" << path
                  << "' (" << ma_result_description(result) << ")\n";
        return;
    }

    // MA_SOUND_FLAG_STREAM + looping nativo (ma_sound_set_looping): o miniaudio reinicia
    // o stream internamente no fim da faixa, sem gap/silencio entre o ultimo frame e o
    // primeiro do proximo ciclo (loop e responsabilidade do decoder, nao do host).
    ma_sound_set_looping(sound.get(), loop ? MA_TRUE : MA_FALSE);

    // FADE-IN (M6 F4, ADR-011): fader nativo do miniaudio, mesmo padrao de stop_music.
    // <= 0 (inclui o default 0.0f) = pula o fade -> volume cheio imediato, identico ao
    // comportamento anterior (F1/F3) sem essa chamada.
    if (fade_in_seconds > 0.0f) {
        const auto fade_ms = static_cast<ma_uint64>(fade_in_seconds * 1000.0f);
        ma_sound_set_fade_in_milliseconds(sound.get(), 0.0f, 1.0f, fade_ms);
    }

    ma_sound_start(sound.get());
    impl_->current_music = std::move(sound);
    ++music_play_count_;  // hook de teste (M6 F4) - so conta o caminho que TOCOU de fato
}

void AudioEngine::stop_music(float fade_seconds) {
    if (!available() || !impl_->current_music) {
        return;
    }
    const float clamped_seconds = fade_seconds > 0.0f ? fade_seconds : 0.0f;
    const auto fade_ms = static_cast<ma_uint64>(clamped_seconds * 1000.0f);
    // Fader NATIVO do miniaudio (ADR-011 item 3: "miniaudio tem fader nativo em
    // ma_sound, use o que a lib da em vez de reinventar"). O sound continua existindo
    // ate o proximo play_music trocar ou o destrutor limpar - nao ha polling nesta API
    // minima pra saber quando o fade termina.
    ma_sound_stop_with_fade_in_milliseconds(impl_->current_music.get(), fade_ms);
}

void AudioEngine::set_master_volume(float volume) {
    master_volume_ = std::clamp(volume, 0.0f, 1.0f);
    if (available()) {
        ma_engine_set_volume(&impl_->engine, master_volume_);
    }
}

float AudioEngine::master_volume() const noexcept { return master_volume_; }

void AudioEngine::set_music_volume(float volume) {
    music_volume_ = std::clamp(volume, 0.0f, 1.0f);
    if (available()) {
        ma_sound_group_set_volume(&impl_->music_group, music_volume_);
    }
}

float AudioEngine::music_volume() const noexcept { return music_volume_; }

void AudioEngine::set_sfx_volume(float volume) {
    sfx_volume_ = std::clamp(volume, 0.0f, 1.0f);
    if (available()) {
        ma_sound_group_set_volume(&impl_->sfx_group, sfx_volume_);
    }
}

float AudioEngine::sfx_volume() const noexcept { return sfx_volume_; }

unsigned int AudioEngine::sfx_play_count() const noexcept { return sfx_play_count_; }

unsigned int AudioEngine::music_play_count() const noexcept { return music_play_count_; }

bool AudioEngine::music_is_playing() const noexcept {
    if (!available() || !impl_->current_music) {
        return false;
    }
    return ma_sound_is_playing(impl_->current_music.get()) == MA_TRUE;
}

}  // namespace gus::platform::audio
