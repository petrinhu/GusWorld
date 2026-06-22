// gus/domain/src/combat/combo_table.cpp
//
// Implementacao do resolvedor de combo (secao 10). Ver header para o contrato. Espelha
// ComboTable.cs 1:1: receitas curadas (2 mockups) + casamento por assinatura exata.
// POCO puro, ZERO Qt.

#include "gus/domain/combat/combo_table.hpp"

namespace gus::domain::combat::ComboTable {

namespace {

// Constroi as receitas uma vez (espelha o static readonly Recipes do C#). DisplayName
// carrega a KEY de traducao (UPPER_SNAKE_CASE), nao o texto; resolucao key->pt-br no
// DISPLAY/UI (F2-G.5), nunca no runtime de combate.
const std::vector<ComboRecipe>& build_recipes() {
    static const std::vector<ComboRecipe> kRecipes = [] {
        std::vector<ComboRecipe> r;

        ComboRecipe pulso_stream;
        pulso_stream.combo_id = "pulso_stream";
        pulso_stream.display_name = "COMBO_PULSO_STREAM_NAME";
        pulso_stream.signature = {
            {PipelineSlotKind::Card, "pulso.eletrico"},
            {PipelineSlotKind::Modifier, "Stream"},
        };
        pulso_stream.result_status = std::nullopt;
        pulso_stream.mult_combo = 2.0f;
        pulso_stream.discoverable = true;
        r.push_back(pulso_stream);

        ComboRecipe raiz_null;
        raiz_null.combo_id = "raiz_null";
        raiz_null.display_name = "COMBO_RAIZ_NULL_NAME";
        raiz_null.signature = {
            {PipelineSlotKind::Card, "raiz.bioquimico"},
            {PipelineSlotKind::Modifier, "Null"},
        };
        raiz_null.result_status = StatusEffect{
            StatusId::Regen, /*magnitude=*/5, /*duration=*/2, StackRule::Refresh,
            CardFamily::Bioquimico};
        raiz_null.mult_combo = 1.5f;
        raiz_null.discoverable = false;
        r.push_back(raiz_null);

        return r;
    }();
    return kRecipes;
}

// Assinatura casa a pipeline: mesmo tamanho + (Kind, Ref) iguais slot a slot, em ordem.
bool signature_matches(const std::vector<PipelineSlot>& signature,
                       const std::vector<PipelineSlot>& pipeline) {
    if (signature.size() != pipeline.size())
        return false;
    for (std::size_t i = 0; i < signature.size(); ++i) {
        if (signature[i].kind != pipeline[i].kind || signature[i].ref != pipeline[i].ref)
            return false;
    }
    return true;
}

}  // namespace

const std::vector<ComboRecipe>& recipes() { return build_recipes(); }

std::optional<ComboRecipe> match(const std::vector<PipelineSlot>& pipeline) {
    if (pipeline.empty())
        return std::nullopt;

    for (const auto& recipe : build_recipes()) {
        if (signature_matches(recipe.signature, pipeline))
            return recipe;
    }
    return std::nullopt;
}

}  // namespace gus::domain::combat::ComboTable
