// placeholder_cards_test.cpp
//
// Spec executavel (Catch2 v3) do registry in-memory de cartas placeholder do vertical
// slice (1 por familia), portado de engine/foundation/turn_combat/PlaceholderCards.cs.
// POCO puro, ZERO Qt. Serve so para exercitar UseCard; nao e o CardRepository final.
//
// PlaceholderCards.cs nao tem teste xUnit dedicado; aqui fixamos as 5 cartas + seus
// campos canonicos (familia, base type, power, mana, status aplicado), pra a FSM e a
// cena de combate consumirem com paridade.
//
// Cross-ref: engine/foundation/turn_combat/PlaceholderCards.cs; docs/design/mecanicas/combat.md secao 9/10.

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/placeholder_cards.hpp"

using namespace gus::domain::combat;

TEST_CASE("placeholder_cards: 5 cartas, uma por familia", "[domain][combat][cards]") {
    const auto& cards = PlaceholderCards::all();
    REQUIRE(cards.size() == 5);
    REQUIRE(cards.count("pulso_eletrico") == 1);
    REQUIRE(cards.count("raiz_cura") == 1);
    REQUIRE(cards.count("eco_sonico") == 1);
    REQUIRE(cards.count("fenda_criptica") == 1);
    REQUIRE(cards.count("impacto_cinetico") == 1);
}

TEST_CASE("placeholder_cards: pulso_eletrico = Eletrico/Pulso power 6 + Stun",
          "[domain][combat][cards]") {
    const Card& c = PlaceholderCards::all().at("pulso_eletrico");
    REQUIRE(c.display_name == "CARD_PULSO_ELETRICO_NAME");
    REQUIRE(c.family == CardFamily::Eletrico);
    REQUIRE(c.base_type == CardBaseType::Pulso);
    REQUIRE(c.mana_cost == 1);
    REQUIRE(c.ap_cost == 1);
    REQUIRE(c.power == 6);
    REQUIRE(c.target_shape == TargetShape::Single);
    REQUIRE(c.crit_chance == 0);
    REQUIRE(c.mastery == 0);
    REQUIRE(c.modifiers.empty());
    REQUIRE(c.status_applied.has_value());
    REQUIRE(c.status_applied->id == StatusId::Stun);
    REQUIRE(c.status_applied->magnitude == 2);
    REQUIRE(c.status_applied->duration == 1);
    REQUIRE(c.status_applied->stack_rule == StackRule::Replace);
    REQUIRE(c.status_applied->family_origin == CardFamily::Eletrico);
}

TEST_CASE("placeholder_cards: raiz_cura = Bioquimico/Raiz self Regen, power 0",
          "[domain][combat][cards]") {
    const Card& c = PlaceholderCards::all().at("raiz_cura");
    REQUIRE(c.family == CardFamily::Bioquimico);
    REQUIRE(c.base_type == CardBaseType::Raiz);
    REQUIRE(c.mana_cost == 2);
    REQUIRE(c.power == 0);
    REQUIRE(c.target_shape == TargetShape::Self);
    REQUIRE(c.status_applied->id == StatusId::Regen);
    REQUIRE(c.status_applied->magnitude == 8);
    REQUIRE(c.status_applied->duration == 2);
    REQUIRE(c.status_applied->stack_rule == StackRule::Refresh);
}

TEST_CASE("placeholder_cards: eco_sonico Disrupt, fenda_criptica Expose, impacto_cinetico Knockback",
          "[domain][combat][cards]") {
    const Card& eco = PlaceholderCards::all().at("eco_sonico");
    REQUIRE(eco.family == CardFamily::Sonico);
    REQUIRE(eco.base_type == CardBaseType::Eco);
    REQUIRE(eco.power == 4);
    REQUIRE(eco.status_applied->id == StatusId::Disrupt);
    REQUIRE(eco.status_applied->magnitude == 20);

    const Card& fenda = PlaceholderCards::all().at("fenda_criptica");
    REQUIRE(fenda.family == CardFamily::Criptografico);
    REQUIRE(fenda.base_type == CardBaseType::Fenda);
    REQUIRE(fenda.mana_cost == 2);
    REQUIRE(fenda.power == 5);
    REQUIRE(fenda.status_applied->id == StatusId::Expose);
    REQUIRE(fenda.status_applied->magnitude == 30);
    REQUIRE(fenda.status_applied->duration == 2);
    REQUIRE(fenda.status_applied->stack_rule == StackRule::Refresh);

    const Card& impacto = PlaceholderCards::all().at("impacto_cinetico");
    REQUIRE(impacto.family == CardFamily::Cinetico);
    REQUIRE(impacto.base_type == CardBaseType::Pulso);
    REQUIRE(impacto.power == 7);
    REQUIRE(impacto.status_applied->id == StatusId::Knockback);
    REQUIRE(impacto.status_applied->magnitude == 1);
}

TEST_CASE("placeholder_cards: mesma instancia imutavel em toda chamada",
          "[domain][combat][cards]") {
    REQUIRE(&PlaceholderCards::all() == &PlaceholderCards::all());
}
