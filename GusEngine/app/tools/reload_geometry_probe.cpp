// SPDX-License-Identifier: Apache-2.0
// PROBE EFEMERO - investiga se scroll_element_into_view desloca a lista mesmo
// quando o item ja esta visivel, ou se o desvio vem so do reload_()/re-load do
// documento (achado do team-lead, 2026-08-01: -6px medido, precisa separar a
// causa).
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <SDL3/SDL.h>
#include <glintfx/ui_layer.hpp>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/save_load_menu.hpp"
#include "gus/app/screens/save_load_menu_rml.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"

namespace fs = std::filesystem;

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) { std::cerr << "SDL_Init falhou\n"; return 1; }
    int kW = 960, kH = 540;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_Window* window = SDL_CreateWindow("reload_geometry_probe", kW, kH, SDL_WINDOW_OPENGL);
    if (!window) { std::cerr << "CreateWindow falhou\n"; return 1; }
    SDL_GLContext gl = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl);
    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        std::cerr << "gl3_load_functions falhou\n"; return 1;
    }

    gus::app::i18n::Translator translator;
    const std::string tr_path = gus::app::i18n::resolve_translations_path();
    translator.load_from_file(tr_path);

    using gus::app::screens::SaveLoadMenuState;
    using gus::app::screens::SaveLoadMode;
    using gus::app::screens::SaveSlotPreview;
    using gus::domain::save::kAutosaveSlot;
    using gus::domain::save::kSlotCount;

    gus::domain::save::SaveData d1, d2;
    d1.current_scene_path = "distritos_inferiores"; d1.timestamp_ms = 1783455240000LL; d1.playtime_seconds = 2532.0;
    d2 = d1;

    std::array<SaveSlotPreview, kSlotCount> slots{};
    slots[kAutosaveSlot] = gus::app::screens::build_slot_preview(d1, kAutosaveSlot);
    slots[1] = gus::app::screens::build_slot_preview(d2, 1);
    for (int i = 2; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = gus::app::screens::empty_slot_preview(i);

    SaveLoadMenuState state;
    gus::app::screens::save_load_menu_open(state, SaveLoadMode::Load, slots);
    state.selected = 1;  // slot 1 JA selecionado/focado - "ja esta visivel"

    const fs::path stage = fs::temp_directory_path() / "gusworld_reload_geometry_probe";
    std::error_code ec;
    fs::create_directories(stage, ec);
    std::string fonts_dir = GUSWORLD_FONTS_DIR;
    if (const char* envf = std::getenv("GUSWORLD_FONTS")) if (envf[0] != '\0') fonts_dir = envf;
    fs::copy_file(fonts_dir + "/PixelOperatorMono.ttf", stage / "PixelOperatorMono.ttf", fs::copy_options::overwrite_existing, ec);
    fs::copy_file(fonts_dir + "/PixelOperatorMono-Bold.ttf", stage / "PixelOperatorMono-Bold.ttf", fs::copy_options::overwrite_existing, ec);

    auto write_rml = [&](const SaveLoadMenuState& s) {
        std::string rml = gus::app::screens::build_save_load_menu_rml(s, translator);
        const fs::path out = stage / "save_load_menu.rml";
        std::ofstream f(out);
        f << rml;
        return out.string();
    };

    const float dp_ratio = static_cast<float>(kW) / 960.0f;
    glintfx::UiLayer ui(glintfx::UiLayer::Config{960, 540, true, dp_ratio});
    if (!ui.ok()) { std::cerr << "UiLayer::ok()=false\n"; return 1; }
    ui.set_asset_base_url(stage.string().c_str());
    ui.load_font_face(glintfx::FontFaceDesc{"PixelOperatorMono.ttf", "Pixel Operator Mono"});
    ui.load_font_face(glintfx::FontFaceDesc{"PixelOperatorMono-Bold.ttf", "Pixel Operator Mono",
                                             glintfx::FontStyle::Normal, glintfx::FontWeight::Bold});
    ui.load(write_rml(state).c_str());
    ui.set_viewport(kW, kH);
    ui.set_dp_ratio(dp_ratio);
    ui.update();

    auto print_box = [&](const char* label) {
        const glintfx::ElementBox box = ui.get_element_box("slmenu-slot-1");
        std::cout << label << ": slot-1 box y=" << box.y << " (found=" << box.found << ")\n";
    };

    print_box("[0] apos load()+update() inicial");

    // [A] RELOAD PURO (recarrega o MESMO documento, SEM chamar
    // scroll_element_into_view nenhuma vez) - isola se o desvio vem so de
    // recarregar o RML.
    ui.load(write_rml(state).c_str());
    ui.set_viewport(kW, kH);
    ui.set_dp_ratio(dp_ratio);
    ui.update();
    print_box("[A] apos reload PURO (sem scroll_element_into_view)");

    // [B] Agora chama scroll_element_into_view pro slot 1 (JA selecionado,
    // "ja esta visivel" segundo o comentario) - isola se ELE desloca algo.
    ui.scroll_element_into_view("slmenu-slot-1");
    print_box("[B] apos scroll_element_into_view(slot-1), item JA selecionado");

    // [C] outro reload + scroll_element_into_view, pra ver se o desvio
    // ACUMULA a cada rodada (MESMO padrao do teste real, motion 1/2/3).
    ui.load(write_rml(state).c_str());
    ui.set_viewport(kW, kH);
    ui.set_dp_ratio(dp_ratio);
    ui.update();
    ui.scroll_element_into_view("slmenu-slot-1");
    print_box("[C] apos 2o (reload+scroll)");

    ui.load(write_rml(state).c_str());
    ui.set_viewport(kW, kH);
    ui.set_dp_ratio(dp_ratio);
    ui.update();
    ui.scroll_element_into_view("slmenu-slot-1");
    print_box("[D] apos 3o (reload+scroll)");

    // [E] CORRECAO (2026-08-01): reload de novo, mas agora aplicando a MESMA
    // logica condicional que entrou em SaveLoadScreen::reload_() - so chama
    // scroll_element_into_view se o item NAO estiver totalmente dentro da
    // caixa de '.slot-list'. O item ja esta em y=136 (topo, dentro da area
    // visivel) - a expectativa e a caixa NAO se mover desta vez (prova de que
    // a correcao faz "no-op quando ja visivel" ser verdade).
    ui.load(write_rml(state).c_str());
    ui.set_viewport(kW, kH);
    ui.set_dp_ratio(dp_ratio);
    ui.update();
    {
        const glintfx::ElementBox list_box = ui.get_element_box("slmenu-list");
        const glintfx::ElementBox item_box = ui.get_element_box("slmenu-slot-1");
        bool need_scroll = true;
        if (list_box.found && item_box.found) {
            const bool fully_visible = item_box.y >= list_box.y &&
                                        (item_box.y + item_box.h) <= (list_box.y + list_box.h);
            need_scroll = !fully_visible;
        }
        std::cout << "[E] geometria: list_box.y=" << list_box.y << " item_box.y=" << item_box.y
                  << " item_box.h=" << item_box.h << " need_scroll=" << need_scroll << "\n";
        if (need_scroll) ui.scroll_element_into_view("slmenu-slot-1");
    }
    print_box("[E] apos reload + logica CONDICIONAL (item ja visivel, NAO deveria mover)");

    // [F] CORRECAO, o OUTRO LADO do teste (pedido do team-lead: item FORA da
    // vista ainda tem que rolar). Preenche os 7 slots + foca o ULTIMO (fora
    // do recorte de 300dp sem rolar) - a logica CONDICIONAL tem que detectar
    // "nao esta totalmente visivel" e chamar scroll_element_into_view mesmo.
    std::array<SaveSlotPreview, kSlotCount> slots_full{};
    slots_full[kAutosaveSlot] = gus::app::screens::build_slot_preview(d1, kAutosaveSlot);
    for (int i = 1; i < kSlotCount; ++i) {
        gus::domain::save::SaveData di = d1;
        slots_full[static_cast<std::size_t>(i)] = gus::app::screens::build_slot_preview(di, i);
    }
    SaveLoadMenuState state_full;
    gus::app::screens::save_load_menu_open(state_full, SaveLoadMode::Load, slots_full);
    state_full.selected = kSlotCount - 1;  // ultimo slot, fora da vista inicial

    ui.load(write_rml(state_full).c_str());
    ui.set_viewport(kW, kH);
    ui.set_dp_ratio(dp_ratio);
    ui.update();
    {
        const std::string last_id = "slmenu-slot-" + std::to_string(kSlotCount - 1);
        const glintfx::ElementBox list_box = ui.get_element_box("slmenu-list");
        const glintfx::ElementBox item_box_before = ui.get_element_box(last_id.c_str());
        bool need_scroll = true;
        if (list_box.found && item_box_before.found) {
            const bool fully_visible =
                item_box_before.y >= list_box.y &&
                (item_box_before.y + item_box_before.h) <= (list_box.y + list_box.h);
            need_scroll = !fully_visible;
        }
        std::cout << "[F] geometria ANTES: list_box.y=" << list_box.y
                  << " list_box.h=" << list_box.h << " item_box.y=" << item_box_before.y
                  << " need_scroll=" << need_scroll << "\n";
        if (need_scroll) ui.scroll_element_into_view(last_id.c_str());
        const glintfx::ElementBox item_box_after = ui.get_element_box(last_id.c_str());
        std::cout << "[F] apos a decisao: item_box.y=" << item_box_after.y
                  << " (dentro da lista [" << list_box.y << "," << (list_box.y + list_box.h)
                  << "]? " << (item_box_after.y >= list_box.y &&
                               (item_box_after.y + item_box_after.h) <= (list_box.y + list_box.h))
                  << ")\n";
    }

    SDL_GL_DestroyContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
