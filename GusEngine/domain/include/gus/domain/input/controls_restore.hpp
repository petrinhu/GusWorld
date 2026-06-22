// gus/domain/input/controls_restore.hpp
//
// Restauracao de controles (a partir do backup embutido no save) + fabrica do
// esquema de FABRICA (default) (ADR-007 item 5). POCO puro, ZERO Qt, ZERO disco.
//
//   - default_controls(): a fonte UNICA do esquema de fabrica (WASD + setas + stick
//     etc.), cobrindo as 37 actions do ActionRegistry. Usado pelo migrator V3->V4 e
//     como fallback quando nao ha save nenhum (jogo novo / arquivo ausente). Pura,
//     deterministica.
//   - restore_from_save(save): devolve save.input_remap_backup. A reescrita do
//     arquivo no disco (I/O) e recomputo do hash sao PLATFORM, fora daqui.
//
// Regra "se nao restaurar, proximo save grava o esquema novo" (ADR-007 item 5) e
// consequencia natural: o save sempre fotografa o estado vigente; nenhuma logica
// especial mora aqui.
//
// Cross-ref: docs/tech/adr/ADR-007-controls-json-hash128-save-v4.md (item 5),
//            gus/domain/input/input_binding.hpp, gus/domain/save/save_data.hpp.

#ifndef GUS_DOMAIN_INPUT_CONTROLS_RESTORE_HPP
#define GUS_DOMAIN_INPUT_CONTROLS_RESTORE_HPP

#include "gus/domain/input/input_binding.hpp"

namespace gus::domain::save {
struct SaveData;  // fwd: domain->domain, dentro da mesma camada
}

namespace gus::domain::input {

// Esquema de controles de FABRICA (fonte unica). Cobre as 37 actions canonicas, com
// ao menos um binding por action; config_version = 1. Pura, deterministica.
[[nodiscard]] InputRemapConfig default_controls();

// Restaura os controles a partir do backup embutido no save (mais recente, escolhido
// pela camada de I/O). Pura: devolve save.input_remap_backup.
[[nodiscard]] InputRemapConfig restore_from_save(const gus::domain::save::SaveData& most_recent);

}  // namespace gus::domain::input

#endif  // GUS_DOMAIN_INPUT_CONTROLS_RESTORE_HPP
