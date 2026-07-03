// GusEngine/app/tests/battle_cockpit_verb_ids_test.cpp
//
// GLINTFX-CLICK: Catch2 (headless) do mapeamento id<->indice de verbo (ver header). POCO
// puro, SEM SDL/glintfx. Cobertura PERMANENTE (nao depende da geometria manual aposentada
// battle_cockpit_pills.hpp - essa equivalencia, ja provada uma vez, vive so no historico
// git do commit de migracao).

#include <catch2/catch_test_macros.hpp>

#include "gus/app/screens/battle_cockpit_verb_ids.hpp"
#include "gus/app/screens/battle_menu.hpp"

using gus::app::screens::BattleVerb;
using gus::app::screens::cockpit_verb_index_for_click_id;
using gus::app::screens::kBattleVerbCount;
using gus::app::screens::kCockpitVerbElementIds;
using gus::app::screens::verb_key;

TEST_CASE("verb_ids: ha exatamente 1 id por verbo, na ORDEM do enum BattleVerb",
          "[cockpit_verb_ids]") {
    REQUIRE(kBattleVerbCount == 6);
    // static_cast<BattleVerb>(i) == a ordem de exibicao dos pills (Scan..Flee).
    REQUIRE(std::string_view(kCockpitVerbElementIds[static_cast<int>(BattleVerb::Scan)]) ==
            "verb-scan");
    REQUIRE(std::string_view(kCockpitVerbElementIds[static_cast<int>(BattleVerb::Gambito)]) ==
            "verb-gambito");
    REQUIRE(std::string_view(kCockpitVerbElementIds[static_cast<int>(BattleVerb::Atacar)]) ==
            "verb-atacar");
    REQUIRE(std::string_view(kCockpitVerbElementIds[static_cast<int>(BattleVerb::Defender)]) ==
            "verb-defender");
    REQUIRE(std::string_view(kCockpitVerbElementIds[static_cast<int>(BattleVerb::Compilar)]) ==
            "verb-compilar");
    REQUIRE(std::string_view(kCockpitVerbElementIds[static_cast<int>(BattleVerb::Flee)]) ==
            "verb-flee");
}

TEST_CASE("verb_ids: cada id e 'verb-' + verb_key(verb) (sem drift da fonte unica de nomes)",
          "[cockpit_verb_ids]") {
    for (int i = 0; i < kBattleVerbCount; ++i) {
        const std::string esperado =
            std::string("verb-") + std::string(verb_key(static_cast<BattleVerb>(i)));
        REQUIRE(std::string(kCockpitVerbElementIds[i]) == esperado);
    }
}

TEST_CASE("verb_ids: cockpit_verb_index_for_click_id mapeia cada id de volta pro seu indice",
          "[cockpit_verb_ids]") {
    for (int i = 0; i < kBattleVerbCount; ++i) {
        REQUIRE(cockpit_verb_index_for_click_id(kCockpitVerbElementIds[i]) == i);
    }
}

TEST_CASE("verb_ids: id desconhecido/vazio/nulo devolve -1 (nao 'erra' pro pill errado)",
          "[cockpit_verb_ids]") {
    // Elementos VIZINHOS do cockpit que TAMBEM tem id proprio no RML (#cockpit/#combat/
    // #actor/#portrait/#vitals/#log/#opening/...) - clicar neles NAO deve acionar verbo.
    REQUIRE(cockpit_verb_index_for_click_id("combat") == -1);
    REQUIRE(cockpit_verb_index_for_click_id("cockpit") == -1);
    REQUIRE(cockpit_verb_index_for_click_id("vitals") == -1);
    REQUIRE(cockpit_verb_index_for_click_id("log") == -1);
    // "" e o que o glintfx devolve quando nenhum ancestral tem id.
    REQUIRE(cockpit_verb_index_for_click_id("") == -1);
    REQUIRE(cockpit_verb_index_for_click_id(nullptr) == -1);
    // Prefixo parecido mas nao-exato (guarda contra match parcial/substring).
    REQUIRE(cockpit_verb_index_for_click_id("verb-atacarx") == -1);
    REQUIRE(cockpit_verb_index_for_click_id("verb-ataca") == -1);
    REQUIRE(cockpit_verb_index_for_click_id("VERB-ATACAR") == -1);  // case-sensitive
}
