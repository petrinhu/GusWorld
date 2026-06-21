// gus/domain/domain_info.hpp
// Stub de identidade da camada de dominio (M0). Existe so para dar simbolo e
// target ao build antes do porte real (save/i18n/progression/templates/combat
// chegam em M3+). POCO puro: ZERO Qt.
#ifndef GUS_DOMAIN_DOMAIN_INFO_HPP
#define GUS_DOMAIN_DOMAIN_INFO_HPP

#include <string_view>

namespace gus::domain {

// Versao do schema de save. No C# atual e save_version=2 (CharacterStates).
// Aqui fica como ancora forward-only; o porte do save (M3) assume esta fonte.
inline constexpr int kSaveSchemaVersion = 2;

// Rotulo da camada de dominio, util para diagnostico/log na fase de andaime.
std::string_view domain_label() noexcept;

}  // namespace gus::domain

#endif  // GUS_DOMAIN_DOMAIN_INFO_HPP
