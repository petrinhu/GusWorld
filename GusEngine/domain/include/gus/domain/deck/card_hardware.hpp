// gus/domain/deck/card_hardware.hpp
//
// Camada FISICA de uma carta possuida (origem ROM/EPROM/pirata, bateria CR2032,
// integridade/virus oculto, conector RSB) - CARDS-HARDWARE-ENGINE incremento 1
// (CARDS-HW-1, TODO.md). POCO puro, ZERO SDL/GL/RmlUi/IO (invariante de domain/,
// engine-design.md secao 2). Fonte de verdade: docs/design/mecanicas/
// cartas-spec-dados.md secao 5 (enums + CardPhysicalState + funcoes puras
// derivadas) + secao 6 (invariantes) + secao 7 (clone-falso).
//
// ATOM-1 (decomposicao atomica, herda o principio "peca+modulo novo" de ATOM-2,
// generaliza ADR-019): CardPhysicalState deixou de ser um struct monolitico e virou
// um AGREGADO C++20 que HERDA publicamente das 3 PECAS componiveis - cada uma no seu
// proprio modulo estreito:
//   - gus::domain::hardware::CardProvenance  (origin)                - card_provenance.hpp
//   - gus::domain::hardware::BatteryState    (cycles, deficit)       - battery_state.hpp
//   - gus::domain::infection::IntegrityState (infected/diagnosed/virus_kind) -
//     integrity_state.hpp
// A heranca de agregados preserva o ACESSO FLAT (p.origin, p.battery_recharge_cycles,
// p.is_infected, ...) e o save_serializer.cpp (que le/escreve esses campos direto)
// SEM EDICAO - as bases publicas expoem os campos como se fossem membros diretos.
// is_burned_out NAO pertence a nenhuma peca (nao faz parte de invariante correlato
// com outra peca) - fica direto no agregado. Este header tambem re-exporta em
// gus::domain::deck (via using-declaration, MESMA identidade de tipo) os nomes que
// migraram de lugar (CardOrigin, VirusKind, RsbConnector, HardwareClass, as
// constantes e as funcoes puras) - card_hardware_test.cpp continua compilando
// intocado. HardwareClass + hardware_class_of() moram em hardware/hardware_class.hpp
// (referenciam gus::domain::cards::CardTier, o lar pos-ATOM-2).
//
// ESCOPO deste incremento (secao 1 do doc-fonte, "fora do escopo" explicito):
//   - Rolagem de contaminacao na aquisicao (loot/compra/upload homebrew) - onda
//     FUTURA de deck_transactions.hpp (acquire-com-origem).
//   - Gate de bateria-vazia/is_burned_out impedindo cast em combate, propagacao de
//     worm, payload de virus (logic bomb/backdoor/etc) - onda FUTURA do
//     gameplay_engineer/executor techMagic.
//   - UI/RunaDex, gate "nao vaza is_infected antes de is_diagnosed" (contrato de
//     FRONTEIRA nomeado em secao 6 inv.3 do doc-fonte, NAO implementacao aqui).
//   - CardCollection::add_to_active(..., initial_physical) - extensao sugerida na
//     secao 11 do doc-fonte, NAO implementada nesta onda (CardCollection continua
//     intocada; quem cria a CardInstance seta physical diretamente por ora).
//
// Cross-ref: docs/design/mecanicas/cartas-spec-dados.md,
//            docs/design/mecanicas/cartas-numeros-proposta.md,
//            gus/domain/hardware/card_provenance.hpp,
//            gus/domain/hardware/battery_state.hpp,
//            gus/domain/hardware/hardware_class.hpp,
//            gus/domain/infection/integrity_state.hpp,
//            gus/domain/deck/card_hardware_constants.hpp (os NUMEROS, TABELA
//            config - battery_capacity_for/contamination_percent_for),
//            gus/domain/deck/deck_records.hpp (CardInstance::physical),
//            gus/domain/combat/combat_records.hpp (Card::mimics_special_id).

#ifndef GUS_DOMAIN_DECK_CARD_HARDWARE_HPP
#define GUS_DOMAIN_DECK_CARD_HARDWARE_HPP

#include <cstdint>

#include "gus/domain/hardware/battery_state.hpp"
#include "gus/domain/hardware/card_provenance.hpp"
#include "gus/domain/hardware/hardware_class.hpp"
#include "gus/domain/infection/integrity_state.hpp"

namespace gus::domain::deck {

// ---- Re-exporta os tipos/constantes/funcoes das pecas (LAR CANONICO: hardware/ e
// infection/, ATOM-1). Using-declaration preserva a IDENTIDADE de tipo - nenhum
// consumidor existente (card_hardware_test.cpp, save_v7_test.cpp, ...) precisa mudar.
using hardware::battery_charge_remaining;
using hardware::CardOrigin;
using hardware::connector_of;
using hardware::contamination_percent_for;
using hardware::hardware_class_of;
using hardware::HardwareClass;
using hardware::is_battery_dead;
using hardware::kBatteryDeadSohFloorPercent;
using hardware::kBatteryDegradationPerRechargeCyclePp;
using hardware::kCardOriginCount;
using hardware::kContaminationPercentTable;
using hardware::RsbConnector;
using hardware::state_of_health_percent;
using infection::kVirusKindCount;
using infection::kWormPropagationChancePercent;
using infection::VirusKind;

// Estado fisico MUTAVEL de UMA copia especifica de carta (CardInstance::physical,
// deck_records.hpp). Default = CardPhysicalState{} = "ROM original legitima,
// bateria cheia, sem infeccao" - o estado mais SEGURO/generoso, tanto para uma
// instancia nova quanto para um save V6 migrado (que nunca teve este campo) -
// cartas-spec-dados.md secao 5.2, "zero e seguro". AGREGADO das 3 pecas (heranca
// publica, ATOM-1) + is_burned_out (fora de qualquer peca, sem invariante correlato).
struct CardPhysicalState : public hardware::CardProvenance,
                            public hardware::BatteryState,
                            public infection::IntegrityState {
    // Chip permanentemente destruido (queima): resultado possivel da tentativa de
    // cura do Turing OU da arma-industrial (VirusKind::IndustrialWeapon). Carta com
    // is_burned_out=true fica inutilizavel em combate (gate futuro do
    // gameplay_engineer). Distinto de bateria morta (SoH<=piso): aquilo e
    // recuperavel trocando bateria; isto e PERMANENTE (exceto redencao scriptada).
    bool is_burned_out = false;

    [[nodiscard]] bool operator==(const CardPhysicalState&) const = default;

    // Fail-fast (cartas-spec-dados.md secao 6): virus_kind != None exige
    // is_infected==true (inv.1); is_diagnosed==true exige is_infected==true
    // (inv.1); origin/virus_kind dentro do dominio canonico (defesa em
    // profundidade, mesmo padrao de DifficultyLevel::validate() em
    // save_data.cpp/SaveData::validate() - um payload selado mas schema-
    // divergente nao e aceito silenciosamente). DELEGA as 2 pecas com invariante
    // (CardProvenance::validate() + IntegrityState::validate() - BatteryState nao
    // tem invariante proprio), preservando TIPO (std::invalid_argument) e MENSAGEM
    // de cada excecao. Lanca std::invalid_argument.
    void validate() const;
};

}  // namespace gus::domain::deck

#endif  // GUS_DOMAIN_DECK_CARD_HARDWARE_HPP
