// gus/core/src/anim/boot_pixel_sequence.cpp
//
// Implementacao dos POCO boot_pixel_frame_index/boot_pixel_safety_alpha. Ver header.
// Travado por GusEngine/tests/boot_pixel_sequence_test.cpp (TEST-FIRST).

#include "gus/core/anim/boot_pixel_sequence.hpp"

#include <cmath>  // std::isfinite/std::fmod - validar o float ANTES do cast (ver abaixo)

namespace gus::core::anim {

namespace {

[[nodiscard]] float clamp01(float v) noexcept {
    if (v < 0.0f) {
        return 0.0f;
    }
    if (v > 1.0f) {
        return 1.0f;
    }
    return v;
}

[[nodiscard]] int round_to_int(float v) noexcept {
    return static_cast<int>(v + 0.5f);
}

}  // namespace

int boot_pixel_frame_index(BootPixelLeg leg, float t, int frame_count) noexcept {
    if (frame_count <= 0) {
        return 0;  // "asset vazio" - resposta bem definida, sem divisao por zero.
    }
    if (frame_count == 1) {
        return 0;  // 1 frame so: sempre ele, qualquer t/leg.
    }

    const float ct = clamp01(t);
    const int half = frame_count / 2;               // 1a metade: [0, half-1]
    const int second_half_size = frame_count - half; // 2a metade: [half, frame_count-1]
    const int last = frame_count - 1;

    int idx = 0;
    switch (leg) {
        case BootPixelLeg::kToBattleDarkening:
            // [0, half-1] ASCENDENTE.
            idx = round_to_int(ct * static_cast<float>(half - 1));
            break;
        case BootPixelLeg::kToBattleRevealing:
            // [half, last] ASCENDENTE - termina no ULTIMO frame.
            idx = half + round_to_int(ct * static_cast<float>(second_half_size - 1));
            break;
        case BootPixelLeg::kFromBattleDarkening:
            // [last, half] DESCENDENTE - espelha kToBattleRevealing.
            idx = last - round_to_int(ct * static_cast<float>(second_half_size - 1));
            break;
        case BootPixelLeg::kFromBattleRevealing:
            // [half-1, 0] DESCENDENTE - espelha kToBattleDarkening, termina no 1o frame.
            idx = (half - 1) - round_to_int(ct * static_cast<float>(half - 1));
            break;
    }

    if (idx < 0) {
        idx = 0;
    } else if (idx > last) {
        idx = last;
    }
    return idx;
}

int boot_pixel_idle_frame_index(float elapsed_seconds, int frame_count) noexcept {
    if (frame_count <= 0) {
        return 0;  // "asset vazio" - mesma garantia de boot_pixel_frame_index.
    }
    const int window =
        frame_count < kBootPixelIdleWindowFrames ? frame_count : kBootPixelIdleWindowFrames;
    const int start = frame_count - window;  // window>=1 sempre (frame_count>=1 aqui).

    // VALIDAR O FLOAT ANTES DO CAST (licao canonica do projeto - achado QA
    // pos-implementacao, float-cast-overflow em elapsed_seconds astronomico/nao-
    // finito): `elapsed_seconds` vem de um relogio REAL (SDL_GetTicksNS() em
    // title_menu_loop.cpp) que so cresce - NaN ja degrada pra 0 pela comparacao
    // `> 0.0f` abaixo (IEEE 754: qualquer comparacao com NaN e false), mas
    // +Infinity (ou um float finito astronomico, tela de titulo aberta por decadas)
    // passa reto por essa comparacao. std::isfinite() aqui e o guard EXPLICITO -
    // nao-finito degrada pra 0 (MESMO racional do "negativo vira 0" ja existente).
    const float safe_elapsed = (elapsed_seconds > 0.0f && std::isfinite(elapsed_seconds))
                                    ? elapsed_seconds
                                    : 0.0f;

    // REDUZ pro periodo de UMA volta completa da janela ANTES de dividir/castar -
    // std::fmod de 2 floats FINITOS e SEMPRE bem definido, resultado em [0, period)
    // POR MAIOR que safe_elapsed seja (1 segundo ou 1e30 segundos, o range do
    // resultado e o MESMO) - o cast a seguir NUNCA ve um valor fora de [0, window),
    // eliminando o float-cast-overflow POR CONSTRUCAO (nao so pro caso "plausivel").
    const float period = static_cast<float>(window) * kBootPixelIdleFrameSeconds;
    const float phase = std::fmod(safe_elapsed, period);  // [0, period), sempre finito
    int step = static_cast<int>(phase / kBootPixelIdleFrameSeconds);
    // Defensivo (arredondamento de ponto-flutuante perto da borda de fmod pode
    // empurrar phase/kBootPixelIdleFrameSeconds pra window OU, por erro de
    // representacao no lado de baixo, pra -epsilon) - MESMO estilo dos clamps
    // idx<0/idx>last de boot_pixel_frame_index acima.
    if (step < 0) {
        step = 0;
    } else if (step >= window) {
        step = window - 1;
    }
    return start + step;
}

float boot_pixel_safety_alpha(BootPixelLeg leg, float t) noexcept {
    const float ct = clamp01(t);
    switch (leg) {
        case BootPixelLeg::kToBattleDarkening:
        case BootPixelLeg::kFromBattleDarkening:
            return ct;  // escurecendo: cresce com t (igual fade_overlay_alpha kOut).
        case BootPixelLeg::kToBattleRevealing:
        case BootPixelLeg::kFromBattleRevealing:
            return 1.0f - ct;  // revelando: decresce com t (igual fade_overlay_alpha kIn).
    }
    return 0.0f;  // inalcancavel (enum exaustivo acima) - guard defensivo pro compilador.
}

}  // namespace gus::core::anim
