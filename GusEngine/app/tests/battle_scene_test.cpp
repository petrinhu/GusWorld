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

#include <algorithm>
#include <string>
#include <vector>

#include "gus/app/screens/battle_floaters.hpp"   // Floater/HitChannel
#include "gus/app/screens/battle_hud_model.hpp"  // kHpBarW/kArenaHpBarH p/ casar rects
#include "gus/app/screens/battle_layout.hpp"     // kBattleLogical*/kActorSlotW p/ rects
#include "gus/app/screens/battle_log_model.hpp"  // LogLine/LogLineKind
#include "gus/app/screens/battle_menu.hpp"       // BattleVerb
#include "gus/app/screens/battle_pacing.hpp"     // PacingState/constantes
#include "gus/app/screens/battle_scene.hpp"
#include "gus/app/screens/battle_sprite_anim.hpp"  // ActorSpriteSet/BattleClipId (W3)
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"  // kMultFraco (expectativa D3 no teste)
#include "gus/domain/combat/weakness_wheel.hpp"     // WeaknessWheel (fraco = 1.5 no teste)
#include "gus/platform/audio/audio_engine.hpp"  // AudioEngine/SoundId (M6 F3, ADR-011)
#include "gus/platform/render2d/i_renderer.hpp"

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>

using gus::app::screens::BattleScene;
using gus::app::screens::BattleStatusIconSet;
using gus::app::screens::BattleVerb;
using gus::app::screens::kFloaterLifeSeconds;
using gus::app::screens::kPacingAnnounceSeconds;
using gus::app::screens::kPacingStepDelaySeconds;
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

// CONTATO do golpe do jogador (W2, battle-anim.md par.3.1): confirmar [Atacar] inicia
// o WINDUP (aproximacao melee) e a RESOLUCAO e DEFERIDA ate o contato (fim da
// aproximacao). Este helper bombeia o tempo ate o motor resolver (player_action_
// in_flight cai) - o equivalente de "esperar a animacao conectar".
void pump_player_strike(BattleScene& scene) {
    for (int i = 0; i < 120 && scene.player_action_in_flight(); ++i) {
        scene.update(1.0f / 60.0f);
    }
}

// Ataque COMPLETO do jogador no alvo pre-selecionado (D3): escolhe Atacar, entra no
// modo-mira (menu_confirm NAO resolve mais na hora) e confirma o alvo (aim_confirm ->
// windup -> CONTATO resolve). Substitui o antigo "select_verb(Atacar)+menu_confirm"
// (que resolvia direto), agora que Atacar abre a mira. Sem navegar, mira o alvo
// sugerido = first_alive_enemy (D3 (b)) - o MESMO alvo do hardcode antigo, mantendo
// estes testes validos. Pos-W2, bombeia o windup ate o contato resolver (a resolucao
// e deferida pela animacao; battle-anim.md par.3.1).
void player_attack(BattleScene& scene) {
    // Comando-livre 1B (§4.1): se a vez da party abriu no PICKER (>1 elegivel), confirma o
    // PRE-SELECIONADO (maior SPD) pra cair no menu de verbos do MESMO ator - transparente pros
    // testes que so querem "o jogador ativo ataca" (mesmo espirito do aim_confirm somado aqui
    // quando [Atacar] passou a abrir a mira no Incremento A). Os testes DO picker navegam/
    // confirmam explicitamente (nao usam este helper).
    if (scene.is_choosing_actor()) {
        scene.actor_picker_confirm();
    }
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();       // entra em modo-mira (NAO resolve)
    scene.aim_confirm();        // confirma o alvo sugerido -> inicia o windup
    pump_player_strike(scene);  // CONTATO: o motor resolve no fim da aproximacao
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
            break;
        }
        scene.skip();                // acelera o anuncio/delay corrente
        scene.update(1.0f / 60.0f);  // conduz 1 beat por vez
    }
    // COMANDO-LIVRE 1B (§4.1): a vez da party agora ABRE com a ESCOLHA DE ATOR quando ha >1
    // elegivel (picker), ANTES do menu de verbos. Estes testes (pre-picker) exercitam o menu
    // de verbos do MESMO ator que agia antes (o de maior SPD = o pre-selecionado): confirmar o
    // cursor default cai exatamente nele, mantendo o comportamento identico. Os testes DO
    // picker param antes (pump_to_actor_picker) e navegam/confirmam explicitamente.
    if (scene.is_choosing_actor()) {
        scene.actor_picker_confirm();
    }
}

// PICKER (comando-livre 1B, §4.1): bombeia ATE a ESCOLHA DE ATOR abrir (a 1a vez da party com
// >1 elegivel), SEM confirmar - pros testes do picker inspecionarem/navegarem/confirmarem. Sai
// tambem se o combate acabar. Ao contrario de pump_to_player_turn (que confirma o default), este
// PARA no picker com o cursor no pre-selecionado.
void pump_to_actor_picker(BattleScene& scene) {
    if (scene.is_intro()) {
        scene.start_combat();
    }
    for (int i = 0; i < 400; ++i) {
        if (scene.combat_over() || scene.is_choosing_actor()) {
            return;
        }
        scene.skip();
        scene.update(1.0f / 60.0f);
    }
}

// Conduz o combate ATE o ator de id `want` ser o ATIVO (ou o combate acabar). Joga os
// turnos de jogador com um ataque simples e bombeia o ritmo nos turnos de inimigo, entao a
// vez circula por TODA a fila (todo ator age em uma rodada). Robusto a reordenacao por
// Haste: so depende de a vez chegar em `want`. true se chegou; false se o combate acabou
// antes. Usado pra testar a fila CTB na vez de um ator de SPD BAIXA (Jaci).
bool pump_until_active(BattleScene& scene, const std::string& want) {
    if (scene.is_intro()) {
        scene.start_combat();
    }
    for (int i = 0; i < 600; ++i) {
        const auto* a = scene.active_actor();
        if (a != nullptr && a->id() == want) {
            return true;
        }
        if (scene.combat_over()) {
            return false;
        }
        if (scene.waiting_player_input()) {
            player_attack(scene);        // resolve o turno do jogador ativo
        } else {
            scene.skip();
            scene.update(1.0f / 60.0f);  // bombeia 1 beat do turno de inimigo
        }
    }
    return false;
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

// ---- FIX da fila CTB (bug reportado pelo lider ao vivo 2026-07-01, M5) --------------
// Dois sintomas na fila CTB (topo): (1) o destaque de "ativo" ficava PRESO no slot de
// MAIOR SPD porque a fila mostrava os top-5 FIXOS por SPD (order[0..4]) em vez de
// rotacionar pra comecar em quem joga AGORA; (2) um membro de SPD baixa (Jaci, rank 5 de
// 7) NUNCA aparecia (caia fora da janela de 5). O fix: a fila e uma JANELA ROTACIONADA
// que comeca no ator ATIVO (ctb_window()[0] == active_actor()) e segue a ordem de jogo
// com WRAP. Prova via ctb_window() - a MESMA fonte que o render consome.

TEST_CASE("CTB fix: na vez da Jaci (SPD baixa) ela LIDERA a fila e fica VISIVEL",
          "[battle_scene][ctb]") {
    BattleScene scene;
    // Circula a fila ate a vez da Jaci (SPD 7, o mais baixo da party). Imune a reordenacao
    // por Haste do Caua: so depende de a vez chegar nela (toda a fila age numa rodada).
    REQUIRE(pump_until_active(scene, "jaci"));
    const auto* active = scene.active_actor();
    REQUIRE(active != nullptr);
    REQUIRE(active->id() == "jaci");

    const auto window = scene.ctb_window();
    // Janela cheia = min(fila, 5) celulas, sem buracos.
    REQUIRE(window.size() ==
            static_cast<std::size_t>(std::min(scene.queue_len(), 5)));
    // SINTOMA 1 (destaque preso no maior SPD): a 1a celula da fila e o ATOR ATIVO (Jaci),
    // nao o de maior SPD. Antes do fix, window[0] era o topo por SPD (nunca a Jaci) -> RED.
    REQUIRE(window[0] == active);
    // SINTOMA 2 (Jaci nunca aparece): a Jaci ESTA na janela visivel. Antes do fix a janela
    // eram os top-5 fixos por SPD e a Jaci (rank 5 de 7) caia fora -> RED.
    bool jaci_visible = false;
    for (const auto* a : window) {
        if (a != nullptr && a->id() == "jaci") {
            jaci_visible = true;
        }
    }
    REQUIRE(jaci_visible);
    // WRAP sem duplicar: com 7 atores e janela de 5, nenhum ator se repete (a rotacao da no
    // maximo uma volta parcial, nunca reinclui o slot 0).
    for (std::size_t i = 0; i < window.size(); ++i) {
        for (std::size_t j = i + 1; j < window.size(); ++j) {
            REQUIRE(window[i] != window[j]);
        }
    }
}

TEST_CASE("CTB fix: o 1o da fila ACOMPANHA o ator ativo (rotaciona por atores distintos)",
          "[battle_scene][ctb]") {
    BattleScene scene;
    // A lideranca da fila deve seguir de quem e a VEZ, por atores DIFERENTES (nao travar no
    // de maior SPD). Alcanca 3 party-members distintos (SPD alto->baixo) e confirma que cada
    // um LIDERA a fila (window[0]) no seu turno. Antes do fix, window[0] era SEMPRE o topo por
    // SPD (order[0]) -> falha no 1o alvo cujo turno nao e o do maior SPD.
    for (const char* want : {"caua", "gus", "jaci"}) {
        REQUIRE(pump_until_active(scene, want));
        const auto* active = scene.active_actor();
        REQUIRE(active != nullptr);
        REQUIRE(active->id() == want);
        const auto window = scene.ctb_window();
        REQUIRE_FALSE(window.empty());
        REQUIRE(window[0] == active);  // cada ator distinto lidera a fila na sua vez
    }
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
    // Limpa os floaters ANTIGOS (dos turnos de inimigo ja animados): na espera do
    // jogador o update so envelhece/poda (o pacing espera o menu). Baseline ZERO -
    // necessario pos-W2 porque o windup do ataque (0.7s) podaria floaters antigos e
    // o delta bruto (size > before) ficaria falso mesmo com o floater novo spawnado.
    scene.update(kFloaterLifeSeconds + 0.1f);
    REQUIRE(scene.floaters().empty());

    player_attack(scene);  // windup -> CONTATO: o floater nasce na resolucao
    REQUIRE(scene.floaters().size() > 0);
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
    // ENCARAR: o jogador comeca o combate. FIX W1: o 1o beat e o RESPIRO INICIAL
    // (WaitingDelay), NAO o anuncio - o 1o turno de inimigo ganha o mesmo delay de entrada
    // dos demais. Nada resolveu ainda (HP intacto, sem log, sem floater).
    scene.start_combat();
    scene.update(0.01f);  // respiro inicial (ainda nao anunciou)
    REQUIRE(scene.pacing_state() == PacingState::WaitingDelay);
    REQUIRE(scene.machine().log().empty());
    // Passado o respiro, entra no BEAT 1 (anuncio): ainda SEM resolver.
    scene.update(kPacingStepDelaySeconds + 0.01f);
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
    // FIX W1: o 1o beat e o RESPIRO INICIAL (WaitingDelay), depois o anuncio. Passa o
    // respiro pra chegar ao BEAT 1 (anuncio): o 1o turno de inimigo ANUNCIA (sem resolver
    // nada). Nenhuma acao logada ainda - o ataque NAO foi feito (corrige "ja feito").
    scene.update(kPacingStepDelaySeconds + 0.01f);
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
    scene.update(kPacingStepDelaySeconds + 0.001f);  // passa o respiro inicial -> BEAT 1
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

TEST_CASE("pacing FIX W1: o 1o ataque inimigo tem RESPIRO INICIAL antes do anuncio/golpe",
          "[battle_scene]") {
    // Lider no display: "ao Encarar, se o 1o a agir e um inimigo, o golpe resolve rapido
    // demais - ja comecei apanhando". Causa: o 1o turno NAO tinha o respiro de entrada que
    // todo turno subsequente tem (a pausa pos-resolucao do anterior). Fix: begin_combat
    // entra no mesmo delay (kPacingStepDelaySeconds) ANTES do 1o anuncio. Esta prova crava:
    //   (1) no INSTANTE do start_combat (dt minusculo) NINGUEM levou dano - o HP do player
    //       nao muda junto do Encarar (era o sintoma); estamos no RESPIRO (WaitingDelay),
    //       ainda NAO no anuncio;
    //   (2) o golpe so conecta DEPOIS do respiro + anuncio (>= kPacingStepDelaySeconds +
    //       kPacingAnnounceSeconds de tempo simulado), igual aos demais turnos.
    BattleScene scene;
    // O 1o ator do demo e um inimigo (maior SPD) - o cenario da queixa.
    REQUIRE(scene.active_actor() != nullptr);
    REQUIRE_FALSE(scene.active_actor()->is_player_side());

    // HP total da party ANTES de Encarar (soma - robusto a qual player e o alvo).
    auto party_hp_total = [&scene]() {
        int t = 0;
        for (const auto* a : scene.machine().queue().order()) {
            if (a != nullptr && a->is_player_side()) {
                t += a->hp();
            }
        }
        return t;
    };
    const int hp_before = party_hp_total();

    scene.start_combat();       // ENCARAR
    scene.update(0.01f);        // INSTANTE do Encarar (dt minusculo)
    // (1) nada resolveu junto do Encarar: HP da party intacto, log vazio, e estamos no
    // RESPIRO (WaitingDelay), NAO no anuncio.
    REQUIRE(scene.pacing_state() == PacingState::WaitingDelay);
    REQUIRE(scene.machine().log().empty());
    REQUIRE(party_hp_total() == hp_before);

    // Completa o respiro -> entra no ANUNCIO (BEAT 1): ainda SEM resolver (party intacta).
    scene.update(kPacingStepDelaySeconds);
    REQUIRE(scene.pacing_state() == PacingState::AnnouncingEnemy);
    REQUIRE(scene.machine().log().empty());
    REQUIRE(party_hp_total() == hp_before);

    // (2) so DEPOIS do anuncio (BEAT 2) o 1o golpe inimigo resolve.
    scene.update(kPacingAnnounceSeconds + 0.1f);
    REQUIRE(scene.machine().log().size() >= 1);
    REQUIRE(party_hp_total() < hp_before);  // agora sim a party levou o golpe
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

TEST_CASE("mira teclas: 1-9 miram DIRETO o N-esimo inimigo e confirmam na hora",
          "[battle_scene][mira]") {
    // Item 3 do lote W1 (pedido do lider): espelha as teclas 1/2/3 do picker de ator para a
    // MIRA - tecla N = mira E confirma o N-esimo inimigo miravel (1-based, ordem de fila).
    BattleScene scene;
    pump_to_player_turn(scene);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();  // entra na mira (NAO resolve)
    REQUIRE(scene.is_aiming());
    REQUIRE(scene.aim_count() >= 2);  // a demo tem 4 inimigos vivos miraveis
    // aim_candidates_ segue a ordem de fila dos inimigos vivos == alive_enemy_at.
    gus::domain::combat::CombatActor* second = alive_enemy_at(scene, 1);  // tecla "2"
    REQUIRE(second != nullptr);
    const int hp_before = second->hp();
    const std::size_t log_before = scene.machine().log().size();

    scene.aim_hotkey(2);  // mira+confirma o 2o miravel na hora (sem navegar/Enter)

    REQUIRE_FALSE(scene.is_aiming());  // confirmou IMEDIATAMENTE (o windup ja partiu)
    pump_player_strike(scene);         // W2: a resolucao vem no CONTATO da aproximacao
    REQUIRE(scene.machine().log().size() > log_before);   // o golpe resolveu
    REQUIRE(second->hp() < hp_before);                    // o 2o inimigo (escolhido) levou o dano
}

TEST_CASE("mira teclas: numero SEM inimigo miravel e no-op (nao mira, nao confirma)",
          "[battle_scene][mira]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    REQUIRE(scene.is_aiming());
    const int n = scene.aim_count();
    const auto* cursor_before = scene.aim_target();
    const std::size_t log_before = scene.machine().log().size();

    scene.aim_hotkey(n + 1);  // "apertar N+1" com so N miraveis: fora de faixa
    REQUIRE(scene.is_aiming());                            // NAO confirmou (segue mirando)
    REQUIRE(scene.aim_target() == cursor_before);          // cursor intacto
    REQUIRE(scene.machine().log().size() == log_before);   // nada resolveu

    scene.aim_hotkey(0);      // 0 e invalido (o atalho e 1-based)
    REQUIRE(scene.is_aiming());
    REQUIRE(scene.aim_target() == cursor_before);
    REQUIRE(scene.machine().log().size() == log_before);
}

TEST_CASE("mira teclas: aim_hotkey fora do modo-mira e no-op", "[battle_scene][mira]") {
    // Guarda de modo (espelha actor_picker_hotkey): sem mira ativa, a tecla numerica nao faz
    // nada (nem entra na mira, nem resolve). No menu de verbos (fora da mira) e inerte.
    BattleScene scene;
    pump_to_player_turn(scene);
    REQUIRE_FALSE(scene.is_aiming());
    const std::size_t log_before = scene.machine().log().size();
    scene.aim_hotkey(1);
    REQUIRE_FALSE(scene.is_aiming());
    REQUIRE(scene.machine().log().size() == log_before);
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
    pump_player_strike(scene);  // W2: a resolucao vem no CONTATO da aproximacao

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

// ----- Previa de dano no modo-mira (feedback do lider): SO em [Atacar] -----
// A cena LE preview_basic_attack_damage do motor e desenha "-N" no cluster de info da mira
// (nome + HP + fraqueza). O badge standalone e EXATAMENTE "-N": linhas de log sao strings
// completas ("a -> b -N") e floaters de dano sao "N" (sem sinal), entao "-N" isola o badge.

TEST_CASE("mira [Atacar]: render mostra a PREVIA de dano ('-N') do alvo mirado",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    REQUIRE(scene.is_aiming());
    const auto* attacker = scene.active_actor();
    const auto* target = scene.aim_target();
    REQUIRE(attacker != nullptr);
    REQUIRE(target != nullptr);

    // Numero = o MESMO do motor (a cena NUNCA recalcula regra; so LE + desenha).
    const int dano = scene.machine().preview_basic_attack_damage(*attacker, *target);
    REQUIRE(dano >= 1);  // piso kMinDamage; a demo nao tem Shield no start
    const std::string badge = "-" + std::to_string(dano);

    CountingRenderer r;
    scene.render(r, 960.0f, 540.0f);
    bool found = false;
    for (const auto& t : r.texts) {
        if (t.text == badge) {
            found = true;
        }
    }
    REQUIRE(found);
}

TEST_CASE("mira [Atacar]: a previa REFLETE o Shield do alvo AO VIVO (absorve tudo -> -0)",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    // Alvo pre-selecionado (D3 b) = front da fila = alive_enemy_at(0). Ponteiro MUTAVEL pra
    // montar o cenario de Shield (a mira so LE o alvo).
    gus::domain::combat::CombatActor* target = alive_enemy_at(scene, 0);
    REQUIRE(target != nullptr);

    // (1) baseline SEM shield: numero cru (>= 1).
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    REQUIRE(scene.aim_target() == target);
    const auto* attacker = scene.active_actor();
    REQUIRE(attacker != nullptr);
    const int base = scene.machine().preview_basic_attack_damage(*attacker, *target);
    REQUIRE(base >= 1);
    scene.aim_cancel();  // volta ao menu SEM consumir o turno (mesmo ator ativo)

    // (2) Shield que absorve TUDO: a previa deve cair pra 0 (a UI reflete a reducao AO VIVO).
    target->add_status(gus::domain::combat::StatusEffect{
        gus::domain::combat::StatusId::Shield, /*magnitude=*/999, /*duration=*/1,
        gus::domain::combat::StackRule::Replace,
        gus::domain::combat::CardFamily::Eletrico});
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    REQUIRE(scene.aim_target() == target);
    REQUIRE(scene.machine().preview_basic_attack_damage(*attacker, *target) == 0);

    CountingRenderer r;
    scene.render(r, 960.0f, 540.0f);
    bool found_zero = false;
    bool found_base = false;
    for (const auto& t : r.texts) {
        if (t.text == "-0") {
            found_zero = true;
        }
        if (t.text == "-" + std::to_string(base)) {
            found_base = true;
        }
    }
    REQUIRE(found_zero);        // badge agora "-0" (Shield absorve tudo)
    REQUIRE_FALSE(found_base);  // o numero MUDOU vs o baseline: previa VIVA, nao estatica
}

TEST_CASE("mira [Scan]: NAO mostra previa de dano (Scan nao causa dano)",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    select_verb(scene, BattleVerb::Scan);
    scene.menu_confirm();
    REQUIRE(scene.is_aiming());
    const auto* attacker = scene.active_actor();
    const auto* target = scene.aim_target();
    REQUIRE(attacker != nullptr);
    REQUIRE(target != nullptr);

    // O badge "-N" (N = dano do ataque basico) NAO deve ser desenhado no modo Scan.
    const int dano = scene.machine().preview_basic_attack_damage(*attacker, *target);
    const std::string badge = "-" + std::to_string(dano);

    CountingRenderer r;
    scene.render(r, 960.0f, 540.0f);
    for (const auto& t : r.texts) {
        REQUIRE(t.text != badge);
    }
}

// ---- MOUSE (Incremento A2): hit-test de clique no inimigo (mira) + aim_select ----
// aim_index_at_arena/aim_select sao a PONTE do clique de mouse: o host converte o clique
// (px) pra coordenadas de MUNDO logico (960x540) e pergunta "que inimigo miravel esta aqui".

TEST_CASE("mouse-mira: o CENTRO do slot de cada inimigo mapeia pro seu indice de mira",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();  // entra na mira; aim_candidates_ = inimigos vivos em ordem de fila
    REQUIRE(scene.is_aiming());
    REQUIRE(scene.aim_count() == scene.enemy_count());

    // Os slots de inimigo do arena_layout seguem a MESMA ordem (i-esimo vivo -> i-esimo slot),
    // entao o centro do slot i deve casar o candidato de mira i.
    const gus::app::screens::ArenaLayout arena = gus::app::screens::arena_layout(
        scene.party_count(), scene.enemy_count(), scene.gus_party_index());
    for (int i = 0; i < scene.aim_count(); ++i) {
        const gus::core::spatial::Rect& s = arena.enemies[static_cast<std::size_t>(i)].rect;
        const float cx = s.x + s.w * 0.5f;
        const float cy = s.y + s.h * 0.5f;
        REQUIRE(scene.aim_index_at_arena(cx, cy) == i);
    }
}

TEST_CASE("mouse-mira: clique fora de qualquer inimigo devolve -1 (nao erra alvo)",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    REQUIRE(scene.is_aiming());

    // Canto superior-esquerdo (area do cockpit/fila, longe da coluna de inimigos): -1.
    REQUIRE(scene.aim_index_at_arena(2.0f, 2.0f) == -1);
    // Sobre a COLUNA DA PARTY (nao e inimigo miravel): -1. Usa o slot do 1o party vivo.
    const gus::app::screens::ArenaLayout arena = gus::app::screens::arena_layout(
        scene.party_count(), scene.enemy_count(), scene.gus_party_index());
    const gus::core::spatial::Rect& p0 = arena.party[0].rect;
    REQUIRE(scene.aim_index_at_arena(p0.x + p0.w * 0.5f, p0.y + p0.h * 0.5f) == -1);
}

TEST_CASE("mouse-mira: aim_select pousa a mira DIRETO no indice (clique = mirar alvo)",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    REQUIRE(scene.is_aiming());
    const int n = scene.aim_count();
    REQUIRE(n >= 2);

    // aim_select(i) pousa no i-esimo inimigo vivo em ordem de fila == alive_enemy_at(scene,i)
    // (a MESMA base de aim_candidates_). Ponteiro identico prova o mapeamento exato.
    for (int i = 0; i < n; ++i) {
        scene.aim_select(i);
        REQUIRE(scene.aim_target() == alive_enemy_at(scene, i));
    }
    // aim_select fora de faixa e no-op (mantem o alvo corrente).
    scene.aim_select(0);
    const auto* t0 = scene.aim_target();
    scene.aim_select(-1);
    REQUIRE(scene.aim_target() == t0);
    scene.aim_select(n + 5);
    REQUIRE(scene.aim_target() == t0);
}

TEST_CASE("mouse-mira: FORA do modo-mira, aim_index_at_arena e sempre -1 e aim_select no-op",
          "[battle_scene]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    REQUIRE_FALSE(scene.is_aiming());
    // Sem mira: nenhum candidato -> -1 em qualquer ponto (mesmo sobre um inimigo).
    const gus::app::screens::ArenaLayout arena = gus::app::screens::arena_layout(
        scene.party_count(), scene.enemy_count(), scene.gus_party_index());
    const gus::core::spatial::Rect& e0 = arena.enemies[0].rect;
    REQUIRE(scene.aim_index_at_arena(e0.x + e0.w * 0.5f, e0.y + e0.h * 0.5f) == -1);
    // aim_select fora da mira nao entra em mira nem quebra nada.
    scene.aim_select(0);
    REQUIRE_FALSE(scene.is_aiming());
    REQUIRE(scene.aim_target() == nullptr);
}

// ---- INCREMENTO B (UI): comando-livre 1B / escolha de ator (combat.md §4.1) ----------
// A vez do BLOCO da party ABRE numa ESCOLHA DE ATOR quando ha >1 elegivel: o jogador comanda
// QUAL membro age (a SPD so SUGERE o pre-selecionado). O motor 1B (pending_party_actors/
// preselected_party_actor/select_party_actor) ja esta pronto e verificado; aqui provamos a
// camada de UI: entrada no modo, navegacao, pre-selecao por SPD, override, mouse, teclas 1/2/3.

TEST_CASE("picker: a vez da party ABRE na escolha de ator (>1 elegivel), ANTES do menu",
          "[battle_scene][picker]") {
    BattleScene scene;
    pump_to_actor_picker(scene);
    REQUIRE(scene.is_choosing_actor());
    REQUIRE(scene.actor_pick_count() == 3);  // demo: party de 3 abre os 3 elegiveis
    // E a vez do lado do jogador, mas o TURNO ainda nao comecou (begin deferido): o menu de
    // verbos NAO opera enquanto se escolhe o ator.
    REQUIRE(scene.current_actor_is_player());
    const auto* before = scene.active_actor();
    scene.menu_confirm();  // no-op durante a escolha (begin deferido)
    REQUIRE_FALSE(scene.is_aiming());
    REQUIRE(scene.is_choosing_actor());
    REQUIRE(scene.active_actor() == before);
    // Banner distinto de "sua vez": "escolha quem age".
    REQUIRE(scene.turn_banner_key() ==
            std::string_view("COMBAT_BANNER_CHOOSE_ACTOR"));
}

TEST_CASE("picker: o pre-selecionado e o de MAIOR SPD (SPD sugere, jogador escolhe)",
          "[battle_scene][picker]") {
    BattleScene scene;
    pump_to_actor_picker(scene);
    REQUIRE(scene.is_choosing_actor());
    const auto* target = scene.actor_pick_target();
    REQUIRE(target != nullptr);
    // == o pre-selecionado do MOTOR (front de pending_party_actors, maior SPD).
    REQUIRE(target == scene.machine().preselected_party_actor());
    // Na demo o maior SPD da party e o Caua (12 > Gus 9 > Jaci 7).
    REQUIRE(target->id() == "caua");
    // Ninguem na lista tem SPD maior que o pre-selecionado (front = maior SPD).
    for (const auto* a : scene.actor_choices()) {
        REQUIRE(a->spd() <= target->spd());
    }
}

TEST_CASE("picker: navegar move o cursor entre os elegiveis (WRAP)",
          "[battle_scene][picker]") {
    BattleScene scene;
    pump_to_actor_picker(scene);
    const int n = scene.actor_pick_count();
    REQUIRE(n == 3);
    const auto* c0 = scene.actor_pick_target();
    scene.actor_picker_move(+1);
    REQUIRE(scene.actor_pick_target() != c0);  // mudou de membro
    for (int i = 0; i < n - 1; ++i) {
        scene.actor_picker_move(+1);
    }
    REQUIRE(scene.actor_pick_target() == c0);  // volta completa (N passos)
    scene.actor_picker_move(-1);
    REQUIRE(scene.actor_pick_target() != c0);  // wrap pra tras
}

TEST_CASE("picker: confirmar o PRE-SELECIONADO inicia o turno dele (menu pronto)",
          "[battle_scene][picker]") {
    BattleScene scene;
    pump_to_actor_picker(scene);
    const auto* pre = scene.actor_pick_target();
    REQUIRE(pre != nullptr);
    const std::string pre_id = pre->id();

    scene.actor_picker_confirm();

    REQUIRE_FALSE(scene.is_choosing_actor());
    REQUIRE(scene.active_actor() != nullptr);
    REQUIRE(scene.active_actor()->id() == pre_id);   // o pre-selecionado virou o ATIVO
    REQUIRE(scene.active_actor()->ap() == 3);        // begin_turn rodou (AP recarregado)
    REQUIRE(scene.current_actor_is_player());
    // O menu de verbos volta a operar (nao mais deferido): Atacar abre a mira normalmente.
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    REQUIRE(scene.is_aiming());
}

TEST_CASE("picker: escolher um NAO-pre-selecionado troca quem age (OVERRIDE 1B)",
          "[battle_scene][picker]") {
    BattleScene scene;
    pump_to_actor_picker(scene);
    const auto* pre = scene.actor_pick_target();
    REQUIRE(pre != nullptr);
    REQUIRE(scene.actor_pick_count() >= 2);
    const std::string pre_id = pre->id();

    // Poe o cursor num elegivel DIFERENTE do pre-selecionado (indice 1) e confirma.
    scene.actor_picker_select(1);
    const auto* chosen = scene.actor_pick_target();
    REQUIRE(chosen != nullptr);
    REQUIRE(chosen != pre);  // realmente outro membro
    const std::string chosen_id = chosen->id();

    scene.actor_picker_confirm();

    // O motor recebeu select_party_actor(chosen): o ATIVO agora e o ESCOLHIDO, NAO o
    // pre-selecionado de maior SPD. Prova o comando livre (override da sugestao por SPD).
    REQUIRE_FALSE(scene.is_choosing_actor());
    REQUIRE(scene.active_actor() != nullptr);
    REQUIRE(scene.active_actor()->id() == chosen_id);
    REQUIRE(scene.active_actor()->id() != pre_id);

    // E o PRE-SELECIONADO (que NAO agiu) segue PENDENTE: apos o escolhido agir, a proxima
    // escolha da party volta a inclui-lo (nao foi pulado nem consumido).
    player_attack(scene);  // o escolhido (ja e o ativo) age
    pump_to_actor_picker(scene);
    if (scene.is_choosing_actor()) {
        bool pre_still_pending = false;
        for (const auto* a : scene.actor_choices()) {
            if (a->id() == pre_id) {
                pre_still_pending = true;
            }
        }
        REQUIRE(pre_still_pending);
    }
}

TEST_CASE("picker mouse: hit-test casa o slot do membro; clique confirma nele (override)",
          "[battle_scene][picker]") {
    BattleScene scene;
    pump_to_actor_picker(scene);
    REQUIRE(scene.actor_pick_count() >= 2);
    const auto choices = scene.actor_choices();
    // Alvo do clique = o 2o elegivel (indice 1, != pre-selecionado) pra o clique tambem
    // provar o override. Acha o SLOT dele na arena pela MESMA ordem que o render usa (alive
    // party em queue().order()), como arena_rect_for_actor faz internamente.
    const std::string want = choices[1]->id();
    const gus::app::screens::ArenaLayout arena = gus::app::screens::arena_layout(
        scene.party_count(), scene.enemy_count(), scene.gus_party_index());
    gus::core::spatial::Rect slot{};
    {
        int k = 0;
        for (const auto* a : scene.machine().queue().order()) {
            if (a == nullptr || !a->is_alive() || !a->is_player_side()) {
                continue;
            }
            if (a->id() == want) {
                slot = arena.party[static_cast<std::size_t>(k)].rect;
                break;
            }
            ++k;
        }
    }
    const float cx = slot.x + slot.w * 0.5f;
    const float cy = slot.y + slot.h * 0.5f;
    // O CENTRO do slot mapeia pro indice 1 em actor_choices_ (a base do clique de mouse).
    REQUIRE(scene.actor_pick_index_at_arena(cx, cy) == 1);
    // Fora de qualquer slot elegivel (canto do cockpit/fila) -> -1 (nao "erra" a escolha).
    REQUIRE(scene.actor_pick_index_at_arena(2.0f, 2.0f) == -1);

    // Clique = select + confirm (mesma filosofia do A2): inicia o turno do membro CLICADO.
    scene.actor_picker_select(scene.actor_pick_index_at_arena(cx, cy));
    scene.actor_picker_confirm();
    REQUIRE_FALSE(scene.is_choosing_actor());
    REQUIRE(scene.active_actor()->id() == want);
}

TEST_CASE("picker mouse: fora do modo, actor_pick_index_at_arena e sempre -1",
          "[battle_scene][picker]") {
    BattleScene scene;
    pump_to_player_turn(scene);   // confirma o picker -> NAO esta mais escolhendo
    REQUIRE_FALSE(scene.is_choosing_actor());
    // Sem escolha ativa: nenhum candidato -> -1 mesmo sobre um slot de party.
    const gus::app::screens::ArenaLayout arena = gus::app::screens::arena_layout(
        scene.party_count(), scene.enemy_count(), scene.gus_party_index());
    const gus::core::spatial::Rect& p0 = arena.party[0].rect;
    REQUIRE(scene.actor_pick_index_at_arena(p0.x + p0.w * 0.5f,
                                            p0.y + p0.h * 0.5f) == -1);
    // actor_picker_select/move fora do modo sao no-op (nao entram em escolha).
    scene.actor_picker_select(0);
    scene.actor_picker_move(+1);
    REQUIRE_FALSE(scene.is_choosing_actor());
    REQUIRE(scene.actor_pick_target() == nullptr);
}

TEST_CASE("picker teclas: 1/2/3 escolhem DIRETO o N-esimo elegivel e confirmam na hora",
          "[battle_scene][picker]") {
    BattleScene scene;
    pump_to_actor_picker(scene);
    REQUIRE(scene.actor_pick_count() == 3);
    const auto choices = scene.actor_choices();
    const std::string second = choices[1]->id();  // o 2o elegivel (tecla "2")

    scene.actor_picker_hotkey(2);

    REQUIRE_FALSE(scene.is_choosing_actor());       // confirmou IMEDIATAMENTE (sem Enter)
    REQUIRE(scene.active_actor() != nullptr);
    REQUIRE(scene.active_actor()->id() == second);  // o 2o virou o ativo
}

TEST_CASE("picker teclas: numero SEM elegivel e no-op (nao seleciona, nao confirma)",
          "[battle_scene][picker]") {
    BattleScene scene;
    pump_to_actor_picker(scene);
    const int n = scene.actor_pick_count();
    REQUIRE(n == 3);
    const auto* cursor_before = scene.actor_pick_target();

    scene.actor_picker_hotkey(n + 1);  // "apertar 4" com so 3 elegiveis
    REQUIRE(scene.is_choosing_actor());                    // NAO confirmou
    REQUIRE(scene.actor_pick_target() == cursor_before);   // cursor intacto

    scene.actor_picker_hotkey(0);      // 0 e invalido (atalho e 1-based)
    REQUIRE(scene.is_choosing_actor());
    REQUIRE(scene.actor_pick_target() == cursor_before);
}

TEST_CASE("picker: com 1 SO elegivel a cena AUTO-inicia (sem picker) - decisao >1",
          "[battle_scene][picker]") {
    // DECISAO documentada: o picker so abre quando ha >1 elegivel (ha escolha a fazer). Com 1
    // so pendente (o ULTIMO membro da party na rodada), a cena auto-inicia o turno sem friccao.
    BattleScene scene;
    pump_to_actor_picker(scene);          // picker com 3
    REQUIRE(scene.actor_pick_count() == 3);
    scene.actor_picker_confirm();         // inicia o 1o (pre-selecionado)
    player_attack(scene);                 // 1o age
    pump_to_actor_picker(scene);          // restam 2 -> outro picker
    REQUIRE(scene.is_choosing_actor());
    REQUIRE(scene.actor_pick_count() == 2);
    scene.actor_picker_confirm();         // inicia o 2o
    player_attack(scene);                 // 2o age
    // Resta 1 (ultimo da party): a cena NAO abre picker; auto-inicia esse turno direto no menu.
    pump_to_player_turn(scene);
    if (!scene.combat_over()) {
        REQUIRE_FALSE(scene.is_choosing_actor());  // sem escolha pro unico elegivel
        REQUIRE(scene.current_actor_is_player());
        REQUIRE(scene.waiting_player_input());      // menu direto (turno ja comecou)
    }
}

TEST_CASE("picker: destaque MULTIMODAL na arena (contorno + badge numerado + nome), nao so cor",
          "[battle_scene][picker]") {
    BattleScene scene;
    pump_to_actor_picker(scene);
    REQUIRE(scene.is_choosing_actor());
    // Poe o cursor num membro que NAO e o ativo/default do cockpit (indice 1), pra o NOME so
    // poder vir do realce da escolha (o cockpit escreve o nome do ativo/default = indice 0).
    scene.actor_picker_move(+1);
    const auto* cursor = scene.actor_pick_target();
    REQUIRE(cursor != nullptr);

    CountingRenderer r;
    scene.render(r, 960.0f, 540.0f);  // sem translator: o banner nao escreve nome

    // (NOME) o membro sob o cursor tem o nome desenhado como TEXTO (pista WCAG, nao so cor).
    bool found_name = false;
    for (const auto& t : r.texts) {
        if (t.text == cursor->display_name()) {
            found_name = true;
        }
    }
    REQUIRE(found_name);
    // (BADGE = tecla-atalho) os digitos "1"/"2"/"3" aparecem como texto (multimodal + ensina
    // a tecla). "1" so pode vir do badge do 1o elegivel (numeros do cockpit sao "hp/max").
    bool found_badge = false;
    for (const auto& t : r.texts) {
        if (t.text == "1" || t.text == "2" || t.text == "3") {
            found_badge = true;
        }
    }
    REQUIRE(found_badge);
    // (CONTORNO) o realce da escolha ADICIONA outlines: confirmando (sai do modo) e
    // re-renderizando, o numero de outlines CAI (contorno = pista de FORMA, nao so cor).
    const std::size_t outlines_choosing = r.outlines.size();
    scene.actor_picker_confirm();
    CountingRenderer r2;
    scene.render(r2, 960.0f, 540.0f);
    REQUIRE(outlines_choosing > r2.outlines.size());
}

TEST_CASE("picker: e SO da party - o enemy-block assume sozinho (nunca abre escolha)",
          "[battle_scene][picker]") {
    // Roda a batalha inteira jogando os turnos do jogador (player_attack atravessa qualquer
    // picker) e bombeando os de inimigo. INVARIANTE: is_choosing_actor() so pode ser true
    // quando o ATIVO e da party; em NENHUM turno de INIMIGO a escolha abre (enemy-block
    // automatico, ja cuidado pelo motor). Prova que a UI de escolha e exclusiva da party.
    BattleScene scene;
    scene.start_combat();  // ENCARAR: sai da abertura parada (input-gated) e libera o 1o turno
    int guard = 0;
    int enemy_turns_seen = 0;
    while (!scene.combat_over() && guard++ < 4000) {
        if (scene.is_choosing_actor()) {
            REQUIRE(scene.current_actor_is_player());  // escolha => ativo e da party
        }
        if (scene.waiting_player_input()) {
            player_attack(scene);  // confirma o picker (se houver) + age
        } else {
            const auto* a = scene.active_actor();
            if (a != nullptr && !a->is_player_side() && !scene.is_intro()) {
                REQUIRE_FALSE(scene.is_choosing_actor());  // inimigo NUNCA abre escolha
                ++enemy_turns_seen;
            }
            scene.skip();
            scene.update(1.0f / 60.0f);
        }
    }
    REQUIRE(scene.combat_over());
    REQUIRE(enemy_turns_seen >= 1);  // inimigos agiram (automatico), todos sem picker
}

// ---- W2: ANIMACAO DE COMBATE na cena (battle-anim.md par.2/3) -----------------------
//
// A base funcional: melee desloca-golpeia-volta (par.2.2), hit-react do alvo (par.2.3),
// gancho de magia/projetil dormante (par.2.1), timing pelos beats (par.3.1 - a tabela e
// lei: windup no Beat 1, contato no inicio do Beat 2, recovery no delay do Beat 2).

namespace {

// Alvo MIRADO como ponteiro MUTAVEL (pra ler hp()/add_status no cenario): o aim_target
// e const; achamos o MESMO ator na fila (identidade de ponteiro).
gus::domain::combat::CombatActor* aimed_mutable(BattleScene& scene) {
    for (gus::domain::combat::CombatActor* a : scene.machine().queue().order()) {
        if (a == scene.aim_target()) {
            return a;
        }
    }
    return nullptr;
}

}  // namespace

TEST_CASE("anim W2: confirmar [Atacar] inicia o windup NA HORA e NAO resolve (<100ms)",
          "[battle_scene][anim]") {
    using gus::app::screens::ActorAnimKind;
    BattleScene scene;
    pump_to_player_turn(scene);
    scene.update(kFloaterLifeSeconds + 0.1f);  // some com floaters antigos (baseline 0)
    const auto* self = scene.active_actor();
    REQUIRE(self != nullptr);
    const std::string self_id = self->id();

    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();  // entra na mira
    gus::domain::combat::CombatActor* victim = aimed_mutable(scene);
    REQUIRE(victim != nullptr);
    const int hp_before = victim->hp();
    const std::size_t log_before = scene.machine().log().size();

    scene.aim_confirm();  // COMANDA o golpe: windup parte JA (regra de ouro < 100ms)
    REQUIRE(scene.player_action_in_flight());
    REQUIRE(scene.anim().kind_for(self_id) == ActorAnimKind::MeleeApproach);
    scene.update(1.0f / 60.0f);  // 1 frame (~16ms): resposta VISIVEL (o sprite ja saiu)
    REQUIRE(scene.anim().offset_for(self_id).x > 0.0f);  // party avanca pra DIREITA (Pillar 3)

    // NADA resolveu no windup (par.3.1: a aproximacao e o anuncio; HP intacto).
    REQUIRE(victim->hp() == hp_before);
    REQUIRE(scene.floaters().empty());
    REQUIRE(scene.machine().log().size() == log_before);
}

TEST_CASE("anim W2: o CONTATO resolve (dano + floater juntos) e o atacante VOLTA ao repouso EXATO",
          "[battle_scene][anim]") {
    using gus::app::screens::ActorAnimKind;
    BattleScene scene;
    pump_to_player_turn(scene);
    scene.update(kFloaterLifeSeconds + 0.1f);
    const std::string self_id = scene.active_actor()->id();

    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    gus::domain::combat::CombatActor* victim = aimed_mutable(scene);
    REQUIRE(victim != nullptr);
    const int hp_before = victim->hp();

    scene.aim_confirm();
    pump_player_strike(scene);  // bombeia ate o CONTATO (fim da aproximacao)

    // O contato resolveu: motor aplicou o dano E o floater nasceu NO MESMO instante.
    REQUIRE_FALSE(scene.player_action_in_flight());
    REQUIRE(victim->hp() < hp_before);
    REQUIRE(scene.floaters().size() > 0);
    // Recovery (par.3.1): o atacante VOLTA (Return) e termina EXATAMENTE no repouso -
    // sem ficar deslocado (checklist par.6), dentro do delay do Beat 2.
    REQUIRE(scene.anim().kind_for(self_id) == ActorAnimKind::MeleeReturn);
    for (int i = 0; i < 45; ++i) {  // 0.75s > kPlayerMeleeReturnSeconds (volta do jogador)
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.anim().offset_for(self_id).x == 0.0f);
    REQUIRE(scene.anim().offset_for(self_id).y == 0.0f);
}

TEST_CASE("anim W3.2: aproximacao/volta do JOGADOR sao DESACOPLADAS e mais longas que o "
          "ritmo do inimigo (regressao playtest: corrida rapida/'de-costas')",
          "[battle_scene][anim]") {
    // Regressao (lider 2026-07-02): a corrida de perfil do Gus passava rapido demais
    // (~0.7s: a MESMA janela do anuncio do inimigo, por acaso de implementacao) e
    // "parecia de costas" - aliasing temporal (roda-de-carroca), nao ordem/pose. A cura
    // e DURACAO: o approach/volta do JOGADOR ganharam constantes proprias, mais longas,
    // SEM tocar o Beat 1 / a volta do inimigo (ambos aprovados AO VIVO no W1).
    using gus::app::screens::ActorAnimKind;
    using gus::app::screens::kMeleeReturnSeconds;
    using gus::app::screens::kPlayerMeleeApproachSeconds;
    using gus::app::screens::kPlayerMeleeReturnSeconds;

    // --- Invariantes de tuning (constantes) ---
    // O approach do jogador e mais longo que o Beat 1 do inimigo (mais tempo pra LER).
    REQUIRE(kPlayerMeleeApproachSeconds > kPacingAnnounceSeconds);
    // A volta do jogador e mais longa que a do inimigo MAS cabe no delay do Beat 2 -
    // senao o Gus deslizaria por cima do anuncio do proximo turno.
    REQUIRE(kPlayerMeleeReturnSeconds > kMeleeReturnSeconds);
    REQUIRE(kPlayerMeleeReturnSeconds <= kPacingStepDelaySeconds);
    // REGRESSAO ANTI-TOQUE: o ritmo do inimigo NAO pode ter mudado (aprovado no W1).
    REQUIRE(kPacingAnnounceSeconds == 0.7f);  // Beat 1 anuncio do inimigo, CRU
    REQUIRE(kMeleeReturnSeconds == 0.4f);     // volta do inimigo, CRUA

    // --- Comportamento: o approach do JOGADOR de fato usa a duracao propria ---
    BattleScene scene;
    pump_to_player_turn(scene);
    scene.update(kFloaterLifeSeconds + 0.1f);  // some com floaters antigos
    REQUIRE(scene.active_actor() != nullptr);
    REQUIRE(scene.active_actor()->is_player_side());
    const std::string self_id = scene.active_actor()->id();

    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();  // entra na mira
    scene.aim_confirm();   // o windup (aproximacao) parte AGORA, sem update ainda
    REQUIRE(scene.player_action_in_flight());
    // A fase Approach recem-iniciada (elapsed 0) dura kPlayerMeleeApproachSeconds, NAO
    // kPacingAnnounceSeconds - prova comportamental do desacoplamento (nao pegou a carona).
    const float rem = scene.anim().phase_remaining_seconds(self_id);
    REQUIRE(rem > kPacingAnnounceSeconds);  // > 0.7s: nao e mais a janela do inimigo
    REQUIRE(rem > kPlayerMeleeApproachSeconds - 0.02f);
    REQUIRE(rem < kPlayerMeleeApproachSeconds + 0.02f);

    // --- A volta do jogador usa kPlayerMeleeReturnSeconds e cabe no delay do Beat 2 ---
    pump_player_strike(scene);  // bombeia ate o CONTATO (fim da aproximacao)
    REQUIRE(scene.anim().kind_for(self_id) == ActorAnimKind::MeleeReturn);
    const float ret = scene.anim().phase_remaining_seconds(self_id);
    REQUIRE(ret > kPlayerMeleeReturnSeconds - 0.02f);
    REQUIRE(ret < kPlayerMeleeReturnSeconds + 0.02f);
    // Ao longo do delay INTEIRO do Beat 2 a volta (0.7s < 0.8s) termina no repouso EXATO.
    const int delay_frames = static_cast<int>(kPacingStepDelaySeconds * 60.0f) + 1;
    for (int i = 0; i < delay_frames; ++i) {
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.anim().offset_for(self_id).x == 0.0f);
    REQUIRE(scene.anim().offset_for(self_id).y == 0.0f);
}

TEST_CASE("anim W2: hit-react - o alvo RECUA um tranco (pra tras) e VOLTA a battle-idle",
          "[battle_scene][anim]") {
    using gus::app::screens::ActorAnimKind;
    BattleScene scene;
    pump_to_player_turn(scene);
    scene.update(kFloaterLifeSeconds + 0.1f);

    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    gus::domain::combat::CombatActor* victim = aimed_mutable(scene);
    REQUIRE(victim != nullptr);
    const std::string victim_id = victim->id();

    scene.aim_confirm();
    pump_player_strike(scene);

    // No contato o alvo entra em HIT-REACT (sofrimento + recuo, par.2.3).
    REQUIRE(scene.anim().kind_for(victim_id) == ActorAnimKind::HitReact);
    // Meio do tranco: recuou pra DIREITA (inimigo recua pra tras = pra longe da party).
    for (int i = 0; i < 9; ++i) {  // ~0.15s = pico do seno
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.anim().offset_for(victim_id).x > 0.0f);
    // E VOLTA exata a battle-idle (offset zero) ao fim do tranco.
    for (int i = 0; i < 30; ++i) {
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.anim().offset_for(victim_id).x == 0.0f);
    REQUIRE(scene.anim().kind_for(victim_id) == ActorAnimKind::None);
}

// FALHA sem hit-react (battle-anim.md par.2.3): o GANCHO existe na cena (o hit-react so
// dispara nos canais Common/Crit de parse_hit; Fail/Heal NAO trancam o alvo), mas NAO e
// testavel fim-a-fim HOJE: o ataque BASICO e deterministico (dano minimo >= 1 e o motor
// loga o dano BRUTO pre-Shield) - o canal FALHA so nasce da formula de cartas (combat.md
// par.11), que ainda e placeholder de UI. A classificacao de canal ja e coberta pelos
// testes de parse_hit (battle_floaters_test) e o gate esta documentado em
// spawn_floaters_from_new_logs; o teste fim-a-fim entra junto com o COMPILAR real.

TEST_CASE("anim W2: turno de INIMIGO - aproximacao mora no Beat 1 ANUNCIO; contato no Beat 2",
          "[battle_scene][anim]") {
    using gus::app::screens::ActorAnimKind;
    BattleScene scene;
    scene.start_combat();
    // Chega no 1o ANUNCIO de inimigo (skip encurta o respiro; o anuncio toca inteiro).
    for (int i = 0; i < 600 && scene.pacing_state() != PacingState::AnnouncingEnemy;
         ++i) {
        scene.skip();
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.pacing_state() == PacingState::AnnouncingEnemy);
    const auto* atk = scene.active_actor();
    REQUIRE(atk != nullptr);
    REQUIRE_FALSE(atk->is_player_side());
    const std::string atk_id = atk->id();
    const std::size_t log_before = scene.machine().log().size();

    // MEIO do anuncio: o inimigo ja se DESLOCA (pra ESQUERDA, sem flip) e NADA resolveu
    // (HP intacto, sem floater, sem log de acao) - o telegraph do Pillar 1.
    for (int i = 0; i < 12; ++i) {  // ~0.2s dos 0.7s do anuncio
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.pacing_state() == PacingState::AnnouncingEnemy);
    REQUIRE(scene.anim().offset_for(atk_id).x < 0.0f);
    REQUIRE(scene.machine().log().size() == log_before);
    REQUIRE(scene.floaters().empty());

    // FIM do anuncio -> Beat 2: o CONTATO resolve (log + floater nascem) e o atacante
    // entra na VOLTA (Return) dentro do delay do Beat 2.
    for (int i = 0;
         i < 90 && scene.pacing_state() == PacingState::AnnouncingEnemy; ++i) {
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.machine().log().size() > log_before);
    REQUIRE(scene.floaters().size() > 0);
    REQUIRE(scene.anim().kind_for(atk_id) == ActorAnimKind::MeleeReturn);
}

TEST_CASE("anim W2: skip NAO colapsa o windup do jogador (menu inerte durante o voo)",
          "[battle_scene][anim]") {
    BattleScene scene;
    pump_to_player_turn(scene);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    const std::size_t log_before = scene.machine().log().size();
    scene.aim_confirm();
    REQUIRE(scene.player_action_in_flight());

    // skip (acelera delays) NAO resolve o golpe em voo: o windup toca seu tempo proprio
    // (mesma regra do anuncio do inimigo, par.3.2 - sem "ataque colado/duplo").
    scene.skip();
    scene.update(1.0f / 60.0f);
    REQUIRE(scene.player_action_in_flight());
    REQUIRE(scene.machine().log().size() == log_before);

    // Menu INERTE durante o voo: nao navega nem re-confirma (o turno ja foi comandado).
    const int sel = scene.menu().selected_index();
    scene.menu_move(+1);
    REQUIRE(scene.menu().selected_index() == sel);
    scene.menu_confirm();
    REQUIRE(scene.player_action_in_flight());  // nao disparou outra acao

    pump_player_strike(scene);  // termina limpo
    REQUIRE_FALSE(scene.player_action_in_flight());
}

TEST_CASE("anim W2 (dormante): cast demo conjura NO LUGAR, projetil viaja e o impacto da hit-react",
          "[battle_scene][anim]") {
    using gus::app::screens::ActorAnimKind;
    // ABERTURA (intro): o demo e 100% COSMETICO (zero motor) - valida o esqueleto
    // cast -> viagem -> impacto que o COMPILAR real vai reusar (par.2.1).
    BattleScene scene;
    std::string caster_id, target_id;
    for (const auto* a : scene.machine().queue().order()) {
        if (a == nullptr || !a->is_alive()) {
            continue;
        }
        if (a->is_player_side() && caster_id.empty()) {
            caster_id = a->id();
        }
        if (!a->is_player_side() && target_id.empty()) {
            target_id = a->id();
        }
    }
    REQUIRE_FALSE(caster_id.empty());
    REQUIRE_FALSE(target_id.empty());
    const auto* target = scene.machine().queue().order()[0];  // qualquer; hp via lookup
    (void)target;
    const std::size_t log_before = scene.machine().log().size();

    scene.debug_cast_demo();
    REQUIRE(scene.anim().is_casting(caster_id));
    REQUIRE(scene.anim().offset_for(caster_id).x == 0.0f);  // conjura NO LUGAR

    // Windup termina -> o projetil placeholder spawna e viaja.
    for (int i = 0; i < 120 && scene.anim().projectiles().empty(); ++i) {
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.anim().projectiles().size() == 1);
    REQUIRE(scene.anim().projectiles()[0].target_id == target_id);

    // Impacto: hit-react VISUAL no alvo; ZERO motor (sem dano, sem log).
    for (int i = 0; i < 120 && !scene.anim().projectiles().empty(); ++i) {
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.anim().kind_for(target_id) == ActorAnimKind::HitReact);
    REQUIRE(scene.machine().log().size() == log_before);
    // O tranco volta sozinho a battle-idle.
    for (int i = 0; i < 30; ++i) {
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.anim().kind_for(target_id) == ActorAnimKind::None);
}

TEST_CASE("anim W2: render desenha a bolinha do projetil (placeholder) em voo",
          "[battle_scene][anim]") {
    BattleScene scene;
    scene.debug_cast_demo();
    for (int i = 0; i < 120 && scene.anim().projectiles().empty(); ++i) {
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.anim().projectiles().size() == 1);
    scene.update(1.0f / 60.0f);  // avanca um tico (posicao entre caster e alvo)
    const auto pos = scene.anim().projectiles()[0].position();

    CountingRenderer r;
    scene.render(r, 960.0f, 540.0f);
    // Ha fill(s) CENTRADOS na posicao do projetil (a bolinha = fatias de rect).
    int near = 0;
    for (const auto& f : r.fills) {
        const float cx = f.rect.x + f.rect.w * 0.5f;
        const float cy = f.rect.y + f.rect.h * 0.5f;
        if (std::abs(cx - pos.x) <= 6.0f && std::abs(cy - pos.y) <= 6.0f) {
            ++near;
        }
    }
    REQUIRE(near >= 3);  // as 3 fatias da bolinha
}

// ---- M6 F3: SFX no hit (ADR-011) -----------------------------------------------------
//
// A cena dispara play_sfx no EVENTO DE CONTATO do golpe - o MESMO instante onde o
// hit-react visual + o floater ja nascem (spawn_floaters_from_new_logs pro melee/
// UseCard; o consumo de anim_.take_impacts() em update() pro projetil). Prova, com o
// null-device do AudioEngine (F1, sem hardware): (a) exatamente 1 play_sfx por hit;
// (b) ZERO play_sfx durante Beat 1/anuncio/windup/aproximacao (o golpe ainda nao
// conectou); (c) party E inimigo disparam, cada um no PROPRIO contato. Ancorado no
// evento de contato via as MESMAS transicoes que os testes de hit-react acima usam
// (melee_arrived / Beat 2 resolve / impacto de projetil) - NUNCA no "Beat 2 do pacing"
// como proxy (o CTO revisou: isso dessincronizaria o som do baque visual).

namespace {

// WAV PCM16 mono minimo (tom puro) - mesmo gerador de platform/tests/audio_engine_test.cpp
// e app/src/audio_smoke.cpp: da um SoundId REAL (nao kInvalidSound) pro null-device tocar
// de fato (sfx_play_count so incrementa em play_sfx que TOCOU, nao em no-op).
std::string write_hit_test_tone_wav(const std::filesystem::path& path) {
    constexpr int kSampleRate = 22050;
    constexpr float kDurationS = 0.05f;
    constexpr float kFreqHz = 660.0f;
    const auto num_samples = static_cast<std::uint32_t>(
        static_cast<float>(kSampleRate) * kDurationS);
    std::vector<std::int16_t> samples(num_samples);
    for (std::uint32_t i = 0; i < num_samples; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(kSampleRate);
        const float s = 0.2f * std::sin(2.0f * 3.14159265f * kFreqHz * t);
        samples[i] = static_cast<std::int16_t>(s * 32767.0f);
    }
    const std::uint32_t data_bytes = num_samples * sizeof(std::int16_t);
    const std::uint32_t byte_rate = static_cast<std::uint32_t>(kSampleRate) * 2;
    const std::uint16_t block_align = 2;
    const std::uint16_t bits_per_sample = 16;
    const std::uint32_t riff_size = 36 + data_bytes;

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    auto w32 = [&out](std::uint32_t v) { out.write(reinterpret_cast<const char*>(&v), 4); };
    auto w16 = [&out](std::uint16_t v) { out.write(reinterpret_cast<const char*>(&v), 2); };
    out.write("RIFF", 4);
    w32(riff_size);
    out.write("WAVE", 4);
    out.write("fmt ", 4);
    w32(16);
    w16(1);  // PCM
    w16(1);  // mono
    w32(static_cast<std::uint32_t>(kSampleRate));
    w32(byte_rate);
    w16(block_align);
    w16(bits_per_sample);
    out.write("data", 4);
    w32(data_bytes);
    out.write(reinterpret_cast<const char*>(samples.data()),
              static_cast<std::streamsize>(data_bytes));
    return path.string();
}

}  // namespace

TEST_CASE("F3 M6: contato do JOGADOR dispara 1 play_sfx; windup/aproximacao ficam MUDOS",
          "[battle_scene][audio]") {
    using gus::platform::audio::AudioEngine;
    using gus::platform::audio::kInvalidSound;

    AudioEngine audio(/*device_active=*/false);  // null-device: sem hardware
    const auto tmp =
        std::filesystem::temp_directory_path() / "gusworld_test_hit_sfx_player.wav";
    write_hit_test_tone_wav(tmp);
    const auto hit_id = audio.load_sfx(tmp.string().c_str());
    REQUIRE(hit_id != kInvalidSound);

    BattleScene scene;
    scene.set_audio(&audio, hit_id);
    pump_to_player_turn(scene);
    scene.update(kFloaterLifeSeconds + 0.1f);
    // BASELINE apos pump_to_player_turn: a fila e por SPD (combat.md §4.1) - inimigo(s)
    // mais rapido(s) que a party podem ja ter agido (contato deles) ANTES do 1o turno do
    // jogador. Por isso comparamos DELTA a partir daqui (mesmo padrao dos testes de
    // hp_before/floaters acima), nao um valor absoluto.
    const unsigned int baseline = audio.sfx_play_count();

    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();  // entra na mira: ainda MUDO
    REQUIRE(audio.sfx_play_count() == baseline);

    scene.aim_confirm();  // COMANDA o golpe: windup parte JA (o anuncio/telegraph)
    REQUIRE(scene.player_action_in_flight());
    REQUIRE(audio.sfx_play_count() == baseline);  // windup comandado - AINDA nao conectou
    scene.update(1.0f / 60.0f);                   // 1 frame do voo
    REQUIRE(audio.sfx_play_count() == baseline);  // em pleno voo - segue MUDO (b)

    pump_player_strike(scene);  // bombeia ate o CONTATO (fim da aproximacao)
    REQUIRE_FALSE(scene.player_action_in_flight());
    // o CONTATO disparou EXATAMENTE 1 play_sfx a mais (a).
    REQUIRE(audio.sfx_play_count() == baseline + 1);

    std::filesystem::remove(tmp);
}

TEST_CASE("F3 M6: contato do INIMIGO dispara 1 play_sfx; Beat 1 anuncio fica MUDO",
          "[battle_scene][audio]") {
    using gus::platform::audio::AudioEngine;
    using gus::platform::audio::kInvalidSound;

    AudioEngine audio(/*device_active=*/false);
    const auto tmp =
        std::filesystem::temp_directory_path() / "gusworld_test_hit_sfx_enemy.wav";
    write_hit_test_tone_wav(tmp);
    const auto hit_id = audio.load_sfx(tmp.string().c_str());
    REQUIRE(hit_id != kInvalidSound);

    BattleScene scene;
    scene.set_audio(&audio, hit_id);
    scene.start_combat();
    // Chega no 1o ANUNCIO de inimigo (Beat 1) - mesmo bombeio do teste de hit-react acima.
    for (int i = 0; i < 600 && scene.pacing_state() != PacingState::AnnouncingEnemy;
         ++i) {
        scene.skip();
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.pacing_state() == PacingState::AnnouncingEnemy);
    REQUIRE(audio.sfx_play_count() == 0);

    // MEIO do anuncio: o inimigo ja se desloca (telegraph), NADA resolveu ainda - MUDO (b).
    for (int i = 0; i < 12; ++i) {
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.pacing_state() == PacingState::AnnouncingEnemy);
    REQUIRE(audio.sfx_play_count() == 0);

    // FIM do anuncio -> Beat 2: o CONTATO resolve (log + floater + hit-react nascem
    // JUNTOS) - e AGORA que o play_sfx dispara, exatamente 1 vez (a).
    for (int i = 0;
         i < 90 && scene.pacing_state() == PacingState::AnnouncingEnemy; ++i) {
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(audio.sfx_play_count() == 1);

    std::filesystem::remove(tmp);
}

TEST_CASE("F3 M6: party E inimigo disparam, cada um no PROPRIO contato (2 hits = 2 plays)",
          "[battle_scene][audio]") {
    using gus::platform::audio::AudioEngine;
    using gus::platform::audio::kInvalidSound;

    AudioEngine audio(/*device_active=*/false);
    const auto tmp =
        std::filesystem::temp_directory_path() / "gusworld_test_hit_sfx_both_sides.wav";
    write_hit_test_tone_wav(tmp);
    const auto hit_id = audio.load_sfx(tmp.string().c_str());
    REQUIRE(hit_id != kInvalidSound);

    BattleScene scene;
    scene.set_audio(&audio, hit_id);

    // BASELINE (mesmo motivo do teste anterior): a fila e por SPD - inimigo(s) mais
    // rapidos podem ja ter contatado a party antes do 1o turno do jogador.
    pump_to_player_turn(scene);
    scene.update(kFloaterLifeSeconds + 0.1f);
    const unsigned int baseline = audio.sfx_play_count();

    // (c.1) O JOGADOR ataca: 1 play_sfx a mais no contato dele.
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    scene.aim_confirm();
    pump_player_strike(scene);
    REQUIRE(audio.sfx_play_count() == baseline + 1);

    // (c.2) Bombeia ate o proximo contato (o INIMIGO age, Beat 1 -> Beat 2) - mais 1
    // play_sfx no contato DELE (contador acumula, nao reseta por lado).
    for (int i = 0;
         i < 600 && audio.sfx_play_count() < baseline + 2 && !scene.combat_over(); ++i) {
        scene.skip();
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(audio.sfx_play_count() == baseline + 2);

    std::filesystem::remove(tmp);
}

TEST_CASE("F3 M6: call-site do PROJETIL (impacto) dispara 1 play_sfx; conjuracao/voo mudos",
          "[battle_scene][audio]") {
    using gus::platform::audio::AudioEngine;
    using gus::platform::audio::kInvalidSound;

    AudioEngine audio(/*device_active=*/false);
    const auto tmp =
        std::filesystem::temp_directory_path() / "gusworld_test_hit_sfx_projectile.wav";
    write_hit_test_tone_wav(tmp);
    const auto hit_id = audio.load_sfx(tmp.string().c_str());
    REQUIRE(hit_id != kInvalidSound);

    BattleScene scene;
    scene.set_audio(&audio, hit_id);

    scene.debug_cast_demo();
    REQUIRE(audio.sfx_play_count() == 0);  // conjuracao NO LUGAR: ainda mudo

    // Windup termina -> o projetil spawna e viaja: segue mudo (o call-site so dispara
    // quando anim_.take_impacts() devolve o alvo, ou seja, no FIM do voo).
    for (int i = 0; i < 120 && scene.anim().projectiles().empty(); ++i) {
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.anim().projectiles().size() == 1);
    REQUIRE(audio.sfx_play_count() == 0);

    // Impacto: o call-site do projetil dispara EXATAMENTE 1 play_sfx (mesmo hit_sfx_id_
    // do melee).
    for (int i = 0; i < 120 && !scene.anim().projectiles().empty(); ++i) {
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.anim().projectiles().empty());
    REQUIRE(audio.sfx_play_count() == 1);

    std::filesystem::remove(tmp);
}

TEST_CASE("F3 M6: sem set_audio, a cena roda MUDA (no-op seguro, nunca depende de audio)",
          "[battle_scene][audio]") {
    // Nenhum AudioEngine plugado (default audio_engine_==nullptr) - o mesmo fluxo de
    // ataque do jogador do 1o teste, mas sem crashar nem exigir hardware/engine.
    BattleScene scene;
    pump_to_player_turn(scene);
    scene.update(kFloaterLifeSeconds + 0.1f);
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    scene.aim_confirm();
    pump_player_strike(scene);
    REQUIRE_FALSE(scene.player_action_in_flight());
    SUCCEED("cena sem audio plugado resolveu o combate normalmente (degradacao graciosa)");
}

// ---- W3: SPRITE ANIMADO do ator na arena (battle_sprite_anim + wiring) --------------
//
// A cena recebe um ActorSpriteSet por ator (mesmo padrao de set_portraits: handles
// prontos, cena agnostica de I/O) e o update(dt) dirige o relogio de clip pela FASE
// do BattleAnimDirector. Clips SINTETICOS (handles falsos): zero PNG no teste.

namespace {

// Sprite set sintetico FRONT-ONLY (sem os clipes de perfil): exercita o degrau de
// FALLBACK direcional da cena (run_east/run_west -> run; attack_melee_east ->
// attack_melee) - o cenario de um ator cujas anims de perfil ainda nao existem.
gus::app::screens::ActorSpriteSet synthetic_sprite_set() {
    using gus::app::screens::BattleClipId;
    using gus::app::screens::default_clip_fps;
    using gus::app::screens::default_clip_loop;
    gus::app::screens::ActorSpriteSet set;
    const auto fill = [&](BattleClipId id, int n, unsigned base) {
        auto& c = set.clips[static_cast<std::size_t>(id)];
        for (int i = 0; i < n; ++i) {
            c.frames.push_back(base + static_cast<unsigned>(i));
        }
        c.fps = default_clip_fps(id);
        c.loop = default_clip_loop(id);
    };
    fill(BattleClipId::Idle, 7, 100u);
    fill(BattleClipId::Run, 7, 200u);
    fill(BattleClipId::AttackMelee, 7, 300u);
    fill(BattleClipId::HurtPhysical, 5, 400u);
    return set;
}

// Sprite set sintetico COMPLETO (2026-07-01): front-facing + os 3 clipes de PERFIL
// do melee. Frame count espelha o disco POS-cap do loader (attack_melee_east = 6:
// f6-f8 derivados nao carregam - clip_frame_cap).
gus::app::screens::ActorSpriteSet synthetic_sprite_set_with_profiles() {
    using gus::app::screens::BattleClipId;
    using gus::app::screens::default_clip_fps;
    using gus::app::screens::default_clip_loop;
    gus::app::screens::ActorSpriteSet set = synthetic_sprite_set();
    const auto fill = [&](BattleClipId id, int n, unsigned base) {
        auto& c = set.clips[static_cast<std::size_t>(id)];
        for (int i = 0; i < n; ++i) {
            c.frames.push_back(base + static_cast<unsigned>(i));
        }
        c.fps = default_clip_fps(id);
        c.loop = default_clip_loop(id);
    };
    fill(BattleClipId::RunEast, 9, 500u);
    fill(BattleClipId::RunWest, 9, 600u);
    fill(BattleClipId::AttackMeleeEast, 6, 700u);  // = cap do loader (f0..f5)
    return set;
}

}  // namespace

TEST_CASE("sprite W3: ator com sprite set toca battle_idle no repouso (relogio anda)",
          "[battle_scene][sprite]") {
    using gus::app::screens::BattleClipId;
    BattleScene scene;
    scene.set_actor_sprites("gus", synthetic_sprite_set());

    // Sem sprite set instalado -> nullopt (caua/inimigos seguem no retrato placeholder).
    REQUIRE_FALSE(scene.actor_sprite_frame("caua").has_value());

    scene.update(1.0f / 60.0f);  // instala o clip do repouso
    const auto f0 = scene.actor_sprite_frame("gus");
    REQUIRE(f0.has_value());
    REQUIRE(f0->first == BattleClipId::Idle);
    REQUIRE(f0->second == 0);

    // O relogio anda: idle @ 8 fps -> depois de ~0.3s ja passou do frame 0.
    for (int i = 0; i < 20; ++i) {
        scene.update(1.0f / 60.0f);
    }
    const auto f1 = scene.actor_sprite_frame("gus");
    REQUIRE(f1.has_value());
    REQUIRE(f1->first == BattleClipId::Idle);
    REQUIRE(f1->second > 0);
}

TEST_CASE("sprite W3: melee do GUS com set FRONT-ONLY - fallback direcional degrada "
          "pro run/attack_melee (perfis ausentes)",
          "[battle_scene][sprite]") {
    using gus::app::screens::ActorAnimKind;
    using gus::app::screens::BattleClipId;
    using gus::app::screens::kMeleeSwingSeconds;
    BattleScene scene;
    scene.set_actor_sprites("gus", synthetic_sprite_set());

    // Dirige ate o PICKER (§4.1) e escolhe o GUS (nao o pre-selecionado por SPD).
    pump_to_actor_picker(scene);
    REQUIRE(scene.is_choosing_actor());
    for (int i = 0; i < 4 && (scene.actor_pick_target() == nullptr ||
                              scene.actor_pick_target()->id() != "gus");
         ++i) {
        scene.actor_picker_move(+1);
    }
    REQUIRE(scene.actor_pick_target() != nullptr);
    REQUIRE(scene.actor_pick_target()->id() == "gus");
    scene.actor_picker_confirm();
    REQUIRE(scene.active_actor()->id() == "gus");

    // [Atacar] confirmado: o windup parte JA (W2) e o sprite CORRE (run) na ida.
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    scene.aim_confirm();
    REQUIRE(scene.player_action_in_flight());
    scene.update(1.0f / 60.0f);
    auto f = scene.actor_sprite_frame("gus");
    REQUIRE(f.has_value());
    REQUIRE(f->first == BattleClipId::Run);

    // CAUDA da aproximacao (restam <= swing): o attack_melee one-shot comeca, pra
    // terminar exatamente no contato.
    for (int i = 0; i < 120 && scene.anim().phase_remaining_seconds("gus") >
                                   kMeleeSwingSeconds;
         ++i) {
        scene.update(1.0f / 60.0f);
    }
    scene.update(1.0f / 60.0f);  // tick pos-transicao (o clip troca no update)
    f = scene.actor_sprite_frame("gus");
    REQUIRE(f.has_value());
    REQUIRE(f->first == BattleClipId::AttackMelee);

    // CONTATO resolve e o Return parte: correndo de volta.
    pump_player_strike(scene);
    REQUIRE(scene.anim().kind_for("gus") == ActorAnimKind::MeleeReturn);
    f = scene.actor_sprite_frame("gus");
    REQUIRE(f.has_value());
    REQUIRE(f->first == BattleClipId::Run);

    // A volta termina (0.4s) -> repouso: battle_idle de novo, do frame 0.
    for (int i = 0; i < 45; ++i) {
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.anim().kind_for("gus") == ActorAnimKind::None);
    f = scene.actor_sprite_frame("gus");
    REQUIRE(f.has_value());
    REQUIRE(f->first == BattleClipId::Idle);
}

TEST_CASE("sprite W3.1: melee do GUS com PERFIS - run_east na ida, murro de perfil "
          "CRAVA em <= f5 no contato, run_west na volta",
          "[battle_scene][sprite]") {
    using gus::app::screens::ActorAnimKind;
    using gus::app::screens::BattleClipId;
    using gus::app::screens::kMeleeSwingSeconds;
    BattleScene scene;
    scene.set_actor_sprites("gus", synthetic_sprite_set_with_profiles());

    pump_to_actor_picker(scene);
    REQUIRE(scene.is_choosing_actor());
    for (int i = 0; i < 4 && (scene.actor_pick_target() == nullptr ||
                              scene.actor_pick_target()->id() != "gus");
         ++i) {
        scene.actor_picker_move(+1);
    }
    REQUIRE(scene.actor_pick_target() != nullptr);
    REQUIRE(scene.actor_pick_target()->id() == "gus");
    scene.actor_picker_confirm();
    REQUIRE(scene.active_actor()->id() == "gus");

    // IDA: corrida de PERFIL encarando o inimigo (run_east; clipe proprio, sem flip).
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();
    scene.aim_confirm();
    REQUIRE(scene.player_action_in_flight());
    scene.update(1.0f / 60.0f);
    auto f = scene.actor_sprite_frame("gus");
    REQUIRE(f.has_value());
    REQUIRE(f->first == BattleClipId::RunEast);

    // CAUDA: o murro de perfil (attack_melee_east) parte e, ate o CONTATO, o frame
    // NUNCA passa do pico f5 (frames derivados f6-f8 nem existem no set - cap do
    // loader). Varre TODO update da aproximacao provando o invariante; o ultimo
    // update do loop ja pode resolver o contato e emendar no Return (o consumidor
    // resolve Hold ~0 no mesmo update - decisao "swing na cauda").
    bool saw_swing = false;
    int swing_peak = -1;
    for (int i = 0; i < 120 && scene.anim().kind_for("gus") ==
                                   ActorAnimKind::MeleeApproach;
         ++i) {
        scene.update(1.0f / 60.0f);
        f = scene.actor_sprite_frame("gus");
        REQUIRE(f.has_value());
        if (f->first == BattleClipId::AttackMeleeEast) {
            saw_swing = true;
            swing_peak = f->second;
            REQUIRE(f->second <= 5);  // estica ate f5 e crava; derivado inalcancavel
        }
    }
    // O swing de fato apareceu na cauda e CRAVOU no pico f5 antes do contato.
    REQUIRE(saw_swing);
    REQUIRE(swing_peak == 5);

    // CONTATO resolve e o Return parte: volta de perfil-esquerda (run_west).
    pump_player_strike(scene);
    REQUIRE(scene.anim().kind_for("gus") == ActorAnimKind::MeleeReturn);
    f = scene.actor_sprite_frame("gus");
    REQUIRE(f.has_value());
    REQUIRE(f->first == BattleClipId::RunWest);

    // Volta termina -> repouso front-facing (battle_idle).
    for (int i = 0; i < 45; ++i) {
        scene.update(1.0f / 60.0f);
    }
    REQUIRE(scene.anim().kind_for("gus") == ActorAnimKind::None);
    f = scene.actor_sprite_frame("gus");
    REQUIRE(f.has_value());
    REQUIRE(f->first == BattleClipId::Idle);
}
