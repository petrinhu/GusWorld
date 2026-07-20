// gus/domain/combat/combat_state.hpp
//
// View read-only do estado de combate, passada ao action provider (jogador/AI) e ao
// IEnemyBrain. Nao permite mutacao direta: o consumidor decide uma acao e a FSM aplica.
// Portado de engine/foundation/turn_combat/CombatState.cs. POCO puro, ZERO Qt
// (invariante de domain/, engine-design.md secao 2). Header-only: views triviais sobre
// a InitiativeQueue.
//
// PORTE DE REFERENCIA: o C# guarda referencia a InitiativeQueue + CombatActor. No C++ a
// view guarda ponteiros NAO-DONOS (const InitiativeQueue& / CombatActor*). Os atores e a
// fila vivem no escopo da CombatStateMachine (chunk 4) / teste.
//
// CardRegistry (IReadOnlyDictionary<string,Card>) -> const std::unordered_map<string,
// Card>* (nullptr => registry vazio, equivalente ao EmptyRegistry default do C#).
//
// AlivePlayers/AliveEnemies retornam IEnumerable<CombatActor> (lazy) no C#; aqui
// materializamos em std::vector<CombatActor*> (a fila e pequena: <= kMaxPartySize +
// inimigos do encontro). FindById retorna nullable -> CombatActor* (nullptr se ausente).
//
// LEAKED_INTEL (CARDS-HW-2C, Backdoor SIGNAL-ONLY; docs/design/mecanicas/
// cartas-spec-logica.md secao 4.2): sinal continuo, populado pela CombatStateMachine a
// cada construcao de CombatState a partir do integrity_ledger (owner_actor_id de toda
// carta infectada com VirusKind::Backdoor na mao/equipada de um aliado). Campo ADITIVO
// (default vazio, todo call site pre-existente preservado). Esta fatia SO expoe o sinal -
// NENHUM brain pondera isto ainda (o UtilityBrain que consumiria o vies, cartas-spec-
// logica.md secao 4.2, e jogo posterior; o ScriptedBrain de hoje e determinístico e
// ignora o campo por completo). owner_actor_id e a chave OPACA do ledger
// (card_integrity_ledger.hpp) - NAO precisa casar com CombatActor::id() nesta fatia.
//
// Cross-ref: engine/foundation/turn_combat/CombatState.cs;
//            docs/design/mecanicas/combat.md secao 3/13; ADR-006;
//            docs/design/mecanicas/cartas-spec-logica.md secao 4.2.

#ifndef GUS_DOMAIN_COMBAT_COMBAT_STATE_HPP
#define GUS_DOMAIN_COMBAT_COMBAT_STATE_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/initiative_queue.hpp"

namespace gus::domain::combat {

// Snapshot consultavel do combate em ActionSelect. POCO. secao 3/13.
class CombatState {
public:
    CombatState(const InitiativeQueue& queue,
                CombatActor* active_actor,
                int round_index,
                const std::unordered_map<std::string, Card>* card_registry = nullptr,
                std::vector<int> leaked_intel = {})
        : queue_(&queue),
          active_actor_(active_actor),
          round_index_(round_index),
          card_registry_(card_registry),
          leaked_intel_(std::move(leaked_intel)) {}

    // Ator cujo turno esta em andamento.
    [[nodiscard]] CombatActor* active_actor() const noexcept { return active_actor_; }

    // Indice de rodada atual (ramp de mana).
    [[nodiscard]] int round_index() const noexcept { return round_index_; }

    // Banco de cartas jogaveis (id -> carta). UseCard resolve o CardId aqui (secao 7/11).
    // Vazio quando o combate nao carrega deck (testes de FSM basica).
    [[nodiscard]] const std::unordered_map<std::string, Card>& card_registry() const noexcept {
        return card_registry_ != nullptr ? *card_registry_ : empty_registry();
    }

    // Ordem visivel da fila (proximos a jogar). secao 4.
    [[nodiscard]] const std::vector<CombatActor*>& order() const noexcept { return queue_->order(); }

    // Atores da party ainda vivos.
    [[nodiscard]] std::vector<CombatActor*> alive_players() const {
        std::vector<CombatActor*> out;
        for (CombatActor* a : queue_->order())
            if (a->is_player_side() && a->is_alive())
                out.push_back(a);
        return out;
    }

    // Inimigos ainda vivos.
    [[nodiscard]] std::vector<CombatActor*> alive_enemies() const {
        std::vector<CombatActor*> out;
        for (CombatActor* a : queue_->order())
            if (!a->is_player_side() && a->is_alive())
                out.push_back(a);
        return out;
    }

    // Lookup de ator por id (alvo de acao). Retorna nullptr se ausente.
    [[nodiscard]] CombatActor* find_by_id(const std::string& id) const {
        for (CombatActor* a : queue_->order())
            if (a->id() == id)
                return a;
        return nullptr;
    }

    // Sinal continuo de vazamento do Backdoor (CARDS-HW-2C, secao 4.2 acima). Vazio quando
    // nao ha carta Backdoor-infectada no ledger (ou o ledger e nulo). Ninguem consome isto
    // ainda - so-query, ver comentario da classe.
    [[nodiscard]] const std::vector<int>& leaked_intel() const noexcept { return leaked_intel_; }

private:
    // Registry vazio compartilhado (equivalente ao EmptyRegistry static do C#).
    [[nodiscard]] static const std::unordered_map<std::string, Card>& empty_registry() {
        static const std::unordered_map<std::string, Card> kEmpty;
        return kEmpty;
    }

    const InitiativeQueue* queue_;
    CombatActor* active_actor_;
    int round_index_;
    const std::unordered_map<std::string, Card>* card_registry_;
    std::vector<int> leaked_intel_;
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_COMBAT_STATE_HPP
