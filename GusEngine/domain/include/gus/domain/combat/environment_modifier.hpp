// gus/domain/combat/environment_modifier.hpp
//
// Record POCO de um ambiente de combate (secao 18.10), portado de
// engine/foundation/turn_combat/EnvironmentModifier.cs. POCO puro, ZERO Qt (invariante
// de domain/, engine-design.md secao 2). Header-only: dados imutaveis por convencao +
// helpers triviais.
//
// Cada ambiente age por ate quatro canais (secao 18.1):
//   1. mult_ambiente por familia (0.66..1.5)            -> family_mults
//   2. facilita UM status do enum existente (secao 9)   -> facilitated_status
//   3. efeitos em fila / SPD (deltas)                   -> jogo posterior (campos preparados)
//   4. interacao com hardware (Scan/Matriz/Tavus)       -> hardware
//
// MAPEAMENTO de tipos C# -> C++:
//   readonly record struct FacilitatedStatus -> struct de campos publicos + operator==.
//   readonly record struct HardwareHook      -> idem.
//   sealed record EnvironmentModifier         -> struct (dados); igualdade por valor.
//   IReadOnlyDictionary<CardFamily,float>     -> std::map<CardFamily,float> (ordenado,
//                                                igualdade por valor, deterministico).
//   FacilitatedStatus? (nullable)             -> std::optional<FacilitatedStatus>.
//   string DisplayNameKey                     -> std::string display_name_key (KEY i18n).
//
// Cross-ref: engine/foundation/turn_combat/EnvironmentModifier.cs;
//            docs/design/mecanicas/combat.md secao 18 (canais)/11 (stacking)/9 (status);
//            ADR-004 (contrato mult_ambiente); ADR-006.

#ifndef GUS_DOMAIN_COMBAT_ENVIRONMENT_MODIFIER_HPP
#define GUS_DOMAIN_COMBAT_ENVIRONMENT_MODIFIER_HPP

#include <map>
#include <optional>
#include <string>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/environment_enums.hpp"

namespace gus::domain::combat {

// Status que um ambiente facilita (secao 18, canal 2). NAO cria status novo: referencia
// o enum existente (secao 9) e descreve o INCREMENTO que o ambiente da (ex: "Disrupt +1
// mag" => id=Disrupt, magnitude_delta=1). "Root" = Slow de magnitude extrema (secao 18.1,
// NAO e status novo).
struct FacilitatedStatus {
    StatusId id = StatusId::Stun;
    int magnitude_delta = 0;
    int duration_delta = 0;

    [[nodiscard]] bool operator==(const FacilitatedStatus&) const = default;

    // Aplica o incremento ambiental a um StatusEffect do MESMO id (secao 18, canal 2):
    // soma os deltas de magnitude/duracao. No-op se o id nao bate (o ambiente so facilita
    // o SEU status). Reusa o enum existente (secao 9) - nao cria status novo.
    [[nodiscard]] StatusEffect apply_to(StatusEffect status) const {
        if (status.id != id)
            return status;
        status.magnitude += magnitude_delta;
        status.duration += duration_delta;
        return status;
    }
};

// Hooks do triangulo de hardware (Pillar 3: Oculos/Scan, Matriz Ortodontica, Tavus-Drive)
// que um ambiente altera (secao 18, canal 4). Deltas neutros (0) = sem efeito. O cap
// anti-Scan (secao 18.1) e aplicado pela curadoria que monta o conjunto ativo - o record
// so DECLARA o delta da camada.
struct HardwareHook {
    // Delta de custo de AP do Scan (+1 encarece; negativo barateia; 0 = neutro).
    int scan_ap_delta = 0;
    // Scan gratis nesta camada (Metal-Condutor, Dia, Bioluminescencia). Sobrepoe
    // scan_ap_delta > 0.
    bool scan_free = false;
    // Delta de antecipacao do Gambito-Prever (+1 le mais a frente; -1 encurta).
    int prever_turn_delta = 0;
    // Delta de SPD da party enquanto a camada esta ativa (Vento/Aurora = +1).
    int party_spd_delta = 0;

    [[nodiscard]] bool operator==(const HardwareHook&) const = default;
};

// Ambiente de combate (uma entrada de qualquer camada). Imutavel por convencao; o
// catalogo (EnvironmentCatalog) e a unica fonte de instancias canonicas (secao 18.10).
struct EnvironmentModifier {
    // Identidade estavel (secao 18). Chave do catalogo.
    EnvironmentId id = EnvironmentId::None;

    // Camada: Terreno (fixo) / Clima (transitorio) / Periodo (roda automatica). secao 18.1.
    EnvironmentLayer layer = EnvironmentLayer::Terreno;

    // Tier de leitura: Visivel ou Codex (efeito so apos Scan-ambiente). secao 18.4/18.5.
    EnvironmentTier tier = EnvironmentTier::Visivel;

    // KEY i18n do nome de exibicao (resolvido via tr() na UI). Nunca string user-facing
    // hardcoded (i18n canonico GusWorld).
    std::string display_name_key;

    // Multiplicador por familia (secao 18, canal 1). So as familias com up/down documentado
    // entram; as ausentes sao neutras (1.0). Faixa por camada 0.66..1.5. NUNCA toca
    // mult_fraqueza (secao 11).
    std::map<CardFamily, float> family_mults;

    // Status que esta camada facilita (secao 18, canal 2). nullopt = nenhum.
    std::optional<FacilitatedStatus> facilitated_status;

    // Hooks de hardware desta camada (secao 18, canal 4). Default = neutro.
    HardwareHook hardware;

    // So for Periodo: duracao da fase em turnos de fila (secao 18.3: Dia 5 / Crep 2 /
    // Noite 5 / Aurora 2). 0 nas camadas nao-periodo.
    int period_duration = 0;

    [[nodiscard]] bool operator==(const EnvironmentModifier&) const = default;

    // Multiplicador desta camada para a familia dada (secao 18, canal 1). Familias
    // nao-listadas retornam 1.0 (neutro). Nunca e mult_fraqueza (secao 11): e fator
    // proprio da cadeia divisiva.
    [[nodiscard]] float mult_for(CardFamily family) const {
        const auto it = family_mults.find(family);
        return it != family_mults.end() ? it->second : 1.0f;
    }
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_ENVIRONMENT_MODIFIER_HPP
