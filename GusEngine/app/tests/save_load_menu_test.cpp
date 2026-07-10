// GusEngine/app/tests/save_load_menu_test.cpp
//
// Catch2 (TEST-FIRST) de save_load_menu.hpp (SAVE-LOAD-UI). Cobre: derivacao de
// capitulo/XP, formatacao de timestamp/playtime, preview de slot vazio/ocupado,
// selecionabilidade por modo, e navegacao/confirmacao de sobrescrita.
//
// CRIT-1 (auditoria AUD-SAVE-LOAD-UI-2026-07-09): a secao dedicada mais abaixo
// toca DISCO de proposito (gus/platform/fs/save_file_store.hpp, mesmo estilo de
// platform/tests/save_file_store_test.cpp) - reproduz FIELMENTE o repro do
// dossie (grava, corrompe 1 byte do primario, monta o preview EXATAMENTE como
// build_previews_and_cache faria) pra provar que a confirmacao de sobrescrita
// nao pula mais nesse cenario.

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

#include "gus/app/screens/save_load_menu.hpp"
#include "gus/platform/fs/save_file_store.hpp"

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
//
// HISTORIA MANDA: main_story PRESENTE ignora xp por completo (passamos xp=999999
// em alguns casos abaixo de proposito, pra provar que NAO influencia o resultado).

TEST_CASE("chapter_from_quest_progress: quest_progress vazio + xp=0 devolve Cap. 1 (mesmo "
          "do mock, nenhuma quest real grava ainda e nenhum char state tem xp)",
          "[save_load_menu]") {
    REQUIRE(chapter_from_quest_progress({}, /*xp=*/0) == 1);
}

TEST_CASE("chapter_from_quest_progress: main_story=0 devolve Cap. 1 (xp ignorado)",
          "[save_load_menu]") {
    REQUIRE(chapter_from_quest_progress({{"main_story", 0}}, /*xp=*/999999) == 1);
}

TEST_CASE("chapter_from_quest_progress: main_story=3 devolve Cap. 4 (xp ignorado)",
          "[save_load_menu]") {
    REQUIRE(chapter_from_quest_progress({{"main_story", 3}}, /*xp=*/0) == 4);
}

TEST_CASE("chapter_from_quest_progress: clampa no teto kChapterCount (6), xp ignorado",
          "[save_load_menu]") {
    REQUIRE(chapter_from_quest_progress({{"main_story", 99}}, /*xp=*/0) == kChapterCount);
}

TEST_CASE("chapter_from_quest_progress: main_story negativo (defensivo) clampa em 1",
          "[save_load_menu]") {
    REQUIRE(chapter_from_quest_progress({{"main_story", -5}}, /*xp=*/0) == 1);
}

// ---------------------------------------------------------------- chapter_from_xp_fallback
//
// PROVISORIO (kChapterXpThresholds = {100, 300, 600, 1000, 1500}, ver header) -
// so entra em jogo quando main_story esta AUSENTE (coberto acima via
// chapter_from_quest_progress({}, xp) - aqui testamos a funcao isolada).

TEST_CASE("chapter_from_xp_fallback: bandas monotonicamente crescentes",
          "[save_load_menu]") {
    REQUIRE(chapter_from_xp_fallback(0) == 1);
    REQUIRE(chapter_from_xp_fallback(99) == 1);
    REQUIRE(chapter_from_xp_fallback(100) == 2);
    REQUIRE(chapter_from_xp_fallback(299) == 2);
    REQUIRE(chapter_from_xp_fallback(300) == 3);
    REQUIRE(chapter_from_xp_fallback(599) == 3);
    REQUIRE(chapter_from_xp_fallback(600) == 4);
    REQUIRE(chapter_from_xp_fallback(999) == 4);
    REQUIRE(chapter_from_xp_fallback(1000) == 5);
    REQUIRE(chapter_from_xp_fallback(1499) == 5);
    REQUIRE(chapter_from_xp_fallback(1500) == 6);
}

TEST_CASE("chapter_from_xp_fallback: clampa no teto kChapterCount mesmo com xp gigante",
          "[save_load_menu]") {
    REQUIRE(chapter_from_xp_fallback(999999) == kChapterCount);
}

TEST_CASE("chapter_from_xp_fallback: xp negativo (defensivo) fica no piso 1",
          "[save_load_menu]") {
    REQUIRE(chapter_from_xp_fallback(-50) == 1);
}

TEST_CASE("chapter_from_quest_progress: main_story ausente cai no estimador de xp "
          "(forward-compat com build_slot_preview)",
          "[save_load_menu]") {
    REQUIRE(chapter_from_quest_progress({}, /*xp=*/340) == 3);
    REQUIRE(chapter_from_quest_progress({{"outra_quest", 5}}, /*xp=*/1500) == 6);
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

TEST_CASE("empty_slot_preview: occupied=false, present_unreadable=false, is_autosave "
          "correto",
          "[save_load_menu]") {
    const SaveSlotPreview manual = empty_slot_preview(1);
    REQUIRE_FALSE(manual.occupied);
    REQUIRE_FALSE(manual.present_unreadable);
    REQUIRE_FALSE(manual.is_autosave);
    REQUIRE(manual.slot_id == 1);

    const SaveSlotPreview autosave = empty_slot_preview(kAutosaveSlot);
    REQUIRE(autosave.is_autosave);
}

TEST_CASE("unreadable_slot_preview (CRIT-1): occupied=false (MESMO visual de "
          "vazio) mas present_unreadable=true",
          "[save_load_menu][CRIT-1]") {
    const SaveSlotPreview manual =
        unreadable_slot_preview(1, gus::domain::save::LoadResult::Corrupt);
    REQUIRE_FALSE(manual.occupied);
    REQUIRE(manual.present_unreadable);
    REQUIRE_FALSE(manual.is_autosave);
    REQUIRE(manual.slot_id == 1);

    const SaveSlotPreview autosave =
        unreadable_slot_preview(kAutosaveSlot, gus::domain::save::LoadResult::HmacInvalid);
    REQUIRE(autosave.is_autosave);
    REQUIRE(autosave.present_unreadable);
}

// ---------------------------------------------------------------- unreadable_slot_preview:
// motivo (SAVE-LOAD-AVISOS)

TEST_CASE("unreadable_slot_preview: LoadResult::VersionTooNew vira "
          "UnreadableReason::VersionTooNew (irrecuperavel, forward-only)",
          "[save_load_menu][save-load-avisos]") {
    const SaveSlotPreview preview =
        unreadable_slot_preview(1, gus::domain::save::LoadResult::VersionTooNew);
    REQUIRE(preview.unreadable_reason == UnreadableReason::VersionTooNew);
}

TEST_CASE("unreadable_slot_preview: qualquer OUTRO LoadResult != Ok vira "
          "UnreadableReason::Damaged (recuperavel via backup)",
          "[save_load_menu][save-load-avisos]") {
    using gus::domain::save::LoadResult;
    for (const LoadResult r :
         {LoadResult::HmacInvalid, LoadResult::Corrupt, LoadResult::Invalid,
          LoadResult::WrongSlot}) {
        REQUIRE(unreadable_slot_preview(1, r).unreadable_reason == UnreadableReason::Damaged);
    }
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

TEST_CASE("slot_selectable (SAVE-LOAD-AVISOS): modo Load TAMBEM inclui slots "
          "present_unreadable (danificado/versao) - so o GENUINAMENTE vazio "
          "fica fora",
          "[save_load_menu][save-load-avisos]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    slots[1] = unreadable_slot_preview(1, gus::domain::save::LoadResult::HmacInvalid);
    slots[2] = unreadable_slot_preview(2, gus::domain::save::LoadResult::VersionTooNew);
    slots[3] = empty_slot_preview(3);
    save_load_menu_open(state, SaveLoadMode::Load, slots);

    REQUIRE(slot_selectable(state, 1));
    REQUIRE(slot_selectable(state, 2));
    REQUIRE_FALSE(slot_selectable(state, 3));
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
    (void)save_load_menu_key_down(state, SDLK_RETURN);  // abre o dialogo

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
    (void)save_load_menu_key_down(state, SDLK_RETURN);  // abre o dialogo

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
    (void)save_load_menu_key_down(state, SDLK_RETURN);  // abre o dialogo

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

// ---------------------------------------------------------------- save_load_menu_click_slot
//
// Clique de mouse (fora de qualquer mini-dialogo): "focar + Enter" (MESMA
// convencao de system_menu_click_option).

TEST_CASE("save_load_menu_click_slot: clique num slot selecionavel foca e aplica a "
          "mesma regra do Enter (vazio em Save = SlotChosen direto)",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    REQUIRE(state.selected == 1);

    REQUIRE(save_load_menu_click_slot(state, 3) == SaveLoadMenuAction::SlotChosen);
    REQUIRE(state.selected == 3);  // o clique MOVEU o foco pro slot clicado
}

TEST_CASE("save_load_menu_click_slot: clique num slot manual OCUPADO em modo Save "
          "abre o mini-dialogo de sobrescrita (mesmo efeito do Enter)",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[3] = build_slot_preview(make_save_data(100), 3);
    save_load_menu_open(state, SaveLoadMode::Save, slots);

    REQUIRE(save_load_menu_click_slot(state, 3) == SaveLoadMenuAction::None);
    REQUIRE(state.confirming_overwrite);
    REQUIRE(state.selected == 3);
}

TEST_CASE("save_load_menu_click_slot: clique no autosave em modo Save (nao "
          "selecionavel, so-leitura) e no-op",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[kAutosaveSlot] = build_slot_preview(make_save_data(550), kAutosaveSlot);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    const int before = state.selected;

    REQUIRE(save_load_menu_click_slot(state, kAutosaveSlot) == SaveLoadMenuAction::None);
    REQUIRE(state.selected == before);  // foco NAO mudou (readonly nao clicavel)
}

TEST_CASE("save_load_menu_click_slot: index fora do intervalo e no-op (defensivo)",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    REQUIRE(save_load_menu_click_slot(state, -1) == SaveLoadMenuAction::None);
    REQUIRE(save_load_menu_click_slot(state, kSlotCount) == SaveLoadMenuAction::None);
}

TEST_CASE("save_load_menu_click_slot: clique na lista e no-op enquanto um "
          "mini-dialogo (sobrescrita) ja esta aberto",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = build_slot_preview(make_save_data(100), 1);
    slots[2] = build_slot_preview(make_save_data(200), 2);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    (void)save_load_menu_key_down(state, SDLK_RETURN);  // abre overwrite no slot 1
    REQUIRE(state.confirming_overwrite);

    REQUIRE(save_load_menu_click_slot(state, 2) == SaveLoadMenuAction::None);
    REQUIRE(state.selected == 1);  // clique na lista ignorado, foco NAO mudou pro 2
}

// ---------------------------------------------------------------- exclusao (feature "Apagar")

TEST_CASE("save_load_menu_request_delete: slot OCUPADO abre o mini-dialogo com "
          "delete_target_slot correto e default seguro = Nao",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[2] = build_slot_preview(make_save_data(100), 2);
    save_load_menu_open(state, SaveLoadMode::Save, slots);

    save_load_menu_request_delete(state, 2);
    REQUIRE(state.confirming_delete);
    REQUIRE(state.delete_confirm_selected == 1);
    REQUIRE(state.delete_target_slot == 2);
}

TEST_CASE("save_load_menu_request_delete: slot VAZIO e no-op (nada a apagar)",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    save_load_menu_open(state, SaveLoadMode::Save, slots);

    save_load_menu_request_delete(state, 2);
    REQUIRE_FALSE(state.confirming_delete);
}

TEST_CASE("save_load_menu_request_delete: autosave OCUPADO tambem abre o dialogo "
          "(decisao do lider: Auto tambem apagavel)",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[kAutosaveSlot] = build_slot_preview(make_save_data(550), kAutosaveSlot);
    save_load_menu_open(state, SaveLoadMode::Save, slots);

    save_load_menu_request_delete(state, kAutosaveSlot);
    REQUIRE(state.confirming_delete);
    REQUIRE(state.delete_target_slot == kAutosaveSlot);
}

TEST_CASE("save_load_menu_request_delete: index fora do intervalo e no-op (defensivo)",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    save_load_menu_open(state, SaveLoadMode::Save, slots);

    save_load_menu_request_delete(state, -1);
    REQUIRE_FALSE(state.confirming_delete);
    save_load_menu_request_delete(state, kSlotCount);
    REQUIRE_FALSE(state.confirming_delete);
}

TEST_CASE("save_load_menu_request_delete: no-op se o dialogo de sobrescrita ja "
          "estiver aberto (nunca 2 dialogos simultaneos)",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = build_slot_preview(make_save_data(100), 1);
    slots[2] = build_slot_preview(make_save_data(200), 2);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    (void)save_load_menu_key_down(state, SDLK_RETURN);  // abre overwrite no slot 1
    REQUIRE(state.confirming_overwrite);

    save_load_menu_request_delete(state, 2);
    REQUIRE_FALSE(state.confirming_delete);  // ignorado - overwrite ja aberto
}

TEST_CASE("save_load_menu_key_down: Delete sobre o slot focado (ocupado) abre o "
          "mini-dialogo de exclusao",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = build_slot_preview(make_save_data(100), 1);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    REQUIRE(state.selected == 1);

    REQUIRE(save_load_menu_key_down(state, SDLK_DELETE) == SaveLoadMenuAction::None);
    REQUIRE(state.confirming_delete);
    REQUIRE(state.delete_target_slot == 1);
}

TEST_CASE("save_load_menu_key_down: mini-dialogo de exclusao - Enter com Nao "
          "devolve DeleteCancelled e fecha o dialogo",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = build_slot_preview(make_save_data(100), 1);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    (void)save_load_menu_key_down(state, SDLK_DELETE);  // abre o dialogo

    REQUIRE(save_load_menu_key_down(state, SDLK_RETURN) == SaveLoadMenuAction::DeleteCancelled);
    REQUIRE_FALSE(state.confirming_delete);
}

TEST_CASE("save_load_menu_key_down: mini-dialogo de exclusao - alternar pra Sim e "
          "confirmar devolve DeleteConfirmed com o slot certo",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = build_slot_preview(make_save_data(100), 1);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    (void)save_load_menu_key_down(state, SDLK_DELETE);  // abre o dialogo

    REQUIRE(save_load_menu_key_down(state, SDLK_LEFT) == SaveLoadMenuAction::None);
    REQUIRE(state.delete_confirm_selected == 0);
    REQUIRE(save_load_menu_key_down(state, SDLK_RETURN) == SaveLoadMenuAction::DeleteConfirmed);
    REQUIRE_FALSE(state.confirming_delete);
    REQUIRE(state.delete_target_slot == 1);  // o CHAMADOR ainda le o alvo apos confirmar
}

TEST_CASE("save_load_menu_key_down: mini-dialogo de exclusao - Esc equivale a Nao "
          "(seguranca)",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = build_slot_preview(make_save_data(100), 1);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    (void)save_load_menu_key_down(state, SDLK_DELETE);  // abre o dialogo

    REQUIRE(save_load_menu_key_down(state, SDLK_ESCAPE) == SaveLoadMenuAction::DeleteCancelled);
    REQUIRE_FALSE(state.confirming_delete);
}

TEST_CASE("save_load_menu_key_down: Delete sobre slot VAZIO/nao-selecionavel e "
          "no-op (nao abre dialogo)",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    save_load_menu_open(state, SaveLoadMode::Save, slots);

    REQUIRE(save_load_menu_key_down(state, SDLK_DELETE) == SaveLoadMenuAction::None);
    REQUIRE_FALSE(state.confirming_delete);
}

// ---------------------------------------------------------------- click nas pills dos mini-dialogos

TEST_CASE("save_load_menu_click_overwrite_confirm: clique em 'Sim' (pill 0) "
          "devolve OverwriteConfirmed e fecha o dialogo",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = build_slot_preview(make_save_data(100), 1);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    (void)save_load_menu_key_down(state, SDLK_RETURN);  // abre o dialogo
    REQUIRE(state.confirming_overwrite);

    REQUIRE(save_load_menu_click_overwrite_confirm(state, 0) ==
            SaveLoadMenuAction::OverwriteConfirmed);
    REQUIRE_FALSE(state.confirming_overwrite);
}

TEST_CASE("save_load_menu_click_overwrite_confirm: clique em 'Nao' (pill 1) "
          "devolve OverwriteCancelled",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = build_slot_preview(make_save_data(100), 1);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    (void)save_load_menu_key_down(state, SDLK_RETURN);  // abre o dialogo

    REQUIRE(save_load_menu_click_overwrite_confirm(state, 1) ==
            SaveLoadMenuAction::OverwriteCancelled);
    REQUIRE_FALSE(state.confirming_overwrite);
}

TEST_CASE("save_load_menu_click_overwrite_confirm: no-op se o dialogo nao estiver "
          "aberto (defensivo)",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    REQUIRE(save_load_menu_click_overwrite_confirm(state, 0) == SaveLoadMenuAction::None);
}

TEST_CASE("save_load_menu_click_delete_confirm: clique em 'Sim' (pill 0) devolve "
          "DeleteConfirmed mantendo delete_target_slot; 'Nao' devolve "
          "DeleteCancelled",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[2] = build_slot_preview(make_save_data(100), 2);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    save_load_menu_request_delete(state, 2);
    REQUIRE(state.confirming_delete);

    REQUIRE(save_load_menu_click_delete_confirm(state, 0) ==
            SaveLoadMenuAction::DeleteConfirmed);
    REQUIRE_FALSE(state.confirming_delete);
    REQUIRE(state.delete_target_slot == 2);

    save_load_menu_request_delete(state, 2);
    REQUIRE(save_load_menu_click_delete_confirm(state, 1) ==
            SaveLoadMenuAction::DeleteCancelled);
}

TEST_CASE("save_load_menu_click_delete_confirm: no-op se o dialogo nao estiver "
          "aberto (defensivo)",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    REQUIRE(save_load_menu_click_delete_confirm(state, 0) == SaveLoadMenuAction::None);
}

// ---------------------------------------------------------------- save_load_menu_reselect_if_needed

TEST_CASE("save_load_menu_reselect_if_needed: slot selecionado ainda valido - no-op",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    const int before = state.selected;

    save_load_menu_reselect_if_needed(state);
    REQUIRE(state.selected == before);
}

TEST_CASE("save_load_menu_reselect_if_needed: slot selecionado ficou vazio (apos "
          "delete) em modo Load - move para o PROXIMO ocupado",
          "[save_load_menu]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = build_slot_preview(make_save_data(100), 1);
    slots[2] = build_slot_preview(make_save_data(200), 2);
    save_load_menu_open(state, SaveLoadMode::Load, slots);
    REQUIRE(state.selected == 1);

    // Simula o CHAMADOR ja tendo apagado o slot 1 em disco e atualizado o preview.
    state.slots[1] = empty_slot_preview(1);
    save_load_menu_reselect_if_needed(state);
    REQUIRE(state.selected == 2);
}

// ---------------------------------------------------------------- most_recent_occupied_slot
//
// SAVE-LOAD-UI etapa 4 (TELA DE TITULO): "Continuar" carrega o save mais recente
// entre TODOS os slots ocupados (Auto + manuais) por timestamp_ms - nao um slot
// escolhido a dedo.

TEST_CASE("most_recent_occupied_slot: nenhum slot ocupado devolve -1", "[save_load_menu]") {
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    REQUIRE(most_recent_occupied_slot(slots) == -1);
}

TEST_CASE("most_recent_occupied_slot: unico slot ocupado vence por definicao",
          "[save_load_menu]") {
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    SaveData data = make_save_data(100);
    data.timestamp_ms = 1000;
    slots[3] = build_slot_preview(data, 3);
    REQUIRE(most_recent_occupied_slot(slots) == 3);
}

TEST_CASE("most_recent_occupied_slot: escolhe o MAIOR timestamp_ms entre varios "
          "ocupados (inclui o autosave concorrendo)",
          "[save_load_menu]") {
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);

    SaveData auto_data = make_save_data(550);
    auto_data.timestamp_ms = 1783461600000LL;  // 08/07/2026 01:20 (mock)
    slots[kAutosaveSlot] = build_slot_preview(auto_data, kAutosaveSlot);

    SaveData slot1 = make_save_data(340);
    slot1.timestamp_ms = 1783455240000LL;  // 07/07/2026 20:14 (mock, mais antigo)
    slots[1] = build_slot_preview(slot1, 1);

    SaveData slot2 = make_save_data(810);
    slot2.timestamp_ms = 1783378260000LL;  // 06/07/2026 23:51 (mock, mais antigo ainda)
    slots[2] = build_slot_preview(slot2, 2);

    // O Auto e o MAIS RECENTE dos 3 - Continuar deve mirar nele.
    REQUIRE(most_recent_occupied_slot(slots) == kAutosaveSlot);
}

TEST_CASE("most_recent_occupied_slot: empate no timestamp - o PRIMEIRO indice "
          "(ordem crescente) vence (defensivo)",
          "[save_load_menu]") {
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    SaveData a = make_save_data(1);
    a.timestamp_ms = 5000;
    SaveData b = make_save_data(2);
    b.timestamp_ms = 5000;
    slots[2] = build_slot_preview(a, 2);
    slots[4] = build_slot_preview(b, 4);
    REQUIRE(most_recent_occupied_slot(slots) == 2);
}

// ---------------------------------------------------------------- CRIT-1 (auditoria
// AUD-SAVE-LOAD-UI-2026-07-09, docs/auditoria/AUDIT-SAVE-LOAD-UI-2026-07-09/
// auditoria_integridade_dados_save.md): repro END-TO-END, MESMO ESTILO/PASSOS do
// programa de verificacao efemero do dossie - grava 2x em disco (gera backup1),
// corrompe 1 byte do PRIMARIO, confirma via load_game que o LoadResult != Ok, monta o
// preview EXATAMENTE como build_previews_and_cache faria pra este caso
// (unreadable_slot_preview, NAO empty_slot_preview), abre a tela em modo Save e clica
// no slot (save_load_menu_click_slot, a MESMA funcao que o mouse real dispara em
// save_load_menu_loop.cpp) - PROVA que a confirmacao de sobrescrita agora abre (ANTES
// do fix: action==SlotChosen, gravava DIRETO sem aviso nenhum).

TEST_CASE("CRIT-1: slot com primario presente-mas-corrompido PEDE confirmacao de "
          "sobrescrita em modo Save (nao regrava direto por cima, protege a cadeia "
          "de backup)",
          "[save_load_menu][CRIT-1]") {
    const auto dir = std::filesystem::temp_directory_path() / "gusworld_crit1_overwrite_test";
    std::filesystem::remove_all(dir);

    gus::domain::save::SaveData first;
    first.current_scene_path = "city_intro";
    first.party_roster = {"gus"};
    first.party_active = {"gus"};
    first.slot_id = 1;
    REQUIRE(gus::platform::fs::save_game(first, 1, dir.string()));

    gus::domain::save::SaveData second = first;
    second.playtime_seconds = 5.0;
    REQUIRE(gus::platform::fs::save_game(second, 1, dir.string()));  // gera save_1.backup1.sav

    // Corrompe 1 byte do PRIMARIO (adulteracao/bit-rot - MESMO metodo de
    // platform/tests/save_file_store_test.cpp::"byte do envelope adulterado").
    {
        std::fstream f(dir / "save_1.sav", std::ios::in | std::ios::out | std::ios::binary);
        REQUIRE(f.is_open());
        char byte = 0;
        f.seekg(10);
        f.read(&byte, 1);
        byte = static_cast<char>(byte ^ 0xFF);
        f.seekp(10);
        f.write(&byte, 1);
    }

    REQUIRE(gus::platform::fs::has_save(1, dir.string()));  // arquivo AINDA existe
    const auto outcome = gus::platform::fs::load_game(1, dir.string());
    REQUIRE(outcome.has_value());  // presente != ausente
    REQUIRE(outcome->result != gus::domain::save::LoadResult::Ok);  // HmacInvalid

    // Monta o preview EXATAMENTE como build_previews_and_cache decidiria pra este
    // caso (has_save=true + load != Ok -> unreadable_slot_preview).
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = unreadable_slot_preview(1, outcome->result);
    REQUIRE_FALSE(slots[1].occupied);       // visual continua "Vazio" (nao muda, deliberado)
    REQUIRE(slots[1].present_unreadable);   // ... mas marcado ilegivel

    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Save, slots);

    // Clica no slot 1 - MESMA funcao (save_load_menu_click_slot) que o mouse real
    // dispara em save_load_menu_loop.cpp.
    const SaveLoadMenuAction action = save_load_menu_click_slot(state, 1);

    // ANTES do fix (reproduzido no dossie): action==SlotChosen (grava DIRETO, sem
    // dialogo). DEPOIS do fix: action==None + confirming_overwrite==true.
    REQUIRE(action == SaveLoadMenuAction::None);
    REQUIRE(state.confirming_overwrite);
    REQUIRE(state.confirm_selected == 1);  // default seguro = Nao

    std::filesystem::remove_all(dir);
}

TEST_CASE("CRIT-1: MESMO cenario via teclado (Enter, nao so clique de mouse) "
          "tambem pede confirmacao",
          "[save_load_menu][CRIT-1]") {
    const auto dir = std::filesystem::temp_directory_path() / "gusworld_crit1_keyboard_test";
    std::filesystem::remove_all(dir);

    gus::domain::save::SaveData data;
    data.current_scene_path = "city_intro";
    data.party_roster = {"gus"};
    data.party_active = {"gus"};
    data.slot_id = 1;
    REQUIRE(gus::platform::fs::save_game(data, 1, dir.string()));
    {
        std::fstream f(dir / "save_1.sav", std::ios::in | std::ios::out | std::ios::binary);
        REQUIRE(f.is_open());
        char byte = 0;
        f.seekg(10);
        f.read(&byte, 1);
        byte = static_cast<char>(byte ^ 0xFF);
        f.seekp(10);
        f.write(&byte, 1);
    }
    const auto outcome = gus::platform::fs::load_game(1, dir.string());
    REQUIRE(outcome->result != gus::domain::save::LoadResult::Ok);

    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[1] = unreadable_slot_preview(1, outcome->result);

    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Save, slots);
    REQUIRE(state.selected == 1);  // 1o slot selecionavel apos o autosave

    REQUIRE(save_load_menu_key_down(state, SDLK_RETURN) == SaveLoadMenuAction::None);
    REQUIRE(state.confirming_overwrite);

    std::filesystem::remove_all(dir);
}

// ---------------------------------------------------------------- SAVE-LOAD-AVISOS
// (aviso #1, mock Tela 4a): selecionar um slot present_unreadable EM MODO LOAD abre
// o aviso dedicado (Damaged com "Tentar recuperar"/Cancelar, Version so-Cancelar) em
// vez de fingir um SlotChosen.

namespace {

std::array<SaveSlotPreview, kSlotCount> slots_with_unreadable(
    int slot, gus::domain::save::LoadResult result) {
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[static_cast<std::size_t>(slot)] = unreadable_slot_preview(slot, result);
    return slots;
}

}  // namespace

TEST_CASE("save_load_menu_key_down: Enter num slot Danificado em modo Load abre "
          "o aviso Damaged (2 botoes, default seguro = Cancelar)",
          "[save_load_menu][save-load-avisos]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Load,
                         slots_with_unreadable(1, gus::domain::save::LoadResult::HmacInvalid));
    REQUIRE(state.selected == 1);

    REQUIRE(save_load_menu_key_down(state, SDLK_RETURN) == SaveLoadMenuAction::None);
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::Damaged);
    REQUIRE(state.warning_selected == 1);  // default seguro = Cancelar
}

TEST_CASE("save_load_menu_key_down: Enter num slot de Versao incompativel em "
          "modo Load abre o aviso Version (so Cancelar)",
          "[save_load_menu][save-load-avisos]") {
    SaveLoadMenuState state;
    save_load_menu_open(
        state, SaveLoadMode::Load,
        slots_with_unreadable(1, gus::domain::save::LoadResult::VersionTooNew));

    REQUIRE(save_load_menu_key_down(state, SDLK_RETURN) == SaveLoadMenuAction::None);
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::Version);
}

TEST_CASE("save_load_menu_click_slot: clique num slot Danificado em modo Load "
          "abre o mesmo aviso Damaged (mesmo efeito do Enter)",
          "[save_load_menu][save-load-avisos]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Load,
                         slots_with_unreadable(1, gus::domain::save::LoadResult::Corrupt));

    REQUIRE(save_load_menu_click_slot(state, 1) == SaveLoadMenuAction::None);
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::Damaged);
    REQUIRE(state.selected == 1);
}

TEST_CASE("save_load_menu_key_down: aviso Damaged - Esc equivale a Cancelar "
          "(WarningCancelled, fecha o aviso)",
          "[save_load_menu][save-load-avisos]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Load,
                         slots_with_unreadable(1, gus::domain::save::LoadResult::HmacInvalid));
    (void)save_load_menu_key_down(state, SDLK_RETURN);  // abre o aviso

    REQUIRE(save_load_menu_key_down(state, SDLK_ESCAPE) == SaveLoadMenuAction::WarningCancelled);
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::None);
}

TEST_CASE("save_load_menu_key_down: aviso Damaged - Enter com Cancelar focado "
          "(default) devolve WarningCancelled, sem alterar o slot",
          "[save_load_menu][save-load-avisos]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Load,
                         slots_with_unreadable(1, gus::domain::save::LoadResult::HmacInvalid));
    (void)save_load_menu_key_down(state, SDLK_RETURN);  // abre o aviso

    REQUIRE(save_load_menu_key_down(state, SDLK_RETURN) == SaveLoadMenuAction::WarningCancelled);
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::None);
}

TEST_CASE("save_load_menu_key_down: aviso Damaged - alternar pra 'Tentar "
          "recuperar' e confirmar devolve RecoverRequested com o slot certo",
          "[save_load_menu][save-load-avisos]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Load,
                         slots_with_unreadable(2, gus::domain::save::LoadResult::Corrupt));
    (void)save_load_menu_click_slot(state, 2);  // foca e abre o aviso no slot 2
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::Damaged);

    REQUIRE(save_load_menu_key_down(state, SDLK_LEFT) == SaveLoadMenuAction::None);
    REQUIRE(state.warning_selected == 0);
    REQUIRE(save_load_menu_key_down(state, SDLK_RETURN) == SaveLoadMenuAction::RecoverRequested);
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::None);
    REQUIRE(state.selected == 2);  // o CHAMADOR ainda le state.selected apos RecoverRequested
}

TEST_CASE("save_load_menu_key_down: aviso Version - LEFT/RIGHT/UP/DOWN nao "
          "fazem nada (so 1 botao) e Enter equivale a Cancelar",
          "[save_load_menu][save-load-avisos]") {
    SaveLoadMenuState state;
    save_load_menu_open(
        state, SaveLoadMode::Load,
        slots_with_unreadable(1, gus::domain::save::LoadResult::VersionTooNew));
    (void)save_load_menu_key_down(state, SDLK_RETURN);  // abre o aviso Version

    REQUIRE(save_load_menu_key_down(state, SDLK_LEFT) == SaveLoadMenuAction::None);
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::Version);  // ainda aberto

    REQUIRE(save_load_menu_key_down(state, SDLK_RETURN) == SaveLoadMenuAction::WarningCancelled);
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::None);
}

TEST_CASE("save_load_menu_click_warning_recover: no-op se o aviso nao for "
          "Damaged (fechado, Version ou RecoverFailed)",
          "[save_load_menu][save-load-avisos]") {
    SaveLoadMenuState state;  // warning_kind default = None
    REQUIRE(save_load_menu_click_warning_recover(state) == SaveLoadMenuAction::None);

    save_load_menu_open(
        state, SaveLoadMode::Load,
        slots_with_unreadable(1, gus::domain::save::LoadResult::VersionTooNew));
    (void)save_load_menu_key_down(state, SDLK_RETURN);  // abre Version
    REQUIRE(save_load_menu_click_warning_recover(state) == SaveLoadMenuAction::None);
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::Version);  // intocado
}

TEST_CASE("save_load_menu_click_warning_recover: clique direto (sem passar pelo "
          "teclado) devolve RecoverRequested quando Damaged",
          "[save_load_menu][save-load-avisos]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Load,
                         slots_with_unreadable(3, gus::domain::save::LoadResult::HmacInvalid));
    (void)save_load_menu_click_slot(state, 3);
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::Damaged);

    REQUIRE(save_load_menu_click_warning_recover(state) == SaveLoadMenuAction::RecoverRequested);
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::None);
}

TEST_CASE("save_load_menu_click_warning_cancel: fecha QUALQUER warning_kind "
          "aberto (Damaged/Version); no-op se ja fechado",
          "[save_load_menu][save-load-avisos]") {
    SaveLoadMenuState state;
    REQUIRE(save_load_menu_click_warning_cancel(state) == SaveLoadMenuAction::None);

    save_load_menu_open(state, SaveLoadMode::Load,
                         slots_with_unreadable(1, gus::domain::save::LoadResult::Corrupt));
    (void)save_load_menu_click_slot(state, 1);
    REQUIRE(save_load_menu_click_warning_cancel(state) == SaveLoadMenuAction::WarningCancelled);
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::None);
}

TEST_CASE("save_load_menu_click_slot/save_load_menu_request_delete: no-op "
          "enquanto o AVISO esta aberto (nunca 2 dialogos simultaneos)",
          "[save_load_menu][save-load-avisos]") {
    SaveLoadMenuState state;
    std::array<SaveSlotPreview, kSlotCount> slots =
        slots_with_unreadable(1, gus::domain::save::LoadResult::HmacInvalid);
    slots[2] = build_slot_preview(make_save_data(100), 2);
    save_load_menu_open(state, SaveLoadMode::Load, slots);
    (void)save_load_menu_click_slot(state, 1);  // abre o aviso no slot 1
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::Damaged);

    REQUIRE(save_load_menu_click_slot(state, 2) == SaveLoadMenuAction::None);
    REQUIRE(state.selected == 1);  // foco NAO mudou pro 2, aviso ainda cobre a tela

    save_load_menu_request_delete(state, 2);
    REQUIRE_FALSE(state.confirming_delete);  // ignorado - aviso ja aberto
}

// SAVE-LOAD-AVISOS end-to-end: "Tentar recuperar" -> load_game_from_backup real,
// MESMO estilo de repro do CRIT-1 acima (grava 2x em disco, corrompe o primario,
// confirma via load_game que reprova, monta o preview via unreadable_slot_preview,
// abre o aviso, e prova o ramo de SUCESSO/FALHA da recuperacao via a funcao real
// gus::platform::fs::load_game_from_backup - o CHAMADOR real, save_load_menu_loop.
// cpp, e quem invoca isso ao receber RecoverRequested; aqui testamos que a funcao
// devolve o dado certo pro CHAMADOR aplicar).

TEST_CASE("SAVE-LOAD-AVISOS: RecoverRequested + load_game_from_backup real "
          "recupera o backup bom (fluxo completo ate o ponto que o CHAMADOR usa)",
          "[save_load_menu][save-load-avisos]") {
    const auto dir =
        std::filesystem::temp_directory_path() / "gusworld_avisos_recover_sucesso_test";
    std::filesystem::remove_all(dir);

    gus::domain::save::SaveData first;
    first.current_scene_path = "city_intro";
    first.party_roster = {"gus"};
    first.party_active = {"gus"};
    first.slot_id = 1;
    first.playtime_seconds = 1.0;
    REQUIRE(gus::platform::fs::save_game(first, 1, dir.string()));

    gus::domain::save::SaveData second = first;
    second.playtime_seconds = 2.0;
    REQUIRE(gus::platform::fs::save_game(second, 1, dir.string()));  // first -> backup1

    {
        std::fstream f(dir / "save_1.sav", std::ios::in | std::ios::out | std::ios::binary);
        REQUIRE(f.is_open());
        char byte = 0;
        f.seekg(10);
        f.read(&byte, 1);
        byte = static_cast<char>(byte ^ 0xFF);
        f.seekp(10);
        f.write(&byte, 1);
    }
    const auto primary_outcome = gus::platform::fs::load_game(1, dir.string());
    REQUIRE(primary_outcome->result != gus::domain::save::LoadResult::Ok);

    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Load,
                         slots_with_unreadable(1, primary_outcome->result));
    (void)save_load_menu_click_slot(state, 1);
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::Damaged);

    REQUIRE(save_load_menu_click_warning_recover(state) == SaveLoadMenuAction::RecoverRequested);
    // O CHAMADOR (save_load_menu_loop.cpp) faria isto agora - provamos aqui que
    // devolve o dado BOM (backup1 = a geracao anterior, "first").
    const auto recovered = gus::platform::fs::load_game_from_backup(1, dir.string());
    REQUIRE(recovered.has_value());
    REQUIRE(recovered->result == gus::domain::save::LoadResult::Ok);
    REQUIRE(recovered->data.playtime_seconds == 1.0);

    std::filesystem::remove_all(dir);
}

TEST_CASE("SAVE-LOAD-AVISOS: RecoverRequested + load_game_from_backup real "
          "FALHA (nenhum backup bom) - o CHAMADOR transita pra RecoverFailed",
          "[save_load_menu][save-load-avisos]") {
    const auto dir =
        std::filesystem::temp_directory_path() / "gusworld_avisos_recover_falha_test";
    std::filesystem::remove_all(dir);

    gus::domain::save::SaveData data;
    data.current_scene_path = "city_intro";
    data.party_roster = {"gus"};
    data.party_active = {"gus"};
    data.slot_id = 1;
    REQUIRE(gus::platform::fs::save_game(data, 1, dir.string()));  // so o primario, sem backup

    {
        std::fstream f(dir / "save_1.sav", std::ios::in | std::ios::out | std::ios::binary);
        REQUIRE(f.is_open());
        char byte = 0;
        f.seekg(10);
        f.read(&byte, 1);
        byte = static_cast<char>(byte ^ 0xFF);
        f.seekp(10);
        f.write(&byte, 1);
    }
    const auto primary_outcome = gus::platform::fs::load_game(1, dir.string());
    REQUIRE(primary_outcome->result != gus::domain::save::LoadResult::Ok);

    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Load,
                         slots_with_unreadable(1, primary_outcome->result));
    (void)save_load_menu_click_slot(state, 1);
    REQUIRE(save_load_menu_click_warning_recover(state) == SaveLoadMenuAction::RecoverRequested);

    REQUIRE_FALSE(gus::platform::fs::load_game_from_backup(1, dir.string()).has_value());
    // O CHAMADOR muta state.warning_kind pra RecoverFailed neste caso (mutacao
    // direta de campo publico, MESMO padrao ja usado por build_previews_and_cache/
    // do_delete em save_load_menu_loop.cpp) - provamos so o estado RESULTANTE
    // esperado (a mutacao em si e trivial, nao ha logica de dominio nela).
    state.warning_kind = SaveLoadMenuState::WarningKind::RecoverFailed;
    state.warning_selected = 1;
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::RecoverFailed);

    // O aviso RecoverFailed tambem so tem Cancelar (mesmo contrato de Version).
    REQUIRE(save_load_menu_key_down(state, SDLK_LEFT) == SaveLoadMenuAction::None);
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::RecoverFailed);  // intocado
    REQUIRE(save_load_menu_key_down(state, SDLK_RETURN) == SaveLoadMenuAction::WarningCancelled);
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::None);

    std::filesystem::remove_all(dir);
}
