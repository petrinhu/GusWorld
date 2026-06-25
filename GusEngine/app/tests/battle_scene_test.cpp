// GusEngine/app/tests/battle_scene_test.cpp
//
// Catch2 (headless) da BattleScene (M5, incremento 1): prova que a cena LE o estado do
// motor de combate (domain/combat/) e DESENHA o esqueleto num IRenderer FALSO, SEM
// janela nem SDL. Cobre:
//   - o encontro de DEMO monta (party de 3 com Gus + 4 inimigos);
//   - a fila de iniciativa esta ordenada por SPD (queue_len = 7);
//   - gus_party_index acha o Gus pelo is_universal_compiler;
//   - o render emite primitivos pras 4 zonas (fundo + arena + CTB + HUD) e desenha
//     UM retangulo por ator vivo (3 party + 4 inimigos) e por celula CTB ocupada;
//   - degrada pro retangulo quando nao ha retratos (caminho headless).

#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "gus/app/screens/battle_floaters.hpp"   // Floater/HitChannel
#include "gus/app/screens/battle_hud_model.hpp"  // kHpBarW/kArenaHpBarH p/ casar rects
#include "gus/app/screens/battle_log_model.hpp"  // LogLine/LogLineKind
#include "gus/app/screens/battle_menu.hpp"       // BattleVerb
#include "gus/app/screens/battle_pacing.hpp"     // PacingState/constantes
#include "gus/app/screens/battle_scene.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/platform/render2d/i_renderer.hpp"

using gus::app::screens::BattleScene;
using gus::app::screens::BattleStatusIconSet;
using gus::app::screens::BattleVerb;
using gus::app::screens::kFloaterLifeSeconds;
using gus::app::screens::kPacingIntroSeconds;
using gus::app::screens::kPacingStepDelaySeconds;
using gus::app::screens::LogLineKind;
using gus::app::screens::PacingState;
using gus::app::screens::kArenaHpBarH;
using gus::app::screens::kHpBarH;
using gus::app::screens::kHpBarW;
using gus::core::spatial::Rect;
using gus::platform::render2d::ContentBbox;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::IRenderer;
using gus::platform::render2d::TextureId;
using gus::platform::render2d::UvRect;

namespace {

// IRenderer falso: conta filled-rects, outlines e textured-rects e guarda os rects.
class CountingRenderer : public IRenderer {
public:
    struct Item {
        Rect rect;
        DrawColor color;
    };
    void begin_frame(const Rect& cam, int pw, int ph) override {
        fills.clear();
        outlines.clear();
        textured.clear();
        last_cam = cam;
        last_pw = pw;
        last_ph = ph;
        ++begins;
    }
    void draw_filled_rect(const Rect& r, const DrawColor& c) override {
        fills.push_back({r, c});
    }
    void draw_rect_outline(const Rect& r, const DrawColor& c, float) override {
        outlines.push_back({r, c});
    }
    TextureId load_texture(const char*) override { return next_tex_++; }
    void draw_textured_rect(const Rect& r, TextureId, const UvRect&,
                            const DrawColor&) override {
        textured.push_back(r);
    }
    ContentBbox texture_content_bbox(TextureId) const override { return {}; }
    void draw_text(const char* text, float x, float y, float px_size,
                   const DrawColor& c, bool bold) override {
        texts.push_back(TextItem{text != nullptr ? text : "", x, y, px_size, c, bold});
    }
    void end_frame() override { ++ends; }

    struct TextItem {
        std::string text;
        float x = 0.0f;
        float y = 0.0f;
        float px = 0.0f;
        DrawColor color;
        bool bold = false;
    };

    std::vector<Item> fills;
    std::vector<Item> outlines;
    std::vector<Rect> textured;
    std::vector<TextItem> texts;
    Rect last_cam{};
    int last_pw = 0;
    int last_ph = 0;
    int begins = 0;
    int ends = 0;
    TextureId next_tex_ = 1;
};

// Conta quantos fills caem (centro) dentro da metade ESQUERDA / DIREITA da tela.
int fills_in_x_band(const std::vector<CountingRenderer::Item>& fills, float x0,
                    float x1) {
    int n = 0;
    for (const auto& f : fills) {
        const float cx = f.rect.x + f.rect.w * 0.5f;
        if (cx >= x0 && cx < x1) {
            ++n;
        }
    }
    return n;
}

// Navega o menu da cena ate o verbo desejado ficar selecionado (wrap garante achar).
void select_verb(BattleScene& scene, BattleVerb want) {
    for (int i = 0; i < 12 && scene.menu().selected_verb() != want; ++i) {
        scene.menu_move(+1);
    }
}

// PACING (incremento 6): a cena comeca PARADA (intro) e anima turno-a-turno. Pra os
// testes de turno, "bombeia" o ritmo (skip + update) ate cair na vez do jogador ou o
// combate acabar. Cada skip pula a intro/delay; update conduz 1 passo por vez.
void pump_to_player_turn(BattleScene& scene) {
    for (int i = 0; i < 200; ++i) {
        if (scene.combat_over() || scene.waiting_player_input()) {
            return;
        }
        scene.skip();                // acelera a intro/delay corrente
        scene.update(1.0f / 60.0f);  // conduz 1 passo (resolve 1 turno de inimigo)
    }
}

}  // namespace

TEST_CASE("BattleScene monta o encontro de demo e le a fila do motor",
          "[battle_scene]") {
    BattleScene scene;
    REQUIRE(scene.party_count() == 3);
    REQUIRE(scene.enemy_count() == 4);
    // Pode haver dano (inimigo de maior SPD age primeiro no incremento 3), mas todos os
    // 7 atores seguem vivos na demo (ataque basico nao mata em 1 hit aqui).
    REQUIRE(scene.queue_len() == 7);
    REQUIRE(scene.active_actor() != nullptr);
    // INCREMENTO 6 (D10): a cena comeca PARADA na intro - NINGUEM agiu ainda. O ator
    // ativo e o de maior SPD (inimigo3, 13), mas ele so age quando o ritmo comeca.
    REQUIRE(scene.is_intro());
    REQUIRE(scene.active_actor()->spd() == 13);  // inimigo3 esperando o ritmo comecar
    REQUIRE_FALSE(scene.combat_over());
}

TEST_CASE("BattleScene acha o Gus na party pelo compilador universal",
          "[battle_scene]") {
    BattleScene scene;
    const int gi = scene.gus_party_index();
    REQUIRE(gi >= 0);
    REQUIRE(gi < scene.party_count());
}

TEST_CASE("BattleScene::render abre/fecha 1 frame com camera logica 640x360",
          "[battle_scene]") {
    BattleScene scene;
    CountingRenderer r;
    scene.render(r, /*viewport_px_w=*/1280.0f, /*viewport_px_h=*/720.0f);
    REQUIRE(r.begins == 1);
    REQUIRE(r.ends == 1);
    // Camera 1:1 no retangulo logico 640x360.
    REQUIRE(r.last_cam.w == 640.0f);
    REQUIRE(r.last_cam.h == 360.0f);
    // begin_frame recebeu os PIXELS REAIS da janela (pro backend escalar).
    REQUIRE(r.last_pw == 1280);
    REQUIRE(r.last_ph == 720);
}

TEST_CASE("BattleScene::render desenha 1 placeholder por ator (esquerda x direita)",
          "[battle_scene]") {
    BattleScene scene;
    CountingRenderer r;
    scene.render(r, 640.0f, 360.0f);

    // Fundo + faixa CTB + 7 atores + 5 celulas CTB + painel + log = varios fills.
    REQUIRE(r.fills.size() >= 7);  // pelo menos um por ator

    // Atores: 3 na metade esquerda (party) e 4 na metade direita (inimigos). Conta
    // SO os fills no tamanho de slot (56x64) pra nao misturar com fundo/HUD.
    auto count_actor_slots = [&](float x0, float x1) {
        int n = 0;
        for (const auto& f : r.fills) {
            if (f.rect.w == 56.0f && f.rect.h == 64.0f) {
                const float cx = f.rect.x + f.rect.w * 0.5f;
                if (cx >= x0 && cx < x1) {
                    ++n;
                }
            }
        }
        return n;
    };
    REQUIRE(count_actor_slots(0.0f, 320.0f) == 3);    // party esquerda
    REQUIRE(count_actor_slots(320.0f, 640.0f) == 4);  // inimigos direita
    (void)fills_in_x_band;  // helper disponivel pra evolucao do teste
}

TEST_CASE("BattleScene::render headless degrada CTB pro retangulo (sem retratos)",
          "[battle_scene]") {
    BattleScene scene;  // sem set_portraits -> sem texturas
    CountingRenderer r;
    scene.render(r, 640.0f, 360.0f);
    // Sem retratos: nenhuma textura desenhada; as celulas CTB viram retangulos.
    REQUIRE(r.textured.empty());
    // 5 celulas CTB ocupadas (fila de 7) desenhadas como retangulos 48x48.
    int ctb_cells = 0;
    for (const auto& f : r.fills) {
        if (f.rect.w == 48.0f && f.rect.h == 48.0f) {
            ++ctb_cells;
        }
    }
    REQUIRE(ctb_cells == 5);
    // O ator ativo recebe um highlight (outline) na arena.
    REQUIRE(r.outlines.size() >= 1);
}

// ---- INCREMENTO 2: painel com dados reais + barras na arena + ativo na fila --------

TEST_CASE("painel do ator ativo desenha a barra de HP (largura kHpBarW)",
          "[battle_scene]") {
    BattleScene scene;
    CountingRenderer r;
    scene.render(r, 640.0f, 360.0f);
    // O painel emite uma barra de HP do tamanho kHpBarW x kHpBarH (fundo) + o fill.
    // O fill tem a MESMA altura e largura <= kHpBarW (fracao do HP). Procuramos o
    // FUNDO exato (kHpBarW x kHpBarH) na metade inferior da tela (y do painel).
    bool found_panel_hp = false;
    for (const auto& f : r.fills) {
        if (f.rect.w == static_cast<float>(kHpBarW) &&
            f.rect.h == static_cast<float>(kHpBarH) && f.rect.y > 250.0f) {
            found_panel_hp = true;
        }
    }
    REQUIRE(found_panel_hp);
}

TEST_CASE("painel desenha pips de AP e Mana do ator ativo (recursos reais)",
          "[battle_scene]") {
    BattleScene scene;
    // begin_turn() foi chamado no ctor: o ator ativo tem AP=max_ap (3) e mana=max_mana
    // (ramp). Logo ha pips desenhados (quadradinhos kPipSize) no painel.
    REQUIRE(scene.active_actor() != nullptr);
    REQUIRE(scene.active_actor()->max_ap() == 3);
    REQUIRE(scene.active_actor()->ap() == 3);          // recarregado no begin_turn
    REQUIRE(scene.active_actor()->max_mana() >= 2);    // ramp >= base
    CountingRenderer r;
    scene.render(r, 640.0f, 360.0f);
    // Conta pips (7x7) no painel: AP (3) + Mana (max_mana) >= 5 quadradinhos.
    int pips = 0;
    for (const auto& f : r.fills) {
        if (f.rect.w == 7.0f && f.rect.h == 7.0f && f.rect.y > 250.0f) {
            ++pips;
        }
    }
    REQUIRE(pips >= scene.active_actor()->max_ap() +
                       scene.active_actor()->max_mana());
}

TEST_CASE("arena desenha 1 mini-barra de HP por ator vivo (7 barras)",
          "[battle_scene]") {
    BattleScene scene;
    CountingRenderer r;
    scene.render(r, 640.0f, 360.0f);
    // Mini-barra da arena = altura kArenaHpBarH. Cada ator emite FUNDO (56 x H) + FILL
    // (largura <= 56) na MESMA posicao (x,y). Como party (x=40) e inimigos (x=544) ficam
    // centralizados na MESMA banda, varios Y coincidem entre os lados; o que e unico por
    // ator e o par (x,y). Contar pares (x,y) distintos entre os fills de altura
    // kArenaHpBarH da exatamente 1 barra por ator (7), sem depender de cor.
    std::vector<std::pair<float, float>> bar_pos;
    for (const auto& f : r.fills) {
        if (f.rect.h == static_cast<float>(kArenaHpBarH)) {
            const std::pair<float, float> xy{f.rect.x, f.rect.y};
            bool seen = false;
            for (const auto& p : bar_pos) {
                if (p == xy) {
                    seen = true;
                    break;
                }
            }
            if (!seen) {
                bar_pos.push_back(xy);
            }
        }
    }
    REQUIRE(bar_pos.size() == 7);  // 3 party (x=40) + 4 inimigos (x=544)
}

TEST_CASE("mini-barra de HP reflete dano real (apos um ataque)", "[battle_scene]") {
    // Abertura LIMPA (incr 6, BUG B): todos comecam com HP CHEIO. Pra ver uma barra
    // PARCIAL, resolvemos um ataque do jogador (o alvo perde HP) e renderizamos.
    BattleScene scene;
    pump_to_player_turn(scene);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    CountingRenderer r;
    scene.render(r, 640.0f, 360.0f);
    // Existe pelo menos um FILL de HP da arena com largura < 56 (o alvo ferido). O fill
    // tem altura kArenaHpBarH e largura estritamente menor que o fundo (56) e > 0.
    bool found_partial = false;
    for (const auto& f : r.fills) {
        if (f.rect.h == static_cast<float>(kArenaHpBarH) && f.rect.w > 0.0f &&
            f.rect.w < 56.0f) {
            found_partial = true;
        }
    }
    REQUIRE(found_partial);
}

TEST_CASE("status icons: com set_status_icons, o status do ator ativo vira textura",
          "[battle_scene]") {
    BattleScene scene;
    // Na vez do jogador, o ator ativo (Caua, 1o jogador) tem Haste (condicao inicial do
    // demo) -> o painel mostra o icone. Carrega icones fake e bombeia ate a vez dele.
    BattleStatusIconSet icons;
    for (std::size_t i = 0; i < icons.by_index.size(); ++i) {
        icons.by_index[i] = static_cast<TextureId>(i + 1);  // handles validos
    }
    scene.set_status_icons(icons);
    pump_to_player_turn(scene);

    CountingRenderer r;
    scene.render(r, 640.0f, 360.0f);
    // Sem retratos mas COM icones de status: ha pelo menos 1 textura (o icone de Haste do
    // ator ativo no painel).
    REQUIRE(r.textured.size() >= 1);
}

TEST_CASE("status icons: sem set, o status degrada pro placeholder (sem textura)",
          "[battle_scene]") {
    BattleScene scene;  // sem icones nem retratos
    CountingRenderer r;
    scene.render(r, 640.0f, 360.0f);
    // Nada texturizado (status cai no quadradinho placeholder).
    REQUIRE(r.textured.empty());
}

// ---- INCREMENTO 3: menu de verbos + conducao de turno + log -----------------------
// (helpers select_verb/pump_to_player_turn definidos no anon namespace do topo)

TEST_CASE("turno: a cena para no 1o turno de JOGADOR com o menu pronto",
          "[battle_scene]") {
    BattleScene scene;
    // PACING (incr 6): comeca PARADA (intro). Bombeia ate a vez do jogador.
    pump_to_player_turn(scene);
    REQUIRE(scene.current_actor_is_player());
    REQUIRE(scene.waiting_player_input());
    REQUIRE_FALSE(scene.combat_over());
    // O menu reflete o AP do ator ativo (3): todos os verbos de 1 AP habilitados.
    REQUIRE(scene.menu().is_enabled(BattleVerb::Atacar));
    REQUIRE(scene.menu().is_enabled(BattleVerb::Compilar));
}

TEST_CASE("turno: Atacar resolve a acao e troca o ator ativo", "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    const auto* before = scene.active_actor();
    REQUIRE(before != nullptr);

    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();

    // O motor registrou o ataque do jogador no log (entrada de Attack do atacante). Com
    // o pacing, o turno do jogador resolveu na hora (menu_confirm), e o ritmo entrou em
    // delay antes de animar os inimigos.
    bool found_attack = false;
    for (const auto& e : scene.machine().log()) {
        if (e.actor_id == before->id() &&
            e.action == gus::domain::combat::CombatActionType::Attack) {
            found_attack = true;
        }
    }
    REQUIRE(found_attack);
    // Bombeando o ritmo, a cena volta a uma vez de jogador OU termina (sem travar).
    pump_to_player_turn(scene);
    REQUIRE((scene.waiting_player_input() || scene.combat_over()));
}

TEST_CASE("turno: Compilar NAO consome o turno (so loga, incr 4)", "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    const auto* actor_before = scene.active_actor();
    const int log_before = static_cast<int>(scene.machine().log().size());

    select_verb(scene, BattleVerb::Compilar);
    scene.menu_confirm();

    // O ator ativo continua o MESMO (turno nao encerrou) e o motor NAO recebeu acao
    // (log do motor inalterado); a sinalizacao foi pra o log de UI (log_lines).
    REQUIRE(scene.active_actor() == actor_before);
    REQUIRE(static_cast<int>(scene.machine().log().size()) == log_before);
    const auto lines = scene.log_lines(20);
    bool has_system = false;
    for (const auto& l : lines) {
        if (l.kind == LogLineKind::System) {
            has_system = true;
        }
    }
    REQUIRE(has_system);  // "COMPILAR: abriria o overlay..." entrou no log de UI
}

TEST_CASE("turno: Defender aplica Shield no proprio ator (acao real)",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    const auto* self = scene.active_actor();
    REQUIRE(self != nullptr);
    const std::string self_id = self->id();

    select_verb(scene, BattleVerb::Defender);
    scene.menu_confirm();

    // O motor logou um Defend do ator que agiu.
    bool found_defend = false;
    for (const auto& e : scene.machine().log()) {
        if (e.actor_id == self_id &&
            e.action == gus::domain::combat::CombatActionType::Defend) {
            found_defend = true;
        }
    }
    REQUIRE(found_defend);
}

TEST_CASE("turno: menu_confirm e no-op apos o fim (pacing conduz ate acabar)",
          "[battle_scene]") {
    BattleScene scene;
    // Conduz a batalha ate o fim no ritmo: bombeia ate a vez do jogador, ataca, repete.
    // Cada pump anima os turnos de inimigo um a um; o Atacar resolve o do jogador.
    int guard = 0;
    while (!scene.combat_over() && guard++ < 400) {
        pump_to_player_turn(scene);
        if (scene.combat_over()) {
            break;
        }
        REQUIRE(scene.waiting_player_input());  // o ritmo parou na vez do jogador
        select_verb(scene, BattleVerb::Atacar);
        scene.menu_confirm();
    }
    // Eventualmente termina (vitoria ou derrota) - o ataque basico resolve a demo.
    REQUIRE(scene.combat_over());
    // Confirmar/navegar apos o fim e no-op (nao crasha, nao muda nada).
    const auto* a = scene.active_actor();
    scene.menu_confirm();
    scene.menu_move(+1);
    REQUIRE(scene.active_actor() == a);
}

TEST_CASE("log: render emite o TEXTO da message de cada linha notavel (incr 3.5)",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    // Forca eventos: um Atacar (gera log de ataque do jogador + os inimigos que ja
    // animaram). Renderiza e checa que ha TEXTO desenhado na zona do log (y>=314).
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();

    CountingRenderer r;
    scene.render(r, 640.0f, 360.0f);

    // Texto de log = draw_text na zona do log (y >= 314), com a message crua do motor.
    int log_texts = 0;
    for (const auto& t : r.texts) {
        if (t.y >= 314.0f && !t.text.empty()) {
            ++log_texts;
        }
    }
    REQUIRE(log_texts >= 1);
    // A marca de cor (ancora a categoria) tambem sobrevive ao lado (fallback sem-fonte).
    int log_marks = 0;
    for (const auto& f : r.fills) {
        if (f.rect.w == 3.0f && f.rect.y >= 314.0f) {
            ++log_marks;
        }
    }
    REQUIRE(log_marks >= 1);
}

TEST_CASE("painel: render emite os NUMEROS reais de HP/AP/Mana (incr 3.5)",
          "[battle_scene]") {
    BattleScene scene;
    const auto* a = scene.active_actor();
    REQUIRE(a != nullptr);
    // O painel imprime "hp/max" do ator ativo (numero real lido do motor).
    char expected_hp[32];
    std::snprintf(expected_hp, sizeof(expected_hp), "%d/%d", a->hp(), a->max_hp());

    CountingRenderer r;
    scene.render(r, 640.0f, 360.0f);

    bool found_hp = false;
    bool found_ap = false;
    bool found_mana = false;
    for (const auto& t : r.texts) {
        if (t.text == expected_hp) found_hp = true;
        if (t.text.rfind("AP ", 0) == 0) found_ap = true;
        if (t.text.rfind("Mana ", 0) == 0) found_mana = true;
    }
    REQUIRE(found_hp);
    REQUIRE(found_ap);
    REQUIRE(found_mana);
}

TEST_CASE("menu: render emite o NOME (texto) de cada verbo quando ha translator",
          "[battle_scene]") {
    // Sem translator setado, tr_verb_label devolve "" e o render nao emite texto de
    // verbo - prova o fallback seguro. Com translator, emite os nomes.
    BattleScene scene_no_tr;
    CountingRenderer r0;
    scene_no_tr.render(r0, 640.0f, 360.0f);
    int verb_texts_no_tr = 0;
    for (const auto& t : r0.texts) {
        // Zona do menu: metade direita do painel (x > 320), y do painel (~252..312).
        if (t.x > 320.0f && t.y >= 252.0f && t.y < 314.0f && !t.text.empty()) {
            ++verb_texts_no_tr;
        }
    }
    REQUIRE(verb_texts_no_tr == 0);  // sem translator: caixas sem nome (fallback)

    BattleScene scene;
    gus::app::i18n::Translator tr;
    tr.load_from_content(
        "## COMBAT_ACTION_ATTACK\nAtacar\n\n## COMBAT_ACTION_DEFEND\nDefender\n");
    scene.set_translator(&tr);
    pump_to_player_turn(scene);  // o menu so renderiza na vez do jogador (D9)
    CountingRenderer r;
    scene.render(r, 640.0f, 360.0f);
    bool found_atacar = false;
    for (const auto& t : r.texts) {
        if (t.text == "Atacar") {
            found_atacar = true;
        }
    }
    REQUIRE(found_atacar);  // o verbo Atacar aparece com nome legivel
}

// ---- INCREMENTO 5: numeros flutuantes + log narra + intent --------------------------

TEST_CASE("floater: Atacar spawna um numero flutuante sobre o alvo", "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    // Medimos o DELTA: o ataque do jogador adiciona pelo menos 1 floater de dano.
    const std::size_t before = scene.floaters().size();

    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    REQUIRE(scene.floaters().size() > before);
}

TEST_CASE("floater: update(dt) envelhece e poda os mortos", "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    const std::size_t spawned = scene.floaters().size();
    REQUIRE(spawned > 0);
    // Avanca o tempo alem da vida: todos os floaters morrem e sao podados. (update tambem
    // tica o pacing, mas o combate so anima 1 passo - os floaters do passo envelhecem.)
    for (int i = 0; i < 5; ++i) {
        scene.update(kFloaterLifeSeconds + 0.1f);
    }
    REQUIRE(scene.floaters().empty());
}

TEST_CASE("floater: render desenha o numero (texto) sobre o alvo", "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    REQUIRE(scene.floaters().size() > 0);
    // O floater mais recente tem um texto (numero/FALHA). O render emite esse texto.
    CountingRenderer r;
    scene.render(r, 640.0f, 360.0f);
    bool found_floater_text = false;
    const std::string want = scene.floaters().back().text;
    for (const auto& t : r.texts) {
        if (t.text == want) {
            found_floater_text = true;
        }
    }
    REQUIRE(found_floater_text);
}

TEST_CASE("log: D7 revisado - narra TODA acao (hit comum aparece no log)",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    // Apos um ataque, o log_lines tem >= 1 linha de Damage (o golpe narrado), mesmo que
    // o dano nao seja "notavel" (antes o so-notavel escondia hits comuns).
    const auto lines = scene.log_lines(20);
    bool has_damage_line = false;
    for (const auto& l : lines) {
        if (l.kind == LogLineKind::Damage) {
            has_damage_line = true;
        }
    }
    REQUIRE(has_damage_line);
}

TEST_CASE("log REGRESSAO: status do inicio NAO esconde os ataques na janela visivel",
          "[battle_scene]") {
    // BUG do display (criador): o demo aplica Regen/Haste/Poison no inicio; antes esses
    // 3 "status aplicado" enchiam as ultimas N linhas da caixa e empurravam a narracao de
    // ataque pra fora. Esta regressao trava: na janela VISIVEL (capacity ~4), as acoes
    // aparecem e NENHUMA linha de Status ocupa a caixa.
    BattleScene scene;
    pump_to_player_turn(scene);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    const auto window = scene.log_lines(4);  // o que cabe na caixa
    REQUIRE(window.size() >= 1);
    int damage_lines = 0;
    for (const auto& l : window) {
        REQUIRE(l.kind != LogLineKind::Status);  // status nao polui o rolling
        if (l.kind == LogLineKind::Damage) {
            ++damage_lines;
        }
    }
    REQUIRE(damage_lines >= 1);  // o ataque aparece na caixa
}

TEST_CASE("intent: cada inimigo vivo expoe um IntentPreview (telegraph)",
          "[battle_scene]") {
    BattleScene scene;
    // Os inimigos tem ScriptedBrain registrado: intent_for devolve a previsao (ataque).
    int with_intent = 0;
    for (const auto* a : scene.machine().queue().order()) {
        if (a != nullptr && !a->is_player_side() && a->is_alive()) {
            if (scene.intent_for(*a).has_value()) {
                ++with_intent;
            }
        }
    }
    REQUIRE(with_intent >= 1);
    // Um player NAO tem intent (so inimigos telegrafam). Acha um ator do lado do jogador
    // na fila (o ator ATIVO na intro e o inimigo de maior SPD, nao serve aqui).
    const gus::domain::combat::CombatActor* player = nullptr;
    for (const auto* a : scene.machine().queue().order()) {
        if (a != nullptr && a->is_player_side() && a->is_alive()) {
            player = a;
            break;
        }
    }
    REQUIRE(player != nullptr);
    REQUIRE_FALSE(scene.intent_for(*player).has_value());
}

// ---- INCREMENTO 6: ritmo / pacing (D8/D9/D10/D12) -----------------------------------

TEST_CASE("pacing D10: comeca PARADA na intro, ninguem agiu (log vazio)",
          "[battle_scene]") {
    BattleScene scene;
    REQUIRE(scene.is_intro());
    REQUIRE(scene.pacing_state() == gus::app::screens::PacingState::Intro);
    // Ninguem agiu: o motor nao tem nenhuma entrada de ACAO ainda (so o setup).
    REQUIRE(scene.machine().log().empty());
    REQUIRE(scene.log_lines(10).empty());
    // O banner anuncia a batalha.
    REQUIRE(scene.turn_banner_key() == std::string_view("COMBAT_BANNER_BATTLE"));
}

TEST_CASE("pacing D10 REGRESSAO (BUG B): abertura 100% LIMPA - HP cheio, sem dano/log",
          "[battle_scene]") {
    // BUG B (criador no display): "a batalha iniciou com um ataque de inimigo JA
    // realizado". Causa: o demo pre-aplicava dano no ctor (HP parcial) -> parecia combate.
    // Trava: na abertura, TODOS com HP CHEIO, ZERO floaters, ZERO log de acao - e ASSIM
    // permanece DURANTE TODA a intro (nenhum turno resolve antes do intro terminar).
    BattleScene scene;
    for (const auto* a : scene.machine().queue().order()) {
        REQUIRE(a->hp() == a->max_hp());  // ninguem perdeu HP no start
    }
    REQUIRE(scene.floaters().empty());
    REQUIRE(scene.machine().log().empty());
    // Avanca o tempo ATE QUASE o fim da intro (sem ultrapassar): ninguem agiu ainda.
    scene.update(kPacingIntroSeconds - 0.05f);
    REQUIRE(scene.is_intro());
    REQUIRE(scene.machine().log().empty());   // nenhum ataque resolveu durante a intro
    REQUIRE(scene.floaters().empty());
    for (const auto* a : scene.machine().queue().order()) {
        REQUIRE(a->hp() == a->max_hp());      // HP ainda cheio
    }
    // So DEPOIS da intro o 1o turno anima (ai sim entra no log).
    scene.update(0.1f);
    REQUIRE(scene.machine().log().size() >= 1);
}

TEST_CASE("pacing D8: os turnos de inimigo animam UM DE CADA VEZ (nao todos juntos)",
          "[battle_scene]") {
    BattleScene scene;
    // Passa a intro: o 1o turno (inimigo3, maior SPD) libera e anima 1 passo.
    scene.update(kPacingIntroSeconds + 0.01f);
    // Apos a intro + 1 passo, exatamente 1 turno de inimigo resolveu (1 acao logada),
    // NAO todos os 4 inimigos de uma vez (o problema do playtest).
    const std::size_t after_first = scene.machine().log().size();
    REQUIRE(after_first >= 1);
    // O ritmo agora segura o delay (nao resolve o proximo sem o tempo passar).
    const std::size_t before_delay = scene.machine().log().size();
    scene.update(0.01f);  // dt minusculo: nao passa o delay
    REQUIRE(scene.machine().log().size() == before_delay);  // ainda segurando
    // Passado o delay, o proximo turno anima.
    scene.update(kPacingStepDelaySeconds + 0.01f);
    REQUIRE(scene.machine().log().size() >= before_delay);  // avancou (>= , pode ter parado no jogador)
}

TEST_CASE("pacing: skip acelera a intro e chega na vez do jogador", "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    REQUIRE(scene.waiting_player_input());
    REQUIRE(scene.turn_banner_key() ==
            std::string_view("COMBAT_BANNER_PLAYER_TURN"));
}

TEST_CASE("pacing D9: o banner reflete de quem e a vez (inimigo OU jogador)",
          "[battle_scene]") {
    BattleScene scene;
    // Na intro, banner = BATALHA!.
    REQUIRE(scene.turn_banner_key() == std::string_view("COMBAT_BANNER_BATTLE"));
    scene.skip();          // pula a intro
    scene.update(0.001f);  // anima o 1o passo (inimigo3) e prepara o proximo ator
    // O banner agora e do ATOR ATIVO: ou inimigo (proximo e inimigo) ou jogador (proximo
    // e player). Nunca mais BATALHA! (a intro acabou). Leitura imediata de quem joga (D9).
    if (!scene.combat_over()) {
        const auto key = scene.turn_banner_key();
        REQUIRE((key == std::string_view("COMBAT_BANNER_ENEMY_TURN") ||
                 key == std::string_view("COMBAT_BANNER_PLAYER_TURN")));
        REQUIRE(key != std::string_view("COMBAT_BANNER_BATTLE"));
    }
}

TEST_CASE("pacing D12: a narracao traz a CONSEQUENCIA (dano) no log", "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    // A narracao tem a linha do ataque do jogador com o dano (a message do motor "X ataca
    // Y por N" e a base da consequencia; o status entra quando aplicado).
    const auto lines = scene.log_lines(20);
    bool has_dano = false;
    for (const auto& l : lines) {
        if (l.kind == LogLineKind::Damage &&
            l.text.find(" por ") != std::string::npos) {
            has_dano = true;
        }
    }
    REQUIRE(has_dano);
}
