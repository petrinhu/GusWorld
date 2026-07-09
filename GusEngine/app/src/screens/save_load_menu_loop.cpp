// gus/app/src/screens/save_load_menu_loop.cpp
//
// Implementacao do loop interativo da tela de save/load. Ver header para o
// contrato completo. GL/glintfx-heavy (mesma familia de system_menu_loop.cpp) -
// sem unidade de teste direta (a logica PURA testavel ja fica em
// save_load_menu.hpp/save_load_menu_test.cpp e save_load_menu_rml.hpp/
// save_load_menu_rml_test.cpp; este .cpp so orquestra SDL/GL + o I/O de disco
// em torno delas).

#include "gus/app/screens/save_load_menu_loop.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <glintfx/element_box.hpp>
#include <glintfx/ui_layer.hpp>

#include "gus/app/screens/save_load_menu_rml.hpp"
#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Rect
#include "gus/domain/save/save_serializer.hpp"  // LoadResult
#include "gus/platform/fs/save_file_store.hpp"  // has_save/save_game/load_game
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

// Stage PROPRIO (nao colide com o do menu de sistema, ver system_menu_loop.cpp).
std::string save_load_stage_dir() {
    return (fs::temp_directory_path() / "gusworld_glintfx_saveload").string();
}

std::string write_save_load_rml_file(const SaveLoadMenuState& state,
                                      const gus::app::i18n::Translator& tr) {
    const fs::path stage = save_load_stage_dir();
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

    std::string rml = build_save_load_menu_rml(state, tr);
    const std::string needle = "<style>\n";
    const std::size_t pos = rml.find(needle);
    if (pos != std::string::npos) {
        rml.insert(pos + needle.size(),
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "src: \"PixelOperatorMono.ttf\"; }\n"
                    "@font-face { font-family: \"Pixel Operator Mono\"; "
                    "font-weight: bold; src: \"PixelOperatorMono-Bold.ttf\"; }\n");
    }

    const fs::path out = stage / "save_load_menu.rml";
    std::ofstream f(out);
    f << rml;
    return out.string();
}

// Le TODOS os slots do disco e monta os previews + (modo Load) um cache do
// SaveData ja carregado por slot (evita ler o arquivo 2x ao confirmar). Um save
// PRESENTE mas NAO Ok (HmacInvalid/Corrupt/VersionTooNew/Invalid/WrongSlot)
// degrada, POR ORA, como slot VAZIO (os avisos dedicados sao etapa futura, fora
// do nucleo desta onda, ver TODO.md) - logado pra nao silenciar o caso.
std::array<SaveSlotPreview, gus::domain::save::kSlotCount> build_previews_and_cache(
    const std::string& saves_dir,
    std::array<std::optional<gus::domain::save::SaveData>, gus::domain::save::kSlotCount>&
        loaded_cache) {
    std::array<SaveSlotPreview, gus::domain::save::kSlotCount> previews{};
    for (int slot = 0; slot < gus::domain::save::kSlotCount; ++slot) {
        loaded_cache[static_cast<std::size_t>(slot)].reset();
        if (!gus::platform::fs::has_save(slot, saves_dir)) {
            previews[static_cast<std::size_t>(slot)] = empty_slot_preview(slot);
            continue;
        }
        const auto outcome = gus::platform::fs::load_game(slot, saves_dir);
        if (outcome.has_value() &&
            outcome->result == gus::domain::save::LoadResult::Ok) {
            previews[static_cast<std::size_t>(slot)] =
                build_slot_preview(outcome->data, slot);
            loaded_cache[static_cast<std::size_t>(slot)] = outcome->data;
        } else {
            std::cerr << "[save_load_menu_loop] aviso: slot " << slot
                      << " tem arquivo mas NAO carregou Ok (adulterado/corrompido/"
                         "versao incompativel/slot trocado) - degradando como "
                         "vazio nesta onda (avisos dedicados sao etapa futura).\n";
            previews[static_cast<std::size_t>(slot)] = empty_slot_preview(slot);
        }
    }
    return previews;
}

}  // namespace

SaveLoadLoopExit run_save_load_menu_loop_gl_current(
    SDL_Window* window, const gus::app::i18n::Translator& translator,
    SaveLoadMode mode, const std::string& saves_dir,
    const std::function<gus::domain::save::SaveData()>& build_current_save_data,
    const std::function<void(const gus::domain::save::SaveData&)>&
        apply_loaded_save_data,
    const std::string& frozen_background_png) {
    std::array<std::optional<gus::domain::save::SaveData>, gus::domain::save::kSlotCount>
        loaded_cache{};
    const std::array<SaveSlotPreview, gus::domain::save::kSlotCount> previews =
        build_previews_and_cache(saves_dir, loaded_cache);

    SaveLoadMenuState state;
    save_load_menu_open(state, mode, previews);

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
        std::cerr << "SaveLoadMenuLoop: glintfx::UiLayer::ok()=false (attach "
                     "falhou) - fechando sem desenhar (degradacao segura).\n";
        return SaveLoadLoopExit::BackToPause;
    }

    const std::string stage = save_load_stage_dir();
    ui.set_asset_base_url(stage.c_str());
    std::string rml_path = write_save_load_rml_file(state, translator);
    ui.load(rml_path.c_str());
    ui.set_viewport(pw, ph);
    ui.set_dp_ratio(dp_ratio);

    gus::platform::render2d::Render2dGl3 backdrop(/*gl_active=*/true);
    const gus::platform::render2d::TextureId frozen_bg_tex =
        frozen_background_png.empty()
            ? gus::platform::render2d::kInvalidTexture
            : backdrop.load_texture(frozen_background_png.c_str());

    auto reload = [&] {
        rml_path = write_save_load_rml_file(state, translator);
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
                cam, frozen_bg_tex, gus::platform::render2d::UvRect{0.0f, 0.0f, 1.0f, 1.0f},
                gus::platform::render2d::DrawColor{1.0f, 1.0f, 1.0f, 1.0f});
        }
        backdrop.end_frame();
        ui.update();
        ui.render();
        SDL_GL_SwapWindow(window);
    };

    // Confirma um slot em modo SAVE: pede o SaveData VIVO ao CHAMADOR (timestamp
    // fresco), grava de fato, e ATUALIZA o preview do slot NA HORA (sem fechar a
    // tela - o jogador ve o novo timestamp/playtime imediatamente).
    auto do_save = [&](int slot) {
        if (!build_current_save_data) return;  // defensivo: chamador nao forneceu
        gus::domain::save::SaveData data = build_current_save_data();
        data.slot_id = slot;
        const bool ok = gus::platform::fs::save_game(data, slot, saves_dir);
        if (!ok) {
            std::cerr << "[save_load_menu_loop] falha ao gravar slot " << slot
                      << " (I/O - disco cheio/permissao?) - estado em memoria "
                         "intocado, nada persistiu.\n";
            return;
        }
        state.slots[static_cast<std::size_t>(slot)] = build_slot_preview(data, slot);
        loaded_cache[static_cast<std::size_t>(slot)] = data;
    };

    // DIAGNOSTICO/PROVA (SAVE-LOAD-UI etapa 6, prova visual headless Xvfb :99):
    // GUSWORLD_SAVELOAD_SCREENSHOT_DIR=<dir> assenta alguns frames (bake de
    // fonte/layout - MESMA cautela de todos os outros self-tests com captura
    // desta base de codigo, ex. HOVER-SELFTEST em battle_preview.cpp) e salva 1
    // PNG (save_load_save.png OU save_load_load.png, conforme `mode`) ANTES de
    // entrar no loop interativo - bypassa por completo (nunca abre pra input
    // real, MESMO espirito de GUSWORLD_SYSMENU_HOVER_SELFTEST em
    // system_menu_loop.cpp).
    const char* screenshot_dir = std::getenv("GUSWORLD_SAVELOAD_SCREENSHOT_DIR");
    if (screenshot_dir != nullptr && screenshot_dir[0] != '\0') {
        for (int i = 0; i < 6; ++i) present_frame();
        std::vector<unsigned char> buf(static_cast<std::size_t>(pw) *
                                        static_cast<std::size_t>(ph) * 4);
        if (gus::platform::rmlui::gl3_read_backbuffer_rgba(pw, ph, buf.data())) {
            const std::string suffix = (mode == SaveLoadMode::Save) ? "save" : "load";
            const std::string out =
                join(std::string(screenshot_dir), "save_load_" + suffix + ".png");
            stbi_write_png(out.c_str(), pw, ph, 4, buf.data(), pw * 4);
            std::cout << "SaveLoadMenuLoop: [screenshot] " << out << " (" << pw << "x"
                      << ph << ")\n";
        } else {
            std::cerr << "SaveLoadMenuLoop: [screenshot] gl3_read_backbuffer_rgba "
                         "falhou\n";
        }
        return SaveLoadLoopExit::BackToPause;
    }

    while (true) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) {
                return SaveLoadLoopExit::QuitApp;
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
                const SaveLoadMenuAction action =
                    save_load_menu_key_down(state, ev.key.key);
                switch (action) {
                    case SaveLoadMenuAction::None:
                        reload();
                        break;
                    case SaveLoadMenuAction::Back:
                        return SaveLoadLoopExit::BackToPause;
                    case SaveLoadMenuAction::SlotChosen:
                        if (mode == SaveLoadMode::Save) {
                            do_save(state.selected);
                            reload();
                        } else {
                            const auto& cached =
                                loaded_cache[static_cast<std::size_t>(state.selected)];
                            if (cached.has_value() && apply_loaded_save_data) {
                                apply_loaded_save_data(*cached);
                                return SaveLoadLoopExit::ClosedAfterLoad;
                            }
                            // Defensivo: slot selecionavel em Load SEMPRE tem cache
                            // (ver build_previews_and_cache) - se nao tiver, no-op
                            // seguro (fica na lista) em vez de fingir um load.
                            std::cerr << "[save_load_menu_loop] BUG defensivo: slot "
                                      << state.selected
                                      << " selecionavel em Load sem cache - "
                                         "ignorando (nao finge um load).\n";
                            reload();
                        }
                        break;
                    case SaveLoadMenuAction::OverwriteConfirmed:
                        do_save(state.selected);
                        reload();
                        break;
                    case SaveLoadMenuAction::OverwriteCancelled:
                        reload();
                        break;
                }
            }
        }
        present_frame();
    }
}

}  // namespace gus::app::screens
