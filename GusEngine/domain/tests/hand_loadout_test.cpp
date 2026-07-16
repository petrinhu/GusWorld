// hand_loadout_test.cpp
//
// Spec executavel (Catch2 v3) do HandLoadout (DECK-2): a MAO (selecao/loadout) sobre o
// deck ATIVO (CardCollection, DECK-1). POCO puro, ZERO SDL/glintfx. Cobre:
//
//   - anti-dup (secao 7 inv.2): a mao NAO move/copia carta, so guarda instance_id;
//     a carta continua no deck ativo E aparece na selecao ao mesmo tempo.
//   - so aceita instance_id presente no deck ATIVO (rejeita ID do morto/inexistente).
//   - teto de capacidade comum (hand_capacity(), secao 8c) - over-capacity rejeitado.
//   - slot especial dedicado (kGusSpecialHandSlots=1): so quem tem a flag
//     is_universal_compiler; rejeita 2a especial mesmo pro Gus; rejeita QUALQUER
//     especial pra nao-Gus.
//   - hand_capacity(): teto (~6) e o delta -1 do Bento.
//   - set_selection/swap_card sao atomicos: falha nao muta a selecao atual.
//   - revalidate()/find_orphan_instance_ids(): detecta e remove ID orfao (carta saiu
//     do deck ativo por venda/descarte feito por fora desta mao).
//
// Cross-ref: docs/design/mecanicas/deck-mao-sistema.md secao 3/4/7/8c;
//            gus/domain/deck/hand_loadout.hpp / card_collection.hpp / deck_constants.hpp.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/deck/card_collection.hpp"
#include "gus/domain/deck/deck_constants.hpp"
#include "gus/domain/deck/hand_loadout.hpp"

using namespace gus::domain::deck;
using gus::domain::combat::CardTier;

namespace {

// Mesma convencao do card_collection_test.cpp: sufixo "_especial"/"_super" marca a
// classe protegida (secao 7.9), sem depender do registry real de combate.
CardTier fake_tier_of(const std::string& card_id) {
    if (card_id.find("_especial") != std::string::npos) return CardTier::Especial;
    if (card_id.find("_super") != std::string::npos) return CardTier::Super;
    return CardTier::Comum;
}

}  // namespace

// ---- Anti-dup (secao 7 inv.2): mao SO guarda ID, nunca move/copia -----------------

TEST_CASE("hand_loadout: add_card nao move nem copia a carta do deck ativo",
          "[domain][deck][hand_loadout][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    const CardInstance a = deck.add_to_active("pulso_eletrico");
    HandLoadout hand(kHandSizeBase);

    hand.add_card(a.instance_id, deck, fake_tier_of);

    // A carta CONTINUA no deck ativo (nao foi movida) e aparece na selecao (nao foi
    // copiada como container novo - so o ID).
    REQUIRE(deck.active_count() == 1);
    REQUIRE(deck.active().front().instance_id == a.instance_id);
    REQUIRE(hand.size() == 1);
    REQUIRE(hand.selection().front() == a.instance_id);
}

TEST_CASE("hand_loadout: mesmo instance_id 2x na selecao e rejeitado (duplicata)",
          "[domain][deck][hand_loadout][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    const CardInstance a = deck.add_to_active("pulso_eletrico");
    HandLoadout hand(kHandSizeBase);
    hand.add_card(a.instance_id, deck, fake_tier_of);

    REQUIRE_THROWS_AS(hand.add_card(a.instance_id, deck, fake_tier_of), std::invalid_argument);
    REQUIRE(hand.size() == 1);
}

// ---- So aceita ID presente no deck ATIVO ------------------------------------------

TEST_CASE("hand_loadout: rejeita instance_id inexistente no deck ativo",
          "[domain][deck][hand_loadout]") {
    CardCollection deck(kDeckCapacityTier1);
    HandLoadout hand(kHandSizeBase);

    REQUIRE_THROWS_AS(hand.add_card(999, deck, fake_tier_of), std::invalid_argument);
    REQUIRE(hand.size() == 0);
}

TEST_CASE("hand_loadout: rejeita instance_id que esta no deck MORTO (nao no ativo)",
          "[domain][deck][hand_loadout]") {
    CardCollection deck(kDeckCapacityTier1);
    const CardInstance a = deck.add_to_active("pulso_eletrico");
    deck.discard_to_dead(a.instance_id, fake_tier_of);
    HandLoadout hand(kHandSizeBase);

    // ID valido (existiu), mas nao esta mais no ATIVO - mao so puxa do ativo (inv.6).
    REQUIRE_THROWS_AS(hand.add_card(a.instance_id, deck, fake_tier_of), std::invalid_argument);
    REQUIRE(hand.size() == 0);
}

// ---- Teto de capacidade comum (hand_capacity, secao 8c) ---------------------------

TEST_CASE("hand_loadout: rejeita selecao acima da capacidade de comuns",
          "[domain][deck][hand_loadout]") {
    CardCollection deck(kDeckCapacityTier1);
    HandLoadout hand(/*comum_capacity=*/2);

    const CardInstance a = deck.add_to_active("pulso_a");
    const CardInstance b = deck.add_to_active("pulso_b");
    const CardInstance c = deck.add_to_active("pulso_c");

    hand.add_card(a.instance_id, deck, fake_tier_of);
    hand.add_card(b.instance_id, deck, fake_tier_of);
    // 3a comum estoura a capacidade (2) - rejeitada, selecao permanece com 2.
    REQUIRE_THROWS_AS(hand.add_card(c.instance_id, deck, fake_tier_of), std::invalid_argument);
    REQUIRE(hand.size() == 2);
}

TEST_CASE("hand_loadout: set_selection tambem valida o teto de comuns",
          "[domain][deck][hand_loadout]") {
    CardCollection deck(kDeckCapacityTier1);
    HandLoadout hand(/*comum_capacity=*/1);
    const CardInstance a = deck.add_to_active("pulso_a");
    const CardInstance b = deck.add_to_active("pulso_b");

    REQUIRE_THROWS_AS(
        hand.set_selection({a.instance_id, b.instance_id}, deck, fake_tier_of),
        std::invalid_argument);
    // Atomico: selecao continua vazia (nada aplicado parcialmente).
    REQUIRE(hand.size() == 0);
}

// ---- Slot especial dedicado (kGusSpecialHandSlots=1) ------------------------------

TEST_CASE("hand_loadout: personagem SEM flag is_universal_compiler rejeita QUALQUER especial",
          "[domain][deck][hand_loadout][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    HandLoadout hand(kHandSizeBase, /*is_universal_compiler=*/false);
    const CardInstance esp = deck.add_to_active("mestre_hipotenusa_especial");

    REQUIRE_THROWS_AS(hand.add_card(esp.instance_id, deck, fake_tier_of), std::invalid_argument);
    REQUIRE(hand.size() == 0);
}

TEST_CASE("hand_loadout: personagem SEM flag rejeita carta Super tambem",
          "[domain][deck][hand_loadout][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    HandLoadout hand(kHandSizeBase, /*is_universal_compiler=*/false);
    const CardInstance sup = deck.add_to_active("helon_tusk_super");

    REQUIRE_THROWS_AS(hand.add_card(sup.instance_id, deck, fake_tier_of), std::invalid_argument);
    REQUIRE(hand.size() == 0);
}

TEST_CASE("hand_loadout: Gus (is_universal_compiler=true) aceita 1 especial no slot dedicado",
          "[domain][deck][hand_loadout]") {
    CardCollection deck(kDeckCapacityTier1);
    HandLoadout hand(kHandSizeBase, /*is_universal_compiler=*/true);
    const CardInstance esp = deck.add_to_active("mestre_hipotenusa_especial");

    REQUIRE_NOTHROW(hand.add_card(esp.instance_id, deck, fake_tier_of));
    REQUIRE(hand.size() == 1);
}

TEST_CASE("hand_loadout: rejeita 2a carta especial MESMO pro Gus (limite 1 slot)",
          "[domain][deck][hand_loadout][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    HandLoadout hand(kHandSizeBase, /*is_universal_compiler=*/true);
    const CardInstance esp1 = deck.add_to_active("mestre_hipotenusa_especial");
    const CardInstance esp2 = deck.add_to_active("mestre_pitagoras_especial");

    hand.add_card(esp1.instance_id, deck, fake_tier_of);
    REQUIRE_THROWS_AS(hand.add_card(esp2.instance_id, deck, fake_tier_of), std::invalid_argument);
    REQUIRE(hand.size() == 1);
}

TEST_CASE("hand_loadout: slot especial do Gus NAO compete com a capacidade de comuns",
          "[domain][deck][hand_loadout]") {
    CardCollection deck(kDeckCapacityTier1);
    HandLoadout hand(/*comum_capacity=*/2, /*is_universal_compiler=*/true);
    const CardInstance c1 = deck.add_to_active("pulso_a");
    const CardInstance c2 = deck.add_to_active("pulso_b");
    const CardInstance esp = deck.add_to_active("mestre_hipotenusa_especial");

    hand.add_card(c1.instance_id, deck, fake_tier_of);
    hand.add_card(c2.instance_id, deck, fake_tier_of);
    // Comuns ja na capacidade (2/2), mas a especial usa o slot DEDICADO, nao o pool
    // de comuns - deve caber sem estourar.
    REQUIRE_NOTHROW(hand.add_card(esp.instance_id, deck, fake_tier_of));
    REQUIRE(hand.size() == 3);
}

// ---- hand_capacity(): teto (~6) e delta -1 do Bento --------------------------------

TEST_CASE("hand_capacity: base normal sem bonus retorna o proprio base",
          "[domain][deck][hand_loadout][constants]") {
    REQUIRE(hand_capacity(kHandSizeBase, 0, 0) == kHandSizeBase);
}

TEST_CASE("hand_capacity: delta do Bento reduz a mao base em 1",
          "[domain][deck][hand_loadout][constants]") {
    const int bento_base = kHandSizeBase + kHandDeltaBento;
    REQUIRE(bento_base == 4);
    REQUIRE(hand_capacity(bento_base, 0, 0) == 4);
}

TEST_CASE("hand_capacity: teto do stat mental trava em kHandSizeMentalStatCap (anti-snowball)",
          "[domain][deck][hand_loadout][constants]") {
    // base 5 + hardware 5 + stat mental 5 estouraria 15 sem teto - trava em 6.
    REQUIRE(hand_capacity(kHandSizeBase, 5, 5) == kHandSizeMentalStatCap);
}

TEST_CASE("hand_capacity: soma normal abaixo do teto passa direto",
          "[domain][deck][hand_loadout][constants]") {
    REQUIRE(hand_capacity(kHandSizeBase, 1, 0) == kHandSizeBase + 1);
}

// ---- Atomicidade: falha nao muta a selecao atual -----------------------------------

TEST_CASE("hand_loadout: swap_card falho preserva a selecao atual (atomico)",
          "[domain][deck][hand_loadout]") {
    CardCollection deck(kDeckCapacityTier1);
    HandLoadout hand(kHandSizeBase, /*is_universal_compiler=*/false);
    const CardInstance a = deck.add_to_active("pulso_a");
    const CardInstance esp = deck.add_to_active("mestre_hipotenusa_especial");
    hand.add_card(a.instance_id, deck, fake_tier_of);

    // Trocar 'a' por uma especial falha (sem flag is_universal_compiler) - selecao
    // deve continuar com 'a', NAO ficar vazia nem com a especial.
    REQUIRE_THROWS_AS(hand.swap_card(a.instance_id, esp.instance_id, deck, fake_tier_of),
                       std::invalid_argument);
    REQUIRE(hand.size() == 1);
    REQUIRE(hand.selection().front() == a.instance_id);
}

TEST_CASE("hand_loadout: swap_card de sucesso troca o ID exato",
          "[domain][deck][hand_loadout]") {
    CardCollection deck(kDeckCapacityTier1);
    HandLoadout hand(kHandSizeBase);
    const CardInstance a = deck.add_to_active("pulso_a");
    const CardInstance b = deck.add_to_active("pulso_b");
    hand.add_card(a.instance_id, deck, fake_tier_of);

    hand.swap_card(a.instance_id, b.instance_id, deck, fake_tier_of);

    REQUIRE(hand.size() == 1);
    REQUIRE(hand.selection().front() == b.instance_id);
}

TEST_CASE("hand_loadout: swap_card falha se old_instance_id nao esta na selecao",
          "[domain][deck][hand_loadout]") {
    CardCollection deck(kDeckCapacityTier1);
    HandLoadout hand(kHandSizeBase);
    const CardInstance b = deck.add_to_active("pulso_b");

    REQUIRE_THROWS_AS(hand.swap_card(999, b.instance_id, deck, fake_tier_of),
                       std::invalid_argument);
    REQUIRE(hand.size() == 0);
}

TEST_CASE("hand_loadout: remove_card falha se o ID nao esta selecionado",
          "[domain][deck][hand_loadout]") {
    HandLoadout hand(kHandSizeBase);
    REQUIRE_THROWS_AS(hand.remove_card(123), std::invalid_argument);
}

TEST_CASE("hand_loadout: remove_card retira exatamente o ID pedido",
          "[domain][deck][hand_loadout]") {
    CardCollection deck(kDeckCapacityTier1);
    HandLoadout hand(kHandSizeBase);
    const CardInstance a = deck.add_to_active("pulso_a");
    const CardInstance b = deck.add_to_active("pulso_b");
    hand.add_card(a.instance_id, deck, fake_tier_of);
    hand.add_card(b.instance_id, deck, fake_tier_of);

    hand.remove_card(a.instance_id);

    REQUIRE(hand.size() == 1);
    REQUIRE(hand.selection().front() == b.instance_id);
}

// ---- Revalidacao: ID orfao (carta saiu do deck ativo por fora da mao) --------------

TEST_CASE("hand_loadout: find_orphan_instance_ids detecta carta descartada por fora",
          "[domain][deck][hand_loadout][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    HandLoadout hand(kHandSizeBase);
    const CardInstance a = deck.add_to_active("pulso_a");
    const CardInstance b = deck.add_to_active("pulso_b");
    hand.add_card(a.instance_id, deck, fake_tier_of);
    hand.add_card(b.instance_id, deck, fake_tier_of);

    // 'a' sai do deck ativo por fora da mao (descarte/venda - DECK-1/3).
    deck.discard_to_dead(a.instance_id, fake_tier_of);

    const auto orphans = hand.find_orphan_instance_ids(deck);
    REQUIRE(orphans.size() == 1);
    REQUIRE(orphans.front() == a.instance_id);
    // find_orphan_instance_ids e read-only - a selecao ainda tem os 2 IDs.
    REQUIRE(hand.size() == 2);
}

TEST_CASE("hand_loadout: revalidate remove os IDs orfaos da selecao",
          "[domain][deck][hand_loadout][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    HandLoadout hand(kHandSizeBase);
    const CardInstance a = deck.add_to_active("pulso_a");
    const CardInstance b = deck.add_to_active("pulso_b");
    hand.add_card(a.instance_id, deck, fake_tier_of);
    hand.add_card(b.instance_id, deck, fake_tier_of);

    deck.discard_to_dead(a.instance_id, fake_tier_of);
    const auto removed = hand.revalidate(deck);

    REQUIRE(removed.size() == 1);
    REQUIRE(removed.front() == a.instance_id);
    REQUIRE(hand.size() == 1);
    REQUIRE(hand.selection().front() == b.instance_id);
}

TEST_CASE("hand_loadout: revalidate sem orfaos e no-op (nada removido)",
          "[domain][deck][hand_loadout]") {
    CardCollection deck(kDeckCapacityTier1);
    HandLoadout hand(kHandSizeBase);
    const CardInstance a = deck.add_to_active("pulso_a");
    hand.add_card(a.instance_id, deck, fake_tier_of);

    const auto removed = hand.revalidate(deck);

    REQUIRE(removed.empty());
    REQUIRE(hand.size() == 1);
}

// ---- Construtor: fail-fast em capacidade negativa ----------------------------------

TEST_CASE("hand_loadout: construtor rejeita comum_capacity negativa",
          "[domain][deck][hand_loadout]") {
    REQUIRE_THROWS_AS(HandLoadout(-1), std::invalid_argument);
}

// ---- Getters basicos ----------------------------------------------------------------

TEST_CASE("hand_loadout: getters refletem capacidade e flag construidas",
          "[domain][deck][hand_loadout]") {
    HandLoadout hand(4, /*is_universal_compiler=*/true);
    REQUIRE(hand.comum_capacity() == 4);
    REQUIRE(hand.is_universal_compiler());
    REQUIRE(hand.size() == 0);
    REQUIRE(hand.selection().empty());
}
