// gus/app/tools/poc_gl_single_context_probe.cpp
//
// POC (passo 1, GATE) do plano FLASH-CTX Opcao C - contexto de video UNICO
// (docs/tech/pivot/menu-flash-contexto-unico-plano.md, secao 3 Opcao C / secao 5
// passo 1). Prova, headless (Xvfb), que:
//
//   1. UMA janela SDL_WINDOW_OPENGL + UM contexto GL 3.3 core/stencil 8, vivo do
//      inicio ao fim, serve TANTO a cidade (Render2dGl3, via OverworldSim::render -
//      que so fala com IRenderer, entao nao muda NADA da cidade) QUANTO o menu de
//      sistema (glintfx::UiLayer, via gus::app::screens::run_system_menu_loop_
//      gl_current - o nucleo "assume contexto corrente" que JA EXISTE hoje, usado
//      em producao pela batalha).
//   2. O menu abre e fecha N vezes NESSE MESMO contexto, sem NENHUM
//      SDL_CreateRenderer/SDL_DestroyRenderer/SDL_GL_DestroyContext no meio (o
//      ponto inteiro: sem troca de contexto, o SDL_ReconfigureWindow que causa o
//      flash - ver diagnostico no plano - nunca roda).
//   3. glGetError() e checado nos pontos-chave; RSS (VmRSS) e amostrado antes/
//      depois dos ciclos (vazamento de UiLayer/contexto-longevo).
//   4. PNGs de evidencia: a cidade em Render2dGl3 (comparada com uma referencia
//      Render2dSdl, gerada numa janela/renderer DESCARTAVEL e SEPARADA - NAO faz
//      parte da arquitetura sob teste, so da o "control group" visual) + frames
//      logo apos cada fechamento de menu.
//
// ISOLADO: nao toca gus/app/maestro.cpp nem gus/app/sdl_window.cpp (producao).
// Alvo de CMake proprio (ver app/tools/CMakeLists.txt, target
// poc_gl_single_context_probe) - nao entra no build principal (linux-release).
//
// Uso: GUSWORLD_POC_OUT=<dir> ./poc_gl_single_context_probe (default: /var/tmp/
// gusworld_poc_gl_single_context). Sob Xvfb: DISPLAY=:99 (NUNCA :0).

#include <SDL3/SDL.h>

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/city_loader.hpp"
#include "gus/app/screens/overworld_sim.hpp"
#include "gus/app/screens/player_sprites_loader.hpp"
#include "gus/app/screens/system_menu_loop.hpp"
#include "gus/core/asset_paths.hpp"
#include "gus/platform/audio/audio_engine.hpp"
#include "gus/platform/fs/save_file_store.hpp"
#include "gus/platform/fs/settings_file_store.hpp"
#include "gus/platform/render2d/render2d_gl3.hpp"
#include "gus/platform/render2d/render2d_sdl.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"

// glad (DECLARACOES; a IMPLEMENTACAO ja mora dentro de gusengine_platform.a, ver
// gl3_loader.cpp - GLAD_GL_IMPLEMENTATION NAO e definido aqui de proposito, senao
// duplicaria simbolos). So pra termos glGetError/GL_NO_ERROR disponiveis, MESMA
// tecnica que qualquer TU de platform/ ja usa.
#include "RmlUi_Include_GL3.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace fs = std::filesystem;

namespace {

constexpr int kWindowW = 1280;
constexpr int kWindowH = 720;
constexpr int kMenuCycles = 5;

int g_gl_error_count = 0;

// Drena TODOS os glGetError() pendentes (podem empilhar) e loga cada um. Devolve
// true se limpo (nenhum erro desde a ultima checagem).
bool check_gl(const char* stage) {
    bool any = false;
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "[GL ERROR] " << stage << ": 0x" << std::hex << err << std::dec
                  << "\n";
        any = true;
        ++g_gl_error_count;
    }
    if (!any) {
        std::cout << "[GL OK] " << stage << "\n";
    }
    return !any;
}

long vm_rss_kb() {
    std::ifstream f("/proc/self/status");
    std::string line;
    while (std::getline(f, line)) {
        if (line.rfind("VmRSS:", 0) == 0) {
            long kb = 0;
            std::sscanf(line.c_str(), "VmRSS: %ld kB", &kb);
            return kb;
        }
    }
    return -1;
}

std::string out_dir() {
    const char* env = std::getenv("GUSWORLD_POC_OUT");
    if (env != nullptr && env[0] != '\0') {
        return env;
    }
    return "/var/tmp/gusworld_poc_gl_single_context";
}

bool write_png(const std::string& path, int w, int h, const unsigned char* rgba) {
    const int ok = stbi_write_png(path.c_str(), w, h, 4, rgba, w * 4);
    std::cout << "PNG salvo: " << path << " (ok=" << ok << ")\n";
    return ok != 0;
}

// REFERENCIA (control group): a MESMA cidade (mesmo .gmap, mesmo spawn - o loader
// e deterministico, ver city_loader.cpp) via Render2dSdl, numa janela/renderer
// PROPRIOS e DESCARTADOS logo em seguida. NAO faz parte do contexto GL unico sob
// teste - so gera o PNG de comparacao visual (criterio "paridade visual").
bool render_sdl_reference(const std::string& out_path) {
    SDL_Window* window = SDL_CreateWindow("poc_ref_sdl", kWindowW, kWindowH,
                                          SDL_WINDOW_HIDDEN);
    if (window == nullptr) {
        std::cerr << "[ref-sdl] SDL_CreateWindow falhou: " << SDL_GetError() << "\n";
        return false;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr) {
        std::cerr << "[ref-sdl] SDL_CreateRenderer falhou: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        return false;
    }
    std::cout << "[ref-sdl] driver=" << SDL_GetRendererName(renderer) << "\n";

    gus::platform::render2d::Render2dSdl r2d(renderer);
    gus::app::screens::CityLoadOutcome city = gus::app::screens::load_city_or_fallback();
    gus::app::screens::OverworldSim sim(std::move(city.sim));
    const std::string gus_dir = gus::app::screens::resolve_sprites_dir(
        std::string(gus::core::assets::kGusSpritesDir));
    sim.set_player_sprites(gus::app::screens::load_gus_sprites(r2d, gus_dir));

    sim.render(r2d, static_cast<float>(kWindowW), static_cast<float>(kWindowH), 0.0f);

    SDL_Surface* surf = SDL_RenderReadPixels(renderer, nullptr);
    bool ok = false;
    if (surf != nullptr) {
        SDL_Surface* conv = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);
        if (conv != nullptr) {
            ok = write_png(out_path, conv->w, conv->h,
                           static_cast<const unsigned char*>(conv->pixels));
            SDL_DestroySurface(conv);
        }
        SDL_DestroySurface(surf);
    } else {
        std::cerr << "[ref-sdl] SDL_RenderReadPixels falhou: " << SDL_GetError()
                  << "\n";
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return ok;
}

}  // namespace

int main() {
    const std::string outdir = out_dir();
    std::error_code ec;
    fs::create_directories(outdir, ec);

    // GUSWORLD_HOME isolado: este probe chama o menu de sistema DE VERDADE (nao um
    // mock), entao settings.json/saves seriam escritos - isola do ~/.gusworld real.
    const std::string gusworld_home = (fs::path(outdir) / "gusworld_home").string();
    fs::create_directories(gusworld_home, ec);
    setenv("GUSWORLD_HOME", gusworld_home.c_str(), 1);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }

    std::cout << "======================================================\n"
              << "POC FLASH-CTX passo 1: contexto GL UNICO\n"
              << "(cidade Render2dGl3 + menu glintfx no MESMO contexto,\n"
              << kMenuCycles << " ciclos abrir/fechar, SEM recriar nada)\n"
              << "outdir=" << outdir << "\n"
              << "======================================================\n";

    // ------------------------------------------------------------------
    // 0) REFERENCIA (control group): cidade via Render2dSdl, janela PROPRIA e ja
    // descartada ANTES do contexto GL unico nascer.
    // ------------------------------------------------------------------
    const std::string ref_sdl_png =
        (fs::path(outdir) / "poc_city_sdl_reference.png").string();
    const bool ref_ok = render_sdl_reference(ref_sdl_png);
    std::cout << "[criterio paridade] referencia Render2dSdl gerada = " << ref_ok
              << "\n";

    // ------------------------------------------------------------------
    // 1) O CONTEXTO GL UNICO - vivo ate o fim do processo (MESMA receita de
    // run_battle_preview_embedded / run_system_menu_loop_owning_gl: profile core
    // 3.3, doublebuffer, stencil 8).
    // ------------------------------------------------------------------
    SDL_Window* window =
        SDL_CreateWindow("poc_gl_single_context", kWindowW, kWindowH,
                         SDL_WINDOW_OPENGL);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow (GL) falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GLContext gl = SDL_GL_CreateContext(window);
    if (gl == nullptr) {
        std::cerr << "SDL_GL_CreateContext falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_GL_MakeCurrent(window, gl);
    SDL_GL_SetSwapInterval(1);
    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        std::cerr << "gl3_load_functions falhou\n";
        return 1;
    }
    check_gl("apos boot do contexto GL unico");

    int pw = 0, ph = 0;
    SDL_GetWindowSizeInPixels(window, &pw, &ph);
    std::cout << "janela GL " << pw << "x" << ph << " pixels\n";

    bool gl_error_free = true;
    bool all_cycles_ok = true;
    bool all_quit_false = true;

    // Escopo proprio: 'renderer'/'sim'/'pixels' morrem ANTES do contexto/janela
    // (SDL_GL_DestroyContext/SDL_DestroyWindow abaixo) - MESMA ordem de
    // battle_preview.cpp (o bloco { Render2dGl3 renderer(...); ... } antes do
    // shutdown do contexto).
    {
        // --------------------------------------------------------------
        // 2) A cidade REAL em Render2dGl3 - OverworldSim::render so fala com
        // IRenderer, entao NADA muda em app/screens/overworld_sim.*; so o backend
        // concreto passado troca de Render2dSdl para Render2dGl3.
        // --------------------------------------------------------------
        gus::platform::render2d::Render2dGl3 renderer(/*gl_active=*/true);
        gus::app::screens::CityLoadOutcome city =
            gus::app::screens::load_city_or_fallback();
        gus::app::screens::OverworldSim sim(std::move(city.sim));
        const std::string gus_dir = gus::app::screens::resolve_sprites_dir(
            std::string(gus::core::assets::kGusSpritesDir));
        sim.set_player_sprites(gus::app::screens::load_gus_sprites(renderer, gus_dir));
        gl_error_free &= check_gl("apos load_texture (sprites do Gus, GL3)");

        std::vector<unsigned char> pixels(static_cast<std::size_t>(pw) * ph * 4);

        // Desenha 1 frame da cidade (Render2dGl3) e captura o backbuffer ANTES do
        // swap (gl3_read_backbuffer_rgba le GL_BACK) - depois faz o swap.
        auto render_and_capture_city = [&](const std::string& path,
                                           const char* gl_stage) {
            sim.render(renderer, static_cast<float>(pw), static_cast<float>(ph),
                      0.0f);
            gl_error_free &= check_gl(gl_stage);
            gus::platform::rmlui::gl3_read_backbuffer_rgba(pw, ph, pixels.data());
            write_png(path, pw, ph, pixels.data());
            SDL_GL_SwapWindow(window);
        };

        const std::string gl3_png =
            (fs::path(outdir) / "poc_city_gl3_frame0.png").string();
        render_and_capture_city(gl3_png, "apos sim.render (cidade GL3, frame estavel)");
        std::cout << "[criterio a] cidade em Render2dGl3 (frame estavel) capturada.\n";

        // --------------------------------------------------------------
        // 3) N ciclos de abrir/fechar o menu de sistema NO MESMO CONTEXTO, sem
        // destruir nada. run_system_menu_loop_gl_current e o nucleo "assume
        // contexto corrente" que JA EXISTE em producao (usado hoje pela batalha);
        // aqui o chamamos direto da CIDADE, provando a peca central da Opcao C.
        //
        // A funcao e BLOQUEANTE (loop interativo de verdade - poll de eventos +
        // present_frame ate ESC/Sair/fechar-janela). Simulamos o jogador
        // apertando ESC (fecha o Pause = "Continuar", NAO "Sair") de uma THREAD
        // separada apos ~150ms - tempo de sobra pro loop real desenhar VARIOS
        // frames do menu (present_frame roda a cada iteracao do while(true),
        // sem delay) antes de fechar; prova que o menu de fato RENDERIZOU no
        // contexto (nao so abriu-e-fechou sem desenhar nada).
        // --------------------------------------------------------------
        gus::app::i18n::Translator translator;
        const std::string tr_path = gus::app::i18n::resolve_translations_path();
        if (!translator.load_from_file(tr_path)) {
            std::cerr << "Translator::load_from_file falhou: " << tr_path << "\n";
            return 1;
        }
        gus::platform::audio::AudioEngine audio(/*device_active=*/false);
        const std::string settings_dir = gus::platform::fs::resolve_settings_dir();
        const std::string saves_dir = gus::platform::fs::resolve_saves_dir();

        const long rss_before_all = vm_rss_kb();
        std::cout << "\n[soak] VmRSS antes dos " << kMenuCycles
                  << " ciclos = " << rss_before_all << " kB\n";

        for (int cycle = 0; cycle < kMenuCycles; ++cycle) {
            std::cout << "\n--- ciclo " << cycle
                      << ": abrindo o menu de sistema (mesmo contexto GL) ---\n";
            std::thread closer([window] {
                SDL_Delay(150);
                SDL_Event ev{};
                ev.type = SDL_EVENT_KEY_DOWN;
                ev.key.windowID = SDL_GetWindowID(window);
                ev.key.key = SDLK_ESCAPE;
                ev.key.down = true;
                ev.key.repeat = false;
                SDL_PushEvent(&ev);
            });
            const auto outcome = gus::app::screens::run_system_menu_loop_gl_current(
                window, audio, translator, settings_dir, saves_dir);
            closer.join();
            std::cout << "ciclo " << cycle << ": outcome.quit_app=" << outcome.quit_app
                      << " (esperado: false = Continuar, nao Sair)\n";
            if (outcome.quit_app) {
                all_quit_false = false;
            }
            gl_error_free &=
                check_gl(("apos fechar o menu (ciclo " + std::to_string(cycle) + ")")
                            .c_str());

            // Frames REAIS pos-fechamento, na MESMA cidade/renderer/sim/textura -
            // NUNCA recriados (e exatamente o que a Opcao C promete no lugar da
            // maquina release_renderer/reacquire_renderer que causa o flash).
            const std::string post0 =
                (fs::path(outdir) / ("poc_city_gl3_postclose_cycle" +
                                     std::to_string(cycle) + "_frame0.png"))
                    .string();
            render_and_capture_city(
                post0, ("apos frame pos-close (ciclo " + std::to_string(cycle) + ")")
                          .c_str());

            // 2 swaps de "assentamento" (sem PNG) + um 2o frame capturado - se
            // houvesse corrupcao TRANSITORIA na troca de contexto, apareceria
            // AQUI diferente do frame0 (este POC existe pra provar que NAO ha).
            for (int settle = 0; settle < 2; ++settle) {
                sim.render(renderer, static_cast<float>(pw), static_cast<float>(ph),
                          0.0f);
                SDL_GL_SwapWindow(window);
            }
            const std::string post1 =
                (fs::path(outdir) / ("poc_city_gl3_postclose_cycle" +
                                     std::to_string(cycle) + "_frame1.png"))
                    .string();
            render_and_capture_city(
                post1, ("apos assentamento (ciclo " + std::to_string(cycle) + ")")
                          .c_str());
            std::cout << "[criterio b] ciclo " << cycle
                      << ": 2 frames pos-fechamento capturados.\n";

            const long rss_now = vm_rss_kb();
            std::cout << "ciclo " << cycle << ": VmRSS=" << rss_now
                      << " kB (delta desde o inicio dos ciclos="
                      << (rss_now - rss_before_all) << " kB)\n";
        }

        const long rss_after_all = vm_rss_kb();
        std::cout << "\n[soak] VmRSS apos " << kMenuCycles
                  << " ciclos = " << rss_after_all
                  << " kB (delta total=" << (rss_after_all - rss_before_all)
                  << " kB)\n";

        gl_error_free &= check_gl("final (antes do shutdown do contexto)");
    }  // 'renderer'/'sim'/'pixels' destroem AQUI, contexto ainda corrente.

    std::cout << "\n======================================================\n"
              << "RESUMO\n"
              << "GL errors totais: " << g_gl_error_count
              << " (limpo=" << gl_error_free << ")\n"
              << "todos os " << kMenuCycles
              << " ciclos outcome.quit_app=false (Continuar, nao Sair): "
              << all_quit_false << "\n"
              << "referencia Render2dSdl gerada: " << ref_ok << "\n"
              << "SEM SDL_CreateRenderer/SDL_DestroyRenderer/SDL_GL_DestroyContext "
                 "durante os ciclos (por construcao - so este main() chama "
                 "SDL_GL_CreateContext/DestroyContext, UMA VEZ cada, no boot/"
                 "shutdown; grep no fonte confirma).\n"
              << "PNGs em: " << outdir << "\n"
              << "======================================================\n";

    SDL_GL_DestroyContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();

    const bool pass = gl_error_free && ref_ok && all_cycles_ok;
    return pass ? 0 : 1;
}
