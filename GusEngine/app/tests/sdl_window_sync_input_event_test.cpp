// GusEngine/app/tests/sdl_window_sync_input_event_test.cpp
//
// F4-1a QA-FOLLOWUP (auditoria adversarial, achado 1): SdlWindow::sync_input_event
// tinha ZERO teste chamando a funcao de verdade - a auditoria mutou 3 jeitos (no-op;
// repassa tudo incluindo KEY_DOWN; so FOCUS_LOST, dropa KEY_UP) e os 3 SOBREVIVERAM
// (2417/2417 verde nos 3 casos). O teste antigo "[f4-1a][regressao-dialogo]" de
// platform/tests/sdl_input_test.cpp chamava SdlInput::handle_event() DIRETO,
// pulando exatamente a peca que esta fatia introduz.
//
// Este arquivo chama gus::app::SdlWindow::sync_input_event() DE VERDADE (headless -
// SdlWindow() nao precisa de SDL_Init/janela/GL, MESMO estado que sdl_window_
// marker_defer_test.cpp ja prova seguro) e observa o efeito via input_dx()/
// input_dy() (novo accessor so-leitura) - mata os 3 mutantes:
//   - "no-op" e "so FOCUS_LOST" (dropam KEY_UP): o teste de SOLTA falha (dx
//     continuaria 1 em vez de virar 0).
//   - "repassa tudo incluindo KEY_DOWN": o teste de PRESS-filtrado falha (dx
//     viraria 1 mesmo sem nenhuma pressao legitima).
// process_key_for_test() (novo, "caminho TESTAVEL sem SDL", MESMO padrao de
// SdlInput::process_key/mutable_gamepad) arma a precondicao "D ja pressionado
// ANTES do modal" sem depender de SDL_PushEvent (que FALHA sem SDL_Init,
// confirmado empiricamente antes de escrever este teste).

#include <catch2/catch_test_macros.hpp>

#include "gus/app/sdl_window.hpp"

using gus::app::SdlWindow;

namespace {

constexpr int kKeyD = 'd';  // SDLK_D - MESMA tecla/constante de sdl_input_test.cpp

SDL_Event make_key_event(Uint32 type, SDL_Keycode key, bool repeat = false) {
    SDL_Event ev{};
    ev.type = type;
    ev.key.key = key;
    ev.key.repeat = repeat;
    return ev;
}

}  // namespace

TEST_CASE("SdlWindow::sync_input_event repassa KEY_UP - solta uma tecla ja "
          "pressionada (mata os mutantes 'no-op' e 'so FOCUS_LOST/dropa KEY_UP')",
          "[sdlwindow][f4-1a][mutation]") {
    SdlWindow city;  // ctor headless - render2d_==nullptr, input_ zerado

    city.process_key_for_test(kKeyD, /*pressed=*/true);  // segura D ANTES do modal
    REQUIRE(city.input_dx() == 1);

    const SDL_Event key_up = make_key_event(SDL_EVENT_KEY_UP, kKeyD);
    city.sync_input_event(key_up);  // a SOLTA "durante o modal", ao vivo

    REQUIRE(city.input_dx() == 0);  // D nao fica presa - mata "no-op"/"so FOCUS_LOST"
}

TEST_CASE("SdlWindow::sync_input_event NAO repassa KEY_DOWN (mata o mutante "
          "'repassa tudo')",
          "[sdlwindow][f4-1a][mutation]") {
    SdlWindow city;  // D nunca foi pressionado por nenhum caminho legitimo

    const SDL_Event key_down = make_key_event(SDL_EVENT_KEY_DOWN, kKeyD);
    city.sync_input_event(key_down);

    // Se o mutante "repassa tudo" estivesse ativo, isto teria armado D=pressed
    // (dx()==1) so por um evento que chegou pelo sync_hook - o filtro real NAO
    // deixa isso acontecer (ver o racional completo no .hpp: uma tecla de
    // NAVEGACAO do proprio modal nao pode "vazar" pra movimento da cidade).
    REQUIRE(city.input_dx() == 0);
}

TEST_CASE("SdlWindow::sync_input_event repassa WINDOW_FOCUS_LOST (zera tudo, "
          "MESMO efeito de SdlInput::clear())",
          "[sdlwindow][f4-1a][mutation]") {
    SdlWindow city;
    city.process_key_for_test(kKeyD, /*pressed=*/true);
    REQUIRE(city.input_dx() == 1);

    SDL_Event focus_lost{};
    focus_lost.type = SDL_EVENT_WINDOW_FOCUS_LOST;
    city.sync_input_event(focus_lost);

    REQUIRE(city.input_dx() == 0);
}

TEST_CASE("SdlWindow::sync_input_event ignora um tipo IRRELEVANTE (MOUSE_MOTION) "
          "- nao mexe no input de movimento",
          "[sdlwindow][f4-1a][mutation]") {
    SdlWindow city;
    city.process_key_for_test(kKeyD, /*pressed=*/true);
    REQUIRE(city.input_dx() == 1);

    SDL_Event motion{};
    motion.type = SDL_EVENT_MOUSE_MOTION;
    city.sync_input_event(motion);

    REQUIRE(city.input_dx() == 1);  // intocado - MOUSE_MOTION nao e KEY_UP/FOCUS_LOST
}
