// GusEngine/platform/tests/key_translation_test.cpp
//
// Qt Test da traducao de keycode Qt -> keycode Godot (M1, platform/input).
// TEST-FIRST. Headless: nao abre janela, so exercita a tabela de traducao.
//
// POR QUE EXISTE: o esquema de fabrica (domain/input/controls_restore.cpp) usa os
// keycodes do enum Godot Key (decisao do porte M3, congelada nos saves). O backend
// Qt produz Qt::Key, cujos valores DIVERGEM para teclas nomeadas (setas, Shift,
// Enter, ...). As letras ASCII coincidem por acaso (Qt::Key_A == 'A' == 65), mas
// nao se deve depender do acaso: a tabela traduz explicitamente. Esta e a "1 peca"
// que casa o backend de evento com o mapa logico puro (engine-design.md secao 3).
//
// CONTRATO exercitado:
//   - qt_key_to_godot_keycode(qt_key) -> long long no esquema Godot;
//   - teclas ASCII (letras/numeros) passam inalteradas (Qt == ASCII == Godot);
//   - teclas nomeadas usadas no overworld (setas, Shift) mapeiam para o valor
//     Godot exato de controls_restore.cpp (senao WASD-vs-setas divergiria);
//   - tecla desconhecida -> 0 (sentinela "sem binding"), nunca lixo.

#include <QtTest/QtTest>

#include "gus/platform/input/key_translation.hpp"

using gus::platform::input::qt_key_to_godot_keycode;

namespace {
// Espelho dos literais Godot de domain/src/input/controls_restore.cpp (fonte
// canonica). Se aquele arquivo mudar um valor, este teste pega a divergencia.
constexpr long long kGodotLeft = 4194319;
constexpr long long kGodotRight = 4194321;
constexpr long long kGodotUp = 4194320;
constexpr long long kGodotDown = 4194322;
constexpr long long kGodotShift = 4194325;
constexpr long long kGodotEnter = 4194309;
constexpr long long kGodotEscape = 4194305;
constexpr long long kGodotTab = 4194308;

// Valores Qt::Key (qnamespace.h) das teclas exercitadas.
constexpr int kQtKeyW = 0x57;
constexpr int kQtKeyA = 0x41;
constexpr int kQtKeyS = 0x53;
constexpr int kQtKeyD = 0x44;
constexpr int kQtKey1 = 0x31;
constexpr int kQtKeyLeft = 0x01000012;
constexpr int kQtKeyRight = 0x01000014;
constexpr int kQtKeyUp = 0x01000013;
constexpr int kQtKeyDown = 0x01000015;
constexpr int kQtKeyShift = 0x01000020;
constexpr int kQtKeyReturn = 0x01000004;
constexpr int kQtKeyEnter = 0x01000005;
constexpr int kQtKeyEscape = 0x01000000;
constexpr int kQtKeyTab = 0x01000001;
}  // namespace

class KeyTranslationTest : public QObject {
    Q_OBJECT

private slots:
    void letras_ascii_passam_inalteradas() {
        // Letras: Qt::Key_X == 'X' == Godot 'X'. WASD precisa casar para o
        // movimento default funcionar.
        QCOMPARE(qt_key_to_godot_keycode(kQtKeyW), static_cast<long long>('W'));
        QCOMPARE(qt_key_to_godot_keycode(kQtKeyA), static_cast<long long>('A'));
        QCOMPARE(qt_key_to_godot_keycode(kQtKeyS), static_cast<long long>('S'));
        QCOMPARE(qt_key_to_godot_keycode(kQtKeyD), static_cast<long long>('D'));
    }

    void numeros_ascii_passam_inalterados() {
        QCOMPARE(qt_key_to_godot_keycode(kQtKey1), static_cast<long long>('1'));
    }

    void setas_mapeiam_para_o_godot_exato() {
        // Aqui esta o ponto: Qt::Key_Left (0x01000012) != Godot Left (4194319).
        QCOMPARE(qt_key_to_godot_keycode(kQtKeyLeft), kGodotLeft);
        QCOMPARE(qt_key_to_godot_keycode(kQtKeyRight), kGodotRight);
        QCOMPARE(qt_key_to_godot_keycode(kQtKeyUp), kGodotUp);
        QCOMPARE(qt_key_to_godot_keycode(kQtKeyDown), kGodotDown);
    }

    void shift_mapeia_para_o_godot_exato() {
        // move_run = Shift no esquema de fabrica; precisa casar.
        QCOMPARE(qt_key_to_godot_keycode(kQtKeyShift), kGodotShift);
    }

    void teclas_nomeadas_de_ui_mapeiam() {
        // Return e Enter ambos viram o Enter Godot (o esquema usa um so codigo).
        QCOMPARE(qt_key_to_godot_keycode(kQtKeyReturn), kGodotEnter);
        QCOMPARE(qt_key_to_godot_keycode(kQtKeyEnter), kGodotEnter);
        QCOMPARE(qt_key_to_godot_keycode(kQtKeyEscape), kGodotEscape);
        QCOMPARE(qt_key_to_godot_keycode(kQtKeyTab), kGodotTab);
    }

    void tecla_desconhecida_vira_zero() {
        // Um keycode Qt fora da tabela e que nao e ASCII imprimivel -> 0
        // (sentinela "sem binding"), nunca um valor arbitrario.
        QCOMPARE(qt_key_to_godot_keycode(0x01FFFFFF), 0LL);
        QCOMPARE(qt_key_to_godot_keycode(-1), 0LL);
    }
};

QTEST_MAIN(KeyTranslationTest)
#include "key_translation_test.moc"
