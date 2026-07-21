// gus/domain/deck/contamination_service.hpp
//
// Rolagem de contaminacao por virus NA AQUISICAO de uma carta - a fatia que ACENDE o
// sistema de virus (ate aqui so ligado por hook de teste; CARDS-HARDWARE-ENGINE fatia
// 3 incremento B, "CARDS-HW-3B", TODO.md; docs/design/mecanicas/cartas-spec-logica.md
// secao 5.1, pseudocodigo on_card_acquired/AMB-07 RESOLVIDA; cartas-numeros-
// proposta.md secao 3 (risco % por classe) e 3a (payload ponderado por classe). POCO
// puro, ZERO SDL/GL/RmlUi/IO (invariante de domain/, engine-design.md secao 2).
//
// FUNCOES LIVRES sem estado (mesmo estilo de turing_service.hpp - servico stateless
// sobre estado alheio, NAO um agregado com instancia propria).
//
// MODULO: deck/, NAO hardware/ nem infection/ (MESMO racional de turing_service.hpp,
// CARDS-HW-2A): este servico precisa de combat::IRandomSource, aresta que hardware/ e
// infection/ NAO tem por design/gate de camadas (so cards:: e permitido la). deck/ ja
// cruza as duas arestas naturalmente (card_hardware.hpp herda de
// hardware::CardProvenance/infection::IntegrityState; card_collection.hpp inclui
// combat/combat_enums.hpp; turing_service.hpp ja usa combat::IRandomSource aqui do
// mesmo jeito) - morar aqui reusa arestas EXISTENTES, sem abrir excecao pontual em
// hardware/infection/.
//
// CHAMADOR: gus/domain/deck/deck_transactions.hpp (acquire()/craft()), no MESMO ponto
// onde a origem fisica ja foi resolvida (CARDS-HW-3A) - "no MESMO ponto onde a origem
// e setada" (brief da fatia). Esta funcao NAO seta origin, so LE o que o chamador ja
// setou pra classificar.
//
// ESCOPO desta fatia (fora do escopo, explicito): NAO existe ainda canal de mercado
// negro/loja pirata (CardOrigin::PirateClone real) nem clone-falso (mimics_special de
// verdade) - so acquire() (sempre ComumOriginal, risco 1%) e craft() (sempre
// HomebrewEprom, risco 55%) chamam este servico hoje. A tabela de pesos/risco e
// implementada COMPLETA (as 5 classes) pra estar pronta quando o mercado existir - os
// testes deste arquivo cobrem as 5, mesmo sem um canal de producao pra 3 delas ainda.
//
// Cross-ref: docs/design/mecanicas/cartas-spec-logica.md secao 5.1 (formula + AMB-07);
//            docs/design/mecanicas/cartas-numeros-proposta.md secao 3 (risco % por
//            classe) e 3a (payload ponderado por classe, AMB-07 RESOLVIDA);
//            gus/domain/hardware/hardware_class.hpp (HardwareClass/hardware_class_of/
//            contamination_percent_for); gus/domain/deck/deck_transactions.hpp
//            (chamador); gus/domain/deck/turing_service.hpp (mesmo estilo - cura/
//            diagnostico, a PONTA OPOSTA do ciclo de vida da infeccao);
//            gus/domain/combat/urandom_algorithm.hpp (weighted_pick_urandom_faixa -
//            MESMO padrao de weighted-pick por peso relativo, reaplicado aqui pra
//            VirusKind).

#ifndef GUS_DOMAIN_DECK_CONTAMINATION_SERVICE_HPP
#define GUS_DOMAIN_DECK_CONTAMINATION_SERVICE_HPP

#include <string_view>

#include "gus/domain/cards/card_enums.hpp"
#include "gus/domain/combat/random_source.hpp"
#include "gus/domain/deck/card_hardware.hpp"

namespace gus::domain::deck {

// Desfecho de roll_contamination_on_acquisition() - secao 5.1.
enum class ContaminationRollOutcome {
    Infected,              // draw < risco% da HardwareClass - is_infected+virus_kind setados
    Clean,                 // draw >= risco% (ou risco==0%) - nada muda
    SkippedProtectedTier,  // guard defensivo: Especial/Super, NUNCA rola (nada muda) - a
                            // propria tabela ja da 0% pra EspecialSelada (secao 5.1); este
                            // guard e defesa em profundidade EXTRA (mesmo espirito do
                            // guard 1 de turing_service::attempt_cure), nao deveria
                            // disparar em jogo normal.
};

// Rola contaminacao 1x na aquisicao de UMA instancia (secao 5.1, pseudocodigo
// on_card_acquired). `physical.origin` PRECISA ja estar resolvido pelo CHAMADOR
// (deck_transactions.hpp acquire()/craft(), CARDS-HW-3A) - esta funcao NAO seta
// origin, so LE ele pra classificar. `catalog_tier` vem do catalogo (TierLookup do
// chamador); `mimics_special` vem de Card::mimics_special_id.has_value() do catalogo
// (SEMPRE false hoje nos dois canais reais - o canal pirata/clone-falso ainda nao
// existe, CARDS-HW-3B escopo).
//
// Guards NESTA ordem:
//   1. catalog_tier == Especial || Super -> SkippedProtectedTier (nada muta).
//   2. classe = hardware_class_of(catalog_tier, physical.origin, mimics_special);
//      risco = contamination_percent_for(classe) (0% pra EspecialSelada tambem cai
//      aqui naturalmente - guard 1 e so defesa redundante).
// Sorteio (1 draw de rng.next(100), SO se passou os guards):
//   draw < risco  -> Infected: physical.is_infected=true,
//                     physical.virus_kind = pick_weighted_payload(classe, rng)
//                     (2o draw, SO neste ramo, secao 3a/AMB-07).
//   draw >= risco -> Clean: nada muta.
//
// Pos-estado sempre passa physical.validate() nos 3 desfechos (inclusive os que nao
// mutam nada - o estado de entrada ja era valido por construcao).
[[nodiscard]] ContaminationRollOutcome roll_contamination_on_acquisition(
    CardPhysicalState& physical, cards::CardTier catalog_tier, bool mimics_special,
    combat::IRandomSource& rng);

// Sorteia 1 VirusKind por peso relativo, DADO QUE JA INFECTOU (secao 3a, AMB-07
// RESOLVIDA pelo lider 2026-07-20: ponderado por classe de origem, "pirata puxa
// payload pior" - quanto mais suja a proveniencia, mais o payload tende pro lado
// destrutivo). SO os 4 tipos disparaveis hoje - Backdoor/Worm/LogicBomb/ZipBomb
// (Adware e opt-in, fora da rolagem; FalseBenign bloqueado ate a feat
// EFEITOS-ADIADOS-OCULTOS existir; IndustrialWeapon e evento scriptado da
// Sterling/Dante, fora deste sistema geral) - NUNCA None/FalseBenign/AdwareSterling/
// IndustrialWeapon. 1 UNICO draw de rng.next(total_dos_pesos) - determinismo
// canonico (secao 11), MESMO padrao de weighted_pick_urandom_faixa
// (combat/urandom_algorithm.hpp).
//
// `hardware_class` == HardwareClass::EspecialSelada nunca deveria chegar aqui (a
// classe protegida e guardada upstream por roll_contamination_on_acquisition, guard
// 1) - fallback defensivo devolve VirusKind::Backdoor (o payload mais brando) se isso
// acontecer mesmo assim, nunca comportamento indefinido.
[[nodiscard]] infection::VirusKind pick_weighted_payload(HardwareClass hardware_class,
                                                          combat::IRandomSource& rng);

// ---- Log diegetico AMBIGUO (regra "todo efeito loga",
// feedback_todo_efeito_loga_terminal + decisao do lider CARDS-HW-3B: a infeccao NAO
// se revela na hora da aquisicao, so o Turing confirma - secao 6) - mapeamento PURO
// outcome -> chave i18n, SEM I/O (mesmo padrao de turing_service::translation_key_for
// - as chaves ainda NAO existem em pt_br.md/en_intl.md, conteudo real e onda futura
// do technical-writer/narrative-writer). Infected loga um AVISO AMBIGUO ("checksum
// estranho..."), NUNCA confirma "infectada" (a certeza fica reservada ao diagnostico
// do Turing); Clean/SkippedProtectedTier logam a chave "normal" (nada suspeito).
[[nodiscard]] std::string_view translation_key_for(ContaminationRollOutcome outcome);

}  // namespace gus::domain::deck

#endif  // GUS_DOMAIN_DECK_CONTAMINATION_SERVICE_HPP
