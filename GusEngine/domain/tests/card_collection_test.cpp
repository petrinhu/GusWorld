// card_collection_test.cpp
//
// Spec executavel (Catch2 v3) do agregado CardCollection (DECK-1): fundacao do sistema
// de deck/mao (bolsa -> mao, docs/design/mecanicas/deck-mao-sistema.md). POCO puro,
// ZERO SDL/glintfx. Cobre os invariantes anti-exploit da secao 7 do spec:
//
//   inv.1 - carta = instancia unica com ID, vive em EXATAMENTE UM container (XOR).
//   inv.3 - deck morto e INERTE: nao conta na capacidade/contagem do deck ativo.
//   inv.4 - one-way por codigo: existe ativo->morto, NAO existe morto->ativo.
//   inv.9 - classe PROTEGIDA (Especial/Super) nunca sai do deck ativo (nem descarte
//           nem venda) - guard de tier.
//
// A MAO (selecao/loadout) e o mercado NPC/loja (DECK-2/DECK-3) ficam FORA de escopo
// aqui - so a mecanica de container (agregado unico dono dos vetores ativo/morto).
//
// Cross-ref: docs/design/mecanicas/deck-mao-sistema.md secao 4/6.2/7/8c;
//            gus/domain/combat/combat_enums.hpp (CardTier);
//            gus/domain/deck/card_collection.hpp / deck_constants.hpp / deck_records.hpp.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/deck/card_collection.hpp"
#include "gus/domain/deck/deck_constants.hpp"
#include "gus/domain/deck/deck_records.hpp"

using namespace gus::domain::deck;
using gus::domain::combat::CardTier;

namespace {

// Registry-de-catalogo fake pro teste: so precisamos do tier por card_id (assinatura
// TierLookup = CardTier(const std::string&)), sem depender do registry real de combate.
// Convencao local: sufixo "_especial"/"_super" marca a classe protegida (secao 7.9).
CardTier fake_tier_of(const std::string& card_id) {
    if (card_id.find("_especial") != std::string::npos) return CardTier::Especial;
    if (card_id.find("_super") != std::string::npos) return CardTier::Super;
    return CardTier::Comum;
}

}  // namespace

// ---- add_to_active: geracao deterministica de instance_id -------------------------

TEST_CASE("card_collection: add_to_active gera instance_id sequencial deterministico",
          "[domain][deck][card_collection]") {
    CardCollection deck(kDeckCapacityTier1);

    const CardInstance a = deck.add_to_active("pulso_eletrico");
    const CardInstance b = deck.add_to_active("pulso_eletrico");  // mesmo card_id, instancia distinta

    REQUIRE(a.instance_id != b.instance_id);
    REQUIRE(a.card_id == "pulso_eletrico");
    REQUIRE(b.card_id == "pulso_eletrico");
    // Determinismo: sequencial, comeca em 1, sem RNG/timestamp (spec secao 7 inv.1).
    REQUIRE(a.instance_id == 1);
    REQUIRE(b.instance_id == 2);
}

TEST_CASE("card_collection: instance_id nunca reusa mesmo apos ir pro deck morto",
          "[domain][deck][card_collection]") {
    CardCollection deck(kDeckCapacityTier1);

    const CardInstance a = deck.add_to_active("pulso_eletrico");
    deck.discard_to_dead(a.instance_id, fake_tier_of);
    const CardInstance b = deck.add_to_active("pulso_eletrico");

    REQUIRE(b.instance_id != a.instance_id);
    REQUIRE(b.instance_id == 2);  // contador nao volta atras
}

TEST_CASE("card_collection: add_to_active com instance_id explicito ja presente e rejeitado",
          "[domain][deck][card_collection]") {
    CardCollection deck(kDeckCapacityTier1);
    deck.add_to_active("pulso_eletrico", /*instance_id_override=*/std::uint64_t{42});

    REQUIRE_THROWS_AS(
        deck.add_to_active("pulso_bio", /*instance_id_override=*/std::uint64_t{42}),
        std::invalid_argument);
    // Rejeitado -> nao duplicou o container.
    REQUIRE(deck.active_count() == 1);
}

// ---- XOR de container: instancia nunca em ativo E morto ao mesmo tempo ------------

TEST_CASE("card_collection: instancia esta SEMPRE em EXATAMENTE um container (XOR)",
          "[domain][deck][card_collection][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    const CardInstance a = deck.add_to_active("pulso_eletrico");

    // Recem-criada: presente no ativo, ausente no morto.
    bool in_active = false;
    for (const auto& c : deck.active()) {
        if (c.instance_id == a.instance_id) in_active = true;
    }
    REQUIRE(in_active);
    REQUIRE(deck.dead().empty());

    deck.discard_to_dead(a.instance_id, fake_tier_of);

    // Pos-descarte: ausente no ativo, presente no morto. Nunca os dois.
    bool still_in_active = false;
    for (const auto& c : deck.active()) {
        if (c.instance_id == a.instance_id) still_in_active = true;
    }
    bool now_in_dead = false;
    for (const auto& c : deck.dead()) {
        if (c.instance_id == a.instance_id) now_in_dead = true;
    }
    REQUIRE_FALSE(still_in_active);
    REQUIRE(now_in_dead);
}

// ---- discard_to_dead: move ativo -> morto ------------------------------------------

TEST_CASE("card_collection: discard_to_dead move a instancia do ativo pro morto",
          "[domain][deck][card_collection]") {
    CardCollection deck(kDeckCapacityTier1);
    const CardInstance a = deck.add_to_active("pulso_eletrico");
    REQUIRE(deck.active_count() == 1);

    deck.discard_to_dead(a.instance_id, fake_tier_of);

    REQUIRE(deck.active_count() == 0);
    REQUIRE(deck.dead().size() == 1);
    REQUIRE(deck.dead().front().instance_id == a.instance_id);
    REQUIRE(deck.dead().front().card_id == "pulso_eletrico");
}

TEST_CASE("card_collection: discard_to_dead de instance_id ausente do ativo falha",
          "[domain][deck][card_collection]") {
    CardCollection deck(kDeckCapacityTier1);
    REQUIRE_THROWS_AS(deck.discard_to_dead(999, fake_tier_of), std::invalid_argument);
}

// ---- Deck morto e INERTE: nao conta na capacidade/contagem do ativo (inv.3) -------

TEST_CASE("card_collection: deck morto NAO conta na capacidade/contagem do deck ativo",
          "[domain][deck][card_collection][invariant]") {
    CardCollection deck(/*active_capacity=*/2);

    const CardInstance a = deck.add_to_active("pulso_eletrico");
    deck.add_to_active("pulso_bio");
    REQUIRE(deck.active_is_full());

    deck.discard_to_dead(a.instance_id, fake_tier_of);
    REQUIRE(deck.dead().size() == 1);
    // Capacidade liberada: o morto nao ocupa slot do ativo.
    REQUIRE_FALSE(deck.active_is_full());
    REQUIRE(deck.active_count() == 1);

    // E da pra adicionar de novo ate a capacidade, independente do morto ter 1 carta.
    deck.add_to_active("pulso_sonico");
    REQUIRE(deck.active_is_full());
    REQUIRE(deck.dead().size() == 1);
}

// ---- Capacidade por patamar (34/55/89, secao 8c) -----------------------------------

TEST_CASE("card_collection: capacidade do deck ativo respeita o patamar 34/55/89",
          "[domain][deck][card_collection][constants]") {
    REQUIRE(kDeckCapacityTier1 == 34);
    REQUIRE(kDeckCapacityTier2 == 55);
    REQUIRE(kDeckCapacityTier3 == 89);

    CardCollection deck(kDeckCapacityTier1);
    for (int i = 0; i < kDeckCapacityTier1; ++i) {
        deck.add_to_active("pulso_" + std::to_string(i));
    }
    REQUIRE(deck.active_is_full());
    REQUIRE_THROWS_AS(deck.add_to_active("estouro"), std::logic_error);
    REQUIRE(deck.active_count() == static_cast<std::size_t>(kDeckCapacityTier1));
}

// ---- Classe PROTEGIDA: Especial/Super nunca saem do ativo (inv.9, secao 7.9) ------

TEST_CASE("card_collection: discard_to_dead RECUSA carta Especial (classe protegida)",
          "[domain][deck][card_collection][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    const CardInstance esp = deck.add_to_active("mestre_hipotenusa_especial");

    REQUIRE_THROWS_AS(deck.discard_to_dead(esp.instance_id, fake_tier_of),
                       std::invalid_argument);
    // Rejeitado -> continua no ativo, deck morto continua vazio.
    REQUIRE(deck.active_count() == 1);
    REQUIRE(deck.dead().empty());
}

TEST_CASE("card_collection: discard_to_dead RECUSA carta Super (classe protegida)",
          "[domain][deck][card_collection][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    const CardInstance sup = deck.add_to_active("helon_tusk_super");

    REQUIRE_THROWS_AS(deck.discard_to_dead(sup.instance_id, fake_tier_of),
                       std::invalid_argument);
    REQUIRE(deck.active_count() == 1);
    REQUIRE(deck.dead().empty());
}

TEST_CASE("card_collection: discard_to_dead de carta Comum funciona normalmente",
          "[domain][deck][card_collection]") {
    CardCollection deck(kDeckCapacityTier1);
    const CardInstance comum = deck.add_to_active("pulso_eletrico");

    REQUIRE_NOTHROW(deck.discard_to_dead(comum.instance_id, fake_tier_of));
    REQUIRE(deck.dead().size() == 1);
}

// ---- remove_for_sale: retira do ativo (DECK-3 monta a transacao por cima) ---------

TEST_CASE("card_collection: remove_for_sale retira a instancia do deck ativo",
          "[domain][deck][card_collection]") {
    CardCollection deck(kDeckCapacityTier1);
    const CardInstance a = deck.add_to_active("pulso_bio");

    const CardInstance removed = deck.remove_for_sale(a.instance_id, fake_tier_of);

    REQUIRE(removed.instance_id == a.instance_id);
    REQUIRE(removed.card_id == "pulso_bio");
    REQUIRE(deck.active_count() == 0);
    // remove_for_sale NAO manda pro deck morto (e uma saida diferente - venda).
    REQUIRE(deck.dead().empty());
}

TEST_CASE("card_collection: remove_for_sale RECUSA carta Especial/Super (mesmo guard)",
          "[domain][deck][card_collection][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    const CardInstance esp = deck.add_to_active("mestre_pitagoras_especial");
    const CardInstance sup = deck.add_to_active("helon_tusk_super");

    REQUIRE_THROWS_AS(deck.remove_for_sale(esp.instance_id, fake_tier_of),
                       std::invalid_argument);
    REQUIRE_THROWS_AS(deck.remove_for_sale(sup.instance_id, fake_tier_of),
                       std::invalid_argument);
    REQUIRE(deck.active_count() == 2);  // nenhuma das duas saiu
}

TEST_CASE("card_collection: remove_for_sale de instance_id ausente falha",
          "[domain][deck][card_collection]") {
    CardCollection deck(kDeckCapacityTier1);
    REQUIRE_THROWS_AS(deck.remove_for_sale(999, fake_tier_of), std::invalid_argument);
}

// ---- One-way por codigo: NAO existe API morto->ativo (inv.4) ----------------------
// Detector de compilacao (SFINAE/void_t): se algum dia alguem adicionar um metodo tipo
// restore_from_dead/revive/dead_to_active em CardCollection, este trait deixa de ser
// false_type. O invariante do spec ("existe API ativo->morto; NAO existe morto->ativo")
// e garantido por AUSENCIA de metodo - este teste documenta e trava isso em compile-time.
namespace {
template <typename T, typename = void>
struct has_revive_from_dead : std::false_type {};

template <typename T>
struct has_revive_from_dead<
    T, std::void_t<decltype(std::declval<T&>().restore_from_dead(std::uint64_t{}))>>
    : std::true_type {};

static_assert(!has_revive_from_dead<CardCollection>::value,
              "CardCollection NAO pode expor um caminho morto->ativo (deck-mao-sistema.md "
              "secao 7 inv.4 - one-way por AUSENCIA de API, nao por guard em runtime)");
}  // namespace

TEST_CASE("card_collection: NAO existe API morto->ativo (one-way por ausencia, inv.4)",
          "[domain][deck][card_collection][invariant]") {
    // O static_assert acima ja falha a COMPILACAO se o metodo existir; este TEST_CASE
    // so registra o invariante na suite (visibilidade no relatorio do ctest).
    SUCCEED("one-way garantido em compile-time: CardCollection nao expoe morto->ativo");
}

// ---- Constantes de deck_constants.hpp: presenca + valores //PLAYTEST (secao 8c) ---

TEST_CASE("deck_constants: valores baseline //PLAYTEST da mao/deck (secao 8c)",
          "[domain][deck][constants]") {
    REQUIRE(kHandSizeBase == 5);
    REQUIRE(kHandDeltaBento == -1);
    REQUIRE(kHandSizeMentalStatCap == 6);
    REQUIRE(kGusSpecialHandSlots == 1);
    REQUIRE(kDeadDeckLimit == 8);
    REQUIRE(kUploadCreditMin == 2);
    REQUIRE(kUploadCreditMax == 3);
    REQUIRE(kShopBuyPriceMin == 12);
    REQUIRE(kShopBuyPriceMax == 18);
    REQUIRE(kNpcSellPriceMin == 3);
    REQUIRE(kNpcSellPriceMax == 5);
    REQUIRE(kShopSellPriceMin == 4);
    REQUIRE(kShopSellPriceMax == 6);
    REQUIRE(kGrassPityCap == 13);
    REQUIRE(kGrassBaseRatePercentMin == 10);
    REQUIRE(kGrassBaseRatePercentMax == 15);
}
