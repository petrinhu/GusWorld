// gus/app/src/screens/battle_input.cpp
//
// Ver header. AC-E11 A1: implementacao do roteamento de input extraido de
// battle_preview.cpp (ADR-019). Corpo das funcoes MOVIDO verbatim - nenhuma logica mudou,
// so o arquivo que a hospeda.

#include "gus/app/screens/battle_input.hpp"

#include <glintfx/ui_event.hpp>

#include "gus/app/screens/battle_cockpit_verb_ids.hpp"  // GLINTFX-CLICK: id->indice de verbo

namespace gus::app::screens {

// ADR-010 F1 SMOKE: ponte SDL_Event -> glintfx::UiEvent (mapa de design da doc de prep).
// Retorna false p/ eventos sem mapeamento (nao injeta ruido). Cobre mouse-move/button,
// teclas de navegacao, texto e resize (pixels reais via SDL_GetWindowSizeInPixels). 'Type'
// e enum class (scoped) na v0.2.1 -> UiEvent::Type::*.
bool sdl_to_glintfx(const SDL_Event& ev, SDL_Window* window, glintfx::UiEvent* out) {
    using K = glintfx::Key;
    using T = glintfx::UiEvent::Type;
    glintfx::UiEvent e{};
    switch (ev.type) {
        case SDL_EVENT_MOUSE_MOTION:
            e.type = T::MouseMove;
            e.x = ev.motion.x;
            e.y = ev.motion.y;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            e.type = T::MouseButton;
            e.x = ev.button.x;
            e.y = ev.button.y;
            e.pressed = (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
            e.button = ev.button.button == SDL_BUTTON_RIGHT    ? 1
                       : ev.button.button == SDL_BUTTON_MIDDLE ? 2
                                                               : 0;
            break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            e.type = T::Key;
            e.pressed = (ev.type == SDL_EVENT_KEY_DOWN);
            e.modifiers = ((ev.key.mod & SDL_KMOD_SHIFT) ? glintfx::Mod_Shift : 0) |
                          ((ev.key.mod & SDL_KMOD_CTRL) ? glintfx::Mod_Ctrl : 0) |
                          ((ev.key.mod & SDL_KMOD_ALT) ? glintfx::Mod_Alt : 0);
            switch (ev.key.key) {
                case SDLK_UP: e.key = K::Up; break;
                case SDLK_DOWN: e.key = K::Down; break;
                case SDLK_LEFT: e.key = K::Left; break;
                case SDLK_RIGHT: e.key = K::Right; break;
                case SDLK_RETURN:
                case SDLK_KP_ENTER: e.key = K::Enter; break;
                case SDLK_ESCAPE: e.key = K::Escape; break;
                case SDLK_TAB: e.key = K::Tab; break;
                case SDLK_SPACE: e.key = K::Space; break;
                case SDLK_BACKSPACE: e.key = K::Backspace; break;
                default: e.key = K::None; break;
            }
            break;
        case SDL_EVENT_TEXT_INPUT:
            e.type = T::Text;
            e.text = ev.text.text;  // valido so no escopo do evento; process_event copia
            break;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
            int pw = 0, ph = 0;
            SDL_GetWindowSizeInPixels(window, &pw, &ph);
            e.type = T::Resize;
            e.width = pw;
            e.height = ph;
            break;
        }
        default:
            return false;  // sem mapeamento -> nao injeta
    }
    *out = e;
    return true;
}

// GLINTFX-CLICK (v0.2.5): aciona o verbo do PILL cujo `id` RCSS (fonte unica em
// gus/app/screens/battle_cockpit_verb_ids.hpp) bate. Wired como o callback de
// glintfx::UiLayer::set_click_callback (ver setup do loop em battle_preview.cpp) -- o
// GLINTFX faz o hit-test ele mesmo (o MESMO que ja move o :hover nativo) e devolve so o
// `id`; aqui so traduzimos esse id pra acao do motor (BattleScene). Extraida em
// funcao-livre (nao inline no lambda do callback) pra ser chamavel DIRETO pelo self-test
// sintetico (GUSWORLD_BATTLE_MOUSE_SELFTEST) sem precisar simular pixel/evento SDL algum
// -- o glintfx ja resolveu o pixel; o self-test so precisa saber que id->verbo bate.
//
// GUARDA: replica, byte a byte, a MESMA ordem de prioridade que battle_mouse_click usa pro
// resto do cockpit (escolha-de-ator > mira > menu). O motivo: os pills SEGUEM no DOM
// (RmlUi data-if="!intro") mesmo durante a escolha de ator ou a mira -- um clique sobre a
// coluna do cockpit nesses estados dispararia este callback EM PARALELO ao hit-test de
// mundo/arena de battle_mouse_click (sao dois listeners INDEPENDENTES, nao mutuamente
// exclusivos como a cadeia if/else antiga). Sem esta guarda, clicar sobre o cockpit
// enquanto mira/escolhe ator selecionaria um verbo por baixo -- regressao. Na ABERTURA
// (is_intro()) o bloco #combat inteiro (pills inclusos) nem existe no DOM -- o id nunca
// bateria mesmo sem a guarda, mas ela fica explicita por clareza/defesa-em-profundidade.
// COCKPIT-SFX-HOVER-CLIQUE: devolve true SE o clique de fato ACIONOU um pill de verbo
// valido (id resolvido + estado que aceita clique de pill) - o callback do glintfx usa
// isso pra tocar o SFX de clique SO quando uma pill foi acionada (nunca em id de outro
// elemento do cockpit nem durante mira/escolha-de-ator/abertura). false = no-op.
bool battle_cockpit_verb_click(BattleScene& scene, const char* element_id) {
    if (scene.is_choosing_actor() || scene.is_aiming() || scene.is_intro()) {
        return false;
    }
    const int idx = gus::app::screens::cockpit_verb_index_for_click_id(element_id);
    if (idx < 0) {
        return false;  // id de outro elemento do cockpit (#combat/#vitals/#log/...) ou "" -> NO-OP
    }
    // Clique = SELECIONA e CONFIRMA. menu_move (delta ate o indice) + menu_confirm; ambos
    // ja sao NO-OP fora do turno do jogador (mesma guarda do teclado) -> seguro em turno
    // de inimigo/combate acabado. menu_move faz WRAP, mas o delta idx-sel (ambos 0..5) cai
    // exato no indice. menu_confirm respeita 'enabled' (verbo sem AP: seleciona, nao aciona).
    scene.menu_move(idx - scene.menu().selected_index());
    scene.menu_confirm();
    return true;  // acionou um pill de verbo -> o chamador pode tocar o SFX de clique
}

// ADR-010 / Incremento A2 (MOUSE), revisado GLINTFX-CLICK: hit-tests de MUNDO/ARENA
// (escolha de ator + mira de alvo) resolvidos AQUI, no host, em coordenadas de MUNDO
// (a projecao ESTICA 960x540 pra pw x ph, NAO-uniforme -- ver viewport_transform
// world_to_screen -> world_x = px/pw*960, world_y = px/ph*540, Y por ph!). O hit-test vem
// do motor de cena (actor_pick_index_at_arena / aim_index_at_arena, casam
// arena_rect_for_actor). O clique nos PILLS DE VERBO NAO passa mais por aqui -- resolvido
// pelo callback do glintfx (battle_cockpit_verb_click acima), que roda em paralelo a esta
// funcao pro MESMO evento SDL (ver o loop de eventos: ambos os caminhos recebem o clique).
// Pressuposto: mouse em px de janela == px do viewport (sem HiDPI neste alvo; MESMO pressuposto
// do forward glintfx). Se houver escala HiDPI no futuro, converter mouse(pontos)->px antes.
void battle_mouse_click(BattleScene& scene, float mx, float my, int pw, int ph) {
    if (pw < 1 || ph < 1) {
        return;
    }
    if (scene.is_choosing_actor()) {
        // ESCOLHA DE ATOR (§4.1): clique num SLOT da party. MESMA conversao MUNDO/arena do modo-
        // mira (estica 960x540; Y por ph) -> reusa o hit-test do motor (actor_pick_index_at_arena,
        // que casa arena_rect_for_actor dos slots da party). Clique unico = ESCOLHE e CONFIRMA o
        // membro - entra no PREVIEW dele (menu de verbos, motor ainda intocado; o turno real so
        // comeca na 1a acao, ver commit_previewed_actor/bug1), a mesma filosofia "aciona na hora"
        // do A2.
        const float wx = mx / static_cast<float>(pw) * 960.0f;
        const float wy = my / static_cast<float>(ph) * 540.0f;
        const int idx = scene.actor_pick_index_at_arena(wx, wy);
        if (idx >= 0) {
            scene.actor_picker_select(idx);   // poe o cursor no membro clicado
            scene.actor_picker_confirm();      // clique unico = escolhe E ENTRA no preview dele
        }
        // Fora de qualquer membro elegivel: NO-OP (o picker precede o menu; nao ha o que cancelar).
        return;
    }
    if (scene.is_aiming()) {
        // Clique num INIMIGO (mira): coordenadas de MUNDO da arena (estica; Y por ph).
        const float wx = mx / static_cast<float>(pw) * 960.0f;
        const float wy = my / static_cast<float>(ph) * 540.0f;
        const int idx = scene.aim_index_at_arena(wx, wy);
        if (idx >= 0) {
            scene.aim_select(idx);  // pousa a mira no alvo clicado
            scene.aim_confirm();    // clique unico = mira E confirma (aciona), igual ao verbo
        }
        // Fora de qualquer inimigo: NO-OP (nao cancela, nao "erra" o alvo) -- escopo A2.
        return;
    }
    // Fora de escolha-de-ator/mira: nao ha mais hit-test de MUNDO a fazer aqui (a ABERTURA
    // tambem nao tem nada clicavel em coordenadas de mundo). O clique nos pills de verbo e'
    // resolvido pelo callback do glintfx (battle_cockpit_verb_click), nao por esta funcao.
}

// Incremento A2 (HOVER, nice-to-have): SO o inimigo. Durante a mira, mover o mouse sobre um
// inimigo PRE-SELECIONA ele (reusa o realce multimodal do alvo do Incremento A, dirigido por
// aim_target()). ZERO risco: nao toca RCSS nem o estado do glintfx. Pill NAO tem hover: o
// realce .sel do RCSS e dirigido pelo MOTOR (data-class-sel); um :hover exigiria mexer na RCSS
// aprovada do cockpit -> fora do escopo A2 (documentado; o CLIQUE no pill ja funciona).
void battle_mouse_hover(BattleScene& scene, float mx, float my, int pw, int ph) {
    if (pw < 1 || ph < 1) {
        return;
    }
    // ESCOLHA DE ATOR (§4.1): passar o mouse sobre um SLOT elegivel da party PRE-SELECIONA ele
    // (move o cursor do picker, SEM confirmar) - o analogo do hover de mira. Checado ANTES da
    // mira (quando is_choosing_actor(), a mira nem existe). MESMA conversao MUNDO/arena.
    if (scene.is_choosing_actor()) {
        const float wx = mx / static_cast<float>(pw) * 960.0f;
        const float wy = my / static_cast<float>(ph) * 540.0f;
        const int idx = scene.actor_pick_index_at_arena(wx, wy);
        if (idx >= 0) {
            scene.actor_picker_select(idx);  // hover destaca (move o cursor, SEM confirmar)
        }
        return;
    }
    if (!scene.is_aiming()) {
        return;
    }
    const float wx = mx / static_cast<float>(pw) * 960.0f;
    const float wy = my / static_cast<float>(ph) * 540.0f;
    const int idx = scene.aim_index_at_arena(wx, wy);
    if (idx >= 0) {
        scene.aim_select(idx);  // hover destaca (move o cursor de mira, SEM confirmar)
    }
}

// Roteamento de TECLADO do host, EXTRAIDO do loop de eventos pra ser CHAMAVEL pelo self-test
// sintetico E pelos testes Catch2 (battle_key_routing_test.cpp; ambos headless, sem SDL_Init -
// ver declaracao em battle_input.hpp). Espelha battle_mouse_click, que ja e uma funcao-livre
// testavel. MESMA ordem e semantica de antes; ADITIVO: a ESCOLHA DE ATOR (§4.1) ganha
// PRIORIDADE MAXIMA sobre mira/menu, porque quando is_choosing_actor() o menu de verbos nem
// existe (begin_turn deferido).
// Digito 1-9 de uma tecla numerica (fileira OU numpad); 0 se nao for numerica 1-9. Fonte
// unica do mapeamento tecla->N pros atalhos numericos (mira e escolha de ator).
int battle_digit_for_key(SDL_Keycode key) noexcept {
    switch (key) {
        case SDLK_1: case SDLK_KP_1: return 1;
        case SDLK_2: case SDLK_KP_2: return 2;
        case SDLK_3: case SDLK_KP_3: return 3;
        case SDLK_4: case SDLK_KP_4: return 4;
        case SDLK_5: case SDLK_KP_5: return 5;
        case SDLK_6: case SDLK_KP_6: return 6;
        case SDLK_7: case SDLK_KP_7: return 7;
        case SDLK_8: case SDLK_KP_8: return 8;
        case SDLK_9: case SDLK_KP_9: return 9;
        default: return 0;
    }
}

void battle_key_down(BattleScene& scene, SDL_Keycode key, bool& running,
                      BattleEscEffect* out_effect) {
    // MENU-PAUSA-CONFIG-SOM: limpa o out-param NO INICIO da funcao (contrato "sempre
    // escrito" - o CHAMADOR nao precisa lembrar de resetar antes de cada chamada; so
    // o ramo Esc-na-pilha-vazia, mais abaixo, o reescreve pra OpenPauseMenu). Cobre
    // TAMBEM os retornos antecipados (teclas-atalho numericas abaixo) - qualquer
    // tecla que nao seja "Esc na pilha vazia" deixa *out_effect == None.
    if (out_effect != nullptr) {
        *out_effect = BattleEscEffect::None;
    }
    // TECLAS-ATALHO NUMERICAS (1-9, fileira + numpad). PRIORIDADE: MODO-MIRA (§3.5) > ESCOLHA
    // DE ATOR (§4.1). Os dois modos nunca sao simultaneos (a mira so abre no menu de verbos,
    // ja fora do picker), mas a ordem deixa explicito: mirando, N mira+confirma o N-esimo
    // inimigo (aim_hotkey); escolhendo ator, N escolhe+confirma o N-esimo membro
    // (actor_picker_hotkey). Ambos ja sao NO-OP fora do seu modo / fora de faixa (guarda
    // interna) -> mapeamento seguro. Consumidas aqui (nao caem no switch abaixo).
    if (const int nth = battle_digit_for_key(key); nth != 0) {
        if (scene.is_aiming()) {
            scene.aim_hotkey(nth);
        } else if (scene.is_choosing_actor()) {
            scene.actor_picker_hotkey(nth);
        }
        return;
    }
    switch (key) {
        case SDLK_ESCAPE:
            // PILHA DE MODAIS (FIX bug2 do playtest do lider 2026-07: "aperto Esc e FECHA A
            // TELA" com um picker/preview aberto). Antes o Esc so conhecia 2 estados (mira ->
            // cancel; qualquer outra coisa, INCLUSIVE o picker/menu de verbos -> sai do
            // viewer), entao Esc durante a escolha de ator fechava a janela - o bug. Agora
            // DESEMPILHA UM NIVEL por vez, do mais aninhado pro mais externo:
            //   (1) MODO-MIRA (§3.5): cancela a mira, volta ao menu/preview de verbos (sem
            //       consumir o turno) - COMPORTAMENTO INTACTO, so reordenado na pilha.
            //   (2) PREVIEW DE ATOR (§4.1, estagio 2: menu de verbos do escolhido, motor
            //       ainda intocado) - volta a LISTA (estagio 1). Nada foi comprometido (sem
            //       begin_turn/tick), entao isto e SEMPRE seguro (ver actor_preview_cancel).
            //   (3) LISTA (§4.1, estagio 1: badges na arena) - TOPO da pilha quando so a
            //       lista esta aberta. NAO ha nivel anterior pra desempilhar (a rodada da
            //       party PRECISA de alguem escolhido pra avancar; Esc != Enter, entao NAO
            //       aceita o pre-selecionado por engano) - MAS e um dos DOIS pontos que abrem
            //       o MENU DE PAUSA (FIX bug do lider 2026-07-05: "o menu de pausa nunca abre
            //       durante o combate normal" - o jogador SEMPRE esta neste estagio OU no (2)
            //       durante o turno, entao a pilha-vazia do estagio (4) nunca era alcancada
            //       na pratica). MESMA logica condicional do estagio (4): com out_effect
            //       nao-nulo (HOST REAL), sinaliza OpenPauseMenu; com out_effect==nullptr
            //       (os testes de battle_key_routing_test.cpp, o --battle standalone via
            //       wrapper, os self-tests sinteticos que passam `dummy`), preserva o NO-OP
            //       ANTIGO - ZERO regressao nesses call-sites.
            //   (4) PILHA VAZIA (nenhum modal aberto): com out_effect nao-nulo, sinaliza
            //       OpenPauseMenu (MENU-PAUSA-CONFIG-SOM, gancho ja integrado); com
            //       out_effect==nullptr, sai do viewer (running=false), como sempre.
            if (scene.is_aiming()) {
                scene.aim_cancel();
            } else if (scene.is_actor_preview()) {
                scene.actor_preview_cancel();
            } else if (scene.is_choosing_actor()) {
                // (3) LISTA DE ATOR: mesma condicional do estagio (4) - ver comentario acima.
                // O HOST REAL (out_effect != nullptr) abre o menu de pausa aqui TAMBEM, porque
                // e o estagio onde o jogador passa a maior parte do turno. Callers com
                // out_effect == nullptr mantem o no-op ANTIGO (nada muda pra eles).
                if (out_effect != nullptr) {
                    *out_effect = BattleEscEffect::OpenPauseMenu;
                }
            } else if (out_effect != nullptr) {
                // MENU-PAUSA-CONFIG-SOM (INTEGRACAO FINAL): o gancho do TODO(menu-de-
                // pause) antigo virou real. O HOST REAL passa out_effect nao-nulo: NAO
                // mexe em running (o viewer continua rodando por baixo do menu) - so
                // sinaliza o pedido; quem decide o resto (abrir o loop do menu, e so
                // entao considerar running=false se o jogador confirmar Sair ou
                // fechar a janela DURANTE o menu) e o CHAMADOR (run_battle_preview_
                // embedded). Callers com out_effect==nullptr (todo o resto: os testes
                // de battle_key_routing_test.cpp, o --battle standalone via wrapper e
                // os self-tests sinteticos que passam `dummy`) preservam o
                // comportamento ANTIGO no ramo else abaixo.
                *out_effect = BattleEscEffect::OpenPauseMenu;
            } else {
                running = false;
            }
            break;
        // Navegacao vertical. PRIORIDADE: escolha de ator (§4.1) > mira (§3.5) > menu de verbos.
        // A party e uma COLUNA vertical na arena -> UP/DOWN move o cursor do picker (com WRAP),
        // a mesma linguagem da mira/menu.
        case SDLK_UP:
        case SDLK_W:
            if (scene.is_choosing_actor()) {
                scene.actor_picker_move(-1);
            } else if (scene.is_aiming()) {
                scene.aim_move(-1);
            } else {
                scene.menu_move(-1);
            }
            break;
        case SDLK_DOWN:
        case SDLK_S:
            if (scene.is_choosing_actor()) {
                scene.actor_picker_move(+1);
            } else if (scene.is_aiming()) {
                scene.aim_move(+1);
            } else {
                scene.menu_move(+1);
            }
            break;
        // LEFT/RIGHT: aliases horizontais de navegacao na escolha de ator (§4.1) e na mira
        // (§3.5). Fora desses modos, sem efeito (o menu de verbos e vertical).
        case SDLK_LEFT:
            if (scene.is_choosing_actor()) {
                scene.actor_picker_move(-1);
            } else if (scene.is_aiming()) {
                scene.aim_move(-1);
            }
            break;
        case SDLK_RIGHT:
            if (scene.is_choosing_actor()) {
                scene.actor_picker_move(+1);
            } else if (scene.is_aiming()) {
                scene.aim_move(+1);
            }
            break;
        // (As teclas-atalho numericas 1-9 sao tratadas no TOPO desta funcao, com prioridade
        // mira > escolha-de-ator, antes deste switch.)
        case SDLK_RETURN:
        case SDLK_KP_ENTER:  // Enter do numpad tambem confirma
        case SDLK_SPACE:
            // ABERTURA (lider 2026-06-25): na tela "BATALHA!" parada, Enter ENCARA. Depois, por
            // PRIORIDADE: ESCOLHA DE ATOR (§4.1) confirma o membro sob o cursor -> ENTRA no
            // preview dele (menu de verbos, motor intocado; o turno real so comeca na 1a acao,
            // bug1); MODO-MIRA (§3.5) confirma o ALVO (resolve, e o commit se ainda em preview);
            // vez do jogador (menu) confirma o verbo (Atacar/Scan ENTRAM na mira; Defender/Flee
            // resolvem - e o commit se ainda em preview). Fora disso, ACELERA o ritmo.
            if (scene.is_intro()) {
                scene.start_combat();  // Encarar
            } else if (scene.is_choosing_actor()) {
                scene.actor_picker_confirm();  // confirma o membro -> entra no preview dele
            } else if (scene.is_aiming()) {
                scene.aim_confirm();  // confirma o alvo mirado
            } else if (scene.waiting_player_input()) {
                scene.menu_confirm();  // Atacar/Scan -> abre a mira
            } else {
                scene.skip();
            }
            break;
        case SDLK_Q:
            // "[Q] Resolver sem encarar" (verbo OPT-IN, so TRASH na abertura). Placeholder
            // neste incremento: a cena loga "[auto-resolve: a implementar]".
            scene.request_auto_resolve();
            break;
        default:
            break;
    }
}

}  // namespace gus::app::screens
