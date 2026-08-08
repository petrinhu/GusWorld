// SPDX-License-Identifier: Apache-2.0
// gus/domain/templates/enemy_difficulty_constants.hpp
//
// Multiplicador de DIFICULDADE sobre HP/Atk de inimigo (DIFICULDADE-TABELA-DADO,
// TODO.md), cruzando EnemyTier (Trash/Elite, enemy_template.hpp) com
// save::DifficultyLevel (save_data.hpp) - MESMO padrao de tabela+lookup de
// gus::domain::deck::battery_capacity_for (card_hardware_constants.hpp): tabela
// constexpr fechada + funcao inline de lookup, NUNCA `if` de dificuldade espalhado
// pelo motor de combate (lei do atomo, ADR-020: cada regra por dificuldade e DADO,
// nao condicional por unidade).
//
// POR QUE AQUI (templates/, e nao um umbrella noutro lugar, como
// card_hardware_constants.hpp precisou): aquele umbrella fugiu de hardware/ porque
// "hardware/ nao pode incluir save/" (limite documentado la, hardware/ e leaf de
// proposito). templates/ NAO tem essa restricao - nada em save/ inclui templates/
// (so um comentario cruzado em save_serializer.hpp, nao um #include real; ver
// domain/CMakeLists.txt: os dois sub-modulos compilam no MESMO alvo estatico, sem
// biblioteca separada por pasta) -, entao templates/ pode incluir save/ sem risco de
// ciclo. EnemyTier e um conceito de TEMPLATE (classificacao de balanceamento do
// inimigo), entao a tabela que o usa como eixo fica ao lado de enemy_template.hpp, no
// mesmo espirito de manter a peca de dado junto do tipo que ela classifica.
//
// ESCOPO DESTA FATIA (decisao do lider): SO Medio (1.0x) e valor REAL - e o nivel
// NEUTRO por definicao (nao e "ainda nao calibrado", e o proprio ponto-zero contra o
// qual os outros 3 seriam medidos). Facil/Dificil/Hardcore AINDA NAO foram calibrados
// - 1.0x tambem, mas por SEGURANCA (mesmo principio "zero/neutro e seguro" ja usado em
// outros lugares do codigo ate alguem medir de verdade), marcados //PLAYTEST na
// tabela abaixo. Os dois "1.0" tem proposito DIFERENTE mesmo sendo o mesmo numero
// hoje - quando Facil/Dificil/Hardcore forem calibrados, so ELES mudam; Medio
// permanece 1.0 para sempre.
//
// Header-only (sem .cpp), mesmo padrao de card_hardware_constants.hpp.
//
// Cross-ref: docs/design/mecanicas/combat.md secao 17 ("Stats de referencia do
// encontro" - Sentinela-Bit Trash, Daemon-Guard Elite); ADR-020; TODO.md item
// DIFICULDADE-TABELA-DADO (dependencia BAL-STATLINES-APLICAR).

#ifndef GUS_DOMAIN_TEMPLATES_ENEMY_DIFFICULTY_CONSTANTS_HPP
#define GUS_DOMAIN_TEMPLATES_ENEMY_DIFFICULTY_CONSTANTS_HPP

#include <array>
#include <cstddef>

#include "gus/domain/save/save_data.hpp"            // DifficultyLevel
#include "gus/domain/templates/enemy_template.hpp"  // EnemyTier

namespace gus::domain::templates {

// ---- Multiplicador de dificuldade sobre HP/Atk de inimigo -------------------------
//
// 2 EnemyTier x 4 DifficultyLevel. Linhas na ORDEM ordinal de EnemyTier (Trash=0,
// Elite=1); colunas na ORDEM ordinal de DifficultyLevel (Facil=0..Hardcore=3,
// save_data.hpp). Ver nota de escopo no topo do arquivo: todas as 8 celulas valem
// 1.0f hoje, por DOIS motivos distintos (Medio definitivo; os outros 3 placeholder).
inline constexpr std::array<std::array<float, 4>, 2> kEnemyDifficultyMultiplierTable = {{
    // Trash:  Facil //PLAYTEST | Medio 1.0x DEFINITIVO | Dificil //PLAYTEST | Hardcore //PLAYTEST
    {{1.0f, 1.0f, 1.0f, 1.0f}},
    // Elite:  Facil //PLAYTEST | Medio 1.0x DEFINITIVO | Dificil //PLAYTEST | Hardcore //PLAYTEST
    {{1.0f, 1.0f, 1.0f, 1.0f}},
}};

// Multiplicador (aplicado IGUALMENTE sobre HP e Atk do inimigo, DIFICULDADE-TABELA-
// DADO) para (EnemyTier, DifficultyLevel). Table lookup puro - EnemyTier tem
// exatamente 2 valores canonicos (kEnemyTierCount) e DifficultyLevel exatamente 4
// (kDifficultyLevelCount, save_data.hpp); os chamadores desta funcao SEMPRE passam
// valores ja validados (nunca u32 cru de payload nao-confiavel - o hardening de
// ordinal fica em EnemyTemplate::validate(), nao aqui, mesmo padrao de
// battery_capacity_for/card_hardware_constants.hpp).
[[nodiscard]] inline float difficulty_multiplier_for(
    EnemyTier tier, gus::domain::save::DifficultyLevel difficulty) noexcept {
    return kEnemyDifficultyMultiplierTable[static_cast<std::size_t>(tier)]
                                           [static_cast<std::size_t>(difficulty)];
}

}  // namespace gus::domain::templates

#endif  // GUS_DOMAIN_TEMPLATES_ENEMY_DIFFICULTY_CONSTANTS_HPP
