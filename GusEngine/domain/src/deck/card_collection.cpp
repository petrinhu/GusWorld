// gus/domain/deck/card_collection.cpp
//
// Implementacao do agregado CardCollection (DECK-1). Ver card_collection.hpp pros
// invariantes garantidos (secao 7 do spec: XOR de container, deck morto inerte,
// one-way por ausencia de API, guard de tier pra classe protegida).

#include "gus/domain/deck/card_collection.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace gus::domain::deck {

using gus::domain::combat::CardTier;

CardCollection::CardCollection(int active_capacity, std::uint64_t next_instance_id)
    : active_capacity_(active_capacity), next_instance_id_(next_instance_id) {
    if (active_capacity_ <= 0) {
        throw std::invalid_argument(
            "CardCollection: active_capacity deve ser positivo (deck-mao-sistema.md "
            "secao 8c - capacidade do deck ativo)");
    }
}

bool CardCollection::present_in_active(std::uint64_t instance_id) const noexcept {
    return std::any_of(active_.begin(), active_.end(),
                        [instance_id](const CardInstance& c) { return c.instance_id == instance_id; });
}

bool CardCollection::present_in_dead(std::uint64_t instance_id) const noexcept {
    return std::any_of(dead_.begin(), dead_.end(),
                        [instance_id](const CardInstance& c) { return c.instance_id == instance_id; });
}

void CardCollection::guard_protected_tier(const std::string& card_id,
                                           const TierLookup& tier_of) const {
    const CardTier tier = tier_of(card_id);
    if (tier == CardTier::Especial || tier == CardTier::Super) {
        throw std::invalid_argument(
            "CardCollection: carta '" + card_id +
            "' e classe PROTEGIDA (CardTier::Especial/Super, deck-mao-sistema.md secao "
            "7 inv.9) - nao pode sair do deck ativo (nem descarte nem venda)");
    }
}

CardInstance CardCollection::add_to_active(std::string card_id,
                                            std::optional<std::uint64_t> instance_id_override,
                                            CardPhysicalState initial_physical) {
    std::uint64_t id;
    if (instance_id_override.has_value()) {
        id = *instance_id_override;
        // XOR de container (inv.1): instancia ja presente em QUALQUER lado - rejeita.
        if (present_in_active(id) || present_in_dead(id)) {
            throw std::invalid_argument(
                "CardCollection::add_to_active: instance_id ja presente (viola secao 7 "
                "inv.1 - instancia unica, vive em exatamente um container)");
        }
        // Avanca o contador pra nao colidir com um id restaurado explicitamente.
        if (id >= next_instance_id_) {
            next_instance_id_ = id + 1;
        }
    } else {
        id = next_instance_id_;
        ++next_instance_id_;
    }

    if (active_.size() >= static_cast<std::size_t>(active_capacity_)) {
        throw std::logic_error(
            "CardCollection::add_to_active: deck ativo na capacidade maxima - descarte "
            "ou venda uma carta antes de adicionar (deck-mao-sistema.md secao 8c)");
    }

    CardInstance instance{id, std::move(card_id), std::move(initial_physical)};
    active_.push_back(instance);
    return instance;
}

void CardCollection::discard_to_dead(std::uint64_t instance_id, const TierLookup& tier_of) {
    auto it = std::find_if(active_.begin(), active_.end(), [instance_id](const CardInstance& c) {
        return c.instance_id == instance_id;
    });
    if (it == active_.end()) {
        throw std::invalid_argument(
            "CardCollection::discard_to_dead: instance_id nao esta no deck ativo");
    }
    // COPIA a instancia ANTES do callback opaco tier_of (dentro de guard_protected_tier):
    // `it` e it->card_id apontam pro buffer de active_; se tier_of tocar este agregado e
    // realocar active_, tanto o iterador quanto a referencia da string penduram
    // (heap-use-after-free). Guardamos sobre a COPIA (estavel) e reancoramos o iterador
    // depois - fail-fast se a instancia sumiu no meio (tier_of nao pode mutar o agregado).
    CardInstance moved = *it;
    guard_protected_tier(moved.card_id, tier_of);  // guard sobre a copia estavel
    it = std::find_if(active_.begin(), active_.end(), [instance_id](const CardInstance& c) {
        return c.instance_id == instance_id;
    });
    if (it == active_.end()) {
        throw std::invalid_argument(
            "CardCollection::discard_to_dead: instance_id sumiu do deck ativo durante o "
            "lookup de tier (tier_of nao pode mutar o agregado)");
    }
    active_.erase(it);
    dead_.push_back(std::move(moved));
}

CardInstance CardCollection::remove_for_sale(std::uint64_t instance_id, const TierLookup& tier_of) {
    auto it = std::find_if(active_.begin(), active_.end(), [instance_id](const CardInstance& c) {
        return c.instance_id == instance_id;
    });
    if (it == active_.end()) {
        throw std::invalid_argument(
            "CardCollection::remove_for_sale: instance_id nao esta no deck ativo");
    }
    // COPIA a instancia ANTES do callback opaco tier_of (mesma raiz de discard_to_dead):
    // guard sobre a copia estavel, reancora o iterador depois - imune a uma realocacao
    // de active_ provocada por tier_of.
    CardInstance removed = *it;
    guard_protected_tier(removed.card_id, tier_of);
    it = std::find_if(active_.begin(), active_.end(), [instance_id](const CardInstance& c) {
        return c.instance_id == instance_id;
    });
    if (it == active_.end()) {
        throw std::invalid_argument(
            "CardCollection::remove_for_sale: instance_id sumiu do deck ativo durante o "
            "lookup de tier (tier_of nao pode mutar o agregado)");
    }
    active_.erase(it);
    return removed;
}

const std::vector<CardInstance>& CardCollection::active() const noexcept { return active_; }

const std::vector<CardInstance>& CardCollection::dead() const noexcept { return dead_; }

std::size_t CardCollection::active_count() const noexcept { return active_.size(); }

int CardCollection::active_capacity() const noexcept { return active_capacity_; }

bool CardCollection::active_is_full() const noexcept {
    return active_.size() >= static_cast<std::size_t>(active_capacity_);
}

std::uint64_t CardCollection::next_instance_id() const noexcept { return next_instance_id_; }

}  // namespace gus::domain::deck
