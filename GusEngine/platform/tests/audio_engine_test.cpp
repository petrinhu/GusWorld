// GusEngine/platform/tests/audio_engine_test.cpp
//
// Catch2 do AudioEngine (platform/audio, M6 F1, ADR-011). TEST-FIRST.
//
// O irredutivel de HARDWARE de audio (device real tocando som audivel) e coberto pelo
// smoke opcional (GUSWORLD_AUDIO_SMOKE=1, app/src/audio_smoke.cpp) - aqui exercitamos a
// LOGICA do AudioEngine que da pra travar SEM garantia de placa de som no CI/xvfb:
//   - modo NULL-DEVICE forcado (device_active=false): o miniaudio noDevice=true nao
//     depende de hardware - available() fica deterministicamente true, e prova o
//     "caminho null-device pra testes" exigido pelo alicerce.
//   - modo device REAL (device_active=true): available() pode dar true OU false
//     dependendo do host (CI tipicamente sem placa de som) - o teste NAO assume um dos
//     dois; so exige que NENHUMA chamada subsequente crashe (degradacao graciosa).
//   - carregamento/playback fim-a-fim usa um WAV SINTETICO gerado em runtime (PCM16
//     mono, tom puro) escrito num arquivo temporario - nao depende de nenhum asset de
//     audio real (a curadoria do kit CC0 e F2, fase futura, fora deste alicerce).

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "gus/platform/audio/audio_engine.hpp"

using gus::platform::audio::AudioEngine;
using gus::platform::audio::kInvalidSound;
using gus::platform::audio::SoundId;

namespace {

// Escreve um WAV PCM16 mono minimo (tom puro, sample_rate/duration/freq dados) no
// caminho pedido. Usado SO pelos testes headless (nenhum asset real de audio existe
// ainda - a curadoria do kit CC0 e F2, fase futura). Devolve o caminho por conveniencia.
std::string write_test_tone_wav(const std::filesystem::path& path,
                                int sample_rate = 22050, float duration_s = 0.1f,
                                float freq_hz = 440.0f) {
    const auto num_samples =
        static_cast<std::uint32_t>(static_cast<float>(sample_rate) * duration_s);
    std::vector<std::int16_t> samples(num_samples);
    for (std::uint32_t i = 0; i < num_samples; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(sample_rate);
        const float s = 0.2f * std::sin(2.0f * 3.14159265f * freq_hz * t);
        samples[i] = static_cast<std::int16_t>(s * 32767.0f);
    }

    const std::uint32_t data_bytes = num_samples * sizeof(std::int16_t);
    const std::uint32_t byte_rate = static_cast<std::uint32_t>(sample_rate) * 2;
    const std::uint16_t block_align = 2;
    const std::uint16_t bits_per_sample = 16;
    const std::uint32_t riff_size = 36 + data_bytes;

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    auto w32 = [&out](std::uint32_t v) { out.write(reinterpret_cast<const char*>(&v), 4); };
    auto w16 = [&out](std::uint16_t v) { out.write(reinterpret_cast<const char*>(&v), 2); };

    out.write("RIFF", 4);
    w32(riff_size);
    out.write("WAVE", 4);
    out.write("fmt ", 4);
    w32(16);                                    // tamanho do subchunk fmt
    w16(1);                                     // PCM
    w16(1);                                     // mono
    w32(static_cast<std::uint32_t>(sample_rate));
    w32(byte_rate);
    w16(block_align);
    w16(bits_per_sample);
    out.write("data", 4);
    w32(data_bytes);
    out.write(reinterpret_cast<const char*>(samples.data()),
              static_cast<std::streamsize>(data_bytes));

    return path.string();
}

}  // namespace

TEST_CASE("AudioEngine null-device forcado: available() deterministico",
          "[audio_engine]") {
    AudioEngine engine(/*device_active=*/false);
    // Modo null-device do miniaudio nao depende de hardware - deve sempre subir.
    REQUIRE(engine.available());
}

TEST_CASE("AudioEngine device real: nunca crasha independente do host ter placa de som",
          "[audio_engine]") {
    AudioEngine engine(/*device_active=*/true);
    // NAO assume available() true nem false (depende do host/CI) - so exige
    // seguranca: toda a API abaixo deve ser um no-op silencioso se indisponivel.
    engine.set_master_volume(0.5f);
    REQUIRE(engine.master_volume() == Catch::Approx(0.5f));
    const SoundId sfx = engine.load_sfx("nao/existe.wav");
    engine.play_sfx(sfx);
    const SoundId music = engine.load_music("nao/existe.wav");
    engine.play_music(music, /*loop=*/true);
    engine.stop_music(0.5f);
    SUCCEED("device real nao crashou em nenhum ambiente (com ou sem hardware)");
}

TEST_CASE("AudioEngine load_sfx com caminho inexistente degrada para kInvalidSound",
          "[audio_engine]") {
    AudioEngine engine(/*device_active=*/false);
    REQUIRE(engine.load_sfx("caminho/que/nao/existe.wav") == kInvalidSound);
}

TEST_CASE("AudioEngine load_music com caminho inexistente degrada para kInvalidSound",
          "[audio_engine]") {
    AudioEngine engine(/*device_active=*/false);
    REQUIRE(engine.load_music("caminho/que/nao/existe.wav") == kInvalidSound);
}

TEST_CASE("AudioEngine play_sfx(kInvalidSound) e no-op seguro", "[audio_engine]") {
    AudioEngine engine(/*device_active=*/false);
    engine.play_sfx(kInvalidSound);
    SUCCEED("no-op nao crashou");
}

TEST_CASE("AudioEngine play_music(kInvalidSound) e no-op seguro", "[audio_engine]") {
    AudioEngine engine(/*device_active=*/false);
    engine.play_music(kInvalidSound, /*loop=*/true);
    SUCCEED("no-op nao crashou");
}

TEST_CASE("AudioEngine stop_music sem musica tocando e no-op seguro", "[audio_engine]") {
    AudioEngine engine(/*device_active=*/false);
    engine.stop_music(1.0f);
    SUCCEED("no-op nao crashou");
}

TEST_CASE("AudioEngine set_master_volume clampa para [0,1]", "[audio_engine]") {
    AudioEngine engine(/*device_active=*/false);
    engine.set_master_volume(-5.0f);
    REQUIRE(engine.master_volume() == Catch::Approx(0.0f));
    engine.set_master_volume(5.0f);
    REQUIRE(engine.master_volume() == Catch::Approx(1.0f));
    engine.set_master_volume(0.3f);
    REQUIRE(engine.master_volume() == Catch::Approx(0.3f));
}

TEST_CASE("AudioEngine load_sfx com WAV sintetico valido: id valido + play repetido",
          "[audio_engine]") {
    AudioEngine engine(/*device_active=*/false);  // null-device: decode nao depende de hardware
    const auto tmp = std::filesystem::temp_directory_path() / "gusworld_test_sfx_tone.wav";
    write_test_tone_wav(tmp);

    const SoundId id = engine.load_sfx(tmp.string().c_str());
    REQUIRE(id != kInvalidSound);

    // Dispara varias vezes seguidas (exercita o pool de instancias efemeras + a poda
    // preguicosa antes de cada novo play - nao deve crescer sem limite nem crashar).
    for (int i = 0; i < 5; ++i) {
        engine.play_sfx(id);
    }

    std::filesystem::remove(tmp);
    SUCCEED("load + play repetido de SFX real (sintetico) nao crashou");
}

TEST_CASE("AudioEngine load_music com WAV sintetico valido: play/loop/stop com fade",
          "[audio_engine]") {
    AudioEngine engine(/*device_active=*/false);
    const auto tmp =
        std::filesystem::temp_directory_path() / "gusworld_test_music_tone.wav";
    write_test_tone_wav(tmp, /*sample_rate=*/22050, /*duration_s=*/0.2f, /*freq_hz=*/220.0f);

    const SoundId id = engine.load_music(tmp.string().c_str());
    REQUIRE(id != kInvalidSound);

    engine.play_music(id, /*loop=*/true);
    // Troca de faixa (mesma id de novo): deve substituir sem crashar.
    engine.play_music(id, /*loop=*/false);
    engine.stop_music(0.2f);  // fade nativo do miniaudio

    std::filesystem::remove(tmp);
    SUCCEED("load + play/loop/stop de musica real (sintetica) nao crashou");
}

TEST_CASE("AudioEngine: SFX e musica sao pools independentes (mesmo id, engines vazios)",
          "[audio_engine]") {
    AudioEngine engine(/*device_active=*/false);
    // Sem nenhum load, qualquer id > 0 esta fora dos dois pools - no-op seguro.
    engine.play_sfx(1);
    engine.play_music(1, true);
    SUCCEED("ids fora de alcance nos dois pools nao crasham");
}

TEST_CASE("AudioEngine play_music sem fade_in_seconds preserva o comportamento antigo "
          "(default 0.0f = volume cheio imediato, chamada de 2 args ainda compila)",
          "[audio_engine][m6_f4]") {
    AudioEngine engine(/*device_active=*/false);
    const auto tmp =
        std::filesystem::temp_directory_path() / "gusworld_test_music_default.wav";
    write_test_tone_wav(tmp, /*sample_rate=*/22050, /*duration_s=*/0.2f, /*freq_hz=*/220.0f);

    const SoundId id = engine.load_music(tmp.string().c_str());
    REQUIRE(id != kInvalidSound);

    // Chamada de 2 argumentos (assinatura antiga, F1/F3) continua valida - a extensao e
    // ADITIVA (fade_in_seconds tem default 0.0f).
    engine.play_music(id, /*loop=*/true);
    REQUIRE(engine.music_is_playing());
    REQUIRE(engine.music_play_count() == 1);

    std::filesystem::remove(tmp);
}

TEST_CASE("AudioEngine play_music com fade_in_seconds toca em loop e conta 1x "
          "(M6 F4, ADR-011)",
          "[audio_engine][m6_f4]") {
    AudioEngine engine(/*device_active=*/false);
    const auto tmp =
        std::filesystem::temp_directory_path() / "gusworld_test_music_fadein.wav";
    write_test_tone_wav(tmp, /*sample_rate=*/22050, /*duration_s=*/0.2f, /*freq_hz=*/220.0f);

    const SoundId id = engine.load_music(tmp.string().c_str());
    REQUIRE(id != kInvalidSound);
    REQUIRE_FALSE(engine.music_is_playing());  // nada tocando antes do play_music

    engine.play_music(id, /*loop=*/true, /*fade_in_seconds=*/2.0f);
    REQUIRE(engine.music_is_playing());
    REQUIRE(engine.music_play_count() == 1);

    std::filesystem::remove(tmp);
}

TEST_CASE("AudioEngine music_play_count: kInvalidSound/id fora de alcance NAO incrementa "
          "(M6 F4, hook de teste headless)",
          "[audio_engine][m6_f4]") {
    AudioEngine engine(/*device_active=*/false);
    REQUIRE(engine.music_play_count() == 0);

    engine.play_music(kInvalidSound, /*loop=*/true, /*fade_in_seconds=*/1.0f);
    engine.play_music(999, /*loop=*/true);
    REQUIRE(engine.music_play_count() == 0);
    REQUIRE_FALSE(engine.music_is_playing());
}

TEST_CASE("AudioEngine stop_music com fade para o node de musica (music_is_playing "
          "reflete o estado apos o fade programado terminar de ser AGENDADO)",
          "[audio_engine][m6_f4]") {
    AudioEngine engine(/*device_active=*/false);
    const auto tmp =
        std::filesystem::temp_directory_path() / "gusworld_test_music_stopfade.wav";
    write_test_tone_wav(tmp, /*sample_rate=*/22050, /*duration_s=*/0.2f, /*freq_hz=*/220.0f);

    const SoundId id = engine.load_music(tmp.string().c_str());
    REQUIRE(id != kInvalidSound);

    engine.play_music(id, /*loop=*/true, /*fade_in_seconds=*/0.5f);
    REQUIRE(engine.music_is_playing());

    // stop_music agenda um fade-out (nao para instantaneamente) - o node continua
    // "playing" (node_state_started, com fade programado) ate o tempo do fade decorrer;
    // aqui so provamos que a chamada e segura e nao derruba o estado imediatamente
    // (mesmo espirito do teste original "play/loop/stop com fade" da F1).
    engine.stop_music(0.3f);
    SUCCEED("stop_music com fade nao crashou e nao exige polling nesta API minima");

    std::filesystem::remove(tmp);
}

TEST_CASE("AudioEngine: SFX e musica coexistem - tocar o hit NAO para a musica "
          "(grupos music/sfx independentes, M6 F4)",
          "[audio_engine][m6_f4]") {
    AudioEngine engine(/*device_active=*/false);

    const auto music_tmp =
        std::filesystem::temp_directory_path() / "gusworld_test_coexist_music.wav";
    write_test_tone_wav(music_tmp, /*sample_rate=*/22050, /*duration_s=*/0.2f,
                         /*freq_hz=*/220.0f);
    const auto sfx_tmp =
        std::filesystem::temp_directory_path() / "gusworld_test_coexist_sfx.wav";
    write_test_tone_wav(sfx_tmp, /*sample_rate=*/22050, /*duration_s=*/0.1f,
                         /*freq_hz=*/880.0f);

    const SoundId music_id = engine.load_music(music_tmp.string().c_str());
    const SoundId sfx_id = engine.load_sfx(sfx_tmp.string().c_str());
    REQUIRE(music_id != kInvalidSound);
    REQUIRE(sfx_id != kInvalidSound);

    engine.play_music(music_id, /*loop=*/true, /*fade_in_seconds=*/1.0f);
    REQUIRE(engine.music_is_playing());

    // Dispara o SFX varias vezes (simula golpes repetidos) - a musica segue tocando.
    engine.play_sfx(sfx_id);
    engine.play_sfx(sfx_id);
    REQUIRE(engine.music_is_playing());
    REQUIRE(engine.sfx_play_count() == 2);
    REQUIRE(engine.music_play_count() == 1);

    std::filesystem::remove(music_tmp);
    std::filesystem::remove(sfx_tmp);
}

TEST_CASE("AudioEngine sfx_play_count: conta so os play_sfx que TOCARAM de fato "
          "(M6 F3, hook de teste headless)",
          "[audio_engine]") {
    AudioEngine engine(/*device_active=*/false);
    REQUIRE(engine.sfx_play_count() == 0);

    // No-op (id kInvalidSound / fora de alcance): NAO incrementa.
    engine.play_sfx(kInvalidSound);
    engine.play_sfx(999);
    REQUIRE(engine.sfx_play_count() == 0);

    const auto tmp =
        std::filesystem::temp_directory_path() / "gusworld_test_sfx_count.wav";
    write_test_tone_wav(tmp);
    const SoundId id = engine.load_sfx(tmp.string().c_str());
    REQUIRE(id != kInvalidSound);

    engine.play_sfx(id);
    REQUIRE(engine.sfx_play_count() == 1);
    engine.play_sfx(id);
    engine.play_sfx(id);
    REQUIRE(engine.sfx_play_count() == 3);  // 1 por chamada, mesmo id repetido

    std::filesystem::remove(tmp);
}
