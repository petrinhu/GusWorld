// gus/domain/combat/weakness_wheel.hpp
//
// Roda de fraqueza fechada e deterministica (secao 6). Tabela consultavel:
// mult_fraqueza por par (atacante, alvo). Sem RNG. Portado de
// engine/foundation/turn_combat/WeaknessWheel.cs. POCO puro, ZERO Qt (invariante de
// domain/, engine-design.md secao 2). Header-only: logica pura sem estado.
//
// Ciclo (forte contra a seguinte):
//   Eletrico -> Cinetico -> Criptografico -> Sonico -> Bioquimico -> Eletrico
//
// O C# modela como `static class` de funcoes estaticas. Aqui adotamos um namespace
// `WeaknessWheel` com funcoes livres (equivalente C++ idiomatico de static class):
// chamadas WeaknessWheel::tier_for(...) / WeaknessWheel::multiplier(...) espelham
// 1:1 o WeaknessWheel.TierFor(...) / WeaknessWheel.Multiplier(...) do C#.
//
// Cross-ref: engine/foundation/turn_combat/WeaknessWheel.cs;
//            docs/design/mecanicas/combat.md secao 6/11/17; ADR-006.

#ifndef GUS_DOMAIN_COMBAT_WEAKNESS_WHEEL_HPP
#define GUS_DOMAIN_COMBAT_WEAKNESS_WHEEL_HPP

#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_enums.hpp"

namespace gus::domain::combat::WeaknessWheel {

// Sucessor no ciclo: cada familia e forte contra o seu sucessor. (espelha
// WeaknessWheel.StrongAgainst, private no C#; aqui no detail por ser implementacao).
namespace detail {

[[nodiscard]] constexpr CardFamily strong_against(CardFamily attacker) {
    switch (attacker) {
        case CardFamily::Eletrico:
            return CardFamily::Cinetico;
        case CardFamily::Cinetico:
            return CardFamily::Criptografico;
        case CardFamily::Criptografico:
            return CardFamily::Sonico;
        case CardFamily::Sonico:
            return CardFamily::Bioquimico;
        case CardFamily::Bioquimico:
            return CardFamily::Eletrico;
        case CardFamily::Universal:
            // Universal (PS-R1) esta FORA da roda de 5: tier_for() curto-circuita antes
            // de chegar aqui (Universal nunca e passado como attacker/target pra esta
            // funcao). Case explicito so pra manter o switch exaustivo (-Wswitch);
            // retornamos a propria familia (== o idioma do fallback abaixo).
            return attacker;
    }
    // Familia desconhecida: o C# lanca ArgumentOutOfRangeException. No dominio POCO
    // C++ mantemos puro (sem excecao de runtime fora do contrato): retornamos a
    // propria familia, que classifica como Neutro (degrade seguro). Inalcancavel
    // para os 6 valores canonicos do enum.
    return attacker;
}

}  // namespace detail

// Tier de relacao entre atacante e alvo. secao 6.
// Imune (0.0) NAO faz parte da roda base; e flag de inimigo/lore (incremento futuro).
//
// Universal (PS-R1, decisao do criador 2026-07-14): curto-circuito ANTES de consultar
// o ciclo. Universal NAO compete na roda — se atacante OU alvo for Universal, o tier e
// SEMPRE Neutro (1.0), em qualquer combinacao (inclusive Universal x Universal).
[[nodiscard]] constexpr WeaknessTier tier_for(CardFamily attacker, CardFamily target) {
    if (attacker == CardFamily::Universal || target == CardFamily::Universal)
        return WeaknessTier::Neutro;
    if (detail::strong_against(attacker) == target)
        return WeaknessTier::Fraco;  // atacante forte contra alvo => alvo e fraco
    if (detail::strong_against(target) == attacker)
        return WeaknessTier::Resistente;  // alvo forte contra atacante
    return WeaknessTier::Neutro;
}

// Multiplicador de fraqueza pra formula de dano (secao 11): 1.5 / 1.0 / 0.66.
// (Imune 0.0 e tratado fora da roda base; ver tier_for.)
[[nodiscard]] constexpr float multiplier(CardFamily attacker, CardFamily target) {
    switch (tier_for(attacker, target)) {
        case WeaknessTier::Fraco:
            return combat_constants::kMultFraco;
        case WeaknessTier::Resistente:
            return combat_constants::kMultResistente;
        case WeaknessTier::Imune:
            return combat_constants::kMultImune;
        case WeaknessTier::Neutro:
            return combat_constants::kMultNeutro;
    }
    return combat_constants::kMultNeutro;
}

}  // namespace gus::domain::combat::WeaknessWheel

#endif  // GUS_DOMAIN_COMBAT_WEAKNESS_WHEEL_HPP
