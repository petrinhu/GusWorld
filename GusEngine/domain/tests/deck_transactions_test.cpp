// deck_transactions_test.cpp
//
// Spec executavel (Catch2 v3) das transacoes atomicas do sistema de deck/mao (DECK-3):
// sell/upload/acquire/craft sobre CardCollection (DECK-1) + wallet (int64_t credits).
// POCO puro, ZERO SDL/glintfx. Cobre:
//
//   - sell(): remove do ativo + credita; idempotente (2a venda da mesma instancia
//     rejeitada, credita 1x so); rejeita CardTier::Especial/Super (herda o guard de
//     CardCollection::remove_for_sale, inv.9).
//   - upload(): mesma mecanica de sell() com preco proprio (upload ao commons).
//   - acquire(): debita saldo + adiciona ao ativo; saldo insuficiente nao muta nada;
//     estouro de capacidade do ativo nao muta nada; preco 0 (loot garantido/achado) nao
//     debita.
//   - craft(): consome materiais (via MaterialConsumer) + adiciona carta; consumer
//     que devolve false nao adiciona carta (nem consome de novo); capacidade cheia
//     do ativo NUNCA invoca o consumer (nao "gasta" material a toa).
//   - idempotencia geral: toda falha deixa collection E credits EXATAMENTE como
//     estavam antes da chamada (garantia forte).
//
// Cross-ref: docs/design/mecanicas/deck-mao-sistema.md secao 6.1/6.2/7/8c;
//            gus/domain/deck/deck_transactions.hpp / card_collection.hpp /
//            deck_constants.hpp.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/deck/card_collection.hpp"
#include "gus/domain/deck/deck_constants.hpp"
#include "gus/domain/deck/deck_transactions.hpp"

using namespace gus::domain::deck;
using gus::domain::combat::CardTier;

namespace {

// Mesma convencao do card_collection_test.cpp/hand_loadout_test.cpp: sufixo
// "_especial"/"_super" marca a classe protegida (secao 7.9), sem depender do registry
// real de combate.
CardTier fake_tier_of(const std::string& card_id) {
    if (card_id.find("_especial") != std::string::npos) return CardTier::Especial;
    if (card_id.find("_super") != std::string::npos) return CardTier::Super;
    return CardTier::Comum;
}

}  // namespace

// ---- sell(): venda basica + idempotencia (double-sell) -----------------------------

TEST_CASE("deck_transactions: sell remove do ativo e credita o preco",
          "[domain][deck][deck_transactions]") {
    CardCollection deck(kDeckCapacityTier1);
    std::int64_t credits = 0;
    const CardInstance a = deck.add_to_active("pulso_eletrico");

    const SellResult result = sell(deck, credits, a.instance_id, kNpcSellPriceMin, fake_tier_of);

    REQUIRE(result.ok());
    REQUIRE(result.instance.instance_id == a.instance_id);
    REQUIRE(result.credited == kNpcSellPriceMin);
    REQUIRE(credits == kNpcSellPriceMin);
    REQUIRE(deck.active_count() == 0);
}

TEST_CASE("deck_transactions: double-sell da mesma instancia e rejeitado, credita SO 1x",
          "[domain][deck][deck_transactions][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    std::int64_t credits = 0;
    const CardInstance a = deck.add_to_active("pulso_eletrico");

    const SellResult first = sell(deck, credits, a.instance_id, kNpcSellPriceMin, fake_tier_of);
    REQUIRE(first.ok());
    REQUIRE(credits == kNpcSellPriceMin);

    // 2a venda da MESMA instancia (ja fora do ativo) - idempotencia (inv.5): rejeitada,
    // credits continua exatamente no valor da 1a venda (nunca credita 2x).
    const SellResult second = sell(deck, credits, a.instance_id, kNpcSellPriceMin, fake_tier_of);
    REQUIRE_FALSE(second.ok());
    REQUIRE(second.error == TransactionError::InstanceNotInActive);
    REQUIRE(credits == kNpcSellPriceMin);
}

TEST_CASE("deck_transactions: sell rejeita carta ESPECIAL (classe protegida, inv.9)",
          "[domain][deck][deck_transactions][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    std::int64_t credits = 100;
    const CardInstance esp = deck.add_to_active("mestre_hipotenusa_especial");

    const SellResult result = sell(deck, credits, esp.instance_id, kNpcSellPriceMax, fake_tier_of);

    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error == TransactionError::ProtectedTier);
    // Nada mutou: credits intocado, a especial continua no ativo.
    REQUIRE(credits == 100);
    REQUIRE(deck.active_count() == 1);
}

TEST_CASE("deck_transactions: sell rejeita carta SUPER (classe protegida, inv.9)",
          "[domain][deck][deck_transactions][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    std::int64_t credits = 0;
    const CardInstance sup = deck.add_to_active("helon_tusk_super");

    const SellResult result = sell(deck, credits, sup.instance_id, kNpcSellPriceMax, fake_tier_of);

    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error == TransactionError::ProtectedTier);
    REQUIRE(credits == 0);
    REQUIRE(deck.active_count() == 1);
}

TEST_CASE("deck_transactions: sell rejeita instance_id inexistente (nunca existiu)",
          "[domain][deck][deck_transactions]") {
    CardCollection deck(kDeckCapacityTier1);
    std::int64_t credits = 0;

    const SellResult result = sell(deck, credits, 999, kNpcSellPriceMin, fake_tier_of);

    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error == TransactionError::InstanceNotInActive);
    REQUIRE(credits == 0);
}

TEST_CASE("deck_transactions: sell com preco negativo e fail-fast (invariante de programacao)",
          "[domain][deck][deck_transactions]") {
    CardCollection deck(kDeckCapacityTier1);
    std::int64_t credits = 0;
    const CardInstance a = deck.add_to_active("pulso_a");

    REQUIRE_THROWS_AS(sell(deck, credits, a.instance_id, -1, fake_tier_of), std::invalid_argument);
}

// ---- upload(): mesma mecanica de sell() com preco proprio (secao 6.2) --------------

TEST_CASE("deck_transactions: upload remove do ativo e credita o preco de upload",
          "[domain][deck][deck_transactions]") {
    CardCollection deck(kDeckCapacityTier1);
    std::int64_t credits = 0;
    const CardInstance a = deck.add_to_active("pulso_eletrico");

    const UploadResult result = upload(deck, credits, a.instance_id, kUploadCreditMax, fake_tier_of);

    REQUIRE(result.ok());
    REQUIRE(result.instance.instance_id == a.instance_id);
    REQUIRE(result.credited == kUploadCreditMax);
    REQUIRE(credits == kUploadCreditMax);
    REQUIRE(deck.active_count() == 0);
}

TEST_CASE("deck_transactions: double-upload da mesma instancia e rejeitado, credita SO 1x",
          "[domain][deck][deck_transactions][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    std::int64_t credits = 0;
    const CardInstance a = deck.add_to_active("pulso_eletrico");

    const UploadResult first = upload(deck, credits, a.instance_id, kUploadCreditMin, fake_tier_of);
    REQUIRE(first.ok());

    const UploadResult second = upload(deck, credits, a.instance_id, kUploadCreditMin, fake_tier_of);
    REQUIRE_FALSE(second.ok());
    REQUIRE(second.error == TransactionError::InstanceNotInActive);
    REQUIRE(credits == kUploadCreditMin);
}

TEST_CASE("deck_transactions: upload rejeita carta ESPECIAL (classe protegida, inv.9)",
          "[domain][deck][deck_transactions][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    std::int64_t credits = 0;
    const CardInstance esp = deck.add_to_active("mestre_pitagoras_especial");

    const UploadResult result = upload(deck, credits, esp.instance_id, kUploadCreditMax, fake_tier_of);

    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error == TransactionError::ProtectedTier);
    REQUIRE(credits == 0);
    REQUIRE(deck.active_count() == 1);
}

// ---- acquire(): compra/loot/achado - MESMA primitiva --------------------------------

TEST_CASE("deck_transactions: acquire debita o preco e adiciona ao ativo",
          "[domain][deck][deck_transactions]") {
    CardCollection deck(kDeckCapacityTier1);
    std::int64_t credits = kShopBuyPriceMax;

    const AcquireResult result = acquire(deck, credits, "pulso_novo", kShopBuyPriceMax);

    REQUIRE(result.ok());
    REQUIRE(result.instance.card_id == "pulso_novo");
    REQUIRE(result.debited == kShopBuyPriceMax);
    REQUIRE(credits == 0);
    REQUIRE(deck.active_count() == 1);
}

TEST_CASE("deck_transactions: acquire com saldo insuficiente nao muta nada",
          "[domain][deck][deck_transactions][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    std::int64_t credits = 5;  // menor que o preco de loja (12-18)

    const AcquireResult result = acquire(deck, credits, "pulso_novo", kShopBuyPriceMin);

    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error == TransactionError::InsufficientCredits);
    REQUIRE(credits == 5);
    REQUIRE(deck.active_count() == 0);
}

TEST_CASE("deck_transactions: acquire que estouraria a capacidade do ativo nao muta nada",
          "[domain][deck][deck_transactions][invariant]") {
    CardCollection deck(/*active_capacity=*/1);
    std::int64_t credits = 1000;
    deck.add_to_active("ja_ocupa_o_unico_slot");

    const AcquireResult result = acquire(deck, credits, "carta_nova", kShopBuyPriceMin);

    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error == TransactionError::ActiveCapacityFull);
    // Nada mutou: saldo intocado, ativo continua com so a carta original.
    REQUIRE(credits == 1000);
    REQUIRE(deck.active_count() == 1);
    REQUIRE(deck.active().front().card_id == "ja_ocupa_o_unico_slot");
}

TEST_CASE("deck_transactions: acquire com preco 0 (loot garantido/achado) nao debita",
          "[domain][deck][deck_transactions]") {
    CardCollection deck(kDeckCapacityTier1);
    std::int64_t credits = 42;

    const AcquireResult result = acquire(deck, credits, "loot_garantido", 0);

    REQUIRE(result.ok());
    REQUIRE(result.debited == 0);
    REQUIRE(credits == 42);
    REQUIRE(deck.active_count() == 1);
}

TEST_CASE("deck_transactions: acquire com preco negativo e fail-fast (invariante de programacao)",
          "[domain][deck][deck_transactions]") {
    CardCollection deck(kDeckCapacityTier1);
    std::int64_t credits = 0;

    REQUIRE_THROWS_AS(acquire(deck, credits, "carta", -1), std::invalid_argument);
}

// ---- craft(): materiais desacoplados via MaterialConsumer ---------------------------

TEST_CASE("deck_transactions: craft de sucesso consome material e adiciona a carta",
          "[domain][deck][deck_transactions]") {
    CardCollection deck(kDeckCapacityTier1);
    bool consumed = false;
    auto consumer = [&consumed]() {
        consumed = true;
        return true;
    };

    const CraftResult result = craft(deck, "carta_craftada", consumer);

    REQUIRE(result.ok());
    REQUIRE(consumed);
    REQUIRE(result.instance.card_id == "carta_craftada");
    REQUIRE(deck.active_count() == 1);
}

TEST_CASE("deck_transactions: craft sem material (consumer devolve false) nao adiciona carta",
          "[domain][deck][deck_transactions][invariant]") {
    CardCollection deck(kDeckCapacityTier1);
    auto consumer = []() { return false; };

    const CraftResult result = craft(deck, "carta_craftada", consumer);

    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error == TransactionError::MaterialsUnavailable);
    REQUIRE(deck.active_count() == 0);
}

TEST_CASE("deck_transactions: craft que nao cabe no ativo NUNCA invoca o consumer (nao gasta material a toa)",
          "[domain][deck][deck_transactions][invariant]") {
    CardCollection deck(/*active_capacity=*/1);
    deck.add_to_active("ja_ocupa_o_unico_slot");
    bool consumer_called = false;
    auto consumer = [&consumer_called]() {
        consumer_called = true;
        return true;
    };

    const CraftResult result = craft(deck, "carta_craftada", consumer);

    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error == TransactionError::ActiveCapacityFull);
    REQUIRE_FALSE(consumer_called);
    REQUIRE(deck.active_count() == 1);
}
