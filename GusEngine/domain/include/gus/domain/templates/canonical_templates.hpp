// gus/domain/templates/canonical_templates.hpp
//
// Fonte-de-verdade EM CODIGO dos templates do encontro de referencia do vertical
// slice (combat.md secao 17). Portado de engine/foundation/data/CanonicalTemplates.cs.
// POCO puro, versionavel, testavel sem Godot/Qt.
//
// Por que dado em codigo: os valores canonicos viram diff legivel em PR e ficam
// travados por canonical_templates_test (RED se alguem desviar do canon). A geracao
// do .gdt fisico em res:// e editor-only (FORA deste marco).
//
// Gus = Family Eletrico (ancora) + is_universal_compiler=true (a universalidade e a
// FLAG, nao um valor de enum, combat.md secao 6.1).
//
// POCO puro, ZERO Qt (invariante de domain/, engine-design.md secao 2).
//
// Cross-ref: engine/foundation/data/CanonicalTemplates.cs; combat.md secao 17/6.1/13.

#ifndef GUS_DOMAIN_TEMPLATES_CANONICAL_TEMPLATES_HPP
#define GUS_DOMAIN_TEMPLATES_CANONICAL_TEMPLATES_HPP

#include <vector>

#include "gus/domain/templates/character_template.hpp"
#include "gus/domain/templates/enemy_template.hpp"

namespace gus::domain::templates::canonical {

// ---- Party (combat.md secao 17) -------------------------------------------

// Gus Vector Tavus Vance: HP34/Atk8/Def5/SPD9, compilador universal. secao 17/6.1/2.1.
[[nodiscard]] CharacterTemplate gus();

// Caua "Volt" Berenger: Striker eletrico, HP55/Atk14/Def8/SPD13. secao 17.
[[nodiscard]] CharacterTemplate caua();

// Jaci "Proxy" Vanderbist: Healer bioquimica, HP55/Atk9/Def10/SPD7. secao 17.
[[nodiscard]] CharacterTemplate jaci();

// ---- Inimigos do encontro de referencia (combat.md secao 17) --------------

// Sentinela-Bit: Trash, HP55/Def8, Cinetico, Scripted. secao 17.
[[nodiscard]] EnemyTemplate sentinela_bit();

// Daemon-Guard: Elite, HP144/Def14, Cinetico, Scripted (placeholder). secao 17/13.
[[nodiscard]] EnemyTemplate daemon_guard();

// ---- Agregadores -----------------------------------------------------------

// Todos os CharacterTemplate canonicos (gus, caua, jaci).
[[nodiscard]] std::vector<CharacterTemplate> all_characters();

// Todos os EnemyTemplate canonicos (sentinela_bit, daemon_guard).
[[nodiscard]] std::vector<EnemyTemplate> all_enemies();

}  // namespace gus::domain::templates::canonical

#endif  // GUS_DOMAIN_TEMPLATES_CANONICAL_TEMPLATES_HPP
