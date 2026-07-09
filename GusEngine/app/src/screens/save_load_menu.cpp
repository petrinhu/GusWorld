// gus/app/src/screens/save_load_menu.cpp
//
// Implementacao de save_load_menu.hpp. Ver header para o contrato completo.

#include "gus/app/screens/save_load_menu.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>

namespace gus::app::screens {

namespace {

using gus::domain::save::is_autosave;
using gus::domain::save::kAutosaveSlot;
using gus::domain::save::kSlotCount;

// Enter (varias plataformas mandam RETURN ou KP_ENTER - mesma convencao de
// system_menu.cpp).
bool is_confirm_key(SDL_Keycode key) noexcept {
    return key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE;
}

bool is_back_key(SDL_Keycode key) noexcept { return key == SDLK_ESCAPE; }

bool is_up_key(SDL_Keycode key) noexcept { return key == SDLK_UP || key == SDLK_W; }
bool is_down_key(SDL_Keycode key) noexcept { return key == SDLK_DOWN || key == SDLK_S; }
bool is_left_key(SDL_Keycode key) noexcept { return key == SDLK_LEFT || key == SDLK_A; }
bool is_right_key(SDL_Keycode key) noexcept { return key == SDLK_RIGHT || key == SDLK_D; }

}  // namespace

int chapter_from_xp_fallback(int xp) noexcept {
    int chapter = 1;
    for (const int threshold : kChapterXpThresholds) {
        if (xp >= threshold) ++chapter;
    }
    return std::clamp(chapter, 1, kChapterCount);
}

int chapter_from_quest_progress(const std::map<std::string, int>& quest_progress,
                                 int xp) noexcept {
    const auto it = quest_progress.find("main_story");
    if (it != quest_progress.end()) {
        // HISTORIA MANDA: xp e ignorado (ver o comentario do header).
        const int chapter = 1 + it->second;
        return std::clamp(chapter, 1, kChapterCount);
    }
    // main_story ausente: estimador PROVISORIO por faixas de XP.
    return chapter_from_xp_fallback(xp);
}

int save_xp_for_display(const gus::domain::save::SaveData& data) noexcept {
    const auto it = data.character_states.find("gus");
    if (it == data.character_states.end()) return 0;
    return std::max(0, it->second.xp);
}

std::string format_timestamp_ms(std::int64_t timestamp_ms) {
    if (timestamp_ms < 0) timestamp_ms = 0;
    const std::time_t epoch_seconds = static_cast<std::time_t>(timestamp_ms / 1000);

    // gmtime (UTC, NAO localtime): deterministico independente do fuso horario da
    // maquina que roda o teste/CI - o mock nao especifica fuso, so o FORMATO
    // "DD/MM/AAAA HH:MM".
    std::tm tm_utc{};
#if defined(_WIN32)
    gmtime_s(&tm_utc, &epoch_seconds);
#else
    gmtime_r(&epoch_seconds, &tm_utc);
#endif

    char buf[32];
    std::snprintf(buf, sizeof(buf), "%02d/%02d/%04d %02d:%02d", tm_utc.tm_mday,
                  tm_utc.tm_mon + 1, tm_utc.tm_year + 1900, tm_utc.tm_hour,
                  tm_utc.tm_min);
    return std::string(buf);
}

std::string format_playtime_seconds(double playtime_seconds) {
    if (playtime_seconds < 0.0 || std::isnan(playtime_seconds)) playtime_seconds = 0.0;
    const long long total_minutes =
        static_cast<long long>(playtime_seconds / 60.0);
    const long long hours = total_minutes / 60;
    const long long minutes = total_minutes % 60;

    char buf[32];
    std::snprintf(buf, sizeof(buf), "%lldh %02lldm", hours, minutes);
    return std::string(buf);
}

SaveSlotPreview empty_slot_preview(int slot_id) noexcept {
    SaveSlotPreview preview;
    preview.slot_id = slot_id;
    preview.occupied = false;
    preview.is_autosave = is_autosave(slot_id);
    return preview;
}

SaveSlotPreview build_slot_preview(const gus::domain::save::SaveData& data, int slot_id) {
    SaveSlotPreview preview;
    preview.slot_id = slot_id;
    preview.occupied = true;
    preview.is_autosave = is_autosave(slot_id);
    preview.scene_path = data.current_scene_path;
    preview.timestamp_ms = data.timestamp_ms;
    preview.playtime_seconds = data.playtime_seconds;
    preview.xp = save_xp_for_display(data);
    preview.chapter = chapter_from_quest_progress(data.quest_progress, preview.xp);
    return preview;
}

bool slot_selectable(const SaveLoadMenuState& state, int index) noexcept {
    if (index < 0 || index >= kSlotCount) return false;
    const SaveSlotPreview& slot = state.slots[static_cast<std::size_t>(index)];
    if (state.mode == SaveLoadMode::Save) {
        // Auto e so-leitura em modo Save (mock Tela 2: "o espaco Auto e so-leitura").
        return !slot.is_autosave;
    }
    // Load: qualquer slot OCUPADO e selecionavel (inclusive o autosave); vazio nao.
    return slot.occupied;
}

namespace {

// Proximo indice SELECIONAVEL a partir de `from`, andando em `step` (+1/-1) com
// wrap-around. Devolve `from` inalterado se NENHUM slot for selecionavel (evita
// loop infinito - caso de borda defensivo, ver doc do header).
int next_selectable(const SaveLoadMenuState& state, int from, int step) noexcept {
    for (int i = 0; i < kSlotCount; ++i) {
        from = (from + step + kSlotCount) % kSlotCount;
        if (slot_selectable(state, from)) return from;
    }
    return state.selected;
}

}  // namespace

void save_load_menu_open(
    SaveLoadMenuState& state, SaveLoadMode mode,
    const std::array<SaveSlotPreview, kSlotCount>& slots) noexcept {
    state.mode = mode;
    state.slots = slots;
    state.confirming_overwrite = false;
    state.confirm_selected = 1;

    // Selecao inicial: o PRIMEIRO slot selecionavel a partir do slot logo apos o
    // autosave (mock: slot 1 "sel", nunca o Auto) - reusa next_selectable partindo
    // de kAutosaveSlot andando +1, que ja pula o proprio Auto quando nao
    // selecionavel (modo Save) e qualquer vazio (modo Load).
    state.selected = kAutosaveSlot;
    state.selected = next_selectable(state, kAutosaveSlot, +1);
}

SaveLoadMenuAction save_load_menu_key_down(SaveLoadMenuState& state,
                                            SDL_Keycode key) noexcept {
    if (state.confirming_overwrite) {
        if (is_back_key(key)) {
            // Esc no mini-dialogo = "Nao" (MESMA seguranca de
            // controls_confirming_discard em system_menu.cpp).
            state.confirming_overwrite = false;
            state.confirm_selected = 1;
            return SaveLoadMenuAction::OverwriteCancelled;
        }
        if (is_left_key(key) || is_right_key(key) || is_up_key(key) || is_down_key(key)) {
            state.confirm_selected = (state.confirm_selected == 0) ? 1 : 0;
            return SaveLoadMenuAction::None;
        }
        if (is_confirm_key(key)) {
            const bool yes = (state.confirm_selected == 0);
            state.confirming_overwrite = false;
            state.confirm_selected = 1;
            return yes ? SaveLoadMenuAction::OverwriteConfirmed
                       : SaveLoadMenuAction::OverwriteCancelled;
        }
        return SaveLoadMenuAction::None;
    }

    if (is_back_key(key)) return SaveLoadMenuAction::Back;

    if (is_up_key(key)) {
        state.selected = next_selectable(state, state.selected, -1);
        return SaveLoadMenuAction::None;
    }
    if (is_down_key(key)) {
        state.selected = next_selectable(state, state.selected, +1);
        return SaveLoadMenuAction::None;
    }

    if (is_confirm_key(key)) {
        if (!slot_selectable(state, state.selected)) return SaveLoadMenuAction::None;
        const SaveSlotPreview& slot =
            state.slots[static_cast<std::size_t>(state.selected)];
        if (state.mode == SaveLoadMode::Save && slot.occupied) {
            // Slot manual OCUPADO em modo Save: pede confirmacao (decisao (e)).
            state.confirming_overwrite = true;
            state.confirm_selected = 1;  // default seguro = Nao
            return SaveLoadMenuAction::None;
        }
        return SaveLoadMenuAction::SlotChosen;
    }

    return SaveLoadMenuAction::None;
}

}  // namespace gus::app::screens
