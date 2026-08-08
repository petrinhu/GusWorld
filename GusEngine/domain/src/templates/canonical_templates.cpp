// SPDX-License-Identifier: Apache-2.0
// gus/domain/src/templates/canonical_templates.cpp
//
// Factory dos templates canonicos do encontro de referencia (combat.md secao 17).
// Ver header. Valores travados pelos testes. POCO puro, ZERO Qt.

#include "gus/domain/templates/canonical_templates.hpp"

namespace gus::domain::templates::canonical {

CharacterTemplate gus() {
    return CharacterTemplate{
        /*id=*/"gus",
        /*max_hp=*/34,
        /*atk=*/8,
        /*def=*/5,
        /*spd=*/9,
        /*family=*/CardFamily::Eletrico,  // familia-ancora; universalidade e a flag
        /*is_universal_compiler=*/true,
        /*base_deck=*/{"pulso_eletrico", "scan_basico"}};
}

CharacterTemplate caua() {
    return CharacterTemplate{
        /*id=*/"caua",
        /*max_hp=*/55,
        /*atk=*/14,
        /*def=*/8,
        /*spd=*/13,
        /*family=*/CardFamily::Eletrico,
        /*is_universal_compiler=*/false,
        /*base_deck=*/{"pulso_eletrico"}};
}

CharacterTemplate jaci() {
    return CharacterTemplate{
        /*id=*/"jaci",
        /*max_hp=*/55,
        /*atk=*/9,
        /*def=*/10,
        /*spd=*/7,
        /*family=*/CardFamily::Bioquimico,
        /*is_universal_compiler=*/false,
        /*base_deck=*/{"raiz_cura"}};
}

EnemyTemplate sentinela_bit() {
    return EnemyTemplate{
        /*id=*/"sentinela_bit",
        // BAL-STATLINES-APLICAR (2026-08-07): HP 55->33, Atk 6->12. Ponto vencedor da
        // Fase A do PACING-SIM (analise-pacing-fase-a.md secao 4). Achado a parte, nao
        // decidido nesta tarefa: o Atk anterior (6) nunca foi o "10 provisorio" que
        // combat.md secao 17 documentava como valor de producao - ficou parado no valor
        // do commit M3 (2026-06-21), 4 dias antes da decisao "10 provisorio" (2026-06-25)
        // existir, e nenhum teste travava este campo (ver canonical_templates_test.cpp).
        /*max_hp=*/33,
        /*atk=*/12,
        /*def=*/8,
        /*spd=*/4,
        /*family=*/CardFamily::Cinetico,
        /*brain=*/BrainKind::Scripted,
        /*is_boss=*/false,
        /*base_deck=*/{},
        // MODOS-MORTE Fase 0 (§3.1): construto digital, nao humano - fauna/sentinela
        // da Selve/dos Dutos, mesma classe diegetica de daemon_guard abaixo.
        /*kind=*/EnemyKind::Creature};
}

EnemyTemplate daemon_guard() {
    return EnemyTemplate{
        /*id=*/"daemon_guard",
        /*max_hp=*/144,  // Fibonacci canon
        /*atk=*/11,
        /*def=*/14,
        /*spd=*/6,
        /*family=*/CardFamily::Cinetico,
        /*brain=*/BrainKind::Scripted,
        /*is_boss=*/false,
        /*base_deck=*/{},
        // MODOS-MORTE Fase 0 (§3.1): daemon = construto/processo, nao humano.
        /*kind=*/EnemyKind::Creature};
}

std::vector<CharacterTemplate> all_characters() {
    return {gus(), caua(), jaci()};
}

std::vector<EnemyTemplate> all_enemies() {
    return {sentinela_bit(), daemon_guard()};
}

}  // namespace gus::domain::templates::canonical
