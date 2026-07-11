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

// Delete (tecla dedicada da feature "Apagar") - propositalmente SEPARADA de
// is_back_key/is_confirm_key (nenhuma tecla de navegacao existente e reaproveitada).
bool is_delete_key(SDL_Keycode key) noexcept { return key == SDLK_DELETE; }

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
    preview.present_unreadable = false;
    preview.is_autosave = is_autosave(slot_id);
    return preview;
}

SaveSlotPreview unreadable_slot_preview(int slot_id,
                                         gus::domain::save::LoadResult result) noexcept {
    SaveSlotPreview preview;
    preview.slot_id = slot_id;
    preview.occupied = false;         // CRIT-1: mesmo visual de vazio ("Vazio N")
    preview.present_unreadable = true; // ... mas GATEIA a confirmacao de sobrescrita
    // SAVE-LOAD-AVISOS: so VersionTooNew e Version (irrecuperavel, forward-only);
    // qualquer outro motivo (HmacInvalid/Corrupt/Invalid/WrongSlot) e Damaged
    // (RECUPERAVEL via "Tentar recuperar"/load_game_from_backup).
    preview.unreadable_reason = (result == gus::domain::save::LoadResult::VersionTooNew)
                                     ? UnreadableReason::VersionTooNew
                                     : UnreadableReason::Damaged;
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

int most_recent_occupied_slot(
    const std::array<SaveSlotPreview, kSlotCount>& slots) noexcept {
    int best = -1;
    for (int i = 0; i < kSlotCount; ++i) {
        const SaveSlotPreview& slot = slots[static_cast<std::size_t>(i)];
        if (!slot.occupied) continue;
        if (best == -1 ||
            slot.timestamp_ms > slots[static_cast<std::size_t>(best)].timestamp_ms) {
            best = i;
        }
    }
    return best;
}

bool slot_selectable(const SaveLoadMenuState& state, int index) noexcept {
    if (index < 0 || index >= kSlotCount) return false;
    const SaveSlotPreview& slot = state.slots[static_cast<std::size_t>(index)];
    if (state.mode == SaveLoadMode::Save) {
        // Auto e so-leitura em modo Save (mock Tela 2: "o espaco Auto e so-leitura").
        return !slot.is_autosave;
    }
    // Load: OCUPADO (inclusive autosave) OU present_unreadable (SAVE-LOAD-AVISOS -
    // selecionar um slot ilegivel abre o aviso dedicado, ver confirm_selected_slot)
    // e selecionavel; GENUINAMENTE vazio (nem um nem outro) nao.
    return slot.occupied || slot.present_unreadable;
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
    state.confirming_delete = false;
    state.delete_confirm_selected = 1;
    state.delete_target_slot = -1;
    state.warning_kind = SaveLoadMenuState::WarningKind::None;
    state.warning_selected = 1;

    // Selecao inicial: o PRIMEIRO slot selecionavel a partir do slot logo apos o
    // autosave (mock: slot 1 "sel", nunca o Auto) - reusa next_selectable partindo
    // de kAutosaveSlot andando +1, que ja pula o proprio Auto quando nao
    // selecionavel (modo Save) e qualquer vazio (modo Load).
    state.selected = kAutosaveSlot;
    state.selected = next_selectable(state, kAutosaveSlot, +1);
}

void save_load_menu_reselect_if_needed(SaveLoadMenuState& state) noexcept {
    if (slot_selectable(state, state.selected)) return;
    state.selected = next_selectable(state, state.selected, +1);
}

void save_load_menu_request_delete(SaveLoadMenuState& state, int slot) noexcept {
    if (slot < 0 || slot >= kSlotCount) return;                      // defensivo
    if (state.confirming_overwrite || state.confirming_delete ||
        state.warning_kind != SaveLoadMenuState::WarningKind::None) {
        return;  // ja tem dialogo/aviso aberto
    }
    const SaveSlotPreview& preview = state.slots[static_cast<std::size_t>(slot)];
    if (!preview.occupied) return;  // nada a apagar

    state.confirming_delete = true;
    state.delete_confirm_selected = 1;  // default seguro = Nao
    state.delete_target_slot = slot;
}

namespace {

// Aplica a regra de confirmacao do slot ATUALMENTE focado (state.selected) -
// EXTRAIDO do corpo do ramo is_confirm_key de save_load_menu_key_down pra ser
// reusado por save_load_menu_click_slot (clique de mouse = "focar + Enter", MESMA
// convencao de system_menu_click_option).
SaveLoadMenuAction confirm_selected_slot(SaveLoadMenuState& state) noexcept {
    if (!slot_selectable(state, state.selected)) return SaveLoadMenuAction::None;
    const SaveSlotPreview& slot = state.slots[static_cast<std::size_t>(state.selected)];
    // AJUSTE polish playtest 2026-07-10 (decisao do lider): TODO save em modo Save
    // agora pede confirmacao, INCLUSIVE slot VAZIO (antes so occupied/present_
    // unreadable - CRIT-1, auditoria AUD-SAVE-LOAD-UI-2026-07-09 - abriam o
    // mini-dialogo; um slot GENUINAMENTE vazio salvava DIRETO). O mini-dialogo e o
    // MESMO (confirming_overwrite/confirm_selected, ids slmenu-confirm-yes/no) -
    // so a COPY do body/botao "Sim" muda conforme occupied/present_unreadable
    // ("Sobrescrever este slot?") vs GENUINAMENTE vazio ("Deseja salvar no slot
    // [N] (vazio)?"), ver save_load_menu_rml.cpp. do_save (CHAMADOR,
    // save_load_menu_loop.cpp) ja era generico sobre occupied/vazio - nenhuma
    // mudanca de I/O aqui, so a UX de confirmar ANTES.
    if (state.mode == SaveLoadMode::Save) {
        state.confirming_overwrite = true;
        state.confirm_selected = 1;  // default seguro = Nao/Cancelar
        return SaveLoadMenuAction::None;
    }
    // SAVE-LOAD-AVISOS (aviso #1): slot present_unreadable em modo Load abre o
    // AVISO dedicado em vez de fingir um SlotChosen (que carregaria dados
    // invalidos/nao existentes) - o motivo (Damaged/VersionTooNew) ja veio
    // pronto do CHAMADOR via unreadable_slot_preview.
    if (state.mode == SaveLoadMode::Load && slot.present_unreadable) {
        state.warning_kind = (slot.unreadable_reason == UnreadableReason::VersionTooNew)
                                  ? SaveLoadMenuState::WarningKind::Version
                                  : SaveLoadMenuState::WarningKind::Damaged;
        state.warning_selected = 1;  // default seguro = Cancelar
        return SaveLoadMenuAction::None;
    }
    return SaveLoadMenuAction::SlotChosen;
}

}  // namespace

SaveLoadMenuAction save_load_menu_click_slot(SaveLoadMenuState& state, int slot) noexcept {
    if (slot < 0 || slot >= kSlotCount) return SaveLoadMenuAction::None;
    if (state.confirming_overwrite || state.confirming_delete ||
        state.warning_kind != SaveLoadMenuState::WarningKind::None) {
        return SaveLoadMenuAction::None;  // dialogo/aviso aberto - clique na lista nao se aplica
    }
    if (!slot_selectable(state, slot)) return SaveLoadMenuAction::None;
    state.selected = slot;
    return confirm_selected_slot(state);
}

SaveLoadMenuAction save_load_menu_click_overwrite_confirm(SaveLoadMenuState& state,
                                                           int pill) noexcept {
    if (!state.confirming_overwrite) return SaveLoadMenuAction::None;
    if (pill != 0 && pill != 1) return SaveLoadMenuAction::None;  // defensivo
    const bool yes = (pill == 0);
    state.confirming_overwrite = false;
    state.confirm_selected = 1;
    return yes ? SaveLoadMenuAction::OverwriteConfirmed : SaveLoadMenuAction::OverwriteCancelled;
}

SaveLoadMenuAction save_load_menu_click_delete_confirm(SaveLoadMenuState& state,
                                                        int pill) noexcept {
    if (!state.confirming_delete) return SaveLoadMenuAction::None;
    if (pill != 0 && pill != 1) return SaveLoadMenuAction::None;  // defensivo
    const bool yes = (pill == 0);
    state.confirming_delete = false;
    state.delete_confirm_selected = 1;
    return yes ? SaveLoadMenuAction::DeleteConfirmed : SaveLoadMenuAction::DeleteCancelled;
}

SaveLoadMenuAction save_load_menu_click_warning_recover(SaveLoadMenuState& state) noexcept {
    // O botao "Tentar recuperar" SO EXISTE quando warning_kind==Damaged (ver a
    // doc de save_load_menu_click_warning_recover no header) - qualquer outro
    // caso (aviso fechado, Version, RecoverFailed) e no-op defensivo.
    if (state.warning_kind != SaveLoadMenuState::WarningKind::Damaged) {
        return SaveLoadMenuAction::None;
    }
    state.warning_kind = SaveLoadMenuState::WarningKind::None;
    state.warning_selected = 1;
    return SaveLoadMenuAction::RecoverRequested;
}

SaveLoadMenuAction save_load_menu_click_warning_cancel(SaveLoadMenuState& state) noexcept {
    if (state.warning_kind == SaveLoadMenuState::WarningKind::None) {
        return SaveLoadMenuAction::None;
    }
    state.warning_kind = SaveLoadMenuState::WarningKind::None;
    state.warning_selected = 1;
    return SaveLoadMenuAction::WarningCancelled;
}

SaveLoadMenuAction save_load_menu_key_down(SaveLoadMenuState& state,
                                            SDL_Keycode key) noexcept {
    if (state.warning_kind != SaveLoadMenuState::WarningKind::None) {
        // Damaged tem 2 botoes (0=Tentar recuperar, 1=Cancelar); Version/
        // RecoverFailed tem SO Cancelar (nao ha pill 0 pra alternar - ver a doc
        // de WarningKind no header).
        const bool two_buttons = (state.warning_kind == SaveLoadMenuState::WarningKind::Damaged);
        if (is_back_key(key)) return save_load_menu_click_warning_cancel(state);
        if (two_buttons &&
            (is_left_key(key) || is_right_key(key) || is_up_key(key) || is_down_key(key))) {
            state.warning_selected = (state.warning_selected == 0) ? 1 : 0;
            return SaveLoadMenuAction::None;
        }
        if (is_confirm_key(key)) {
            return (two_buttons && state.warning_selected == 0)
                       ? save_load_menu_click_warning_recover(state)
                       : save_load_menu_click_warning_cancel(state);
        }
        return SaveLoadMenuAction::None;
    }

    if (state.confirming_delete) {
        if (is_back_key(key)) {
            // Esc no mini-dialogo = "Nao" (MESMA seguranca de confirming_overwrite/
            // controls_confirming_discard em system_menu.cpp).
            state.confirming_delete = false;
            state.delete_confirm_selected = 1;
            return SaveLoadMenuAction::DeleteCancelled;
        }
        if (is_left_key(key) || is_right_key(key) || is_up_key(key) || is_down_key(key)) {
            state.delete_confirm_selected = (state.delete_confirm_selected == 0) ? 1 : 0;
            return SaveLoadMenuAction::None;
        }
        if (is_confirm_key(key)) {
            const bool yes = (state.delete_confirm_selected == 0);
            state.confirming_delete = false;
            state.delete_confirm_selected = 1;
            // delete_target_slot fica SETADO (nao resetado) quando Sim: o CHAMADOR
            // le state.delete_target_slot logo apos receber DeleteConfirmed pra
            // saber QUAL slot apagar de fato em disco (mesmo espirito de
            // state.selected em OverwriteConfirmed).
            return yes ? SaveLoadMenuAction::DeleteConfirmed
                       : SaveLoadMenuAction::DeleteCancelled;
        }
        return SaveLoadMenuAction::None;
    }

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

    if (is_delete_key(key)) {
        // Alvo = o slot ATUALMENTE focado (o clique de mouse no icone por-linha
        // usa save_load_menu_request_delete direto, targeting a linha clicada -
        // ver save_load_menu_loop.cpp). No-op silencioso (via o guard interno de
        // save_load_menu_request_delete) se vazio/ja tem dialogo aberto.
        save_load_menu_request_delete(state, state.selected);
        return SaveLoadMenuAction::None;
    }

    if (is_confirm_key(key)) return confirm_selected_slot(state);

    return SaveLoadMenuAction::None;
}

}  // namespace gus::app::screens
