// gus/app/screen_state.hpp
//
// F4-1a (onda F4 "casca SDL -> App mode do glintfx", fatia 1: converter os loops
// modais BLOQUEANTES - cada um com o proprio SDL_PollEvent independente - num
// ESTADO tickado por um loop UNICO, com um UNICO pump de eventos por frame).
// Escopo desta fatia: SO gus/app/screens/npc_dialogue_loop_gl.cpp (a tela do
// bug relatado ao vivo pelo lider, "Gus anda sozinho apos fechar o dialogo") -
// as outras 5 telas (dificuldade, salvar/carregar, titulo, pausa/sistema,
// batalha) ficam pra fatias futuras (F4-1b em diante), reusando o MESMO
// contrato definido aqui. Decisao de FORMA (nao inventada por este agente -
// escolha do lider, apos eu propor o corte menor): objeto com CICLO DE VIDA
// EXPLICITO (enter/handle_event/tick/finished/exit), nao closures de lambda
// fechando sobre estado local (o padrao antigo de title_menu_loop.cpp/
// system_menu_loop.cpp/battle_preview.cpp/etc) - sobrevive a troca de casca da
// F4-3 (o LOOP de cima muda quando a casca SDL virar o App mode do glintfx; o
// CONTRATO de tela nao muda).
//
// EXCLUSIVIDADE DO UILAYER (requisito DURO, nao detalhe de implementacao):
// abrir um 2o glintfx::UiLayer com o 1o AINDA vivo crashava DE VERDADE ao vivo
// ("Element meta pool not empty on shutdown, 75 object(s) leaked" - ver
// gus/app/src/screens/battle_preview.cpp:1043-1059, o historico do revert
// BATTLE-ESC-PAUSE-ACTOR-LIST). run_screen_state() abaixo empurra essa
// invariante pra FORA da disciplina de quem escreve a proxima tela:
//   1) toma UMA UNICA ScreenState& (nunca um vector/pilha de ponteiros - nao
//      ha COMO passar 2 "correntes" ao mesmo tempo, a API nao oferece isso);
//   2) chama enter() (onde a tela cria seus recursos GL/UiLayer) SEMPRE antes
//      do 1o handle_event/tick, e exit() (onde ela os libera) SEMPRE logo
//      apos o ultimo tick - nunca os dois intercalados, nunca exit() pulado;
//   3) telas ANINHADAS (fatias futuras - ex.: titulo->dificuldade, pausa->
//      salvar/carregar) devem seguir o MESMO padrao ja usado pelos loops
//      antigos (ui_opt.reset() ANTES da chamada aninhada, ui_opt.emplace()
//      DEPOIS que ela retorna): a tela PAI so pode considerar a UiLayer da
//      FILHA destruida (ela ja retornou) antes de recriar a sua propria - ou
//      seja, run_screen_state(pai) deve retornar (chamando pai.exit(), que
//      libera a UiLayer do pai) ANTES de qualquer codigo chamar
//      run_screen_state(filha). Isso e garantido pela propria sequencialidade
//      de chamada de funcao em C++ (run_screen_state(pai) so devolve o
//      controle DEPOIS de rodar pai.exit()) - nao depende de o autor da FILHA
//      "lembrar" de nada.
//
// PUMP UNICO + clear_input() (a raiz do bug, ver o comentario de
// SdlWindow::clear_input() em sdl_window.hpp): o loop antigo de cada tela
// fazia o PROPRIO SDL_PollEvent, entao um SDL_EVENT_KEY_UP que acontecesse
// DURANTE o modal nunca chegava no SdlInput persistente da cidade (input_ so
// via eventos dentro de SdlWindow::step(), que a Maestro NAO chama enquanto o
// modal esta aberto) - dai a tecla "grudava" pressionada. run_screen_state()
// pumpa 1 UNICA vez por frame e entrega o MESMO SDL_Event tanto pra tela
// corrente (handle_event) QUANTO, se `sync_hook` for fornecido, pro estado de
// input persistente (ex.: gus::app::SdlWindow::sync_input_event) - a cidade
// "ve" o evento no MESMO instante em que ele acontece, nao so num flush ao
// entrar/sair. Ver gus/app/src/screens/npc_dialogue_loop_gl.cpp e
// gus/app/maestro.cpp::to_npc_dialogue (o consumidor real desta fatia).

#ifndef GUS_APP_SCREEN_STATE_HPP
#define GUS_APP_SCREEN_STATE_HPP

#include <functional>

#include <SDL3/SDL.h>

namespace gus::app {

// Contrato MINIMO que toda tela modal (dialogo nesta fatia; dificuldade/
// salvar-carregar/titulo/pausa/batalha nas fatias futuras) implementa.
class ScreenState {
public:
    virtual ~ScreenState() = default;

    // Chamado UMA vez por run_screen_state(), antes do 1o handle_event/tick -
    // e o UNICO lugar onde recursos GL/UiLayer/ids de SFX desta tela sao
    // criados (ver a nota de exclusividade acima). Pode desenhar o 1o frame
    // (varias telas antigas desenhavam 1 frame antes do while(true) real -
    // preservar isso aqui, se aplicavel, e responsabilidade de cada tela:
    // run_screen_state() nao assume nada sobre o 1o frame).
    virtual void enter() = 0;

    // Um SDL_Event JA pumpado pelo runner - esta funcao NUNCA chama
    // SDL_PollEvent (contrato do "1 unico pump", ver run_screen_state()).
    virtual void handle_event(const SDL_Event& ev) = 0;

    // Avanca (e desenha) 1 frame; dt em segundos (MESMO relogio real de
    // sempre - diferenca de SDL_GetTicksNS/now_fn injetado). As telas antigas
    // ja faziam update+render juntos (reload()+present_frame() no fim de cada
    // iteracao do while(true) antigo) - nao ha necessidade de separar nesta
    // fatia (YAGNI: se uma fatia futura precisar separar update de render,
    // separa la, sem mudar este contrato base).
    virtual void tick(float dt) = 0;

    // true quando a tela terminou (ex.: dialogo alcancou @exit) OU a janela
    // foi fechada (ver window_closed() abaixo) - run_screen_state() para de
    // chamar handle_event/tick assim que isto (ou window_closed()) vira true,
    // e chama exit() em seguida.
    [[nodiscard]] virtual bool finished() const = 0;

    // Chamado UMA vez, logo apos o ultimo tick() (ou logo apos enter(), se a
    // tela terminar sem nunca entrar no loop - ex.: glintfx::UiLayer::ok()==
    // false, OU um self-test headless que bypassa o loop interativo de
    // proposito) - libera os recursos criados em enter(). Depois de exit(), a
    // PROXIMA tela (se houver, ver a nota de aninhamento acima) pode chamar
    // seu proprio enter() com seguranca.
    virtual void exit() = 0;

    // true se o jogador pediu pra FECHAR A JANELA durante esta tela (SDL_
    // EVENT_QUIT) - MESMO contrato de retorno bool que todo loop modal antigo
    // ja tinha (to_battle()/open_pause_from_city()/to_npc_dialogue()): o
    // chamador (Maestro::run()) encerra o programa se isto for true.
    [[nodiscard]] virtual bool window_closed() const = 0;
};

// Hook de sincronizacao: recebe TODO evento pumpado por run_screen_state(),
// no MESMO frame em que run_screen_state() os entrega a screen.handle_event()
// - a peca que fecha o bug "Gus anda sozinho apos fechar o dialogo" pela RAIZ
// (ver o comentario grande no topo do arquivo). Tipicamente
// gus::app::SdlWindow::sync_input_event; opcional (nullptr = nao sincroniza
// nada, MESMO comportamento de uma tela sem estado de input persistente por
// tras, ex.: um self-test/probe standalone).
using EventSyncHook = std::function<void(const SDL_Event&)>;

// Fonte de eventos "ja pumpados" - por padrao (nullptr) SDL_PollEvent real.
// Testes headless injetam uma fonte SINTETICA (sequencia de SDL_Event
// pre-fabricados, sem precisar de SDL_Init/janela/display nenhum) - devolve
// true enquanto houver evento (preenchendo `ev`), false quando a fila esgota
// (MESMO contrato de retorno de SDL_PollEvent).
using EventPollFn = std::function<bool(SDL_Event& ev)>;

// Relogio monotonico em nanossegundos - por padrao (nullptr) SDL_GetTicksNS
// real. Testes injetam uma sequencia SINTETICA deterministica (sem depender
// de tempo real nem de nenhum subsistema SDL).
using ClockNowNsFn = std::function<unsigned long long()>;

// O LOOP UNICO desta fatia: 1 pump por iteracao (poll_fn, default
// SDL_PollEvent), despachando CADA evento pumpado pra `sync_hook` (se
// fornecido) E para `screen.handle_event()`, nesta ORDEM (a cidade "ve" o
// evento tao cedo quanto a propria tela). Roda screen.enter() uma vez antes
// do loop; screen.tick(dt) uma vez por iteracao de pump (dt real, via now_fn,
// default SDL_GetTicksNS) - SO se a tela nao tiver terminado/fechado a janela
// durante o proprio drenar de eventos desta iteracao (MESMO guard que os
// loops antigos ja faziam: "if (window_closed || finished) break;" antes do
// reload+present do frame - nao desenha 1 frame extra depois do evento que
// encerrou a tela); e screen.exit() UMA vez ao sair (finished() OU
// window_closed()) - GARANTE a exclusividade do UiLayer descrita no topo do
// arquivo (so este runner decide quando enter()/exit() rodam, nunca 2 telas
// com enter() chamado sem o exit() da anterior ja ter rodado).
void run_screen_state(ScreenState& screen, const EventSyncHook& sync_hook = nullptr,
                      EventPollFn poll_fn = nullptr, ClockNowNsFn now_fn = nullptr);

}  // namespace gus::app

#endif  // GUS_APP_SCREEN_STATE_HPP
