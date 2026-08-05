// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/rml_escape_test.cpp
//
// Catch2 (TEST-FIRST) de rml_escape (I18N-ESCAPE-MARKUP) + a costura ponta-a-ponta
// dela nas 5 telas que injetam texto EXTERNO no RML que elas mesmas montam.
//
// O ACHADO QUE ORIGINOU (2026-08-04): o parser de RML proprio do glintfx recebeu 15
// fixtures com o markup final destas telas e rejeitou UMA - o '&' literal de
// "Menu & Diálogo" (CONTROLS_GROUP_MENU_DIALOGUE) - com "malformed entity reference:
// '&' not closed by a terminating ';'". O RmlUi de hoje tolera, entao nunca houve bug
// visivel; o que se fecha aqui e o risco FUTURO (traducao en-intl planejada: "Save &
// Exit", "A < B").
//
// ESTRUTURA (4 blocos):
//   (1) unidade: a ORDEM ('&' primeiro) e os 3 metacaracteres;
//   (2) o caso REAL: "Menu & Diálogo" chegando escapado na tela de Controles;
//   (3) ponta-a-ponta por tela, com catalogo HOSTIL (todo valor com '<a & b>');
//   (4) GUARDA ENUMERANTE: varre TODO '&' do markup gerado e exige que cada um abra
//       uma entidade bem formada - a MESMA regra que o parser estrito do glintfx
//       aplicou. Enumerar em vez de procurar o '&' que eu suspeito e o que faz esta
//       guarda pegar um call-site que alguem esquecer de escapar no futuro.

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string>
#include <vector>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/screens/difficulty_menu.hpp"
#include "gus/app/screens/difficulty_menu_rml.hpp"
#include "gus/app/screens/npc_dialogue_rml.hpp"
#include "gus/app/screens/rml_escape.hpp"
#include "gus/app/screens/save_load_menu.hpp"
#include "gus/app/screens/save_load_menu_rml.hpp"
#include "gus/app/screens/system_menu.hpp"
#include "gus/app/screens/system_menu_rml.hpp"
#include "gus/app/screens/title_menu.hpp"
#include "gus/app/screens/title_menu_rml.hpp"
#include "gus/domain/dialogue/dialogue_graph.hpp"

using namespace gus::app::screens;
using gus::app::i18n::Translator;
using gus::domain::dialogue::DialogueNode;
using gus::domain::dialogue::DialogueOption;
using gus::domain::save::kAutosaveSlot;
using gus::domain::save::kSlotCount;
using gus::domain::save::SaveData;

// ============================================================ (1) UNIDADE

TEST_CASE("rml_escape: '&' vira &amp;, '<' vira &lt;, '>' vira &gt; (os 3 sozinhos)",
          "[rml_escape]") {
    REQUIRE(rml_escape("&") == "&amp;");
    REQUIRE(rml_escape("<") == "&lt;");
    REQUIRE(rml_escape(">") == "&gt;");
}

// A ARMADILHA PRINCIPAL da fatia. Se '<' fosse trocado ANTES de '&', o '&' que o
// proprio escape introduz ao escrever "&lt;" seria reescapado na passada do '&' e
// sairia "&amp;lt;" - texto quebrado na tela, e o mutante mais obvio desta funcao.
TEST_CASE("rml_escape: a ORDEM importa - '<a & b>' vira '&lt;a &amp; b&gt;', "
          "NUNCA '&amp;lt;' (o '&' tem de ser substituido PRIMEIRO)",
          "[rml_escape]") {
    const std::string out = rml_escape("<a & b>");
    REQUIRE(out == "&lt;a &amp; b&gt;");
    REQUIRE(out.find("&amp;lt;") == std::string::npos);
    REQUIRE(out.find("&amp;gt;") == std::string::npos);
}

TEST_CASE("rml_escape: o caso REAL do catalogo - 'Menu & Diálogo' "
          "(CONTROLS_GROUP_MENU_DIALOGUE, o valor que o parser do glintfx rejeitou)",
          "[rml_escape]") {
    REQUIRE(rml_escape("Menu & Diálogo") == "Menu &amp; Diálogo");
}

TEST_CASE("rml_escape: texto sem metacaractere passa INTACTO (o caso comum - 343 dos "
          "344 valores do pt_br.md)",
          "[rml_escape]") {
    REQUIRE(rml_escape("") == "");
    REQUIRE(rml_escape("Continuar") == "Continuar");
    REQUIRE(rml_escape("Cima/Baixo navega - Enter seleciona") ==
            "Cima/Baixo navega - Enter seleciona");
}

TEST_CASE("rml_escape: bytes UTF-8 acentuados atravessam sem corrupcao (o catalogo "
          "dev e pt-br: 'Opções', 'Configurações')",
          "[rml_escape]") {
    REQUIRE(rml_escape("Opções") == "Opções");
    REQUIRE(rml_escape("Configurações & Diálogo") == "Configurações &amp; Diálogo");
}

TEST_CASE("rml_escape: '{0}' e os marcadores de interpolacao NAO sao metacaracteres "
          "e passam intactos (escapar DEPOIS de interpolar continua valido)",
          "[rml_escape]") {
    REQUIRE(rml_escape("{0} confirma, {1} volta ao jogo") ==
            "{0} confirma, {1} volta ao jogo");
}

TEST_CASE("rml_escape: multiplas ocorrencias do MESMO metacaractere, todas trocadas",
          "[rml_escape]") {
    REQUIRE(rml_escape("A & B & C") == "A &amp; B &amp; C");
    REQUIRE(rml_escape("<<>>") == "&lt;&lt;&gt;&gt;");
}

// DOCUMENTA (nao deseja) a NAO-idempotencia: escapar duas vezes produz "&amp;amp;".
// Isto e proposital - o que impede o duplo escape NAO e a funcao (que nao tem como
// distinguir "&" de texto de um "&" que ela mesma escreveu), e o PONTO UNICO de
// aplicacao: a string externa passa por rml_escape UMA vez, no instante em que entra
// no ostringstream. Se um dia esta assercao "falhar" porque alguem tornou a funcao
// idempotente, leia o header: idempotencia MASCARARIA um call-site escapando 2x.
TEST_CASE("rml_escape: NAO e idempotente por design - escapar 2x da '&amp;amp;'; o "
          "que impede o duplo escape e o PONTO UNICO de aplicacao, nao a funcao",
          "[rml_escape]") {
    REQUIRE(rml_escape(rml_escape("&")) == "&amp;amp;");
    REQUIRE(rml_escape(rml_escape("<")) == "&amp;lt;");
}

// ================================================== helpers do ponta-a-ponta

namespace {

// Valor HOSTIL injetado em todo o catalogo: exercita os 3 metacaracteres de uma vez
// e, de quebra, a ordem (o '<' vem antes do '&' no texto).
constexpr const char* kHostile = "<a & b>";
constexpr const char* kHostileEscaped = "&lt;a &amp; b&gt;";

// GUARDA ENUMERANTE (bloco 4): devolve os '&' do markup que NAO abrem uma entidade
// bem formada - exatamente a queixa do parser estrito do glintfx ("'&' not closed by
// a terminating ';'"). Aceita nomeada (&amp;), decimal (&#62;) e hexadecimal (&#x3E;).
// Varre TODO '&' da string: nao procura o caso que eu suspeito, enumera o conjunto
// fechado.
std::vector<std::string> stray_ampersands(const std::string& rml) {
    std::vector<std::string> bad;
    for (std::size_t i = 0; i < rml.size(); ++i) {
        if (rml[i] != '&') continue;
        std::size_t j = i + 1;
        if (j < rml.size() && rml[j] == '#') {
            ++j;
            if (j < rml.size() && (rml[j] == 'x' || rml[j] == 'X')) ++j;
        }
        const std::size_t name_start = j;
        while (j < rml.size() && (std::isalnum(static_cast<unsigned char>(rml[j])) != 0)) ++j;
        const bool well_formed = (j > name_start) && (j < rml.size()) && (rml[j] == ';');
        if (!well_formed) {
            bad.push_back(rml.substr(i, std::min<std::size_t>(24, rml.size() - i)));
        }
    }
    return bad;
}

// Catalogo em que TODA chave consumida pelas telas resolve para o valor hostil. Nao
// da pra "carregar tudo hostil" de uma vez (o parser e KEY->valor), entao a lista
// enumera as chaves REAIS de cada tela - mesma tecnica das fixtures ja existentes em
// system_menu_rml_test.cpp/save_load_menu_rml_test.cpp, so que com o valor venenoso.
Translator make_hostile_translator(const std::vector<const char*>& keys) {
    std::string md;
    for (const char* k : keys) {
        md += "## ";
        md += k;
        md += "\n";
        md += kHostile;
        md += "\n\n";
    }
    Translator tr;
    tr.load_from_content(md);
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

std::array<SaveSlotPreview, kSlotCount> make_slots() {
    std::array<SaveSlotPreview, kSlotCount> slots{};
    slots[kAutosaveSlot] = build_slot_preview(make_save_data(550), kAutosaveSlot);
    slots[1] = build_slot_preview(make_save_data(340), 1);
    for (int i = 2; i < kSlotCount; ++i) {
        slots[static_cast<std::size_t>(i)] = empty_slot_preview(i);
    }
    return slots;
}

// Bateria comum: o texto hostil NAO pode aparecer cru, TEM de aparecer escapado, e
// NUNCA pode sair duplo-escapado.
void require_escaped_and_clean(const std::string& rml) {
    INFO("markup gerado:\n" << rml);
    REQUIRE(rml.find(kHostile) == std::string::npos);
    REQUIRE(rml.find(kHostileEscaped) != std::string::npos);
    REQUIRE(rml.find("&amp;lt;") == std::string::npos);
    REQUIRE(rml.find("&amp;amp;") == std::string::npos);
    const std::vector<std::string> bad = stray_ampersands(rml);
    INFO("'&' que nao abrem entidade: " << bad.size());
    for (const std::string& s : bad) {
        INFO("  -> " << s);
    }
    REQUIRE(bad.empty());
}

}  // namespace

// ==================================== (2) o caso REAL, na tela de Controles

TEST_CASE("build_system_menu_rml (Controles): o rotulo REAL 'Menu & Diálogo' sai "
          "como 'Menu &amp; Diálogo' - o markup que o parser estrito do glintfx "
          "rejeitou nao se reproduz mais",
          "[rml_escape][system_menu_rml]") {
    SystemMenuState state;
    system_menu_open(state);
    state.screen = SystemMenuScreen::Controls;

    Translator tr;
    tr.load_from_content(
        "## MENU_SYSTEM_KICKER\nSistema\n\n"
        "## SETTINGS_CONTROLS\nControles\n\n"
        "## CONTROLS_GROUP_MENU_DIALOGUE\nMenu & Diálogo\n\n");

    const std::string rml = build_system_menu_rml(state, tr);
    INFO("markup gerado:\n" << rml);
    REQUIRE(rml.find("Menu &amp; Diálogo") != std::string::npos);
    REQUIRE(rml.find("Menu & Diálogo") == std::string::npos);
    REQUIRE(stray_ampersands(rml).empty());
}

// ============================ (3) + (4) ponta-a-ponta HOSTIL, tela por tela

TEST_CASE("build_title_menu_rml: catalogo hostil ('<a & b>' em toda chave) sai "
          "inteiramente escapado, sem '&' solto",
          "[rml_escape][title_menu_rml]") {
    const Translator tr = make_hostile_translator(
        {"MENU_CONTINUE", "MENU_NEW_GAME", "MENU_QUIT", "TITLE_LOGO_PREFIX",
         "TITLE_LOGO_SUFFIX", "TITLE_SUBTITLE", "TITLE_FOOTER_HINT",
         "TITLE_NEW_GAME_CONFIRM", "TITLE_NEW_GAME_CONFIRM_YES",
         "TITLE_NEW_GAME_CONFIRM_NO"});

    TitleMenuState state;
    title_menu_open(state, /*has_any_save=*/true);
    require_escaped_and_clean(build_title_menu_rml(state, tr));

    // O mini-dialogo de Novo Jogo e um SEGUNDO ponto de retorno da funcao (substitui
    // a lista inteira) - se so a lista fosse escapada, este caminho passaria batido.
    state.confirming_new_game = true;
    require_escaped_and_clean(build_title_menu_rml(state, tr));
}

TEST_CASE("build_difficulty_menu_rml: catalogo hostil sai inteiramente escapado, "
          "lista E splash de confirmacao",
          "[rml_escape][difficulty_menu_rml]") {
    const Translator tr = make_hostile_translator(
        {"SAVE_DIFFICULTY_TITLE", "SAVE_DIFFICULTY_HINT", "SAVE_DIFFICULTY_FOOTER_HINT",
         "SAVE_DIFFICULTY_FACIL_LABEL", "SAVE_DIFFICULTY_FACIL_DESC",
         "SAVE_DIFFICULTY_MEDIO_LABEL", "SAVE_DIFFICULTY_MEDIO_DESC",
         "SAVE_DIFFICULTY_MEDIO_BADGE", "SAVE_DIFFICULTY_DIFICIL_LABEL",
         "SAVE_DIFFICULTY_DIFICIL_DESC", "SAVE_DIFFICULTY_HARDCORE_LABEL",
         "SAVE_DIFFICULTY_HARDCORE_DESC_LOCKED", "SAVE_DIFFICULTY_CONFIRM_BODY",
         "SAVE_DIFFICULTY_CONFIRM_NO", "SAVE_DIFFICULTY_CONFIRM_TITLE_MEDIO",
         "SAVE_DIFFICULTY_CONFIRM_YES_MEDIO"});

    DifficultyMenuState state;
    difficulty_menu_open(state, /*hardcore_unlocked=*/false);
    require_escaped_and_clean(build_difficulty_menu_rml(state, tr));

    state.confirming = true;
    require_escaped_and_clean(build_difficulty_menu_rml(state, tr));
}

TEST_CASE("build_save_load_menu_rml: catalogo hostil sai inteiramente escapado, "
          "inclusive nos rotulos INTERPOLADOS ({0} -> numero do slot)",
          "[rml_escape][save_load_menu_rml]") {
    const Translator tr = make_hostile_translator(
        {"SAVE_SCREEN_TITLE_SAVE", "SAVE_SCREEN_TITLE_LOAD", "SAVE_SCREEN_SUBTITLE_SAVE",
         "SAVE_SCREEN_SUBTITLE_LOAD", "SAVE_SCREEN_FOOTER_SAVE", "SAVE_SCREEN_FOOTER_LOAD",
         "SAVE_SLOT_EMPTY", "SAVE_SLOT_LABEL", "SAVE_SLOT_AUTO_NAME",
         "SAVE_SLOT_READONLY_TAG", "SAVE_XP_LABEL", "SAVE_CHAPTER_LABEL",
         "SAVE_DELETE_BUTTON_LABEL", "SETTINGS_BACK", "LOCATION_PRACA_COMPILACAO",
         "LOCATION_UNKNOWN", "SAVE_CONFIRM_OVERWRITE", "SAVE_OVERWRITE_CONFIRM_YES",
         "SAVE_OVERWRITE_CONFIRM_NO", "SAVE_CONFIRM_DELETE", "SAVE_DELETE_CONFIRM_YES",
         "SAVE_DELETE_CONFIRM_NO", "SAVE_LOAD_WARN_DAMAGED", "SAVE_LOAD_RECOVER_TRY",
         "SAVE_LOAD_WARN_CANCEL"});

    SaveLoadMenuState state;
    save_load_menu_open(state, SaveLoadMode::Save, make_slots());
    require_escaped_and_clean(build_save_load_menu_rml(state, tr));

    // Os 3 mini-dialogos sao pontos de retorno PROPRIOS (cada um substitui a lista).
    state.confirming_overwrite = true;
    require_escaped_and_clean(build_save_load_menu_rml(state, tr));
    state.confirming_overwrite = false;

    state.confirming_delete = true;
    require_escaped_and_clean(build_save_load_menu_rml(state, tr));
    state.confirming_delete = false;

    state.warning_kind = SaveLoadMenuState::WarningKind::Damaged;
    require_escaped_and_clean(build_save_load_menu_rml(state, tr));
}

TEST_CASE("build_system_menu_rml: catalogo hostil sai escapado nas 6 telas do menu "
          "de sistema (Pause/Config/Audio/Controles/Video/Language)",
          "[rml_escape][system_menu_rml]") {
    const Translator tr = make_hostile_translator(
        {"MENU_SYSTEM_KICKER", "MENU_PAUSE_TITLE", "MENU_PAUSE_HINT", "MENU_CONTINUE",
         "MENU_SAVE_GAME", "MENU_LOAD_GAME", "SETTINGS_TITLE", "MENU_TO_TITLE",
         "MENU_QUIT", "MENU_TO_TITLE_CONFIRM_TITLE", "MENU_TO_TITLE_CONFIRM_YES",
         "MENU_TO_TITLE_CONFIRM_NO", "SETTINGS_AUDIO", "SETTINGS_VIDEO",
         "SETTINGS_LANGUAGE", "SETTINGS_CONTROLS", "SETTINGS_BACK",
         "SETTINGS_MUSIC_VOLUME", "SETTINGS_SFX_VOLUME", "MENU_PLACEHOLDER_TEXT",
         "SETTINGS_RESET_DEFAULTS", "SETTINGS_APPLY", "CONTROLS_COL_ACTION",
         "CONTROLS_COL_KEYBOARD", "CONTROLS_COL_GAMEPAD", "CONTROLS_NAV_HINT",
         "CONTROLS_NO_BINDING", "CONTROLS_CAPTURE_PROMPT", "CONTROLS_GROUP_MOVEMENT",
         "CONTROLS_GROUP_WORLD", "CONTROLS_GROUP_COMBAT", "CONTROLS_GROUP_MENU_DIALOGUE",
         "CONTROLS_RESTORE_CONFIRM_TITLE", "CONTROLS_RESTORE_CONFIRM_YES",
         "CONTROLS_RESTORE_CONFIRM_NO", "CONTROLS_DISCARD_CONFIRM_TITLE",
         "CONTROLS_DISCARD_CONFIRM_YES", "CONTROLS_DISCARD_CONFIRM_NO",
         "CONTROLS_SWAP_NOTICE"});

    SystemMenuState state;
    system_menu_open(state);

    const std::array<SystemMenuScreen, 6> screens = {
        SystemMenuScreen::Pause,       SystemMenuScreen::ConfigCategories,
        SystemMenuScreen::Audio,       SystemMenuScreen::Controls,
        SystemMenuScreen::Video,       SystemMenuScreen::Language,
    };
    for (const SystemMenuScreen s : screens) {
        state.screen = s;
        require_escaped_and_clean(build_system_menu_rml(state, tr));
    }

    // Os 2 mini-dialogos da tela Controles sao pontos de retorno PROPRIOS.
    state.screen = SystemMenuScreen::Controls;
    state.controls_confirming_restore = true;
    require_escaped_and_clean(build_system_menu_rml(state, tr));
    state.controls_confirming_restore = false;

    state.controls_confirming_discard = true;
    require_escaped_and_clean(build_system_menu_rml(state, tr));
    state.controls_confirming_discard = false;

    // Aviso de troca de tecla: CONTROLS_SWAP_NOTICE interpola OUTRA string traduzida
    // ("{0}" -> rotulo da outra action). As DUAS partes tem de sair escapadas, e o
    // resultado composto NAO pode ser escapado 2x.
    state.controls_last_action_swapped = true;
    state.controls_last_swapped_with_label_key = "CONTROLS_GROUP_COMBAT";
    require_escaped_and_clean(build_system_menu_rml(state, tr));
}

TEST_CASE("build_npc_dialogue_rml: catalogo hostil sai escapado no no LINEAR e no "
          "no de ESCOLHAS - e as entidades DECORATIVAS ('&gt; ' do cursor, "
          "'&nbsp;' do recuo) continuam intactas, nao viram literal",
          "[rml_escape][npc_dialogue_rml]") {
    const Translator tr = make_hostile_translator(
        {"ACTOR_BERTOLDO", "DIALOGUE_CONTINUE", "DIALOGUE_LINE", "DIALOGUE_OPT_A",
         "DIALOGUE_OPT_B"});

    DialogueNode linear;
    linear.id = "n0";
    linear.speaker_id = "bertoldo";
    linear.text_key = "DIALOGUE_LINE";
    require_escaped_and_clean(build_npc_dialogue_rml(
        linear, tr, /*selected_option=*/-1, "retrato_seu_bertoldo_caim.png"));

    DialogueNode choices;
    choices.id = "n1";
    choices.speaker_id = "bertoldo";
    choices.text_key = "DIALOGUE_LINE";
    choices.options = {DialogueOption{"DIALOGUE_OPT_A", "n2a", std::nullopt},
                       DialogueOption{"DIALOGUE_OPT_B", "n2b", std::nullopt}};
    const std::string rml =
        build_npc_dialogue_rml(choices, tr, /*selected_option=*/0, "retrato_gus.png");
    require_escaped_and_clean(rml);

    // As 2 entidades que NOS escrevemos a mao no gerador continuam la. Se o escape
    // fosse aplicado a saida montada (em vez do texto que entra), elas teriam virado
    // "&amp;gt;"/"&amp;nbsp;" e o jogador veria "&gt;" literal na tela.
    REQUIRE(rml.find("&gt; ") != std::string::npos);
    REQUIRE(rml.find("&nbsp;") != std::string::npos);
    REQUIRE(rml.find("&amp;gt;") == std::string::npos);
    REQUIRE(rml.find("&amp;nbsp;") == std::string::npos);
}

// Texto de DIALOGO tambem e conteudo EXTERNO: speaker_id e text_key vem de
// resources/dialogues/*.dlg.txt, sem restricao de charset no parser
// (dialogue_text.cpp le "speaker: <val>" cru). Os dois chegam ao markup pelos
// caminhos de FALLBACK: speaker_id quando a chave ACTOR_<ID> nao existe no catalogo,
// e text_key quando a chave da fala nao existe (Translator::tr devolve a PROPRIA
// chave). Este teste exercita justamente os fallbacks - catalogo VAZIO de proposito.
TEST_CASE("build_npc_dialogue_rml: speaker_id e text_key vindos do .dlg.txt saem "
          "escapados quando caem no FALLBACK (catalogo sem a chave)",
          "[rml_escape][npc_dialogue_rml]") {
    Translator empty;  // nenhuma chave: tr(k) devolve k, e o ator cai no speaker_id

    DialogueNode node;
    node.id = "n0";
    node.speaker_id = "Mae & Filho";       // como um autor escreveria no .dlg.txt
    node.text_key = "FALA_<NAO_EXISTE>";   // chave ausente -> vaza crua pro markup

    const std::string rml =
        build_npc_dialogue_rml(node, empty, /*selected_option=*/-1, "retrato_x.png");
    INFO("markup gerado:\n" << rml);
    REQUIRE(rml.find("Mae &amp; Filho") != std::string::npos);
    REQUIRE(rml.find("Mae & Filho") == std::string::npos);
    REQUIRE(rml.find("FALA_&lt;NAO_EXISTE&gt;") != std::string::npos);
    REQUIRE(stray_ampersands(rml).empty());
}
