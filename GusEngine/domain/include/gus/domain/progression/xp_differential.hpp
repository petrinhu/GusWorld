// gus/domain/progression/xp_differential.hpp
//
// Formula PURA de XP differential por zona (F2-G.XP). Aritmetica isolada e
// testavel. Portado de engine/foundation/knowledge/XpDifferential.cs.
//
// POCO puro, ZERO Qt (invariante de domain/, engine-design.md secao 2). Sem
// estado, sem I/O.
//
// CANONICA (combat.md secao 11):
//   xp = base_xp x max(0, 1 - (player_zone - enemy_zone) x 0.15)
//
// PROPOSITO (knowledge-progression.md secao 4, anti-grind): farmar inimigo de
// zona muito abaixo da do player faz o XP despencar (chega a 0 com gap de 7
// zonas). Converge com o decaimento de variancia (Eixo A) para o mesmo sinal:
// repetir nao da poder.
//
// DECISAO DE DESIGN preservada do C# (clamp superior em 1.0, ja flagada ao
// criador no porte original): a spec escreve "max(0, 1 - gap x 0.15)" sem teto
// superior. Quando o player esta ABAIXO do inimigo (enemy_zone > player_zone), o
// termo (player_zone - enemy_zone) e negativo e o fator passaria de 1.0 (bonus
// por cacar acima do nivel). Optamos por CLAMPAR em 1.0: KP secao 7 proibe
// catch-up/pity/rubber-band. Bonificar overlevel reverso seria exatamente o
// catch-up que o pillar recusa. Logo o fator vive em [0.0, 1.0].
//
// Cross-ref: docs/design/mecanicas/combat.md secao 11, knowledge-progression.md
//            secao 4/7; engine/foundation/knowledge/XpDifferential.cs (origem).

#ifndef GUS_DOMAIN_PROGRESSION_XP_DIFFERENTIAL_HPP
#define GUS_DOMAIN_PROGRESSION_XP_DIFFERENTIAL_HPP

namespace gus::domain::progression {

// Penalidade por zona de diferenca (combat.md secao 11).
inline constexpr double kPenaltyPerZone = 0.15;

// Fator multiplicador do XP em [0.0, 1.0] dado o diferencial de zona:
//   clamp(1 - max(0, player_zone - enemy_zone) x 0.15, 0, 1).
// Player ACIMA do inimigo penaliza; player ABAIXO NAO bonifica (clamp 1.0,
// anti catch-up secao 7).
//
// Invariante (fail-fast): zonas devem ser >= 0; caso contrario lanca
// std::invalid_argument (espelha o ArgumentOutOfRangeException do C#).
[[nodiscard]] double xp_factor(int player_zone, int enemy_zone);

// XP concedido = base_xp x xp_factor(...), arredondado round-half-away-from-zero
// (deterministico). Nunca negativo.
//
// Invariante (fail-fast): base_xp e zonas devem ser >= 0; senao
// std::invalid_argument.
[[nodiscard]] int xp_award(int base_xp, int player_zone, int enemy_zone);

}  // namespace gus::domain::progression

#endif  // GUS_DOMAIN_PROGRESSION_XP_DIFFERENTIAL_HPP
