// gus/app/src/screens/system_menu_loop.cpp
//
// Implementacao do loop interativo do MENU DE SISTEMA. Ver header para o contrato
// completo. GL/glintfx-heavy (mesma familia de battle_preview.cpp) - sem unidade
// de teste direta (irredutivel, mesmo racional de run_battle_preview_embedded: a
// logica PURA testavel ja fica em system_menu.hpp/system_menu_test.cpp e
// system_menu_rml.hpp/system_menu_rml_test.cpp; este .cpp so orquestra SDL/GL em
// torno dela).
//
// EFEITO DE PRESS (MENU-PAUSA-CONFIG-SOM, onda arvore): quando o jogador aciona
// uma pill/categoria/Voltar (Enter/Espaco no TECLADO ou clique de MOUSE), o loop
// renderiza ALGUNS FRAMES com a classe "pressed" no item ativado (flash cyan
// intenso, ver .verb-pill.pressed/.btn-back.pressed em system_menu_rml.cpp) ANTES
// de aplicar a transicao de fato (trocar de tela/fechar o menu/pedir Sair). Isto e
// deliberadamente um efeito NOSSO (nao do glintfx - a lib nao tem estado "active"
// disparado por teclado, so :focus/:hover via classe) - flash_pressed() abaixo e o
// UNICO lugar que gera esse frame extra.

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
// app/). Devolve o path do .rml escrito. `pressed_index` repassado direto pra
// build_system_menu_rml (ver seu header) - default -1 (nenhum item pressionado).
std::string write_system_menu_rml_file(const SystemMenuState& state,
                                        const gus::app::i18n::Translator& tr,
                                        int pressed_index = -1) {
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

    std::string rml = build_system_menu_rml(state, tr, pressed_index);
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

// Track ids (system_menu_rml.cpp: "slider-track-<indice>", indice = AudioItem).
std::string track_id_for_item(int item) {
    return "slider-track-" + std::to_string(item);
}

// Ids das PILLS do Pause / categorias de ConfigCategories / campos+Voltar do
// Audio (system_menu_rml.cpp: "pause-item-<indice>"/"category-item-<indice>"/
// "audio-item-<indice>" - clique de mouse aciona/foca a opcao). O Voltar das 3
// telas placeholder usa 1 UNICO id fixo ("placeholder-back", ver
// build_placeholder_body): so 1 placeholder fica carregado por vez.
std::string pause_item_id(int item) {
    return "pause-item-" + std::to_string(item);
}
std::string category_item_id(int item) {
    return "category-item-" + std::to_string(item);
}
std::string audio_item_id(int item) {
    return "audio-item-" + std::to_string(item);
}
constexpr const char* kPlaceholderBackId = "placeholder-back";

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

    gus::platform::render2d::Render2dGl3 backdrop(/*gl_active=*/true);

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

    // Desenha um frame do backdrop+UI e apresenta (MESMA sequencia do corpo do
    // loop principal abaixo) - fatorado pra ser reusado pelo flash de PRESS.
    auto present_frame = [&] {
        const gus::core::spatial::Rect cam{0.0f, 0.0f, static_cast<float>(pw),
                                            static_cast<float>(ph)};
        backdrop.begin_frame(cam, pw, ph);  // clear + vinheta radial (fundo abstrato)
        backdrop.end_frame();
        ui.update();
        ui.render();
        SDL_GL_SwapWindow(window);
    };

    // EFEITO DE PRESS (ver comentario do topo do arquivo): renderiza a tela
    // `pre_action_state` (snapshot tirado ANTES da mutacao que ja aconteceu em
    // `state`) com o item `item_index` marcado ".pressed", por ~100ms (4 frames
    // de ~25ms - varios swaps garantem que o compositor/driver apresente pelo
    // menos 1 frame do flash mesmo sob vsync), e SO DEPOIS devolve - o chamador
    // segue com handle_action/reload usando o `state` JA MUTADO (a transicao
    // real acontece normalmente no proximo reload/return).
    auto flash_pressed = [&](const SystemMenuState& pre_action_state, int item_index) {
        rml_path = write_system_menu_rml_file(pre_action_state, translator, item_index);
        ui.load(rml_path.c_str());
        ui.set_viewport(pw, ph);
        ui.set_dp_ratio(dp_ratio);
        for (int frame = 0; frame < 4; ++frame) {
            present_frame();
            SDL_Delay(25);
        }
    };

    int drag_item = -1;  // -1 = nenhum arrasto em curso; 0=Music, 1=Sfx

    // Roteia UMA action (vinda do teclado OU de um clique de mouse) pro mesmo
    // efeito de mundo (persistir volume, recarregar o RML) - compartilhado
    // pelos dois canais de entrada pra nao duplicar a logica de
    // Continue/RequestQuit/VolumeChanged/Navigated. Devolve true se o CHAMADOR
    // deve retornar `outcome` na hora (Continue/RequestQuit ja setaram outcome).
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
        // None/Navigated: o ESTADO pode ter mudado mesmo assim (navegacao/foco
        // move e devolve None, ou trocou de tela e devolve Navigated) - reload
        // sempre.
        reload();
        return false;
    };

    // Confirma se `action` merece o flash de PRESS (ver topo do arquivo): SO as
    // acoes que de fato "acionam uma opcao" (pill/categoria/Voltar) - nunca
    // VolumeChanged (drag de slider nao pisca) nem None (nao aconteceu nada).
    auto is_confirming = [](SystemMenuAction action) {
        return action == SystemMenuAction::Continue ||
               action == SystemMenuAction::RequestQuit ||
               action == SystemMenuAction::Navigated;
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
                const bool is_confirm_key = (ev.key.key == SDLK_RETURN ||
                                              ev.key.key == SDLK_KP_ENTER ||
                                              ev.key.key == SDLK_SPACE);
                if (is_confirm_key) {
                    // Enter/Espaco: captura a tela+item ATUAIS (antes da mutacao)
                    // pra poder desenhar o flash de PRESS na tela DE ORIGEM caso a
                    // action resultante confirme algo (ver is_confirming acima).
                    const SystemMenuState pre_action_state = state;
                    int item_index = -1;
                    switch (state.screen) {
                        case SystemMenuScreen::Pause:
                            item_index = state.pause_selected;
                            break;
                        case SystemMenuScreen::ConfigCategories:
                            item_index = state.config_categories_selected;
                            break;
                        case SystemMenuScreen::Audio:
                            item_index = state.audio_selected;
                            break;
                        case SystemMenuScreen::Save:
                        case SystemMenuScreen::Video:
                        case SystemMenuScreen::Language:
                            item_index = kPlaceholderBackIndex;
                            break;
                        case SystemMenuScreen::Hidden:
                            break;
                    }
                    const SystemMenuAction action =
                        system_menu_key_down(state, ev.key.key);
                    if (is_confirming(action)) flash_pressed(pre_action_state, item_index);
                    if (handle_action(action)) return outcome;
                } else {
                    const SystemMenuAction action =
                        system_menu_key_down(state, ev.key.key);
                    if (handle_action(action)) return outcome;
                }
            } else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                       ev.button.button == SDL_BUTTON_LEFT) {
                bool handled = false;
                if (state.screen == SystemMenuScreen::Pause) {
                    // Clicar numa pill (Continuar/Salvar/Configuracoes/Sair)
                    // SELECIONA E ACIONA na hora - equivalente a focar + ENTER.
                    for (int item = 0; item < kPauseItemCount && !handled; ++item) {
                        const glintfx::ElementBox box =
                            ui.get_element_box(pause_item_id(item).c_str());
                        if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                        handled = true;
                        const SystemMenuState pre_action_state = state;
                        const SystemMenuAction action =
                            system_menu_click_option(state, item);
                        if (is_confirming(action)) flash_pressed(pre_action_state, item);
                        if (handle_action(action)) return outcome;
                    }
                } else if (state.screen == SystemMenuScreen::ConfigCategories) {
                    // Categorias (Audio/Video/Lingua/Voltar) - botoes simples, SEM
                    // slider: clicar SEMPRE seleciona E aciona na hora.
                    for (int item = 0; item < kConfigCategoriesItemCount && !handled;
                         ++item) {
                        const glintfx::ElementBox box =
                            ui.get_element_box(category_item_id(item).c_str());
                        if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                        handled = true;
                        const SystemMenuState pre_action_state = state;
                        const SystemMenuAction action =
                            system_menu_click_option(state, item);
                        if (is_confirming(action)) flash_pressed(pre_action_state, item);
                        if (handle_action(action)) return outcome;
                    }
                } else if (state.screen == SystemMenuScreen::Audio) {
                    // (1) Tracks dos sliders (drag-start, receita PRE-EXISTENTE) -
                    // checado PRIMEIRO porque a caixa do track fica DENTRO da
                    // caixa do campo/rotulo (audio-item-<i>, ver (3) abaixo) - o
                    // mais especifico tem que vencer quando o clique cai nos dois.
                    for (int item = 0; item < 2 && !handled; ++item) {
                        const glintfx::ElementBox box =
                            ui.get_element_box(track_id_for_item(item).c_str());
                        if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                        handled = true;
                        drag_item = item;
                        state.audio_selected = item;
                        if (box.w > 0.0f) {
                            const float ratio = (ev.button.x - box.x) / box.w;
                            system_menu_set_slider_ratio(state, item, ratio);
                            apply_and_persist(state, audio, settings_dir);
                        }
                        reload();
                    }
                    // (2) Botao Voltar - ACIONA na hora (equivalente a focar + ENTER).
                    if (!handled) {
                        const int back_index = static_cast<int>(AudioItem::Back);
                        const glintfx::ElementBox box =
                            ui.get_element_box(audio_item_id(back_index).c_str());
                        if (hit_test(box, ev.button.x, ev.button.y)) {
                            handled = true;
                            const SystemMenuState pre_action_state = state;
                            const SystemMenuAction action =
                                system_menu_click_option(state, back_index);
                            if (is_confirming(action)) {
                                flash_pressed(pre_action_state, back_index);
                            }
                            if (handle_action(action)) return outcome;
                        }
                    }
                    // (3) Campo/rotulo do slider (fora do track) - SO FOCA (nao
                    // ajusta volume - isso e papel exclusivo do track, ver (1)).
                    for (int item = 0; item < 2 && !handled; ++item) {
                        const glintfx::ElementBox box =
                            ui.get_element_box(audio_item_id(item).c_str());
                        if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                        handled = true;
                        const SystemMenuAction action =
                            system_menu_click_option(state, item);
                        if (handle_action(action)) return outcome;
                    }
                } else if (state.screen == SystemMenuScreen::Save ||
                           state.screen == SystemMenuScreen::Video ||
                           state.screen == SystemMenuScreen::Language) {
                    // Placeholder ("em breve"): so o Voltar e clicavel.
                    const glintfx::ElementBox box = ui.get_element_box(kPlaceholderBackId);
                    if (hit_test(box, ev.button.x, ev.button.y)) {
                        handled = true;
                        const SystemMenuState pre_action_state = state;
                        const SystemMenuAction action = system_menu_click_option(
                            state, kPlaceholderBackIndex);
                        if (is_confirming(action)) {
                            flash_pressed(pre_action_state, kPlaceholderBackIndex);
                        }
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

        present_frame();
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
