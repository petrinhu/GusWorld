// gus/domain/combat/environment_transitions.cpp
//
// Implementacao da tabela fechada de mutabilidade de ambiente (secao 18.6), portada de
// engine/foundation/turn_combat/EnvironmentTransitions.cs. Deterministica, sem RNG. As
// 16 linhas sao transcritas DIRETO da tabela do doc, na mesma ordem. POCO puro, ZERO Qt.
//
// Cross-ref: engine/foundation/turn_combat/EnvironmentTransitions.cs;
//            docs/design/mecanicas/combat.md secao 18.6/18.4; ADR-006.

#include "gus/domain/combat/environment_transitions.hpp"

namespace gus::domain::combat::EnvironmentTransitions {
namespace {

// "ar limpo" = remocao da camada (espelha o const Limpo = EnvironmentId.None do C#).
constexpr EnvironmentId kLimpo = EnvironmentId::None;

// Tabela canonica (secao 18.6), na ordem da tabela do doc. Construida uma vez.
const std::vector<EnvironmentTransition>& table() {
    static const std::vector<EnvironmentTransition> kTable = {
        {EnvironmentTrigger::EletricoOuCalor, EnvironmentId::Lamacento, EnvironmentId::Seco,
         "Eletrico forte OU Calor seca o Lamacento -> Seco."},
        {EnvironmentTrigger::EletricoOuCalor, EnvironmentId::Vinhas, EnvironmentId::Seco,
         "Eletrico forte OU Calor queima as Vinhas -> Seco."},
        {EnvironmentTrigger::EletricoOuCalor, EnvironmentId::Gelo, EnvironmentId::AguaAlagado,
         "Eletrico forte OU Calor derrete o Gelo -> Agua / Alagado."},
        {EnvironmentTrigger::SonicoForte, EnvironmentId::Neblina, kLimpo,
         "Sonico forte dissipa a Neblina -> ar limpo."},
        {EnvironmentTrigger::SonicoForte, EnvironmentId::Fumaca, kLimpo,
         "Sonico forte dissipa a Fumaca -> ar limpo."},
        {EnvironmentTrigger::SonicoForte, EnvironmentId::Estatica, kLimpo,
         "Sonico forte dissipa a Estatica -> ar limpo."},
        {EnvironmentTrigger::Vento, EnvironmentId::Neblina, kLimpo,
         "Vento dissipa a Neblina -> ar limpo."},
        {EnvironmentTrigger::Vento, EnvironmentId::Fumaca, kLimpo,
         "Vento dissipa a Fumaca/Cinzas -> ar limpo."},
        {EnvironmentTrigger::Chuva, EnvironmentId::Calor, EnvironmentId::Neblina,
         "Chuva + Calor gera Vapor (Neblina + calor leve)."},
        {EnvironmentTrigger::Chuva, EnvironmentId::Vinhas, EnvironmentId::Vinhas,
         "Chuva + Vinhas: vinhas crescem mais rapido (avanca estagio; mesma camada)."},
        {EnvironmentTrigger::Chuva, EnvironmentId::TempestadeEletrica, EnvironmentId::TempestadeEletrica,
         "Chuva + Eletrico ambiente escala para Tempestade Eletrica."},
        {EnvironmentTrigger::EletricoOuCalor, EnvironmentId::SoloFertilRecursivo, EnvironmentId::SoloFertilRecursivo,
         "Calor em T5 Solo Fertil: Object pula para o estagio 3 (mesma camada, estagio avanca)."},
        {EnvironmentTrigger::Gelo, EnvironmentId::SoloFertilRecursivo, EnvironmentId::SoloFertilRecursivo,
         "Gelo em T5 Solo Fertil congela o crescimento (trava estagio; mesma camada)."},
        {EnvironmentTrigger::Agua, EnvironmentId::MetalCondutor, EnvironmentId::MetalCondutor,
         "Agua sobre Metal-Condutor: conducao total (Eletrico x1.5 por 2 turnos; buff temporario)."},
        {EnvironmentTrigger::AcaceiroSaudavel, EnvironmentId::AnomaliaPerlin, EnvironmentId::AnomaliaPerlin,
         "Acaceiro saudavel proximo: Anomalia Contida - T6 Scan volta a funcionar (purificacao)."},
        {EnvironmentTrigger::AshlarVencidoEntreEncontros, EnvironmentId::AshlarBruto, EnvironmentId::PavimentoTesselado,
         "T4 Ashlar Bruto vencido entre encontros: lapida -> T1 Pavimento Tesselado (progressao)."},
    };
    return kTable;
}

}  // namespace

const std::vector<EnvironmentTransition>& all() { return table(); }

std::optional<EnvironmentId> resolve(EnvironmentTrigger trigger, EnvironmentId target) {
    for (const auto& t : table())
        if (t.trigger == trigger && t.target == target)
            return t.result;
    return std::nullopt;
}

}  // namespace gus::domain::combat::EnvironmentTransitions
