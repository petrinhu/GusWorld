// GusEngine/app/tests/save_load_menu_rml_test.cpp
//
// Catch2 (TEST-FIRST) de build_save_load_menu_rml (SAVE-LOAD-UI). SO checagem
// ESTRUTURAL da string RML gerada (ids presentes, rotulos traduzidos
// substituidos, slot focado marcado, slot vazio/autosave/ocupado com a classe
// certa, mini-dialogo de sobrescrita) - MESMO espirito de
// system_menu_rml_test.cpp (nao valida pixel/renderizacao real).

#include <catch2/catch_test_macros.hpp>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/save_load_menu.hpp"
#include "gus/app/screens/save_load_menu_rml.hpp"

using namespace gus::app::screens;
using gus::app::i18n::Translator;
using gus::domain::save::kAutosaveSlot;
using gus::domain::save::kSlotCount;
using gus::domain::save::SaveData;

namespace {

Translator make_translator() {
    Translator tr;
    tr.load_from_content(
        "## SAVE_SCREEN_TITLE_SAVE\nSalvar\n\n"
        "## SAVE_SCREEN_TITLE_LOAD\nCarregar\n\n"
        "## SAVE_SCREEN_SUBTITLE_SAVE\n{0} espacos\n\n"
        "## SAVE_SCREEN_SUBTITLE_LOAD\n{0} espacos\n\n"
        "## SAVE_SCREEN_FOOTER_SAVE\nEnter grava\n\n"
        "## SAVE_SCREEN_FOOTER_LOAD\nEnter carrega\n\n"
        "## SAVE_SLOT_EMPTY\nEspaco {0} - vazio\n\n"
        "## SAVE_SLOT_LABEL\nEspaco {0}\n\n"
        "## SAVE_SLOT_AUTO_NAME\nAuto\n\n"
        "## SAVE_SLOT_READONLY_TAG\n(so-leitura)\n\n"
        "## SAVE_XP_LABEL\nXP {0}\n\n"
        "## SAVE_CHAPTER_LABEL\nCap. {0}\n\n"
        "## SAVE_CONFIRM_OVERWRITE\nSobrescrever este slot?\n\n"
        "## SAVE_OVERWRITE_CONFIRM_YES\nSim, sobrescrever\n\n"
        "## SAVE_OVERWRITE_CONFIRM_NO\nCancelar\n\n"
        "## SETTINGS_BACK\nVoltar\n\n"
        "## LOCATION_PRACA_COMPILACAO\nPraca da Compilacao\n\n"
        "## LOCATION_UNKNOWN\nLocal desconhecido\n\n");
    return tr;
}

SaveData make_save_data(int gus_xp) {
    SaveData data;
    data.timestamp_ms = 1751918040000LL;
    data.playtime_seconds = 2532.0;
    data.current_scene_path = "distritos_inferiores";
    data.character_states["gus"].xp = gus_xp;
    return data;
}

std::array<SaveSlotPreview, kSlotCount> make_slots_mock_like() {
    std::array<SaveSlotPreview, kSlotCount> slots{};
    slots[kAutosaveSlot] = build_slot_preview(make_save_data(550), kAutosaveSlot);
    slots[1] = build_slot_preview(make_save_data(340), 1);
    for (int i = 2; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    return slots;
}

}  // namespace

TEST_CASE("build_save_load_menu_rml: modo Save mostra titulo Salvar e rotulo do slot 1 "
          "focado",
          "[save_load_menu_rml]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Save, make_slots_mock_like());

    const std::string rml = build_save_load_menu_rml(state, make_translator());
    REQUIRE(rml.find("Salvar") != std::string::npos);
    REQUIRE(rml.find("id=\"slmenu-slot-1\"") != std::string::npos);
    REQUIRE(rml.find("XP 340") != std::string::npos);
    REQUIRE(rml.find("Cap. 1") != std::string::npos);
    REQUIRE(rml.find("Praca da Compilacao") != std::string::npos);
}

TEST_CASE("build_save_load_menu_rml: modo Save marca o autosave como so-leitura",
          "[save_load_menu_rml]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Save, make_slots_mock_like());

    const std::string rml = build_save_load_menu_rml(state, make_translator());
    REQUIRE(rml.find("(so-leitura)") != std::string::npos);
    // slot autosave tem classe "readonly".
    const auto pos = rml.find("id=\"slmenu-slot-0\"");
    REQUIRE(pos != std::string::npos);
    const auto div_start = rml.rfind("<div class=\"", pos);
    REQUIRE(div_start != std::string::npos);
    REQUIRE(rml.substr(div_start, pos - div_start).find("readonly") != std::string::npos);
}

TEST_CASE("build_save_load_menu_rml: modo Load mostra titulo Carregar e slot vazio com "
          "rotulo 'vazio'",
          "[save_load_menu_rml]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Load, make_slots_mock_like());

    const std::string rml = build_save_load_menu_rml(state, make_translator());
    REQUIRE(rml.find("Carregar") != std::string::npos);
    REQUIRE(rml.find("vazio") != std::string::npos);
}

TEST_CASE("build_save_load_menu_rml: mini-dialogo de sobrescrita substitui a lista",
          "[save_load_menu_rml]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Save, make_slots_mock_like());
    save_load_menu_key_down(state, SDLK_RETURN);  // slot 1 ocupado -> abre o dialogo
    REQUIRE(state.confirming_overwrite);

    const std::string rml = build_save_load_menu_rml(state, make_translator());
    REQUIRE(rml.find("Sobrescrever este slot?") != std::string::npos);
    REQUIRE(rml.find("id=\"slmenu-confirm-yes\"") != std::string::npos);
    REQUIRE(rml.find("id=\"slmenu-confirm-no\"") != std::string::npos);
    // default seguro: "Nao" focado.
    const auto pos_no = rml.find("id=\"slmenu-confirm-no\"");
    const auto div_start = rml.rfind("<div class=\"", pos_no);
    REQUIRE(rml.substr(div_start, pos_no - div_start).find("focused") != std::string::npos);
}

TEST_CASE("build_save_load_menu_rml: pressed_index marca o slot certo com a classe "
          "'pressed'",
          "[save_load_menu_rml]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Save, make_slots_mock_like());

    const std::string rml = build_save_load_menu_rml(state, make_translator(), /*pressed_index=*/1);
    const auto pos = rml.find("id=\"slmenu-slot-1\"");
    REQUIRE(pos != std::string::npos);
    const auto div_start = rml.rfind("<div class=\"", pos);
    REQUIRE(rml.substr(div_start, pos - div_start).find("pressed") != std::string::npos);
}
