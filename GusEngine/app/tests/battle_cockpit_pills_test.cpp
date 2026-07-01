// GusEngine/app/tests/battle_cockpit_pills_test.cpp
//
// Catch2 (headless) da GEOMETRIA PURA dos pills de verbo do cockpit RCSS (Incremento A2,
// hit-test de mouse). Prova SEM SDL/glintfx: os 6 retangulos em dp replicam os numeros da
// RCSS (largura/altura/gap/padding/borda/origem), sao contiguos no passo, e o hit-test
// mapeia ponto->indice de pill (centro acerta, gap ladrilha, fora -> -1). Funcoes PURAS.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/app/screens/battle_cockpit_pills.hpp"
#include "gus/app/screens/battle_menu.hpp"  // BattleVerb (ordem dos pills)

using Catch::Matchers::WithinAbs;
using gus::app::screens::cockpit_pill_index_at;
using gus::app::screens::cockpit_pill_rect;
using gus::app::screens::kCockpitFirstPillTopDp;
using gus::app::screens::kCockpitPillBorderBoxHeightDp;
using gus::app::screens::kCockpitPillBorderBoxWidthDp;
using gus::app::screens::kCockpitPillCount;
using gus::app::screens::kCockpitPillLeftDp;
using gus::app::screens::kCockpitPillPitchDp;
using gus::core::spatial::Rect;

namespace {
float cx(const Rect& r) { return r.x + r.w * 0.5f; }
float cy(const Rect& r) { return r.y + r.h * 0.5f; }
}  // namespace

TEST_CASE("pills: os numeros derivam da RCSS (border-box 136x20, passo 24dp)",
          "[cockpit_pills]") {
    // content 110 + 2*padding 12 + 2*borda 1 = 136; content 18 + 2*borda 1 = 20.
    REQUIRE_THAT(kCockpitPillBorderBoxWidthDp, WithinAbs(136.0f, 1e-4f));
    REQUIRE_THAT(kCockpitPillBorderBoxHeightDp, WithinAbs(20.0f, 1e-4f));
    // passo topo->topo = border-box height 20 + margin-bottom 4 = 24.
    REQUIRE_THAT(kCockpitPillPitchDp, WithinAbs(24.0f, 1e-4f));
    REQUIRE(kCockpitPillCount == 6);
}

TEST_CASE("pills: cada retangulo ancora em left=12dp e empilha no passo", "[cockpit_pills]") {
    for (int i = 0; i < kCockpitPillCount; ++i) {
        const Rect r = cockpit_pill_rect(i);
        REQUIRE_THAT(r.x, WithinAbs(kCockpitPillLeftDp, 1e-4f));  // left = 12dp
        REQUIRE_THAT(r.w, WithinAbs(kCockpitPillBorderBoxWidthDp, 1e-4f));
        REQUIRE_THAT(r.h, WithinAbs(kCockpitPillBorderBoxHeightDp, 1e-4f));
        const float esperado_top =
            kCockpitFirstPillTopDp + static_cast<float>(i) * kCockpitPillPitchDp;
        REQUIRE_THAT(r.y, WithinAbs(esperado_top, 1e-4f));
    }
}

TEST_CASE("pills: os retangulos casam os valores MEDIDOS na captura do cockpit vivo",
          "[cockpit_pills]") {
    // Ancoras medidas (subpixel) na captura do cockpit em combate: SCAN top ~279.6dp,
    // GAMBITO ~303.6dp (passo 24dp). Tolerancia 1.5dp (borda do .sel desloca +2dp os pills
    // abaixo do selecionado; a origem nominal fica dentro dessa folga). Prova ancoragem real.
    REQUIRE_THAT(cockpit_pill_rect(static_cast<int>(gus::app::screens::BattleVerb::Scan)).y,
                 WithinAbs(279.6f, 1.5f));
    REQUIRE_THAT(cockpit_pill_rect(static_cast<int>(gus::app::screens::BattleVerb::Gambito)).y,
                 WithinAbs(303.6f, 1.5f));
    // left do pill == borda esquerda medida (x=24px / dp_ratio 2 = 12dp).
    REQUIRE_THAT(cockpit_pill_rect(0).x, WithinAbs(12.0f, 1.0f));
}

TEST_CASE("hit-test: o CENTRO de cada pill mapeia pro seu indice", "[cockpit_pills]") {
    for (int i = 0; i < kCockpitPillCount; ++i) {
        const Rect r = cockpit_pill_rect(i);
        REQUIRE(cockpit_pill_index_at(cx(r), cy(r)) == i);
    }
}

TEST_CASE("hit-test: a ordem dos indices casa a ordem dos verbos (Scan..Flee)",
          "[cockpit_pills]") {
    using gus::app::screens::BattleVerb;
    // O indice geometrico (de cima pra baixo) == o valor do enum BattleVerb.
    REQUIRE(cockpit_pill_index_at(cx(cockpit_pill_rect(0)), cy(cockpit_pill_rect(0))) ==
            static_cast<int>(BattleVerb::Scan));
    REQUIRE(cockpit_pill_index_at(cx(cockpit_pill_rect(2)), cy(cockpit_pill_rect(2))) ==
            static_cast<int>(BattleVerb::Atacar));
    REQUIRE(cockpit_pill_index_at(cx(cockpit_pill_rect(5)), cy(cockpit_pill_rect(5))) ==
            static_cast<int>(BattleVerb::Flee));
}

TEST_CASE("hit-test: os bands LADRILHAM o passo sem zonas mortas (gap -> pill de cima)",
          "[cockpit_pills]") {
    // O gap de 4dp abaixo de um pill pertence a ele (band = passo cheio). Um ponto no gap
    // logo abaixo do pill i (topo_i + 21dp, alem do border-box 20) ainda mapeia pra i.
    const float x = kCockpitPillLeftDp + 5.0f;  // dentro da coluna
    for (int i = 0; i < kCockpitPillCount; ++i) {
        const float y_no_gap = kCockpitFirstPillTopDp +
                               static_cast<float>(i) * kCockpitPillPitchDp + 21.0f;
        REQUIRE(cockpit_pill_index_at(x, y_no_gap) == i);
    }
    // Fronteira exata entre band i e i+1: o topo do passo i+1 ja e o proximo indice.
    const float y_borda = kCockpitFirstPillTopDp + 1.0f * kCockpitPillPitchDp;
    REQUIRE(cockpit_pill_index_at(x, y_borda) == 1);
    REQUIRE(cockpit_pill_index_at(x, y_borda - 0.1f) == 0);
}

TEST_CASE("hit-test: fora da coluna/pilha devolve -1 (o clique nao 'erra')",
          "[cockpit_pills]") {
    const Rect r0 = cockpit_pill_rect(0);
    // Acima da pilha (area de vitals/pips): -1.
    REQUIRE(cockpit_pill_index_at(cx(r0), kCockpitFirstPillTopDp - 2.0f) == -1);
    // Abaixo do ultimo pill (area do log): -1.
    const float y_abaixo =
        kCockpitFirstPillTopDp + static_cast<float>(kCockpitPillCount) * kCockpitPillPitchDp +
        1.0f;
    REQUIRE(cockpit_pill_index_at(cx(r0), y_abaixo) == -1);
    // A esquerda da coluna (x < 12dp): -1.
    REQUIRE(cockpit_pill_index_at(kCockpitPillLeftDp - 1.0f, cy(r0)) == -1);
    // A direita do border-box (cockpit vazio, x > 148dp): -1.
    REQUIRE(cockpit_pill_index_at(kCockpitPillLeftDp + kCockpitPillBorderBoxWidthDp + 2.0f,
                                  cy(r0)) == -1);
}
