// SPDX-License-Identifier: Apache-2.0
// gus/domain/combat/healer_card.hpp
//
// Motor PURO da carta de cura da Jaci "Sylvesse-Religação" (cartas-comuns-statlines.md,
// Bioquímico/Utilidade/CURA) - TODO.md item CURANDEIRA-LIMITE-USOS (W2). Resolve o USO de
// UMA cópia da carta contra a bateria FÍSICA daquela cópia (hardware::BatteryState), nos
// termos fixados pelo líder em `docs/design/mecanicas/cartas-hardware-pirataria-energia.md`
// §3 ("PRINCÍPIO CANÔNICO", 2026-08-08): "todo mecanismo de uso de carta depende da
// originalidade da carta E da degradação da bateria, as duas variáveis JUNTAS" - o exemplo
// CANÔNICO citado ali é EXATAMENTE esta carta (cura fixa+dreno fixo pra original, cura
// RNG+dreno RNG maior pra pirata, falha total proporcional à degradação da bateria).
//
// POR QUE NÃO É UM EffectKind NOVO (ADR-016): a carta de cura é CardTier::Comum (não é
// uma das ~20 especiais nomeadas do CARD-ENGINE-MANIFESTO) - o executor techMagic (gus/
// domain/combat/techmagic.hpp) só roda pra ESPECIAL/SUPER (card_records.hpp: "ADR-016
// continua so ESPECIAL/SUPER"; docs/design/mecanicas/cartas-spec-logica.md §1: "COMUM
// resolve pelo caminho record-base... vírus/bateria têm que agir em QUALQUER tier de
// carta... por isso embrulha o resolve_use_card inteiro (CardHardwareLayer), em vez de
// virar mais um EffectKind"). Este header é o pedaço PURO/TESTÁVEL desse embrulho maior
// (o CardHardwareLayer completo - gate pré-cast por mana/bateria vazia, wiring real em
// resolve_use_card, escolha de QUAL CardInstance específica está sendo jogada - é
// escopo FUTURO, fora desta fatia; ver nota "ESCOPO EXPLICITAMENTE FORA" abaixo e o
// relato da onda no TODO.md/commit).
//
// GATE DE CAMADAS: combat/ pode incluir hardware/ e save/ (nenhum dos dois inclui
// combat/, gate auditado no CI) mas NUNCA deck/ (deck/ inclui combat/, incluir o
// inverso criaria ciclo deck->combat->deck - mesmo racional documentado em
// gus/domain/combat/card_collection_snapshot.hpp). Por isso `battery_capacity` chega
// como parâmetro JÁ RESOLVIDO pelo chamador (via gus::domain::deck::battery_capacity_for,
// card_hardware_constants.hpp) - mesmo padrão de hardware::battery_charge_remaining
// ("Recebe a capacidade JA RESOLVIDA... esta funcao NAO conhece dificuldade/classe").
//
// TABELA (lei do átomo, ADR-019/020 - mesmo padrão de kBatteryCapacityTable/
// kEnemyDifficultyMultiplierTable): 2 HardwareClass suportadas x 4 DifficultyLevel.
// Só ComumOriginal e PirataComum têm número hoje (os dois únicos citados pelo líder na
// especificação desta fatia). PirataEspecialFalso NÃO se aplica: essa classe é
// EXCLUSIVA de clone-falso de carta ESPECIAL (Card::mimics_special_id preenchido,
// hardware_class.hpp) - Sylvesse-Religação é Comum, nunca teria essa classe de
// hardware. HomebrewEprom/EspecialSelada não foram especificados pelo líder pra esta
// carta - `healer_card_row_for` LANÇA std::logic_error pra eles (fail-fast: nenhum
// número foi inventado, ver regra "não decida números sozinho").
//
// Medio é DEFINITIVO (valores da especificação do líder); Facil/Dificil/Hardcore
// replicam o MESMO valor por enquanto, marcados //PLAYTEST (mesmíssimo padrão de
// enemy_difficulty_constants.hpp/DIFICULDADE-TABELA-DADO - "not calibrated yet" != "not
// neutral", os dois "iguais" tem motivo DIFERENTE).
//
// ESCOPO EXPLICITAMENTE FORA desta fatia (fiel ao card_hardware.hpp: "Gate de bateria-
// vazia/is_burned_out impedindo cast em combate... onda FUTURA do gameplay_engineer/
// executor techMagic"):
//   - QUAL CardInstance específica (com seu BatteryState PERSISTENTE, através de
//     batalhas, até o jogador trocar a bateria) está sendo jogada num resolve_use_card
//     real - a FSM hoje só enxerga o Card de catálogo (card_registry), nunca
//     CardInstance/CardCollection (gate de camadas, card_collection_snapshot.hpp). Este
//     header não decide OFF-battle vs ON-battle persistence da bateria - fica com o
//     chamador (app/ ou uma peça de wiring futura), que já é dono do BatteryState real
//     via deck/CardCollection.
//   - Gate de "carta esgotada nem aparece jogável" (ação de UI/seleção) - fora de
//     resolve_healer_card_use, que assume o chamador só invoca quando a carga
//     remanescente > 0 (ver doc-comment de resolve_healer_card_use abaixo).
//
// Cross-ref: docs/design/mecanicas/cartas-hardware-pirataria-energia.md §3/§5;
//            docs/design/mecanicas/cartas-comuns-statlines.md (Sylvesse-Religação);
//            docs/design/mecanicas/cartas-spec-logica.md §1 (CardHardwareLayer);
//            gus/domain/hardware/battery_state.hpp; gus/domain/deck/
//            card_hardware_constants.hpp (battery_capacity_for); TODO.md
//            CURANDEIRA-LIMITE-USOS.

#ifndef GUS_DOMAIN_COMBAT_HEALER_CARD_HPP
#define GUS_DOMAIN_COMBAT_HEALER_CARD_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/random_source.hpp"
#include "gus/domain/hardware/battery_state.hpp"
#include "gus/domain/hardware/hardware_class.hpp"
#include "gus/domain/save/save_data.hpp"

namespace gus::domain::combat {

// Faixa de cura + faixa de dreno (em pontos percentuais da carga ATUAL remanescente, não
// da capacidade) para 1 célula (HardwareClass suportada x DifficultyLevel). `*_min ==
// *_max` = valor FIXO (0 consumo de RNG pra esse eixo, ver resolve_healer_card_use).
struct HealerCardTuning {
    int heal_min = 0;
    int heal_max = 0;
    int drain_percent_min = 0;
    int drain_percent_max = 0;
};

// ---- Tabela: 2 linhas (ComumOriginal=0, PirataComum=1) x 4 DifficultyLevel ----------
//
// Valores da especificação do líder (2026-08-08): original cura 12 HP fixo, drena 50%
// fixo da carga atual; pirata cura 9-11 HP (RNG uniforme), drena 55-70% (RNG uniforme)
// da carga atual. Médio DEFINITIVO; Facil/Dificil/Hardcore = MESMO valor, //PLAYTEST
// (placeholder de segurança, não calibrado - mesmo padrão de
// enemy_difficulty_constants.hpp).
inline constexpr std::array<std::array<HealerCardTuning, 4>, 2> kHealerCardTuningTable = {{
    // ComumOriginal: Facil //PLAYTEST | Medio DEFINITIVO | Dificil //PLAYTEST | Hardcore //PLAYTEST
    {{
        HealerCardTuning{12, 12, 50, 50},
        HealerCardTuning{12, 12, 50, 50},
        HealerCardTuning{12, 12, 50, 50},
        HealerCardTuning{12, 12, 50, 50},
    }},
    // PirataComum: Facil //PLAYTEST | Medio DEFINITIVO | Dificil //PLAYTEST | Hardcore //PLAYTEST
    {{
        HealerCardTuning{9, 11, 55, 70},
        HealerCardTuning{9, 11, 55, 70},
        HealerCardTuning{9, 11, 55, 70},
        HealerCardTuning{9, 11, 55, 70},
    }},
}};

// Indice de linha em kHealerCardTuningTable para `hw`. Lanca std::logic_error para
// qualquer HardwareClass fora de {ComumOriginal, PirataComum} - nenhum numero foi
// inventado pra Homebrew/PirataEspecialFalso/EspecialSelada nesta carta (fail-fast,
// mesmo espirito de "EffectKind sem handler" em techmagic.hpp: melhor travar alto e
// visivel do que aplicar um tuning que ninguem decidiu).
[[nodiscard]] inline std::size_t healer_card_row_for(hardware::HardwareClass hw) {
    switch (hw) {
        case hardware::HardwareClass::ComumOriginal:
            return 0;
        case hardware::HardwareClass::PirataComum:
            return 1;
        case hardware::HardwareClass::HomebrewEprom:
        case hardware::HardwareClass::PirataEspecialFalso:
        case hardware::HardwareClass::EspecialSelada:
            break;
    }
    throw std::logic_error(
        "healer_card_row_for: HardwareClass sem tuning de cura definido pelo lider "
        "(so ComumOriginal/PirataComum tem numero nesta fatia, CURANDEIRA-LIMITE-USOS) - "
        "pare e reporte em vez de inventar.");
}

// Tuning (cura + dreno) para (hw, difficulty). Table lookup puro, mesmo padrao de
// gus::domain::templates::difficulty_multiplier_for/gus::domain::deck::
// battery_capacity_for.
[[nodiscard]] inline const HealerCardTuning& healer_card_tuning_for(
    hardware::HardwareClass hw, gus::domain::save::DifficultyLevel difficulty) {
    return kHealerCardTuningTable[healer_card_row_for(hw)][static_cast<std::size_t>(
        difficulty)];
}

// Resultado de 1 uso da carta de cura.
struct HealerCardUseResult {
    // true = a bateria degradada fez a cura falhar por completo (SoH-based, ver
    // resolve_healer_card_use). Nem cura nem dreno acontecem quando failed == true.
    bool failed = false;
    // HP efetivamente curado (0 se failed). Ja aplicado em `target` (CombatActor::heal,
    // clamp em max_hp) quando esta funcao retorna.
    int healed_amount = 0;
    // Pontos de carga debitados de `battery.battery_charge_deficit` (0 se failed).
    int drain_applied = 0;
    // Chance de falha (%) que foi rolada (100 - SoH). -1 quando SoH==100 (nunca falha,
    // 0 consumo de RNG pra este eixo - mesmo padrao de RepeatLastAction/Mandelbrot
    // magnitude==0, techmagic.hpp).
    int roll_fail_percent = -1;
};

// Resolve 1 uso de Sylvesse-Religação/"carta de cura" contra a bateria FISICA `battery`
// da copia especifica sendo jogada (chamador e dono do BatteryState real - ver nota de
// escopo no topo do header).
//
// ORDEM de consumo de RNG (determinismo, TESTE-REI - mesmo racional de
// techmagic.cpp/RepeatLastAction):
//   1. Chance de falha por degradacao (hardware::state_of_health_percent(battery)):
//      fail_percent = 100 - SoH%. SoH==100 (bateria nova) NUNCA rola (0 consumo,
//      fail_percent fica -1 no resultado) - deterministico "nunca falha" por
//      construcao, nao por sorte. SoH<100 consome 1 rng.next(100); falha se
//      roll < fail_percent.
//      NOTA (is_battery_dead): hardware::is_battery_dead() governa SO elegibilidade de
//      RECARGA-EM-LUGAR ("nao serve mais pra recarregar em-lugar", battery_state.hpp) -
//      e ORTOGONAL a conseguir USAR a carga que ainda resta. Por isso esta funcao NAO
//      trata is_battery_dead(battery) como gate separado: o proprio SoH baixo ja
//      modela a falta de confiabilidade via fail_percent alto (SoH no piso de descarte,
//      21%, da 79% de chance de falha - severo, mas nao automatico; SO SoH==0 e 100%
//      automatico). Se o lider quiser is_battery_dead() como veto TOTAL adicional
//      (mesmo com SoH>21% residual chance de sucesso), isso e uma decisao de design
//      NOVA, fora do que foi especificado - reportada, nao inventada aqui.
//   2. Falhou -> retorna (healed_amount=0, drain_applied=0), NENHUM outro rng
//      consumido, `target`/`battery` intocados.
//   3. Nao falhou -> cura: tuning.heal_min==heal_max e FIXO (0 consumo); senao 1
//      rng.next(heal_max-heal_min+1) uniforme em [heal_min, heal_max].
//   4. Dreno: mesmo padrao FIXO-ou-1-rng.next(...) pro percentual; aplicado sobre a
//      carga ATUAL remanescente (hardware::battery_charge_remaining(battery,
//      battery_capacity) ANTES desta mutacao - "nao da capacidade maxima", spec do
//      lider), floor (trunca pra baixo - nunca super-drena). Escrito em
//      battery.battery_charge_deficit (a funcao MUTA `battery` in-place); a propria
//      semantica de deficit (>= capacity == 0 remanescente, battery_state.hpp) ja
//      garante "nunca fica negativa" sem clamp extra aqui.
//   5. Cura aplicada em `target` via CombatActor::heal (clamp em max_hp de CombatActor).
//
// PRE-CONDICAO (bug de call site, nao gate de gameplay - ver nota de escopo no topo):
// lanca std::logic_error se hardware::battery_charge_remaining(battery,
// battery_capacity) == 0 no MOMENTO da chamada - "carta esgotada fica inerte ate
// trocar" (cartas-hardware-pirataria-energia.md §5); o CHAMADOR (o futuro gate de
// selecao de acao) deve impedir que esta carta seja jogavel com carga zerada ANTES de
// invocar esta funcao, mesmo padrao de CombatActor::spend_ap/spend_mana (lancam se
// insuficiente, nao fazem no-op silencioso).
//
// `log` (opcional, nullptr desativa - mesmo padrao de TechMagicContext::log): recebe 1
// CombatLogEntry descrevendo o resultado (regra "todo efeito loga no terminal").
[[nodiscard]] HealerCardUseResult resolve_healer_card_use(
    hardware::HardwareClass hardware_class, gus::domain::save::DifficultyLevel difficulty,
    hardware::BatteryState& battery, std::uint32_t battery_capacity, CombatActor& caster,
    CombatActor& target, IRandomSource& rng, std::vector<CombatLogEntry>* log = nullptr);

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_HEALER_CARD_HPP
