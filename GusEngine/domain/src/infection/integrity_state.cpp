// gus/domain/src/infection/integrity_state.cpp
//
// Implementacao de IntegrityState::validate(). Ver header para o contrato. ATOM-1:
// extraido de gus/domain/src/deck/card_hardware.cpp.

#include "gus/domain/infection/integrity_state.hpp"

#include <stdexcept>

namespace gus::domain::infection {

void IntegrityState::validate() const {
    // Defesa em profundidade (mesmo padrao de DifficultyLevel::validate() em
    // save_data.cpp): um payload selado mas schema-divergente (ordinal fora do
    // dominio canonico do enum) nao e aceito silenciosamente.
    if (static_cast<std::uint32_t>(virus_kind) >= kVirusKindCount)
        throw std::invalid_argument(
            "CardPhysicalState: virus_kind fora do dominio (ordinal invalido).");

    // Secao 6 inv.1: nao existe "tipo de virus" sem infeccao.
    if (virus_kind != VirusKind::None && !is_infected)
        throw std::invalid_argument(
            "CardPhysicalState: virus_kind != None exige is_infected == true "
            "(cartas-spec-dados.md secao 6 inv.1).");
    // Secao 6 inv.1: nao se diagnostica infeccao que nao existe.
    if (is_diagnosed && !is_infected)
        throw std::invalid_argument(
            "CardPhysicalState: is_diagnosed == true exige is_infected == true "
            "(cartas-spec-dados.md secao 6 inv.1).");
}

}  // namespace gus::domain::infection
