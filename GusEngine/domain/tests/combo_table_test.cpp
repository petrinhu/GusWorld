// combo_table_test.cpp
//
// Spec executavel (Catch2 v3) do ComboTable (resolvedor de pipeline de combo, secao 10),
// portado de engine/foundation/turn_combat/ComboTable.cs. POCO puro, ZERO Qt, headless.
//
// O ComboTable.cs nao tem teste xUnit dedicado (e exercitado pela FSM em CombatInc2Tests);
// aqui validamos o contrato puro do resolvedor: casamento por assinatura EXATA (Kind+Ref,
// mesma ordem e tamanho), as 2 receitas mockup do slice, e os no-match.
//
// Cross-ref: engine/foundation/turn_combat/ComboTable.cs; docs/design/mecanicas/combat.md secao 10.

#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combo_table.hpp"

using namespace gus::domain::combat;

TEST_CASE("combo_table: expoe as 2 receitas mockup do slice",
          "[domain][combat][combo]") {
    const auto& recipes = ComboTable::recipes();
    REQUIRE(recipes.size() == 2);
    REQUIRE(recipes[0].combo_id == "pulso_stream");
    REQUIRE(recipes[0].display_name == "COMBO_PULSO_STREAM_NAME");
    REQUIRE(recipes[0].mult_combo == 2.0f);
    REQUIRE(recipes[0].discoverable == true);
    REQUIRE_FALSE(recipes[0].result_status.has_value());

    REQUIRE(recipes[1].combo_id == "raiz_null");
    REQUIRE(recipes[1].display_name == "COMBO_RAIZ_NULL_NAME");
    REQUIRE(recipes[1].mult_combo == 1.5f);
    REQUIRE(recipes[1].discoverable == false);
    REQUIRE(recipes[1].result_status.has_value());
    REQUIRE(recipes[1].result_status->id == StatusId::Regen);
    REQUIRE(recipes[1].result_status->magnitude == 5);
    REQUIRE(recipes[1].result_status->duration == 2);
    REQUIRE(recipes[1].result_status->stack_rule == StackRule::Refresh);
    REQUIRE(recipes[1].result_status->family_origin == CardFamily::Bioquimico);
}

TEST_CASE("combo_table: casa assinatura exata pulso.eletrico + Stream",
          "[domain][combat][combo]") {
    std::vector<PipelineSlot> pipeline = {
        {PipelineSlotKind::Card, "pulso.eletrico"},
        {PipelineSlotKind::Modifier, "Stream"},
    };
    const auto combo = ComboTable::match(pipeline);
    REQUIRE(combo.has_value());
    REQUIRE(combo->combo_id == "pulso_stream");
    REQUIRE(combo->mult_combo == 2.0f);
}

TEST_CASE("combo_table: casa raiz.bioquimico + Null",
          "[domain][combat][combo]") {
    std::vector<PipelineSlot> pipeline = {
        {PipelineSlotKind::Card, "raiz.bioquimico"},
        {PipelineSlotKind::Modifier, "Null"},
    };
    const auto combo = ComboTable::match(pipeline);
    REQUIRE(combo.has_value());
    REQUIRE(combo->combo_id == "raiz_null");
}

TEST_CASE("combo_table: pipeline vazia nao casa", "[domain][combat][combo]") {
    REQUIRE_FALSE(ComboTable::match({}).has_value());
}

TEST_CASE("combo_table: assinatura de tamanho diferente nao casa",
          "[domain][combat][combo]") {
    std::vector<PipelineSlot> so_carta = {{PipelineSlotKind::Card, "pulso.eletrico"}};
    REQUIRE_FALSE(ComboTable::match(so_carta).has_value());
}

TEST_CASE("combo_table: kind ou ref diferentes nao casam",
          "[domain][combat][combo]") {
    // ref errada
    std::vector<PipelineSlot> ref_errada = {
        {PipelineSlotKind::Card, "pulso.outro"},
        {PipelineSlotKind::Modifier, "Stream"},
    };
    REQUIRE_FALSE(ComboTable::match(ref_errada).has_value());

    // kind trocado no 2o slot
    std::vector<PipelineSlot> kind_errado = {
        {PipelineSlotKind::Card, "pulso.eletrico"},
        {PipelineSlotKind::Card, "Stream"},
    };
    REQUIRE_FALSE(ComboTable::match(kind_errado).has_value());

    // ordem invertida
    std::vector<PipelineSlot> ordem_invertida = {
        {PipelineSlotKind::Modifier, "Stream"},
        {PipelineSlotKind::Card, "pulso.eletrico"},
    };
    REQUIRE_FALSE(ComboTable::match(ordem_invertida).has_value());
}
