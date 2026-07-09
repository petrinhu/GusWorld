// save_slots_test.cpp
//
// Spec executavel (Catch2 v3) da POLITICA DE SLOTS do save (domain/save). Camada
// PURA, ZERO Qt, ZERO disco: opera sobre IDs e NOMES LOGICOS de slot.
//
// REQUISITO DO LIDER (ADR-006 + 2026-06-21, bump SAVE-LOAD-UI etapa 6): 1 slot de
// AUTO-SAVE + 6 slots de save MANUAL = 7 no total (bump aditivo 5->6 manuais, ver
// save_slots.hpp). O auto-save sobrescreve o proprio slot; os 6 manuais o
// jogador escolhe. O C# de referencia (game/scripts/.../SaveManager.cs) usava
// 1 autosave (slot 0) + 4 manuais (1..4); aqui ADAPTAMOS para 1 + 6 (1..6).
//
// O mapeamento slot -> nome logico ("autosave", "save_1".."save_6") e a fronteira
// estavel que a camada de I/O (platform/, futura) traduz para caminho de arquivo.
// NENHUM caminho de disco aparece aqui: nome logico, nao path.
//
// Cross-ref: game/scripts/foundation/save_system/SaveManager.cs (referencia,
//            adaptada de 1+4 para 1+6), ADR-006.

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

#include "gus/domain/save/save_slots.hpp"

using gus::domain::save::is_autosave;
using gus::domain::save::is_valid_slot;
using gus::domain::save::kAutosaveSlot;
using gus::domain::save::kManualSlotCount;
using gus::domain::save::kSlotCount;
using gus::domain::save::slot_logical_name;

// ---- contagem de slots: 1 auto + 6 manuais = 7 ----------------------------

TEST_CASE("slots: politica 1 autosave + 6 manuais (7 no total)",
          "[domain][save][slots]") {
    REQUIRE(kManualSlotCount == 6);
    REQUIRE(kSlotCount == 7);
    REQUIRE(kAutosaveSlot == 0);
}

// ---- validade de slot ------------------------------------------------------

TEST_CASE("slots: slots 0..6 sao validos, fora da faixa nao",
          "[domain][save][slots]") {
    REQUIRE(is_valid_slot(0));
    REQUIRE(is_valid_slot(1));
    REQUIRE(is_valid_slot(6));
    REQUIRE_FALSE(is_valid_slot(-1));
    REQUIRE_FALSE(is_valid_slot(7));
    REQUIRE_FALSE(is_valid_slot(99));
}

// ---- autosave vs manual ----------------------------------------------------

TEST_CASE("slots: so o slot 0 e o auto-save; 1..6 sao manuais",
          "[domain][save][slots]") {
    REQUIRE(is_autosave(0));
    REQUIRE_FALSE(is_autosave(1));
    REQUIRE_FALSE(is_autosave(6));
}

// ---- mapeamento slot -> nome logico ---------------------------------------

TEST_CASE("slots: nome logico do autosave e 'autosave'",
          "[domain][save][slots]") {
    REQUIRE(slot_logical_name(kAutosaveSlot) == "autosave");
}

TEST_CASE("slots: nomes logicos dos manuais sao save_1..save_6",
          "[domain][save][slots]") {
    REQUIRE(slot_logical_name(1) == "save_1");
    REQUIRE(slot_logical_name(2) == "save_2");
    REQUIRE(slot_logical_name(3) == "save_3");
    REQUIRE(slot_logical_name(4) == "save_4");
    REQUIRE(slot_logical_name(5) == "save_5");
    REQUIRE(slot_logical_name(6) == "save_6");
}

TEST_CASE("slots: nome logico de slot invalido lanca (fail-fast)",
          "[domain][save][slots]") {
    REQUIRE_THROWS_AS(slot_logical_name(-1), std::out_of_range);
    REQUIRE_THROWS_AS(slot_logical_name(7), std::out_of_range);
}

TEST_CASE("slots: nomes logicos sao todos distintos (7 nomes unicos)",
          "[domain][save][slots]") {
    for (int a = 0; a < kSlotCount; ++a) {
        for (int b = a + 1; b < kSlotCount; ++b) {
            REQUIRE(slot_logical_name(a) != slot_logical_name(b));
        }
    }
}
