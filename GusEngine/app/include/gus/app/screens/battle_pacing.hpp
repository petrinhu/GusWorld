// gus/app/screens/battle_pacing.hpp
//
// DIRETOR DE PACING da BattleScreen (M5, incremento 6, D8/D10): POCO 100% testavel SEM
// SDL. Dita o RITMO do combate - a apresentacao processa UM evento de cada vez (1 turno
// -> mostra floater/log -> pausa ~0.8s -> proximo), em vez de drenar tudo do motor de
// uma vez (o problema do playtest: "inimigos ja atacaram antes de eu ver"). O diretor
// NAO toca a FSM nem SDL: so guarda o ESTADO do ritmo + um timer, e diz QUANDO o proximo
// passo pode acontecer (ready_to_step). A BattleScene consome esse sinal pra resolver 1
// turno e re-arma o diretor (begin_enemy_step / begin_player_turn / player_acted).
//
// ESTADOS (D8):
//   Intro              : abertura PARADA (D10) - "BATALHA!", ninguem agiu. Sai por tempo
//                        (kPacingIntroSeconds) OU por skip (1a tecla).
//   WaitingDelay       : acabou de animar 1 turno; segura ~0.8s pro jogador LER o
//                        numero/log. Sai por tempo OU skip (jogador acelera).
//   WaitingPlayerInput : vez do jogador (D9 "sua vez"); NAO auto-avanca - espera o menu.
//                        skip/tempo NAO pulam (so a acao do jogador retoma o ritmo).
//
// ready_to_step() = true significa "a cena PODE resolver o proximo turno agora". A cena
// chama begin_enemy_step() apos animar um turno de inimigo (-> WaitingDelay), ou
// begin_player_turn() ao cair num turno de jogador (-> WaitingPlayerInput), ou
// player_acted() apos a acao do jogador (-> WaitingDelay).
//
// Cross-ref: docs/design/mecanicas/battle-screen.md par.5.2 (D8-D12);
//            gus/app/screens/battle_scene.hpp (consumidor).

#ifndef GUS_APP_SCREENS_BATTLE_PACING_HPP
#define GUS_APP_SCREENS_BATTLE_PACING_HPP

namespace gus::app::screens {

// Duracao da abertura PARADA (D10): "BATALHA!" + arena monta, ninguem age. Curta.
inline constexpr float kPacingIntroSeconds = 0.9f;

// Delay entre turnos (D8): segura o numero/log na tela pro jogador LER. ~0.8s.
inline constexpr float kPacingStepDelaySeconds = 0.8f;

// Estado do ritmo.
enum class PacingState : int {
    Intro = 0,               // abertura parada (D10)
    WaitingDelay = 1,        // pausa pos-turno (~0.8s) pro jogador ler
    WaitingPlayerInput = 2,  // vez do jogador (D9): espera o menu, nao auto-avanca
};

// Diretor de ritmo. POCO: timer + estado. NAO toca a FSM nem SDL.
class PacingDirector {
public:
    PacingDirector() = default;

    // Estado atual do ritmo.
    [[nodiscard]] PacingState state() const noexcept { return state_; }

    // true se a cena PODE resolver o proximo turno agora (intro/delay terminou). false em
    // WaitingPlayerInput (espera o menu) e enquanto o tempo/skip nao liberou.
    [[nodiscard]] bool ready_to_step() const noexcept;

    // true se e a vez do jogador (a casca mostra "sua vez" + opera o menu).
    [[nodiscard]] bool waiting_player_input() const noexcept {
        return state_ == PacingState::WaitingPlayerInput;
    }

    // Avanca o tempo (segundos). Decrementa o timer da intro/delay; quando zera, libera o
    // passo (ready_to_step). No-op em WaitingPlayerInput (o tempo nao avanca a vez).
    void tick(float dt_seconds) noexcept;

    // Tecla de ACELERAR (D8): zera o timer da intro/delay na hora -> libera o passo. NAO
    // pula o turno do jogador (WaitingPlayerInput ignora skip).
    void skip() noexcept;

    // A cena acabou de animar 1 turno de INIMIGO: entra em WaitingDelay (segura ~0.8s).
    void begin_enemy_step() noexcept;

    // A cena caiu num turno de JOGADOR vivo: entra em WaitingPlayerInput (espera o menu).
    void begin_player_turn() noexcept;

    // A cena resolveu a acao do JOGADOR: entra em WaitingDelay (o golpe do jogador
    // tambem respeita o ritmo antes do proximo turno).
    void player_acted() noexcept;

private:
    PacingState state_ = PacingState::Intro;
    // Tempo restante do timer corrente (intro/delay). <= 0 = liberado.
    float timer_ = kPacingIntroSeconds;
};

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_PACING_HPP
