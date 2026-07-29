// SPDX-License-Identifier: GPL-3.0-or-later
// PROBE EFEMERO (D2D-2-SENTINELA, verificacao kCockpitTextPx 8px -> 9px): reproduz o
// TRECHO real do cockpit (BattleScene::render, battle_scene_render.cpp ~linha 289-303)
// usando as MESMAS funcoes publicas de layout/metrica que o jogo usa de verdade
// (cockpit_rect/cockpit_portrait_rect/cockpit_hp_bar_rect de battle_layout.hpp,
// bar_fill/bar_fill_rect de battle_hud_model.hpp, text_width de text_metrics.hpp,
// IRenderer::draw_text com o MESMO font atlas via Render2dSdl/SDL_Renderer software -
// sem Xvfb/GL/compositor). NAO instancia BattleScene inteira porque os 5 atores demo
// (make_demo_actors) tem max_hp fixo (55/60/50/28/22, NENHUM chega a 144) e o pedido
// do lider e justamente o CASO DIFICIL "144/144" (Fibonacci, valor real do jogo) - o
// ator aqui e um CombatActor avulso com hp=144, so pra este probe. kCockpitTextPx em
// si e privado (namespace anonimo de battle_scene_render.cpp): os dois tamanhos sao
// passados como PARAMETRO (8.0f = valor ANTES do bump, 9.0f = valor DEPOIS), reproduzindo
// o commit sem duplicar o arquivo inteiro. Descartavel apos a verificacao.

#include <SDL3/SDL.h>

#include <cstdio>
#include <iostream>
#include <string>

#include "gus/app/screens/battle_hud_model.hpp"  // bar_fill/bar_fill_rect
#include "gus/app/screens/battle_layout.hpp"     // cockpit_rect/cockpit_portrait_rect/cockpit_hp_bar_rect
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/platform/render2d/i_renderer.hpp"
#include "gus/platform/render2d/render2d_sdl.hpp"
#include "gus/platform/render2d/text_metrics.hpp"

#include "stb_image_write.h"

namespace {

using gus::app::screens::bar_fill;
using gus::app::screens::bar_fill_rect;
using gus::app::screens::cockpit_hp_bar_rect;
using gus::app::screens::cockpit_portrait_rect;
using gus::app::screens::cockpit_rect;
using gus::core::spatial::Rect;
using gus::domain::combat::CardFamily;
using gus::domain::combat::CombatActor;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::IRenderer;
using gus::platform::render2d::text_width;

constexpr DrawColor kPanelColor{0.106f, 0.133f, 0.220f, 1.0f};   // #1B2238 (kBg1Color real)
constexpr DrawColor kBarBackColor{0.039f, 0.051f, 0.086f, 1.0f}; // #0a0d16 (kSlotDarkColor real)
constexpr DrawColor kHpFillColor{0.247f, 0.725f, 0.478f, 1.0f};  // #3FB97A (kHp real)
constexpr DrawColor kLineColor{0.165f, 0.204f, 0.314f, 1.0f};    // #2a3450 (kLine real)
constexpr DrawColor kCyan{0.133f, 0.827f, 0.933f, 1.0f};         // #22D3EE (kCyan real)
constexpr DrawColor kInk{0.812f, 0.902f, 0.933f, 1.0f};          // #cfe6ee (kInk real)
constexpr DrawColor kSlotDark{0.039f, 0.051f, 0.086f, 1.0f};

// MESMA sequencia de draw_bar() real (battle_scene_render.cpp:222-227): fundo + fill +
// contorno.
void draw_bar(IRenderer& r, const Rect& frame, float fraction, const DrawColor& fill) {
    r.draw_filled_rect(frame, kBarBackColor);
    r.draw_filled_rect(bar_fill_rect(frame, fraction), fill);
    r.draw_rect_outline(frame, kLineColor, 1.0f);
}

// Reproduz o trecho real (nome + barra de HP + numero "hp/max" centrado por cima),
// MESMAS chamadas de cockpit_hp_bar_rect()/text_width()/draw_text() do jogo, so com
// kCockpitTextPx passado como parametro `num_px` (8.0f ANTES / 9.0f DEPOIS do bump).
void render_cockpit_case(IRenderer& r, const CombatActor& actor, float num_px,
                         float name_px, const std::string& out_path) {
    const Rect screen{0.0f, 0.0f, 960.0f, 540.0f};
    r.begin_frame(screen, 960, 540);

    // fundo geral (so pra nao ficar preto puro - contexto visual)
    r.draw_filled_rect(screen, DrawColor{0.047f, 0.059f, 0.102f, 1.0f});

    const Rect cp = cockpit_rect();
    r.draw_filled_rect(cp, kPanelColor);
    r.draw_rect_outline(Rect{cp.x + cp.w - 1.0f, cp.y, 1.0f, cp.h}, kLineColor, 1.0f);

    // Retrato placeholder (sem textura - caminho headless real do jogo).
    const Rect pr = cockpit_portrait_rect();
    r.draw_filled_rect(pr, kSlotDark);
    r.draw_rect_outline(pr, kCyan, 1.0f);

    // Nome, centrado sob o retrato (MESMA formula real de battle_scene_render.cpp:290-293).
    const std::string who = actor.display_name();
    const float nw = text_width(who, name_px);
    r.draw_text(who.c_str(), pr.x + (pr.w - nw) * 0.5f, pr.y + pr.h + 4.0f, name_px,
               kCyan, /*bold=*/true);

    // HP: barra + "hp/max" por cima, centrado (MESMA formula real, linhas 296-303).
    const Rect hp = cockpit_hp_bar_rect();
    draw_bar(r, hp, bar_fill(actor.hp(), actor.max_hp()), kHpFillColor);
    char num[40];
    std::snprintf(num, sizeof(num), "%d/%d", actor.hp(), actor.max_hp());
    const float hw = text_width(num, num_px);
    r.draw_text(num, hp.x + (hp.w - hw) * 0.5f, hp.y + (hp.h - num_px) * 0.5f, num_px,
               kInk, /*bold=*/false);

    std::printf("[%s] hp_bar: x=%.1f y=%.1f w=%.1f h=%.1f | texto '%s' num_px=%.1f "
                "text_w=%.2f (margem lateral=%.2f, margem vertical=%.2f)\n",
                out_path.c_str(), hp.x, hp.y, hp.w, hp.h, num, num_px, hw,
                (hp.w - hw) * 0.5f, (hp.h - num_px) * 0.5f);

    r.end_frame();
}

bool dump_png(SDL_Renderer* renderer, const std::string& out_path) {
    SDL_Surface* surf = SDL_RenderReadPixels(renderer, nullptr);
    if (surf == nullptr) {
        std::cerr << "SDL_RenderReadPixels falhou: " << SDL_GetError() << "\n";
        return false;
    }
    SDL_Surface* conv = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);
    bool ok = false;
    if (conv != nullptr) {
        ok = stbi_write_png(out_path.c_str(), conv->w, conv->h, 4, conv->pixels,
                            conv->pitch) != 0;
        std::cout << "PNG salvo (" << ok << "): " << out_path << "\n";
        SDL_DestroySurface(conv);
    }
    SDL_DestroySurface(surf);
    return ok;
}

}  // namespace

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Window* window =
        SDL_CreateWindow("cockpit_hp_text_probe", 960, 540, SDL_WINDOW_HIDDEN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr) {
        std::cerr << "SDL_CreateRenderer falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    std::cout << "renderer driver = " << SDL_GetRendererName(renderer) << "\n";

    gus::platform::render2d::Render2dSdl r2d(renderer);

    // CASO DIFICIL (pedido explicito do lider): hp/max = 144/144 (Fibonacci, valor
    // real do jogo) + nome de ator mais longo que qualquer um do demo (pra provar que
    // nada colide na coluna do cockpit ao lado). "Cripto-Sentinela-Prime" (23 chars)
    // e mais longo que qualquer display_name canonico usado ate hoje no combate.
    CombatActor hard_actor("probe_hard", "Cripto-Sentinela-Prime", /*max_hp=*/144,
                          /*atk=*/11, /*def=*/5, /*spd=*/9, CardFamily::Criptografico,
                          /*is_player_side=*/true);

    bool all_ok = true;

    render_cockpit_case(r2d, hard_actor, /*num_px=*/8.0f, /*name_px=*/15.0f,
                        "8px (ANTES)");
    all_ok &= dump_png(renderer,
                       "/var/tmp/builds/claude-1000/cockpit-8-vs-9/cockpit_8px_hp144.png");

    render_cockpit_case(r2d, hard_actor, /*num_px=*/9.0f, /*name_px=*/15.0f,
                        "9px (DEPOIS)");
    all_ok &= dump_png(renderer,
                       "/var/tmp/builds/claude-1000/cockpit-8-vs-9/cockpit_9px_hp144.png");

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return all_ok ? 0 : 1;
}
