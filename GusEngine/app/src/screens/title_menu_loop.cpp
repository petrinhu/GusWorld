// gus/app/src/screens/title_menu_loop.cpp
//
// Implementacao do loop interativo da TELA DE TITULO. Ver header para o
// contrato completo. GL/glintfx-heavy (mesma familia de save_load_menu_loop.cpp/
// system_menu_loop.cpp) - sem unidade de teste direta (a logica PURA testavel ja
// fica em title_menu.hpp/title_menu_test.cpp e title_menu_rml.hpp/
// title_menu_rml_test.cpp; este .cpp so orquestra SDL/GL + o I/O de disco em
// torno delas).

#include "gus/app/screens/title_menu_loop.hpp"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <glintfx/element_box.hpp>
#include <glintfx/ui_layer.hpp>

#include "gus/app/screens/save_load_menu.hpp"  // SaveSlotPreview/most_recent_occupied_slot
#include "gus/app/screens/title_menu_rml.hpp"
#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect
#include "gus/domain/save/save_serializer.hpp"  // LoadResult
#include "gus/platform/fs/save_file_store.hpp"  // has_save/load_game
#include "gus/platform/render2d/render2d_gl3.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"  // glad load + gl3_read_backbuffer_rgba

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
using gus::domain::save::kSlotCount;

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

// Stage PROPRIO (nao colide com o stage do menu de sistema/save-load).
std::string title_stage_dir() {
    return (fs::temp_directory_path() / "gusworld_glintfx_title").string();
}

std::string write_title_rml_file(const TitleMenuState& state,
                                  const gus::app::i18n::Translator& tr,
                                  int pressed_index = -1) {
    const fs::path stage = title_stage_dir();
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

    std::string rml = build_title_menu_rml(state, tr, pressed_index);
    const std::string needle = "<style>\n";
    const std::size_t pos = rml.find(needle);
    if (pos != std::string::npos) {
        rml.insert(pos + needle.size(),
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "src: \"PixelOperatorMono.ttf\"; }\n"
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "font-weight: bold; src: \"PixelOperatorMono-Bold.ttf\"; }\n");
    }

    const fs::path out = stage / "title_menu.rml";
    std::ofstream f(out);
    f << rml;
    return out.string();
}

// Hit-test simples (MESMA receita de system_menu_loop.cpp/battle_preview.cpp):
// cursor (espaco-janela) dentro da caixa border-box devolvida por
// glintfx::UiLayer::get_element_box.
bool hit_test(const glintfx::ElementBox& box, float x, float y) {
    if (!box.found) return false;
    return x >= box.x && x <= box.x + box.w && y >= box.y && y <= box.y + box.h;
}

// Varredura de disco (o UNICO I/O desta tela): monta os previews de TODOS os
// slots + um cache do SaveData ja carregado por slot (evita ler o arquivo 2x ao
// confirmar "Continuar") + any_save_exists + most_recent_occupied_slot. MESMA
// politica de degradacao de build_previews_and_cache (save_load_menu_loop.cpp):
// um save PRESENTE mas NAO Ok (adulterado/corrompido/versao incompativel/slot
// trocado) degrada, POR ORA, como slot vazio (os avisos dedicados sao etapa
// futura, fora do escopo desta dispatch) - logado pra nao silenciar o caso.
struct TitleDiskScan {
    std::array<SaveSlotPreview, kSlotCount> previews{};
    std::array<std::optional<gus::domain::save::SaveData>, kSlotCount> loaded{};
    bool any_save_exists = false;
    int most_recent_slot = -1;
};

TitleDiskScan scan_saves(const std::string& saves_dir) {
    TitleDiskScan scan;
    for (int slot = 0; slot < kSlotCount; ++slot) {
        if (!gus::platform::fs::has_save(slot, saves_dir)) {
            scan.previews[static_cast<std::size_t>(slot)] = empty_slot_preview(slot);
            continue;
        }
        const auto outcome = gus::platform::fs::load_game(slot, saves_dir);
        if (outcome.has_value() &&
            outcome->result == gus::domain::save::LoadResult::Ok) {
            scan.previews[static_cast<std::size_t>(slot)] =
                build_slot_preview(outcome->data, slot);
            scan.loaded[static_cast<std::size_t>(slot)] = outcome->data;
            scan.any_save_exists = true;
        } else {
            std::cerr << "[title_menu_loop] aviso: slot " << slot
                      << " tem arquivo mas NAO carregou Ok (adulterado/corrompido/"
                         "versao incompativel/slot trocado) - degradando como "
                         "vazio nesta onda (avisos dedicados sao etapa futura).\n";
            scan.previews[static_cast<std::size_t>(slot)] = empty_slot_preview(slot);
        }
    }
    scan.most_recent_slot = most_recent_occupied_slot(scan.previews);
    return scan;
}

}  // namespace

bool run_title_menu_loop_owning_gl(SDL_Window* window,
                                    const gus::app::i18n::Translator& translator,
                                    const std::string& saves_dir,
                                    TitleLoopExit* out_exit,
                                    gus::domain::save::SaveData* out_loaded_save,
                                    const std::string& frozen_background_png) {
    if (out_exit != nullptr) *out_exit = TitleLoopExit::QuitApp;

    // MESMA receita de run_system_menu_loop_owning_gl/run_battle_preview_embedded:
    // os atributos GL sao setados a CADA entrada (viabilidade ja provada
    // empiricamente pela troca cidade<->batalha/cidade<->menu).
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GLContext gl = SDL_GL_CreateContext(window);
    if (gl == nullptr) {
        std::cerr << "TitleMenuLoop: SDL_GL_CreateContext falhou: " << SDL_GetError()
                  << "\n";
        return false;
    }
    SDL_GL_MakeCurrent(window, gl);
    SDL_GL_SetSwapInterval(1);

    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        std::cerr << "TitleMenuLoop: falha ao carregar funcoes OpenGL (glad)\n";
        SDL_GL_DestroyContext(gl);
        return false;
    }

    // Corpo do loop numa lambda: garante que SDL_GL_DestroyContext roda em
    // QUALQUER caminho de saida (return dentro da lambda so sai DELA, nao da
    // funcao) - MESMA disciplina de limpeza incondicional das demais telas.
    const auto run_body = [&] {
        const TitleDiskScan scan = scan_saves(saves_dir);

        TitleMenuState state;
        title_menu_open(state, scan.any_save_exists);

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
            std::cerr << "TitleMenuLoop: glintfx::UiLayer::ok()=false (attach "
                         "falhou) - fechando sem desenhar (degradacao segura, "
                         "comeca fresco).\n";
            *out_exit = TitleLoopExit::NewGame;
            return;
        }

        const std::string stage = title_stage_dir();
        ui.set_asset_base_url(stage.c_str());
        std::string rml_path = write_title_rml_file(state, translator);
        ui.load(rml_path.c_str());
        ui.set_viewport(pw, ph);
        ui.set_dp_ratio(dp_ratio);

        gus::platform::render2d::Render2dGl3 backdrop(/*gl_active=*/true);
        const gus::platform::render2d::TextureId frozen_bg_tex =
            frozen_background_png.empty()
                ? gus::platform::render2d::kInvalidTexture
                : backdrop.load_texture(frozen_background_png.c_str());

        auto reload = [&] {
            rml_path = write_title_rml_file(state, translator);
            ui.load(rml_path.c_str());
            ui.set_viewport(pw, ph);
            ui.set_dp_ratio(dp_ratio);
        };

        auto present_frame = [&] {
            const gus::core::spatial::Rect cam{0.0f, 0.0f, static_cast<float>(pw),
                                                static_cast<float>(ph)};
            backdrop.begin_frame(cam, pw, ph);
            if (frozen_bg_tex != gus::platform::render2d::kInvalidTexture) {
                backdrop.draw_textured_rect(
                    cam, frozen_bg_tex,
                    gus::platform::render2d::UvRect{0.0f, 0.0f, 1.0f, 1.0f},
                    gus::platform::render2d::DrawColor{1.0f, 1.0f, 1.0f, 1.0f});
            }
            backdrop.end_frame();
            ui.update();
            ui.render();
            SDL_GL_SwapWindow(window);
        };

        // Confirma "Continuar": o save mais recente JA esta no cache (scan_saves) -
        // nao le o disco de novo. Defensivo (nunca deveria acontecer - Continuar
        // so e selecionavel quando any_save_exists, que implica most_recent_slot
        // >= 0): se por algum motivo o cache estiver vazio, degrada pra NewGame
        // em vez de devolver um SaveData por default-construir (nunca finge um
        // load que nao aconteceu).
        auto confirm_continue = [&] {
            if (scan.most_recent_slot >= 0 &&
                scan.loaded[static_cast<std::size_t>(scan.most_recent_slot)]
                    .has_value()) {
                if (out_loaded_save != nullptr) {
                    *out_loaded_save =
                        *scan.loaded[static_cast<std::size_t>(scan.most_recent_slot)];
                }
                *out_exit = TitleLoopExit::ContinueGame;
            } else {
                std::cerr << "[title_menu_loop] BUG defensivo: ContinueGame sem "
                             "slot no cache - degradando pra Novo Jogo (nao finge "
                             "um load que nao aconteceu).\n";
                *out_exit = TitleLoopExit::NewGame;
            }
        };

        // DIAGNOSTICO/PROVA (SAVE-LOAD-UI etapa 4, prova visual headless Xvfb
        // :99): GUSWORLD_TITLE_SCREENSHOT_DIR=<dir> assenta alguns frames (bake
        // de fonte/layout, MESMA cautela dos demais self-tests) e salva 1 PNG
        // ANTES de entrar no loop interativo - bypassa por completo (nunca abre
        // pra input real), MESMO espirito de GUSWORLD_SAVELOAD_SCREENSHOT_DIR em
        // save_load_menu_loop.cpp. Nome do arquivo reflete o ESTADO REAL varrido
        // do disco (Continuar habilitado/desabilitado) - rodar o processo 2x com
        // 2 GUSWORLD_HOME diferentes (1 com save, 1 vazio) produz os 2 PNGs
        // pedidos sem precisar renomear nada por fora.
        const char* screenshot_dir = std::getenv("GUSWORLD_TITLE_SCREENSHOT_DIR");
        if (screenshot_dir != nullptr && screenshot_dir[0] != '\0') {
            for (int i = 0; i < 6; ++i) present_frame();
            std::vector<unsigned char> buf(static_cast<std::size_t>(pw) *
                                            static_cast<std::size_t>(ph) * 4);
            if (gus::platform::rmlui::gl3_read_backbuffer_rgba(pw, ph, buf.data())) {
                const std::string suffix = scan.any_save_exists
                                                ? "title_continue_enabled"
                                                : "title_continue_disabled";
                const std::string out =
                    join(std::string(screenshot_dir), suffix + ".png");
                stbi_write_png(out.c_str(), pw, ph, 4, buf.data(), pw * 4);
                std::cout << "TitleMenuLoop: [screenshot] " << out << " (" << pw
                          << "x" << ph << ")\n";
            } else {
                std::cerr << "TitleMenuLoop: [screenshot] gl3_read_backbuffer_rgba "
                             "falhou\n";
            }
            return;
        }

        while (true) {
            SDL_Event ev;
            while (SDL_PollEvent(&ev)) {
                if (ev.type == SDL_EVENT_QUIT) {
                    *out_exit = TitleLoopExit::QuitApp;
                    return;
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
                    const TitleMenuAction action =
                        title_menu_key_down(state, ev.key.key);
                    switch (action) {
                        case TitleMenuAction::None:
                            reload();
                            break;
                        case TitleMenuAction::ContinueGame:
                            confirm_continue();
                            return;
                        case TitleMenuAction::StartNewGame:
                            *out_exit = TitleLoopExit::NewGame;
                            return;
                        case TitleMenuAction::RequestQuit:
                            *out_exit = TitleLoopExit::QuitApp;
                            return;
                    }
                } else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                           ev.button.button == SDL_BUTTON_LEFT) {
                    bool handled = false;
                    if (state.confirming_new_game) {
                        for (int i = 0; i < 2 && !handled; ++i) {
                            const glintfx::ElementBox box = ui.get_element_box(
                                ("title-confirm-" + std::to_string(i)).c_str());
                            if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                            handled = true;
                            const TitleMenuAction action =
                                title_menu_click_option(state, i);
                            if (action == TitleMenuAction::StartNewGame) {
                                *out_exit = TitleLoopExit::NewGame;
                                return;
                            }
                            reload();  // Nao/cancelou - permanece na tela
                        }
                    } else {
                        for (int i = 0; i < kTitleItemCount && !handled; ++i) {
                            const glintfx::ElementBox box = ui.get_element_box(
                                ("title-item-" + std::to_string(i)).c_str());
                            if (!hit_test(box, ev.button.x, ev.button.y)) continue;
                            handled = true;
                            const TitleMenuAction action =
                                title_menu_click_option(state, i);
                            switch (action) {
                                case TitleMenuAction::None:
                                    reload();
                                    break;
                                case TitleMenuAction::ContinueGame:
                                    confirm_continue();
                                    return;
                                case TitleMenuAction::StartNewGame:
                                    *out_exit = TitleLoopExit::NewGame;
                                    return;
                                case TitleMenuAction::RequestQuit:
                                    *out_exit = TitleLoopExit::QuitApp;
                                    return;
                            }
                        }
                    }
                } else if (ev.type == SDL_EVENT_MOUSE_MOTION) {
                    // Hover NATIVO (:hover RCSS, ver title_menu_rml.cpp) - MESMO
                    // pipeline de encaminhamento das demais telas. Sem SFX aqui
                    // (a tela de titulo nao recebe AudioEngine - fora do escopo
                    // desta dispatch, ver TODO.md).
                    glintfx::UiEvent hover_ev{};
                    hover_ev.type = glintfx::UiEvent::Type::MouseMove;
                    hover_ev.x = ev.motion.x;
                    hover_ev.y = ev.motion.y;
                    ui.process_event(hover_ev);
                }
            }
            present_frame();
        }
    };

    run_body();
    SDL_GL_DestroyContext(gl);
    return true;
}

}  // namespace gus::app::screens
