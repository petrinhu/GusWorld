// gus/core/anim/breath_oscillator.hpp
//
// BreathOscillator: respiracao CALMA procedural, POCO C++ puro (ZERO SDL, ZERO I/O,
// ZERO GPU). E uma SENOIDE continua no tempo, NAO uma troca de quadros - por isso
// nao tem staccato: o render usa value() pra um bob/escala suave do sprite parado
// (ex.: +-2-3% de escala vertical, ou 1-2 px de sobe-desce). Substitui o idle
// "travado" de 5 quadros a baixa cadencia quando o personagem esta DESCANSADO.
//
// O oposto do AnimClock (que troca QUADROS discretos): aqui a saida e CONTINUA. Mora
// no core/ por ser matematica determinista (fase no tempo -> sin) e testavel sem janela.
//
// CONTRATO:
//   - cycles_per_min: cadencia respiratoria (ex.: ~16 ciclos/min = humano calmo).
//     <= 0 saneado (vira um minimo) pra nao travar nem gerar NaN.
//   - advance(dt): dt em segundos (<= 0 ignorado). Avanca a FASE; um ciclo inteiro
//     leva 60/cycles_per_min segundos.
//   - value(): offset normalizado em [-1, 1]. Em fase 0 vale 0 (piso/expirado) e
//     SOBE (inspira), de forma que ENTRAR no idle nao da pulo. Quarto de ciclo -> +1
//     (pico inspirado); tres quartos -> -1.
//   - set_cycles_per_min(): troca a cadencia SEM saltar o value corrente (mantem a
//     fase atual; so muda a velocidade dali pra frente).

#ifndef GUS_CORE_ANIM_BREATH_OSCILLATOR_HPP
#define GUS_CORE_ANIM_BREATH_OSCILLATOR_HPP

namespace gus::core::anim {

class BreathOscillator {
public:
    // Cadencia minima saneada (evita divisao por ~0 e respiracao "parada").
    static constexpr float kMinCyclesPerMin = 0.5f;

    // cycles_per_min: ciclos respiratorios por minuto (humano calmo ~16). <= 0 e
    // clampado para kMinCyclesPerMin. Fase inicial = 0 (piso, subindo).
    explicit BreathOscillator(float cycles_per_min = 16.0f) noexcept;

    // Avanca a fase pelo dt (segundos; <= 0 nao faz nada). Wrap em [0, 1).
    void advance(float dt) noexcept;

    // Offset normalizado da respiracao em [-1, 1] (sin(2*pi*fase)).
    [[nodiscard]] float value() const noexcept;

    // Troca a cadencia (clampada) sem mexer na fase corrente (value nao salta).
    void set_cycles_per_min(float cycles_per_min) noexcept;

    [[nodiscard]] float cycles_per_min() const noexcept { return cycles_per_min_; }
    [[nodiscard]] float phase() const noexcept { return phase_; }

private:
    float cycles_per_min_ = 16.0f;
    float phase_ = 0.0f;  // fase normalizada em [0, 1) (0 = piso, 0.25 = pico).
};

}  // namespace gus::core::anim

#endif  // GUS_CORE_ANIM_BREATH_OSCILLATOR_HPP
