// gus/app/src/screens/system_menu.cpp
//
// Implementacao da logica pura do menu de sistema (MENU-PAUSA-CONFIG-SOM,
// onda arvore). Ver header. Travada por app/tests/system_menu_test.cpp
// (TEST-FIRST).

#include "gus/app/screens/system_menu.hpp"

#include <algorithm>  // std::clamp

#include "gus/app/screens/ui_hover.hpp"  // COCKPIT-SFX-HOVER-CLIQUE: POCO de hover generico (delegacao)

namespace gus::app::screens {

namespace {

// Navega um indice [0, count) com WRAP (delta=+1 ou -1).
int wrap_move(int current, int delta, int count) noexcept {
    int next = (current + delta) % count;
    if (next < 0) next += count;
    return next;
}

}  // namespace

SystemMenuScreen parent_screen_of(SystemMenuScreen screen) noexcept {
    switch (screen) {
        case SystemMenuScreen::Save:
        case SystemMenuScreen::ConfigCategories:
            return SystemMenuScreen::Pause;
        case SystemMenuScreen::Audio:
        case SystemMenuScreen::Video:
        case SystemMenuScreen::Language:
            return SystemMenuScreen::ConfigCategories;
        case SystemMenuScreen::Hidden:
        case SystemMenuScreen::Pause:
            return screen;  // sem pai na arvore (defensivo - nao deveria ser chamado aqui)
    }
    return screen;
}

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
            // Pause e a RAIZ da arvore - ESC (footer: "ESC volta ao jogo") FECHA o
            // menu inteiro, mesmo efeito de confirmar Continuar, de qualquer item
            // selecionado. Diferente de qualquer outra tela (ESC la SOBE 1 nivel).
            return SystemMenuAction::Continue;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_SPACE: {
            switch (static_cast<PauseItem>(state.pause_selected)) {
                case PauseItem::Continue:
                    return SystemMenuAction::Continue;
                case PauseItem::Save:
                    state.screen = SystemMenuScreen::Save;
                    return SystemMenuAction::Navigated;
                case PauseItem::Settings:
                    state.screen = SystemMenuScreen::ConfigCategories;
                    state.config_categories_selected =
                        static_cast<int>(ConfigCategoryItem::Audio);
                    return SystemMenuAction::Navigated;
                case PauseItem::Quit:
                    return SystemMenuAction::RequestQuit;
            }
            return SystemMenuAction::None;
        }
        default:
            return SystemMenuAction::None;
    }
}

SystemMenuAction handle_config_categories_key(SystemMenuState& state,
                                               SDL_Keycode key) noexcept {
    switch (key) {
        case SDLK_UP:
        case SDLK_W:
            state.config_categories_selected =
                wrap_move(state.config_categories_selected, -1, kConfigCategoriesItemCount);
            return SystemMenuAction::None;
        case SDLK_DOWN:
        case SDLK_S:
            state.config_categories_selected =
                wrap_move(state.config_categories_selected, +1, kConfigCategoriesItemCount);
            return SystemMenuAction::None;
        case SDLK_ESCAPE:
            state.screen = parent_screen_of(SystemMenuScreen::ConfigCategories);  // Pause
            return SystemMenuAction::Navigated;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_SPACE: {
            switch (static_cast<ConfigCategoryItem>(state.config_categories_selected)) {
                case ConfigCategoryItem::Audio:
                    state.screen = SystemMenuScreen::Audio;
                    state.audio_selected = static_cast<int>(AudioItem::Music);
                    return SystemMenuAction::Navigated;
                case ConfigCategoryItem::Video:
                    state.screen = SystemMenuScreen::Video;
                    return SystemMenuAction::Navigated;
                case ConfigCategoryItem::Language:
                    state.screen = SystemMenuScreen::Language;
                    return SystemMenuAction::Navigated;
                case ConfigCategoryItem::Back:
                    state.screen = parent_screen_of(SystemMenuScreen::ConfigCategories);
                    return SystemMenuAction::Navigated;
            }
            return SystemMenuAction::None;
        }
        default:
            return SystemMenuAction::None;
    }
}

SystemMenuAction handle_audio_key(SystemMenuState& state, SDL_Keycode key) noexcept {
    switch (key) {
        case SDLK_UP:
        case SDLK_W:
            state.audio_selected = wrap_move(state.audio_selected, -1, kAudioItemCount);
            return SystemMenuAction::None;
        case SDLK_DOWN:
        case SDLK_S:
            state.audio_selected = wrap_move(state.audio_selected, +1, kAudioItemCount);
            return SystemMenuAction::None;
        case SDLK_ESCAPE:
            state.screen = parent_screen_of(SystemMenuScreen::Audio);  // ConfigCategories
            return SystemMenuAction::Navigated;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_SPACE:
            if (static_cast<AudioItem>(state.audio_selected) == AudioItem::Back) {
                state.screen = parent_screen_of(SystemMenuScreen::Audio);
                return SystemMenuAction::Navigated;
            }
            return SystemMenuAction::None;  // ENTER num slider nao faz nada
        case SDLK_LEFT:
        case SDLK_A:
        case SDLK_RIGHT:
        case SDLK_D: {
            // WASD completo (pedido do lider, MENU-PAUSA-CONFIG-SOM): A=D no
            // eixo horizontal do Audio espelha LEFT/RIGHT (A=esquerda/diminui,
            // D=direita/aumenta) - mesmo par que W/S ja espelha UP/DOWN acima.
            // Nas demais telas nao ha eixo horizontal - A/D cai no default=None.
            const float delta =
                (key == SDLK_LEFT || key == SDLK_A) ? -kVolumeStep : kVolumeStep;
            switch (static_cast<AudioItem>(state.audio_selected)) {
                case AudioItem::Music:
                    state.music_volume =
                        std::clamp(state.music_volume + delta, 0.0f, 1.0f);
                    return SystemMenuAction::VolumeChanged;
                case AudioItem::Sfx:
                    state.sfx_volume =
                        std::clamp(state.sfx_volume + delta, 0.0f, 1.0f);
                    return SystemMenuAction::VolumeChanged;
                case AudioItem::Back:
                    return SystemMenuAction::None;  // Voltar nao e slider
            }
            return SystemMenuAction::None;
        }
        default:
            return SystemMenuAction::None;
    }
}

// Telas placeholder (Save/Video/Language): 1 unico item (Voltar). Sem eixo de
// navegacao (UP/DOWN/A/D no-op - nao ha entre-o-que escolher). ESC/ENTER/SPACE
// sobem pro pai (parent_screen_of).
SystemMenuAction handle_placeholder_key(SystemMenuState& state, SDL_Keycode key) noexcept {
    switch (key) {
        case SDLK_ESCAPE:
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_SPACE:
            state.screen = parent_screen_of(state.screen);
            return SystemMenuAction::Navigated;
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
        case SystemMenuScreen::ConfigCategories:
            return handle_config_categories_key(state, key);
        case SystemMenuScreen::Audio:
            return handle_audio_key(state, key);
        case SystemMenuScreen::Save:
        case SystemMenuScreen::Video:
        case SystemMenuScreen::Language:
            return handle_placeholder_key(state, key);
    }
    return SystemMenuAction::None;
}

void system_menu_set_slider_ratio(SystemMenuState& state, int item,
                                   float ratio) noexcept {
    const float clamped = std::clamp(ratio, 0.0f, 1.0f);
    if (item == static_cast<int>(AudioItem::Music)) {
        state.music_volume = clamped;
    } else if (item == static_cast<int>(AudioItem::Sfx)) {
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
        case PauseItem::Save:
            state.screen = SystemMenuScreen::Save;
            return SystemMenuAction::Navigated;
        case PauseItem::Settings:
            state.screen = SystemMenuScreen::ConfigCategories;
            state.config_categories_selected = static_cast<int>(ConfigCategoryItem::Audio);
            return SystemMenuAction::Navigated;
        case PauseItem::Quit:
            return SystemMenuAction::RequestQuit;
    }
    return SystemMenuAction::None;
}

SystemMenuAction click_config_categories_option(SystemMenuState& state, int index) noexcept {
    if (index < 0 || index >= kConfigCategoriesItemCount) return SystemMenuAction::None;
    state.config_categories_selected = index;
    // MESMA logica de handle_config_categories_key/SDLK_RETURN - categorias sao
    // botoes simples (nao sliders), clicar SEMPRE confirma na hora.
    switch (static_cast<ConfigCategoryItem>(index)) {
        case ConfigCategoryItem::Audio:
            state.screen = SystemMenuScreen::Audio;
            state.audio_selected = static_cast<int>(AudioItem::Music);
            return SystemMenuAction::Navigated;
        case ConfigCategoryItem::Video:
            state.screen = SystemMenuScreen::Video;
            return SystemMenuAction::Navigated;
        case ConfigCategoryItem::Language:
            state.screen = SystemMenuScreen::Language;
            return SystemMenuAction::Navigated;
        case ConfigCategoryItem::Back:
            state.screen = parent_screen_of(SystemMenuScreen::ConfigCategories);
            return SystemMenuAction::Navigated;
    }
    return SystemMenuAction::None;
}

SystemMenuAction click_audio_option(SystemMenuState& state, int index) noexcept {
    if (index < 0 || index >= kAudioItemCount) return SystemMenuAction::None;
    state.audio_selected = index;
    if (static_cast<AudioItem>(index) == AudioItem::Back) {
        // MESMA logica de handle_audio_key/SDLK_RETURN em Voltar - confirma na
        // hora (Navigated pro pai).
        state.screen = parent_screen_of(SystemMenuScreen::Audio);
        return SystemMenuAction::Navigated;
    }
    // Musica/SFX: clicar no NOME/rotulo so foca (audio_selected ja mudou
    // acima) - o volume so muda por drag/clique no TRACK
    // (system_menu_set_slider_ratio), nao por clicar no rotulo.
    return SystemMenuAction::None;
}

SystemMenuAction click_placeholder_option(SystemMenuState& state, int index) noexcept {
    if (index != kPlaceholderBackIndex) return SystemMenuAction::None;
    state.screen = parent_screen_of(state.screen);
    return SystemMenuAction::Navigated;
}

}  // namespace

SystemMenuAction system_menu_click_option(SystemMenuState& state,
                                           int index) noexcept {
    switch (state.screen) {
        case SystemMenuScreen::Hidden:
            return SystemMenuAction::None;  // menu fechado: no-op defensivo
        case SystemMenuScreen::Pause:
            return click_pause_option(state, index);
        case SystemMenuScreen::ConfigCategories:
            return click_config_categories_option(state, index);
        case SystemMenuScreen::Audio:
            return click_audio_option(state, index);
        case SystemMenuScreen::Save:
        case SystemMenuScreen::Video:
        case SystemMenuScreen::Language:
            return click_placeholder_option(state, index);
    }
    return SystemMenuAction::None;
}

int system_menu_hover_index(const SystemMenuState& state, float mouse_x, float mouse_y,
                             const SystemMenuHoverBox boxes[kSystemMenuMaxHoverItems]) noexcept {
    int count = 0;
    switch (state.screen) {
        case SystemMenuScreen::Pause:
            count = kPauseItemCount;
            break;
        case SystemMenuScreen::ConfigCategories:
            count = kConfigCategoriesItemCount;
            break;
        case SystemMenuScreen::Audio:
            count = kAudioItemCount;
            break;
        case SystemMenuScreen::Save:
        case SystemMenuScreen::Video:
        case SystemMenuScreen::Language:
            count = kPlaceholderItemCount;
            break;
        case SystemMenuScreen::Hidden:
            return -1;  // menu fechado: nenhum item hover-testavel
    }
    // COCKPIT-SFX-HOVER-CLIQUE: DELEGA a decisao geometrica pro POCO generico
    // (ui_hover.hpp) - o menu so decide QUANTAS caixas sao relevantes (count, pela
    // tela). SystemMenuHoverBox e UiHoverBox tem os MESMOS campos; converte in-place.
    UiHoverBox generic[kSystemMenuMaxHoverItems];
    for (int i = 0; i < count; ++i) {
        generic[i] = UiHoverBox{boxes[i].found, boxes[i].x, boxes[i].y, boxes[i].w,
                                boxes[i].h};
    }
    return ui_hover_index(mouse_x, mouse_y, generic, count);
}

bool system_menu_hover_entered_new_item(int previous_index, int current_index) noexcept {
    // COCKPIT-SFX-HOVER-CLIQUE: edge-detect e identico entre menu e cockpit -> delega.
    return ui_hover_entered_new_item(previous_index, current_index);
}

}  // namespace gus::app::screens
