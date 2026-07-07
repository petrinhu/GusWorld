// gus/domain/src/input/controls_remap_apply.cpp
//
// Implementacao do swap-on-conflict (tela Controles/M2). Ver header para o
// contrato completo. Travado por domain/tests/controls_remap_apply_test.cpp
// (TEST-FIRST).

#include "gus/domain/input/controls_remap_apply.hpp"

#include "gus/domain/input/action_registry.hpp"

namespace gus::domain::input {

namespace {

ActionBindings* find_mut(InputRemapConfig& cfg, const std::string& name) {
    for (auto& a : cfg.actions) {
        if (a.action_name == name) return &a;
    }
    return nullptr;
}

bool has_key(const ActionBindings& a, const KeyBinding& k) {
    for (const auto& existing : a.keys) {
        if (existing == k) return true;
    }
    return false;
}

}  // namespace

KeyRemapResult apply_key_remap(const InputRemapConfig& cfg, const std::string& action_name,
                                const KeyBinding& new_key) {
    KeyRemapResult result;
    result.config = cfg;

    if (ActionRegistry::get_by_name(action_name) == nullptr) {
        return result;  // action desconhecida: no-op defensivo
    }
    ActionBindings* target = find_mut(result.config, action_name);
    if (target == nullptr) {
        return result;  // config nao tem a action (defensivo - nao deveria acontecer)
    }
    if (has_key(*target, new_key)) {
        return result;  // no-op: jogador re-selecionou a tecla atual
    }

    const KeyBinding old_key_of_target =
        target->keys.empty() ? KeyBinding{} : target->keys.front();

    // Procura a PRIMEIRA outra action (ordem do ActionRegistry) que ja tem new_key.
    ActionBindings* conflicting = nullptr;
    for (const auto& def : ActionRegistry::actions()) {
        if (def.action_name == action_name) continue;
        ActionBindings* candidate = find_mut(result.config, def.action_name);
        if (candidate != nullptr && has_key(*candidate, new_key)) {
            conflicting = candidate;
            break;
        }
    }

    target->keys = {new_key};
    result.changed = true;

    if (conflicting != nullptr) {
        conflicting->keys = {old_key_of_target};
        result.swapped = true;
        result.swapped_with_action_name = conflicting->action_name;
        if (const ActionDefinition* def = ActionRegistry::get_by_name(conflicting->action_name);
            def != nullptr) {
            result.swapped_with_label_i18n_key = def->label_i18n_key;
        }
    }
    return result;
}

}  // namespace gus::domain::input
