// gus/app/src/screens/battle_scene_picker.cpp
//
// AC-E11 A5 (TU-split, ADR-019): DEFINICOES dos metodos de escolha de ator / Janela de
// Comando da Party (§4.1, comando-livre 1B) de BattleScene - extraidos de
// battle_scene.cpp (que ficou grande demais). PURO TU-split: o header
// (battle_scene.hpp) NAO muda, nenhuma assinatura muda, battle_scene_test.cpp continua
// intocado. Ver battle_scene.cpp pro resto da classe (ctor, motor de turno) e
// battle_scene_aim.cpp pro modo-mira.

#include "gus/app/screens/battle_scene.hpp"

#include <vector>

#include "gus/domain/combat/combat_actor.hpp"

namespace gus::app::screens {

using gus::domain::combat::CombatActor;

// ---- Escolha de ator / Janela de Comando da Party (§4.1, comando-livre 1B) ----------
// APRESENTACAO PURA sobre o motor 1B: a cena so deixa o jogador ESCOLHER qual membro
// pendente da party age; o motor (pending_party_actors/select_party_actor) faz o resto
// (agrupar por lado, recomputar quem abre, regroup Gambito-safe). O begin_turn e DEFERIDO
// ate o confirm (start_active_turn e no-op enquanto choosing_actor_).

bool BattleScene::should_offer_actor_picker() const {
    // Gate do modo: e a vez do bloco da party, o combate esta rolando (nao intro/nao fim) e ha
    // MAIS DE UM elegivel. active_actor() == queue_.current(); apos o regroup por lado (§4.1) o
    // bloco da party e contiguo, entao "current e player-side vivo" == "abriu o bloco da party".
    //
    // >1 (DECISAO documentada): com 1 so elegivel NAO ha escolha a fazer - a cena auto-inicia o
    // turno (sem friccao). O picker existe pra dar AGENCIA sobre QUEM age; com um unico pendente
    // nao ha agencia a exercer (distinto da mira, cujo confirm COMPROMETE um ataque com previa
    // de dano). Isto reforca o modelo mental "escolho quando ha varios". Trocar pra ">= 1"
    // (mostrar sempre, como a mira com 1 alvo) e UMA LINHA, se o lider preferir consistencia.
    if (combat_over() || is_intro()) {
        return false;
    }
    const CombatActor* a = active_actor();
    if (a == nullptr || !a->is_player_side() || !a->is_alive()) {
        return false;
    }
    return machine_->pending_party_actors().size() > 1;
}

void BattleScene::enter_actor_picker() {
    // Snapshot dos elegiveis (maior SPD -> menor; front = pre-selecionado). Cursor no front.
    // NAO chama begin_turn: o confirm so entra no PREVIEW (estagio 2); o turno real so
    // comeca no commit_previewed_actor (1a acao resolvida). Chamada tambem por
    // actor_preview_cancel (Esc no preview reabre a lista do zero, mesma fonte).
    actor_choices_ = machine_->pending_party_actors();
    choosing_actor_ = true;
    actor_pick_index_ = 0;  // front de pending_party_actors == preselected_party_actor (SPD)
}

std::vector<const CombatActor*> BattleScene::actor_choices() const {
    return std::vector<const CombatActor*>(actor_choices_.begin(), actor_choices_.end());
}

const CombatActor* BattleScene::actor_pick_target() const noexcept {
    if (!choosing_actor_ || actor_pick_index_ < 0 ||
        actor_pick_index_ >= static_cast<int>(actor_choices_.size())) {
        return nullptr;
    }
    return actor_choices_[static_cast<std::size_t>(actor_pick_index_)];
}

void BattleScene::actor_picker_move(int delta) noexcept {
    if (!choosing_actor_ || actor_choices_.empty()) {
        return;
    }
    const int n = static_cast<int>(actor_choices_.size());
    actor_pick_index_ = ((actor_pick_index_ + delta) % n + n) % n;  // WRAP nos extremos
}

void BattleScene::actor_picker_select(int index) noexcept {
    // MOUSE/atalho: poe o cursor DIRETO no index (sem wrap). No-op fora do modo ou invalido.
    if (!choosing_actor_ || index < 0 ||
        index >= static_cast<int>(actor_choices_.size())) {
        return;
    }
    actor_pick_index_ = index;
}

int BattleScene::actor_pick_index_at_arena(float world_x, float world_y) const {
    // Casa o ponto (mundo/logico) com o SLOT de cada elegivel. arena_rect_for_actor e generico
    // por id (trata party E inimigo), entao vale pros membros da party (confirmado: ele separa
    // os lados e devolve o slot certo). Fora do modo a lista e vazia (-1). Mesma logica de
    // aim_index_at_arena, sobre actor_choices_ (party) -> reusa o pipeline de hit-test do A2.
    for (int i = 0; i < static_cast<int>(actor_choices_.size()); ++i) {
        const CombatActor* p = actor_choices_[static_cast<std::size_t>(i)];
        if (p == nullptr) {
            continue;
        }
        const std::optional<gus::core::spatial::Rect> slot = arena_rect_for_actor(p->id());
        if (!slot.has_value()) {
            continue;
        }
        const gus::core::spatial::Rect& r = *slot;
        if (world_x >= r.x && world_x <= r.x + r.w && world_y >= r.y &&
            world_y <= r.y + r.h) {
            return i;
        }
    }
    return -1;
}

void BattleScene::actor_picker_confirm() {
    if (!choosing_actor_) {
        return;
    }
    if (actor_pick_index_ < 0 ||
        actor_pick_index_ >= static_cast<int>(actor_choices_.size())) {
        return;  // defensivo (cursor invalido): nao confirma
    }
    CombatActor* chosen = actor_choices_[static_cast<std::size_t>(actor_pick_index_)];

    // FIX bug1 (playtest do lider 2026-07: "escolho e nao consigo trocar - fica FIXO"): NAO
    // grava a escolha no motor nem chama begin_turn aqui. Antes o confirm chamava
    // begin_active_turn_now() na hora, que faz apply_status_tick (IRREVERSIVEL) colado a
    // simples SELECAO do ator - por isso "travava". Agora so entra no estagio (2) PREVIEW: o
    // menu de verbos DELE aparece (AP/mana corretos, ver refresh abaixo), mas o motor segue
    // 100% intocado (sem select_party_actor, sem begin_turn, sem tick). O commit real fica pra
    // commit_previewed_actor() (chamado so quando a 1a acao de fato resolve - menu_confirm ou
    // aim_confirm). Ate la, Esc (actor_preview_cancel) reabre esta MESMA lista do zero e o
    // jogador troca de ator sem custo algum.
    choosing_actor_ = false;
    actor_choices_.clear();
    actor_pick_index_ = 0;
    actor_preview_ = true;
    preview_actor_ = chosen;

    // AP/MANA no PREVIEW: sem isto o cockpit mostraria ap()==0 (o campo so e populado por
    // refresh_resources_for_turn, chamado ate agora SO dentro do begin_turn) e TODO verbo
    // apareceria desabilitado - preview inutil. refresh_resources_for_turn e INOCUO/
    // IDEMPOTENTE (sem RNG; so recarrega ap_/mana_ pela formula deterministica de
    // round_index) e e chamado de novo, com o MESMO round_index, dentro do begin_turn REAL em
    // commit_previewed_actor() - byte-identico, sem efeito duplicado observavel. Isto e o
    // unico ponto onde a cena toca um metodo do ATOR (POCO ja publico) fora do begin_turn do
    // motor; NENHUM arquivo de domain/ foi alterado (combat_state_machine/combat_actor
    // intocados) - reportado como checkpoint (a) da tarefa.
    chosen->refresh_resources_for_turn(machine_->queue().round_index());
    menu_.refresh(chosen->ap());
}

void BattleScene::commit_previewed_actor() {
    if (!actor_preview_) {
        return;  // sem preview pendente: <=1 elegivel (begin_active_turn_now ja rodou direto
                 // em start_active_turn) OU este ator ja foi commitado antes. No-op seguro -
                 // chamado incondicionalmente no topo de toda resolucao real de acao.
    }
    // PONTO-DE-NAO-RETORNO (bug1, regra fechada Caetano+lider): grava a escolha no motor
    // (select_party_actor) e sai do preview ANTES de chamar begin_active_turn_now - assim
    // active_actor() ja cai no caminho normal (machine_->active_actor()) dentro dele, que o
    // proprio begin_turn alinha via bring_to_current(chosen). select_party_actor lanca
    // std::invalid_argument se `preview_actor_` nao e mais elegivel; como nada mutou o motor
    // entre o confirm e este commit (o preview e 100% apresentacao), ele SEMPRE e elegivel.
    machine_->select_party_actor(preview_actor_);
    actor_preview_ = false;
    preview_actor_ = nullptr;
    // O begin_turn REAL: bring_to_current + refresh (idempotente, ja rodou uma vez no
    // preview) + o TICK DE STATUS - agora sim IRREVERSIVEL (Poison floata, Haste/Slow
    // aplicam, etc). turn_started_ segue false ate aqui (garantido por start_active_turn
    // nunca ter chamado begin_active_turn_now enquanto actor_preview_ era true).
    begin_active_turn_now();
}

void BattleScene::actor_preview_cancel() noexcept {
    if (!actor_preview_) {
        return;  // fora do preview: nada a desfazer (picker fechado, ou ja commitado - nao ha
                 // volta apos a 1a acao, ver commit_previewed_actor).
    }
    actor_preview_ = false;
    preview_actor_ = nullptr;
    // Reabre a LISTA do zero (mesma fonte, pending_party_actors()): como o preview NUNCA
    // tocou o motor, a lista e IDENTICA a de antes (nao precisamos guardar/restaurar nada).
    // should_offer_actor_picker() nao e re-checado de proposito (mesmo padrao do confirm): sabemos
    // que havia >1 elegivel quando este preview abriu, e nada consumiu nenhum deles.
    enter_actor_picker();
}

void BattleScene::actor_picker_hotkey(int nth) {
    // TECLA-ATALHO 1/2/3: escolhe o nth-esimo elegivel (1-based) e CONFIRMA na hora. No-op se
    // fora do modo ou nth sem elegivel (fora de faixa) - assim "apertar 3 com so 2 pendentes"
    // nao faz nada (nem seleciona, nem confirma). Fonte unica do host (teclas) e dos testes.
    if (!choosing_actor_) {
        return;
    }
    const int idx = nth - 1;  // 1-based -> 0-based
    if (idx < 0 || idx >= static_cast<int>(actor_choices_.size())) {
        return;  // numero sem elegivel: no-op
    }
    actor_pick_index_ = idx;
    actor_picker_confirm();
}

}  // namespace gus::app::screens
