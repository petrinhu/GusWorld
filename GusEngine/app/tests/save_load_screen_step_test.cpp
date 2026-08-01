// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/save_load_screen_step_test.cpp
//
// Catch2 (headless, SEM SDL_Init/janela/GL/glintfx::UiLayer real) de
// gus::app::screens::save_load_screen_step (F4-1b.2, onda F4 "casca SDL -> App
// mode do glintfx", fatia 1b.2 - SEGUNDA tela convertida ao contrato
// ScreenState, MESMO template de F4-1b.1/difficulty_screen_step_test.cpp). A
// funcao e PURA o suficiente pra ser exercitada com SDL_Event/
// glintfx::ElementBox construidos a mao - ela NUNCA consulta a UiLayer, as
// boxes entram como parametro (SaveLoadStepBoxes). O CONSUMIDOR real
// (SaveLoadScreen, GL-heavy) continua sem teste direto - ver o topo de
// gus/app/src/screens/save_load_menu_loop.cpp (a interacao GL/SFX/hit-test
// REAL segue coberta por save_load_menu_interaction_test.cpp +
// save_load_menu_loop_interaction_test.cpp).
//
// OS 3 EXITS (QuitApp/ClosedAfterLoad/BackToPause): QuitApp so existe no nivel
// do WRAPPER (run_save_load_menu_loop_gl_current mapeia window_closed=true ->
// QuitApp, ver save_load_menu_loop.cpp) - no nivel PURO ele e o flag
// `window_closed` (testado abaixo); BackToPause e ClosedAfterLoad SAO
// devolvidos por save_load_screen_step via `result.exit` quando 100%
// deterministicos SEM I/O (Back->BackToPause sempre; SlotChosen num slot
// OCUPADO em modo Load->ClosedAfterLoad, assumindo o INVARIANTE "slot
// selecionavel em Load SEMPRE tem cache" - ver save_load_menu.hpp) - ambos
// testados abaixo por evento sintetico.

#include <array>
#include <optional>

#include <catch2/catch_test_macros.hpp>

#include <SDL3/SDL.h>
#include <glintfx/element_box.hpp>

#include "gus/app/screens/save_load_menu.hpp"
#include "gus/app/screens/save_load_menu_loop.hpp"
#include "gus/domain/save/save_data.hpp"
#include "gus/domain/save/save_serializer.hpp"
#include "gus/domain/save/save_slots.hpp"

using gus::app::screens::SaveLoadLoopExit;
using gus::app::screens::save_load_screen_step;
using gus::app::screens::SaveLoadMenuAction;
using gus::app::screens::SaveLoadMenuState;
using gus::app::screens::SaveLoadMode;
using gus::app::screens::SaveLoadSfxKind;
using gus::app::screens::SaveLoadStepBoxes;
using gus::app::screens::SaveLoadStepResult;
using gus::app::screens::SaveSlotPreview;
using gus::domain::save::kAutosaveSlot;
using gus::domain::save::kSlotCount;
using gus::domain::save::SaveData;

namespace {

glintfx::ElementBox make_box(float x, float y, float w, float h) {
    glintfx::ElementBox box;
    box.found = true;
    box.x = x;
    box.y = y;
    box.w = w;
    box.h = h;
    return box;
}

SDL_Event key_down_event(SDL_Keycode key) {
    SDL_Event ev{};
    ev.type = SDL_EVENT_KEY_DOWN;
    ev.key.key = key;
    ev.key.repeat = 0;
    return ev;
}

SDL_Event mouse_button_down_event(float x, float y) {
    SDL_Event ev{};
    ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    ev.button.button = SDL_BUTTON_LEFT;
    ev.button.x = x;
    ev.button.y = y;
    return ev;
}

SaveData make_save_data() {
    SaveData data;
    data.current_scene_path = "distritos_inferiores";
    data.timestamp_ms = 1783455240000LL;
    data.playtime_seconds = 2520.0;
    data.character_states["gus"].xp = 10;
    return data;
}

// Snapshot dos campos ESCALARES de SaveLoadMenuState (navegacao/dialogo/aviso) -
// `slots` (arvore de SaveSlotPreview) fica de fora de proposito: nenhuma funcao
// exercitada aqui muta previews (isso e do_save_/do_delete_, I/O real, fora do
// escopo puro) - MESMO racional de state_eq em difficulty_screen_step_test.cpp.
struct StateSnapshot {
    SaveLoadMode mode;
    int selected;
    bool confirming_overwrite;
    int confirm_selected;
    bool confirming_delete;
    int delete_confirm_selected;
    int delete_target_slot;
    SaveLoadMenuState::WarningKind warning_kind;
    int warning_selected;
};

StateSnapshot snapshot(const SaveLoadMenuState& s) {
    return {s.mode,       s.selected,
            s.confirming_overwrite, s.confirm_selected,
            s.confirming_delete,    s.delete_confirm_selected,
            s.delete_target_slot,   s.warning_kind,
            s.warning_selected};
}

bool operator==(const StateSnapshot& a, const StateSnapshot& b) {
    return a.mode == b.mode && a.selected == b.selected &&
           a.confirming_overwrite == b.confirming_overwrite &&
           a.confirm_selected == b.confirm_selected &&
           a.confirming_delete == b.confirming_delete &&
           a.delete_confirm_selected == b.delete_confirm_selected &&
           a.delete_target_slot == b.delete_target_slot &&
           a.warning_kind == b.warning_kind && a.warning_selected == b.warning_selected;
}

// Estado com o slot 1 OCUPADO (demais vazios) - selecionavel em AMBOS os modos
// (Save: slot manual, sempre selecionavel; Load: ocupado, selecionavel).
SaveLoadMenuState open_state_with_occupied_slot1(SaveLoadMode mode) {
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) {
        slots[static_cast<std::size_t>(i)] =
            gus::app::screens::empty_slot_preview(i);
    }
    slots[1] = gus::app::screens::build_slot_preview(make_save_data(), 1);
    SaveLoadMenuState state;
    gus::app::screens::save_load_menu_open(state, mode, slots);
    return state;
}

// Estado em modo Load com o slot 1 PRESENTE-mas-ILEGIVEL (Damaged) - ver
// SAVE-LOAD-AVISOS em save_load_menu.hpp.
SaveLoadMenuState open_state_with_damaged_slot1() {
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) {
        slots[static_cast<std::size_t>(i)] =
            gus::app::screens::empty_slot_preview(i);
    }
    slots[1] = gus::app::screens::unreadable_slot_preview(
        1, gus::domain::save::LoadResult::Corrupt);
    SaveLoadMenuState state;
    gus::app::screens::save_load_menu_open(state, SaveLoadMode::Load, slots);
    return state;
}

}  // namespace

// ---------------------------------------------------------------- QUIT

TEST_CASE("save_load_screen_step: SDL_EVENT_QUIT vira window_closed=true, sem "
          "tocar estado/sfx/reload/action/exit",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_occupied_slot1(SaveLoadMode::Save);
    const StateSnapshot before = snapshot(state);

    SDL_Event ev{};
    ev.type = SDL_EVENT_QUIT;
    const SaveLoadStepBoxes boxes{};
    const SaveLoadStepResult result = save_load_screen_step(state, ev, boxes);

    REQUIRE(result.window_closed);
    REQUIRE_FALSE(result.resize);
    REQUIRE_FALSE(result.reload);
    REQUIRE_FALSE(result.action.has_value());
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(result.sfx == SaveLoadSfxKind::None);
    REQUIRE(snapshot(state) == before);
}

// ---------------------------------------------------------------- RESIZE

TEST_CASE("save_load_screen_step: WINDOW_RESIZED/PIXEL_SIZE_CHANGED viram "
          "resize=true e sao NO-OP DE ESTADO - o CHAMADOR reposiciona a UiLayer",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_occupied_slot1(SaveLoadMode::Load);
    const StateSnapshot before = snapshot(state);
    const SaveLoadStepBoxes boxes{};

    SDL_Event resized{};
    resized.type = SDL_EVENT_WINDOW_RESIZED;
    const SaveLoadStepResult r1 = save_load_screen_step(state, resized, boxes);
    REQUIRE(r1.resize);
    REQUIRE_FALSE(r1.window_closed);
    REQUIRE_FALSE(r1.reload);
    REQUIRE_FALSE(r1.action.has_value());
    REQUIRE(snapshot(state) == before);

    SDL_Event pixel_changed{};
    pixel_changed.type = SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;
    const SaveLoadStepResult r2 = save_load_screen_step(state, pixel_changed, boxes);
    REQUIRE(r2.resize);
    REQUIRE(snapshot(state) == before);
}

// ---------------------------------------------------------------- ESC na lista -> BackToPause

TEST_CASE("save_load_screen_step: ESC fora de qualquer dialogo/aviso devolve "
          "action=Back e exit=BackToPause (decidido SEM I/O)",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_occupied_slot1(SaveLoadMode::Load);
    const SaveLoadStepBoxes boxes{};

    const SDL_Event ev = key_down_event(SDLK_ESCAPE);
    const SaveLoadStepResult result = save_load_screen_step(state, ev, boxes);

    REQUIRE(result.action.has_value());
    REQUIRE(*result.action == SaveLoadMenuAction::Back);
    REQUIRE(result.exit.has_value());
    REQUIRE(*result.exit == SaveLoadLoopExit::BackToPause);
    REQUIRE_FALSE(result.reload);  // Back nao passa por reload - o CHAMADOR
                                   // retorna na hora (MESMO padrao de Cancelled
                                   // em difficulty_screen_step).
}

TEST_CASE("save_load_screen_step: clique de mouse REAL em 'Voltar' devolve "
          "action=Back e exit=BackToPause, tocando Click",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_occupied_slot1(SaveLoadMode::Save);
    SaveLoadStepBoxes boxes{};
    boxes.back = make_box(10.0f, 400.0f, 100.0f, 30.0f);

    const SDL_Event ev = mouse_button_down_event(30.0f, 410.0f);
    const SaveLoadStepResult result = save_load_screen_step(state, ev, boxes);

    REQUIRE(result.action.has_value());
    REQUIRE(*result.action == SaveLoadMenuAction::Back);
    REQUIRE(result.exit.has_value());
    REQUIRE(*result.exit == SaveLoadLoopExit::BackToPause);
    REQUIRE(result.sfx == SaveLoadSfxKind::Click);
}

// ---------------------------------------------------------------- navegacao (setas)

TEST_CASE("save_load_screen_step: navegar (seta Baixo/Cima) entre slots "
          "selecionaveis e action=None + reload=true, SEM sfx (teclado nunca "
          "tocou som nesta tela)",
          "[save_load_screen_step][f4-1b2]") {
    // slot 1 ocupado, autosave (0) ocupado tambem em modo Load - 2 selecionaveis.
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) {
        slots[static_cast<std::size_t>(i)] = gus::app::screens::empty_slot_preview(i);
    }
    slots[kAutosaveSlot] = gus::app::screens::build_slot_preview(make_save_data(), kAutosaveSlot);
    slots[1] = gus::app::screens::build_slot_preview(make_save_data(), 1);
    SaveLoadMenuState state;
    gus::app::screens::save_load_menu_open(state, SaveLoadMode::Load, slots);
    REQUIRE(state.selected == 1);  // save_load_menu_open pula o autosave (mock: slot 1 "sel")

    const SaveLoadStepBoxes boxes{};
    const SDL_Event ev = key_down_event(SDLK_UP);
    const SaveLoadStepResult result = save_load_screen_step(state, ev, boxes);

    REQUIRE(state.selected == kAutosaveSlot);
    REQUIRE(result.action.has_value());
    REQUIRE(*result.action == SaveLoadMenuAction::None);
    REQUIRE(result.reload);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(result.sfx == SaveLoadSfxKind::None);
}

// ---------------------------------------------------------------- confirmacao de sobrescrita (modo Save)

TEST_CASE("save_load_screen_step: Enter num slot manual em modo Save abre o "
          "mini-dialogo de sobrescrita (action=None, reload=true, "
          "confirming_overwrite vira true) - MESMO fluxo pra slot ocupado E "
          "GENUINAMENTE vazio (AJUSTE polish playtest 2026-07-10)",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_occupied_slot1(SaveLoadMode::Save);
    REQUIRE(state.selected == 1);
    const SaveLoadStepBoxes boxes{};

    const SDL_Event ev = key_down_event(SDLK_RETURN);
    const SaveLoadStepResult result = save_load_screen_step(state, ev, boxes);

    REQUIRE(result.action.has_value());
    REQUIRE(*result.action == SaveLoadMenuAction::None);
    REQUIRE(result.reload);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(state.confirming_overwrite);
}

TEST_CASE("save_load_screen_step: dentro do dialogo de sobrescrita, confirmar "
          "('Sim') devolve action=OverwriteConfirmed + reload=true (I/O real "
          "fica com o CHAMADOR - exit nao e decidido aqui); cancelar ('Nao'/Esc) "
          "devolve OverwriteCancelled",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_occupied_slot1(SaveLoadMode::Save);
    (void)save_load_screen_step(state, key_down_event(SDLK_RETURN), SaveLoadStepBoxes{});
    REQUIRE(state.confirming_overwrite);
    state.confirm_selected = 0;  // foca "Sim"

    SECTION("Confirmar (Enter)") {
        const SaveLoadStepResult result =
            save_load_screen_step(state, key_down_event(SDLK_RETURN), SaveLoadStepBoxes{});
        REQUIRE(result.action.has_value());
        REQUIRE(*result.action == SaveLoadMenuAction::OverwriteConfirmed);
        REQUIRE(result.reload);
        REQUIRE_FALSE(result.exit.has_value());
        REQUIRE_FALSE(state.confirming_overwrite);
    }

    SECTION("Cancelar (Esc)") {
        const SaveLoadStepResult result =
            save_load_screen_step(state, key_down_event(SDLK_ESCAPE), SaveLoadStepBoxes{});
        REQUIRE(result.action.has_value());
        REQUIRE(*result.action == SaveLoadMenuAction::OverwriteCancelled);
        REQUIRE(result.reload);
        REQUIRE_FALSE(state.confirming_overwrite);
    }

    SECTION("Cancelar via clique na pill 'Nao'") {
        SaveLoadStepBoxes boxes{};
        boxes.overwrite_confirm[1] = make_box(100.0f, 0.0f, 50.0f, 50.0f);
        const SDL_Event ev = mouse_button_down_event(110.0f, 10.0f);
        const SaveLoadStepResult result = save_load_screen_step(state, ev, boxes);
        REQUIRE(result.action.has_value());
        REQUIRE(*result.action == SaveLoadMenuAction::OverwriteCancelled);
        REQUIRE(result.sfx == SaveLoadSfxKind::Click);
        REQUIRE_FALSE(state.confirming_overwrite);
    }
}

// ---------------------------------------------------------------- SlotChosen (modo Load) -> ClosedAfterLoad

TEST_CASE("save_load_screen_step: Enter num slot OCUPADO em modo Load devolve "
          "action=SlotChosen e exit=ClosedAfterLoad (decidido SEM I/O, assume o "
          "INVARIANTE de cache - ver save_load_menu.hpp)",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_occupied_slot1(SaveLoadMode::Load);
    REQUIRE(state.selected == 1);
    const SaveLoadStepBoxes boxes{};

    const SDL_Event ev = key_down_event(SDLK_RETURN);
    const SaveLoadStepResult result = save_load_screen_step(state, ev, boxes);

    REQUIRE(result.action.has_value());
    REQUIRE(*result.action == SaveLoadMenuAction::SlotChosen);
    REQUIRE(result.exit.has_value());
    REQUIRE(*result.exit == SaveLoadLoopExit::ClosedAfterLoad);
    REQUIRE_FALSE(result.reload);
}

TEST_CASE("save_load_screen_step: clique de mouse REAL DENTRO da box de um "
          "slot OCUPADO em modo Load devolve SlotChosen/ClosedAfterLoad, "
          "tocando Click; clique FORA de qualquer box e NO-OP TOTAL",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_occupied_slot1(SaveLoadMode::Load);

    SECTION("clique DENTRO da box do slot 1") {
        SaveLoadStepBoxes boxes{};
        boxes.slots[1] = make_box(10.0f, 10.0f, 100.0f, 40.0f);
        const SDL_Event ev = mouse_button_down_event(30.0f, 25.0f);
        const SaveLoadStepResult result = save_load_screen_step(state, ev, boxes);

        REQUIRE(result.sfx == SaveLoadSfxKind::Click);
        REQUIRE(result.action.has_value());
        REQUIRE(*result.action == SaveLoadMenuAction::SlotChosen);
        REQUIRE(result.exit.has_value());
        REQUIRE(*result.exit == SaveLoadLoopExit::ClosedAfterLoad);
        REQUIRE(state.selected == 1);
    }

    SECTION("clique FORA de qualquer box (canto vazio, nenhuma box cobre)") {
        const StateSnapshot before = snapshot(state);
        SaveLoadStepBoxes boxes{};
        boxes.slots[1] = make_box(10.0f, 10.0f, 100.0f, 40.0f);
        const SDL_Event ev = mouse_button_down_event(500.0f, 500.0f);
        const SaveLoadStepResult result = save_load_screen_step(state, ev, boxes);

        REQUIRE(result.sfx == SaveLoadSfxKind::None);
        REQUIRE_FALSE(result.reload);
        REQUIRE_FALSE(result.action.has_value());
        REQUIRE_FALSE(result.exit.has_value());
        REQUIRE(snapshot(state) == before);
    }
}

// ---------------------------------------------------------------- SAVE-LOAD-AVISOS (Damaged)

TEST_CASE("save_load_screen_step: Enter num slot Damaged em modo Load abre o "
          "AVISO dedicado (action=None, reload=true, warning_kind=Damaged) em "
          "vez de fingir um SlotChosen",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_damaged_slot1();
    REQUIRE(state.selected == 1);
    const SaveLoadStepBoxes boxes{};

    const SDL_Event ev = key_down_event(SDLK_RETURN);
    const SaveLoadStepResult result = save_load_screen_step(state, ev, boxes);

    REQUIRE(result.action.has_value());
    REQUIRE(*result.action == SaveLoadMenuAction::None);
    REQUIRE(result.reload);
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::Damaged);
}

TEST_CASE("save_load_screen_step: dentro do AVISO Damaged, 'Tentar recuperar' "
          "devolve action=RecoverRequested SEM reload/exit (I/O real decide o "
          "exit de fato, ver SaveLoadScreen::do_recover_); 'Cancelar' devolve "
          "WarningCancelled com reload=true",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_damaged_slot1();
    (void)save_load_screen_step(state, key_down_event(SDLK_RETURN), SaveLoadStepBoxes{});
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::Damaged);

    SECTION("Tentar recuperar (Enter com warning_selected=0)") {
        state.warning_selected = 0;
        const SaveLoadStepResult result =
            save_load_screen_step(state, key_down_event(SDLK_RETURN), SaveLoadStepBoxes{});
        REQUIRE(result.action.has_value());
        REQUIRE(*result.action == SaveLoadMenuAction::RecoverRequested);
        REQUIRE_FALSE(result.reload);
        REQUIRE_FALSE(result.exit.has_value());
        REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::None);
    }

    SECTION("Cancelar (Esc)") {
        const SaveLoadStepResult result =
            save_load_screen_step(state, key_down_event(SDLK_ESCAPE), SaveLoadStepBoxes{});
        REQUIRE(result.action.has_value());
        REQUIRE(*result.action == SaveLoadMenuAction::WarningCancelled);
        REQUIRE(result.reload);
        REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::None);
    }

    SECTION("clique real em 'Tentar recuperar'") {
        SaveLoadStepBoxes boxes{};
        boxes.warn_recover = make_box(0.0f, 0.0f, 80.0f, 30.0f);
        const SDL_Event ev = mouse_button_down_event(10.0f, 10.0f);
        const SaveLoadStepResult result = save_load_screen_step(state, ev, boxes);
        REQUIRE(result.sfx == SaveLoadSfxKind::Click);
        REQUIRE(result.action.has_value());
        REQUIRE(*result.action == SaveLoadMenuAction::RecoverRequested);
    }
}

// ---------------------------------------------------------------- feature "Apagar"

TEST_CASE("save_load_screen_step: tecla Delete num slot OCUPADO abre o "
          "mini-dialogo de EXCLUSAO (action=None, reload=true, "
          "confirming_delete=true, delete_target_slot=slot focado)",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_occupied_slot1(SaveLoadMode::Load);
    REQUIRE(state.selected == 1);
    const SaveLoadStepBoxes boxes{};

    const SDL_Event ev = key_down_event(SDLK_DELETE);
    const SaveLoadStepResult result = save_load_screen_step(state, ev, boxes);

    REQUIRE(result.action.has_value());
    REQUIRE(*result.action == SaveLoadMenuAction::None);
    REQUIRE(result.reload);
    REQUIRE(state.confirming_delete);
    REQUIRE(state.delete_target_slot == 1);
}

TEST_CASE("save_load_screen_step: clique real no icone de apagar de um slot "
          "OCUPADO abre o mini-dialogo de EXCLUSAO SEM passar por uma "
          "SaveLoadMenuAction (action=nullopt, reload=true, sfx=Click) - MESMA "
          "mutacao DIRETA via save_load_menu_request_delete do while(true) "
          "antigo",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_occupied_slot1(SaveLoadMode::Load);
    SaveLoadStepBoxes boxes{};
    boxes.delete_icons[1] = make_box(200.0f, 10.0f, 20.0f, 20.0f);

    const SDL_Event ev = mouse_button_down_event(210.0f, 20.0f);
    const SaveLoadStepResult result = save_load_screen_step(state, ev, boxes);

    REQUIRE_FALSE(result.action.has_value());
    REQUIRE(result.reload);
    REQUIRE(result.sfx == SaveLoadSfxKind::Click);
    REQUIRE(state.confirming_delete);
    REQUIRE(state.delete_target_slot == 1);
}

TEST_CASE("save_load_screen_step: dentro do dialogo de EXCLUSAO, confirmar "
          "('Sim') devolve DeleteConfirmed + reload=true (do_delete real fica "
          "com o CHAMADOR); cancelar devolve DeleteCancelled",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_occupied_slot1(SaveLoadMode::Load);
    (void)save_load_screen_step(state, key_down_event(SDLK_DELETE), SaveLoadStepBoxes{});
    REQUIRE(state.confirming_delete);
    state.delete_confirm_selected = 0;  // foca "Sim"

    SECTION("Confirmar (Enter)") {
        const SaveLoadStepResult result =
            save_load_screen_step(state, key_down_event(SDLK_RETURN), SaveLoadStepBoxes{});
        REQUIRE(result.action.has_value());
        REQUIRE(*result.action == SaveLoadMenuAction::DeleteConfirmed);
        REQUIRE(result.reload);
        REQUIRE_FALSE(state.confirming_delete);
    }

    SECTION("Cancelar (Esc)") {
        const SaveLoadStepResult result =
            save_load_screen_step(state, key_down_event(SDLK_ESCAPE), SaveLoadStepBoxes{});
        REQUIRE(result.action.has_value());
        REQUIRE(*result.action == SaveLoadMenuAction::DeleteCancelled);
        REQUIRE(result.reload);
        REQUIRE_FALSE(state.confirming_delete);
    }
}

// ---------------------------------------------------------------- MOUSE_MOTION

TEST_CASE("save_load_screen_step: MOUSE_MOTION vira mouse_move=true com (x,y) "
          "repassados - o CHAMADOR encaminha pro glintfx::UiEvent::MouseMove "
          "(hover NATIVO), sem tocar estado/sfx/action/exit aqui",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_occupied_slot1(SaveLoadMode::Save);
    const StateSnapshot before = snapshot(state);

    SDL_Event ev{};
    ev.type = SDL_EVENT_MOUSE_MOTION;
    ev.motion.x = 42.0f;
    ev.motion.y = 84.0f;
    const SaveLoadStepResult result = save_load_screen_step(state, ev, SaveLoadStepBoxes{});

    REQUIRE(result.mouse_move);
    REQUIRE(result.mouse_x == 42.0f);
    REQUIRE(result.mouse_y == 84.0f);
    REQUIRE_FALSE(result.reload);
    REQUIRE(result.sfx == SaveLoadSfxKind::None);
    REQUIRE_FALSE(result.action.has_value());
    REQUIRE(snapshot(state) == before);
}

// ---------------------------------------------------------------- eventos nao roteados aqui

TEST_CASE("save_load_screen_step: SDL_EVENT_MOUSE_WHEEL e NO-OP TOTAL (exige "
          "SDL_GetMouseState, consulta IMPURA - tratado FORA desta funcao, em "
          "SaveLoadScreen::handle_event)",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_occupied_slot1(SaveLoadMode::Save);
    const StateSnapshot before = snapshot(state);

    SDL_Event ev{};
    ev.type = SDL_EVENT_MOUSE_WHEEL;
    ev.wheel.y = 1.0f;
    const SaveLoadStepResult result = save_load_screen_step(state, ev, SaveLoadStepBoxes{});

    REQUIRE_FALSE(result.window_closed);
    REQUIRE_FALSE(result.resize);
    REQUIRE_FALSE(result.mouse_move);
    REQUIRE_FALSE(result.reload);
    REQUIRE(result.sfx == SaveLoadSfxKind::None);
    REQUIRE_FALSE(result.action.has_value());
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(snapshot(state) == before);
}

TEST_CASE("save_load_screen_step: tipo de evento que esta tela nao roteia "
          "(ex.: TEXT_INPUT) e NO-OP TOTAL (resultado default)",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_occupied_slot1(SaveLoadMode::Save);
    const StateSnapshot before = snapshot(state);

    SDL_Event ev{};
    ev.type = SDL_EVENT_TEXT_INPUT;
    const SaveLoadStepResult result = save_load_screen_step(state, ev, SaveLoadStepBoxes{});

    REQUIRE_FALSE(result.window_closed);
    REQUIRE_FALSE(result.reload);
    REQUIRE(result.sfx == SaveLoadSfxKind::None);
    REQUIRE_FALSE(result.action.has_value());
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(snapshot(state) == before);
}

// ---------------------------------------------------------------- repeat de tecla

TEST_CASE("save_load_screen_step: SDL_EVENT_KEY_DOWN com key.repeat!=0 "
          "(auto-repeat do SO) e NO-OP TOTAL - so a 1a borda (repeat=0) e "
          "roteada, MESMO guard do while(true) antigo",
          "[save_load_screen_step][f4-1b2]") {
    SaveLoadMenuState state = open_state_with_occupied_slot1(SaveLoadMode::Save);
    const StateSnapshot before = snapshot(state);

    SDL_Event ev{};
    ev.type = SDL_EVENT_KEY_DOWN;
    ev.key.key = SDLK_DOWN;
    ev.key.repeat = 1;  // auto-repeat - NAO deve navegar
    const SaveLoadStepResult result = save_load_screen_step(state, ev, SaveLoadStepBoxes{});

    REQUIRE_FALSE(result.reload);
    REQUIRE(result.sfx == SaveLoadSfxKind::None);
    REQUIRE_FALSE(result.action.has_value());
    REQUIRE_FALSE(result.exit.has_value());
    REQUIRE(snapshot(state) == before);
}
