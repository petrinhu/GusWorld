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
        "## SAVE_CONFIRM_EMPTY\nDeseja salvar no Espaco {0} (vazio)?\n\n"
        "## SAVE_EMPTY_CONFIRM_YES\nSalvar\n\n"
        "## SAVE_EMPTY_CONFIRM_NO\nCancelar\n\n"
        "## SAVE_CONFIRM_DELETE\nApagar este espaco?\n\n"
        "## SAVE_DELETE_CONFIRM_YES\nSim, apagar\n\n"
        "## SAVE_DELETE_CONFIRM_NO\nCancelar\n\n"
        "## SAVE_DELETE_BUTTON_LABEL\nApagar\n\n"
        "## SAVE_LOAD_WARN_DAMAGED\nEste save esta danificado.\n\n"
        "## SAVE_LOAD_WARN_VERSION\nEste save e de uma versao mais nova.\n\n"
        "## SAVE_LOAD_RECOVER_TRY\nTentar recuperar\n\n"
        "## SAVE_LOAD_RECOVER_FAILED\nNao foi possivel recuperar.\n\n"
        "## SAVE_LOAD_SLOT_DAMAGED_LABEL\n! Danificado\n\n"
        "## SAVE_LOAD_SLOT_VERSION_LABEL\n! Versao incompativel\n\n"
        "## SAVE_LOAD_WARN_CANCEL\nCancelar\n\n"
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
    // Cap. 3 (nao mais Cap. 1): SAVE-LOAD-UI etapa 6, chapter_from_quest_progress
    // agora estima o capitulo por XP quando main_story esta ausente (ver
    // save_load_menu.hpp/kChapterXpThresholds) - make_save_data() acima nao seta
    // quest_progress, entao xp=340 cai na banda [300,600) = Cap. 3.
    REQUIRE(rml.find("Cap. 3") != std::string::npos);
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
    (void)save_load_menu_key_down(state, SDLK_RETURN);  // slot 1 ocupado -> abre o dialogo
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

// AJUSTE polish playtest 2026-07-10 (decisao do lider): slot GENUINAMENTE vazio
// agora tambem abre confirming_overwrite (item 2, retoque ao vivo 2026-07-10/11) -
// mas com COPY PROPRIA (nao promete "sobrescrever" nada que nao existe).
TEST_CASE("build_save_load_menu_rml: mini-dialogo de confirmacao num slot "
          "GENUINAMENTE VAZIO usa a copy do VAZIO ('Deseja salvar...'), NAO a de "
          "sobrescrita, e mesmo assim usa os MESMOS ids fixos slmenu-confirm-yes/no",
          "[save_load_menu_rml]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Save, make_slots_mock_like());
    state.selected = 2;  // slot 2: GENUINAMENTE vazio em make_slots_mock_like()
    (void)save_load_menu_key_down(state, SDLK_RETURN);
    REQUIRE(state.confirming_overwrite);

    const std::string rml = build_save_load_menu_rml(state, make_translator());
    REQUIRE(rml.find("Deseja salvar no Espaco 2 (vazio)?") != std::string::npos);
    // A copy de OVERWRITE NAO deve vazar pro caso vazio.
    REQUIRE(rml.find("Sobrescrever este slot?") == std::string::npos);
    REQUIRE(rml.find("Sim, sobrescrever") == std::string::npos);
    // Botao "Sim" do vazio e "Salvar" (nao "Sim, sobrescrever") - MESMO id fixo
    // slmenu-confirm-yes (save_load_menu_loop.cpp roteia por id, nao por copy).
    REQUIRE(rml.find("id=\"slmenu-confirm-yes\"") != std::string::npos);
    REQUIRE(rml.find("id=\"slmenu-confirm-no\"") != std::string::npos);
    const auto pos_yes = rml.find("id=\"slmenu-confirm-yes\"");
    const auto div_yes_start = rml.rfind("<div class=\"", pos_yes);
    const std::string yes_pill = rml.substr(div_yes_start, pos_yes - div_yes_start + 60);
    REQUIRE(yes_pill.find("Salvar") != std::string::npos);
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

// ---------------------------------------------------------------- feature "Apagar"

TEST_CASE("build_save_load_menu_rml: slots OCUPADOS (inclusive Auto) ganham o icone "
          "de apagar; slot vazio NAO ganha",
          "[save_load_menu_rml]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Save, make_slots_mock_like());

    const std::string rml = build_save_load_menu_rml(state, make_translator());
    REQUIRE(rml.find("id=\"slmenu-delete-0\"") != std::string::npos);  // autosave ocupado
    REQUIRE(rml.find("id=\"slmenu-delete-1\"") != std::string::npos);  // manual ocupado
    REQUIRE(rml.find("id=\"slmenu-delete-2\"") == std::string::npos);  // vazio - sem icone
}

TEST_CASE("build_save_load_menu_rml: mini-dialogo de EXCLUSAO substitui a lista "
          "(ids proprios, default seguro = Nao)",
          "[save_load_menu_rml]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Save, make_slots_mock_like());
    save_load_menu_request_delete(state, 1);
    REQUIRE(state.confirming_delete);

    const std::string rml = build_save_load_menu_rml(state, make_translator());
    REQUIRE(rml.find("Apagar este espaco?") != std::string::npos);
    REQUIRE(rml.find("id=\"slmenu-delete-confirm-yes\"") != std::string::npos);
    REQUIRE(rml.find("id=\"slmenu-delete-confirm-no\"") != std::string::npos);
    // Nao aparece a LISTA de slots enquanto o dialogo esta aberto.
    REQUIRE(rml.find("id=\"slmenu-list\"") == std::string::npos);

    const auto pos_no = rml.find("id=\"slmenu-delete-confirm-no\"");
    const auto div_start = rml.rfind("<div class=\"", pos_no);
    REQUIRE(rml.substr(div_start, pos_no - div_start).find("focused") != std::string::npos);
}

// ---------------------------------------------------------------- SAVE-LOAD-AVISOS
// (aviso #1, mock Tela 4a: warn-box/warn-btn.danger)

std::array<SaveSlotPreview, kSlotCount> make_slots_with_unreadable(
    int slot, gus::domain::save::LoadResult result) {
    std::array<SaveSlotPreview, kSlotCount> slots{};
    for (int i = 0; i < kSlotCount; ++i) slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    slots[static_cast<std::size_t>(slot)] = unreadable_slot_preview(slot, result);
    return slots;
}

TEST_CASE("build_save_load_menu_rml: modo Load rotula um slot Danificado com "
          "'! Danificado' (nao 'vazio')",
          "[save_load_menu_rml][save-load-avisos]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Load,
                         make_slots_with_unreadable(1, gus::domain::save::LoadResult::HmacInvalid));

    const std::string rml = build_save_load_menu_rml(state, make_translator());
    REQUIRE(rml.find("Danificado") != std::string::npos);
    REQUIRE(rml.find("Espaco 1 - vazio") == std::string::npos);  // NAO mostra "vazio"
}

TEST_CASE("build_save_load_menu_rml: modo Load rotula um slot de Versao "
          "incompativel com '! Versao incompativel'",
          "[save_load_menu_rml][save-load-avisos]") {
    SaveLoadMenuState state;
    save_load_menu_open(
        state, SaveLoadMode::Load,
        make_slots_with_unreadable(1, gus::domain::save::LoadResult::VersionTooNew));

    const std::string rml = build_save_load_menu_rml(state, make_translator());
    REQUIRE(rml.find("Versao incompativel") != std::string::npos);
}

TEST_CASE("build_save_load_menu_rml: modo Save NAO usa o rotulo dedicado (CRIT-1 "
          "intocado - continua mostrando 'vazio')",
          "[save_load_menu_rml][save-load-avisos]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Save,
                         make_slots_with_unreadable(1, gus::domain::save::LoadResult::HmacInvalid));

    const std::string rml = build_save_load_menu_rml(state, make_translator());
    REQUIRE(rml.find("Espaco 1 - vazio") != std::string::npos);
    REQUIRE(rml.find("Danificado") == std::string::npos);
}

TEST_CASE("build_save_load_menu_rml: aviso Damaged substitui a lista e mostra os "
          "2 botoes (Tentar recuperar + Cancelar), Cancelar focado por padrao",
          "[save_load_menu_rml][save-load-avisos]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Load,
                         make_slots_with_unreadable(1, gus::domain::save::LoadResult::Corrupt));
    (void)save_load_menu_key_down(state, SDLK_RETURN);  // abre o aviso Damaged
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::Damaged);

    const std::string rml = build_save_load_menu_rml(state, make_translator());
    REQUIRE(rml.find("Este save esta danificado.") != std::string::npos);
    REQUIRE(rml.find("id=\"slmenu-warn-recover\"") != std::string::npos);
    REQUIRE(rml.find("id=\"slmenu-warn-cancel\"") != std::string::npos);
    REQUIRE(rml.find("id=\"slmenu-list\"") == std::string::npos);  // lista NAO aparece

    const auto pos_cancel = rml.find("id=\"slmenu-warn-cancel\"");
    const auto div_start = rml.rfind("<div class=\"", pos_cancel);
    REQUIRE(rml.substr(div_start, pos_cancel - div_start).find("focused") != std::string::npos);
}

TEST_CASE("build_save_load_menu_rml: aviso Damaged - alternar pra 'Tentar "
          "recuperar' move o foco pra ele (Cancelar perde o foco)",
          "[save_load_menu_rml][save-load-avisos]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Load,
                         make_slots_with_unreadable(1, gus::domain::save::LoadResult::Corrupt));
    (void)save_load_menu_key_down(state, SDLK_RETURN);
    (void)save_load_menu_key_down(state, SDLK_LEFT);  // alterna pra "Tentar recuperar"
    REQUIRE(state.warning_selected == 0);

    const std::string rml = build_save_load_menu_rml(state, make_translator());
    const auto pos_recover = rml.find("id=\"slmenu-warn-recover\"");
    const auto div_start = rml.rfind("<div class=\"", pos_recover);
    REQUIRE(rml.substr(div_start, pos_recover - div_start).find("focused") != std::string::npos);

    const auto pos_cancel = rml.find("id=\"slmenu-warn-cancel\"");
    const auto div_start_cancel = rml.rfind("<div class=\"", pos_cancel);
    REQUIRE(rml.substr(div_start_cancel, pos_cancel - div_start_cancel).find("focused") ==
            std::string::npos);
}

TEST_CASE("build_save_load_menu_rml: aviso Version so mostra Cancelar (SEM "
          "'Tentar recuperar' - forward-only, botao nem existe)",
          "[save_load_menu_rml][save-load-avisos]") {
    SaveLoadMenuState state;
    save_load_menu_open(
        state, SaveLoadMode::Load,
        make_slots_with_unreadable(1, gus::domain::save::LoadResult::VersionTooNew));
    (void)save_load_menu_key_down(state, SDLK_RETURN);
    REQUIRE(state.warning_kind == SaveLoadMenuState::WarningKind::Version);

    const std::string rml = build_save_load_menu_rml(state, make_translator());
    REQUIRE(rml.find("Este save e de uma versao mais nova.") != std::string::npos);
    REQUIRE(rml.find("id=\"slmenu-warn-recover\"") == std::string::npos);  // botao NAO existe
    REQUIRE(rml.find("id=\"slmenu-warn-cancel\"") != std::string::npos);
}

TEST_CASE("build_save_load_menu_rml: aviso RecoverFailed mostra a mensagem de "
          "falha e so Cancelar (mesmo contrato de Version)",
          "[save_load_menu_rml][save-load-avisos]") {
    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Load,
                         make_slots_with_unreadable(1, gus::domain::save::LoadResult::Corrupt));
    (void)save_load_menu_key_down(state, SDLK_RETURN);
    // Simula o CHAMADOR transitando pra RecoverFailed apos load_game_from_backup
    // falhar (mesmo padrao ja usado por build_previews_and_cache/do_delete).
    state.warning_kind = SaveLoadMenuState::WarningKind::RecoverFailed;

    const std::string rml = build_save_load_menu_rml(state, make_translator());
    REQUIRE(rml.find("Nao foi possivel recuperar.") != std::string::npos);
    REQUIRE(rml.find("id=\"slmenu-warn-recover\"") == std::string::npos);
    REQUIRE(rml.find("id=\"slmenu-warn-cancel\"") != std::string::npos);
}
