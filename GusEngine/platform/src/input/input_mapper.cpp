// gus/platform/src/input/input_mapper.cpp
//
// Implementacao do InputMapper (M1). Ver header. Travado por
// platform/tests/input_mapper_test.cpp (TEST-FIRST).

#include "gus/platform/input/input_mapper.hpp"

#include <utility>  // std::move

namespace gus::platform::input {

InputMapper::InputMapper(gus::domain::input::InputRemapConfig config)
    : config_(std::move(config)) {
    // Pre-resolve os keycodes das 5 acoes de movimento uma vez (consulta linear
    // no config so aqui; por frame e O(teclas da acao), tipicamente 1-2).
    left_ = keycodes_for("move_left");
    right_ = keycodes_for("move_right");
    up_ = keycodes_for("move_forward");
    down_ = keycodes_for("move_backward");
    run_ = keycodes_for("move_run");
}

void InputMapper::press(long long keycode) {
    if (keycode == 0) {
        return;  // sentinela "sem binding": ignora
    }
    pressed_.insert(keycode);
}

void InputMapper::release(long long keycode) {
    pressed_.erase(keycode);
}

void InputMapper::clear() noexcept {
    pressed_.clear();
}

std::vector<long long> InputMapper::keycodes_for(std::string_view action_name) const {
    std::vector<long long> out;
    for (const auto& a : config_.actions) {
        if (a.action_name == action_name) {
            out.reserve(a.keys.size());
            for (const auto& kb : a.keys) {
                out.push_back(kb.keycode);
            }
            break;  // action_name e unico no config
        }
    }
    return out;
}

bool InputMapper::any_pressed(const std::vector<long long>& codes) const {
    for (long long c : codes) {
        if (pressed_.find(c) != pressed_.end()) {
            return true;
        }
    }
    return false;
}

bool InputMapper::is_action_active(std::string_view action_name) const {
    return any_pressed(keycodes_for(action_name));
}

int InputMapper::movement_dx() const {
    int dx = 0;
    if (any_pressed(right_)) {
        ++dx;
    }
    if (any_pressed(left_)) {
        --dx;
    }
    return dx;  // teclas opostas se cancelam (resultado 0)
}

int InputMapper::movement_dy() const {
    int dy = 0;
    if (any_pressed(down_)) {
        ++dy;  // +Y e baixo (move_backward / S)
    }
    if (any_pressed(up_)) {
        --dy;  // -Y e cima (move_forward / W)
    }
    return dy;
}

bool InputMapper::run_active() const {
    return any_pressed(run_);
}

}  // namespace gus::platform::input
