// gus/app/src/screen_state.cpp
//
// Implementacao de run_screen_state(). Ver o header para o contrato completo
// (F4-1a). PURA o suficiente pra ser testada headless com poll_fn/now_fn
// sinteticos (screen_state_test.cpp) - nao chama SDL_PollEvent/SDL_GetTicksNS
// direto no corpo do loop, so nos defaults de poll_fn/now_fn quando o chamador
// real (produção) nao injeta nada.
//
// F4-1a QA-FOLLOWUP (auditoria adversarial, achado 2): a EXCLUSIVIDADE DO
// UILAYER descrita no .hpp NAO era estrutural de verdade - um ScreenState cujo
// tick()/handle_event() chamasse run_screen_state() de novo (reentrada)
// compilava e RODAVA sem guarda nenhuma (a auditoria provou isto ao vivo,
// 2418/2418 verde com o cenario). A garantia dependia de "quem escreve a
// proxima tela seguir a convencao" - exatamente o que o lider pediu pra NAO
// deixar acontecer. `g_active` (thread_local - nao global puro, pra nao
// acoplar indevidamente threads futuras que quisessem rodar seu PROPRIO loop
// independente) vira a invariante em CODIGO: uma 2a chamada enquanto a 1a
// ainda esta "dentro" (entre enter() e exit()) lanca std::logic_error na
// hora, ANTES de tocar screen.enter() - nunca silenciosamente deixa 2
// ScreenState com recursos GL/UiLayer vivos ao mesmo tempo (o crash real "2
// UiLayers vivos" ja derrubou o lider ao vivo uma vez, ver o .hpp). Prova:
// app/tests/screen_state_test.cpp, caso "[mutation]".

#include <stdexcept>

#include "gus/app/screen_state.hpp"

namespace gus::app {

namespace {

// thread_local (nao global puro): cada thread que eventualmente rodar seu
// PROPRIO run_screen_state (nenhuma hoje - o jogo e single-threaded - mas a
// guarda nao deve impedir isso arbitrariamente) tem sua propria flag.
thread_local bool g_run_screen_state_active = false;

}  // namespace

void run_screen_state(ScreenState& screen, const EventSyncHook& sync_hook,
                      EventPollFn poll_fn, ClockNowNsFn now_fn) {
    if (g_run_screen_state_active) {
        throw std::logic_error(
            "gus::app::run_screen_state: REENTRADA detectada - uma ScreenState "
            "chamou run_screen_state() de dentro do proprio enter()/"
            "handle_event()/tick() (violaria a EXCLUSIVIDADE DO UILAYER, ver "
            "gus/app/screen_state.hpp - o crash real de 2 glintfx::UiLayer "
            "vivos ao mesmo tempo ja derrubou o lider ao vivo uma vez, "
            "'Element meta pool not empty on shutdown'). Aninhamento de telas "
            "deve ser SEQUENCIAL: esta chamada de run_screen_state precisa "
            "RETORNAR (rodando screen.exit()) ANTES que qualquer codigo chame "
            "a PROXIMA run_screen_state - nunca uma de dentro da outra.");
    }

    // RAII: libera a guarda ao sair de run_screen_state por QUALQUER caminho
    // (retorno normal OU uma excecao propagando de enter()/handle_event()/
    // tick()/exit()) - uma excecao nao deve deixar a guarda "travada" pra
    // sempre (o chamador pode querer tentar de novo, ou o processo pode
    // continuar rodando outros ScreenState legitimos depois de tratar o erro).
    struct ActiveGuard {
        ActiveGuard() { g_run_screen_state_active = true; }
        ~ActiveGuard() { g_run_screen_state_active = false; }
    } active_guard;

    if (!poll_fn) {
        poll_fn = [](SDL_Event& ev) { return SDL_PollEvent(&ev); };
    }
    if (!now_fn) {
        now_fn = [] { return SDL_GetTicksNS(); };
    }

    screen.enter();

    bool have_last = false;
    unsigned long long last_ns = 0;

    while (!screen.finished() && !screen.window_closed()) {
        SDL_Event ev;
        while (poll_fn(ev)) {
            if (sync_hook) {
                sync_hook(ev);
            }
            screen.handle_event(ev);
            if (screen.finished() || screen.window_closed()) {
                break;  // MESMO guard dos loops antigos: para de drenar a
                        // fila assim que a tela terminar/fechar a janela -
                        // eventos restantes desta rajada ficam pro PROXIMO
                        // pump (da proxima tela/da cidade), nao se perdem.
            }
        }
        if (screen.finished() || screen.window_closed()) {
            break;  // nao desenha 1 frame extra apos o evento que encerrou.
        }

        const unsigned long long now_ns = now_fn();
        float dt = 0.0f;
        if (have_last) {
            dt = static_cast<float>(now_ns - last_ns) / 1.0e9f;
        }
        have_last = true;
        last_ns = now_ns;

        screen.tick(dt);
    }

    screen.exit();
}

}  // namespace gus::app
