// gus/domain/deck/hand_loadout.cpp
//
// Implementacao do HandLoadout (DECK-2). Ver hand_loadout.hpp pros invariantes
// garantidos (anti-dup, so-deck-ativo, teto de comum, slot especial dedicado,
// revalidacao de orfaos).

#include "gus/domain/deck/hand_loadout.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace gus::domain::deck {

using gus::domain::combat::CardTier;

int hand_capacity(int base_por_personagem, int hardware_bonus, int mental_stat_bonus) noexcept {
    const int raw = base_por_personagem + hardware_bonus + mental_stat_bonus;
    return std::clamp(raw, 0, kHandSizeMentalStatCap);
}

HandLoadout::HandLoadout(int comum_capacity, bool is_universal_compiler)
    : comum_capacity_(comum_capacity), is_universal_compiler_(is_universal_compiler) {
    if (comum_capacity_ < 0) {
        throw std::invalid_argument(
            "HandLoadout: comum_capacity nao pode ser negativa (deck-mao-sistema.md secao 8c)");
    }
}

void HandLoadout::validate_candidate(const std::vector<std::uint64_t>& candidate,
                                      const CardCollection& deck, const TierLookup& tier_of) const {
    const int special_capacity = is_universal_compiler_ ? kGusSpecialHandSlots : 0;
    int comum_count = 0;
    int special_count = 0;

    for (std::size_t i = 0; i < candidate.size(); ++i) {
        const std::uint64_t id = candidate[i];

        // Duplicata dentro da propria selecao candidata (invariante anti-dup - a mesma
        // instancia nao pode ocupar 2 "slots" da mao, mesmo sendo so um ID repetido).
        for (std::size_t j = 0; j < i; ++j) {
            if (candidate[j] == id) {
                throw std::invalid_argument(
                    "HandLoadout: instance_id " + std::to_string(id) +
                    " selecionado 2x na mesma mao (duplicata - secao 7 inv.2)");
            }
        }

        // Presenca no deck ATIVO (secao 7 inv.6 - "mao so puxa do ativo"). Rejeita ID
        // do deck morto, de venda, ou inexistente.
        const auto it = std::find_if(deck.active().begin(), deck.active().end(),
                                      [id](const CardInstance& c) { return c.instance_id == id; });
        if (it == deck.active().end()) {
            throw std::invalid_argument(
                "HandLoadout: instance_id " + std::to_string(id) +
                " nao esta no deck ATIVO (carta morta/vendida/inexistente - secao 7 inv.6)");
        }

        // COPIA o card_id ANTES de invocar o callback opaco tier_of. `it->card_id` e
        // uma referencia pro buffer de deck.active(); tier_of e fornecido pelo chamador
        // e nao ha garantia estatica de que ele nao toque o CardCollection (ex.: um
        // lookup que carregue/expanda o catalogo e, por efeito colateral, mute o deck).
        // Se active_ realocar durante a chamada, `it->card_id` penduraria
        // (heap-use-after-free - ASan confirma em hand_loadout.cpp:59). Passar uma copia
        // estavel desacopla a corretude do lifetime do elemento do container.
        const std::string card_id = it->card_id;
        const CardTier tier = tier_of(card_id);
        if (tier == CardTier::Comum) {
            ++comum_count;
        } else {
            ++special_count;
        }
    }

    if (comum_count > comum_capacity_) {
        throw std::invalid_argument(
            "HandLoadout: mao excede a capacidade de comuns (" + std::to_string(comum_count) +
            " > " + std::to_string(comum_capacity_) + ", deck-mao-sistema.md secao 8c)");
    }
    if (special_count > special_capacity) {
        throw std::invalid_argument(
            is_universal_compiler_
                ? "HandLoadout: mao excede o slot especial dedicado (secao 8c, "
                  "kGusSpecialHandSlots = 1)"
                : "HandLoadout: personagem sem flag is_universal_compiler nao pode ter carta "
                  "Especial/Super na mao (secao 4/8c - slot exclusivo do Gus)");
    }
}

void HandLoadout::set_selection(std::vector<std::uint64_t> instance_ids, const CardCollection& deck,
                                 const TierLookup& tier_of) {
    validate_candidate(instance_ids, deck, tier_of);
    selection_ = std::move(instance_ids);
}

void HandLoadout::add_card(std::uint64_t instance_id, const CardCollection& deck,
                            const TierLookup& tier_of) {
    std::vector<std::uint64_t> candidate = selection_;
    candidate.push_back(instance_id);
    validate_candidate(candidate, deck, tier_of);
    selection_ = std::move(candidate);
}

void HandLoadout::remove_card(std::uint64_t instance_id) {
    const auto it = std::find(selection_.begin(), selection_.end(), instance_id);
    if (it == selection_.end()) {
        throw std::invalid_argument("HandLoadout::remove_card: instance_id nao esta selecionado na mao");
    }
    selection_.erase(it);
}

void HandLoadout::swap_card(std::uint64_t old_instance_id, std::uint64_t new_instance_id,
                             const CardCollection& deck, const TierLookup& tier_of) {
    const auto it = std::find(selection_.begin(), selection_.end(), old_instance_id);
    if (it == selection_.end()) {
        throw std::invalid_argument(
            "HandLoadout::swap_card: old_instance_id nao esta selecionado na mao");
    }

    std::vector<std::uint64_t> candidate = selection_;
    const auto idx = static_cast<std::size_t>(std::distance(selection_.begin(), it));
    candidate[idx] = new_instance_id;
    validate_candidate(candidate, deck, tier_of);
    selection_ = std::move(candidate);
}

std::vector<std::uint64_t> HandLoadout::find_orphan_instance_ids(const CardCollection& deck) const {
    std::vector<std::uint64_t> orphans;
    for (const std::uint64_t id : selection_) {
        const bool present = std::any_of(deck.active().begin(), deck.active().end(),
                                          [id](const CardInstance& c) { return c.instance_id == id; });
        if (!present) {
            orphans.push_back(id);
        }
    }
    return orphans;
}

std::vector<std::uint64_t> HandLoadout::revalidate(const CardCollection& deck) {
    const std::vector<std::uint64_t> orphans = find_orphan_instance_ids(deck);
    if (orphans.empty()) {
        return orphans;
    }
    selection_.erase(std::remove_if(selection_.begin(), selection_.end(),
                                     [&orphans](std::uint64_t id) {
                                         return std::find(orphans.begin(), orphans.end(), id) !=
                                                orphans.end();
                                     }),
                      selection_.end());
    return orphans;
}

const std::vector<std::uint64_t>& HandLoadout::selection() const noexcept { return selection_; }

std::size_t HandLoadout::size() const noexcept { return selection_.size(); }

int HandLoadout::comum_capacity() const noexcept { return comum_capacity_; }

bool HandLoadout::is_universal_compiler() const noexcept { return is_universal_compiler_; }

}  // namespace gus::domain::deck
