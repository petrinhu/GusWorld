// canonical_templates_test.cpp
//
// Spec executavel (Catch2 v3) dos templates CANONICOS do encontro de referencia do
// vertical slice (combat.md secao 17), portada de engine/tests/data/CanonicalTemplatesTests.cs
// e da fonte CanonicalTemplates.cs. Trava os valores canonicos (HP/Atk/Def/SPD/
// Family/Brain/IsUniversalCompiler): RED se alguem desviar do canon.
//
// Subsistema: domain/templates (marco M3). POCO puro, ZERO Qt.
//
// Cross-ref: engine/foundation/data/CanonicalTemplates.cs; combat.md secao 17/6.1/13.

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/templates/canonical_templates.hpp"
#include "gus/domain/templates/card_family.hpp"
#include "gus/domain/templates/enemy_template.hpp"
#include "gus/domain/templates/template_serializer.hpp"

namespace canon = gus::domain::templates::canonical;
using gus::domain::templates::BrainKind;
using gus::domain::templates::CardFamily;

// ---- Party (combat.md secao 17) -------------------------------------------

TEST_CASE("canon: Gus HP34/Atk8/Def5/SPD9 universal eletrico",
          "[domain][templates][canonical]") {
    const auto g = canon::gus();
    REQUIRE(g.id == "gus");
    REQUIRE(g.max_hp == 34);
    REQUIRE(g.atk == 8);
    REQUIRE(g.def == 5);
    REQUIRE(g.spd == 9);
    REQUIRE(g.family == CardFamily::Eletrico);
    REQUIRE(g.is_universal_compiler == true);  // a flag, nao um valor de enum
    REQUIRE_NOTHROW(g.validate());
}

TEST_CASE("canon: Caua HP55/Atk14/Def8/SPD13 eletrico nao-universal",
          "[domain][templates][canonical]") {
    const auto c = canon::caua();
    REQUIRE(c.id == "caua");
    REQUIRE(c.max_hp == 55);
    REQUIRE(c.atk == 14);
    REQUIRE(c.def == 8);
    REQUIRE(c.spd == 13);
    REQUIRE(c.family == CardFamily::Eletrico);
    REQUIRE(c.is_universal_compiler == false);
    REQUIRE_NOTHROW(c.validate());
}

TEST_CASE("canon: Jaci HP55/Atk9/Def10/SPD7 bioquimico healer",
          "[domain][templates][canonical]") {
    const auto j = canon::jaci();
    REQUIRE(j.id == "jaci");
    REQUIRE(j.max_hp == 55);
    REQUIRE(j.atk == 9);
    REQUIRE(j.def == 10);
    REQUIRE(j.spd == 7);
    REQUIRE(j.family == CardFamily::Bioquimico);
    REQUIRE(j.is_universal_compiler == false);
    REQUIRE_NOTHROW(j.validate());
}

// ---- Inimigos do encontro de referencia (combat.md secao 17) --------------

TEST_CASE("canon: Sentinela-Bit Trash HP55/Def8 cinetico scripted",
          "[domain][templates][canonical]") {
    const auto s = canon::sentinela_bit();
    REQUIRE(s.id == "sentinela_bit");
    REQUIRE(s.max_hp == 55);
    REQUIRE(s.def == 8);
    REQUIRE(s.family == CardFamily::Cinetico);
    REQUIRE(s.brain == BrainKind::Scripted);
    REQUIRE(s.is_boss == false);
    REQUIRE_NOTHROW(s.validate());
}

TEST_CASE("canon: Daemon-Guard Elite HP144/Def14 cinetico scripted",
          "[domain][templates][canonical]") {
    const auto d = canon::daemon_guard();
    REQUIRE(d.id == "daemon_guard");
    REQUIRE(d.max_hp == 144);  // Fibonacci canon
    REQUIRE(d.def == 14);
    REQUIRE(d.family == CardFamily::Cinetico);
    REQUIRE(d.brain == BrainKind::Scripted);
    REQUIRE(d.is_boss == false);
    REQUIRE_NOTHROW(d.validate());
}

// ---- Agregadores -----------------------------------------------------------

TEST_CASE("canon: all_characters lista os 3 da party",
          "[domain][templates][canonical]") {
    const auto all = canon::all_characters();
    REQUIRE(all.size() == 3u);
    REQUIRE(all[0].id == "gus");
    REQUIRE(all[1].id == "caua");
    REQUIRE(all[2].id == "jaci");
}

TEST_CASE("canon: all_enemies lista os 2 inimigos",
          "[domain][templates][canonical]") {
    const auto all = canon::all_enemies();
    REQUIRE(all.size() == 2u);
    REQUIRE(all[0].id == "sentinela_bit");
    REQUIRE(all[1].id == "daemon_guard");
}

// ---- Os 5 atores roundtrippam pelo serializer (integracao) ----------------

TEST_CASE("canon: os 5 atores roundtrippam pelo serializer",
          "[domain][templates][canonical]") {
    namespace t = gus::domain::templates;
    for (const auto& ch : canon::all_characters()) {
        REQUIRE(t::deserialize_character(t::serialize_character(ch)) == ch);
    }
    for (const auto& en : canon::all_enemies()) {
        REQUIRE(t::deserialize_enemy(t::serialize_enemy(en)) == en);
    }
}
