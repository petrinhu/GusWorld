// SPDX-License-Identifier: Apache-2.0
// enemy_difficulty_constants_test.cpp
//
// Spec executavel (Catch2 v3) de enemy_difficulty_constants.hpp (DIFICULDADE-TABELA-
// DADO, TODO.md): tabela+lookup do multiplicador de dificuldade sobre HP/Atk de
// inimigo, cruzando EnemyTier x save::DifficultyLevel. MESMO padrao de teste de
// card_hardware_test.cpp::battery_capacity_for.
//
// Escopo desta fatia (ver o cabecalho do header pro racional completo): SO Medio e
// valor REAL/definitivo (1.0x, neutro por definicao). Facil/Dificil/Hardcore sao
// PLACEHOLDER (1.0x tambem, por seguranca "zero/neutro" ate alguem calibrar de
// verdade). Os testes abaixo travam o valor NUMERICO atual (1.0 em toda a tabela) SEM
// reivindicar que os 3 modos nao-Medio estao calibrados - os nomes/comentarios
// distinguem os dois "1.0" de proposito; quando alguem calibrar Facil/Dificil/
// Hardcore de verdade, o teste "placeholder" fica RED (sinal correto: "a tabela
// mudou, atualize aqui"), enquanto o teste "Medio e definitivo" continua verde para
// sempre.
//
// Subsistema: domain/templates. POCO puro, ZERO Qt.
//
// Cross-ref: docs/design/mecanicas/combat.md secao 17; ADR-020; TODO.md item
// DIFICULDADE-TABELA-DADO.

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/save/save_data.hpp"
#include "gus/domain/templates/enemy_difficulty_constants.hpp"
#include "gus/domain/templates/enemy_template.hpp"

using gus::domain::save::DifficultyLevel;
using gus::domain::templates::difficulty_multiplier_for;
using gus::domain::templates::EnemyTier;

TEST_CASE("enemy_difficulty_constants: Medio e 1.0x NEUTRO para Trash e Elite (valor "
          "REAL desta fatia, NAO placeholder)",
          "[domain][templates][difficulty]") {
    REQUIRE(difficulty_multiplier_for(EnemyTier::Trash, DifficultyLevel::Medio) == 1.0f);
    REQUIRE(difficulty_multiplier_for(EnemyTier::Elite, DifficultyLevel::Medio) == 1.0f);
}

TEST_CASE("enemy_difficulty_constants: Facil/Dificil/Hardcore sao 1.0x PLACEHOLDER "
          "(nao calibrados nesta fatia; mesma decisao de seguranca 'neutro' do Medio, "
          "mas por OUTRO motivo)",
          "[domain][templates][difficulty]") {
    for (const auto tier : {EnemyTier::Trash, EnemyTier::Elite}) {
        REQUIRE(difficulty_multiplier_for(tier, DifficultyLevel::Facil) == 1.0f);
        REQUIRE(difficulty_multiplier_for(tier, DifficultyLevel::Dificil) == 1.0f);
        REQUIRE(difficulty_multiplier_for(tier, DifficultyLevel::Hardcore) == 1.0f);
    }
}

TEST_CASE("enemy_difficulty_constants: tabela fechada, as 8 celulas (2 tier x 4 "
          "dificuldade) batem 1.0x nesta fatia",
          "[domain][templates][difficulty]") {
    for (const auto tier : {EnemyTier::Trash, EnemyTier::Elite}) {
        for (const auto diff : {DifficultyLevel::Facil, DifficultyLevel::Medio,
                                 DifficultyLevel::Dificil, DifficultyLevel::Hardcore}) {
            REQUIRE(difficulty_multiplier_for(tier, diff) == 1.0f);
        }
    }
}

TEST_CASE("enemy_difficulty_constants: Trash e Elite NAO divergem entre si nesta "
          "fatia (o eixo que varia hoje e SO a dificuldade, nao o tier)",
          "[domain][templates][difficulty]") {
    for (const auto diff : {DifficultyLevel::Facil, DifficultyLevel::Medio,
                             DifficultyLevel::Dificil, DifficultyLevel::Hardcore}) {
        REQUIRE(difficulty_multiplier_for(EnemyTier::Trash, diff) ==
                difficulty_multiplier_for(EnemyTier::Elite, diff));
    }
}
