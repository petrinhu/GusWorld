// gus/app/src/screens/battle_scene_aim.cpp
//
// AC-E11 A5 (TU-split, ADR-019): DEFINICOES dos metodos do modo-mira/target selection
// (§3.5, D3) de BattleScene - extraidos de battle_scene.cpp (que ficou grande demais).
// PURO TU-split: o header (battle_scene.hpp) NAO muda, nenhuma assinatura muda,
// battle_scene_test.cpp continua intocado. Ver battle_scene.cpp pro resto da classe
// (ctor, motor de turno) e battle_scene_picker.cpp pro modo de escolha de ator.

#include "gus/app/screens/battle_scene.hpp"

#include <optional>
#include <string>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/weakness_wheel.hpp"  // WeaknessWheel (pre-selecao D3 de mira)

namespace gus::app::screens {

using gus::domain::combat::CardFamily;
using gus::domain::combat::CombatAction;
using gus::domain::combat::CombatActor;

// ---- Modo-mira / target selection (§3.5, D3) -------------------------------------
// APRESENTACAO PURA: navega/confirma o ALVO; o motor (resolve_primary_target) ja aceita
// qualquer target_id. Substitui o hardcode first_alive_enemy de Atacar/Scan. So teclado.

void BattleScene::rebuild_aim_candidates() {
    // Inimigos VIVOS em ordem de fila (frente->tras): o [0] == first_alive_enemy (o mais a
    // frente = age antes, fallback D3 (b)). Exclui MORTOS (a mira nunca pousa num morto).
    aim_candidates_.clear();
    for (const CombatActor* a : machine_->queue().order()) {
        if (a != nullptr && !a->is_player_side() && a->is_alive()) {
            aim_candidates_.push_back(a);
        }
    }
}

std::optional<CardFamily> BattleScene::action_family(BattleVerb verb) const {
    // FALLBACK documentado (§3.5): o motor NAO aplica a roda de fraqueza no ataque BASICO
    // (dano = atk - def, resolve_basic_attack) e o Scan e utilitario sem familia. Pra a
    // sugestao D3 (a) ser significativa e "premiar o Scan", ATACAR usa a familia-assinatura
    // do ATOR ATIVO como "familia da acao". SCAN nao tem familia (nullopt): cai sempre no
    // fallback (b) (o front da fila = ameaca iminente; escanear um ja-escaneado nao ajuda).
    // COMPILAR (carta COM familia propria) entra num incremento futuro.
    if (verb == BattleVerb::Atacar) {
        const CombatActor* self = active_actor();
        if (self != nullptr) {
            return self->family();
        }
    }
    return std::nullopt;
}

int BattleScene::preselect_aim_index(BattleVerb verb) const {
    // D3 (a): havendo info de Scan, mira o 1o inimigo (na ordem de fila) ESCANEADO e FRACO
    // (multFraqueza 1.5) a familia da acao. Premia o Scan (Pillar 1: info habilita acao).
    const std::optional<CardFamily> fam = action_family(verb);
    if (fam.has_value()) {
        for (int i = 0; i < static_cast<int>(aim_candidates_.size()); ++i) {
            const CombatActor* e = aim_candidates_[static_cast<std::size_t>(i)];
            if (e->is_scanned() && !e->is_universal_compiler() &&
                gus::domain::combat::WeaknessWheel::multiplier(*fam, e->family()) ==
                    gus::domain::combat::combat_constants::kMultFraco) {
                return i;
            }
        }
    }
    // D3 (b) fallback: front da fila (== first_alive_enemy) = aim_candidates_[0].
    return 0;
}

bool BattleScene::enter_aim_mode(BattleVerb verb) {
    rebuild_aim_candidates();
    if (aim_candidates_.empty()) {
        return false;  // sem inimigo vivo: nao entra na mira (vitoria iminente)
    }
    aim_verb_ = verb;
    aiming_ = true;
    aim_index_ = preselect_aim_index(verb);
    return true;
}

void BattleScene::aim_move(int delta) noexcept {
    if (!aiming_ || aim_candidates_.empty()) {
        return;
    }
    const int n = static_cast<int>(aim_candidates_.size());
    aim_index_ = ((aim_index_ + delta) % n + n) % n;  // WRAP nos extremos
}

void BattleScene::aim_select(int index) noexcept {
    // MOUSE (Incremento A2): pousa a mira DIRETO no index (sem wrap). No-op fora da mira ou
    // com index invalido (nao mexe no cursor corrente).
    if (!aiming_ || index < 0 || index >= static_cast<int>(aim_candidates_.size())) {
        return;
    }
    aim_index_ = index;
}

int BattleScene::aim_index_at_arena(float world_x, float world_y) const {
    // Casa o ponto (mundo/logico) com o SLOT de cada inimigo MIRAVEL. aim_candidates_ segue a
    // ordem de fila dos inimigos vivos, a MESMA base que arena_rect_for_actor usa pra ordenar
    // os slots -> o i-esimo candidato casa o i-esimo slot. Fora da mira a lista e vazia (-1).
    for (int i = 0; i < static_cast<int>(aim_candidates_.size()); ++i) {
        const CombatActor* e = aim_candidates_[static_cast<std::size_t>(i)];
        if (e == nullptr) {
            continue;
        }
        const std::optional<gus::core::spatial::Rect> slot = arena_rect_for_actor(e->id());
        if (!slot.has_value()) {
            continue;  // alvo sem slot visivel (nao deveria, mira so tem vivos): pula
        }
        const gus::core::spatial::Rect& r = *slot;
        if (world_x >= r.x && world_x <= r.x + r.w && world_y >= r.y &&
            world_y <= r.y + r.h) {
            return i;
        }
    }
    return -1;
}

const CombatActor* BattleScene::aim_target() const noexcept {
    if (!aiming_ || aim_index_ < 0 ||
        aim_index_ >= static_cast<int>(aim_candidates_.size())) {
        return nullptr;
    }
    return aim_candidates_[static_cast<std::size_t>(aim_index_)];
}

void BattleScene::aim_cancel() noexcept {
    // Volta ao menu de verbos SEM consumir o turno: nada resolve, o pacing segue em
    // WaitingPlayerInput e o menu do ator ativo continua ativo.
    aiming_ = false;
    aim_index_ = 0;
    aim_candidates_.clear();
}

void BattleScene::aim_hotkey(int nth) {
    // TECLA-ATALHO 1-9 (mira): escolhe o nth-esimo inimigo miravel (1-based) e CONFIRMA na
    // hora. Espelha actor_picker_hotkey: no-op se fora do modo ou nth sem candidato (fora de
    // faixa) - "apertar 5 com so 4 miraveis" nao faz nada (nem mira, nem confirma). Fonte
    // unica do host (teclas) e dos testes.
    if (!aiming_) {
        return;
    }
    const int idx = nth - 1;  // 1-based -> 0-based
    if (idx < 0 || idx >= static_cast<int>(aim_candidates_.size())) {
        return;  // numero sem inimigo miravel: no-op
    }
    aim_index_ = idx;
    aim_confirm();
}

void BattleScene::aim_confirm() {
    if (!aiming_) {
        return;
    }
    const CombatActor* t = aim_target();
    if (t == nullptr) {
        aim_cancel();  // sem alvo valido: aborta a mira sem consumir turno
        return;
    }

    // PONTO-DE-NAO-RETORNO (bug1, §4.1): confirmar o ALVO resolve o turno (Atacar/Scan) - a
    // mira em si (enter_aim_mode/aim_move/aim_cancel) NUNCA commita, so aqui. Se o ator ainda
    // esta em PREVIEW, este e o commit: grava a escolha + begin_turn REAL (tick incluso) ANTES
    // de montar/resolver a acao. No-op se ja commitado.
    commit_previewed_actor();
    const bool is_scan = (aim_verb_ == BattleVerb::Scan);
    // Monta a acao com o ALVO ESCOLHIDO (nao mais o hardcode first_alive_enemy).
    CombatAction action = is_scan ? CombatAction::scan(t->id())
                                  : CombatAction::attack(t->id());
    const std::string target_id = t->id();
    // Sai da mira ANTES de resolver (o turno avanca; o proximo turno reabre o menu).
    aiming_ = false;
    aim_index_ = 0;
    aim_candidates_.clear();
    pending_action_ = std::move(action);

    // [ATACAR] = golpe MELEE (W2, battle-anim.md par.2.2/3.1): a confirmacao inicia o
    // WINDUP na hora (regra de ouro < 100ms: o sprite ja parte no proximo frame) e a
    // RESOLUCAO fica DEFERIDA ate o CONTATO (fim da aproximacao; a acao espera no
    // mailbox) - a aproximacao E o proprio anuncio do turno do jogador. O ritmo
    // continua em WaitingPlayerInput durante o voo (o menu fica inerte pelos guards);
    // o update(dt) resolve no contato e entra no delay (player_acted). A duracao usa
    // kPlayerMeleeApproachSeconds (DESACOPLADA do Beat 1 do inimigo, que segue em
    // kPacingAnnounceSeconds cru): o approach do jogador NAO tem beat de pacing atrelado
    // (o contato e gated por anim_.melee_arrived, nao por timer), entao dura o que a
    // LEITURA pede - o playtest (lider 2026-07-02) pediu mais tempo pra VER a corrida.
    // Se o slot nao estiver visivel (nunca no fluxo normal), degrada: resolve na hora.
    if (!is_scan) {
        const CombatActor* self = active_actor();
        if (self != nullptr &&
            start_melee_toward(self->id(), target_id, kPlayerMeleeApproachSeconds)) {
            player_strike_pending_ = true;
            player_strike_attacker_ = self->id();
            return;  // contato (e resolucao) vem no update(dt)
        }
    }
    // Scan (utilitario, nao e golpe) e o fallback degradado resolvem NA HORA (mesmo
    // fluxo do menu_confirm antigo): o update(dt) anima os inimigos um a um depois.
    resolve_one_turn();
    pacing_.player_acted();
    menu_.refresh(active_actor() != nullptr ? active_actor()->ap() : 0);
}

}  // namespace gus::app::screens
