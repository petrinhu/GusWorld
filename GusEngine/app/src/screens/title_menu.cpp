// gus/app/src/screens/title_menu.cpp
//
// Implementacao da logica pura da TELA DE TITULO. Ver header para o contrato
// completo. Travada por app/tests/title_menu_test.cpp (TEST-FIRST).

#include "gus/app/screens/title_menu.hpp"

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

// Proximo indice SELECIONAVEL a partir de `from`, andando em `step` (+1/-1) com
// wrap-around. Devolve `from` inalterado se NENHUM item for selecionavel (nunca
// deveria acontecer aqui - Novo Jogo e Sair sao SEMPRE selecionaveis, ver
// title_item_selectable - mas defensivo, MESMA receita de next_selectable em
// save_load_menu.cpp).
int next_selectable(const TitleMenuState& state, int from, int step) noexcept {
    for (int i = 0; i < kTitleItemCount; ++i) {
        from = (from + step + kTitleItemCount) % kTitleItemCount;
        if (title_item_selectable(state, from)) return from;
    }
    return state.selected;
}

}  // namespace

bool title_item_selectable(const TitleMenuState& state, int index) noexcept {
    if (index < 0 || index >= kTitleItemCount) return false;
    if (static_cast<TitleMenuItem>(index) == TitleMenuItem::Continue) {
        return state.any_save_exists;
    }
    return true;  // Novo Jogo / Sair sempre selecionaveis
}

void title_menu_open(TitleMenuState& state, bool any_save_exists) noexcept {
    state.any_save_exists = any_save_exists;
    state.confirming_new_game = false;
    state.confirm_selected = 1;
    // Foco inicial = Continuar SE selecionavel (mock: item "sel" por padrao),
    // senao o proximo selecionavel a partir dele (Novo Jogo, indice 1) - reusa
    // next_selectable partindo de Continue-1 andando +1, que ja pula Continuar
    // quando desabilitado.
    state.selected = static_cast<int>(TitleMenuItem::Continue);
    if (!title_item_selectable(state, state.selected)) {
        state.selected = next_selectable(state, state.selected, +1);
    }
}

TitleMenuAction title_menu_key_down(TitleMenuState& state, SDL_Keycode key) noexcept {
    if (state.confirming_new_game) {
        if (key == SDLK_ESCAPE) {
            // Esc no mini-dialogo = "Nao" (MESMA seguranca de
            // controls_confirming_discard/confirming_overwrite).
            state.confirming_new_game = false;
            state.confirm_selected = 1;
            return TitleMenuAction::None;
        }
        if (is_axis_key(key)) {
            state.confirm_selected = (state.confirm_selected == 0) ? 1 : 0;
            return TitleMenuAction::None;
        }
        if (is_confirm_key(key)) {
            const bool yes = (state.confirm_selected == 0);
            state.confirming_new_game = false;
            state.confirm_selected = 1;
            return yes ? TitleMenuAction::StartNewGame : TitleMenuAction::None;
        }
        return TitleMenuAction::None;
    }

    if (is_up_key(key)) {
        state.selected = next_selectable(state, state.selected, -1);
        return TitleMenuAction::None;
    }
    if (is_down_key(key)) {
        state.selected = next_selectable(state, state.selected, +1);
        return TitleMenuAction::None;
    }

    if (is_confirm_key(key)) {
        if (!title_item_selectable(state, state.selected)) {
            return TitleMenuAction::None;  // defensivo (nao deveria acontecer)
        }
        switch (static_cast<TitleMenuItem>(state.selected)) {
            case TitleMenuItem::Continue:
                return TitleMenuAction::ContinueGame;
            case TitleMenuItem::NewGame:
                if (state.any_save_exists) {
                    // Ha save gravado: pede confirmacao antes de comecar (decisao do
                    // lider, mesma mecanica de confirming_overwrite).
                    state.confirming_new_game = true;
                    state.confirm_selected = 1;  // default seguro = Nao
                    return TitleMenuAction::None;
                }
                return TitleMenuAction::StartNewGame;  // sem save nenhum: comeca direto
            case TitleMenuItem::Quit:
                return TitleMenuAction::RequestQuit;
        }
        return TitleMenuAction::None;
    }

    return TitleMenuAction::None;  // ESC (raiz, sem pai) e qualquer outra tecla: no-op
}

TitleMenuAction title_menu_click_option(TitleMenuState& state, int index) noexcept {
    if (state.confirming_new_game) {
        if (index != 0 && index != 1) return TitleMenuAction::None;
        state.confirm_selected = index;
        state.confirming_new_game = false;
        if (index == 0) {  // Sim, comecar
            return TitleMenuAction::StartNewGame;
        }
        return TitleMenuAction::None;  // Nao: cancela
    }

    if (!title_item_selectable(state, index)) {
        return TitleMenuAction::None;  // item desabilitado (Continuar sem save): no-op TOTAL
    }
    state.selected = index;
    switch (static_cast<TitleMenuItem>(index)) {
        case TitleMenuItem::Continue:
            return TitleMenuAction::ContinueGame;
        case TitleMenuItem::NewGame:
            if (state.any_save_exists) {
                state.confirming_new_game = true;
                state.confirm_selected = 1;
                return TitleMenuAction::None;
            }
            return TitleMenuAction::StartNewGame;
        case TitleMenuItem::Quit:
            return TitleMenuAction::RequestQuit;
    }
    return TitleMenuAction::None;
}

int title_keyboard_focus_index(const TitleMenuState& state) noexcept {
    return state.confirming_new_game ? state.confirm_selected : state.selected;
}

int title_hover_index(const TitleMenuState& state, float mouse_x, float mouse_y,
                       const UiHoverBox boxes[kTitleItemCount]) noexcept {
    const int count = state.confirming_new_game ? 2 : kTitleItemCount;
    return ui_hover_index(mouse_x, mouse_y, boxes, count);
}

}  // namespace gus::app::screens
