// gus/domain/combat/environment_clock.hpp
//
// Roda temporal do PERIODO (secao 18.3): Dia(5) -> Crepusculo(2) -> Noite(5) ->
// Aurora(2) -> Dia... em turnos de fila. Deterministica (sem RNG). Portado de
// engine/foundation/turn_combat/EnvironmentClock.cs. POCO puro, ZERO Qt (invariante de
// domain/, engine-design.md secao 2).
//
// As fases curtas (Crep/Aurora) ja funcionam como telegraph de graca da proxima fase
// forte; alem disso a roda telegrafa N turnos antes QUALQUER troca de fase (Pillar 4:
// nunca RNG punitivo; Scan revela o numero exato). Default telegraph_turns = 2.
//
// MAPEAMENTO de excecoes C# -> C++:
//   ArgumentException            -> std::invalid_argument (fase de inicio nao e periodo);
//   ArgumentOutOfRangeException  -> std::out_of_range (telegraph_turns < 0; project < 0).
//
// Cross-ref: engine/foundation/turn_combat/EnvironmentClock.cs;
//            engine/tests/turn_combat/environments/EnvironmentClockTests.cs;
//            docs/design/mecanicas/combat.md secao 18.1 (telegraph)/18.3 (roda)/18.6; ADR-006.

#ifndef GUS_DOMAIN_COMBAT_ENVIRONMENT_CLOCK_HPP
#define GUS_DOMAIN_COMBAT_ENVIRONMENT_CLOCK_HPP

#include <initializer_list>
#include <vector>

#include "gus/domain/combat/environment_enums.hpp"
#include "gus/domain/combat/environment_modifier.hpp"

namespace gus::domain::combat {

// Motor deterministico da roda de periodo (secao 18.3). POCO. Avanca por turno; expoe a
// fase ativa, os turnos restantes e a proxima fase (telegraph).
class EnvironmentClock {
public:
    // Telegraph default (secao 18.1 "N turnos antes"): a troca e avisada quando faltam <= N
    // turnos na fase atual. Proposta gameplay_engineer = 2 (a fase curta Crep/Aurora e a
    // propria janela-ponte).
    static constexpr int kDefaultTelegraphTurns = 2;

    // Inicia a roda. start_phase deve ser uma fase de periodo (secao 18.3); default Dia.
    // telegraph_turns = antecedencia do aviso de troca. Lanca std::invalid_argument se
    // start_phase nao e fase de periodo; std::out_of_range se telegraph_turns < 0.
    explicit EnvironmentClock(EnvironmentId start_phase = EnvironmentId::Dia,
                              int telegraph_turns = kDefaultTelegraphTurns);

    // Fase de periodo ativa (secao 18.3).
    [[nodiscard]] EnvironmentId current_phase() const noexcept;

    // Ambiente da fase ativa (catalogo).
    [[nodiscard]] const EnvironmentModifier& current() const;

    // Proxima fase na roda (telegraph). Roda fechada de 4: nunca pula estados (secao 18.6).
    [[nodiscard]] EnvironmentId next_phase() const noexcept;

    // Duracao total (turnos) da fase ativa (secao 18.3: 5/2/5/2).
    [[nodiscard]] int phase_duration() const;

    // Turnos restantes na fase ativa (Scan revela este numero exato, secao 18.1). Quando
    // chega a 0, o proximo advance() troca de fase.
    [[nodiscard]] int turns_remaining() const;

    // true quando a troca de fase esta dentro da janela de telegraph (secao 18.1: avisada N
    // turnos antes). A UI mostra icone persistente; Scan revela o numero exato.
    [[nodiscard]] bool transition_telegraphed() const;

    // Avanca um turno de fila. Deterministico: ao esgotar a duracao da fase, troca para a
    // proxima da roda (sem pular estados, secao 18.6). Retorna true se HOUVE troca de fase
    // neste avanco (o caller emite o evento de mudanca de camada).
    bool advance();

    // Sequencia projetada de fases a partir de AGORA por `turns` turnos (sem mutar o
    // relogio). Util para o telegraph visual e para testar a contagem 5/2/5/2. Lanca
    // std::out_of_range se turns < 0.
    [[nodiscard]] std::vector<EnvironmentId> project(int turns) const;

    // Vetor anti-padrao exclusivo do boss (secao 18.4 / N.2 R3): true quando o ambiente e a
    // Anomalia Perlin (T6). Degrada Scan/Gambito (ruido), NAO o dano. Utilitario de leitura.
    [[nodiscard]] static bool is_chaotic_vector(EnvironmentId id) noexcept;

    // true se algum ambiente ativo e a Anomalia Perlin (T6).
    [[nodiscard]] static bool any_chaotic(std::initializer_list<EnvironmentId> active) noexcept;
    [[nodiscard]] static bool any_chaotic(const std::vector<EnvironmentId>& active) noexcept;

private:
    int telegraph_turns_;
    int phase_index_;
    int turns_into_phase_ = 0;  // 0-based dentro da fase atual
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_ENVIRONMENT_CLOCK_HPP
