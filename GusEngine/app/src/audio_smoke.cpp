// gus/app/src/audio_smoke.cpp
//
// Ver header. Diagnostico opcional de device de audio REAL (M6 F1, ADR-011, item 5).

#include "gus/app/audio_smoke.hpp"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>  // std::getenv
#include <cstring>  // std::strcmp
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "gus/platform/audio/audio_engine.hpp"

namespace gus::app {

namespace {

// Escreve um WAV PCM16 mono minimo (tom puro) no caminho pedido - o MESMO tipo de
// gerador usado em platform/tests/audio_engine_test.cpp, aqui pra provar o device
// REAL (a onda so tem device sob prova, o kit CC0 de verdade e curadoria F2 futura).
void write_tone_wav(const std::filesystem::path& path, int sample_rate,
                     float duration_s, float freq_hz) {
    const auto num_samples =
        static_cast<std::uint32_t>(static_cast<float>(sample_rate) * duration_s);
    std::vector<std::int16_t> samples(num_samples);
    for (std::uint32_t i = 0; i < num_samples; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(sample_rate);
        // Envelope simples (fade in/out de ~10ms) pra evitar clique de borda audivel.
        const float fade_s = 0.01f;
        float env = 1.0f;
        if (t < fade_s) {
            env = t / fade_s;
        } else if (duration_s - t < fade_s) {
            env = (duration_s - t) / fade_s;
        }
        const float s = 0.3f * env * std::sin(2.0f * 3.14159265f * freq_hz * t);
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
    w32(16);
    w16(1);  // PCM
    w16(1);  // mono
    w32(static_cast<std::uint32_t>(sample_rate));
    w32(byte_rate);
    w16(block_align);
    w16(bits_per_sample);
    out.write("data", 4);
    w32(data_bytes);
    out.write(reinterpret_cast<const char*>(samples.data()),
              static_cast<std::streamsize>(data_bytes));
}

}  // namespace

bool audio_smoke_requested() {
    const char* env = std::getenv("GUSWORLD_AUDIO_SMOKE");
    return env != nullptr && std::strcmp(env, "1") == 0;
}

int run_audio_smoke() {
    std::cout << "GusEngine audio smoke: inicializando device REAL...\n";
    gus::platform::audio::AudioEngine engine(/*device_active=*/true);

    if (engine.available()) {
        std::cout << "  device de audio REAL disponivel (available()==true).\n";
    } else {
        std::cout << "  device de audio REAL indisponivel neste host - degradacao "
                     "graciosa OK (available()==false, API vira no-op).\n";
    }

    const auto tone_path =
        std::filesystem::temp_directory_path() / "gusworld_audio_smoke_tone.wav";
    write_tone_wav(tone_path, /*sample_rate=*/44100, /*duration_s=*/0.4f,
                    /*freq_hz=*/523.25f);  // Do5, tom curto neutro

    const auto sfx_id = engine.load_sfx(tone_path.string().c_str());
    std::cout << "  load_sfx: " << (sfx_id != gus::platform::audio::kInvalidSound
                                        ? "OK (som carregado)"
                                        : "falhou (kInvalidSound)")
              << "\n";
    engine.play_sfx(sfx_id);
    std::cout << "  play_sfx disparado - aguardando ~0.5s...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    const auto music_id = engine.load_music(tone_path.string().c_str());
    std::cout << "  load_music: " << (music_id != gus::platform::audio::kInvalidSound
                                          ? "OK (faixa carregada)"
                                          : "falhou (kInvalidSound)")
              << "\n";
    engine.play_music(music_id, /*loop=*/true);
    std::cout << "  play_music (loop) disparado - aguardando ~0.5s...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    engine.stop_music(/*fade_seconds=*/0.3f);
    std::cout << "  stop_music com fade de 0.3s - aguardando o fade terminar...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    engine.set_master_volume(0.7f);
    std::cout << "  set_master_volume(0.7) -> master_volume()=" << engine.master_volume()
              << "\n";

    std::error_code ec;
    std::filesystem::remove(tone_path, ec);

    std::cout << "GusEngine audio smoke OK: nenhuma etapa crashou (device "
              << (engine.available() ? "real ativo" : "indisponivel/degradado")
              << ").\n";
    // engine sai de escopo aqui -> destrutor RAII fecha o device com seguranca.
    return 0;
}

}  // namespace gus::app
