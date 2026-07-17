// deck_tier_callback_lifetime_test.cpp
//
// REGRESSAO (heap-use-after-free): o sistema de deck/mao passa uma REFERENCIA pra um
// elemento de CardCollection::active() (o card_id) pra dentro do callback opaco
// TierLookup (tier_of). Enquanto o iterador/referencia esta "vivo", se tier_of tocar o
// agregado e realocar o vetor active_ (ex.: add_to_active -> push_back que cresce o
// buffer), a referencia PENDURA - qualquer leitura posterior de card_id (ou do proprio
// iterador) e use-after-free.
//
// A raiz nao dependia de RNG: era 100% deterministica dado um tier_of que muta o deck.
// A suite normal usava so tier_of PUROS, entao o buraco era latente (aparecia como
// crash intermitente/ASLR-dependente em heap corruption, nunca pego pela suite verde).
// Estes casos tornam o UAF DETERMINISTICO: com um tier_of adversarial que forca uma
// realocacao de active_ ANTES de ler card_id, o codigo pre-fix estoura ASan em
// hand_loadout.cpp / deck_transactions.cpp / card_collection.cpp; o codigo pos-fix
// (copia estavel do card_id antes do callback + reancoragem do iterador) sobrevive e
// mantem o comportamento correto.
//
// Rode sob ASan (preset *asan*) pra que o UAF vire erro fatal; no build normal os casos
// simplesmente passam (a copia torna a operacao segura). POCO puro, ZERO SDL/glintfx.
//
// Cross-ref: hand_loadout.cpp::validate_candidate; deck_transactions.cpp::
// remove_and_credit; card_collection.cpp::discard_to_dead/remove_for_sale.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/deck/card_collection.hpp"
#include "gus/domain/deck/deck_transactions.hpp"
#include "gus/domain/deck/hand_loadout.hpp"

using namespace gus::domain::deck;
using gus::domain::combat::CardTier;

namespace {

// tier_of ADVERSARIAL: muta o MESMO agregado (realoca active_ com folga) ANTES de ler o
// card_id recebido. No codigo pre-fix, card_id era uma referencia pro buffer que acabou
// de ser realocado -> heap-use-after-free na leitura abaixo. No codigo pos-fix, card_id
// e uma copia estavel -> leitura segura. Retorna sempre Comum (deixa a operacao seguir
// ate o erase/push_back, exercitando tambem a reancoragem do iterador).
CardCollection::TierLookup make_reallocating_tier_of(CardCollection& deck) {
    return [&deck](const std::string& card_id) -> CardTier {
        // Cresce active_ MUITO alem da capacidade atual do buffer -> realocacao garantida
        // -> o buffer antigo (que card_id referenciava, pre-fix) e liberado.
        for (int k = 0; k < 48; ++k) {
            deck.add_to_active("realloc_filler_" + std::to_string(k));
        }
        // Leitura de card_id APOS a realocacao: o gatilho exato do UAF pre-fix.
        volatile std::size_t sink = card_id.size();
        (void)card_id.rfind("esp_", 0);
        (void)sink;
        return CardTier::Comum;
    };
}

}  // namespace

TEST_CASE("deck lifetime: HandLoadout::add_card sobrevive a tier_of que realoca active_ "
          "(sem use-after-free do card_id)",
          "[domain][deck][lifetime][regression]") {
    CardCollection deck(/*active_capacity*/ 100000);
    for (int i = 0; i < 2; ++i) deck.add_to_active("card_" + std::to_string(i));
    const std::uint64_t target = deck.active().front().instance_id;

    HandLoadout hand(/*comum_capacity*/ 8, /*is_universal_compiler*/ true);
    auto evil = make_reallocating_tier_of(deck);

    REQUIRE_NOTHROW(hand.add_card(target, deck, evil));  // validate_candidate -> evil(card_id)
    REQUIRE(hand.size() == 1);
    REQUIRE(hand.selection().front() == target);
}

TEST_CASE("deck lifetime: HandLoadout::set_selection sobrevive a tier_of que realoca "
          "active_",
          "[domain][deck][lifetime][regression]") {
    CardCollection deck(100000);
    for (int i = 0; i < 3; ++i) deck.add_to_active("card_" + std::to_string(i));
    const std::uint64_t a = deck.active()[0].instance_id;
    const std::uint64_t b = deck.active()[1].instance_id;

    HandLoadout hand(8, true);
    auto evil = make_reallocating_tier_of(deck);

    REQUIRE_NOTHROW(hand.set_selection({a, b}, deck, evil));
    REQUIRE(hand.size() == 2);
}

TEST_CASE("deck lifetime: HandLoadout::swap_card sobrevive a tier_of que realoca active_",
          "[domain][deck][lifetime][regression]") {
    CardCollection deck(100000);
    for (int i = 0; i < 2; ++i) deck.add_to_active("card_" + std::to_string(i));
    const std::uint64_t a = deck.active()[0].instance_id;
    const std::uint64_t b = deck.active()[1].instance_id;

    HandLoadout hand(8, true);
    // Monta a mao com um tier_of puro antes (o swap e quem exercita o adversarial).
    CardCollection::TierLookup pure = [](const std::string&) { return CardTier::Comum; };
    hand.add_card(a, deck, pure);

    auto evil = make_reallocating_tier_of(deck);
    REQUIRE_NOTHROW(hand.swap_card(a, b, deck, evil));
    REQUIRE(hand.size() == 1);
    REQUIRE(hand.selection().front() == b);
}

TEST_CASE("deck lifetime: sell()/upload() sobrevivem a tier_of que realoca active_",
          "[domain][deck][lifetime][regression]") {
    CardCollection deck(100000);
    for (int i = 0; i < 2; ++i) deck.add_to_active("card_" + std::to_string(i));
    const std::uint64_t sell_id = deck.active()[0].instance_id;
    const std::uint64_t upload_id = deck.active()[1].instance_id;
    std::int64_t credits = 0;

    auto evil = make_reallocating_tier_of(deck);

    const SellResult sr = sell(deck, credits, sell_id, /*price*/ 5, evil);
    REQUIRE(sr.ok());
    REQUIRE(credits == 5);

    const UploadResult ur = upload(deck, credits, upload_id, /*price*/ 3, evil);
    REQUIRE(ur.ok());
    REQUIRE(credits == 8);
}

TEST_CASE("deck lifetime: CardCollection::discard_to_dead sobrevive a tier_of que realoca "
          "active_",
          "[domain][deck][lifetime][regression]") {
    CardCollection deck(100000);
    for (int i = 0; i < 2; ++i) deck.add_to_active("card_" + std::to_string(i));
    const std::uint64_t target = deck.active().front().instance_id;

    auto evil = make_reallocating_tier_of(deck);
    REQUIRE_NOTHROW(deck.discard_to_dead(target, evil));

    // A instancia alvo saiu do ativo e foi pro morto (comportamento preservado).
    bool in_dead = false;
    for (const auto& c : deck.dead()) {
        if (c.instance_id == target) in_dead = true;
    }
    REQUIRE(in_dead);
}

TEST_CASE("deck lifetime: CardCollection::remove_for_sale sobrevive a tier_of que realoca "
          "active_",
          "[domain][deck][lifetime][regression]") {
    CardCollection deck(100000);
    for (int i = 0; i < 2; ++i) deck.add_to_active("card_" + std::to_string(i));
    const std::uint64_t target = deck.active().front().instance_id;

    auto evil = make_reallocating_tier_of(deck);
    CardInstance removed;
    REQUIRE_NOTHROW(removed = deck.remove_for_sale(target, evil));
    REQUIRE(removed.instance_id == target);

    // Nao voltou pro ativo nem foi parar no morto (remove_for_sale so retira).
    bool still_active = false;
    for (const auto& c : deck.active()) {
        if (c.instance_id == target) still_active = true;
    }
    REQUIRE_FALSE(still_active);
}
