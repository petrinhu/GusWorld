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
// save, escolhida na criacao - docs/design/mecanicas/modos-morte.md §3.2); V6
// +CardCollectionState (deck ativo/morto com instance_id) +hand_selection por
// personagem, em SUBSTITUICAO ao campo legado CharacterSaveState::deck (DECK-4,
// docs/design/mecanicas/deck-mao-sistema.md) - migrator V5->V6 converte o deck
// legado (vector<string> de card_id) nas instancias novas do deck ativo; +credits
// (int64_t) UMA VEZ no nivel do SaveData (carteira UNICA da party, docs/design/
// mecanicas/economia.md - economia single-currency, NAO per-character); V7
// +CardPhysicalState (gus/domain/deck/card_hardware.hpp) dentro de CADA
// CardInstance (ativo e morto) de CardCollectionState - CARDS-HARDWARE-ENGINE
// incremento 1 (CARDS-HW-1, TODO.md), a camada FISICA de carta possuida
// (origem ROM/EPROM/pirata, bateria CR2032, integridade/virus oculto) -
// migrator V6->V7 popula physical = CardPhysicalState{} (default seguro) em toda
// instancia ja existente.
// Ancora forward-only e FONTE UNICA da versao (o comentario "V2/V3/V4/V5/V6" em
// outros arquivos NAO e autoridade; esta constante e).
inline constexpr int kSaveSchemaVersion = 7;

// Rotulo da camada de dominio, util para diagnostico/log na fase de andaime.
std::string_view domain_label() noexcept;

}  // namespace gus::domain

#endif  // GUS_DOMAIN_DOMAIN_INFO_HPP
