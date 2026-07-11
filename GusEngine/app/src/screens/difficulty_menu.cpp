// gus/app/src/screens/difficulty_menu.cpp
//
// Implementacao da logica pura da TELA DE SELECAO DE DIFICULDADE. Ver header para
// o contrato completo. Travada por app/tests/difficulty_menu_test.cpp (TEST-FIRST).

#include "gus/app/screens/difficulty_menu.hpp"

namespace gus::app::screens {

namespace {

bool is_confirm_key(SDL_Keycode key) noexcept {
    return key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE;
}

bool is_up_key(SDL_Keycode key) noexcept { return key == SDLK_UP || key == SDLK_W; }
bool is_down_key(SDL_Keycode key) noexcept { return key == SDLK_DOWN || key == SDLK_S; }
bool is_axis_key(SDL_Keycode key) noexcept {
    return key == SDLK_UP || key == SDLK_DOWN || key == SDLK_LEFT || key == SDLK_RIGHT ||
           key == SDLK_W || key == SDLK_S || key == SDLK_A || key == SDLK_D;
}

}  // namespace

bool difficulty_item_selectable(const DifficultyMenuState& state, int index) noexcept {
    if (index < 0 || index >= kDifficultyItemCount) return false;
    if (static_cast<DifficultyMenuItem>(index) == DifficultyMenuItem::Hardcore) {
        return state.hardcore_unlocked;
    }
    return true;  // Facil/Medio/Dificil sempre selecionaveis
}

void difficulty_menu_open(DifficultyMenuState& state, bool hardcore_unlocked) noexcept {
    state.hardcore_unlocked = hardcore_unlocked;
    state.selected = static_cast<int>(DifficultyMenuItem::Medio);  // §2.1
    state.confirming = false;
    state.confirm_selected = 1;  // default seguro = Cancelar
}

DifficultyMenuAction difficulty_menu_key_down(DifficultyMenuState& state,
                                               SDL_Keycode key) noexcept {
    if (state.confirming) {
        if (key == SDLK_ESCAPE) {
            // Esc no splash = "Cancelar" (MESMA seguranca de confirming_new_game
            // em title_menu.hpp) - fecha o splash, volta pra lista.
            state.confirming = false;
            state.confirm_selected = 1;
            return DifficultyMenuAction::None;
        }
        if (is_axis_key(key)) {
            state.confirm_selected = (state.confirm_selected == 0) ? 1 : 0;
            return DifficultyMenuAction::None;
        }
        if (is_confirm_key(key)) {
            const bool confirmed = (state.confirm_selected == 0);
            state.confirming = false;
            state.confirm_selected = 1;
            return confirmed ? DifficultyMenuAction::Chosen
                              : DifficultyMenuAction::None;
        }
        return DifficultyMenuAction::None;
    }

    if (is_up_key(key)) {
        // Wrap-around SIMPLES (sem pular BLOQUEADOS - Hardcore continua
        // visitavel, ver difficulty_item_selectable no header).
        state.selected = (state.selected - 1 + kDifficultyItemCount) %
                          kDifficultyItemCount;
        return DifficultyMenuAction::None;
    }
    if (is_down_key(key)) {
        state.selected = (state.selected + 1) % kDifficultyItemCount;
        return DifficultyMenuAction::None;
    }
    if (is_confirm_key(key)) {
        if (!difficulty_item_selectable(state, state.selected)) {
            // Hardcore BLOQUEADO nesta Fase 0: no-op TOTAL (nao abre o splash,
            // MESMO padrao de "Continuar" desabilitado em title_menu.hpp).
            return DifficultyMenuAction::None;
        }
        // Selecionar dispara o Aviso #2 (splash confirmar/cancelar, §2.2) - a
        // dificuldade so e devolvida ao CHAMADOR depois do splash confirmado.
        state.confirming = true;
        state.confirm_selected = 1;  // default seguro
        return DifficultyMenuAction::None;
    }
    if (key == SDLK_ESCAPE) {
        // GAP preenchido (ver o comentario grande no header): ESC na lista aborta
        // Novo Jogo, volta pra tela de titulo.
        return DifficultyMenuAction::Cancelled;
    }
    return DifficultyMenuAction::None;
}

DifficultyMenuAction difficulty_menu_click_option(DifficultyMenuState& state,
                                                   int index) noexcept {
    if (state.confirming) {
        if (index != 0 && index != 1) return DifficultyMenuAction::None;
        state.confirm_selected = index;
        state.confirming = false;
        return (index == 0) ? DifficultyMenuAction::Chosen
                             : DifficultyMenuAction::None;
    }

    if (!difficulty_item_selectable(state, index)) {
        // Hardcore BLOQUEADO (ou index fora do intervalo): no-op TOTAL, nem o
        // foco muda (MESMO padrao de title_menu_click_option pra "Continuar"
        // desabilitado).
        return DifficultyMenuAction::None;
    }
    state.selected = index;
    state.confirming = true;
    state.confirm_selected = 1;
    return DifficultyMenuAction::None;
}

int difficulty_keyboard_focus_index(const DifficultyMenuState& state) noexcept {
    return state.confirming ? state.confirm_selected : state.selected;
}

int difficulty_hover_index(const DifficultyMenuState& state, float mouse_x,
                            float mouse_y,
                            const UiHoverBox boxes[kDifficultyItemCount]) noexcept {
    const int count = state.confirming ? 2 : kDifficultyItemCount;
    return ui_hover_index(mouse_x, mouse_y, boxes, count);
}

}  // namespace gus::app::screens
