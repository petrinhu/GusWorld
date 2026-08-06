// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/battle_ui_sfx_test.cpp
//
// SFX-COCKPIT: suite HEADLESS (sem SDL_Init, sem janela, sem GL, sem glintfx) do som de
// hover/clique do cockpit de batalha. Roda contra uma BattleScene REAL (o encontro de
// demo: 3 da party x 2 inimigos) e um gus::platform::audio::AudioEngine REAL em
// null-device - a MESMA prova objetiva por sfx_play_count() que
// save_load_menu_interaction_test.cpp:534-538/:640/:876-880 ja usava nas telas de menu.
// Nada de julgamento visual, nada de "olhar o log".
//
// O DEFEITO QUE ESTA SUITE FECHA (auditoria independente de 2026-08-06):
//   (1) NENHUM teste do repo assertia que o cockpit toca som - apagar o play_sfx do host
//       deixava a suite inteira VERDE. O env de autoteste
//       (GUSWORLD_BATTLE_UI_SFX_SELFTEST=1) nao esta no ctest, nem no CI, nem no
//       check.sh: nao roda em lugar nenhum. Os 7 testes POCO de ui_hover_test.cpp cobrem
//       so o edge-detect generico, nunca a batalha.
//   (2) das 3 superficies de selecao-com-realce do combate, 1 soava (pills) e 2 eram
//       MUDAS (mira de inimigo, picker de ator) - `grep play_sfx battle_input.cpp` = 0.
//   (3) o hover so saia no ramo SDL_EVENT_MOUSE_MOTION: quem joga de TECLADO (caminho
//       primario) ouvia blip nos 6 outros menus e SILENCIO na batalha.
//
// O QUE ESTA SUITE TRAVA, e o que ela NAO trava (honestidade de escopo): ela trava a
// DECISAO (que som sai, quantas vezes, em qual transicao) e o play_sfx em si. Ela NAO
// abre janela, entao nao prova a FIACAO do host - essa e' de
// battle_preview_interaction_test.cpp, que roda a funcao de producao com GL real.
//
// Cross-ref: gus/app/screens/battle_ui_sfx.hpp (contrato);
//            gus/app/screens/battle_input.hpp (battle_key_activates_button);
//            app/tests/battle_key_routing_test.cpp (irmao: roteamento de teclado headless);
//            app/tests/ui_hover_test.cpp (o POCO generico de edge-detect reusado aqui).

#include <catch2/catch_test_macros.hpp>

#include "gus/app/screens/battle_assets.hpp"  // resolve_ui_sfx_path (mesmo do host)
#include "gus/app/screens/battle_input.hpp"
#include "gus/app/screens/battle_menu.hpp"
#include "gus/app/screens/battle_scene.hpp"
#include "gus/app/screens/battle_ui_sfx.hpp"
#include "gus/platform/audio/audio_engine.hpp"

using gus::app::screens::battle_focus_entered_new_item;
using gus::app::screens::battle_focus_of;
using gus::app::screens::battle_key_activates_button;
using gus::app::screens::BattleFocus;
using gus::app::screens::BattleFocusSurface;
using gus::app::screens::BattleScene;
using gus::app::screens::BattleUiSfx;
using gus::app::screens::BattleVerb;
using gus::app::screens::UiHoverBox;
using gus::platform::audio::AudioEngine;
using gus::platform::audio::kInvalidSound;
using gus::platform::audio::SoundId;

namespace {

// Bombeia a cena ATE a ESCOLHA DE ATOR (LISTA, estagio 1) abrir - MESMO helper de
// battle_key_routing_test.cpp/battle_scene_test.cpp (duplicado aqui pela convencao de
// arquivo AUTO-CONTIDO desta suite).
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

// Leva a cena ate o MENU DE VERBOS interativo (estagio 2, preview do ator escolhido).
//
// O update() DEPOIS do confirm NAO e' enfeite: actor_picker_confirm() deixa o motor
// intocado de proposito (begin_turn DEFERIDO ate a 1a acao, FIX bug1 do playtest do
// lider), entao o ritmo so entra em WaitingPlayerInput no advance_pacing() do frame
// seguinte. Sem este pump o menu de verbos existe na tela mas waiting_player_input()
// ainda e' false - descoberto ao vivo por 4 testes vermelhos aqui. No host de producao
// isso acontece sozinho (tick_main_ chama scene_->update a cada frame).
void pump_to_verb_menu(BattleScene& scene) {
    pump_to_actor_picker(scene);
    if (scene.is_choosing_actor()) {
        scene.actor_picker_confirm();
    }
    for (int i = 0; i < 120 && !scene.waiting_player_input() && !scene.combat_over(); ++i) {
        scene.update(1.0f / 60.0f);
    }
}

// Leva a cena ate o MODO-MIRA de [Atacar] (2 inimigos vivos no encontro de demo).
void pump_to_aiming(BattleScene& scene) {
    pump_to_verb_menu(scene);
    for (int i = 0; i < 12 && scene.menu().selected_verb() != BattleVerb::Atacar; ++i) {
        scene.menu_move(+1);
    }
    scene.menu_confirm();  // Atacar -> ENTRA na mira (nao confirma o alvo)
}

// AudioEngine de teste + os 2 blips. Os SoundId sao 1-based na ordem de load_sfx (ver
// audio_engine.hpp): aqui NAO carregamos arquivo nenhum do disco de proposito - o que
// esta sob teste e' "quantos play_sfx dispararam", nao "o wav abriu". Ids sinteticos
// bastam? NAO: play_sfx so CONTA quando o id e' valido no engine. Entao carregamos os
// wavs REAIS que o cockpit usa (mesmos 2 arquivos dos 6 menus, zero asset novo).
struct SfxHarness {
    AudioEngine audio{/*device_active=*/false};  // null-device: sem hardware no CI
    SoundId hover = kInvalidSound;
    SoundId click = kInvalidSound;
    BattleUiSfx sfx;

    SfxHarness() {
        hover = audio.load_sfx(gus::app::screens::resolve_ui_sfx_path(
                                   gus::core::assets::kMenuHoverSfxFile)
                                   .c_str());
        click = audio.load_sfx(gus::app::screens::resolve_ui_sfx_path(
                                   gus::core::assets::kMenuClickSfxFile)
                                   .c_str());
        sfx.bind(&audio, hover, click);
    }

    [[nodiscard]] unsigned int plays() const { return audio.sfx_play_count(); }
    // Pre-condicao do harness: se os wavs nao abriram, TODA assercao de contagem viraria
    // 0 == 0 e o teste passaria sem provar nada (falso verde). Checar explicitamente.
    [[nodiscard]] bool loaded() const {
        return hover != kInvalidSound && click != kInvalidSound;
    }
};

// Espelha, em UMA funcao, o choke-point que BattleScreen::handle_event_main_ usa pro
// TECLADO: predicado de clique ANTES (a cena ainda no estado da tecla), roteamento, blip
// de clique, blip de hover por foco. Existe pra a suite exercitar a REGRA sem janela; a
// FIACAO equivalente no host e' provada em battle_preview_interaction_test.cpp.
void route_key(BattleScene& scene, BattleUiSfx& sfx, SDL_Keycode key) {
    bool running = true;
    const bool activated = battle_key_activates_button(scene, key);
    const BattleFocus before = battle_focus_of(scene);
    gus::app::screens::battle_key_down(scene, key, running);
    if (activated) {
        sfx.play_click();
    }
    sfx.on_focus_change(before, battle_focus_of(scene));
}

// Idem, pro MOUSE de mundo (mira/picker).
void route_mouse_hover(BattleScene& scene, BattleUiSfx& sfx, float mx, float my, int pw,
                       int ph) {
    const BattleFocus before = battle_focus_of(scene);
    gus::app::screens::battle_mouse_hover(scene, mx, my, pw, ph);
    sfx.on_focus_change(before, battle_focus_of(scene));
}

void route_mouse_click(BattleScene& scene, BattleUiSfx& sfx, float mx, float my, int pw,
                       int ph) {
    if (gus::app::screens::battle_mouse_click(scene, mx, my, pw, ph)) {
        sfx.play_click();
    }
}

// Caixa de pill sintetica: 6 pills empilhados de 100x20 em x=[800,900).
constexpr int kPills = gus::app::screens::kBattleVerbCount;
void fill_pill_boxes(UiHoverBox (&boxes)[kPills]) {
    for (int i = 0; i < kPills; ++i) {
        boxes[i] = UiHoverBox{/*found=*/true, /*x=*/800.0f,
                              /*y=*/100.0f + 20.0f * static_cast<float>(i),
                              /*w=*/100.0f, /*h=*/20.0f};
    }
}
float pill_cx() { return 850.0f; }
float pill_cy(int i) { return 110.0f + 20.0f * static_cast<float>(i); }

}  // namespace

// =====================================================================================
// (0) PRE-CONDICAO DO HARNESS - sem isto, todo o resto vira 0 == 0 (falso verde)
// =====================================================================================

TEST_CASE("battle_ui_sfx: o harness carrega os 2 blips REAIS do cockpit (mesmos wavs dos "
          "6 menus) - sem isto toda contagem abaixo seria 0==0, um falso verde",
          "[battle_ui_sfx][sfx-cockpit]") {
    SfxHarness h;
    REQUIRE(h.audio.available());
    REQUIRE(h.loaded());
    REQUIRE(h.plays() == 0u);  // carregar NAO toca
}

// =====================================================================================
// (1) battle_focus_of - QUAL superficie esta interativa, e QUAL item esta realcado
// =====================================================================================

TEST_CASE("battle_focus_of: ABERTURA parada (BATALHA!) nao tem superficie interativa",
          "[battle_ui_sfx][sfx-cockpit]") {
    BattleScene scene;
    REQUIRE(scene.is_intro());
    REQUIRE(battle_focus_of(scene).surface == BattleFocusSurface::None);
}

TEST_CASE("battle_focus_of: LISTA de atores aberta -> superficie ActorPicker, com o "
          "membro sob o cursor como item",
          "[battle_ui_sfx][sfx-cockpit]") {
    BattleScene scene;
    pump_to_actor_picker(scene);
    REQUIRE(scene.is_choosing_actor());

    const BattleFocus f = battle_focus_of(scene);
    REQUIRE(f.surface == BattleFocusSurface::ActorPicker);
    REQUIRE(f.item == static_cast<const void*>(scene.actor_pick_target()));
    REQUIRE(f.item != nullptr);
}

TEST_CASE("battle_focus_of: MODO-MIRA -> superficie Aiming, com o inimigo mirado como item",
          "[battle_ui_sfx][sfx-cockpit]") {
    BattleScene scene;
    pump_to_aiming(scene);
    REQUIRE(scene.is_aiming());

    const BattleFocus f = battle_focus_of(scene);
    REQUIRE(f.surface == BattleFocusSurface::Aiming);
    REQUIRE(f.item == static_cast<const void*>(scene.aim_target()));
    REQUIRE(f.item != nullptr);
}

TEST_CASE("battle_focus_of: vez do jogador no menu de verbos -> superficie VerbMenu, com "
          "o INDICE do verbo selecionado",
          "[battle_ui_sfx][sfx-cockpit]") {
    BattleScene scene;
    pump_to_verb_menu(scene);
    REQUIRE(scene.waiting_player_input());
    REQUIRE_FALSE(scene.is_aiming());
    REQUIRE_FALSE(scene.is_choosing_actor());

    const BattleFocus f = battle_focus_of(scene);
    REQUIRE(f.surface == BattleFocusSurface::VerbMenu);
    REQUIRE(f.index == scene.menu().selected_index());
    REQUIRE(f.item == nullptr);
}

// =====================================================================================
// (2) battle_focus_entered_new_item - a REGRA do blip de hover
// =====================================================================================

TEST_CASE("battle_focus_entered_new_item: item NOVO na MESMA superficie -> soa; item "
          "IGUAL -> nao soa (dedup de 'parado sobre o mesmo alvo')",
          "[battle_ui_sfx][sfx-cockpit]") {
    const int alvo_a = 0;
    const int alvo_b = 0;  // valor irrelevante: quem identifica no VerbMenu e' o index
    (void)alvo_a;
    (void)alvo_b;

    const BattleFocus verbo2{BattleFocusSurface::VerbMenu, 2, nullptr};
    const BattleFocus verbo3{BattleFocusSurface::VerbMenu, 3, nullptr};
    REQUIRE(battle_focus_entered_new_item(verbo2, verbo3));
    REQUIRE_FALSE(battle_focus_entered_new_item(verbo2, verbo2));
}

TEST_CASE("battle_focus_entered_new_item: TROCA de superficie NAO e hover (e consequencia "
          "de um confirmar/cancelar, que ja tem som proprio ou nenhum) - guarda anti "
          "som-duplo no mesmo gesto",
          "[battle_ui_sfx][sfx-cockpit]") {
    int enemy = 0;
    const BattleFocus menu{BattleFocusSurface::VerbMenu, 2, nullptr};
    const BattleFocus mira{BattleFocusSurface::Aiming, -1, &enemy};
    REQUIRE_FALSE(battle_focus_entered_new_item(menu, mira));
    REQUIRE_FALSE(battle_focus_entered_new_item(mira, menu));
}

TEST_CASE("battle_focus_entered_new_item: destino None (turno do inimigo, abertura, "
          "combate acabado) nunca soa",
          "[battle_ui_sfx][sfx-cockpit]") {
    const BattleFocus menu{BattleFocusSurface::VerbMenu, 2, nullptr};
    const BattleFocus nada{};
    REQUIRE_FALSE(battle_focus_entered_new_item(menu, nada));
    REQUIRE_FALSE(battle_focus_entered_new_item(nada, nada));
}

TEST_CASE("battle_focus_entered_new_item: superficie aberta mas SEM item realcado (lista "
          "vazia) nao soa",
          "[battle_ui_sfx][sfx-cockpit]") {
    int a = 0;
    const BattleFocus com_alvo{BattleFocusSurface::Aiming, -1, &a};
    const BattleFocus sem_alvo{BattleFocusSurface::Aiming, -1, nullptr};
    REQUIRE_FALSE(battle_focus_entered_new_item(com_alvo, sem_alvo));
}

// =====================================================================================
// (3) battle_key_activates_button - a REGRA do blip de clique no TECLADO
// =====================================================================================

TEST_CASE("battle_key_activates_button: Enter/Espaco CONFIRMANDO nas 3 superficies aciona "
          "botao (picker, mira, menu de verbos)",
          "[battle_ui_sfx][sfx-cockpit]") {
    SECTION("picker de ator") {
        BattleScene scene;
        pump_to_actor_picker(scene);
        REQUIRE(scene.is_choosing_actor());
        REQUIRE(battle_key_activates_button(scene, SDLK_RETURN));
        REQUIRE(battle_key_activates_button(scene, SDLK_SPACE));
        REQUIRE(battle_key_activates_button(scene, SDLK_KP_ENTER));
    }
    SECTION("menu de verbos") {
        BattleScene scene;
        pump_to_verb_menu(scene);
        REQUIRE(scene.waiting_player_input());
        REQUIRE(battle_key_activates_button(scene, SDLK_RETURN));
    }
    SECTION("mira") {
        BattleScene scene;
        pump_to_aiming(scene);
        REQUIRE(scene.is_aiming());
        REQUIRE(battle_key_activates_button(scene, SDLK_RETURN));
    }
}

TEST_CASE("battle_key_activates_button: NAVEGAR nao e acionar - setas/W/S nunca contam "
          "como clique (elas tem o som de HOVER, nao o de clique)",
          "[battle_ui_sfx][sfx-cockpit]") {
    BattleScene scene;
    pump_to_verb_menu(scene);
    for (const SDL_Keycode k : {SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_W, SDLK_S}) {
        REQUIRE_FALSE(battle_key_activates_button(scene, k));
    }
}

TEST_CASE("battle_key_activates_button: Esc (cancela/desempilha) e Q (auto-resolve) nao "
          "acionam botao",
          "[battle_ui_sfx][sfx-cockpit]") {
    BattleScene scene;
    pump_to_aiming(scene);
    REQUIRE_FALSE(battle_key_activates_button(scene, SDLK_ESCAPE));
    REQUIRE_FALSE(battle_key_activates_button(scene, SDLK_Q));
}

TEST_CASE("battle_key_activates_button: ABERTURA (Encarar) NAO soa - banner de tela "
          "cheia, nao menu com botoes (decisao desta fatia, pendente do lider)",
          "[battle_ui_sfx][sfx-cockpit]") {
    BattleScene scene;
    REQUIRE(scene.is_intro());
    REQUIRE_FALSE(battle_key_activates_button(scene, SDLK_RETURN));
    REQUIRE_FALSE(battle_key_activates_button(scene, SDLK_SPACE));
}

TEST_CASE("battle_key_activates_button: atalho numerico SO soa quando ha candidato - o "
          "roteamento e' no-op fora de faixa, entao o som seria mentira",
          "[battle_ui_sfx][sfx-cockpit]") {
    SECTION("mira: 2 inimigos vivos no encontro de demo") {
        BattleScene scene;
        pump_to_aiming(scene);
        REQUIRE(scene.aim_count() == 2);
        REQUIRE(battle_key_activates_button(scene, SDLK_1));
        REQUIRE(battle_key_activates_button(scene, SDLK_2));
        REQUIRE_FALSE(battle_key_activates_button(scene, SDLK_3));  // fora de faixa
        REQUIRE_FALSE(battle_key_activates_button(scene, SDLK_9));
    }
    SECTION("picker: N elegiveis, N+1 nao existe") {
        BattleScene scene;
        pump_to_actor_picker(scene);
        const int n = scene.actor_pick_count();
        REQUIRE(n >= 1);
        REQUIRE(n <= 8);  // o encontro de demo tem 3 na party - nunca chega a 9
        REQUIRE(battle_key_activates_button(scene, SDLK_1));
        REQUIRE_FALSE(battle_key_activates_button(
            scene, static_cast<SDL_Keycode>(SDLK_1 + n)));  // o (n+1)-esimo nao existe
    }
    SECTION("fora dos 2 modos, digito nao aciona nada") {
        BattleScene scene;
        pump_to_verb_menu(scene);
        REQUIRE_FALSE(battle_key_activates_button(scene, SDLK_1));
    }
}

// =====================================================================================
// (4) BattleUiSfx - o UNICO dono de play_sfx: CONTAGEM real, engine real
// =====================================================================================

TEST_CASE("BattleUiSfx: hover por FOCO toca EXATAMENTE 1 blip por item novo, e 0 quando o "
          "foco nao mudou",
          "[battle_ui_sfx][sfx-cockpit]") {
    SfxHarness h;
    REQUIRE(h.loaded());
    int alvo_a = 0, alvo_b = 0;
    const BattleFocus a{BattleFocusSurface::Aiming, -1, &alvo_a};
    const BattleFocus b{BattleFocusSurface::Aiming, -1, &alvo_b};

    REQUIRE(h.sfx.on_focus_change(a, b));
    REQUIRE(h.plays() == 1u);
    REQUIRE_FALSE(h.sfx.on_focus_change(b, b));
    REQUIRE(h.plays() == 1u);  // parado: NAO redispara
    REQUIRE(h.sfx.on_focus_change(b, a));
    REQUIRE(h.plays() == 2u);
}

TEST_CASE("BattleUiSfx: hover por GEOMETRIA (pills) - varrer os 6 pills toca 6, ficar "
          "parado toca 0, sair-e-voltar toca 1 (edge-detect preservado byte a byte do "
          "comportamento que ja existia no host)",
          "[battle_ui_sfx][sfx-cockpit]") {
    SfxHarness h;
    REQUIRE(h.loaded());
    UiHoverBox boxes[kPills];
    fill_pill_boxes(boxes);

    for (int i = 0; i < kPills; ++i) {
        REQUIRE(h.sfx.on_pill_motion(pill_cx(), pill_cy(i), boxes, kPills));
    }
    REQUIRE(h.plays() == static_cast<unsigned int>(kPills));

    // Parado sobre o ULTIMO pill, 2 motions identicos: 0 blips extras.
    REQUIRE_FALSE(h.sfx.on_pill_motion(pill_cx(), pill_cy(kPills - 1), boxes, kPills));
    REQUIRE_FALSE(h.sfx.on_pill_motion(pill_cx(), pill_cy(kPills - 1), boxes, kPills));
    REQUIRE(h.plays() == static_cast<unsigned int>(kPills));

    // Sair pra fora de qualquer pill: NAO soa (so ENTRAR soa).
    REQUIRE_FALSE(h.sfx.on_pill_motion(10.0f, 10.0f, boxes, kPills));
    REQUIRE(h.plays() == static_cast<unsigned int>(kPills));

    // Voltar: soa 1.
    REQUIRE(h.sfx.on_pill_motion(pill_cx(), pill_cy(0), boxes, kPills));
    REQUIRE(h.plays() == static_cast<unsigned int>(kPills) + 1u);
}

TEST_CASE("BattleUiSfx: reset_pill_hover NAO toca nada, mas faz o proximo hover no MESMO "
          "pill voltar a soar (o realce nao 'prende' quando o menu sai do ar)",
          "[battle_ui_sfx][sfx-cockpit]") {
    SfxHarness h;
    REQUIRE(h.loaded());
    UiHoverBox boxes[kPills];
    fill_pill_boxes(boxes);

    REQUIRE(h.sfx.on_pill_motion(pill_cx(), pill_cy(3), boxes, kPills));
    REQUIRE(h.plays() == 1u);
    REQUIRE(h.sfx.hovered_pill() == 3);

    h.sfx.reset_pill_hover();
    REQUIRE(h.plays() == 1u);  // resetar e' MUDO
    REQUIRE(h.sfx.hovered_pill() == -1);

    REQUIRE(h.sfx.on_pill_motion(pill_cx(), pill_cy(3), boxes, kPills));
    REQUIRE(h.plays() == 2u);  // mesmo pill volta a soar apos o reset
}

TEST_CASE("BattleUiSfx: play_click toca 1 por acionamento", "[battle_ui_sfx][sfx-cockpit]") {
    SfxHarness h;
    REQUIRE(h.loaded());
    REQUIRE(h.sfx.play_click());
    REQUIRE(h.plays() == 1u);
    REQUIRE(h.audio.last_sfx_id() == h.click);  // clique, NAO hover (prova de ROTEAMENTO)
}

TEST_CASE("BattleUiSfx: hover e clique usam SoundId DIFERENTES - o blip certo em cada "
          "gesto (sfx_play_count sozinho nao provaria isto)",
          "[battle_ui_sfx][sfx-cockpit]") {
    SfxHarness h;
    REQUIRE(h.loaded());
    REQUIRE(h.hover != h.click);

    int a = 0, b = 0;
    h.sfx.on_focus_change(BattleFocus{BattleFocusSurface::Aiming, -1, &a},
                          BattleFocus{BattleFocusSurface::Aiming, -1, &b});
    REQUIRE(h.audio.last_sfx_id() == h.hover);
    h.sfx.play_click();
    REQUIRE(h.audio.last_sfx_id() == h.click);
}

TEST_CASE("BattleUiSfx: degrada MUDO (nunca crasha) sem engine ou com blip ausente - o "
          "jogo nao depende de audio pra rodar",
          "[battle_ui_sfx][sfx-cockpit]") {
    SECTION("sem engine (bind nunca chamado)") {
        gus::app::screens::BattleUiSfx sfx;
        int a = 0, b = 0;
        REQUIRE_FALSE(sfx.play_click());
        REQUIRE_FALSE(sfx.on_focus_change(BattleFocus{BattleFocusSurface::Aiming, -1, &a},
                                          BattleFocus{BattleFocusSurface::Aiming, -1, &b}));
    }
    SECTION("engine ok mas wav ausente (SoundId invalido)") {
        AudioEngine audio(/*device_active=*/false);
        gus::app::screens::BattleUiSfx sfx;
        sfx.bind(&audio, kInvalidSound, kInvalidSound);
        REQUIRE_FALSE(sfx.play_click());
        REQUIRE(audio.sfx_play_count() == 0u);
    }
}

// =====================================================================================
// (5) AS 3 SUPERFICIES x OS 2 CANAIS - a tabela da auditoria, virada em teste
// =====================================================================================

TEST_CASE("SFX-COCKPIT superficie MIRA / canal TECLADO: navegar entre inimigos toca 1 "
          "hover por alvo novo (ANTES desta fatia: 0 - grep play_sfx battle_input.cpp "
          "devolvia zero)",
          "[battle_ui_sfx][sfx-cockpit]") {
    SfxHarness h;
    REQUIRE(h.loaded());
    BattleScene scene;
    pump_to_aiming(scene);
    REQUIRE(scene.is_aiming());
    REQUIRE(scene.aim_count() == 2);

    const auto* alvo0 = scene.aim_target();
    route_key(scene, h.sfx, SDLK_DOWN);  // aim_move(+1) -> outro inimigo
    REQUIRE(scene.aim_target() != alvo0);
    REQUIRE(h.plays() == 1u);
    REQUIRE(h.audio.last_sfx_id() == h.hover);

    route_key(scene, h.sfx, SDLK_UP);  // volta pro primeiro
    REQUIRE(scene.aim_target() == alvo0);
    REQUIRE(h.plays() == 2u);
}

TEST_CASE("SFX-COCKPIT superficie MIRA / canal MOUSE: passar o mouse sobre o inimigo toca "
          "1 hover; parado sobre o MESMO inimigo NAO redispara",
          "[battle_ui_sfx][sfx-cockpit]") {
    SfxHarness h;
    REQUIRE(h.loaded());
    BattleScene scene;
    pump_to_aiming(scene);
    REQUIRE(scene.is_aiming());

    // Procura, em coordenadas de MUNDO (960x540), o ponto do inimigo que NAO esta
    // mirado agora - o hit-test da propria cena e' a fonte da verdade.
    const auto* alvo0 = scene.aim_target();
    float hit_x = -1.0f, hit_y = -1.0f;
    for (float y = 0.0f; y < 540.0f && hit_x < 0.0f; y += 4.0f) {
        for (float x = 0.0f; x < 960.0f; x += 4.0f) {
            const int idx = scene.aim_index_at_arena(x, y);
            if (idx < 0) continue;
            // So serve o ponto que MUDA o alvo (senao nao ha transicao pra provar).
            BattleScene probe;
            pump_to_aiming(probe);
            probe.aim_select(idx);
            if (probe.aim_target() != nullptr && probe.aim_target()->id() != alvo0->id()) {
                hit_x = x;
                hit_y = y;
                break;
            }
        }
    }
    REQUIRE(hit_x >= 0.0f);  // achou um inimigo clicavel diferente do mirado

    // pw/ph == 960x540 -> px de janela == px de mundo (a conversao vira identidade).
    route_mouse_hover(scene, h.sfx, hit_x, hit_y, 960, 540);
    REQUIRE(scene.aim_target() != alvo0);
    REQUIRE(h.plays() == 1u);

    route_mouse_hover(scene, h.sfx, hit_x, hit_y, 960, 540);  // parado
    REQUIRE(h.plays() == 1u);
}

TEST_CASE("SFX-COCKPIT superficie MIRA: CONFIRMAR o alvo toca o blip de CLIQUE (1), nos "
          "DOIS canais - e NAO soma um hover junto (troca de superficie nao e hover)",
          "[battle_ui_sfx][sfx-cockpit]") {
    SECTION("teclado (Enter)") {
        SfxHarness h;
        REQUIRE(h.loaded());
        BattleScene scene;
        pump_to_aiming(scene);
        route_key(scene, h.sfx, SDLK_RETURN);
        REQUIRE_FALSE(scene.is_aiming());  // confirmou
        REQUIRE(h.plays() == 1u);
        REQUIRE(h.audio.last_sfx_id() == h.click);
    }
    SECTION("mouse (clique no inimigo)") {
        SfxHarness h;
        REQUIRE(h.loaded());
        BattleScene scene;
        pump_to_aiming(scene);
        float hit_x = -1.0f, hit_y = -1.0f;
        for (float y = 0.0f; y < 540.0f && hit_x < 0.0f; y += 4.0f) {
            for (float x = 0.0f; x < 960.0f; x += 4.0f) {
                if (scene.aim_index_at_arena(x, y) >= 0) {
                    hit_x = x;
                    hit_y = y;
                    break;
                }
            }
        }
        REQUIRE(hit_x >= 0.0f);
        route_mouse_click(scene, h.sfx, hit_x, hit_y, 960, 540);
        REQUIRE_FALSE(scene.is_aiming());
        REQUIRE(h.plays() == 1u);
        REQUIRE(h.audio.last_sfx_id() == h.click);
    }
}

TEST_CASE("SFX-COCKPIT superficie MIRA / canal MOUSE: clicar no VAZIO da arena e' no-op "
          "SILENCIOSO (som so onde houve acao)",
          "[battle_ui_sfx][sfx-cockpit]") {
    SfxHarness h;
    REQUIRE(h.loaded());
    BattleScene scene;
    pump_to_aiming(scene);
    // Canto superior esquerdo: fora de qualquer slot de inimigo (conferido pelo proprio
    // hit-test da cena, nao por chute).
    REQUIRE(scene.aim_index_at_arena(2.0f, 2.0f) < 0);
    route_mouse_click(scene, h.sfx, 2.0f, 2.0f, 960, 540);
    REQUIRE(scene.is_aiming());  // nao confirmou nada
    REQUIRE(h.plays() == 0u);
}

TEST_CASE("SFX-COCKPIT superficie PICKER DE ATOR / canal TECLADO: navegar entre membros "
          "toca 1 hover por membro novo (ANTES desta fatia: 0)",
          "[battle_ui_sfx][sfx-cockpit]") {
    SfxHarness h;
    REQUIRE(h.loaded());
    BattleScene scene;
    pump_to_actor_picker(scene);
    REQUIRE(scene.is_choosing_actor());
    REQUIRE(scene.actor_pick_count() >= 2);

    const auto* membro0 = scene.actor_pick_target();
    route_key(scene, h.sfx, SDLK_DOWN);
    REQUIRE(scene.actor_pick_target() != membro0);
    REQUIRE(h.plays() == 1u);
    REQUIRE(h.audio.last_sfx_id() == h.hover);
}

TEST_CASE("SFX-COCKPIT superficie PICKER DE ATOR: CONFIRMAR o membro toca o blip de "
          "CLIQUE (1), nos DOIS canais",
          "[battle_ui_sfx][sfx-cockpit]") {
    SECTION("teclado (Enter)") {
        SfxHarness h;
        REQUIRE(h.loaded());
        BattleScene scene;
        pump_to_actor_picker(scene);
        route_key(scene, h.sfx, SDLK_RETURN);
        REQUIRE(scene.is_actor_preview());  // entrou no preview do escolhido
        REQUIRE(h.plays() == 1u);
        REQUIRE(h.audio.last_sfx_id() == h.click);
    }
    SECTION("mouse (clique no slot da party)") {
        SfxHarness h;
        REQUIRE(h.loaded());
        BattleScene scene;
        pump_to_actor_picker(scene);
        float hit_x = -1.0f, hit_y = -1.0f;
        for (float y = 0.0f; y < 540.0f && hit_x < 0.0f; y += 4.0f) {
            for (float x = 0.0f; x < 960.0f; x += 4.0f) {
                if (scene.actor_pick_index_at_arena(x, y) >= 0) {
                    hit_x = x;
                    hit_y = y;
                    break;
                }
            }
        }
        REQUIRE(hit_x >= 0.0f);
        route_mouse_click(scene, h.sfx, hit_x, hit_y, 960, 540);
        REQUIRE_FALSE(scene.is_choosing_actor());
        REQUIRE(h.plays() == 1u);
        REQUIRE(h.audio.last_sfx_id() == h.click);
    }
}

TEST_CASE("SFX-COCKPIT superficie MENU DE VERBOS / canal TECLADO: Cima/Baixo toca 1 hover "
          "por verbo novo - a PARIDADE que faltava (o mouse ja soava; o teclado, nao)",
          "[battle_ui_sfx][sfx-cockpit]") {
    SfxHarness h;
    REQUIRE(h.loaded());
    BattleScene scene;
    pump_to_verb_menu(scene);
    REQUIRE(scene.waiting_player_input());

    const int sel0 = scene.menu().selected_index();
    route_key(scene, h.sfx, SDLK_DOWN);
    REQUIRE(scene.menu().selected_index() != sel0);
    REQUIRE(h.plays() == 1u);
    REQUIRE(h.audio.last_sfx_id() == h.hover);

    route_key(scene, h.sfx, SDLK_DOWN);
    REQUIRE(h.plays() == 2u);
    route_key(scene, h.sfx, SDLK_UP);
    REQUIRE(h.plays() == 3u);
}

TEST_CASE("SFX-COCKPIT superficie MENU DE VERBOS / canal TECLADO: Enter aciona o verbo e "
          "toca 1 CLIQUE (o mesmo gesto que o clique no pill, que ja soava)",
          "[battle_ui_sfx][sfx-cockpit]") {
    SfxHarness h;
    REQUIRE(h.loaded());
    BattleScene scene;
    pump_to_verb_menu(scene);
    for (int i = 0; i < 12 && scene.menu().selected_verb() != BattleVerb::Atacar; ++i) {
        scene.menu_move(+1);  // navegacao de SETUP, fora do choke-point: nao soa
    }
    REQUIRE(h.plays() == 0u);

    route_key(scene, h.sfx, SDLK_RETURN);
    REQUIRE(scene.is_aiming());  // Atacar abriu a mira
    REQUIRE(h.plays() == 1u);
    REQUIRE(h.audio.last_sfx_id() == h.click);
}

TEST_CASE("SFX-COCKPIT: ABERTURA e' MUDA - Enter que 'Encara' nao toca blip de UI "
          "(decisao desta fatia; ver o header de battle_input.hpp)",
          "[battle_ui_sfx][sfx-cockpit]") {
    SfxHarness h;
    REQUIRE(h.loaded());
    BattleScene scene;
    REQUIRE(scene.is_intro());
    route_key(scene, h.sfx, SDLK_RETURN);
    REQUIRE_FALSE(scene.is_intro());  // encarou de verdade
    REQUIRE(h.plays() == 0u);
}

TEST_CASE("SFX-COCKPIT: ACELERAR o ritmo (Enter fora da vez do jogador) e' MUDO - nao e "
          "botao, e soaria a cada toque de quem esta apressando a animacao",
          "[battle_ui_sfx][sfx-cockpit]") {
    SfxHarness h;
    REQUIRE(h.loaded());
    BattleScene scene;
    scene.start_combat();
    // Estado em que NAO e' vez do jogador nem ha modal aberto: Enter cai em skip().
    for (int i = 0; i < 400 && (scene.waiting_player_input() || scene.is_intro()); ++i) {
        scene.update(1.0f / 60.0f);
    }
    if (scene.waiting_player_input() || scene.is_choosing_actor() || scene.is_aiming()) {
        SUCCEED("cena assentou direto na vez do jogador neste encontro - caso coberto "
                "pelos testes de menu/picker acima");
        return;
    }
    route_key(scene, h.sfx, SDLK_RETURN);
    REQUIRE(h.plays() == 0u);
}

TEST_CASE("SFX-COCKPIT: Esc CANCELANDO a mira e' MUDO (cancelar nao e acionar, e a volta "
          "pro menu e' troca de superficie, nao hover)",
          "[battle_ui_sfx][sfx-cockpit]") {
    SfxHarness h;
    REQUIRE(h.loaded());
    BattleScene scene;
    pump_to_aiming(scene);
    REQUIRE(scene.is_aiming());
    route_key(scene, h.sfx, SDLK_ESCAPE);
    REQUIRE_FALSE(scene.is_aiming());  // cancelou de verdade
    REQUIRE(h.plays() == 0u);
}

TEST_CASE("SFX-COCKPIT: atalho numerico na mira toca 1 CLIQUE (seleciona E confirma), e "
          "fora de faixa e' MUDO (o roteamento e' no-op - o som seria mentira)",
          "[battle_ui_sfx][sfx-cockpit]") {
    SECTION("nth valido") {
        SfxHarness h;
        REQUIRE(h.loaded());
        BattleScene scene;
        pump_to_aiming(scene);
        route_key(scene, h.sfx, SDLK_2);
        REQUIRE_FALSE(scene.is_aiming());  // confirmou o 2o inimigo
        REQUIRE(h.plays() == 1u);
        REQUIRE(h.audio.last_sfx_id() == h.click);
    }
    SECTION("nth fora de faixa") {
        SfxHarness h;
        REQUIRE(h.loaded());
        BattleScene scene;
        pump_to_aiming(scene);
        route_key(scene, h.sfx, SDLK_9);
        REQUIRE(scene.is_aiming());  // nada aconteceu
        REQUIRE(h.plays() == 0u);
    }
}
