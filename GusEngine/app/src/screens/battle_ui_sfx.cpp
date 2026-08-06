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

// SFX-COCKPIT-AJUSTES (decisao do lider, 2026-08-06). A tabela inteira, em 4 linhas:
//   abertura ("Encarar")            -> Opening   (timbre em kBattleOpeningSfxSlot)
//   picker de ator / mira           -> Activated (o alvo/membro existe: confirmar aciona)
//   menu de verbos, verbo COM AP    -> Activated
//   menu de verbos, verbo SEM AP    -> Blocked   <- a mudanca: antes saia o CLIQUE normal,
//       ou seja, o jogador ouvia a confirmacao de uma acao que menu_confirm() recusou
//       (battle_scene.cpp: "verbo sem AP: confirm e no-op").
//   nada realcado (turno do inimigo, acao em voo, combate acabado) -> None (Enter cai em
//       skip(), que ACELERA o ritmo e nao e' botao).
BattleClickOutcome battle_confirm_outcome(bool is_intro, BattleFocusSurface surface,
                                          bool selected_enabled) noexcept {
    if (is_intro) {
        // DOMINA de proposito: na abertura battle_focus_of devolve None (ninguem navega),
        // entao sem esta guarda o "Encarar" cairia no ramo mudo la embaixo.
        return BattleClickOutcome::Opening;
    }
    switch (surface) {
        case BattleFocusSurface::VerbMenu:
            return selected_enabled ? BattleClickOutcome::Activated
                                    : BattleClickOutcome::Blocked;
        case BattleFocusSurface::Aiming:
        case BattleFocusSurface::ActorPicker:
            // Estas 2 superficies nao tem item "desabilitado": o cursor so pousa em
            // candidato vivo/elegivel (aim_target/actor_pick_target ja filtram). Se um dia
            // tiverem, o `selected_enabled` daqui passa a valer pra elas tambem.
            return BattleClickOutcome::Activated;
        case BattleFocusSurface::None:
            return BattleClickOutcome::None;
    }
    return BattleClickOutcome::None;
}

void BattleUiSfx::bind(gus::platform::audio::AudioEngine* audio,
                       gus::platform::audio::SoundId hover_sfx,
                       gus::platform::audio::SoundId click_sfx,
                       gus::platform::audio::SoundId blocked_sfx,
                       gus::platform::audio::SoundId confirm_sfx) noexcept {
    audio_ = audio;
    hover_sfx_ = hover_sfx;
    click_sfx_ = click_sfx;
    blocked_sfx_ = blocked_sfx;
    confirm_sfx_ = confirm_sfx;
}

gus::platform::audio::SoundId BattleUiSfx::sound_of(BattleUiSfxSlot slot) const noexcept {
    switch (slot) {
        case BattleUiSfxSlot::Hover: return hover_sfx_;
        case BattleUiSfxSlot::Click: return click_sfx_;
        case BattleUiSfxSlot::Blocked: return blocked_sfx_;
        case BattleUiSfxSlot::Confirm: return confirm_sfx_;
    }
    return gus::platform::audio::kInvalidSound;
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

bool BattleUiSfx::play_slot(BattleUiSfxSlot slot) { return play_(sound_of(slot)); }

bool BattleUiSfx::play_outcome(BattleClickOutcome outcome) {
    switch (outcome) {
        case BattleClickOutcome::Activated: return play_slot(BattleUiSfxSlot::Click);
        case BattleClickOutcome::Blocked: return play_slot(BattleUiSfxSlot::Blocked);
        // O timbre da ABERTURA e' um DADO (kBattleOpeningSfxSlot, ver o header): o lider
        // troca aquela constante e nada aqui muda.
        case BattleClickOutcome::Opening: return play_slot(kBattleOpeningSfxSlot);
        case BattleClickOutcome::None: return false;
    }
    return false;
}

}  // namespace gus::app::screens
