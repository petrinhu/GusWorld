// gus/app/src/screens/battle_cockpit_pills.cpp
//
// Implementacao da geometria PURA dos pills de verbo do cockpit RCSS (ver header).
// Aritmetica de dp deterministica, sem SDL/glintfx. Ponto unico do hit-test de mouse.

#include "gus/app/screens/battle_cockpit_pills.hpp"

namespace gus::app::screens {

Rect cockpit_pill_rect(int verb_index) noexcept {
    if (verb_index < 0 || verb_index >= kCockpitPillCount) {
        return Rect{0.0f, 0.0f, 0.0f, 0.0f};  // fora de faixa: retangulo vazio
    }
    const float top =
        kCockpitFirstPillTopDp + static_cast<float>(verb_index) * kCockpitPillPitchDp;
    return Rect{kCockpitPillLeftDp, top, kCockpitPillBorderBoxWidthDp,
                kCockpitPillBorderBoxHeightDp};
}

int cockpit_pill_index_at(float dp_x, float dp_y) noexcept {
    // X: dentro da largura border-box do pill (a coluna do menu). Fora -> -1.
    if (dp_x < kCockpitPillLeftDp ||
        dp_x > kCockpitPillLeftDp + kCockpitPillBorderBoxWidthDp) {
        return -1;
    }
    // Y: ladrilha o passo a partir do topo do 1o pill. rel<0 = acima da pilha (area dos
    // vitals/pips) -> -1. index = piso(rel/passo); >= contagem = abaixo da pilha -> -1.
    const float rel = dp_y - kCockpitFirstPillTopDp;
    if (rel < 0.0f) {
        return -1;
    }
    const int index = static_cast<int>(rel / kCockpitPillPitchDp);
    if (index >= kCockpitPillCount) {
        return -1;
    }
    return index;
}

}  // namespace gus::app::screens
