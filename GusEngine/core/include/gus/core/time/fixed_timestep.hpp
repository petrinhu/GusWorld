// gus/core/time/fixed_timestep.hpp
//
// Loop de tempo fixo (M1) - POCO C++ puro, ZERO Qt, ZERO I/O, ZERO relogio real.
// Implementa o classico "Fix Your Timestep" (Gaffer on Games): o update logico
// roda a passo FIXO e o render interpola com um alpha residual.
//
// POR QUE PASSO FIXO: simulacao deterministica e estavel (fisica/colisao/IA nao
// dependem do framerate da maquina); o save/replay reproduz; maquinas fracas e
// fortes simulam igual, mudando so a suavidade do render.
//
// CONTRATO:
//   - fixed_dt: duracao de um passo logico em segundos (default 1/60). Cada
//     advance(frame_dt) soma frame_dt (tempo REAL decorrido no frame) ao
//     acumulador interno e devolve quantos passos fixos "cabem" + o alpha.
//   - alpha = accumulator / fixed_dt, sempre em [0,1) APOS consumir os ticks.
//     O render usa alpha pra interpolar entre o estado anterior e o atual da
//     simulacao (movimento suave; o lider VAI ver o retangulo deslizar liso).
//   - max_ticks_per_frame: teto anti "spiral of death". Se um frame demorou
//     muito (debug pause, troca de janela, hitch de disco), o acumulador podia
//     pedir dezenas de ticks e nunca alcançar o tempo real (cascata). Limita-se
//     o numero de ticks; o tempo excedente e DESCARTADO (o acumulador e zerado
//     ate < fixed_dt) pra nao herdar a divida no proximo frame. Efeito: o jogo
//     desacelera por um instante, em vez de travar acumulando.
//
// Deterministico: o tempo entra como PARAMETRO (advance recebe o dt); nenhuma
// chamada a relogio aqui. A casca Qt (app/) mede o dt real e passa pra ca.

#ifndef GUS_CORE_TIME_FIXED_TIMESTEP_HPP
#define GUS_CORE_TIME_FIXED_TIMESTEP_HPP

namespace gus::core::time {

// Resultado de um frame: quantos passos logicos rodar e o alpha de interpolacao.
struct FrameSteps {
    int ticks = 0;       // numero de updates fixos a executar neste frame (>= 0)
    double alpha = 0.0;  // residuo/fixed_dt em [0,1) pra interpolar o render
};

class FixedTimestep {
public:
    // fixed_dt em segundos (default 1/60 = 60 Hz); max_ticks_per_frame e o teto
    // anti spiral-of-death (default 5). Parametros invalidos (dt <= 0,
    // max_ticks <= 0) sao saturados para defaults coerentes (objeto sempre
    // utilizavel; nunca um estado quebrado).
    explicit FixedTimestep(double fixed_dt = 1.0 / 60.0,
                           int max_ticks_per_frame = 5) noexcept;

    // Soma frame_dt (segundos reais do frame) ao acumulador e devolve quantos
    // passos fixos rodar + o alpha residual. frame_dt <= 0 e ignorado (nao gera
    // tick, nao baixa o acumulador). Aplica o clamp de max_ticks_per_frame.
    [[nodiscard]] FrameSteps advance(double frame_dt) noexcept;

    [[nodiscard]] double fixed_dt() const noexcept { return fixed_dt_; }
    [[nodiscard]] int max_ticks_per_frame() const noexcept { return max_ticks_; }

    // Tempo residual acumulado (sempre em [0, fixed_dt) apos um advance, mas pode
    // ser observado entre construcao e o primeiro advance: comeca em 0).
    [[nodiscard]] double accumulator() const noexcept { return accumulator_; }

    // alpha = accumulator/fixed_dt, em [0,1). Conveniencia pra o render fora do
    // fluxo de advance (ex.: reusar o ultimo alpha).
    [[nodiscard]] double alpha() const noexcept { return accumulator_ / fixed_dt_; }

    // Zera o acumulador (ex.: ao retomar de uma pausa longa, pra nao despejar
    // ticks acumulados). Nao altera fixed_dt nem max_ticks.
    void reset() noexcept { accumulator_ = 0.0; }

private:
    double fixed_dt_;
    int max_ticks_;
    double accumulator_ = 0.0;
};

}  // namespace gus::core::time

#endif  // GUS_CORE_TIME_FIXED_TIMESTEP_HPP
