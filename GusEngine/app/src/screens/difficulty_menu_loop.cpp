// gus/app/src/screens/difficulty_menu_loop.cpp
//
// Implementacao do loop interativo da tela de selecao de dificuldade. Ver header
// para o contrato completo. GL/glintfx-heavy (mesma familia de
// title_menu_loop.cpp/save_load_menu_loop.cpp) - sem unidade de teste direta (a
// logica PURA testavel ja fica em difficulty_menu.hpp/difficulty_menu_test.cpp e
// difficulty_menu_rml.hpp/difficulty_menu_rml_test.cpp; este .cpp so orquestra
// SDL/GL em torno delas).
//
// ANINHADO (NAO owning_gl): ao contrario de title_menu_loop.cpp, esta funcao NAO
// cria/destroi o contexto GL - ela roda DENTRO do contexto que o CHAMADOR
// (title_menu_loop.cpp) ja deixou corrente, MESMA tecnica de
// run_save_load_menu_loop_gl_current (o CHAMADOR ja destruiu a PROPRIA UiLayer
// ANTES de chamar esta funcao - RmlUi so aceita 1 instancia viva por processo).

#include "gus/app/screens/difficulty_menu_loop.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <glintfx/element_box.hpp>
#include <glintfx/ui_layer.hpp>

#include "gus/app/screens/difficulty_menu_rml.hpp"
#include "gus/core/asset_paths.hpp"  // kMenuHoverSfxFile/kMenuClickSfxFile/kSfxDir
#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect
#include "gus/platform/assets/asset_source.hpp"  // FilesystemAssetSource (resolve do SFX)
#include "gus/platform/render2d/render2d_gl3.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"  // gl3_read_backbuffer_rgba (prova visual)

// stb_image_write: SO a declaracao aqui (a IMPLEMENTACAO ja vive UMA vez em
// battle_preview.cpp, MESMA lib gusengine_app - nao redefinir
// STB_IMAGE_WRITE_IMPLEMENTATION aqui, senao da symbol duplicado no link).
#include "stb_image_write.h"

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

// Stage PROPRIO (nao colide com o stage de title_menu_loop.cpp/system_menu_loop.cpp).
std::string difficulty_stage_dir() {
    return (fs::temp_directory_path() / "gusworld_glintfx_difficulty").string();
}

std::string write_difficulty_rml_file(const DifficultyMenuState& state,
                                       const gus::app::i18n::Translator& tr,
                                       int pressed_index = -1) {
    const fs::path stage = difficulty_stage_dir();
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

    std::string rml = build_difficulty_menu_rml(state, tr, pressed_index);
    const std::string needle = "<style>\n";
    const std::size_t pos = rml.find(needle);
    if (pos != std::string::npos) {
        rml.insert(pos + needle.size(),
                   "@font-face { font-family: \"Pixel Operator Mono\"; "
                   "src: \"PixelOperatorMono.ttf\"; }\n"
                   "@font-face { font-family: \"Pixel Operator Mono\"; "
                   "font-weight: bold; src: \"PixelOperatorMono-Bold.ttf\"; }\n");
    }

    const fs::path out = stage / "difficulty_menu.rml";
    std::ofstream f(out);
    f << rml;
    return out.string();
}

// Hit-test simples (MESMA receita de title_menu_loop.cpp/system_menu_loop.cpp):
// cursor (espaco-janela) dentro da caixa border-box devolvida por
// glintfx::UiLayer::get_element_box.
bool hit_test(const glintfx::ElementBox& box, float x, float y) {
    if (!box.found) return false;
    return x >= box.x && x <= box.x + box.w && y >= box.y && y <= box.y + box.h;
}

std::string resolve_menu_sfx_path(std::string_view file) {
    const std::string id = join(std::string(gus::core::assets::kSfxDir), std::string(file));
    return gus::platform::assets::FilesystemAssetSource().resolve_path(id);
}

// SFX-MIGRATE-V0.9: filtro NAVEGAVEL pro callback NATIVO de hover (MESMA receita de
// is_navigable_hover_id em title_menu_loop.cpp) - ids "difficulty-item-<i>" (lista)
// ou "difficulty-confirm-<i>" (splash), conforme o modo ATUAL.
bool is_navigable_hover_id(const DifficultyMenuState& state, const std::string& id) {
    if (state.confirming) {
        return id == "difficulty-confirm-0" || id == "difficulty-confirm-1";
    }
    for (int i = 0; i < kDifficultyItemCount; ++i) {
        if (id == "difficulty-item-" + std::to_string(i)) return true;
    }
    return false;
}

}  // namespace

DifficultyLoopExit run_difficulty_menu_loop_gl_current(
    SDL_Window* window, gus::platform::audio::AudioEngine& audio,
    const gus::app::i18n::Translator& translator,
    gus::domain::save::DifficultyLevel* out_difficulty,
    const std::string& frozen_background_png) {
    DifficultyMenuState state;
    difficulty_menu_open(state);

    int pw = 0, ph = 0;
    SDL_GetWindowSizeInPixels(window, &pw, &ph);
    if (pw < 1) pw = 1;
    if (ph < 1) ph = 1;
    const float dp_ratio = static_cast<float>(pw) / 960.0f;

    glintfx::UiLayer ui(glintfx::UiLayer::Config{/*logical_width=*/960,
                                                  /*logical_height=*/540,
                                                  /*load_gl=*/true,
                                                  /*dp_ratio=*/dp_ratio});
    if (!ui.ok()) {
        std::cerr << "DifficultyMenuLoop: glintfx::UiLayer::ok()=false (attach "
                     "falhou) - abortando Novo Jogo, volta pra tela de titulo "
                     "(degradacao segura).\n";
        return DifficultyLoopExit::Cancelled;
    }

    const std::string stage = difficulty_stage_dir();
    ui.set_asset_base_url(stage.c_str());
    std::string rml_path = write_difficulty_rml_file(state, translator);
    ui.load(rml_path.c_str());
    ui.set_viewport(pw, ph);
    ui.set_dp_ratio(dp_ratio);
    // SFX-MIGRATE-V0.9: 1 update() de "assentamento" (MESMO achado empirico de
    // title_menu_loop.cpp/save_load_menu_loop.cpp - o hover NATIVO so resolve
    // elemento sob o cursor apos pelo menos 1 Context::Update() do documento
    // recem-carregado).
    ui.update();

    gus::platform::render2d::Render2dGl3 backdrop(/*gl_active=*/true);
    const gus::platform::render2d::TextureId frozen_bg_tex =
        frozen_background_png.empty()
            ? gus::platform::render2d::kInvalidTexture
            : backdrop.load_texture(frozen_background_png.c_str());

    const std::string hover_sfx_path =
        resolve_menu_sfx_path(gus::core::assets::kMenuHoverSfxFile);
    const std::string click_sfx_path =
        resolve_menu_sfx_path(gus::core::assets::kMenuClickSfxFile);
    const gus::platform::audio::SoundId hover_sfx_id = audio.load_sfx(hover_sfx_path.c_str());
    const gus::platform::audio::SoundId click_sfx_id = audio.load_sfx(click_sfx_path.c_str());

    auto reload = [&] {
        rml_path = write_difficulty_rml_file(state, translator);
        ui.load(rml_path.c_str());
        ui.set_viewport(pw, ph);
        ui.set_dp_ratio(dp_ratio);
        ui.update();  // MESMO assentamento a cada troca de documento
    };

    auto present_frame = [&] {
        const gus::core::spatial::Rect cam{0.0f, 0.0f, static_cast<float>(pw),
                                            static_cast<float>(ph)};
        backdrop.begin_frame(cam, pw, ph);
        if (frozen_bg_tex != gus::platform::render2d::kInvalidTexture) {
            backdrop.draw_textured_rect(
                cam, frozen_bg_tex, gus::platform::render2d::UvRect{0.0f, 0.0f, 1.0f, 1.0f},
                gus::platform::render2d::DrawColor{1.0f, 1.0f, 1.0f, 1.0f});
        }
        backdrop.end_frame();
        ui.update();
        ui.render();
        SDL_GL_SwapWindow(window);
    };

    std::string last_hover_sfx_id;
    auto hover_cb = [&](const char* raw_id, bool entered) {
        const std::string id = raw_id != nullptr ? raw_id : "";
        if (!entered) {
            if (id == last_hover_sfx_id) last_hover_sfx_id.clear();
            return;
        }
        if (id == last_hover_sfx_id || !is_navigable_hover_id(state, id)) return;
        last_hover_sfx_id = id;
        audio.play_sfx(hover_sfx_id);
    };
    ui.set_hover_callback(hover_cb);

    auto handle_mouse_motion = [&](float mx, float my) {
        glintfx::UiEvent hover_ev{};
        hover_ev.type = glintfx::UiEvent::Type::MouseMove;
        hover_ev.x = mx;
        hover_ev.y = my;
        ui.process_event(hover_ev);
    };

    // Roteia UMA DifficultyMenuAction pro efeito de mundo comum aos pontos de
    // entrada (Enter, clique) - devolve o exit se o CHAMADOR deve retornar NA
    // HORA (Chosen/Cancelled), senao reload() e continua no loop.
    auto route_action = [&](DifficultyMenuAction action) -> std::optional<DifficultyLoopExit> {
        switch (action) {
            case DifficultyMenuAction::None:
                reload();
                return std::nullopt;
            case DifficultyMenuAction::Chosen:
                if (out_difficulty != nullptr) {
                    *out_difficulty = difficulty_level_for_item(state.selected);
                }
                return DifficultyLoopExit::Chosen;
            case DifficultyMenuAction::Cancelled:
                return DifficultyLoopExit::Cancelled;
        }
        return std::nullopt;
    };

    // DIAGNOSTICO/PROVA (prova visual headless Xvfb :99): GUSWORLD_DIFFICULTY_
    // SCREENSHOT_DIR=<dir> assenta alguns frames e salva 1 PNG ANTES de entrar no
    // loop interativo - bypassa por completo (MESMO espirito de
    // GUSWORLD_TITLE_SCREENSHOT_DIR em title_menu_loop.cpp).
    const char* screenshot_dir = std::getenv("GUSWORLD_DIFFICULTY_SCREENSHOT_DIR");
    if (screenshot_dir != nullptr && screenshot_dir[0] != '\0') {
        // GUSWORLD_DIFFICULTY_SCREENSHOT_CONFIRM=1 (opcional): captura o SPLASH
        // de confirmacao (Aviso #2) em vez da lista - abre via a MESMA
        // maquina de estado PURA (Enter na lista, mesma logica de um clique
        // real) antes do reload() que gera o RML novo.
        const char* confirm_flag = std::getenv("GUSWORLD_DIFFICULTY_SCREENSHOT_CONFIRM");
        const bool want_confirm = confirm_flag != nullptr && confirm_flag[0] != '\0';
        if (want_confirm) {
            (void)difficulty_menu_key_down(state, SDLK_RETURN);  // abre o splash
            reload();
        }
        for (int i = 0; i < 6; ++i) present_frame();
        std::vector<unsigned char> buf(static_cast<std::size_t>(pw) *
                                        static_cast<std::size_t>(ph) * 4);
        if (gus::platform::rmlui::gl3_read_backbuffer_rgba(pw, ph, buf.data())) {
            const std::string filename =
                want_confirm ? "difficulty_menu_confirm.png" : "difficulty_menu.png";
            const std::string out = join(std::string(screenshot_dir), filename);
            stbi_write_png(out.c_str(), pw, ph, 4, buf.data(), pw * 4);
            std::cout << "DifficultyMenuLoop: [screenshot] " << out << " (" << pw << "x"
                      << ph << ")\n";
        } else {
            std::cerr << "DifficultyMenuLoop: [screenshot] gl3_read_backbuffer_rgba "
                         "falhou\n";
        }
        return DifficultyLoopExit::Cancelled;
    }

    while (true) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) {
                return DifficultyLoopExit::QuitApp;
            }
            if (ev.type == SDL_EVENT_WINDOW_RESIZED ||
                ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                SDL_GetWindowSizeInPixels(window, &pw, &ph);
                if (pw < 1) pw = 1;
                if (ph < 1) ph = 1;
                ui.set_viewport(pw, ph);
                ui.set_dp_ratio(static_cast<float>(pw) / 960.0f);
                continue;
            }
            if (ev.type == SDL_EVENT_KEY_DOWN && !ev.key.repeat) {
                const bool is_confirm_key = (ev.key.key == SDLK_RETURN ||
                                              ev.key.key == SDLK_KP_ENTER ||
                                              ev.key.key == SDLK_SPACE);
                if (is_confirm_key) {
                    audio.play_sfx(click_sfx_id);
                    const DifficultyMenuAction action =
                        difficulty_menu_key_down(state, ev.key.key);
                    if (const auto exit = route_action(action)) return *exit;
                } else {
                    // Navegacao (setas/WASD/ESC) - SOM DE HOVER paridade teclado x
                    // mouse (MESMA tecnica de title_menu_loop.cpp): so toca se o
                    // MODO (lista vs splash) NAO mudou E moveu pra um item NOVO.
                    const bool confirming_before = state.confirming;
                    const int kb_index_before = difficulty_keyboard_focus_index(state);
                    const DifficultyMenuAction action =
                        difficulty_menu_key_down(state, ev.key.key);
                    if (state.confirming == confirming_before) {
                        const int kb_index_after = difficulty_keyboard_focus_index(state);
                        if (ui_hover_entered_new_item(kb_index_before, kb_index_after)) {
                            audio.play_sfx(hover_sfx_id);
                        }
                    }
                    if (const auto exit = route_action(action)) return *exit;
                }
            } else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                       ev.button.button == SDL_BUTTON_LEFT) {
                bool handled = false;
                if (state.confirming) {
                    for (int i = 0; i < 2 && !handled; ++i) {
                        const glintfx::ElementBox box = ui.get_element_box(
                            ("difficulty-confirm-" + std::to_string(i)).c_str());
                        if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                        handled = true;
                        audio.play_sfx(click_sfx_id);
                        const DifficultyMenuAction action =
                            difficulty_menu_click_option(state, i);
                        if (const auto exit = route_action(action)) return *exit;
                    }
                } else {
                    for (int i = 0; i < kDifficultyItemCount && !handled; ++i) {
                        const glintfx::ElementBox box = ui.get_element_box(
                            ("difficulty-item-" + std::to_string(i)).c_str());
                        if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                        handled = true;
                        audio.play_sfx(click_sfx_id);  // todos os 3 sempre selecionaveis
                        const DifficultyMenuAction action =
                            difficulty_menu_click_option(state, i);
                        if (const auto exit = route_action(action)) return *exit;
                    }
                }
            } else if (ev.type == SDL_EVENT_MOUSE_MOTION) {
                handle_mouse_motion(ev.motion.x, ev.motion.y);
            }
        }
        present_frame();
    }
}

}  // namespace gus::app::screens
