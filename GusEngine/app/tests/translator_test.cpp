// GusEngine/app/tests/translator_test.cpp
//
// Catch2 (headless) do Translator de UI (M5, incremento 3.5). Prova, SEM I/O obrigatorio
// (injeta o catalogo): tr(key) devolve o valor da chave, faz fallback pro proprio key
// quando ausente, e o mapeamento verbo->chave i18n resolve os 6 verbos.

#include <catch2/catch_test_macros.hpp>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/battle_menu.hpp"

using gus::app::i18n::Translator;
using gus::app::i18n::verb_label_key;
using gus::app::screens::BattleVerb;

TEST_CASE("Translator: tr devolve o valor da chave carregada", "[translator]") {
    Translator t;
    t.load_from_content("## COMBAT_ACTION_ATTACK\nAtacar\n\n## COMBAT_VERB_COMPILAR\nCompilar\n");
    REQUIRE(t.tr("COMBAT_ACTION_ATTACK") == "Atacar");
    REQUIRE(t.tr("COMBAT_VERB_COMPILAR") == "Compilar");
}

TEST_CASE("Translator: chave ausente faz fallback pro proprio key", "[translator]") {
    Translator t;
    t.load_from_content("## X\nv\n");
    // Fallback: devolve a chave (visivel na UI, sinaliza traducao faltando sem crashar).
    REQUIRE(t.tr("CHAVE_INEXISTENTE") == "CHAVE_INEXISTENTE");
}

TEST_CASE("Translator: vazio antes de carregar devolve o key (fallback)",
          "[translator]") {
    Translator t;
    REQUIRE(t.tr("QUALQUER") == "QUALQUER");
}

TEST_CASE("verb_label_key mapeia os 6 verbos pra chaves i18n existentes",
          "[translator]") {
    // As chaves devem existir no catalogo pt_br (parity garante en tambem). Aqui so
    // checamos o MAPEAMENTO (string da chave), nao o valor.
    REQUIRE(verb_label_key(BattleVerb::Scan) == "COMBAT_ACTION_SCAN");
    REQUIRE(verb_label_key(BattleVerb::Gambito) == "COMBAT_VERB_GAMBITO");
    REQUIRE(verb_label_key(BattleVerb::Atacar) == "COMBAT_ACTION_ATTACK");
    REQUIRE(verb_label_key(BattleVerb::Defender) == "COMBAT_ACTION_DEFEND");
    REQUIRE(verb_label_key(BattleVerb::Compilar) == "COMBAT_VERB_COMPILAR");
    REQUIRE(verb_label_key(BattleVerb::Flee) == "COMBAT_FLEE");
}
