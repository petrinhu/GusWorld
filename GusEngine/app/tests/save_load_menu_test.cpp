// GusEngine/app/tests/save_load_menu_test.cpp
//
// Catch2 (TEST-FIRST) de save_load_menu.hpp (SAVE-LOAD-UI). Cobre: derivacao de
// capitulo/XP, formatacao de timestamp/playtime, preview de slot vazio/ocupado,
// selecionabilidade por modo, e navegacao/confirmacao de sobrescrita.

#include <catch2/catch_test_macros.hpp>

#include "gus/app/screens/save_load_menu.hpp"

using namespace gus::app::screens;
using gus::domain::save::kAutosaveSlot;
using gus::domain::save::kSlotCount;
using gus::domain::save::SaveData;

namespace {

SaveData make_save_data(int gus_xp, int main_story_stage_or_absent = -1) {
    SaveData data;
    data.timestamp_ms = 1783455240000LL;  // 07/07/2026 20:14 UTC (mesmo do mock)
    data.playtime_seconds = 2520.0 + 12.0;  // 42m12s -> "0h 42m"
    data.current_scene_path = "distritos_inferiores";
    data.character_states["gus"].xp = gus_xp;
    if (main_story_stage_or_absent >= 0) {
        data.quest_progress["main_story"] = main_story_stage_or_absent;
    }
    return data;
}

}  // namespace

// ---------------------------------------------------------------- chapter_from_quest_progress

TEST_CASE("chapter_from_quest_progress: quest_progress vazio devolve Cap. 1 (mesmo do mock, "
          "nenhuma quest real grava ainda)",
          "[save_load_menu]") {
    REQUIRE(chapter_from_quest_progress({}) == 1);
}

TEST_CASE("chapter_from_quest_progress: main_story=0 devolve Cap. 1", "[save_load_menu]") {
    REQUIRE(chapter_from_quest_progress({{"main_story", 0}}) == 1);
}

TEST_CASE("chapter_from_quest_progress: main_story=3 devolve Cap. 4", "[save_load_menu]") {
    REQUIRE(chapter_from_quest_progress({{"main_story", 3}}) == 4);
}

TEST_CASE("chapter_from_quest_progress: clampa no teto kChapterCount (6)", "[save_load_menu]") {
    REQUIRE(chapter_from_quest_progress({{"main_story", 99}}) == kChapterCount);
}

TEST_CASE("chapter_from_quest_progress: main_story negativo (defensivo) clampa em 1",
          "[save_load_menu]") {
    REQUIRE(chapter_from_quest_progress({{"main_story", -5}}) == 1);
}

// ---------------------------------------------------------------- save_xp_for_display

TEST_CASE("save_xp_for_display: le CharacterSaveState::xp do Gus", "[save_load_menu]") {
    REQUIRE(save_xp_for_display(make_save_data(340)) == 340);
}

TEST_CASE("save_xp_for_display: sem char 'gus' devolve 0 (defensivo)", "[save_load_menu]") {
    SaveData data;
    REQUIRE(save_xp_for_display(data) == 0);
}

// ---------------------------------------------------------------- format_timestamp_ms

TEST_CASE("format_timestamp_ms: epoch conhecido vira DD/MM/AAAA HH:MM (UTC)",
          "[save_load_menu]") {
    REQUIRE(format_timestamp_ms(1783455240000LL) == "07/07/2026 20:14");
}

TEST_CASE("format_timestamp_ms: negativo (defensivo) vira epoch 0", "[save_load_menu]") {
    REQUIRE(format_timestamp_ms(-1) == "01/01/1970 00:00");
}

// ---------------------------------------------------------------- format_playtime_seconds

TEST_CASE("format_playtime_seconds: 2532s vira '0h 42m' (mesmo do mock, slot 1)",
          "[save_load_menu]") {
    REQUIRE(format_playtime_seconds(2532.0) == "0h 42m");
}

TEST_CASE("format_playtime_seconds: 1h18m vira '1h 18m' (mesmo do mock, slot 2)",
          "[save_load_menu]") {
    REQUIRE(format_playtime_seconds(78.0 * 60.0) == "1h 18m");
}

TEST_CASE("format_playtime_seconds: negativo (defensivo) vira '0h 00m'", "[save_load_menu]") {
    REQUIRE(format_playtime_seconds(-5.0) == "0h 00m");
}

// ---------------------------------------------------------------- previews

TEST_CASE("empty_slot_preview: occupied=false, is_autosave correto", "[save_load_menu]") {
    const SaveSlotPreview manual = empty_slot_preview(1);
    REQUIRE_FALSE(manual.occupied);
    REQUIRE_FALSE(manual.is_autosave);
    REQUIRE(manual.slot_id == 1);

    const SaveSlotPreview autosave = empty_slot_preview(kAutosaveSlot);
    REQUIRE(autosave.is_autosave);
}

TEST_CASE("build_slot_preview: preenche xp/capitulo/timestamp/playtime/scene a partir do "
          "SaveData",
          "[save_load_menu]") {
    const SaveData data = make_save_data(340, /*main_story_stage=*/0);
    const SaveSlotPreview preview = build_slot_preview(data, 1);
    REQUIRE(preview.occupied);
    REQUIRE_FALSE(preview.is_autosave);
    REQUIRE(preview.xp == 340);
    REQUIRE(preview.chapter == 1);
    REQUIRE(preview.scene_path == "distritos_inferiores");
}

// ---------------------------------------------------------------- slot_selectable

TEST_CASE("slot_selectable: modo Save exclui o autosave (so-leitura), inclui manuais "
          "ocupados e vazios",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    slots[kAutosaveSlot] = build_slot_preview(make_save_data(550), kAutosaveSlot);
    slots[1] = build_slot_preview(make_save_data(340), 1);
    slots[2] = empty_slot_preview(2);
    save_load_menu_open(state, SaveLoadMode::Save, slots);

    REQUIRE_FALSE(slot_selectable(state, kAutosaveSlot));
    REQUIRE(slot_selectable(state, 1));
    REQUIRE(slot_selectable(state, 2));  // vazio ainda selecionavel em Save (grava aqui)
}

TEST_CASE("slot_selectable: modo Load exclui slots vazios, inclui autosave ocupado",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    slots[kAutosaveSlot] = build_slot_preview(make_save_data(550), kAutosaveSlot);
    slots[1] = build_slot_preview(make_save_data(340), 1);
    slots[2] = empty_slot_preview(2);
    save_load_menu_open(state, SaveLoadMode::Load, slots);

    REQUIRE(slot_selectable(state, kAutosaveSlot));
    REQUIRE(slot_selectable(state, 1));
    REQUIRE_FALSE(slot_selectable(state, 2));
}

TEST_CASE("slot_selectable: index fora do intervalo devolve false (defensivo)",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    REQUIRE_FALSE(slot_selectable(state, -1));
    REQUIRE_FALSE(slot_selectable(state, kSlotCount));
}

// ---------------------------------------------------------------- save_load_menu_open

TEST_CASE("save_load_menu_open: selecao inicial pula o autosave em modo Save",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    save_load_menu_open(state, SaveLoadMode::Save, slots);

    REQUIRE(state.selected != kAutosaveSlot);
    REQUIRE(slot_selectable(state, state.selected));
    REQUIRE_FALSE(state.confirming_overwrite);
}

TEST_CASE("save_load_menu_open: selecao inicial pula slots vazios em modo Load",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[3] = build_slot_preview(make_save_data(100), 3);
    save_load_menu_open(state, SaveLoadMode::Load, slots);

    REQUIRE(state.selected == 3);
}

// ---------------------------------------------------------------- navegacao

TEST_CASE("save_load_menu_key_down: Baixo/Cima navegam so entre slots selecionaveis "
          "(pula o autosave em Save)",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    REQUIRE(state.selected == 1);

    REQUIRE(save_load_menu_key_down(state, SDLK_DOWN) == SaveLoadMenuAction::None);
    REQUIRE(state.selected == 2);

    REQUIRE(save_load_menu_key_down(state, SDLK_UP) == SaveLoadMenuAction::None);
    REQUIRE(state.selected == 1);

    // Subir a partir do primeiro manual dá wrap pro ULTIMO manual (pula o autosave).
    REQUIRE(save_load_menu_key_down(state, SDLK_UP) == SaveLoadMenuAction::None);
    REQUIRE(state.selected == kSlotCount - 1);
}

TEST_CASE("save_load_menu_key_down: Enter num slot VAZIO em modo Save confirma DIRETO "
          "(sem mini-dialogo)",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    save_load_menu_open(state, SaveLoadMode::Save, slots);

    REQUIRE(save_load_menu_key_down(state, SDLK_RETURN) == SaveLoadMenuAction::SlotChosen);
    REQUIRE_FALSE(state.confirming_overwrite);
}

TEST_CASE("save_load_menu_key_down: Enter num slot OCUPADO em modo Save abre o "
          "mini-dialogo de sobrescrita",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = build_slot_preview(make_save_data(100), 1);
    save_load_menu_open(state, SaveLoadMode::Save, slots);

    REQUIRE(save_load_menu_key_down(state, SDLK_RETURN) == SaveLoadMenuAction::None);
    REQUIRE(state.confirming_overwrite);
    REQUIRE(state.confirm_selected == 1);  // default seguro = Nao
}

TEST_CASE("save_load_menu_key_down: mini-dialogo - Enter com Nao devolve "
          "OverwriteCancelled e fecha o dialogo",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = build_slot_preview(make_save_data(100), 1);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    save_load_menu_key_down(state, SDLK_RETURN);  // abre o dialogo

    REQUIRE(save_load_menu_key_down(state, SDLK_RETURN) ==
            SaveLoadMenuAction::OverwriteCancelled);
    REQUIRE_FALSE(state.confirming_overwrite);
}

TEST_CASE("save_load_menu_key_down: mini-dialogo - alternar pra Sim e confirmar devolve "
          "OverwriteConfirmed",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = build_slot_preview(make_save_data(100), 1);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    save_load_menu_key_down(state, SDLK_RETURN);  // abre o dialogo

    REQUIRE(save_load_menu_key_down(state, SDLK_LEFT) == SaveLoadMenuAction::None);
    REQUIRE(state.confirm_selected == 0);
    REQUIRE(save_load_menu_key_down(state, SDLK_RETURN) ==
            SaveLoadMenuAction::OverwriteConfirmed);
    REQUIRE_FALSE(state.confirming_overwrite);
}

TEST_CASE("save_load_menu_key_down: mini-dialogo - Esc equivale a Nao (seguranca)",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = build_slot_preview(make_save_data(100), 1);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    save_load_menu_key_down(state, SDLK_RETURN);  // abre o dialogo

    REQUIRE(save_load_menu_key_down(state, SDLK_ESCAPE) ==
            SaveLoadMenuAction::OverwriteCancelled);
    REQUIRE_FALSE(state.confirming_overwrite);
}

TEST_CASE("save_load_menu_key_down: Enter num slot ocupado em modo Load confirma DIRETO",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = build_slot_preview(make_save_data(100), 1);
    save_load_menu_open(state, SaveLoadMode::Load, slots);

    REQUIRE(save_load_menu_key_down(state, SDLK_RETURN) == SaveLoadMenuAction::SlotChosen);
    REQUIRE_FALSE(state.confirming_overwrite);
}

TEST_CASE("save_load_menu_key_down: Esc fora do mini-dialogo devolve Back",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    save_load_menu_open(state, SaveLoadMode::Save, slots);

    REQUIRE(save_load_menu_key_down(state, SDLK_ESCAPE) == SaveLoadMenuAction::Back);
}
