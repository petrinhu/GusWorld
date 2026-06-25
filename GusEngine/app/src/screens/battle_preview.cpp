// gus/app/src/screens/battle_preview.cpp
//
// Ver header. Casca SDL do viewer da BattleScene (esqueleto M5). Reusa Render2dSdl
// (atras de IRenderer) e o mesmo padrao de loop do anim_preview (poll -> render). A
// cena LE o motor de combate; aqui so abrimos janela, carregamos os retratos 48px e
// desenhamos o esqueleto a cada frame. Esc/fechar encerra.

#include "gus/app/screens/battle_preview.hpp"

#include <cstdlib>  // std::getenv
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

#include "gus/app/screens/battle_hud_model.hpp"  // status_icon_file/index
#include "gus/app/screens/battle_scene.hpp"
#include "gus/domain/combat/combat_enums.hpp"  // StatusId
#include "gus/platform/render2d/render2d_sdl.hpp"

// Raiz resources/ do repo, embutida pelo CMake (mesma macro do resolver de sprites).
#ifndef GUSWORLD_ASSETS_DIR
#define GUSWORLD_ASSETS_DIR ""
#endif

namespace gus::app::screens {

namespace {

constexpr int kWindowW = 1280;  // 640x360 * 2 (escala inteira x2, D1)
constexpr int kWindowH = 720;

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) {
        return b;
    }
    if (a.back() == '/') {
        return a + b;
    }
    return a + "/" + b;
}

// Mapeia o id de ator de DEMO -> arquivo de retrato 48px. Os inimigos (inimigoN)
// compartilham retrato_inimigo. Ponto unico; quando os retratos forem por-personagem
// reais, troca-se aqui (ou vira data-driven).
std::string retrato_file_for(const std::string& actor_id) {
    // Gus na BATALHA usa o retrato de COMBATE (meio corpo do sprite de jogo: cabelo
    // revolto + oculos taticos + aparelho + antena + casaco tatico). O retrato_gus.png
    // (terno/formal) NAO entra na luta (vira quadro na casa dos pais / narracoes).
    if (actor_id == "gus") return "retrato_gus_combate.png";
    if (actor_id == "caua") return "retrato_caua.png";
    if (actor_id == "jaci") return "retrato_jaci.png";
    // inimigo1..4 e qualquer outro -> retrato generico de inimigo.
    return "retrato_inimigo.png";
}

}  // namespace

std::string resolve_retratos_dir() {
    if (const char* env = std::getenv("GUSWORLD_ASSETS")) {
        if (env[0] != '\0') {
            return join(env, "sprites/icons-m5/retratos");
        }
    }
    const std::string compiled = GUSWORLD_ASSETS_DIR;
    if (!compiled.empty()) {
        return join(compiled, "sprites/icons-m5/retratos");
    }
    return "resources/sprites/icons-m5/retratos";
}

std::string resolve_status_icons_dir() {
    if (const char* env = std::getenv("GUSWORLD_ASSETS")) {
        if (env[0] != '\0') {
            return join(env, "sprites/icons-m5/status");
        }
    }
    const std::string compiled = GUSWORLD_ASSETS_DIR;
    if (!compiled.empty()) {
        return join(compiled, "sprites/icons-m5/status");
    }
    return "resources/sprites/icons-m5/status";
}

std::string resolve_intent_icons_dir() {
    if (const char* env = std::getenv("GUSWORLD_ASSETS")) {
        if (env[0] != '\0') {
            return join(env, "sprites/icons-m5/intent");
        }
    }
    const std::string compiled = GUSWORLD_ASSETS_DIR;
    if (!compiled.empty()) {
        return join(compiled, "sprites/icons-m5/intent");
    }
    return "resources/sprites/icons-m5/intent";
}

int run_battle_preview() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "BattlePreview: SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Window* window = nullptr;
    SDL_Renderer* sdl_renderer = nullptr;
    if (!SDL_CreateWindowAndRenderer("GusWorld BattlePreview (M5 esqueleto)",
                                     kWindowW, kWindowH, SDL_WINDOW_RESIZABLE,
                                     &window, &sdl_renderer)) {
        std::cerr << "BattlePreview: SDL_CreateWindowAndRenderer falhou: "
                  << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderVSync(sdl_renderer, 1);

    {
        gus::platform::render2d::Render2dSdl renderer(sdl_renderer);

        // A cena monta o encontro de demo e ja le a fila do motor.
        BattleScene scene;

        // Carrega os retratos 48px da fila CTB (handles resolvidos pelo renderer) e os
        // entrega a cena. Cada id de ator -> seu retrato; ausencia degrada pro retangulo.
        const std::string dir = resolve_retratos_dir();
        BattlePortraitSet portraits;
        for (const auto* actor : scene.machine().queue().order()) {
            if (actor == nullptr) {
                continue;
            }
            const std::string path = join(dir, retrato_file_for(actor->id()));
            const gus::platform::render2d::TextureId tex =
                renderer.load_texture(path.c_str());
            portraits.by_id.emplace_back(actor->id(), tex);
        }
        scene.set_portraits(std::move(portraits));

        // Carrega os icones de status (14px), indexados por StatusId (status_icon_index),
        // e os entrega a cena. Ausencia degrada pro quadradinho placeholder.
        const std::string sdir = resolve_status_icons_dir();
        BattleStatusIconSet status_icons;
        for (int i = 0; i < static_cast<int>(status_icons.by_index.size()); ++i) {
            const auto id = static_cast<gus::domain::combat::StatusId>(i);
            const std::string spath = join(sdir, std::string(status_icon_file(id)));
            status_icons.by_index[static_cast<std::size_t>(i)] =
                renderer.load_texture(spath.c_str());
        }
        scene.set_status_icons(status_icons);

        // Carrega os icones de INTENT (telegraph, incremento 5) e os entrega a cena.
        // Ausencia => marca placeholder ambar sobre o inimigo.
        const std::string sdir_intent = resolve_intent_icons_dir();
        BattleIntentIconSet intent_icons;
        intent_icons.atacar =
            renderer.load_texture(join(sdir_intent, "intent_atacar.png").c_str());
        intent_icons.defender =
            renderer.load_texture(join(sdir_intent, "intent_defender.png").c_str());
        intent_icons.aplicar_status = renderer.load_texture(
            join(sdir_intent, "intent_aplicar_status.png").c_str());
        intent_icons.ruido = renderer.load_texture(
            join(sdir_intent, "intent_ruido_patchzero.png").c_str());
        scene.set_intent_icons(intent_icons);

        // Carrega o catalogo de traducao (pt_br.md) e o entrega a cena, pra os verbos do
        // menu aparecerem com NOME legivel (incremento 3.5). Ausencia => fallback (caixa
        // colorida sem nome, mas nao crasha). O Translator vive aqui (casca), a cena so
        // aponta pra ele (nao-dono): mantemos o objeto vivo ate o fim do loop.
        gus::app::i18n::Translator translator;
        const std::string tr_path = gus::app::i18n::resolve_translations_path();
        const bool tr_ok = translator.load_from_file(tr_path);
        scene.set_translator(&translator);

        std::cout << "BattlePreview: traducao "
                  << (tr_ok ? "carregada" : "AUSENTE (fallback)") << " de " << tr_path
                  << "\n  party=" << scene.party_count()
                  << " inimigos=" << scene.enemy_count()
                  << " fila=" << scene.queue_len() << " retratos em " << dir
                  << "\n  Cima/Baixo: navega o menu | Enter/Espaco: na sua vez confirma o "
                     "verbo, senao ACELERA o ritmo | Esc: sai\n";

        bool running = true;
        bool have_last = false;
        unsigned long long last_ns = 0;
        while (running) {
            SDL_Event ev;
            while (SDL_PollEvent(&ev)) {
                if (ev.type == SDL_EVENT_QUIT) {
                    running = false;
                } else if (ev.type == SDL_EVENT_KEY_DOWN) {
                    switch (ev.key.key) {
                        case SDLK_ESCAPE:
                            running = false;
                            break;
                        // Navegacao do menu de verbos (incremento 3). So opera no turno
                        // de jogador (a cena ignora fora dele); a cena auto-encadeia os
                        // turnos de inimigo ate o proximo turno de jogador ou o fim.
                        case SDLK_UP:
                        case SDLK_W:
                            scene.menu_move(-1);
                            break;
                        case SDLK_DOWN:
                        case SDLK_S:
                            scene.menu_move(+1);
                            break;
                        case SDLK_RETURN:
                        case SDLK_SPACE:
                            // D8/D9: na vez do jogador, confirma o verbo; fora dela
                            // (intro/delay entre turnos), ACELERA o ritmo (pula a pausa).
                            if (scene.waiting_player_input()) {
                                scene.menu_confirm();
                            } else {
                                scene.skip();
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            if (!running) {
                break;
            }

            // dt real desde o ultimo frame (segundos): envelhece os numeros flutuantes.
            const unsigned long long now_ns = SDL_GetTicksNS();
            float dt = 0.0f;
            if (have_last) {
                dt = static_cast<float>(now_ns - last_ns) / 1.0e9f;
            }
            have_last = true;
            last_ns = now_ns;
            scene.update(dt);  // anima os floaters (incremento 5); nao toca a FSM

            int pw = kWindowW, ph = kWindowH;
            SDL_GetCurrentRenderOutputSize(sdl_renderer, &pw, &ph);
            scene.render(renderer, static_cast<float>(pw),
                         static_cast<float>(ph));
        }
    }  // Render2dSdl destruido antes do renderer SDL

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

}  // namespace gus::app::screens
