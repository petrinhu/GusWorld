// GusEngine/platform/tests/input_mapper_test.cpp
//
// Qt Test do InputMapper (M1, platform/input): estado de teclas pressionadas ->
// acoes logicas ativas -> vetor de movimento cardinal. TEST-FIRST. Headless: nao
// abre janela; alimenta keycodes (ja no esquema Godot) e consulta o resultado.
//
// O InputMapper consome um InputRemapConfig (de default_controls(), o esquema de
// fabrica puro). E a ponte: o app traduz QKeyEvent -> keycode Godot (via
// key_translation) e chama press/release; o mapper diz quais ACOES estao ativas.
// A regra de qual tecla e qual acao continua no dado puro (domain/), nao no codigo.
//
// CONVENCAO DE EIXO (segue tile_grid.hpp: +X direita, +Y BAIXO, top-down):
//   move_left  -> dx = -1   move_right -> dx = +1
//   move_forward (W, "pra cima" na tela) -> dy = -1
//   move_backward (S, "pra baixo")       -> dy = +1
// O vetor devolvido e CARDINAL CRU (componentes em {-1,0,1}); a normalizacao da
// diagonal (feel) NAO e responsabilidade do mapper (decisao do lider, RF-3).
//
// CONTRATO exercitado:
//   - press(keycode)/release(keycode) mantem o conjunto pressionado;
//   - is_action_active(name) true sse alguma tecla bound aquela acao esta down;
//   - movement_dx()/movement_dy() somam as 4 direcoes ativas (cancelam se opostas);
//   - run_active() reflete move_run (Shift);
//   - clear() esvazia tudo.

#include <QtTest/QtTest>

#include "gus/domain/input/controls_restore.hpp"
#include "gus/platform/input/input_mapper.hpp"

using gus::domain::input::default_controls;
using gus::platform::input::InputMapper;

namespace {
// Keycodes Godot (esquema de fabrica) das teclas exercitadas.
constexpr long long kW = 'W';
constexpr long long kA = 'A';
constexpr long long kS = 'S';
constexpr long long kD = 'D';
constexpr long long kShift = 4194325;
}  // namespace

class InputMapperTest : public QObject {
    Q_OBJECT

private slots:
    void inicial_sem_teclas_sem_movimento() {
        InputMapper m(default_controls());
        QCOMPARE(m.movement_dx(), 0);
        QCOMPARE(m.movement_dy(), 0);
        QVERIFY(!m.is_action_active("move_right"));
        QVERIFY(!m.run_active());
    }

    void d_anda_para_a_direita() {
        InputMapper m(default_controls());
        m.press(kD);
        QVERIFY(m.is_action_active("move_right"));
        QCOMPARE(m.movement_dx(), 1);
        QCOMPARE(m.movement_dy(), 0);
    }

    void a_anda_para_a_esquerda() {
        InputMapper m(default_controls());
        m.press(kA);
        QCOMPARE(m.movement_dx(), -1);
    }

    void w_anda_para_cima_y_negativo() {
        InputMapper m(default_controls());
        m.press(kW);
        QVERIFY(m.is_action_active("move_forward"));
        QCOMPARE(m.movement_dy(), -1);  // +Y e baixo; "frente"/W e cima
    }

    void s_anda_para_baixo_y_positivo() {
        InputMapper m(default_controls());
        m.press(kS);
        QCOMPARE(m.movement_dy(), 1);
    }

    void diagonal_soma_cardinal_cru() {
        // W + D = cima-direita: (dx,dy) = (1,-1) CRU (sem normalizar; isso e feel).
        InputMapper m(default_controls());
        m.press(kW);
        m.press(kD);
        QCOMPARE(m.movement_dx(), 1);
        QCOMPARE(m.movement_dy(), -1);
    }

    void teclas_opostas_se_cancelam() {
        // A + D pressionadas ao mesmo tempo -> dx = 0 (cancelam).
        InputMapper m(default_controls());
        m.press(kA);
        m.press(kD);
        QCOMPARE(m.movement_dx(), 0);
    }

    void release_para_o_movimento() {
        InputMapper m(default_controls());
        m.press(kD);
        QCOMPARE(m.movement_dx(), 1);
        m.release(kD);
        QCOMPARE(m.movement_dx(), 0);
        QVERIFY(!m.is_action_active("move_right"));
    }

    void shift_ativa_run() {
        InputMapper m(default_controls());
        m.press(kShift);
        QVERIFY(m.run_active());
        m.release(kShift);
        QVERIFY(!m.run_active());
    }

    void tecla_sem_binding_nao_afeta_movimento() {
        // Keycode 0 (sentinela "sem binding") ou tecla nao mapeada nao mexe nada.
        InputMapper m(default_controls());
        m.press(0);
        m.press('K');  // K nao e movimento no esquema de fabrica
        QCOMPARE(m.movement_dx(), 0);
        QCOMPARE(m.movement_dy(), 0);
    }

    void press_repetido_e_idempotente() {
        // Auto-repeat do SO: varios press da mesma tecla sem release == 1 tecla.
        InputMapper m(default_controls());
        m.press(kD);
        m.press(kD);
        m.press(kD);
        QCOMPARE(m.movement_dx(), 1);
        m.release(kD);  // um release zera (conjunto, nao contador)
        QCOMPARE(m.movement_dx(), 0);
    }

    void clear_esvazia_tudo() {
        InputMapper m(default_controls());
        m.press(kW);
        m.press(kD);
        m.press(kShift);
        m.clear();
        QCOMPARE(m.movement_dx(), 0);
        QCOMPARE(m.movement_dy(), 0);
        QVERIFY(!m.run_active());
    }
};

QTEST_MAIN(InputMapperTest)
#include "input_mapper_test.moc"
