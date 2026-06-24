// GusEngine/app/tests/battle_hud_model_test.cpp
//
// Catch2 (headless) do MODELO PURO do HUD da BattleScreen (M5, incremento 2). Prova,
// SEM janela nem SDL:
//   - status_icon_file/index cobre TODO StatusId do enum (Stun..Slow) com arquivo
//     existente em resources/sprites/icons-m5/status/;
//   - bar_fill clampa 0..1 (HP); bar_fill_rect cresce da esquerda;
//   - resource_pips: total/lit/saturacao no max (AP=3, Mana cap 8);
//   - arena_hp_bar_frame: mini-barra centrada sob o slot.
// Determinismo: mesma entrada -> mesma saida.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <string_view>

#include "gus/app/screens/battle_hud_model.hpp"
#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_enums.hpp"

using Catch::Matchers::WithinAbs;
using gus::app::screens::arena_hp_bar_frame;
using gus::app::screens::bar_fill;
using gus::app::screens::bar_fill_rect;
using gus::app::screens::kArenaHpBarGapY;
using gus::app::screens::kArenaHpBarH;
using gus::app::screens::kPipGap;
using gus::app::screens::kPipSize;
using gus::app::screens::kStatusIdCount;
using gus::app::screens::resource_pips;
using gus::app::screens::status_icon_file;
using gus::app::screens::status_icon_index;
using gus::core::spatial::Rect;
using gus::domain::combat::StatusId;

namespace {

// Todos os StatusId do enum (Stun=0 .. Slow=12), na ordem do valor.
constexpr StatusId kAllStatus[] = {
    StatusId::Stun,    StatusId::Poison,    StatusId::Corrode, StatusId::Disrupt,
    StatusId::Silence, StatusId::Knockback, StatusId::Break,   StatusId::Expose,
    StatusId::Decrypt, StatusId::Shield,    StatusId::Regen,   StatusId::Haste,
    StatusId::Slow,
};

}  // namespace

TEST_CASE("status_icon_file mapeia TODO StatusId pra um arquivo nao-vazio",
          "[battle_hud]") {
    static_assert(sizeof(kAllStatus) / sizeof(kAllStatus[0]) == kStatusIdCount,
                  "lista de teste deve cobrir todos os StatusId");
    for (StatusId id : kAllStatus) {
        const std::string_view f = status_icon_file(id);
        REQUIRE_FALSE(f.empty());
        // Convencao: status_<algo>.png.
        REQUIRE(f.substr(0, 7) == std::string_view("status_"));
        REQUIRE(f.substr(f.size() - 4) == std::string_view(".png"));
    }
    // Casos pontuais (sincronia com os assets do disco).
    REQUIRE(status_icon_file(StatusId::Poison) == std::string_view("status_poison.png"));
    REQUIRE(status_icon_file(StatusId::Shield) == std::string_view("status_shield.png"));
    REQUIRE(status_icon_file(StatusId::Slow) == std::string_view("status_slow.png"));
}

TEST_CASE("status_icon_index = valor do enum (estavel 0..12)", "[battle_hud]") {
    REQUIRE(status_icon_index(StatusId::Stun) == 0);
    REQUIRE(status_icon_index(StatusId::Slow) == kStatusIdCount - 1);
    // Indice distinto por status.
    for (int i = 0; i < kStatusIdCount; ++i) {
        REQUIRE(status_icon_index(kAllStatus[i]) == i);
    }
}

TEST_CASE("bar_fill clampa em [0,1] e divide certo (HP)", "[battle_hud]") {
    REQUIRE_THAT(bar_fill(55, 55), WithinAbs(1.0f, 1e-5f));   // cheio
    REQUIRE_THAT(bar_fill(0, 55), WithinAbs(0.0f, 1e-5f));    // morto
    REQUIRE_THAT(bar_fill(27, 54), WithinAbs(0.5f, 1e-5f));   // metade
    REQUIRE_THAT(bar_fill(99, 55), WithinAbs(1.0f, 1e-5f));   // overheal clampa
    REQUIRE_THAT(bar_fill(-5, 55), WithinAbs(0.0f, 1e-5f));   // negativo clampa
    REQUIRE_THAT(bar_fill(10, 0), WithinAbs(0.0f, 1e-5f));    // max 0 (sem div/0)
}

TEST_CASE("bar_fill_rect cresce da esquerda, mesma altura", "[battle_hud]") {
    const Rect frame{10.0f, 20.0f, 100.0f, 8.0f};
    const Rect half = bar_fill_rect(frame, 0.5f);
    REQUIRE_THAT(half.x, WithinAbs(10.0f, 1e-5f));
    REQUIRE_THAT(half.y, WithinAbs(20.0f, 1e-5f));
    REQUIRE_THAT(half.w, WithinAbs(50.0f, 1e-5f));
    REQUIRE_THAT(half.h, WithinAbs(8.0f, 1e-5f));
    // Clamp: fracao > 1 nao ultrapassa o frame.
    REQUIRE_THAT(bar_fill_rect(frame, 1.7f).w, WithinAbs(100.0f, 1e-5f));
    REQUIRE_THAT(bar_fill_rect(frame, -0.3f).w, WithinAbs(0.0f, 1e-5f));
}

TEST_CASE("resource_pips: AP de 3 pips, 2 acesos", "[battle_hud]") {
    const auto pips = resource_pips(40.0f, 60.0f, /*total=*/3, /*lit=*/2,
                                    /*max_pips=*/gus::domain::combat::combat_constants::kBaseApPerTurn);
    REQUIRE(pips.size() == 3);
    REQUIRE(pips[0].lit);
    REQUIRE(pips[1].lit);
    REQUIRE_FALSE(pips[2].lit);
    // Layout horizontal: passo = kPipSize + kPipGap; mesma linha (y).
    REQUIRE_THAT(pips[0].rect.x, WithinAbs(40.0f, 1e-5f));
    REQUIRE_THAT(pips[1].rect.x, WithinAbs(40.0f + kPipSize + kPipGap, 1e-5f));
    REQUIRE_THAT(pips[2].rect.y, WithinAbs(60.0f, 1e-5f));
    REQUIRE_THAT(pips[0].rect.w, WithinAbs(static_cast<float>(kPipSize), 1e-5f));
}

TEST_CASE("resource_pips: Mana satura no cap 8", "[battle_hud]") {
    const int cap = gus::domain::combat::combat_constants::kManaCap;  // 8
    // total absurdo (99) satura no cap; lit clampa em total.
    const auto pips = resource_pips(0.0f, 0.0f, /*total=*/99, /*lit=*/5, /*max_pips=*/cap);
    REQUIRE(static_cast<int>(pips.size()) == cap);
    int on = 0;
    for (const auto& p : pips) {
        if (p.lit) ++on;
    }
    REQUIRE(on == 5);
    // lit > total clampa em total.
    const auto full = resource_pips(0.0f, 0.0f, /*total=*/4, /*lit=*/9, /*max_pips=*/cap);
    REQUIRE(full.size() == 4);
    for (const auto& p : full) {
        REQUIRE(p.lit);
    }
}

TEST_CASE("resource_pips: total 0 nao gera pip", "[battle_hud]") {
    const auto pips = resource_pips(0.0f, 0.0f, 0, 0, 8);
    REQUIRE(pips.empty());
}

TEST_CASE("arena_hp_bar_frame: centrada sob o slot, logo abaixo da base",
          "[battle_hud]") {
    const Rect slot{40.0f, 100.0f, 56.0f, 64.0f};
    const Rect bar = arena_hp_bar_frame(slot);
    // Mesma largura e X do slot (centrada sob ele).
    REQUIRE_THAT(bar.x, WithinAbs(slot.x, 1e-5f));
    REQUIRE_THAT(bar.w, WithinAbs(slot.w, 1e-5f));
    // Logo abaixo da base do sprite (base + gap).
    REQUIRE_THAT(bar.y, WithinAbs(slot.y + slot.h + kArenaHpBarGapY, 1e-5f));
    REQUIRE_THAT(bar.h, WithinAbs(static_cast<float>(kArenaHpBarH), 1e-5f));
}
