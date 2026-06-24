// gus/app/src/screens/battle_hud_model.cpp
//
// Implementacao do modelo PURO do HUD (ver header). Aritmetica de pixels logicos,
// deterministica, sem SDL. Catalogo StatusId->arquivo, fracao de barra, retangulo de
// preenchimento, layout de pips de AP/Mana, quadro da mini-barra de HP da arena.

#include "gus/app/screens/battle_hud_model.hpp"

#include <algorithm>  // std::clamp, std::min, std::max

namespace gus::app::screens {

std::string_view status_icon_file(StatusId id) noexcept {
    switch (id) {
        case StatusId::Stun:      return "status_stun.png";
        case StatusId::Poison:    return "status_poison.png";
        case StatusId::Corrode:   return "status_corrode.png";
        case StatusId::Disrupt:   return "status_disrupt.png";
        case StatusId::Silence:   return "status_silence.png";
        case StatusId::Knockback: return "status_knockback.png";
        case StatusId::Break:     return "status_break.png";
        case StatusId::Expose:    return "status_expose.png";
        case StatusId::Decrypt:   return "status_decrypt.png";
        case StatusId::Shield:    return "status_shield.png";
        case StatusId::Regen:     return "status_regen.png";
        case StatusId::Haste:     return "status_haste.png";
        case StatusId::Slow:      return "status_slow.png";
    }
    // Inalcancavel (enum coberto); guarda de seguranca pra StatusId futuro nao mapeado.
    return "status_stun.png";
}

int status_icon_index(StatusId id) noexcept {
    return static_cast<int>(id);
}

float bar_fill(int current, int max) noexcept {
    if (max <= 0) {
        return 0.0f;
    }
    if (current <= 0) {
        return 0.0f;
    }
    if (current >= max) {
        return 1.0f;
    }
    return static_cast<float>(current) / static_cast<float>(max);
}

Rect bar_fill_rect(const Rect& frame, float fraction) noexcept {
    const float f = std::clamp(fraction, 0.0f, 1.0f);
    return Rect{frame.x, frame.y, frame.w * f, frame.h};
}

std::vector<ResourcePip> resource_pips(float x, float y, int total, int lit,
                                       int max_pips) noexcept {
    std::vector<ResourcePip> pips;
    const int n = std::clamp(total, 0, std::max(max_pips, 0));
    const int on = std::clamp(lit, 0, n);
    pips.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        const float px = x + static_cast<float>(i) * (kPipSize + kPipGap);
        pips.push_back(ResourcePip{
            Rect{px, y, static_cast<float>(kPipSize), static_cast<float>(kPipSize)},
            /*lit=*/i < on});
    }
    return pips;
}

Rect arena_hp_bar_frame(const Rect& actor_slot) noexcept {
    // Mesma largura do slot, centrada (slot ja e o quadro do ator), logo abaixo da base.
    const float y = actor_slot.y + actor_slot.h + static_cast<float>(kArenaHpBarGapY);
    return Rect{actor_slot.x, y, actor_slot.w, static_cast<float>(kArenaHpBarH)};
}

}  // namespace gus::app::screens
