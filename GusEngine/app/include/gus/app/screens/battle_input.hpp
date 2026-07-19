// gus/app/screens/battle_input.hpp
//
// AC-E11 A1 (ADR-019, decomposicao atomica de battle_preview.cpp): roteamento de INPUT
// do host da BattleScene. Reune a ponte SDL_Event -> glintfx::UiEvent (sdl_to_glintfx), o
// hit-test/roteamento de MOUSE (clique nos pills de verbo via callback do glintfx, clique/
// hover de MUNDO na escolha de ator e na mira) e o roteamento de TECLADO (battle_key_down +
// battle_digit_for_key, atalhos numericos). Extraido verbatim de battle_preview.cpp - ZERO
// mudanca de comportamento, so de arquivo (module atomico: input isolado do resto da casca -
// montagem RML, resolucao de assets, loop de render - que seguem em modulos irmaos).
//
// Tudo aqui e testavel HEADLESS (sem SDL_Init nem janela - SDL_Keycode/SDL_Event sao so
// structs/enums de compilacao; gusengine_app ja linka SDL3, ver app/CMakeLists.txt).
//
// Cross-ref: gus/app/screens/battle_preview.hpp (casca dona do loop de render, unico
//            chamador de producao destas funcoes); gus/app/screens/battle_scene.hpp (motor
//            que estas funcoes dirigem); gus/app/screens/battle_cockpit_verb_ids.hpp
//            (mapeamento id RCSS -> indice de verbo, usado por battle_cockpit_verb_click);
//            app/tests/battle_key_routing_test.cpp (suite headless de battle_key_down);
//            docs/tech/adr/ADR-019 (arquitetura de conteudo/apresentacao atomica que motiva
//            esta extracao, onda AC-E11).

#ifndef GUS_APP_SCREENS_BATTLE_INPUT_HPP
#define GUS_APP_SCREENS_BATTLE_INPUT_HPP

#include <SDL3/SDL.h>

#include "gus/app/screens/battle_scene.hpp"

// Forward-decl PROPOSITAL (nao #include <glintfx/ui_event.hpp> aqui): este header e
// PUBLICO dentro de gusengine_app (battle_preview.hpp o inclui, main.cpp inclui
// battle_preview.hpp) mas glintfx e linkado PRIVATE (GATE de 4 camadas / vazamento de
// dependencia externa - ver app/CMakeLists.txt). Um #include cheio aqui vazaria os headers
// do glintfx pra fora de app/ via qualquer TU que inclua battle_preview.hpp transitivamente
// (main.cpp nao linka glintfx::glintfx direto - so gusengine_app o faz). A definicao
// completa (com UiEvent::Type/campos) so e' necessaria em battle_input.cpp, que inclui o
// header real.
namespace glintfx {
struct UiEvent;
}  // namespace glintfx

namespace gus::app::screens {

// MENU-PAUSA-CONFIG-SOM (M7-COSTURA, INTEGRACAO FINAL): sinal devolvido por
// battle_key_down (out-param opcional) quando o Esc bateu na PILHA VAZIA (nenhum
// modal de combate aberto). None = nada disso aconteceu (tecla sem esse efeito
// especifico). OpenPauseMenu = o Esc pediu o MENU DE PAUSA - so reportado quando o
// CHAMADOR passa um `out_effect` nao-nulo (ver battle_key_down); nesse caso
// `running` NAO e mexido (o viewer continua rodando; o chamador decide o que fazer
// com o pedido, tipicamente abrindo gus/app/screens/system_menu_loop.hpp).
enum class BattleEscEffect {
    None,
    OpenPauseMenu,
};

// Digito 1-9 de uma tecla numerica (fileira OU numpad); 0 se nao for numerica 1-9. Fonte
// unica do mapeamento tecla->N pros atalhos numericos (mira e escolha de ator). Exposto
// pra ser CHAMAVEL pelo self-test sintetico E pelos testes Catch2 do roteamento de teclado
// (battle_key_routing_test.cpp) - headless, sem SDL_Init.
[[nodiscard]] int battle_digit_for_key(SDL_Keycode key) noexcept;

// Roteamento de TECLADO do host (extraido do loop de eventos - ver definicao/comentario
// completo em battle_input.cpp). Esc DESEMPILHA 1 nivel de modal por vez (FIX bug2 do
// playtest do lider: "Esc fecha a tela" com um picker/preview aberto) - mira > preview de
// ator > picker > (pilha vazia). NA PILHA VAZIA, o comportamento depende de `out_effect`:
//   out_effect == nullptr (default - PRESERVA TODO call-site existente, inclusive os
//     casos de app/tests/battle_key_routing_test.cpp e o --battle STANDALONE/selftests):
//     `running` vira false (sai do viewer), COMPORTAMENTO INTACTO de sempre.
//   out_effect != nullptr (o HOST REAL, run_battle_preview_embedded): `running` NAO
//     muda; `*out_effect` vira OpenPauseMenu - o chamador abre o menu de pausa (nested
//     loop) e so entao decide se running deve virar false (ex.: o jogador escolheu Sair
//     ou fechou a janela DURANTE o menu).
// Exposto pra ser testavel headless (Catch2), sem abrir janela SDL nem chamar SDL_Init.
void battle_key_down(BattleScene& scene, SDL_Keycode key, bool& running,
                      BattleEscEffect* out_effect = nullptr);

// ADR-010 F1 SMOKE: ponte SDL_Event -> glintfx::UiEvent (mapa de design da doc de prep).
// Retorna false p/ eventos sem mapeamento (nao injeta ruido). Cobre mouse-move/button,
// teclas de navegacao, texto e resize (pixels reais via SDL_GetWindowSizeInPixels). 'Type'
// e enum class (scoped) na v0.2.1 -> UiEvent::Type::*.
bool sdl_to_glintfx(const SDL_Event& ev, SDL_Window* window, glintfx::UiEvent* out);

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
bool battle_cockpit_verb_click(BattleScene& scene, const char* element_id);

// ADR-010 / Incremento A2 (MOUSE), revisado GLINTFX-CLICK: hit-tests de MUNDO/ARENA
// (escolha de ator + mira de alvo) resolvidos AQUI, no host, em coordenadas de MUNDO
// (a projecao ESTICA 960x540 pra pw x ph, NAO-uniforme -- ver viewport_transform
// world_to_screen -> world_x = px/pw*960, world_y = px/ph*540, Y por ph!). O hit-test vem
// do motor de cena (actor_pick_index_at_arena / aim_index_at_arena, casam
// arena_rect_for_actor). O clique nos PILLS DE VERBO NAO passa por aqui -- resolvido pelo
// callback do glintfx (battle_cockpit_verb_click acima), que roda em paralelo a esta
// funcao pro MESMO evento SDL (ver o loop de eventos em battle_preview.cpp: ambos os
// caminhos recebem o clique). Pressuposto: mouse em px de janela == px do viewport (sem
// HiDPI neste alvo; MESMO pressuposto do forward glintfx). Se houver escala HiDPI no
// futuro, converter mouse(pontos)->px antes.
void battle_mouse_click(BattleScene& scene, float mx, float my, int pw, int ph);

// Incremento A2 (HOVER, nice-to-have): SO o inimigo. Durante a mira, mover o mouse sobre um
// inimigo PRE-SELECIONA ele (reusa o realce multimodal do alvo do Incremento A, dirigido por
// aim_target()). ZERO risco: nao toca RCSS nem o estado do glintfx. Pill NAO tem hover: o
// realce .sel do RCSS e dirigido pelo MOTOR (data-class-sel); um :hover exigiria mexer na RCSS
// aprovada do cockpit -> fora do escopo A2 (documentado; o CLIQUE no pill ja funciona).
void battle_mouse_hover(BattleScene& scene, float mx, float my, int pw, int ph);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_INPUT_HPP
