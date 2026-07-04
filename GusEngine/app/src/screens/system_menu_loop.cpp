// gus/app/src/screens/system_menu_loop.cpp
//
// Implementacao do loop interativo do MENU DE SISTEMA. Ver header para o contrato
// completo. GL/glintfx-heavy (mesma familia de battle_preview.cpp) - sem unidade
// de teste direta (irredutivel, mesmo racional de run_battle_preview_embedded: a
// logica PURA testavel ja fica em system_menu.hpp/system_menu_test.cpp e
// system_menu_rml.hpp/system_menu_rml_test.cpp; este .cpp so orquestra SDL/GL em
// torno dela).

#include "gus/app/screens/system_menu_loop.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <glintfx/element_box.hpp>
#include <glintfx/ui_layer.hpp>

#include "gus/app/screens/system_menu.hpp"
#include "gus/app/screens/system_menu_rml.hpp"
#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect
#include "gus/domain/settings/system_settings.hpp"
#include "gus/platform/fs/settings_file_store.hpp"
#include "gus/platform/render2d/render2d_gl3.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"  // glad load (variante owning_gl)

// Pasta das fontes (.ttf), embutida pelo CMake (mesma macro que battle_preview.cpp
// ja usa - PRIVATE no CMakeLists do target app, aplica a TODO .cpp do target).
#ifndef GUSWORLD_FONTS_DIR
#define GUSWORLD_FONTS_DIR ""
#endif

namespace gus::app::screens {

namespace {

namespace fs = std::filesystem;

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

// Diretorio de stage do RML do menu (mesma receita de glintfx_cockpit_stage_dir em
// battle_preview.cpp: tempfile - o glintfx carrega documento por PATH). Nome
// PROPRIO (nao colide com o stage do cockpit da batalha).
std::string menu_stage_dir() {
    return (fs::temp_directory_path() / "gusworld_glintfx_sysmenu").string();
}

// Escreve o RML do menu (build_system_menu_rml, ver system_menu_rml.hpp) num
// arquivo dentro do stage, com o @font-face injetado logo apos <style> (o
// glintfx::UiLayer NAO expoe Rml::LoadFontFace - so o doc registra a familia via
// @font-face, MESMA receita de write_baked_cockpit_rml/write_live_cockpit_rml em
// battle_preview.cpp). Copia as 2 fontes pro stage (fonte: GUSWORLD_FONTS_DIR, env
// GUSWORLD_FONTS tem prioridade - mesma ordem de resolucao de asset do resto do
// app/). Devolve o path do .rml escrito.
std::string write_system_menu_rml_file(const SystemMenuState& state,
                                        const gus::app::i18n::Translator& tr) {
    const fs::path stage = menu_stage_dir();
    std::error_code ec;
    fs::create_directories(stage, ec);

    std::string fonts_dir = GUSWORLD_FONTS_DIR;
    if (const char* envf = std::getenv("GUSWORLD_FONTS")) {
        if (envf[0] != '\0') fonts_dir = envf;
    }
    if (!fonts_dir.empty()) {
        fs::copy_file(join(fonts_dir, "PixelOperatorMono.ttf"),
                      stage / "PixelOperatorMono.ttf",
                      fs::copy_options::overwrite_existing, ec);
        fs::copy_file(join(fonts_dir, "PixelOperatorMono-Bold.ttf"),
                      stage / "PixelOperatorMono-Bold.ttf",
                      fs::copy_options::overwrite_existing, ec);
    }

    std::string rml = build_system_menu_rml(state, tr);
    const std::string needle = "<style>\n";
    const std::size_t pos = rml.find(needle);
    if (pos != std::string::npos) {
        rml.insert(pos + needle.size(),
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "src: \"PixelOperatorMono.ttf\"; }\n"
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "font-weight: bold; src: \"PixelOperatorMono-Bold.ttf\"; }\n");
    }

    const fs::path out = stage / "system_menu.rml";
    std::ofstream f(out);
    f << rml;
    return out.string();
}

void apply_and_persist(const SystemMenuState& state,
                        gus::platform::audio::AudioEngine& audio,
                        const std::string& settings_dir) {
    audio.set_music_volume(state.music_volume);
    audio.set_sfx_volume(state.sfx_volume);
    gus::domain::settings::SystemSettings settings;
    settings.music_volume = state.music_volume;
    settings.sfx_volume = state.sfx_volume;
    if (!gus::platform::fs::save_system_settings(settings, settings_dir)) {
        // Best-effort: o volume ja vale nesta sessao (aplicado no AudioEngine acima);
        // so nao persistiu (ex. disco cheio / permissao). Nao e fatal.
        std::cerr << "[system_menu] aviso: falha ao salvar settings.json "
                     "(volume vale nesta sessao, mas nao persistiu)\n";
    }
}

// Track ids (system_menu_rml.cpp: "slider-track-<indice>", indice = ConfigItem).
std::string track_id_for_item(int item) {
    return "slider-track-" + std::to_string(item);
}

// Ids das PILLS do Pause / dos campos do Config (system_menu_rml.cpp:
// "pause-item-<indice>" e "config-item-<indice>" - MENU-PAUSA-CONFIG-SOM,
// clique de mouse aciona/foca a opcao).
std::string pause_item_id(int item) {
    return "pause-item-" + std::to_string(item);
}
std::string config_item_id(int item) {
    return "config-item-" + std::to_string(item);
}

// Hit-test simples: cursor (x,y, espaco-janela) dentro da caixa border-box
// devolvida por glintfx::UiLayer::get_element_box (MESMO espaco de coordenadas
// - ver docs/embed-integration.md secao 10, ja citado em outros comentarios
// deste arquivo). box.found=false conta como "fora".
bool hit_test(const glintfx::ElementBox& box, float x, float y) {
    if (!box.found) return false;
    return x >= box.x && x <= box.x + box.w && y >= box.y && y <= box.y + box.h;
}

}  // namespace

SystemMenuLoopOutcome run_system_menu_loop_gl_current(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator, const std::string& settings_dir) {
    SystemMenuLoopOutcome outcome;

    SystemMenuState state;
    state.music_volume = audio.music_volume();
    state.sfx_volume = audio.sfx_volume();
    system_menu_open(state);

    int pw = 0, ph = 0;
    SDL_GetWindowSizeInPixels(window, &pw, &ph);
    if (pw < 1) pw = 1;
    if (ph < 1) ph = 1;
    float dp_ratio = static_cast<float>(pw) / 960.0f;

    glintfx::UiLayer ui(glintfx::UiLayer::Config{/*logical_width=*/960,
                                                  /*logical_height=*/540,
                                                  /*load_gl=*/true,
                                                  /*dp_ratio=*/dp_ratio});
    if (!ui.ok()) {
        std::cerr << "SystemMenuLoop: glintfx::UiLayer::ok()=false (attach falhou) - "
                     "fechando o menu sem desenhar nada (degradacao segura).\n";
        return outcome;  // quit_app=false: o chamador so retoma a cena
    }

    const std::string stage = menu_stage_dir();
    ui.set_asset_base_url(stage.c_str());
    std::string rml_path = write_system_menu_rml_file(state, translator);
    ui.load(rml_path.c_str());
    ui.set_viewport(pw, ph);
    ui.set_dp_ratio(dp_ratio);

    // Reconstroi o RML/reflete no glintfx apos QUALQUER mutacao de estado (navegacao,
    // troca de tela, volume) - ver o comentario de build_system_menu_rml/
    // write_system_menu_rml_file: reload-on-change e simples e barato o bastante
    // (o menu muda so em input do jogador, nao a cada frame).
    auto reload = [&] {
        rml_path = write_system_menu_rml_file(state, translator);
        ui.load(rml_path.c_str());
        ui.set_viewport(pw, ph);
        ui.set_dp_ratio(dp_ratio);
    };

    gus::platform::render2d::Render2dGl3 backdrop(/*gl_active=*/true);

    int drag_item = -1;  // -1 = nenhum arrasto em curso; 0=Music, 1=Sfx

    // Roteia UMA action (vinda do teclado OU de um clique de mouse) pro mesmo
    // efeito de mundo (persistir volume, recarregar o RML) - compartilhado
    // pelos dois canais de entrada pra nao duplicar a logica de
    // Continue/RequestQuit/VolumeChanged. Devolve true se o CHAMADOR deve
    // retornar `outcome` na hora (Continue/RequestQuit ja setaram outcome).
    auto handle_action = [&](SystemMenuAction action) -> bool {
        if (action == SystemMenuAction::Continue) {
            return true;  // quit_app=false: retoma a cena
        }
        if (action == SystemMenuAction::RequestQuit) {
            outcome.quit_app = true;
            return true;
        }
        if (action == SystemMenuAction::VolumeChanged) {
            apply_and_persist(state, audio, settings_dir);
        }
        // None/OpenSettings/BackToPause: o ESTADO pode ter mudado mesmo assim
        // (navegacao/foco move e devolve None) - reload sempre.
        reload();
        return false;
    };

    while (true) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) {
                outcome.quit_app = true;
                return outcome;
            }
            if (ev.type == SDL_EVENT_WINDOW_RESIZED ||
                ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                SDL_GetWindowSizeInPixels(window, &pw, &ph);
                if (pw < 1) pw = 1;
                if (ph < 1) ph = 1;
                dp_ratio = static_cast<float>(pw) / 960.0f;
                ui.set_viewport(pw, ph);
                ui.set_dp_ratio(dp_ratio);
                continue;
            }
            if (ev.type == SDL_EVENT_KEY_DOWN && !ev.key.repeat) {
                const SystemMenuAction action =
                    system_menu_key_down(state, ev.key.key);
                if (handle_action(action)) return outcome;
            } else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                       ev.button.button == SDL_BUTTON_LEFT) {
                bool handled = false;
                if (state.screen == SystemMenuScreen::Pause) {
                    // Clicar numa pill (Continuar/Configuracoes/Sair) SELECIONA E
                    // ACIONA na hora - equivalente a focar + ENTER.
                    for (int item = 0; item < kPauseItemCount && !handled; ++item) {
                        const glintfx::ElementBox box =
                            ui.get_element_box(pause_item_id(item).c_str());
                        if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                        handled = true;
                        const SystemMenuAction action =
                            system_menu_click_option(state, item);
                        if (handle_action(action)) return outcome;
                    }
                } else if (state.screen == SystemMenuScreen::Config) {
                    // (1) Tracks dos sliders (drag-start, receita PRE-EXISTENTE) -
                    // checado PRIMEIRO porque a caixa do track fica DENTRO da
                    // caixa do campo/rotulo (config-item-<i>, ver (3) abaixo) - o
                    // mais especifico tem que vencer quando o clique cai nos dois.
                    for (int item = 0; item < 2 && !handled; ++item) {
                        const glintfx::ElementBox box =
                            ui.get_element_box(track_id_for_item(item).c_str());
                        if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                        handled = true;
                        drag_item = item;
                        state.config_selected = item;
                        if (box.w > 0.0f) {
                            const float ratio = (ev.button.x - box.x) / box.w;
                            system_menu_set_slider_ratio(state, item, ratio);
                            apply_and_persist(state, audio, settings_dir);
                        }
                        reload();
                    }
                    // (2) Botao Voltar - ACIONA na hora (equivalente a focar + ENTER).
                    if (!handled) {
                        const glintfx::ElementBox box =
                            ui.get_element_box("config-back");
                        if (hit_test(box, ev.button.x, ev.button.y)) {
                            handled = true;
                            const SystemMenuAction action = system_menu_click_option(
                                state, static_cast<int>(ConfigItem::Back));
                            if (handle_action(action)) return outcome;
                        }
                    }
                    // (3) Campo/rotulo do slider (fora do track) - SO FOCA (nao
                    // ajusta volume - isso e papel exclusivo do track, ver (1)).
                    for (int item = 0; item < 2 && !handled; ++item) {
                        const glintfx::ElementBox box =
                            ui.get_element_box(config_item_id(item).c_str());
                        if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                        handled = true;
                        const SystemMenuAction action =
                            system_menu_click_option(state, item);
                        if (handle_action(action)) return outcome;
                    }
                }
            } else if (ev.type == SDL_EVENT_MOUSE_MOTION && drag_item >= 0) {
                const std::string id = track_id_for_item(drag_item);
                const glintfx::ElementBox box = ui.get_element_box(id.c_str());
                if (box.found && box.w > 0.0f) {
                    const float ratio = (ev.motion.x - box.x) / box.w;
                    system_menu_set_slider_ratio(state, drag_item, ratio);
                    apply_and_persist(state, audio, settings_dir);
                    reload();
                }
            } else if (ev.type == SDL_EVENT_MOUSE_BUTTON_UP &&
                       ev.button.button == SDL_BUTTON_LEFT) {
                drag_item = -1;
            }
        }

        const gus::core::spatial::Rect cam{0.0f, 0.0f, static_cast<float>(pw),
                                            static_cast<float>(ph)};
        backdrop.begin_frame(cam, pw, ph);  // clear + vinheta radial (fundo abstrato)
        backdrop.end_frame();
        ui.update();
        ui.render();
        SDL_GL_SwapWindow(window);
    }
}

bool run_system_menu_loop_owning_gl(SDL_Window* window,
                                     gus::platform::audio::AudioEngine& audio,
                                     const gus::app::i18n::Translator& translator,
                                     const std::string& settings_dir,
                                     SystemMenuLoopOutcome* out_outcome) {
    // MESMA receita de run_battle_preview_embedded (battle_preview.cpp): os
    // atributos GL sao setados a CADA entrada (nao precisam ter sido setados na
    // criacao da janela) - viabilidade ja provada empiricamente pela troca
    // cidade<->batalha (ver maestro.cpp::to_battle).
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GLContext gl = SDL_GL_CreateContext(window);
    if (gl == nullptr) {
        std::cerr << "SystemMenuLoop: SDL_GL_CreateContext falhou: " << SDL_GetError()
                  << "\n";
        return false;
    }
    SDL_GL_MakeCurrent(window, gl);
    SDL_GL_SetSwapInterval(1);

    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        std::cerr << "SystemMenuLoop: falha ao carregar funcoes OpenGL (glad)\n";
        SDL_GL_DestroyContext(gl);
        return false;
    }

    const SystemMenuLoopOutcome outcome =
        run_system_menu_loop_gl_current(window, audio, translator, settings_dir);
    if (out_outcome != nullptr) {
        *out_outcome = outcome;
    }

    SDL_GL_DestroyContext(gl);
    return true;
}

}  // namespace gus::app::screens
