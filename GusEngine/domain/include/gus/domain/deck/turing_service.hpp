// gus/domain/deck/turing_service.hpp
//
// Servico de diagnostico/cura do Turing (CARDS-HW-2 fatia A, "servico de cura/
// diagnostico do Turing", CARDS-HW-2A, TODO.md; docs/design/mecanicas/
// cartas-spec-logica.md secao 6, AttemptCure). Acao FORA DE COMBATE (bancada/
// oficina do Turing) - NAO faz parte da FSM de combate (combat.md secao 3),
// portanto NAO depende de combat_state_machine.hpp - so da PORTA de RNG
// (combat/random_source.hpp, IRandomSource), a mesma porta injetavel usada no
// combate, reaproveitada aqui pelo MESMO motivo: determinismo testavel sem RNG
// global no dominio (secao 11).
//
// FUNCOES LIVRES sem estado (mesmo estilo das demais funcoes puras de deck/ - NAO
// e um struct com instancia; e um SERVICO stateless sobre estado alheio).
//
// MODULO: deck/, NAO infection/ (CARDS-HW-2A refactor mecanico de eliminacao de
// ciclo). Este servico opera sobre o AGREGADO deck::CardPhysicalState (escreve
// is_burned_out, campo que so existe no agregado, nao na peca IntegrityState
// isolada) e sobre combat::IRandomSource (a porta canonica de RNG do dominio) - as
// DUAS ja sao dependencias normais de deck/ (card_hardware.hpp herda de
// infection::IntegrityState; card_collection.hpp inclui combat/combat_enums.hpp),
// entao morar aqui reusa arestas EXISTENTES em vez de abrir uma excecao pontual em
// infection/ (que volta a ser folha pura - gate de camadas sem excecao, ver
// integrity_state.hpp). cards::CardTier (guard de classe protegida) e permitido
// sem excecao (regra ja documentada em card_enums.hpp/card_integrity_ledger.hpp).
//
// Cross-ref: docs/design/mecanicas/cartas-spec-logica.md secao 6 (state machine +
//            AttemptCure; AMB-T1 RESOLVIDA - queima = SUCATA, NAO destruicao);
//            docs/design/mecanicas/cartas-numeros-proposta.md secao 6 (split 62/38%
//            - kTuringCureSuccessPercent/kTuringCureBurnoutPercent JA DEFINIDOS em
//            gus/domain/deck/card_hardware_constants.hpp; reaproveitados aqui, SEM
//            redefinicao - fonte unica);
//            gus/domain/infection/integrity_state.hpp (IntegrityState, a peca);
//            gus/domain/deck/card_hardware.hpp (CardPhysicalState, o agregado);
//            gus/domain/combat/random_source.hpp (IRandomSource);
//            gus/domain/combat/card_integrity_ledger.hpp (fatia 1 da mesma onda,
//            VIRUS EM COMBATE - trata da INFECCAO dentro do combate; este arquivo
//            trata da CURA fora do combate, guards e escopos SEPARADOS, zero
//            superposicao); TODO.md CARDS-HW-2A.

#ifndef GUS_DOMAIN_DECK_TURING_SERVICE_HPP
#define GUS_DOMAIN_DECK_TURING_SERVICE_HPP

#include <string_view>

#include "gus/domain/cards/card_enums.hpp"
#include "gus/domain/combat/random_source.hpp"
#include "gus/domain/deck/card_hardware.hpp"
#include "gus/domain/infection/integrity_state.hpp"

namespace gus::domain::deck {

// Resultado de diagnose(). Diagnosticar exige infeccao previa - nao existe
// "diagnostico" de carta limpa (guard simetrico ao inv.1 de
// IntegrityState::validate(): is_diagnosed==true exige is_infected==true).
enum class DiagnoseOutcome {
    Diagnosed,             // is_diagnosed setado (ou ja estava - idempotente)
    RejectedNotInfected,   // carta limpa; nada muda
};

// Diagnostica UMA instancia. Opera direto na PECA IntegrityState (nao precisa do
// agregado CardPhysicalState - diagnose() so mexe em is_diagnosed). Se
// state.is_infected==false, e no-op (RejectedNotInfected) - nunca viola o inv.1.
// Se ja estava diagnosticada, idempotente (Diagnosed de novo, sem duplo efeito
// observavel). Mutacao direta do parametro, zero I/O, zero RNG.
[[nodiscard]] DiagnoseOutcome diagnose(infection::IntegrityState& state) noexcept;

// Resultado de attempt_cure() - secao 6 do doc-fonte, state machine
// InfectedDiagnosed -> {Cured, ChipsetBurned} via roll(0.62).
enum class CureOutcome {
    Cured,                    // roll < 62%: limpa (volta ao estado seguro)
    Burned,                   // roll >= 62% (38%): sucata (AMB-T1, is_burned_out=true)
    RejectedNotDiagnosed,     // guard: nao cura no escuro (nada muta)
    RejectedProtectedTier,    // guard defensivo: Especial/Super (nada muta)
};

// Tenta curar uma copia FISICA de carta (precisa do AGREGADO, nao so da peca - a
// queima escreve is_burned_out, que mora em CardPhysicalState). Guards NESTA ordem
// (secao 6 pseudocodigo):
//   1. tier == Especial || tier == Super -> RejectedProtectedTier. Classe protegida
//      tem 0% de risco de infeccao geral (secao 5.1) - defensivo, nao deveria
//      disparar em jogo normal; a excecao Sterling/Faraday e evento narrativo
//      scriptado a parte, fora deste sistema geral.
//   2. !physical.is_diagnosed -> RejectedNotDiagnosed. Nao cura no escuro.
// Sorteio (SO se passar os 2 guards, exatamente 1 draw de rng.next(100)):
//   roll < kTuringCureSuccessPercent (62)  -> Cured: is_infected/virus_kind/
//                                             is_diagnosed voltam ao default seguro.
//   roll >= 62 (38%)                        -> Burned: is_burned_out=true, SUCATA
//                                             (AMB-T1 RESOLVIDA pelo lider: a carta
//                                             PERMANECE na colecao, inutilizavel,
//                                             vende no ferro-velho - NAO e destruida
//                                             nem removida). is_infected/virus_kind
//                                             ficam como estavam (a carta so nao
//                                             roda mais - gate de gameplay futuro).
// Pos-estado SEMPRE passa physical.validate() nos 4 desfechos (inclusive os 2 guards
// que nao mutam nada - o estado de entrada ja era valido por construcao).
[[nodiscard]] CureOutcome attempt_cure(CardPhysicalState& physical,
                                        cards::CardTier tier,
                                        combat::IRandomSource& rng);

// ---- Log diegetico (regra "todo efeito loga", feedback_todo_efeito_loga_terminal)
// Mapeamento PURO outcome -> chave i18n (UPPER_SNAKE_CASE, gus/domain/i18n/
// md_translation_loader.hpp) SEM I/O - a emissao real no terminal/UI e do
// CHAMADOR (app/, onda futura), nao deste servico. As chaves ainda NAO existem em
// pt_br.md/en_intl.md (conteudo real e onda futura do technical-writer/
// narrative-writer; esta fatia so entrega o MAPEAMENTO puro e testavel).
[[nodiscard]] std::string_view translation_key_for(DiagnoseOutcome outcome);
[[nodiscard]] std::string_view translation_key_for(CureOutcome outcome);

}  // namespace gus::domain::deck

#endif  // GUS_DOMAIN_DECK_TURING_SERVICE_HPP
