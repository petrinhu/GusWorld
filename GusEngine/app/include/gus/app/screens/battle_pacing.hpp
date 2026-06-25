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
// ESTADOS (D8 + 2 BEATS do turno de inimigo):
//   Intro              : abertura PARADA (D10) - "BATALHA!", ninguem agiu. Sai por tempo
//                        (kPacingIntroSeconds) OU por skip (1a tecla).
//   AnnouncingEnemy    : BEAT 1 do turno de inimigo - "Vez de <nome>" + highlight forte,
//                        NADA resolveu ainda (HP intacto, sem floater, sem log de acao).
//                        E aqui que a ANIMACAO de ataque (windup) vai morar no futuro.
//                        Segura ~0.7s OU skip; depois libera o BEAT 2 (resolucao).
//   WaitingDelay       : pausa POS-resolucao (beat 2 do inimigo OU acao do jogador);
//                        segura ~0.7s pro jogador LER o numero/log. Sai por tempo OU skip.
//   WaitingPlayerInput : vez do jogador (D9 "sua vez"); NAO auto-avanca - espera o menu.
//                        skip/tempo NAO pulam (so a acao do jogador retoma o ritmo).
//
// O TURNO DE INIMIGO TEM 2 BEATS (o lider no display: "a tela aparece com o ataque ja
// feito" - faltava anunciar ANTES de resolver):
//   ready -> begin_enemy_announce() [AnnouncingEnemy] -> (pausa) -> ready ->
//   resolve_one_turn() + begin_enemy_step() [WaitingDelay] -> (pausa) -> proximo.
// O turno do JOGADOR ja pausa no menu (WaitingPlayerInput), entao nao precisa do beat 1.
//
// ready_to_step() = true significa "a cena PODE dar o proximo passo agora" (anunciar OU
// resolver, conforme a cena rastreia). A cena chama begin_enemy_announce() (beat 1),
// begin_enemy_step() (apos resolver, beat 2 -> WaitingDelay), begin_player_turn() (vez do
// jogador), ou player_acted() (apos a acao do jogador -> WaitingDelay).
//
// Cross-ref: docs/design/mecanicas/battle-screen.md par.5.2 (D8-D12);
//            gus/app/screens/battle_scene.hpp (consumidor).

#ifndef GUS_APP_SCREENS_BATTLE_PACING_HPP
#define GUS_APP_SCREENS_BATTLE_PACING_HPP

namespace gus::app::screens {

// ABERTURA AGUARDANDO INPUT (decisao do lider 2026-06-25): o Intro NAO tem mais timer -
// fica PARADO exibindo "BATALHA!" + prompt ([Enter] Encarar / [Q] Resolver sem encarar)
// ate o jogador apertar. Resolve de vez "o intro passou antes de eu olhar" e garante que
// o jogador VE o BATALHA!. A constante antiga fica so como referencia historica (nao e
// mais usada como timer; o Intro espera input, igual ao turno do jogador).
inline constexpr float kPacingIntroSeconds = 0.9f;  // (legado: nao temporiza mais o Intro)

// Delay entre turnos (D8): segura o numero/log na tela pro jogador LER. ~0.8s.
inline constexpr float kPacingStepDelaySeconds = 0.8f;

// BEAT 1 do turno de inimigo (anuncio "Vez de <nome>"): segura ~0.7s ANTES de resolver,
// pro jogador ver de quem e a vez (e onde a animacao de ataque entra). Sai por tempo OU
// skip (modelo hibrido D8).
inline constexpr float kPacingAnnounceSeconds = 0.7f;

// Estado do ritmo.
enum class PacingState : int {
    Intro = 0,                // abertura parada (D10)
    AnnouncingEnemy = 1,      // BEAT 1 do turno de inimigo: anuncia, NADA resolveu ainda
    WaitingDelay = 2,         // pausa POS-resolucao (~0.8s) pro jogador ler
    WaitingPlayerInput = 3,   // vez do jogador (D9): espera o menu, nao auto-avanca
};

// Diretor de ritmo. POCO: timer + estado. NAO toca a FSM nem SDL.
class PacingDirector {
public:
    PacingDirector() = default;

    // Estado atual do ritmo.
    [[nodiscard]] PacingState state() const noexcept { return state_; }

    // true se a cena PODE resolver o proximo passo agora (delay/anuncio terminou). false
    // em Intro (espera o jogador ENCARAR), em WaitingPlayerInput (espera o menu), e
    // enquanto o tempo/skip nao liberou um estado timed.
    [[nodiscard]] bool ready_to_step() const noexcept;

    // true se e a vez do jogador (a casca mostra "sua vez" + opera o menu).
    [[nodiscard]] bool waiting_player_input() const noexcept {
        return state_ == PacingState::WaitingPlayerInput;
    }

    // true durante a ABERTURA parada (BATALHA! + prompt Encarar/Resolver). Espera input.
    [[nodiscard]] bool waiting_intro() const noexcept {
        return state_ == PacingState::Intro;
    }

    // Avanca o tempo (segundos) nos estados TIMED (anuncio/delay). No-op em Intro e
    // WaitingPlayerInput (ambos esperam INPUT, nao tempo): a abertura nunca auto-avanca.
    void tick(float dt_seconds) noexcept;

    // Tecla de ACELERAR (D8): zera o timer SO da pausa de leitura pos-resolucao
    // (WaitingDelay) -> libera o proximo passo. NAO afeta: Intro (espera Encarar),
    // WaitingPlayerInput (espera o menu), nem AnnouncingEnemy (o anuncio "Vez de <nome>"
    // SEMPRE toca seu tempo proprio - BUG do lider: pular o anuncio fazia o ataque sair
    // "colado/duplo"; o anuncio e o beat do windup da animacao e nao pode ser pulado).
    void skip() noexcept;

    // ENCARAR (decisao do lider 2026-06-25): o jogador apertou Enter na abertura. Sai do
    // Intro e LIBERA o 1o passo do combate (o 1o turno comeca a animar com os 2 beats).
    // No-op se nao esta no Intro.
    void begin_combat() noexcept;

    // BEAT 1 do turno de inimigo: a cena vai ANUNCIAR ("Vez de <nome>") sem resolver.
    // Entra em AnnouncingEnemy e segura ~kPacingAnnounceSeconds (ou skip). Quando libera
    // (ready_to_step), a cena resolve o golpe (beat 2) e chama begin_enemy_step().
    void begin_enemy_announce() noexcept;

    // true se esta no BEAT 1 (anunciando o turno de inimigo, nada resolvido ainda).
    [[nodiscard]] bool announcing_enemy() const noexcept {
        return state_ == PacingState::AnnouncingEnemy;
    }

    // BEAT 2: a cena acabou de RESOLVER o turno de inimigo: entra em WaitingDelay (~0.8s).
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
