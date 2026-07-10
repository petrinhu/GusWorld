// gus/domain/domain_info.hpp
// Stub de identidade da camada de dominio (M0). Existe so para dar simbolo e
// target ao build antes do porte real (save/i18n/progression/templates/combat
// chegam em M3+). POCO puro: ZERO Qt.
#ifndef GUS_DOMAIN_DOMAIN_INFO_HPP
#define GUS_DOMAIN_DOMAIN_INFO_HPP

#include <string_view>

namespace gus::domain {

// Versao do schema de save. Historico: V1 inicial; V2 +CharacterStates; V3
// +EnemyKnowledge; V4 +input_remap_backup +controls_hash128 +slot_id (ADR-007:
// persistencia de controles + deteccao de adulteracao + slot-id selado); V5
// +difficulty +difficult_recovery_stage (MODOS-MORTE Fase 0: dificuldade FIXA do
// save, escolhida na criacao - docs/design/mecanicas/modos-morte.md §3.2). Ancora
// forward-only e FONTE UNICA da versao (o comentario "V2/V3/V4" em outros arquivos
// NAO e autoridade; esta constante e).
inline constexpr int kSaveSchemaVersion = 5;

// Rotulo da camada de dominio, util para diagnostico/log na fase de andaime.
std::string_view domain_label() noexcept;

}  // namespace gus::domain

#endif  // GUS_DOMAIN_DOMAIN_INFO_HPP
