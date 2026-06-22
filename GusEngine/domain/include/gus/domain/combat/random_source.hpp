// gus/domain/combat/random_source.hpp
//
// Porta INJETAVEL de aleatoriedade do combate (secao 11). PORTE FIEL da porta que o
// C# ja injeta na CombatStateMachine via `Random? rng`: nos testes o C# passa um
// FixedRandom (engine/tests/turn_combat/FixedRandom.cs); em jogo passaria Random.Shared.
// Ou seja, o RNG ja e uma PORTA injetavel, NAO um singleton global - portamos isso 1:1.
//
// A FSM (combat_state_machine) recebe um IRandomSource& por injecao e NUNCA chama RNG
// global dentro do dominio: mantem domain/ puro/deterministico (POCO, ZERO Qt, ZERO I/O).
// A superficie espelha EXATAMENTE o que a FSM consome de System.Random:
//   System.Random.NextDouble()      -> double next_double()  (variancia Knowledge Decay)
//   System.Random.Next(int maxValue)-> int    next(int)      (chance de critico por carta)
//
// A semente real (data+hora+ms, ADR-006 item 5) e injetada na FRONTEIRA app/ DEPOIS
// (uma impl concreta semeada por relogio), NAO neste porte. Aqui so existe o contrato +
// (nos testes) o duplo deterministico FixedRandom. Sem seeding por relogio no dominio.
//
// Cross-ref: engine/foundation/turn_combat/CombatStateMachine.cs (campo _rng);
//            engine/tests/turn_combat/FixedRandom.cs; docs/design/mecanicas/combat.md secao 11;
//            ADR-006.

#ifndef GUS_DOMAIN_COMBAT_RANDOM_SOURCE_HPP
#define GUS_DOMAIN_COMBAT_RANDOM_SOURCE_HPP

namespace gus::domain::combat {

// Porta de aleatoriedade do combate. So os dois metodos que a FSM usa (secao 11);
// espelha a superficie de System.Random injetada no C#. Implementacoes: FixedRandom
// (testes, deterministico) e uma impl semeada por relogio na fronteira app/ (futuro).
class IRandomSource {
public:
    virtual ~IRandomSource() = default;

    // Espelha Random.NextDouble(): double em [0,1). Usado na variancia Knowledge Decay.
    [[nodiscard]] virtual double next_double() = 0;

    // Espelha Random.Next(maxValue): inteiro em [0, max_value). Usado no roll de critico
    // por carta (is_crit = next(100) < crit_chance). max_value <= 0 => 0 (defensivo).
    [[nodiscard]] virtual int next(int max_value) = 0;
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_RANDOM_SOURCE_HPP
