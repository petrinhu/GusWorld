// gus/app/src/screens/battle_floaters.cpp
//
// Implementacao do modelo PURO dos numeros flutuantes (ver header). Parser de canal,
// cor por canal, e a animacao (subida + fade) como funcao pura do tempo. Sem SDL.

#include "gus/app/screens/battle_floaters.hpp"

#include <algorithm>  // std::clamp

namespace gus::app::screens {

namespace {
using gus::platform::render2d::DrawColor;

bool contains(const std::string& s, const char* sub) noexcept {
    return s.find(sub) != std::string::npos;
}
}  // namespace

HitResult parse_hit(const std::string& message, int value, bool is_heal) {
    HitResult out;
    out.value = value;

    if (is_heal) {
        out.channel = HitChannel::Heal;
        out.text = "+" + std::to_string(value);
        return out;
    }
    if (contains(message, "[CRITICO]")) {
        out.channel = HitChannel::Crit;
        out.text = std::to_string(value);
        return out;
    }
    if (contains(message, "FALHA DE COMPILACAO") || value <= 0) {
        // Falha de compilacao OU dano 0 (imune/sem efeito): estetica de erro.
        out.channel = HitChannel::Fail;
        out.text = "FALHA";
        return out;
    }
    out.channel = HitChannel::Common;
    out.text = std::to_string(value);
    return out;
}

DrawColor floater_color_for_channel(HitChannel channel) noexcept {
    switch (channel) {
        case HitChannel::Common:
            return DrawColor{0.96f, 0.96f, 0.98f, 1.0f};  // branco/claro
        case HitChannel::Crit:
            return DrawColor{0.30f, 0.95f, 0.98f, 1.0f};  // ciano
        case HitChannel::Fail:
            // #F43F5E -> (0.957, 0.247, 0.369) vermelho-erro.
            return DrawColor{0.957f, 0.247f, 0.369f, 1.0f};
        case HitChannel::Heal:
            return DrawColor{0.35f, 0.85f, 0.40f, 1.0f};  // verde
    }
    return DrawColor{1.0f, 1.0f, 1.0f, 1.0f};
}

bool floater_alive(float age_seconds) noexcept {
    return age_seconds >= 0.0f && age_seconds <= kFloaterLifeSeconds;
}

float floater_offset_y(float age_seconds) noexcept {
    const float t = std::clamp(age_seconds / kFloaterLifeSeconds, 0.0f, 1.0f);
    // Sobe linearmente: 0 -> -kFloaterRisePx (eixo +Y pra baixo, "subir" = negativo).
    return -kFloaterRisePx * t;
}

float floater_alpha(float age_seconds) noexcept {
    if (age_seconds <= 0.0f) {
        return 1.0f;
    }
    const float t = std::clamp(age_seconds / kFloaterLifeSeconds, 0.0f, 1.0f);
    return 1.0f - t;  // fade linear 1 -> 0
}

}  // namespace gus::app::screens
