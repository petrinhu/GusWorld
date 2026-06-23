// GusEngine/app/tests/sprite_animation_test.cpp
//
// Qt Test da LOGICA de animacao de locomocao (app/screens/sprite_animation),
// POCO sem Qt/GPU. TEST-FIRST. Cobre as duas decisoes que o render so consome:
//   (1) vetor de movimento -> direcao cardinal (com regra de diagonal e parado);
//   (2) distancia percorrida -> quadro de walk ciclico; parado -> neutro; run com
//       passada mais longa (troca menos vezes que walk na mesma distancia).
//
// CANON: docs/design/mecanicas/locomotion.md.

#include <QtTest/QtTest>

#include "gus/app/screens/sprite_animation.hpp"

using gus::app::screens::Direction;
using gus::app::screens::direction_from_move;
using gus::app::screens::kWalkFrameCount;
using gus::app::screens::WalkCycle;

class SpriteAnimationTest : public QObject {
    Q_OBJECT

private slots:
    // --- direcao a partir do vetor -------------------------------------------
    void cardinais_puras_mapeiam_direto() {
        QCOMPARE(direction_from_move(0, 1, Direction::North), Direction::South);
        QCOMPARE(direction_from_move(0, -1, Direction::South), Direction::North);
        QCOMPARE(direction_from_move(1, 0, Direction::West), Direction::East);
        QCOMPARE(direction_from_move(-1, 0, Direction::East), Direction::West);
    }

    void parado_mantem_a_direcao_anterior() {
        QCOMPARE(direction_from_move(0, 0, Direction::East), Direction::East);
        QCOMPARE(direction_from_move(0, 0, Direction::North), Direction::North);
    }

    void diagonal_horizontal_vence() {
        // Criterio documentado: na diagonal, o eixo HORIZONTAL (Leste/Oeste) ganha.
        QCOMPARE(direction_from_move(1, 1, Direction::North), Direction::East);
        QCOMPARE(direction_from_move(1, -1, Direction::South), Direction::East);
        QCOMPARE(direction_from_move(-1, 1, Direction::North), Direction::West);
        QCOMPARE(direction_from_move(-1, -1, Direction::South), Direction::West);
    }

    // --- ciclo de walk por distancia -----------------------------------------
    void parado_fica_neutro() {
        WalkCycle c;
        c.advance(0.0f, false);
        QVERIFY(!c.is_moving());
        QCOMPARE(c.current_frame(), WalkCycle::kNeutralFrame);
    }

    void anda_sai_do_neutro_no_primeiro_passo() {
        WalkCycle c;
        c.advance(2.0f, false);  // andou um pouco (< 8 px)
        QVERIFY(c.is_moving());
        QCOMPARE(c.current_frame(), 0);  // primeiro quadro de walk
    }

    void troca_de_quadro_a_cada_passo_de_distancia() {
        // Default 8 px por quadro. Acumular 8 px troca pro proximo quadro.
        WalkCycle c(WalkCycle::Config{/*walk*/ 8.0f, /*run*/ 11.0f});
        c.advance(8.0f, false);   // cruzou 8 px -> quadro 1
        QCOMPARE(c.current_frame(), 1);
        c.advance(8.0f, false);   // -> quadro 2
        QCOMPARE(c.current_frame(), 2);
        c.advance(8.0f, false);   // -> quadro 3
        QCOMPARE(c.current_frame(), 3);
        c.advance(8.0f, false);   // -> volta ao quadro 0 (ciclico)
        QCOMPARE(c.current_frame(), 0);
    }

    void quadro_e_sempre_valido() {
        WalkCycle c;
        float total = 0.0f;
        for (int i = 0; i < 100; ++i) {
            c.advance(3.3f, false);
            total += 3.3f;
            const int f = c.current_frame();
            QVERIFY(f >= 0 && f < kWalkFrameCount);
        }
        Q_UNUSED(total);
    }

    void parar_no_meio_volta_ao_neutro() {
        WalkCycle c;
        c.advance(10.0f, false);   // andando (quadro 1)
        QVERIFY(c.is_moving());
        c.advance(0.0f, false);    // parou
        QVERIFY(!c.is_moving());
        QCOMPARE(c.current_frame(), WalkCycle::kNeutralFrame);
    }

    void run_troca_menos_que_walk_na_mesma_distancia() {
        // Passada de run e mais LONGA: na mesma distancia, troca de quadro MENOS
        // vezes que walk (nao mais rapido). Conto trocas em 33 px.
        WalkCycle walk(WalkCycle::Config{8.0f, 11.0f});
        WalkCycle run(WalkCycle::Config{8.0f, 11.0f});
        // Avanca em passos de 1 px contando quantas vezes o quadro muda.
        auto count_changes = [](WalkCycle& c, bool running) {
            int changes = 0;
            int last = c.current_frame();
            for (int i = 0; i < 33; ++i) {
                c.advance(1.0f, running);
                if (c.current_frame() != last) {
                    ++changes;
                    last = c.current_frame();
                }
            }
            return changes;
        };
        const int walk_changes = count_changes(walk, false);
        const int run_changes = count_changes(run, true);
        QVERIFY(run_changes < walk_changes);  // run troca menos (passada longa)
    }

    void reset_volta_ao_neutro() {
        WalkCycle c;
        c.advance(20.0f, false);
        QVERIFY(c.is_moving());
        c.reset();
        QVERIFY(!c.is_moving());
        QCOMPARE(c.current_frame(), WalkCycle::kNeutralFrame);
        QCOMPARE(c.accumulated(), 0.0f);
    }
};

QTEST_MAIN(SpriteAnimationTest)
#include "sprite_animation_test.moc"
