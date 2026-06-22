// gus/platform/src/input/key_translation.cpp
//
// Tabela de traducao Qt::Key -> keycode Godot (M1). Ver header. Travado por
// platform/tests/key_translation_test.cpp (TEST-FIRST).
//
// Os literais Godot espelham domain/src/input/controls_restore.cpp (fonte unica
// do esquema de fabrica). Os literais Qt espelham qnamespace.h (Qt::Key). Inclui
// <QtCore> APENAS por estar na camada platform/ (permitido); poderia usar os
// literais crus, mas referenciar Qt::Key documenta a intencao.

#include "gus/platform/input/key_translation.hpp"

#include <qnamespace.h>  // Qt::Key_* (camada platform/, Qt permitido)

namespace gus::platform::input {

namespace {

// Esquema Godot Key enum (mesmos valores de controls_restore.cpp).
constexpr long long kGodotEnter = 4194309;
constexpr long long kGodotEscape = 4194305;
constexpr long long kGodotTab = 4194308;
constexpr long long kGodotLeft = 4194319;
constexpr long long kGodotRight = 4194321;
constexpr long long kGodotUp = 4194320;
constexpr long long kGodotDown = 4194322;
constexpr long long kGodotShift = 4194325;
constexpr long long kGodotSpace = 32;  // ASCII, coincide

}  // namespace

long long qt_key_to_godot_keycode(int qt_key) noexcept {
    // 1) Teclas nomeadas: mapeamento explicito (os valores divergem do Godot).
    switch (qt_key) {
        case Qt::Key_Left:
            return kGodotLeft;
        case Qt::Key_Right:
            return kGodotRight;
        case Qt::Key_Up:
            return kGodotUp;
        case Qt::Key_Down:
            return kGodotDown;
        case Qt::Key_Shift:
            return kGodotShift;
        // Return (teclado principal) e Enter (numerico) viram o mesmo Enter
        // Godot, espelhando o esquema (que usa um codigo so).
        case Qt::Key_Return:
        case Qt::Key_Enter:
            return kGodotEnter;
        case Qt::Key_Escape:
            return kGodotEscape;
        case Qt::Key_Tab:
            return kGodotTab;
        case Qt::Key_Space:
            return kGodotSpace;
        default:
            break;
    }

    // 2) ASCII imprimivel (0x20..0x7E): passa direto (Qt == ASCII == Godot).
    //    Letras minusculas (a..z) sao normalizadas para maiuscula, pois o esquema
    //    de fabrica usa o codigo maiusculo ('W', 'A', ...). Na pratica o Qt ja
    //    entrega maiuscula em QKeyEvent::key(), mas a normalizacao e barata e
    //    blinda contra qualquer fonte que mande minuscula.
    if (qt_key >= 0x20 && qt_key <= 0x7E) {
        if (qt_key >= 'a' && qt_key <= 'z') {
            return static_cast<long long>(qt_key - ('a' - 'A'));
        }
        return static_cast<long long>(qt_key);
    }

    // 3) Desconhecida (nao-ASCII fora da tabela): sentinela "sem binding".
    return 0;
}

}  // namespace gus::platform::input
