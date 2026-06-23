// gus/core/anim/anim_clock.hpp
//
// AnimClock: relogio de animacao por TEMPO, POCO C++ puro (ZERO SDL, ZERO I/O,
// ZERO GPU). Avanca um quadro ciclico [0, frame_count) a uma taxa de fps dada,
// acumulando o dt real (segundos). E o oposto do WalkCycle (que avanca por
// DISTANCIA, pro overworld): aqui a animacao toca sozinha no tempo, como num
// VIEWER de preview, onde nao ha movimento de mundo pra dirigir o passo.
//
// Por que no core/ e nao no app/: e matematica pura e deterministica (acumulador
// de tempo -> indice de quadro), testavel sem janela. A casca (varredura de
// pasta, carga de PNG, desenho) fica no app/ (anim_catalog / anim_preview). A
// invariante das 4 camadas manda: core nao toca SDL/IO.
//
// CONTRATO:
//   - frame_count >= 1; fps em [kMinFps, kMaxFps]. Construtor saneia.
//   - advance(dt): dt em segundos (>= 0; negativo/zero e ignorado). Acumula e, a
//     cada 1/fps acumulado, avanca 1 quadro com wrap em frame_count. Num dt grande
//     (stall), avanca varios quadros de uma vez (sem engasgar).
//   - set_fps(fps): ajusta a taxa (clampada). Nao reseta o quadro.
//   - set_frame_count(n): troca de animacao (n >= 1). Reseta quadro=0 e acumulador.
//   - frame(): indice corrente em [0, frame_count).

#ifndef GUS_CORE_ANIM_ANIM_CLOCK_HPP
#define GUS_CORE_ANIM_ANIM_CLOCK_HPP

namespace gus::core::anim {

class AnimClock {
public:
    // Limites de saneamento. fps minimo evita divisao por ~0 e "anim parada"; o
    // maximo so contem absurdos (um preview nao precisa de mais que isso).
    static constexpr float kMinFps = 0.1f;
    static constexpr float kMaxFps = 240.0f;

    // frame_count: quadros da animacao (>= 1). fps: taxa de troca (clampada em
    // [kMinFps, kMaxFps]). Default 10 fps (pedido do viewer).
    explicit AnimClock(int frame_count = 1, float fps = 10.0f) noexcept;

    // Avanca o relogio com o dt real deste frame (segundos). dt <= 0 nao faz nada.
    void advance(float dt) noexcept;

    // Ajusta a taxa (clampada). Nao mexe no quadro corrente nem no acumulador.
    void set_fps(float fps) noexcept;

    // Soma um delta ao fps corrente (atalho pros controles +/- do viewer).
    void nudge_fps(float delta) noexcept { set_fps(fps_ + delta); }

    // Troca a animacao: novo numero de quadros (>= 1), reseta quadro=0 e acumulador.
    void set_frame_count(int frame_count) noexcept;

    // Volta ao quadro 0 e zera o acumulador (recomecar a anim corrente).
    void reset() noexcept;

    [[nodiscard]] int frame() const noexcept { return frame_; }
    [[nodiscard]] int frame_count() const noexcept { return frame_count_; }
    [[nodiscard]] float fps() const noexcept { return fps_; }
    [[nodiscard]] float accumulated() const noexcept { return accum_; }

private:
    int frame_count_ = 1;
    float fps_ = 10.0f;
    int frame_ = 0;
    float accum_ = 0.0f;  // tempo acumulado (s) ainda nao "gasto" numa troca
};

}  // namespace gus::core::anim

#endif  // GUS_CORE_ANIM_ANIM_CLOCK_HPP
