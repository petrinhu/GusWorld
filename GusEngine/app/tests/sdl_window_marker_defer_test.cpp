// gus/app/tests/sdl_window_marker_defer_test.cpp
//
// REGRESSAO do SIGSEGV real (coredump gusworld_app, playtest ao vivo do lider +
// filho, 2026-07-17 17:20): menu de pausa da cidade -> Save/Load -> acao LOAD ->
// crash em SdlWindow::load_enemy_marker_texture() (frame #0), chamado por
// Maestro::apply_loaded_save_data (frame #1).
//
// RAIZ: Maestro::open_pause_from_city() faz city_->release_renderer() ANTES de
// abrir o menu (o menu roda num contexto GL PROPRIO na MESMA janela) -> render2d_
// e o SDL_Renderer da cidade ficam SOLTOS (render2d_ == nullptr) enquanto o menu
// esta vivo. A acao LOAD chama apply_loaded_save_data -> city_->set_enemy_marker
// -> load_enemy_marker_texture(), que derefava render2d_ (nulo) -> SIGSEGV.
//
// Este teste NAO precisa de janela/GL: uma SdlWindow recem-construida ja tem
// render2d_ == nullptr (init/init_attached NUNCA foram chamados) - o MESMO estado
// que release_renderer() produz durante o menu. Antes do fix, a chamada abaixo
// crashava o processo (o ctest inteiro morria); depois do fix ela retorna sem
// tocar render2d_ (defer): o aabb do marcador fica armazenado e reacquire_renderer()
// recarrega a textura ao a cidade retomar (caminho de producao coberto pelo smoke).

#include <catch2/catch_test_macros.hpp>

#include "gus/app/sdl_window.hpp"
#include "gus/core/spatial/grid_collision.hpp"  // gus::core::spatial::Aabb

using gus::core::spatial::Aabb;

// Estado do bug reproduzido: SdlWindow sem renderer (render2d_ == nullptr), igual
// ao contexto do menu de pausa apos release_renderer(). Antes do fix: SIGSEGV.
TEST_CASE(
    "sdl_window: set_enemy_marker sem renderer (menu de pausa) NAO crasha - defer",
    "[sdlwindow][save-load][regression]") {
    gus::app::SdlWindow city;  // ctor so carrega a cidade (POCO); render2d_ == nullptr

    // Chamada EXATA de apply_loaded_save_data (enemy_defeated_ == false ->
    // city_->set_enemy_marker(enemy_aabb_)) que produziu o coredump.
    REQUIRE_NOTHROW(
        city.set_enemy_marker(Aabb{100.0f, 50.0f, 8.0f, 8.0f}));

    // Reaplicar (re-load do mesmo save) tambem e seguro - idempotente sem renderer.
    REQUIRE_NOTHROW(
        city.set_enemy_marker(Aabb{120.0f, 60.0f, 8.0f, 8.0f}));

    // clear tambem nunca toca render2d_ (so o sim_, sempre vivo) - segue seguro.
    REQUIRE_NOTHROW(city.clear_enemy_marker());
}

// Gemeo do marcador do Bertoldo (mesma raiz, mesmo caminho pelo menu de pausa).
TEST_CASE(
    "sdl_window: set_npc_bertoldo_marker sem renderer NAO crasha - defer",
    "[sdlwindow][save-load][regression]") {
    gus::app::SdlWindow city;  // render2d_ == nullptr

    REQUIRE_NOTHROW(
        city.set_npc_bertoldo_marker(Aabb{140.0f, 70.0f, 8.0f, 8.0f}));
    REQUIRE_NOTHROW(city.clear_npc_bertoldo_marker());
}
