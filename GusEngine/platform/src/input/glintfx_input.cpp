// SPDX-License-Identifier: GPL-3.0-or-later
// gus/platform/src/input/glintfx_input.cpp
//
// Implementacao do GlintfxInput. Ver header. Travado por
// platform/tests/glintfx_input_test.cpp (TEST-FIRST) - provider FALSO, headless.

#include "gus/platform/input/glintfx_input.hpp"

#include <optional>
#include <utility>  // std::move

#include "gus/domain/input/controls_restore.hpp"
#include "gus/platform/input/key_translation_glintfx.hpp"

namespace gus::platform::input {

GlintfxInput::GlintfxInput(KeyStateProvider provider)
    : GlintfxInput(gus::domain::input::default_controls(), std::move(provider)) {}

GlintfxInput::GlintfxInput(gus::domain::input::InputRemapConfig config, KeyStateProvider provider)
    : config_(std::move(config)), mapper_(config_), provider_(std::move(provider)) {
    rebuild_watched_keys();
}

void GlintfxInput::rebuild_watched_keys() {
    watched_keys_.clear();
    for (const auto& action : config_.actions) {
        for (const auto& kb : action.keys) {
            const std::optional<glintfx::Key> key = godot_keycode_to_glintfx_key(kb.keycode);
            if (!key.has_value()) {
                // Sem correspondente conhecido (sentinela 0 incluso, ou uma das 4
                // exclusoes documentadas em key_translation_glintfx.hpp) - nunca
                // polado, MESMA semantica de sentinela que InputMapper::press ja usa.
                continue;
            }
            // SEM dedup de proposito: se 2 acoes diferentes repetirem o MESMO
            // keycode (ex.: default_controls() tem diary_open='J' e um remap de
            // move_forward pra 'J' colide com ele - exercitado pelos testes de
            // config remapeado/set_controls), watched_keys_ ganha 2 entradas pro
            // MESMO par (key,keycode). Isto e COMPORTAMENTO-INVARIANTE (poll()
            // chama o provider e sintetiza a MESMA transicao 2x, mapper_.press/
            // release ja sao idempotentes por conjunto - InputMapper::press):
            // o custo e so um provider() a mais por quadro nesse caso raro, nunca
            // um resultado errado. Dedupicar aqui so trocaria esse custo
            // desprezivel por uma branch que nenhuma assercao observavel
            // consegue matar por mutacao (o "certo" e o "errado" produzem o
            // MESMO dx/dy/run) - preferimos o codigo mais simples, sem a branch
            // morta.
            watched_keys_.push_back(WatchedKey{.key = *key, .godot_keycode = kb.keycode});
        }
    }
}

void GlintfxInput::poll() {
    for (auto& w : watched_keys_) {
        const bool now_pressed = provider_(w.key);
        if (now_pressed == w.pressed) {
            continue;  // sem transicao: nada a sintetizar
        }
        if (now_pressed) {
            mapper_.press(w.godot_keycode);
        } else {
            mapper_.release(w.godot_keycode);
        }
        w.pressed = now_pressed;
    }
}

void GlintfxInput::clear() noexcept {
    mapper_.clear();
    for (auto& w : watched_keys_) {
        w.pressed = false;
    }
}

void GlintfxInput::set_controls(gus::domain::input::InputRemapConfig config) {
    config_ = std::move(config);
    mapper_ = InputMapper(config_);
    rebuild_watched_keys();
}

int GlintfxInput::dx() const noexcept { return mapper_.movement_dx(); }

int GlintfxInput::dy() const noexcept { return mapper_.movement_dy(); }

bool GlintfxInput::run() const noexcept { return mapper_.run_active(); }

bool GlintfxInput::is_action_active(std::string_view action_name) const {
    return mapper_.is_action_active(action_name);
}

}  // namespace gus::platform::input
