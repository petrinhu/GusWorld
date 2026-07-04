// gus/app/src/screens/system_menu.cpp
//
// Implementacao da logica pura do menu de sistema (MENU-PAUSA-CONFIG-SOM). Ver
// header. Travada por app/tests/system_menu_test.cpp (TEST-FIRST).

#include "gus/app/screens/system_menu.hpp"

#include <algorithm>  // std::clamp

namespace gus::app::screens {

namespace {

// Navega um indice [0, count) com WRAP (delta=+1 ou -1).
int wrap_move(int current, int delta, int count) noexcept {
    int next = (current + delta) % count;
    if (next < 0) next += count;
    return next;
}

}  // namespace

void system_menu_open(SystemMenuState& state) noexcept {
    state.screen = SystemMenuScreen::Pause;
    state.pause_selected = static_cast<int>(PauseItem::Continue);
}

void system_menu_close(SystemMenuState& state) noexcept {
    state.screen = SystemMenuScreen::Hidden;
}

namespace {

SystemMenuAction handle_pause_key(SystemMenuState& state, SDL_Keycode key) noexcept {
    switch (key) {
        case SDLK_UP:
        case SDLK_W:
            state.pause_selected =
                wrap_move(state.pause_selected, -1, kPauseItemCount);
            return SystemMenuAction::None;
        case SDLK_DOWN:
        case SDLK_S:
            state.pause_selected =
                wrap_move(state.pause_selected, +1, kPauseItemCount);
            return SystemMenuAction::None;
        case SDLK_ESCAPE:
            // Footer do mock: "ESC volta ao jogo" - mesmo efeito de confirmar
            // Continuar, de qualquer item selecionado.
            return SystemMenuAction::Continue;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_SPACE: {
            switch (static_cast<PauseItem>(state.pause_selected)) {
                case PauseItem::Continue:
                    return SystemMenuAction::Continue;
                case PauseItem::Settings:
                    state.screen = SystemMenuScreen::Config;
                    state.config_selected = static_cast<int>(ConfigItem::Music);
                    return SystemMenuAction::OpenSettings;
                case PauseItem::Quit:
                    return SystemMenuAction::RequestQuit;
            }
            return SystemMenuAction::None;
        }
        default:
            return SystemMenuAction::None;
    }
}

SystemMenuAction handle_config_key(SystemMenuState& state, SDL_Keycode key) noexcept {
    switch (key) {
        case SDLK_UP:
        case SDLK_W:
            state.config_selected =
                wrap_move(state.config_selected, -1, kConfigItemCount);
            return SystemMenuAction::None;
        case SDLK_DOWN:
        case SDLK_S:
            state.config_selected =
                wrap_move(state.config_selected, +1, kConfigItemCount);
            return SystemMenuAction::None;
        case SDLK_ESCAPE:
            state.screen = SystemMenuScreen::Pause;
            return SystemMenuAction::BackToPause;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_SPACE:
            if (static_cast<ConfigItem>(state.config_selected) == ConfigItem::Back) {
                state.screen = SystemMenuScreen::Pause;
                return SystemMenuAction::BackToPause;
            }
            return SystemMenuAction::None;  // ENTER num slider nao faz nada
        case SDLK_LEFT:
        case SDLK_A:
        case SDLK_RIGHT:
        case SDLK_D: {
            // WASD completo (pedido do lider, MENU-PAUSA-CONFIG-SOM): A=D no
            // eixo horizontal do Config espelha LEFT/RIGHT (A=esquerda/diminui,
            // D=direita/aumenta) - mesmo par que W/S ja espelha UP/DOWN acima.
            // No Pause nao ha eixo horizontal (sem case LEFT/RIGHT/A/D la) -
            // A/D cai no default=None, exatamente como o pedido permite.
            const float delta =
                (key == SDLK_LEFT || key == SDLK_A) ? -kVolumeStep : kVolumeStep;
            switch (static_cast<ConfigItem>(state.config_selected)) {
                case ConfigItem::Music:
                    state.music_volume =
                        std::clamp(state.music_volume + delta, 0.0f, 1.0f);
                    return SystemMenuAction::VolumeChanged;
                case ConfigItem::Sfx:
                    state.sfx_volume =
                        std::clamp(state.sfx_volume + delta, 0.0f, 1.0f);
                    return SystemMenuAction::VolumeChanged;
                case ConfigItem::Back:
                    return SystemMenuAction::None;  // Voltar nao e slider
            }
            return SystemMenuAction::None;
        }
        default:
            return SystemMenuAction::None;
    }
}

}  // namespace

SystemMenuAction system_menu_key_down(SystemMenuState& state,
                                       SDL_Keycode key) noexcept {
    switch (state.screen) {
        case SystemMenuScreen::Hidden:
            return SystemMenuAction::None;  // menu fechado: no-op defensivo
        case SystemMenuScreen::Pause:
            return handle_pause_key(state, key);
        case SystemMenuScreen::Config:
            return handle_config_key(state, key);
    }
    return SystemMenuAction::None;
}

void system_menu_set_slider_ratio(SystemMenuState& state, int item,
                                   float ratio) noexcept {
    const float clamped = std::clamp(ratio, 0.0f, 1.0f);
    if (item == static_cast<int>(ConfigItem::Music)) {
        state.music_volume = clamped;
    } else if (item == static_cast<int>(ConfigItem::Sfx)) {
        state.sfx_volume = clamped;
    }
    // item==Back (ou qualquer outro) e no-op: nao e um slider.
}

namespace {

SystemMenuAction click_pause_option(SystemMenuState& state, int index) noexcept {
    if (index < 0 || index >= kPauseItemCount) return SystemMenuAction::None;
    state.pause_selected = index;
    // MESMA logica de handle_pause_key/SDLK_RETURN acima - clicar numa pill do
    // Pause SEMPRE confirma na hora (nao ha estado "so foco, sem confirmar").
    switch (static_cast<PauseItem>(index)) {
        case PauseItem::Continue:
            return SystemMenuAction::Continue;
        case PauseItem::Settings:
            state.screen = SystemMenuScreen::Config;
            state.config_selected = static_cast<int>(ConfigItem::Music);
            return SystemMenuAction::OpenSettings;
        case PauseItem::Quit:
            return SystemMenuAction::RequestQuit;
    }
    return SystemMenuAction::None;
}

SystemMenuAction click_config_option(SystemMenuState& state, int index) noexcept {
    if (index < 0 || index >= kConfigItemCount) return SystemMenuAction::None;
    state.config_selected = index;
    if (static_cast<ConfigItem>(index) == ConfigItem::Back) {
        // MESMA logica de handle_config_key/SDLK_RETURN em Voltar - confirma na
        // hora (BackToPause).
        state.screen = SystemMenuScreen::Pause;
        return SystemMenuAction::BackToPause;
    }
    // Musica/SFX: clicar no NOME/rotulo so foca (config_selected ja mudou
    // acima) - o volume so muda por drag/clique no TRACK
    // (system_menu_set_slider_ratio), nao por clicar no rotulo.
    return SystemMenuAction::None;
}

}  // namespace

SystemMenuAction system_menu_click_option(SystemMenuState& state,
                                           int index) noexcept {
    switch (state.screen) {
        case SystemMenuScreen::Hidden:
            return SystemMenuAction::None;  // menu fechado: no-op defensivo
        case SystemMenuScreen::Pause:
            return click_pause_option(state, index);
        case SystemMenuScreen::Config:
            return click_config_option(state, index);
    }
    return SystemMenuAction::None;
}

}  // namespace gus::app::screens
