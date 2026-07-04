// GusEngine/app/tests/battle_key_routing_test.cpp
//
// Catch2 (headless, SEM SDL_Init/janela) do ROTEAMENTO DE TECLADO do host da BattleScene
// (battle_key_down, extraido em battle_preview.cpp/.hpp). Cobre o FIX do bug2 relatado ao
// vivo pelo lider ("aperto Esc e FECHA A TELA" com um picker/preview de ator aberto):
// Esc agora DESEMPILHA UM NIVEL de modal por vez (mira -> preview de ator -> lista -> so
// ENTAO sai do viewer), em vez do generico antigo "qualquer coisa != mira sai do viewer".
//
// SDL_Keycode e so um typedef de inteiro (SDLK_* sao constantes de compilacao): usa-los
// aqui NAO exige SDL_Init nem janela - gusengine_app ja linka SDL3 (ver app/CMakeLists.txt),
// entao battle_key_down e testavel 100% headless, no mesmo espirito do resto da suite.
//
// Cross-ref: gus/app/screens/battle_preview.hpp (declaracoes);
//            gus/app/screens/battle_scene.hpp (is_actor_preview/actor_preview_cancel, §4.1);
//            docs/design/mecanicas/combat.md §4.1 (comando-livre 1B).

#include <catch2/catch_test_macros.hpp>

#include "gus/app/screens/battle_menu.hpp"
#include "gus/app/screens/battle_preview.hpp"
#include "gus/app/screens/battle_scene.hpp"

using gus::app::screens::battle_key_down;
using gus::app::screens::BattleEscEffect;
using gus::app::screens::BattleScene;
using gus::app::screens::BattleVerb;

namespace {

// Bombeia a cena ATE a ESCOLHA DE ATOR (LISTA, estagio 1) abrir - mesma logica de
// pump_to_actor_picker em battle_scene_test.cpp (duplicada aqui pra este arquivo ficar
// self-contained, mesmo padrao dos demais arquivos de teste desta suite).
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

// Navega o menu ate o verbo desejado ficar selecionado (mesmo helper de battle_scene_test).
void select_verb(BattleScene& scene, BattleVerb want) {
    for (int i = 0; i < 12 && scene.menu().selected_verb() != want; ++i) {
        scene.menu_move(+1);
    }
}

}  // namespace

TEST_CASE("Esc: pilha VAZIA (nenhum modal aberto) sai do viewer - baseline intacto",
          "[battle_key_routing]") {
    // Antes de qualquer picker/mira: comportamento IGUAL ao de sempre (Esc fecha o viewer).
    BattleScene scene;
    bool running = true;
    battle_key_down(scene, SDLK_ESCAPE, running);
    REQUIRE_FALSE(running);
}

TEST_CASE("Esc FIX bug2: no picker (LISTA, estagio 1) e NO-OP - NAO fecha a tela",
          "[battle_key_routing]") {
    // O BUG relatado ao vivo: Esc com o picker aberto fechava a janela. Prova o fix: `running`
    // segue true e a lista continua aberta (nao ha nivel anterior pra desempilhar aqui).
    BattleScene scene;
    pump_to_actor_picker(scene);
    REQUIRE(scene.is_choosing_actor());

    bool running = true;
    battle_key_down(scene, SDLK_ESCAPE, running);

    REQUIRE(running);                    // NAO fechou a tela (o bug)
    REQUIRE(scene.is_choosing_actor());  // continua na lista
}

TEST_CASE("Esc FIX bug2: no PREVIEW de ator (estagio 2) desempilha pra LISTA - NAO fecha",
          "[battle_key_routing]") {
    BattleScene scene;
    pump_to_actor_picker(scene);
    const auto* pre = scene.actor_pick_target();
    scene.actor_picker_confirm();  // entra no preview (menu de verbos do escolhido)
    REQUIRE(scene.is_actor_preview());

    bool running = true;
    battle_key_down(scene, SDLK_ESCAPE, running);

    REQUIRE(running);                       // NAO fechou a tela
    REQUIRE(scene.is_choosing_actor());     // desempilhou 1 nivel: de volta a LISTA
    REQUIRE_FALSE(scene.is_actor_preview());
    REQUIRE(scene.actor_pick_target() == pre);  // mesma lista/cursor (nada foi comprometido)
}

TEST_CASE("Esc: na MIRA cancela o alvo (comportamento intacto) - volta ao PREVIEW, nao ao picker",
          "[battle_key_routing]") {
    // Esc na mira segue tratado ANTES do preview na pilha (nivel mais aninhado primeiro) -
    // este comportamento ja existia e NAO pode regredir (invariante explicita da tarefa).
    BattleScene scene;
    pump_to_actor_picker(scene);
    scene.actor_picker_confirm();  // preview
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();  // entra em modo-mira (NAO commita ainda - so a mira abriu)
    REQUIRE(scene.is_aiming());
    REQUIRE(scene.is_actor_preview());  // o ator ESCOLHIDO ainda esta em preview (mira nao commita)

    bool running = true;
    battle_key_down(scene, SDLK_ESCAPE, running);

    REQUIRE(running);
    REQUIRE_FALSE(scene.is_aiming());   // mira cancelada
    REQUIRE(scene.is_actor_preview());  // volta ao PREVIEW (nao pulou direto pro picker)
    REQUIRE_FALSE(scene.is_choosing_actor());
}

TEST_CASE("Esc: pilha completa desempilha 1 NIVEL por tecla (mira -> preview -> lista -> no-op)",
          "[battle_key_routing]") {
    BattleScene scene;
    pump_to_actor_picker(scene);
    const auto* pre = scene.actor_pick_target();
    scene.actor_picker_confirm();
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();  // mira aberta
    REQUIRE(scene.is_aiming());

    bool running = true;

    // 1a tecla: mira -> preview.
    battle_key_down(scene, SDLK_ESCAPE, running);
    REQUIRE(running);
    REQUIRE_FALSE(scene.is_aiming());
    REQUIRE(scene.is_actor_preview());

    // 2a tecla: preview -> lista.
    battle_key_down(scene, SDLK_ESCAPE, running);
    REQUIRE(running);
    REQUIRE_FALSE(scene.is_actor_preview());
    REQUIRE(scene.is_choosing_actor());
    REQUIRE(scene.actor_pick_target() == pre);

    // 3a tecla: lista -> no-op (topo da pilha; NAO fecha a tela, nao commita ninguem).
    battle_key_down(scene, SDLK_ESCAPE, running);
    REQUIRE(running);
    REQUIRE(scene.is_choosing_actor());

    // Nada foi comprometido em NENHUM ponto desta sequencia: o pre-selecionado segue
    // pendente (motor 100% intocado pelas 3 teclas de Esc).
    const auto pending = scene.machine().pending_party_actors();
    bool still_pending = false;
    for (const auto* a : pending) {
        if (a == pre) {
            still_pending = true;
        }
    }
    REQUIRE(still_pending);
}

TEST_CASE("Esc FIX bug2 invariante: apos a acao resolvida (comprometido), Esc volta a sair",
          "[battle_key_routing]") {
    // Espelha a REGRA #5 da tarefa: "depois que o ator agiu, nao ha volta" - uma vez
    // comprometido (commit_previewed_actor ja rodou), nem preview nem picker seguem abertos
    // POR CAUSA da acao em si; se nao houver outro modal aberto, o Esc volta ao baseline
    // (sai do viewer) - NAO fica preso num estado fantasma.
    BattleScene scene;
    pump_to_actor_picker(scene);
    scene.actor_picker_confirm();
    select_verb(scene, BattleVerb::Defender);
    scene.menu_confirm();  // commit + resolve (Defender nao abre mira)
    REQUIRE_FALSE(scene.is_actor_preview());

    if (!scene.is_choosing_actor() && !scene.is_aiming()) {
        // Sem outro modal reaberto (ex.: nao caiu direto no picker do proximo ator): Esc
        // segue o baseline de sempre.
        bool running = true;
        battle_key_down(scene, SDLK_ESCAPE, running);
        REQUIRE_FALSE(running);
    }
}

// MENU-PAUSA-CONFIG-SOM (M7-COSTURA, INTEGRACAO FINAL): out_effect (4o parametro,
// default nullptr) e o gancho novo do menu de pausa. Os 8 TEST_CASEs acima chamam
// battle_key_down com 3 args (out_effect implicito = nullptr) - continuam intactos,
// comportamento ANTIGO preservado byte a byte (running=false na pilha vazia). Estes
// casos NOVOS cobrem o 4o arg explicitamente.

TEST_CASE("Esc na pilha VAZIA com out_effect: sinaliza OpenPauseMenu e NAO mexe em running",
          "[battle_key_routing]") {
    BattleScene scene;
    bool running = true;
    BattleEscEffect effect = BattleEscEffect::None;
    battle_key_down(scene, SDLK_ESCAPE, running, &effect);

    REQUIRE(running);  // NAO fechou o viewer (diferente do baseline sem out_effect)
    REQUIRE(effect == BattleEscEffect::OpenPauseMenu);
}

TEST_CASE("Esc na MIRA com out_effect: cancela a mira, effect fica None (pilha nao-vazia)",
          "[battle_key_routing]") {
    // A PRIORIDADE da pilha (mira > preview > picker > pilha-vazia) e IGUAL com ou sem
    // out_effect - so o passo FINAL (pilha vazia) muda de comportamento.
    BattleScene scene;
    pump_to_actor_picker(scene);
    scene.actor_picker_confirm();
    select_verb(scene, BattleVerb::Atacar);
    scene.menu_confirm();  // mira aberta
    REQUIRE(scene.is_aiming());

    bool running = true;
    BattleEscEffect effect = BattleEscEffect::OpenPauseMenu;  // valor-sentinela: prova que MUDOU pra None
    battle_key_down(scene, SDLK_ESCAPE, running, &effect);

    REQUIRE(running);
    REQUIRE_FALSE(scene.is_aiming());
    REQUIRE(effect == BattleEscEffect::None);  // pilha NAO estava vazia - sem efeito de pausa
}

TEST_CASE("Esc no PICKER (LISTA) com out_effect: continua NO-OP, effect fica None",
          "[battle_key_routing]") {
    BattleScene scene;
    pump_to_actor_picker(scene);
    REQUIRE(scene.is_choosing_actor());

    bool running = true;
    BattleEscEffect effect = BattleEscEffect::OpenPauseMenu;  // sentinela
    battle_key_down(scene, SDLK_ESCAPE, running, &effect);

    REQUIRE(running);
    REQUIRE(scene.is_choosing_actor());
    REQUIRE(effect == BattleEscEffect::None);
}

TEST_CASE("Esc na pilha VAZIA SEM out_effect (nullptr, default): comportamento ANTIGO intacto",
          "[battle_key_routing]") {
    // Prova explicita da retro-compatibilidade: chamar com o 4o arg OMITIDO (default
    // nullptr) e IDENTICO ao 1o TEST_CASE deste arquivo (running vira false).
    BattleScene scene;
    bool running = true;
    battle_key_down(scene, SDLK_ESCAPE, running);  // out_effect omitido = nullptr
    REQUIRE_FALSE(running);
}
