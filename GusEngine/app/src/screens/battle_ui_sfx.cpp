// SPDX-License-Identifier: Apache-2.0
// gus/app/src/screens/battle_ui_sfx.cpp
//
// SFX-COCKPIT: implementacao. Ver o header pro contrato completo e pro defeito que este
// modulo fecha (2 das 3 superficies clicaveis do cockpit eram MUDAS, e o teclado nao
// soava em nenhuma das 3).

#include "gus/app/screens/battle_ui_sfx.hpp"

namespace gus::app::screens {

bool operator==(const BattleFocus& a, const BattleFocus& b) noexcept {
    return a.surface == b.surface && a.index == b.index && a.item == b.item;
}

bool operator!=(const BattleFocus& a, const BattleFocus& b) noexcept { return !(a == b); }

BattleFocus battle_focus_of(const BattleScene& scene) noexcept {
    BattleFocus focus;
    // ABERTURA parada ("BATALHA!"): nada realcado, ninguem navega - a mesma guarda que
    // BattleScreen::handle_cockpit_hover_ ja tinha.
    if (scene.is_intro()) {
        return focus;
    }
    // PRIORIDADE identica a de battle_key_down/battle_mouse_click: picker > mira > menu.
    // Os dois primeiros nunca sao simultaneos (a mira so abre no menu de verbos, ja fora
    // do picker), mas a ordem deixa explicito e casa com o roteamento - se um dia isso
    // mudar, som e acao continuam falando da MESMA superficie.
    if (scene.is_choosing_actor()) {
        focus.surface = BattleFocusSurface::ActorPicker;
        focus.item = scene.actor_pick_target();
        return focus;
    }
    if (scene.is_aiming()) {
        focus.surface = BattleFocusSurface::Aiming;
        focus.item = scene.aim_target();
        return focus;
    }
    if (scene.waiting_player_input()) {
        focus.surface = BattleFocusSurface::VerbMenu;
        focus.index = scene.menu().selected_index();
        return focus;
    }
    // Turno do inimigo / acao em voo / combate acabado: nada interativo.
    return focus;
}

bool battle_focus_entered_new_item(const BattleFocus& before,
                                   const BattleFocus& after) noexcept {
    if (after.surface == BattleFocusSurface::None) {
        return false;
    }
    if (before.surface != after.surface) {
        return false;  // troca de superficie e' consequencia de confirmar/cancelar
    }
    if (after.index < 0 && after.item == nullptr) {
        return false;  // superficie aberta mas sem item realcado (ex.: lista vazia)
    }
    return before != after;
}

void BattleUiSfx::bind(gus::platform::audio::AudioEngine* audio,
                       gus::platform::audio::SoundId hover_sfx,
                       gus::platform::audio::SoundId click_sfx) noexcept {
    audio_ = audio;
    hover_sfx_ = hover_sfx;
    click_sfx_ = click_sfx;
}

bool BattleUiSfx::play_(gus::platform::audio::SoundId id) {
    // Degradacao graciosa em DOIS niveis: sem engine (host montou sem audio) e sem som
    // carregado (arquivo ausente - play_sfx(kInvalidSound) ja e no-op, mas checar aqui
    // deixa o retorno HONESTO: "nao tocou" em vez de "chamei o no-op").
    if (audio_ == nullptr || id == gus::platform::audio::kInvalidSound) {
        return false;
    }
    audio_->play_sfx(id);
    return true;
}

bool BattleUiSfx::on_focus_change(const BattleFocus& before, const BattleFocus& after) {
    if (!battle_focus_entered_new_item(before, after)) {
        return false;
    }
    return play_(hover_sfx_);
}

bool BattleUiSfx::on_pill_motion(float mouse_x, float mouse_y, const UiHoverBox* boxes,
                                 int count) {
    const int new_hover = ui_hover_index(mouse_x, mouse_y, boxes, count);
    const bool entered = ui_hover_entered_new_item(hovered_pill_, new_hover);
    hovered_pill_ = new_hover;
    if (!entered) {
        return false;
    }
    return play_(hover_sfx_);
}

void BattleUiSfx::reset_pill_hover() noexcept { hovered_pill_ = -1; }

bool BattleUiSfx::play_click() { return play_(click_sfx_); }

}  // namespace gus::app::screens
