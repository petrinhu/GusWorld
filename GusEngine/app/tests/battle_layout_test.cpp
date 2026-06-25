// GusEngine/app/tests/battle_layout_test.cpp
//
// Catch2 (headless) do LAYOUT PURO da BattleScreen (M5). Prova, SEM janela nem SDL, as
// decisoes de layout FECHADAS (battle-screen.md par.5):
//   D1  tela logica 640x360;
//   D2/D3 arena: coluna unica de cada lado, espacamento FIXO, party esquerda / inimigos
//         direita, CENTRALIZADOS no eixo vertical, Gus RECUADO em X;
//   D4  fila CTB de 5 celulas de 48px com "proximo" na 1a e overflow "+N" na 5a;
//   zonas de HUD (painel do ator, log) dentro da tela e sem sobrepor a arena.
// Tudo funcao PURA: mesma contagem -> mesmos retangulos (determinismo).

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/app/screens/battle_layout.hpp"

using Catch::Matchers::WithinAbs;
using gus::app::screens::active_panel_rect;
using gus::app::screens::ArenaLayout;
using gus::app::screens::arena_layout;
using gus::app::screens::arena_slot_height;
using gus::app::screens::battle_screen_rect;
using gus::app::screens::ctb_strip;
using gus::app::screens::CtbStrip;
using gus::app::screens::kActivePanelTop;
using gus::app::screens::kActorSlotGapY;
using gus::app::screens::kActorSlotH;
using gus::app::screens::kActorSlotMinH;
using gus::app::screens::kArenaBottom;
using gus::app::screens::kArenaTop;
using gus::app::screens::kBattleLogicalH;
using gus::app::screens::kBattleLogicalW;
using gus::app::screens::kCtbPortraitPx;
using gus::app::screens::kCtbVisibleCells;
using gus::app::screens::kEnemyColumnX;
using gus::app::screens::kGusRecuoX;
using gus::app::screens::kMaxEnemySlots;
using gus::app::screens::kMaxPartySlots;
using gus::app::screens::kPartyColumnX;
using gus::app::screens::log_panel_rect;
using gus::core::spatial::Rect;

namespace {

// Centro vertical de um Rect.
float cy(const Rect& r) { return r.y + r.h * 0.5f; }

}  // namespace

TEST_CASE("battle_screen_rect e a tela logica 640x360 na origem", "[battle_layout]") {
    const Rect s = battle_screen_rect();
    REQUIRE_THAT(s.x, WithinAbs(0.0f, 1e-4f));
    REQUIRE_THAT(s.y, WithinAbs(0.0f, 1e-4f));
    REQUIRE_THAT(s.w, WithinAbs(static_cast<float>(kBattleLogicalW), 1e-4f));
    REQUIRE_THAT(s.h, WithinAbs(static_cast<float>(kBattleLogicalH), 1e-4f));
}

TEST_CASE("arena: party na esquerda, inimigos na direita (D2)", "[battle_layout]") {
    const ArenaLayout a = arena_layout(3, 4, /*gus_party_index=*/-1);
    REQUIRE(a.party_count == 3);
    REQUIRE(a.enemy_count == 4);
    // Todo slot de party fica a ESQUERDA de todo slot de inimigo.
    for (int p = 0; p < a.party_count; ++p) {
        REQUIRE(a.party[p].occupied);
        for (int e = 0; e < a.enemy_count; ++e) {
            REQUIRE(a.enemies[e].occupied);
            REQUIRE(a.party[p].rect.x < a.enemies[e].rect.x);
        }
    }
    // Coluna fixa (X base), sem Gus recuado neste caso.
    REQUIRE_THAT(a.party[1].rect.x, WithinAbs(static_cast<float>(kPartyColumnX), 1e-4f));
    REQUIRE_THAT(a.enemies[0].rect.x, WithinAbs(static_cast<float>(kEnemyColumnX), 1e-4f));
}

TEST_CASE("arena: espacamento vertical CONSISTENTE entre slots empilhados (D2)",
          "[battle_layout]") {
    const ArenaLayout a = arena_layout(3, 4, -1);
    // Passo entre slots consecutivos da party e constante (= altura adaptativa + gap).
    const float step01 = a.party[1].rect.y - a.party[0].rect.y;
    const float step12 = a.party[2].rect.y - a.party[1].rect.y;
    REQUIRE_THAT(step12, WithinAbs(step01, 1e-4f));
    // O passo = altura do slot + gap (a altura e adaptativa, mas o passo a respeita).
    REQUIRE_THAT(step01, WithinAbs(a.party[0].rect.h + kActorSlotGapY, 1e-4f));
    REQUIRE(step01 > a.party[0].rect.h);  // gap positivo
    // Inimigos: mesmo passo entre os 4.
    REQUIRE_THAT(a.enemies[1].rect.y - a.enemies[0].rect.y,
                 WithinAbs(a.enemies[3].rect.y - a.enemies[2].rect.y, 1e-4f));
}

TEST_CASE("arena: colunas CENTRALIZADAS na banda vertical (D3)", "[battle_layout]") {
    // 1 inimigo so: fica centralizado na banda [kArenaTop, kArenaBottom].
    const ArenaLayout a1 = arena_layout(1, 1, -1);
    const float band_center = (kArenaTop + kArenaBottom) * 0.5f;
    REQUIRE_THAT(cy(a1.enemies[0].rect), WithinAbs(band_center, 0.6f));
    REQUIRE_THAT(cy(a1.party[0].rect), WithinAbs(band_center, 0.6f));

    // 2 inimigos: o centro do bloco (entre os 2) tambem cai no centro da banda.
    const ArenaLayout a2 = arena_layout(2, 2, -1);
    const float mid = (cy(a2.enemies[0].rect) + cy(a2.enemies[1].rect)) * 0.5f;
    REQUIRE_THAT(mid, WithinAbs(band_center, 0.6f));
}

TEST_CASE("arena: contagens 1..4 cabem na banda (altura adaptativa, sem transbordar)",
          "[battle_layout]") {
    // FIX 2026-06-25: com muitos atores a altura do slot ENCOLHE pra a coluna caber na
    // banda [kArenaTop, kArenaBottom] - melhor que transbordar pra dentro do painel.
    for (int n = 1; n <= kMaxEnemySlots; ++n) {
        const ArenaLayout a = arena_layout(3, n, -1);
        REQUIRE(a.enemy_count == n);
        const int expect_h = arena_slot_height(n);
        for (int i = 0; i < n; ++i) {
            REQUIRE(a.enemies[i].occupied);
            // Altura adaptativa, igual pra todos do lado, dentro de [min, teto].
            REQUIRE_THAT(a.enemies[i].rect.h, WithinAbs(static_cast<float>(expect_h),
                                                        1e-4f));
            REQUIRE(a.enemies[i].rect.h <= static_cast<float>(kActorSlotH));
            REQUIRE(a.enemies[i].rect.h >= static_cast<float>(kActorSlotMinH));
            // CONTIDO na banda: nenhum slot ultrapassa kArenaBottom (-> nem o painel).
            REQUIRE(a.enemies[i].rect.y >= static_cast<float>(kArenaTop) - 1e-4f);
            REQUIRE(a.enemies[i].rect.y + a.enemies[i].rect.h <=
                    static_cast<float>(kArenaBottom) + 1e-4f);
        }
        for (int i = n; i < kMaxEnemySlots; ++i) {
            REQUIRE_FALSE(a.enemies[i].occupied);
        }
    }
}

TEST_CASE("arena: NENHUM slot (party+inimigo, 1..4) invade o painel (kActivePanelTop)",
          "[battle_layout]") {
    // O teste-chave do FIX (lider pegou os inimigos transbordando pro menu): pra TODA
    // combinacao de contagens, todo slot de ator termina ACIMA do painel.
    for (int p = 1; p <= kMaxPartySlots; ++p) {
        for (int e = 1; e <= kMaxEnemySlots; ++e) {
            const ArenaLayout a = arena_layout(p, e, 0);
            for (int i = 0; i < a.party_count; ++i) {
                REQUIRE(a.party[i].rect.y + a.party[i].rect.h <=
                        static_cast<float>(kActivePanelTop));
            }
            for (int i = 0; i < a.enemy_count; ++i) {
                REQUIRE(a.enemies[i].rect.y + a.enemies[i].rect.h <=
                        static_cast<float>(kActivePanelTop));
            }
        }
    }
}

TEST_CASE("arena: o Gus recua em X (D3), os demais nao", "[battle_layout]") {
    const ArenaLayout a = arena_layout(3, 2, /*gus_party_index=*/0);
    // Slot 0 (Gus) recuado pra dentro (mais a direita que a coluna base).
    REQUIRE_THAT(a.party[0].rect.x,
                 WithinAbs(static_cast<float>(kPartyColumnX + kGusRecuoX), 1e-4f));
    // Slots 1 e 2 na coluna base.
    REQUIRE_THAT(a.party[1].rect.x, WithinAbs(static_cast<float>(kPartyColumnX), 1e-4f));
    REQUIRE_THAT(a.party[2].rect.x, WithinAbs(static_cast<float>(kPartyColumnX), 1e-4f));
    // O Gus continua a esquerda dos inimigos mesmo recuado.
    REQUIRE(a.party[0].rect.x + a.party[0].rect.w < a.enemies[0].rect.x);
}

TEST_CASE("arena: contagens saturam nos maximos", "[battle_layout]") {
    const ArenaLayout a = arena_layout(99, 99, -1);
    REQUIRE(a.party_count == kMaxPartySlots);
    REQUIRE(a.enemy_count == kMaxEnemySlots);
}

TEST_CASE("CTB: 5 celulas de 48px, primeira marcada 'proximo' (D4)", "[battle_layout]") {
    const CtbStrip s = ctb_strip(7);  // fila com 7 atores
    // Todas as 5 celulas ocupadas; 48px; alinhadas no eixo Y; crescentes em X.
    for (int i = 0; i < kCtbVisibleCells; ++i) {
        REQUIRE(s.cells[i].occupied);
        REQUIRE_THAT(s.cells[i].rect.w, WithinAbs(static_cast<float>(kCtbPortraitPx),
                                                  1e-4f));
        REQUIRE_THAT(s.cells[i].rect.h, WithinAbs(static_cast<float>(kCtbPortraitPx),
                                                  1e-4f));
        if (i > 0) {
            REQUIRE(s.cells[i].rect.x > s.cells[i - 1].rect.x);
            REQUIRE_THAT(s.cells[i].rect.y, WithinAbs(s.cells[0].rect.y, 1e-4f));
        }
    }
    // "proximo" so na 1a; as demais nao.
    REQUIRE(s.cells[0].is_next);
    for (int i = 1; i < kCtbVisibleCells; ++i) {
        REQUIRE_FALSE(s.cells[i].is_next);
    }
}

TEST_CASE("CTB: fila maior que 5 marca overflow '+N' na 5a (D4)", "[battle_layout]") {
    const CtbStrip s = ctb_strip(7);
    REQUIRE(s.cells[kCtbVisibleCells - 1].is_overflow);
    // "+N": atores alem das 4 primeiras casas visiveis (7 - 4 = 3).
    REQUIRE(s.cells[kCtbVisibleCells - 1].overflow_count == 3);
    // Sem overflow quando a fila cabe em 5.
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

    const CtbStrip s0 = ctb_strip(0);  // fila vazia: nenhuma celula, nenhum "proximo"
    REQUIRE_FALSE(s0.cells[0].occupied);
    REQUIRE_FALSE(s0.cells[0].is_next);
}

TEST_CASE("HUD: painel do ator e log dentro da tela e abaixo da arena",
          "[battle_layout]") {
    const Rect panel = active_panel_rect();
    const Rect log = log_panel_rect();
    // Dentro da tela 640x360.
    REQUIRE(panel.x >= 0.0f);
    REQUIRE(panel.y + panel.h <= static_cast<float>(kBattleLogicalH));
    REQUIRE(log.x >= 0.0f);
    REQUIRE(log.y + log.h <= static_cast<float>(kBattleLogicalH));
    // Ambos comecam ABAIXO da banda da arena (nao invadem os atores).
    REQUIRE(panel.y >= static_cast<float>(kArenaBottom));
    // Log fica ABAIXO do painel (base da tela).
    REQUIRE(log.y >= panel.y + panel.h);
    // Largura util (margens dos dois lados, nao tela cheia).
    REQUIRE(panel.w < static_cast<float>(kBattleLogicalW));
    REQUIRE(panel.w > 0.0f);
}

TEST_CASE("layout e deterministico (mesma entrada -> mesma saida)", "[battle_layout]") {
    const ArenaLayout a = arena_layout(3, 4, 0);
    const ArenaLayout b = arena_layout(3, 4, 0);
    for (int i = 0; i < kMaxPartySlots; ++i) {
        REQUIRE_THAT(a.party[i].rect.x, WithinAbs(b.party[i].rect.x, 1e-6f));
        REQUIRE_THAT(a.party[i].rect.y, WithinAbs(b.party[i].rect.y, 1e-6f));
    }
}
