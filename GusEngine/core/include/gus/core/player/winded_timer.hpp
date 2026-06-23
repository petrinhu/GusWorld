// gus/core/player/winded_timer.hpp
//
// WindedTimer: o TIMER DE FOLEGO (do CORPO) do Gus - POCO C++ puro (ZERO SDL, ZERO
// I/O, ZERO GPU). SEPARADO da Carga do aparato (core::player::Stamina). Decisao do
// lider 2026-06-23 (docs/design/mecanicas/stamina.md, secao "Timer de folego vs
// Carga"): ao PARAR de correr estando cansado, o Gus ofega por um MINIMO de ~5 s que
// ESCALA com quanto tempo ele correu (ate um teto), INDEPENDENTE de a Carga do aparato
// ja ter recarregado.
//
// Por que existe (separado da Stamina): a Carga regenera RAPIDO ao parar (~13/s, canon),
// entao amarrar a ofegancia a Carga fazia o Gus ofegar so ~2-3 s - curto demais. O
// "folego do corpo" tem inercia propria: o peito demora a acalmar mesmo com o aparato
// ja recarregado. Sao duas leituras: Carga = hardware; folego = consequencia no corpo
// (Pillar 4, vulnerabilidade do corpo sem virar atleta).
//
// MODELO:
//   - Enquanto CORRE (tick_running): ACUMULA tempo de corrida (run_accumulated). Nao
//     esta ofegante correndo; voltar a correr LIMPA qualquer folego ativo e reinicia o
//     acumulo (o esforco recomeca).
//   - Ao PARAR (primeiro tick_stopped apos correr): se o acumulo passou do limiar de
//     corrida (run_threshold_seconds), DISPARA o folego ativo com uma duracao =
//       min_winded + (max_winded - min_winded) * clamp(run_acc / run_for_max, 0, 1)
//     (escala LINEAR, saturada no teto). Abaixo do limiar, nao ofega (corrida curta).
//   - PARADO com folego ativo: DECAI remaining pelo dt ate zerar (clamp em 0). Quando
//     remaining chega a 0, is_winded() volta a false.
//
// CONTRATO:
//   - tick_running(dt) / tick_stopped(dt): dt em segundos (<= 0 ignorado).
//   - is_winded(): true enquanto remaining() > 0 (forca o idle ofegante no consumidor,
//     INDEPENDENTE da Carga).
//   - remaining(): segundos de folego restantes (>= 0).
//   - Numeros degenerados saneados no ctor (teto < piso vira piso; negativos viram 0).
//
// Tudo configuravel vem do OverworldTuning (ponto unico de feel do overworld); este
// POCO so guarda os numeros ja saneados e aplica a regra. Determinista -> mora no core/.

#ifndef GUS_CORE_PLAYER_WINDED_TIMER_HPP
#define GUS_CORE_PLAYER_WINDED_TIMER_HPP

namespace gus::core::player {

// Parametros do folego do corpo (em segundos). Numeros do lider 2026-06-23.
struct WindedConfig {
    // Piso da ofegancia ao parar (apos passar do limiar de corrida). CANON: 5 s.
    float min_winded_seconds = 5.0f;
    // Teto da ofegancia (corrida longa nao ofega alem disso). CANON: 8 s.
    float max_winded_seconds = 8.0f;
    // Tempo de corrida sustentada que atinge o TETO da ofegancia (escala linear ate
    // aqui; alem disso satura). CANON: 8 s de corrida -> teto de folego.
    float run_for_max_winded = 8.0f;
    // Corrida MINIMA (s) pra parar e ofegar. Abaixo disso, parou de uma corrida curta
    // e NAO ofega. CANON: 2 s.
    float run_threshold_seconds = 2.0f;
};

class WindedTimer {
public:
    // Constroi inativo (sem folego, sem corrida acumulada). Saneia a config (ver .cpp).
    explicit WindedTimer(WindedConfig cfg = {}) noexcept;

    // Um passo CORRENDO (sprint real): acumula tempo de corrida. Se havia folego ativo,
    // ele e LIMPO (o esforco recomeca; correndo nao se esta ofegante). dt <= 0 ignorado.
    void tick_running(float dt) noexcept;

    // Um passo PARADO (ou andando, sem correr): no primeiro passo apos correr, AVALIA o
    // gatilho (acumulo > limiar -> dispara o folego escalado). Depois, DECAI remaining
    // pelo dt ate zerar. dt <= 0 ainda avalia o gatilho (transicao corrida->parado), mas
    // nao decai.
    void tick_stopped(float dt) noexcept;

    // true enquanto resta folego (forca idle ofegante no consumidor, independe da Carga).
    [[nodiscard]] bool is_winded() const noexcept { return remaining_ > 0.0f; }

    // Segundos de folego restantes (>= 0).
    [[nodiscard]] float remaining() const noexcept { return remaining_; }

    // Tempo de corrida acumulado desde o ultimo "parar avaliado" (debug/teste).
    [[nodiscard]] float run_accumulated() const noexcept { return run_acc_; }

    [[nodiscard]] const WindedConfig& config() const noexcept { return cfg_; }

    // Zera tudo (debug/teste/respawn).
    void reset() noexcept;

private:
    WindedConfig cfg_{};
    float run_acc_ = 0.0f;     // tempo de corrida acumulado (define a duracao ao parar)
    float remaining_ = 0.0f;   // folego ativo restante (s); > 0 => ofegante
    bool was_running_ = false; // estava correndo no tick anterior (detecta a transicao)
};

}  // namespace gus::core::player

#endif  // GUS_CORE_PLAYER_WINDED_TIMER_HPP
