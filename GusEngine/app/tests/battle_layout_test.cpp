// GusEngine/app/tests/battle_layout_test.cpp
//
// Catch2 (headless) do LAYOUT PURO da BattleScreen (variante C "Tatico Cockpit"). Prova,
// SEM janela nem SDL: tela 960x540; cockpit lateral esq; arena a direita com party/
// inimigos DISTRIBUIDOS (space-around) sem sobreposicao; CTB no topo; banner em faixa
// propria; log no rodape; nenhuma zona invade outra (cockpit x arena x log).
// Tudo funcao PURA: mesma contagem -> mesmos retangulos (determinismo).

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/app/screens/battle_layout.hpp"

using Catch::Matchers::WithinAbs;
using gus::app::screens::arena_banner_rect;
using gus::app::screens::ArenaLayout;
using gus::app::screens::arena_layout;
using gus::app::screens::arena_rect;
using gus::app::screens::battle_screen_rect;
using gus::app::screens::cockpit_hp_bar_rect;
using gus::app::screens::cockpit_menu_zone;
using gus::app::screens::cockpit_portrait_rect;
using gus::app::screens::cockpit_rect;
using gus::app::screens::ctb_strip;
using gus::app::screens::CtbStrip;
using gus::app::screens::kArenaBottom;
using gus::app::screens::kArenaTop;
using gus::app::screens::kBattleLogicalH;
using gus::app::screens::kBattleLogicalW;
using gus::app::screens::kCockpitW;
using gus::app::screens::kCtbPortraitPx;
using gus::app::screens::kCtbVisibleCells;
using gus::app::screens::kGusRecuoX;
using gus::app::screens::kMaxEnemySlots;
using gus::app::screens::kMaxPartySlots;
using gus::app::screens::kRightZoneX;
using gus::app::screens::log_panel_rect;
using gus::core::spatial::Rect;

namespace {
float cy(const Rect& r) { return r.y + r.h * 0.5f; }
bool overlap_v(const Rect& a, const Rect& b) {
    return a.y < b.y + b.h && b.y < a.y + a.h;
}
}  // namespace

TEST_CASE("battle_screen_rect e a tela logica 960x540 na origem", "[battle_layout]") {
    const Rect s = battle_screen_rect();
    REQUIRE_THAT(s.w, WithinAbs(static_cast<float>(kBattleLogicalW), 1e-4f));
    REQUIRE_THAT(s.h, WithinAbs(static_cast<float>(kBattleLogicalH), 1e-4f));
}

TEST_CASE("cockpit: coluna lateral esquerda full-height", "[battle_layout]") {
    const Rect cp = cockpit_rect();
    REQUIRE_THAT(cp.x, WithinAbs(0.0f, 1e-4f));
    REQUIRE_THAT(cp.h, WithinAbs(static_cast<float>(kBattleLogicalH), 1e-4f));
    REQUIRE_THAT(cp.w, WithinAbs(static_cast<float>(kCockpitW), 1e-4f));
    // O retrato, a barra de HP e a zona do menu ficam DENTRO do cockpit, sem vazar.
    const Rect pr = cockpit_portrait_rect();
    const Rect hp = cockpit_hp_bar_rect();
    const Rect mz = cockpit_menu_zone();
    for (const Rect& r : {pr, hp, mz}) {
        REQUIRE(r.x >= cp.x);
        REQUIRE(r.x + r.w <= cp.x + cp.w + 1e-3f);
        REQUIRE(r.y >= cp.y);
        REQUIRE(r.y + r.h <= cp.y + cp.h + 1e-3f);
    }
    // Ordem vertical: retrato acima do HP acima do menu.
    REQUIRE(pr.y < hp.y);
    REQUIRE(hp.y < mz.y);
}

TEST_CASE("arena: a DIREITA do cockpit; party a esq-da-arena, inimigos a dir (D2)",
          "[battle_layout]") {
    const ArenaLayout a = arena_layout(3, 4, /*gus_party_index=*/-1);
    REQUIRE(a.party_count == 3);
    REQUIRE(a.enemy_count == 4);
    // Todo slot de party fica a ESQUERDA de todo inimigo, e TODOS a direita do cockpit.
    for (int p = 0; p < a.party_count; ++p) {
        REQUIRE(a.party[p].rect.x >= static_cast<float>(kRightZoneX) - 1e-3f);
        for (int e = 0; e < a.enemy_count; ++e) {
            REQUIRE(a.party[p].rect.x < a.enemies[e].rect.x);
        }
    }
    // Os inimigos compartilham a mesma coluna X (mesmo x).
    for (int e = 1; e < a.enemy_count; ++e) {
        REQUIRE_THAT(a.enemies[e].rect.x, WithinAbs(a.enemies[0].rect.x, 1e-4f));
    }
}

TEST_CASE("arena: atores DISTRIBUIDOS (space-around) sem sobreposicao vertical",
          "[battle_layout]") {
    for (int n = 1; n <= kMaxEnemySlots; ++n) {
        const ArenaLayout a = arena_layout(3, n, -1);
        REQUIRE(a.enemy_count == n);
        // Nenhum par de inimigos se sobrepoe verticalmente (cada um na sua faixa).
        for (int i = 0; i < n; ++i) {
            REQUIRE(a.enemies[i].occupied);
            // Contido na banda da arena.
            REQUIRE(a.enemies[i].rect.y >= static_cast<float>(kArenaTop) - 1e-3f);
            REQUIRE(a.enemies[i].rect.y + a.enemies[i].rect.h <=
                    static_cast<float>(kArenaBottom) + 1e-3f);
            for (int j = i + 1; j < n; ++j) {
                REQUIRE_FALSE(overlap_v(a.enemies[i].rect, a.enemies[j].rect));
            }
        }
        for (int i = n; i < kMaxEnemySlots; ++i) {
            REQUIRE_FALSE(a.enemies[i].occupied);
        }
    }
}

TEST_CASE("arena: 1 ator fica centralizado na banda (space-around)", "[battle_layout]") {
    const ArenaLayout a1 = arena_layout(1, 1, -1);
    const float band_center = (kArenaTop + kArenaBottom) * 0.5f;
    REQUIRE_THAT(cy(a1.enemies[0].rect), WithinAbs(band_center, 0.6f));
    REQUIRE_THAT(cy(a1.party[0].rect), WithinAbs(band_center, 0.6f));
}

TEST_CASE("arena: o Gus recua em X (D3), os demais nao", "[battle_layout]") {
    const ArenaLayout a = arena_layout(3, 2, /*gus_party_index=*/0);
    // Slot 0 (Gus) recuado kGusRecuoX pra dentro (mais a direita que a coluna base).
    REQUIRE_THAT(a.party[0].rect.x,
                 WithinAbs(a.party[1].rect.x + kGusRecuoX, 1e-4f));
    REQUIRE_THAT(a.party[1].rect.x, WithinAbs(a.party[2].rect.x, 1e-4f));
    // O Gus continua a esquerda dos inimigos mesmo recuado.
    REQUIRE(a.party[0].rect.x + a.party[0].rect.w < a.enemies[0].rect.x);
}

TEST_CASE("arena: contagens saturam nos maximos", "[battle_layout]") {
    const ArenaLayout a = arena_layout(99, 99, -1);
    REQUIRE(a.party_count == kMaxPartySlots);
    REQUIRE(a.enemy_count == kMaxEnemySlots);
}

TEST_CASE("CTB: 5 celulas de 48px, primeira 'proximo', a direita do cockpit (D4)",
          "[battle_layout]") {
    const CtbStrip s = ctb_strip(7);
    for (int i = 0; i < kCtbVisibleCells; ++i) {
        REQUIRE(s.cells[i].occupied);
        REQUIRE_THAT(s.cells[i].rect.w, WithinAbs(static_cast<float>(kCtbPortraitPx),
                                                  1e-4f));
        REQUIRE(s.cells[i].rect.x >= static_cast<float>(kRightZoneX) - 1e-3f);
        if (i > 0) {
            REQUIRE(s.cells[i].rect.x > s.cells[i - 1].rect.x);
            REQUIRE_THAT(s.cells[i].rect.y, WithinAbs(s.cells[0].rect.y, 1e-4f));
        }
    }
    REQUIRE(s.cells[0].is_next);
    for (int i = 1; i < kCtbVisibleCells; ++i) {
        REQUIRE_FALSE(s.cells[i].is_next);
    }
}

TEST_CASE("CTB: fila maior que 5 marca overflow '+N' na 5a (D4)", "[battle_layout]") {
    const CtbStrip s = ctb_strip(7);
    REQUIRE(s.cells[kCtbVisibleCells - 1].is_overflow);
    REQUIRE(s.cells[kCtbVisibleCells - 1].overflow_count == 3);
    const CtbStrip s5 = ctb_strip(5);
    REQUIRE_FALSE(s5.cells[kCtbVisibleCells - 1].is_overflow);
}

TEST_CASE("CTB: fila curta deixa celulas vazias (sem 'proximo' se vazia)",
          "[battle_layout]") {
    const CtbStrip s = ctb_strip(2);
    REQUIRE(s.cells[0].occupied);
    REQUIRE(s.cells[1].occupied);
    REQUIRE_FALSE(s.cells[2].occupied);
    REQUIRE(s.cells[0].is_next);
    const CtbStrip s0 = ctb_strip(0);
    REQUIRE_FALSE(s0.cells[0].occupied);
    REQUIRE_FALSE(s0.cells[0].is_next);
}

TEST_CASE("zonas variante C: cockpit/arena/banner/log NAO se sobrepoem",
          "[battle_layout]") {
    const Rect cp = cockpit_rect();
    const Rect ar = arena_rect();
    const Rect bn = arena_banner_rect();
    const Rect lg = log_panel_rect();
    // Todas as zonas a direita ficam a DIREITA do cockpit (x >= cockpit.right).
    const float cock_right = cp.x + cp.w;
    for (const Rect& r : {ar, bn, lg}) {
        REQUIRE(r.x >= cock_right - 1e-3f);
        REQUIRE(r.x + r.w <= static_cast<float>(kBattleLogicalW) + 1e-3f);
        REQUIRE(r.y + r.h <= static_cast<float>(kBattleLogicalH) + 1e-3f);
    }
    // Verticalmente: banner ACIMA da arena ACIMA do log (faixas proprias, sem overlap).
    REQUIRE(bn.y + bn.h <= ar.y + 1e-3f);   // banner termina acima/no topo da arena
    REQUIRE(ar.y + ar.h <= lg.y + 1e-3f);   // arena termina acima do log
}

TEST_CASE("arena: NENHUM slot de ator invade o log (rodape)", "[battle_layout]") {
    const Rect lg = log_panel_rect();
    for (int p = 1; p <= kMaxPartySlots; ++p) {
        for (int e = 1; e <= kMaxEnemySlots; ++e) {
            const ArenaLayout a = arena_layout(p, e, 0);
            for (int i = 0; i < a.party_count; ++i) {
                REQUIRE(a.party[i].rect.y + a.party[i].rect.h <= lg.y);
                REQUIRE(a.party[i].rect.x >= kRightZoneX - 1e-3f);  // nao invade cockpit
            }
            for (int i = 0; i < a.enemy_count; ++i) {
                REQUIRE(a.enemies[i].rect.y + a.enemies[i].rect.h <= lg.y);
            }
        }
    }
}

TEST_CASE("layout e deterministico (mesma entrada -> mesma saida)", "[battle_layout]") {
    const ArenaLayout a = arena_layout(3, 4, 0);
    const ArenaLayout b = arena_layout(3, 4, 0);
    for (int i = 0; i < kMaxPartySlots; ++i) {
        REQUIRE_THAT(a.party[i].rect.x, WithinAbs(b.party[i].rect.x, 1e-6f));
        REQUIRE_THAT(a.party[i].rect.y, WithinAbs(b.party[i].rect.y, 1e-6f));
    }
}
