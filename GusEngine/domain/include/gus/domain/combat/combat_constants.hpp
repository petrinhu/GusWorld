// gus/domain/combat/combat_constants.hpp
//
// Constantes canonicas do combate (secao 2/5/6/11/18). Centralizadas pra evitar
// magic numbers. Portado de engine/foundation/turn_combat/CombatConstants.cs
// (static class de const). POCO puro, ZERO Qt (invariante de domain/,
// engine-design.md secao 2). Header-only: sao constexpr, sem .cpp.
//
// O C# usa PascalCase (BaseApPerTurn); aqui adotamos o prefixo k + PascalCase
// (kBaseApPerTurn), convencao de constante de compilacao em C++. Os VALORES sao
// 1:1 com o C#; so o nome muda de estilo.
//
// Cross-ref: engine/foundation/turn_combat/CombatConstants.cs;
//            docs/design/mecanicas/combat.md secao 2/5/6/11/18; ADR-006.

#ifndef GUS_DOMAIN_COMBAT_COMBAT_CONSTANTS_HPP
#define GUS_DOMAIN_COMBAT_COMBAT_CONSTANTS_HPP

namespace gus::domain::combat::combat_constants {

// ---- Parametros macro de recurso (secao 2/5) ----

// AP fixo por turno no vertical slice (secao 5). Parametrizavel via skill tree depois.
inline constexpr int kBaseApPerTurn = 3;

// Mana base: mana_max = kBaseMana + turno_index (secao 5).
inline constexpr int kBaseMana = 2;

// Cap de mana (secao 5).
inline constexpr int kManaCap = 8;

// Tamanho minimo de party suportado pelo modulo de engine (secao 2).
inline constexpr int kMinPartySize = 1;

// Tamanho maximo de party suportado pelo modulo de engine (secao 2).
inline constexpr int kMaxPartySize = 4;

// Dano minimo por ataque (clamp secao 11), exceto imunidade (mult 0.0).
inline constexpr int kMinDamage = 1;

// ---- Multiplicadores da roda de fraqueza (secao 6) ----

// Fraco: atacante forte contra alvo.
inline constexpr float kMultFraco = 1.5f;

// Neutro: sem relacao na roda.
inline constexpr float kMultNeutro = 1.0f;

// Resistente: alvo forte contra atacante.
inline constexpr float kMultResistente = 0.66f;

// Imune: caso especial de inimigo/lore.
inline constexpr float kMultImune = 0.0f;

// ---- Ambientes de combate (secao 18, mult_ambiente) ----

// Piso do cap final de mult_ambiente (secao 11). 0.66 x 0.66 ~ 0.4356 -> trava em 0.44.
inline constexpr float kMultAmbienteCapMin = 0.44f;

// Teto do cap final de mult_ambiente (secao 11). 1.5 x 1.5 = 2.25. A curadoria de
// transicoes (secao 18.6) impede 2 fontes x1.5 da MESMA familia organicamente; o cap
// e a trava de seguranca numerica.
inline constexpr float kMultAmbienteCapMax = 2.25f;

// mult_ambiente quando nao ha ambiente marcado (retrocompat secao 11: combate inalterado).
inline constexpr float kMultAmbienteDefault = 1.0f;

// Pico de familia x1.5 (secao 18, faixa por camada).
inline constexpr float kEnvMultPico = 1.5f;

// Boost moderado de familia x1.3 (secao 18).
inline constexpr float kEnvMultAlto = 1.3f;

// Hostil leve x0.85 (secao 18).
inline constexpr float kEnvMultHostilLeve = 0.85f;

// Hostil forte x0.66 (secao 18, espelha o piso da roda de fraqueza).
inline constexpr float kEnvMultHostilForte = 0.66f;

}  // namespace gus::domain::combat::combat_constants

#endif  // GUS_DOMAIN_COMBAT_COMBAT_CONSTANTS_HPP
