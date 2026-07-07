// gus/app/src/screens/system_menu.cpp
//
// Implementacao da logica pura do menu de sistema (MENU-PAUSA-CONFIG-SOM,
// onda arvore). Ver header. Travada por app/tests/system_menu_test.cpp
// (TEST-FIRST).

#include "gus/app/screens/system_menu.hpp"

#include <algorithm>  // std::clamp

#include "gus/app/screens/ui_hover.hpp"  // COCKPIT-SFX-HOVER-CLIQUE: POCO de hover generico (delegacao)
#include "gus/domain/input/action_registry.hpp"        // ActionRegistry (label i18n do swap)
#include "gus/domain/input/controls_remap_apply.hpp"   // apply_key_remap (swap-on-conflict)
#include "gus/domain/input/controls_restore.hpp"       // default_controls (Restaurar padrao)

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
        case SystemMenuScreen::Controls:
        case SystemMenuScreen::Language:
            return SystemMenuScreen::ConfigCategories;
        case SystemMenuScreen::Hidden:
        case SystemMenuScreen::Pause:
            return screen;  // sem pai na arvore (defensivo - nao deveria ser chamado aqui)
    }
    return screen;
}

namespace {

// Ordem CURADA/AGRUPADA das 30 actions na tela Controles (Movimento/Mundo/
// Combate/Menu&Dialogo - MESMOS grupos do mock docs/design/mockups/
// 06-controles-remap.html, estendidos para cobrir as 30 actions do
// ActionRegistry - o mock so ilustrava um subconjunto). Fonte UNICA da ordem:
// controls_selected (0..kControlsActionCount-1) indexa este array; render
// (system_menu_rml.cpp) e captura (system_menu_controls_capture_key) usam a
// MESMA fonte, nunca duplicam a lista.
struct ControlsRow {
    const char* action_name;
    int group;  // 0=Movimento, 1=Mundo, 2=Combate, 3=Menu & Dialogo
};

constexpr ControlsRow kControlsRows[kControlsActionCount] = {
    // Movimento (5) - ActionCategory::Movement do registry.
    {"move_forward", 0},
    {"move_backward", 0},
    {"move_left", 0},
    {"move_right", 0},
    {"move_run", 0},
    // Mundo (4) - Interact + Inventory + Diary do registry, agrupados pelo mock
    // sob "Mundo" (interagir/abrir inventario/abrir diario sao todas acoes de
    // MUNDO, nao de menu). inventory_close somado (mock so ilustrava
    // inventory_open) - decisao de cobertura completa das 30 actions.
    {"interact", 1},
    {"inventory_open", 1},
    {"inventory_close", 1},
    {"diary_open", 1},
    // Combate (7) - ActionCategory::Combat completo (mock so ilustrava 4 das
    // 7; card_1/2/3 somadas aqui pela mesma razao de cobertura completa).
    {"combat_attack_basic", 2},
    {"combat_defend", 2},
    {"combat_cast", 2},
    {"combat_card_1", 2},
    {"combat_card_2", 2},
    {"combat_card_3", 2},
    {"combat_end_turn", 2},
    // Menu & Dialogo (14) - ActionCategory::Menu + Dialogue completos (mock so
    // ilustrava 3 das 14; nav_up/down/left/right + open/close + choice_1..4
    // somadas pela mesma razao).
    {"menu_open", 3},
    {"menu_close", 3},
    {"menu_confirm", 3},
    {"menu_cancel", 3},
    {"menu_nav_up", 3},
    {"menu_nav_down", 3},
    {"menu_nav_left", 3},
    {"menu_nav_right", 3},
    {"dialogue_continue", 3},
    {"dialogue_skip", 3},
    {"dialogue_choice_1", 3},
    {"dialogue_choice_2", 3},
    {"dialogue_choice_3", 3},
    {"dialogue_choice_4", 3},
};

}  // namespace

std::string_view controls_action_name_at(int index) noexcept {
    if (index < 0 || index >= kControlsActionCount) return std::string_view{};
    return kControlsRows[index].action_name;
}

int controls_group_at(int index) noexcept {
    if (index < 0 || index >= kControlsActionCount) return -1;
    return kControlsRows[index].group;
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
                case ConfigCategoryItem::Controls:
                    state.screen = SystemMenuScreen::Controls;
                    state.controls_selected = 0;
                    state.controls_capturing = false;
                    state.controls_confirming_restore = false;
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

// Tela Controles (M2): navegacao NORMAL (30 actions curadas + Restaurar padrao +
// Voltar) e o sub-fluxo de CONFIRMACAO do restaurar-padrao (decisao 4 do lider:
// pede confirmacao antes de resetar). O MODO DE CAPTURA (controls_capturing==
// true) NAO e tratado aqui - o CHAMADOR (loop) roteia pra
// system_menu_controls_capture_key nesse caso (ver o header, o motivo de nao
// reusar este roteador generico: toda tecla vira candidata a binding).
SystemMenuAction handle_controls_key(SystemMenuState& state, SDL_Keycode key) noexcept {
    if (state.controls_capturing) {
        // Defensivo: o chamador nao deveria rotear aqui neste modo; no-op.
        return SystemMenuAction::None;
    }

    if (state.controls_confirming_restore) {
        switch (key) {
            case SDLK_UP:
            case SDLK_DOWN:
            case SDLK_LEFT:
            case SDLK_RIGHT:
            case SDLK_W:
            case SDLK_S:
            case SDLK_A:
            case SDLK_D:
                // 2 escolhas (Sim/Nao): qualquer eixo alterna entre elas.
                state.controls_restore_confirm_selected =
                    1 - state.controls_restore_confirm_selected;
                return SystemMenuAction::None;
            case SDLK_ESCAPE:
                state.controls_confirming_restore = false;  // cancela, sem mudar config
                return SystemMenuAction::None;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
            case SDLK_SPACE:
                state.controls_confirming_restore = false;
                if (state.controls_restore_confirm_selected == 0) {  // Sim
                    state.controls_config = gus::domain::input::default_controls();
                    state.controls_last_action_swapped = false;
                    state.controls_last_swapped_with_action.clear();
                    state.controls_last_swapped_with_label_key.clear();
                    return SystemMenuAction::ControlsChanged;
                }
                return SystemMenuAction::None;  // Nao: cancela sem mudar nada
            default:
                return SystemMenuAction::None;
        }
    }

    switch (key) {
        case SDLK_UP:
        case SDLK_W:
            state.controls_selected = wrap_move(state.controls_selected, -1, kControlsItemCount);
            state.controls_last_action_swapped = false;  // navegar limpa o aviso de troca
            return SystemMenuAction::None;
        case SDLK_DOWN:
        case SDLK_S:
            state.controls_selected = wrap_move(state.controls_selected, +1, kControlsItemCount);
            state.controls_last_action_swapped = false;
            return SystemMenuAction::None;
        case SDLK_ESCAPE:
            state.screen = parent_screen_of(SystemMenuScreen::Controls);  // ConfigCategories
            return SystemMenuAction::Navigated;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_SPACE:
            if (state.controls_selected < kControlsActionCount) {
                state.controls_capturing = true;
                state.controls_last_action_swapped = false;
                return SystemMenuAction::None;
            }
            if (state.controls_selected == kControlsRestoreIndex) {
                state.controls_confirming_restore = true;
                state.controls_restore_confirm_selected = 1;  // default seguro = Nao
                return SystemMenuAction::None;
            }
            // kControlsBackIndex
            state.screen = parent_screen_of(SystemMenuScreen::Controls);
            return SystemMenuAction::Navigated;
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
        case SystemMenuScreen::Controls:
            return handle_controls_key(state, key);
        case SystemMenuScreen::Save:
        case SystemMenuScreen::Video:
        case SystemMenuScreen::Language:
            return handle_placeholder_key(state, key);
    }
    return SystemMenuAction::None;
}

SystemMenuAction system_menu_controls_capture_key(SystemMenuState& state, bool is_escape,
                                                   long long godot_keycode) noexcept {
    if (!state.controls_capturing) {
        return SystemMenuAction::None;  // defensivo: nao deveria ser chamado fora do modo
    }
    if (is_escape) {
        state.controls_capturing = false;  // "Esc cancela a captura" (footer do mock)
        return SystemMenuAction::None;
    }
    if (godot_keycode == 0) {
        // Tecla sem correspondente conhecido (ver key_translation.hpp) - ignora,
        // permanece capturando (o jogador tenta outra tecla).
        return SystemMenuAction::None;
    }

    const std::string_view action_name = controls_action_name_at(state.controls_selected);
    if (action_name.empty()) {
        state.controls_capturing = false;  // defensivo: indice invalido
        return SystemMenuAction::None;
    }

    const gus::domain::input::KeyBinding new_key{
        .keycode = godot_keycode, .ctrl_pressed = false, .shift_pressed = false, .alt_pressed = false};
    const gus::domain::input::KeyRemapResult result =
        gus::domain::input::apply_key_remap(state.controls_config, std::string(action_name), new_key);

    state.controls_capturing = false;
    if (!result.changed) {
        // No-op (mesma tecla de antes): nada a persistir, sem aviso de troca.
        state.controls_last_action_swapped = false;
        state.controls_last_swapped_with_action.clear();
        state.controls_last_swapped_with_label_key.clear();
        return SystemMenuAction::None;
    }

    state.controls_config = result.config;
    state.controls_last_action_swapped = result.swapped;
    state.controls_last_swapped_with_action = result.swapped_with_action_name;
    state.controls_last_swapped_with_label_key = result.swapped_with_label_i18n_key;
    return SystemMenuAction::ControlsChanged;
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
        case ConfigCategoryItem::Controls:
            state.screen = SystemMenuScreen::Controls;
            state.controls_selected = 0;
            state.controls_capturing = false;
            state.controls_confirming_restore = false;
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

// Tela Controles (mouse): enquanto capturando, clique nao faz sentido (o
// jogador precisa apertar uma tecla FISICA - ver system_menu_controls_capture_key)
// - no-op. Enquanto confirmando o restaurar-padrao, `index` e reinterpretado
// como a escolha do prompt (0=Sim, 1=Nao - MESMA convencao de
// controls_restore_confirm_selected), clicavel nas 2 pills do mini-dialogo.
// Caso contrario, `index` e a lista normal (0..kControlsActionCount-1 =
// action, kControlsRestoreIndex/kControlsBackIndex = rodape) - clicar SEMPRE
// foca+confirma na hora (equivalente a focar + ENTER), MESMA convencao das
// outras telas.
SystemMenuAction click_controls_option(SystemMenuState& state, int index) noexcept {
    if (state.controls_capturing) return SystemMenuAction::None;

    if (state.controls_confirming_restore) {
        if (index != 0 && index != 1) return SystemMenuAction::None;
        state.controls_restore_confirm_selected = index;
        state.controls_confirming_restore = false;
        if (index == 0) {  // Sim
            state.controls_config = gus::domain::input::default_controls();
            state.controls_last_action_swapped = false;
            state.controls_last_swapped_with_action.clear();
            state.controls_last_swapped_with_label_key.clear();
            return SystemMenuAction::ControlsChanged;
        }
        return SystemMenuAction::None;  // Nao: cancela
    }

    if (index < 0 || index >= kControlsItemCount) return SystemMenuAction::None;
    state.controls_selected = index;
    state.controls_last_action_swapped = false;

    if (index < kControlsActionCount) {
        state.controls_capturing = true;
        return SystemMenuAction::None;
    }
    if (index == kControlsRestoreIndex) {
        state.controls_confirming_restore = true;
        state.controls_restore_confirm_selected = 1;  // default seguro = Nao
        return SystemMenuAction::None;
    }
    // kControlsBackIndex
    state.screen = parent_screen_of(SystemMenuScreen::Controls);
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
        case SystemMenuScreen::Controls:
            return click_controls_option(state, index);
        case SystemMenuScreen::Save:
        case SystemMenuScreen::Video:
        case SystemMenuScreen::Language:
            return click_placeholder_option(state, index);
    }
    return SystemMenuAction::None;
}

bool controls_row_visible_in_list(float row_top, float row_h, float list_top,
                                   float list_h) noexcept {
    // Sobreposicao de intervalos 1D [row_top, row_top+row_h) x [list_top,
    // list_top+list_h) - MESMA formula classica de overlap de segmentos (nao
    // ha overlap sse um termina antes do outro comecar). Qualquer sobreposicao
    // (mesmo parcial, ex. linha cortada na borda do recorte) conta como
    // "visivel o bastante pra interagir" - so linhas SEM NENHUMA sobreposicao
    // (ver BUG-A no header) sao excluidas.
    return row_top < (list_top + list_h) && (row_top + row_h) > list_top;
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
        case SystemMenuScreen::Controls:
            // Capturando: nada e hover-testavel (o jogador precisa apertar uma
            // tecla FISICA, o mouse nao participa desse modo). Confirmando o
            // restaurar-padrao: so as 2 pills do mini-dialogo (Sim/Nao, MESMA
            // convencao de indice de click_controls_option). Caso contrario: a
            // lista normal inteira (30 actions + Restaurar + Voltar).
            if (state.controls_capturing) {
                count = 0;
            } else if (state.controls_confirming_restore) {
                count = 2;
            } else {
                count = kControlsItemCount;
            }
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
