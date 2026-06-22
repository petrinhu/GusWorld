// GusEngine/app/tests/overworld_sim_test.cpp
//
// Qt Test da simulacao do overworld (M1, app/screens): junta input cardinal +
// resolve_move (colisao slide) + clamp_camera + interpolacao de render. TEST-FIRST.
// Headless: usa um IRenderer FALSO que so registra o que seria desenhado, sem GPU.
//
// O OverworldSim e a "regra de jogo POCO fora da casca Qt" (engine-design.md
// secao 2/4): recebe dx/dy CRUS (int, ja resolvidos pelo InputMapper) + dt fixo,
// nao QKeyEvent. A casca Qt (GameWindow/main) so o alimenta e desenha. Isso o
// torna testavel sem janela - e prova o feel pedido pelo lider (desliza nas
// paredes, camera presa ao mapa) de forma deterministica.
//
// CONTRATO exercitado:
//   - construcao com TileGrid + Aabb inicial + velocidade (tiles/s);
//   - step_fixed(dx,dy,run,dt) move o jogador deslizando nas paredes (delega a
//     core::spatial::resolve_move): bate na parede num eixo, desliza no outro;
//   - camera_view(vw,vh) = clamp_camera centrado no jogador, preso ao mapa;
//   - render(IRenderer&, alpha) interpola a posicao do jogador entre o passo
//     anterior e o atual (alpha 0 -> anterior, 1 -> atual) e emite os quads das
//     paredes + do jogador. O fake conta/inspeciona os quads.

#include <QtTest/QtTest>

#include <cmath>
#include <vector>

#include "gus/app/screens/overworld_sim.hpp"
#include "gus/app/screens/overworld_tuning.hpp"
#include "gus/core/spatial/tile_grid.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

using gus::app::screens::OverworldSim;
using gus::app::screens::OverworldTuning;
using gus::core::spatial::Aabb;
using gus::core::spatial::CameraView;
using gus::core::spatial::Rect;
using gus::core::spatial::TileGrid;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::IRenderer;

namespace {

// IRenderer falso: registra cada chamada (cor + retangulo de mundo) pra inspecao,
// sem tocar GPU. begin/end so marcam que o frame abriu/fechou e guardam a camera.
class FakeRenderer : public IRenderer {
public:
    struct Cmd {
        Rect rect;
        DrawColor color;
        bool filled;
    };

    void begin_frame(const Rect& camera_world, int pixel_w, int pixel_h) override {
        ++begins;
        last_camera = camera_world;
        last_px = pixel_w;
        last_py = pixel_h;
        cmds.clear();
    }
    void draw_filled_rect(const Rect& world_rect, const DrawColor& c) override {
        cmds.push_back({world_rect, c, true});
    }
    void draw_rect_outline(const Rect& world_rect, const DrawColor& c,
                           float /*thickness_world*/) override {
        cmds.push_back({world_rect, c, false});
    }
    void end_frame() override { ++ends; }

    int begins = 0;
    int ends = 0;
    int last_px = 0;
    int last_py = 0;
    Rect last_camera;
    std::vector<Cmd> cmds;

    // Conta quantos quads PREENCHIDOS (paredes) foram emitidos.
    int filled_count() const {
        int n = 0;
        for (const auto& c : cmds) {
            if (c.filled) {
                ++n;
            }
        }
        return n;
    }
    // Devolve o ultimo quad NAO-preenchido (contorno) = o jogador (desenhado por
    // ultimo, como contorno da AABB).
    const Cmd* player_cmd() const {
        for (auto it = cmds.rbegin(); it != cmds.rend(); ++it) {
            if (!it->filled) {
                return &*it;
            }
        }
        return nullptr;
    }
};

// Mapa de teste 5x5, tile 16, com um bloco interno em (2,2) pra exercitar slide.
//   #####
//   #...#
//   #.#.#
//   #...#
//   #####
TileGrid make_map() {
    return TileGrid::from_rows(
        {
            "#####",
            "#...#",
            "#.#.#",
            "#...#",
            "#####",
        },
        16.0f);
}

constexpr float kEps = 1e-3f;
}  // namespace

class OverworldSimTest : public QObject {
    Q_OBJECT

private slots:
    void anda_para_a_direita_em_campo_livre() {
        // Jogador 8x8 na celula (1,1) (canto em mundo 16,16). Velocidade 4 tiles/s
        // = 64 u/s. dt = 1/60 -> ~1.0667 u por passo. Move dx=+1.
        TileGrid g = make_map();
        OverworldSim sim(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, 4.0f);
        float x0 = sim.player().x;
        sim.step_fixed(1, 0, false, 1.0f / 60.0f);
        QVERIFY(sim.player().x > x0);                 // andou para +X
        QVERIFY(qAbs(sim.player().y - 16.0f) < kEps);  // Y nao mudou
    }

    void parede_bloqueia_o_eixo_mas_desliza_no_outro() {
        // Jogador encostando na parede de baixo da celula (1,1), tentando ir
        // para baixo-e-direita. A parede em (2,2) e o desenho do mapa forcam o
        // teste do slide: indo (dx=+1, dy=+1) contra um bloco, ainda anda no eixo
        // livre. Posiciono o jogador colado a face esquerda do bloco (2,2).
        TileGrid g = make_map();
        // Bloco (2,2) ocupa mundo [32,48]x[32,48]. Jogador 8x8 colado a esquerda
        // dele: x = 24 (face direita em 32), y = 34 (sobrepoe a faixa do bloco).
        OverworldSim sim(g, Aabb{24.0f, 34.0f, 8.0f, 8.0f}, 6.0f);
        sim.step_fixed(1, 1, true, 1.0f / 60.0f);  // tenta direita+baixo, correndo
        // X deve ser bloqueado pelo bloco (nao atravessa 32 - 8 = 24); Y desliza.
        QVERIFY(sim.player().x <= 24.0f + kEps);  // barrado em X pelo bloco
        QVERIFY(sim.player().y > 34.0f);          // deslizou em Y (eixo livre)
    }

    void nao_atravessa_a_borda_do_mapa() {
        // Jogador colado a borda esquerda (celula (1,1), x=16 toca a parede (0,*)).
        // Empurrando para a esquerda, nao deve passar de x=16 (borda = parede).
        TileGrid g = make_map();
        OverworldSim sim(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, 8.0f);
        for (int i = 0; i < 10; ++i) {
            sim.step_fixed(-1, 0, false, 1.0f / 60.0f);
        }
        QVERIFY(sim.player().x >= 16.0f - kEps);  // nunca entra na parede
    }

    void camera_segue_o_jogador_presa_ao_mapa() {
        // Mapa 5x5 tile 16 -> mundo 80x80. Viewport 40x40. Jogador no centro.
        TileGrid g = make_map();
        OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, 4.0f);
        CameraView v = sim.camera_view(40.0f, 40.0f);
        // Centro do jogador = (40,40); viewport 40 cabe no mapa 80 -> sem clamp.
        QVERIFY(qAbs(v.center.x - 40.0f) < kEps);
        QVERIFY(qAbs(v.center.y - 40.0f) < kEps);
        QVERIFY(qAbs(v.rect.x - 20.0f) < kEps);  // 40 - 40/2
    }

    void camera_clampa_na_borda() {
        // Jogador no canto superior-esquerdo: a camera nao pode mostrar fora do mapa.
        TileGrid g = make_map();
        OverworldSim sim(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, 4.0f);
        CameraView v = sim.camera_view(40.0f, 40.0f);
        QVERIFY(v.rect.x >= -kEps);  // retangulo preso em x>=0
        QVERIFY(v.rect.y >= -kEps);
    }

    void render_emite_paredes_e_jogador() {
        // O sim deve mandar desenhar as celulas bloqueadas (preenchidas) e o
        // jogador (contorno). O mapa 5x5 tem 16 celulas de borda + 1 interna = 17
        // bloqueadas. Pode otimizar pra desenhar so as visiveis; aqui o mapa
        // inteiro cabe no viewport, entao espera-se 17 paredes.
        TileGrid g = make_map();
        OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, 4.0f);
        FakeRenderer r;
        sim.render(r, 80.0f, 80.0f, 0.0f);
        QCOMPARE(r.begins, 1);
        QCOMPARE(r.ends, 1);
        QCOMPARE(r.filled_count(), 17);          // 16 borda + 1 bloco interno
        QVERIFY(r.player_cmd() != nullptr);       // jogador desenhado
    }

    void render_interpola_posicao_do_jogador() {
        // Apos um passo, render(alpha=0) desenha na posicao ANTERIOR, alpha=1 na
        // ATUAL, alpha=0.5 no meio. Prova o movimento suave (o lider vai ver liso).
        TileGrid g = make_map();
        OverworldSim sim(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, 60.0f);  // rapido p/ delta visivel
        float xprev = sim.player().x;
        sim.step_fixed(1, 0, false, 1.0f / 60.0f);
        float xcurr = sim.player().x;
        QVERIFY(xcurr > xprev);  // sanidade: andou

        FakeRenderer r0;
        sim.render(r0, 80.0f, 80.0f, 0.0f);
        QVERIFY(qAbs(r0.player_cmd()->rect.x - xprev) < kEps);  // alpha 0 -> anterior

        FakeRenderer r1;
        sim.render(r1, 80.0f, 80.0f, 1.0f);
        QVERIFY(qAbs(r1.player_cmd()->rect.x - xcurr) < kEps);  // alpha 1 -> atual

        FakeRenderer rh;
        sim.render(rh, 80.0f, 80.0f, 0.5f);
        float mid = (xprev + xcurr) * 0.5f;
        QVERIFY(qAbs(rh.player_cmd()->rect.x - mid) < kEps);    // alpha 0.5 -> meio
    }

    void passo_parado_nao_move() {
        TileGrid g = make_map();
        OverworldSim sim(g, Aabb{36.0f, 36.0f, 8.0f, 8.0f}, 4.0f);
        float x0 = sim.player().x;
        float y0 = sim.player().y;
        sim.step_fixed(0, 0, false, 1.0f / 60.0f);
        QVERIFY(qAbs(sim.player().x - x0) < kEps);
        QVERIFY(qAbs(sim.player().y - y0) < kEps);
    }

    void run_move_mais_que_andar() {
        // Mesma direcao/tempo: correr cobre mais distancia que andar.
        TileGrid g = make_map();
        OverworldSim walk(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, 4.0f);
        OverworldSim run(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, 4.0f);
        walk.step_fixed(1, 0, false, 1.0f / 60.0f);
        run.step_fixed(1, 0, true, 1.0f / 60.0f);
        QVERIFY(run.player().x > walk.player().x);
    }

    void ctor_com_tuning_define_velocidade() {
        // O ctor que recebe OverworldTuning deve respeitar a velocidade dele.
        TileGrid g = make_map();
        OverworldTuning t;
        t.walk_speed_tiles_per_sec = 4.0f;
        OverworldSim sim(g, Aabb{16.0f, 16.0f, 8.0f, 8.0f}, t);
        float x0 = sim.player().x;
        sim.step_fixed(1, 0, false, 1.0f / 60.0f);
        QVERIFY(sim.player().x > x0);
    }

    void corner_assist_ligado_contorna_quina() {
        // Tile 16. Parede so na celula (1,0); abertura em (1,1). Jogador 8x8
        // levemente desalinhado (4 u na faixa da parede), indo para a direita.
        // Com corner-assist (default LIGADO no tuning), deve ser empurrado pra
        // baixo e contornar; sem ele, travaria.
        TileGrid g = TileGrid::from_rows({
            ".#.",
            "...",
        }, 16.0f);
        OverworldTuning on;  // corner.enabled = true (default)
        OverworldSim sim_on(g, Aabb{8.0f, 12.0f, 8.0f, 8.0f}, on);
        // velocidade alta pra o passo cruzar a quina num tick.
        on.walk_speed_tiles_per_sec = 8.0f;
        OverworldSim sim_on2(g, Aabb{8.0f, 12.0f, 8.0f, 8.0f}, on);
        sim_on2.step_fixed(1, 0, false, 1.0f / 60.0f);
        QVERIFY(sim_on2.player().y > 12.0f);  // empurrado para a abertura (baixo)

        OverworldTuning off;
        off.walk_speed_tiles_per_sec = 8.0f;
        off.corner.enabled = false;
        OverworldSim sim_off(g, Aabb{8.0f, 12.0f, 8.0f, 8.0f}, off);
        sim_off.step_fixed(1, 0, false, 1.0f / 60.0f);
        QVERIFY(qAbs(sim_off.player().y - 12.0f) < kEps);  // sem assist: nao empurra
    }

    void normalize_diagonal_iguala_velocidade_a_cardinal() {
        // Campo aberto. Com normalize_diagonal=true, a distancia percorrida na
        // diagonal (1,1) deve igualar a de um eixo so (modulo do vetor = 1), nao
        // ~1.41x. Comparo o deslocamento total.
        TileGrid g(9, 9, 16.0f);
        OverworldTuning norm;
        norm.normalize_diagonal = true;
        norm.walk_speed_tiles_per_sec = 4.0f;

        OverworldSim card(g, Aabb{64.0f, 64.0f, 8.0f, 8.0f}, norm);
        OverworldSim diag(g, Aabb{64.0f, 64.0f, 8.0f, 8.0f}, norm);
        card.step_fixed(1, 0, false, 1.0f / 60.0f);
        diag.step_fixed(1, 1, false, 1.0f / 60.0f);

        const float card_dx = card.player().x - 64.0f;  // deslocamento cardinal
        const float diag_dx = diag.player().x - 64.0f;
        const float diag_dy = diag.player().y - 64.0f;
        const float diag_len = std::sqrt(diag_dx * diag_dx + diag_dy * diag_dy);
        // Normalizada: o COMPRIMENTO do passo diagonal == o passo cardinal.
        QVERIFY(qAbs(diag_len - card_dx) < 1e-3f);
    }

    void diagonal_crua_por_padrao_anda_mais_que_cardinal() {
        // Sem normalizar (default): a diagonal cobre ~1.41x o eixo unico.
        TileGrid g(9, 9, 16.0f);
        OverworldTuning raw;  // normalize_diagonal = false (default)
        raw.walk_speed_tiles_per_sec = 4.0f;
        OverworldSim card(g, Aabb{64.0f, 64.0f, 8.0f, 8.0f}, raw);
        OverworldSim diag(g, Aabb{64.0f, 64.0f, 8.0f, 8.0f}, raw);
        card.step_fixed(1, 0, false, 1.0f / 60.0f);
        diag.step_fixed(1, 1, false, 1.0f / 60.0f);
        const float card_dx = card.player().x - 64.0f;
        const float diag_dx = diag.player().x - 64.0f;
        QVERIFY(qAbs(diag_dx - card_dx) < kEps);  // cada eixo igual ao cardinal
        // logo o comprimento diagonal ~ 1.41x (mais rapido) - comportamento atual.
    }
};

QTEST_MAIN(OverworldSimTest)
#include "overworld_sim_test.moc"
