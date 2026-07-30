// SPDX-License-Identifier: GPL-3.0-or-later
// gus/platform/src/input/key_translation_glintfx.cpp
//
// Tabela BIDIRECIONAL glintfx::Key <-> keycode Godot. Ver header pro "porque" de cada
// escolha (anchors ja congelados, as 4 exclusoes documentadas, a fusao Left/Right).
// Travado por platform/tests/key_translation_glintfx_test.cpp (TEST-FIRST).
//
// FONTE UNICA DE PARES (kPairs): as DUAS direcoes (glintfx_key_to_godot_keycode e
// godot_keycode_to_glintfx_key) sao derivadas desta UNICA tabela - nao ha lista
// duplicada em cada direcao, entao as duas NUNCA divergem por edicao esquecida de um
// lado so.
//
// Os 9 anchors ja congelados (Escape/Tab/Enter/Left/Up/Right/Down/Shift/Space) usam OS
// MESMOS literais de key_translation.cpp/controls_restore.cpp (fonte canonica do
// fossil). Os demais valores (F-keys, Ctrl/Alt/Super, teclas nomeadas, pontuacao,
// numpad) sao os valores REAIS do enum Key do Godot 4 upstream (core/os/keyboard.h,
// SPECIAL = 1<<22 = 4194304 | offset hexadecimal - conferido na fonte oficial em
// 2026-07-24, nao inventados), escolhidos por NAO colidirem nem com os anchors nem
// entre si (conferencia por script em 2026-07-24, ver o commit desta fatia).

#include "gus/platform/input/key_translation_glintfx.hpp"

namespace gus::platform::input {

namespace {

// Base do bloco "especial" do Key enum do Godot 4 upstream (1<<22). Todo offset
// hexadecimal abaixo e SPECIAL | offset, EXATAMENTE como core/os/keyboard.h upstream
// define (conferido lendo a fonte oficial do Godot em 2026-07-24) - EXCETO onde o
// comentario ao lado avisa que o valor e o ANCHOR ja congelado deste projeto (que
// diverge do upstream real, um desvio pre-existente, nao desta fatia).
constexpr long long kSpecial = 1LL << 22;

struct KeyGodotPair {
    glintfx::Key key;
    long long godot;
};

// Ordem IMPORTA para a direcao REVERSA (godot_keycode_to_glintfx_key): quando mais de
// uma entrada aponta pro MESMO godot (fusao Left/Right), a PRIMEIRA da lista vence -
// por isso toda variante Left* vem ANTES da Right* correspondente.
constexpr KeyGodotPair kPairs[] = {
    // --- Bloco locked (None..PageDown do glintfx) - so as 12 realmente nomeaveis
    //     (None e Count ficam de fora, nao sao teclas de verdade). Anchors JA
    //     congelados por controls_restore.cpp/key_translation.cpp: Up/Down/Left/
    //     Right/Enter/Escape/Tab/Space.
    {glintfx::Key::Up, kSpecial | 0x10},       // 4194320, ANCHOR (kGodotUp)
    {glintfx::Key::Down, kSpecial | 0x12},     // 4194322, ANCHOR (kGodotDown)
    {glintfx::Key::Left, kSpecial | 0x0F},     // 4194319, ANCHOR (kGodotLeft)
    {glintfx::Key::Right, kSpecial | 0x11},    // 4194321, ANCHOR (kGodotRight)
    {glintfx::Key::Enter, kSpecial | 0x05},    // 4194309, ANCHOR (kGodotEnter)
    {glintfx::Key::Escape, kSpecial | 0x01},   // 4194305, ANCHOR (kGodotEscape)
    // Tab: ANCHOR (kGodotTab = 4194308 = SPECIAL|0x04). NOTA: 0x04 e o offset REAL do
    // Godot 4 upstream pra BACKSPACE, nao pra Tab (Tab real seria SPECIAL|0x02) - um
    // desvio PRE-EXISTENTE congelado no fossil deste projeto (controls_restore.cpp),
    // fora do escopo desta fatia (nao mexemos em domain/). E por isso que Backspace
    // fica de fora da tabela (ver header): o valor real dele ja esta "ocupado" por
    // este Tab historico.
    {glintfx::Key::Tab, kSpecial | 0x04},      // 4194308, ANCHOR (kGodotTab)
    {glintfx::Key::Space, 32},                 // ANCHOR (kGodotSpace), ASCII
    // Backspace: DE PROPOSITO FORA da tabela - ver nota acima e o header.
    {glintfx::Key::Delete, kSpecial | 0x08},   // 4194312 (Godot KEY_DELETE)
    {glintfx::Key::Home, kSpecial | 0x0D},     // 4194317 (Godot HOME)
    {glintfx::Key::End, kSpecial | 0x0E},      // 4194318 (Godot END)
    {glintfx::Key::PageUp, kSpecial | 0x13},   // 4194323 (Godot PAGEUP)
    {glintfx::Key::PageDown, kSpecial | 0x14}, // 4194324 (Godot PAGEDOWN)

    // --- Letras (ASCII maiusculo, mesma convencao do esquema de fabrica).
    {glintfx::Key::A, 'A'}, {glintfx::Key::B, 'B'}, {glintfx::Key::C, 'C'},
    {glintfx::Key::D, 'D'}, {glintfx::Key::E, 'E'}, {glintfx::Key::F, 'F'},
    {glintfx::Key::G, 'G'}, {glintfx::Key::H, 'H'}, {glintfx::Key::I, 'I'},
    {glintfx::Key::J, 'J'}, {glintfx::Key::K, 'K'}, {glintfx::Key::L, 'L'},
    {glintfx::Key::M, 'M'}, {glintfx::Key::N, 'N'}, {glintfx::Key::O, 'O'},
    {glintfx::Key::P, 'P'}, {glintfx::Key::Q, 'Q'}, {glintfx::Key::R, 'R'},
    {glintfx::Key::S, 'S'}, {glintfx::Key::T, 'T'}, {glintfx::Key::U, 'U'},
    {glintfx::Key::V, 'V'}, {glintfx::Key::W, 'W'}, {glintfx::Key::X, 'X'},
    {glintfx::Key::Y, 'Y'}, {glintfx::Key::Z, 'Z'},

    // --- Digitos (ASCII, mesma convencao).
    {glintfx::Key::Digit0, '0'}, {glintfx::Key::Digit1, '1'}, {glintfx::Key::Digit2, '2'},
    {glintfx::Key::Digit3, '3'}, {glintfx::Key::Digit4, '4'}, {glintfx::Key::Digit5, '5'},
    {glintfx::Key::Digit6, '6'}, {glintfx::Key::Digit7, '7'}, {glintfx::Key::Digit8, '8'},
    {glintfx::Key::Digit9, '9'},

    // --- F1-F12 (valores reais do Godot 4 upstream, nao anchorados ate agora).
    {glintfx::Key::F1, kSpecial | 0x1C}, {glintfx::Key::F2, kSpecial | 0x1D},
    {glintfx::Key::F3, kSpecial | 0x1E}, {glintfx::Key::F4, kSpecial | 0x1F},
    {glintfx::Key::F5, kSpecial | 0x20}, {glintfx::Key::F6, kSpecial | 0x21},
    {glintfx::Key::F7, kSpecial | 0x22}, {glintfx::Key::F8, kSpecial | 0x23},
    {glintfx::Key::F9, kSpecial | 0x24}, {glintfx::Key::F10, kSpecial | 0x25},
    {glintfx::Key::F11, kSpecial | 0x26}, {glintfx::Key::F12, kSpecial | 0x27},

    // --- Modificadores Left/Right: FUNDEM no mesmo godot (Left listado PRIMEIRO =
    //     canonico na direcao reversa - ver header).
    {glintfx::Key::LeftShift, kSpecial | 0x15},     // = ANCHOR kGodotShift
    {glintfx::Key::RightShift, kSpecial | 0x15},    // = ANCHOR kGodotShift
    {glintfx::Key::LeftControl, kSpecial | 0x16},   // Godot CTRL (nao anchorado ainda)
    {glintfx::Key::RightControl, kSpecial | 0x16},
    {glintfx::Key::LeftAlt, kSpecial | 0x18},       // Godot ALT (nao anchorado ainda)
    {glintfx::Key::RightAlt, kSpecial | 0x18},
    {glintfx::Key::LeftSuper, kSpecial | 0x17},     // Godot META (nao anchorado ainda)
    {glintfx::Key::RightSuper, kSpecial | 0x17},

    // --- Teclas nomeadas (valores reais do Godot 4 upstream).
    {glintfx::Key::Insert, kSpecial | 0x07},
    {glintfx::Key::CapsLock, kSpecial | 0x19},
    {glintfx::Key::ScrollLock, kSpecial | 0x1B},
    {glintfx::Key::NumLock, kSpecial | 0x1A},
    {glintfx::Key::PrintScreen, kSpecial | 0x0A},  // Godot PRINT
    {glintfx::Key::Pause, kSpecial | 0x09},
    {glintfx::Key::Menu, kSpecial | 0x42},

    // --- Pontuacao (ASCII, casa com o passthrough generico do SDL - ver teste de
    //     paridade).
    {glintfx::Key::Apostrophe, '\''},
    {glintfx::Key::Comma, ','},
    {glintfx::Key::Minus, '-'},
    {glintfx::Key::Period, '.'},
    {glintfx::Key::Slash, '/'},
    {glintfx::Key::Semicolon, ';'},
    {glintfx::Key::Equal, '='},
    {glintfx::Key::LeftBracket, '['},
    {glintfx::Key::Backslash, '\\'},
    {glintfx::Key::RightBracket, ']'},
    {glintfx::Key::GraveAccent, '`'},
    // World1/World2: DE PROPOSITO FORA da tabela - ver o header (sem entrada
    // equivalente no Key enum do Godot 4).

    // --- Numpad (valores reais do Godot 4 upstream).
    {glintfx::Key::Kp0, kSpecial | 0x86}, {glintfx::Key::Kp1, kSpecial | 0x87},
    {glintfx::Key::Kp2, kSpecial | 0x88}, {glintfx::Key::Kp3, kSpecial | 0x89},
    {glintfx::Key::Kp4, kSpecial | 0x8A}, {glintfx::Key::Kp5, kSpecial | 0x8B},
    {glintfx::Key::Kp6, kSpecial | 0x8C}, {glintfx::Key::Kp7, kSpecial | 0x8D},
    {glintfx::Key::Kp8, kSpecial | 0x8E}, {glintfx::Key::Kp9, kSpecial | 0x8F},
    {glintfx::Key::KpDecimal, kSpecial | 0x84},  // Godot KP_PERIOD
    {glintfx::Key::KpDivide, kSpecial | 0x82},
    {glintfx::Key::KpMultiply, kSpecial | 0x81},
    {glintfx::Key::KpSubtract, kSpecial | 0x83},
    {glintfx::Key::KpAdd, kSpecial | 0x85},
    // KpEqual: DE PROPOSITO FORA da tabela - ver o header (Godot 4 nao tem KP_EQUAL).
    // KpEnter (BUMP v0.26.0): DE PROPOSITO FORA da tabela - ver o header (token
    // vocabulario-so do proprio glintfx, nunca emitido de verdade - a tecla FISICA
    // continua dobrando em Key::Enter, ja coberto pelo ANCHOR acima).
};

}  // namespace

long long glintfx_key_to_godot_keycode(glintfx::Key key) noexcept {
    for (const KeyGodotPair& pair : kPairs) {
        if (pair.key == key) {
            return pair.godot;
        }
    }
    return 0;  // desconhecida / None / Count / cast fora de faixa / exclusao documentada
}

std::optional<glintfx::Key> godot_keycode_to_glintfx_key(long long godot_keycode) noexcept {
    if (godot_keycode == 0) {
        return std::nullopt;  // 0 e sempre sentinela, nunca um keycode godot valido
    }
    for (const KeyGodotPair& pair : kPairs) {
        if (pair.godot == godot_keycode) {
            return pair.key;  // 1a ocorrencia = canonica (Left antes de Right na tabela)
        }
    }
    return std::nullopt;
}

GlintfxCapturedKey glintfx_capture_key(glintfx::Key key, int modifiers) noexcept {
    GlintfxCapturedKey out;
    out.is_escape = (key == glintfx::Key::Escape);
    if (out.is_escape) {
        // MESMA semantica do caminho SDL (system_menu_loop.cpp: "is_escape ? 0 :
        // ..."): Esc sobrescreve qualquer traducao, binding fica no default
        // (keycode=0), modifiers ignorados - Esc nunca vira um binding valido.
        return out;
    }
    out.binding.keycode = glintfx_key_to_godot_keycode(key);
    out.binding.ctrl_pressed = (modifiers & glintfx::Mod_Ctrl) != 0;
    out.binding.shift_pressed = (modifiers & glintfx::Mod_Shift) != 0;
    out.binding.alt_pressed = (modifiers & glintfx::Mod_Alt) != 0;
    return out;
}

}  // namespace gus::platform::input
