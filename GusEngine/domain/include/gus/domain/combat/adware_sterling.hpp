// gus/domain/combat/adware_sterling.hpp
//
// Sequencia de propaganda da carta Adware Sterling (CARDS-HW-3C; docs/design/mecanicas/
// cartas-spec-logica.md secao 9): decisao PURA de "mostrar anuncio completo" vs "pular",
// consumida pelo gate pre-cast de resolve_use_card (secao 1 do doc-fonte: intercepta ANTES
// do debito de recurso). E opt-in consciente do jogador (o jogador aceitou a carta gratis
// sabendo, cartas-spec-logica.md secao 4.1 linha Adware) - DIFERENTE de VirusKind::
// AdwareSterling (gus/domain/infection/integrity_state.hpp), que e o valor do enum de
// infeccao HOSTIL e fica FORA da rolagem de contaminacao (Adware nunca e sorteado como
// payload; e um flag de CATALOGO, Card::has_adware). POCO puro, ZERO SDL/GL/RmlUi/IO
// (invariante de domain/, engine-design.md secao 2).
//
// AMB-11 resolvida pelo lider (2026-07-20/21): exposicoes 1, 2 e 3 NESTA instancia de
// combate SEMPRE mostram o anuncio completo (0 consumo de RNG); da 4a exposicao em diante,
// kAdwareShowChanceAfter3% mostra / resto pula (1 consumo de IRandomSource::next(100)).
//
// Escopo do contador (decisao de engenharia desta fatia - AMB-11 nao fechou "sessao vs
// combate"): vive por-INSTANCIA de CombatStateMachine, reseta a cada combate novo (mesmo
// racional de specials_cast_/token_refund_used_, "escopo = vida desta CombatStateMachine").
// Classe standalone/injetavel de proposito: se o lider quiser estender pra um contador
// cross-combate (sessao de jogo inteira, leitura literal do doc-fonte), basta a camada
// app/ possuir a instancia de AdwareExposureTracker num escopo maior (servico de sessao
// futuro, ainda nao existente) e injeta-la - nada aqui muda. Ver CombatStateMachine::
// adware_tracker_.
//
// Timing (5s) e o botao-X so liberar apos o minimo: sao DADOS pra UI, NAO logica deste
// header (kAdwareMinWatchSeconds) - a FSM nao conta tempo real; quem conta e mostra/
// habilita o X e a camada glintfx (fatia de UI FUTURA, fora deste slice). Este header NUNCA
// "espera" - roll_exposure decide e retorna na hora (o motor e sincrono/turn-based).
//
// Cross-ref: docs/design/mecanicas/cartas-spec-logica.md secao 1/4.1/9; gus/domain/cards/
//            card_records.hpp (Card::has_adware); combat_state_machine.cpp::resolve_use_card.

#ifndef GUS_DOMAIN_COMBAT_ADWARE_STERLING_HPP
#define GUS_DOMAIN_COMBAT_ADWARE_STERLING_HPP

#include "gus/domain/combat/random_source.hpp"

namespace gus::domain::combat {

// Numero de exposicoes (1-based) que SEMPRE mostram o anuncio completo, deterministico,
// ZERO consumo de RNG. //PLAYTEST.
inline constexpr int kAdwareAlwaysShowThreshold = 3;

// Chance% (0-100) de mostrar o anuncio completo a partir da (kAdwareAlwaysShowThreshold+1)a
// exposicao em diante (roll < isto => ShowFull). //PLAYTEST.
inline constexpr int kAdwareShowChanceAfter3 = 70;

// Segundos minimos de espera antes do botao-X liberar (secao 9). DADO puro pra UI - este
// header nao conta tempo real, so expoe a constante. //PLAYTEST.
inline constexpr int kAdwareMinWatchSeconds = 5;

// Desfecho da avaliacao de UMA exposicao ao anuncio.
enum class AdwareOutcome {
    ShowFull,  // exibe o anuncio completo (UI futura aguarda kAdwareMinWatchSeconds antes
               // de liberar o X).
    Skip,      // pula direto, sem espera (so possivel da 4a exposicao em diante).
};

// Resultado de 1 avaliacao (roll_exposure), consumido pelo caller (resolve_use_card) pra
// logar + (fatia futura de UI) sinalizar o overlay.
struct AdwareGateResult {
    AdwareOutcome outcome = AdwareOutcome::ShowFull;
    int exposure_index = 0;  // 1-based, nesta instancia de tracker.

    [[nodiscard]] bool operator==(const AdwareGateResult&) const = default;
};

// Contador de exposicoes + decisor da sequencia de propaganda. Estado de RUNTIME puro,
// NUNCA persistido/serializado (nao faz parte de CombatResult nem do save) - ver nota de
// escopo no topo do arquivo.
class AdwareExposureTracker {
public:
    // Incrementa o contador e decide o desfecho (secao 9, AMB-11). Exposicoes 1..
    // kAdwareAlwaysShowThreshold: sempre ShowFull, 0 consumo de RNG. Dali em diante: 1
    // consumo de IRandomSource::next(100) (roll < kAdwareShowChanceAfter3 => ShowFull).
    // Deliberadamente NAO recebe CombatActor/Card - a decisao e 100% desacoplada de
    // recurso/mana (o debito so acontece DEPOIS, no caller resolve_use_card).
    [[nodiscard]] AdwareGateResult roll_exposure(IRandomSource& rng);

    // Numero de exposicoes ja avaliadas nesta instancia (observabilidade/log/teste).
    [[nodiscard]] int exposure_count() const noexcept { return exposure_count_; }

private:
    int exposure_count_ = 0;
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_ADWARE_STERLING_HPP
