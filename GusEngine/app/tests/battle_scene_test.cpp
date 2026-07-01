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
#include "gus/app/screens/battle_layout.hpp"     // kBattleLogical*/kActorSlotW p/ rects
#include "gus/app/screens/battle_log_model.hpp"  // LogLine/LogLineKind
#include "gus/app/screens/battle_menu.hpp"       // BattleVerb
#include "gus/app/screens/battle_pacing.hpp"     // PacingState/constantes
#include "gus/app/screens/battle_scene.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"  // kMultFraco (expectativa D3 no teste)
#include "gus/domain/combat/weakness_wheel.hpp"     // WeaknessWheel (fraco = 1.5 no teste)
#include "gus/platform/render2d/i_renderer.hpp"

using gus::app::screens::BattleScene;
using gus::app::screens::BattleStatusIconSet;
using gus::app::screens::BattleVerb;
using gus::app::screens::kFloaterLifeSeconds;
using gus::app::screens::kPacingAnnounceSeconds;
using gus::app::screens::LogLineKind;
using gus::app::screens::PacingState;
using gus::app::screens::arena_rect;
using gus::app::screens::cockpit_hp_bar_rect;
using gus::app::screens::cockpit_menu_zone;
using gus::app::screens::cockpit_rect;
using gus::app::screens::kArenaHpBarH;
using gus::app::screens::kActorSlotH;
using gus::app::screens::kActorSlotW;
using gus::app::screens::kBattleLogicalH;
using gus::app::screens::kBattleLogicalW;
using gus::app::screens::kCockpitW;
using gus::app::screens::kPipSize;
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

// Ataque COMPLETO do jogador no alvo pre-selecionado (D3): escolhe Atacar, entra no
// modo-mira (menu_confirm NAO resolve mais na hora) e confirma o alvo (aim_confirm ->
// resolve). Substitui o antigo "select_verb(Atacar)+menu_confirm" (que resolvia direto),
// agora que Atacar abre a mira. Sem navegar, mira o alvo sugerido = first_alive_enemy
// (D3 (b)) - o MESMO alvo do hardcode antigo, mantendo estes testes validos.
void player_attack(BattleScene& scene) {
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();  // entra em modo-mira (NAO resolve)
    scene.aim_confirm();   // confirma o alvo sugerido -> resolve o turno
}

// N-esimo inimigo VIVO na ordem de fila (ponteiro MUTAVEL: a const-ref da fila so fixa o
// PONTEIRO; o pointee CombatActor nao e const). Deixa o teste montar cenario de mira
// (set_scanned / take_damage) sem seam extra na cena. nullptr se nao ha n-esimo.
gus::domain::combat::CombatActor* alive_enemy_at(BattleScene& scene, int n) {
    int i = 0;
    for (gus::domain::combat::CombatActor* a : scene.machine().queue().order()) {
        if (a != nullptr && !a->is_player_side() && a->is_alive()) {
            if (i == n) {
                return a;
            }
            ++i;
        }
    }
    return nullptr;
}

// PACING (incremento 6): a cena comeca PARADA na ABERTURA esperando o jogador ENCARAR
// (start_combat). Pra os testes de turno, ENCARA primeiro e depois "bombeia" o ritmo
// (skip + update) ate cair na vez do jogador ou o combate acabar. Cada skip encurta o
// anuncio/delay; update conduz 1 beat por vez.
void pump_to_player_turn(BattleScene& scene) {
    if (scene.is_intro()) {
        scene.start_combat();  // ENCARAR: sai da abertura e libera o 1o turno
    }
    for (int i = 0; i < 200; ++i) {
        if (scene.combat_over() || scene.waiting_player_input()) {
            return;
        }
        scene.skip();                // acelera o anuncio/delay corrente
        scene.update(1.0f / 60.0f);  // conduz 1 beat por vez
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

TEST_CASE("BattleScene::render abre/fecha 1 frame com camera logica 960x540",
          "[battle_scene]") {
    BattleScene scene;
    CountingRenderer r;
    scene.render(r, /*viewport_px_w=*/1920.0f, /*viewport_px_h=*/1080.0f);
    REQUIRE(r.begins == 1);
    REQUIRE(r.ends == 1);
    // Camera 1:1 no retangulo logico 960x540 (D1, lider 2026-06-25).
    REQUIRE(r.last_cam.w == static_cast<float>(kBattleLogicalW));
    REQUIRE(r.last_cam.h == static_cast<float>(kBattleLogicalH));
    // begin_frame recebeu os PIXELS REAIS da janela (pro backend escalar x2).
    REQUIRE(r.last_pw == 1920);
    REQUIRE(r.last_ph == 1080);
}

TEST_CASE("BattleScene::render desenha 1 placeholder por ator (esquerda x direita)",
          "[battle_scene]") {
    BattleScene scene;
    CountingRenderer r;
    scene.render(r, 640.0f, 360.0f);

    // Fundo + faixa CTB + 7 atores + 5 celulas CTB + painel + log = varios fills.
    REQUIRE(r.fills.size() >= 7);  // pelo menos um por ator

    // Atores: 3 na metade esquerda (party) e 4 na metade direita (inimigos). O slot tem
    // LARGURA e ALTURA FIXAS (kActorSlotW x kActorSlotH, sem escala dinamica - D3, 960x540
    // lider 2026-06-25). Conta os fills de largura kActorSlotW (so os slots de ator tem
    // essa largura; a barra de HP da arena e mais estreita, painel/log mais largos, CTB e
    // 48).
    const float mid_x = static_cast<float>(kBattleLogicalW) * 0.5f;
    auto count_actor_slots = [&](float x0, float x1) {
        int n = 0;
        for (const auto& f : r.fills) {
            if (f.rect.w == static_cast<float>(kActorSlotW) &&
                f.rect.h == static_cast<float>(kActorSlotH)) {  // slot de ator (fixo)
                const float cx = f.rect.x + f.rect.w * 0.5f;
                if (cx >= x0 && cx < x1) {
                    ++n;
                }
            }
        }
        return n;
    };
    REQUIRE(count_actor_slots(0.0f, mid_x) == 3);  // party esquerda
    REQUIRE(count_actor_slots(mid_x, static_cast<float>(kBattleLogicalW)) == 4);  // inimigos
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

TEST_CASE("cockpit do ator ativo desenha a barra de HP (variante C)",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);  // o cockpit so mostra dados FORA da abertura (pos-Encarar)
    CountingRenderer r;
    scene.render(r, 960.0f, 540.0f);
    // O cockpit emite a barra de HP em cockpit_hp_bar_rect (fundo) + fill. Procuramos o
    // FUNDO exato (largura/altura do cockpit_hp_bar) na coluna do cockpit (x pequeno).
    const Rect hp = cockpit_hp_bar_rect();
    bool found = false;
    for (const auto& f : r.fills) {
        if (f.rect.w == hp.w && f.rect.h == hp.h && f.rect.x < kCockpitW) {
            found = true;
        }
    }
    REQUIRE(found);
}

TEST_CASE("cockpit desenha pips de AP e Mana do ator ativo (recursos reais)",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);  // o cockpit so mostra na vez do jogador (pos-Encarar)
    REQUIRE(scene.active_actor() != nullptr);
    REQUIRE(scene.active_actor()->max_ap() == 3);
    REQUIRE(scene.active_actor()->ap() == 3);          // recarregado no begin_turn
    REQUIRE(scene.active_actor()->max_mana() >= 2);    // ramp >= base
    CountingRenderer r;
    scene.render(r, 960.0f, 540.0f);
    // Pips (kPipSize x kPipSize) na coluna do cockpit (x < cockpit largura).
    int pips = 0;
    for (const auto& f : r.fills) {
        if (f.rect.w == static_cast<float>(kPipSize) &&
            f.rect.h == static_cast<float>(kPipSize) && f.rect.x < kCockpitW) {
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
    player_attack(scene);
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

    player_attack(scene);

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
        player_attack(scene);
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
    player_attack(scene);

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

TEST_CASE("cockpit: render emite os NUMEROS reais de HP + rotulos AP/MANA (variante C)",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);  // o cockpit so mostra FORA da abertura (apos Encarar)
    const auto* a = scene.active_actor();
    REQUIRE(a != nullptr);
    // O cockpit imprime "hp/max" do ator ativo (numero real lido do motor) sobre a barra,
    // e os rotulos "AP"/"MANA" ao lado dos pips.
    char expected_hp[32];
    std::snprintf(expected_hp, sizeof(expected_hp), "%d/%d", a->hp(), a->max_hp());

    CountingRenderer r;
    scene.render(r, 960.0f, 540.0f);

    bool found_hp = false, found_ap = false, found_mana = false, found_name = false;
    for (const auto& t : r.texts) {
        if (t.text == expected_hp) found_hp = true;
        if (t.text == "AP") found_ap = true;
        if (t.text == "MANA") found_mana = true;
        if (t.text == a->display_name()) found_name = true;
    }
    REQUIRE(found_hp);
    REQUIRE(found_ap);
    REQUIRE(found_mana);
    REQUIRE(found_name);  // nome do ator ativo no cockpit
}

TEST_CASE("menu: render emite o NOME (texto) de cada verbo quando ha translator",
          "[battle_scene]") {
    // Sem translator setado, tr_verb_label devolve "" e o render nao emite texto de
    // verbo - prova o fallback seguro. Com translator, emite os nomes.
    BattleScene scene_no_tr;
    pump_to_player_turn(scene_no_tr);  // o menu so renderiza na vez do jogador (cockpit)
    CountingRenderer r0;
    scene_no_tr.render(r0, 960.0f, 540.0f);
    int verb_texts_no_tr = 0;
    const Rect mz = cockpit_menu_zone();
    for (const auto& t : r0.texts) {
        // Zona do MENU: cockpit lateral esquerdo (x < cockpit), dentro da faixa do menu.
        if (t.x < static_cast<float>(kCockpitW) && t.y >= mz.y &&
            t.y <= mz.y + mz.h && !t.text.empty()) {
            ++verb_texts_no_tr;
        }
    }
    REQUIRE(verb_texts_no_tr == 0);  // sem translator: verbos sem nome (fallback)

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

    player_attack(scene);
    REQUIRE(scene.floaters().size() > before);
}

TEST_CASE("floater: update(dt) envelhece e poda os mortos", "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    player_attack(scene);
    REQUIRE(scene.floaters().size() > 0);  // o ataque do jogador spawnou floater(s)

    // Conduz o combate ate o FIM (no fim nao ha mais turno => o pacing nao spawna mais
    // floater novo). So entao avancar o tempo poda TODOS os floaters restantes.
    int guard = 0;
    while (!scene.combat_over() && guard++ < 400) {
        pump_to_player_turn(scene);
        if (scene.combat_over()) {
            break;
        }
        player_attack(scene);
    }
    REQUIRE(scene.combat_over());
    // Combate acabou: nenhum turno novo, nenhum floater novo. Avanca alem da vida -> poda.
    scene.update(kFloaterLifeSeconds + 0.1f);
    REQUIRE(scene.floaters().empty());
}

TEST_CASE("floater: render desenha o numero (texto) sobre o alvo", "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    player_attack(scene);
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
    player_attack(scene);
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
    player_attack(scene);
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
    // ABERTURA INPUT-GATED (lider 2026-06-25): o TEMPO nao avanca a abertura. Por mais
    // que rode update, fica no BATALHA! parado ate o jogador ENCARAR. Ninguem agiu.
    for (int i = 0; i < 60; ++i) {
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.is_intro());                // continua no BATALHA! (nao auto-avancou)
    REQUIRE(scene.machine().log().empty());   // nenhum ataque resolveu
    REQUIRE(scene.floaters().empty());
    for (const auto* a : scene.machine().queue().order()) {
        REQUIRE(a->hp() == a->max_hp());      // HP ainda cheio
    }
    // ENCARAR: o jogador comeca o combate. BEAT 1 (anuncio): o 1o turno de inimigo
    // anuncia SEM resolver (HP intacto, sem log, sem floater).
    scene.start_combat();
    scene.update(0.01f);  // entra no anuncio
    REQUIRE(scene.pacing_state() == PacingState::AnnouncingEnemy);
    REQUIRE(scene.machine().log().empty());   // anuncio: NADA resolveu ainda
    REQUIRE(scene.floaters().empty());
    for (const auto* a : scene.machine().queue().order()) {
        REQUIRE(a->hp() == a->max_hp());      // HP ainda cheio no anuncio (alvo intacto)
    }
    // BEAT 2: so APOS o anuncio o golpe resolve (log + floater).
    scene.update(kPacingAnnounceSeconds + 0.1f);
    REQUIRE(scene.machine().log().size() >= 1);
}

TEST_CASE("pacing D8: os turnos de inimigo animam UM DE CADA VEZ (nao todos juntos)",
          "[battle_scene]") {
    BattleScene scene;
    scene.start_combat();  // ENCARAR: sai da abertura parada
    // BEAT 1 (incremento 6): o 1o turno de inimigo ANUNCIA (sem resolver nada). Nenhuma
    // acao logada ainda - o ataque NAO foi feito (corrige "ja feito").
    scene.update(0.01f);
    REQUIRE(scene.pacing_state() == PacingState::AnnouncingEnemy);
    REQUIRE(scene.machine().log().empty());  // anuncio: nada resolveu

    // O anuncio segura ~kPacingAnnounceSeconds; um dt minusculo nao resolve ainda.
    scene.update(0.01f);
    REQUIRE(scene.machine().log().empty());

    // BEAT 2: passado o anuncio, AGORA resolve UM turno (1 acao logada) - NAO todos os 4
    // inimigos de uma vez (o problema do playtest).
    scene.update(kPacingAnnounceSeconds + 0.01f);
    const std::size_t after_resolve = scene.machine().log().size();
    REQUIRE(after_resolve >= 1);

    // Pos-resolucao: o ritmo segura o delay (nao anima o proximo sem o tempo passar).
    scene.update(0.01f);  // dt minusculo: nao passa o delay
    REQUIRE(scene.machine().log().size() == after_resolve);  // ainda segurando
}

TEST_CASE("abertura: ESPERA o jogador ENCARAR (nao auto-avanca por tempo)",
          "[battle_scene]") {
    // Decisao do lider (2026-06-25): a abertura fica PARADA no "BATALHA!" ate o jogador
    // encarar. O tempo NAO inicia a luta (resolve "o intro passou antes de eu olhar").
    BattleScene scene;
    REQUIRE(scene.is_intro());
    REQUIRE(scene.turn_banner_key() == std::string_view("COMBAT_BANNER_BATTLE"));
    // Roda MUITO tempo: continua na abertura, ninguem agiu.
    for (int i = 0; i < 600; ++i) {
        scene.update(1.0f / 60.0f);  // 10 segundos simulados
    }
    REQUIRE(scene.is_intro());
    REQUIRE(scene.machine().log().empty());
    // ENCARAR: agora o combate comeca (sai da abertura).
    scene.start_combat();
    REQUIRE_FALSE(scene.is_intro());
}

TEST_CASE("abertura REGRESSAO (UI): NAO renderiza painel/menu/log na abertura",
          "[battle_scene]") {
    // BUG do lider: na abertura o cockpit nao deve expor os dados do INIMIGO ativo (1o da
    // fila por SPD). Fix: o cockpit so mostra HP/AP/Mana/verbos FORA da abertura. Esta
    // regressao trava: na abertura ZERO barra de HP do cockpit e ZERO pip; apos ENCARAR,
    // aparecem. (A faixa do cockpit/log e desenhada sempre como fundo opaco - isso e ok;
    // o que importa e nao mostrar os DADOS do inimigo na abertura.)
    BattleScene scene;
    gus::app::i18n::Translator tr;
    tr.load_from_content(
        "## COMBAT_BANNER_BATTLE\nBATALHA!\n\n## COMBAT_INTRO_ENCARAR\n[Enter] Encarar\n");
    scene.set_translator(&tr);
    REQUIRE(scene.is_intro());
    const Rect hp = cockpit_hp_bar_rect();
    CountingRenderer r0;
    scene.render(r0, 960.0f, 540.0f);
    int cockpit_hp = 0, cockpit_pips = 0;
    for (const auto& f : r0.fills) {
        if (f.rect.w == hp.w && f.rect.h == hp.h && f.rect.x < kCockpitW) {
            ++cockpit_hp;  // barra de HP do cockpit
        }
        if (f.rect.w == static_cast<float>(kPipSize) &&
            f.rect.h == static_cast<float>(kPipSize) && f.rect.x < kCockpitW) {
            ++cockpit_pips;  // pip de AP/Mana do cockpit
        }
    }
    REQUIRE(cockpit_hp == 0);    // sem dados do ator na abertura (nao expoe o inimigo)
    REQUIRE(cockpit_pips == 0);
    // Mas o banner "BATALHA!" + prompt aparecem (texto na abertura).
    bool has_intro_text = false;
    for (const auto& t : r0.texts) {
        if (!t.text.empty()) {
            has_intro_text = true;
        }
    }
    REQUIRE(has_intro_text);

    // ENCARAR: agora o cockpit mostra a barra de HP do ator ativo.
    scene.start_combat();
    pump_to_player_turn(scene);
    CountingRenderer r1;
    scene.render(r1, 960.0f, 540.0f);
    bool cockpit_now = false;
    for (const auto& f : r1.fills) {
        if (f.rect.w == hp.w && f.rect.h == hp.h && f.rect.x < kCockpitW) {
            cockpit_now = true;
        }
    }
    REQUIRE(cockpit_now);  // cockpit aparece DEPOIS de Encarar
}

TEST_CASE("abertura: skip NAO comeca a luta (so Encarar/start_combat)",
          "[battle_scene]") {
    BattleScene scene;
    scene.skip();  // acelerar nao deve pular a abertura
    REQUIRE(scene.is_intro());
    REQUIRE(scene.machine().log().empty());
}

TEST_CASE("abertura: TRASH oferece '[Q] Resolver sem encarar' (placeholder nao-destrutivo)",
          "[battle_scene]") {
    BattleScene scene;  // demo: todos os inimigos sao trash (nao-boss)
    REQUIRE(scene.offers_auto_resolve());  // oferecido na abertura (trash)

    const std::size_t engine_log_before = scene.machine().log().size();
    const bool intro_before = scene.is_intro();
    scene.request_auto_resolve();  // PLACEHOLDER: so loga, nada destrutivo

    // Nao mexe no motor (sem acao resolvida) e NAO sai da abertura (placeholder).
    REQUIRE(scene.machine().log().size() == engine_log_before);
    REQUIRE(scene.is_intro() == intro_before);
    // Sinalizou no log de UI ("[auto-resolve: a implementar]").
    bool has_system = false;
    for (const auto& l : scene.log_lines(20)) {
        if (l.kind == LogLineKind::System) {
            has_system = true;
        }
    }
    REQUIRE(has_system);

    // Depois de encarar (sai da abertura), o verbo NAO e mais oferecido.
    scene.start_combat();
    REQUIRE_FALSE(scene.offers_auto_resolve());
}

TEST_CASE("pacing 2-BEATS REGRESSAO: no ANUNCIO o alvo esta INTACTO; so o RESOLVE bate",
          "[battle_scene]") {
    // O lider no display: "a tela aparece com o ataque ja feito". Fix = 2 beats no turno
    // de inimigo. Esta regressao trava: no BEAT 1 (anuncio) NENHUM dano foi aplicado (HP
    // do alvo intacto) e o log NAO tem a linha da acao; SO no BEAT 2 (resolve) aparecem.
    BattleScene scene;
    scene.start_combat();  // ENCARAR: sai da abertura (o 1o ator e inimigo3, maior SPD)
    scene.update(0.001f);  // entra no BEAT 1 (anuncio)
    REQUIRE(scene.pacing_state() == PacingState::AnnouncingEnemy);
    REQUIRE(scene.turn_banner_key() == std::string_view("COMBAT_BANNER_ENEMY_TURN"));

    // Snapshot do HP de TODOS no anuncio: ninguem levou dano ainda; nenhum floater; sem
    // log de acao. O ataque NAO conectou (e aqui que a animacao de windup vai morar).
    std::vector<int> hp_announce;
    for (const auto* a : scene.machine().queue().order()) {
        hp_announce.push_back(a->hp());
        REQUIRE(a->hp() == a->max_hp());  // intacto no anuncio
    }
    REQUIRE(scene.machine().log().empty());
    REQUIRE(scene.floaters().empty());

    // BEAT 2: passa o anuncio -> AGORA resolve. Algum alvo perde HP, ha log e floater.
    scene.update(kPacingAnnounceSeconds + 0.01f);
    REQUIRE(scene.pacing_state() == PacingState::WaitingDelay);  // pos-resolucao
    REQUIRE(scene.machine().log().size() >= 1);                  // a acao foi logada
    REQUIRE(scene.floaters().size() >= 1);                       // dano flutuou
    // ALGUEM perdeu HP (o alvo do golpe do inimigo) - a comparacao com o snapshot prova
    // que o dano so aconteceu DEPOIS do anuncio.
    bool someone_lost_hp = false;
    int idx = 0;
    for (const auto* a : scene.machine().queue().order()) {
        if (idx < static_cast<int>(hp_announce.size()) &&
            a->hp() < hp_announce[static_cast<std::size_t>(idx)]) {
            someone_lost_hp = true;
        }
        ++idx;
    }
    REQUIRE(someone_lost_hp);
}

TEST_CASE("pacing REGRESSAO (ataque colado): skip NAO colapsa o beat de ANUNCIO",
          "[battle_scene]") {
    // BUG do lider no display: "o ataque seguinte sai COLADO com o do inimigo anterior,
    // impressao de ataque duplo". Causa: apertar a tecla durante o anuncio (skip) zerava
    // o timer do anuncio -> o anuncio durava 1 frame e o golpe resolvia colado. Fix: skip
    // so acelera a PAUSA pos-resolucao (WaitingDelay), nunca o anuncio. Esta regressao
    // trava: mesmo com skip TODO frame, o inimigo passa por VARIOS frames de ANUNCIO
    // antes de resolver (o anuncio toca seu tempo proprio; o windup nunca some).
    BattleScene scene;
    scene.start_combat();
    int announce_frames = 0;
    const std::size_t log_before = scene.machine().log().size();
    const float dt = 1.0f / 60.0f;
    for (int f = 0; f < 120; ++f) {
        scene.skip();  // jogador apertando a tecla a cada frame (impaciente)
        scene.update(dt);
        if (scene.pacing_state() == PacingState::AnnouncingEnemy) {
            ++announce_frames;
        }
        // Para assim que o 1o inimigo RESOLVE (log cresceu).
        if (scene.machine().log().size() > log_before) {
            break;
        }
    }
    // O anuncio NAO colapsou: durou MUITO mais que 1 frame, mesmo com skip todo frame.
    // (Antes do fix era 1; agora ~kPacingAnnounceSeconds/dt ~= 42 frames.)
    REQUIRE(announce_frames > 10);
}

TEST_CASE("pacing REGRESSAO: na fila REAL, TODO inimigo tem seu proprio ANUNCIO",
          "[battle_scene]") {
    // O bug so aparece rodando a FILA inteira (Drone->Caua->Sentinela->Gus->Drone->...).
    // Trava: ao longo de varios turnos, CADA resolucao de inimigo foi precedida por um
    // estado de ANUNCIO proprio (nenhum inimigo resolve "colado" sem anunciar). Conta os
    // inimigos resolvidos e os que tiveram anuncio - devem bater.
    BattleScene scene;
    scene.start_combat();
    const float dt = 1.0f / 60.0f;
    bool saw_announce_since_log = false;
    std::size_t last_log = scene.machine().log().size();
    int enemy_resolves = 0;
    int enemy_with_announce = 0;

    for (int f = 0; f < 3000 && !scene.combat_over(); ++f) {
        if (scene.waiting_player_input()) {
            player_attack(scene);
        }
        if (scene.pacing_state() == PacingState::AnnouncingEnemy) {
            saw_announce_since_log = true;
        }
        scene.update(dt);
        const std::size_t now = scene.machine().log().size();
        if (now > last_log) {
            // Houve nova entrada de log. Se foi de um INIMIGO, checa se houve anuncio.
            const auto& e = scene.machine().log().back();
            if (e.actor_id.rfind("inimigo", 0) == 0) {  // ids do demo: inimigo1..4
                ++enemy_resolves;
                if (saw_announce_since_log) {
                    ++enemy_with_announce;
                }
            }
            saw_announce_since_log = false;
            last_log = now;
        }
    }
    REQUIRE(enemy_resolves >= 2);                    // a fila resolveu varios inimigos
    REQUIRE(enemy_with_announce == enemy_resolves);  // CADA UM teve seu anuncio
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
    // Na abertura, banner = BATALHA! (e fica ate o jogador encarar - nao auto-avanca).
    REQUIRE(scene.turn_banner_key() == std::string_view("COMBAT_BANNER_BATTLE"));
    scene.start_combat();  // ENCARAR: sai da abertura
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
    player_attack(scene);
    // POLISH 3 (veredito do lider: log ENXUTO estilo terminal): a linha de dano tem a forma
    // CURTA "<atacante> -> <alvo> -<dano>" (ex.: "gus -> inimigo1 -8"), montada na APRESENTACAO
    // a partir dos campos estruturados do evento. A message CRUA do motor ("X ataca Y por N")
    // segue inalterada no dominio (testada la); aqui checamos so a linha APRESENTAVEL: kind
    // Damage + a seta " -> " (assinatura da linha curta) + o "-" do dano.
    const auto lines = scene.log_lines(20);
    bool has_dano = false;
    for (const auto& l : lines) {
        if (l.kind == LogLineKind::Damage &&
            l.text.find(" -> ") != std::string::npos &&
            l.text.find(" -") != std::string::npos) {
            has_dano = true;
        }
    }
    REQUIRE(has_dano);
}

// ---- INCREMENTO A: modo-mira / target selection (battle-screen.md §3.5, D3) ---------
// Zero motor: a cena so navega/confirma o ALVO; o motor ja resolve qualquer target_id.

TEST_CASE("mira: [Atacar] ENTRA em modo-mira (nao resolve na hora)", "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    const std::size_t log_before = scene.machine().log().size();

    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();

    // Entrou na mira e NADA resolveu (fim do hardcode first_alive_enemy): o motor nao
    // recebeu acao e o ritmo segue esperando o jogador.
    REQUIRE(scene.is_aiming());
    REQUIRE(scene.aim_target() != nullptr);
    REQUIRE(scene.machine().log().size() == log_before);
    REQUIRE(scene.waiting_player_input());
    REQUIRE(scene.aim_count() == scene.enemy_count());  // todos os inimigos vivos miraveis
}

TEST_CASE("mira: [Scan] tambem ENTRA em modo-mira", "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    const std::size_t log_before = scene.machine().log().size();

    select_verb(scene, BattleVerb::Scan);
    scene.menu_confirm();

    REQUIRE(scene.is_aiming());
    REQUIRE(scene.aim_target() != nullptr);
    REQUIRE(scene.machine().log().size() == log_before);  // Scan tambem so mira aqui
}

TEST_CASE("mira: navega entre inimigos vivos com WRAP", "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    REQUIRE(scene.is_aiming());
    const int n = scene.aim_count();
    REQUIRE(n >= 2);  // a demo tem 4 inimigos: da pra navegar

    const auto* t0 = scene.aim_target();
    scene.aim_move(+1);
    REQUIRE(scene.aim_target() != t0);  // mudou de alvo
    // WRAP: no total N passos +1 voltam ao inicio (ja demos 1; +N-1 fecha a volta).
    for (int i = 0; i < n - 1; ++i) {
        scene.aim_move(+1);
    }
    REQUIRE(scene.aim_target() == t0);
    // -1 a partir do inicio vai pro ULTIMO (wrap pra tras).
    scene.aim_move(-1);
    REQUIRE(scene.aim_target() != t0);
}

TEST_CASE("mira D3 (b): SEM scan pre-seleciona o front da fila (age antes)",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    // Nenhum inimigo escaneado no start: a sugestao cai no fallback (b) = o inimigo mais a
    // FRENTE na fila (== o 1o inimigo vivo em queue().order()).
    const gus::domain::combat::CombatActor* front = alive_enemy_at(scene, 0);
    REQUIRE(front != nullptr);
    REQUIRE_FALSE(front->is_scanned());

    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    REQUIRE(scene.aim_target() == front);
}

TEST_CASE("mira D3 (a): COM scan pre-seleciona o inimigo FRACO a familia da acao",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    const gus::domain::combat::CombatActor* self = scene.active_actor();
    REQUIRE(self != nullptr);
    const gus::domain::combat::CardFamily fam = self->family();

    // Acha o 1o inimigo (na ordem de fila) FRACO a familia do ator ativo (mult 1.5) e que
    // NAO seja ja o front (pra a sugestao (a) diferir de (b) e o teste ser inequivoco).
    const gus::domain::combat::CombatActor* front = alive_enemy_at(scene, 0);
    gus::domain::combat::CombatActor* weak = nullptr;
    for (int i = 0; (weak == nullptr); ++i) {
        gus::domain::combat::CombatActor* e = alive_enemy_at(scene, i);
        if (e == nullptr) {
            break;
        }
        if (e != front &&
            gus::domain::combat::WeaknessWheel::multiplier(fam, e->family()) ==
                gus::domain::combat::combat_constants::kMultFraco) {
            weak = e;
        }
    }
    REQUIRE(weak != nullptr);  // a demo garante um fraco != front (Sentinela vs Caua)

    // Escaneia SO o fraco: a pre-selecao (a) deve apontar pra ele (premia o Scan), NAO pro
    // front (b). Prova que Scan muda a sugestao de mira.
    weak->set_scanned(true);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    REQUIRE(scene.aim_target() == weak);
    REQUIRE(scene.aim_target() != front);
}

TEST_CASE("mira: CONFIRMAR monta a acao com o alvo ESCOLHIDO (nao mais o 1o)",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    const auto* self = scene.active_actor();
    REQUIRE(self != nullptr);
    const std::string self_id = self->id();

    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    const auto* suggested = scene.aim_target();  // (b) front da fila
    scene.aim_move(+1);                           // NAVEGA pra outro inimigo
    const auto* chosen = scene.aim_target();
    REQUIRE(chosen != suggested);                 // realmente escolheu outro alvo
    const std::string chosen_id = chosen->id();

    scene.aim_confirm();

    // O motor logou o Attack do jogador no alvo ESCOLHIDO - NAO no 1o da fila (o hardcode
    // antigo sempre batia no first_alive_enemy).
    bool found = false;
    for (const auto& e : scene.machine().log()) {
        if (e.actor_id == self_id &&
            e.action == gus::domain::combat::CombatActionType::Attack) {
            REQUIRE(e.target_id.has_value());
            REQUIRE(*e.target_id == chosen_id);
            found = true;
        }
    }
    REQUIRE(found);
    REQUIRE_FALSE(scene.is_aiming());  // confirmou: saiu da mira
}

TEST_CASE("mira: CANCELAR volta ao menu SEM consumir o turno", "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    const auto* actor_before = scene.active_actor();
    const std::size_t log_before = scene.machine().log().size();

    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    REQUIRE(scene.is_aiming());

    scene.aim_cancel();

    // Voltou ao menu: nao esta mirando, o ator ativo e o MESMO (turno nao consumido), o
    // motor NAO recebeu acao e o ritmo segue na vez do jogador.
    REQUIRE_FALSE(scene.is_aiming());
    REQUIRE(scene.aim_target() == nullptr);
    REQUIRE(scene.active_actor() == actor_before);
    REQUIRE(scene.machine().log().size() == log_before);
    REQUIRE(scene.waiting_player_input());
    // E o menu volta a operar (pode escolher outro verbo e agir).
    select_verb(scene, BattleVerb::Defender);
    scene.menu_confirm();
    REQUIRE(scene.active_actor() != actor_before);  // Defender resolveu -> trocou de ator
}

TEST_CASE("mira: PULA inimigos mortos (fora da lista miravel)", "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    const int alive_before = scene.enemy_count();
    REQUIRE(alive_before >= 2);

    // Mata o inimigo do FRONT direto (hp 0). O motor so poda a fila no advance; mas a mira
    // filtra is_alive(), entao o morto ja sai da lista miravel.
    gus::domain::combat::CombatActor* victim = alive_enemy_at(scene, 0);
    REQUIRE(victim != nullptr);
    victim->take_damage(victim->max_hp());
    REQUIRE_FALSE(victim->is_alive());

    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    REQUIRE(scene.is_aiming());
    REQUIRE(scene.aim_count() == alive_before - 1);  // o morto saiu da lista
    // O alvo sugerido esta VIVO e NAO e o morto.
    REQUIRE(scene.aim_target() != nullptr);
    REQUIRE(scene.aim_target()->is_alive());
    REQUIRE(scene.aim_target() != victim);
    // Navegar por toda a lista nunca pousa no morto.
    for (int i = 0; i < alive_before + 2; ++i) {
        REQUIRE(scene.aim_target() != victim);
        REQUIRE(scene.aim_target()->is_alive());
        scene.aim_move(+1);
    }
}

TEST_CASE("mira: destaque MULTIMODAL do alvo no render (contorno + nome), nao so cor",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    REQUIRE(scene.is_aiming());
    const auto* target = scene.aim_target();
    REQUIRE(target != nullptr);

    CountingRenderer r;
    scene.render(r, 960.0f, 540.0f);
    // (NOME) o nome do inimigo mirado e desenhado como TEXTO. Sem translator o banner nao
    // escreve nome; o cockpit escreve o nome do ATOR ATIVO (player). Logo o nome do INIMIGO
    // so pode vir da MIRA (pista textual = WCAG, nao depende de cor).
    bool found_name = false;
    for (const auto& t : r.texts) {
        if (t.text == target->display_name()) {
            found_name = true;
        }
    }
    REQUIRE(found_name);

    // (CONTORNO) a mira adiciona um reticulo (outline duplo). Cancelando e re-renderizando,
    // o numero de outlines CAI - prova que o contorno da mira e uma pista de FORMA (nao so
    // cor), somada ao highlight normal do ativo.
    const std::size_t outlines_aiming = r.outlines.size();
    scene.aim_cancel();
    CountingRenderer r2;
    scene.render(r2, 960.0f, 540.0f);
    REQUIRE(outlines_aiming > r2.outlines.size());
}

TEST_CASE("mira Scan: CONFIRMAR escaneia o alvo ESCOLHIDO", "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    const auto* self = scene.active_actor();
    REQUIRE(self != nullptr);
    const std::string self_id = self->id();

    select_verb(scene, BattleVerb::Scan);
    scene.menu_confirm();
    REQUIRE(scene.is_aiming());
    scene.aim_move(+1);  // escolhe um alvo != sugerido
    const auto* chosen = scene.aim_target();
    REQUIRE(chosen != nullptr);
    const std::string chosen_id = chosen->id();
    REQUIRE_FALSE(chosen->is_scanned());

    scene.aim_confirm();

    // O motor escaneou o alvo ESCOLHIDO (is_scanned) e logou um Scan com esse target_id.
    REQUIRE(chosen->is_scanned());
    bool found_scan = false;
    for (const auto& e : scene.machine().log()) {
        if (e.actor_id == self_id &&
            e.action == gus::domain::combat::CombatActionType::Scan) {
            REQUIRE(e.target_id.has_value());
            REQUIRE(*e.target_id == chosen_id);
            found_scan = true;
        }
    }
    REQUIRE(found_scan);
}
