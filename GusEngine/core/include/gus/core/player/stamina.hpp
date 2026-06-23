// gus/core/player/stamina.hpp
//
// Stamina: a CARGA DO APARATO (Tavus-Drive) do jogador - ver
// docs/design/mecanicas/stamina.md (NAO e "stamina fisica"; o anti-pillar veta
// cansaco gerenciavel). Nome tecnico "Stamina" mantido (rename pra "Carga" seria
// churn alto em tuning/sim/testes); ler como "Carga do aparato". POCO C++ puro
// (ZERO SDL, ZERO I/O, ZERO GPU). Decide quanta Carga o aparato tem (0..max) e como
// ela DRENA ao correr (sprint) e REGENERA andando/parado. Serve pra alternar a
// respiracao do idle entre CALMA (descansado) e OFEGANTE (overflow no corpo), e fica
// disponivel pra futuras mecanicas (scan, Celula de Pulso). Matematica determinista
// (clamp de um acumulador no tempo) -> mora no core/, testavel sem janela.
//
// SEAM DE TRES ESTADOS (canon 2026-06-23): a taxa efetiva depende do que o jogador
// faz. Antes existia um unico recover_per_sec; agora a regeneracao PARADO e mais
// rapida que ANDANDO (recompensa parar; explorador que so anda nunca trava):
//   - Running (sprint): DRENA drain_per_sec.
//   - Walking (anda):   REGENERA recover_walk_per_sec (devagar).
//   - Idle (parado):    REGENERA recover_idle_per_sec (rapido).
//
// CONTRATO:
//   - O valor vive em [0, max]. max > 0 (saneado no ctor: <= 0 vira 1).
//   - tick(state, dt): dt em segundos (>= 0; negativo/zero ignorado). A direcao e a
//     taxa vem do MoveState; o resultado e CLAMPADO em [0, max] todo passo.
//   - is_tired(): true quando a Carga corrente cai ABAIXO do limiar tired_value (em
//     UNIDADE de Carga, NAO percentual). Gatilho do idle ofegante (overflow).
//   - Taxas em unidades de Carga por segundo. Saneadas (negativas viram 0): a direcao
//     (drenar vs regenerar) vem do MoveState, nao do sinal da taxa.
//   - REGRA DE OURO: Carga 0 remove SO o sprint; andar NUNCA trava (esta regra mora
//     no consumidor - este POCO so contabiliza; quem decide "pode correr?" e o sim).
//
// Tudo configuravel vem do OverworldTuning (ponto unico de feel do overworld); este
// POCO so guarda os numeros ja saneados e aplica a regra.

#ifndef GUS_CORE_PLAYER_STAMINA_HPP
#define GUS_CORE_PLAYER_STAMINA_HPP

namespace gus::core::player {

// O que o jogador esta fazendo neste tick - define a taxa efetiva (seam de 3 estados).
enum class MoveState {
    Running,  // sprint (drena drain_per_sec)
    Walking,  // andando (regenera recover_walk_per_sec, devagar)
    Idle      // parado (regenera recover_idle_per_sec, rapido)
};

// Parametros de Carga (em unidades de Carga; numeros CANONICOS em Fibonacci - ver
// docs/design/mecanicas/stamina.md). max define a escala.
struct StaminaConfig {
    float max = 89.0f;                    // teto (e valor inicial). > 0. CANON: 89.
    float drain_per_sec = 8.0f;           // queda/s CORRENDO (sprint). CANON: 8 (~11s).
    float recover_walk_per_sec = 5.0f;    // ganho/s ANDANDO (devagar). CANON: 5.
    float recover_idle_per_sec = 13.0f;   // ganho/s PARADO (rapido). CANON: 13.
    float tired_value = 34.0f;            // ABAIXO disto (em UNIDADE) -> ofegante. CANON: 34.
};

class Stamina {
public:
    // Constroi cheio (value == max). Saneia: max <= 0 vira 1; taxas negativas viram
    // 0; tired_value e clampado em [0, max].
    explicit Stamina(StaminaConfig cfg = {}) noexcept;

    // Um passo no tempo, segundo o MoveState (seam de 3 estados). dt em segundos
    // (<= 0 nao faz nada). O valor e clampado em [0, max] ao fim.
    void tick(MoveState state, float dt) noexcept;

    [[nodiscard]] float value() const noexcept { return value_; }
    [[nodiscard]] float max() const noexcept { return cfg_.max; }

    // Fracao corrente em PORCENTAGEM [0, 100] (value/max * 100). Util pra HUD/aura.
    [[nodiscard]] float percent() const noexcept { return value_ / cfg_.max * 100.0f; }

    // true quando a Carga caiu abaixo do limiar em UNIDADE (idle ofegante / overflow).
    // Comparacao ESTRITA: exatamente no limiar ainda conta como descansado (calmo).
    [[nodiscard]] bool is_tired() const noexcept { return value_ < cfg_.tired_value; }

    // Reseta para cheio (debug/teste/respawn).
    void refill() noexcept { value_ = cfg_.max; }

    [[nodiscard]] const StaminaConfig& config() const noexcept { return cfg_; }

private:
    StaminaConfig cfg_{};
    float value_ = 89.0f;  // Carga corrente em [0, max].
};

}  // namespace gus::core::player

#endif  // GUS_CORE_PLAYER_STAMINA_HPP
