// character_template_test.cpp
//
// Spec executavel (Catch2 v3) do CharacterTemplate + EnemyTemplate + CardFamily
// + BrainKind, portada de engine/foundation/data/{CharacterTemplate,EnemyTemplate}.cs
// e dos testes C# (xUnit). Record imutavel + Validate() fail-fast.
//
// Subsistema: domain/templates (engine-design.md secao 2, marco M3). POCO puro,
// ZERO Qt, headless. Invariantes espelham CombatActor.cs (Id nao vazio; MaxHp>0;
// Atk/Def/Spd>=0; BaseDeck nunca null, sem id vazio).
//
// Cross-ref: engine/foundation/data/CharacterTemplate.cs, EnemyTemplate.cs;
//            docs/design/mecanicas/combat.md secao 17; ADR-006.

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>
#include <vector>

#include "gus/domain/templates/card_family.hpp"
#include "gus/domain/templates/character_template.hpp"
#include "gus/domain/templates/enemy_template.hpp"

using gus::domain::templates::BrainKind;
using gus::domain::templates::CardFamily;
using gus::domain::templates::CharacterTemplate;
using gus::domain::templates::EnemyTemplate;

namespace {

CharacterTemplate gus_fixture() {
    return CharacterTemplate{
        /*id=*/"gus",
        /*max_hp=*/34,
        /*atk=*/8,
        /*def=*/5,
        /*spd=*/9,
        /*family=*/CardFamily::Eletrico,
        /*is_universal_compiler=*/true,
        /*base_deck=*/{"pulso_eletrico", "scan_basico"}};
}

EnemyTemplate sentinela_fixture() {
    return EnemyTemplate{
        /*id=*/"sentinela_bit",
        /*max_hp=*/55,
        /*atk=*/6,
        /*def=*/8,
        /*spd=*/4,
        /*family=*/CardFamily::Cinetico,
        /*brain=*/BrainKind::Scripted,
        /*is_boss=*/false,
        /*base_deck=*/{}};
}

}  // namespace

// ---- CharacterTemplate: campos preservados --------------------------------

TEST_CASE("character_template: guarda os campos do construtor",
          "[domain][templates][character]") {
    const auto t = gus_fixture();
    REQUIRE(t.id == "gus");
    REQUIRE(t.max_hp == 34);
    REQUIRE(t.atk == 8);
    REQUIRE(t.def == 5);
    REQUIRE(t.spd == 9);
    REQUIRE(t.family == CardFamily::Eletrico);
    REQUIRE(t.is_universal_compiler == true);
    REQUIRE(t.base_deck == std::vector<std::string>{"pulso_eletrico", "scan_basico"});
}

TEST_CASE("character_template: igualdade por valor (record)",
          "[domain][templates][character]") {
    REQUIRE(gus_fixture() == gus_fixture());
    auto outro = gus_fixture();
    outro.atk = 99;
    REQUIRE(gus_fixture() != outro);
}

// ---- CharacterTemplate: Validate() fail-fast ------------------------------

TEST_CASE("character_template: id vazio lanca", "[domain][templates][character]") {
    auto t = gus_fixture();
    t.id = "";
    REQUIRE_THROWS_AS(t.validate(), std::invalid_argument);
}

TEST_CASE("character_template: id so com espacos lanca",
          "[domain][templates][character]") {
    auto t = gus_fixture();
    t.id = "   ";
    REQUIRE_THROWS_AS(t.validate(), std::invalid_argument);
}

TEST_CASE("character_template: max_hp nao positivo lanca",
          "[domain][templates][character]") {
    auto t = gus_fixture();
    t.max_hp = 0;
    REQUIRE_THROWS_AS(t.validate(), std::invalid_argument);
    t.max_hp = -1;
    REQUIRE_THROWS_AS(t.validate(), std::invalid_argument);
}

TEST_CASE("character_template: atk/def/spd negativos lancam",
          "[domain][templates][character]") {
    {
        auto t = gus_fixture();
        t.atk = -1;
        REQUIRE_THROWS_AS(t.validate(), std::invalid_argument);
    }
    {
        auto t = gus_fixture();
        t.def = -1;
        REQUIRE_THROWS_AS(t.validate(), std::invalid_argument);
    }
    {
        auto t = gus_fixture();
        t.spd = -1;
        REQUIRE_THROWS_AS(t.validate(), std::invalid_argument);
    }
}

TEST_CASE("character_template: base_deck com id vazio lanca",
          "[domain][templates][character]") {
    auto t = gus_fixture();
    t.base_deck = {"ok", ""};
    REQUIRE_THROWS_AS(t.validate(), std::invalid_argument);
}

TEST_CASE("character_template: base_deck vazio e valido",
          "[domain][templates][character]") {
    auto t = gus_fixture();
    t.base_deck = {};
    REQUIRE_NOTHROW(t.validate());
}

TEST_CASE("character_template: valido nao lanca",
          "[domain][templates][character]") {
    REQUIRE_NOTHROW(gus_fixture().validate());
}

// ---- EnemyTemplate: campos + Validate() -----------------------------------

TEST_CASE("enemy_template: guarda os campos do construtor",
          "[domain][templates][enemy]") {
    const auto e = sentinela_fixture();
    REQUIRE(e.id == "sentinela_bit");
    REQUIRE(e.max_hp == 55);
    REQUIRE(e.atk == 6);
    REQUIRE(e.def == 8);
    REQUIRE(e.spd == 4);
    REQUIRE(e.family == CardFamily::Cinetico);
    REQUIRE(e.brain == BrainKind::Scripted);
    REQUIRE(e.is_boss == false);
    REQUIRE(e.base_deck.empty());
}

TEST_CASE("enemy_template: igualdade por valor", "[domain][templates][enemy]") {
    REQUIRE(sentinela_fixture() == sentinela_fixture());
    auto outro = sentinela_fixture();
    outro.is_boss = true;
    REQUIRE(sentinela_fixture() != outro);
}

TEST_CASE("enemy_template: id vazio lanca", "[domain][templates][enemy]") {
    auto e = sentinela_fixture();
    e.id = "";
    REQUIRE_THROWS_AS(e.validate(), std::invalid_argument);
}

TEST_CASE("enemy_template: max_hp nao positivo lanca",
          "[domain][templates][enemy]") {
    auto e = sentinela_fixture();
    e.max_hp = 0;
    REQUIRE_THROWS_AS(e.validate(), std::invalid_argument);
}

TEST_CASE("enemy_template: stats negativos lancam",
          "[domain][templates][enemy]") {
    auto e = sentinela_fixture();
    e.def = -1;
    REQUIRE_THROWS_AS(e.validate(), std::invalid_argument);
}

TEST_CASE("enemy_template: valido nao lanca", "[domain][templates][enemy]") {
    REQUIRE_NOTHROW(sentinela_fixture().validate());
}

// ---- CardFamily / BrainKind: contrato de ordinais (binario) ---------------

TEST_CASE("card_family: ordinais espelham CombatEnums.cs",
          "[domain][templates][card_family]") {
    REQUIRE(static_cast<std::uint32_t>(CardFamily::Eletrico) == 0u);
    REQUIRE(static_cast<std::uint32_t>(CardFamily::Bioquimico) == 1u);
    REQUIRE(static_cast<std::uint32_t>(CardFamily::Sonico) == 2u);
    REQUIRE(static_cast<std::uint32_t>(CardFamily::Cinetico) == 3u);
    REQUIRE(static_cast<std::uint32_t>(CardFamily::Criptografico) == 4u);
}

// CARD-FAMILY-UNIVERSAL (2026-07-14, PS-R1): dois contadores desde a introducao de
// Universal — kWheelFamilyCount (roda, 0..4, usado na validacao de template) e
// kCardFamilyCount (dominio TOTAL do enum, 0..5, inclui Universal). NAO confundir.
TEST_CASE("card_family: kWheelFamilyCount (5) e kCardFamilyCount (6) refletem so-cartas",
          "[domain][templates][card_family][card_family_universal]") {
    REQUIRE(gus::domain::templates::kWheelFamilyCount == 5u);
    REQUIRE(gus::domain::templates::kCardFamilyCount == 6u);
}

TEST_CASE("brain_kind: ordinais espelham EnemyTemplate.cs",
          "[domain][templates][enemy]") {
    REQUIRE(static_cast<std::uint32_t>(BrainKind::Scripted) == 0u);
    REQUIRE(static_cast<std::uint32_t>(BrainKind::Utility) == 1u);
}

// ---- A1 (auditoria M3): validate() rejeita ordinal de family/brain fora do dominio ----
//
// O fechamento do A1 religou templates::CardFamily a fonte canonica do combate e endureceu
// o validate() pra rejeitar um ordinal de enum fora do conjunto valido (o C# Validate() NAO
// cobria isso; e um hardening alem da paridade, pedido pela auditoria). Um .gdt selado mas
// schema-divergente (family=9999) deixa de ser aceito silenciosamente.

TEST_CASE("character_template: family com ordinal fora do dominio lanca",
          "[domain][templates][a1]") {
    auto c = gus_fixture();
    c.family = static_cast<CardFamily>(9999u);  // fora de {0..4}
    REQUIRE_THROWS_AS(c.validate(), std::invalid_argument);
}

TEST_CASE("character_template: family no limite superior valido (Criptografico) nao lanca",
          "[domain][templates][a1]") {
    auto c = gus_fixture();
    c.family = CardFamily::Criptografico;  // ordinal 4, ultimo valido
    REQUIRE_NOTHROW(c.validate());
}

TEST_CASE("enemy_template: family com ordinal fora do dominio lanca",
          "[domain][templates][a1]") {
    auto e = sentinela_fixture();
    e.family = static_cast<CardFamily>(7u);  // fora de {0..4}
    REQUIRE_THROWS_AS(e.validate(), std::invalid_argument);
}

// ---- CARD-FAMILY-UNIVERSAL: Universal (ordinal 5) e SO-CARTAS ------------------------
//
// Decisao do criador 2026-07-14 (PS-R1): Universal vale SO PARA CARTAS. Templates de
// personagem/inimigo continuam na roda de 5 (kWheelFamilyCount); ordinal 5
// (CardFamily::Universal) e rejeitado tal qual qualquer outro ordinal fora do dominio.
// Comportamento INALTERADO em relacao ao A1: so muda o "porque" o 5 e invalido aqui.

TEST_CASE("character_template: family = Universal (ordinal 5) e rejeitado (so-cartas)",
          "[domain][templates][a1][card_family_universal]") {
    auto c = gus_fixture();
    c.family = CardFamily::Universal;  // ordinal 5, fora da roda de personagem/inimigo
    REQUIRE_THROWS_AS(c.validate(), std::invalid_argument);
}

TEST_CASE("enemy_template: family = Universal (ordinal 5) e rejeitado (so-cartas)",
          "[domain][templates][a1][card_family_universal]") {
    auto e = sentinela_fixture();
    e.family = CardFamily::Universal;  // ordinal 5, fora da roda de personagem/inimigo
    REQUIRE_THROWS_AS(e.validate(), std::invalid_argument);
}

TEST_CASE("enemy_template: brain com ordinal fora do dominio lanca",
          "[domain][templates][a1]") {
    auto e = sentinela_fixture();
    e.brain = static_cast<BrainKind>(2u);  // fora de {0..1}
    REQUIRE_THROWS_AS(e.validate(), std::invalid_argument);
}

TEST_CASE("enemy_template: brain Utility (ordinal 1, ultimo valido) nao lanca",
          "[domain][templates][a1]") {
    auto e = sentinela_fixture();
    e.brain = BrainKind::Utility;
    REQUIRE_NOTHROW(e.validate());
}
