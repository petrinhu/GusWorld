// GusEngine/app/tests/battle_menu_test.cpp
//
// Catch2 (headless) do MODELO PURO do menu de verbos (M5, incremento 3). Prova, SEM
// janela nem SDL: os 6 verbos + custos de AP (combat.md par.5), navegacao com wrap,
// habilitacao por AP (refresh), layout em coluna, e o mapeamento verbo->CombatActionType.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "gus/app/screens/battle_menu.hpp"
#include "gus/domain/combat/combat_enums.hpp"

using Catch::Matchers::WithinAbs;
using gus::app::screens::BattleMenu;
using gus::app::screens::BattleVerb;
using gus::app::screens::kBattleVerbCount;
using gus::app::screens::verb_action_type;
using gus::app::screens::verb_ap_cost;
using gus::app::screens::verb_key;
using gus::core::spatial::Rect;
using gus::domain::combat::CombatActionType;

TEST_CASE("verbos: custo de AP segue combat.md par.5", "[battle_menu]") {
    REQUIRE(verb_ap_cost(BattleVerb::Scan) == 1);
    REQUIRE(verb_ap_cost(BattleVerb::Gambito) == 1);
    REQUIRE(verb_ap_cost(BattleVerb::Atacar) == 1);
    REQUIRE(verb_ap_cost(BattleVerb::Defender) == 1);
    REQUIRE(verb_ap_cost(BattleVerb::Flee) == 1);
    REQUIRE(verb_ap_cost(BattleVerb::Compilar) == 0);  // so abre o overlay (incr 4)
}

TEST_CASE("verbos: chave e tipo de acao corretos", "[battle_menu]") {
    REQUIRE(verb_key(BattleVerb::Atacar) == std::string_view("atacar"));
    REQUIRE(verb_action_type(BattleVerb::Atacar) == CombatActionType::Attack);
    REQUIRE(verb_action_type(BattleVerb::Defender) == CombatActionType::Defend);
    REQUIRE(verb_action_type(BattleVerb::Scan) == CombatActionType::Scan);
    REQUIRE(verb_action_type(BattleVerb::Gambito) == CombatActionType::GambitPredict);
    REQUIRE(verb_action_type(BattleVerb::Flee) == CombatActionType::Flee);
    // Compilar nao tem acao de motor (sentinela Pass): tratado pela UI no incr 3/4.
    REQUIRE(verb_action_type(BattleVerb::Compilar) == CombatActionType::Pass);
}

TEST_CASE("menu: navegacao com WRAP nos dois sentidos", "[battle_menu]") {
    BattleMenu m;
    // default = Atacar (indice 2).
    REQUIRE(m.selected_verb() == BattleVerb::Atacar);
    m.move(+1);
    REQUIRE(m.selected_verb() == BattleVerb::Defender);
    m.move(-1);
    REQUIRE(m.selected_verb() == BattleVerb::Atacar);
    // Wrap pra tras a partir do 1o.
    while (m.selected_index() != 0) {
        m.move(-1);
    }
    m.move(-1);
    REQUIRE(m.selected_index() == kBattleVerbCount - 1);  // virou pro ultimo
    m.move(+1);
    REQUIRE(m.selected_index() == 0);  // e volta
}

TEST_CASE("menu: habilitacao por AP (refresh)", "[battle_menu]") {
    BattleMenu m;
    // Com 3 AP, tudo habilitado.
    m.refresh(3);
    REQUIRE(m.is_enabled(BattleVerb::Atacar));
    REQUIRE(m.is_enabled(BattleVerb::Gambito));
    REQUIRE(m.is_enabled(BattleVerb::Compilar));

    // Com 0 AP, so Compilar (custo 0) fica habilitado; os de 1 AP desabilitam.
    m.refresh(0);
    REQUIRE_FALSE(m.is_enabled(BattleVerb::Atacar));
    REQUIRE_FALSE(m.is_enabled(BattleVerb::Scan));
    REQUIRE_FALSE(m.is_enabled(BattleVerb::Flee));
    REQUIRE(m.is_enabled(BattleVerb::Compilar));

    // refresh nao MOVE a selecao sozinho (o jogador ve o desabilitado e escolhe outro).
    const int before = m.selected_index();
    m.refresh(1);
    REQUIRE(m.selected_index() == before);
}

TEST_CASE("menu: selected_enabled reflete o AP do item selecionado", "[battle_menu]") {
    BattleMenu m;
    // Seleciona Atacar (1 AP) e zera o AP: selecionado fica DESABILITADO.
    while (m.selected_verb() != BattleVerb::Atacar) {
        m.move(+1);
    }
    m.refresh(0);
    REQUIRE_FALSE(m.selected_enabled());
    // Seleciona Compilar (0 AP): habilitado mesmo com 0 AP.
    while (m.selected_verb() != BattleVerb::Compilar) {
        m.move(+1);
    }
    REQUIRE(m.selected_enabled());
}

TEST_CASE("menu: layout em coluna unica, 6 itens iguais dentro da zona",
          "[battle_menu]") {
    BattleMenu m;
    m.refresh(3);
    const Rect zone{320.0f, 252.0f, 314.0f, 60.0f};  // metade direita do painel
    const auto items = m.layout(zone);
    REQUIRE(items.size() == static_cast<std::size_t>(kBattleVerbCount));
    // Empilhados: Y crescente, mesma largura, mesma altura, dentro da zona.
    for (int i = 0; i < kBattleVerbCount; ++i) {
        REQUIRE(items[i].verb == static_cast<BattleVerb>(i));
        REQUIRE(items[i].rect.x >= zone.x);
        REQUIRE(items[i].rect.x + items[i].rect.w <= zone.x + zone.w + 1e-3f);
        REQUIRE(items[i].rect.y >= zone.y - 1e-3f);
        REQUIRE(items[i].rect.y + items[i].rect.h <= zone.y + zone.h + 1e-3f);
        if (i > 0) {
            REQUIRE(items[i].rect.y > items[i - 1].rect.y);
            REQUIRE_THAT(items[i].rect.h, WithinAbs(items[0].rect.h, 1e-4f));
            REQUIRE_THAT(items[i].rect.w, WithinAbs(items[0].rect.w, 1e-4f));
        }
    }
    // Habilitacao propagada pro item.
    REQUIRE(items[static_cast<int>(BattleVerb::Compilar)].enabled);
}
